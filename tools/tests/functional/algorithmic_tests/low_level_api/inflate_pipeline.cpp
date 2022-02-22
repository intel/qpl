/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "test_cases.hpp"
#include "operation_test.hpp"

#include <memory>
#include "ta_ll_common.hpp"
#include "random_generator.h"
#include "source_provider.hpp"

namespace qpl::test
{

    class InflatePipelineTest : public JobFixtureWithTestCases<std::string>
    {
    public:
        void SetUpBeforeIteration() override
        {
            if (!deflate_job_ptr)
            {
                uint32_t job_size = 0;
                auto     status   = qpl_get_job_size(GetExecutionPath(), &job_size);
                ASSERT_EQ(QPL_STS_OK, status);

                job_buffer      = std::make_unique<uint8_t[]>(job_size);
                deflate_job_ptr = reinterpret_cast<qpl_job *>(job_buffer.get());
            }

            auto dataset = util::TestEnvironment::GetInstance().GetAlgorithmicDataset();

            source = dataset[GetTestCase()];
            compressed_source.resize(source.size() * 3, 0u);
            destination.resize(source.size(), 0u);

            auto status = qpl_init_job(GetExecutionPath(), deflate_job_ptr);
            ASSERT_EQ(QPL_STS_OK, status);

            deflate_job_ptr->op = qpl_op_compress;
            job_ptr->op         = qpl_op_decompress;
        }

        void InitializeTestCases() override
        {
            for (auto &dataset: util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data())
            {
                AddNewTestCase(dataset.first);
            }
        }

    protected:
        testing::AssertionResult EndOfBlockStreamManipulationTest(qpl_compression_levels level)
        {
            auto source_size = static_cast<uint32_t>(source.size());
            qpl::test::random random(0u, 0u, GetSeed());

            random.set_range(1u, 5000u);
            auto block_size = static_cast<uint32_t>(random);

            uint32_t expected_block_count = source_size / block_size;
            expected_block_count += (0u == source_size % block_size) ? 0 : 1;

            auto deflate_status = DeflateStreamByBlocks(source,
                                                        compressed_source,
                                                        block_size,
                                                        level);

            compressed_source.resize(deflate_job_ptr->total_out);

            EXPECT_TRUE(deflate_status);

            // Try any EOB
            job_ptr->next_in_ptr              = compressed_source.data();
            job_ptr->available_in             = static_cast<uint32_t>(compressed_source.size());
            job_ptr->next_out_ptr             = destination.data();
            job_ptr->available_out            = static_cast<uint32_t>(destination.size());
            job_ptr->flags               = QPL_FLAG_FIRST | QPL_FLAG_DECOMP_FLUSH_ALWAYS;
            job_ptr->decomp_end_processing = qpl_stop_on_any_eob;

            auto status = run_job_api(job_ptr);
            EXPECT_EQ(QPL_STS_OK, status);

            // Check expected block position
            EXPECT_EQ(job_ptr->total_out, block_size);

            EXPECT_TRUE(CompareVectors(source, destination, block_size));

            // Reset total out field before the next test
            job_ptr->total_out = 0u;

            // Try final EOB
            job_ptr->next_in_ptr              = compressed_source.data();
            job_ptr->available_in             = static_cast<uint32_t>(compressed_source.size());
            job_ptr->next_out_ptr             = destination.data();
            job_ptr->available_out            = static_cast<uint32_t>(destination.size());
            job_ptr->flags               = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DECOMP_FLUSH_ALWAYS;
            job_ptr->decomp_end_processing = qpl_check_for_bfinal_eob;

            status = run_job_api(job_ptr);
            EXPECT_EQ(QPL_STS_OK, status);

            // Check expected block position
            EXPECT_EQ(job_ptr->total_out, source_size);

            return CompareVectors(source, destination, source_size);
        }

        std::unique_ptr<uint8_t[]> job_buffer;
        qpl_job                     *deflate_job_ptr = nullptr;
        std::vector<uint8_t>       compressed_source;

    private:
        testing::AssertionResult DeflateStreamByBlocks(std::vector<uint8_t> &source_stream,
                                                       std::vector<uint8_t> &compressed_destination,
                                                       uint32_t block_size,
                                                       qpl_compression_levels level)
        {
            auto source_size = static_cast<uint32_t>(source_stream.size());

            deflate_job_ptr->next_in_ptr   = source_stream.data();
            deflate_job_ptr->next_out_ptr  = compressed_destination.data();
            deflate_job_ptr->available_out = static_cast<uint32_t>(compressed_destination.size());
            deflate_job_ptr->level = level;

            deflate_job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_DYNAMIC_HUFFMAN | QPL_FLAG_OMIT_VERIFY;

            uint32_t blocks = 0;
            while (deflate_job_ptr->total_in < source_size)
            {
                blocks++;
                if (deflate_job_ptr->total_in + block_size >= source_size)
                {
                    deflate_job_ptr->available_in = source_size - deflate_job_ptr->total_in;
                    deflate_job_ptr->flags |= QPL_FLAG_LAST;
                }
                else
                {
                    deflate_job_ptr->available_in += block_size;
                }

                auto status = run_job_api(deflate_job_ptr);
                EXPECT_EQ(QPL_STS_OK, status);

                if (QPL_STS_OK != status) {
                    break;
                }

                deflate_job_ptr->flags |= QPL_FLAG_START_NEW_BLOCK;
                deflate_job_ptr->flags &= ~QPL_FLAG_FIRST;
            }

            return testing::AssertionSuccess();
        }
    };

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(inflate_chain_mode, process_by_chunks, InflatePipelineTest)
    {
        const uint32_t max_chunk_size = 1024;

        auto file_size = static_cast<uint32_t>(source.size());

        qpl::test::random chunk_size(30, max_chunk_size, GetSeed());

        deflate_job_ptr->flags    = (QPL_FLAG_FIRST | QPL_FLAG_OMIT_VERIFY | QPL_FLAG_DYNAMIC_HUFFMAN);
        deflate_job_ptr->next_in_ptr   = source.data();
        deflate_job_ptr->next_out_ptr  = compressed_source.data();
        deflate_job_ptr->available_out = static_cast<uint32_t>(compressed_source.size());
        deflate_job_ptr->level = qpl_default_level;

        auto next_in_chunk = (uint32_t) chunk_size;

        while (deflate_job_ptr->total_in < file_size)
        {
            if ((deflate_job_ptr->total_in + next_in_chunk) >= file_size)
            {
                next_in_chunk = file_size - deflate_job_ptr->total_in;
                deflate_job_ptr->flags |= QPL_FLAG_LAST;
            }

            deflate_job_ptr->available_in = next_in_chunk;

            auto deflate_status = run_job_api(deflate_job_ptr);
            ASSERT_EQ(QPL_STS_OK, deflate_status);

            deflate_job_ptr->flags &= ~QPL_FLAG_FIRST; // Reset first flag
        }

        job_ptr->next_in_ptr   = compressed_source.data();
        job_ptr->next_out_ptr  = destination.data();
        job_ptr->available_out = static_cast<uint32_t>(destination.size());
        job_ptr->flags    = QPL_FLAG_FIRST;

        next_in_chunk = (uint32_t) chunk_size;

        while (job_ptr->total_in < deflate_job_ptr->total_out)
        {
            if ((job_ptr->total_in + next_in_chunk) >= deflate_job_ptr->total_out)
            {
                next_in_chunk = deflate_job_ptr->total_out - job_ptr->total_in;
                job_ptr->flags |= QPL_FLAG_LAST;
            }

            job_ptr->available_in = next_in_chunk;

            auto inflate_status = run_job_api(job_ptr);

            if (QPL_STS_MORE_OUTPUT_NEEDED == inflate_status && job_ptr->total_out >= file_size)
            {
                // There is a case when Inflate doesn't set available_in with 0 but set total_out to expected final result (orig file's size)
                // and decompress buffer's content is also equal to original file, i.e. need to investigate this case deeply

                break;
            }
            else
            {
                ASSERT_EQ(QPL_STS_OK, inflate_status);
            }

            if (job_ptr->total_out >= file_size)
            {
                break;
            }

            job_ptr->flags &= ~QPL_FLAG_FIRST; // Reset FIRST flag
        }

        destination.resize(job_ptr->total_out);

        bool vectors_equality = source == destination;

        ASSERT_TRUE(vectors_equality);
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(inflate_chain_mode, process_byte_in_time, InflatePipelineTest)
    {
        auto file_size = static_cast<uint32_t>(source.size());

        deflate_job_ptr->flags    = (QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY);
        deflate_job_ptr->next_in_ptr   = source.data();
        deflate_job_ptr->available_in  = static_cast<uint32_t>(source.size());
        deflate_job_ptr->next_out_ptr  = compressed_source.data();
        deflate_job_ptr->available_out = static_cast<uint32_t>(compressed_source.size());
        deflate_job_ptr->level = qpl_default_level;

        auto status = run_job_api(deflate_job_ptr);

        ASSERT_EQ(QPL_STS_OK, status);

        job_ptr->next_in_ptr   = compressed_source.data();
        job_ptr->available_in  = static_cast<uint32_t>(compressed_source.size());
        job_ptr->next_out_ptr  = destination.data();
        job_ptr->available_out = static_cast<uint32_t>(destination.size());
        job_ptr->flags    = QPL_FLAG_FIRST;

        while (job_ptr->total_in < deflate_job_ptr->total_out)
        {
            job_ptr->available_in = 1; // Decompress a single byte

            if ((job_ptr->total_in + job_ptr->available_in) >= deflate_job_ptr->total_out)
            {
                job_ptr->flags |= QPL_FLAG_LAST;
            }

            status = run_job_api(job_ptr);

            if (QPL_STS_MORE_OUTPUT_NEEDED == status && job_ptr->total_out >= file_size)
            {
                // There is a case when Inflate doesn't set available_in with 0 but set total_out to expected final result (orig file's size)
                // and decompress buffer's content is also equal to original file, i.e. need to investigate this case deeply

                break;
            }
            else
            {
                ASSERT_EQ(QPL_STS_OK, status);
            }

            if (job_ptr->total_out >= file_size)
            {
                break;
            }

            job_ptr->flags &= ~QPL_FLAG_FIRST; // Reset FIRST flag
        }

        bool equal_vectors = source == destination;

        EXPECT_TRUE(equal_vectors);
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(inflate_chain_mode, end_of_block_stream_manipulation, InflatePipelineTest)
    {
        auto result = EndOfBlockStreamManipulationTest(qpl_default_level);

        ASSERT_TRUE(result);
    }
}
