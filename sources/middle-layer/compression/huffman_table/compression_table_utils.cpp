/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <algorithm>
#include <compression/deflate/utils/compression_defs.hpp>
#include "compression_table_utils.hpp"
#include "canned_utils.hpp"
#include "util/util.hpp"

extern "C" void own_qpl_huffman_table_to_isal(const qpl_compression_huffman_table *p_table,
                                              isal_hufftables *p_hufftables,
                                              qpl::ml::compression::endianness_t endianness);

namespace qpl::ml::compression {

static const uint8_t own_match_length_codes_bases[29] = {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x0A, 0x0C, 0x0E,
        0x10, 0x14, 0x18, 0x1C,
        0x20, 0x28, 0x30, 0x38,
        0x40, 0x50, 0x60, 0x70,
        0x80, 0xA0, 0xC0, 0xE0,
        0xFF
};

static const uint8_t own_match_lengths_extra_bits[29] = {
        0, 0, 0, 0,
        0, 0, 0, 0,
        1, 1, 1, 1,
        2, 2, 2, 2,
        3, 3, 3, 3,
        4, 4, 4, 4,
        5, 5, 5, 5,
        0
};


void qpl_compression_table_to_isal(const qplc_compression_huffman_table *qpl_compression_table,
                                   isal_hufftables *isal_compression_table) noexcept {
    const auto *c_huffman_table = reinterpret_cast<const qpl_compression_huffman_table *>(qpl_compression_table);

    own_qpl_huffman_table_to_isal(c_huffman_table, isal_compression_table, little_endian);
}

void isal_compression_table_to_qpl(const isal_hufftables *isal_table_ptr,
                                   qplc_compression_huffman_table *qpl_table_ptr) noexcept {
    // Variables
    const auto isal_match_lengths_mask   = util::build_mask<uint16_t, 15u>();

    // Convert literals codes
    for (uint32_t i = 0; i < literals_table_size; i++) {
        const uint16_t code   = isal_table_ptr->lit_table[i];
        const uint8_t  length = isal_table_ptr->lit_table_sizes[i];

        qpl_table_ptr->literals_matches[i] = reverse_bits(code, length) | (uint32_t) (length << 15u);
    }

    // Convert match lengths codes
    for (uint32_t i = 0; i < matches_table_size; i++) {
        const uint16_t code   = isal_table_ptr->len_table[own_match_length_codes_bases[i]] >> 5u;
        uint8_t length = isal_table_ptr->len_table[own_match_length_codes_bases[i]] & isal_match_lengths_mask;

        // Normally, (in all cases except for huffman only) ISAL assignes code for every match length token, but
        // this can be a huffman only table, without match lengths codes, so additionaly check if code length is more than zero
        // to prevent the overflow of code's length
        if (0u != length) {
            length -= own_match_lengths_extra_bits[i];
            qpl_table_ptr->literals_matches[i + literals_table_size] = reverse_bits(code, length) | (uint32_t) (length << 15u);
        } else {
            // Write zero otherwise
            qpl_table_ptr->literals_matches[i + literals_table_size] = 0u;
        }
    }

    // Convert offsets codes
    for (uint32_t i = 0; i < offsets_table_size; i++) {
        const uint16_t code   = isal_table_ptr->dcodes[i];
        const uint8_t  length = isal_table_ptr->dcodes_sizes[i];

        qpl_table_ptr->offsets[i] = reverse_bits(code, length) | (uint32_t) (length << 15u);
    }
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage=4096"
#endif

auto build_compression_table(const uint32_t *literals_lengths_histogram_ptr,
                             const uint32_t *distances_histogram_ptr,
                             compression_huffman_table &compression_table) noexcept -> qpl_ml_status {
    if (compression_table.is_huffman_only() &&
        compression_table.is_deflate_header_used()) {
            return status_list::status_invalid_params;
        }

    if (compression_table.is_hw_compression_table_used()) {
        // HW table format is equal to SW table
        compression_table.enable_sw_compression_table();

        auto increment_zeros = [](const uint32_t &n) {
            const_cast<uint32_t&>(n) += (n == 0) ? 1u : 0u;
        };

        std::for_each(literals_lengths_histogram_ptr,
                      literals_lengths_histogram_ptr + ISAL_DEF_LIT_LEN_SYMBOLS,
                      increment_zeros);

        std::for_each(distances_histogram_ptr,
                      distances_histogram_ptr + ISAL_DEF_DIST_SYMBOLS,
                      increment_zeros);
    }

    if (compression_table.is_sw_compression_table_used() ||
        compression_table.is_deflate_header_used()) {
        // Create isal huffman table and histograms
        isal_huff_histogram histogram = {{0u}, {0u}, {0u}};

        if (compression_table.is_huffman_only()) {
            // Copy literals (except for EOB symbol) histogram to ISAL histogram
            fill_histogram_literals_only(literals_lengths_histogram_ptr, &histogram);

            // Main pipeline here, use ISAL to create huffman tables
            isal_create_hufftables_literals_only(compression_table.get_isal_compression_table(), &histogram);
            compression_table.set_deflate_header_bit_size(0);
        } else {
            // Fill isal histogram from the given one
            fill_histogram(literals_lengths_histogram_ptr, distances_histogram_ptr, &histogram);

            // Main pipeline here, use ISAL to create huffman tables
            isal_create_hufftables(compression_table.get_isal_compression_table(), &histogram);

        }
        // Store huffman codes if required
        if (compression_table.is_sw_compression_table_used()) {
            isal_compression_table_to_qpl(compression_table.get_isal_compression_table(), compression_table.get_sw_compression_table());
        }

        // Store deflate header content if required
        if (compression_table.is_deflate_header_used()) {
            store_isal_deflate_header(compression_table.get_isal_compression_table(), compression_table);
        }
    }

    return status_list::ok;
}
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
