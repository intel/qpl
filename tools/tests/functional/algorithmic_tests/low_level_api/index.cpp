/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tests
 */

#include <map>

// tests_common
#include "operation_test.hpp"
#include "test_cases.hpp"

#include "ta_ll_common.hpp"
#include "source_provider.hpp"

namespace qpl::test {
enum CompressionMode {
    SINGLE_BUF_FIXED,
    SINGLE_BUF_DYNAMIC,
    MULT_BUF_FIXED,
    MULT_BUF_DYNAMIC
};

enum BlockUsage {
    SINGLE_BLOCK,
    MULTIPLE_BLOCKS
};

struct IndexTestCase {
    BlockUsage          block_usage;
    CompressionMode     compression_mode;
    qpl_mini_block_size mini_block_size;
    uint32_t            chunk_size;
    bool                gzip_mode;
    std::string         file_name;
};

std::ostream &operator<<(std::ostream &os, const IndexTestCase &test_case) {
    std::string block_usage;
    std::string compression_mode;
    std::string header = (test_case.gzip_mode) ? "Gzip" : "No header";

    if (SINGLE_BLOCK == test_case.block_usage) {
        block_usage = "Single block";
    } else if (MULTIPLE_BLOCKS == test_case.block_usage) {
        block_usage = "Multiple blocks";
    }

    if (SINGLE_BUF_FIXED == test_case.compression_mode) {
        compression_mode = "Single buffer fixed";
    } else if (SINGLE_BUF_DYNAMIC == test_case.compression_mode) {
        compression_mode = "Single buffer dynamic";
    } else if (MULT_BUF_FIXED == test_case.compression_mode) {
        compression_mode = "Multiple buffer fixed";
    } else if (MULT_BUF_DYNAMIC == test_case.compression_mode) {
        compression_mode = "Multiple buffer dynamic";
    }

    os << "Header type: " << header << "\n";
    os << "Block usage: " << block_usage << "\n";
    os << "Compression mode: " << compression_mode << "\n";
    os << "Chunk size: " << test_case.chunk_size << "\n";
    os << "File : " << test_case.file_name << "\n";
    return os;
}

class IndexTest : public JobFixtureWithTestCases<IndexTestCase> {
public:
    static void SetUpTestSuite() {
    }

    static uint32_t UpdateCRC(uint32_t seed, uint8_t byte) {
        uint64_t rem;
        uint32_t cvt = ~seed;
        rem = cvt;
        const uint32_t polynomial = 0xEDB88320; // IEEE standard

        rem = rem ^ byte;

        for (uint32_t i = 0; i < 8U; i++) {
            rem = (rem & 0x1ULL) ? (rem >> 1) ^ polynomial : (rem >> 1);
        }

        return (uint32_t) ~rem;
    }

    static uint32_t NumberOfIndexValues(uint32_t buffer_size, uint32_t mini_block_size, uint32_t block_size) {
        uint32_t result;

        result = (buffer_size + mini_block_size - 1) / mini_block_size;

        if (block_size == 0) {
            result += 2;
        } else {
            result += 2 * ((buffer_size + block_size - 1) / block_size);
        }

        return result + 1;
    }

    testing::AssertionResult GetMiniblock(uint32_t required_number_of_mblocks,
                                          uint32_t mblocks_per_block,
                                          std::vector<uint8_t> &block_buffer) {
        qpl_job    *inflate_job_ptr;
        uint32_t   job_size;
        qpl_status status = qpl_get_job_size(GetExecutionPath(), &job_size);

        if (status != QPL_STS_OK) {
            return testing::AssertionFailure() << "Couldn't get job size";
        }

        auto job_buffer = std::make_unique<uint8_t[]>(job_size);
        inflate_job_ptr = reinterpret_cast<qpl_job *>(job_buffer.get());
        qpl_init_job(GetExecutionPath(), inflate_job_ptr);

        inflate_job_ptr->op = qpl_op_decompress;
        uint32_t block_number;
        uint32_t block_header;
        uint32_t bit_start;
        uint32_t bit_end;
        uint32_t mini_blocks_in_block;

        uint64_t header_index_start;
        uint64_t header_index_finish;

        if (SINGLE_BLOCK == current_test_case.block_usage) {
            // Current compression is SINGLE_BLOCK
            block_number         = 0; // For single block compression the block number is always 0
            mini_blocks_in_block = required_number_of_mblocks;
        } else {
            block_number         = required_number_of_mblocks / mblocks_per_block;
            mini_blocks_in_block = required_number_of_mblocks % mblocks_per_block;
        }

        block_header = block_number * (mblocks_per_block + 2);

        header_index_start  = index_array[block_header];
        header_index_finish = index_array[block_header + 1];

        bit_start = (uint32_t) header_index_start;
        bit_end   = (uint32_t) header_index_finish;

        uint8_t *start = destination.data() + bit_start / 8;

        // FIRST | RND_ACCESS means that Intel QPL should only read block header
        inflate_job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_RND_ACCESS;

        inflate_job_ptr->ignore_start_bits = bit_start & 7;
        inflate_job_ptr->ignore_end_bits   = 7 & (0 - bit_end);
        inflate_job_ptr->available_in      = ((bit_end + 7) / 8) - (bit_start / 8);
        inflate_job_ptr->next_out_ptr      = block_buffer.data();
        inflate_job_ptr->available_out     = (uint32_t) block_buffer.size();
        inflate_job_ptr->next_in_ptr       = start;

        qpl_status decompression_status = run_job_api(inflate_job_ptr);

        if (decompression_status != QPL_STS_OK) {
            return testing::AssertionFailure() << "Restoring mini-block qpl_inflate failed with status "
                                               << decompression_status;
        }

        inflate_job_ptr->flags = QPL_FLAG_RND_ACCESS;
        uint32_t mblock_start_index = (1 + block_number * (mblocks_per_block + 2) + mini_blocks_in_block);
        bit_start = (uint32_t) (index_array[mblock_start_index]);
        bit_end   = (uint32_t) (index_array[mblock_start_index + 1]);

        inflate_job_ptr->next_in_ptr       = destination.data() + bit_start / 8;
        inflate_job_ptr->ignore_start_bits = bit_start & 7;
        inflate_job_ptr->ignore_end_bits   = 7 & (0 - bit_end);
        inflate_job_ptr->available_in      = ((bit_end + 7) / 8) - (bit_start / 8);
        inflate_job_ptr->crc               = (uint32_t) ((index_array[mblock_start_index]) >> 32);
        inflate_job_ptr->next_out_ptr      = block_buffer.data();
        inflate_job_ptr->available_out     = (uint32_t) block_buffer.size();

        auto required_crc = (uint32_t) ((index_array[mblock_start_index + 1]) >> 32);

        decompression_status = run_job_api(inflate_job_ptr);

        if (decompression_status != QPL_STS_OK) {
            return testing::AssertionFailure() << "Mini-block decompression failure, status "
                                               << decompression_status;
        }

        if (required_crc != inflate_job_ptr->crc) {
            return testing::AssertionFailure() << "Mini-block CRC is wrong, req " << required_crc
                                               << " got " << inflate_job_ptr->crc;
        }

        return testing::AssertionSuccess();
    }

    std::vector<uint64_t> index_array;
    std::vector<uint64_t> crc_array;
    IndexTestCase         current_test_case;

    static std::map<std::string, std::vector<uint8_t>> calgary_sources;

    void SetUpBeforeIteration() override {
        current_test_case = GetTestCase();

        auto dataset = util::TestEnvironment::GetInstance().GetAlgorithmicDataset();

        std::vector<uint8_t> source_ptr = dataset[current_test_case.file_name];

        destination.resize(source_ptr.size() * 2);

        job_ptr->next_in_ptr  = source_ptr.data();
        job_ptr->available_in = static_cast<uint32_t>(source_ptr.size());

        job_ptr->next_out_ptr  = destination.data();
        job_ptr->available_out = static_cast<uint32_t>(destination.size());

        job_ptr->op              = qpl_op_compress;
        job_ptr->mini_block_size = current_test_case.mini_block_size;

        uint32_t required_indexes = NumberOfIndexValues(job_ptr->available_in,
                                                        (1U << (job_ptr->mini_block_size + 8U)),
                                                        (current_test_case.chunk_size == 0)
                                                        ? job_ptr->available_in
                                                        : current_test_case.chunk_size);

        index_array.resize(required_indexes * 20);
        crc_array.resize(required_indexes * 20);

        job_ptr->idx_array    = (uint64_t *) index_array.data();
        job_ptr->idx_max_size = static_cast<uint32_t>(index_array.size());
    }

    void InitializeTestCases() override {
        uint32_t block_usage = SINGLE_BLOCK;

        // All single block test cases
        for (uint32_t mini_block_size = qpl_mblk_size_512; mini_block_size <= qpl_mblk_size_32k; mini_block_size++) {
            for (uint32_t compression_mode = SINGLE_BUF_FIXED;
                 compression_mode <= SINGLE_BUF_DYNAMIC; compression_mode++) {
                for (uint32_t gzip_mode = 0; gzip_mode < 2; gzip_mode++) {
                    for (auto &dataset: util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
                        IndexTestCase test_case;
                        test_case.block_usage      = (BlockUsage) block_usage;
                        test_case.chunk_size       = 0;
                        test_case.mini_block_size  = (qpl_mini_block_size) mini_block_size;
                        test_case.compression_mode = (CompressionMode) compression_mode;
                        test_case.gzip_mode        = (gzip_mode) != 0;
                        test_case.file_name        = dataset.first;

                        AddNewTestCase(test_case);
                    }
                }
            }
        }

        block_usage = MULTIPLE_BLOCKS;
        uint32_t mini_block_size = qpl_mblk_size_512;

        // All multiple blocks test cases
        for (uint32_t chunk_size : {512U, 1024U, 1024U * 32U}) {
            for (uint32_t compression_mode = MULT_BUF_FIXED;
                 compression_mode <= MULT_BUF_DYNAMIC; compression_mode++) {
                for (uint32_t gzip_mode = 0; gzip_mode < 2; gzip_mode++) {
                    for (auto &dataset: util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
                        IndexTestCase test_case;
                        test_case.block_usage      = (BlockUsage) block_usage;
                        test_case.chunk_size       = chunk_size;
                        test_case.mini_block_size  = (qpl_mini_block_size) mini_block_size;
                        test_case.compression_mode = (CompressionMode) compression_mode;
                        test_case.gzip_mode        = (gzip_mode) != 0;
                        test_case.file_name        = dataset.first;

                        AddNewTestCase(test_case);
                    }
                }
            }
        }
    }
};

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(deflate_index_extended, PerformOperation, IndexTest) {
    auto dataset = util::TestEnvironment::GetInstance().GetAlgorithmicDataset();
    std::vector<uint8_t> source  = dataset[current_test_case.file_name];
    uint32_t             input_size  = static_cast<uint32_t>(source.size());

    uint32_t bytes_remain          = input_size;
    uint32_t chunk_size            = 0;
    uint32_t mini_block_size_bytes = (1U << (job_ptr->mini_block_size + 8U));

    job_ptr->next_in_ptr = source.data();
    job_ptr->available_in = input_size;

    job_ptr->level = qpl_default_level;
    job_ptr->flags = QPL_FLAG_FIRST;

    while (bytes_remain > 0) {
        switch (current_test_case.compression_mode) {
            case SINGLE_BUF_FIXED:
                chunk_size = bytes_remain;
                break;

            case SINGLE_BUF_DYNAMIC:
                job_ptr->flags |= QPL_FLAG_DYNAMIC_HUFFMAN;
                chunk_size = bytes_remain;
                break;

            case MULT_BUF_FIXED:
                chunk_size = current_test_case.chunk_size;
                job_ptr->flags |= QPL_FLAG_START_NEW_BLOCK;
                break;

            case MULT_BUF_DYNAMIC:
                chunk_size = current_test_case.chunk_size;
                job_ptr->flags |= QPL_FLAG_START_NEW_BLOCK;
                job_ptr->flags |= QPL_FLAG_DYNAMIC_HUFFMAN;
                break;
        }

        if (current_test_case.gzip_mode == 1) {
            job_ptr->flags |= QPL_FLAG_GZIP_MODE;
        }

        if (chunk_size >= bytes_remain) {
            chunk_size = bytes_remain;
            job_ptr->flags |= QPL_FLAG_LAST;
        }

        job_ptr->available_in = chunk_size;

        auto result = run_job_api(job_ptr);

        ASSERT_EQ(QPL_STS_OK, result);

        bytes_remain -= chunk_size;
        job_ptr->flags &= ~QPL_FLAG_FIRST;
    }

    auto     source_it = source.begin();
    uint32_t crc_value = 0;
    uint32_t crc_next;

    for (crc_next = 0; crc_next <= input_size / mini_block_size_bytes; crc_next++) {
        for (uint32_t next_byte = 0; next_byte < mini_block_size_bytes; next_byte++) {
            if (source_it >= source.end()) { // End of input buffer
                break;
            }
            crc_value = UpdateCRC(crc_value, *source_it++);
        }

        crc_array[crc_next] = crc_value;
    }

    uint32_t current_number_index = NumberOfIndexValues(input_size,
                                                        mini_block_size_bytes,
                                                        (current_test_case.chunk_size == 0)
                                                        ? job_ptr->available_in
                                                        : current_test_case.chunk_size);

    EXPECT_EQ(current_number_index, job_ptr->idx_num_written);

    uint64_t previous_crc = 0;
    crc_next = 0;

    for (uint32_t crc_number = 0; crc_number < job_ptr->idx_num_written; crc_number++) {
        uint64_t qpl_crc = (index_array[crc_number] >> 32) & 0xFFFFFFFF;

        if (qpl_crc == previous_crc) {
            continue;
        }

        ASSERT_EQ(qpl_crc, crc_array[crc_next]);

        previous_crc = crc_array[crc_next++];
    }

    uint32_t block_size = (current_test_case.chunk_size == 0) ? input_size
                                                              : current_test_case.chunk_size;

    uint32_t number_of_mini_blocks = (input_size + mini_block_size_bytes - 1) / mini_block_size_bytes;
    uint32_t mini_blocks_per_block = block_size / mini_block_size_bytes;

    for (uint32_t current_mini_block = 0; current_mini_block < number_of_mini_blocks; current_mini_block++) {
        uint32_t             next_mini_block     = current_mini_block;
        std::vector<uint8_t> restored_mini_block;
        restored_mini_block.resize(mini_block_size_bytes * 2);

        ASSERT_TRUE(GetMiniblock(next_mini_block, mini_blocks_per_block, restored_mini_block));
    }
}
}
