/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#if defined(__linux__)

#include "hw_queue.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h> // write return type
#include <unistd.h>    // write syscall
#include <x86intrin.h> // _mm_pause

// core-iaa
#include "hw_descriptors_api.h"

#ifdef DYNAMIC_LOADING_LIBACCEL_CONFIG
#include "hw_configuration_driver.h"
#else //DYNAMIC_LOADING_LIBACCEL_CONFIG=OFF
#include "hw_definitions.h"
#include "hw_devices.h"
#include "libaccel_config.h"
#endif //DYNAMIC_LOADING_LIBACCEL_CONFIG

#ifdef QPL_LOG_IAA_TIME
#include "util/hw_timing_util.hpp"
#endif //QPL_LOG_IAA_TIME

#define QPL_HWSTS_RET(expr, err_code)    \
    {                                    \
        if (expr) { return (err_code); } \
    }

namespace qpl::ml::dispatcher {

hw_queue::hw_queue(hw_queue&& other) noexcept
    : block_on_fault_(other.block_on_fault_)
    , priority_(other.priority_)
    , portal_mask_(other.portal_mask_)
    , portal_ptr_(other.portal_ptr_)
    , portal_offset_(0)
    , op_cfg_enabled_(other.op_cfg_enabled_)
    , op_cfg_register_(other.op_cfg_register_)
    , mmap_done_(other.mmap_done_)
    , fd_(other.fd_) {

    // to avoid close/freeing resources in the destructor twice
    other.fd_         = -1;
    other.portal_ptr_ = nullptr;
}

auto hw_queue::operator=(hw_queue&& other) noexcept -> hw_queue& {
    if (this != &other) {
        block_on_fault_  = other.block_on_fault_;
        priority_        = other.priority_;
        portal_mask_     = other.portal_mask_;
        portal_ptr_      = other.portal_ptr_;
        portal_offset_   = 0;
        op_cfg_enabled_  = other.op_cfg_enabled_;
        op_cfg_register_ = other.op_cfg_register_;
        mmap_done_       = other.mmap_done_;
        fd_              = other.fd_;

        // to avoid close/freeing resources in the destructor twice
        other.fd_         = -1;
        other.portal_ptr_ = nullptr;
    }
    return *this;
}

/**
 * @brief Destructor for the hw_queue class.
 *
 * If the hw_queue object has memory mapped using mmap, it will unmap the memory region.
 * It also closes the file descriptor associated with the hw_queue object.
 */
hw_queue::~hw_queue() noexcept {
    if (is_wq_mmaped()) {
        munmap(portal_ptr_, 0x1000U);

        portal_ptr_ = nullptr;
        mmap_done_  = false;
    } else { // since kept it open for write syscall
        close(fd_);
    }
}

void hw_queue::set_portal_ptr(void* value_ptr) noexcept {
    portal_offset_ = reinterpret_cast<uint64_t>(value_ptr) & OWN_PAGE_MASK;
    portal_mask_   = reinterpret_cast<uint64_t>(value_ptr) & (~OWN_PAGE_MASK);
    portal_ptr_    = value_ptr;
}

auto hw_queue::get_portal_ptr() const noexcept -> void* {
    uint64_t offset = portal_offset_++;
    offset          = (offset << 6) & OWN_PAGE_MASK;
    return reinterpret_cast<void*>(offset | portal_mask_);
}

/**
 * Enqueues a descriptor into the hardware queue.
 *
 * This function is used to enqueue a descriptor into the hardware queue.
 * If the hardware queue is memory-mapped, the descriptor is enqueued using ENQCMD.
 * Otherwise, the descriptor is written to the file descriptor associated with the hardware queue.
 *
 * @param desc_ptr A pointer to the descriptor to be enqueued.
 * @return The status of the enqueue operation. Returns QPL_STS_OK if the enqueue was successful, or an error code
 *         if the enqueue failed.
 */
auto hw_queue::enqueue_descriptor(void* desc_ptr) const noexcept -> qpl_status {
    if (is_wq_mmaped()) {
        uint8_t retry = 0U; //NOLINT(misc-const-correctness)

        void* current_place_ptr = get_portal_ptr();
        asm volatile(
                "sfence\t\n"
                ".byte 0xf2, 0x0f, 0x38, 0xf8, 0x02\t\n"
                "setz %0\t\n"
                : "=r"(retry)
                : "a"(current_place_ptr), "d"(desc_ptr));

#ifdef QPL_LOG_IAA_TIME
        if (!retry) {
            // If enqueue is successful, record the start time and meta data
            record_meta_data(desc_ptr);
            record_iaa_start_time();
        }
#endif

        // add with different verbosity level to not crowd output
        // DIAG(" ENQCMD submitted\n");

        return static_cast<qpl_status>(retry);
    } else {
        const ssize_t ret = write(fd_, desc_ptr, sizeof(hw_decompress_analytics_descriptor));

        // add with different verbosity level to not crowd output
        // DIAG(" write submitted\n");

        if (ret == sizeof(hw_decompress_analytics_descriptor)) {
#ifdef QPL_LOG_IAA_TIME
            // If enqueue is successful, record the start time and meta data
            record_meta_data(desc_ptr);
            record_iaa_start_time();
#endif
            return QPL_STS_OK;
        } else {
            DIAG(" write returned %ld, expected %ld\n", ret, sizeof(hw_decompress_analytics_descriptor));
            return QPL_STS_INIT_HW_NOT_SUPPORTED;
        }
    }
}

/**
 * @brief Execute NOOP operation to test out whether submitting to the Intel (R) Analytics Accelerator is possible.
 * Particularly, this function is used to test out whether the write system call is supported.
 *
 * @return The status of the enqueue + wait operation.
 */
auto hw_queue::execute_noop() const noexcept -> qpl_status {
    hw_descriptor HW_PATH_ALIGN_STRUCTURE                         desc {};
    HW_PATH_VOLATILE hw_completion_record HW_PATH_ALIGN_STRUCTURE completion_record {};

    hw_iaa_descriptor_init_noop_operation(&desc);
    hw_iaa_descriptor_set_completion_record(&desc, &completion_record);

    const qpl_status status = enqueue_descriptor(&desc);

#ifdef QPL_LOG_IAA_TIME
    record_invalid_end_time_to_skip_iaa_timing();
#endif

    if (QPL_STS_OK == status) {
        while (completion_record.status == 0) {
            _mm_pause();
        }
        if (completion_record.status == AD_STATUS_SUCCESS) { return QPL_STS_OK; }
    } else
        return status;

    return QPL_STS_INIT_HW_NOT_SUPPORTED;
}

auto hw_queue::initialize_new_queue(void* wq_descriptor_ptr) noexcept -> hw_accelerator_status {

    auto* work_queue_ptr = reinterpret_cast<accfg_wq*>(wq_descriptor_ptr);
    char  path[64];
#ifdef LOG_HW_INIT
    auto work_queue_dev_name = accfg_wq_get_devname(work_queue_ptr);
#endif

    if (ACCFG_WQ_ENABLED != accfg_wq_get_state(work_queue_ptr)) {
        DIAG("     %7s: DISABLED\n", work_queue_dev_name);
        return HW_ACCELERATOR_WORK_QUEUES_NOT_AVAILABLE;
    }

    if (ACCFG_WQ_SHARED != accfg_wq_get_mode(work_queue_ptr)) {
        DIAG("     %7s: UNSUPPOTED\n", work_queue_dev_name);
        return HW_ACCELERATOR_WORK_QUEUES_NOT_AVAILABLE;
    }

    DIAG("     %7s:\n", work_queue_dev_name);
    auto status = accfg_wq_get_user_dev_path(work_queue_ptr, path, 64 - 1);
    QPL_HWSTS_RET((0 > status), HW_ACCELERATOR_LIBACCEL_ERROR);

    DIAG("     %7s: opening descriptor %s", work_queue_dev_name, path);
    auto fd = open(path, O_RDWR);
    if (0 > fd) {
        DIAGA(", access denied\n");
        return HW_ACCELERATOR_LIBACCEL_ERROR;
    }

    auto* region_ptr = mmap(nullptr, 0x1000U, PROT_WRITE, MAP_SHARED | MAP_POPULATE, fd, 0);
    if (MAP_FAILED == region_ptr) {
        DIAGA(", limited MSI-X mapping failed\n");

        mmap_done_ = false;
        fd_        = fd; // Store fd since we need it open to write
                         // to the descriptor in the case of write syscall fallback

        if (QPL_STS_OK != execute_noop()) {
            DIAGA(", write system call failed.\n");

            fd_ = -1;
            close(fd);

            return HW_ACCELERATOR_SUPPORT_ERR;
        }
        DIAGA(", write system call succeeded.\n");
    } else {
        DIAGA(", MSI-X mapping done.\n");

        mmap_done_ = true;
        close(fd);
    }
    DIAGA("\n");

    priority_       = accfg_wq_get_priority(work_queue_ptr);
    block_on_fault_ = accfg_wq_get_block_on_fault(work_queue_ptr);

    accfg_op_config op_cfg;
    int32_t get_op_cfg_status = accfg_wq_get_op_config(work_queue_ptr, &op_cfg); //NOLINT(misc-const-correctness)
    if (get_op_cfg_status) {
        DIAGA("Failed to load API accfg_wq_get_op_config from accel-config, WQ operation configs will not be used.\n");
        op_cfg_enabled_ = false;
    } else {
        for (uint8_t bit_group_index = 0; bit_group_index < TOTAL_OP_CFG_BIT_GROUPS; bit_group_index++) {
            op_cfg_register_[bit_group_index] = op_cfg.bits[bit_group_index];
        }
        for (uint8_t bit_group_index = 0; bit_group_index < TOTAL_OP_CFG_BIT_GROUPS; bit_group_index++) {
            DIAG("%5s: OPCFG[%i]: 0x%08" PRIx32 "\n", work_queue_dev_name, bit_group_index,
                 op_cfg_register_[bit_group_index]);
        }
        op_cfg_enabled_ = true;
    }

#if 0 //NOLINT(readability-avoid-unconditional-preprocessor-if)
    DIAG("     %7s: size:        %d\n", work_queue_dev_name, accfg_wq_get_size(work_queue_ptr));
    DIAG("     %7s: threshold:   %d\n", work_queue_dev_name, accfg_wq_get_threshold(work_queue_ptr));
    DIAG("     %7s: priority:    %d\n", work_queue_dev_name, priority_);
    DIAG("     %7s: group:       %d\n", work_queue_dev_name, group_id);

    for(struct accfg_engine *engine = accfg_engine_get_first(device_ptr);
            engine != NULL; engine = accfg_engine_get_next(engine))
    {
        if(accfg_engine_get_group_id(engine) == group_id)
            DIAG("            %s\n", accfg_engine_get_devname(engine));
    }
#else
    DIAG("     %7s: priority:    %d\n", work_queue_dev_name, priority_);
    DIAG("     %7s: bof:         %d\n", work_queue_dev_name, block_on_fault_);
    DIAG("     %7s: fd:          %d\n", work_queue_dev_name, fd_);
#endif

    hw_queue::set_portal_ptr(region_ptr);

    return HW_ACCELERATOR_STATUS_OK;
}

auto hw_queue::priority() const noexcept -> int32_t {
    return priority_;
}

auto hw_queue::get_block_on_fault() const noexcept -> bool {
    return block_on_fault_;
}

auto hw_queue::get_op_configuration_support() const noexcept -> bool {
    return op_cfg_enabled_;
}

auto hw_queue::get_op_config_register() const noexcept -> op_config_register_t {
    return op_cfg_register_;
}

auto hw_queue::is_wq_mmaped() const noexcept -> bool {
    return mmap_done_;
}
} // namespace qpl::ml::dispatcher
#endif //__linux__
