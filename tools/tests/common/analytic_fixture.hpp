/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tests
 */

#ifndef QPL_TEST_ANALYTICS_FIXTURE_HPP
#define QPL_TEST_ANALYTICS_FIXTURE_HPP

#include <stdexcept>

// tests_common
#include "operation_test.hpp"
#include "test_cases.hpp"

// tool_generator
#include "index_table.hpp"

// tool_common
#include "source_provider.hpp"
#include "util.hpp"

namespace qpl::test {
struct AnalyticTestCase {
    qpl_operation operation                 = (qpl_operation)0;
    uint32_t      number_of_elements        = 0U;
    uint32_t      source_bit_width          = 0U;
    uint32_t      destination_bit_width     = 0U;
    uint32_t      lower_bound               = 0U;
    uint32_t      upper_bound               = 0U;
    uint32_t      second_input_bit_width    = 0U;
    uint32_t      second_input_num_elements = 0U;
    uint64_t      flags                     = 0U;
    qpl_parser    parser                    = (qpl_parser)0;
    uint32_t      dropped_bytes             = 0U;
};

inline void FillJob(qpl_job* qpl_job_ptr, AnalyticTestCase& test_case) {
    qpl_job_ptr->op                 = test_case.operation;
    qpl_job_ptr->param_low          = test_case.lower_bound;
    qpl_job_ptr->param_high         = test_case.upper_bound;
    qpl_job_ptr->num_input_elements = test_case.number_of_elements;
    qpl_job_ptr->src1_bit_width     = test_case.source_bit_width;
    qpl_job_ptr->src2_bit_width     = test_case.second_input_bit_width;
    qpl_job_ptr->out_bit_width      = uint_to_qpl_output(test_case.destination_bit_width);
    qpl_job_ptr->parser             = test_case.parser;
    qpl_job_ptr->flags              = test_case.flags;
    //qpl_job_ptr->drop_initial_bytes = test_case.dropped_bytes;
}

static std::ostream& operator<<(std::ostream& os, const AnalyticTestCase& test_case) {
    os << "Operation: " << OperationToString(test_case.operation) << '\n';
    os << "Number of elements: " << test_case.number_of_elements << '\n';
    os << "Source-1 bit width: " << test_case.source_bit_width << '\n';
    os << "Destination bit width: " << test_case.destination_bit_width << '\n';
    os << "Lower parameter: " << test_case.lower_bound << '\n';
    os << "Upper parameter: " << test_case.upper_bound << '\n';
    os << "Source-2 bit width " << test_case.second_input_bit_width << '\n';
    os << "Source-2 number of elements: " << test_case.second_input_num_elements << '\n';
    os << "Flags: " << test_case.flags << '\n';
    os << "Parser: " << ParserToString(test_case.parser) << '\n';
    os << "Dropped bytes: " << test_case.dropped_bytes << '\n';

    return os;
}

class AnalyticFixture
    : public ReferenceFixture
    , public TestCases<AnalyticTestCase> {
public:
    std::vector<uint8_t> GetCompressedSource(bool is_indexing_enabled = false) {
        uint32_t job_size = 0U;
        auto     status   = qpl_get_job_size(GetExecutionPath(), &job_size);

        if (QPL_STS_OK != status) { throw std::runtime_error("Couldn't get compression job size\n"); }

        auto     job_buffer      = std::make_unique<uint8_t[]>(job_size);
        qpl_job* deflate_job_ptr = reinterpret_cast<qpl_job*>(job_buffer.get());

        status = qpl_init_job(GetExecutionPath(), deflate_job_ptr);

        if (QPL_STS_OK != status) { throw std::runtime_error("Couldn't init compression job\n"); }

        const uint32_t MINIMAL_DESTINATION_SIZE = 100U;
        uint32_t       destination_size         = static_cast<uint32_t>(source.size()) * 2U;
        destination_size = (destination_size < MINIMAL_DESTINATION_SIZE) ? MINIMAL_DESTINATION_SIZE : destination_size;

        std::vector<uint8_t> compressed_source(destination_size, 0);

        deflate_job_ptr->op            = qpl_op_compress;
        deflate_job_ptr->level         = qpl_default_level;
        deflate_job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DYNAMIC_HUFFMAN;
        deflate_job_ptr->available_in  = static_cast<uint32_t>(source.size());
        deflate_job_ptr->next_in_ptr   = source.data();
        deflate_job_ptr->available_out = static_cast<uint32_t>(compressed_source.size());
        deflate_job_ptr->next_out_ptr  = compressed_source.data();

        if (is_indexing_enabled) {
            deflate_job_ptr->mini_block_size = qpl_mblk_size_32k;
            auto mini_blocks_count           = static_cast<uint32_t>(compressed_source.size() / qpl_mblk_size_32k);
            index_table.reset(mini_blocks_count, mini_blocks_count);

            deflate_job_ptr->idx_array    = reinterpret_cast<uint64_t*>(index_table.data());
            deflate_job_ptr->idx_max_size = static_cast<uint32_t>(index_table.size());
        } else {
            deflate_job_ptr->flags |= QPL_FLAG_OMIT_VERIFY;
        }

        status = qpl_execute_job(deflate_job_ptr);

        if (QPL_STS_OK != status) {
            std::string error_message = "Compression returned " + std::to_string(status) + " status\n";
            throw std::runtime_error(error_message);
        }

        if (is_indexing_enabled) {
            job_ptr->idx_array       = deflate_job_ptr->idx_array;
            job_ptr->idx_max_size    = deflate_job_ptr->idx_max_size;
            job_ptr->idx_num_written = deflate_job_ptr->idx_num_written;
        }

        compressed_source.resize(deflate_job_ptr->total_out);

        return compressed_source;
    }

protected:
    void SetUp() override { ReferenceFixture::SetUp(); }

    void SetUpBeforeIteration() override {
        current_test_case = GetTestCase();

        // Generate and fill in source, destination and reference destination.
        SetBuffers();

        // Set specific job parameters to current test case parameters.
        FillJob(job_ptr, current_test_case);
        FillJob(reference_job_ptr, current_test_case);

        // (Re-)Initialize some of the parameters to 0 in between iterations.
        job_ptr->total_in     = 0U;
        job_ptr->total_out    = 0U;
        job_ptr->crc          = 0U;
        job_ptr->xor_checksum = 0U;

        reference_job_ptr->total_in     = 0U;
        reference_job_ptr->total_out    = 0U;
        reference_job_ptr->crc          = 0U;
        reference_job_ptr->xor_checksum = 0U;
    }

    static std::vector<uint32_t> GenerateNumberOfElementsVector() {
        std::vector<uint32_t> result;

        for (uint32_t i = 1U; i <= 128U; i++) {
            result.push_back(i);
        }

        for (uint32_t i = 7U; i <= 12U; i++) {
            auto left_index  = 1U << i;
            auto right_index = left_index << 1U;
            result.push_back((right_index + left_index) >> 1U);
        }

        return result;
    }

    void GenerateBuffers() {
        source_provider source_gen(current_test_case.number_of_elements, current_test_case.source_bit_width, GetSeed(),
                                   current_test_case.parser);

        ASSERT_NO_THROW(source = source_gen.get_source()); //NOLINT(cppcoreguidelines-avoid-goto)

        uint32_t actual_out_bit_width = current_test_case.destination_bit_width;

        if (1U == current_test_case.destination_bit_width) {
            actual_out_bit_width = current_test_case.source_bit_width;
        }

        uint32_t dest_size = current_test_case.number_of_elements *
                             ((actual_out_bit_width + max_bit_index) >> bit_to_byte_shift_offset);

        std::fill(destination.begin(), destination.end(), 0);
        std::fill(reference_destination.begin(), reference_destination.end(), 0);

        destination.resize(dest_size);
        reference_destination.resize(dest_size);
    }

    virtual void SetBuffers() {
        GenerateBuffers();

        job_ptr->available_in           = static_cast<uint32_t>(source.size());
        job_ptr->next_in_ptr            = source.data();
        reference_job_ptr->available_in = static_cast<uint32_t>(source.size());
        reference_job_ptr->next_in_ptr  = source.data();

        job_ptr->available_out           = static_cast<uint32_t>(destination.size());
        job_ptr->next_out_ptr            = destination.data();
        reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());
        reference_job_ptr->next_out_ptr  = reference_destination.data();
    }

    AnalyticTestCase current_test_case;
    IndexTable       index_table;
};
} // namespace qpl::test

#endif //QPL_TEST_ANALYTICS_FIXTURE_HPP
