/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @brief Tests histogram based on https://tools.ietf.org/html/rfc1951 specification.
 */

#include "deflate_defs.h"
#include "deflate_hash_table.h"
#include "deflate_histogram.h"
#include "gtest/gtest.h"
#include "qpl_test_environment.hpp"
#include "random_generator.h"
#include "t_common.hpp"

namespace qpl::test {

constexpr uint8_t RFC1951_OFFSET_TABLE_SIZE = 30U;
/**
     * @brief Represents offset table described in https://tools.ietf.org/html/rfc1951
     */
static inline uint16_t offset_correspondence_table[RFC1951_OFFSET_TABLE_SIZE][2] = {
        {1, 1},       {2, 2},       {3, 3},        {4, 4},         {5, 6},         {7, 8},
        {9, 12},      {13, 16},     {17, 24},      {25, 32},       {33, 48},       {49, 64},
        {65, 96},     {97, 128},    {129, 192},    {193, 256},     {257, 384},     {385, 512},
        {513, 768},   {769, 1024},  {1025, 1536},  {1537, 2048},   {2049, 3072},   {3073, 4096},
        {4097, 6144}, {6145, 8192}, {8193, 12288}, {12289, 16384}, {16385, 24576}, {24577, 32768},
};

/**
     * @brief Tests offset index calculation in accordance with @ref offset_correspondence_table
     */
QPL_UNIT_API_ALGORITHMIC_TEST(deflate_histogramm, offset_index_calculation) {
    // Variables
    qpl::test::random   random_offset(0U, 0U, util::TestEnvironment::GetInstance().GetSeed());
    deflate_histogram_t histogram;
    deflate_match_t     match = {};

    // Reset histogram table
    std::fill(reinterpret_cast<uint8_t*>(&histogram), reinterpret_cast<uint8_t*>(&histogram) + sizeof(histogram), 0U);

    // Fill histogram with offsets
    for (auto& borders : offset_correspondence_table) {
        random_offset.set_range(borders[0], borders[1]);

        match.offset = static_cast<uint32_t>(random_offset);

        deflate_histogram_update_match(&histogram, match);
    }

    // Validate offset table
    auto offset_table = histogram.offsets;

    for (uint32_t i = 0; i < RFC1951_OFFSET_TABLE_SIZE; i++) {
        EXPECT_EQ(offset_table[i], 1U) << "Incorrect statistic for " << i << "offset group";
    }
}
} // namespace qpl::test
