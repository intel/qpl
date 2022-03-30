/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "base_cross_test_fixture.hpp"
#include "zero_test_cases.hpp"

namespace qpl::test {
    class SimpleZeroCompressDecompressFixture: public BaseCrossTestFixture,
                                       public TestCases<SimpleZeroTestCase> {
    protected:
        void SetUp() override {
            BaseCrossTestFixture::SetUp();
            this->InitializeTestCases();
        }

        void InitializeTestCases() {

            for (int size = 640; size <= 6400; size += 64)
            {
                SimpleZeroTestCase test_case{};
                test_case.ref_size = size;
                test_case.flag_crc = 0;
                AddNewTestCase(test_case);

                test_case.flag_crc = 1;
                AddNewTestCase(test_case);
            }
        }

        std::vector<uint8_t>  reference_text;
        SimpleZeroTestCase current_test_case {};

    public:
        testing::AssertionResult ValidateCompressHwDecompressSw(qpl_operation qpl_op_z_comp, qpl_operation qpl_op_z_decomp) {
            hw_job_ptr->op            = qpl_op_z_comp;
            hw_job_ptr->next_in_ptr   = reference_text.data();
            hw_job_ptr->available_in  = static_cast<uint32_t>(reference_text.size());
            hw_job_ptr->next_out_ptr  = hw_destination.data();
            hw_job_ptr->available_out = static_cast<uint32_t>(hw_destination.size());

            hw_job_ptr->flags |= QPL_FLAG_FIRST | QPL_FLAG_LAST;
            if (current_test_case.flag_crc)
                hw_job_ptr->flags |= QPL_FLAG_CRC32C;

            auto status = run_job_api(hw_job_ptr);
            ASSERT_ERR_STATUS(status) << "Compression status: " << status;

            sw_job_ptr->op            = qpl_op_z_decomp;
            sw_job_ptr->next_in_ptr   = hw_destination.data();
            sw_job_ptr->available_in  = hw_job_ptr->total_out;
            sw_job_ptr->next_out_ptr  = sw_destination.data();
            sw_job_ptr->available_out = static_cast<uint32_t>(sw_destination.size());

            sw_job_ptr->flags |= QPL_FLAG_FIRST | QPL_FLAG_LAST;

            status = run_job_api(sw_job_ptr);
            ASSERT_ERR_STATUS(status) << "Decompression status: " << status;

            sw_destination.resize(sw_job_ptr->total_out);

            bool vectors_is_equal = reference_text == sw_destination;

            bool crcs_is_equal = false;
            if (current_test_case.flag_crc) {
                crcs_is_equal = sw_job_ptr->crc == hw_job_ptr->crc;
            }

            if (!vectors_is_equal) {
                return testing::AssertionFailure() << "Decompressed data don't match with reference one";
            }
            else if (current_test_case.flag_crc && !crcs_is_equal) {
                return testing::AssertionFailure() << "Compression CRC don't match with deompression CRC";
            }
            else {
                return testing::AssertionSuccess();
            }
        }

        testing::AssertionResult ValidateCompressSwDecompressHw(qpl_operation qpl_op_z_comp, qpl_operation qpl_op_z_decomp) {
            sw_job_ptr->op            = qpl_op_z_comp;
            sw_job_ptr->next_in_ptr   = reference_text.data();
            sw_job_ptr->available_in  = static_cast<uint32_t>(reference_text.size());
            sw_job_ptr->next_out_ptr  = sw_destination.data();
            sw_job_ptr->available_out = static_cast<uint32_t>(sw_destination.size());

            sw_job_ptr->flags |= QPL_FLAG_FIRST | QPL_FLAG_LAST;
            if (current_test_case.flag_crc)
                hw_job_ptr->flags |= QPL_FLAG_CRC32C;

            auto status = run_job_api(sw_job_ptr);
            ASSERT_ERR_STATUS(status) << "Compression status: " << status;

            hw_job_ptr->op            = qpl_op_z_decomp;
            hw_job_ptr->next_in_ptr   = sw_destination.data();
            hw_job_ptr->available_in  = sw_job_ptr->total_out;
            hw_job_ptr->next_out_ptr  = hw_destination.data();
            hw_job_ptr->available_out = static_cast<uint32_t>(hw_destination.size());

            hw_job_ptr->flags |= QPL_FLAG_FIRST | QPL_FLAG_LAST;

            status = run_job_api(hw_job_ptr);
            ASSERT_ERR_STATUS(status) << "Decompression status: " << status;

            hw_destination.resize(hw_job_ptr->total_out);

            bool vectors_is_equal = reference_text == hw_destination;

            bool crcs_is_equal = false;
            if (current_test_case.flag_crc) {
                crcs_is_equal = sw_job_ptr->crc == hw_job_ptr->crc;
            }

            if (!vectors_is_equal) {
                return testing::AssertionFailure() << "Decompressed data don't match with reference one";
            }
            else if (current_test_case.flag_crc && !crcs_is_equal) {
                return testing::AssertionFailure() << "Compression CRC don't match with deompression CRC";
            }
            else {
                return testing::AssertionSuccess();
            }
        }

        void SetUpBeforeIteration() override {
            current_test_case = GetTestCase();

            // generate random data
            reference_text.clear();
            uint8_t symbol;

            while (reference_text.size() < current_test_case.ref_size - 1) {
                symbol = static_cast<uint8_t>(rand() % 95 + 32);
                reference_text.push_back(symbol);
            }
            reference_text.push_back('\0');

            sw_destination.resize(reference_text.size() * 2u);
            hw_destination.resize(reference_text.size() * 2u);
        }
    };


    QPL_LOW_LEVEL_API_CROSS_TEST_TC(zero, SimpleZeroCompressDecompressFixture, compress_sw_decompress_hw_32)
    {
        ASSERT_TRUE(ValidateCompressSwDecompressHw(qpl_op_z_compress32, qpl_op_z_decompress32));
    }

    QPL_LOW_LEVEL_API_CROSS_TEST_TC(zero, SimpleZeroCompressDecompressFixture, compress_hw_decompress_sw_32)
    {
        ASSERT_TRUE(ValidateCompressHwDecompressSw(qpl_op_z_compress32, qpl_op_z_decompress32));
    }

    QPL_LOW_LEVEL_API_CROSS_TEST_TC(zero, SimpleZeroCompressDecompressFixture, compress_sw_decompress_hw_16)
    {
        ASSERT_TRUE(ValidateCompressSwDecompressHw(qpl_op_z_compress16, qpl_op_z_decompress16));
    }

    QPL_LOW_LEVEL_API_CROSS_TEST_TC(zero, SimpleZeroCompressDecompressFixture, compress_hw_decompress_sw_16)
    {
        ASSERT_TRUE(ValidateCompressHwDecompressSw(qpl_op_z_compress16, qpl_op_z_decompress16));
    }
}
