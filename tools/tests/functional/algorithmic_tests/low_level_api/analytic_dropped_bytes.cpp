/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tests
 */

#include "analytic_fixture.hpp"
#include "job_helper.hpp"
#include "operation_test.hpp"
#include "qpl_api_ref.h"
#include "random_generator.h"
#include "ta_ll_common.hpp"
#include "test_sources.hpp"

namespace qpl::test {

struct InputStreamMeta {
    qpl_parser parser         = qpl_p_le_packed_array;
    uint32_t   elements_count = 0;
    uint32_t   bit_width      = 0;
    bool       is_compressed  = false;
    uint32_t   prologue       = 0U;
};

static std::ostream& operator<<(std::ostream& os, const InputStreamMeta& test_case) {
    os << "Parser: " << ParserToString(test_case.parser) << '\n';
    os << "Number of elements: " << test_case.elements_count << '\n';
    os << "Source bit width: " << test_case.bit_width << '\n';
    os << "Destination bit width(hardcoded): Nominal\n";
    os << "Compressed: " << test_case.is_compressed << '\n';
    os << "Dropped bytes: " << test_case.prologue << '\n';

    return os;
}

class SimpleAnalyticFixture
    : public ReferenceFixture
    , public TestCases<InputStreamMeta> {

private:
    InputStreamMeta input_stream_meta;

protected:
    void InitializeTestCases() {
        InputStreamMeta stream_meta;

        qpl::test::random random(10, 1000, GetSeed());

        stream_meta.elements_count = static_cast<uint32_t>(random);
        random.set_range(1U, UINT16_MAX);
        stream_meta.prologue = 0; //static_cast<uint32_t>(random);
        random.set_range(1U, 32U);
        stream_meta.bit_width     = static_cast<uint32_t>(random);
        stream_meta.is_compressed = false;
        stream_meta.parser        = qpl_p_le_packed_array;
        AddNewTestCase(stream_meta);

        random.set_range(1U, bits_to_bytes(stream_meta.elements_count * stream_meta.bit_width));
        stream_meta.prologue      = static_cast<uint32_t>(random);
        stream_meta.is_compressed = true;
        AddNewTestCase(stream_meta);
    }

    void SetUp() override {
        ReferenceFixture::SetUp();
        InitializeTestCases();
    }

    void SetUpBeforeIteration() override {
        input_stream_meta = GetTestCase();

        job_ptr->num_input_elements = input_stream_meta.elements_count;
        job_ptr->src1_bit_width     = input_stream_meta.bit_width;
        job_ptr->parser             = input_stream_meta.parser;
        job_ptr->drop_initial_bytes = input_stream_meta.prologue;
        job_ptr->flags              = (input_stream_meta.is_compressed) ? QPL_FLAG_DECOMPRESS_ENABLE : 0U;

        job_ptr->out_bit_width = qpl_ow_nom;
        job_ptr->crc           = 0;
        job_ptr->xor_checksum  = 0;

        reference_job_ptr->num_input_elements = input_stream_meta.elements_count;
        reference_job_ptr->src1_bit_width     = input_stream_meta.bit_width;
        reference_job_ptr->parser             = input_stream_meta.parser;
        reference_job_ptr->drop_initial_bytes = input_stream_meta.prologue;
        reference_job_ptr->flags              = 0;

        reference_job_ptr->out_bit_width = qpl_ow_nom;
        reference_job_ptr->crc           = 0;
        reference_job_ptr->xor_checksum  = 0;
    }
};

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(analytic_with_dropped_bytes, scan, SimpleAnalyticFixture) {
    auto stream_meta = GetTestCase();
    auto status      = QPL_STS_OK;

    AnalyticInputStream input_stream(stream_meta.elements_count, stream_meta.bit_width, stream_meta.parser,
                                     stream_meta.prologue);

    destination.resize(input_stream.elements_count());
    reference_destination.resize(input_stream.elements_count());

    job_ptr->op            = qpl_op_scan_eq;
    job_ptr->next_out_ptr  = destination.data();
    job_ptr->available_out = static_cast<uint32_t>(destination.size());

    if (stream_meta.is_compressed) {
        auto compressed_input = util::compress_stream(input_stream);

        job_ptr->next_in_ptr  = compressed_input.data();
        job_ptr->available_in = static_cast<uint32_t>(compressed_input.size());
        status                = run_job_api(job_ptr);
    } else {
        job_ptr->next_in_ptr  = input_stream.data();
        job_ptr->available_in = static_cast<uint32_t>(input_stream.size());
        status                = run_job_api(job_ptr);
    }

    reference_job_ptr->op            = qpl_op_scan_eq;
    reference_job_ptr->next_in_ptr   = input_stream.data();
    reference_job_ptr->available_in  = static_cast<uint32_t>(input_stream.size());
    reference_job_ptr->next_out_ptr  = reference_destination.data();
    reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());

    auto reference_status = ref_compare(reference_job_ptr);

    ASSERT_EQ(QPL_STS_OK, status);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(compare_crc32_field(job_ptr, reference_job_ptr));
    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(analytic_with_dropped_bytes, extract, SimpleAnalyticFixture) {
    auto stream_meta = GetTestCase();
    auto status      = QPL_STS_OK;

    AnalyticInputStream input_stream(stream_meta.elements_count, stream_meta.bit_width, stream_meta.parser,
                                     stream_meta.prologue);

    destination.resize(input_stream.elements_count() * sizeof(uint32_t));
    reference_destination.resize(input_stream.elements_count() * sizeof(uint32_t));

    job_ptr->op            = qpl_op_extract;
    job_ptr->next_out_ptr  = destination.data();
    job_ptr->available_out = static_cast<uint32_t>(destination.size());
    job_ptr->param_low     = static_cast<uint32_t>(input_stream.elements_count() / 4U);
    job_ptr->param_high    = static_cast<uint32_t>(input_stream.elements_count() / 4U * 3U);

    if (stream_meta.is_compressed) {
        auto compressed_input = util::compress_stream(input_stream);

        job_ptr->next_in_ptr  = compressed_input.data();
        job_ptr->available_in = static_cast<uint32_t>(compressed_input.size());
        status                = run_job_api(job_ptr);
    } else {
        job_ptr->next_in_ptr  = input_stream.data();
        job_ptr->available_in = static_cast<uint32_t>(input_stream.size());
        status                = run_job_api(job_ptr);
    }

    reference_job_ptr->op            = qpl_op_extract;
    reference_job_ptr->next_in_ptr   = input_stream.data();
    reference_job_ptr->available_in  = static_cast<uint32_t>(input_stream.size());
    reference_job_ptr->next_out_ptr  = reference_destination.data();
    reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());
    reference_job_ptr->param_low     = static_cast<uint32_t>(input_stream.elements_count() / 4U);
    reference_job_ptr->param_high    = static_cast<uint32_t>(input_stream.elements_count() / 4U * 3U);

    auto reference_status = ref_extract(reference_job_ptr);

    ASSERT_EQ(QPL_STS_OK, status);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(compare_crc32_field(job_ptr, reference_job_ptr));
    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
}

static inline auto correct_bit_width(uint32_t bit_width, bool is_compressed) noexcept -> uint32_t {
    constexpr auto SET_OPERATION_MAX_ELEMENT_BIT_WIDTH            = 15U;
    constexpr auto SET_OPERATION_MAX_ELEMENT_BIT_WIDTH_COMPRESSED = 8U;

    auto max_acceptable_bit_width =
            (is_compressed) ? SET_OPERATION_MAX_ELEMENT_BIT_WIDTH_COMPRESSED : SET_OPERATION_MAX_ELEMENT_BIT_WIDTH;

    return (bit_width > max_acceptable_bit_width) ? max_acceptable_bit_width : bit_width;
}

} // namespace qpl::test
