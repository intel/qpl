/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_DESCRIPTOR_PROCESSING_HPP
#define QPL_DESCRIPTOR_PROCESSING_HPP

#include <array>
#include <emmintrin.h>

#include "hw_definitions.h"

// middle-layer
#include "accelerator/hw_accelerator_api.h"
#include "util/awaiter.hpp"
#include "util/completion_record.hpp"
#include "util/hw_status_converting.hpp"

#ifdef QPL_LOG_IAA_TIME
#include "util/hw_timing_util.hpp"
#endif

namespace qpl::ml::util {

enum class execution_mode_t { sync, async };

template <typename return_t>
inline auto wait_descriptor_result(HW_PATH_VOLATILE hw_completion_record* const completion_record_ptr) -> return_t {
    awaiter::wait_for(&completion_record_ptr->status, AD_STATUS_INPROG);

    return ml::util::completion_record_convert_to_result<return_t>(completion_record_ptr);
}

template <typename return_t, execution_mode_t mode>
inline auto process_descriptor(hw_descriptor* const                         descriptor_ptr,
                               HW_PATH_VOLATILE hw_completion_record* const completion_record_ptr,
                               int32_t                                      numa_id = -1) noexcept -> return_t {
    return_t operation_result;

    hw_iaa_descriptor_set_completion_record(descriptor_ptr, completion_record_ptr);
    completion_record_ptr->status = AD_STATUS_INPROG; // Mark completion record as not completed

    auto accel_status = hw_enqueue_descriptor(descriptor_ptr, numa_id);

    if constexpr (mode == execution_mode_t::sync) {
        uint32_t status = convert_hw_accelerator_status_to_qpl_status(accel_status); //NOLINT(misc-const-correctness)
        if (status_list::ok != status) {
            if constexpr (std::is_same<decltype(status), return_t>::value) {
                return status;
            } else {
                operation_result.status_code_ = status;
                return operation_result;
            }
        }

        operation_result = wait_descriptor_result<return_t>(completion_record_ptr);

        // Simple Page Faults handling: if status AD_STATUS_READ_PAGE_FAULT or AD_STATUS_WRITE_PAGE_FAULT,
        // check that the Fault Address is available, touch the memory and resubmit descriptor again.
        if ((AD_STATUS_READ_PAGE_FAULT == completion_record_ptr->status ||
             AD_STATUS_WRITE_PAGE_FAULT == completion_record_ptr->status)) {

            uint8_t  fault_info    = 0U; // not in use currently
            uint64_t fault_address = 0U;
            hw_iaa_completion_record_get_fault_address(completion_record_ptr, &fault_info, &fault_address);

            DIAG("Page Fault happened with completion record status equals %d, Fault Address is %p\n",
                 (int)completion_record_ptr->status, (void*)fault_address);

            // If Fault Address is available, try to resubmit the job.
            // TODO: Add logic for figuring out the size of the faulted memory to touch all the related pages.
            // TODO: On 2nd generation, we could additionally check if Fault Address is available via Fault Info.
            if (fault_address != 0U) {
                if (AD_STATUS_READ_PAGE_FAULT == completion_record_ptr->status) {
                    volatile char* read_fault_address = (char*)fault_address;
                    *read_fault_address;
                } else { // AD_STATUS_WRITE_PAGE_FAULT
                    volatile char* write_fault_address = (char*)fault_address;
                    *write_fault_address               = *write_fault_address;
                }

                // Mark completion record as not completed for awaiter
                completion_record_ptr->status = AD_STATUS_INPROG;

                auto     enqueue_status = hw_enqueue_descriptor(descriptor_ptr, numa_id);
                uint32_t status =
                        convert_hw_accelerator_status_to_qpl_status(enqueue_status); //NOLINT(misc-const-correctness)
                if (status_list::ok != status) {
                    if constexpr (std::is_same<decltype(status), return_t>::value) {
                        return status;
                    } else {
                        operation_result.status_code_ = status;
                        return operation_result;
                    }
                }

                operation_result = wait_descriptor_result<return_t>(completion_record_ptr);
            }
        }

        if constexpr (std::is_same<other::crc_operation_result_t, return_t>::value) {
            operation_result.processed_bytes_ =
                    reinterpret_cast<hw_decompress_analytics_descriptor*>(descriptor_ptr)->src1_size;
        }
    } else {
        // Async path
#ifdef QPL_LOG_IAA_TIME
        qpl::ml::dispatcher::record_invalid_end_time_to_skip_iaa_timing();
#endif

        if constexpr (std::is_same<other::crc_operation_result_t, return_t>::value) {
            operation_result.status_code_ = convert_hw_accelerator_status_to_qpl_status(accel_status);
        } else {
            operation_result = static_cast<return_t>(convert_hw_accelerator_status_to_qpl_status(accel_status));
        }
    }

    return operation_result;
}

template <typename return_t, uint32_t number_of_descriptors>
inline auto process_descriptor(std::array<hw_descriptor, number_of_descriptors>&        descriptors,
                               std::array<hw_completion_record, number_of_descriptors>& completion_records,
                               int32_t                                                  numa_id) noexcept -> return_t {
    return_t operation_result {};

    for (uint32_t i = 0; i < descriptors.size(); i++) {
        hw_iaa_descriptor_set_completion_record(&descriptors[i], &completion_records[i]);
        completion_records[i].status = AD_STATUS_INPROG; // Mark completion record as not completed

        if constexpr (std::is_same_v<return_t, uint32_t>) {
            operation_result = process_descriptor<uint32_t, execution_mode_t::async>(&descriptors[i],
                                                                                     &completion_records[i], numa_id);
            if (operation_result != status_list::ok) { return operation_result; }
        } else {
            operation_result.status_code_ = process_descriptor<uint32_t, execution_mode_t::async>(
                    &descriptors[i], &completion_records[i], numa_id);
            if (operation_result.status_code_ != status_list::ok) { return operation_result; }
        }
    }

    for (uint32_t i = 0; i < descriptors.size(); i++) {
        auto execution_status = ml::util::wait_descriptor_result<return_t>(&completion_records[i]);

        if (execution_status.status_code_ != status_list::ok) {
            operation_result.status_code_ = execution_status.status_code_;
            return operation_result;
        } else {
            operation_result.output_bytes_ += execution_status.output_bytes_;
            operation_result.last_bit_offset_ =
                    execution_status
                            .last_bit_offset_; // TODO: In case of number_of_elements per descriptor modification should be adapted
        }
    }
    return operation_result;
}

} // namespace qpl::ml::util

#endif //QPL_DESCRIPTOR_PROCESSING_HPP
