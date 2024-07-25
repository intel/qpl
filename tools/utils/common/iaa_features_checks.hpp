/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TOOLS_UTILS_COMMON_IAA_FEATURES_CHECKS_HPP_
#define QPL_TOOLS_UTILS_COMMON_IAA_FEATURES_CHECKS_HPP_

#include "test_hw_dispatcher.hpp"

namespace qpl::test {

/**
 * @brief Function to check IAACAP bit and return a boolean to
 * indicate whether Generation 2 Minimum Capabilities are present,
 * in particular:
 * 1. The decompress internal state in the AECS is Format 2
 * 2. Completion Record checksum fields are written correctly for CRC64
 * 3. CRC Select can be used with the CRC64 opcode
 * 4. If the operation is other than No-op or Drain, at least one of
 *    Source 1 Transfer Size, Source 2 Transfer Size, and Maximum Destination
 *    Size must be non-zero
 * 5. Ignore End Bits Extension is supported
 * 6. Last Descriptor Bit in the compression AECS is present
 *
 * @note It is safe to check IAACAP value on a single device only,
 * since we do not expect to have devices with different generations
 * available on the same host.
*/
inline bool are_iaa_gen_2_min_capabilities_present() {
    bool are_gen2_capabilities_present = false;

#if defined(__linux__)
    static auto& dispatcher = hw_dispatcher::get_instance();
    if (dispatcher.is_hw_support()) {
        const auto& device            = dispatcher.device(0);
        are_gen2_capabilities_present = device.get_gen_2_min_capabilities();
    }
#endif

    return are_gen2_capabilities_present;
}

/**
 * @brief Function to check IAACAP bit and return a boolean to
 * indicate whether dictionary compression is supported.
 *
 * @note It is safe to check IAACAP value on a single device only,
 * since we do not expect to have devices with different generations
 * available on the same host.
*/
inline bool is_iaa_dictionary_compress_supported() {
    bool is_dict_compress_supported = false;

#if defined(__linux__)
    static auto& dispatcher = hw_dispatcher::get_instance();
    if (dispatcher.is_hw_support()) {
        const auto& device         = dispatcher.device(0);
        is_dict_compress_supported = device.get_dict_compress_support();
    }
#endif

    return is_dict_compress_supported;
}

/**
 * @brief Function to check IAACAP bit and return a boolean to
 * indicate whether force array output modification is supported.
 * 
 * @note It is safe to check IAACAP value on a single device only,
 * since we do not expect to have devices with different generations
 * available on the same host.
*/
inline bool is_iaa_force_array_output_mod_supported() {
    bool is_force_array_output_mod_supported = false;

#if defined(__linux__)
    static auto& dispatcher = hw_dispatcher::get_instance();
    if (dispatcher.is_hw_support()) {
        const auto& device                  = dispatcher.device(0);
        is_force_array_output_mod_supported = device.get_force_array_output_mod_support();
    }
#endif

    return is_force_array_output_mod_supported;
}

} // namespace qpl::test

#endif //QPL_TOOLS_UTILS_COMMON_IAA_FEATURES_CHECKS_HPP_
