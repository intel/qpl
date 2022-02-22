/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/* @todo

#include "operation_test.hpp"
#include "gendefs.hpp"
#include "igenerator.h"
#include "qpl_api.h"
#include "own_qpl_defs.h"
#include "tn_common.hpp"
#include "own_qpl_structures.h"

// Test via middle layer as component
#include "compression/verification/verification_state_builder.hpp"
#include "compression/verification/verify.hpp"

namespace qpl::test
{
    namespace proxy {

    template <qpl::ml::execution_path_t path>
    auto setup_and_perform_verification(qpl_job *job_ptr) noexcept -> uint32_t {
        using namespace qpl::ml::compression;

        auto *qpl_state = reinterpret_cast<qpl_compression_state *>(job_ptr->data_ptr.compress_state_ptr);

        auto verification_buffer_size = verify_state<path>::get_buffer_size();
        auto verification_buffer_ptr = std::make_unique<uint8_t[]>(verification_buffer_size);

        qpl::ml::allocation_buffer_t state_buffer(verification_buffer_ptr.get(),
                                                  verification_buffer_ptr.get() + verification_buffer_size);

        const qpl::ml::util::linear_allocator allocator(state_buffer);

        auto builder = (job_ptr->flags & QPL_FLAG_FIRST) ?
                       verification_state_builder<path>::create(allocator) :
                       verification_state_builder<path>::restore(allocator);

        auto state = builder.build();

        state.input(qpl_state->verification_meta.index_verify_start_ptr,
                    qpl_state->verification_meta.index_verify_start_ptr
                    + qpl_state->verification_meta.index_verify_length)
             .required_crc(job_ptr->crc);

        auto verification_result = perform_verification<path, verification_mode_t::verify_deflate_stream>(state);

        if (verification_result.status == parser_status_t::error) {
            return qpl::ml::status_list::verify_error;
        } else {
            return qpl::ml::status_list::ok;
        }
    }

    uint32_t ml_verify(qpl_job *job_ptr) {
        return setup_and_perform_verification<qpl::ml::execution_path_t::software>(job_ptr);
    }
    }

    class VerifyErrorsTest : public JobFixture
    {
    public:
        void SetUp() override
        {
            JobFixture::SetUp();
        }

        qpl_status VerifyRunBrokenStream(TestType test_type)
        {
            std::vector<uint8_t> encoded_data_buffer(0);
            std::vector<uint8_t> decoded_data_buffer(0);

            GenStatus  generator_status = GEN_OK;
            TestFactor test_factor;
            test_factor.seed = GetSeed();
            test_factor.type = test_type;

            gz_generator::InflateGenerator data_generator;

            generator_status = data_generator.generate(encoded_data_buffer,
                                                       decoded_data_buffer,
                                                       test_factor);

            EXPECT_EQ(GEN_OK, generator_status);

            constexpr uint32_t indexing_buffer_size = 1 << (qpl_mblk_size_32k + 8u);

            std::vector<uint8_t>  indexBuffer(indexing_buffer_size);
            std::vector<uint64_t>  indexArray(1024);

            auto *state = (qpl_compression_state *) job_ptr->data_ptr.compress_state_ptr;
            state->verification_meta.index_verify_start_ptr = encoded_data_buffer.data();
            state->verification_meta.index_verify_length    = static_cast<uint32_t>(encoded_data_buffer.size());
            state->verification_meta.mini_block_buffer_ptr  = indexBuffer.data();
            state->verification_meta.index_bits_processed   = 0u;

            job_ptr->idx_array        = indexArray.data();
            job_ptr->idx_max_size     = (uint32_t) indexArray.size();
            job_ptr->idx_num_written  = 0u;
            job_ptr->mini_block_size  = qpl_mblk_size_32k;
            job_ptr->flags            = QPL_FLAG_FIRST;

            return static_cast<qpl_status>(proxy::ml_verify(job_ptr));
        }

        void TearDown() override
        {
            JobFixture::TearDown();
        }
    };

    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(verify, VerifyErrorsTest, undefined_code_length_code)
    {
        TestType  test_type      = UNDEFINED_CL_CODE;
        qpl_status verify_status = VerifyRunBrokenStream(test_type);

        EXPECT_EQ(QPL_STS_VERIFY_ERR, verify_status);
    }

    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(verify, VerifyErrorsTest, first_literal_lengths_code_is_16)
    {
        TestType  test_type      = FIRST_LL_CODE_16;
        qpl_status verify_status = VerifyRunBrokenStream(test_type);

        EXPECT_EQ(QPL_STS_VERIFY_ERR, verify_status);
    }

    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(verify, VerifyErrorsTest, first_distance_lengths_code_is16)
    {
        TestType  test_type      = FIRST_D_CODE_16;
        qpl_status verify_status = VerifyRunBrokenStream(test_type);

        EXPECT_EQ(QPL_STS_VERIFY_ERR, verify_status);
    }

    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(verify, VerifyErrorsTest, no_literal_lengths_code)
    {
        TestType  test_type      = NO_LL_CODE;
        qpl_status verify_status = VerifyRunBrokenStream(test_type);

        EXPECT_EQ(QPL_STS_VERIFY_ERR, verify_status);
    }

    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(verify, VerifyErrorsTest, strading_code_length_codes)
    {
        TestType  test_type      = CL_CODES_SPAN_LL_WITH_D;
        qpl_status verify_status = VerifyRunBrokenStream(test_type);
        
        EXPECT_EQ(QPL_STS_VERIFY_ERR, verify_status);
    }

    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(verify, VerifyErrorsTest, many_literal_length_codes_declared)
    {
        TestType  test_type      = TOO_MANY_LL_CODES;
        qpl_status verify_status = VerifyRunBrokenStream(test_type);
        
        EXPECT_EQ(QPL_STS_VERIFY_ERR, verify_status);
    }

    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(verify, VerifyErrorsTest, big_count_repeating_literal_length_codes)
    {
        TestType  test_type      = BIG_REPEAT_COUNT_LL;
        qpl_status verify_status = VerifyRunBrokenStream(test_type);

        EXPECT_EQ(QPL_STS_VERIFY_ERR, verify_status);
    }

    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(verify, VerifyErrorsTest, many_distance_codes)
    {
        TestType  test_type      = TOO_MANY_D_CODES;
        qpl_status verify_status = VerifyRunBrokenStream(test_type);

        EXPECT_EQ(QPL_STS_VERIFY_ERR, verify_status);
    }

    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(verify, VerifyErrorsTest, many_distance_codes_declared)
    {
        TestType  test_type      = TOO_MANY_D_CODES_V2;
        qpl_status verify_status = VerifyRunBrokenStream(test_type);
        
        EXPECT_EQ(QPL_STS_VERIFY_ERR, verify_status);
    }

    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(verify, VerifyErrorsTest, large_count_repeating_distance_codes)
    {
        TestType  test_type      = BIG_REPEAT_COUNT_D;
        qpl_status verify_status = VerifyRunBrokenStream(test_type);

        EXPECT_EQ(QPL_STS_VERIFY_ERR, verify_status);
    }

    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(verify, VerifyErrorsTest, oversubscribed_code_lengths_tree)
    {
        TestType  test_type      = OVERSUBSCRIBED_CL_TREE;
        qpl_status verify_status = VerifyRunBrokenStream(test_type);

        EXPECT_EQ(QPL_STS_VERIFY_ERR, verify_status);
    }

    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(verify, VerifyErrorsTest, oversubscribed_literal_lengths_tree)
    {
        TestType  test_type      = OVERSUBSCRIBED_LL_TREE;
        qpl_status verify_status = VerifyRunBrokenStream(test_type);

        EXPECT_EQ(QPL_STS_VERIFY_ERR, verify_status);
    }

    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(verify, VerifyErrorsTest, oversubscribed_distance_lengths_tree)
    {
        TestType  test_type      = OVERSUBSCRIBED_D_TREE;
        qpl_status verify_status = VerifyRunBrokenStream(test_type);

        EXPECT_EQ(QPL_STS_VERIFY_ERR, verify_status);
    }

    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(verify, VerifyErrorsTest, using_reserved_literal_length_code)
    {
        TestType  test_type      = BLOCK_CONTAIN_UNALLOWABLE_LL_CODE;
        qpl_status verify_status = VerifyRunBrokenStream(test_type);

        EXPECT_EQ(QPL_STS_VERIFY_ERR, verify_status);
    }

    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(verify, VerifyErrorsTest, using_reserved_distance_length_code)
    {
        TestType  test_type      = BLOCK_CONTAIN_UNALLOWABLE_D_CODE;
        qpl_status verify_status = VerifyRunBrokenStream(test_type);

        EXPECT_EQ(QPL_STS_VERIFY_ERR, verify_status);
    }

    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(verify, VerifyErrorsTest, invalid_block_type)
    {
        TestType  test_type      = INVALID_BLOCK_TYPE;
        qpl_status verify_status = VerifyRunBrokenStream(test_type);

        EXPECT_EQ(QPL_STS_VERIFY_ERR, verify_status);
    }

    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(verify, VerifyErrorsTest, invalid_stored_length)
    {
        TestType  test_type      = BAD_STORED_LEN;
        qpl_status verify_status = VerifyRunBrokenStream(test_type);

        EXPECT_EQ(QPL_STS_VERIFY_ERR, verify_status);
    }

    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(verify, VerifyErrorsTest, bad_distance)
    {
        TestType  test_type      = BAD_DIST;
        qpl_status verify_status = VerifyRunBrokenStream(test_type);

        EXPECT_EQ(QPL_STS_VERIFY_ERR, verify_status);
    }

    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(verify, VerifyErrorsTest, buffer_overreading_via_distance)
    {
        TestType  test_type      = DIST_BEFORE_START;
        qpl_status verify_status = VerifyRunBrokenStream(test_type);

        EXPECT_EQ(QPL_STS_VERIFY_ERR, verify_status);
    }
}

*/
