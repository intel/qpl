/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

// tests_common
#include <memory>

#include "operation_test.hpp"
#include "random_generator.h"
#include "source_provider.hpp"
#include "ta_ll_common.hpp"
#include "test_cases.hpp"

namespace qpl::test {
class inflate_stop_condtition_test : public JobFixture {
    void SetUp() override { JobFixture::SetUp(); }

protected:
    void deflate_stream_by_blocks(std::vector<uint8_t>& source_stream, std::vector<uint8_t>& compressed_destination,
                                  uint32_t block_size) {
        std::unique_ptr<uint8_t[]> job_buffer;
        qpl_job*                   deflate_job_ptr = nullptr;

        uint32_t job_size = 0;
        auto     status   = qpl_get_job_size(GetExecutionPath(), &job_size);
        ASSERT_EQ(QPL_STS_OK, status);

        job_buffer      = std::make_unique<uint8_t[]>(job_size);
        deflate_job_ptr = reinterpret_cast<qpl_job*>(job_buffer.get());

        status = qpl_init_job(GetExecutionPath(), deflate_job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        auto source_size = static_cast<uint32_t>(source_stream.size());

        deflate_job_ptr->op            = qpl_op_compress;
        deflate_job_ptr->next_in_ptr   = source_stream.data();
        deflate_job_ptr->next_out_ptr  = compressed_destination.data();
        deflate_job_ptr->available_out = static_cast<uint32_t>(compressed_destination.size());
        deflate_job_ptr->level         = qpl_default_level;

        deflate_job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_OMIT_VERIFY;

        while (deflate_job_ptr->total_in < source_size) {
            if (deflate_job_ptr->total_in + block_size >= source_size) {
                deflate_job_ptr->available_in = source_size - deflate_job_ptr->total_in;
                deflate_job_ptr->flags |= QPL_FLAG_LAST;
            } else {
                deflate_job_ptr->available_in += block_size;
            }

            auto status = run_job_api(deflate_job_ptr);
            ASSERT_EQ(QPL_STS_OK, status);

            deflate_job_ptr->flags |= QPL_FLAG_START_NEW_BLOCK;
            deflate_job_ptr->flags &= ~QPL_FLAG_FIRST;
        }

        compressed_destination.resize(deflate_job_ptr->total_out);
    }
};

// 1.a Identical test to 1, except for the fact, that there's only one bfinal eob symbol
QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(inflate_stop_conditions, dont_stop_on_eob, inflate_stop_condtition_test) {
    auto dataset = util::TestEnvironment::GetInstance().GetCompleteDataset();
    source       = dataset["file1"];

    std::vector<uint8_t> compressed_source(source.size() * 2);
    destination.resize(compressed_source.size() * 2);

    auto              source_size = static_cast<uint32_t>(source.size());
    qpl::test::random random(0U, 0U, GetSeed());

    random.set_range(1U, 5000U);
    auto block_size = static_cast<uint32_t>(random);

    // Get stream compressed with severals deflate blocks
    // Which means we have multiple eob symbols here
    deflate_stream_by_blocks(source, compressed_source, block_size);

    // Set up job, set dont_stop_or_check condition
    job_ptr->next_in_ptr           = compressed_source.data();
    job_ptr->available_in          = static_cast<uint32_t>(compressed_source.size());
    job_ptr->next_out_ptr          = destination.data();
    job_ptr->available_out         = static_cast<uint32_t>(destination.size());
    job_ptr->flags                 = QPL_FLAG_FIRST | QPL_FLAG_DECOMP_FLUSH_ALWAYS;
    job_ptr->decomp_end_processing = qpl_dont_stop_or_check;

    auto status = run_job_api(job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);

    // Expect the whole compressed stream to be decompressed
    ASSERT_EQ(job_ptr->total_out, source_size);

    ASSERT_TRUE(CompareVectors(source, destination, source_size));
}

// 2. Stop on any EOB symbol
QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(inflate_stop_conditions, stop_on_eob, inflate_stop_condtition_test) {
    auto dataset = util::TestEnvironment::GetInstance().GetCompleteDataset();
    source       = dataset["file1"];

    std::vector<uint8_t> compressed_source(source.size() * 2);
    destination.resize(compressed_source.size() * 2);

    auto              source_size = static_cast<uint32_t>(source.size());
    qpl::test::random random(0U, 0U, GetSeed());

    random.set_range(1U, 5000U);
    auto block_size = static_cast<uint32_t>(random);

    // Get stream compressed with severals deflate blocks
    // Which means we have multiple eob symbols here
    deflate_stream_by_blocks(source, compressed_source, block_size);

    // Set up job, set stop_on_eob condition
    job_ptr->next_in_ptr           = compressed_source.data();
    job_ptr->available_in          = static_cast<uint32_t>(compressed_source.size());
    job_ptr->next_out_ptr          = destination.data();
    job_ptr->available_out         = static_cast<uint32_t>(destination.size());
    job_ptr->flags                 = QPL_FLAG_FIRST | QPL_FLAG_DECOMP_FLUSH_ALWAYS;
    job_ptr->decomp_end_processing = qpl_stop_on_any_eob;

    auto status = run_job_api(job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);

    // Expect just the 1st deflate block to be decompressed
    ASSERT_EQ(job_ptr->total_out, block_size);

    ASSERT_TRUE(CompareVectors(source, destination, block_size));
}
} // namespace qpl::test
