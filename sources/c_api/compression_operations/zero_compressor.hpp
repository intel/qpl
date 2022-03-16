/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Job API (private C++ API)
 */

#ifndef QPL_SOURCES_C_API_COMPRESSION_OPERATIONS_ZERO_COMPRESSOR_HPP_
#define QPL_SOURCES_C_API_COMPRESSION_OPERATIONS_ZERO_COMPRESSOR_HPP_

#include "common/defs.hpp"
#include "qpl/c_api/defs.h"

/**
 * @anchor ZCOMPRESS_OPERATIONS
 * @name Zero Compress Operations API
 *
 * @details Performs Compression/Decompression of streams using zero compress method.
 *          In this method, the data (both compressed and uncompressed) is considered to be made of a series
 *          of "words", where the word length is either 16-bits or 32-bits, depending on the operation.
 *          This means that the input buffer size must be a multiple of the appropriate word size and that the
 *          generated output will also be such a multiple. In compression, the words are processed N at a time,
 *          where N=32 for the 32-bit case, and N=64 for the 16-bit case. That means that the uncompressed data
 *          is always processed 128 bytes at a time, regardless of the word length. This is called a block.
 *          The compressed data consists of N tag bits, where a 0-bit means that the corresponding word
 *          has a value of 0, and a 1-bit means that the corresponding word has a value other than 0.
 *          This is followed by the non-zero words within that block. So, for the 32-bit wide case,
 *          there are 32 tag bits forming one tag word, and in the 16-bit case, there are 64 tag bits,
 *          forming four tag words. In general, the last block will not be full; that is,
 *          it will not have 128 bytes in it (although the size must still be a multiple of the word size).
 *          To represent this, the "missing" words are encoded in the header as 1-bits (as if they were non-zero),
 *          but no data will be written to the output. So, when the decompressor sees that a word should be present,
 *          but that word is missing (due to reaching the end of the input stream), it stops outputting words.
 *
 *  | OP_CODE                     | Function                   |
 *  |-----------------------------|----------------------------|
 *  | @ref qpl_op_z_compress32    | @ref qpl_zero_compress_16u   |
 *  | @ref qpl_op_z_compress16    | @ref qpl_zero_compress_32u   |
 *  | @ref qpl_op_z_decompress32  | @ref qpl_zero_decompress_32u |
 *  | @ref qpl_op_z_decompress16  | @ref qpl_zero_decompress_16u |
 *
 * @note Functions can work with 2 streams:
 *         - A `Source` stream that contains data to process
 *         - A `Destination` stream that is used to store operation result
 *
 * @note   If the operation is completed with success, the following @ref qpl_job fields are updated:
 *              - @ref qpl_job.total_in       - updates with a total count of bytes read from the `Source`
 *              - @ref qpl_job.next_in_ptr        - increases the pointer value on the @ref qpl_job.total_in bytes
 *              - @ref qpl_job.available_in       - decreases on the @ref qpl_job.total_in bytes
 *              - @ref qpl_job.total_out      - updates with the total count of bytes written to the `Destination`
 *              - @ref qpl_job.next_out_ptr       - increases pointer value on the @ref qpl_job.total_out bytes
 *              - @ref qpl_job.available_out      - decreases on the @ref qpl_job.total_out bytes
 * @{
 *
 */

namespace qpl {

/**
 * @brief Compress/Decompresses data is compressed in zero compress format for 16-bit/32-bit words
 *
 * @param [in,out] job_ptr pointer onto the user specified @ref qpl_job
 * @param [in] buffer_ptr  pointer onto buffer
 * @param [in] buffer_size buffer size
 *
 * @details For operation execution, set the following parameters in the `qpl_job_ptr`:
 *      - `Input` properties:
 *          - @ref qpl_job.next_in_ptr    - start address
 *          - @ref qpl_job.available_in   - number of available bytes
 *      - `Output` properties:
 *          - @ref qpl_job.next_out_ptr   - start address of memory region to store the result of the operation
 *          - @ref qpl_job.available_in   - number of available bytes
 *
 * @return
 *      - @ref QPL_STS_OK
 *      - @ref QPL_STS_NULL_PTR_ERR
 *      - @ref QPL_STS_DST_IS_SHORT_ERR
 *      - @ref QPL_STS_SIZE_ERR in case if available_in or available_out is 0 or isn't a multiple of 4
 *
 * Example of main usage:
 * @snippet low-level-api/zero_compression_example.cpp QPL_LOW_LEVEL_ZERO_COMPRESSION_EXAMPLE
 *
 */

uint32_t perform_zero_compress(qpl_job *job_ptr, uint8_t *buffer_ptr, uint32_t buffer_size) noexcept;

}

/** @} */

#endif //QPL_SOURCES_C_API_COMPRESSION_OPERATIONS_ZERO_COMPRESSOR_HPP_
