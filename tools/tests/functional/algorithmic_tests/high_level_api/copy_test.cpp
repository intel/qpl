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
#include "qpl/cpp_api/operations/other/copy_operation.hpp"

#include "ta_hl_common.hpp"
#include "source_provider.hpp"
#include "qpl/qpl.h"
#include "qpl_api_ref.h"
#include "format_generator.hpp"

namespace qpl::test {

static inline auto get_test_case_info(const uint32_t seed,
                                      const uint32_t number_of_elements) -> std::string {
    std::string result = std::string("\nTest parameters:") +
                         "\n" + "Seed: " + std::to_string(seed) +
                         "\n" + "Number of elements: " + std::to_string(number_of_elements) +
                         "\n";

    return result;
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(copy, common) {
    const auto bit_width      = 8u;
    const auto seed           = util::TestEnvironment::GetInstance().GetSeed();
    const auto execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    qpl_get_job_size(execution_path, &job_size);

    auto reference_job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto reference_job_ptr    = reinterpret_cast<qpl_job *>(reference_job_buffer.get());

    auto source_length = format_generator::generate_length_sequence();

    reference_job_ptr->op = qpl_op_memcpy;

    for (auto number_of_elements : source_length) {
        source_provider source_generator(number_of_elements,
                                         bit_width,
                                         seed);

        auto source = source_generator.get_source();

        auto destination_size = static_cast<uint32_t>(source.size());

        std::vector<uint8_t> destination(destination_size, 0);
        std::vector<uint8_t> reference_destination(destination_size, 0);

        auto test_case_info = get_test_case_info(seed, number_of_elements);

        auto copy_operation = qpl::copy_operation();

        uint32_t result_elements_count = 0;

        ASSERT_NO_THROW(auto copy_operation_result = test::execute(copy_operation, source, destination);
                                result_elements_count = handle_result(copy_operation_result)) << test_case_info;

        ASSERT_EQ(destination_size, result_elements_count) << test_case_info;

        reference_job_ptr->available_in  = static_cast<uint32_t>(source.size());
        reference_job_ptr->next_in_ptr   = source.data();
        reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());
        reference_job_ptr->next_out_ptr  = reference_destination.data();

        auto status = ref_copy_8u(reference_job_ptr);
        ASSERT_EQ(QPL_STS_OK, status) << test_case_info;

        for (uint32_t i = 0; i < reference_job_ptr->total_out; i++) {
            ASSERT_EQ(reference_destination[i], destination[i])
                                        << test_case_info
                                        << "Invalid byte index: "
                                        << i
                                        << "\n";
        }
    } // number_of_elements cycle
} // TEST

}
