/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_ML_UTIL_IAA_FEATURES_CHECKS
#define QPL_ML_UTIL_IAA_FEATURES_CHECKS

#include "dispatcher/hw_dispatcher.hpp"

namespace qpl::ml::util {

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
    static auto& dispatcher = qpl::ml::dispatcher::hw_dispatcher::get_instance();
    if (dispatcher.is_hw_support()) {
        const auto& device            = dispatcher.device(0);
        are_gen2_capabilities_present = device.get_gen_2_min_capabilities();
    }
#endif

    return are_gen2_capabilities_present;
}

} // namespace qpl::ml::util

#endif //QPL_ML_UTIL_IAA_FEATURES_CHECKS
