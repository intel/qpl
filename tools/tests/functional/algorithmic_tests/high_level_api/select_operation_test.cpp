/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <iostream>
#include <vector>
#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "qpl/cpp_api/operations/analytics/select_operation.hpp"

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
                                      const uint32_t output_bit_width) -> std::string {
    std::string result = std::string("\nTest parameters:\n") + "Seed: " + std::to_string(seed) +
                         "\n" + "Number of elements: " + std::to_string(number_of_elements) +
                         "\n" + "Input bit width: " + std::to_string(input_bit_width) +
                         "\n" + "Output bit width: " + std::to_string(output_bit_width) +
                         "\n";

    return result;
}

static inline auto output_is_indexes(uint32_t input_bit_width, uint32_t output_bit_width) -> bool {
    return (input_bit_width == 1u && output_bit_width != 1u);
}

static inline auto is_valid_test_case(uint32_t input_bit_width,
                                      uint32_t output_bit_width,
                                      uint32_t number_of_elements) -> bool {
    if (output_is_indexes(input_bit_width, output_bit_width)) {
        if ((1u << output_bit_width) <= number_of_elements) {
            return false;
        }
    } else {
        if (output_bit_width != 1u && input_bit_width > output_bit_width) {
            return false;
        }
    }
    return true;
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(select, little_endian) {
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
        reference_job_ptr->op                 = qpl_op_select;

        for (uint32_t output_bit_width : {1, 8, 16, 32}) {
            for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
                if (!is_valid_test_case(input_bit_width, output_bit_width, number_of_elements)) {
                    break;
                }

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

                source_provider mask_generator(number_of_elements,
                                               bit_bit_width,
                                               seed,
                                               qpl_p_le_packed_array);

                auto mask = mask_generator.get_source();

                auto test_case_info = get_test_case_info(seed,
                                                         number_of_elements,
                                                         input_bit_width,
                                                         output_bit_width);

                auto select_operation = typename select_operation::builder(mask.data(), mask.size())
                        .parser<parser>(number_of_elements)
                        .output_vector_width(output_bit_width)
                        .input_vector_width(input_bit_width)
                        .build();

                uint32_t result_elements_count = 0;

                ASSERT_NO_THROW(auto select_operation_result = test::execute(select_operation,
                                                                             source,
                                                                             destination);
                                        result_elements_count = handle_result(select_operation_result))
                                            << test_case_info;

                reference_job_ptr->src1_bit_width = input_bit_width;
                reference_job_ptr->src2_bit_width = bit_bit_width;
                reference_job_ptr->out_bit_width  = uint_to_qpl_output(output_bit_width);

                reference_job_ptr->available_in   = static_cast<uint32_t>(source.size());
                reference_job_ptr->next_in_ptr    = source.data();
                reference_job_ptr->available_out  = static_cast<uint32_t>(reference_destination.size());
                reference_job_ptr->next_out_ptr   = reference_destination.data();
                reference_job_ptr->available_src2 = static_cast<uint32_t>(mask.size());
                reference_job_ptr->next_src2_ptr  = mask.data();

                auto status = ref_select(reference_job_ptr);
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

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(select, big_endian) {
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
        reference_job_ptr->op                 = qpl_op_select;

        for (uint32_t output_bit_width : {1, 8, 16, 32}) {
            for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
                if (!is_valid_test_case(input_bit_width, output_bit_width, number_of_elements)) {
                    break;
                }

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

                source_provider mask_generator(number_of_elements,
                                               bit_bit_width,
                                               seed,
                                               qpl_p_le_packed_array);

                auto mask = mask_generator.get_source();

                auto test_case_info = get_test_case_info(seed,
                                                         number_of_elements,
                                                         input_bit_width,
                                                         output_bit_width);

                auto select_operation = typename select_operation::builder(mask.data(), mask.size())
                        .parser<parser>(number_of_elements)
                        .output_vector_width(output_bit_width)
                        .input_vector_width(input_bit_width)
                        .build();

                uint32_t result_elements_count = 0;

                ASSERT_NO_THROW(auto select_operation_result = test::execute(select_operation,
                                                                             source,
                                                                             destination);
                                        result_elements_count = handle_result(select_operation_result))
                                            << test_case_info;

                reference_job_ptr->src1_bit_width = input_bit_width;
                reference_job_ptr->src2_bit_width = bit_bit_width;
                reference_job_ptr->out_bit_width  = uint_to_qpl_output(output_bit_width);

                reference_job_ptr->available_in   = static_cast<uint32_t>(source.size());
                reference_job_ptr->next_in_ptr    = source.data();
                reference_job_ptr->available_out  = static_cast<uint32_t>(reference_destination.size());
                reference_job_ptr->next_out_ptr   = reference_destination.data();
                reference_job_ptr->available_src2 = static_cast<uint32_t>(mask.size());
                reference_job_ptr->next_src2_ptr  = mask.data();

                auto status = ref_select(reference_job_ptr);
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

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(select, parquet_rle) {
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
        reference_job_ptr->op                 = qpl_op_select;

        for (uint32_t output_bit_width : {1, 8, 16, 32}) {
            for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
                if (!is_valid_test_case(input_bit_width, output_bit_width, number_of_elements)) {
                    break;
                }

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

                source_provider mask_generator(number_of_elements,
                                               bit_bit_width,
                                               seed,
                                               qpl_p_le_packed_array);

                auto mask = mask_generator.get_source();

                auto test_case_info = get_test_case_info(seed,
                                                         number_of_elements,
                                                         input_bit_width,
                                                         output_bit_width);

                auto select_operation = typename select_operation::builder(mask.data(), mask.size())
                        .parser<parser>(number_of_elements)
                        .output_vector_width(output_bit_width)
                        .input_vector_width(input_bit_width)
                        .build();

                uint32_t result_elements_count = 0;

                ASSERT_NO_THROW(auto select_operation_result = test::execute(select_operation,
                                                                             source,
                                                                             destination);
                                        result_elements_count = handle_result(select_operation_result))
                                            << test_case_info;

                reference_job_ptr->src1_bit_width = input_bit_width;
                reference_job_ptr->src2_bit_width = bit_bit_width;
                reference_job_ptr->out_bit_width  = uint_to_qpl_output(output_bit_width);

                reference_job_ptr->available_in   = static_cast<uint32_t>(source.size());
                reference_job_ptr->next_in_ptr    = source.data();
                reference_job_ptr->available_out  = static_cast<uint32_t>(reference_destination.size());
                reference_job_ptr->next_out_ptr   = reference_destination.data();
                reference_job_ptr->available_src2 = static_cast<uint32_t>(mask.size());
                reference_job_ptr->next_src2_ptr  = mask.data();

                auto status = ref_select(reference_job_ptr);
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

} // namespace qpl::test
