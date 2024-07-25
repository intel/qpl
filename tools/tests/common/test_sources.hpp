/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tests
 */

#ifndef QPL_TESTS_COMMON_TEST_SOURCES_HPP_
#define QPL_TESTS_COMMON_TEST_SOURCES_HPP_

#include <stdexcept>
#include <vector>

#include "qpl/qpl.h"

// tool_generator
#include "format_generator.hpp"

namespace qpl::test {
namespace util {
template <class stream_t>
auto compress_stream(stream_t stream) -> std::vector<uint8_t>;
}

class AnalyticStream {
public:
    AnalyticStream() = delete;

    AnalyticStream(size_t element_count, uint8_t bit_width, qpl_parser parser);

    auto data() noexcept -> uint8_t*;

    auto size() noexcept -> size_t;

    virtual auto bit_width() noexcept -> uint8_t = 0;

    virtual auto parser() -> qpl_parser = 0;

protected:
    qpl_parser parser_;
    uint8_t    bit_width_;
    size_t     element_count_;

    std::vector<uint8_t> data_;
};

class AnalyticInputStream : public AnalyticStream {
public:
    AnalyticInputStream() = delete;

    AnalyticInputStream(size_t element_count, uint8_t element_bit_width, qpl_parser parser = qpl_p_le_packed_array,
                        uint16_t prologue = 0U);

    auto elements_count() noexcept -> size_t;

    auto bit_width() noexcept -> uint8_t override;

    auto parser() -> qpl_parser override;

private:
    uint16_t prologue_ = 0U;
};

class AnalyticMaskStream : public AnalyticStream {
public:
    AnalyticMaskStream() = delete;

    AnalyticMaskStream(size_t element_count, qpl_parser parser = qpl_p_le_packed_array);

    auto bit_width() noexcept -> uint8_t override;

    auto parser() -> qpl_parser override;

private:
    static constexpr uint8_t BIT_WIDTH_ = 1U;
};

class AnalyticCountersStream : public AnalyticStream {
public:
    AnalyticCountersStream(size_t counters_count, uint8_t counter_width, qpl_parser parser = qpl_p_le_packed_array,
                           uint16_t prologue = 0U);

    static auto elements_count() noexcept -> size_t;

    auto packed_elements_count() const noexcept -> size_t;

    auto bit_width() noexcept -> uint8_t override;

    auto parser() -> qpl_parser override;

private:
    uint16_t prologue_              = 0U;
    size_t   packed_elements_count_ = 0;
};

} // namespace qpl::test

#endif //QPL_TESTS_COMMON_TEST_SOURCES_HPP_
