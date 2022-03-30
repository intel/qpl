/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "canned_utils.hpp"
#include "compression/inflate/inflate.hpp"
#include "util/memory.hpp"
#include "util/util.hpp"
#include "util/descriptor_processing.hpp"

#include <array>
#include <algorithm>
#include <cstdlib>

namespace qpl::ml::compression {

auto initialize_inflate_state_from_deflate_header(uint8_t *deflate_header_data_ptr,
                                                  uint32_t deflate_header_bit_size,
                                                  isal_inflate_state *isal_state_ptr) noexcept -> qpl_ml_status {
    // Prepare inflate state to parse Deflate header
    constexpr auto end_processing_condition = end_processing_condition_t::stop_and_check_for_bfinal_eob;

    uint32_t deflate_header_byte_size = util::bit_to_byte(deflate_header_bit_size);

    isal_state_ptr->next_in   = deflate_header_data_ptr;
    isal_state_ptr->avail_in  = deflate_header_byte_size;
    isal_state_ptr->next_out  = deflate_header_data_ptr; // No rewrites
    isal_state_ptr->avail_out = 0u;

    allocation_buffer_t buffer(reinterpret_cast<uint8_t*>(isal_state_ptr),
                               reinterpret_cast<uint8_t*>(isal_state_ptr) + sizeof(isal_inflate_state));

    auto state = inflate_state<qpl::ml::execution_path_t::software>::create(buffer);

    auto status = inflate<execution_path_t::software>(state, end_processing_condition).status_code_;

    // This is work-around, current inflate function can perform not just deflate header decompression
    // but go further and perform decompression of deflate block, which may cause the following error. Ignore it.
    // TODO: fix
    if (status_list::compression_reference_before_start == status) {
        status = status_list::ok;
    }

    isal_state_ptr->tmp_out_valid = 0;

    return status;
}

inline auto triplets_to_sw_compression_table(const qpl_triplet *triplets_ptr,
                                             std::size_t triplets_count,
                                             qplc_huffman_table_default_format *compression_table) -> qpl_ml_status {
    for (std::size_t i = 0; i < triplets_count; i++) {
        qpl_triplet current_triplet = triplets_ptr[i];

        uint32_t literal_length_table_index = current_triplet.character;

        qplc_huffman_table_write_ll_code(compression_table, literal_length_table_index, current_triplet.code);
        qplc_huffman_table_write_ll_code_length(compression_table, literal_length_table_index, current_triplet.code_length);
    }

    return status_list::ok;
}

inline auto triplets_code_values_comparator(const void *a, const void *b) noexcept -> int {
    auto first_triplet  = reinterpret_cast<const qpl_triplet *>(a);
    auto second_triplet = reinterpret_cast<const qpl_triplet *>(b);

    return (int) first_triplet->code > second_triplet->code;
}

void triplets_to_sw_decompression_table(const qpl_triplet *triplets_ptr,
                                        size_t triplets_count,
                                        qplc_huffman_table_flat_format *decompression_table_ptr) noexcept {
    // Variables
    uint32_t empty_position = 0u;

    // Calculate code lengths histogram
    std::for_each(triplets_ptr, triplets_ptr + triplets_count,
                  [decompression_table_ptr](const qpl_triplet &item) {
                      if (item.code_length != 0) {
                          decompression_table_ptr->number_of_codes[item.code_length - 1u]++;
                      }
                  });

    // Calculate first codes
    for (uint32_t i = 1u; i <= 15u; i++) {
        std::array<qpl_triplet, 256> filtered{};

        if (decompression_table_ptr->number_of_codes[i - 1u] == 0) {
            continue;
        }

        // Filtering by code length
        const auto last_filtered = std::copy_if(triplets_ptr,
                                                triplets_ptr + triplets_count,
                                                filtered.begin(),
                                                [i](const qpl_triplet triplet) {
                                                    return triplet.code_length == i;
                                                });

        // Sorting to get the right order for mapping table (charToSortedCode)
        size_t number_of_elements_to_sort = std::distance(filtered.begin(), last_filtered);
        qsort(filtered.data(), number_of_elements_to_sort, sizeof(qpl_triplet), triplets_code_values_comparator);

        decompression_table_ptr->first_codes[i - 1u]         = filtered[0].code;
        decompression_table_ptr->first_table_indexes[i - 1u] = empty_position;

        // Writing of sorted codes
        const uint32_t start_position = empty_position;

        while (empty_position < (start_position + std::distance(filtered.begin(), last_filtered))) {
            decompression_table_ptr->index_to_char[empty_position] = filtered[empty_position -
                                                                              start_position].character;
            empty_position++;
        }
    }
}

void convert_software_tables(qplc_huffman_table_default_format *compression_table_ptr,
                             qplc_huffman_table_flat_format *decompression_table_ptr) noexcept {
    std::array<qpl_triplet, 256u> triplets_array = {};

    for (uint32_t i = 0; i < 256u; i++) {
        triplets_array[i].character   = i;
        triplets_array[i].code_length = qplc_huffman_table_get_ll_code_length(compression_table_ptr, i);
        triplets_array[i].code        = qplc_huffman_table_get_ll_code(compression_table_ptr, i);
    }

    triplets_to_sw_decompression_table(triplets_array.data(), triplets_array.size(), decompression_table_ptr);
}

auto triplets_to_compression_table(const qpl_triplet *triplets_ptr,
                                   std::size_t triplets_count,
                                   compression_huffman_table &compression_table) noexcept -> qpl_ml_status {
    if (compression_table.is_sw_compression_table_used()) {
        triplets_to_sw_compression_table(triplets_ptr, triplets_count, compression_table.get_sw_compression_table());
    }

    if (compression_table.get_hw_compression_table()) {
        // just a stab there
    }

    return status_list::ok;
}

auto triplets_to_decompression_table(const qpl_triplet *triplets_ptr,
                                     size_t triplets_count,
                                     decompression_huffman_table &decompression_table) noexcept -> qpl_ml_status {
    if (decompression_table.is_sw_decompression_table_used()) {
        triplets_to_sw_decompression_table(triplets_ptr,
                                           triplets_count,
                                           decompression_table.get_sw_decompression_table());
    }

    if (decompression_table.is_hw_decompression_table_used()) {
        // just a stab there
    }

    return status_list::ok;
}

auto inline validate_representation_flags(compression_huffman_table &compression_table,
                                          decompression_huffman_table &decompression_table) noexcept -> qpl_ml_status {
    if (decompression_table.is_deflate_header_used() &&
        !compression_table.is_deflate_header_used()) {
        return status_list::status_invalid_params;
    }

    if (decompression_table.is_sw_decompression_table_used() &&
        !compression_table.is_sw_compression_table_used()) {
        return status_list::status_invalid_params;
    }

    if (decompression_table.is_hw_decompression_table_used() &&
        !compression_table.is_hw_compression_table_used()) {
        return status_list::status_invalid_params;
    }

    return status_list::ok;
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage=4096"
#endif

auto comp_to_decompression_table(compression_huffman_table &compression_table,
                                 decompression_huffman_table &decompression_table) noexcept -> qpl_ml_status {
    auto validation_status = validate_representation_flags(compression_table, decompression_table);

    if (status_list::ok != validation_status) {
        return validation_status;
    }

    if (decompression_table.is_deflate_header_used()) {
        // Copy deflate header from compression to decompression table
        auto deflate_header_byte_size = util::bit_to_byte(compression_table.get_deflate_header_bit_size());

        util::copy(compression_table.get_deflate_header_data(),
            compression_table.get_deflate_header_data() + deflate_header_byte_size,
            decompression_table.get_deflate_header_data());

        decompression_table.set_deflate_header_bit_size(compression_table.get_deflate_header_bit_size());

        isal_inflate_state temporary_state = {nullptr, 0u, 0u, nullptr, 0u, 0u, 0, {{0u}, {0u}}, 
                                              {{0u}, {0u}}, (isal_block_state)0, 0u, 0u, 0u, 0u, 0u,
                                              0u, 0, 0, 0, 0, 0u, 0, 0, 0, 0, {0u}, {0u}, 0u, 0u, 0u};

        // Parse deflate header and load it into the temporary state
        auto status = initialize_inflate_state_from_deflate_header(decompression_table.get_deflate_header_data(),
                                                                   decompression_table.get_deflate_header_bit_size(),
                                                                   &temporary_state);

        // Copy lookup tables from temporary state to decompression table
        auto *lit_huff_code_ptr = reinterpret_cast<uint8_t *>(&temporary_state.lit_huff_code);

        util::copy(lit_huff_code_ptr,
                   lit_huff_code_ptr + sizeof(temporary_state.lit_huff_code),
                   reinterpret_cast<uint8_t *>(&decompression_table.get_canned_table()->literal_huffman_codes));

        auto *dist_huff_code_ptr = reinterpret_cast<uint8_t *>(&temporary_state.dist_huff_code);

        util::copy(dist_huff_code_ptr,
                   dist_huff_code_ptr + sizeof(temporary_state.dist_huff_code),
                   reinterpret_cast<uint8_t *>(&decompression_table.get_canned_table()->distance_huffman_codes));

        // Copy eob symbol properties
        decompression_table.get_canned_table()->eob_code_and_len = temporary_state.eob_code_and_len;

        decompression_table.get_canned_table()->is_final_block = (temporary_state.bfinal == 1);

        if (status_list::ok != status) {
            return status;
        }
    }

    if (decompression_table.is_sw_decompression_table_used()) {
        convert_software_tables(compression_table.get_sw_compression_table(),
                                decompression_table.get_sw_decompression_table());
    }

    if (decompression_table.is_hw_decompression_table_used()) {
        hw_descriptor HW_PATH_ALIGN_STRUCTURE descriptor;
        HW_PATH_VOLATILE hw_completion_record HW_PATH_ALIGN_STRUCTURE completion_record;

        std::fill(descriptor.data, descriptor.data + HW_PATH_DESCRIPTOR_SIZE, 0u);

        auto *const header_ptr = compression_table.get_deflate_header_data();
        const auto header_bit_size = compression_table.get_deflate_header_bit_size();
        hw_iaa_aecs *const aecs_ptr = decompression_table.get_hw_decompression_state();

        util::set_zeros(aecs_ptr, sizeof(hw_iaa_aecs_analytic));

        uint32_t input_bytes_count = (header_bit_size + 7u) >> 3u;
        uint8_t  ignore_end_bits   = max_bit_index & (0u - header_bit_size);

        hw_iaa_descriptor_set_input_buffer(&descriptor, header_ptr, input_bytes_count);

        hw_iaa_descriptor_init_inflate_header(&descriptor,
                                              reinterpret_cast<hw_iaa_aecs_analytic *>(aecs_ptr),
                                              ignore_end_bits,
                                              hw_aecs_toggle_rw);

        hw_iaa_descriptor_set_completion_record(&descriptor, &completion_record);

        return ml::util::process_descriptor<qpl_ml_status,
                                            ml::util::execution_mode_t::sync>(&descriptor,
                                                                              &completion_record);
    }

    return status_list::ok;
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
} // namespace qpl::ml::compression

#include "compression/huffman_table/compression_table.hpp"
#include "compression/huffman_table/decompression_table.hpp"
#include "compression/huffman_table/canned_utils.hpp"
#include "compression/huffman_table/compression_table_utils.hpp"

static_assert(sizeof(qplc_huffman_table_default_format) <=
              sizeof(qpl_compression_huffman_table::sw_compression_table_data));

uint32_t own_comp_to_decompression_table(const qpl_compression_huffman_table *compression_table_ptr,
                                         qpl_decompression_huffman_table *decompression_table_ptr,
                                         uint32_t representation_flags) {
    using namespace qpl::ml::compression;

    // This is a workaround, because currently compression_huffman_table cannot accept pointer to const data
    // as argument in it's constructor, TODO: remove const cast
    auto casted_compression_table = const_cast<qpl_compression_huffman_table *>(compression_table_ptr);

    // Setup compression table
    auto sw_compression_table_data_ptr   = reinterpret_cast<uint8_t *>(&casted_compression_table->sw_compression_table_data);
    auto isal_compression_table_data_ptr = reinterpret_cast<uint8_t *>(&casted_compression_table->isal_compression_table_data);
    auto hw_compression_table_data_ptr   = reinterpret_cast<uint8_t *>(&casted_compression_table->hw_compression_table_data);
    auto comp_deflate_header_buffer_ptr  = reinterpret_cast<uint8_t *>(&casted_compression_table->deflate_header_buffer);

    compression_huffman_table compression_table(sw_compression_table_data_ptr,
                                                isal_compression_table_data_ptr,
                                                hw_compression_table_data_ptr,
                                                comp_deflate_header_buffer_ptr);

    if (compression_table_ptr->representation_mask & QPL_DEFLATE_REPRESENTATION) {
        compression_table.enable_deflate_header();
    }

    if (compression_table_ptr->representation_mask & QPL_SW_REPRESENTATION) {
        compression_table.enable_sw_compression_table();
    }

    if (compression_table_ptr->representation_mask & QPL_HW_REPRESENTATION) {
        compression_table.enable_hw_compression_table();
    }

    auto sw_flattened_table_ptr           = reinterpret_cast<uint8_t *>(&decompression_table_ptr->sw_flattened_table);
    auto hw_decompression_state_ptr       = reinterpret_cast<uint8_t *>(&decompression_table_ptr->hw_decompression_state);
    auto decomp_deflate_header_buffer_ptr = reinterpret_cast<uint8_t *>(&decompression_table_ptr->deflate_header_buffer);
    auto lookup_table_buffer_ptr          = reinterpret_cast<uint8_t *>(&decompression_table_ptr->lookup_table_buffer);

    // Setup decompression table
    decompression_huffman_table decompression_table(sw_flattened_table_ptr,
                                                    hw_decompression_state_ptr,
                                                    decomp_deflate_header_buffer_ptr,
                                                    lookup_table_buffer_ptr);

    if (representation_flags & QPL_DEFLATE_REPRESENTATION) {
        decompression_table.enable_deflate_header();
        decompression_table_ptr->representation_mask |= QPL_DEFLATE_REPRESENTATION;
    }

    if (representation_flags & QPL_SW_REPRESENTATION) {
        decompression_table.enable_sw_decompression_table();
        decompression_table_ptr->representation_mask |= QPL_SW_REPRESENTATION;
    }

    if (representation_flags & QPL_HW_REPRESENTATION) {
        decompression_table.enable_hw_decompression_table();
        decompression_table_ptr->representation_mask |= QPL_HW_REPRESENTATION;
    }

    auto status = comp_to_decompression_table(compression_table, decompression_table);

    return status;
}

uint32_t own_build_compression_table(const uint32_t *literal_lengths_histogram_ptr,
                                     const uint32_t *offsets_histogram_ptr,
                                     qpl_compression_huffman_table *compression_table_ptr,
                                     uint32_t representation_flags) {
    using namespace qpl::ml;
    using namespace qpl::ml::compression;

    auto sw_compression_table_data_ptr   = reinterpret_cast<uint8_t *>(&compression_table_ptr->sw_compression_table_data);
    auto isal_compression_table_data_ptr = reinterpret_cast<uint8_t *>(&compression_table_ptr->isal_compression_table_data);
    auto hw_compression_table_data_ptr   = reinterpret_cast<uint8_t *>(&compression_table_ptr->hw_compression_table_data);
    auto deflate_header_buffer_ptr       = reinterpret_cast<uint8_t *>(&compression_table_ptr->deflate_header_buffer);

    compression_huffman_table compression_table(sw_compression_table_data_ptr,
                                                isal_compression_table_data_ptr,
                                                hw_compression_table_data_ptr,
                                                deflate_header_buffer_ptr);

    if (representation_flags & QPL_DEFLATE_REPRESENTATION) {
        compression_table.enable_deflate_header();
        compression_table_ptr->representation_mask |= QPL_DEFLATE_REPRESENTATION;
    }

    if (representation_flags & QPL_SW_REPRESENTATION) {
        compression_table.enable_sw_compression_table();
        compression_table_ptr->representation_mask |= QPL_SW_REPRESENTATION;
    }

    if (representation_flags & QPL_HW_REPRESENTATION) {
        compression_table.enable_hw_compression_table();
        compression_table_ptr->representation_mask |= QPL_HW_REPRESENTATION;
    }

    if (representation_flags & QPL_HUFFMAN_ONLY_REPRESENTATION) {
        compression_table.make_huffman_only();
        compression_table_ptr->representation_mask |= QPL_HUFFMAN_ONLY_REPRESENTATION;
    }

    auto status = build_compression_table(literal_lengths_histogram_ptr, offsets_histogram_ptr, compression_table);

    return status;
}

extern "C" {

uint8_t *get_sw_decompression_table_buffer(qpl_decompression_huffman_table *const decompression_table_ptr) {
    return reinterpret_cast<uint8_t *>(&decompression_table_ptr->sw_flattened_table);
}

uint8_t *get_hw_decompression_table_buffer(qpl_decompression_huffman_table *const decompression_table_ptr) {
    return reinterpret_cast<uint8_t *>(&decompression_table_ptr->hw_decompression_state);
}

uint8_t *get_deflate_header_buffer(qpl_decompression_huffman_table *const decompression_table_ptr) {
    return reinterpret_cast<uint8_t *>(&decompression_table_ptr->deflate_header_buffer);
}

bool is_sw_representation_used(qpl_decompression_huffman_table *const decompression_table_ptr) {
    if (decompression_table_ptr->representation_mask & QPL_SW_REPRESENTATION) {
        return 1u;
    } else {
        return 0u;
    }
}

bool is_hw_representation_used(qpl_decompression_huffman_table *const decompression_table_ptr) {
    if (decompression_table_ptr->representation_mask & QPL_HW_REPRESENTATION) {
        return 1u;
    } else {
        return 0u;
    }
}

bool is_deflate_representation_used(qpl_decompression_huffman_table *const decompression_table_ptr) {
    if (decompression_table_ptr->representation_mask & QPL_DEFLATE_REPRESENTATION) {
        return true;
    } else {
        return false;
    }
}

uint8_t *get_lookup_table_buffer_ptr(qpl_decompression_huffman_table *decompression_table_ptr) {
    return reinterpret_cast<uint8_t *>(&decompression_table_ptr->lookup_table_buffer);
}

void *get_aecs_decompress(qpl_decompression_huffman_table *decompression_table_ptr) {
    using namespace qpl::ml::compression;

    auto sw_flattened_table_ptr     = reinterpret_cast<uint8_t *>(&decompression_table_ptr->sw_flattened_table);
    auto hw_decompression_state_ptr = reinterpret_cast<uint8_t *>(&decompression_table_ptr->hw_decompression_state);
    auto deflate_header_buffer_ptr  = reinterpret_cast<uint8_t *>(&decompression_table_ptr->deflate_header_buffer);
    auto lookup_table_buffer_ptr    = reinterpret_cast<uint8_t *>(&decompression_table_ptr->lookup_table_buffer);

    decompression_huffman_table decompression_table(sw_flattened_table_ptr,
                                                    hw_decompression_state_ptr,
                                                    deflate_header_buffer_ptr,
                                                    lookup_table_buffer_ptr);

    return decompression_table.get_hw_decompression_state();
}

uint32_t *get_literals_lengths_table_ptr(qpl_compression_huffman_table *const huffman_table_ptr) {
    using namespace qpl::ml::compression;
    auto sw_compression_table = reinterpret_cast<qplc_huffman_table_default_format *>(&huffman_table_ptr->sw_compression_table_data);

    return sw_compression_table->literals_matches;
}

uint32_t *get_offsets_table_ptr(qpl_compression_huffman_table *const huffman_table_ptr) {
    using namespace qpl::ml::compression;
    auto sw_compression_table = reinterpret_cast<qplc_huffman_table_default_format *>(&huffman_table_ptr->sw_compression_table_data);

    return sw_compression_table->offsets;
}

uint8_t *get_deflate_header_ptr(qpl_compression_huffman_table *const huffman_table_ptr) {
    using namespace qpl::ml::compression;
    return reinterpret_cast<deflate_header *>(&huffman_table_ptr->deflate_header_buffer)->data;
}

uint32_t get_deflate_header_bits_size(qpl_compression_huffman_table *const huffman_table_ptr) {
    using namespace qpl::ml::compression;
    return reinterpret_cast<deflate_header *>(&huffman_table_ptr->deflate_header_buffer)->header_bit_size;
}

void set_deflate_header_bits_size(qpl_compression_huffman_table *const huffman_table_ptr, uint32_t header_bits) {
    using namespace qpl::ml::compression;
    reinterpret_cast<deflate_header *>(&huffman_table_ptr->deflate_header_buffer)->header_bit_size = header_bits;
}

uint8_t *get_sw_compression_huffman_table_ptr(qpl_compression_huffman_table *const huffman_table_ptr) {
    return reinterpret_cast<uint8_t *>(&huffman_table_ptr->sw_compression_table_data);
}

uint8_t * get_isal_compression_huffman_table_ptr(qpl_compression_huffman_table *const huffman_table_ptr) {
    return reinterpret_cast<uint8_t *>(&huffman_table_ptr->isal_compression_table_data);
}

uint8_t * get_hw_compression_huffman_table_ptr(qpl_compression_huffman_table *const huffman_table_ptr) {
    return reinterpret_cast<uint8_t *>(&huffman_table_ptr->hw_compression_table_data);
}

uint16_t *get_number_of_codes_ptr(qpl_decompression_huffman_table *const decompression_table_ptr) {
    using namespace qpl::ml::compression;
    auto sw_decompr_table = reinterpret_cast<qplc_huffman_table_flat_format *>(&decompression_table_ptr->sw_flattened_table);

    return sw_decompr_table->number_of_codes;
}

uint16_t *get_first_codes_ptr(qpl_decompression_huffman_table *const decompression_table_ptr) {
    using namespace qpl::ml::compression;
    auto sw_decompr_table = reinterpret_cast<qplc_huffman_table_flat_format *>(&decompression_table_ptr->sw_flattened_table);

    return sw_decompr_table->first_codes;
}

uint16_t *get_first_table_indexes_ptr(qpl_decompression_huffman_table *const decompression_table_ptr) {
    using namespace qpl::ml::compression;
    auto sw_decompr_table = reinterpret_cast<qplc_huffman_table_flat_format *>(&decompression_table_ptr->sw_flattened_table);

    return sw_decompr_table->first_table_indexes;
}

uint8_t *get_index_to_char_ptr(qpl_decompression_huffman_table *const decompression_table_ptr) {
    using namespace qpl::ml::compression;
    auto sw_decompr_table = reinterpret_cast<qplc_huffman_table_flat_format *>(&decompression_table_ptr->sw_flattened_table);

    return sw_decompr_table->index_to_char;
}
}
