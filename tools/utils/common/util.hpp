/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TEST_UTIL_HPP
#define QPL_TEST_UTIL_HPP

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <vector>

#include "qpl/qpl.h"

#include "qpl_test_environment.hpp"
#include "random_generator.h"
#include "test_hw_status.h"

#define HIGH_BIT_MASK   0x80
#define BYTE_BIT_LENGTH 8U

namespace qpl::test {
static std::vector<hw_compression_level> hw_levels {hw_compression_level::SMALL, hw_compression_level::LARGE,
                                                    hw_compression_level::HW_LEVEL_1, hw_compression_level::HW_LEVEL_2,
                                                    hw_compression_level::HW_LEVEL_3};

static std::vector<sw_compression_level> sw_levels {sw_compression_level::LEVEL_0, sw_compression_level::LEVEL_1,
                                                    sw_compression_level::LEVEL_2, sw_compression_level::LEVEL_3,
                                                    sw_compression_level::LEVEL_4, sw_compression_level::LEVEL_9};

static constexpr size_t   max_input_bit_width              = 32;
static constexpr uint32_t bit_bit_width                    = 1U;
static constexpr uint32_t additional_bytes_for_compression = 100U;

constexpr uint32_t max_bit_index              = 7U;
constexpr uint32_t bit_to_byte_shift_offset   = 3U;
constexpr uint32_t bib_eobs_bfinal_block_size = 3857U; // Every (except for the last one) has exactly this size in bytes
constexpr uint32_t bib_eobs_decompressed_size =
        111261U; // Originally compressed file was bib, so assign bib file's size to this variable

enum source_sizes_sequence_t { generic };

// Status printer
class hw_accelerator_status {
public:
    hw_accelerator_status(::qpl_test_hw_accelerator_status status) : _status {status} {}

    operator ::qpl_test_hw_accelerator_status() const { return _status; }

private:
    ::qpl_test_hw_accelerator_status _status;
};

static std::ostream& operator<<(std::ostream& strm, hw_accelerator_status const& status_ex) {
    const ::qpl_test_hw_accelerator_status status = status_ex;
    strm << status;
    switch (status) {
        case QPL_TEST_HW_ACCELERATOR_STATUS_OK: strm << ": QPL_TEST_HW_ACCELERATOR_STATUS_OK"; break;
        case QPL_TEST_HW_ACCELERATOR_NULL_PTR_ERR:
            strm << ": QPL_TEST_HW_ACCELERATOR_NULL_PTR_ERR";
            strm << " - null pointer error";
            break;
        case QPL_TEST_HW_ACCELERATOR_LIBACCEL_NOT_FOUND:
            strm << ": QPL_TEST_HW_ACCELERATOR_LIBACCEL_NOT_FOUND";
            strm << " - proper version of libaccel-config.so.1 was not found in the /usr/lib64";
            break;
        case QPL_TEST_HW_ACCELERATOR_LIBACCEL_ERROR:
            strm << ": QPL_TEST_HW_ACCELERATOR_LIBACCEL_ERROR";
            strm << " - libaccel-config internal error";
            break;
        case QPL_TEST_HW_ACCELERATOR_WORK_QUEUES_NOT_AVAILABLE:
            strm << ": QPL_TEST_HW_ACCELERATOR_WORK_QUEUES_NOT_AVAILABLE";
            strm << " - No enabled shared WQ";
            break;
        case QPL_TEST_HW_ACCELERATOR_SUPPORT_ERR:
            strm << ": QPL_TEST_HW_ACCELERATOR_SUPPORT_ERR";
            strm << " - System doesn't support accelerator";
            break;
        case QPL_TEST_HW_ACCELERATOR_WQ_IS_BUSY:
            strm << ": QPL_TEST_HW_ACCELERATOR_WQ_IS_BUSY";
            strm << " - Work queue is busy with task processing";
            break;
    }
    return strm;
}

static uint32_t bits_to_bytes(uint32_t bits_count) {
    const uint32_t bytes_count = (bits_count + max_bit_index) >> bit_to_byte_shift_offset;
    return bytes_count;
}

static std::string OperationToString(qpl_operation operation) {
    switch (operation) {
        case qpl_op_scan_eq: return "ScanEQ";

        case qpl_op_scan_ne: return "ScanNE";

        case qpl_op_scan_lt: return "ScanLT";

        case qpl_op_scan_le: return "ScanLE";

        case qpl_op_scan_gt: return "ScanGT";

        case qpl_op_scan_ge: return "ScanGE";

        case qpl_op_scan_range: return "ScanRange";

        case qpl_op_scan_not_range: return "ScanNotRange";

        case qpl_op_extract: return "Extract";

        case qpl_op_select: return "Select";

        case qpl_op_expand: return "Expand";

        case qpl_op_compress: return "Compress";

        case qpl_op_decompress: return "Decompress";

        case qpl_op_crc64: return "CRC";

        default: return "";
    }
}

static std::string ParserToString(qpl_parser parser) {
    switch (parser) {
        case qpl_p_le_packed_array: return "LE";

        case qpl_p_be_packed_array: return "BE";

        case qpl_p_parquet_rle: return "PRLE";

        default: return "";
    }
}

static uint8_t* set_format_count(uint32_t format, uint32_t count, uint8_t* destination_ptr) {
    uint8_t value = (uint8_t)format & 1U;   // format is 1 low bit
    value |= (uint8_t)(count & 0x3f) << 1U; // get 6 significant bits of count
    count >>= 6U;                           // remove these 6 bits from count
    if (0U < count)                         // if there are more bits
    {
        value |= HIGH_BIT_MASK;                         // mark var-int val
        *(destination_ptr++) = value;                   // put 1st byte to dst
        value                = (uint8_t)(count & 0x7f); // get next bits from count
        count >>= 7U;                                   // remove these 7 bits from count
    } else {
        *(destination_ptr++) = value; // put last byte to dst
        return destination_ptr;
    }
    if (0U < count) // if there are more bits
    {
        value |= HIGH_BIT_MASK;                         // mark var-int val
        *(destination_ptr++) = value;                   // put 2nd byte to dst
        value                = (uint8_t)(count & 0x7f); // get next bits from count
        count >>= 7U;                                   // remove these 7 bits from count
    } else {
        *(destination_ptr++) = value; // put last byte to dst
        return destination_ptr;
    }
    if (0 < count) // if there are more bits
    {
        value |= HIGH_BIT_MASK;                         // mark var-int val
        *(destination_ptr++) = value;                   // put 3rd byte to dst
        value                = (uint8_t)(count & 0x7f); // get next bits from count
        count >>= 7;                                    // remove these 7 bits from count
    } else {
        *(destination_ptr++) = value; // put last byte to dst
        return destination_ptr;
    }
    if (0 < count) // if there are more bits
    {
        value |= HIGH_BIT_MASK;                         // mark var-int val
        *(destination_ptr++) = value;                   // put 4th byte to dst
        value                = (uint8_t)(count & 0x1f); // get last 5 bits from count
    } else {
        *(destination_ptr++) = value; // put last byte to dst
        return destination_ptr;
    }
    *(destination_ptr++) = value; // put last byte to dst
    return destination_ptr;
}

static uint32_t qpl_output_to_uint(qpl_out_format output) {
    switch (output) {
        case qpl_ow_nom: return 1U;

        case qpl_ow_8: return 8U;

        case qpl_ow_16: return 16U;

        case qpl_ow_32: return 32U;

        default: throw std::exception();
    }
}

static qpl_out_format uint_to_qpl_output(uint32_t output) {
    switch (output) {
        case 1U: return qpl_ow_nom;

        case 8U: return qpl_ow_8;

        case 16U: return qpl_ow_16;

        case 32U: return qpl_ow_32;

        default: return qpl_ow_nom;
    }
}

static uint32_t get_second_source_bit_length(qpl_operation operation, uint32_t first_source_bit_width,
                                             uint32_t first_source_number_of_elements) {
    uint32_t result_bit_length = 0U;

    switch (operation) {
        case qpl_op_select: result_bit_length = first_source_number_of_elements; break;

        case qpl_op_expand: result_bit_length = first_source_number_of_elements; break;

        default: result_bit_length = 0U; break;
    }

    return result_bit_length;
}

/**
     * @brief Return address aligned to `alignment` boundary.
     *
     * @warning It is user responsibility to ensure correct alignment and enough space
     * in ptr_in buffer.
    */
static inline void align_ptr(size_t alignment, void* ptr_in, void** aligned_ptr_out) {
    const std::uintptr_t mask = ~(std::uintptr_t)(alignment - 1U);
    *aligned_ptr_out          = (void*)(((std::uintptr_t)ptr_in + alignment - 1U) & mask);
}

/**
     * @brief Generates a vector containing randomized dictionary sizes from selected range.
    */
static inline auto get_dictionary_lengths() {
    std::vector<uint32_t> result;

    auto generate = [&](uint32_t bound_low, uint32_t bound_high, uint32_t step) -> auto {
        for (uint32_t i = bound_low; i <= bound_high; i += step) {
            result.push_back(i);
        }
    };

    generate(1, 32, 2);
    generate(33, 4094, (4094 - 32) / 15);
    generate(4095, 4097, 1);

    return result;
}

// Skips entire test for specified path, will also skip all remaining test cases
#define QPL_SKIP_TEST_FOR(path) \
    if (qpl::test::util::TestEnvironment::GetInstance().GetExecutionPath() == (path)) GTEST_SKIP()

// Skips entire test for specified path, will also skip all remaining test cases
// Prints a message w/ skipped test
#define QPL_SKIP_TEST_FOR_VERBOSE(path, message) \
    if (qpl::test::util::TestEnvironment::GetInstance().GetExecutionPath() == (path)) GTEST_SKIP_(message)

// Will skip entire test case if expr evaluates to true, will also skip all remaining test cases
// Prints a message w/ skipped testcase
#define QPL_SKIP_TEST_FOR_EXPR_VERBOSE(expr, message) \
    if (expr) { GTEST_SKIP_(message); }

// Will skip a specific test case, will continue with running other test cases in test
// Prints a message w/ skipped testcase
#define QPL_SKIP_TC_TEST(expr, message)                          \
    if (expr) {                                                  \
        std::cout << "skipped test case: " << (message) << '\n'; \
        return;                                                  \
    }

} // namespace qpl::test

#endif // QPL_TEST_UTIL_HPP
