/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef _CONFIGURATOR_BAD_DISTANCE_H_
#define _CONFIGURATOR_BAD_DISTANCE_H_

#include "base_configurator.hpp"

namespace gz_generator {
class BadDistanceConfigurator : public TestConfigurator {
public:
    BadDistanceConfigurator(Gen32u seed) : TestConfigurator(seed) {}

    BadDistanceConfigurator() = delete;

    GenStatus generate() override;
};
} // namespace gz_generator
#endif //_CONFIGURATOR_BAD_DISTANCE_H_
