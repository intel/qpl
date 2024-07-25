/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef _CONFIGURATOR_MANY_DISTANCE_CODES_V2_HPP_
#define _CONFIGURATOR_MANY_DISTANCE_CODES_V2_HPP_

#include "base_configurator.hpp"

namespace gz_generator {
class ManyDistanceCodesConfiguratorVersion2 : public TestConfigurator {
public:
    ManyDistanceCodesConfiguratorVersion2(Gen32u seed) : TestConfigurator(seed) {}

    ManyDistanceCodesConfiguratorVersion2() = delete;

    GenStatus generate() override;
};
} // namespace gz_generator

#endif //_MANY_DISTANCE_CODES_HPP_
