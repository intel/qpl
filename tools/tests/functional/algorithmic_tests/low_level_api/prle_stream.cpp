/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <vector>

#include "prle_generator.hpp"
#include "source_provider.hpp"
#include "util.hpp"

// tests_common
#include "operation_test.hpp"
#include "qpl_api_ref.h"
#include "ta_ll_common.hpp"

constexpr uint32_t rle_element_bit_width = 7U;
constexpr uint32_t rle_element_value     = 113U;

constexpr uint32_t rle_burst_counter_element_value = 3U;
constexpr uint32_t rle_burst_counter_bit_width     = 8U;

constexpr uint32_t extract_lower_index = 100U;
constexpr uint32_t extract_upper_index = 1000U;

constexpr uint32_t rle_burst_src2_bit_width = 11U;

constexpr qpl_parser second_source_parser = qpl_p_le_packed_array;

namespace qpl::test {

typedef qpl_status (*reference_function_ptr_t)(qpl_job*);

class ParquetRLEParserTest : public ReferenceFixture {
public:
    template <reference_function_ptr_t reference_function_ptr>
    void verify_operation_on_single_rle_group(rle_element_t& rle_element) {
        auto parquet_group = create_rle_group(rle_element);

        // Source should contain single rle group + 1st byte as prle stream bit width
        source.resize(parquet_group.size() + 1);
        source[0] = static_cast<uint32_t>(rle_element.bit_width);

        std::copy(parquet_group.begin(), parquet_group.end(), source.begin() + 1);

        const uint32_t destination_size = rle_element.repeat_count * (bits_to_bytes(rle_element.bit_width));

        destination.resize(destination_size);
        reference_destination.resize(destination_size);

        job_ptr->next_in_ptr        = source.data();
        job_ptr->available_in       = static_cast<uint32_t>(source.size());
        job_ptr->next_out_ptr       = destination.data();
        job_ptr->available_out      = static_cast<uint32_t>(destination.size());
        job_ptr->num_input_elements = rle_element.repeat_count;
        job_ptr->parser             = qpl_p_parquet_rle;

        // TODO: REMOVE
        job_ptr->src1_bit_width = rle_element.bit_width;

        reference_job_ptr->next_in_ptr        = source.data();
        reference_job_ptr->available_in       = static_cast<uint32_t>(source.size());
        reference_job_ptr->next_out_ptr       = reference_destination.data();
        reference_job_ptr->available_out      = static_cast<uint32_t>(reference_destination.size());
        reference_job_ptr->num_input_elements = rle_element.repeat_count;
        reference_job_ptr->parser             = qpl_p_parquet_rle;

        // TODO: REMOVE
        reference_job_ptr->src1_bit_width = rle_element.bit_width;

        auto library_status = run_job_api(job_ptr);

        ASSERT_EQ(QPL_STS_OK, library_status);

        auto reference_status = reference_function_ptr(reference_job_ptr);

        ASSERT_EQ(QPL_STS_OK, reference_status);

        ASSERT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    void verify_rle_burst_on_single_rle_group(rle_element_t& rle_element) {
        auto parquet_group = create_rle_group(rle_element);

        // Source should contain single rle group + 1st byte as prle stream bit width
        source.resize(parquet_group.size() + 1);
        source[0] = static_cast<uint32_t>(rle_element.bit_width);

        std::copy(parquet_group.begin(), parquet_group.end(), source.begin() + 1);

        const uint32_t destination_size =
                rle_element.repeat_count * bits_to_bytes(job_ptr->src2_bit_width) * rle_element.element_value;

        destination.resize(destination_size);
        reference_destination.resize(destination_size);

        job_ptr->next_in_ptr        = source.data();
        job_ptr->available_in       = static_cast<uint32_t>(source.size());
        job_ptr->next_out_ptr       = destination.data();
        job_ptr->available_out      = static_cast<uint32_t>(destination.size());
        job_ptr->num_input_elements = rle_element.repeat_count;
        job_ptr->parser             = qpl_p_parquet_rle;

        // TODO: REMOVE
        job_ptr->src1_bit_width = rle_element.bit_width;

        reference_job_ptr->next_in_ptr        = source.data();
        reference_job_ptr->available_in       = static_cast<uint32_t>(source.size());
        reference_job_ptr->next_out_ptr       = reference_destination.data();
        reference_job_ptr->available_out      = static_cast<uint32_t>(reference_destination.size());
        reference_job_ptr->num_input_elements = rle_element.repeat_count;
        reference_job_ptr->parser             = qpl_p_parquet_rle;

        // TODO: REMOVE
        reference_job_ptr->src1_bit_width = rle_element.bit_width;

        auto library_status = run_job_api(job_ptr);

        ASSERT_EQ(QPL_STS_OK, library_status);

        auto reference_status = ref_expand_rle(reference_job_ptr);

        ASSERT_EQ(QPL_STS_OK, reference_status);

        ASSERT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }
};

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(prle_parser, scan_large_single_rle_group, ParquetRLEParserTest) {
    job_ptr->op           = qpl_op_scan_eq;
    reference_job_ptr->op = qpl_op_scan_eq;

    rle_element_t rle_element;
    rle_element.bit_width     = rle_element_bit_width;
    rle_element.element_value = rle_element_value;
    rle_element.repeat_count  = 4000U;

    verify_operation_on_single_rle_group<ref_compare>(rle_element);

    rle_element.repeat_count = 10000U;
    verify_operation_on_single_rle_group<ref_compare>(rle_element);

    rle_element.repeat_count = 30000U;
    verify_operation_on_single_rle_group<ref_compare>(rle_element);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(prle_parser, extract_large_single_rle_group, ParquetRLEParserTest) {
    job_ptr->op           = qpl_op_extract;
    reference_job_ptr->op = qpl_op_extract;

    job_ptr->param_low  = extract_lower_index;
    job_ptr->param_high = extract_upper_index;

    reference_job_ptr->param_low  = extract_lower_index;
    reference_job_ptr->param_high = extract_upper_index;

    rle_element_t rle_element;
    rle_element.bit_width     = rle_element_bit_width;
    rle_element.element_value = rle_element_value;
    rle_element.repeat_count  = 4000U;

    verify_operation_on_single_rle_group<ref_extract>(rle_element);

    rle_element.repeat_count = 10000U;
    verify_operation_on_single_rle_group<ref_extract>(rle_element);

    rle_element.repeat_count = 30000U;
    verify_operation_on_single_rle_group<ref_extract>(rle_element);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(prle_parser, expand_large_single_rle_group, ParquetRLEParserTest) {
    job_ptr->op           = qpl_op_expand;
    reference_job_ptr->op = qpl_op_expand;

    source_provider src2_generator(30000U, 1U, GetSeed());

    auto source2 = src2_generator.get_source();

    job_ptr->src2_bit_width           = 1U;
    reference_job_ptr->src2_bit_width = 1U;

    rle_element_t rle_element;
    rle_element.bit_width     = rle_element_bit_width;
    rle_element.element_value = rle_element_value;
    rle_element.repeat_count  = 4000U;

    job_ptr->next_src2_ptr            = source2.data();
    job_ptr->available_src2           = bits_to_bytes(rle_element.repeat_count);
    reference_job_ptr->next_src2_ptr  = source2.data();
    reference_job_ptr->available_src2 = bits_to_bytes(rle_element.repeat_count);
    verify_operation_on_single_rle_group<ref_expand>(rle_element);

    rle_element.repeat_count          = 10000U;
    job_ptr->next_src2_ptr            = source2.data();
    job_ptr->available_src2           = bits_to_bytes(rle_element.repeat_count);
    reference_job_ptr->next_src2_ptr  = source2.data();
    reference_job_ptr->available_src2 = bits_to_bytes(rle_element.repeat_count);
    verify_operation_on_single_rle_group<ref_expand>(rle_element);

    rle_element.repeat_count          = 30000U;
    job_ptr->next_src2_ptr            = source2.data();
    job_ptr->available_src2           = bits_to_bytes(rle_element.repeat_count);
    reference_job_ptr->next_src2_ptr  = source2.data();
    reference_job_ptr->available_src2 = bits_to_bytes(rle_element.repeat_count);
    verify_operation_on_single_rle_group<ref_expand>(rle_element);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(prle_parser, select_large_single_rle_group, ParquetRLEParserTest) {
    job_ptr->op           = qpl_op_select;
    reference_job_ptr->op = qpl_op_select;

    source_provider src2_generator(30000U, 1U, GetSeed());

    auto source2 = src2_generator.get_source();

    job_ptr->src2_bit_width           = 1U;
    reference_job_ptr->src2_bit_width = 1U;

    rle_element_t rle_element;
    rle_element.bit_width     = rle_element_bit_width;
    rle_element.element_value = rle_element_value;
    rle_element.repeat_count  = 4000U;

    job_ptr->next_src2_ptr            = source2.data();
    job_ptr->available_src2           = bits_to_bytes(rle_element.repeat_count);
    reference_job_ptr->next_src2_ptr  = source2.data();
    reference_job_ptr->available_src2 = bits_to_bytes(rle_element.repeat_count);
    verify_operation_on_single_rle_group<ref_select>(rle_element);

    rle_element.repeat_count          = 10000U;
    job_ptr->next_src2_ptr            = source2.data();
    job_ptr->available_src2           = bits_to_bytes(rle_element.repeat_count);
    reference_job_ptr->next_src2_ptr  = source2.data();
    reference_job_ptr->available_src2 = bits_to_bytes(rle_element.repeat_count);
    verify_operation_on_single_rle_group<ref_select>(rle_element);

    rle_element.repeat_count          = 30000U;
    job_ptr->next_src2_ptr            = source2.data();
    job_ptr->available_src2           = bits_to_bytes(rle_element.repeat_count);
    reference_job_ptr->next_src2_ptr  = source2.data();
    reference_job_ptr->available_src2 = bits_to_bytes(rle_element.repeat_count);
    verify_operation_on_single_rle_group<ref_select>(rle_element);
}

} // namespace qpl::test
