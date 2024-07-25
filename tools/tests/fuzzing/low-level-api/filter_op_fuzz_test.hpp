/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <memory>
#include <vector>

#include "qpl/qpl.h"

enum scan_comparator {
    not_in_range      = 0u,
    in_range          = 1u,
    equals            = 2u,
    not_equals        = 3u,
    less_than         = 4u,
    less_or_equals    = 5u,
    greater_than      = 6u,
    greater_or_equals = 7u,
};

struct common_properties {
    uint16_t destination_size = 0;
};

struct analytics_properties : public common_properties {
    uint32_t       input_bit_width    = 0;
    size_t         number_of_elements = 0;
    qpl_out_format output_bit_width   = qpl_ow_nom;
};

struct scan_properties : public analytics_properties {
    uint32_t        lower_boundary = 0;
    uint32_t        upper_boundary = 0;
    scan_comparator comparator;
};

struct extract_properties : public analytics_properties {
    uint32_t high_index_boundary = 0;
    uint32_t low_index_boundary  = 0;
};

struct select_properties : public analytics_properties {
    size_t   mask_byte_length  = 0;
    uint32_t input_bit_width_2 = 0;
};

struct expand_properties : public analytics_properties {
    size_t   mask_byte_length  = 0;
    uint32_t input_bit_width_2 = 0;
};

static inline qpl_operation convert_comparator_to_scan_operation(scan_comparator comparator) {

    qpl_operation result = qpl_op_scan_eq;

    switch (comparator) {
        case equals: result = qpl_op_scan_eq; break;
        case not_equals: result = qpl_op_scan_ne; break;
        case less_than: result = qpl_op_scan_lt; break;
        case less_or_equals: result = qpl_op_scan_le; break;
        case greater_than: result = qpl_op_scan_gt; break;
        case greater_or_equals: result = qpl_op_scan_ge; break;
        case in_range: result = qpl_op_scan_range; break;
        case not_in_range: result = qpl_op_scan_not_range; break;
        default: break;
    }

    return result;
}

static inline int scan_test_case(const uint8_t* data_ptr, size_t size, qpl_parser parser) {
    if (size > sizeof(scan_properties)) {
        auto*                properties_ptr = reinterpret_cast<const scan_properties*>(data_ptr);
        std::vector<uint8_t> source(data_ptr + sizeof(scan_properties), data_ptr + size);
        std::vector<uint8_t> destination(properties_ptr->destination_size);

        qpl_status status;
        uint32_t   job_size = 0;

        // Job initialization
        status = qpl_get_job_size(qpl_path_software, &job_size);
        if (status != QPL_STS_OK) { return 0; }

        auto     job_buffer = std::make_unique<uint8_t[]>(job_size);
        qpl_job* job_ptr    = reinterpret_cast<qpl_job*>(job_buffer.get());

        status = qpl_init_job(qpl_path_software, job_ptr);
        if (status != QPL_STS_OK) { return 0; }

        qpl_operation operation = convert_comparator_to_scan_operation(properties_ptr->comparator);

        job_ptr->next_in_ptr        = source.data();
        job_ptr->available_in       = source.size();
        job_ptr->next_out_ptr       = destination.data();
        job_ptr->available_out      = destination.size();
        job_ptr->op                 = operation;
        job_ptr->num_input_elements = properties_ptr->number_of_elements;
        job_ptr->src1_bit_width     = properties_ptr->input_bit_width;
        job_ptr->param_low          = properties_ptr->lower_boundary;
        job_ptr->param_high         = properties_ptr->upper_boundary;
        job_ptr->out_bit_width      = properties_ptr->output_bit_width;
        job_ptr->parser             = parser;

        status = qpl_execute_job(job_ptr);
    }

    return 0;
}

static inline int extract_test_case(const uint8_t* data_ptr, size_t size, qpl_parser parser) {
    if (size > sizeof(extract_properties)) {
        auto*                properties_ptr = reinterpret_cast<const extract_properties*>(data_ptr);
        std::vector<uint8_t> source(data_ptr + sizeof(extract_properties), data_ptr + size);
        std::vector<uint8_t> destination(properties_ptr->destination_size);

        qpl_status status;
        uint32_t   job_size = 0;

        // Job initialization
        status = qpl_get_job_size(qpl_path_software, &job_size);
        if (status != QPL_STS_OK) { return 0; }

        auto     job_buffer = std::make_unique<uint8_t[]>(job_size);
        qpl_job* job_ptr    = reinterpret_cast<qpl_job*>(job_buffer.get());

        status = qpl_init_job(qpl_path_software, job_ptr);
        if (status != QPL_STS_OK) { return 0; }

        job_ptr->next_in_ptr        = source.data();
        job_ptr->available_in       = source.size();
        job_ptr->next_out_ptr       = destination.data();
        job_ptr->available_out      = destination.size();
        job_ptr->op                 = qpl_op_extract;
        job_ptr->num_input_elements = properties_ptr->number_of_elements;
        job_ptr->src1_bit_width     = properties_ptr->input_bit_width;
        job_ptr->param_low          = properties_ptr->low_index_boundary;
        job_ptr->param_high         = properties_ptr->high_index_boundary;
        job_ptr->out_bit_width      = properties_ptr->output_bit_width;
        job_ptr->parser             = parser;

        status = qpl_execute_job(job_ptr);
    }

    return 0;
}

static inline int select_test_case(const uint8_t* data_ptr, size_t size, qpl_parser parser) {
    if (size > sizeof(select_properties)) {
        auto* properties_ptr   = reinterpret_cast<const select_properties*>(data_ptr);
        auto  mask_byte_length = properties_ptr->mask_byte_length % 4096;
        if (size > sizeof(select_properties) + mask_byte_length) {
            std::vector<uint8_t> mask(data_ptr + sizeof(select_properties),
                                      data_ptr + sizeof(select_properties) + mask_byte_length);

            std::vector<uint8_t> source(data_ptr + sizeof(select_properties) + mask_byte_length, data_ptr + size);
            std::vector<uint8_t> destination(properties_ptr->destination_size);

            qpl_status status;
            uint32_t   job_size = 0;

            // Job initialization
            status = qpl_get_job_size(qpl_path_software, &job_size);
            if (status != QPL_STS_OK) { return 0; }

            auto     job_buffer = std::make_unique<uint8_t[]>(job_size);
            qpl_job* job_ptr    = reinterpret_cast<qpl_job*>(job_buffer.get());

            status = qpl_init_job(qpl_path_software, job_ptr);
            if (status != QPL_STS_OK) { return 0; }

            job_ptr->next_in_ptr        = source.data();
            job_ptr->available_in       = source.size();
            job_ptr->next_src2_ptr      = mask.data();
            job_ptr->available_src2     = mask.size();
            job_ptr->next_out_ptr       = destination.data();
            job_ptr->available_out      = destination.size();
            job_ptr->op                 = qpl_op_select;
            job_ptr->num_input_elements = properties_ptr->number_of_elements;
            job_ptr->src1_bit_width     = properties_ptr->input_bit_width;
            job_ptr->src2_bit_width     = properties_ptr->input_bit_width_2;
            job_ptr->out_bit_width      = properties_ptr->output_bit_width;
            job_ptr->parser             = parser;

            status = qpl_execute_job(job_ptr);
        }
    }

    return 0;
}

static inline int expand_test_case(const uint8_t* data_ptr, size_t size, qpl_parser parser) {
    if (size > sizeof(expand_properties)) {
        auto* properties_ptr   = reinterpret_cast<const expand_properties*>(data_ptr);
        auto  mask_byte_length = properties_ptr->mask_byte_length % 4096;
        if (size > sizeof(expand_properties) + mask_byte_length) {
            std::vector<uint8_t> mask(data_ptr + sizeof(expand_properties),
                                      data_ptr + sizeof(expand_properties) + mask_byte_length);

            std::vector<uint8_t> source(data_ptr + sizeof(expand_properties) + mask_byte_length, data_ptr + size);
            std::vector<uint8_t> destination(properties_ptr->destination_size);

            qpl_status status;
            uint32_t   job_size = 0;

            // Job initialization
            status = qpl_get_job_size(qpl_path_software, &job_size);
            if (status != QPL_STS_OK) { return 0; }

            auto     job_buffer = std::make_unique<uint8_t[]>(job_size);
            qpl_job* job_ptr    = reinterpret_cast<qpl_job*>(job_buffer.get());

            status = qpl_init_job(qpl_path_software, job_ptr);
            if (status != QPL_STS_OK) { return 0; }

            job_ptr->next_in_ptr        = source.data();
            job_ptr->available_in       = source.size();
            job_ptr->next_src2_ptr      = mask.data();
            job_ptr->available_src2     = mask.size();
            job_ptr->next_out_ptr       = destination.data();
            job_ptr->available_out      = destination.size();
            job_ptr->op                 = qpl_op_expand;
            job_ptr->num_input_elements = properties_ptr->number_of_elements;
            job_ptr->src1_bit_width     = properties_ptr->input_bit_width;
            job_ptr->src2_bit_width     = properties_ptr->input_bit_width_2;
            job_ptr->out_bit_width      = properties_ptr->output_bit_width;
            job_ptr->parser             = parser;

            status = qpl_execute_job(job_ptr);
        }
    }

    return 0;
}
