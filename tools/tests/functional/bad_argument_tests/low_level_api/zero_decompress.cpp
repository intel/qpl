/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/


/**
 * @brief Bad Args tests for qplZeroDecompress functions
 * 
 * qpl_zero_decompress_16u
 * qpl_zero_decompress_32u
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
template<class DataType>
static qpl_job *fillQplJob(qpl_job *job_ptr, DataType *src_ptr, size_t srcLength, DataType *dst_ptr, size_t dstLength)
{
    job_ptr->next_in_ptr   = (uint8_t *) (src_ptr);
    job_ptr->available_in  = (uint32_t) srcLength * sizeof(DataType);
    job_ptr->next_out_ptr  = (uint8_t *) (dst_ptr);
    job_ptr->available_out = (uint32_t) dstLength * sizeof(DataType);

    return job_ptr;
}

/*
 *@brief Helper for calling ZeroDecompress functions
 */
template<class DataType>
static inline qpl_status ZeroDecompress(qpl_job *job) {
    return QPL_STS_LIBRARY_INTERNAL_ERR;
}

template<>
inline qpl_status ZeroDecompress<uint16_t>(qpl_job *job)
{
    if (job)
        job->op = qpl_op_z_decompress16;

    return run_job_api(job);
}

template<>
inline qpl_status ZeroDecompress<uint32_t>(qpl_job *job)
{
    if (job)
        job->op = qpl_op_z_decompress32;

    return run_job_api(job);
}

/*
 *@brief Main function to test bad args processing for ZeroDecompress
 *
 * Performs check of correct processing situations when:
 * - job pointer = NULL
 * - next_in_ptr pointer = NULL
 * - next_out_ptr pointer = NULL
 * - available_in = 0
 * - available_out = 0
 * - tag is incomplete (for 16u case only - it should be 4 uint16_t elements)
 *
 *@return 
 * - resOK - success
 * - resFAIL - failure
 */
template<class DataType>
static inline void TestZeroDecompress(qpl_job *const job_ptr) {
    constexpr uint32_t blockElementsNum = BLOCK_SIZE / sizeof(DataType);
    constexpr uint32_t tagWordSize      = blockElementsNum / (8 * sizeof(DataType));
    constexpr uint32_t length           = (2 == sizeof(DataType)) ? 4 : 1;

    std::array<DataType, length * 2> source{};
    std::array<DataType, length * 2> destination{};

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR,
              ZeroDecompress<DataType>(nullptr)) << "NULL job check";
    EXPECT_EQ(QPL_STS_NULL_PTR_ERR,
             ZeroDecompress<DataType>(fillQplJob(job_ptr, (DataType *) nullptr, length, destination.data(), length)))
             << " NULL src check";
    EXPECT_EQ(QPL_STS_NULL_PTR_ERR,
             ZeroDecompress<DataType>(fillQplJob(job_ptr, source.data(), length, (DataType *) nullptr, length)))
            << " NULL dst check";
    EXPECT_EQ(QPL_STS_INVALID_ZERO_DECOMP_HDR,
             ZeroDecompress<DataType>(fillQplJob(job_ptr, source.data(), length - 1, destination.data(), length)))
            << " less than min length of src check";
    EXPECT_EQ(QPL_STS_DST_IS_SHORT_ERR,
             ZeroDecompress<DataType>(fillQplJob(job_ptr, source.data(), length, destination.data(), 0)))
            << " 0 length of dst check";
    //EXPECT_EQ((QPL_STS_SIZE_ERR,  ZeroDecompress<DataType>(fillQplJob(&job, source.data(),  1, destination.data(), 1)) && tagWordSize > 1), "incomplete tag check");

    fillQplJob(job_ptr, source.data(), length, destination.data(), length);
    job_ptr->available_in++; // Corrupt source length
    EXPECT_EQ(QPL_STS_SRC_IS_SHORT_ERR, ZeroDecompress<DataType>(job_ptr));
}

/*
 *@brief Tests the situation when target buffer has length exact or less than required
 *
 * Function tests following cases 
 *  - when the dst buffer has exact length as required (function should complete with success)
 *  - when the dst buffer has less length than required (test that function returns error in this case)
 */
template<class DataType>
static inline void TestZeroDecompressDstNotEnough(qpl_job *const job_ptr) {
    constexpr uint32_t blockElementsNum = BLOCK_SIZE / sizeof(DataType); // define how many elements in block
    constexpr uint32_t tagWordSize      = blockElementsNum / (8 * sizeof(DataType)); // define how many words in tag
    constexpr uint32_t zerosNum         = tagWordSize; // only this number of zeroes will be added to src, to have an ability to forecast dst length
    constexpr uint32_t numOfBlocks      = 2; // should be enough for this test TODO: check if it should be updated for further development
    constexpr uint32_t length           = blockElementsNum * numOfBlocks;

    // seed generation
    uint32_t seed = qpl::test::util::TestEnvironment::GetInstance().GetSeed();

    std::array<DataType, length> source{};
    std::array<DataType, length> destination{};
    destination.fill(0);

    // generate non zero values
    qpl::test::random rndGen(1, 3, seed);
    std::generate(source.begin(), source.end(),
                  [&rndGen]()
                  {
                      return static_cast<DataType>(rndGen);
                  });

    // src was filled with non zero elements
    // now make tag elements thus dst length should be equal to <length>
    DataType firstTagWord = ((DataType) 0 - 1) << zerosNum;

    for (int i = 0; i < numOfBlocks; i++)
    {
        source[i * blockElementsNum + 0] = firstTagWord;
        for (int j = 1; j < tagWordSize; j++)
        {
            // for uint16_t case all bits of rest of tag elements are 1
            source[i * blockElementsNum + j] = ((DataType) 0 - 1);
        }
    }

    // Test dst buffer which has exact length as required (function should complete with success)
    fillQplJob(job_ptr, source.data(), (uint32_t) source.size(), destination.data(), (uint32_t) destination.size());

    qpl_status status = ZeroDecompress<DataType>(job_ptr);

    ASSERT_EQ(QPL_STS_OK, status) << "Test dst buffer which has exact length as required";

    // Test dst buffer which has less length than required (function should complete with error)
    fillQplJob(job_ptr, source.data(), (uint32_t) source.size(), destination.data(), (uint32_t) destination.size() - 1);

    status = ZeroDecompress<DataType>(job_ptr);

    ASSERT_EQ(QPL_STS_DST_IS_SHORT_ERR, status) << "Test dst buffer which has less length than required";
}
/////////////////////////////////////////////////////////////////////////

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(zero_decompress16, base) {
    TestZeroDecompress<uint16_t>(job_ptr);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(zero_decompress32, base) {
    TestZeroDecompress<uint32_t>(job_ptr);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(zero_decompress16, short_destination) {
    TestZeroDecompressDstNotEnough<uint16_t>(job_ptr);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(zero_decompress32, short_destination) {
    TestZeroDecompressDstNotEnough<uint32_t>(job_ptr);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(zero_decompress16, buffers_overlap) {
    check_buffer_overlap(job_ptr, qpl_op_z_decompress16, OPERATION_FLAGS);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(zero_decompress32, buffers_overlap) {
    check_buffer_overlap(job_ptr, qpl_op_z_decompress32, OPERATION_FLAGS);
}

}