/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "base_cross_test_fixture.hpp"
#include "deflate_test_cases.hpp"

// tool_common
#include "compression_huffman_table.hpp"

namespace qpl::test {
class SimpleCompressDecompressFixture : public BaseCrossTestFixture,
                                        public TestCases<SimpleDeflateTestCase> {
private:
    std::vector<uint8_t>  reference_text;
    qpl_huffman_table_t   huffman_table;
    SimpleDeflateTestCase current_test_case{};

protected:
    void SetUp() override {
        BaseCrossTestFixture::SetUp();
        this->InitializeTestCases();

        auto status = qpl_deflate_huffman_table_create(compression_table_type,
                                                       qpl_path_auto,
                                                       DEFAULT_ALLOCATOR_C,
                                                       &huffman_table);
        if (status != QPL_STS_OK) {
            throw std::runtime_error("An error acquired during table creation.");
        }

        status = fill_compression_table(huffman_table);
        if (status != QPL_STS_OK) {
            throw std::runtime_error("An error acquired when compression table is filled.");
        }

    }

    void InitializeTestCases() {

        for (auto &dataset: util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
            SimpleDeflateTestCase test_case{};
            test_case.file_name  = dataset.first;
            test_case.header     = no_header;
            test_case.block_type = block_dynamic;
            AddNewTestCase(test_case);

            test_case.block_type = block_fixed;
            AddNewTestCase(test_case);

            test_case.block_type = block_static;
            AddNewTestCase(test_case);

            test_case.header = gzip_header;
            AddNewTestCase(test_case);

            test_case.header = zlib_header;
            AddNewTestCase(test_case);
        }
    }

public:
    testing::AssertionResult ValidateCompressHwDecompressSw() {
        hw_job_ptr->op            = qpl_op_compress;
        hw_job_ptr->level         = qpl_default_level;
        hw_job_ptr->next_in_ptr   = reference_text.data();
        hw_job_ptr->available_in  = static_cast<uint32_t>(reference_text.size());
        hw_job_ptr->next_out_ptr  = hw_destination.data();
        hw_job_ptr->available_out = static_cast<uint32_t>(hw_destination.size());

        hw_job_ptr->flags |= QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY;

        auto status = qpl_execute_job(hw_job_ptr);
        ASSERT_ERR_STATUS(status) << "Compression status: " << status;

        sw_job_ptr->op            = qpl_op_decompress;
        sw_job_ptr->level         = qpl_default_level;
        sw_job_ptr->next_in_ptr   = hw_destination.data();
        sw_job_ptr->available_in  = hw_job_ptr->total_out;
        sw_job_ptr->next_out_ptr  = sw_destination.data();
        sw_job_ptr->available_out = static_cast<uint32_t>(sw_destination.size());

        sw_job_ptr->flags |= QPL_FLAG_FIRST | QPL_FLAG_LAST;

        status = qpl_execute_job(sw_job_ptr);
        ASSERT_ERR_STATUS(status) << "Decompression status: " << status;

        sw_destination.resize(sw_job_ptr->total_out);

        bool vectors_is_equal = reference_text == sw_destination;

        if (!vectors_is_equal) {
            return testing::AssertionFailure() << "Decompressed data don't match with reference one";
        } else {
            return testing::AssertionSuccess();
        }
    }

    testing::AssertionResult ValidateCompressSwDecompressHw(qpl_compression_levels level) {
        if (level == qpl_high_level && current_test_case.block_type == block_fixed) {
            return testing::AssertionSuccess() << "Is not supported feature: " << current_test_case;
        }

        sw_job_ptr->op            = qpl_op_compress;
        sw_job_ptr->level         = level;
        sw_job_ptr->next_in_ptr   = reference_text.data();
        sw_job_ptr->available_in  = static_cast<uint32_t>(reference_text.size());
        sw_job_ptr->next_out_ptr  = sw_destination.data();
        sw_job_ptr->available_out = static_cast<uint32_t>(sw_destination.size());

        sw_job_ptr->flags |= QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY;

        auto status = qpl_execute_job(sw_job_ptr);
        ASSERT_ERR_STATUS(status) << "Compression status: " << status;

        hw_job_ptr->op            = qpl_op_decompress;
        hw_job_ptr->next_in_ptr   = sw_destination.data();
        hw_job_ptr->available_in  = sw_job_ptr->total_out;
        hw_job_ptr->next_out_ptr  = hw_destination.data();
        hw_job_ptr->available_out = static_cast<uint32_t>(hw_destination.size());

        hw_job_ptr->flags |= QPL_FLAG_FIRST | QPL_FLAG_LAST;

        status = qpl_execute_job(hw_job_ptr);
        ASSERT_ERR_STATUS(status) << "Decompression status: " << status;

        hw_destination.resize(hw_job_ptr->total_out);

        bool vectors_is_equal = reference_text == hw_destination;

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

        sw_job_ptr->flags = current_test_case.header;
        hw_job_ptr->flags = current_test_case.header;

        if (current_test_case.block_type == block_static) {
            sw_job_ptr->huffman_table = huffman_table;
            hw_job_ptr->huffman_table = huffman_table;
        } else if (current_test_case.block_type == block_fixed) {
            sw_job_ptr->huffman_table = nullptr;
            hw_job_ptr->huffman_table = nullptr;
        } else {
            sw_job_ptr->flags |= QPL_FLAG_DYNAMIC_HUFFMAN;
            hw_job_ptr->flags |= QPL_FLAG_DYNAMIC_HUFFMAN;
        }
    }
};

QPL_LOW_LEVEL_API_CROSS_TEST_TC(deflate, SimpleCompressDecompressFixture, compress_sw_decompress_hw_high_level) {
    ASSERT_TRUE(ValidateCompressSwDecompressHw(qpl_high_level));
}

QPL_LOW_LEVEL_API_CROSS_TEST_TC(deflate, SimpleCompressDecompressFixture, compress_sw_decompress_hw_default_level) {
    ASSERT_TRUE(ValidateCompressSwDecompressHw(qpl_default_level));
}

QPL_LOW_LEVEL_API_CROSS_TEST_TC(deflate, SimpleCompressDecompressFixture, compress_hw_decompress_sw) {
    ASSERT_TRUE(ValidateCompressHwDecompressSw());
}
}
