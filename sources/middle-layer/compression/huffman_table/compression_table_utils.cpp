/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <algorithm>

#include "common/bit_reverse.hpp"

#include "compression/deflate/utils/compression_defs.hpp"
#include "compression/huffman_table/canned_utils.hpp"
#include "compression/huffman_table/compression_table_utils.hpp"

#include "util/util.hpp"
#include "util/memory.hpp"

#include "qplc_compression_consts.h"
#include "qplc_huffman_table.h"
#include "deflate_histogram.h"

namespace qpl::ml::compression {

namespace details {

static const uint8_t match_length_codes_bases[29] = {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x0A, 0x0C, 0x0E,
        0x10, 0x14, 0x18, 0x1C,
        0x20, 0x28, 0x30, 0x38,
        0x40, 0x50, 0x60, 0x70,
        0x80, 0xA0, 0xC0, 0xE0,
        0xFF
};

static const uint8_t match_lengths_extra_bits[29] = {
        0, 0, 0, 0,
        0, 0, 0, 0,
        1, 1, 1, 1,
        2, 2, 2, 2,
        3, 3, 3, 3,
        4, 4, 4, 4,
        5, 5, 5, 5,
        0
};

struct own_huffman_code {
    uint16_t code;               /**< Huffman code */
    uint8_t  extra_bit_count;    /**< Number of extra bits */
    uint8_t  length;             /**< Huffman code length */
};

static inline void create_code_tables(uint16_t *const code_table_ptr,
                                          uint8_t *const code_length_table_ptr,
                                          const uint32_t length,
                                          const struct own_huffman_code *const huffman_table_ptr) {
    for (uint32_t i = 0; i < length; i++) {
        code_table_ptr[i]        = huffman_table_ptr[i].code;
        code_length_table_ptr[i] = huffman_table_ptr[i].length;
    }
}

static inline void create_packed_match_lengths_table(uint32_t *const packed_table_ptr,
                                                         const struct own_huffman_code *const huffman_table_ptr) {
    // Variables
    uint8_t  count            = 0;
    uint16_t extra_bits_count = 0;
    uint16_t gain_extra_bits  = QPLC_DEFLATE_EXTRA_BITS_START_POSITION;

    // Main cycle
    for (uint32_t i = 257; i < QPLC_DEFLATE_LL_TABLE_SIZE - 1; i++) {
        for (uint16_t extra_bits = 0; extra_bits < (1u << extra_bits_count); extra_bits++) {
            if (count > 254) {
                break;
            }

            packed_table_ptr[count++] = (extra_bits << (huffman_table_ptr[i].length + QPLC_CODE_LENGTH_BIT_LENGTH)) |
                                        (huffman_table_ptr[i].code << QPLC_CODE_LENGTH_BIT_LENGTH) |
                                        (huffman_table_ptr[i].length + extra_bits_count);
        }

        if (i == gain_extra_bits) {
            gain_extra_bits += QPLC_DEFLATE_LENGTH_EXTRA_BITS_INTERVAL;
            extra_bits_count += 1;
        }
    }

    packed_table_ptr[count] =
            (huffman_table_ptr[QPLC_DEFLATE_LL_TABLE_SIZE - 1].code << QPLC_CODE_LENGTH_BIT_LENGTH) |
            (huffman_table_ptr[QPLC_DEFLATE_LL_TABLE_SIZE - 1].length);
}

static inline void create_packed_offset_table(uint32_t *const packed_table_ptr,
                                                  const uint32_t length,
                                                  const struct own_huffman_code *const huffman_table_ptr) {
    // Variables
    uint32_t count            = 0;
    uint16_t extra_bits_count = 0;
    uint16_t gain_extra_bits  = QPLC_DEFLATE_EXTRA_OFFSETS_BEGIN_VALUE;

    // Main cycle
    for (uint32_t i = 0; i < QPLC_DEFLATE_OFFSETS_COUNT; i++) {
        for (uint16_t extra_bits = 0; extra_bits < (1u << extra_bits_count); extra_bits++) {
            if (count >= length) {
                return;
            }

            packed_table_ptr[count++] = (extra_bits << (huffman_table_ptr[i].length + QPLC_CODE_LENGTH_BIT_LENGTH)) |
                                        (huffman_table_ptr[i].code << QPLC_CODE_LENGTH_BIT_LENGTH) |
                                        (huffman_table_ptr[i].length + extra_bits_count);
        }

        if (i == gain_extra_bits) {
            gain_extra_bits += QPLC_DEFLATE_OFFSETS_EXTRA_BITS_INTERVAL;
            extra_bits_count += 1;
        }
    }
}

static void inline fill_histogram(const uint32_t *literals_lengths_histogram_ptr,
                                  const uint32_t *distances_histogram_ptr,
                                  isal_huff_histogram *histogram) {
    for (uint32_t i = 0u; i < QPLC_DEFLATE_LL_TABLE_SIZE; i++) {
        histogram->lit_len_histogram[i] = static_cast<uint32_t>(literals_lengths_histogram_ptr[i]);
    }

    for (uint32_t i = 0u; i < QPLC_DEFLATE_D_TABLE_SIZE; i++) {
        histogram->dist_histogram[i] = static_cast<uint32_t>(distances_histogram_ptr[i]);
    }
}

static void inline fill_histogram_literals_only(const uint32_t *literals_lengths_histogram_ptr,
                                                isal_huff_histogram *histogram) {
    for (uint32_t i = 0u; i < QPLC_LITERALS_COUNT; i++) {
        histogram->lit_len_histogram[i] = static_cast<uint32_t>(literals_lengths_histogram_ptr[i]);
    }
}

void inline store_isal_deflate_header(isal_hufftables *isal_huffman_table,
                                      compression_huffman_table &compression_table) noexcept {
    auto header_complete_byte_size = isal_huffman_table->deflate_hdr_count;
    header_complete_byte_size += (0u == isal_huffman_table->deflate_hdr_extra_bits) ? 0u : 1u;

    // Use copy kernel to copy deflate header from isal huffman tables
    auto copy_kernel = dispatcher::kernels_dispatcher::get_instance().get_memory_copy_table();
    copy_kernel[0]((uint8_t *) isal_huffman_table->deflate_hdr,
                   compression_table.get_deflate_header_data(),
                   header_complete_byte_size);

    // Calculate and store deflate header bits size
    const auto header_bit_size = isal_huffman_table->deflate_hdr_count * byte_bits_size
                                 + (isal_huffman_table->deflate_hdr_extra_bits);
    compression_table.set_deflate_header_bit_size(header_bit_size);
}

static inline void qpl_huffman_table_to_isal(const qpl_compression_huffman_table *qpl_table_ptr,
                                             struct isal_hufftables *const isal_table_ptr,
                                             const endianness_t endian) {
    auto table_ptr = const_cast<qpl_compression_huffman_table *>(qpl_table_ptr);

    // Variables
    const uint32_t qpl_code_mask     = (1u << QPLC_HUFFMAN_CODE_BIT_LENGTH) - 1u;
    // First 15 bits [14:0]
    const uint32_t qpl_length_mask   = QPLC_HUFFMAN_CODE_LENGTH_MASK << QPLC_HUFFMAN_CODE_BIT_LENGTH; // Bits [18:15]
    uint32_t       header_byte_size  = 0;
    uint32_t       header_extra_bits = 0;

    struct own_huffman_code literals_matches_table[QPLC_DEFLATE_LL_TABLE_SIZE] = {{0u, 0u, 0u}};
    struct own_huffman_code offsets_huffman_table[QPLC_DEFLATE_D_TABLE_SIZE]   = {{0u, 0u, 0u}};

    // Memory initialization
    qpl::ml::util::set_zeros((uint8_t *) isal_table_ptr, sizeof(struct isal_hufftables));

    // Copying literals and match lengths Huffman table
    for (uint32_t i = 0; i < QPLC_DEFLATE_LL_TABLE_SIZE; i++) {
        const auto code   = static_cast<uint16_t>(get_literals_lengths_table_ptr(table_ptr)[i] & qpl_code_mask);
        const auto length = static_cast<uint8_t>((get_literals_lengths_table_ptr(table_ptr)[i] & qpl_length_mask)
                >> 15u);

        literals_matches_table[i].length = length;
        literals_matches_table[i].code   = endian == little_endian ? reverse_bits(code, length) : code;
    }

    // Copying offsets Huffman table
    for (uint32_t i = 0; i < QPLC_DEFLATE_D_TABLE_SIZE; i++) {
        const auto code   = static_cast<uint16_t>(get_offsets_table_ptr(table_ptr)[i] & qpl_code_mask);
        const auto length = static_cast<uint8_t>((get_offsets_table_ptr(table_ptr)[i] & qpl_length_mask) >> 15u);

        offsets_huffman_table[i].length = length;
        offsets_huffman_table[i].code   = endian == little_endian ? reverse_bits(code, length) : code;
    }

    // Generating ISA-L tables
    create_code_tables(isal_table_ptr->dcodes,
                           isal_table_ptr->dcodes_sizes,
                           QPLC_DEFLATE_D_TABLE_SIZE,
                           offsets_huffman_table);
    create_code_tables(isal_table_ptr->lit_table,
                           isal_table_ptr->lit_table_sizes,
                           QPLC_DEFLATE_LITERALS_COUNT,
                           literals_matches_table);

    create_packed_match_lengths_table(isal_table_ptr->len_table, literals_matches_table);
    create_packed_offset_table(isal_table_ptr->dist_table, QPLC_OFFSET_TABLE_SIZE, offsets_huffman_table);

    // Setting header information
    header_extra_bits = get_deflate_header_bits_size(table_ptr) % 8;
    header_byte_size  = (get_deflate_header_bits_size(table_ptr) / 8) + (header_extra_bits == 0 ? 0 : 1);

    isal_table_ptr->deflate_hdr_count      = get_deflate_header_bits_size(table_ptr) / 8;
    isal_table_ptr->deflate_hdr_extra_bits = header_extra_bits;

    uint8_t *deflate_header_ptr = get_deflate_header_ptr(table_ptr);

    for (uint32_t i = 0; i < header_byte_size; i++) {
        isal_table_ptr->deflate_hdr[i] = deflate_header_ptr[i];
    }

    // Forcedly set final bit of header, ISA-L will reset it if current block not final
    isal_table_ptr->deflate_hdr[0] |= 1u;
}

}

void qpl_compression_table_to_isal(const qplc_huffman_table_default_format *qpl_compression_table,
                                   isal_hufftables *isal_compression_table) noexcept {
    const auto *c_huffman_table = reinterpret_cast<const qpl_compression_huffman_table *>(qpl_compression_table);

    details::qpl_huffman_table_to_isal(c_huffman_table, isal_compression_table, little_endian);
}

void isal_compression_table_to_qpl(const isal_hufftables *isal_table_ptr,
                                   qplc_huffman_table_default_format *qpl_table_ptr) noexcept {
    // Variables
    const auto isal_match_lengths_mask = util::build_mask<uint16_t, 15u>();

    // Convert literals codes
    for (uint32_t i = 0; i < QPLC_DEFLATE_LITERALS_COUNT; i++) {
        const uint16_t code   = isal_table_ptr->lit_table[i];
        const uint8_t  length = isal_table_ptr->lit_table_sizes[i];

        qpl_table_ptr->literals_matches[i] = reverse_bits(code, length) | (uint32_t) (length << 15u);
    }

    // Convert match lengths codes
    for (uint32_t i = 0; i < QPLC_DEFLATE_MATCHES_COUNT; i++) {
        const uint16_t code   = isal_table_ptr->len_table[details::match_length_codes_bases[i]] >> 5u;
        uint8_t        length = isal_table_ptr->len_table[details::match_length_codes_bases[i]] & isal_match_lengths_mask;

        // Normally, (in all cases except for huffman only) ISAL assignes code for every match length token, but
        // this can be a huffman only table, without match lengths codes, so additionaly check if code length is more than zero
        // to prevent the overflow of code's length
        if (0u != length) {
            length -= details::match_lengths_extra_bits[i];
            qpl_table_ptr->literals_matches[i + QPLC_DEFLATE_LITERALS_COUNT] =
                    reverse_bits(code, length) | (uint32_t) (length << 15u);
        } else {
            // Write zero otherwise
            qpl_table_ptr->literals_matches[i + QPLC_DEFLATE_LITERALS_COUNT] = 0u;
        }
    }

    // Convert offsets codes
    for (uint32_t i = 0; i < QPLC_DEFLATE_OFFSETS_COUNT; i++) {
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
            const_cast<uint32_t &>(n) += (n == 0) ? 1u : 0u;
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
        isal_huff_histogram histogram = {{0u},
                                         {0u},
                                         {0u}};

        if (compression_table.is_huffman_only()) {
            // Copy literals (except for EOB symbol) histogram to ISAL histogram
            details::fill_histogram_literals_only(literals_lengths_histogram_ptr, &histogram);

            // Main pipeline here, use ISAL to create huffman tables
            isal_create_hufftables_literals_only(compression_table.get_isal_compression_table(), &histogram);
            compression_table.set_deflate_header_bit_size(0);
        } else {
            // Fill isal histogram from the given one
            details::fill_histogram(literals_lengths_histogram_ptr, distances_histogram_ptr, &histogram);

            // Main pipeline here, use ISAL to create huffman tables
            isal_create_hufftables(compression_table.get_isal_compression_table(), &histogram);

        }
        // Store huffman codes if required
        if (compression_table.is_sw_compression_table_used()) {
            isal_compression_table_to_qpl(compression_table.get_isal_compression_table(),
                                          compression_table.get_sw_compression_table());
        }

        // Store deflate header content if required
        if (compression_table.is_deflate_header_used()) {
            details::store_isal_deflate_header(compression_table.get_isal_compression_table(), compression_table);
        }
    }

    return status_list::ok;
}
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
