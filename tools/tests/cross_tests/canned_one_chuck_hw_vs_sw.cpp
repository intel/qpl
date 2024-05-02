/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "base_cross_test_fixture.hpp"
#include "canned_test_cases.hpp"

// tool_common
#include "compression_huffman_table.hpp"

namespace qpl::test {

class SimpleCannedOneChuckCompressDecompressFixture : public BaseCrossTestFixture,
                                                      public TestCases<SimpleCannedOneChuckTestCase> {
private:
        std::vector<uint8_t>  reference_text;
        qpl_huffman_table_t   c_huffman_table;
        qpl_huffman_table_t   d_huffman_table;
        SimpleCannedOneChuckTestCase  current_test_case{};

protected:
    template <class Iterator>
    auto init_compression_huffman_table(qpl_huffman_table_t huffman_table,
                                        Iterator begin,
                                        Iterator end,
                                        qpl_compression_levels level,
                                        qpl_path_t path) -> void {
        auto           *source_ptr = &*begin;
        const uint32_t source_size = std::distance(begin, end);

        qpl_histogram deflate_histogram{};

        auto status = qpl_gather_deflate_statistics(source_ptr,
                                                    source_size,
                                                    &deflate_histogram,
                                                    level,
                                                    path);
        ASSERT_EQ(status, QPL_STS_OK) << "Failed to gather statistics";

        status = qpl_huffman_table_init_with_histogram(huffman_table, &deflate_histogram);
        ASSERT_EQ(status, QPL_STS_OK) << "Failed to build compression table";
    }

    void SetUp() override {
        BaseCrossTestFixture::SetUp();
        this->InitializeTestCases();

        auto status = qpl_deflate_huffman_table_create(compression_table_type,
                                                       qpl_path_auto,
                                                       DEFAULT_ALLOCATOR_C,
                                                       &c_huffman_table);
        if (status != QPL_STS_OK) {
            throw std::runtime_error("Compression table creation failed.");
        }

        //status = fill_compression_table(c_huffman_table);
        //if (status != QPL_STS_OK) {
        //    throw std::runtime_error("Compression table failed to be filled.");
        //}

        status = qpl_deflate_huffman_table_create(decompression_table_type,
                                                  qpl_path_auto,
                                                  DEFAULT_ALLOCATOR_C,
                                                  &d_huffman_table);
        if (status != QPL_STS_OK) {
            throw std::runtime_error("Decompression table creation failed.");
        }
    }

    ~SimpleCannedOneChuckCompressDecompressFixture() {
        if (c_huffman_table != NULL)
            qpl_huffman_table_destroy(c_huffman_table);

        if (d_huffman_table != NULL)
            qpl_huffman_table_destroy(d_huffman_table);
    }

    void InitializeTestCases() {
        for (auto &dataset: util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
            SimpleCannedOneChuckTestCase test_case{};
            test_case.file_name = dataset.first;
            AddNewTestCase(test_case);
        }
    }

public:
    testing::AssertionResult ValidateCompressSwDecompressHw(qpl_compression_levels level) {
        init_compression_huffman_table(c_huffman_table,
                                       reference_text.data(),
                                       reference_text.data() + static_cast<uint32_t>(reference_text.size()),
                                       level,
                                       qpl_path_software);

        sw_job_ptr->op = qpl_op_compress;
        sw_job_ptr->level = level;
        sw_job_ptr->next_in_ptr = reference_text.data();
        sw_job_ptr->available_in  = static_cast<uint32_t>(reference_text.size());
        sw_job_ptr->next_out_ptr  = sw_destination.data();
        sw_job_ptr->available_out = static_cast<uint32_t>(sw_destination.size());

        sw_job_ptr->huffman_table = c_huffman_table;
        sw_job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY | QPL_FLAG_CANNED_MODE;

        auto status = run_job_api(sw_job_ptr);
        ASSERT_ERR_STATUS(status) << "Compression status: " << status;

        hw_job_ptr->op = qpl_op_decompress;
        hw_job_ptr->next_in_ptr = sw_destination.data();
        hw_job_ptr->available_in  = sw_job_ptr->total_out;
        hw_job_ptr->next_out_ptr  = hw_destination.data();
        hw_job_ptr->available_out = static_cast<uint32_t>(hw_destination.size());

        status = qpl_huffman_table_init_with_other(d_huffman_table, c_huffman_table);
        ASSERT_ERR_STATUS(status) << "Decompression table creation status: " << status;

        hw_job_ptr->huffman_table = d_huffman_table;
        hw_job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE;

        status = run_job_api(hw_job_ptr);
        ASSERT_ERR_STATUS(status) << "Decompression status: " << status;

        hw_destination.resize(hw_job_ptr->total_out);

        bool vectors_is_equal = reference_text == hw_destination;

        if (!vectors_is_equal) {
            return testing::AssertionFailure() << "Decompressed data don't match with reference one";
        } else {
            return testing::AssertionSuccess();
        }

    }

    testing::AssertionResult ValidateCompressHwDecompressSw() {
        init_compression_huffman_table(c_huffman_table,
                                       reference_text.data(),
                                       reference_text.data() + static_cast<uint32_t>(reference_text.size()),
                                       qpl_default_level,
                                       qpl_path_hardware);

        hw_job_ptr->op = qpl_op_compress;
        hw_job_ptr->level = qpl_default_level;
        hw_job_ptr->next_in_ptr = reference_text.data();
        hw_job_ptr->available_in  = static_cast<uint32_t>(reference_text.size());
        hw_job_ptr->next_out_ptr  = hw_destination.data();
        hw_job_ptr->available_out = static_cast<uint32_t>(hw_destination.size());

        hw_job_ptr->huffman_table = c_huffman_table;
        hw_job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY | QPL_FLAG_CANNED_MODE;

        auto status = run_job_api(hw_job_ptr);
        ASSERT_ERR_STATUS(status) << "Compression status: " << status;

        sw_job_ptr->op = qpl_op_decompress;
        sw_job_ptr->next_in_ptr = hw_destination.data();
        sw_job_ptr->available_in  = hw_job_ptr->total_out;
        sw_job_ptr->next_out_ptr  = sw_destination.data();
        sw_job_ptr->available_out = static_cast<uint32_t>(sw_destination.size());

        status = qpl_huffman_table_init_with_other(d_huffman_table, c_huffman_table);
        ASSERT_ERR_STATUS(status) << "Decompression table creation status: " << status;

        sw_job_ptr->huffman_table = d_huffman_table;
        sw_job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE;

        status = run_job_api(sw_job_ptr);
        ASSERT_ERR_STATUS(status) << "Decompression status: " << status;

        sw_destination.resize(sw_job_ptr->total_out);

        bool vectors_is_equal = reference_text == sw_destination;

        if (!vectors_is_equal) {
            return testing::AssertionFailure() << "Decompressed data don't match with reference one";
        } else {
            return testing::AssertionSuccess();
        }

    }

    void SetUpBeforeIteration() override {
        current_test_case = GetTestCase();

        auto dataset = util::TestEnvironment::GetInstance().GetAlgorithmicDataset();

        reference_text = dataset[current_test_case.file_name];

        sw_destination.resize(reference_text.size() * 2U);
        hw_destination.resize(reference_text.size() * 2U);
    }
};

QPL_LOW_LEVEL_API_CROSS_TEST_TC(deflate_canned, SimpleCannedOneChuckCompressDecompressFixture, stateless_compress_sw_decompress_hw_default_level) {
    ASSERT_TRUE(ValidateCompressSwDecompressHw(qpl_default_level));
}

QPL_LOW_LEVEL_API_CROSS_TEST_TC(deflate_canned, SimpleCannedOneChuckCompressDecompressFixture, stateless_compress_sw_decompress_hw_high_level) {
    ASSERT_TRUE(ValidateCompressSwDecompressHw(qpl_high_level));
}

QPL_LOW_LEVEL_API_CROSS_TEST_TC(deflate_canned, SimpleCannedOneChuckCompressDecompressFixture, stateless_compress_hw_decompress_sw_default_level) {
    ASSERT_TRUE(ValidateCompressHwDecompressSw());
}
}
