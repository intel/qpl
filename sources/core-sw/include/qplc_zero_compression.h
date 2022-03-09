/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @defgroup SW_KERNELS_ZERO_COMPRESS_API Unpack API
 * @ingroup  SW_KERNELS_PRIVATE_API
 * @{
 * @brief Contains Contains Intel® Query Processing Library (Intel® QPL) Core API for `Zero Compress` operation
 *
 */

#ifndef QPLC_ZERO_COMPRESSION_H_
#define QPLC_ZERO_COMPRESSION_H_

#include "qplc_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef qplc_status_t (*qplc_zero_compress_t_ptr)(uint8_t *source_ptr,
                                                  uint32_t source_size,
                                                  uint8_t *destination_ptr,
                                                  uint32_t destination_max_size,
                                                  uint32_t *output_size);

OWN_QPLC_API(qplc_status_t, qplc_zero_compress_16u, (uint8_t *source_ptr,
                                                     uint32_t source_size,
                                                     uint8_t *destination_ptr,
                                                     uint32_t destination_max_size,
                                                     uint32_t *output_size))

OWN_QPLC_API(qplc_status_t, qplc_zero_decompress_16u, (uint8_t *source_ptr,
                                                       uint32_t source_size,
                                                       uint8_t *destination_ptr,
                                                       uint32_t destination_max_size,
                                                       uint32_t *output_size))

OWN_QPLC_API(qplc_status_t, qplc_zero_compress_32u, (uint8_t *source_ptr,
                                                     uint32_t source_size,
                                                     uint8_t *destination_ptr,
                                                     uint32_t destination_max_size,
                                                     uint32_t *output_size))

OWN_QPLC_API(qplc_status_t, qplc_zero_decompress_32u, (uint8_t *source_ptr,
                                                       uint32_t source_size,
                                                       uint8_t *destination_ptr,
                                                       uint32_t destination_max_size,
                                                       uint32_t *output_size))

#ifdef __cplusplus
}
#endif

#endif //QPLC_ZERO_COMPRESSION_H_
/** @} */
