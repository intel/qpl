/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <algorithm>
#include <array>

#include "compression_huffman_table.hpp"

// tests_common
#include "operation_test.hpp"
#include "random_generator.h"
#include "source_provider.hpp"
#include "ta_ll_common.hpp"
#include "test_cases.hpp"

namespace qpl::test {
class JobReusageTest : public JobFixtureWithTestCases<std::string> {
private:
    std::vector<uint8_t> compressed_source;
    std::vector<uint8_t> decompressed_source;

protected:
    void InitializeTestCases() override {
        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
            AddNewTestCase(dataset.first);
        }
    }

    void SetUpBeforeIteration() override {
        auto current_test_case = GetTestCase();

        auto dataset = util::TestEnvironment::GetInstance().GetAlgorithmicDataset();

        source = dataset[current_test_case];
    }

    void CleanUpAfterIteration() override {}

    void CompressWithJobReusage(bool chunked, qpl_compression_levels level) {
        const uint32_t file_size = static_cast<uint32_t>(source.size());

        const uint32_t min_chunk_size = chunked ? 1024 : file_size;

        compressed_source.resize(source.size() * 2U, 0);
        decompressed_source.resize(source.size(), 0);

        // Compression by chunks

        qpl::test::random chunk_size(min_chunk_size, file_size, GetSeed());

        job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_DYNAMIC_HUFFMAN | QPL_FLAG_OMIT_VERIFY;
        job_ptr->next_in_ptr   = source.data();
        job_ptr->available_out = static_cast<uint32_t>(compressed_source.size());
        job_ptr->next_out_ptr  = compressed_source.data();
        job_ptr->level         = level;
        job_ptr->op            = qpl_op_compress;

        auto next_in_chunk = static_cast<uint32_t>(chunk_size);

        while (job_ptr->total_in < file_size || (job_ptr->flags & QPL_FLAG_FIRST)) {
            auto bytes_processed = (job_ptr->flags & QPL_FLAG_FIRST) ? 0U : job_ptr->total_in;
            if ((bytes_processed + next_in_chunk) >= file_size) {
                next_in_chunk = file_size - bytes_processed;
                job_ptr->flags |= QPL_FLAG_LAST;
            }

            job_ptr->available_in = next_in_chunk;

            auto deflate_status = qpl_execute_job(job_ptr);
            ASSERT_EQ(QPL_STS_OK, deflate_status);

            job_ptr->flags &= ~QPL_FLAG_FIRST; // Reset first flag
        }

        compressed_source.resize(job_ptr->total_out);

        // Decompression by chunks via the same job

        job_ptr->next_in_ptr   = compressed_source.data();
        job_ptr->next_out_ptr  = decompressed_source.data();
        job_ptr->available_out = static_cast<uint32_t>(decompressed_source.size());
        job_ptr->flags         = QPL_FLAG_FIRST;
        job_ptr->op            = qpl_op_decompress;

        next_in_chunk = static_cast<uint32_t>(chunk_size);

        while (job_ptr->total_in < compressed_source.size() || (job_ptr->flags & QPL_FLAG_FIRST)) {
            auto bytes_processed = (job_ptr->flags & QPL_FLAG_FIRST) ? 0U : job_ptr->total_in;
            if ((bytes_processed + next_in_chunk) >= compressed_source.size()) {
                next_in_chunk = static_cast<uint32_t>(compressed_source.size()) - bytes_processed;
                job_ptr->flags |= QPL_FLAG_LAST;
            }

            job_ptr->available_in = next_in_chunk;

            auto inflate_status = qpl_execute_job(job_ptr);

            if (QPL_STS_MORE_OUTPUT_NEEDED == inflate_status && job_ptr->total_out >= file_size) {
                // There is a case when Inflate doesn't set available_in with 0 but set total_out to expected final result (orig file's size)
                // and decompress buffer's content is also equal to original file, i.e. need to investigate this case deeply

                break;
            } else {
                ASSERT_EQ(QPL_STS_OK, inflate_status);
            }

            if (job_ptr->total_out >= file_size) { break; }

            job_ptr->flags &= ~QPL_FLAG_FIRST; // Reset FIRST flag
        }

        ASSERT_TRUE(CompareVectors(decompressed_source, source));
    }
};

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(job_reusage, compression_default_level, JobReusageTest) {
    CompressWithJobReusage(false, qpl_default_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(job_reusage, compression_by_chunks_default_level, JobReusageTest) {
    CompressWithJobReusage(true, qpl_default_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(job_reusage, compression_high_level, JobReusageTest) {
    if (GetExecutionPath() == qpl_path_hardware) {
        if (0 == JobReusageTest::num_test++) {
            GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path";
        }
        return;
    }
    CompressWithJobReusage(false, qpl_high_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(job_reusage, compression_by_chunks_high_level, JobReusageTest) {
    if (GetExecutionPath() == qpl_path_hardware) {
        if (0 == JobReusageTest::num_test++) {
            GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path";
        }
        return;
    }
    CompressWithJobReusage(true, qpl_high_level);
}

} // namespace qpl::test
