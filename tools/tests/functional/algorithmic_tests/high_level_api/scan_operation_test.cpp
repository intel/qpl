/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <vector>
#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "qpl/cpp_api/operations/analytics/scan_operation.hpp"

#include "ta_hl_common.hpp"
#include "high_level_api_util.hpp"
#include "source_provider.hpp"
#include "qpl/qpl.h"
#include "qpl_api_ref.h"
#include "format_generator.hpp"

namespace qpl::test {
static inline auto get_test_case_info(const uint32_t seed,
                                      const uint32_t number_of_elements,
                                      const uint32_t input_bit_width,
                                      const uint32_t output_bit_width,
                                      const uint32_t boundary,
                                      const bool is_inclusive = 0) -> std::string {
    std::string result = std::string("\nTest parameters:\n") + "Seed: " + std::to_string(seed) +
                         "\n" + "Number of elements: " + std::to_string(number_of_elements) +
                         "\n" + "Input bit width: " + std::to_string(input_bit_width) +
                         "\n" + "Output bit width: " + std::to_string(output_bit_width) +
                         "\n" + "Boundary: " + std::to_string(boundary) +
                         "\n" + "Is inclusive: " + std::to_string(is_inclusive) +
                         "\n";

    return result;
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(scan, less_little_endian) {
    const auto comparator       = qpl::less;
    const auto parser           = qpl::little_endian_packed_array;
    const auto low_level_parser = convert_parser(parser);
    const auto seed             = util::TestEnvironment::GetInstance().GetSeed();
    const auto execution_path   = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    qpl_get_job_size(execution_path, &job_size);

    auto reference_job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto reference_job_ptr    = reinterpret_cast<qpl_job *>(reference_job_buffer.get());

    reference_job_ptr->parser = low_level_parser;

    auto source_length    = format_generator::generate_length_sequence();
    auto max_output_bytes = *(--source_length.end()) * 4;

    for (auto number_of_elements : source_length) {
        reference_job_ptr->num_input_elements = static_cast<uint32_t>(number_of_elements);
        for (bool is_inclusive : {false, true}) {
            reference_job_ptr->op = (is_inclusive ? qpl_op_scan_le : qpl_op_scan_lt);

            for (uint32_t output_bit_width : {1, 8, 16, 32}) {
                if (output_bit_width != 1u && (1ull << output_bit_width) <= number_of_elements) {
                    continue;
                }

                for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
                    source_provider source_generator(number_of_elements,
                                                     input_bit_width,
                                                     seed,
                                                     low_level_parser);

                    auto source = source_generator.get_source();

                    uint32_t output_element_bit_width = output_bit_width == 1u
                                                        ? input_bit_width
                                                        : output_bit_width;
                    uint32_t destination_bit_size     = output_element_bit_width * number_of_elements;
                    uint32_t destination_size         = (destination_bit_size + 7u) >> 3u;

                    std::vector<uint8_t> destination(destination_size, 0);
                    std::vector<uint8_t> reference_destination(destination_size, 0);

                    uint32_t alphabet_power = (1u << input_bit_width);
                    uint32_t boundary       = alphabet_power >> 1u;

                    auto test_case_info = get_test_case_info(seed,
                                                             number_of_elements,
                                                             input_bit_width,
                                                             output_bit_width,
                                                             boundary,
                                                             is_inclusive);

                    auto scan_operation = typename scan_operation::builder(comparator, boundary)
                            .parser<parser>(number_of_elements)
                            .output_vector_width(output_bit_width)
                            .input_vector_width(input_bit_width)
                            .is_inclusive(is_inclusive)
                            .build();

                    auto scan_operation_moved = typename scan_operation::builder(scan_operation)
                        .comparator(comparator)
                        .boundary(boundary)
                        .build();

                    reference_job_ptr->param_low      = boundary;
                    reference_job_ptr->src1_bit_width = input_bit_width;
                    reference_job_ptr->out_bit_width  = uint_to_qpl_output(output_bit_width);

                    reference_job_ptr->available_in  = static_cast<uint32_t>(source.size());
                    reference_job_ptr->next_in_ptr   = source.data();
                    reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());
                    reference_job_ptr->next_out_ptr  = reference_destination.data();

                    auto status = ref_compare(reference_job_ptr);

                    uint32_t result_elements_count = 0;

                    ASSERT_NO_THROW(auto scan_operation_result = test::execute(scan_operation_moved,
                                                                               source,
                                                                               destination);
                                            result_elements_count = handle_result(scan_operation_result))
                                                << test_case_info;

                    for (uint32_t i = 0; i < reference_job_ptr->total_out; i++) {
                        ASSERT_EQ(reference_destination[i], destination[i])
                                                    << test_case_info
                                                    << "Invalid byte index: "
                                                    << i
                                                    << "\n";
                    }

                } // input_bit_width cycle
            } // output_bit_width cycle
        } // is_inclusive cycle
    } // number_of_elements cycle
} // TEST

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(scan, greater_little_endian) {
    const auto comparator       = qpl::greater;
    const auto parser           = qpl::little_endian_packed_array;
    const auto low_level_parser = convert_parser(parser);
    const auto seed             = util::TestEnvironment::GetInstance().GetSeed();
    const auto execution_path   = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    qpl_get_job_size(execution_path, &job_size);

    auto reference_job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto reference_job_ptr    = reinterpret_cast<qpl_job *>(reference_job_buffer.get());

    reference_job_ptr->parser = low_level_parser;

    auto source_length = format_generator::generate_length_sequence();

    for (auto number_of_elements : source_length) {
        reference_job_ptr->num_input_elements = static_cast<uint32_t>(number_of_elements);
        for (bool is_inclusive : {false, true}) {
            reference_job_ptr->op = (is_inclusive ? qpl_op_scan_ge : qpl_op_scan_gt);

            for (uint32_t output_bit_width : {1, 8, 16, 32}) {
                if (output_bit_width != 1u && (1ull << output_bit_width) <= number_of_elements) {
                    continue;
                }

                for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
                    source_provider source_generator(number_of_elements,
                                                     input_bit_width,
                                                     seed,
                                                     low_level_parser);

                    auto source = source_generator.get_source();

                    uint32_t output_element_bit_width = output_bit_width == 1u
                                                        ? input_bit_width
                                                        : output_bit_width;
                    uint32_t destination_bit_size     = output_element_bit_width * number_of_elements;
                    uint32_t destination_size         = (destination_bit_size + 7u) >> 3u;

                    std::vector<uint8_t> destination(destination_size, 0);
                    std::vector<uint8_t> reference_destination(destination_size, 0);

                    uint32_t alphabet_power = (1u << input_bit_width);
                    uint32_t boundary       = alphabet_power >> 1u;

                    auto test_case_info = get_test_case_info(seed,
                                                             number_of_elements,
                                                             input_bit_width,
                                                             output_bit_width,
                                                             boundary,
                                                             is_inclusive);

                    auto scan_operation = typename scan_operation::builder(comparator, boundary)
                            .parser<parser>(number_of_elements)
                            .output_vector_width(output_bit_width)
                            .input_vector_width(input_bit_width)
                            .is_inclusive(is_inclusive)
                            .build();

                    uint32_t result_elements_count = 0;

                    ASSERT_NO_THROW(auto scan_operation_result = test::execute(scan_operation,
                                                                               source,
                                                                               destination);
                                            result_elements_count = handle_result(scan_operation_result))
                                                << test_case_info;

                    reference_job_ptr->param_low      = boundary;
                    reference_job_ptr->src1_bit_width = input_bit_width;
                    reference_job_ptr->out_bit_width  = uint_to_qpl_output(output_bit_width);

                    reference_job_ptr->available_in  = static_cast<uint32_t>(source.size());
                    reference_job_ptr->next_in_ptr   = source.data();
                    reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());
                    reference_job_ptr->next_out_ptr  = reference_destination.data();

                    auto status = ref_compare(reference_job_ptr);
                    ASSERT_EQ(QPL_STS_OK, status) << test_case_info;

                    for (uint32_t i = 0; i < reference_job_ptr->total_out; i++) {
                        ASSERT_EQ(reference_destination[i], destination[i])
                                                    << test_case_info
                                                    << "Invalid byte index: "
                                                    << i
                                                    << "\n";
                    }

                } // input_bit_width cycle
            } // output_bit_width cycle
        } // is_inclusive cycle
    } // number_of_elements cycle
} // TEST

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(scan, equals_little_endian) {
    const auto comparator       = qpl::equals;
    const auto parser           = qpl::little_endian_packed_array;
    const auto low_level_parser = convert_parser(parser);
    const auto seed             = util::TestEnvironment::GetInstance().GetSeed();
    const auto execution_path   = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    qpl_get_job_size(execution_path, &job_size);

    auto reference_job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto reference_job_ptr    = reinterpret_cast<qpl_job *>(reference_job_buffer.get());

    reference_job_ptr->parser = low_level_parser;

    auto source_length = format_generator::generate_length_sequence();

    for (auto number_of_elements : source_length) {
        reference_job_ptr->num_input_elements = static_cast<uint32_t>(number_of_elements);
        reference_job_ptr->op                 = qpl_op_scan_eq;

        for (uint32_t output_bit_width : {1, 8, 16, 32}) {
            if (output_bit_width != 1u && (1ull << output_bit_width) <= number_of_elements) {
                continue;
            }

            for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
                source_provider source_generator(number_of_elements,
                                                 input_bit_width,
                                                 seed,
                                                 low_level_parser);

                auto source = source_generator.get_source();

                uint32_t output_element_bit_width = output_bit_width == 1u
                                                    ? input_bit_width
                                                    : output_bit_width;
                uint32_t destination_bit_size     = output_element_bit_width * number_of_elements;
                uint32_t destination_size         = (destination_bit_size + 7u) >> 3u;

                std::vector<uint8_t> destination(destination_size, 0);
                std::vector<uint8_t> reference_destination(destination_size, 0);

                uint32_t alphabet_power = (1u << input_bit_width);
                uint32_t boundary       = alphabet_power >> 1u;

                auto test_case_info = get_test_case_info(seed,
                                                         number_of_elements,
                                                         input_bit_width,
                                                         output_bit_width,
                                                         boundary);

                auto scan_operation = typename scan_operation::builder(comparator, boundary)
                        .parser<parser>(number_of_elements)
                        .output_vector_width(output_bit_width)
                        .input_vector_width(input_bit_width)
                        .build();

                uint32_t result_elements_count = 0;

                ASSERT_NO_THROW(auto scan_operation_result = test::execute(scan_operation,
                                                                           source,
                                                                           destination);
                                        result_elements_count = handle_result(scan_operation_result))
                                            << test_case_info;

                reference_job_ptr->param_low      = boundary;
                reference_job_ptr->src1_bit_width = input_bit_width;
                reference_job_ptr->out_bit_width  = uint_to_qpl_output(output_bit_width);

                reference_job_ptr->available_in  = static_cast<uint32_t>(source.size());
                reference_job_ptr->next_in_ptr   = source.data();
                reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());
                reference_job_ptr->next_out_ptr  = reference_destination.data();

                auto status = ref_compare(reference_job_ptr);
                ASSERT_EQ(QPL_STS_OK, status) << test_case_info;

                for (uint32_t i = 0; i < reference_job_ptr->total_out; i++) {
                    ASSERT_EQ(reference_destination[i], destination[i])
                                                << test_case_info
                                                << "Invalid byte index: "
                                                << i
                                                << "\n";
                }

            } // input_bit_width cycle
        } // output_bit_width cycle
    } // number_of_elements cycle
} // TEST

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(scan, not_equals_little_endian) {
    const auto comparator       = qpl::not_equals;
    const auto parser           = qpl::little_endian_packed_array;
    const auto low_level_parser = convert_parser(parser);
    const auto seed             = util::TestEnvironment::GetInstance().GetSeed();
    const auto execution_path   = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    qpl_get_job_size(execution_path, &job_size);

    auto reference_job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto reference_job_ptr    = reinterpret_cast<qpl_job *>(reference_job_buffer.get());

    reference_job_ptr->parser = low_level_parser;

    auto source_length = format_generator::generate_length_sequence();

    for (auto number_of_elements : source_length) {
        reference_job_ptr->num_input_elements = static_cast<uint32_t>(number_of_elements);
        reference_job_ptr->op                 = qpl_op_scan_ne;

        for (uint32_t output_bit_width : {1, 8, 16, 32}) {
            if (output_bit_width != 1u && (1ull << output_bit_width) <= number_of_elements) {
                continue;
            }

            for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
                source_provider source_generator(number_of_elements,
                                                 input_bit_width,
                                                 seed,
                                                 low_level_parser);

                auto source = source_generator.get_source();

                uint32_t output_element_bit_width = output_bit_width == 1u
                                                    ? input_bit_width
                                                    : output_bit_width;
                uint32_t destination_bit_size     = output_element_bit_width * number_of_elements;
                uint32_t destination_size         = (destination_bit_size + 7u) >> 3u;

                std::vector<uint8_t> destination(destination_size, 0);
                std::vector<uint8_t> reference_destination(destination_size, 0);

                uint32_t alphabet_power = (1u << input_bit_width);
                uint32_t boundary       = alphabet_power >> 1u;

                auto test_case_info = get_test_case_info(seed,
                                                         number_of_elements,
                                                         input_bit_width,
                                                         output_bit_width,
                                                         boundary);

                auto scan_operation = typename scan_operation::builder(comparator, boundary)
                        .parser<parser>(number_of_elements)
                        .output_vector_width(output_bit_width)
                        .input_vector_width(input_bit_width)
                        .build();

                uint32_t result_elements_count = 0;

                ASSERT_NO_THROW(auto scan_operation_result = test::execute(scan_operation,
                                                                           source,
                                                                           destination);
                                        result_elements_count = handle_result(scan_operation_result))
                                            << test_case_info;

                reference_job_ptr->param_low      = boundary;
                reference_job_ptr->src1_bit_width = input_bit_width;
                reference_job_ptr->out_bit_width  = uint_to_qpl_output(output_bit_width);

                reference_job_ptr->available_in  = static_cast<uint32_t>(source.size());
                reference_job_ptr->next_in_ptr   = source.data();
                reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());
                reference_job_ptr->next_out_ptr  = reference_destination.data();

                auto status = ref_compare(reference_job_ptr);
                ASSERT_EQ(QPL_STS_OK, status) << test_case_info;

                for (uint32_t i = 0; i < reference_job_ptr->total_out; i++) {
                    ASSERT_EQ(reference_destination[i], destination[i])
                                                << test_case_info
                                                << "Invalid byte index: "
                                                << i
                                                << "\n";
                }

            } // input_bit_width cycle
        } // output_bit_width cycle
    } // number_of_elements cycle
} // TEST

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(scan, less_big_endian) {
    const auto comparator       = qpl::less;
    const auto parser           = qpl::big_endian_packed_array;
    const auto low_level_parser = convert_parser(parser);
    const auto seed             = util::TestEnvironment::GetInstance().GetSeed();
    const auto execution_path   = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    qpl_get_job_size(execution_path, &job_size);

    auto reference_job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto reference_job_ptr    = reinterpret_cast<qpl_job *>(reference_job_buffer.get());

    reference_job_ptr->parser = low_level_parser;

    auto source_length = format_generator::generate_length_sequence();

    for (auto number_of_elements : source_length) {
        reference_job_ptr->num_input_elements = static_cast<uint32_t>(number_of_elements);
        for (bool is_inclusive : {false, true}) {
            reference_job_ptr->op = (is_inclusive ? qpl_op_scan_le : qpl_op_scan_lt);

            for (uint32_t output_bit_width : {1, 8, 16, 32}) {
                if (output_bit_width != 1u && (1ull << output_bit_width) <= number_of_elements) {
                    continue;
                }

                for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
                    source_provider source_generator(number_of_elements,
                                                     input_bit_width,
                                                     seed,
                                                     low_level_parser);

                    auto source = source_generator.get_source();

                    uint32_t output_element_bit_width = output_bit_width == 1u
                                                        ? input_bit_width
                                                        : output_bit_width;
                    uint32_t destination_bit_size     = output_element_bit_width * number_of_elements;
                    uint32_t destination_size         = (destination_bit_size + 7u) >> 3u;

                    std::vector<uint8_t> destination(destination_size, 0);
                    std::vector<uint8_t> reference_destination(destination_size, 0);

                    uint32_t alphabet_power = (1u << input_bit_width);
                    uint32_t boundary       = alphabet_power >> 1u;

                    auto test_case_info = get_test_case_info(seed,
                                                             number_of_elements,
                                                             input_bit_width,
                                                             output_bit_width,
                                                             boundary,
                                                             is_inclusive);

                    auto scan_operation = typename scan_operation::builder(comparator, boundary)
                            .parser<parser>(number_of_elements)
                            .output_vector_width(output_bit_width)
                            .input_vector_width(input_bit_width)
                            .is_inclusive(is_inclusive)
                            .build();

                    uint32_t result_elements_count = 0;

                    ASSERT_NO_THROW(auto scan_operation_result = test::execute(scan_operation,
                                                                               source,
                                                                               destination);
                                            result_elements_count = handle_result(scan_operation_result))
                                                << test_case_info;

                    reference_job_ptr->param_low      = boundary;
                    reference_job_ptr->src1_bit_width = input_bit_width;
                    reference_job_ptr->out_bit_width  = uint_to_qpl_output(output_bit_width);

                    reference_job_ptr->available_in  = static_cast<uint32_t>(source.size());
                    reference_job_ptr->next_in_ptr   = source.data();
                    reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());
                    reference_job_ptr->next_out_ptr  = reference_destination.data();

                    auto status = ref_compare(reference_job_ptr);
                    ASSERT_EQ(QPL_STS_OK, status) << test_case_info;

                    for (uint32_t i = 0; i < reference_job_ptr->total_out; i++) {
                        ASSERT_EQ(reference_destination[i], destination[i])
                                                    << test_case_info
                                                    << "Invalid byte index: "
                                                    << i
                                                    << "\n";
                    }

                } // input_bit_width cycle
            } // output_bit_width cycle
        } // is_inclusive cycle
    } // number_of_elements cycle
} // TEST

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(scan, greater_big_endian) {
    const auto comparator       = qpl::greater;
    const auto parser           = qpl::big_endian_packed_array;
    const auto low_level_parser = convert_parser(parser);
    const auto seed             = util::TestEnvironment::GetInstance().GetSeed();
    const auto execution_path   = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    qpl_get_job_size(execution_path, &job_size);

    auto reference_job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto reference_job_ptr    = reinterpret_cast<qpl_job *>(reference_job_buffer.get());

    reference_job_ptr->parser = low_level_parser;

    auto source_length = format_generator::generate_length_sequence();

    for (auto number_of_elements : source_length) {
        reference_job_ptr->num_input_elements = static_cast<uint32_t>(number_of_elements);
        for (bool is_inclusive : {false, true}) {
            reference_job_ptr->op = (is_inclusive ? qpl_op_scan_ge : qpl_op_scan_gt);

            for (uint32_t output_bit_width : {1, 8, 16, 32}) {
                if (output_bit_width != 1u && (1ull << output_bit_width) <= number_of_elements) {
                    continue;
                }

                for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
                    source_provider source_generator(number_of_elements,
                                                     input_bit_width,
                                                     seed,
                                                     low_level_parser);

                    auto source = source_generator.get_source();

                    uint32_t output_element_bit_width = output_bit_width == 1u
                                                        ? input_bit_width
                                                        : output_bit_width;
                    uint32_t destination_bit_size     = output_element_bit_width * number_of_elements;
                    uint32_t destination_size         = (destination_bit_size + 7u) >> 3u;

                    std::vector<uint8_t> destination(destination_size, 0);
                    std::vector<uint8_t> reference_destination(destination_size, 0);

                    uint32_t alphabet_power = (1u << input_bit_width);
                    uint32_t boundary       = alphabet_power >> 1u;

                    auto test_case_info = get_test_case_info(seed,
                                                             number_of_elements,
                                                             input_bit_width,
                                                             output_bit_width,
                                                             boundary,
                                                             is_inclusive);

                    auto scan_operation = typename scan_operation::builder(comparator, boundary)
                            .parser<parser>(number_of_elements)
                            .output_vector_width(output_bit_width)
                            .input_vector_width(input_bit_width)
                            .is_inclusive(is_inclusive)
                            .build();

                    uint32_t result_elements_count = 0;

                    ASSERT_NO_THROW(auto scan_operation_result = test::execute(scan_operation,
                                                                               source,
                                                                               destination);
                                            result_elements_count = handle_result(scan_operation_result))
                                                << test_case_info;

                    reference_job_ptr->param_low      = boundary;
                    reference_job_ptr->src1_bit_width = input_bit_width;
                    reference_job_ptr->out_bit_width  = uint_to_qpl_output(output_bit_width);

                    reference_job_ptr->available_in  = static_cast<uint32_t>(source.size());
                    reference_job_ptr->next_in_ptr   = source.data();
                    reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());
                    reference_job_ptr->next_out_ptr  = reference_destination.data();

                    auto status = ref_compare(reference_job_ptr);
                    ASSERT_EQ(QPL_STS_OK, status) << test_case_info;

                    for (uint32_t i = 0; i < reference_job_ptr->total_out; i++) {
                        ASSERT_EQ(reference_destination[i], destination[i])
                                                    << test_case_info
                                                    << "Invalid byte index: "
                                                    << i
                                                    << "\n";
                    }

                } // input_bit_width cycle
            } // output_bit_width cycle
        } // is_inclusive cycle
    } // number_of_elements cycle
} // TEST

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(scan, equals_big_endian) {
    const auto comparator       = qpl::equals;
    const auto parser           = qpl::big_endian_packed_array;
    const auto low_level_parser = convert_parser(parser);
    const auto seed             = util::TestEnvironment::GetInstance().GetSeed();
    const auto execution_path   = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    qpl_get_job_size(execution_path, &job_size);

    auto reference_job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto reference_job_ptr    = reinterpret_cast<qpl_job *>(reference_job_buffer.get());

    reference_job_ptr->parser = low_level_parser;

    auto source_length = format_generator::generate_length_sequence();

    for (auto number_of_elements : source_length) {
        reference_job_ptr->num_input_elements = static_cast<uint32_t>(number_of_elements);
        reference_job_ptr->op                 = qpl_op_scan_eq;

        for (uint32_t output_bit_width : {1, 8, 16, 32}) {
            if (output_bit_width != 1u && (1ull << output_bit_width) <= number_of_elements) {
                continue;
            }

            for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
                source_provider source_generator(number_of_elements,
                                                 input_bit_width,
                                                 seed,
                                                 low_level_parser);

                auto source = source_generator.get_source();

                uint32_t output_element_bit_width = output_bit_width == 1u
                                                    ? input_bit_width
                                                    : output_bit_width;
                uint32_t destination_bit_size     = output_element_bit_width * number_of_elements;
                uint32_t destination_size         = (destination_bit_size + 7u) >> 3u;

                std::vector<uint8_t> destination(destination_size, 0);
                std::vector<uint8_t> reference_destination(destination_size, 0);

                uint32_t alphabet_power = (1u << input_bit_width);
                uint32_t boundary       = alphabet_power >> 1u;

                auto test_case_info = get_test_case_info(seed,
                                                         number_of_elements,
                                                         input_bit_width,
                                                         output_bit_width,
                                                         boundary);

                auto scan_operation = typename scan_operation::builder(comparator, boundary)
                        .parser<parser>(number_of_elements)
                        .output_vector_width(output_bit_width)
                        .input_vector_width(input_bit_width)
                        .build();

                uint32_t result_elements_count = 0;

                ASSERT_NO_THROW(auto scan_operation_result = test::execute(scan_operation,
                                                                           source,
                                                                           destination);
                                        result_elements_count = handle_result(scan_operation_result))
                                            << test_case_info;

                reference_job_ptr->param_low      = boundary;
                reference_job_ptr->src1_bit_width = input_bit_width;
                reference_job_ptr->out_bit_width  = uint_to_qpl_output(output_bit_width);

                reference_job_ptr->available_in  = static_cast<uint32_t>(source.size());
                reference_job_ptr->next_in_ptr   = source.data();
                reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());
                reference_job_ptr->next_out_ptr  = reference_destination.data();

                auto status = ref_compare(reference_job_ptr);
                ASSERT_EQ(QPL_STS_OK, status) << test_case_info;

                for (uint32_t i = 0; i < reference_job_ptr->total_out; i++) {
                    ASSERT_EQ(reference_destination[i], destination[i])
                                                << test_case_info
                                                << "Invalid byte index: "
                                                << i
                                                << "\n";
                }

            } // input_bit_width cycle
        } // output_bit_width cycle
    } // number_of_elements cycle
} // TEST

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(scan, not_equals_big_endian) {
    const auto comparator       = qpl::not_equals;
    const auto parser           = qpl::big_endian_packed_array;
    const auto low_level_parser = convert_parser(parser);
    const auto seed             = util::TestEnvironment::GetInstance().GetSeed();
    const auto execution_path   = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    qpl_get_job_size(execution_path, &job_size);

    auto reference_job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto reference_job_ptr    = reinterpret_cast<qpl_job *>(reference_job_buffer.get());

    reference_job_ptr->parser = low_level_parser;

    auto source_length = format_generator::generate_length_sequence();

    for (auto number_of_elements : source_length) {
        reference_job_ptr->num_input_elements = static_cast<uint32_t>(number_of_elements);
        reference_job_ptr->op                 = qpl_op_scan_ne;

        for (uint32_t output_bit_width : {1, 8, 16, 32}) {
            if (output_bit_width != 1u && (1ull << output_bit_width) <= number_of_elements) {
                continue;
            }

            for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
                source_provider source_generator(number_of_elements,
                                                 input_bit_width,
                                                 seed,
                                                 low_level_parser);

                auto source = source_generator.get_source();

                uint32_t output_element_bit_width = output_bit_width == 1u
                                                    ? input_bit_width
                                                    : output_bit_width;
                uint32_t destination_bit_size     = output_element_bit_width * number_of_elements;
                uint32_t destination_size         = (destination_bit_size + 7u) >> 3u;

                std::vector<uint8_t> destination(destination_size, 0);
                std::vector<uint8_t> reference_destination(destination_size, 0);

                uint32_t alphabet_power = (1u << input_bit_width);
                uint32_t boundary       = alphabet_power >> 1u;

                auto test_case_info = get_test_case_info(seed,
                                                         number_of_elements,
                                                         input_bit_width,
                                                         output_bit_width,
                                                         boundary);

                auto scan_operation = typename scan_operation::builder(comparator, boundary)
                        .parser<parser>(number_of_elements)
                        .output_vector_width(output_bit_width)
                        .input_vector_width(input_bit_width)
                        .build();

                uint32_t result_elements_count = 0;

                ASSERT_NO_THROW(auto scan_operation_result = test::execute(scan_operation,
                                                                           source,
                                                                           destination);
                                        result_elements_count = handle_result(scan_operation_result))
                                            << test_case_info;

                reference_job_ptr->param_low      = boundary;
                reference_job_ptr->src1_bit_width = input_bit_width;
                reference_job_ptr->out_bit_width  = uint_to_qpl_output(output_bit_width);

                reference_job_ptr->available_in  = static_cast<uint32_t>(source.size());
                reference_job_ptr->next_in_ptr   = source.data();
                reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());
                reference_job_ptr->next_out_ptr  = reference_destination.data();

                auto status = ref_compare(reference_job_ptr);
                ASSERT_EQ(QPL_STS_OK, status) << test_case_info;

                for (uint32_t i = 0; i < reference_job_ptr->total_out; i++) {
                    ASSERT_EQ(reference_destination[i], destination[i])
                                                << test_case_info
                                                << "Invalid byte index: "
                                                << i
                                                << "\n";
                }

            } // input_bit_width cycle
        } // output_bit_width cycle
    } // number_of_elements cycle
} // TEST

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(scan, less_parquet_rle) {
    const auto comparator       = qpl::less;
    const auto parser           = qpl::parquet_rle;
    const auto low_level_parser = convert_parser(parser);
    const auto seed             = util::TestEnvironment::GetInstance().GetSeed();
    const auto execution_path   = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    qpl_get_job_size(execution_path, &job_size);

    auto reference_job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto reference_job_ptr    = reinterpret_cast<qpl_job *>(reference_job_buffer.get());

    reference_job_ptr->parser = low_level_parser;

    auto source_length = format_generator::generate_length_sequence();

    for (auto number_of_elements : source_length) {
        reference_job_ptr->num_input_elements = static_cast<uint32_t>(number_of_elements);
        for (bool is_inclusive : {false, true}) {
            reference_job_ptr->op = (is_inclusive ? qpl_op_scan_le : qpl_op_scan_lt);

            for (uint32_t output_bit_width : {1, 8, 16, 32}) {
                if (output_bit_width != 1u && (1ull << output_bit_width) <= number_of_elements) {
                    continue;
                }

                for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
                    source_provider source_generator(number_of_elements,
                                                     input_bit_width,
                                                     seed,
                                                     low_level_parser);

                    auto source = source_generator.get_source();

                    uint32_t output_element_bit_width = output_bit_width == 1u
                                                        ? input_bit_width
                                                        : output_bit_width;
                    uint32_t destination_bit_size     =
                                     output_element_bit_width * number_of_elements;
                    uint32_t destination_size         = (destination_bit_size + 7u) >> 3u;

                    std::vector<uint8_t> destination(destination_size, 0);
                    std::vector<uint8_t> reference_destination(destination_size, 0);

                    uint32_t alphabet_power = (1u << input_bit_width);
                    uint32_t boundary       = alphabet_power >> 1u;

                    auto test_case_info = get_test_case_info(seed,
                                                             number_of_elements,
                                                             input_bit_width,
                                                             output_bit_width,
                                                             boundary,
                                                             is_inclusive);

                    auto scan_operation = typename scan_operation::builder(comparator, boundary)
                            .parser<parser>(number_of_elements)
                            .output_vector_width(output_bit_width)
                            .input_vector_width(input_bit_width)
                            .is_inclusive(is_inclusive)
                            .build();

                    uint32_t result_elements_count = 0;

                    ASSERT_NO_THROW(auto scan_operation_result = test::execute(scan_operation,
                                                                               source,
                                                                               destination);
                                            result_elements_count = handle_result(scan_operation_result))
                                                << test_case_info;

                    reference_job_ptr->param_low      = boundary;
                    reference_job_ptr->src1_bit_width = input_bit_width;
                    reference_job_ptr->out_bit_width  = uint_to_qpl_output(output_bit_width);

                    reference_job_ptr->available_in  = static_cast<uint32_t>(source.size());
                    reference_job_ptr->next_in_ptr   = source.data();
                    reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());
                    reference_job_ptr->next_out_ptr  = reference_destination.data();

                    auto status = ref_compare(reference_job_ptr);
                    ASSERT_EQ(QPL_STS_OK, status) << test_case_info;

                    for (uint32_t i = 0; i < reference_job_ptr->total_out; i++) {
                        ASSERT_EQ(reference_destination[i], destination[i])
                                                    << test_case_info
                                                    << "Invalid byte index: "
                                                    << i
                                                    << "\n";
                    }

                } // input_bit_width cycle
            } // output_bit_width cycle
        } // is_inclusive cycle
    } // number_of_elements cycle
} // TEST

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(scan, greater_parquet_rle) {
    const auto comparator       = qpl::greater;
    const auto parser           = qpl::parquet_rle;
    const auto low_level_parser = convert_parser(parser);
    const auto seed             = util::TestEnvironment::GetInstance().GetSeed();
    const auto execution_path   = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    qpl_get_job_size(execution_path, &job_size);

    auto reference_job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto reference_job_ptr    = reinterpret_cast<qpl_job *>(reference_job_buffer.get());

    reference_job_ptr->parser = low_level_parser;

    auto source_length = format_generator::generate_length_sequence();

    for (auto number_of_elements : source_length) {
        reference_job_ptr->num_input_elements = static_cast<uint32_t>(number_of_elements);
        for (bool is_inclusive : {false, true}) {
            reference_job_ptr->op = (is_inclusive ? qpl_op_scan_ge : qpl_op_scan_gt);

            for (uint32_t output_bit_width : {1, 8, 16, 32}) {
                if (output_bit_width != 1u && (1ull << output_bit_width) <= number_of_elements) {
                    continue;
                }

                for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
                    source_provider source_generator(number_of_elements,
                                                     input_bit_width,
                                                     seed,
                                                     low_level_parser);

                    auto source = source_generator.get_source();

                    uint32_t output_element_bit_width = output_bit_width == 1u
                                                        ? input_bit_width
                                                        : output_bit_width;
                    uint32_t destination_bit_size     = output_element_bit_width * number_of_elements;
                    uint32_t destination_size         = (destination_bit_size + 7u) >> 3u;

                    std::vector<uint8_t> destination(destination_size, 0);
                    std::vector<uint8_t> reference_destination(destination_size, 0);

                    uint32_t alphabet_power = (1u << input_bit_width);
                    uint32_t boundary       = alphabet_power >> 1u;

                    auto test_case_info = get_test_case_info(seed,
                                                             number_of_elements,
                                                             input_bit_width,
                                                             output_bit_width,
                                                             boundary,
                                                             is_inclusive);

                    auto scan_operation = typename scan_operation::builder(comparator, boundary)
                            .parser<parser>(number_of_elements)
                            .output_vector_width(output_bit_width)
                            .input_vector_width(input_bit_width)
                            .is_inclusive(is_inclusive)
                            .build();

                    uint32_t result_elements_count = 0;

                    ASSERT_NO_THROW(auto scan_operation_result = test::execute(scan_operation,
                                                                               source,
                                                                               destination);
                                            result_elements_count = handle_result(scan_operation_result))
                                                << test_case_info;

                    reference_job_ptr->param_low      = boundary;
                    reference_job_ptr->src1_bit_width = input_bit_width;
                    reference_job_ptr->out_bit_width  = uint_to_qpl_output(output_bit_width);

                    reference_job_ptr->available_in  = static_cast<uint32_t>(source.size());
                    reference_job_ptr->next_in_ptr   = source.data();
                    reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());
                    reference_job_ptr->next_out_ptr  = reference_destination.data();

                    auto status = ref_compare(reference_job_ptr);
                    ASSERT_EQ(QPL_STS_OK, status) << test_case_info;

                    for (uint32_t i = 0; i < reference_job_ptr->total_out; i++) {
                        ASSERT_EQ(reference_destination[i], destination[i])
                                                    << test_case_info
                                                    << "Invalid byte index: "
                                                    << i
                                                    << "\n";
                    }

                } // input_bit_width cycle
            } // output_bit_width cycle
        } // is_inclusive cycle
    } // number_of_elements cycle
} // TEST

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(scan, equals_parquet_rle) {
    const auto comparator       = qpl::equals;
    const auto parser           = qpl::parquet_rle;
    const auto low_level_parser = convert_parser(parser);
    const auto seed             = util::TestEnvironment::GetInstance().GetSeed();
    const auto execution_path   = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    qpl_get_job_size(execution_path, &job_size);

    auto reference_job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto reference_job_ptr    = reinterpret_cast<qpl_job *>(reference_job_buffer.get());

    reference_job_ptr->parser = low_level_parser;

    auto source_length = format_generator::generate_length_sequence();

    for (auto number_of_elements : source_length) {
        reference_job_ptr->num_input_elements = static_cast<uint32_t>(number_of_elements);
        reference_job_ptr->op                 = qpl_op_scan_eq;

        for (uint32_t output_bit_width : {1, 8, 16, 32}) {
            if (output_bit_width != 1u && (1ull << output_bit_width) <= number_of_elements) {
                continue;
            }

            for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
                source_provider source_generator(number_of_elements,
                                                 input_bit_width,
                                                 seed,
                                                 low_level_parser);

                auto source = source_generator.get_source();

                uint32_t output_element_bit_width = output_bit_width == 1u
                                                    ? input_bit_width
                                                    : output_bit_width;
                uint32_t destination_bit_size     = output_element_bit_width * number_of_elements;
                uint32_t destination_size         = (destination_bit_size + 7u) >> 3u;

                std::vector<uint8_t> destination(destination_size, 0);
                std::vector<uint8_t> reference_destination(destination_size, 0);

                uint32_t alphabet_power = (1u << input_bit_width);
                uint32_t boundary       = alphabet_power >> 1u;

                auto test_case_info = get_test_case_info(seed,
                                                         number_of_elements,
                                                         input_bit_width,
                                                         output_bit_width,
                                                         boundary);

                auto scan_operation = typename scan_operation::builder(comparator, boundary)
                        .parser<parser>(number_of_elements)
                        .output_vector_width(output_bit_width)
                        .input_vector_width(input_bit_width)
                        .build();

                uint32_t result_elements_count = 0;

                ASSERT_NO_THROW(auto scan_operation_result = test::execute(scan_operation,
                                                                           source,
                                                                           destination);
                                        result_elements_count = handle_result(scan_operation_result))
                                            << test_case_info;

                reference_job_ptr->param_low      = boundary;
                reference_job_ptr->src1_bit_width = input_bit_width;
                reference_job_ptr->out_bit_width  = uint_to_qpl_output(output_bit_width);

                reference_job_ptr->available_in  = static_cast<uint32_t>(source.size());
                reference_job_ptr->next_in_ptr   = source.data();
                reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());
                reference_job_ptr->next_out_ptr  = reference_destination.data();

                auto status = ref_compare(reference_job_ptr);
                ASSERT_EQ(QPL_STS_OK, status) << test_case_info;

                for (uint32_t i = 0; i < reference_job_ptr->total_out; i++) {
                    ASSERT_EQ(reference_destination[i], destination[i])
                                                << test_case_info
                                                << "Invalid byte index: "
                                                << i
                                                << "\n";
                }

            } // input_bit_width cycle
        } // output_bit_width cycle
    } // number_of_elements cycle
} // TEST

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(scan, not_equals_parquet_rle) {
    const auto comparator       = qpl::not_equals;
    const auto parser           = qpl::parquet_rle;
    const auto low_level_parser = convert_parser(parser);
    const auto seed             = util::TestEnvironment::GetInstance().GetSeed();
    const auto execution_path   = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    qpl_get_job_size(execution_path, &job_size);

    auto reference_job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto reference_job_ptr    = reinterpret_cast<qpl_job *>(reference_job_buffer.get());

    reference_job_ptr->parser = low_level_parser;

    auto source_length = format_generator::generate_length_sequence();

    for (auto number_of_elements : source_length) {
        reference_job_ptr->num_input_elements = static_cast<uint32_t>(number_of_elements);
        reference_job_ptr->op                 = qpl_op_scan_ne;

        for (uint32_t output_bit_width : {1, 8, 16, 32}) {
            if (output_bit_width != 1u && (1ull << output_bit_width) <= number_of_elements) {
                continue;
            }

            for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
                source_provider source_generator(number_of_elements,
                                                 input_bit_width,
                                                 seed,
                                                 low_level_parser);

                auto source = source_generator.get_source();

                uint32_t output_element_bit_width = output_bit_width == 1u
                                                    ? input_bit_width
                                                    : output_bit_width;
                uint32_t destination_bit_size     = output_element_bit_width * number_of_elements;
                uint32_t destination_size         = (destination_bit_size + 7u) >> 3u;

                std::vector<uint8_t> destination(destination_size, 0);
                std::vector<uint8_t> reference_destination(destination_size, 0);

                uint32_t alphabet_power = (1u << input_bit_width);
                uint32_t boundary       = alphabet_power >> 1u;

                auto test_case_info = get_test_case_info(seed,
                                                         number_of_elements,
                                                         input_bit_width,
                                                         output_bit_width,
                                                         boundary);

                auto scan_operation = typename scan_operation::builder(comparator, boundary)
                        .parser<parser>(number_of_elements)
                        .output_vector_width(output_bit_width)
                        .input_vector_width(input_bit_width)
                        .build();

                uint32_t result_elements_count = 0;

                ASSERT_NO_THROW(auto scan_operation_result = test::execute(scan_operation,
                                                                           source,
                                                                           destination);
                                        result_elements_count = handle_result(scan_operation_result))
                                            << test_case_info;

                reference_job_ptr->param_low      = boundary;
                reference_job_ptr->src1_bit_width = input_bit_width;
                reference_job_ptr->out_bit_width  = uint_to_qpl_output(output_bit_width);

                reference_job_ptr->available_in  = static_cast<uint32_t>(source.size());
                reference_job_ptr->next_in_ptr   = source.data();
                reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());
                reference_job_ptr->next_out_ptr  = reference_destination.data();

                auto status = ref_compare(reference_job_ptr);
                ASSERT_EQ(QPL_STS_OK, status) << test_case_info;

                for (uint32_t i = 0; i < reference_job_ptr->total_out; i++) {
                    ASSERT_EQ(reference_destination[i], destination[i])
                                                << test_case_info
                                                << "Invalid byte index: "
                                                << i
                                                << "\n";
                }

            } // input_bit_width cycle
        } // output_bit_width cycle
    } // number_of_elements cycle
} // TEST

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(scan, initial_output_index) {
    const auto comparator = qpl::less;
    const auto parser = qpl::little_endian_packed_array;
    const auto low_level_parser = convert_parser(parser);
    const auto seed = util::TestEnvironment::GetInstance().GetSeed();
    const auto execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    qpl_get_job_size(execution_path, &job_size);

    auto reference_job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto reference_job_ptr = reinterpret_cast<qpl_job *>(reference_job_buffer.get());

    reference_job_ptr->parser = low_level_parser;

    auto source_length = format_generator::generate_length_sequence();
    auto max_output_bytes = *(--source_length.end()) * 4;

    for (auto number_of_elements : source_length) {
        reference_job_ptr->num_input_elements = static_cast<uint32_t>(number_of_elements);
        for (bool is_inclusive : {false, true}) {
            reference_job_ptr->op = (is_inclusive ? qpl_op_scan_le : qpl_op_scan_lt);

            for (uint32_t output_bit_width : {8, 16, 32}) {
                if (output_bit_width != 1u && (1ull << output_bit_width) <= number_of_elements) {
                    continue;
                }

                for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
                    source_provider source_generator(number_of_elements,
                        input_bit_width,
                        seed,
                        low_level_parser);

                    auto source = source_generator.get_source();

                    uint32_t output_element_bit_width = output_bit_width == 1u
                        ? input_bit_width
                        : output_bit_width;
                    uint32_t destination_bit_size = output_element_bit_width * number_of_elements;
                    uint32_t destination_size = (destination_bit_size + 7u) >> 3u;

                    std::vector<uint8_t> destination(destination_size, 0);
                    std::vector<uint8_t> reference_destination(destination_size, 0);

                    uint32_t alphabet_power = (1u << input_bit_width);
                    uint32_t boundary = alphabet_power >> 1u;

                    auto test_case_info = get_test_case_info(seed,
                        number_of_elements,
                        input_bit_width,
                        output_bit_width,
                        boundary,
                        is_inclusive);

                    uint32_t max_possible_index = (uint32_t)((1llu << output_bit_width) - 1u);

                    auto scan_operation = typename scan_operation::builder(comparator, boundary)
                        .parser<parser>(number_of_elements)
                        .output_vector_width(output_bit_width)
                        .input_vector_width(input_bit_width)
                        .is_inclusive(is_inclusive)
                        .initial_output_index(max_possible_index - number_of_elements)
                        .build();

                    reference_job_ptr->param_low = boundary;
                    reference_job_ptr->src1_bit_width = input_bit_width;
                    reference_job_ptr->out_bit_width = uint_to_qpl_output(output_bit_width);

                    reference_job_ptr->available_in = static_cast<uint32_t>(source.size());
                    reference_job_ptr->next_in_ptr = source.data();
                    reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());
                    reference_job_ptr->next_out_ptr = reference_destination.data();
                    reference_job_ptr->initial_output_index = max_possible_index - number_of_elements;

                    auto status = ref_compare(reference_job_ptr);

                    uint32_t result_elements_count = 0;

                    ASSERT_NO_THROW(auto scan_operation_result = test::execute(scan_operation,
                        source,
                        destination);
                    result_elements_count = handle_result(scan_operation_result))
                        << test_case_info;

                    for (uint32_t i = 0; i < reference_job_ptr->total_out; i++) {
                        ASSERT_EQ(reference_destination[i], destination[i])
                            << test_case_info
                            << "Invalid byte index: "
                            << i
                            << "\n";
                    }

                } // input_bit_width cycle
            } // output_bit_width cycle
        } // is_inclusive cycle
    } // number_of_elements cycle
} // TEST

} // namespace qpl::test
