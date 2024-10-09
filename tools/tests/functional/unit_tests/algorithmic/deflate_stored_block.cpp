/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/qpl.h"

#include "t_common.hpp"

// tool_hw_dispatcher
#include "test_hw_device.hpp"

// tool_common
#include "huffman_table_unique.hpp"
#include "system_info.hpp"
#include "topology.hpp"
#include "util.hpp"

// tests_common
#include "execution_wrapper.hpp"

// internals
#include "job.hpp"
#include "legacy_hw_path/hardware_state.h"

namespace qpl::test {

QPL_UNIT_API_ALGORITHMIC_TEST(deflate_static, stored_block_on_last_job) {
    QPL_SKIP_TEST_FOR(qpl_path_software);
    QPL_SKIP_TEST_FOR(qpl_path_auto);

    // LFSR Function
    auto lfsr8 = [](uint32_t& state) -> uint8_t {
        if (state == 0) state = 1;
        for (uint32_t i = 0U; i < 8U; i++) {
            // 24-bit (x^24 + x^23 + x^22 + x^17 + 1)
            if (state & 1) state ^= 0x1c20000;
            state >>= 1;
        }
        return state & 0xFF;
    };

    qpl_status     res = QPL_STS_OK;
    uint32_t       i = 0U, lfsr_state = 1U, size = 0U;
    const uint32_t MAX_SIZE = 100U * 1024U;
    uint8_t        buff[MAX_SIZE];
    uint8_t        out[MAX_SIZE * 2];
    uint8_t        ref[MAX_SIZE];

    qpl_histogram deflate_histogram {};

    // Initialize the histogram with predefined values
    for (i = 0U; i < 286U; i++) {
        if (i < 256U) {
            deflate_histogram.literal_lengths[i] = 1U;
        } else {
            deflate_histogram.literal_lengths[i] = (1U << 15U) + 1U;
        }
    }

    for (i = 0U; i < 30U; i++) {
        deflate_histogram.distances[i] = (1U << 15U);
    }

    const unique_huffman_table table(
            deflate_huffman_table_maker(combined_table_type, qpl_path_software, DEFAULT_ALLOCATOR_C),
            any_huffman_table_deleter);
    ASSERT_NE(table.get(), nullptr) << "Huffman Table creation failed\n";

    // Initialize the Huffman table with the histogram
    res = qpl_huffman_table_init_with_histogram(table.get(), &deflate_histogram);
    ASSERT_EQ(QPL_STS_OK, res) << "An error " << res << " acquired during huffman table initialization.\n";

    // Job initialization
    const qpl_path_t execution_path = qpl::test::util::TestEnvironment::GetInstance().GetExecutionPath();

    res = qpl_get_job_size(execution_path, &size);
    ASSERT_EQ(QPL_STS_OK, res) << "An error " << res << " acquired during job size acquisition.\n";

    std::unique_ptr<uint8_t[]> job_buffer;

    job_buffer   = std::make_unique<uint8_t[]>(size);
    qpl_job* job = reinterpret_cast<qpl_job*>(job_buffer.get());

    res = qpl_init_job(execution_path, job);
    ASSERT_EQ(QPL_STS_OK, res) << "An error " << res << " acquired during job initialization.\n";

    // Initialize input buffer with pseudo-random data
    lfsr_state = 1U;
    for (i = 0U; i < MAX_SIZE; i++) {
        buff[i] = lfsr8(lfsr_state);
    }

    // Preparatory step: trying to build a job with 255 bits stored into the accumulator
    uint32_t stored_bits    = 0U;
    uint32_t first_job_size = 0U;

    job->op            = qpl_op_compress;
    job->level         = qpl_default_level;
    job->flags         = QPL_FLAG_FIRST;
    job->huffman_table = table.get();
    for (i = 20U; i < 1000U; i++) {
        job->next_in_ptr   = &buff[0];
        job->available_in  = i;
        job->next_out_ptr  = &out[0];
        job->available_out = sizeof(out);

        // Use async API here for submission since we read from state afterwards
        res = qpl_submit_job(job);
        ASSERT_EQ(QPL_STS_OK, res) << "An error " << res
                                   << " acquired during compression submission on preparation step.\n";

        res = qpl_wait_job(job);
        ASSERT_EQ(QPL_STS_OK, res) << "An error " << res << " acquired during wait on preparation step.\n";

        auto* const           state_ptr       = reinterpret_cast<qpl_hw_state*>(qpl::job::get_state(job));
        hw_iaa_aecs_compress* actual_aecs_ptr = hw_iaa_aecs_compress_get_aecs_ptr(
                state_ptr->ccfg, state_ptr->aecs_hw_read_offset, state_ptr->aecs_size);
        ASSERT_NE(nullptr, actual_aecs_ptr) << "Actual AECS pointer is null.\n";

        stored_bits = hw_iaa_aecs_compress_accumulator_get_actual_bits(actual_aecs_ptr);
        if (stored_bits == 255) {
            first_job_size = i;
            break;
        }
    }

    // Two-step compression with verification and decompression to check the results
    for (i = 1024U; i < MAX_SIZE; i *= 2) {
        // Two-step compression (1/2)
        // First chunk would leave 255 in the accumulator
        job->op            = qpl_op_compress;
        job->level         = qpl_default_level;
        job->flags         = QPL_FLAG_FIRST;
        job->huffman_table = table.get();
        job->next_in_ptr   = &buff[0];
        job->available_in  = first_job_size;
        job->next_out_ptr  = &out[0];
        job->available_out = sizeof(out);

        res = run_job_api(job);
        ASSERT_EQ(QPL_STS_OK, res) << "An error " << res << " acquired during compression with QPL_FLAG_FIRST.\n";

        job->flags &= ~QPL_FLAG_FIRST;

        // Two-step compression (2/2)
        // Second chunk generates output overflow that should write the stored block
        size               = job->total_out;
        job->flags         = QPL_FLAG_LAST;
        job->available_in  = i;
        job->available_out = i * 1.5;

        res = run_job_api(job);
        ASSERT_EQ(QPL_STS_OK, res) << "An error " << res << " acquired during compression with QPL_FLAG_LAST.\n";

        ASSERT_EQ(qpl_get_safe_deflate_compression_buffer_size(i) > (job->total_out - size), true)
                << "qpl_get_safe_deflate_compression_buffer_size provided size is not sufficient.";

        // Decompress to verify results
        job->op            = qpl_op_decompress;
        job->next_in_ptr   = &out[0];
        job->available_in  = job->total_out;
        job->next_out_ptr  = &ref[0];
        job->available_out = MAX_SIZE;
        job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST;

        res = run_job_api(job);
        ASSERT_EQ(QPL_STS_OK, res) << "An error " << res << " acquired during decompression.\n";

        for (uint32_t j = 0U; j < first_job_size + i; j++) {
            ASSERT_EQ(buff[j], ref[j]) << "Decompressed data does not match the original data.\n";
        }
    }

    res = qpl_fini_job(job);
    ASSERT_EQ(QPL_STS_OK, res) << "An error " << res << " acquired during job finalization.\n";
}

} // namespace qpl::test
