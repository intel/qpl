/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Middle Layer API (private C++ API)
 */

#ifndef QPL_TESTS_TESTS_COMMON_CHECK_RESULT_HPP_
#define QPL_TESTS_TESTS_COMMON_CHECK_RESULT_HPP_

#include <inttypes.h>
#include <vector>

#include "qpl/c_api/job.h"

#include "gtest/gtest.h"

/*
* @brief Function to compare two vectors
*
* @param[in] first                         - First vector to compare
* @param[in] second                        - Second vector to compare
* @param[in] number_of_elements_to_compare - Only this number of elements will be compare, default value 0 means compare all vector elements
* @param[in] fail_message                  - Message that should provide user with some information about what
* @return
* - AssertionSuccess - success
* - AssertionFailure - failure
*/
template <typename DataType>
testing::AssertionResult CompareVectors(const std::vector<DataType>& first, const std::vector<DataType>& second,
                                        const uint32_t     number_of_elements_to_compare = 0U,
                                        const std::string& fail_message                  = "") {
    if (0U == number_of_elements_to_compare) {
        if (first.size() != second.size()) {
            return testing::AssertionFailure() << fail_message << ", "
                                               << "vectors have different length";
        }
    } else {
        if (first.size() < number_of_elements_to_compare) {
            return testing::AssertionFailure() << fail_message << ", "
                                               << "first vector has less elements, that required";
        }

        if (second.size() < number_of_elements_to_compare) {
            return testing::AssertionFailure() << fail_message << ", "
                                               << "second vector has less elements, that required";
        }
    }

    const uint32_t index_bound =
            (number_of_elements_to_compare == 0U) ? static_cast<uint32_t>(first.size()) : number_of_elements_to_compare;

    for (uint32_t i = 0U; i < index_bound; i++) {
        if (first[i] != second[i]) {
            return testing::AssertionFailure() << fail_message << ", "
                                               << "vectors differ at " << i << " index (" << (uint32_t)first[i] << " , "
                                               << (uint32_t)second[i] << ")";
        }
    }

    return testing::AssertionSuccess();
}

template <typename input_iterator, typename output_iterator>
testing::AssertionResult CompareSegments(input_iterator first_begin, input_iterator first_end,
                                         output_iterator second_begin, output_iterator second_end,
                                         const std::string& fail_message = "") {
    if (std::distance(first_begin, first_end) != std::distance(second_begin, second_end)) {
        return testing::AssertionFailure() << fail_message << ", "
                                           << "Segments to compare has a different size";
    }

    const uint32_t size = std::distance(first_begin, first_end);

    auto first  = first_begin;
    auto second = second_begin;

    for (uint32_t i = 0U; i < size; i++, first++, second++) {
        if (*first != *second) {
            return testing::AssertionFailure()
                   << fail_message << ", "
                   << "Segments differ at " << i << " index (" << static_cast<uint32_t>(*first) << " , "
                   << static_cast<uint32_t>(*second) << ")";
        }
    }

    return testing::AssertionSuccess();
}

static inline testing::AssertionResult compare_checksum_fields(const qpl_job* job_ptr,
                                                               const qpl_job* reference_job_ptr) {
    if (job_ptr->crc != reference_job_ptr->crc) {
        return testing::AssertionFailure() << "CRC fields did not match, reference crc = " << reference_job_ptr->crc
                                           << " library crc = " << job_ptr->crc;
    }

    if (job_ptr->xor_checksum != reference_job_ptr->xor_checksum) {
        return testing::AssertionFailure()
               << "XOR checksum fields did not match, reference xor = " << reference_job_ptr->xor_checksum
               << " library xor = " << job_ptr->xor_checksum;
    }

    return testing::AssertionSuccess();
}

static inline testing::AssertionResult compare_crc32_field(const qpl_job* job_ptr, const qpl_job* reference_job_ptr) {
    if (job_ptr->crc != reference_job_ptr->crc) {
        return testing::AssertionFailure() << "CRC fields did not match, reference crc = " << reference_job_ptr->crc
                                           << " library crc = " << job_ptr->crc;
    }

    return testing::AssertionSuccess();
}

#endif //QPL_TESTS_TESTS_COMMON_CHECK_RESULT_HPP_
