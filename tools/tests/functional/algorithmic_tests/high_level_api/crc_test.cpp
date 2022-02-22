/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <vector>
#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "high_level_api_util.hpp"
#include "qpl/cpp_api/operations/other/crc_operation.hpp"

#include "ta_hl_common.hpp"
#include "source_provider.hpp"
#include "qpl/qpl.h"
#include "qpl_api_ref.h"
#include "format_generator.hpp"

namespace qpl::test {

static constexpr const auto default_bit_width = 8u;

static inline auto get_test_case_info(const bool is_inverse,
                                      const uint32_t length,
                                      const uint32_t poly_shift,
                                      const uint32_t seed) -> std::string {
    return std::string("\nTest parameters:") +
           "\n" + "Source length: " + std::to_string(length) +
           "\n" + "Polynomial shift: " + std::to_string(poly_shift) +
           "\n" + "CRC direction: " + (is_inverse ? "inverse" : "forward") +
           "\n" + "Seed: " + std::to_string(seed) +
           "\n";
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(crc, little_endian) {
    const auto seed           = util::TestEnvironment::GetInstance().GetSeed();
    const auto execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    qpl_get_job_size(execution_path, &job_size);

    auto reference_job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto reference_job_ptr    = reinterpret_cast<qpl_job *>(reference_job_buffer.get());

    auto source_length = format_generator::generate_length_sequence();

    reference_job_ptr->op = qpl_op_crc64;

    for (auto number_of_elements : source_length) {
        for (uint32_t poly_shift : {0u, 32u, 48u}) {
            for (auto is_inverse : {false, true}) {
                source_provider source_generator(number_of_elements,
                                                 default_bit_width,
                                                 seed);

                auto source     = source_generator.get_source();
                auto polynomial = format_generator::get_random_crc64_poly(poly_shift, seed);

                auto test_case_info = get_test_case_info(is_inverse,
                                                         number_of_elements,
                                                         poly_shift,
                                                         seed);

                auto crc_operation = qpl::crc_operation::builder(polynomial)
                        .bit_order<parsers::little_endian_packed_array>()
                        .is_inverse(is_inverse)
                        .build();

                uint64_t crc_value         = 0;
                auto     destination_begin = reinterpret_cast<uint8_t *>(&crc_value);
                uint8_t  *destination_end  = destination_begin + 8u;

                switch (execution_path) {
                    case qpl_path_software: {
                        auto result = qpl::execute<qpl::software>(crc_operation,
                                                                  source.begin(),
                                                                  source.end(),
                                                                  destination_begin,
                                                                  destination_end);
                        ASSERT_NO_THROW(handle_result(result)) << test_case_info;
                        break;
                    }
                    case qpl_path_hardware: {
                        auto result = qpl::execute<qpl::hardware>(crc_operation,
                                                                  source.begin(),
                                                                  source.end(),
                                                                  destination_begin,
                                                                  destination_end);
                        ASSERT_NO_THROW(handle_result(result)) << test_case_info;
                        break;
                    }
                    default: {
                        auto result = qpl::execute<qpl::auto_detect>(crc_operation,
                                                                     source.begin(),
                                                                     source.end(),
                                                                     destination_begin,
                                                                     destination_end);
                        ASSERT_NO_THROW(handle_result(result)) << test_case_info;
                    }
                }

                reference_job_ptr->available_in = static_cast<uint32_t>(source.size());
                reference_job_ptr->next_in_ptr  = source.data();
                reference_job_ptr->crc64_poly   = polynomial;

                if (is_inverse) {
                    reference_job_ptr->flags |= QPL_FLAG_CRC64_INV;
                } else {
                    reference_job_ptr->flags &= ~QPL_FLAG_CRC64_INV;
                }

                auto status = ref_crc64(reference_job_ptr);
                ASSERT_EQ(QPL_STS_OK, status) << test_case_info;

                auto reference_crc_value = reference_job_ptr->crc64;

                ASSERT_EQ(reference_crc_value, crc_value)
                                            << test_case_info
                                            << "Invalid crc value.";
            } // is_inverse cycle
        } // poly_shift cycle
    } // number_of_elements cycle
} // TEST

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(crc, big_endian) {
    const auto seed           = util::TestEnvironment::GetInstance().GetSeed();
    const auto execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    qpl_get_job_size(execution_path, &job_size);

    auto reference_job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto reference_job_ptr    = reinterpret_cast<qpl_job *>(reference_job_buffer.get());

    auto source_length = format_generator::generate_length_sequence();

    reference_job_ptr->op = qpl_op_crc64;

    for (auto number_of_elements : source_length) {
        for (uint32_t poly_shift : {0u, 32u, 48u}) {
            for (auto is_inverse : {false, true}) {
                source_provider source_generator(number_of_elements,
                                                 default_bit_width,
                                                 seed);

                auto source     = source_generator.get_source();
                auto polynomial = format_generator::get_random_crc64_poly(poly_shift, seed);

                auto test_case_info = get_test_case_info(is_inverse,
                                                         number_of_elements,
                                                         poly_shift,
                                                         seed);

                auto crc_operation = qpl::crc_operation::builder(polynomial)
                        .bit_order<parsers::big_endian_packed_array>()
                        .is_inverse(is_inverse)
                        .build();

                uint64_t crc_value         = 0;
                auto     destination_begin = reinterpret_cast<uint8_t *>(&crc_value);
                uint8_t  *destination_end  = destination_begin + 8u;

                ASSERT_NO_THROW(auto crc_operation_result = test::execute(crc_operation,
                                                                          source.begin(),
                                                                          source.end(),
                                                                          destination_begin,
                                                                          destination_end);
                                        handle_result(crc_operation_result)) << test_case_info;

                reference_job_ptr->available_in = static_cast<uint32_t>(source.size());
                reference_job_ptr->next_in_ptr  = source.data();
                reference_job_ptr->crc64_poly   = polynomial;
                reference_job_ptr->flags        = QPL_FLAG_CRC64_BE;

                if (is_inverse) {
                    reference_job_ptr->flags |= QPL_FLAG_CRC64_INV;
                }

                auto status = ref_crc64(reference_job_ptr);
                ASSERT_EQ(QPL_STS_OK, status) << test_case_info;

                auto reference_crc_value = reference_job_ptr->crc64;

                ASSERT_EQ(reference_crc_value, crc_value)
                                            << test_case_info
                                            << "Invalid crc value.";
            } // is_inverse cycle
        } // poly_shift cycle
    } // number_of_elements cycle
} // TEST

}
