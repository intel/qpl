/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <vector>
#include <string>
#include "gtest/gtest.h"
#include "qpl/qpl.h"
#include "../../../common/analytic_mask_fixture.hpp"
#include "util.hpp"
#include "qpl_api_ref.h"
#include "ta_ll_common.hpp"
#include "check_result.hpp"

namespace qpl::test
{
    class SelectTest : public AnalyticMaskFixture
    {
    public:
        void InitializeTestCases()
        {
            std::vector<uint32_t> lengths = GenerateNumberOfElementsVector();

            for (uint32_t length : lengths)
            {
                for (uint32_t bit_width = 1; bit_width <= 32u; bit_width++)
                {
                    for (uint32_t destination_bit_width : {1, 8, 16, 32})
                    {
                        const uint32_t max_output_value = (1ULL << destination_bit_width) - 1;

                        if (destination_bit_width != 1u && max_output_value < length) {
                            continue;
                        } else if (destination_bit_width != 1u && bit_width > destination_bit_width) {
                            continue;
                        }

                        for (auto parser : {qpl_p_le_packed_array, qpl_p_be_packed_array, qpl_p_parquet_rle})
                        {
                            AnalyticTestCase test_case;
                            test_case.operation = qpl_op_select;
                            test_case.number_of_elements = length;
                            test_case.source_bit_width = bit_width;
                            test_case.destination_bit_width = destination_bit_width;
                            test_case.lower_bound = 0;
                            test_case.upper_bound = 1;
                            test_case.parser = parser;
                            test_case.flags = 0;
                            test_case.second_input_bit_width = 1;
                            test_case.second_input_num_elements = length;

                            AddNewTestCase(test_case);

                            test_case.flags = QPL_FLAG_SRC2_BE;
                            AddNewTestCase(test_case);

                            test_case.flags = QPL_FLAG_OUT_BE;
                            AddNewTestCase(test_case);
                        }
                    }
                }
            }
        }

        void SetUp() override
        {
            AnalyticMaskFixture::SetUp();
            InitializeTestCases();
        }
    };

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(select, analytic_only, SelectTest)
    {
        auto status = run_job_api(job_ptr);

        auto reference_status = ref_select(reference_job_ptr);

        EXPECT_EQ(QPL_STS_OK, status);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());
        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
        EXPECT_TRUE(CompareVectors(destination, reference_destination));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(select, analytic_with_decompress, SelectTest)
    {
        std::vector<uint8_t> compressed_source;
        ASSERT_NO_THROW(compressed_source = GetCompressedSource());
        job_ptr->available_in = static_cast<uint32_t>(compressed_source.size());
        job_ptr->next_in_ptr  = compressed_source.data();
        job_ptr->flags   |= QPL_FLAG_DECOMPRESS_ENABLE;

        if (GetExecutionPath() == qpl_path_software && current_test_case.parser == qpl_p_parquet_rle) {
            job_ptr->src1_bit_width = 0u;
        }

        auto status = run_job_api(job_ptr);
        EXPECT_EQ(QPL_STS_OK, status);

        auto reference_status = ref_select(reference_job_ptr);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination));
    }
}
