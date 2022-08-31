/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "algorithm"
#include "../../../common/operation_test.hpp"
#include "ta_ll_common.hpp"
#include "source_provider.hpp"

namespace qpl::test {
constexpr uint64_t no_flag = 0;

class DeflateTestHuffmanOnly : public JobFixture {
protected:
    void SetUp() override {
        JobFixture::SetUp();

        uint32_t job_size = 0u;
        auto     status   = qpl_get_job_size(GetExecutionPath(), &job_size);
        ASSERT_EQ(QPL_STS_OK, status);

        job_buffer            = new uint8_t[job_size];
        decompression_job_ptr = reinterpret_cast<qpl_job *>(job_buffer);
        status                = qpl_init_job(GetExecutionPath(), decompression_job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);
    }

    void TearDown() override {
        JobFixture::TearDown();

        qpl_fini_job(decompression_job_ptr);
        delete[] job_buffer;
    }

    uint8_t *job_buffer            = nullptr;
    qpl_job *decompression_job_ptr = nullptr;

    void RunHuffmanOnlyDynamicTest(bool is_big_endian = false, bool omit_verification = true) {
        // Variables
        uint32_t             seed       = GetSeed();
        uint32_t             max_length = 4096;
        std::vector<uint8_t> reference_buffer;
        uint32_t             status     = QPL_STS_OK;
        uint32_t             total_out_ptr[2];

        qpl_huffman_table_t c_huffman_table;

        status = qpl_huffman_only_table_create(compression_table_type,
                                               GetExecutionPath(),
                                               DEFAULT_ALLOCATOR_C,
                                               &c_huffman_table);

        ASSERT_EQ(status, QPL_STS_OK) << "Table creation failed";

        // Initializing buffers
        source_provider source_generator(max_length, 8u, seed);

        ASSERT_NO_THROW(source = source_generator.get_source());

        destination.resize(max_length * 2);
        std::fill(destination.begin(), destination.end(), 0u);

        reference_buffer.resize(max_length);
        std::fill(reference_buffer.begin(), reference_buffer.end(), 0u);

        job_ptr->huffman_table = c_huffman_table;
        job_ptr->flags = QPL_FLAG_FIRST |
            QPL_FLAG_LAST |
            QPL_FLAG_NO_HDRS |
            QPL_FLAG_GEN_LITERALS |
            QPL_FLAG_DYNAMIC_HUFFMAN |
            ((is_big_endian) ? QPL_FLAG_HUFFMAN_BE : no_flag) |
            ((omit_verification) ? QPL_FLAG_OMIT_VERIFY : no_flag);

        for (uint32_t count_exec = 0; count_exec < 2; count_exec++) {
            job_ptr->op = qpl_op_compress;
            job_ptr->next_in_ptr = source.data();
            job_ptr->next_out_ptr = destination.data();
            job_ptr->available_in = max_length;
            job_ptr->available_out = max_length * 2;
            job_ptr->total_in = 0;
            job_ptr->total_out = 0;
            job_ptr->last_bit_offset = 0;
            job_ptr->first_index_min_value = 0;
            job_ptr->crc = 0;

            // Compress
            status = run_job_api(job_ptr);
            ASSERT_EQ(QPL_STS_OK, status);
            total_out_ptr[count_exec] = job_ptr->total_out;
        }
        ASSERT_EQ(total_out_ptr[0], total_out_ptr[1]);

        qpl_huffman_table_t d_huffman_table;

        status = qpl_huffman_only_table_create(decompression_table_type,
                                               GetExecutionPath(),
                                               DEFAULT_ALLOCATOR_C,
                                               &d_huffman_table);

        ASSERT_EQ(status, QPL_STS_OK) << "Table creation failed";

        status = qpl_huffman_table_init_with_other(d_huffman_table, c_huffman_table);

        ASSERT_EQ(QPL_STS_OK, status) << "Decompression table creation failed";

        decompression_job_ptr->op          = qpl_op_decompress;
        decompression_job_ptr->next_in_ptr = destination.data();

        decompression_job_ptr->next_out_ptr                   = reference_buffer.data();
        decompression_job_ptr->available_in                   = job_ptr->total_out;
        decompression_job_ptr->available_out                  = max_length;
        decompression_job_ptr->ignore_end_bits                =  (8 - job_ptr->last_bit_offset) & 7;
        decompression_job_ptr->huffman_table                  = d_huffman_table;
        decompression_job_ptr->flags                          = QPL_FLAG_NO_HDRS |
                                                                   ((is_big_endian) ? QPL_FLAG_HUFFMAN_BE : no_flag);
        decompression_job_ptr->flags                         |= QPL_FLAG_FIRST | QPL_FLAG_LAST;

        // Decompress
        status = run_job_api(decompression_job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        // Free resources
        status = qpl_huffman_table_destroy(c_huffman_table);
        ASSERT_EQ(QPL_STS_OK, status) << "Compression table destruction failed";

        status = qpl_huffman_table_destroy(d_huffman_table);
        ASSERT_EQ(QPL_STS_OK, status) << "Decompression table destruction failed";

        qpl_fini_job(job_ptr);
        qpl_fini_job(decompression_job_ptr);

        // Verify
        ASSERT_TRUE(CompareVectors(source, reference_buffer, max_length));
    }

    void RunHuffmanOnlyStaticTest(bool is_big_endian = false, bool omit_verification = true) {
        // Variables
        uint32_t             seed            = GetSeed();
        uint32_t             max_length      = 4096;
        std::vector<uint8_t> reference_buffer;
        uint32_t             status          = QPL_STS_OK;
        qpl_huffman_table_t  c_huffman_table;
        uint32_t             total_out_ptr[2];

        status = qpl_huffman_only_table_create(compression_table_type,
                                               GetExecutionPath(),
                                               DEFAULT_ALLOCATOR_C,
                                               &c_huffman_table);

        ASSERT_EQ(status, QPL_STS_OK) << "Table creation failed";

        // Initializing buffers
        source_provider      source_generator(max_length,
                                              8u,
                                              seed);

        ASSERT_NO_THROW(source = source_generator.get_source());

        destination.resize(max_length * 2);
        std::fill(destination.begin(), destination.end(), 0u);

        reference_buffer.resize(max_length);
        std::fill(reference_buffer.begin(), reference_buffer.end(), 0u);

        // Building table
        job_ptr->op            = qpl_op_compress;
        job_ptr->next_in_ptr   = source.data();
        job_ptr->next_out_ptr  = destination.data();
        job_ptr->available_in  = max_length;
        job_ptr->available_out = max_length * 2;

        job_ptr->huffman_table = c_huffman_table;
        job_ptr->flags         = QPL_FLAG_FIRST |
                                 QPL_FLAG_LAST |
                                 QPL_FLAG_NO_HDRS |
                                 QPL_FLAG_GEN_LITERALS |
                                 QPL_FLAG_DYNAMIC_HUFFMAN |
                                 ((is_big_endian) ? QPL_FLAG_HUFFMAN_BE : no_flag) |
                                 ((omit_verification) ? QPL_FLAG_OMIT_VERIFY : no_flag);

        status = run_job_api(job_ptr);

        if (qpl_path_software == job_ptr->data_ptr.path) {
            ASSERT_EQ(QPL_STS_OK, status);
        } else {
            if (QPL_STS_OK != status) {
                if (QPL_STS_VERIFY_ERR == status && is_big_endian) {
                    // Fix for HW issue in version 1.0 NO_HDR mode does not work for the case of BE16 compression.
                    // This is because we need to drop up to 15 bits, but we can only drop at most 7 bits.
                    // So in some cases, verify will fail in the BE16 case.
                    // The fix for this is too complicated, and this is really a niche operation.
                    // The recommendation is that we dont fix this, and add documentation that in this case
                    // (no headers and big-endian-16), verify not be used, and if the user wants to verify
                    // the output, they should (within the app) decompress the compressed buffer into a new
                    // buffer, and compare that against the original input.

                    std::cout << "Deflate verify stage failed with status: " << " " << status << "\n";
                    std::cout << "It is known issue for NO_HDR BE version 1.0 - ignoring\n";
                } else {
                    FAIL() << "Deflate status: " << status << "\n";
                }
            }
        }

        // Now initialize compression job
        status = qpl_init_job(GetExecutionPath(), job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        job_ptr->huffman_table = c_huffman_table;
        job_ptr->flags = QPL_FLAG_FIRST |
            QPL_FLAG_LAST |
            QPL_FLAG_NO_HDRS |
            QPL_FLAG_GEN_LITERALS |
            ((is_big_endian) ? QPL_FLAG_HUFFMAN_BE : no_flag) |
            ((omit_verification) ? QPL_FLAG_OMIT_VERIFY : no_flag);

        for (uint32_t count_exec = 0; count_exec < 2; count_exec++) {
            job_ptr->op = qpl_op_compress;
            job_ptr->next_in_ptr = source.data();
            job_ptr->next_out_ptr = destination.data();
            job_ptr->available_in = max_length;
            job_ptr->available_out = max_length * 2;
            job_ptr->total_in = 0;
            job_ptr->total_out = 0;
            job_ptr->last_bit_offset = 0;
            job_ptr->first_index_min_value = 0;
            job_ptr->crc = 0;

            status = run_job_api(job_ptr);

            if (qpl_path_software == job_ptr->data_ptr.path) {
                ASSERT_EQ(QPL_STS_OK, status);
            }
            else {
                if (QPL_STS_OK != status) {
                    if (QPL_STS_VERIFY_ERR == status && is_big_endian) {
                        // Fix for HW issue in version 1.0 NO_HDR mode does not work for the case of BE16 compression.
                        // This is because we need to drop up to 15 bits, but we can only drop at most 7 bits.
                        // So in some cases, verify will fail in the BE16 case.
                        // The fix for this is too complicated, and this is really a niche operation.
                        // The recommendation is that we dont fix this, and add documentation that in this case
                        // (no headers and big-endian-16), verify not be used, and if the user wants to verify
                        // the output, they should (within the app) decompress the compressed buffer into a new
                        // buffer, and compare that against the original input.

                        std::cout << "Deflate verify stage failed with status: " << " " << status << "\n";
                        std::cout << "It is known issue for NO_HDR BE version 1.0 - ignoring\n";
                    }
                    else {
                        FAIL() << "Deflate status: " << status << "\n";
                    }
                }
            }
            total_out_ptr[count_exec] = job_ptr->total_out;
        }
        ASSERT_EQ(total_out_ptr[0], total_out_ptr[1]);

        qpl_huffman_table_t d_huffman_table;

        status = qpl_huffman_only_table_create(decompression_table_type,
                                               GetExecutionPath(),
                                               DEFAULT_ALLOCATOR_C,
                                               &d_huffman_table);

        ASSERT_EQ(status, QPL_STS_OK) << "Table creation failed";

        status = qpl_huffman_table_init_with_other(d_huffman_table, c_huffman_table);

        ASSERT_EQ(QPL_STS_OK, status) << "Decompression table creation failed";

        decompression_job_ptr->op          = qpl_op_decompress;
        decompression_job_ptr->next_in_ptr = destination.data();

        decompression_job_ptr->next_out_ptr                   = reference_buffer.data();
        decompression_job_ptr->available_in                   = job_ptr->total_out;
        decompression_job_ptr->available_out                  = max_length;
        decompression_job_ptr->ignore_end_bits                = (8 - job_ptr->last_bit_offset) & 7;
        decompression_job_ptr->huffman_table                  = d_huffman_table;
        decompression_job_ptr->flags                          = QPL_FLAG_NO_HDRS | ((is_big_endian) ? QPL_FLAG_HUFFMAN_BE : no_flag);
        decompression_job_ptr->flags                         |= QPL_FLAG_FIRST | QPL_FLAG_LAST;

        // Decompress
        status = run_job_api(decompression_job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        // Free resources
        status = qpl_huffman_table_destroy(c_huffman_table);
        ASSERT_EQ(QPL_STS_OK, status) << "Compression table creation failed";

        status = qpl_huffman_table_destroy(d_huffman_table);
        ASSERT_EQ(QPL_STS_OK, status) << "Decompression table creation failed";

        qpl_fini_job(job_ptr);
        qpl_fini_job(decompression_job_ptr);

        // Verify
        ASSERT_TRUE(CompareVectors(source, reference_buffer, max_length));
    }
};

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(huffman_only, dynamic_le, DeflateTestHuffmanOnly) {
    RunHuffmanOnlyDynamicTest();
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(huffman_only, dynamic_be, DeflateTestHuffmanOnly) {
    RunHuffmanOnlyDynamicTest(true);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(huffman_only, static_le, DeflateTestHuffmanOnly) {
    RunHuffmanOnlyStaticTest();
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(huffman_only, static_be, DeflateTestHuffmanOnly) {
    RunHuffmanOnlyStaticTest(true);
}

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(huffman_only_verify, dynamic_le, DeflateTestHuffmanOnly)
    {
        RunHuffmanOnlyDynamicTest(false, false);
    }

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(huffman_only_verify, dynamic_be, DeflateTestHuffmanOnly) {
    RunHuffmanOnlyDynamicTest(true, false);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(huffman_only_verify, static_le, DeflateTestHuffmanOnly) {
    RunHuffmanOnlyStaticTest(false, false);
}

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(huffman_only_verify, static_be, DeflateTestHuffmanOnly)
    {
        RunHuffmanOnlyStaticTest(true, false);
    }
}
