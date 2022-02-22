/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Job API (public C API)
 */

#include "qpl/qpl.h"
#include "util/memory.hpp"
#include "compression/verification/verification_state.hpp"

// Legacy
#include "own_defs.h"
#include "compression_operations/compression_state_t.h"
#include "filter_operations/analytics_state_t.h"
#include "legacy_hw_path/async_hw_api.h"
#include "legacy_hw_path/hardware_state.h"
#include "compression_operations/own_deflate.h"

#ifdef __cplusplus
extern "C" {
#endif

QPL_INLINE void own_init_decompress(qpl_job *qpl_job_ptr);
QPL_INLINE void own_init_compress(qpl_job *qpl_job_ptr);
QPL_INLINE void own_init_analytics(qpl_job *qpl_job_ptr);
QPL_INLINE uint32_t own_get_job_size_decompress();
QPL_INLINE uint32_t own_get_job_size_compress();
QPL_INLINE uint32_t own_get_job_size_analytics();
uint32_t own_get_job_size_middle_layer_buffer();

QPL_FUN(qpl_status, qpl_get_job_size, (qpl_path_t qpl_path, uint32_t *job_size_ptr)) {
    QPL_BAD_PTR_RET(job_size_ptr);
    QPL_BADARG_RET (qpl_path_auto > qpl_path || qpl_path_software < qpl_path, QPL_STS_PATH_ERR);

    // qpl_job_ptr can have any alignment - therefore need additional bytes to align ptr
    *job_size_ptr = QPL_ALIGNED_SIZE(sizeof(qpl_job), QPL_DEFAULT_ALIGNMENT) + QPL_DEFAULT_ALIGNMENT;

    *job_size_ptr += QPL_ALIGNED_SIZE(own_get_job_size_compress(), QPL_DEFAULT_ALIGNMENT);
    *job_size_ptr += QPL_ALIGNED_SIZE(own_get_job_size_decompress(), QPL_DEFAULT_ALIGNMENT);
    *job_size_ptr += QPL_ALIGNED_SIZE(own_get_job_size_analytics(), QPL_DEFAULT_ALIGNMENT);
    *job_size_ptr += QPL_ALIGNED_SIZE(own_get_job_size_middle_layer_buffer(), QPL_DEFAULT_ALIGNMENT);
    *job_size_ptr += QPL_ALIGNED_SIZE(hw_get_job_size(), QPL_DEFAULT_ALIGNMENT);

    return QPL_STS_OK;
}

QPL_FUN(qpl_status, qpl_init_job, (qpl_path_t qpl_path, qpl_job *qpl_job_ptr)) {
    using namespace qpl::ml;

    QPL_BADARG_RET (qpl_path_auto > qpl_path || qpl_path_software < qpl_path, QPL_STS_PATH_ERR);
    QPL_BAD_PTR_RET(qpl_job_ptr);

    uint32_t       status                   = QPL_STS_OK;
    const uint32_t decomp_size              = QPL_ALIGNED_SIZE(own_get_job_size_decompress(), QPL_DEFAULT_ALIGNMENT);
    const uint32_t comp_size                = QPL_ALIGNED_SIZE(own_get_job_size_compress(), QPL_DEFAULT_ALIGNMENT);
    const uint32_t analytics_size           = QPL_ALIGNED_SIZE(own_get_job_size_analytics(), QPL_DEFAULT_ALIGNMENT);
    const uint32_t middle_layer_buffer_size = QPL_ALIGNED_SIZE(own_get_job_size_middle_layer_buffer(), QPL_DEFAULT_ALIGNMENT);
    const uint32_t job_size                 = QPL_ALIGNED_SIZE(sizeof(qpl_job), QPL_DEFAULT_ALIGNMENT);

    util::set_zeros((uint8_t *) qpl_job_ptr, job_size);

    // Calculate pointer to the auxiliary buffer
    // qpl_job_ptr can have any alignment - therefore need to align ptr
    qpl_job_ptr->data_ptr.compress_state_ptr =
            (uint8_t *) QPL_ALIGNED_PTR(((uint8_t *) qpl_job_ptr), QPL_DEFAULT_ALIGNMENT) + job_size;
    qpl_job_ptr->data_ptr.decompress_state_ptr    = qpl_job_ptr->data_ptr.compress_state_ptr + comp_size;
    qpl_job_ptr->data_ptr.analytics_state_ptr     = qpl_job_ptr->data_ptr.decompress_state_ptr + decomp_size;
    qpl_job_ptr->data_ptr.middle_layer_buffer_ptr = qpl_job_ptr->data_ptr.analytics_state_ptr + analytics_size;
    qpl_job_ptr->data_ptr.hw_state_ptr            = qpl_job_ptr->data_ptr.middle_layer_buffer_ptr + middle_layer_buffer_size;
    qpl_job_ptr->data_ptr.path                    = qpl_path;

#ifdef linux
    if (qpl_path_hardware == qpl_path || qpl_path_auto == qpl_path) {
        qpl_job_ptr->numa_id = -1;

        auto *const hw_state_ptr = (qpl_hw_state *) (qpl_job_ptr->data_ptr.hw_state_ptr);
        uint32_t hw_size = QPL_ALIGNED_SIZE(hw_get_job_size(), QPL_DEFAULT_ALIGNMENT);

        util::set_zeros((uint8_t *) hw_state_ptr, hw_size);

        if (qpl_path_hardware == qpl_job_ptr->data_ptr.path) {
            status = hw_accelerator_get_context(&hw_state_ptr->accel_context);

            if (HW_ACCELERATOR_STATUS_OK != status) {
                qpl_job_ptr->data_ptr.path = qpl_path_software;
                status += QPL_INIT_ERROR_BASE;
            }
        }
    }
#else
    if (qpl_path_software != qpl_path) {
        qpl_job_ptr->data_ptr.path = qpl_path_software;
    }
#endif

    // Zero descriptors
    util::set_zeros((uint8_t *) qpl_job_ptr->data_ptr.compress_state_ptr, comp_size);
    util::set_zeros((uint8_t *) qpl_job_ptr->data_ptr.decompress_state_ptr, decomp_size);
    util::set_zeros((uint8_t *) qpl_job_ptr->data_ptr.analytics_state_ptr, analytics_size);
    util::set_zeros((uint8_t *) qpl_job_ptr->data_ptr.middle_layer_buffer_ptr, middle_layer_buffer_size);

    // Initialize states
    own_init_compress(qpl_job_ptr);
    own_init_decompress(qpl_job_ptr);
    own_init_analytics(qpl_job_ptr);

    return static_cast<qpl_status>(status);
}

QPL_FUN(qpl_status, qpl_fini_job, (qpl_job *qpl_job_ptr)) {
    QPL_BAD_PTR_RET(qpl_job_ptr);
    uint32_t status = QPL_STS_OK;

    if (qpl_path_software != qpl_job_ptr->data_ptr.path) {
        status = hw_accelerator_finalize(&((qpl_hw_state *) qpl_job_ptr->data_ptr.hw_state_ptr)->accel_context);
    }

    return static_cast<qpl_status>(status);
}

QPL_INLINE uint32_t own_get_job_size_decompress() {
    uint32_t size = sizeof(own_decompression_state_t) + HUFF_LOOK_UP_TABLE_SIZE;

    return QPL_ALIGNED_SIZE(size, QPL_DEFAULT_ALIGNMENT);
}

QPL_INLINE uint32_t own_get_job_size_compress() {
    using namespace qpl::ml;
    uint32_t size;

    size = QPL_ALIGNED_SIZE(sizeof(own_compression_state_t), QPL_DEFAULT_ALIGNMENT);
    size += QPL_ALIGNED_SIZE(sizeof(own_deflate_job), QPL_DEFAULT_ALIGNMENT);
    size += QPL_ALIGNED_SIZE(compression::verify_state<execution_path_t::software>::get_buffer_size(),
                             QPL_DEFAULT_ALIGNMENT);
    size += QPL_ALIGNED_SIZE((1u << (qpl_mblk_size_32k + 8)), QPL_DEFAULT_ALIGNMENT);    //mini block buffer
    size += QPL_ALIGNED_SIZE(sizeof(deflate_histogram_t), QPL_DEFAULT_ALIGNMENT);

    // Default compression level
    size += QPL_ALIGNED_SIZE(sizeof(struct isal_hufftables), QPL_DEFAULT_ALIGNMENT);

    // High compression level
    size += QPL_ALIGNED_SIZE(sizeof(struct isal_hufftables), QPL_DEFAULT_ALIGNMENT);

    size += QPL_ALIGNED_SIZE(OWN_HIGH_HASH_TABLE_SIZE * sizeof(uint32_t), QPL_DEFAULT_ALIGNMENT);    //table
    size += QPL_ALIGNED_SIZE(OWN_HIGH_HASH_TABLE_SIZE * sizeof(uint32_t), QPL_DEFAULT_ALIGNMENT);    //history

    return size;
}

QPL_INLINE uint32_t own_get_job_size_analytics() {
    uint32_t size = 0u;

    size += QPL_ALIGNED_SIZE(sizeof(own_analytics_state_t), QPL_DEFAULT_ALIGNMENT);
    size += QPL_ALIGNED_SIZE(OWN_INFLATE_BUF_SIZE, QPL_DEFAULT_ALIGNMENT);
    size += QPL_ALIGNED_SIZE(OWN_UNPACK_BUF_SIZE, QPL_DEFAULT_ALIGNMENT);
    size += QPL_ALIGNED_SIZE(OWN_SET_BUF_SIZE, QPL_DEFAULT_ALIGNMENT);
    size += QPL_ALIGNED_SIZE(OWN_SRC2_BUF_SIZE, QPL_DEFAULT_ALIGNMENT);

    return size;
}

uint32_t own_get_job_size_middle_layer_buffer() {
    uint32_t size = 0u;

    size += QPL_ALIGNED_SIZE(ISAL_LEVEL_BUFFER_SIZE, QPL_DEFAULT_ALIGNMENT);
    size += QPL_ALIGNED_SIZE(sizeof(struct BitBuf2), QPL_DEFAULT_ALIGNMENT);
    size += QPL_ALIGNED_SIZE(sizeof(struct isal_hufftables), QPL_DEFAULT_ALIGNMENT);

    return size;
}

QPL_INLINE void own_init_decompress(qpl_job *qpl_job_ptr) {
    auto *data = (own_decompression_state_t *) qpl_job_ptr->data_ptr.decompress_state_ptr;
    data->inflate_state.block_state = ISAL_BLOCK_NEW_HDR;
}

QPL_INLINE void own_init_compress(qpl_job *qpl_job_ptr) {
    using namespace qpl::ml;
    auto *data_ptr = (own_compression_state_t *) qpl_job_ptr->data_ptr.compress_state_ptr;

    // Allocate common structures between different levels
    auto *current_ptr = (uint8_t *) data_ptr;
    current_ptr += QPL_ALIGNED_SIZE(sizeof(own_compression_state_t), QPL_DEFAULT_ALIGNMENT);

    data_ptr->isal_stream.hufftables = (struct isal_hufftables *) current_ptr;
    current_ptr += QPL_ALIGNED_SIZE(sizeof(struct isal_hufftables), QPL_DEFAULT_ALIGNMENT);

    data_ptr->verification_state_buffer_ptr = current_ptr;
    current_ptr += QPL_ALIGNED_SIZE(compression::verify_state<execution_path_t::software>::get_buffer_size(),
                                    QPL_DEFAULT_ALIGNMENT);

    // Allocate buffers for qpl_high_level compression
    auto *deflate_job_ptr = (own_deflate_job *) current_ptr;
    data_ptr->deflate_job_ptr = deflate_job_ptr;
    current_ptr += QPL_ALIGNED_SIZE(sizeof(own_deflate_job), QPL_DEFAULT_ALIGNMENT);

    deflate_job_ptr->histogram_ptr = (deflate_histogram_t *) current_ptr;
    current_ptr += QPL_ALIGNED_SIZE(sizeof(deflate_histogram_t), QPL_DEFAULT_ALIGNMENT);

    deflate_job_ptr->huffman_table_ptr = (struct isal_hufftables *) current_ptr;
    current_ptr += QPL_ALIGNED_SIZE(sizeof(struct isal_hufftables), QPL_DEFAULT_ALIGNMENT);

    deflate_job_ptr->histogram_ptr->table.hash_table_ptr = (uint32_t *) current_ptr;
    current_ptr += QPL_ALIGNED_SIZE(OWN_HIGH_HASH_TABLE_SIZE * sizeof(uint32_t), QPL_DEFAULT_ALIGNMENT);

    deflate_job_ptr->histogram_ptr->table.hash_story_ptr = (uint32_t *) current_ptr;

    deflate_job_ptr->job_status = initial_status;
}

QPL_INLINE void own_init_analytics(qpl_job *qpl_job_ptr) {
    auto *analytics_state_ptr = (own_analytics_state_t *) qpl_job_ptr->data_ptr.analytics_state_ptr;
    analytics_state_ptr->inflate_buf_size = QPL_ALIGNED_SIZE(OWN_INFLATE_BUF_SIZE, QPL_DEFAULT_ALIGNMENT);
    analytics_state_ptr->unpack_buf_size  = QPL_ALIGNED_SIZE(OWN_UNPACK_BUF_SIZE, QPL_DEFAULT_ALIGNMENT);
    analytics_state_ptr->set_buf_size     = QPL_ALIGNED_SIZE(OWN_SET_BUF_SIZE, QPL_DEFAULT_ALIGNMENT);
    analytics_state_ptr->src2_buf_size    = QPL_ALIGNED_SIZE(OWN_SRC2_BUF_SIZE, QPL_DEFAULT_ALIGNMENT);
    analytics_state_ptr->inflate_buf_ptr =
            (uint8_t *) analytics_state_ptr + QPL_ALIGNED_SIZE(sizeof(own_analytics_state_t), QPL_DEFAULT_ALIGNMENT);
    analytics_state_ptr->unpack_buf_ptr  =
            (uint8_t *) analytics_state_ptr->inflate_buf_ptr + analytics_state_ptr->inflate_buf_size;
    analytics_state_ptr->set_buf_ptr     =
            (uint8_t *) analytics_state_ptr->unpack_buf_ptr + analytics_state_ptr->unpack_buf_size;
    analytics_state_ptr->src2_buf_ptr    =
            (uint8_t *) analytics_state_ptr->set_buf_ptr + analytics_state_ptr->set_buf_size;
}


#ifdef __cplusplus
}
#endif
