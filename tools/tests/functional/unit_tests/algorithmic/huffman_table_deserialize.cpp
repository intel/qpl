/*******************************************************************************
 * Copyright (C) 2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

#include "qpl/qpl.h"

#include "t_common.hpp"

// internals
#include "compression/huffman_table/serialization_utils.hpp"
#include "igzip_lib.h"

namespace qpl::test {

namespace {

using namespace qpl::ml::serialization;

/**
 * @brief Serializes a deflate Huffman table of the given type built from a small fixed source.
 */
std::vector<uint8_t> make_serialized_table(qpl_huffman_table_type_e type = combined_table_type) {
    std::array<uint8_t, 1024U> source {};
    for (size_t i = 0U; i < source.size(); i++) {
        source[i] = static_cast<uint8_t>((i * 131U + 17U) & 0xFFU);
    }

    qpl_histogram histogram {};
    EXPECT_EQ(QPL_STS_OK, qpl_gather_deflate_statistics(source.data(), static_cast<uint32_t>(source.size()), &histogram,
                                                        qpl_default_level, qpl_path_software));

    // A decompression table can't be built from a histogram, so build it from a compression table
    const auto source_type = (type == decompression_table_type) ? compression_table_type : type;

    qpl_huffman_table_t table = nullptr;
    EXPECT_EQ(QPL_STS_OK,
              qpl_deflate_huffman_table_create(source_type, qpl_path_software, DEFAULT_ALLOCATOR_C, &table));
    EXPECT_EQ(QPL_STS_OK, qpl_huffman_table_init_with_histogram(table, &histogram));

    if (type == decompression_table_type) {
        qpl_huffman_table_t decompression_table = nullptr;
        EXPECT_EQ(QPL_STS_OK,
                  qpl_deflate_huffman_table_create(type, qpl_path_software, DEFAULT_ALLOCATOR_C, &decompression_table));
        EXPECT_EQ(QPL_STS_OK, qpl_huffman_table_init_with_other(decompression_table, table));
        EXPECT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(table));
        table = decompression_table;
    }

    size_t serialized_size = 0U;
    EXPECT_EQ(QPL_STS_OK,
              qpl_huffman_table_get_serialized_size(table, DEFAULT_SERIALIZATION_OPTIONS, &serialized_size));

    std::vector<uint8_t> serialized(serialized_size);
    EXPECT_EQ(QPL_STS_OK,
              qpl_huffman_table_serialize(table, serialized.data(), serialized.size(), DEFAULT_SERIALIZATION_OPTIONS));
    EXPECT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(table));

    return serialized;
}

size_t meta_size() {
    const qpl::ml::compression::huffman_table_meta_t meta {};
    size_t                                           size = 0U;
    get_meta_size(meta, &size);
    return size;
}

// Offsets of the header fields within the serialized stream,
// see init_compression_table_with_stream() and init_decompression_table_with_stream()
size_t isal_table_offset() {
    const qplc_huffman_table_default_format sw_table {};
    return meta_size() + flatten_table_size(sw_table);
}

size_t isal_header_count_offset() {
    return isal_table_offset() + sizeof(isal_hufftables::deflate_hdr);
}

size_t isal_header_extra_bits_offset() {
    return isal_header_count_offset() + sizeof(isal_hufftables::deflate_hdr_count);
}

size_t compression_deflate_header_offset() {
    const isal_hufftables                                    isal_table {};
    const qpl::ml::compression::hw_compression_huffman_table hw_table {};
    return isal_table_offset() + flatten_table_size(isal_table) + flatten_table_size(hw_table);
}

// The decompression table follows the compression table only in a combined stream
size_t decompression_deflate_header_offset(qpl_huffman_table_type_e type = combined_table_type) {
    const qplc_huffman_table_flat_format               sw_table {};
    const qpl::ml::compression::hw_decompression_state hw_state {};
    const size_t compression_size = (type == combined_table_type) ? sizeof(qpl_compression_huffman_table) : 0U;
    return meta_size() + compression_size + flatten_table_size(sw_table) + flatten_table_size(hw_state);
}

/**
 * @brief Writes value at offset into a copy of the serialized table and returns the deserialization status.
 */
qpl_status deserialize_with_field(const std::vector<uint8_t>& serialized, size_t offset, uint32_t value) {
    std::vector<uint8_t> stream = serialized;
    std::memcpy(stream.data() + offset, &value, sizeof(value));

    qpl_huffman_table_t table = nullptr;
    const auto status = qpl_huffman_table_deserialize(stream.data(), stream.size(), DEFAULT_ALLOCATOR_C, &table);

    if (status == QPL_STS_OK) {
        EXPECT_NE(nullptr, table);
        EXPECT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(table));
    } else {
        // A rejected table must already be released by the deserializer
        EXPECT_EQ(nullptr, table);
    }

    return status;
}

uint32_t read_field(const std::vector<uint8_t>& serialized, size_t offset) {
    uint32_t value = 0U;
    std::memcpy(&value, serialized.data() + offset, sizeof(value));
    return value;
}

constexpr uint32_t max_deflate_header_bits = qpl::ml::compression::deflate_header_size * 8U;

} // namespace

QPL_UNIT_API_ALGORITHMIC_TEST(huffman_table_deserialize, valid_table) {
    const auto serialized = make_serialized_table();
    ASSERT_FALSE(serialized.empty());

    // Sanity check of the offsets: a table built by the library has consistent header sizes
    const uint32_t isal_bits = read_field(serialized, isal_header_count_offset()) * 8U +
                               read_field(serialized, isal_header_extra_bits_offset());
    ASSERT_NE(0U, isal_bits);
    ASSERT_EQ(isal_bits, read_field(serialized, compression_deflate_header_offset()));
    ASSERT_EQ(isal_bits, read_field(serialized, decompression_deflate_header_offset()));

    qpl_huffman_table_t table = nullptr;
    ASSERT_EQ(QPL_STS_OK,
              qpl_huffman_table_deserialize(serialized.data(), serialized.size(), DEFAULT_ALLOCATOR_C, &table));
    EXPECT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(table));
}

QPL_UNIT_API_ALGORITHMIC_TEST(huffman_table_deserialize, isal_header_count) {
    const auto serialized = make_serialized_table();
    ASSERT_FALSE(serialized.empty());
    const auto offset = isal_header_count_offset();

    EXPECT_EQ(QPL_STS_OK, deserialize_with_field(serialized, offset, ISAL_DEF_MAX_HDR_SIZE - 1U));
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, deserialize_with_field(serialized, offset, ISAL_DEF_MAX_HDR_SIZE));
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, deserialize_with_field(serialized, offset, UINT32_MAX));
}

QPL_UNIT_API_ALGORITHMIC_TEST(huffman_table_deserialize, isal_header_extra_bits) {
    const auto serialized = make_serialized_table();
    ASSERT_FALSE(serialized.empty());
    const auto offset = isal_header_extra_bits_offset();

    EXPECT_EQ(QPL_STS_OK, deserialize_with_field(serialized, offset, 7U));
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, deserialize_with_field(serialized, offset, 8U));
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, deserialize_with_field(serialized, offset, UINT32_MAX));
}

QPL_UNIT_API_ALGORITHMIC_TEST(huffman_table_deserialize, compression_deflate_header_bit_size) {
    const auto serialized = make_serialized_table();
    ASSERT_FALSE(serialized.empty());
    const auto offset = compression_deflate_header_offset();

    EXPECT_EQ(QPL_STS_OK, deserialize_with_field(serialized, offset, max_deflate_header_bits));
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, deserialize_with_field(serialized, offset, max_deflate_header_bits + 1U));
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, deserialize_with_field(serialized, offset, UINT32_MAX));
}

QPL_UNIT_API_ALGORITHMIC_TEST(huffman_table_deserialize, decompression_deflate_header_bit_size) {
    const auto serialized = make_serialized_table();
    ASSERT_FALSE(serialized.empty());
    const auto offset = decompression_deflate_header_offset();

    EXPECT_EQ(QPL_STS_OK, deserialize_with_field(serialized, offset, max_deflate_header_bits));
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, deserialize_with_field(serialized, offset, max_deflate_header_bits + 1U));
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, deserialize_with_field(serialized, offset, UINT32_MAX));
}

// ------ Known-bad tables: each must be rejected ------ //

QPL_UNIT_API_ALGORITHMIC_TEST(huffman_table_deserialize, reject_compression_table_isal_header_too_large) {
    const auto serialized = make_serialized_table(compression_table_type);
    ASSERT_FALSE(serialized.empty());

    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR,
              deserialize_with_field(serialized, isal_header_count_offset(), ISAL_DEF_MAX_HDR_SIZE));
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, deserialize_with_field(serialized, isal_header_extra_bits_offset(), 8U));
}

QPL_UNIT_API_ALGORITHMIC_TEST(huffman_table_deserialize, reject_compression_table_deflate_header_too_large) {
    const auto serialized = make_serialized_table(compression_table_type);
    ASSERT_FALSE(serialized.empty());

    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR,
              deserialize_with_field(serialized, compression_deflate_header_offset(), max_deflate_header_bits + 1U));
}

QPL_UNIT_API_ALGORITHMIC_TEST(huffman_table_deserialize, reject_decompression_table_deflate_header_too_large) {
    const auto serialized = make_serialized_table(decompression_table_type);
    ASSERT_FALSE(serialized.empty());
    const auto offset = decompression_deflate_header_offset(decompression_table_type);

    // Sanity check of the offset before corrupting it
    ASSERT_NE(0U, read_field(serialized, offset));
    ASSERT_GE(max_deflate_header_bits, read_field(serialized, offset));

    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, deserialize_with_field(serialized, offset, max_deflate_header_bits + 1U));
}

QPL_UNIT_API_ALGORITHMIC_TEST(huffman_table_deserialize, reject_combined_table_all_headers_too_large) {
    std::vector<uint8_t> serialized = make_serialized_table();
    ASSERT_FALSE(serialized.empty());

    const uint32_t too_large_count = ISAL_DEF_MAX_HDR_SIZE;
    const uint32_t too_large_bits  = max_deflate_header_bits + 1U;
    std::memcpy(serialized.data() + isal_header_count_offset(), &too_large_count, sizeof(too_large_count));
    std::memcpy(serialized.data() + compression_deflate_header_offset(), &too_large_bits, sizeof(too_large_bits));

    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR,
              deserialize_with_field(serialized, decompression_deflate_header_offset(), too_large_bits));
}

} // namespace qpl::test
