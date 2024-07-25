/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <algorithm>
#include <array>

#include "ta_ll_common.hpp"
#include "util.hpp"

namespace qpl::test {

// get_existing_dictionary_size() should match the size given by get_dictionary_size for any given dict
// This creates a variety of dictionaries from the dataset and checks that the sizes match
QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, get_existing_dictionary_size) {

    const hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {

        std::vector<uint8_t> source    = dataset.second;
        const uint32_t       dict_size = static_cast<uint32_t>(source.size());

        for (const hw_compression_level hw_compr_level : hw_levels) {
            for (const sw_compression_level sw_compr_level : sw_levels) {

                const uint32_t dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dict_size);

                auto            dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                qpl_dictionary* dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                auto status =
                        qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(), dict_size);
                ASSERT_EQ(QPL_STS_OK, status);

                size_t existing_size = 0;
                qpl_get_existing_dict_size(dictionary_ptr, &existing_size);
                ASSERT_EQ(existing_size, dictionary_buffer_size)
                        << "qpl_get_existing_dict_size returned incorrect size";
            }
        }
    }
}

// A simple test to check that the set_dictionary_id and get_dictionary_id functions execute properly
// Makes sure that the set function sets the same value that the get function returns
QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary, dictionary_id) {

    const sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    const hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    // Raw dict size parameter is arbitrary, does not affect test's functionality
    const size_t dictionary_buffer_size = qpl_get_dictionary_size(sw_compr_level, hw_compr_level, 4096);

    auto            dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
    qpl_dictionary* dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

    const uint32_t dictionary_id = 1; // Arbitrary ID for testing

    qpl_status status = qpl_set_dictionary_id(dictionary_ptr, dictionary_id);
    ASSERT_EQ(QPL_STS_OK, status) << "Setting dict id failed";

    uint32_t dest = 0;
    status        = qpl_get_dictionary_id(dictionary_ptr, &dest);
    ASSERT_EQ(QPL_STS_OK, status) << "Getting dict id failed";

    ASSERT_EQ(dictionary_id, dest) << "get_dictionary_id does not match the original id";
}
} // namespace qpl::test
