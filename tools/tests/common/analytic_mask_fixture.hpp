/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TEST_ANALYTICS_MASK_FIXTURE_HPP
#define QPL_TEST_ANALYTICS_MASK_FIXTURE_HPP

#include "analytic_fixture.hpp"

namespace qpl::test {
class AnalyticMaskFixture : public AnalyticFixture {
protected:
    void GenerateBuffers() {
        source_provider source_gen(current_test_case.number_of_elements, current_test_case.source_bit_width, GetSeed(),
                                   current_test_case.parser);

        ASSERT_NO_THROW(source = source_gen.get_source()); //NOLINT(cppcoreguidelines-avoid-goto)

        source_provider mask_gen(current_test_case.second_input_num_elements, current_test_case.second_input_bit_width,
                                 GetSeed());

        ASSERT_NO_THROW(mask = mask_gen.get_source()); //NOLINT(cppcoreguidelines-avoid-goto)

        uint32_t destination_bit_width = (1U == current_test_case.destination_bit_width)
                                                 ? current_test_case.source_bit_width
                                                 : current_test_case.destination_bit_width;

        uint32_t destination_size = current_test_case.number_of_elements *
                                    ((destination_bit_width + max_bit_index) >> bit_to_byte_shift_offset);

        destination.resize(destination_size);
        reference_destination.resize(destination_size);
    }

    void SetBuffers() override {
        GenerateBuffers();

        job_ptr->available_in             = static_cast<uint32_t>(source.size());
        job_ptr->next_in_ptr              = source.data();
        job_ptr->available_src2           = static_cast<uint32_t>(mask.size());
        job_ptr->next_src2_ptr            = mask.data();
        reference_job_ptr->available_in   = static_cast<uint32_t>(source.size());
        reference_job_ptr->next_in_ptr    = source.data();
        reference_job_ptr->available_src2 = static_cast<uint32_t>(mask.size());
        reference_job_ptr->next_src2_ptr  = mask.data();

        job_ptr->available_out           = static_cast<uint32_t>(destination.size());
        job_ptr->next_out_ptr            = destination.data();
        reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());
        reference_job_ptr->next_out_ptr  = reference_destination.data();
    }

    std::vector<uint8_t> mask;
};
} // namespace qpl::test

#endif //QPL_TEST_ANALYTICS_MASK_FIXTURE_HPP
