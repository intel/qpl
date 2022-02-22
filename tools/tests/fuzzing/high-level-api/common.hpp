/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TESTS_FUZZING_HIGH_LEVEL_API_COMMON_HPP
#define QPL_TESTS_FUZZING_HIGH_LEVEL_API_COMMON_HPP

#include "qpl/cpp_api/operations/analytics/expand_operation.hpp"
#include "qpl/cpp_api/operations/analytics/extract_operation.hpp"
#include "qpl/cpp_api/operations/analytics/scan_operation.hpp"
#include "qpl/cpp_api/operations/analytics/scan_range_operation.hpp"
#include "qpl/cpp_api/operations/analytics/set_membership_operation.hpp"

struct common_properties {
    uint16_t destination_size = 0;
};

struct analytics_properties : public common_properties {
    uint32_t input_bit_width    = 0;
    uint32_t output_bit_width   = 0;
    size_t   number_of_elements = 0;
};

struct expand_properties : public analytics_properties {
    mutable size_t mask_byte_length = 0;
};

struct extract_properties : public analytics_properties {
    uint32_t lower_boundary_ = 0;
    uint32_t upper_boundary_ = std::numeric_limits<uint32_t>::max();
};

struct scan_properties : public analytics_properties {
    uint32_t         boundary   = 0;
    qpl::comparators comparator = qpl::less;
};

struct scan_range_properties : public analytics_properties {
    uint32_t lower_boundary_ = 0;
    uint32_t upper_boundary_ = std::numeric_limits<uint8_t>::max();
};

struct set_membership_properties : public analytics_properties {
    mutable size_t mask_byte_length    = 0;
    uint32_t       high_bits_to_ignore = 0;
    uint32_t       low_bits_to_ignore  = 0;
};

template <qpl::parsers parser>
auto expand_test_case(const uint8_t *data, size_t size) -> void {
    if (size > sizeof(expand_properties)) {
        auto *properties_ptr = reinterpret_cast<const expand_properties *>(data);

        auto mask_byte_length = properties_ptr->mask_byte_length % 4096;
        auto mask             = data + sizeof(expand_properties);

        if (size > sizeof(expand_properties) + mask_byte_length) {
            auto *source_begin = mask + mask_byte_length;
            auto *source_end   = data + size;

            auto *destination_ptr = new uint8_t[properties_ptr->destination_size];

            try {
                auto expand_operation = qpl::expand_operation::builder(mask,
                                                                       mask_byte_length)
                        .parser<parser>(properties_ptr->number_of_elements)
                        .output_vector_width(properties_ptr->output_bit_width)
                        .input_vector_width(properties_ptr->input_bit_width)
                        .build();

                auto result = qpl::execute(expand_operation,
                                           source_begin,
                                           source_end,
                                           destination_ptr,
                                           destination_ptr + properties_ptr->destination_size);

                result.if_absent([](uint32_t status) -> void {
                    // Valid behavior as the operation result is unpredictable
                });
            } catch (qpl::invalid_argument_exception &e) {
                // Handling Exceptions on misconfigured operation
            }

            delete[] destination_ptr;
        }
    }
}

template <qpl::parsers parser>
auto extract_test_case(const uint8_t *data, size_t size) -> void {
    if (size > sizeof(extract_properties)) {
        auto *properties_ptr = reinterpret_cast<const extract_properties *>(data);

        auto *source_begin = data + sizeof(extract_properties);
        auto *source_end = data + size;

        auto *destination_ptr = new uint8_t[properties_ptr->destination_size];

        try {
            auto extract_operation = qpl::extract_operation::builder(properties_ptr->lower_boundary_,
                properties_ptr->upper_boundary_)
                .parser<parser>(properties_ptr->number_of_elements)
                .output_vector_width(properties_ptr->output_bit_width)
                .input_vector_width(properties_ptr->input_bit_width)
                .build();

            auto result = qpl::execute(extract_operation,
                source_begin,
                source_end,
                destination_ptr,
                destination_ptr + properties_ptr->destination_size);

            result.if_absent([](uint32_t status) -> void {
                // Valid behavior as the operation result is unpredictable
                });
        }
        catch (qpl::invalid_argument_exception &e) {
            // Handling Exceptions on misconfigured operation
        }

        delete[] destination_ptr;
    }
}

template <qpl::parsers parser>
auto scan_test_case(const uint8_t *data, size_t size) -> void {
    if (size > sizeof(scan_properties)) {
        auto *properties_ptr = reinterpret_cast<const scan_properties *>(data);

        auto *source_begin = data + sizeof(scan_properties);
        auto *source_end   = data + size;

        for (bool is_inclusive : {false, true}) {
            auto *destination_ptr = new uint8_t[properties_ptr->destination_size];
            try {
                auto scan_operation = qpl::scan_operation::builder(properties_ptr->comparator, properties_ptr->boundary)
                        .parser<parser>(properties_ptr->number_of_elements)
                        .output_vector_width(properties_ptr->output_bit_width)
                        .input_vector_width(properties_ptr->input_bit_width)
                        .is_inclusive(is_inclusive)
                        .build();

                auto result = qpl::execute(scan_operation,
                                           source_begin,
                                           source_end,
                                           destination_ptr,
                                           destination_ptr + properties_ptr->destination_size);

                result.if_absent([](uint32_t status) -> void {
                    // Valid behavior as the operation result is unpredictable
                });
            } catch (qpl::invalid_argument_exception &e) {
                // Handling Exceptions on misconfigured operation
            }

            delete[] destination_ptr;
        }
    }
}

template <qpl::parsers parser>
auto scan_range_test_case(const uint8_t *data, size_t size) -> void {
    if (size > sizeof(scan_properties)) {
        auto *properties_ptr = reinterpret_cast<const scan_range_properties *>(data);

        auto *source_begin = data + sizeof(scan_range_properties);
        auto *source_end   = data + size;

        for (bool is_inclusive : {false, true}) {
            auto *destination_ptr = new uint8_t[properties_ptr->destination_size];
            try {
                auto scan_operation = qpl::scan_range_operation::builder(properties_ptr->lower_boundary_,
                                                                         properties_ptr->upper_boundary_)
                        .parser<parser>(properties_ptr->number_of_elements)
                        .output_vector_width(properties_ptr->output_bit_width)
                        .input_vector_width(properties_ptr->input_bit_width)
                        .is_inclusive(is_inclusive)
                        .build();

                auto result = qpl::execute(scan_operation,
                                           source_begin,
                                           source_end,
                                           destination_ptr,
                                           destination_ptr + properties_ptr->destination_size);

                result.if_absent([](uint32_t status) -> void {
                    // Valid behavior as the operation result is unpredictable
                });
            } catch (qpl::invalid_argument_exception &e) {
                // Handling Exceptions on misconfigured operation
            }

            delete[] destination_ptr;
        }
    }
}

template <qpl::parsers parser>
auto set_membership_test_case(const uint8_t *data, size_t size) -> void {
    if (size > sizeof(set_membership_properties)) {
        auto *properties_ptr = reinterpret_cast<const set_membership_properties *>(data);

        auto mask_byte_length = properties_ptr->mask_byte_length % 4096;
        auto mask             = data + sizeof(set_membership_properties);

        if (size > sizeof(set_membership_properties) + mask_byte_length) {
            auto *source_begin = mask + mask_byte_length;
            auto *source_end   = data + size;

            auto *destination_ptr = new uint8_t[properties_ptr->destination_size];

            try {
                auto set_membership_operation = qpl::set_membership_operation::builder(mask,
                                                                                       mask_byte_length)
                        .parser<parser>(properties_ptr->number_of_elements)
                        .output_vector_width(properties_ptr->output_bit_width)
                        .input_vector_width(properties_ptr->input_bit_width)
                        .number_low_order_bits_to_ignore(properties_ptr->low_bits_to_ignore)
                        .number_high_order_bits_to_ignore(properties_ptr->high_bits_to_ignore)
                        .build();

                auto result = qpl::execute(set_membership_operation,
                                           source_begin,
                                           source_end,
                                           destination_ptr,
                                           destination_ptr + properties_ptr->destination_size);

                result.if_absent([](uint32_t status) -> void {
                    // Valid behavior as the operation result is unpredictable
                });
            } catch (qpl::invalid_argument_exception &e) {
                // Handling Exceptions on misconfigured operation
            }

            delete[] destination_ptr;
        }
    }
}

#endif // QPL_TESTS_FUZZING_HIGH_LEVEL_API_COMMON_HPP
