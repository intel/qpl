/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_ML_UTIL_AECS_FORMAT_CHECKER
#define QPL_ML_UTIL_AECS_FORMAT_CHECKER

#include "dispatcher/hw_dispatcher.hpp"
#include "compression/inflate/inflate_defs.hpp"

namespace qpl::ml::util {

/**
 * @brief Function to check IAACAP bit and return AECS Format,
 * that should be used for submissions for Decompress operation.
 *
 * @note It is safe to check IAACAP value on a single device only,
 * since we do not expect to have devices with different generations
 * available on the same host.
*/
inline qpl::ml::compression::aecs_format get_device_aecs_format() {
#if defined( __linux__ )
    static auto &dispatcher       = qpl::ml::dispatcher::hw_dispatcher::get_instance();
    const auto &device            = dispatcher.device(0);
    return (device.get_gen_2_min_capabilities() == 0) ? qpl::ml::compression::mapping_table
                                                      : qpl::ml::compression::mapping_cam;
#else
    return qpl::ml::compression::mapping_table;
#endif
}

}

#endif //QPL_ML_UTIL_AECS_FORMAT_CHECKER
