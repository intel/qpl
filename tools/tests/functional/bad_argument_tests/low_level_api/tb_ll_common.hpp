/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TESTS_FUNCTIONAL_BADARG_TESTS_LOW_LEVEL_API_COMMON_H_
#define QPL_TESTS_FUNCTIONAL_BADARG_TESTS_LOW_LEVEL_API_COMMON_H_

#include <array>
#include <memory>

#include "qpl/qpl.h"

#include "gtest/gtest.h"
#include "qpl_test_environment.hpp"

// tool_generator
#include "random_generator.h"

// tests_common
#include "execution_wrapper.hpp"
#include "test_name_format.hpp"

#define QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(operation, test) \
    QPL_FIXTURE_TEST(QPL_LOW_LEVEL_SUITE_NAME(tb, operation), test, JobFixture)

using TestEnviroment = qpl::test::util::TestEnvironment;

constexpr uint32_t               SOURCE_ARRAY_SIZE        = 100U;
constexpr uint32_t               MASK_ARRAY_SIZE          = 100U;
constexpr uint32_t               DESTINATION_ARRAY_SIZE   = 100U;
constexpr uint32_t               ELEMENTS_TO_PROCESS      = 10U;
constexpr uint32_t               INPUT_BIT_WIDTH          = 1U;
constexpr uint32_t               MASK_BIT_WIDTH           = 1U;
constexpr uint32_t               DROP_INITIAL_BYTES       = 0U;
constexpr uint32_t               OPERATION_FLAGS          = 0U;
constexpr qpl_parser             INPUT_FORMAT             = qpl_p_le_packed_array;
constexpr qpl_out_format         OUTPUT_BIT_WIDTH         = qpl_ow_nom;
constexpr qpl_path_t             PATH                     = qpl_path_auto;
constexpr qpl_path_t             INCORRECT_PATH           = static_cast<qpl_path_t>(qpl_path_software + 1U);
constexpr qpl_compression_levels INCORRECT_LEVEL          = static_cast<qpl_compression_levels>(qpl_high_level + 1U);
constexpr uint32_t               NOT_APPLICABLE_PARAMETER = 0U;
constexpr uint32_t               INDEX_ARRAY_SIZE         = 3U;
constexpr qpl_mini_block_size    INDEX_MBLK_SIZE          = qpl_mblk_size_8k;

#define LAST_INPUT_PARSER  qpl_p_parquet_rle
#define LAST_OUTPUT_FORMAT qpl_ow_32

#define RESERVED_OPCODES_COUNT 20
extern uint8_t reserved_op_codes[RESERVED_OPCODES_COUNT];

using flags_t = uint32_t;

enum class operation_group_e { filter_single_source, filter_double_source };

void set_input_stream(qpl_job* job_ptr, uint8_t* source_ptr, uint32_t source_size, uint32_t input_bit_width,
                      uint32_t elements_to_process, qpl_parser parser);

void set_mask_stream(qpl_job* job_ptr, uint8_t* mask_ptr, uint32_t mask_size, uint32_t mask_bit_width);

void set_output_stream(qpl_job* job_ptr, uint8_t* destination_ptr, uint32_t destination_size,
                       qpl_out_format output_bit_width);

void set_operation_properties(qpl_job* job_ptr, uint32_t drop_initial_bytes, uint32_t flags, qpl_operation operation);

void set_range(qpl_job* job_ptr, uint32_t low_param, uint32_t high_param);

void set_indexing_parameters(qpl_job* job_ptr, qpl_mini_block_size mini_block_size, uint64_t* index_array,
                             uint32_t index_array_size);

/* ------ Commons Analytic Checks ------ */

static inline void check_input_stream_validation(qpl_job* const job_ptr, qpl_operation operation, flags_t flags) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> mask {};

    // Preset correct parameters
    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);
    set_mask_stream(job_ptr, mask.data(), MASK_ARRAY_SIZE, MASK_BIT_WIDTH);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, flags, operation);

    // Null pointer check
    set_input_stream(job_ptr, nullptr, SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_NULL_PTR_ERR) << "Fail on: source == nullptr";

    // Source size checks
    set_input_stream(job_ptr, source.data(), 0U, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_SIZE_ERR) << "Fail on: source size == 0";

    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, 0U, INPUT_FORMAT);
    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_SIZE_ERR) << "Fail on: number elements == 0";

    // Bit-width checks
    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, 0U, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_BIT_WIDTH_ERR) << "Fail on: bit width != [1:32]";

    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, 33U, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_BIT_WIDTH_ERR) << "Fail on: bit width != [1:32]";

    // Input format check
    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS,
                     (qpl_parser)(LAST_INPUT_PARSER + 1U));
    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_PARSER_ERR) << "Fail on: parser";

    if (TestEnviroment::GetInstance().GetExecutionPath() != qpl_path_hardware) { // @todo check for hw
        // More input needed
        if (!(flags & QPL_FLAG_DECOMPRESS_ENABLE)) {
            // Bit-width checks for PRLE parser
            source[0] = 0U;
            set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, 10U, ELEMENTS_TO_PROCESS, qpl_p_parquet_rle);
            EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_BIT_WIDTH_ERR)
                    << "Fail on: bit width != [1:32], parser = PRLE";

            source[0] = 33U;
            set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, 10U, ELEMENTS_TO_PROCESS, qpl_p_parquet_rle);
            EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_BIT_WIDTH_ERR)
                    << "Fail on: bit width != [1:32], parser = PRLE";

            source[0] = 0U;

            // Source bit width for PRLE format is read directly from 1st byte of input stream
            source[0] = 1U;
            set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, 0U,
                             INPUT_BIT_WIDTH * SOURCE_ARRAY_SIZE / INPUT_BIT_WIDTH + 32U, qpl_p_parquet_rle);
            EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_SRC_IS_SHORT_ERR) << "Fail on: size < needed";
        }
    }

    if (TestEnviroment::GetInstance().GetExecutionPath() == qpl_path_hardware) {
        job_ptr->available_in    = 0U;
        job_ptr->ignore_end_bits = 0U;
        EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_SIZE_ERR);
    }
}

static inline void check_double_source_stream_limits_validation(qpl_job* const job_ptr, qpl_operation operation,
                                                                flags_t  flags,
                                                                uint32_t input_bit_width = INPUT_BIT_WIDTH) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> mask {};

    // Preset correct parameters
    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, input_bit_width, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);
    set_mask_stream(job_ptr, mask.data(), MASK_ARRAY_SIZE, MASK_BIT_WIDTH);

    // Drop limit check
    set_operation_properties(job_ptr, 1U, QPL_FLAG_DECOMPRESS_ENABLE, operation);
    ASSERT_EQ(QPL_STS_DROP_BYTES_ERR, qpl::test::run_job_api(job_ptr));
}

static inline void check_output_stream_validation(qpl_job* const job_ptr, qpl_operation operation, flags_t flags) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, MASK_ARRAY_SIZE>        mask {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    // Preset correct parameters
    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
    set_mask_stream(job_ptr, mask.data(), MASK_ARRAY_SIZE, MASK_BIT_WIDTH);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, flags, operation);

    // Null pointer check
    set_output_stream(job_ptr, nullptr, DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);
    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_NULL_PTR_ERR) << "Fail on: destination == nullptr";

    // Output stream size check
    set_output_stream(job_ptr, destination.data(), 0U, OUTPUT_BIT_WIDTH);
    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_SIZE_ERR) << "Fail on: destination size == 0";

    // Output format check
    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, (qpl_out_format)(LAST_OUTPUT_FORMAT + 1U));
    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_OUT_FORMAT_ERR) << "Fail on: output format error";
}

static inline void check_mask_stream_validation(qpl_job* const job_ptr, qpl_operation operation, flags_t flags) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, MASK_ARRAY_SIZE>        mask {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    // Preset correct parameters
    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, flags, operation);

    // Null pointer check
    set_mask_stream(job_ptr, nullptr, MASK_ARRAY_SIZE, MASK_BIT_WIDTH);
    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_NULL_PTR_ERR) << "Fail on: mask == nullptr";

    // Vector size check
    set_mask_stream(job_ptr, mask.data(), 0U, MASK_BIT_WIDTH);
    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_SIZE_ERR) << "Fail on: mask size == 0";

    // Bit-width check
    set_mask_stream(job_ptr, mask.data(), MASK_ARRAY_SIZE, 2U);
    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_BIT_WIDTH_ERR) << "Fail on: mask bit-width != 1";
}

template <operation_group_e group = operation_group_e::filter_single_source>
static inline void check_initial_output_index_verification(qpl_job* const job_ptr, qpl_operation operation,
                                                           flags_t flags) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, MASK_ARRAY_SIZE>        mask {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    // Preset correct parameters
    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
    set_mask_stream(job_ptr, mask.data(), MASK_ARRAY_SIZE, MASK_BIT_WIDTH);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, flags, operation);

    if constexpr (group == operation_group_e::filter_single_source) {
        set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, qpl_ow_8);
        job_ptr->initial_output_index = 1U << 8U;
        EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_OUTPUT_OVERFLOW_ERR)
                << "Fail on: initial output index error (mode: qpl_ow_8)";

        set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, qpl_ow_16);
        job_ptr->initial_output_index = 1U << 16U;
        EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_OUTPUT_OVERFLOW_ERR)
                << "Fail on: initial output index error (mode: qpl_ow_16)";
    } else {
        set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, qpl_ow_nom);
        job_ptr->initial_output_index = 1U;
        EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_INVALID_PARAM_ERR)
                << "Fail on: initial output index error for double source operation";
    }
}

template <operation_group_e group = operation_group_e::filter_single_source>
static inline void check_drop_initial_bytes_verification(qpl_job* const job_ptr, qpl_operation operation,
                                                         flags_t flags) {
    constexpr uint32_t source_size = UINT16_MAX + 1U;
    constexpr uint32_t mask_size   = (source_size + 7U) / 8U;

    std::array<uint8_t, source_size>            source {};
    std::array<uint8_t, mask_size>              mask {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    // Preset correct parameters
    set_input_stream(job_ptr, source.data(), source.size(), INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
    set_output_stream(job_ptr, destination.data(), destination.size(), OUTPUT_BIT_WIDTH);
    set_mask_stream(job_ptr, mask.data(), mask.size(), MASK_BIT_WIDTH);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, flags, operation);

    if constexpr (group == operation_group_e::filter_single_source) {
        job_ptr->drop_initial_bytes = UINT16_MAX + 1U;
        EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_DROP_BYTES_ERR) << "Fail on: drop initial bytes > 64k";
        job_ptr->drop_initial_bytes = DROP_INITIAL_BYTES;

        if (!(flags & QPL_FLAG_DECOMPRESS_ENABLE)) {
            job_ptr->available_in       = SOURCE_ARRAY_SIZE;
            job_ptr->drop_initial_bytes = SOURCE_ARRAY_SIZE + 1;
            EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_DROP_BYTES_ERR)
                    << "Fail on: drop initial bytes > avail_in";
            job_ptr->drop_initial_bytes = DROP_INITIAL_BYTES;
            job_ptr->available_in       = source.size();
        }

    } else {
        job_ptr->drop_initial_bytes = 1U;
        EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_DROP_BYTES_ERR)
                << "Fail on: `drop initial bytes` feature is not supported";
        job_ptr->drop_initial_bytes = DROP_INITIAL_BYTES;
    }
}

template <operation_group_e group = operation_group_e::filter_single_source>
static inline void check_buffer_overlap(qpl_job* const job_ptr, qpl_operation operation, flags_t flags) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE + MASK_ARRAY_SIZE + DESTINATION_ARRAY_SIZE> buffer {};

    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, flags, operation);

    auto source_begin      = buffer.data();
    auto destination_begin = source_begin + SOURCE_ARRAY_SIZE - 1U;

    set_input_stream(job_ptr, source_begin, SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
    set_output_stream(job_ptr, destination_begin, DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);

    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_BUFFER_OVERLAP_ERR) << "Fail on: destination overlaps source";

    destination_begin = buffer.data();
    source_begin      = destination_begin + DESTINATION_ARRAY_SIZE - 1U;

    set_input_stream(job_ptr, source_begin, SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
    set_output_stream(job_ptr, destination_begin, DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);

    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_BUFFER_OVERLAP_ERR) << "Fail on: source overlaps destination";

    if constexpr (group == operation_group_e::filter_double_source) {
        auto mask_begin   = buffer.data();
        source_begin      = mask_begin + MASK_ARRAY_SIZE - 1;
        destination_begin = mask_begin + MASK_ARRAY_SIZE + SOURCE_ARRAY_SIZE;

        set_mask_stream(job_ptr, mask_begin, MASK_ARRAY_SIZE, MASK_BIT_WIDTH);
        set_input_stream(job_ptr, source_begin, SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
        set_output_stream(job_ptr, destination_begin, DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);

        EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_BUFFER_OVERLAP_ERR) << "Fail on: source overlaps mask";

        destination_begin = mask_begin + MASK_ARRAY_SIZE - 1;
        source_begin      = mask_begin + MASK_ARRAY_SIZE + DESTINATION_ARRAY_SIZE;

        set_input_stream(job_ptr, source_begin, SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
        set_output_stream(job_ptr, destination_begin, DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);

        EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_BUFFER_OVERLAP_ERR) << "Fail on: destination overlaps mask";

        source_begin      = buffer.data();
        mask_begin        = source_begin + SOURCE_ARRAY_SIZE - 1;
        destination_begin = source_begin + MASK_ARRAY_SIZE + SOURCE_ARRAY_SIZE;

        set_mask_stream(job_ptr, mask_begin, MASK_ARRAY_SIZE, MASK_BIT_WIDTH);
        set_input_stream(job_ptr, source_begin, SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
        set_output_stream(job_ptr, destination_begin, DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);

        EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_BUFFER_OVERLAP_ERR) << "Fail on: mask overlaps source";

        destination_begin = buffer.data();
        mask_begin        = destination_begin + DESTINATION_ARRAY_SIZE - 1;
        source_begin      = destination_begin + MASK_ARRAY_SIZE + DESTINATION_ARRAY_SIZE;

        set_mask_stream(job_ptr, mask_begin, MASK_ARRAY_SIZE, MASK_BIT_WIDTH);
        set_input_stream(job_ptr, source_begin, SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
        set_output_stream(job_ptr, destination_begin, DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);

        EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_BUFFER_OVERLAP_ERR) << "Fail on: mask overlaps destination";
    }
}

static inline void analytic_with_mask_check_num_elements_with_index_output(qpl_job* const job_ptr,
                                                                           qpl_operation operation, flags_t flags) {
    const uint32_t size = 65537U;

    std::array<uint8_t, size> source {};
    std::array<uint8_t, size> mask {};
    std::array<uint8_t, size> destination {};

    set_mask_stream(job_ptr, mask.data(), 257, MASK_BIT_WIDTH);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, flags, operation);

    set_input_stream(job_ptr, source.data(), size, INPUT_BIT_WIDTH, 257, INPUT_FORMAT);
    set_output_stream(job_ptr, destination.data(), size, qpl_ow_8);
    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_OUTPUT_OVERFLOW_ERR)
            << "Fail on: number of elements exceeds max index value error (mode: qpl_ow_8)";

    set_mask_stream(job_ptr, mask.data(), size, MASK_BIT_WIDTH);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, flags, operation);

    set_input_stream(job_ptr, source.data(), size, INPUT_BIT_WIDTH, size + 1, INPUT_FORMAT);
    set_output_stream(job_ptr, destination.data(), size, qpl_ow_16);
    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_OUTPUT_OVERFLOW_ERR)
            << "Fail on: number of elements exceeds max index value error (mode: qpl_ow_16)";
}

static inline void check_source_size_not_enough_to_hold_num_input_elements(qpl_job* const job_ptr,
                                                                           qpl_operation operation, flags_t flags) {
    constexpr uint32_t size               = 2048U;
    constexpr uint32_t input_bit_width    = 5U;
    constexpr uint32_t number_of_elements = (size * 8U) / input_bit_width + 3U;

    std::array<uint8_t, size> source {};
    std::array<uint8_t, size> mask {};
    std::array<uint8_t, size> destination {};

    // Preset correct parameters
    set_mask_stream(job_ptr, mask.data(), size, MASK_BIT_WIDTH);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, flags, operation);

    set_input_stream(job_ptr, source.data(), size, input_bit_width, number_of_elements, INPUT_FORMAT);
    set_output_stream(job_ptr, destination.data(), size, qpl_ow_16);
    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_SRC_IS_SHORT_ERR)
            << "Fail on: provided number of elements does not fit into the source size";
}

#endif //QPL_TESTS_FUNCTIONAL_BADARG_TESTS_LOW_LEVEL_API_COMMON_H_
