/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <vector>
#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "format_generator.hpp"
#include "qpl/cpp_api/operations/analytics/find_unique_operation.hpp"

#include "ta_hl_common.hpp"
#include "high_level_api_util.hpp"
#include "source_provider.hpp"
#include "qpl/qpl.h"
#include "qpl_api_ref.h"

namespace qpl::test {
static inline auto get_test_case_info(const uint32_t seed,
                                      const uint32_t number_of_elements,
                                      const uint32_t input_bit_width,
                                      const uint32_t output_bit_width,
                                      const uint32_t low_bits_to_ignore,
                                      const uint32_t high_bits_to_ignore) -> std::string {
    std::string result = std::string("\nTest parameters:\n") + "Seed: " + std::to_string(seed) +
                         "\n" + "Number of elements: " + std::to_string(number_of_elements) +
                         "\n" + "Input bit width: " + std::to_string(input_bit_width) +
                         "\n" + "Output bit width: " + std::to_string(output_bit_width) +
                         "\n" + "Low order bits to ignore: " + std::to_string(low_bits_to_ignore) +
                         "\n" + "High order bits to ignore: " + std::to_string(high_bits_to_ignore) +
                         "\n";

    return result;
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(find_unique, little_endian) {
    const auto parser           = qpl::little_endian_packed_array;
    const auto low_level_parser = convert_parser(parser);
    const auto seed             = util::TestEnvironment::GetInstance().GetSeed();
    const auto execution_path   = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    qpl_get_job_size(execution_path, &job_size);

    auto reference_job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto reference_job_ptr    = reinterpret_cast<qpl_job *>(reference_job_buffer.get());

    reference_job_ptr->parser = low_level_parser;
    reference_job_ptr->op     = qpl_op_find_unique;

    auto source_length = format_generator::generate_length_sequence();

    for (auto number_of_elements : source_length) {
        reference_job_ptr->num_input_elements = static_cast<uint32_t>(number_of_elements);

        for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
            source_provider source_generator(number_of_elements,
                                             input_bit_width,
                                             seed,
                                             low_level_parser);

            auto source = source_generator.get_source();

            uint32_t low_bits_to_ignore  = input_bit_width / 3;
            uint32_t high_bits_to_ignore = input_bit_width / 3;

            uint32_t destination_bit_size = 1u << (input_bit_width - low_bits_to_ignore - high_bits_to_ignore);
            uint32_t destination_size     = (destination_bit_size + 7u) >> 3u;

            std::vector<uint8_t> destination(destination_size, 0);
            std::vector<uint8_t> reference_destination(destination_size, 0);

            auto test_case_info = get_test_case_info(seed,
                                                     number_of_elements,
                                                     input_bit_width,
                                                     bit_bit_width,
                                                     low_bits_to_ignore,
                                                     high_bits_to_ignore);

            auto find_unique_operation = typename find_unique_operation::builder(0u, 0u)
                    .parser<parser>(number_of_elements)
                    .input_vector_width(input_bit_width)
                    .build();

            auto find_unique_operation_moved = typename find_unique_operation::builder(find_unique_operation)
                .number_low_order_bits_to_ignore(low_bits_to_ignore)
                .number_high_order_bits_to_ignore(high_bits_to_ignore)
                .build();

            uint32_t result_elements_count = 0;

            ASSERT_NO_THROW(auto find_unique_operation_result = test::execute(find_unique_operation_moved,
                                                                              source,
                                                                              destination);
                                    result_elements_count = handle_result(find_unique_operation_result))
                                        << test_case_info;

            reference_job_ptr->param_low      = low_bits_to_ignore;
            reference_job_ptr->param_high     = high_bits_to_ignore;
            reference_job_ptr->src1_bit_width = input_bit_width;

            reference_job_ptr->available_in  = static_cast<uint32_t>(source.size());
            reference_job_ptr->next_in_ptr   = source.data();
            reference_job_ptr->available_out = static_cast<uint32_t>(destination_size);
            reference_job_ptr->next_out_ptr  = reference_destination.data();

            auto status = ref_find_unique(reference_job_ptr);
            ASSERT_EQ(QPL_STS_OK, status) << test_case_info;

            for (uint32_t i = 0; i < reference_job_ptr->total_out; i++) {
                ASSERT_EQ(reference_destination[i], destination[i])
                                            << test_case_info
                                            << "Invalid byte index: "
                                            << i
                                            << "\n";
            }
        } // input_bit_width cycle
    } // number_of_elements cycle
} // TEST

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(hl_find_unique_operation_test, big_endian) {
    const auto parser           = qpl::big_endian_packed_array;
    const auto low_level_parser = convert_parser(parser);
    const auto seed             = util::TestEnvironment::GetInstance().GetSeed();
    const auto execution_path   = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    qpl_get_job_size(execution_path, &job_size);

    auto reference_job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto reference_job_ptr    = reinterpret_cast<qpl_job *>(reference_job_buffer.get());

    reference_job_ptr->parser = low_level_parser;
    reference_job_ptr->op     = qpl_op_find_unique;

    auto source_length = format_generator::generate_length_sequence();

    for (auto number_of_elements : source_length) {
        reference_job_ptr->num_input_elements = static_cast<uint32_t>(number_of_elements);

        for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
            source_provider source_generator(number_of_elements,
                                             input_bit_width,
                                             seed,
                                             low_level_parser);

            auto source = source_generator.get_source();

            uint32_t low_bits_to_ignore  = input_bit_width / 3;
            uint32_t high_bits_to_ignore = input_bit_width / 3;

            uint32_t destination_bit_size = 1u << (input_bit_width - low_bits_to_ignore - high_bits_to_ignore);
            uint32_t destination_size     = (destination_bit_size + 7u) >> 3u;

            std::vector<uint8_t> destination(destination_size, 0);
            std::vector<uint8_t> reference_destination(destination_size, 0);

            auto test_case_info = get_test_case_info(seed,
                                                     number_of_elements,
                                                     input_bit_width,
                                                     1,
                                                     low_bits_to_ignore,
                                                     high_bits_to_ignore);

            auto find_unique_operation = typename find_unique_operation::builder(low_bits_to_ignore,
                                                                                 high_bits_to_ignore)
                    .parser<parser>(number_of_elements)
                    .input_vector_width(input_bit_width)
                    .build();

            uint32_t result_elements_count = 0;

            ASSERT_NO_THROW(auto find_unique_operation_result = test::execute(find_unique_operation,
                                                                              source,
                                                                              destination);
                                    result_elements_count = handle_result(find_unique_operation_result))
                                        << test_case_info;

            reference_job_ptr->param_low      = low_bits_to_ignore;
            reference_job_ptr->param_high     = high_bits_to_ignore;
            reference_job_ptr->src1_bit_width = input_bit_width;

            reference_job_ptr->available_in  = static_cast<uint32_t>(source.size());
            reference_job_ptr->next_in_ptr   = source.data();
            reference_job_ptr->available_out = static_cast<uint32_t>(destination_size);
            reference_job_ptr->next_out_ptr  = reference_destination.data();

            auto status = ref_find_unique(reference_job_ptr);
            ASSERT_EQ(QPL_STS_OK, status) << test_case_info;

            for (uint32_t i = 0; i < reference_job_ptr->total_out; i++) {
                ASSERT_EQ(reference_destination[i], destination[i])
                                            << test_case_info
                                            << "Invalid byte index: "
                                            << i
                                            << "\n";
            }
        } // input_bit_width cycle
    } // number_of_elements cycle
} // TEST

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(find_unique, parquet_rle) {
    const auto parser           = qpl::parquet_rle;
    const auto low_level_parser = convert_parser(parser);
    const auto seed             = util::TestEnvironment::GetInstance().GetSeed();
    const auto execution_path   = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    qpl_get_job_size(execution_path, &job_size);

    auto reference_job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto reference_job_ptr    = reinterpret_cast<qpl_job *>(reference_job_buffer.get());

    reference_job_ptr->parser = low_level_parser;
    reference_job_ptr->op     = qpl_op_find_unique;

    auto source_length = format_generator::generate_length_sequence();

    for (auto number_of_elements : source_length) {
        reference_job_ptr->num_input_elements = static_cast<uint32_t>(number_of_elements);

        for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
            source_provider source_generator(number_of_elements,
                                             input_bit_width,
                                             seed,
                                             low_level_parser);

            auto source = source_generator.get_source();

            uint32_t low_bits_to_ignore  = input_bit_width / 3;
            uint32_t high_bits_to_ignore = input_bit_width / 3;

            uint32_t destination_bit_size = 1u << (input_bit_width - low_bits_to_ignore - high_bits_to_ignore);
            uint32_t destination_size     = (destination_bit_size + 7u) >> 3u;

            std::vector<uint8_t> destination(destination_size, 0);
            std::vector<uint8_t> reference_destination(destination_size, 0);

            auto test_case_info = get_test_case_info(seed,
                                                     number_of_elements,
                                                     input_bit_width,
                                                     bit_bit_width,
                                                     low_bits_to_ignore,
                                                     high_bits_to_ignore);

            auto find_unique_operation = typename find_unique_operation::builder(low_bits_to_ignore,
                                                                                 high_bits_to_ignore)
                    .parser<parser>(number_of_elements)
                    .input_vector_width(input_bit_width)
                    .build();

            uint32_t result_elements_count = 0;

            ASSERT_NO_THROW(auto find_unique_operation_result = test::execute(find_unique_operation,
                                                                              source,
                                                                              destination);
                                    result_elements_count = handle_result(find_unique_operation_result))
                                        << test_case_info;

            reference_job_ptr->param_low      = low_bits_to_ignore;
            reference_job_ptr->param_high     = high_bits_to_ignore;
            reference_job_ptr->src1_bit_width = input_bit_width;

            reference_job_ptr->available_in  = static_cast<uint32_t>(source.size());
            reference_job_ptr->next_in_ptr   = source.data();
            reference_job_ptr->available_out = static_cast<uint32_t>(destination_size);
            reference_job_ptr->next_out_ptr  = reference_destination.data();

            auto status = ref_find_unique(reference_job_ptr);
            ASSERT_EQ(QPL_STS_OK, status) << test_case_info;

            for (uint32_t i = 0; i < reference_job_ptr->total_out; i++) {
                ASSERT_EQ(reference_destination[i], destination[i])
                                            << test_case_info
                                            << "Invalid byte index: "
                                            << i
                                            << "\n";
            }
        } // input_bit_width cycle
    } // number_of_elements cycle
} // TEST

} // namespace qpl::test
