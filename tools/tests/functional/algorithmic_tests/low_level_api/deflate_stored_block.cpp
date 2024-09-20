/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <array>
#include <numeric>
#include <string>
#include <vector>

#include "qpl/qpl.h"

#include "gtest/gtest.h"
#include "ta_ll_common.hpp"

// tests_common
#include "compression_huffman_table.hpp"
#include "huffman_table_unique.hpp"
#include "operation_test.hpp"
#include "source_provider.hpp"
#include "util.hpp"

namespace qpl::test {

constexpr uint32_t small_input_data_size    = 256U;
constexpr uint32_t large_input_data_size    = 150000U;
constexpr uint32_t stored_block_header_size = 5U;
constexpr uint32_t max_stored_block_size    = 0xFFFFU;

class StoredBlockTest : public JobFixture {
public:
    template <uint32_t input_size>
    void dynamic_compression_failed_test(qpl_compression_levels level) {
        if (GetExecutionPath() == qpl_path_hardware && level == qpl_high_level) {
            if (0 == JobFixture::num_test++) {
                GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path";
            }
            return;
        }
        constexpr uint32_t number_of_stored_blocks = (input_size + max_stored_block_size - 1) / max_stored_block_size;
        constexpr uint32_t expected_size           = input_size + stored_block_header_size * number_of_stored_blocks;

        std::vector<uint8_t> source;
        std::vector<uint8_t> destination(expected_size);

        source_provider source_gen(input_size, 8U, GetSeed());
        ASSERT_NO_THROW(source = source_gen.get_source()); //NOLINT(cppcoreguidelines-avoid-goto)

        job_ptr->op            = qpl_op_compress;
        job_ptr->next_in_ptr   = source.data();
        job_ptr->available_in  = input_size;
        job_ptr->next_out_ptr  = destination.data();
        job_ptr->available_out = expected_size;
        job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DYNAMIC_HUFFMAN;
        job_ptr->level         = level;

        ASSERT_EQ(run_job_api(job_ptr), QPL_STS_OK);

        auto total_byte_to_compare = input_size;

        auto stored_block_begin = destination.data() + stored_block_header_size;
        auto reference          = source.data();

        for (uint32_t i = 0; i < number_of_stored_blocks; i++) {
            auto bytes_to_compare = std::min(total_byte_to_compare, max_stored_block_size);
            auto stored_block_end = stored_block_begin + bytes_to_compare;

            ASSERT_TRUE(CompareSegments(stored_block_begin, stored_block_end, reference, reference + bytes_to_compare,
                                        "Stored block index: " + std::to_string(i)));

            reference += bytes_to_compare;
            stored_block_begin += bytes_to_compare + stored_block_header_size;
            total_byte_to_compare -= bytes_to_compare;
        }
    }

    template <uint32_t input_size>
    void fixed_compression_failed_test(qpl_compression_levels level) {
        if (GetExecutionPath() == qpl_path_hardware && level == qpl_high_level) {
            if (0 == JobFixture::num_test++) {
                GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path";
            }
            return;
        }
        constexpr uint32_t number_of_stored_blocks = (input_size + max_stored_block_size - 1) / max_stored_block_size;
        constexpr uint32_t expected_size           = input_size + stored_block_header_size * number_of_stored_blocks;

        std::vector<uint8_t> source;
        std::vector<uint8_t> destination(expected_size);

        source_provider source_gen(input_size, 8U, GetSeed());
        ASSERT_NO_THROW(source = source_gen.get_source()); //NOLINT(cppcoreguidelines-avoid-goto)

        job_ptr->op            = qpl_op_compress;
        job_ptr->next_in_ptr   = source.data();
        job_ptr->available_in  = input_size;
        job_ptr->next_out_ptr  = destination.data();
        job_ptr->available_out = expected_size;
        job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST;
        job_ptr->level         = level;

        ASSERT_EQ(run_job_api(job_ptr), QPL_STS_OK);

        auto total_byte_to_compare = input_size;

        auto stored_block_begin = destination.data() + stored_block_header_size;
        auto reference          = source.data();

        for (uint32_t i = 0; i < number_of_stored_blocks; i++) {
            auto bytes_to_compare = std::min(total_byte_to_compare, max_stored_block_size);
            auto stored_block_end = stored_block_begin + bytes_to_compare;

            ASSERT_TRUE(CompareSegments(stored_block_begin, stored_block_end, reference, reference + bytes_to_compare,
                                        "Stored block index: " + std::to_string(i)));

            reference += bytes_to_compare;
            stored_block_begin += bytes_to_compare + stored_block_header_size;
            total_byte_to_compare -= bytes_to_compare;
        }
    }

    template <uint32_t input_size>
    void static_compression_failed_test(qpl_compression_levels level) {
        if (GetExecutionPath() == qpl_path_hardware && level == qpl_high_level) {
            if (0 == JobFixture::num_test++) {
                GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path";
            }
            return;
        }
        constexpr uint32_t number_of_stored_blocks = (input_size + max_stored_block_size - 1) / max_stored_block_size;
        constexpr uint32_t expected_size           = input_size + stored_block_header_size * number_of_stored_blocks;

        std::vector<uint8_t> source;
        std::vector<uint8_t> destination(expected_size);

        source_provider source_gen(input_size, 8U, GetSeed());
        ASSERT_NO_THROW(source = source_gen.get_source()); //NOLINT(cppcoreguidelines-avoid-goto)

        // Create and initialize compression table
        const unique_huffman_table c_table(
                deflate_huffman_table_maker(compression_table_type, GetExecutionPath(), DEFAULT_ALLOCATOR_C),
                any_huffman_table_deleter);
        ASSERT_NE(c_table.get(), nullptr) << "Compression Huffman Table creation failed\n";

        qpl_status status = fill_compression_table(c_table.get());
        ASSERT_EQ(status, QPL_STS_OK) << "Compression table failed to be filled\n";

        job_ptr->huffman_table = c_table.get();

        job_ptr->op            = qpl_op_compress;
        job_ptr->next_in_ptr   = source.data();
        job_ptr->available_in  = input_size;
        job_ptr->next_out_ptr  = destination.data();
        job_ptr->available_out = expected_size;
        job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST;
        job_ptr->level         = level;

        status = run_job_api(job_ptr);
        ASSERT_EQ(status, QPL_STS_OK) << "Compression job failed\n";

        auto total_byte_to_compare = input_size;

        auto stored_block_begin = destination.data() + stored_block_header_size;
        auto reference          = source.data();

        for (uint32_t i = 0; i < number_of_stored_blocks; i++) {
            auto bytes_to_compare = std::min(total_byte_to_compare, max_stored_block_size);
            auto stored_block_end = stored_block_begin + bytes_to_compare;

            const bool compare_segments_result =
                    CompareSegments(stored_block_begin, stored_block_end, reference, reference + bytes_to_compare,
                                    "Stored block index: " + std::to_string(i));
            ASSERT_TRUE(compare_segments_result);

            reference += bytes_to_compare;
            stored_block_begin += bytes_to_compare + stored_block_header_size;
            total_byte_to_compare -= bytes_to_compare;
        }
    }

    template <uint32_t input_size>
    void canned_compression_no_stored_block_check_test(qpl_compression_levels level) {
        if (GetExecutionPath() == qpl_path_hardware && level == qpl_high_level) {
            if (0 == JobFixture::num_test++) {
                GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path";
            }
            return;
        }
        constexpr uint32_t number_of_stored_blocks = (input_size + max_stored_block_size - 1) / max_stored_block_size;
        constexpr uint32_t expected_size           = input_size + stored_block_header_size * number_of_stored_blocks;

        std::vector<uint8_t> source;
        std::vector<uint8_t> destination(expected_size);

        source_provider source_gen(input_size, 8U, GetSeed());
        ASSERT_NO_THROW(source = source_gen.get_source()); //NOLINT(cppcoreguidelines-avoid-goto)

        // Create and initialize compression table
        const unique_huffman_table c_table(
                deflate_huffman_table_maker(compression_table_type, GetExecutionPath(), DEFAULT_ALLOCATOR_C),
                any_huffman_table_deleter);
        ASSERT_NE(c_table.get(), nullptr) << "Compression Huffman Table creation failed\n";

        const qpl_status status = fill_compression_table(c_table.get());
        ASSERT_EQ(status, QPL_STS_OK) << "Compression table failed to be filled";

        job_ptr->huffman_table = c_table.get();

        job_ptr->op            = qpl_op_compress;
        job_ptr->next_in_ptr   = source.data();
        job_ptr->available_in  = input_size;
        job_ptr->next_out_ptr  = destination.data();
        job_ptr->available_out = expected_size;
        job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_CANNED_MODE | QPL_FLAG_LAST;
        job_ptr->level         = level;

        EXPECT_EQ(run_job_api(job_ptr), QPL_STS_MORE_OUTPUT_NEEDED);
    }

    template <uint32_t input_size>
    void static_overflow_check_test(qpl_compression_levels level) {
        constexpr uint32_t number_of_stored_blocks = (input_size + max_stored_block_size - 1) / max_stored_block_size;
        constexpr uint32_t expected_size           = input_size + stored_block_header_size * number_of_stored_blocks;

        std::vector<uint8_t> source;
        std::vector<uint8_t> destination(expected_size - 1U - (number_of_stored_blocks - 1) * max_stored_block_size);

        source_provider source_gen(input_size, 8U, GetSeed());
        ASSERT_NO_THROW(source = source_gen.get_source()); //NOLINT(cppcoreguidelines-avoid-goto)

        // Create and initialize compression table
        const unique_huffman_table c_table(
                deflate_huffman_table_maker(compression_table_type, GetExecutionPath(), DEFAULT_ALLOCATOR_C),
                any_huffman_table_deleter);
        ASSERT_NE(c_table.get(), nullptr) << "Compression Huffman Table creation failed\n";

        const qpl_status status = fill_compression_table(c_table.get());
        ASSERT_EQ(status, QPL_STS_OK) << "Compression table failed to be filled";

        job_ptr->huffman_table = c_table.get();

        job_ptr->op            = qpl_op_compress;
        job_ptr->next_in_ptr   = source.data();
        job_ptr->available_in  = input_size;
        job_ptr->next_out_ptr  = destination.data();
        job_ptr->available_out = static_cast<uint32_t>(destination.size());
        job_ptr->level         = level;
        job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST;
        job_ptr->level         = level;

        EXPECT_EQ(run_job_api(job_ptr), QPL_STS_MORE_OUTPUT_NEEDED);
    }

    template <uint32_t input_size>
    void dynamic_overflow_check_test(qpl_compression_levels level) {
        constexpr uint32_t number_of_stored_blocks = (input_size + max_stored_block_size - 1) / max_stored_block_size;
        constexpr uint32_t expected_size           = input_size + stored_block_header_size * number_of_stored_blocks;

        std::vector<uint8_t> source;
        std::vector<uint8_t> destination(expected_size - 1U - (number_of_stored_blocks - 1) * max_stored_block_size);

        source_provider source_gen(input_size, 8U, GetSeed());
        ASSERT_NO_THROW(source = source_gen.get_source()); //NOLINT(cppcoreguidelines-avoid-goto)

        job_ptr->op            = qpl_op_compress;
        job_ptr->next_in_ptr   = source.data();
        job_ptr->available_in  = input_size;
        job_ptr->next_out_ptr  = destination.data();
        job_ptr->available_out = static_cast<uint32_t>(destination.size());
        job_ptr->level         = level;
        job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DYNAMIC_HUFFMAN;
        job_ptr->level         = level;

        EXPECT_EQ(run_job_api(job_ptr), QPL_STS_MORE_OUTPUT_NEEDED);
    }
};

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, small_dynamic_default_compression_failed, StoredBlockTest) {
    dynamic_compression_failed_test<small_input_data_size>(qpl_default_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, small_dynamic_high_compression_failed, StoredBlockTest) {
    dynamic_compression_failed_test<small_input_data_size>(qpl_high_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, small_fixed_default_compression_failed, StoredBlockTest) {
    fixed_compression_failed_test<small_input_data_size>(qpl_default_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, large_fixed_default_compression_failed, StoredBlockTest) {
    fixed_compression_failed_test<large_input_data_size>(qpl_default_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, small_fixed_high_compression_failed, StoredBlockTest) {
    fixed_compression_failed_test<small_input_data_size>(qpl_high_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, large_fixed_high_compression_failed, StoredBlockTest) {
    fixed_compression_failed_test<large_input_data_size>(qpl_high_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, small_static_default_compression_failed, StoredBlockTest) {
    static_compression_failed_test<small_input_data_size>(qpl_default_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, large_static_default_compression_failed, StoredBlockTest) {
    static_compression_failed_test<large_input_data_size>(qpl_default_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, small_static_high_compression_failed, StoredBlockTest) {
    static_compression_failed_test<small_input_data_size>(qpl_high_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, large_static_high_compression_failed, StoredBlockTest) {
    static_compression_failed_test<large_input_data_size>(qpl_high_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, small_canned_default_compression_no_stored_block_check,
                                     StoredBlockTest) {
    canned_compression_no_stored_block_check_test<small_input_data_size>(qpl_default_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, large_canned_default_compression_no_stored_block_check,
                                     StoredBlockTest) {
    canned_compression_no_stored_block_check_test<large_input_data_size>(qpl_default_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, small_canned_high_compression_no_stored_block_check,
                                     StoredBlockTest) {
    canned_compression_no_stored_block_check_test<small_input_data_size>(qpl_high_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, large_canned_high_compression_no_stored_block_check,
                                     StoredBlockTest) {
    canned_compression_no_stored_block_check_test<large_input_data_size>(qpl_high_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, small_dynamic_overflow_check_default_level,
                                     StoredBlockTest) {
    dynamic_overflow_check_test<small_input_data_size>(qpl_default_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, large_dynamic_overflow_check_default_level,
                                     StoredBlockTest) {
    dynamic_overflow_check_test<large_input_data_size>(qpl_default_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, small_dynamic_overflow_check_high_level, StoredBlockTest) {
    if (GetExecutionPath() == qpl_path_hardware) {
        if (0 == StoredBlockTest::num_test++) {
            GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path";
        }
        return;
    }
    dynamic_overflow_check_test<small_input_data_size>(qpl_high_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, large_dynamic_overflow_check_high_level, StoredBlockTest) {
    if (GetExecutionPath() == qpl_path_hardware) {
        if (0 == StoredBlockTest::num_test++) {
            GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path";
        }
        return;
    }
    dynamic_overflow_check_test<large_input_data_size>(qpl_high_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, small_static_overflow_check_default_level, StoredBlockTest) {
    static_overflow_check_test<small_input_data_size>(qpl_default_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, large_static_overflow_check_default_level, StoredBlockTest) {
    static_overflow_check_test<large_input_data_size>(qpl_default_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, small_static_overflow_check_high_level, StoredBlockTest) {
    if (GetExecutionPath() == qpl_path_hardware) {
        if (0 == StoredBlockTest::num_test++) {
            GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path";
        }
        return;
    }
    static_overflow_check_test<small_input_data_size>(qpl_high_level);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_stored_block, large_static_overflow_check_high_level, StoredBlockTest) {
    if (GetExecutionPath() == qpl_path_hardware) {
        if (0 == StoredBlockTest::num_test++) {
            GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path";
        }
        return;
    }
    static_overflow_check_test<large_input_data_size>(qpl_high_level);
}

} // namespace qpl::test
