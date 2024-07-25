/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Middle Layer API (private C++ API)
 */

#include <cassert>
#include <cstddef>

// tests_common
#include "qpl_test_environment.hpp"
#include "test_sources.hpp"

// tool_generator
#include "format_generator.hpp"

// tool_common
#include "source_provider.hpp"

namespace qpl::test {
namespace util {
template <class stream_t>
auto compress_stream(stream_t stream) -> std::vector<uint8_t> {
    auto path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size        = 0U;
    qpl_job* deflate_job_ptr = nullptr;
    auto     status          = qpl_get_job_size(path, &job_size);

    if (QPL_STS_OK != status) { throw std::runtime_error("Couldn't get compression job size\n"); }

    auto job_buffer = std::make_unique<uint8_t[]>(job_size);
    deflate_job_ptr = reinterpret_cast<qpl_job*>(job_buffer.get());

    status = qpl_init_job(path, deflate_job_ptr);

    if (QPL_STS_OK != status) { throw std::runtime_error("Couldn't init compression job\n"); }

    const uint32_t MINIMAL_DESTINATION_SIZE = 100U;
    uint32_t       destination_size         = static_cast<uint32_t>(stream.size()) * 2;
    destination_size = (destination_size < MINIMAL_DESTINATION_SIZE) ? MINIMAL_DESTINATION_SIZE : destination_size;

    std::vector<uint8_t> compressed_source(destination_size, 0);

    deflate_job_ptr->op            = qpl_op_compress;
    deflate_job_ptr->level         = qpl_default_level;
    deflate_job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DYNAMIC_HUFFMAN;
    deflate_job_ptr->available_in  = static_cast<uint32_t>(stream.size());
    deflate_job_ptr->next_in_ptr   = stream.data();
    deflate_job_ptr->available_out = static_cast<uint32_t>(compressed_source.size());
    deflate_job_ptr->next_out_ptr  = compressed_source.data();
    deflate_job_ptr->flags |= QPL_FLAG_OMIT_VERIFY;

    status = qpl_execute_job(deflate_job_ptr);

    if (QPL_STS_OK != status) {
        const std::string error_message = "Compression returned " + std::to_string(status) + " status\n";
        throw std::runtime_error(error_message);
    }

    compressed_source.resize(deflate_job_ptr->total_out);

    return compressed_source;
}

template auto compress_stream<qpl::test::AnalyticInputStream>(qpl::test::AnalyticInputStream stream)
        -> std::vector<uint8_t>;

template auto compress_stream<qpl::test::AnalyticCountersStream>(qpl::test::AnalyticCountersStream stream)
        -> std::vector<uint8_t>;

template auto compress_stream<std::vector<uint8_t>>(std::vector<uint8_t> stream) -> std::vector<uint8_t>;

} // namespace util

AnalyticStream::AnalyticStream(size_t element_count, uint8_t bit_width, qpl_parser parser)
    : parser_(parser), bit_width_(bit_width), element_count_(element_count) {
    // No actions required
}

auto AnalyticStream::data() noexcept -> uint8_t* {
    return data_.data();
}

auto AnalyticStream::size() noexcept -> size_t {
    return data_.size();
}

AnalyticInputStream::AnalyticInputStream(size_t element_count, uint8_t element_bit_width, qpl_parser parser,
                                         uint16_t prologue)
    : AnalyticStream(element_count, element_bit_width, parser), prologue_(prologue) {
    data_ = format_generator::generate_uint_bit_sequence(static_cast<uint32_t>(element_count_), bit_width_,
                                                         util::TestEnvironment::GetInstance().GetSeed(),
                                                         parser_ == qpl_p_le_packed_array, prologue_);
}

auto AnalyticInputStream::bit_width() noexcept -> uint8_t {
    return bit_width_;
}

auto AnalyticInputStream::parser() -> qpl_parser {
    return parser_;
}

auto AnalyticInputStream::elements_count() noexcept -> size_t {
    return element_count_;
}

AnalyticMaskStream::AnalyticMaskStream(size_t element_count, qpl_parser parser)
    : AnalyticStream(element_count, BIT_WIDTH_, parser) {
    source_provider mask_gen(static_cast<uint32_t>(element_count_), bit_width_,
                             util::TestEnvironment::GetInstance().GetSeed(), parser_);

    data_ = mask_gen.get_source();
}

auto AnalyticMaskStream::bit_width() noexcept -> uint8_t {
    return BIT_WIDTH_;
}

auto AnalyticMaskStream::parser() -> qpl_parser {
    return parser_;
}

AnalyticCountersStream::AnalyticCountersStream(size_t counters_count, uint8_t counter_width, qpl_parser parser,
                                               uint16_t prologue)
    : AnalyticStream(counters_count, counter_width, parser), prologue_(prologue) {
    // Dynamic assert if bit width is not multiple of 8.
    assert(bit_width_ == 8U);
    source_provider counters_generator(static_cast<uint32_t>(element_count_), bit_width_,
                                       util::TestEnvironment::GetInstance().GetSeed(), parser_);
    // @todo add elements calculation depending on bit_width
    data_                  = counters_generator.get_counter_source_expand_rle(prologue_);
    packed_elements_count_ = counters_generator.get_count_expand_rle_value();
}

auto AnalyticCountersStream::bit_width() noexcept -> uint8_t {
    return bit_width_;
}

auto AnalyticCountersStream::parser() -> qpl_parser {
    return parser_;
}

auto AnalyticCountersStream::elements_count() noexcept -> size_t {
    return 0;
}

auto AnalyticCountersStream::packed_elements_count() const noexcept -> size_t {
    return packed_elements_count_;
}

} // namespace qpl::test
