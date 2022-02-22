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
#include "qpl/cpp_api/operations/analytics/rle_burst_operation.hpp"

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
                                      const uint32_t counter_bit_width,
                                      const uint32_t output_bit_width) -> std::string {
    std::string result = std::string("\nTest parameters:\n") + "Seed: " + std::to_string(seed) +
                         "\n" + "Number of elements: " + std::to_string(number_of_elements) +
                         "\n" + "Input bit width: " + std::to_string(input_bit_width) +
                         "\n" + "Counter bit width: " + std::to_string(counter_bit_width) +
                         "\n" + "Output bit width: " + std::to_string(output_bit_width) +
                         "\n";

    return result;
}

template <qpl::parsers parser>
static inline void rle_burst_test_impl() {
    const auto low_level_parser = convert_parser(parser);
    const auto seed             = util::TestEnvironment::GetInstance().GetSeed();
    const auto execution_path   = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    qpl_get_job_size(execution_path, &job_size);

    auto reference_job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto reference_job_ptr    = reinterpret_cast<qpl_job *>(reference_job_buffer.get());

    reference_job_ptr->parser = qpl_p_le_packed_array;

    auto source_length = format_generator::generate_length_sequence();

    for (auto number_of_elements : source_length) {
        reference_job_ptr->num_input_elements = static_cast<uint32_t>(number_of_elements);
        reference_job_ptr->op                 = qpl_op_rle_burst;

        for (uint32_t counter_bit_width  : {8, 16, 32}) {
            for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
                if (32u == counter_bit_width && 1u == number_of_elements) {
                    continue;
                }

                source_provider counters_generator(number_of_elements,
                                                   counter_bit_width,
                                                   seed,
                                                   qpl_p_le_packed_array);

                auto     counters          = counters_generator.get_counter_source_expand_rle();
                uint32_t destination_count = counters_generator.get_count_expand_rle_value();
                uint32_t destination_size  = destination_count * ((input_bit_width + 7u) >> 3u);

                uint32_t symbols_count = counter_bit_width == 32 ? (number_of_elements - 1) : number_of_elements;

                std::vector<uint8_t> destination(destination_size, 0);
                std::vector<uint8_t> reference_destination(destination_size, 0);

                source_provider symbols_generator(symbols_count,
                                                  input_bit_width,
                                                  seed,
                                                  low_level_parser);

                auto symbols = symbols_generator.get_source();

                auto test_case_info = get_test_case_info(seed,
                                                         number_of_elements,
                                                         input_bit_width,
                                                         counter_bit_width,
                                                         bit_bit_width);

                auto rle_burst_operation = typename rle_burst_operation::builder(counters.data(), counters.size())
                        .parser<parser>(number_of_elements)
                        .counter_bit_width(counter_bit_width)
                        .output_vector_width(bit_bit_width)
                        .input_vector_width(input_bit_width)
                        .build();

                uint32_t result_elements_count = 0;

                ASSERT_NO_THROW(auto operation_result = test::execute(rle_burst_operation,
                                                                      symbols,
                                                                      destination);
                                        result_elements_count = handle_result(operation_result))
                                            << test_case_info;

                reference_job_ptr->src1_bit_width = counter_bit_width;
                reference_job_ptr->src2_bit_width = input_bit_width;
                reference_job_ptr->out_bit_width  = uint_to_qpl_output(bit_bit_width);
                reference_job_ptr->flags          = parser == big_endian_packed_array ? QPL_FLAG_SRC2_BE : 0;

                reference_job_ptr->available_in   = static_cast<uint32_t>(counters.size());
                reference_job_ptr->next_in_ptr    = counters.data();
                reference_job_ptr->available_out  = static_cast<uint32_t>(reference_destination.size());
                reference_job_ptr->next_out_ptr   = reference_destination.data();
                reference_job_ptr->available_src2 = static_cast<uint32_t>(symbols.size());
                reference_job_ptr->next_src2_ptr  = symbols.data();

                auto status = ref_expand_rle(reference_job_ptr);
                ASSERT_EQ(QPL_STS_OK, status) << test_case_info;

                for (uint32_t i = 0; i < reference_job_ptr->total_out; i++) {
                    ASSERT_EQ(reference_destination[i], destination[i])
                                                << test_case_info
                                                << "Invalid byte index: "
                                                << i
                                                << "\n";
                }
            } // input_bit_width cycle
        } // counter_bit_width cycle
    } // number_of_elements cycle
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(rle_burst, little_endian) {
    rle_burst_test_impl<qpl::little_endian_packed_array>();
} // TEST

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(rle_burst, big_endian) {
    rle_burst_test_impl<qpl::big_endian_packed_array>();
} // TEST

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(rle_burst, parquet_rle) {
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
        reference_job_ptr->op                 = qpl_op_rle_burst;

        for (uint32_t counter_bit_width  : {8, 16, 32}) {
            for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
                if (32u == counter_bit_width && 1u == number_of_elements) {
                    continue;
                }

                source_provider counters_generator(number_of_elements,
                                                   counter_bit_width,
                                                   seed,
                                                   low_level_parser);

                auto     counters          = counters_generator.get_counter_source_expand_rle();
                uint32_t destination_count = counters_generator.get_count_expand_rle_value();

                uint32_t destination_size = destination_count * ((input_bit_width + 7u) >> 3u);
                uint32_t symbols_count    = counter_bit_width == 32 ? (number_of_elements - 1) : number_of_elements;

                std::vector<uint8_t> destination(destination_size, 0);
                std::vector<uint8_t> reference_destination(destination_size, 0);

                source_provider symbols_generator(symbols_count,
                                                  input_bit_width,
                                                  seed);

                auto symbols = symbols_generator.get_source();

                auto test_case_info = get_test_case_info(seed,
                                                         number_of_elements,
                                                         input_bit_width,
                                                         counter_bit_width,
                                                         bit_bit_width);

                auto rle_burst_operation = typename rle_burst_operation::builder(counters.data(), counters.size())
                        .parser<parser>(number_of_elements)
                        .counter_bit_width(counter_bit_width)
                        .output_vector_width(bit_bit_width)
                        .input_vector_width(input_bit_width)
                        .build();

                uint32_t result_elements_count = 0;

                ASSERT_NO_THROW(auto operation_result = test::execute(rle_burst_operation,
                                                                      symbols,
                                                                      destination);
                                        result_elements_count = handle_result(operation_result))
                                            << test_case_info;

                reference_job_ptr->src1_bit_width = counter_bit_width;
                reference_job_ptr->src2_bit_width = input_bit_width;
                reference_job_ptr->out_bit_width  = uint_to_qpl_output(bit_bit_width);

                reference_job_ptr->available_in   = static_cast<uint32_t>(counters.size());
                reference_job_ptr->next_in_ptr    = counters.data();
                reference_job_ptr->available_out  = static_cast<uint32_t>(reference_destination.size());
                reference_job_ptr->next_out_ptr   = reference_destination.data();
                reference_job_ptr->available_src2 = static_cast<uint32_t>(symbols.size());
                reference_job_ptr->next_src2_ptr  = symbols.data();

                auto status = ref_expand_rle(reference_job_ptr);
                ASSERT_EQ(QPL_STS_OK, status) << test_case_info;

                for (uint32_t i = 0; i < reference_job_ptr->total_out; i++) {
                    ASSERT_EQ(reference_destination[i], destination[i])
                                                << test_case_info
                                                << "Invalid byte index: "
                                                << i
                                                << "\n";
                }
            } // input_bit_width cycle
        } // counter_bit_width cycle
    } // number_of_elements cycle
} // TEST

} // namespace qpl::test
