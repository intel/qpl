/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

// tests_common
#include "analytic_fixture.hpp"
#include "operation_test.hpp"
#include "ref_min_max_sum.h"
#include "ta_ll_common.hpp"

namespace qpl::test {
class MinMaxSumTest : public JobFixtureWithTestCases<AnalyticTestCase> {
private:
    AnalyticTestCase current_test_case;

public:
    void SetUpBeforeIteration() override {
        current_test_case = GetTestCase();

        source_provider source_generator(current_test_case.number_of_elements, current_test_case.source_bit_width,
                                         GetSeed());

        ASSERT_NO_THROW(source = source_generator.get_source()); //NOLINT(cppcoreguidelines-avoid-goto)
        destination.resize(current_test_case.number_of_elements *
                           (current_test_case.destination_bit_width / BYTE_BIT_LENGTH + 1));

        job_ptr->next_in_ptr   = source.data();
        job_ptr->available_in  = static_cast<uint32_t>(source.size());
        job_ptr->next_out_ptr  = destination.data();
        job_ptr->available_out = static_cast<uint32_t>(destination.size());

        job_ptr->param_low          = current_test_case.lower_bound;
        job_ptr->out_bit_width      = uint_to_qpl_output(current_test_case.destination_bit_width);
        job_ptr->src1_bit_width     = current_test_case.source_bit_width;
        job_ptr->num_input_elements = current_test_case.number_of_elements;
        job_ptr->flags              = current_test_case.flags;
    }

    void InitializeTestCases() override {
        for (uint32_t length = 1000U; length < 1005U; length++) {
            for (uint32_t output_format = 0; output_format < 2; output_format++) {
                for (const uint32_t out_bit_width : {1, 8, 16, 32}) {
                    const uint32_t max_output_value = (1ULL << out_bit_width) - 1;

                    if (out_bit_width != 1U && max_output_value < length) { continue; }

                    for (uint32_t input_bit_width = 1U;
                         input_bit_width <= (qpl_ow_nom == out_bit_width ? 32 : out_bit_width); input_bit_width++) {
                        AnalyticTestCase test_case;
                        test_case.number_of_elements    = length;
                        test_case.source_bit_width      = input_bit_width;
                        test_case.flags                 = (1U == output_format) ? QPL_FLAG_OUT_BE : 0U;
                        test_case.destination_bit_width = out_bit_width;
                        test_case.lower_bound = (qpl_ow_nom == out_bit_width) ? 1 : (1 << input_bit_width) / 4;

                        AddNewTestCase(test_case);
                    }
                }
            }
        }
    }
};

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(aggregates, min_max_sum, MinMaxSumTest) {
    job_ptr->op = qpl_op_scan_ne;
    auto status = run_job_api(job_ptr);

    ASSERT_EQ(QPL_STS_OK, status);

    const uint32_t library_min_value = job_ptr->first_index_min_value;
    const uint32_t library_max_value = job_ptr->last_index_max_value;
    const uint32_t library_sum_value = job_ptr->sum_value;

    uint32_t reference_min_value = 0;
    uint32_t reference_max_value = 0;
    uint32_t reference_sum_value = 0;

    ref_min_max_sum(destination.data(), job_ptr->total_out, job_ptr->last_bit_offset, job_ptr->out_bit_width, 1,
                    job_ptr->flags & QPL_FLAG_OUT_BE, 0, &reference_min_value, &reference_max_value,
                    &reference_sum_value);

    ASSERT_EQ(library_min_value, reference_min_value);
    ASSERT_EQ(library_max_value, reference_max_value);
    ASSERT_EQ(library_sum_value, reference_sum_value);
}
} // namespace qpl::test
