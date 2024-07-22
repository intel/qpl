/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_MIDDLE_LAYER_DISPATCHER_HW_QUEUE_HPP_
#define QPL_SOURCES_MIDDLE_LAYER_DISPATCHER_HW_QUEUE_HPP_

#include <array>
#include <atomic>

#include "qpl/c_api/status.h"

#include "hw_status.h"

#define TOTAL_OP_CFG_BIT_GROUPS 8U // 256 bits / 32 bit groups

namespace qpl::ml::dispatcher {

class hw_queue {
public:
    using descriptor_t = void;

    using op_config_register_t = std::array<uint32_t, TOTAL_OP_CFG_BIT_GROUPS>;

    hw_queue() noexcept = default;

    hw_queue(const hw_queue&) noexcept = delete;

    auto operator=(const hw_queue& other) noexcept -> hw_queue& = delete;

    hw_queue(hw_queue&& other) noexcept;

    auto operator=(hw_queue&& other) noexcept -> hw_queue&;

    auto initialize_new_queue(descriptor_t* wq_descriptor_ptr) noexcept -> hw_accelerator_status;

    [[nodiscard]] auto get_portal_ptr() const noexcept -> void*;

    [[nodiscard]] auto enqueue_descriptor(void* desc_ptr) const noexcept -> qpl_status;

    [[nodiscard]] auto priority() const noexcept -> int32_t;

    [[nodiscard]] auto get_block_on_fault() const noexcept -> bool;

    [[nodiscard]] auto get_op_configuration_support() const noexcept -> bool;

    [[nodiscard]] auto get_op_config_register() const noexcept -> op_config_register_t;

    [[nodiscard]] auto is_wq_mmaped() const noexcept -> bool;

    [[nodiscard]] auto execute_noop() const noexcept -> qpl_status;

    void set_portal_ptr(void* portal_ptr) noexcept;

    virtual ~hw_queue() noexcept;

private:
    bool                          block_on_fault_  = false;
    int32_t                       priority_        = 0U;
    uint64_t                      portal_mask_     = 0U; /**< Mask for incrementing portals */
    mutable void*                 portal_ptr_      = nullptr;
    mutable std::atomic<uint64_t> portal_offset_   = 0U; /**< Portal for enqcmd (mod page size)*/
    bool                          op_cfg_enabled_  = false;
    op_config_register_t          op_cfg_register_ = {};    /**< OPCFG register content */
    bool                          mmap_done_       = false; /**< Flag to check whether mmap happened */
    int                           fd_              = -1;    /**< File descriptor for submission via write */
};

} // namespace qpl::ml::dispatcher
#endif //QPL_SOURCES_MIDDLE_LAYER_DISPATCHER_HW_QUEUE_HPP_
