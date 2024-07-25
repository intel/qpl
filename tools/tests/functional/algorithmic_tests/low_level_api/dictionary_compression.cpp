/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <algorithm>
#include <array>

#include "huffman_table_unique.hpp"
#include "iaa_features_checks.hpp"
#include "qpl_test_environment.hpp"
#include "source_provider.hpp"
#include "ta_ll_common.hpp"
#include "util.hpp"

namespace qpl::test {
enum compression_mode { fixed_compression, static_compression, dynamic_compression, canned_compression };

template <compression_mode mode>
void compress_with_chunks(std::vector<uint8_t>& source, std::vector<uint8_t>& destination, uint32_t chunk_size,
                          qpl_dictionary* dictionary_ptr, qpl_huffman_table_t table_ptr, qpl_compression_levels level,
                          qpl_path_t compression_execution_path) {}

template <>
void compress_with_chunks<compression_mode::dynamic_compression>(
        std::vector<uint8_t>& source, std::vector<uint8_t>& destination, uint32_t chunk_size,
        qpl_dictionary*        dictionary_ptr,
        qpl_huffman_table_t    table_ptr, //NOLINT(misc-unused-parameters)
        qpl_compression_levels level, qpl_path_t compression_execution_path) {

    uint32_t compression_job_size = 0;

    auto status = qpl_get_job_size(compression_execution_path, &compression_job_size);
    ASSERT_EQ(QPL_STS_OK, status);

    // Allocate buffers for compression job
    auto compression_job_buffer = std::make_unique<uint8_t[]>(compression_job_size);
    auto compression_job_ptr    = reinterpret_cast<qpl_job*>(compression_job_buffer.get());

    // Initialize compression job
    status = qpl_init_job(compression_execution_path, compression_job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);

    // Configure job
    compression_job_ptr->op = qpl_op_compress;

    compression_job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_DYNAMIC_HUFFMAN | QPL_FLAG_OMIT_VERIFY;

    compression_job_ptr->available_in  = static_cast<uint32_t>(source.size());
    compression_job_ptr->available_out = static_cast<uint32_t>(destination.size());

    compression_job_ptr->next_in_ptr  = source.data();
    compression_job_ptr->next_out_ptr = destination.data();

    compression_job_ptr->dictionary = dictionary_ptr;
    compression_job_ptr->level      = level;

    // Compress
    auto     current_chunk_size = chunk_size;
    uint32_t iteration_count    = 0;
    auto     source_bytes_left  = static_cast<uint32_t>(source.size());

    while (source_bytes_left > 0) {
        if (current_chunk_size >= source_bytes_left) {
            compression_job_ptr->flags |= QPL_FLAG_LAST;
            current_chunk_size = source_bytes_left;
        }

        source_bytes_left -= current_chunk_size;
        compression_job_ptr->next_in_ptr  = source.data() + iteration_count * chunk_size;
        compression_job_ptr->available_in = current_chunk_size;
        status                            = run_job_api(compression_job_ptr);
        ASSERT_EQ(status, QPL_STS_OK);

        compression_job_ptr->flags &= ~QPL_FLAG_FIRST;
        iteration_count++;
    }

    destination.resize(compression_job_ptr->total_out);

    qpl_fini_job(compression_job_ptr);
}

template <>
void compress_with_chunks<compression_mode::static_compression>(std::vector<uint8_t>& source,
                                                                std::vector<uint8_t>& destination, uint32_t chunk_size,
                                                                qpl_dictionary*        dictionary_ptr,
                                                                qpl_huffman_table_t    table_ptr,
                                                                qpl_compression_levels level,
                                                                qpl_path_t             compression_execution_path) {

    uint32_t compression_job_size = 0;

    auto status = qpl_get_job_size(compression_execution_path, &compression_job_size);
    ASSERT_EQ(QPL_STS_OK, status);

    // Allocate buffers for compression job
    auto compression_job_buffer = std::make_unique<uint8_t[]>(compression_job_size);
    auto compression_job_ptr    = reinterpret_cast<qpl_job*>(compression_job_buffer.get());

    // Initialize compression job
    status = qpl_init_job(compression_execution_path, compression_job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);

    // Configure job
    compression_job_ptr->op = qpl_op_compress;

    compression_job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_OMIT_VERIFY;

    compression_job_ptr->available_in  = static_cast<uint32_t>(source.size());
    compression_job_ptr->available_out = static_cast<uint32_t>(destination.size());

    compression_job_ptr->next_in_ptr  = source.data();
    compression_job_ptr->next_out_ptr = destination.data();

    compression_job_ptr->dictionary = dictionary_ptr;
    compression_job_ptr->level      = level;

    compression_job_ptr->huffman_table = table_ptr;

    // Compress
    auto     current_chunk_size = chunk_size;
    uint32_t iteration_count    = 0;
    auto     source_bytes_left  = static_cast<uint32_t>(source.size());

    while (source_bytes_left > 0) {
        if (current_chunk_size >= source_bytes_left) {
            compression_job_ptr->flags |= QPL_FLAG_LAST;
            current_chunk_size = source_bytes_left;
        }

        source_bytes_left -= current_chunk_size;
        compression_job_ptr->next_in_ptr  = source.data() + iteration_count * chunk_size;
        compression_job_ptr->available_in = current_chunk_size;
        status                            = run_job_api(compression_job_ptr);
        ASSERT_EQ(status, QPL_STS_OK);

        compression_job_ptr->flags &= ~QPL_FLAG_FIRST;
        iteration_count++;
    }

    destination.resize(compression_job_ptr->total_out);

    qpl_fini_job(compression_job_ptr);
}

template <>
void compress_with_chunks<compression_mode::canned_compression>(std::vector<uint8_t>& source,
                                                                std::vector<uint8_t>& destination, uint32_t chunk_size,
                                                                qpl_dictionary*        dictionary_ptr,
                                                                qpl_huffman_table_t    table_ptr,
                                                                qpl_compression_levels level,
                                                                qpl_path_t             compression_execution_path) {

    uint32_t compression_job_size = 0;

    auto status = qpl_get_job_size(compression_execution_path, &compression_job_size);
    ASSERT_EQ(QPL_STS_OK, status);

    // Allocate buffers for compression job
    auto compression_job_buffer = std::make_unique<uint8_t[]>(compression_job_size);
    auto compression_job_ptr    = reinterpret_cast<qpl_job*>(compression_job_buffer.get());

    // Initialize compression job
    status = qpl_init_job(compression_execution_path, compression_job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);

    // Configure job
    compression_job_ptr->op = qpl_op_compress;

    compression_job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_CANNED_MODE | QPL_FLAG_OMIT_VERIFY;

    compression_job_ptr->available_in  = static_cast<uint32_t>(source.size());
    compression_job_ptr->available_out = static_cast<uint32_t>(destination.size());

    compression_job_ptr->next_in_ptr  = source.data();
    compression_job_ptr->next_out_ptr = destination.data();

    compression_job_ptr->dictionary = dictionary_ptr;
    compression_job_ptr->level      = level;

    compression_job_ptr->huffman_table = table_ptr;

    // Compress
    auto     current_chunk_size = chunk_size;
    uint32_t iteration_count    = 0;
    auto     source_bytes_left  = static_cast<uint32_t>(source.size());

    while (source_bytes_left > 0) {
        if (current_chunk_size >= source_bytes_left) {
            compression_job_ptr->flags |= QPL_FLAG_LAST;
            current_chunk_size = source_bytes_left;
        }

        source_bytes_left -= current_chunk_size;
        compression_job_ptr->next_in_ptr  = source.data() + iteration_count * chunk_size;
        compression_job_ptr->available_in = current_chunk_size;
        status                            = run_job_api(compression_job_ptr);
        ASSERT_EQ(status, QPL_STS_OK);

        compression_job_ptr->flags &= ~QPL_FLAG_FIRST;
        iteration_count++;
    }

    destination.resize(compression_job_ptr->total_out);

    qpl_fini_job(compression_job_ptr);
}

template <>
void compress_with_chunks<compression_mode::fixed_compression>(
        std::vector<uint8_t>& source, std::vector<uint8_t>& destination, uint32_t chunk_size,
        qpl_dictionary*        dictionary_ptr,
        qpl_huffman_table_t    table_ptr, //NOLINT(misc-unused-parameters)
        qpl_compression_levels level, qpl_path_t compression_execution_path) {

    uint32_t compression_job_size = 0;

    auto status = qpl_get_job_size(compression_execution_path, &compression_job_size);
    ASSERT_EQ(QPL_STS_OK, status);

    // Allocate buffers for compression job
    auto compression_job_buffer = std::make_unique<uint8_t[]>(compression_job_size);
    auto compression_job_ptr    = reinterpret_cast<qpl_job*>(compression_job_buffer.get());

    // Initialize compression job
    status = qpl_init_job(compression_execution_path, compression_job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);

    // Configure job
    compression_job_ptr->op = qpl_op_compress;

    compression_job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_OMIT_VERIFY;

    compression_job_ptr->available_in  = static_cast<uint32_t>(source.size());
    compression_job_ptr->available_out = static_cast<uint32_t>(destination.size());

    compression_job_ptr->next_in_ptr  = source.data();
    compression_job_ptr->next_out_ptr = destination.data();

    compression_job_ptr->dictionary = dictionary_ptr;
    compression_job_ptr->level      = level;

    // Compress
    auto     current_chunk_size = chunk_size;
    uint32_t iteration_count    = 0;
    auto     source_bytes_left  = static_cast<uint32_t>(source.size());

    while (source_bytes_left > 0) {
        if (current_chunk_size >= source_bytes_left) {
            compression_job_ptr->flags |= QPL_FLAG_LAST;
            current_chunk_size = source_bytes_left;
        }

        source_bytes_left -= current_chunk_size;
        compression_job_ptr->next_in_ptr  = source.data() + iteration_count * chunk_size;
        compression_job_ptr->available_in = current_chunk_size;
        status                            = run_job_api(compression_job_ptr);
        ASSERT_EQ(status, QPL_STS_OK);

        compression_job_ptr->flags &= ~QPL_FLAG_FIRST;
        iteration_count++;
    }

    destination.resize(compression_job_ptr->total_out);

    qpl_fini_job(compression_job_ptr);
}

void decompress_with_chunks(std::vector<uint8_t>& compressed_source, std::vector<uint8_t>& destination,
                            uint32_t chunk_size, qpl_dictionary* dictionary_ptr,
                            qpl_path_t decompression_execution_path) {

    uint32_t decompression_job_size = 0;

    auto status = qpl_get_job_size(decompression_execution_path, &decompression_job_size);
    ASSERT_EQ(QPL_STS_OK, status);

    // Allocate buffers for decompression job

    auto decompression_job_buffer = std::make_unique<uint8_t[]>(decompression_job_size);
    auto decompression_job_ptr    = reinterpret_cast<qpl_job*>(decompression_job_buffer.get());

    // Initialize decompression job
    status = qpl_init_job(decompression_execution_path, decompression_job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);

    decompression_job_ptr->op            = qpl_op_decompress;
    decompression_job_ptr->flags         = QPL_FLAG_FIRST;
    decompression_job_ptr->available_in  = static_cast<uint32_t>(compressed_source.size());
    decompression_job_ptr->next_in_ptr   = destination.data();
    decompression_job_ptr->available_out = static_cast<uint32_t>(destination.size());
    decompression_job_ptr->next_out_ptr  = destination.data();
    decompression_job_ptr->dictionary    = dictionary_ptr;

    auto     current_chunk_size = chunk_size;
    uint32_t iteration_count    = 0;
    auto     source_bytes_left  = static_cast<uint32_t>(compressed_source.size());

    while (source_bytes_left > 0) {
        if (current_chunk_size >= source_bytes_left) {
            decompression_job_ptr->flags |= QPL_FLAG_LAST;
            current_chunk_size = source_bytes_left;
        }

        source_bytes_left -= current_chunk_size;
        decompression_job_ptr->next_in_ptr  = compressed_source.data() + iteration_count * chunk_size;
        decompression_job_ptr->available_in = current_chunk_size;
        status                              = run_job_api(decompression_job_ptr);
        ASSERT_EQ(status, QPL_STS_OK);

        decompression_job_ptr->flags &= ~QPL_FLAG_FIRST;
        iteration_count++;
    }

    destination.resize(decompression_job_ptr->total_out);
    qpl_fini_job(decompression_job_ptr);
}

void decompress_with_chunks(std::vector<uint8_t>& compressed_source, std::vector<uint8_t>& destination,
                            uint32_t chunk_size, qpl_dictionary* dictionary_ptr, qpl_huffman_table_t table_ptr,
                            qpl_path_t decompression_execution_path) {

    uint32_t decompression_job_size = 0;

    auto status = qpl_get_job_size(decompression_execution_path, &decompression_job_size);
    ASSERT_EQ(QPL_STS_OK, status);

    // Allocate buffers for decompression job

    auto decompression_job_buffer = std::make_unique<uint8_t[]>(decompression_job_size);
    auto decompression_job_ptr    = reinterpret_cast<qpl_job*>(decompression_job_buffer.get());

    // Initialize decompression job
    status = qpl_init_job(decompression_execution_path, decompression_job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);

    decompression_job_ptr->op            = qpl_op_decompress;
    decompression_job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_CANNED_MODE;
    decompression_job_ptr->available_in  = static_cast<uint32_t>(compressed_source.size());
    decompression_job_ptr->next_in_ptr   = destination.data();
    decompression_job_ptr->available_out = static_cast<uint32_t>(destination.size());
    decompression_job_ptr->next_out_ptr  = destination.data();
    decompression_job_ptr->dictionary    = dictionary_ptr;
    decompression_job_ptr->huffman_table = table_ptr;

    auto     current_chunk_size = chunk_size;
    uint32_t iteration_count    = 0;
    auto     source_bytes_left  = static_cast<uint32_t>(compressed_source.size());

    while (source_bytes_left > 0) {
        if (current_chunk_size >= source_bytes_left) {
            decompression_job_ptr->flags |= QPL_FLAG_LAST;
            current_chunk_size = source_bytes_left;
        }

        source_bytes_left -= current_chunk_size;
        decompression_job_ptr->next_in_ptr  = compressed_source.data() + iteration_count * chunk_size;
        decompression_job_ptr->available_in = current_chunk_size;
        status                              = run_job_api(decompression_job_ptr);
        ASSERT_EQ(status, QPL_STS_OK);

        decompression_job_ptr->flags &= ~QPL_FLAG_FIRST;
        iteration_count++;
    }

    destination.resize(decompression_job_ptr->total_out);
    qpl_fini_job(decompression_job_ptr);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, dynamic_default_stateless) {
    // HW dictionary compression is only enabled with single job, and HW compression only supports
    // default level. So, compression path could only be set to the given execution path (SW or HW)
    // for stateless default level tests. For other tests, compression path is hardcoded to SW.
    auto compression_execution_path = qpl_path_t::qpl_path_software;
    if (util::TestEnvironment::GetInstance().GetExecutionPath() != qpl_path_t::qpl_path_software &&
        is_iaa_dictionary_compress_supported()) {
        compression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();
    }
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
            std::vector<uint8_t> source = dataset.second;

            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size());

            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status) << "Dictionary build failed for source: " << dataset.first;

                compress_with_chunks<compression_mode::dynamic_compression>(
                        source, compressed_destination, source.size(), dictionary_ptr, nullptr,
                        qpl_compression_levels::qpl_default_level, compression_execution_path);

                decompress_with_chunks(compressed_destination, decompressed_destination, compressed_destination.size(),
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source))
                        << "Compressed and decompressed vectors mismatched for source: " << dataset.first;
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, dynamic_default_stateful_compression) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
            std::vector<uint8_t> source;

            source = dataset.second;

            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);

            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                qpl::test::random random(0U, 0U, util::TestEnvironment::GetInstance().GetSeed());
                random.set_range(1000U, 10000U);
                const auto chunk_size = static_cast<uint32_t>(random);

                compress_with_chunks<compression_mode::dynamic_compression>(
                        source, compressed_destination, chunk_size, dictionary_ptr, nullptr,
                        qpl_compression_levels::qpl_default_level, compression_execution_path);

                decompress_with_chunks(compressed_destination, decompressed_destination, compressed_destination.size(),
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, dynamic_default_stateful_decompression) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }
        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;

            source = dataset.second;

            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);
            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                compress_with_chunks<compression_mode::dynamic_compression>(
                        source, compressed_destination, source.size(), dictionary_ptr, nullptr,
                        qpl_compression_levels::qpl_default_level, compression_execution_path);

                qpl::test::random random(0U, 0U, util::TestEnvironment::GetInstance().GetSeed());
                random.set_range((double)compressed_destination.size() / 10, (double)compressed_destination.size() / 5);
                const auto decompression_chunk_size = static_cast<uint32_t>(random);

                decompress_with_chunks(compressed_destination, decompressed_destination, decompression_chunk_size,
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, dynamic_default_stateful_compression_and_decompression) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }
        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;

            source = dataset.second;

            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);

            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                qpl::test::random random(0U, 0U, util::TestEnvironment::GetInstance().GetSeed());
                random.set_range(1000U, 10000U);
                const auto compression_chunk_size = static_cast<uint32_t>(random);

                compress_with_chunks<compression_mode::dynamic_compression>(
                        source, compressed_destination, compression_chunk_size, dictionary_ptr, nullptr,
                        qpl_compression_levels::qpl_default_level, compression_execution_path);

                random.set_range((double)compressed_destination.size() / 10, (double)compressed_destination.size() / 5);
                const auto decompression_chunk_size = static_cast<uint32_t>(random);

                decompress_with_chunks(compressed_destination, decompressed_destination, decompression_chunk_size,
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, dynamic_high_stateless) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;

            source = dataset.second;

            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);

            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());
                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                compress_with_chunks<compression_mode::dynamic_compression>(
                        source, compressed_destination, source.size(), dictionary_ptr, nullptr,
                        qpl_compression_levels::qpl_high_level, compression_execution_path);

                decompress_with_chunks(compressed_destination, decompressed_destination, compressed_destination.size(),
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, dynamic_high_stateful_compression) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;

            source = dataset.second;

            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);
            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());
                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                qpl::test::random random(0U, 0U, util::TestEnvironment::GetInstance().GetSeed());
                random.set_range(1000U, 10000U);
                const auto chunk_size = static_cast<uint32_t>(random);

                compress_with_chunks<compression_mode::dynamic_compression>(
                        source, compressed_destination, chunk_size, dictionary_ptr, nullptr,
                        qpl_compression_levels::qpl_high_level, compression_execution_path);

                decompress_with_chunks(compressed_destination, decompressed_destination, compressed_destination.size(),
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, dynamic_high_stateful_decompression) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        };

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);
            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                compress_with_chunks<compression_mode::dynamic_compression>(
                        source, compressed_destination, source.size(), dictionary_ptr, nullptr,
                        qpl_compression_levels::qpl_high_level, compression_execution_path);

                qpl::test::random random(0U, 0U, util::TestEnvironment::GetInstance().GetSeed());
                random.set_range((double)compressed_destination.size() / 10, (double)compressed_destination.size() / 5);
                const auto decompression_chunk_size = static_cast<uint32_t>(random);

                decompress_with_chunks(compressed_destination, decompressed_destination, decompression_chunk_size,
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, dynamic_high_stateful_compression_and_decompression) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);
            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                qpl::test::random random(0U, 0U, util::TestEnvironment::GetInstance().GetSeed());
                random.set_range(1000U, 10000U);
                const auto compression_chunk_size = static_cast<uint32_t>(random);

                compress_with_chunks<compression_mode::dynamic_compression>(
                        source, compressed_destination, compression_chunk_size, dictionary_ptr, nullptr,
                        qpl_compression_levels::qpl_high_level, compression_execution_path);

                random.set_range((double)compressed_destination.size() / 10, (double)compressed_destination.size() / 5);
                const auto decompression_chunk_size = static_cast<uint32_t>(random);

                decompress_with_chunks(compressed_destination, decompressed_destination, decompression_chunk_size,
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, fixed_default_stateless) {
    auto compression_execution_path = qpl_path_t::qpl_path_software;
    if (util::TestEnvironment::GetInstance().GetExecutionPath() != qpl_path_t::qpl_path_software &&
        is_iaa_dictionary_compress_supported()) {
        compression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();
    }
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);
            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                compress_with_chunks<compression_mode::fixed_compression>(
                        source, compressed_destination, source.size(), dictionary_ptr, nullptr,
                        qpl_compression_levels::qpl_default_level, compression_execution_path);

                decompress_with_chunks(compressed_destination, decompressed_destination, compressed_destination.size(),
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, fixed_default_stateful_compression) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);
            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                qpl::test::random random(0U, 0U, util::TestEnvironment::GetInstance().GetSeed());
                random.set_range(1000U, 10000U);
                const auto chunk_size = static_cast<uint32_t>(random);

                compress_with_chunks<compression_mode::fixed_compression>(
                        source, compressed_destination, chunk_size, dictionary_ptr, nullptr,
                        qpl_compression_levels::qpl_default_level, compression_execution_path);

                decompress_with_chunks(compressed_destination, decompressed_destination, compressed_destination.size(),
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, fixed_default_stateful_decompression) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);
            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                compress_with_chunks<compression_mode::fixed_compression>(
                        source, compressed_destination, source.size(), dictionary_ptr, nullptr,
                        qpl_compression_levels::qpl_default_level, compression_execution_path);

                qpl::test::random random(0U, 0U, util::TestEnvironment::GetInstance().GetSeed());
                random.set_range((double)compressed_destination.size() / 10, (double)compressed_destination.size() / 5);
                const auto decompression_chunk_size = static_cast<uint32_t>(random);

                decompress_with_chunks(compressed_destination, decompressed_destination, decompression_chunk_size,
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, fixed_default_stateful_compression_and_decompression) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);
            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                qpl::test::random random(0U, 0U, util::TestEnvironment::GetInstance().GetSeed());
                random.set_range(1000U, 10000U);
                const auto compression_chunk_size = static_cast<uint32_t>(random);

                compress_with_chunks<compression_mode::fixed_compression>(
                        source, compressed_destination, compression_chunk_size, dictionary_ptr, nullptr,
                        qpl_compression_levels::qpl_default_level, compression_execution_path);

                random.set_range((double)compressed_destination.size() / 10, (double)compressed_destination.size() / 5);
                const auto decompression_chunk_size = static_cast<uint32_t>(random);

                decompress_with_chunks(compressed_destination, decompressed_destination, decompression_chunk_size,
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, fixed_high_stateless) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);
            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                compress_with_chunks<compression_mode::fixed_compression>(
                        source, compressed_destination, source.size(), dictionary_ptr, nullptr,
                        qpl_compression_levels::qpl_high_level, compression_execution_path);

                decompress_with_chunks(compressed_destination, decompressed_destination, compressed_destination.size(),
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, fixed_high_stateful_compression) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);
            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                qpl::test::random random(0U, 0U, util::TestEnvironment::GetInstance().GetSeed());
                random.set_range(1000U, 10000U);
                const auto chunk_size = static_cast<uint32_t>(random);

                compress_with_chunks<compression_mode::fixed_compression>(
                        source, compressed_destination, chunk_size, dictionary_ptr, nullptr,
                        qpl_compression_levels::qpl_high_level, compression_execution_path);

                decompress_with_chunks(compressed_destination, decompressed_destination, compressed_destination.size(),
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, fixed_high_stateful_decompression) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);
            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                compress_with_chunks<compression_mode::fixed_compression>(
                        source, compressed_destination, source.size(), dictionary_ptr, nullptr,
                        qpl_compression_levels::qpl_high_level, compression_execution_path);

                qpl::test::random random(0U, 0U, util::TestEnvironment::GetInstance().GetSeed());
                random.set_range((double)compressed_destination.size() / 10, (double)compressed_destination.size() / 5);
                const auto decompression_chunk_size = static_cast<uint32_t>(random);

                decompress_with_chunks(compressed_destination, decompressed_destination, decompression_chunk_size,
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, fixed_high_stateful_compression_and_decompression) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);
            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                qpl::test::random random(0U, 0U, util::TestEnvironment::GetInstance().GetSeed());
                random.set_range(1000U, 10000U);
                const auto compression_chunk_size = static_cast<uint32_t>(random);

                compress_with_chunks<compression_mode::fixed_compression>(
                        source, compressed_destination, compression_chunk_size, dictionary_ptr, nullptr,
                        qpl_compression_levels::qpl_high_level, compression_execution_path);

                random.set_range((double)compressed_destination.size() / 10, (double)compressed_destination.size() / 5);
                const auto decompression_chunk_size = static_cast<uint32_t>(random);

                decompress_with_chunks(compressed_destination, decompressed_destination, decompression_chunk_size,
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, static_default_stateless) {
    auto compression_execution_path = qpl_path_t::qpl_path_software;
    if (util::TestEnvironment::GetInstance().GetExecutionPath() != qpl_path_t::qpl_path_software &&
        is_iaa_dictionary_compress_supported()) {
        compression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();
    }
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);
            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                // Create and fill the compression table
                const unique_huffman_table table(
                        deflate_huffman_table_maker(compression_table_type, compression_execution_path,
                                                    DEFAULT_ALLOCATOR_C),
                        any_huffman_table_deleter);
                ASSERT_NE(table.get(), nullptr) << "Huffman Table creation failed\n";

                qpl_histogram deflate_histogram {};
                // Build the table
                status = qpl_gather_deflate_statistics(source.data(), static_cast<uint32_t>(source.size()),
                                                       &deflate_histogram, qpl_default_level,
                                                       compression_execution_path);
                ASSERT_EQ(status, QPL_STS_OK) << "Statistics gathering failed";

                status = qpl_huffman_table_init_with_histogram(table.get(), &deflate_histogram);
                ASSERT_EQ(status, QPL_STS_OK) << "Table init failed";

                compress_with_chunks<compression_mode::static_compression>(
                        source, compressed_destination, source.size(), dictionary_ptr, table.get(),
                        qpl_compression_levels::qpl_default_level, compression_execution_path);

                decompress_with_chunks(compressed_destination, decompressed_destination, compressed_destination.size(),
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, static_default_stateful_compression) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);
            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                // Create and fill the compression table
                const unique_huffman_table table(
                        deflate_huffman_table_maker(compression_table_type, compression_execution_path,
                                                    DEFAULT_ALLOCATOR_C),
                        any_huffman_table_deleter);
                ASSERT_NE(table.get(), nullptr) << "Huffman Table creation failed\n";

                qpl_histogram deflate_histogram {};
                // Build the table
                status = qpl_gather_deflate_statistics(source.data(), static_cast<uint32_t>(source.size()),
                                                       &deflate_histogram, qpl_default_level,
                                                       compression_execution_path);
                ASSERT_EQ(status, QPL_STS_OK) << "Statistics gathering failed";

                status = qpl_huffman_table_init_with_histogram(table.get(), &deflate_histogram);
                ASSERT_EQ(status, QPL_STS_OK) << "Table build failed";

                qpl::test::random random(0U, 0U, util::TestEnvironment::GetInstance().GetSeed());
                random.set_range(1000U, 10000U);
                const auto compression_chunk_size = static_cast<uint32_t>(random);

                compress_with_chunks<compression_mode::static_compression>(
                        source, compressed_destination, compression_chunk_size, dictionary_ptr, table.get(),
                        qpl_compression_levels::qpl_default_level, compression_execution_path);

                decompress_with_chunks(compressed_destination, decompressed_destination, compressed_destination.size(),
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, static_default_stateful_decompression) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);
            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                // Create and fill the compression table
                const unique_huffman_table table(
                        deflate_huffman_table_maker(compression_table_type, compression_execution_path,
                                                    DEFAULT_ALLOCATOR_C),
                        any_huffman_table_deleter);
                ASSERT_NE(table.get(), nullptr) << "Huffman Table creation failed\n";

                qpl_histogram deflate_histogram {};
                // Build the table
                status = qpl_gather_deflate_statistics(source.data(), static_cast<uint32_t>(source.size()),
                                                       &deflate_histogram, qpl_default_level,
                                                       compression_execution_path);
                ASSERT_EQ(status, QPL_STS_OK) << "Statistics gathering failed";

                status = qpl_huffman_table_init_with_histogram(table.get(), &deflate_histogram);
                ASSERT_EQ(status, QPL_STS_OK) << "Table build failed";

                compress_with_chunks<compression_mode::static_compression>(
                        source, compressed_destination, source.size(), dictionary_ptr, table.get(),
                        qpl_compression_levels::qpl_default_level, compression_execution_path);

                qpl::test::random random(0U, 0U, util::TestEnvironment::GetInstance().GetSeed());
                random.set_range((double)compressed_destination.size() / 10, (double)compressed_destination.size() / 5);
                const auto decompression_chunk_size = static_cast<uint32_t>(random);

                decompress_with_chunks(compressed_destination, decompressed_destination, decompression_chunk_size,
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, static_default_stateful_compression_and_decompression) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);
            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                // Create and fill the compression table
                const unique_huffman_table table(
                        deflate_huffman_table_maker(compression_table_type, compression_execution_path,
                                                    DEFAULT_ALLOCATOR_C),
                        any_huffman_table_deleter);
                ASSERT_NE(table.get(), nullptr) << "Huffman Table creation failed\n";

                qpl_histogram deflate_histogram {};
                // Build the table
                status = qpl_gather_deflate_statistics(source.data(), static_cast<uint32_t>(source.size()),
                                                       &deflate_histogram, qpl_default_level,
                                                       compression_execution_path);
                ASSERT_EQ(status, QPL_STS_OK) << "Statistics gathering failed";

                status = qpl_huffman_table_init_with_histogram(table.get(), &deflate_histogram);
                ASSERT_EQ(status, QPL_STS_OK) << "Table build failed";

                qpl::test::random random(0U, 0U, util::TestEnvironment::GetInstance().GetSeed());
                random.set_range(1000U, 10000U);
                const auto compression_chunk_size = static_cast<uint32_t>(random);

                compress_with_chunks<compression_mode::static_compression>(
                        source, compressed_destination, compression_chunk_size, dictionary_ptr, table.get(),
                        qpl_compression_levels::qpl_default_level, compression_execution_path);

                random.set_range((double)compressed_destination.size() / 10, (double)compressed_destination.size() / 5);
                const auto decompression_chunk_size = static_cast<uint32_t>(random);

                decompress_with_chunks(compressed_destination, decompressed_destination, decompression_chunk_size,
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, static_high_stateless) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);
            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                // Create and fill the compression table
                const unique_huffman_table table(
                        deflate_huffman_table_maker(compression_table_type, compression_execution_path,
                                                    DEFAULT_ALLOCATOR_C),
                        any_huffman_table_deleter);
                ASSERT_NE(table.get(), nullptr) << "Huffman Table creation failed\n";

                qpl_histogram deflate_histogram {};
                // Build the table
                status = qpl_gather_deflate_statistics(source.data(), static_cast<uint32_t>(source.size()),
                                                       &deflate_histogram, qpl_high_level, compression_execution_path);
                ASSERT_EQ(status, QPL_STS_OK) << "Statistics gathering failed";

                status = qpl_huffman_table_init_with_histogram(table.get(), &deflate_histogram);
                ASSERT_EQ(status, QPL_STS_OK) << "Table build failed";

                compress_with_chunks<compression_mode::static_compression>(
                        source, compressed_destination, source.size(), dictionary_ptr, table.get(),
                        qpl_compression_levels::qpl_high_level, compression_execution_path);

                decompress_with_chunks(compressed_destination, decompressed_destination, compressed_destination.size(),
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, static_high_stateful_compression) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);
            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                // Create and fill the compression table
                const unique_huffman_table table(
                        deflate_huffman_table_maker(compression_table_type, compression_execution_path,
                                                    DEFAULT_ALLOCATOR_C),
                        any_huffman_table_deleter);
                ASSERT_NE(table.get(), nullptr) << "Huffman Table creation failed\n";

                qpl_histogram deflate_histogram {};
                // Build the table
                status = qpl_gather_deflate_statistics(source.data(), static_cast<uint32_t>(source.size()),
                                                       &deflate_histogram, qpl_default_level,
                                                       compression_execution_path);
                ASSERT_EQ(status, QPL_STS_OK) << "Statistics gathering failed";

                status = qpl_huffman_table_init_with_histogram(table.get(), &deflate_histogram);
                ASSERT_EQ(status, QPL_STS_OK) << "Table build failed";

                qpl::test::random random(0U, 0U, util::TestEnvironment::GetInstance().GetSeed());
                random.set_range(1000U, 10000U);
                const auto compression_chunk_size = static_cast<uint32_t>(random);

                compress_with_chunks<compression_mode::static_compression>(
                        source, compressed_destination, compression_chunk_size, dictionary_ptr, table.get(),
                        qpl_compression_levels::qpl_high_level, compression_execution_path);

                decompress_with_chunks(compressed_destination, decompressed_destination, compressed_destination.size(),
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, static_high_stateful_decompression) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);
            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                // Create and fill the compression table
                const unique_huffman_table table(
                        deflate_huffman_table_maker(compression_table_type, compression_execution_path,
                                                    DEFAULT_ALLOCATOR_C),
                        any_huffman_table_deleter);
                ASSERT_NE(table.get(), nullptr) << "Huffman Table creation failed\n";

                qpl_histogram deflate_histogram {};
                // Build the table
                status = qpl_gather_deflate_statistics(source.data(), static_cast<uint32_t>(source.size()),
                                                       &deflate_histogram, qpl_default_level,
                                                       compression_execution_path);
                ASSERT_EQ(status, QPL_STS_OK) << "Statistics gathering failed";

                status = qpl_huffman_table_init_with_histogram(table.get(), &deflate_histogram);
                ASSERT_EQ(status, QPL_STS_OK) << "Table build failed";

                compress_with_chunks<compression_mode::static_compression>(
                        source, compressed_destination, source.size(), dictionary_ptr, table.get(),
                        qpl_compression_levels::qpl_high_level, compression_execution_path);

                qpl::test::random random(0U, 0U, util::TestEnvironment::GetInstance().GetSeed());
                random.set_range((double)compressed_destination.size() / 10, (double)compressed_destination.size() / 5);
                const auto decompression_chunk_size = static_cast<uint32_t>(random);

                decompress_with_chunks(compressed_destination, decompressed_destination, decompression_chunk_size,
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, static_high_stateful_compression_and_decompression) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);
            for (auto dictionary_length : get_dictionary_lengths()) {
                compressed_destination.resize(source.size() * 2);
                decompressed_destination.resize(source.size());

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                                   dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                // Create and fill the compression table
                const unique_huffman_table table(
                        deflate_huffman_table_maker(compression_table_type, compression_execution_path,
                                                    DEFAULT_ALLOCATOR_C),
                        any_huffman_table_deleter);
                ASSERT_NE(table.get(), nullptr) << "Huffman Table creation failed\n";

                qpl_histogram deflate_histogram {};
                // Build the table
                status = qpl_gather_deflate_statistics(source.data(), static_cast<uint32_t>(source.size()),
                                                       &deflate_histogram, qpl_default_level,
                                                       compression_execution_path);
                ASSERT_EQ(status, QPL_STS_OK) << "Statistics gathering failed";

                status = qpl_huffman_table_init_with_histogram(table.get(), &deflate_histogram);
                ASSERT_EQ(status, QPL_STS_OK) << "Table build failed";

                qpl::test::random random(0U, 0U, util::TestEnvironment::GetInstance().GetSeed());
                random.set_range(1000U, 10000U);
                const auto compression_chunk_size = static_cast<uint32_t>(random);

                compress_with_chunks<compression_mode::static_compression>(
                        source, compressed_destination, compression_chunk_size, dictionary_ptr, table.get(),
                        qpl_compression_levels::qpl_high_level, compression_execution_path);

                random.set_range((double)compressed_destination.size() / 10, (double)compressed_destination.size() / 5);
                const auto decompression_chunk_size = static_cast<uint32_t>(random);

                decompress_with_chunks(compressed_destination, decompressed_destination, decompression_chunk_size,
                                       dictionary_ptr, decompression_execution_path);

                ASSERT_TRUE(CompareVectors(decompressed_destination, source));
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, canned_default_stateless) {
    auto compression_execution_path = qpl_path_t::qpl_path_software;
    if (util::TestEnvironment::GetInstance().GetExecutionPath() != qpl_path_t::qpl_path_software &&
        is_iaa_dictionary_compress_supported()) {
        compression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();
    }

    // HW path canned mode dictionary decompression is not yet supported.
    // This is a workaround to perform decompression on SW path.
    // However, an additional restriction apply when mixing SW & HW path for dictionary:
    // raw dictionary must be 4k.
    // This workaround should be replaced by line below after the feature is supported.

    //auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    auto decompression_execution_path = qpl_path_t::qpl_path_software;
    auto dictionary_length            = 4096U;
    auto c_table_path                 = compression_execution_path;
    auto d_table_path                 = decompression_execution_path;
    if (compression_execution_path == qpl_path_hardware && decompression_execution_path == qpl_path_software) {
        c_table_path = qpl_path_auto;
        d_table_path = qpl_path_auto;
    }

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            std::vector<uint8_t> compressed_destination(source.size() * 2);
            std::vector<uint8_t> decompressed_destination(source.size(), 0);

            // Workaround for HW path canned dictionary decompression
            // Should re-enable testing different dictionary_length after HW path is enabled
            // for (auto            dictionary_length: get_dictionary_lengths()) {
            compressed_destination.resize(source.size() * 2);
            decompressed_destination.resize(source.size());

            if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

            auto dictionary_buffer_size = qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

            auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
            auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

            auto status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                               dictionary_length);
            ASSERT_EQ(QPL_STS_OK, status);

            // Create and fill the compression table
            const unique_huffman_table c_table(
                    deflate_huffman_table_maker(compression_table_type, c_table_path, DEFAULT_ALLOCATOR_C),
                    any_huffman_table_deleter);
            ASSERT_NE(c_table.get(), nullptr) << "Huffman Table creation failed\n";

            qpl_histogram deflate_histogram {};
            // Build the table
            status = qpl_gather_deflate_statistics(source.data(), static_cast<uint32_t>(source.size()),
                                                   &deflate_histogram, qpl_default_level, compression_execution_path);
            ASSERT_EQ(status, QPL_STS_OK) << "Statistics gathering failed";

            status = qpl_huffman_table_init_with_histogram(c_table.get(), &deflate_histogram);
            ASSERT_EQ(status, QPL_STS_OK) << "Table init failed";

            compress_with_chunks<compression_mode::canned_compression>(
                    source, compressed_destination, source.size(), dictionary_ptr, c_table.get(),
                    qpl_compression_levels::qpl_default_level, compression_execution_path);

            // Create and fill the decompression table
            const unique_huffman_table d_table(
                    deflate_huffman_table_maker(decompression_table_type, d_table_path, DEFAULT_ALLOCATOR_C),
                    any_huffman_table_deleter);
            ASSERT_NE(d_table.get(), nullptr) << "Huffman Table creation failed\n";

            status = qpl_huffman_table_init_with_other(d_table.get(), c_table.get());
            ASSERT_EQ(status, QPL_STS_OK) << "Decompression table initialization failed\n";

            decompress_with_chunks(compressed_destination, decompressed_destination, compressed_destination.size(),
                                   dictionary_ptr, d_table.get(), decompression_execution_path);
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, canned_default_stateful) {
    auto compression_execution_path   = qpl_path_t::qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_hardware, "HW path dictionary decompression is not supported for canned mode");

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    const auto seed = util::TestEnvironment::GetInstance().GetSeed();

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            for (auto dictionary_length : get_dictionary_lengths()) {

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                // Create and fill the compression table
                const unique_huffman_table c_table(
                        deflate_huffman_table_maker(compression_table_type, compression_execution_path,
                                                    DEFAULT_ALLOCATOR_C),
                        any_huffman_table_deleter);
                ASSERT_NE(c_table.get(), nullptr) << "Huffman Table creation failed\n";

                qpl_histogram deflate_histogram {};
                // Build the table
                auto status = qpl_gather_deflate_statistics(source.data(), static_cast<uint32_t>(source.size()),
                                                            &deflate_histogram, qpl_default_level,
                                                            compression_execution_path);
                ASSERT_EQ(status, QPL_STS_OK) << "Statistics gathering failed";

                status = qpl_huffman_table_init_with_histogram(c_table.get(), &deflate_histogram);
                ASSERT_EQ(status, QPL_STS_OK) << "Table init failed";

                std::vector<uint8_t> destination(source.size() * 2);
                std::vector<uint8_t> reference(source.size(), 0);

                uint32_t compression_job_size   = 0;
                uint32_t decompression_job_size = 0;

                status = qpl_get_job_size(compression_execution_path, &compression_job_size);
                ASSERT_EQ(QPL_STS_OK, status);

                status = qpl_get_job_size(decompression_execution_path, &decompression_job_size);
                ASSERT_EQ(QPL_STS_OK, status);

                // Allocate buffers for compression and decompression jobs

                auto compression_job_buffer = std::make_unique<uint8_t[]>(compression_job_size);
                auto compression_job_ptr    = reinterpret_cast<qpl_job*>(compression_job_buffer.get());

                auto decompression_job_buffer = std::make_unique<uint8_t[]>(decompression_job_size);
                auto decompression_job_ptr    = reinterpret_cast<qpl_job*>(decompression_job_buffer.get());

                // Initialize compression and decompression jobs

                status = qpl_init_job(compression_execution_path, compression_job_ptr);
                ASSERT_EQ(QPL_STS_OK, status);

                status = qpl_init_job(decompression_execution_path, decompression_job_ptr);
                ASSERT_EQ(QPL_STS_OK, status);

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                              dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                // Create and fill the decompression table
                const unique_huffman_table d_table(
                        deflate_huffman_table_maker(decompression_table_type, decompression_execution_path,
                                                    DEFAULT_ALLOCATOR_C),
                        any_huffman_table_deleter);
                ASSERT_NE(d_table.get(), nullptr) << "Huffman Table creation failed\n";

                status = qpl_huffman_table_init_with_other(d_table.get(), c_table.get());
                ASSERT_EQ(QPL_STS_OK, status);

                // Compress
                compression_job_ptr->op            = qpl_op_compress;
                compression_job_ptr->level         = qpl_default_level;
                compression_job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_CANNED_MODE | QPL_FLAG_OMIT_VERIFY;
                compression_job_ptr->available_out = static_cast<uint32_t>(destination.size());
                compression_job_ptr->next_out_ptr  = destination.data();
                compression_job_ptr->dictionary    = dictionary_ptr;
                compression_job_ptr->huffman_table = c_table.get();

                qpl::test::random random(0U, 0U, seed);

                random.set_range(1000U, 10000U);
                const auto chunk_size         = static_cast<uint32_t>(random);
                uint32_t   current_chunk_size = chunk_size;

                uint32_t iteration_count   = 0;
                auto     source_bytes_left = static_cast<uint32_t>(source.size());
                while (source_bytes_left > 0) {
                    if (current_chunk_size >= source_bytes_left) {
                        compression_job_ptr->flags |= QPL_FLAG_LAST;
                        if (current_chunk_size > source_bytes_left) {
                            current_chunk_size = static_cast<uint32_t>(source.size()) % chunk_size;
                        }
                    }

                    source_bytes_left -= current_chunk_size;
                    compression_job_ptr->next_in_ptr  = source.data() + iteration_count * chunk_size;
                    compression_job_ptr->available_in = current_chunk_size;
                    status                            = run_job_api(compression_job_ptr);
                    ASSERT_EQ(status, QPL_STS_OK);

                    compression_job_ptr->flags &= ~QPL_FLAG_FIRST;
                    iteration_count++;
                }

                decompression_job_ptr->op            = qpl_op_decompress;
                decompression_job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE;
                decompression_job_ptr->available_in  = compression_job_ptr->total_out;
                decompression_job_ptr->next_in_ptr   = destination.data();
                decompression_job_ptr->available_out = static_cast<uint32_t>(reference.size());
                decompression_job_ptr->next_out_ptr  = reference.data();
                decompression_job_ptr->dictionary    = dictionary_ptr;
                decompression_job_ptr->huffman_table = d_table.get();

                status = run_job_api(decompression_job_ptr);
                ASSERT_EQ(status, QPL_STS_OK);

                ASSERT_TRUE(CompareVectors(reference, source));
                qpl_fini_job(compression_job_ptr);
                qpl_fini_job(decompression_job_ptr);
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, canned_high_stateless) {
    auto compression_execution_path   = qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_hardware, "HW path dictionary decompression is not supported for canned mode");

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            for (auto dictionary_length : get_dictionary_lengths()) {

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                std::vector<uint8_t> destination(source.size() * 2);
                std::vector<uint8_t> reference(source.size(), 0);

                // Create and fill the compression table
                const unique_huffman_table c_table(
                        deflate_huffman_table_maker(compression_table_type, compression_execution_path,
                                                    DEFAULT_ALLOCATOR_C),
                        any_huffman_table_deleter);
                ASSERT_NE(c_table.get(), nullptr) << "Huffman Table creation failed\n";

                qpl_histogram deflate_histogram {};
                // Build the table
                auto status = qpl_gather_deflate_statistics(source.data(), static_cast<uint32_t>(source.size()),
                                                            &deflate_histogram, qpl_default_level,
                                                            compression_execution_path);
                ASSERT_EQ(status, QPL_STS_OK) << "Statistics gathering failed";

                status = qpl_huffman_table_init_with_histogram(c_table.get(), &deflate_histogram);
                ASSERT_EQ(status, QPL_STS_OK) << "Table init failed";

                uint32_t compression_job_size   = 0;
                uint32_t decompression_job_size = 0;

                status = qpl_get_job_size(compression_execution_path, &compression_job_size);
                ASSERT_EQ(QPL_STS_OK, status);

                status = qpl_get_job_size(decompression_execution_path, &decompression_job_size);
                ASSERT_EQ(QPL_STS_OK, status);

                // Allocate buffers for compression and decompression jobs

                auto compression_job_buffer = std::make_unique<uint8_t[]>(compression_job_size);
                auto compression_job_ptr    = reinterpret_cast<qpl_job*>(compression_job_buffer.get());

                auto decompression_job_buffer = std::make_unique<uint8_t[]>(decompression_job_size);
                auto decompression_job_ptr    = reinterpret_cast<qpl_job*>(decompression_job_buffer.get());

                // Initialize compression and decompression jobs

                status = qpl_init_job(compression_execution_path, compression_job_ptr);
                ASSERT_EQ(QPL_STS_OK, status);

                status = qpl_init_job(decompression_execution_path, decompression_job_ptr);
                ASSERT_EQ(QPL_STS_OK, status);

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                              dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                // Create and fill the decompression table
                const unique_huffman_table d_table(
                        deflate_huffman_table_maker(decompression_table_type, decompression_execution_path,
                                                    DEFAULT_ALLOCATOR_C),
                        any_huffman_table_deleter);
                ASSERT_NE(d_table.get(), nullptr) << "Huffman Table creation failed\n";

                status = qpl_huffman_table_init_with_other(d_table.get(), c_table.get());
                ASSERT_EQ(QPL_STS_OK, status);

                // Compress
                compression_job_ptr->op = qpl_op_compress;
                compression_job_ptr->flags =
                        QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE | QPL_FLAG_OMIT_VERIFY;
                compression_job_ptr->available_in  = static_cast<uint32_t>(source.size());
                compression_job_ptr->next_in_ptr   = source.data();
                compression_job_ptr->available_out = static_cast<uint32_t>(destination.size());
                compression_job_ptr->next_out_ptr  = destination.data();
                compression_job_ptr->dictionary    = dictionary_ptr;
                compression_job_ptr->level         = qpl_compression_levels::qpl_high_level;
                compression_job_ptr->huffman_table = c_table.get();

                status = run_job_api(compression_job_ptr);
                ASSERT_EQ(status, QPL_STS_OK);

                decompression_job_ptr->op            = qpl_op_decompress;
                decompression_job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE;
                decompression_job_ptr->available_in  = compression_job_ptr->total_out;
                decompression_job_ptr->next_in_ptr   = destination.data();
                decompression_job_ptr->available_out = static_cast<uint32_t>(reference.size());
                decompression_job_ptr->next_out_ptr  = reference.data();
                decompression_job_ptr->dictionary    = dictionary_ptr;
                decompression_job_ptr->huffman_table = d_table.get();

                status = run_job_api(decompression_job_ptr);
                ASSERT_EQ(status, QPL_STS_OK);

                ASSERT_TRUE(CompareVectors(reference, source));

                qpl_fini_job(compression_job_ptr);
                qpl_fini_job(decompression_job_ptr);
            }
        }
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, canned_high_stateful) {
    auto compression_execution_path   = qpl_path_software;
    auto decompression_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_hardware, "HW path dictionary decompression is not supported for canned mode");

    uint32_t             num_iterations = 0;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (compression_execution_path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    const auto seed = util::TestEnvironment::GetInstance().GetSeed();

    for (uint32_t i = 0; i < num_iterations; i++) {

        if (compression_execution_path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

            std::vector<uint8_t> source;
            source = dataset.second;
            for (auto dictionary_length : get_dictionary_lengths()) {

                if (dictionary_length > 4096) { dictionary_length = static_cast<uint32_t>(source.size()); }

                // Create and fill the compression table
                const unique_huffman_table c_table(
                        deflate_huffman_table_maker(compression_table_type, compression_execution_path,
                                                    DEFAULT_ALLOCATOR_C),
                        any_huffman_table_deleter);
                ASSERT_NE(c_table.get(), nullptr) << "Huffman Table creation failed\n";

                qpl_histogram deflate_histogram {};
                // Build the table
                auto status = qpl_gather_deflate_statistics(source.data(), static_cast<uint32_t>(source.size()),
                                                            &deflate_histogram, qpl_default_level,
                                                            compression_execution_path);
                ASSERT_EQ(status, QPL_STS_OK) << "Statistics gathering failed";

                status = qpl_huffman_table_init_with_histogram(c_table.get(), &deflate_histogram);
                ASSERT_EQ(status, QPL_STS_OK) << "Table init failed";

                std::vector<uint8_t> destination(source.size() * 2);
                std::vector<uint8_t> reference(source.size(), 0);

                uint32_t compression_job_size   = 0;
                uint32_t decompression_job_size = 0;

                status = qpl_get_job_size(compression_execution_path, &compression_job_size);
                ASSERT_EQ(QPL_STS_OK, status);

                status = qpl_get_job_size(decompression_execution_path, &decompression_job_size);
                ASSERT_EQ(QPL_STS_OK, status);

                // Allocate buffers for compression and decompression jobs

                auto compression_job_buffer = std::make_unique<uint8_t[]>(compression_job_size);
                auto compression_job_ptr    = reinterpret_cast<qpl_job*>(compression_job_buffer.get());

                auto decompression_job_buffer = std::make_unique<uint8_t[]>(decompression_job_size);
                auto decompression_job_ptr    = reinterpret_cast<qpl_job*>(decompression_job_buffer.get());

                // Initialize compression and decompression jobs

                status = qpl_init_job(compression_execution_path, compression_job_ptr);
                ASSERT_EQ(QPL_STS_OK, status);

                status = qpl_init_job(decompression_execution_path, decompression_job_ptr);
                ASSERT_EQ(QPL_STS_OK, status);

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                              dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status);

                // Create and fill the decompression table
                const unique_huffman_table d_table(
                        deflate_huffman_table_maker(decompression_table_type, decompression_execution_path,
                                                    DEFAULT_ALLOCATOR_C),
                        any_huffman_table_deleter);
                ASSERT_NE(d_table.get(), nullptr) << "Huffman Table creation failed\n";

                status = qpl_huffman_table_init_with_other(d_table.get(), c_table.get());
                ASSERT_EQ(QPL_STS_OK, status);

                // Compress
                compression_job_ptr->op            = qpl_op_compress;
                compression_job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_CANNED_MODE | QPL_FLAG_OMIT_VERIFY;
                compression_job_ptr->available_out = static_cast<uint32_t>(destination.size());
                compression_job_ptr->next_out_ptr  = destination.data();
                compression_job_ptr->dictionary    = dictionary_ptr;
                compression_job_ptr->level         = qpl_compression_levels::qpl_high_level;
                compression_job_ptr->huffman_table = c_table.get();

                qpl::test::random random(0U, 0U, seed);

                random.set_range(1000U, 10000U);
                const auto chunk_size         = static_cast<uint32_t>(random);
                uint32_t   current_chunk_size = chunk_size;

                uint32_t iteration_count   = 0;
                auto     source_bytes_left = static_cast<uint32_t>(source.size());
                while (source_bytes_left > 0) {
                    if (current_chunk_size >= source_bytes_left) {
                        compression_job_ptr->flags |= QPL_FLAG_LAST;
                        if (current_chunk_size > source_bytes_left) {
                            current_chunk_size = static_cast<uint32_t>(source.size()) % chunk_size;
                        }
                    }

                    source_bytes_left -= current_chunk_size;
                    compression_job_ptr->next_in_ptr  = source.data() + iteration_count * chunk_size;
                    compression_job_ptr->available_in = current_chunk_size;
                    status                            = run_job_api(compression_job_ptr);
                    ASSERT_EQ(status, QPL_STS_OK);

                    compression_job_ptr->flags &= ~QPL_FLAG_FIRST;
                    iteration_count++;
                }

                decompression_job_ptr->op            = qpl_op_decompress;
                decompression_job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE;
                decompression_job_ptr->available_in  = compression_job_ptr->total_out;
                decompression_job_ptr->next_in_ptr   = destination.data();
                decompression_job_ptr->available_out = static_cast<uint32_t>(reference.size());
                decompression_job_ptr->next_out_ptr  = reference.data();
                decompression_job_ptr->dictionary    = dictionary_ptr;
                decompression_job_ptr->huffman_table = d_table.get();

                status = run_job_api(decompression_job_ptr);
                ASSERT_EQ(status, QPL_STS_OK);

                ASSERT_TRUE(CompareVectors(reference, source));

                qpl_fini_job(compression_job_ptr);
                qpl_fini_job(decompression_job_ptr);
            }
        }
    }
}
} // namespace qpl::test
