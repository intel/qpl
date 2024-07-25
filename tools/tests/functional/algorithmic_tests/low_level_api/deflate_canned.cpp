/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <array>
#include <cstdint>
#include <memory>

#include "huffman_table_unique.hpp"
#include "operation_test.hpp"
#include "source_provider.hpp"
#include "ta_ll_common.hpp"
#include "util.hpp"

namespace qpl::test {
template <class Iterator>
auto init_compression_huffman_table(qpl_huffman_table_t huffman_table, Iterator begin, Iterator end,
                                    qpl_compression_levels level, qpl_path_t path) -> void {
    auto*          source_ptr  = &*begin;
    const uint32_t source_size = std::distance(begin, end);

    qpl_histogram deflate_histogram {};

    auto status = qpl_gather_deflate_statistics(source_ptr, source_size, &deflate_histogram, level, path);

    ASSERT_EQ(status, QPL_STS_OK) << "Failed to gather statistics";

    status = qpl_huffman_table_init_with_histogram(huffman_table, &deflate_histogram);

    ASSERT_EQ(status, QPL_STS_OK) << "Failed to build compression table";
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_canned, default_level, JobFixture) {
    auto path = GetExecutionPath();

    for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
        source = dataset.second;

        destination.resize(source.size() * 2);
        std::vector<uint8_t> reference_buffer(destination.size(), 0U);

        const uint32_t file_size = (uint32_t)source.size();
        ASSERT_NE(0U, file_size) << "Couldn't open file: " << dataset.first;

        // Create and initialize compression table
        const unique_huffman_table c_table(
                deflate_huffman_table_maker(compression_table_type, path, DEFAULT_ALLOCATOR_C),
                any_huffman_table_deleter);
        ASSERT_NE(c_table.get(), nullptr) << "Compression Huffman Table creation failed\n";

        // Init job for a file
        uint32_t   job_size = 0U;
        qpl_status status   = qpl_get_job_size(path, &job_size);
        ASSERT_EQ(QPL_STS_OK, status);

        auto        job_buffer            = std::make_unique<uint8_t[]>(job_size);
        auto* const decompression_job_ptr = reinterpret_cast<qpl_job* const>(job_buffer.get());

        init_compression_huffman_table(c_table.get(), source.data(), source.data() + file_size, qpl_default_level,
                                       path);
        ASSERT_EQ(QPL_STS_OK, status) << "Failed to Initialize Huffman Table\n";

        status = qpl_init_job(path, decompression_job_ptr);
        ASSERT_EQ(QPL_STS_OK, status) << "Failed to init decompression job";

        // Configure compression job fields
        job_ptr->op            = qpl_op_compress;
        job_ptr->level         = qpl_default_level;
        job_ptr->next_in_ptr   = source.data();
        job_ptr->available_in  = file_size;
        job_ptr->next_out_ptr  = destination.data();
        job_ptr->available_out = static_cast<uint32_t>(destination.size());
        job_ptr->huffman_table = c_table.get();
        job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY | QPL_FLAG_CANNED_MODE;

        status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status) << "Compression job failed\n";

#ifdef DEBUG_RATIO
        g_ostrm << "Ratio: " << ((float)file_size / (float)job_ptr->total_out) << "\n";
#endif
        // Create decompression from compression one
        const unique_huffman_table d_table(
                deflate_huffman_table_maker(decompression_table_type, path, DEFAULT_ALLOCATOR_C),
                any_huffman_table_deleter);
        ASSERT_NE(d_table.get(), nullptr) << "Decompression Huffman Table creation failed\n";

        status = qpl_huffman_table_init_with_other(d_table.get(), c_table.get());
        ASSERT_EQ(QPL_STS_OK, status) << "Decompression table initialization failed\n";

        // Configure decompression job fields
        decompression_job_ptr->op            = qpl_op_decompress;
        decompression_job_ptr->next_in_ptr   = destination.data();
        decompression_job_ptr->available_in  = job_ptr->total_out;
        decompression_job_ptr->next_out_ptr  = reference_buffer.data();
        decompression_job_ptr->available_out = static_cast<uint32_t>(reference_buffer.size());
        decompression_job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE;
        decompression_job_ptr->huffman_table = d_table.get();

        status = run_job_api(decompression_job_ptr);
        ASSERT_EQ(QPL_STS_OK, status) << "Decompression job failed\n";

        ASSERT_TRUE(CompareVectors(reference_buffer, source, file_size, "File: " + dataset.first));

        qpl_fini_job(job_ptr);
        qpl_fini_job(decompression_job_ptr);
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_canned, high_level, JobFixture) {
    auto path = GetExecutionPath();
    if (path == qpl_path_hardware) {
        if (0 == JobFixture::num_test++) {
            GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path";
        }
        return;
    }

    for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
        source = dataset.second;

        destination.resize(source.size() * 2);
        std::vector<uint8_t> reference_buffer(destination.size(), 0U);

        const uint32_t file_size = (uint32_t)source.size();
        ASSERT_NE(0U, file_size) << "Couldn't open file: " << dataset.first;

        // Create and initialize compression table
        const unique_huffman_table c_table(
                deflate_huffman_table_maker(compression_table_type, path, DEFAULT_ALLOCATOR_C),
                any_huffman_table_deleter);
        ASSERT_NE(c_table.get(), nullptr) << "Compression Huffman Table creation failed\n";

        // Init job for a file
        uint32_t   job_size = 0U;
        qpl_status status   = qpl_get_job_size(path, &job_size);
        ASSERT_EQ(QPL_STS_OK, status);

        auto        job_buffer            = std::make_unique<uint8_t[]>(job_size);
        auto* const decompression_job_ptr = reinterpret_cast<qpl_job* const>(job_buffer.get());

        init_compression_huffman_table(c_table.get(), source.data(), source.data() + file_size, qpl_high_level, path);
        ASSERT_EQ(QPL_STS_OK, status) << "Failed to initialize Huffman Table\n";

        status = qpl_init_job(path, decompression_job_ptr);
        ASSERT_EQ(QPL_STS_OK, status) << "Failed to init decompression job";

        status = qpl_init_job(path, job_ptr);
        ASSERT_EQ(QPL_STS_OK, status) << "Failed to init compression job";

        // Configure compression job fields
        job_ptr->op            = qpl_op_compress;
        job_ptr->next_in_ptr   = source.data();
        job_ptr->available_in  = file_size;
        job_ptr->next_out_ptr  = destination.data();
        job_ptr->available_out = static_cast<uint32_t>(destination.size());
        job_ptr->huffman_table = c_table.get();
        job_ptr->level         = qpl_high_level;
        job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY | QPL_FLAG_CANNED_MODE;

        status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status) << "Compression failed";

#ifdef DEBUG_RATIO
        std::cout << "Ratio: " << ((float)file_size / (float)job_ptr->total_out) << "\n";
#endif

        // Create decompression from compression one
        const unique_huffman_table d_table(
                deflate_huffman_table_maker(decompression_table_type, path, DEFAULT_ALLOCATOR_C),
                any_huffman_table_deleter);
        ASSERT_NE(d_table.get(), nullptr) << "Decompression Huffman Table creation failed\n";

        status = qpl_huffman_table_init_with_other(d_table.get(), c_table.get());
        ASSERT_EQ(status, QPL_STS_OK) << "Decompression Huffman Table initialization failed\n";

        // Configure decompression job fields
        decompression_job_ptr->op            = qpl_op_decompress;
        decompression_job_ptr->next_in_ptr   = destination.data();
        decompression_job_ptr->available_in  = job_ptr->total_out;
        decompression_job_ptr->next_out_ptr  = reference_buffer.data();
        decompression_job_ptr->available_out = static_cast<uint32_t>(reference_buffer.size());
        decompression_job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE;
        decompression_job_ptr->huffman_table = d_table.get();

        status = run_job_api(decompression_job_ptr);
        ASSERT_EQ(QPL_STS_OK, status) << "Decompression failed";

        ASSERT_TRUE(CompareVectors(reference_buffer, source, file_size, "File: " + dataset.first));

        qpl_fini_job(job_ptr);
        qpl_fini_job(decompression_job_ptr);
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_canned, complex_high_level, JobFixture) {
    uint32_t              required_output_size = 0;
    uint32_t              job_size             = 0U;
    std::vector<uint32_t> file_sizes;
    std::vector<uint32_t> compressed_file_sizes;

    auto path = GetExecutionPath();

    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_hardware, "High level compression is not supported on hardware path");

    // Create and initialize compression table
    const unique_huffman_table c_table(deflate_huffman_table_maker(compression_table_type, path, DEFAULT_ALLOCATOR_C),
                                       any_huffman_table_deleter);
    ASSERT_NE(c_table.get(), nullptr) << "Compression Huffman Table creation failed\n";

    for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
        std::vector<uint8_t> file_content;

        file_content = dataset.second;

        source.insert(source.end(), file_content.begin(), file_content.end());
        required_output_size += (uint32_t)file_content.size();
        file_sizes.push_back((uint32_t)file_content.size());
    }

    destination.resize(required_output_size * 2);
    std::vector<uint8_t> reference_buffer(destination.size(), 0U);

    // Init job for a file
    qpl_status status = qpl_get_job_size(path, &job_size);
    ASSERT_EQ(QPL_STS_OK, status);

    auto        job_buffer            = std::make_unique<uint8_t[]>(job_size);
    auto* const decompression_job_ptr = reinterpret_cast<qpl_job* const>(job_buffer.get());

    init_compression_huffman_table(c_table.get(), source.begin(), source.end(), qpl_high_level, path);
    ASSERT_EQ(QPL_STS_OK, status) << "Failed to initialize Huffman Table\n";

    status = qpl_init_job(path, decompression_job_ptr);
    ASSERT_EQ(QPL_STS_OK, status) << "Failed to init decompression job\n";

    job_ptr->huffman_table = c_table.get();

    uint8_t* current_source_ptr      = &(*source.begin());
    uint8_t* current_destination_ptr = &(*destination.begin());

    for (auto file_size : file_sizes) {
        status = qpl_init_job(path, job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        // Configure compression job fields
        job_ptr->op            = qpl_op_compress;
        job_ptr->next_in_ptr   = current_source_ptr;
        job_ptr->available_in  = file_size;
        job_ptr->next_out_ptr  = current_destination_ptr;
        job_ptr->available_out = file_size * 2;
        job_ptr->huffman_table = c_table.get();
        job_ptr->level         = qpl_high_level;
        job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY | QPL_FLAG_CANNED_MODE;

        status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status) << "Compression job failed\n";

        compressed_file_sizes.push_back(job_ptr->total_out);
        current_destination_ptr += job_ptr->total_out;
        current_source_ptr += file_size;

#ifdef DEBUG_RATIO
        std::cout << "Ratio: " << ((float)file_size / (float)job_ptr->total_out) << "\n";
#endif

        qpl_fini_job(job_ptr);
    }

    uint8_t*       current_reference_buffer_ptr  = &(*reference_buffer.begin());
    uint8_t*       current_compressed_source_ptr = &(*destination.begin());
    const uint32_t output_bytes_available        = (uint32_t)reference_buffer.size();

    // Create decompression from compression one
    const unique_huffman_table d_table(deflate_huffman_table_maker(decompression_table_type, path, DEFAULT_ALLOCATOR_C),
                                       any_huffman_table_deleter);
    ASSERT_NE(d_table.get(), nullptr) << "Decompression Huffman Table creation failed\n";

    status = qpl_huffman_table_init_with_other(d_table.get(), c_table.get());
    ASSERT_EQ(QPL_STS_OK, status) << "Decompression Huffman Table initialization failed\n";

    decompression_job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE;
    decompression_job_ptr->huffman_table = d_table.get();
    decompression_job_ptr->next_out_ptr  = current_reference_buffer_ptr;
    decompression_job_ptr->available_out = output_bytes_available;

    for (auto compressed_size : compressed_file_sizes) {
        decompression_job_ptr->op           = qpl_op_decompress;
        decompression_job_ptr->next_in_ptr  = current_compressed_source_ptr;
        decompression_job_ptr->available_in = compressed_size;

        status = run_job_api(decompression_job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        current_compressed_source_ptr += compressed_size;
    }

    qpl_fini_job(decompression_job_ptr);

    ASSERT_TRUE(CompareVectors(reference_buffer, source, (uint32_t)source.size()));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_canned, complex_default_level, JobFixture) {
    uint32_t              required_output_size = 0;
    uint32_t              job_size             = 0U;
    std::vector<uint32_t> file_sizes;
    std::vector<uint32_t> compressed_file_sizes;

    auto path = GetExecutionPath();

    // Create and initialize compression table
    const unique_huffman_table c_table(deflate_huffman_table_maker(compression_table_type, path, DEFAULT_ALLOCATOR_C),
                                       any_huffman_table_deleter);
    ASSERT_NE(c_table.get(), nullptr) << "Compression Huffman Table creation failed\n";

    for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
        std::vector<uint8_t> file_content = dataset.second;

        source.insert(source.end(), file_content.begin(), file_content.end());
        required_output_size += (uint32_t)file_content.size();
        file_sizes.push_back((uint32_t)file_content.size());
    }

    destination.resize(required_output_size * 2);
    std::vector<uint8_t> reference_buffer(destination.size(), 0U);

    // Init job for a file
    qpl_status status = qpl_get_job_size(path, &job_size);
    ASSERT_EQ(QPL_STS_OK, status);

    auto        job_buffer            = std::make_unique<uint8_t[]>(job_size);
    auto* const decompression_job_ptr = reinterpret_cast<qpl_job* const>(job_buffer.get());

    init_compression_huffman_table(c_table.get(), source.begin(), source.end(), qpl_default_level, path);
    ASSERT_EQ(QPL_STS_OK, status) << "Compression Huffman Table initialization failed\n";

    status = qpl_init_job(path, decompression_job_ptr);
    ASSERT_EQ(QPL_STS_OK, status) << "Failed to init decompression job\n";

    job_ptr->huffman_table = c_table.get();

    uint8_t* current_source_ptr      = &(*source.begin());
    uint8_t* current_destination_ptr = &(*destination.begin());

    for (auto file_size : file_sizes) {
        status = qpl_init_job(path, job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        // Configure compression job fields
        job_ptr->op            = qpl_op_compress;
        job_ptr->level         = qpl_default_level;
        job_ptr->next_in_ptr   = current_source_ptr;
        job_ptr->available_in  = file_size;
        job_ptr->next_out_ptr  = current_destination_ptr;
        job_ptr->available_out = file_size * 2;
        job_ptr->huffman_table = c_table.get();
        job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY | QPL_FLAG_CANNED_MODE;

        status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status) << "Compression job failed\n";

        compressed_file_sizes.push_back(job_ptr->total_out);
        current_destination_ptr += job_ptr->total_out;
        current_source_ptr += file_size;

#ifdef DEBUG_RATIO
        std::cout << "Ratio: " << ((float)file_size / (float)job_ptr->total_out) << "\n";
#endif

        qpl_fini_job(job_ptr);
    }

    uint8_t*       current_reference_buffer_ptr  = &(*reference_buffer.begin());
    uint8_t*       current_compressed_source_ptr = &(*destination.begin());
    const uint32_t output_bytes_available        = (uint32_t)reference_buffer.size();

    // Create decompression from compression one
    const unique_huffman_table d_table(deflate_huffman_table_maker(decompression_table_type, path, DEFAULT_ALLOCATOR_C),
                                       any_huffman_table_deleter);
    ASSERT_NE(d_table.get(), nullptr) << "Decompression Huffman Table creation failed\n";

    status = qpl_huffman_table_init_with_other(d_table.get(), c_table.get());
    ASSERT_EQ(status, QPL_STS_OK) << "Decompression Huffman Table initialization failed\n";

    decompression_job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE;
    decompression_job_ptr->huffman_table = d_table.get();
    decompression_job_ptr->next_out_ptr  = current_reference_buffer_ptr;
    decompression_job_ptr->available_out = output_bytes_available;

    for (auto compressed_size : compressed_file_sizes) {
        decompression_job_ptr->op           = qpl_op_decompress;
        decompression_job_ptr->next_in_ptr  = current_compressed_source_ptr;
        decompression_job_ptr->available_in = compressed_size;

        status = run_job_api(decompression_job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        current_compressed_source_ptr += compressed_size;
    }

    qpl_fini_job(decompression_job_ptr);

    ASSERT_TRUE(CompareVectors(reference_buffer, source, (uint32_t)source.size()));
}

// Test if the CRC is correct when reusing job structure
QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_canned, crc_default_level, JobFixture) {
    auto path = GetExecutionPath();

    uint32_t job_size = 0U;
    auto     status   = qpl_get_job_size(path, &job_size);
    ASSERT_EQ(QPL_STS_OK, status);

    for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
        source = dataset.second;

        destination.resize(source.size() * 2);
        std::vector<uint8_t> reference(destination.size(), 0U);

        const uint32_t file_size = (uint32_t)source.size();
        ASSERT_NE(0U, file_size) << "Couldn't open file: " << dataset.first;

        // Create and initialize compression table
        const unique_huffman_table c_table(
                deflate_huffman_table_maker(compression_table_type, path, DEFAULT_ALLOCATOR_C),
                any_huffman_table_deleter);
        ASSERT_NE(c_table.get(), nullptr) << "Compression Huffman Table creation failed\n";

        init_compression_huffman_table(c_table.get(), source.data(), source.data() + file_size, qpl_default_level,
                                       path);
        ASSERT_EQ(QPL_STS_OK, status) << "Failed to initialize Huffman Table\n";

        // Init compression job for each dataset
        status = qpl_init_job(path, job_ptr);
        ASSERT_EQ(QPL_STS_OK, status) << "Failed to init compression job\n";

        // Configure compression job fields
        job_ptr->op            = qpl_op_compress;
        job_ptr->level         = qpl_default_level;
        job_ptr->next_in_ptr   = source.data();
        job_ptr->available_in  = file_size;
        job_ptr->next_out_ptr  = destination.data();
        job_ptr->available_out = static_cast<uint32_t>(destination.size());
        job_ptr->huffman_table = c_table.get();
        job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE | QPL_FLAG_OMIT_VERIFY;

        status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status) << "Compression job failed\n";

        const uint32_t compressed_size = job_ptr->total_out;

        // Create decompression table from compression one
        const unique_huffman_table d_table(
                deflate_huffman_table_maker(decompression_table_type, path, DEFAULT_ALLOCATOR_C),
                any_huffman_table_deleter);
        ASSERT_NE(d_table.get(), nullptr) << "Decompression Huffman Table creation failed\n";

        status = qpl_huffman_table_init_with_other(d_table.get(), c_table.get());
        ASSERT_EQ(QPL_STS_OK, status) << "Decompression Huffman Table initialization failed\n";

        // Configure decompression job fields
        job_ptr->op            = qpl_op_decompress;
        job_ptr->next_in_ptr   = destination.data();
        job_ptr->available_in  = compressed_size;
        job_ptr->next_out_ptr  = reference.data();
        job_ptr->available_out = static_cast<uint32_t>(reference.size());
        job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE;
        job_ptr->huffman_table = d_table.get();

        status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status) << "Decompression job failed\n";

        const uint32_t first_decompression_crc = job_ptr->crc;

        // Reuse job structure for decompression
        job_ptr->next_in_ptr   = destination.data();
        job_ptr->available_in  = compressed_size;
        job_ptr->next_out_ptr  = reference.data();
        job_ptr->available_out = static_cast<uint32_t>(reference.size());

        status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status) << "Second Decompression job failed\n";
        const uint32_t second_decompression_crc = job_ptr->crc;

        ASSERT_EQ(first_decompression_crc, second_decompression_crc)
                << "Incorrect CRC in canned mode decompression when reusing job structure.\n";
    }
}

// Test if the CRC is correct when reusing job structure
QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_canned, crc_high_level, JobFixture) {
    auto path = GetExecutionPath();

    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_hardware,
                              "Deflate operation doesn't support high compression level on the hardware path");

    uint32_t job_size = 0U;
    auto     status   = qpl_get_job_size(path, &job_size);
    ASSERT_EQ(QPL_STS_OK, status);

    for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
        source = dataset.second;

        destination.resize(source.size() * 2);
        std::vector<uint8_t> reference(destination.size(), 0U);

        const uint32_t file_size = (uint32_t)source.size();
        ASSERT_NE(0U, file_size) << "Couldn't open file: " << dataset.first;

        // Create and initialize compression table
        const unique_huffman_table c_table(
                deflate_huffman_table_maker(compression_table_type, path, DEFAULT_ALLOCATOR_C),
                any_huffman_table_deleter);
        ASSERT_NE(c_table.get(), nullptr) << "Compression Huffman Table creation failed\n";

        init_compression_huffman_table(c_table.get(), source.data(), source.data() + file_size, qpl_high_level, path);
        ASSERT_EQ(QPL_STS_OK, status) << "Failed to initialize Huffman Table\n";

        // Init compression job for each dataset
        status = qpl_init_job(path, job_ptr);
        ASSERT_EQ(QPL_STS_OK, status) << "Failed to init compression job\n";

        // Configure compression job fields
        job_ptr->op            = qpl_op_compress;
        job_ptr->level         = qpl_high_level;
        job_ptr->next_in_ptr   = source.data();
        job_ptr->available_in  = file_size;
        job_ptr->next_out_ptr  = destination.data();
        job_ptr->available_out = static_cast<uint32_t>(destination.size());
        job_ptr->huffman_table = c_table.get();
        job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE | QPL_FLAG_OMIT_VERIFY;

        status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status) << "Compression job failed\n";

        const uint32_t compressed_size = job_ptr->total_out;

        // Create decompression from compression one
        const unique_huffman_table d_table(
                deflate_huffman_table_maker(decompression_table_type, path, DEFAULT_ALLOCATOR_C),
                any_huffman_table_deleter);
        ASSERT_NE(d_table.get(), nullptr) << "Decompression Huffman Table creation failed\n";

        status = qpl_huffman_table_init_with_other(d_table.get(), c_table.get());
        ASSERT_EQ(QPL_STS_OK, status) << "Decompression Huffman Table initialization failed\n";

        // Configure decompression job fields
        job_ptr->op            = qpl_op_decompress;
        job_ptr->next_in_ptr   = destination.data();
        job_ptr->available_in  = compressed_size;
        job_ptr->next_out_ptr  = reference.data();
        job_ptr->available_out = static_cast<uint32_t>(reference.size());
        job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE;
        job_ptr->huffman_table = d_table.get();

        status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status) << "Decompression job failed\n";

        const uint32_t first_decompression_crc = job_ptr->crc;

        // Reuse job structure for decompression
        job_ptr->next_in_ptr   = destination.data();
        job_ptr->available_in  = compressed_size;
        job_ptr->next_out_ptr  = reference.data();
        job_ptr->available_out = static_cast<uint32_t>(reference.size());

        status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status) << "Second Decompression job failed\n";

        const uint32_t second_decompression_crc = job_ptr->crc;

        ASSERT_EQ(first_decompression_crc, second_decompression_crc)
                << "Incorrect CRC in canned mode decompression when reusing job structure.";
    }
}
} // namespace qpl::test
