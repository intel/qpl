/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/qpl.h"

#include "base_analytic_negative_test_fixure.hpp"

namespace qpl::test {
template <qpl_operation operation, qpl_parser parser>
class AnalyticsWithoutMaskNegativeTest : public BaseAnalyticsNegativeTestFixture {

public:
    void SetUpDefaultCase() override {
        job_ptr->op                 = operation;
        job_ptr->num_input_elements = 4096;
        job_ptr->src1_bit_width     = 8U;
        job_ptr->out_bit_width      = qpl_ow_nom;
        job_ptr->parser             = parser;
        job_ptr->flags              = 0;
        job_ptr->param_low          = 0U;
        job_ptr->param_high         = (1U << job_ptr->src1_bit_width) - 1U;
    }

    void SetUp() override { BaseAnalyticsNegativeTestFixture::SetUp(); }
};

template <qpl_operation operation>
class AnalyticsWithoutMaskIncorrectBitwidthPRLETest : public BaseAnalyticsNegativeTestFixture {
public:
    void SetUpDefaultCase() override {
        job_ptr->op                 = operation;
        job_ptr->num_input_elements = 4096;
        job_ptr->src1_bit_width     = 8U;
        job_ptr->out_bit_width      = qpl_ow_nom;
        job_ptr->parser             = qpl_p_parquet_rle;
        job_ptr->flags              = 0;
        job_ptr->param_low          = 0U;
        job_ptr->param_high         = (1U << job_ptr->src1_bit_width) - 1U;
    }

    void SetUp() override { BaseAnalyticsNegativeTestFixture::SetUp(); }

    testing::AssertionResult ValidatePRLEBitwidthError() {
        job_ptr->src1_bit_width = 0U;
        source[0]               = 103U; // Set incorrect 1st byte (incorrect bit width)
        CompressSource();

        return RunStatusTest(QPL_STS_BIT_WIDTH_ERR); /* Invalid bit width */
    }
};

#define REGISTER_NEGATIVE_PRLE_BIT_WIDTH_ERROR_TEST(operation, fixture)                       \
    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(operation, fixture, incorrect_prle_inflate_bit_width) { \
        ASSERT_TRUE(ValidatePRLEBitwidthError());                                             \
    }
#define REGISTER_NEGATIVE_TEST_ON_DROP_INITIAL_BYTE(operation, fixture, postfix)                          \
    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(operation, fixture, tn_deflate_and_drop_error_handling_##postfix) { \
        ASSERT_TRUE(ValidateDecompressAndDropInitialBytesHandling());                                     \
    }

template <qpl_operation operation>
using AnalyticWithoutMaskNegativeTestLE = AnalyticsWithoutMaskNegativeTest<operation, qpl_p_le_packed_array>;

template <qpl_operation operation>
using AnalyticWithoutMaskNegativeTestRLE = AnalyticsWithoutMaskNegativeTest<operation, qpl_p_parquet_rle>;

REGISTER_NEGATIVE_TESTS_LE(scan, AnalyticWithoutMaskNegativeTestLE<qpl_op_scan_eq>)
REGISTER_NEGATIVE_TEST_ON_DROP_INITIAL_BYTE(scan, AnalyticWithoutMaskNegativeTestLE<qpl_op_scan_eq>, le)
REGISTER_NEGATIVE_TESTS_LE(extract, AnalyticWithoutMaskNegativeTestLE<qpl_op_extract>)
REGISTER_NEGATIVE_TEST_ON_DROP_INITIAL_BYTE(extract, AnalyticWithoutMaskNegativeTestLE<qpl_op_extract>, le)

REGISTER_NEGATIVE_TESTS_RLE(scan, AnalyticWithoutMaskNegativeTestRLE<qpl_op_scan_eq>)
REGISTER_NEGATIVE_TEST_ON_DROP_INITIAL_BYTE(scan, AnalyticWithoutMaskNegativeTestRLE<qpl_op_scan_eq>, rle)
REGISTER_NEGATIVE_TESTS_RLE(extract, AnalyticWithoutMaskNegativeTestRLE<qpl_op_extract>)
REGISTER_NEGATIVE_TEST_ON_DROP_INITIAL_BYTE(extract, AnalyticWithoutMaskNegativeTestRLE<qpl_op_extract>, rle)

REGISTER_NEGATIVE_PRLE_BIT_WIDTH_ERROR_TEST(scan, AnalyticsWithoutMaskIncorrectBitwidthPRLETest<qpl_op_scan_eq>)
REGISTER_NEGATIVE_PRLE_BIT_WIDTH_ERROR_TEST(extract, AnalyticsWithoutMaskIncorrectBitwidthPRLETest<qpl_op_extract>)
} // namespace qpl::test
