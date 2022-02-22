/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "compression/deflate/histogram.hpp"
#include "util/descriptor_processing.hpp"
#include "util/memory.hpp"

namespace qpl::ml::compression {

namespace details {
void histogram_join_another(qpl_histogram *first_histogram_ptr,
                            qpl_histogram *second_histogram_ptr) {
    const uint32_t histogram_notes = sizeof(qpl_histogram) / sizeof(uint32_t);

    auto *first_ptr  = reinterpret_cast<uint32_t *>(first_histogram_ptr);
    auto *second_ptr = reinterpret_cast<uint32_t *>(second_histogram_ptr);

    for (uint32_t i = 0; i < histogram_notes; i++) {
        first_ptr[i] += second_ptr[i];
    }
}
}

template<>
auto update_histogram<execution_path_t::hardware>(uint8_t *begin,
                                                  uint8_t *end,
                                                  deflate_histogram &histogram,
                                                  deflate_level level) noexcept -> qpl_ml_status {
    hw_descriptor HW_PATH_ALIGN_STRUCTURE descriptor;
    HW_PATH_VOLATILE hw_completion_record HW_PATH_ALIGN_STRUCTURE completion_record;
    qpl_histogram hw_histogram;

    util::set_zeros(descriptor.data, HW_PATH_DESCRIPTOR_SIZE);

    hw_iaa_descriptor_init_statistic_collector(&descriptor,
                                               begin,
                                               static_cast<uint32_t>(std::distance(begin, end)),
                                               reinterpret_cast<hw_iaa_histogram *const>(&hw_histogram));

    hw_iaa_descriptor_set_completion_record(&descriptor, &completion_record);
    completion_record.status = 0u;

    auto status = util::process_descriptor<qpl_ml_status, util::execution_mode_t::sync>(&descriptor, &completion_record);

    if (status_list::ok == status) {
        details::histogram_join_another(&histogram, &hw_histogram);
    }

    return status;
}

}
