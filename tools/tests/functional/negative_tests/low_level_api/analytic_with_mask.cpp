/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/qpl.h"

#include "base_analytic_negative_test_fixure.hpp"

namespace qpl::test {
template <qpl_operation operation, qpl_parser parser>
class AnalyticWithMaskNegativeTest : public BaseAnalyticsNegativeTestFixture {

public:
    void SetUpDefaultCase() override {
        job_ptr->op                 = operation;
        job_ptr->num_input_elements = 4096U;
        job_ptr->src1_bit_width     = 8U;
        job_ptr->out_bit_width      = qpl_ow_nom;
        job_ptr->parser             = parser;
        job_ptr->flags              = 0U;
        job_ptr->src2_bit_width     = 1U;

        second_input_elements = job_ptr->num_input_elements;
        job_ptr->param_low    = 0U;
        job_ptr->param_high   = job_ptr->num_input_elements / 2U;
    }

    void PrepareMask() {
        source_provider mask_gen(second_input_elements, job_ptr->src2_bit_width, GetSeed());

        ASSERT_NO_THROW(mask = mask_gen.get_source()); //NOLINT(cppcoreguidelines-avoid-goto)

        job_ptr->available_src2 = static_cast<uint32_t>(mask.size());
        job_ptr->next_src2_ptr  = mask.data();
    }

    void SetUp() override {
        BaseAnalyticsNegativeTestFixture::SetUp();
        PrepareMask();
    }

private:
    std::vector<uint8_t> mask;
    uint32_t             second_input_elements = 0U;
};

template <qpl_operation operation>
class AnalyticsWithMaskIncorrectBitwidthPRLETest : public BaseAnalyticsNegativeTestFixture {
public:
    void SetUpDefaultCase() override {
        job_ptr->op                 = operation;
        job_ptr->num_input_elements = 4096;
        job_ptr->src1_bit_width     = 8U;
        job_ptr->out_bit_width      = qpl_ow_nom;
        job_ptr->parser             = qpl_p_parquet_rle;
        job_ptr->flags              = 0U;
        job_ptr->src2_bit_width     = 1U;

        second_input_elements = job_ptr->num_input_elements;
        job_ptr->param_low    = 0U;
        job_ptr->param_high   = job_ptr->num_input_elements / 2U;
    }

    void PrepareMask() {
        source_provider mask_gen(second_input_elements, job_ptr->src1_bit_width, GetSeed());

        ASSERT_NO_THROW(mask = mask_gen.get_source()); //NOLINT(cppcoreguidelines-avoid-goto)

        job_ptr->available_src2 = static_cast<uint32_t>(mask.size());
        job_ptr->next_src2_ptr  = mask.data();
    }

    void SetUp() override {
        BaseAnalyticsNegativeTestFixture::SetUp();
        PrepareMask();
    }

    testing::AssertionResult ValidatePRLEBitwidthError() {
        job_ptr->src1_bit_width = 0U;
        source[0U]              = 103U; // Set incorrect 1st byte (incorrect bit width)
        CompressSource();

        return RunStatusTest(QPL_STS_BIT_WIDTH_ERR); /* Invalid bit width */
    }

private:
    std::vector<uint8_t> mask;
    uint32_t             second_input_elements = 0U;
};

#define REGISTER_NEGATIVE_PRLE_BIT_WIDTH_ERROR_TEST(operation, fixture)                       \
    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(operation, fixture, incorrect_prle_inflate_bit_width) { \
        ASSERT_TRUE(ValidatePRLEBitwidthError());                                             \
    }

template <qpl_operation operation>
using AnalyticWithMaskNegativeTestLE = AnalyticWithMaskNegativeTest<operation, qpl_p_le_packed_array>;

template <qpl_operation operation>
using AnalyticWithMaskNegativeTestRLE = AnalyticWithMaskNegativeTest<operation, qpl_p_parquet_rle>;

template <qpl_out_format out_format>
class ExpandIncorrectNumElementsTest : public BaseAnalyticsNegativeTestFixture {
public:
    void SetUpDefaultCase() override {
        job_ptr->op = qpl_op_expand;
        if constexpr (qpl_ow_8 == out_format) {
            job_ptr->num_input_elements = 257U;
        } else if constexpr (qpl_ow_16 == out_format) {
            job_ptr->num_input_elements = 65537U;
        } else {
            return;
        }

        second_input_elements = job_ptr->num_input_elements;

        job_ptr->src1_bit_width = 1U;
        job_ptr->out_bit_width  = out_format;
        job_ptr->parser         = qpl_p_le_packed_array;
        job_ptr->flags          = 0U;
        job_ptr->src2_bit_width = 1U;
    }

    void PrepareMask() {
        ASSERT_NO_THROW( //NOLINT(cppcoreguidelines-avoid-goto)
                mask.resize(qpl::test::bits_to_bytes(second_input_elements)));

        const uint32_t size = static_cast<uint32_t>(mask.size());

        for (uint32_t i = 0U; i < size; ++i) {
            mask[i] = 0xFFU;
        }

        job_ptr->available_src2 = size;
        job_ptr->next_src2_ptr  = mask.data();
    }

    void SetUp() override {
        BaseAnalyticsNegativeTestFixture::SetUp();
        PrepareMask();
    }

    testing::AssertionResult ValidateIncorrectNumElementsWithIndexOutputError() {
        for (uint32_t i = 0U; i < mask.size(); ++i) {
            source[i] = 0xFFU;
        }

        return RunStatusTest(QPL_STS_OUTPUT_OVERFLOW_ERR); /* Invalid bit width */
    }

private:
    std::vector<uint8_t> mask;
    uint32_t             second_input_elements = 0U;
};

#define REGISTER_NEGATIVE_EXPAND_NUM_ELEMENTS_TEST(fixture, name)        \
    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(qpl_op_expand, fixture, name) {    \
        ASSERT_TRUE(ValidateIncorrectNumElementsWithIndexOutputError()); \
    }

REGISTER_NEGATIVE_TESTS_LE(expand, AnalyticWithMaskNegativeTestLE<qpl_op_expand>)
REGISTER_NEGATIVE_TESTS_LE(select, AnalyticWithMaskNegativeTestLE<qpl_op_select>)

REGISTER_NEGATIVE_TESTS_RLE(expand, AnalyticWithMaskNegativeTestRLE<qpl_op_expand>)
REGISTER_NEGATIVE_TESTS_RLE(select, AnalyticWithMaskNegativeTestRLE<qpl_op_select>)

REGISTER_NEGATIVE_PRLE_BIT_WIDTH_ERROR_TEST(expand, AnalyticsWithMaskIncorrectBitwidthPRLETest<qpl_op_expand>)
REGISTER_NEGATIVE_PRLE_BIT_WIDTH_ERROR_TEST(select, AnalyticsWithMaskIncorrectBitwidthPRLETest<qpl_op_select>)

REGISTER_NEGATIVE_EXPAND_NUM_ELEMENTS_TEST(ExpandIncorrectNumElementsTest<qpl_ow_8>,
                                           incorrect_num_elements_with_index_output_8u)
REGISTER_NEGATIVE_EXPAND_NUM_ELEMENTS_TEST(ExpandIncorrectNumElementsTest<qpl_ow_16>,
                                           incorrect_num_elements_with_index_output_16u)
} // namespace qpl::test
