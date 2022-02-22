/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @brief Contain implementation of functions for `Zero Compression`
 *
 * @details Function list:
 *          - @ref qplc_zero_compress_16u
 *          - @ref qplc_zero_decompress_16u
 *          - @ref qplc_zero_compress_32u
 *          - @ref qplc_zero_decompress_32u
 */

#include "own_qplc_defs.h"

#define OWN_ZERO_COMPRESS_BLOCK_SIZE  128u

OWN_QPLC_FUN(qplc_status_t, qplc_zero_compress_16u, (const uint8_t *const source_ptr,
                                                     const uint32_t source_size,
                                                     uint8_t *destination_ptr,
                                                     const uint32_t destination_max_size,
                                                     uint32_t *output_size)) {
    uint16_t *src = (uint16_t *) source_ptr;
    uint16_t *dst = (uint16_t *) destination_ptr;

    const uint32_t elem_size      = sizeof(*src);                      // size of element in bytes
    const uint32_t elems_in_block = 64;                                // number of elements in block
    const uint32_t tag_elems_num  = 4;                                 // number of current words in tag
    const uint32_t elems_num      = source_size / elem_size;           // number of input elements
    const uint32_t dst_elems_num  = destination_max_size / elem_size;  // number of available output elements
    // block size is 128 bytes
    // should check that available_in is divisible on 16? (i.e. incorrect data)
    // should check that there is enough bytes in out buff at least for one tag?

    uint64_t elemTagInd        = 1;    // index of the current src element in tag field
    uint32_t dst_curr_elem_num = 4;    // index of the current element in dst buffer
    uint32_t dst_tag_num       = 0;    // index of the tag field in dst buffer
    uint64_t tag64             = 0;    // tag field
    uint32_t i                 = 0;    // index of the current element in src buffer

    // this condition  !(dst_curr_elem_num >= dst_elems_num && 0 != src[i]) to count zero input elems even
    // in case when there isn't more space in dst (as example for 2 input elems (!0,0) and available_out = 10)
    for (i = 0; i < elems_num && !(dst_curr_elem_num >= dst_elems_num && 0 != src[i]); i++) {
        elemTagInd = QPL_ONE_64U << (i & (elems_in_block - 1));

        if (0 != src[i]) {// Store non-zero element
            tag64 |= elemTagInd; // set 1 on the current element index in tag
            dst[dst_curr_elem_num++] = src[i];
        }

        if ((QPL_ONE_64U << (elems_in_block - 1)) == elemTagInd) {    // if it is last element in block
            // current data block was finished
            // make an adjustments for the new one
            *(uint64_t *) (dst + dst_tag_num) = tag64;

            dst_tag_num = dst_curr_elem_num;

            // no need to increase dst_curr_elem_num if it is the last block and there are no more elements
            if (i + 1 < elems_num) {
                dst_curr_elem_num += tag_elems_num;
            }

            if (dst_curr_elem_num > dst_elems_num) {
                return QPLC_STS_DST_IS_SHORT_ERR;
            }

            tag64 = 0;
        }
    }

    if (i < elems_num || dst_curr_elem_num > dst_elems_num + tag_elems_num) {
        // if we came here then it means that output buffer is not enough
        //
        // in case if src data just feet exact in OWN_ZERO_COMPRESS_BLOCK_SIZE need to compare
        // with dst_elems_num + 4 to eleminate last dst_curr_elem_num += 4;
        return QPLC_STS_DST_IS_SHORT_ERR;
    }

    if (0 < (elems_num & (elems_in_block - 1))) {
       // src contains incomplete block

        tag64 |= (0 - QPL_ONE_64U) << (elems_num & (elems_in_block - 1)); // set 1 on the current element index in tag
        *(uint64_t *) (dst + dst_tag_num) = tag64;
    }

    *output_size = dst_curr_elem_num * elem_size;

    return QPLC_STS_OK;
}

OWN_QPLC_FUN(qplc_status_t, qplc_zero_decompress_16u, (const uint8_t *const source_ptr,
                                                       const uint32_t source_size,
                                                       uint8_t *destination_ptr,
                                                       const uint32_t destination_max_size,
                                                       uint32_t *output_size)) {
    uint16_t *src = (uint16_t *) source_ptr;
    uint16_t *dst = (uint16_t *) destination_ptr;

    const uint32_t tag_elems_num  = 4; // number of current words in tag
    const uint32_t elem_size      = sizeof(*src);
    const uint32_t elems_num      = source_size / elem_size;
    const uint32_t elems_in_block = OWN_ZERO_COMPRESS_BLOCK_SIZE / elem_size;
    const uint32_t dst_elems_num  = destination_max_size / elem_size;    // number of available output elements

    uint32_t dst_curr_elem_num = 0;
    uint64_t tag64             = 0;    // tag for the block

    uint32_t i = 0;
    while (i < elems_num) {
        // (i + tag_elems_num - 1 < elems_num) this condition prevents us from working with incomplete tag64
        if (i + tag_elems_num - 1 < elems_num) {
            tag64 = *(uint64_t *) (src + i);
            i += tag_elems_num;
        } else {
            return QPLC_STS_INVALID_ZERO_DECOMP_HDR;
        }
        // dst_curr_elem_num < dst_elems_num - while we have enough dst elems
        // (tag64 & (1ULL << j)) != 0 && i >= elems_num - current element in tag has 1 but there is no more elems
        // in Src -> decompression completed
        for (uint32_t j = 0; j < elems_in_block && !((tag64 & (QPL_ONE_64U << j)) != 0 && i >= elems_num); j++) {
            if (dst_curr_elem_num >= dst_elems_num) {
            // if we came here then it means that output buffer is not enough
                return QPLC_STS_DST_IS_SHORT_ERR;
            }

            if (tag64 & (QPL_ONE_64U << j)) {
                dst[dst_curr_elem_num] = src[i++];
            } else {
                dst[dst_curr_elem_num] = 0;
            }
            dst_curr_elem_num++;
        }
    }

    *output_size = dst_curr_elem_num * elem_size;

    return QPLC_STS_OK;
}

OWN_QPLC_FUN(qplc_status_t, qplc_zero_compress_32u, (const uint8_t *const source_ptr,
                                                     const uint32_t source_size,
                                                     uint8_t *destination_ptr,
                                                     const uint32_t destination_max_size,
                                                     uint32_t *output_size)) {
    uint32_t *src = (uint32_t *) source_ptr;
    uint32_t *dst = (uint32_t *) destination_ptr;

    const uint32_t elem_size      = sizeof(*src);                              // size of element in bytes
    const uint32_t elems_in_block = 32;                                        // number of elements in block
    const uint32_t max_32u        = UINT32_MAX;
    const uint32_t elems_num      = source_size / elem_size;     // number of input elements
    const uint32_t dst_elems_num  = destination_max_size / elem_size;    // number of available output elements

    uint32_t dst_curr_elem_num = 0;          // index of the current element in dst buffer
    uint32_t tag_ind           = 0;          // index of the tag field in dst buffer
    uint32_t tag               = max_32u;    // tag field

    uint32_t j = 0; // index of the current element in dst buffer

    // iteraton by block
    for (uint32_t i = 0; i < elems_num; i += elems_in_block) {
        tag     = max_32u;
        tag_ind = dst_curr_elem_num++;
        if ((dst_curr_elem_num > dst_elems_num)) {
            return QPLC_STS_DST_IS_SHORT_ERR;
        }
        // iteration by element
        // this condition  !(dst_curr_elem_num >= dst_elems_num && 0 != src[j]) to count zero input elems
        // even in case when there isn't more space in dst
        for (j = i;
             (j < i + elems_in_block) && (j < elems_num) && !(dst_curr_elem_num >= dst_elems_num && 0 != src[j]);
             j++) {
            if (0 == src[j]) {
                tag &= ~(1 << (j & (elems_in_block - 1)));
            } else {
                dst[dst_curr_elem_num++] = src[j];
            }
        }
        dst[tag_ind] = tag;
    }

    if ((j < elems_num) || (dst_curr_elem_num > dst_elems_num)) {
        // if we came here then it means that output buffer is not enough
        return QPLC_STS_DST_IS_SHORT_ERR;
    }

    *output_size = dst_curr_elem_num * elem_size;

    return QPLC_STS_OK;
}

OWN_QPLC_FUN(qplc_status_t, qplc_zero_decompress_32u, (const uint8_t *const source_ptr,
                                                       const uint32_t source_size,
                                                       uint8_t *destination_ptr,
                                                       const uint32_t destination_max_size,
                                                       uint32_t *output_size)) {
    uint32_t *src = (uint32_t *) source_ptr;
    uint32_t *dst = (uint32_t *) destination_ptr;

    const uint32_t elem_size      = sizeof(*src);
    const uint32_t elems_num      = source_size / elem_size;
    const uint32_t elems_in_block = OWN_ZERO_COMPRESS_BLOCK_SIZE / elem_size;
    const uint32_t dst_elems_num  = destination_max_size / elem_size;    // number of available output elements

    uint32_t dst_curr_elem_num = 0;
    uint32_t tag               = 0;    // tag for the block

    uint32_t i = 0;
    while (i < elems_num && dst_curr_elem_num < dst_elems_num) {
        tag             = src[i++];
        // dst_curr_elem_num < dst_elems_num - while we have enough dst elems
        // (tag & (1 << j)) != 0 && i >= elems_num - current element in tag has 1 but there is no more elems
        // in Src -> decompression completed
        for (uint32_t j = 0; j < elems_in_block && !((tag & (1 << j)) != 0 && i >= elems_num); j++) {
            if (dst_curr_elem_num >= dst_elems_num) {
                // if we came here then it means that output buffer is not enough
                return QPLC_STS_DST_IS_SHORT_ERR;
            }
            if (tag & (1 << j)) {
                dst[dst_curr_elem_num] = src[i++];
            } else {
                dst[dst_curr_elem_num] = 0;
            }
            dst_curr_elem_num++;
        }
    }

    //if ((i < elems_num) || (dst_curr_elem_num > dst_elems_num))
    if (i < elems_num) {
        // if we came here then it means that output buffer is not enough
        return QPLC_STS_DST_IS_SHORT_ERR;
    }

    *output_size = dst_curr_elem_num * elem_size;

    return QPLC_STS_OK;
}
