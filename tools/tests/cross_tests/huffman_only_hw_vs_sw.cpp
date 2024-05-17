/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "base_cross_test_fixture.hpp"
#include "huffman_only_test_cases.hpp"

// tool_common
#include "compression_huffman_table.hpp"
#include "iaa_features_checks.hpp"

namespace qpl::test {

class SimpleHuffmanOnlyCompressDecompressFixture : public BaseCrossTestFixture,
                                                   public TestCases<SimpleHuffmanOnlyTestCase> {
private:
    std::vector<uint8_t>      reference_text;
    qpl_huffman_table_t       c_huffman_table;
    qpl_huffman_table_t       d_huffman_table;
    SimpleHuffmanOnlyTestCase current_test_case{};

protected:
    void SetUp() override {
        BaseCrossTestFixture::SetUp();
        this->InitializeTestCases();
    }

    void InitializeTestCases() {

        for (auto &dataset: util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
            SimpleHuffmanOnlyTestCase test_case{};
            test_case.file_name  = dataset.first;
            test_case.is_huffman_be = false;
            AddNewTestCase(test_case);

            test_case.is_huffman_be = true;
            AddNewTestCase(test_case);
        }
    }

public:
    SimpleHuffmanOnlyCompressDecompressFixture(){};
    SimpleHuffmanOnlyCompressDecompressFixture(const SimpleHuffmanOnlyCompressDecompressFixture &) = delete;
    SimpleHuffmanOnlyCompressDecompressFixture(const SimpleHuffmanOnlyCompressDecompressFixture &&) = delete;
    SimpleHuffmanOnlyCompressDecompressFixture & operator=(const SimpleHuffmanOnlyCompressDecompressFixture &) = delete;
    SimpleHuffmanOnlyCompressDecompressFixture & operator=(const SimpleHuffmanOnlyCompressDecompressFixture &&) = delete;

    ~SimpleHuffmanOnlyCompressDecompressFixture() override {
        if (c_huffman_table != NULL)
            qpl_huffman_table_destroy(c_huffman_table);
        if (d_huffman_table != NULL)
            qpl_huffman_table_destroy(d_huffman_table);
    }

    testing::AssertionResult ValidateCompressHwDecompressSw() {
        // Create Huffman Table objects for compression and decompression
        auto status = qpl_huffman_only_table_create(compression_table_type,
                                                    qpl_path_auto,
                                                    DEFAULT_ALLOCATOR_C,
                                                    &c_huffman_table);
        if (status != QPL_STS_OK) {
            throw std::runtime_error("An error acquired during table creation.");
        }

        status = qpl_huffman_only_table_create(decompression_table_type,
                                               qpl_path_auto,
                                               DEFAULT_ALLOCATOR_C,
                                               &d_huffman_table);
        if (status != QPL_STS_OK) {
            throw std::runtime_error("An error acquired during table creation.");
        }

        hw_job_ptr->op            = qpl_op_compress;
        hw_job_ptr->level         = qpl_default_level;
        hw_job_ptr->next_in_ptr   = reference_text.data();
        hw_job_ptr->available_in  = static_cast<uint32_t>(reference_text.size());
        hw_job_ptr->next_out_ptr  = hw_destination.data();
        hw_job_ptr->available_out = static_cast<uint32_t>(hw_destination.size());

        hw_job_ptr->huffman_table = c_huffman_table;

        hw_job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_GEN_LITERALS | QPL_FLAG_OMIT_VERIFY | QPL_FLAG_NO_HDRS | QPL_FLAG_DYNAMIC_HUFFMAN;
        if (current_test_case.is_huffman_be)
            hw_job_ptr->flags |= QPL_FLAG_HUFFMAN_BE;

        status = run_job_api(hw_job_ptr);
        ASSERT_ERR_STATUS(status) << "Compression status: " << status;

        sw_job_ptr->op            = qpl_op_decompress;
        sw_job_ptr->level         = qpl_default_level;
        sw_job_ptr->next_in_ptr   = hw_destination.data();
        sw_job_ptr->available_in  = hw_job_ptr->total_out;
        sw_job_ptr->next_out_ptr  = sw_destination.data();
        sw_job_ptr->available_out = static_cast<uint32_t>(sw_destination.size());

        if (current_test_case.is_huffman_be) {
            sw_job_ptr->ignore_end_bits = (16 - hw_job_ptr->last_bit_offset) & 15;
        } else {
            sw_job_ptr->ignore_end_bits = (8 - hw_job_ptr->last_bit_offset) & 7;
        }

        status = qpl_huffman_table_init_with_other(d_huffman_table, c_huffman_table);
        ASSERT_ERR_STATUS(status) << "qpl_huffman_table_init_with_other status: " << status;

        sw_job_ptr->huffman_table = d_huffman_table;

        sw_job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_NO_HDRS | QPL_FLAG_DYNAMIC_HUFFMAN;
        if (current_test_case.is_huffman_be)
            sw_job_ptr->flags |= QPL_FLAG_HUFFMAN_BE;

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

    testing::AssertionResult ValidateCompressSwDecompressHw(qpl_compression_levels level) {
        // Create Huffman Table objects for compression and decompression
        auto status = qpl_huffman_only_table_create(compression_table_type,
                                                    qpl_path_auto,
                                                    DEFAULT_ALLOCATOR_C,
                                                    &c_huffman_table);
        if (status != QPL_STS_OK) {
            throw std::runtime_error("An error acquired during table creation.");
        }

        status = qpl_huffman_only_table_create(decompression_table_type,
                                               qpl_path_auto,
                                               DEFAULT_ALLOCATOR_C,
                                               &d_huffman_table);
        if (status != QPL_STS_OK) {
            throw std::runtime_error("An error acquired during table creation.");
        }

        sw_job_ptr->op            = qpl_op_compress;
        sw_job_ptr->level         = level;
        sw_job_ptr->next_in_ptr   = reference_text.data();
        sw_job_ptr->available_in  = static_cast<uint32_t>(reference_text.size());
        sw_job_ptr->next_out_ptr  = sw_destination.data();
        sw_job_ptr->available_out = static_cast<uint32_t>(sw_destination.size());

        sw_job_ptr->huffman_table = c_huffman_table;

        sw_job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_GEN_LITERALS | QPL_FLAG_OMIT_VERIFY | QPL_FLAG_NO_HDRS | QPL_FLAG_DYNAMIC_HUFFMAN;
        if (current_test_case.is_huffman_be)
            sw_job_ptr->flags |= QPL_FLAG_HUFFMAN_BE;

        status = run_job_api(sw_job_ptr);
        ASSERT_ERR_STATUS(status) << "Compression status: " << status;

        hw_job_ptr->op            = qpl_op_decompress;
        hw_job_ptr->next_in_ptr   = sw_destination.data();
        hw_job_ptr->available_in  = sw_job_ptr->total_out;
        hw_job_ptr->next_out_ptr  = hw_destination.data();
        hw_job_ptr->available_out = static_cast<uint32_t>(hw_destination.size());

        if (current_test_case.is_huffman_be) {
            hw_job_ptr->ignore_end_bits = (16 - sw_job_ptr->last_bit_offset) & 15;
        } else {
            hw_job_ptr->ignore_end_bits = (8 - sw_job_ptr->last_bit_offset) & 7;
        }

        status = qpl_huffman_table_init_with_other(d_huffman_table, c_huffman_table);
        ASSERT_ERR_STATUS(status) << "qpl_huffman_table_init_with_other status: " << status;

        hw_job_ptr->huffman_table = d_huffman_table;

        hw_job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_NO_HDRS | QPL_FLAG_DYNAMIC_HUFFMAN;
        if (current_test_case.is_huffman_be)
            hw_job_ptr->flags |= QPL_FLAG_HUFFMAN_BE;

        status = run_job_api(hw_job_ptr);

        // IAA 1.0 limitation: cannot work if ignore_end_bits is greater than 7 bits for BE16 decompress.
        // Expect error in this case.
        // If IAA Gen 2 minimum capabilities are present, Ignore End Bits Extension is supported and thus
        // this limitation will not apply.
        if (QPL_STS_HUFFMAN_BE_IGNORE_MORE_THAN_7_BITS_ERR == status && current_test_case.is_huffman_be &&
            hw_job_ptr->ignore_end_bits > 7 && !are_iaa_gen_2_min_capabilities_present()) {
            return testing::AssertionSuccess();
        }

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
    }
};

QPL_LOW_LEVEL_API_CROSS_TEST_TC(huffman, SimpleHuffmanOnlyCompressDecompressFixture, compress_sw_decompress_hw_high_level) {
    ASSERT_TRUE(ValidateCompressSwDecompressHw(qpl_high_level));
}

QPL_LOW_LEVEL_API_CROSS_TEST_TC(huffman, SimpleHuffmanOnlyCompressDecompressFixture, compress_sw_decompress_hw_default_level) {
    ASSERT_TRUE(ValidateCompressSwDecompressHw(qpl_default_level));
}

QPL_LOW_LEVEL_API_CROSS_TEST_TC(huffman, SimpleHuffmanOnlyCompressDecompressFixture, DISABLED_compress_hw_decompress_sw) {
    ASSERT_TRUE(ValidateCompressHwDecompressSw());
}
}
