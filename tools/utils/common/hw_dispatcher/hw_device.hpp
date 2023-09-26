/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TOOLS_UTILS_COMMON_HW_DEVICE_HPP_
#define QPL_TOOLS_UTILS_COMMON_HW_DEVICE_HPP_

#include "hw_dispatcher/hw_status.h"
#include <stddef.h>

#if defined( __linux__ )

#include "hw_dispatcher/hw_configuration_driver.h"

namespace qpl::test {
/**< Name used to search for devices */
#define QPL_TEST_IAA_DEVICE ((uint32_t)(((uint32_t)0xFF << 24u) | \
                            ((uint32_t)('x') << 16u) | ((uint32_t)('a') << 8u) | (uint32_t)('i')))

/**< Max number of devices, used to initialize the devices array in hw_dispatcher */
#define QPL_TEST_MAX_NUM_DEV 100u

/**< Character mask to convert the first 3 bytes to lower case and make the 4th 0xff */
#define QPL_TEST_CHAR_MSK 0xFF202020

/**< IAACAP bit 0 - generation 2 Minimum Capabilities support */
#define QPL_TEST_IC_GEN_2_MIN_CAP(IAACAP) (((IAACAP))&0x01)

class hw_device final {

public:
    using descriptor_t = void;

    hw_device() noexcept = default;

    [[nodiscard]] auto initialize_new_device(descriptor_t *device_descriptor_ptr) noexcept -> qpl_test_hw_accelerator_status;

    [[nodiscard]] auto get_gen_2_min_capabilities() const noexcept -> bool;

private:
    uint64_t           iaa_cap_register_ = 0u;    /**< IAACAP register content */
    uint32_t           version_major_    = 0u;    /**< Major version of discovered device */
    uint32_t           version_minor_    = 0u;    /**< Minor version of discovered device */
};

}

#endif //__linux__
#endif //QPL_TOOLS_UTILS_COMMON_HW_DEVICE_HPP_
