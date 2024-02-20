/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <algorithm>
#include <array>

#include "compression_huffman_table.hpp"
#include "ta_ll_common.hpp"

// tests_common
#include "execution_wrapper.hpp"
#include "operation_test.hpp"

#include "util.hpp"
#include "source_provider.hpp"
#include "huffman_table_unique.hpp"

namespace qpl::test {
enum HeaderType {
    no_header = 0,
    gzip_header = QPL_FLAG_GZIP_MODE,
    zlib_header = QPL_FLAG_ZLIB_MODE
};

struct DeflateTestCase {
    HeaderType  header;
    std::string file_name;
};

std::ostream &operator<<(std::ostream &os, const DeflateTestCase &test_case) {
    std::string header;

    if (no_header == test_case.header) {
        header = "No header";
    } else if (gzip_header == test_case.header) {
        header = "Gzip header";
    } else if (zlib_header == test_case.header) {
        header = "Zlib header";
    }

    os << "Header type: " << header << ", file name: " << test_case.file_name << "\n";
    return os;
}

class DeflateTest : public JobFixtureWithTestCases<DeflateTestCase> {

public:
    int32_t num_test = 0;

protected:
    void InitializeTestCases() override {
        auto dataset = util::TestEnvironment::GetInstance().GetAlgorithmicDataset();
        for (auto &data: dataset.get_data()) {
            DeflateTestCase test_case{};
            test_case.file_name = data.first;
            test_case.header    = no_header;
            AddNewTestCase(test_case);

            test_case.header = gzip_header;
            AddNewTestCase(test_case);

            test_case.header = zlib_header;
            AddNewTestCase(test_case);
        }
    }

    void SetUpBeforeIteration() override {
        current_test_case = GetTestCase();
        source            = util::TestEnvironment::GetInstance().GetAlgorithmicDataset()[current_test_case.file_name];

        destination.resize(source.size() * 2);

        job_ptr->flags = current_test_case.header;
        job_ptr->op    = qpl_op_compress;
    }

    void CompressDynamicMode(qpl_compression_levels level, bool omit_verification = true) {
        job_ptr->flags |= QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DYNAMIC_HUFFMAN;
        job_ptr->flags |= (omit_verification) ? QPL_FLAG_OMIT_VERIFY : QPL_FLAG_CRC32C;
        job_ptr->available_in  = static_cast<uint32_t>(source.size());
        job_ptr->next_in_ptr   = source.data();
        job_ptr->available_out = static_cast<uint32_t>(destination.size());
        job_ptr->next_out_ptr  = destination.data();
        job_ptr->level         = level;

        std::vector<uint64_t> indexes;

        indexes.resize(source.size());
        std::fill(indexes.begin(), indexes.end(), 0u);
        job_ptr->idx_array    = (uint64_t *) indexes.data();
        job_ptr->idx_max_size = static_cast<uint32_t>(indexes.size());

        auto status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        if (no_header == current_test_case.header) {
            ASSERT_TRUE(CheckIfBlockDynamic(destination));
        }

        destination.resize(job_ptr->total_out);

        std::vector<uint8_t> decompressed_source(source.size());

        DecompressStream(destination, decompressed_source, true);

        ASSERT_TRUE(CompareVectors(decompressed_source, source));
    }

    void CompressStaticMode(qpl_compression_levels level, bool omit_verification = true) {
        // Create and initialize compression table
        unique_huffman_table c_table(deflate_huffman_table_maker(compression_table_type,
                                                                 GetExecutionPath(),
                                                                 DEFAULT_ALLOCATOR_C),
                                     any_huffman_table_deleter);
        ASSERT_NE(c_table.get(), nullptr) << "Compression Huffman Table creation failed\n";

        qpl_status status = fill_compression_table(c_table.get());
        ASSERT_EQ(status, QPL_STS_OK) << "Compression table failed to be filled";

        job_ptr->huffman_table = c_table.get();
        job_ptr->flags        |= QPL_FLAG_FIRST | QPL_FLAG_LAST;
        job_ptr->flags        |= (omit_verification) ? QPL_FLAG_OMIT_VERIFY : QPL_FLAG_CRC32C;
        job_ptr->level         = level;
        job_ptr->available_in  = static_cast<uint32_t>(source.size());
        job_ptr->next_in_ptr   = source.data();
        job_ptr->available_out = static_cast<uint32_t>(destination.size());
        job_ptr->next_out_ptr  = destination.data();

        std::vector<uint64_t> indexes;

        indexes.resize(source.size());
        std::fill(indexes.begin(), indexes.end(), 0u);
        job_ptr->idx_array    = (uint64_t *) indexes.data();
        job_ptr->idx_max_size = static_cast<uint32_t>(indexes.size());

        status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        destination.resize(job_ptr->total_out);

        std::vector<uint8_t> decompressed_source(source.size());

        DecompressStream(destination, decompressed_source);

        ASSERT_TRUE(CompareVectors(decompressed_source, source));
    }

    void CompressFixedMode(qpl_compression_levels level, bool omit_verification = true) {
        job_ptr->huffman_table = nullptr;
        job_ptr->flags |= QPL_FLAG_FIRST | QPL_FLAG_LAST;
        job_ptr->flags |= (omit_verification) ? QPL_FLAG_OMIT_VERIFY : QPL_FLAG_CRC32C;
        job_ptr->available_in  = static_cast<uint32_t>(source.size());
        job_ptr->next_in_ptr   = source.data();
        job_ptr->available_out = static_cast<uint32_t>(destination.size());
        job_ptr->next_out_ptr  = destination.data();
        job_ptr->level         = level;

        std::vector<uint64_t> indexes;

        indexes.resize(source.size());
        std::fill(indexes.begin(), indexes.end(), 0u);
        job_ptr->idx_array    = (uint64_t *) indexes.data();
        job_ptr->idx_max_size = static_cast<uint32_t>(indexes.size());

        auto status = run_job_api(job_ptr);

        ASSERT_EQ(QPL_STS_OK, status);

        destination.resize(job_ptr->total_out);

        std::vector<uint8_t> decompressed_source(source.size());

        DecompressStream(destination, decompressed_source);

        ASSERT_TRUE(CompareVectors(decompressed_source, source));
    }

private:
    void DecompressStream(std::vector<uint8_t> &compressed_source,
                          std::vector<uint8_t> &decompressed_destination,
                          bool use_dynamic_flag = false) {
        uint32_t job_size = 0;

        auto status = qpl_get_job_size(GetExecutionPath(), &job_size);
        ASSERT_EQ(QPL_STS_OK, status);

        auto job_buffer      = std::make_unique<uint8_t[]>(job_size);
        auto inflate_job_ptr = reinterpret_cast<qpl_job *>(job_buffer.get());

        status = qpl_init_job(GetExecutionPath(), inflate_job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        inflate_job_ptr->op    = qpl_op_decompress;
        inflate_job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST;
        inflate_job_ptr->flags |= current_test_case.header;

        inflate_job_ptr->next_in_ptr   = compressed_source.data();
        inflate_job_ptr->available_in  = static_cast<uint32_t>(compressed_source.size());
        inflate_job_ptr->next_out_ptr  = decompressed_destination.data();
        inflate_job_ptr->available_out = static_cast<uint32_t>(decompressed_destination.size());

        auto inflate_status = run_job_api(inflate_job_ptr);

        ASSERT_EQ(QPL_STS_OK, inflate_status);

        decompressed_destination.resize(inflate_job_ptr->total_out);

        status = qpl_fini_job(inflate_job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);
    }

    static testing::AssertionResult CheckIfBlockDynamic(std::vector<uint8_t> &buffer) {
        if (buffer.empty()) {
            return testing::AssertionFailure() << "Given buffer is empty";
        }

        if ((((buffer)[0] & 2u) != 0u) || (((buffer)[0] & 4u) != 4u)) {
            return testing::AssertionFailure() << "Produced block is not dynamic";
        }

        return testing::AssertionSuccess();
    }

    DeflateTestCase current_test_case{};
};

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(deflate, dynamic_blocks_high_level, DeflateTest) {
    if (GetExecutionPath() == qpl_path_hardware) {
        if (0 == DeflateTest::num_test++) {
            GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path";
        }
        return;
    }
    CompressDynamicMode(qpl_high_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(deflate, dynamic_blocks_default_level, DeflateTest) {
    CompressDynamicMode(qpl_default_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(deflate, static_blocks_high_level, DeflateTest) {
    if (GetExecutionPath() == qpl_path_hardware) {
        if (0 == DeflateTest::num_test++) {
            GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path";
        }
        return;
    }
    CompressStaticMode(qpl_high_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(deflate, static_blocks_default_level, DeflateTest) {
    CompressStaticMode(qpl_default_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(deflate, fixed_blocks_high_level, DeflateTest) {
    if (GetExecutionPath() == qpl_path_hardware) {
        if (0 == DeflateTest::num_test++) {
            GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path";
        }
        return;
    }
    CompressFixedMode(qpl_high_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(deflate, fixed_blocks_default_level, DeflateTest) {
    CompressFixedMode(qpl_default_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(deflate_index, dynamic_blocks_high_level, DeflateTest) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_hardware, "Resource management mistake when HW test (replaced by test deflate_high.dynamic_verify");

    job_ptr->mini_block_size = qpl_mblk_size_512;
    CompressDynamicMode(qpl_high_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(deflate_index, static_blocks_high_level, DeflateTest) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_hardware, "Resource management mistake when HW test (replaced by test deflate_high.dynamic_verify");

    job_ptr->mini_block_size = qpl_mblk_size_512;
    CompressStaticMode(qpl_high_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(deflate_index, dynamic_blocks_high_level_verify, DeflateTest) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_hardware, "Resource management mistake when HW test (replaced by test deflate_high.dynamic_verify");

    job_ptr->mini_block_size = qpl_mblk_size_512;
    bool omit_verification = false;
    CompressDynamicMode(qpl_high_level, omit_verification);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(deflate_index, static_blocks_high_level_verify, DeflateTest) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_hardware, "Resource management mistake when HW test (replaced by test deflate_high.dynamic_verify");

    job_ptr->mini_block_size = qpl_mblk_size_512;
    bool omit_verification = false;
    CompressStaticMode(qpl_high_level, omit_verification);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(deflate_index, dynamic_blocks_default_level_verify, DeflateTest) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_hardware, "Resource management mistake when HW test (replaced by test deflate_high.dynamic_verify");

    job_ptr->mini_block_size = qpl_mblk_size_512;
    bool omit_verification = false;
    CompressDynamicMode(qpl_default_level, omit_verification);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(deflate_index, static_blocks_default_level_verify, DeflateTest) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_hardware, "Resource management mistake when HW test (replaced by test deflate_high.dynamic_verify");

    job_ptr->mini_block_size = qpl_mblk_size_512;
    bool omit_verification = false;
    CompressStaticMode(qpl_default_level, omit_verification);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(deflate_verify, dynamic_high_level, DeflateTest) {
    if (GetExecutionPath() == qpl_path_hardware) {
        if (0 == DeflateTest::num_test++) {
            GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path";
        }
        return;
    }

    uint32_t job_size = 0;
    auto     path     = util::TestEnvironment::GetInstance().GetExecutionPath();

    qpl_get_job_size(path, &job_size);

    auto compression_job   = std::make_unique<uint8_t[]>(job_size);
    auto decompression_job = std::make_unique<uint8_t[]>(job_size);

    auto *compr_job   = reinterpret_cast<qpl_job *>(compression_job.get());
    auto *decompr_job = reinterpret_cast<qpl_job *>(decompression_job.get());

    qpl_init_job(path, compr_job);
    qpl_init_job(path, decompr_job);

    for (auto &dataset: util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
        std::cout << dataset.first << std::endl;

        for (uint32_t header: {0u, QPL_FLAG_GZIP_MODE, QPL_FLAG_ZLIB_MODE}) {
            auto                  source = dataset.second;
            std::vector<uint8_t>  destination(source.size(), 0);
            std::vector<uint8_t>  reference(source.size(), 0);
            std::vector<uint64_t> indexes(source.size(), 0);

            // Compress
            compr_job->next_in_ptr     = source.data();
            compr_job->available_in    = static_cast<uint32_t>(source.size());
            compr_job->next_out_ptr    = destination.data();
            compr_job->available_out   = static_cast<uint32_t>(destination.size());
            compr_job->op              = qpl_op_compress;
            compr_job->flags           = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DYNAMIC_HUFFMAN | header;
            compr_job->level           = qpl_high_level;
            compr_job->mini_block_size = qpl_mblk_size_512;
            compr_job->idx_array       = (uint64_t *) indexes.data();
            compr_job->idx_max_size    = static_cast<uint32_t>(indexes.size());

            auto status = run_job_api(compr_job);
            ASSERT_EQ(status, QPL_STS_OK);

            // Decompress
            decompr_job->next_in_ptr   = destination.data();
            decompr_job->available_in  = compr_job->total_out;
            decompr_job->next_out_ptr  = reference.data();
            decompr_job->available_out = static_cast<uint32_t>(reference.size());
            decompr_job->op            = qpl_op_decompress;
            decompr_job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | header;

            status = run_job_api(decompr_job);
            ASSERT_EQ(status, QPL_STS_OK);

            // Check
            ASSERT_TRUE(std::equal(source.begin(), source.end(), reference.begin()));
        }
    }

    qpl_fini_job(compr_job);
    qpl_fini_job(decompr_job);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(deflate_verify, static_high_level, DeflateTest) {

    if (GetExecutionPath() == qpl_path_hardware) {
        if (0 == DeflateTest::num_test++) {
            GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path";
        }
        return;
    }

    auto          dataset      = util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data();
    auto          table_source = dataset.begin()->second;
    uint32_t      job_size     = 0;
    auto          path         = util::TestEnvironment::GetInstance().GetExecutionPath();
    qpl_histogram deflate_histogram{};

    // Create and initialize compression table
    unique_huffman_table c_table(deflate_huffman_table_maker(compression_table_type,
                                                             path,
                                                             DEFAULT_ALLOCATOR_C),
                                 any_huffman_table_deleter);
    ASSERT_NE(c_table.get(), nullptr) << "Compression Huffman Table creation failed\n";

    // Build the table
    qpl_status status = qpl_gather_deflate_statistics(table_source.data(),
                                                      static_cast<uint32_t>(table_source.size()),
                                                      &deflate_histogram,
                                                      qpl_high_level,
                                                      path);
    ASSERT_EQ(status, QPL_STS_OK) << "Statistics gathering failed";

    status = qpl_huffman_table_init_with_histogram(c_table.get(), &deflate_histogram);
    ASSERT_EQ(status, QPL_STS_OK) << "Table build failed";

    // Perform test
    qpl_get_job_size(path, &job_size);

    auto compression_job   = std::make_unique<uint8_t[]>(job_size);
    auto decompression_job = std::make_unique<uint8_t[]>(job_size);

    auto *compr_job   = reinterpret_cast<qpl_job *>(compression_job.get());
    auto *decompr_job = reinterpret_cast<qpl_job *>(decompression_job.get());

    qpl_init_job(path, compr_job);
    qpl_init_job(path, decompr_job);

    for (auto &data: util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
        std::cout << data.first << std::endl;

        for (uint32_t header: {0u, QPL_FLAG_GZIP_MODE, QPL_FLAG_ZLIB_MODE}) {
            auto                  source = data.second;
            std::vector<uint8_t>  destination(source.size() * 2, 0);
            std::vector<uint8_t>  reference(source.size(), 0);
            std::vector<uint64_t> indexes(source.size(), 0);

            // Compress
            compr_job->next_in_ptr     = source.data();
            compr_job->available_in    = static_cast<uint32_t>(source.size());
            compr_job->next_out_ptr    = destination.data();
            compr_job->available_out   = static_cast<uint32_t>(destination.size());
            compr_job->op              = qpl_op_compress;
            compr_job->flags           = QPL_FLAG_FIRST | QPL_FLAG_LAST | header;
            compr_job->level           = qpl_high_level;
            compr_job->huffman_table   = c_table.get();
            compr_job->mini_block_size = qpl_mblk_size_512;
            compr_job->idx_array       = (uint64_t *) indexes.data();
            compr_job->idx_max_size    = static_cast<uint32_t>(indexes.size());

            status = run_job_api(compr_job);
            ASSERT_EQ(status, QPL_STS_OK) << "Compression failed with status: " << status << " source: " << data.first;

            // Decompress
            decompr_job->next_in_ptr   = destination.data();
            decompr_job->available_in  = compr_job->total_out;
            decompr_job->next_out_ptr  = reference.data();
            decompr_job->available_out = static_cast<uint32_t>(reference.size());
            decompr_job->op            = qpl_op_decompress;
            decompr_job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | header;

            status = run_job_api(decompr_job);
            ASSERT_EQ(status, QPL_STS_OK)
                                        << "Decompression failed with status: " << status << " source: " << data.first;

            // Check
            bool source_matches_reference = std::equal(source.begin(), source.end(), reference.begin());
            ASSERT_TRUE(source_matches_reference);
        }
    }

    // Free resources
    qpl_fini_job(compr_job);
    qpl_fini_job(decompr_job);
}

}
