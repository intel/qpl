/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @brief Bad Args tests for qplZeroCompress functions
 * 
 * qpl_zero_compress_16u
 * qpl_zero_compress_32u
 */

#include <array>
#include <algorithm>

#include "gtest/gtest.h"
#include "tb_ll_common.hpp"
#include "../../../common/operation_test.hpp"

namespace qpl::test {
constexpr uint32_t BLOCK_SIZE = 128u; /**< sizeof processing block in bytes */

/*
 *@brief Helper for filling required fields in qpl_job structure
 */
template <class DataType>
static qpl_job *fillQplJob(qpl_job *job_ptr, DataType *src_ptr, size_t srcLength, DataType *dst_ptr, size_t dstLength) {
    job_ptr->next_in_ptr   = (uint8_t *) (src_ptr);
    job_ptr->available_in  = (uint32_t) (srcLength * sizeof(DataType));
    job_ptr->next_out_ptr  = (uint8_t *) (dst_ptr);
    job_ptr->available_out = (uint32_t) (dstLength * sizeof(DataType));

    return job_ptr;
}

/*
 *@brief Helper for calling ZeroCompress functions
 */
template <class DataType>
static inline qpl_status ZeroCompress(qpl_job *job) {
    return QPL_STS_LIBRARY_INTERNAL_ERR;
}

template <>
inline qpl_status ZeroCompress<uint16_t>(qpl_job *job) {
    if (job)
        job->op = qpl_op_z_compress16;

    return run_job_api(job);
}

template <>
inline qpl_status ZeroCompress<uint32_t>(qpl_job *job) {
    if (job)
        job->op = qpl_op_z_compress32;
    return run_job_api(job);
}

/*
 *@brief Main function to test bad args processing for ZeroCompress
 *
 * Performs check of correct processing situations when:
 * - job pointer = NULL
 * - next_in_ptr pointer = NULL
 * - next_out_ptr pointer = NULL
 * - available_in = 0
 * - available_out = 0
 */
template <class DataType>
static inline void TestZeroCompress(qpl_job *const job_ptr) {
    constexpr uint32_t   TEST_ARRAY_SIZE = 1u;
    std::array<DataType, TEST_ARRAY_SIZE * 2> source{};
    std::array<DataType, TEST_ARRAY_SIZE * 2> destination{};

    source.fill(0);
    destination.fill(0);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR,
              ZeroCompress<DataType>(nullptr))
              << "NULL job check";
    EXPECT_EQ(QPL_STS_NULL_PTR_ERR,
              ZeroCompress<DataType>(fillQplJob(job_ptr, (DataType *) nullptr, 1, destination.data(), 1))) <<
            " NULL src check";
    EXPECT_EQ(QPL_STS_NULL_PTR_ERR,
              ZeroCompress<DataType>(fillQplJob(job_ptr, source.data(), 1, (DataType *) nullptr, 1))) <<
            " NULL dst check";
    EXPECT_EQ(QPL_STS_SIZE_ERR,
              ZeroCompress<DataType>(fillQplJob(job_ptr, source.data(), 0, destination.data(), 1))) <<
            " 0 length of src check";
    EXPECT_EQ(QPL_STS_DST_IS_SHORT_ERR,
              ZeroCompress<DataType>(fillQplJob(job_ptr, source.data(), 1, destination.data(), 0))) <<
            " 0 length of dst check";

    fillQplJob(job_ptr, source.data(), 1u, destination.data(), 1u);
    job_ptr->available_in++; //Corrupt source size
    EXPECT_EQ(QPL_STS_SRC_IS_SHORT_ERR, ZeroCompress<DataType>(job_ptr));
}

/*
 *@brief Tests the situation when target buffer has length exact or less than required
 *
 * Function tests following cases 
 *  - when the dst buffer has exact length as required (function should complete with success)
 *  - when the dst buffer has less length than required (test that function returns error in this case)
 */
template <class DataType>
static inline void TestZeroCompressDstNotEnough(qpl_job *const job_ptr) {
    const uint32_t blockElementsNum = BLOCK_SIZE / sizeof(DataType); // define how many elements in block
    const uint32_t tagWordSize      = blockElementsNum / (8 * sizeof(DataType)); // define how many words in tag
    const uint32_t zerosNum         = tagWordSize; // only this number of zeroes will be added to src, to have an ability to forecast dst length
    const uint32_t numOfBlocks      = 2; // should be enough for this test TODO: check if it should be updated for further development
    const uint32_t length           = blockElementsNum * numOfBlocks;

    uint32_t seed = qpl::test::util::TestEnvironment::GetInstance().GetSeed();;

    std::array<DataType, length> source{};
    std::array<DataType, length> destination{};

    source.fill(0);
    destination.fill(0);

    // generate non zero values
    qpl::test::random rndGen(1, (DataType) UINT32_MAX, seed);
    std::generate(source.begin(), source.end(),
                  [&rndGen]()
                  {
                      return static_cast<DataType>(rndGen);
                  });

    // src was filled with non zero elements
    // now fill src data with zeroes thus dst length should is equal to <length>
    for (int i = 0; i < numOfBlocks; i++)
    {
        for (int j = 0; j < zerosNum; j++)
        {
            source[i * blockElementsNum + j] = 0;
        }
    }

    // Test dst buffer which has exact length as required (function should complete with success)
    fillQplJob(job_ptr, source.data(), source.size(), destination.data(), destination.size());

    qpl_status status = ZeroCompress<DataType>(job_ptr);

    ASSERT_EQ(QPL_STS_OK, status) << "Test dst buffer which has exact length as required";

    // Test dst buffer which has less length than required (function should complete with error)
    fillQplJob(job_ptr, source.data(), source.size(), destination.data(), destination.size() - 1);

    status = ZeroCompress<DataType>(job_ptr);

    ASSERT_EQ(QPL_STS_DST_IS_SHORT_ERR, status)
                                << "Test dst buffer which has less length than required, return code: expected";
}

/////////////////////////////////////////////////////////////////////////

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(zero_compress16, base) {
    TestZeroCompress<uint16_t>(job_ptr);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(zero_compress32, base) {
    TestZeroCompress<uint32_t>(job_ptr);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(zero_compress16, short_destination) {
    TestZeroCompressDstNotEnough<uint16_t>(job_ptr);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(zero_compress32, short_destination) {
    TestZeroCompressDstNotEnough<uint32_t>(job_ptr);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(zero_compress16, buffers_overlap) {
    check_buffer_overlap(job_ptr, qpl_op_z_compress16, OPERATION_FLAGS);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(zero_compress32, buffers_overlap) {
    check_buffer_overlap(job_ptr, qpl_op_z_compress32, OPERATION_FLAGS);
}

}
