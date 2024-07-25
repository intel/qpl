/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "base_crc_test_fixture.hpp"
#include "crc32_test_cases.hpp"

namespace qpl::test {
class SimpleCRC32Fixture
    : public BaseCRCTestFixture
    , public TestCases<SimpleCRC32TestCase> {
private:
    SimpleCRC32TestCase current_test_case {};

protected:
    void SetUp() override {
        BaseCRCTestFixture::SetUp();
        this->InitializeTestCases();
    }

    void InitializeTestCases() {

        for (int size = 640; size <= 6400; size += 64) {
            SimpleCRC32TestCase test_case {};
            test_case.source_size = size;

            // generate random polynom
            // 32 bit always equals 1 for CRC32
            test_case.poly = (static_cast<uint64_t>(rand()) << 32) | 0x0000000100000000;

            test_case.is_be_bit_order = 0;
            test_case.is_inverse      = 0;
            AddNewTestCase(test_case);

            test_case.is_inverse = 1;
            AddNewTestCase(test_case);

            test_case.is_be_bit_order = 1;
            test_case.is_inverse      = 0;
            AddNewTestCase(test_case);

            test_case.is_inverse = 1;
            AddNewTestCase(test_case);
        }
    }

public:
    testing::AssertionResult ValidateCRC32SwCRC32Hw() {
        sw_job_ptr->op           = qpl_op_crc64;
        sw_job_ptr->next_in_ptr  = sw_source.data();
        sw_job_ptr->available_in = static_cast<uint32_t>(sw_source.size());
        sw_job_ptr->crc64_poly   = current_test_case.poly;

        sw_job_ptr->flags |= QPL_FLAG_FIRST | QPL_FLAG_LAST;
        if (current_test_case.is_be_bit_order) sw_job_ptr->flags |= QPL_FLAG_CRC64_BE;
        if (current_test_case.is_inverse) sw_job_ptr->flags |= QPL_FLAG_CRC64_INV;

        auto status = run_job_api(sw_job_ptr);
        ASSERT_ERR_STATUS(status) << "SW CRC status: " << status;

        hw_job_ptr->op           = qpl_op_crc64;
        hw_job_ptr->next_in_ptr  = hw_source.data();
        hw_job_ptr->available_in = static_cast<uint32_t>(hw_source.size());
        hw_job_ptr->crc64_poly   = current_test_case.poly;

        hw_job_ptr->flags |= QPL_FLAG_FIRST | QPL_FLAG_LAST;
        if (current_test_case.is_be_bit_order) hw_job_ptr->flags |= QPL_FLAG_CRC64_BE;
        if (current_test_case.is_inverse) hw_job_ptr->flags |= QPL_FLAG_CRC64_INV;

        status = run_job_api(hw_job_ptr);
        ASSERT_ERR_STATUS(status) << "Decompression status: " << status;

        const bool crcs_is_equal = (hw_job_ptr->crc64 == sw_job_ptr->crc64);

        if (!crcs_is_equal) {
            return testing::AssertionFailure() << "CRCs don't match";
        } else {
            return testing::AssertionSuccess();
        }
    }

    void SetUpBeforeIteration() override {
        current_test_case = GetTestCase();

        sw_source.clear();
        hw_source.clear();
        uint8_t symbol = 0U;
        while (sw_source.size() < current_test_case.source_size - 1) {
            symbol = static_cast<uint8_t>(rand() % 95 + 32);
            sw_source.push_back(symbol);
        }
        sw_source.push_back('\0');

        hw_source = sw_source;
    }
};

QPL_LOW_LEVEL_API_CRC_TEST_TC(crc32, SimpleCRC32Fixture, crc32_sw_crc32_hw) {
    ASSERT_TRUE(ValidateCRC32SwCRC32Hw());
}
} // namespace qpl::test
