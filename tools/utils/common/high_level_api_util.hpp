/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TEST_HIGH_LEVEL_API_UTIL_HPP
#define QPL_TEST_HIGH_LEVEL_API_UTIL_HPP

#include "cstdint"

#include "qpl/qpl.h"
#include "qpl/qpl.hpp"
#include "gtest/gtest.h"
#include "qpl_test_environment.hpp"

namespace qpl::test {

static constexpr uint32_t max_chunk_size = 32768u;
static constexpr uint32_t min_chunk_size = 512u;

static auto convert_parser(parsers parser) -> qpl_parser {
    switch (parser) {
        case little_endian_packed_array:
            return qpl_p_le_packed_array;
        case big_endian_packed_array:
            return qpl_p_be_packed_array;
        default:
            return qpl_p_parquet_rle;
    }
}

static auto handle_result(const execution_result<uint32_t> &result) -> uint32_t {
    uint32_t result_value = 0;
    result.handle([&result_value](uint32_t value) -> void {
        result_value = value;
    }, [](uint32_t status) -> void {
        throw std::runtime_error("Error: Status code - " + std::to_string(status));
    });

    return result_value;
}

template <class operation_t, class param_t, class ...params_t>
constexpr auto contains_operation() -> bool {
    if constexpr (sizeof...(params_t) > 0) {
        return std::is_same<operation_t, param_t>::value || contains_operation<operation_t, params_t...>();
    }
    return std::is_same<operation_t, param_t>::value;
}

template <class param_t, class ...params_t>
constexpr auto contains_array_producer() -> bool {
    if constexpr (sizeof...(params_t) > 0) {
        return std::is_same<expand_operation, param_t>::value ||
               std::is_same<extract_operation, param_t>::value ||
               std::is_same<select_operation, param_t>::value ||
               contains_array_producer<params_t...>();
    }
    return std::is_same<expand_operation, param_t>::value ||
           std::is_same<extract_operation, param_t>::value ||
           std::is_same<select_operation, param_t>::value;
}

template <class param_t, class ...params_t>
constexpr auto contains_mask_producer() -> bool {
    if constexpr (sizeof...(params_t) > 0) {
        return std::is_same<scan_operation, param_t>::value ||
               std::is_same<scan_range_operation, param_t>::value ||
               std::is_same<set_membership_operation, param_t>::value ||
               contains_mask_producer<params_t...>();
    }
    return std::is_same<scan_operation, param_t>::value ||
           std::is_same<scan_range_operation, param_t>::value ||
           std::is_same<set_membership_operation, param_t>::value;
}

template <class ...operations_t, class test_case_type_t>
auto is_valid_test_case(const test_case_type_t &test_case) -> bool {
    if constexpr (contains_array_producer<operations_t ...>()) {
        if (test_case.input_bit_width == 1u && test_case.output_bit_width > 1u) {
            if ((1ull << test_case.output_bit_width) <= test_case.number_of_input_elements) {
                return false;
            }
        } else {
            if (test_case.output_bit_width != 1u && test_case.input_bit_width > test_case.output_bit_width) {
                return false;
            }
        }
    }

    if constexpr (contains_mask_producer<operations_t ...>()) {
        if (test_case.output_bit_width > 1u &&
            (1ull << test_case.output_bit_width) <= test_case.number_of_input_elements) {
            return false;
        }
    }

    if constexpr (contains_operation<find_unique_operation, operations_t ...>()) {
        if (test_case.output_bit_width > 1u) {
            return false;
        }
    }

    return true;
}

template <class ...operations_t, class test_case_type_t>
auto get_destination_size(const test_case_type_t &test_case) -> size_t {
    size_t destination_size = 0;
    if constexpr (contains_operation<find_unique_operation, operations_t ...>()) {
        uint32_t bits_to_ignore = (test_case.input_bit_width / 3) * 2;
        destination_size = std::max(destination_size,
                                    static_cast<size_t>((1ull << (test_case.input_bit_width - bits_to_ignore))));
    }

    size_t output_element_bit_width = (test_case.output_bit_width == 1 ?
                                       test_case.input_bit_width :
                                       test_case.output_bit_width
    );

    destination_size = std::max(destination_size,
                                static_cast<size_t>((output_element_bit_width *
                                                     test_case.number_of_input_elements + 7
                                ) >> 3));

    return destination_size;
}

template <class param, class ...params>
constexpr auto filter_operation_exist_and_not_last() -> bool {
    if constexpr ((std::is_same<select_operation, param>::value ||
                   std::is_same<extract_operation, param>::value ||
                   std::is_same<set_membership_operation, param>::value
                  ) &&
                  sizeof...(params) > 0) {
        return true;
    }

    if constexpr (sizeof...(params) > 0) {
        return filter_operation_exist_and_not_last<params ...>();
    }

    return false;
}

template <class operation_t, class input_iterator_t, class output_iterator_t>
static inline auto execute(operation_t &operation,
                           input_iterator_t source_begin,
                           input_iterator_t source_end,
                           output_iterator_t destination_begin,
                           output_iterator_t destination_end) -> execution_result<uint32_t, sync> {
    switch (util::TestEnvironment::GetInstance().GetExecutionPath()) {
        case qpl_path_software: {
            return qpl::execute<qpl::software>(operation,
                                               source_begin,
                                               source_end,
                                               destination_begin,
                                               destination_end);
        }
        case qpl_path_hardware: {
            return qpl::execute<qpl::hardware>(operation,
                                               source_begin,
                                               source_end,
                                               destination_begin,
                                               destination_end);
        }
        default: {
            return qpl::execute<qpl::auto_detect>(operation,
                                                  source_begin,
                                                  source_end,
                                                  destination_begin,
                                                  destination_end);
        }
    }
}

template <class operation_t, class input_container_t, class output_container_t>
static inline auto execute(operation_t &operation,
                           input_container_t &source,
                           output_container_t &destination) -> execution_result<uint32_t, sync> {
    switch (util::TestEnvironment::GetInstance().GetExecutionPath()) {
        case qpl_path_software: {
            return qpl::execute<qpl::software>(operation, source, destination);
        }
        case qpl_path_hardware: {
            return qpl::execute<qpl::hardware>(operation, source, destination);
        }
        default: {
            return qpl::execute<qpl::auto_detect>(operation, source, destination);
        }
    }
}

static inline auto build_deflate_block(deflate_operation &operation,
                                       std::vector<uint8_t> &source,
                                       mini_block_sizes mini_block_size) -> deflate_block<std::allocator> {
    switch (util::TestEnvironment::GetInstance().GetExecutionPath()) {
        case qpl_path_software: {
            return qpl::build_deflate_block<qpl::software>(operation,
                                                           source.begin(),
                                                           source.end(),
                                                           mini_block_size);
        }
        case qpl_path_hardware: {
            return qpl::build_deflate_block<qpl::hardware>(operation,
                                                           source.begin(),
                                                           source.end(),
                                                           mini_block_size);
        }
        default: {
            return qpl::build_deflate_block<qpl::auto_detect>(operation,
                                                              source.begin(),
                                                              source.end(),
                                                              mini_block_size);
        }
    }
}

template <class input_iterator_t>
static inline auto update_deflate_statistics(const input_iterator_t source_begin,
                                             const input_iterator_t source_end,
                                             deflate_histogram &histogram,
                                             compression_levels level) -> void {
    switch (util::TestEnvironment::GetInstance().GetExecutionPath()) {
        case qpl_path_software: {
            qpl::update_deflate_statistics<qpl::software>(source_begin, source_end, histogram, level);
            break;
        }
        case qpl_path_hardware: {
            qpl::update_deflate_statistics<qpl::hardware>(source_begin, source_end, histogram, level);
            break;
        }
        default: {
            qpl::update_deflate_statistics<qpl::auto_detect>(source_begin, source_end, histogram, level);
            break;
        }
    }
}

static inline auto make_deflate_table(deflate_histogram &histogram) -> huffman_table<huffman_table_type::deflate> {
    switch (util::TestEnvironment::GetInstance().GetExecutionPath()) {
        case qpl_path_software: {
            return qpl::make_deflate_table<qpl::software>(histogram);
        }
        case qpl_path_hardware: {
            return qpl::make_deflate_table<qpl::hardware>(histogram);
        }
        default: {
            return qpl::make_deflate_table<qpl::auto_detect>(histogram);
        }
    }
}

static inline auto make_inflate_table(huffman_table<huffman_table_type::deflate> &deflate_table)
-> qpl::huffman_table<huffman_table_type::inflate> {
    switch (util::TestEnvironment::GetInstance().GetExecutionPath()) {
        case qpl_path_software: {
            return qpl::make_inflate_table<qpl::software>(deflate_table);
        }
        case qpl_path_hardware: {
            return qpl::make_inflate_table<qpl::hardware>(deflate_table);
        }
        default: {
            return qpl::make_inflate_table<qpl::auto_detect>(deflate_table);
        }
    }
}

static inline auto mini_block_size_to_string(mini_block_sizes size) -> std::string {
    switch (size) {
        case mini_block_size_none:
            return "None";
        default:
            return std::to_string((1ull << (size + 8)));
    }
}

template <class Iterator>
static inline auto check_deflate_block(const Iterator &source_begin,
                                       const Iterator &source_end,
                                       deflate_block<std::allocator> &deflate_block) -> testing::AssertionResult {
    size_t source_size = std::distance(source_begin, source_end);

    if (source_size != deflate_block.size()) {
        return testing::AssertionFailure() << "deflate block and source contain different number of elements";
    }

    if (deflate_block.compressed_size() == 0) {
        return testing::AssertionFailure() << "deflate block's compressed_size is equal to zero";
    }

    for (size_t start_index : {0, 1}) {
        auto        iterator = source_begin + start_index;
        for (size_t index    = start_index; index < deflate_block.size(); index += 2) {
            if (*iterator != deflate_block[index]) {
                return testing::AssertionFailure() << "source[" << index << "] = " << *iterator << "\n"
                                                   << "deflate_block[" << index << "] = " << deflate_block[index];
            }
            if (index + 2 < deflate_block.size()) {
                iterator += 2;
            }
        }
    }

    return testing::AssertionSuccess();
}

} // namespace qpl::test

#endif // QPL_TEST_HIGH_LEVEL_API_UTIL_HPP
