/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Job API (public C API)
 */

// C_API headers
#include "qpl/qpl.h"

#include "compression_operations/compressor.hpp"
#include "filter_operations/analytics_state_t.h"
#include "filter_operations/filter_operations.hpp"
#include "job.hpp"
#include "other_operations/crc64.hpp"

// Middle layer headers
#include "util/checksum.hpp"
#include "util/hw_status_converting.hpp"

// core-iaa/include
#include "hw_devices.h"

// Legacy
#include "legacy_hw_path/async_hw_api.h"
#include "legacy_hw_path/hardware_state.h"
#include "own_defs.h"

// dispatchers
#include "dispatcher/hw_dispatcher.hpp"

//#define KEEP_DESCRIPTOR_ENABLED

static inline qpl_status sw_execute_job(qpl_job* const qpl_job_ptr) {
    using namespace qpl;

    // add with different verbosity level to not crowd output
    DIAG("Job is executed on qpl_path_software\n");

    uint32_t status                    = QPL_STS_OK;
    qpl_job_ptr->first_index_min_value = UINT32_MAX;

    auto* const analytics_state_ptr =
            reinterpret_cast<own_analytics_state_t*>(qpl_job_ptr->data_ptr.analytics_state_ptr);

    switch (qpl_job_ptr->op) {
        // processing compression
        case qpl_op_decompress: {
            status = perform_decompress<ml::execution_path_t::software>(qpl_job_ptr);

            if (qpl_job_ptr->flags & QPL_FLAG_LAST && QPL_STS_OK == status) {
                auto* const data_begin_ptr = qpl_job_ptr->next_out_ptr - qpl_job_ptr->total_out;
                auto* const data_end_ptr   = qpl_job_ptr->next_out_ptr;

                qpl_job_ptr->xor_checksum = ml::util::xor_checksum(data_begin_ptr, data_end_ptr, 0);
            }
            break;
        }
        case qpl_op_compress: {
            status = perform_compression<ml::execution_path_t::software>(qpl_job_ptr);
            break;
        } // other operations
        case qpl_op_crc64: {
            status = perform_crc64(qpl_job_ptr);
            break;
        }
        case qpl_op_scan_eq:
        case qpl_op_scan_ne:
        case qpl_op_scan_lt:
        case qpl_op_scan_le:
        case qpl_op_scan_gt:
        case qpl_op_scan_ge:
        case qpl_op_scan_range:
        case qpl_op_scan_not_range: {
            status = perform_scan(qpl_job_ptr, analytics_state_ptr->unpack_buf_ptr,
                                  analytics_state_ptr->unpack_buf_size);
            break;
        }
        case qpl_op_extract: {
            if (qpl_job_ptr->param_low > qpl_job_ptr->param_high) {
                qpl_job_ptr->first_index_min_value = 0U;

                return QPL_STS_OK;
            }

            status = perform_extract(qpl_job_ptr, analytics_state_ptr->unpack_buf_ptr,
                                     analytics_state_ptr->unpack_buf_size);
            break;
        }
        case qpl_op_select: {
            status = perform_select(qpl_job_ptr, analytics_state_ptr->unpack_buf_ptr,
                                    analytics_state_ptr->unpack_buf_size, analytics_state_ptr->set_buf_ptr,
                                    analytics_state_ptr->set_buf_size, analytics_state_ptr->src2_buf_ptr,
                                    analytics_state_ptr->src2_buf_size);
            break;
        }
        case qpl_op_expand: {
            status = perform_expand(qpl_job_ptr, analytics_state_ptr->unpack_buf_ptr,
                                    analytics_state_ptr->unpack_buf_size, analytics_state_ptr->set_buf_ptr,
                                    analytics_state_ptr->set_buf_size, analytics_state_ptr->src2_buf_ptr,
                                    analytics_state_ptr->src2_buf_size);
            break;
        }
        default: {
            status = QPL_STS_OPERATION_ERR;
        }
    }
    return static_cast<qpl_status>(status);
}

QPL_FUN("C" qpl_status, qpl_submit_job, (qpl_job * qpl_job_ptr)) {
    using namespace qpl;

    QPL_BAD_PTR_RET(qpl_job_ptr);
    QPL_BAD_PTR_RET(qpl_job_ptr->next_in_ptr);
    QPL_BAD_PTR_RET(qpl_job_ptr->data_ptr.compress_state_ptr);
    QPL_BAD_PTR_RET(qpl_job_ptr->data_ptr.decompress_state_ptr);
    QPL_BAD_PTR_RET(qpl_job_ptr->data_ptr.analytics_state_ptr);
    QPL_BAD_PTR_RET(qpl_job_ptr->data_ptr.hw_state_ptr);
    QPL_BAD_OP_RET(qpl_job_ptr->op);

    if ((qpl_job_ptr->flags & QPL_FLAG_CANNED_MODE) && (qpl_job_ptr->huffman_table == nullptr))
        return QPL_STS_NULL_PTR_ERR;

    qpl_status status         = QPL_STS_OK;
    bool       is_sw_fallback = false;

    const qpl_path_t path = qpl_job_ptr->data_ptr.path;

    if ((qpl_path_hardware == path || qpl_path_auto == path)) {
        auto* state_ptr = reinterpret_cast<qpl_hw_state*>(job::get_state(qpl_job_ptr));

        // Reset is_sw_fallback for the first job
        if (qpl_job_ptr->flags & QPL_FLAG_FIRST) { job::update_is_sw_fallback(qpl_job_ptr, false); }

        if ((qpl_op_compress == qpl_job_ptr->op) && (qpl_high_level == qpl_job_ptr->level)) {
            if (qpl_path_hardware == path) {
                return QPL_STS_UNSUPPORTED_COMPRESSION_LEVEL;
            } else if (qpl_path_auto == path) {
                job::update_is_sw_fallback(qpl_job_ptr, true);
            }
        }

        // Execute job on HW path
        if (!state_ptr->is_sw_fallback) {
#if defined(KEEP_DESCRIPTOR_ENABLED)
            if (state_ptr->descriptor_not_submitted) {
                status = hw_enqueue_descriptor(&state_ptr->desc_ptr, qpl_job_ptr->numa_id);

                if (status == QPL_STS_OK) { state_ptr->descriptor_not_submitted = false; }

                return static_cast<qpl_status>(status);
            }
#endif

            // check that HW is available
            static auto& dispatcher = ml::dispatcher::hw_dispatcher::get_instance();
            if (!dispatcher.is_hw_support()) {
                const hw_accelerator_status hw_status = dispatcher.get_hw_init_status();
                status = ml::util::convert_hw_accelerator_status_to_qpl_status(hw_status);
            }

            if (status == QPL_STS_OK) {
                status = hw_submit_job(qpl_job_ptr);

                if (status == QPL_STS_OK) { state_ptr->job_is_submitted = true; }

#if defined(KEEP_DESCRIPTOR_ENABLED)
                if (status == QPL_STS_QUEUES_ARE_BUSY_ERR && qpl_path_hardware == path) {
                    state_ptr->descriptor_not_submitted = true;
                }
#endif
            }

            /**
             * Use fallback to qpl_path_software in case if qpl_path_hardware returns an error.
             *
             * @warning Disallow falling back to the host execution if failure is not on the
             * first chunk or if QPL_STS_MORE_OUTPUT_NEEDED (output buffer is too small) error happened.
            */
            if (QPL_STS_OK != status && job::is_sw_fallback_supported(qpl_job_ptr, status)) {
                job::update_is_sw_fallback(qpl_job_ptr, true);
            } else {
                return static_cast<qpl_status>(status);
            }
        }
        is_sw_fallback = state_ptr->is_sw_fallback;
    }

    if (qpl_path_software == path || is_sw_fallback) {
        // Execute job on SW path
        qpl_job_ptr->data_ptr.path = qpl_path_software;
        status                     = sw_execute_job(qpl_job_ptr);
        qpl_job_ptr->data_ptr.path = path;

        if (QPL_STS_OK == status && qpl_path_auto == path) {
            auto* state_ptr             = reinterpret_cast<qpl_hw_state*>(job::get_state(qpl_job_ptr));
            state_ptr->job_is_submitted = true;
        }
    }

    return status;
}

QPL_FUN("C" qpl_status, qpl_check_job, (qpl_job * qpl_job_ptr)) {
    using namespace qpl;

    QPL_BAD_PTR_RET(qpl_job_ptr);
    uint32_t status = QPL_STS_OK;

    // If job was submitted on the Auto Path, and fell back to SW, return OK
    if (qpl_path_auto == qpl_job_ptr->data_ptr.path) {
        auto* state_ptr = reinterpret_cast<qpl_hw_state*>(job::get_state(qpl_job_ptr));
        if (state_ptr->is_sw_fallback) { return QPL_STS_OK; }
    }

    if (job::is_supported_on_hardware(qpl_job_ptr)) { status = hw_check_job(qpl_job_ptr); }

    // Do not attempt host execution if the job is being processed
    if (QPL_STS_BEING_PROCESSED == status) { return static_cast<qpl_status>(status); }

    // Use fallback to qpl_path_software in case if qpl_path_hardware returns an error.
    if (QPL_STS_OK != status && job::is_sw_fallback_supported(qpl_job_ptr, static_cast<qpl_status>(status))) {
        job::update_is_sw_fallback(qpl_job_ptr, true);

        // Execute job on SW path
        const qpl_path_t path      = qpl_job_ptr->data_ptr.path;
        qpl_job_ptr->data_ptr.path = qpl_path_software;
        status                     = sw_execute_job(qpl_job_ptr);
        qpl_job_ptr->data_ptr.path = path;
    }

    return static_cast<qpl_status>(status);
}

QPL_FUN("C" qpl_status, qpl_wait_job, (qpl_job * qpl_job_ptr)) {
    using namespace qpl;

    QPL_BAD_PTR_RET(qpl_job_ptr);

    uint32_t status = QPL_STS_OK;

    // If job was submitted on the Auto Path, and fell back to SW, return OK
    if (qpl_path_auto == qpl_job_ptr->data_ptr.path) {
        auto* state_ptr = reinterpret_cast<qpl_hw_state*>(job::get_state(qpl_job_ptr));
        if (state_ptr->is_sw_fallback) { return QPL_STS_OK; }
    }

    // HW path doesn't support qpl_high_level compression ratio and ZLIB headers/trailers
    if (job::is_supported_on_hardware(qpl_job_ptr)) {
        do { //NOLINT(cppcoreguidelines-avoid-do-while)
            status = hw_check_job(qpl_job_ptr);
        } while (QPL_STS_BEING_PROCESSED == status);
    }

    // Use fallback to qpl_path_software in case if qpl_path_hardware returns an error.
    if (QPL_STS_OK != status && job::is_sw_fallback_supported(qpl_job_ptr, static_cast<qpl_status>(status))) {
        job::update_is_sw_fallback(qpl_job_ptr, true);

        // Execute job on SW path
        const qpl_path_t path      = qpl_job_ptr->data_ptr.path;
        qpl_job_ptr->data_ptr.path = qpl_path_software;
        status                     = sw_execute_job(qpl_job_ptr);
        qpl_job_ptr->data_ptr.path = path;
    }

    return static_cast<qpl_status>(status);
}

QPL_FUN("C" qpl_status, qpl_execute_job, (qpl_job * qpl_job_ptr)) {
    using namespace qpl;

    QPL_BAD_PTR_RET(qpl_job_ptr);

    if ((qpl_job_ptr->flags & QPL_FLAG_CANNED_MODE) && (qpl_job_ptr->huffman_table == nullptr))
        return QPL_STS_NULL_PTR_ERR;

    qpl_status status = QPL_STS_OK;
    qpl_path_t path   = qpl_job_ptr->data_ptr.path;

    if ((qpl_path_hardware == path || qpl_path_auto == path)) {
        auto* state_ptr = reinterpret_cast<qpl_hw_state*>(job::get_state(qpl_job_ptr));

        // Reset is_sw_fallback for the first job
        if (qpl_job_ptr->flags & QPL_FLAG_FIRST) { job::update_is_sw_fallback(qpl_job_ptr, false); }

        if ((qpl_op_compress == qpl_job_ptr->op) && (qpl_high_level == qpl_job_ptr->level)) {
            if (qpl_path_hardware == path) {
                return QPL_STS_UNSUPPORTED_COMPRESSION_LEVEL;
            } else if (qpl_path_auto == path) {
                job::update_is_sw_fallback(qpl_job_ptr, true);
            }
        }

        auto* const analytics_state_ptr =
                reinterpret_cast<own_analytics_state_t*>(qpl_job_ptr->data_ptr.analytics_state_ptr);

        if (!state_ptr->is_sw_fallback) {
            // check that HW is available
            static auto& dispatcher = ml::dispatcher::hw_dispatcher::get_instance();
            if (!dispatcher.is_hw_support()) {
                const hw_accelerator_status hw_status = dispatcher.get_hw_init_status();
                status = ml::util::convert_hw_accelerator_status_to_qpl_status(hw_status);
            }

            if (QPL_STS_OK == status) {
                if (job::is_extract(qpl_job_ptr)) {
                    status = static_cast<qpl_status>(perform_extract(qpl_job_ptr, analytics_state_ptr->unpack_buf_ptr,
                                                                     analytics_state_ptr->unpack_buf_size));
                } else if (job::is_scan(qpl_job_ptr)) {
                    status = static_cast<qpl_status>(perform_scan(qpl_job_ptr, analytics_state_ptr->unpack_buf_ptr,
                                                                  analytics_state_ptr->unpack_buf_size));
                } else if (job::is_select(qpl_job_ptr)) {
                    status = static_cast<qpl_status>(perform_select(
                            qpl_job_ptr, analytics_state_ptr->unpack_buf_ptr, analytics_state_ptr->unpack_buf_size,
                            analytics_state_ptr->set_buf_ptr, analytics_state_ptr->set_buf_size,
                            analytics_state_ptr->src2_buf_ptr, analytics_state_ptr->src2_buf_size));
                } else if (job::is_expand(qpl_job_ptr)) {
                    status = static_cast<qpl_status>(perform_expand(
                            qpl_job_ptr, analytics_state_ptr->unpack_buf_ptr, analytics_state_ptr->unpack_buf_size,
                            analytics_state_ptr->set_buf_ptr, analytics_state_ptr->set_buf_size,
                            analytics_state_ptr->src2_buf_ptr, analytics_state_ptr->src2_buf_size));
                } else if (job::is_decompression(qpl_job_ptr)) {
                    status = static_cast<qpl_status>(perform_decompress<ml::execution_path_t::hardware>(qpl_job_ptr));
                } else if (job::is_compression(qpl_job_ptr) &&
                           !(job::is_indexing_enabled(qpl_job_ptr) && job::is_multi_job(qpl_job_ptr))) {
                    status = static_cast<qpl_status>(perform_compression<ml::execution_path_t::hardware>(qpl_job_ptr));
                } else {
                    // For other operations, like crc64, run legacy async code path
                    status = hw_submit_job(qpl_job_ptr);

                    if (QPL_STS_OK == status) {
                        state_ptr->job_is_submitted = true;
                        status                      = qpl_wait_job(qpl_job_ptr);
                    }
                }
            }

            // Use fallback to qpl_path_software in case if qpl_path_hardware returns an error.
            if (QPL_STS_OK != status && job::is_sw_fallback_supported(qpl_job_ptr, status)) {
                job::update_is_sw_fallback(qpl_job_ptr, true);
            }
        }

        // Fallback to qpl_path_software
        if (state_ptr->is_sw_fallback) {
            path                       = qpl_job_ptr->data_ptr.path;
            qpl_job_ptr->data_ptr.path = qpl_path_software;
            status                     = sw_execute_job(qpl_job_ptr);
            qpl_job_ptr->data_ptr.path = path;

            if (QPL_STS_OK == status && qpl_path_auto == qpl_job_ptr->data_ptr.path) {
                auto* state_ptr             = reinterpret_cast<qpl_hw_state*>(job::get_state(qpl_job_ptr));
                state_ptr->job_is_submitted = true;
            }
        }

        return status;
    }

    // For qpl_path_software
    return qpl_submit_job(qpl_job_ptr);
}
