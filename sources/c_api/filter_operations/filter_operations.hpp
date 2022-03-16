/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef JOB_PARSER_H
#define JOB_PARSER_H

#include "qpl/c_api/job.h"

/**
 * @anchor ANALYTIC_OPERATIONS
 * @name Analytic Operations API
 *
 * @details Contains functions that can be used to perform a data filtration.
 *
 * @note Functions can work with 3 streams:
 *           - A `Source-1` stream contains data to process
 *           - A `Source-2` stream contains extra data to perform data processing. Can be presented:
 *               - In the `little-endian` format `by default`
 *               - In the `big-endian format` with using @ref QPL_FLAG_SRC2_BE flag
 *           - A`Destination` stream is used to store operation result
 *               - In the `little-endian` format `by default`
 *               - In the `big-endian format` in case if @ref QPL_FLAG_OUT_BE flag specified
 *
 *  @note If the operation completes with success the following @ref qpl_job fields will be updated:
 *           - @ref qpl_job.total_in       - updates with a total count of read bytes from the`Source-1`
 *           - @ref qpl_job.next_in_ptr        - pointer value increases on @ref qpl_job.total_in bytes
 *           - @ref qpl_job.available_in       - set to zero;
 *           - @ref qpl_job.total_out      - updates with a total count of written bytes to the `Destination`
 *           - @ref qpl_job.next_out_ptr       - pointer value increases on @ref qpl_job.total_out bytes
 *           - @ref qpl_job.available_out      - decreases on @ref qpl_job.total_out bytes
 *           - @ref qpl_job.last_bit_offset - updates with last actual bit in the last output byte
 *
 *  @remark Analytic operations support the following additional features:
 *           - <b> `Initial bytes skipping` </b> with @ref qpl_job.drop_initial_bytes field.
 *           - <b> `Aggregates calculation:` </b><br>
 *               Min, Max, Sum values are calculated and then written into:
 *               - @ref qpl_job.first_index_min_value;
 *               - @ref qpl_job.last_index_max_value;
 *               - @ref qpl_job.sum_value;
 *           - <b> `Checksum calculation:` </b><br>
 *               Crc32 and Xor checksums are calculated and then written into:
 *               - @ref qpl_job.crc;
 *               - @ref qpl_job.xor_checksum;
 *
 * @{
 *
 */

/**
 * @brief Scans `Source` for values that are meet the requirement that are described in the @ref qpl_job
 *
 * @param [in,out] job_ptr pointer onto user specified @ref qpl_job
 * @param [in] buffer_ptr  unpack buffer
 * @param [in] buffer_size unpack buffer size
 *
 * @details For operation execution, you must set the following parameters in `qpl_job_ptr`:
 *      - Operation options:
 *          - @ref qpl_job.op                - comparison predicate
 *          - @ref qpl_job.num_input_elements  - number elements for processing
 *          - @ref qpl_job.param_low          - the first value for comparison (see note 1)
 *          - @ref qpl_job.param_low          - the second value for comparison (see note 1)
 *      - `Source` properties:
 *          - @ref qpl_job.next_in_ptr            - start address
 *          - @ref qpl_job.available_in           - number of available bytes
 *          - @ref qpl_job.src1_bit_width      - element bit-width
 *          - @ref qpl_job.parser            - stream format (@ref qpl_parser)
 *      - `Destination` properties (`Output`):
 *          - @ref qpl_job.next_out_ptr           - start address of memory region to store result of operation
 *          - @ref qpl_job.available_in           - number of available bytes
 *          - @ref qpl_job.out_bit_width       - output format (see note 2)
 *
 * @note 1: Table below describes all supported predicates.
 *      | @ref qpl_job.op value  |    Predicate condition (i - element number)       |
 *      |-----------------------|---------------------------------------------------|
 *      | qpl_op_scan_eq        |  qpl_job_ptr.next_in_ptr[i] == qpl_job_ptr.param_low            |
 *      | qpl_op_scan_ne        |  qpl_job_ptr.next_in_ptr[i] != qpl_job_ptr.param_low            |
 *      | qpl_op_scan_lt        |  qpl_job_ptr.next_in_ptr[i]  < qpl_job_ptr.param_low            |
 *      | qpl_op_scan_le        |  qpl_job_ptr.next_in_ptr[i] <= qpl_job_ptr.param_low            |
 *      | qpl_op_scan_gt        |  qpl_job_ptr.next_in_ptr[i] > qpl_job_ptr.param_low             |
 *      | qpl_op_scan_ge        |  qpl_job_ptr.next_in_ptr[i] >= qpl_job_ptr.param_low            |
 *      | qpl_op_scan_range     |  qpl_job_ptr.param_low >= qpl_job_ptr.next_in_ptr[i] <= qpl_job_ptr.param_high |
 *      | qpl_op_scan_not_range |  qpl_job_ptr.next_in_ptr[i] < qpl_job_ptr.param_low && qpl_job_ptr.param_high
 *                                                                                  > qpl_job_ptr.next_in_ptr[i] |
 *
 * @note 2: `Output` formats:
 *      - If output format is @ref qpl_ow_nom, output will be bit vector.
 *        Bit vector contains comparison results for each element in the stream (1 - condition is true for element,
 *        otherwise false)
 *      - If output format is @ref qpl_ow_8, @ref qpl_ow_16 or @ref qpl_ow_32, output will be indexes vector.
 *        Indexes vector contains indexes of all elements that satisfy condition in the table above.
 *
 * @warning
 *      If index is great than output extension, it will be error.
 *          - @ref qpl_ow_8  - max index is 255;
 *          - @ref qpl_ow_16 - max index is 65,535;
 *          - @ref qpl_ow_32 - max index is 4,294,967,295.
 *
 * @warning If any of @ref qpl_job.available_in, @ref qpl_job.available_out, @ref qpl_job.num_input_elements is 0,
 *          it will be error.
 *
 * @return
 *    - @ref QPL_STS_OK
 *    - @ref QPL_STS_NULL_PTR_ERR
 *    - @ref QPL_STS_SIZE_ERR
 *    - @ref QPL_STS_BIT_WIDTH_ERR
 *    - @ref QPL_STS_SRC_IS_SHORT_ERR
 *    - @ref QPL_STS_DST_IS_SHORT_ERR
 *    - @ref QPL_STS_OUT_FORMAT_ERR
 *    - @ref QPL_STS_PARSER_ERR
 *    - @ref QPL_STS_OPERATION_ERR
 *    - @ref QPL_STS_OUTPUT_OVERFLOW_ERR
 *
 * Example of main usage:
 * @snippet low-level-api/scan_example.cpp QPL_LOW_LEVEL_SCAN_EXAMPLE
 *
 * Example of scanning for unique value:
 * @snippet low-level-api/scan_for_unique_value_example.cpp QPL_LOW_LEVEL_SCAN_FOR_UNIQUE_EXAMPLE
 *
 * Example of scanning with range:
 * @snippet low-level-api/scan_range_example.cpp QPL_LOW_LEVEL_SCAN_RANGE_EXAMPLE
 *
 */
uint32_t perform_scan(qpl_job *job_ptr, uint8_t *buffer_ptr, uint32_t buffer_size);

/**
 * @brief Extracts a sub-vector from the `Source` starting from index param_low and finishing at index param_high
 *
 * @param [in,out] job_ptr pointer onto user specified @ref qpl_job
 * @param [in] buffer_ptr  unpack buffer
 * @param [in] buffer_size unpack buffer size
 *
 * @details For operation execution, you must set the following parameters in `qpl_job_ptr`:
 *      - Operation options:
 *          - @ref qpl_job.num_input_elements  - number elements for processing
 *          - @ref qpl_job.param_low          - the first element index
 *          - @ref qpl_job.param_low          - the second element index
 *      - `Source` properties:
 *          - @ref qpl_job.next_in_ptr            - start address
 *          - @ref qpl_job.available_in           - number of available bytes
 *          - @ref qpl_job.src1_bit_width      - element bit-width
 *          - @ref qpl_job.parser            - stream format (@ref qpl_parser)
 *      - Destination properties (`Output`):
 *          - @ref qpl_job.next_out_ptr           - start address of memory region to store result of operation
 *          - @ref qpl_job.available_in           - number of available bytes
 *          - @ref qpl_job.out_bit_width       - output format (see note)
 *
 * This operation outputs those input elements whose indices (starting at 0) fall within the inclusive range
 * defined by job fields @ref qpl_job.param_low and @ref qpl_job.param_high. So the bit width of the `Destination`
 * element is the same as the bit width of the `Source` element, and the number of output elements
 * should be ( param_high - param_low + 1 ).
 *
 * @note `Output` formats:
 *      - If output format is @ref qpl_ow_nom, corresponding elements will be written as is.
 *      - If output format is @ref qpl_ow_8, @ref qpl_ow_16 or @ref qpl_ow_32, extracted elements will be written
 *        extended by zero.
 *          - @ref qpl_ow_8  - element bit-width will be extended to 8-bit;
 *          - @ref qpl_ow_16 - element bit-width will be extended to 16-bit;
 *          - @ref qpl_ow_32 - element bit-width will be extended to 32-bit.
 *      - If output format is @ref qpl_ow_8, @ref qpl_ow_16 or @ref qpl_ow_32, but @ref qpl_job.src1_bit_width == 1,
 *        output will contain indexes of non-zero elements
 *
 * @warning
 *      If index is great than output extension, it will be error.
 *          - @ref qpl_ow_8  - max index is 255;
 *          - @ref qpl_ow_16 - max index is 65,535;
 *          - @ref qpl_ow_32 - max index is 4,294,967,295.
 *
 * @warning If any of @ref qpl_job.available_in, @ref qpl_job.available_out, @ref qpl_job.num_input_elements is 0,
 *          it will be error.
 *
 * @return
 *    - @ref QPL_STS_OK
 *    - @ref QPL_STS_NULL_PTR_ERR
 *    - @ref QPL_STS_SIZE_ERR
 *    - @ref QPL_STS_BIT_WIDTH_ERR
 *    - @ref QPL_STS_SRC_IS_SHORT_ERR
 *    - @ref QPL_STS_DST_IS_SHORT_ERR
 *    - @ref QPL_STS_PARSER_ERR
 *    - @ref QPL_STS_OPERATION_ERR
 *
 * Example of main usage:
 * @snippet low-level-api/extract_example.cpp QPL_LOW_LEVEL_EXTRACT_EXAMPLE
 *
 */
uint32_t perform_extract(qpl_job *job_ptr, uint8_t *buffer_ptr, uint32_t buffer_size);

/**
 * @brief  Finds values that are present in the `Source` and marks them into the `Set`.
 *
 * @param [in,out] job_ptr pointer onto user specified @ref qpl_job
 * @param [in] unpack_buffer_ptr  unpack buffer
 * @param [in] unpack_buffer_size unpack buffer size
 * @param [in] set_buffer_ptr     buffer to keep set
 * @param [in] set_buffer_size    set byte size
 *
 * @details For operation execution, you must set the following parameters in `qpl_job_ptr`:
 *      - Operation options:
 *          - @ref qpl_job.num_input_elements  - number elements for processing
 *          - @ref qpl_job.param_low          - drop count for low bits
 *          - @ref qpl_job.param_low          - drop count for high bits
 *      - `Source` properties (Source-1):
 *          - @ref qpl_job.next_in_ptr            - start address
 *          - @ref qpl_job.available_in           - number of available bytes
 *          - @ref qpl_job.src1_bit_width      - element bit-width
 *          - @ref qpl_job.parser            - stream format (@ref qpl_parser)
 *      - `Set` properties (`Output`):
 *          - @ref qpl_job.next_out_ptr           - start address of memory region to store result of operation
 *          - @ref qpl_job.available_in           - number of available bytes
 *          - @ref qpl_job.out_bit_width       - output format
 *
 * This function starts with the `Set` bit-vector of size 2^N, where N is the bit-width of the `source-1`.
 * After the operation is completed, the 1-bits in the `Set` specify all of the values from the `source-1`.
 * Function behavior is described with the pseudocode below:
 *      @code
 *          for( i = 0; i < source_length; i++ )
 *          {
 *              set[source[i]] = 1;
 *          }
 *      @endcode
 *
 * @note If the bit-width of the `source-1` elements is too large, the application can reduce the `Set`
 *       by cutting the high and/or low bits from each element. Bits can be dropped from the most-significant end by
 *       the @ref qpl_job.param_low and from the least-significant end by the @ref qpl_job.param_high.
 *       Then the size of the `Set` is 2^N, where N=(input_bit_width - drop_high_bits - drop_low_bits).
 *       When this feature is used, each `source-1` element has the appropriate number of bits dropped from each end,
 *       and the remaining bits are used to index into the `Set`.
 *
 * @note `Output` formats:
 *      - If output format is @ref qpl_ow_nom, output will be bit vector.
 *        Bit vector contains comparison results for each element in the stream (1 - condition is true for element,
 *        otherwise false)
 *      - If output format is @ref qpl_ow_8, @ref qpl_ow_16 or @ref qpl_ow_32, output will be vector of indexes .
 *        Indexes vector contains indexes of all elements that satisfy conditions in the table above.
 *
 * @warning
 *      If index is great than output extension, it will be error.
 *          - @ref qpl_ow_8  - max index is 255;
 *          - @ref qpl_ow_16 - max index is 65,535;
 *          - @ref qpl_ow_32 - max index is 4,294,967,295.
 *
 * @warning The bit-width of the `Source` (@ref qpl_job.src1_bit_width) must be greater than
 *          the sum of these two fields.
 * @warning If any of @ref qpl_job.available_in, @ref qpl_job.available_out, @ref qpl_job.num_input_elements is 0,
 *          it will be error.
 *
 * @return
 *    - @ref QPL_STS_OK
 *    - @ref QPL_STS_NULL_PTR_ERR
 *    - @ref QPL_STS_SIZE_ERR
 *    - @ref QPL_STS_BIT_WIDTH_ERR
 *    - @ref QPL_STS_SRC_IS_SHORT_ERR
 *    - @ref QPL_STS_SRC2_IS_SHORT_ERR
 *    - @ref QPL_STS_DST_IS_SHORT_ERR
 *    - @ref QPL_STS_DROP_BITS_OVERFLOW_ERR
 *    - @ref QPL_STS_PARSER_ERR
 *    - @ref QPL_STS_OPERATION_ERR
 *
 * Example of main usage:
 * @snippet low-level-api/find_unique_example.cpp QPL_LOW_LEVEL_FIND_UNIQUE_EXAMPLE
 *
 */
uint32_t perform_find_unique(qpl_job *job_ptr,
                             uint8_t *unpack_buffer_ptr,
                             uint32_t unpack_buffer_size,
                             uint8_t *set_buffer_ptr,
                             uint32_t set_buffer_size);

/**
 * @brief Checks that element from the `Source` is a part of a specified `Set`.
 *
 * @param [in,out] job_ptr pointer onto user specified @ref qpl_job
 * @param [in] unpack_buffer_ptr  unpack buffer
 * @param [in] unpack_buffer_size unpack buffer size
 * @param [in] set_buffer_ptr     buffer to keep set
 * @param [in] set_buffer_size    set byte size
 *
 * @details For operation execution, you must set the following parameters in `qpl_job_ptr`:
 *      - Operation options:
 *          - @ref qpl_job.num_input_elements  - number elements for processing
 *          - @ref qpl_job.param_low          - drop count for low bits
 *          - @ref qpl_job.param_low          - drop count for high bits
 *      - `Source` properties (Source-1):
 *          - @ref qpl_job.next_in_ptr            - start address
 *          - @ref qpl_job.available_in           - number of available bytes
 *          - @ref qpl_job.src1_bit_width      - element bit-width
 *          - @ref qpl_job.parser            - stream format (@ref qpl_parser)
 *      - `Set` properties (Source-2):
 *          - @ref qpl_job.next_src2_ptr          - start address
 *          - @ref qpl_job.available_src2         - number of bytes
 *          - @ref qpl_job.src2_bit_width      - element bit-width (always 1-bit)
 *      - `Destination` properties (`Output`):
 *          - @ref qpl_job.next_out_ptr           - start address of memory region to store result of operation
 *          - @ref qpl_job.available_in           - number of available bytes
 *          - @ref qpl_job.out_bit_width       - output format
 *
 * `Set` is presented as the `source-2` that contains a bit-vector with 2^N bits, where N is the element bit-width
 * of the `source-1`.
 * Function behavior is described with the pseudocode below:
 *      @code
 *           for( i = 0; i < source_length; i++ )
 *           {
 *              destination[i] = 1 & set[source[i]];
 *           }
 *      @endcode
 *
 * @note If the bit-width of the `Source` is too large, the application can reduce the set by cutting the high and/or
 *       low bits from each element. Bits can be dropped from the most-significant end by the @ref qpl_job.param_low
 *       and from the least-significant end by the @ref qpl_job.param_high. Then the size of the set is 2^N,
 *       where N=(input_bit_width - drop_high_bits - drop_low_bits).
 *       When this feature is used, each `Source` element has the appropriate number of bits dropped from each end,
 *       and the remaining bits are used to index into the `Set`.
 *
 * @note `Output` formats:
 *      - If output format is @ref qpl_ow_nom, output will be bit vector.
 *        Bit vector contains comparison results for each element in the stream (1 - condition is true for element,
 *        otherwise false)
 *      - If output format is @ref qpl_ow_8, @ref qpl_ow_16 or @ref qpl_ow_32, output will be vector of indexes.
 *        Indexes vector contains indexes of all elements that satisfy conditions in the table above.
 *
 * @warning
 *      If index is great than output extension, it will be error.
 *          - @ref qpl_ow_8  - max index is 255;
 *          - @ref qpl_ow_16 - max index is 65,535;
 *          - @ref qpl_ow_32 - max index is 4,294,967,295.
 *
 * @warning The bit-width of the `Source` must be greater than the sum of these two fields.
 * @warning If the `source-2` element bit-width differs from 1, it is the error.
 * @warning If any of @ref qpl_job.available_in, @ref qpl_job.available_src2, @ref qpl_job.available_out,
 *          @ref qpl_job.num_input_elements is 0, it is the error.
 *
 * @return
 *    - @ref QPL_STS_OK
 *    - @ref QPL_STS_NULL_PTR_ERR
 *    - @ref QPL_STS_SIZE_ERR
 *    - @ref QPL_STS_BIT_WIDTH_ERR
 *    - @ref QPL_STS_SRC_IS_SHORT_ERR
 *    - @ref QPL_STS_SRC2_IS_SHORT_ERR
 *    - @ref QPL_STS_DST_IS_SHORT_ERR
 *    - @ref QPL_STS_DROP_BITS_OVERFLOW_ERR
 *    - @ref QPL_STS_PARSER_ERR
 *    - @ref QPL_STS_OPERATION_ERR
 *
 * Example of main usage:
 * @snippet low-level-api/set_membership_example.cpp QPL_LOW_LEVEL_SET_MEMBERSHIP_EXAMPLE
 *
 */
uint32_t perform_set_membership(qpl_job *job_ptr,
                                uint8_t *unpack_buffer_ptr,
                                uint32_t unpack_buffer_size,
                                uint8_t *set_buffer_ptr,
                                uint32_t set_buffer_size);

/**
 * @brief Expands `Source` with using `Mask Stream` (bit-stream). `Mask Stream` modifies an output
 *        by the following way:
 *      - Each 0-bit element from `Mask Stream` writes a zero into the output stream
 *      - Each 1-bit element from `Mask Stream` writes the next entry from input stream
 *
 * @param [in,out] job_ptr pointer onto user specified @ref qpl_job
 * @param [in] unpack_buffer_ptr  unpack buffer
 * @param [in] unpack_buffer_size unpack buffer size
 * @param [in] output_buffer_ptr  output buffer
 * @param [in] output_buffer_size output buffer size
 * @param [in] mask_buffer_ptr    mask
 * @param [in] mask_buffer_size   mask byte size
 *
 * @details For operation execution, you must set the following parameters in `qpl_job_ptr`:
 *      - Operation options:
 *          - @ref qpl_job.num_input_elements  - number elements for processing
 *      - `Source` properties (Source-1):
 *          - @ref qpl_job.next_in_ptr            - start address
 *          - @ref qpl_job.available_in           - number of available bytes
 *          - @ref qpl_job.src1_bit_width      - element bit-width
 *          - @ref qpl_job.parser            - stream format (@ref qpl_parser)
 *      - `Mask Stream` properties (Source-2):
 *          - @ref qpl_job.next_src2_ptr          - start address
 *          - @ref qpl_job.available_src2         - number of bytes
 *          - @ref qpl_job.src2_bit_width      - mask bit-width
 *      - `Destination` properties (`Output`):
 *          - @ref qpl_job.next_out_ptr           - start address of memory region to store result of operation
 *          - @ref qpl_job.available_in           - number of available bytes
 *          - @ref qpl_job.out_bit_width       - output format (see note)
 *
 * @note  Number of `Output` elements is equal to the number of input elements in the `source-2`.
 *        So for this function, the @ref qpl_job.num_input_elements field actually contains the number of elements
 *        in the `source-2` rather than in `source-1`.
 *
 * @note `Output` formats:
 *      - If output format is @ref qpl_ow_nom, corresponding elements will be written as is.
 *      - If output format is @ref qpl_ow_8, @ref qpl_ow_16 or @ref qpl_ow_32, extracted elements will be written
 *        extended by zero.
 *          - @ref qpl_ow_8  - element bit-width will be extended to 8-bit;
 *          - @ref qpl_ow_16 - element bit-width will be extended to 16-bit;
 *          - @ref qpl_ow_32 - element bit-width will be extended to 32-bit.
 *      - If output format is @ref qpl_ow_8, @ref qpl_ow_16 or @ref qpl_ow_32, but @ref qpl_job.src1_bit_width == 1,
 *        output will contain indexes of non-zero elements
 *
 * @warning
 *      If index is great than output extension, it will be error.
 *          - @ref qpl_ow_8  - max index is 255;
 *          - @ref qpl_ow_16 - max index is 65,535;
 *          - @ref qpl_ow_32 - max index is 4,294,967,295.
 *
 * @warning If any of @ref qpl_job.available_in, @ref qpl_job.available_out, @ref qpl_job.num_input_elements is 0,
 *          it will be error.
 * @warning If `source-2` element bit-width differs from 1, it will be error.
 *
 * @return
 *    - @ref QPL_STS_OK
 *    - @ref QPL_STS_NULL_PTR_ERR
 *    - @ref QPL_STS_SIZE_ERR
 *    - @ref QPL_STS_BIT_WIDTH_ERR
 *    - @ref QPL_STS_SRC_IS_SHORT_ERR
 *    - @ref QPL_STS_DST_IS_SHORT_ERR
 *    - @ref QPL_STS_PARSER_ERR
 *    - @ref QPL_STS_OPERATION_ERR
 *
 * Example of main usage:
 * @snippet low-level-api/expand_example.cpp QPL_LOW_LEVEL_EXPAND_EXAMPLE
 *
 */
uint32_t perform_expand(qpl_job *job_ptr,
                        uint8_t *unpack_buffer_ptr,
                        uint32_t unpack_buffer_size,
                        uint8_t *output_buffer_ptr,
                        uint32_t output_buffer_size,
                        uint8_t *mask_buffer_ptr,
                        uint32_t mask_buffer_size);

/**
 * @brief Selects elements from the `Source` in accordance with indexes of non-zero elements in the `Index Stream`.
 *
 * @param [in,out] job_ptr pointer onto user specified @ref qpl_job
 * @param [in] unpack_buffer_ptr   unpack buffer
 * @param [in] unpack_buffer_size  unpack buffer size
 * @param [in] output_buffer_ptr   output buffer
 * @param [in] output_buffer_size  output buffer size
 * @param [in] mask_buffer_ptr     mask
 * @param [in] mask_buffer_size    mask byte size
 *
 * @details For operation execution, you must set the following parameters in `qpl_job_ptr`:
 *      - Operation options:
 *          - @ref qpl_job.num_input_elements  - number elements for processing
 *      - `Source` properties (`Source-1`):
 *          - @ref qpl_job.next_in_ptr            - start address
 *          - @ref qpl_job.available_in           - number of available bytes
 *          - @ref qpl_job.src1_bit_width      - elements bit-width
 *          - @ref qpl_job.parser            - stream format (@ref qpl_parser)
 *      - `Index Stream` properties (`Source-2`):
 *          - @ref qpl_job.next_src2_ptr          - start address
 *          - @ref qpl_job.available_src2         - number of available bytes
 *          - @ref qpl_job.src2_bit_width      - indexes bit-width
 *      - `Destination` properties (`Output`):
 *          - @ref qpl_job.next_out_ptr           - start address of memory region to store result of operation
 *          - @ref qpl_job.available_in           - number of available bytes
 *          - @ref qpl_job.out_bit_width       - output format (see note 2)
 *
 * @note 1: For this operation, `source-2` is a bit-vector that must contain at least as many elements as `source-1`.
 *         Those `source-1` items that correspond to 1-bits in `source-2` will be the output.
 *
 * @note `Output` formats:
 *      - If the output format is @ref qpl_ow_nom, corresponding elements will be written as-is.
 *      - If the output format is @ref qpl_ow_8, @ref qpl_ow_16 or @ref qpl_ow_32, extracted elements will be written
 *        and extended by zero.
 *          - @ref qpl_ow_8  - element bit-width will be extended to 8-bit;
 *          - @ref qpl_ow_16 - element bit-width will be extended to 16-bit;
 *          - @ref qpl_ow_32 - element bit-width will be extended to 32-bit.
 *      - If output format is @ref qpl_ow_8, @ref qpl_ow_16 or @ref qpl_ow_32, but @ref qpl_job.src1_bit_width == 1,
 *        output contains indexes of non-zero elements
 *
 * @warning
 *      If index is great than output extension, it will be error.
 *          - @ref qpl_ow_8  - max index is 255;
 *          - @ref qpl_ow_16 - max index is 65,535;
 *          - @ref qpl_ow_32 - max index is 4,294,967,295.
 *
 * @warning If any of @ref qpl_job.available_in, @ref qpl_job.available_out, @ref qpl_job.num_input_elements is 0,
 *          it is the error.
 * @warning If `source-2` element bit-width differs from 1, it will be error.
 *
 * @return
 *    - @ref QPL_STS_OK
 *    - @ref QPL_STS_NULL_PTR_ERR
 *    - @ref QPL_STS_SIZE_ERR
 *    - @ref QPL_STS_BIT_WIDTH_ERR
 *    - @ref QPL_STS_SRC_IS_SHORT_ERR
 *    - @ref QPL_STS_DST_IS_SHORT_ERR
 *    - @ref QPL_STS_PARSER_ERR
 *    - @ref QPL_STS_OPERATION_ERR
 *
 * Example of main usage:
 * @snippet low-level-api/select_example.cpp QPL_LOW_LEVEL_SELECT_EXAMPLE
 *
 */
uint32_t perform_select(qpl_job *job_ptr,
                        uint8_t *unpack_buffer_ptr,
                        uint32_t unpack_buffer_size,
                        uint8_t *output_buffer_ptr,
                        uint32_t output_buffer_size,
                        uint8_t *mask_buffer_ptr,
                        uint32_t mask_buffer_size);

/**
 * @brief Replicates each element in the `Source` by the number of times defined by the `Counter Stream` element
 *        with the same index.
 *
 * @param [in,out] job_ptr pointer onto user specified @ref qpl_job
 * @param [in] unpack_buffer_ptr   unpack buffer
 * @param [in] unpack_buffer_size  unpack buffer size
 * @param [in] output_buffer_ptr   output buffer
 * @param [in] output_buffer_size  output buffer size
 * @param [in] mask_buffer_ptr     mask
 * @param [in] mask_buffer_size    mask byte size
 *
 * @details For operation execution, you must set the following parameters in `qpl_job_ptr`:
 *      - Operation options:
 *          - @ref qpl_job.num_input_elements  - number elements for processing
 *      - `Counters Stream` properties (`Source-1`):
 *          - @ref qpl_job.next_in_ptr            - start address
 *          - @ref qpl_job.available_in           - number of available bytes
 *          - @ref qpl_job.src1_bit_width         - counters bit-width
 *          - @ref qpl_job.parser                 - stream format (@ref qpl_parser)
 *      - `Source` properties (`Source-2`):
 *          - @ref qpl_job.next_src2_ptr          - start address
 *          - @ref qpl_job.available_src2         - number of bytes
 *          - @ref qpl_job.src2_bit_width         - element bit-width
 *      - `Destination` properties (`Output`):
 *          - @ref qpl_job.next_out_ptr           - start address of memory region to store result of operation
 *          - @ref qpl_job.available_in           - number of available bytes
 *          - @ref qpl_job.out_bit_width          - output format (see note)
 *
 * @note  For this operation, the `source-1` is an array of counters that specifies
 *        how many times each element from the `source-2` should be repeated in the output array
 * @note  The @ref qpl_job.num_input_elements field specifies the number of elements
 *        in `Counters Stream` (`source-1`).
 *
 * @note  For this operation, the `source-2` is a packed array of unsigned integers of any
 *        width (@ref qpl_job.src2_bit_width) from 1 to 32 bits.
 *
 * @note The `Counter Stream` can be presented with the following formats:
 *      - If the @ref qpl_job.src1_bit_width is 8 or 16, each counter specifies the number of times
 *        the corresponding element in the input stream must be replicated. In this case, the number of elements
 *        in the `source-2` == the number of counters.
 *      - If the bit width of `source-1` is 32, each element of the src1 specifies the cumulative number of elements
 *        in the output for that point.
 *        In this case, the number of elements in the `source-2` == the `numberOfCounters - 1`, due to each element of the `source-1` specifies the cumulative number of elements in the output for that point.
 *
 * @note If the counter value is 0, the corresponding element is dropped.
 *
 * @note `Output` formats:
 *      - If the output format is @ref qpl_ow_nom, corresponding elements are written as-is.
 *      - If the output format is @ref qpl_ow_8, @ref qpl_ow_16 or @ref qpl_ow_32, extracted elements will be written
 *        and extended by zero.
 *          - @ref qpl_ow_8  - element bit-width will be extended to 8-bit;
 *          - @ref qpl_ow_16 - element bit-width will be extended to 16-bit;
 *          - @ref qpl_ow_32 - element bit-width will be extended to 32-bit.
 *      - If the output format is @ref qpl_ow_8, @ref qpl_ow_16 or @ref qpl_ow_32, but @ref qpl_job.src2_bit_width == 1,
 *        the output will contain indexes of non-zero elements
 *
 * @warning
 *      If index is great than output extension, it will be error.
 *          - @ref qpl_ow_8  - max index is 255;
 *          - @ref qpl_ow_16 - max index is 65,535;
 *          - @ref qpl_ow_32 - max index is 4,294,967,295.
 *
 * @warning The `source-1` bit-width must be 8, 16, or 32, otherwise, it is the error.
 * @warning If any of @ref qpl_job.available_in, @ref qpl_job.available_out, @ref qpl_job.num_input_elements is 0,
 *          it will be error.
 * @warning The `Output` element bit-width (@ref qpl_job.out_bit_width) must not be less than
 *          the `source-2` element bit-width.
 *
 * @return
 *    - @ref QPL_STS_OK
 *    - @ref QPL_STS_NULL_PTR_ERR
 *    - @ref QPL_STS_SIZE_ERR
 *    - @ref QPL_STS_BIT_WIDTH_ERR
 *    - @ref QPL_STS_SRC_IS_SHORT_ERR
 *    - @ref QPL_STS_DST_IS_SHORT_ERR
 *    - @ref QPL_STS_OUT_FORMAT_ERR
 *
 * Example of main usage:
 * @snippet low-level-api/rle_burst_example.cpp QPL_LOW_LEVEL_SCAN_EXAMPLE
 *
 */
uint32_t perform_rle_burst(qpl_job *job_ptr,
                           uint8_t *unpack_buffer_ptr,
                           uint32_t unpack_buffer_size,
                           uint8_t *output_buffer_ptr,
                           uint32_t output_buffer_size,
                           uint8_t *mask_buffer_ptr,
                           uint32_t mask_buffer_size);
/** @} */

#endif // JOB_PARSER_H
