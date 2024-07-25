/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef _CONFIGURATOR_DISTANCE_BEFORE_START_H_
#define _CONFIGURATOR_DISTANCE_BEFORE_START_H_

#include "base_configurator.hpp"

namespace gz_generator {
class DistanceCodeBeforeStartConfigurator : public TestConfigurator {
public:
    DistanceCodeBeforeStartConfigurator(Gen32u seed) : TestConfigurator(seed) {}

    DistanceCodeBeforeStartConfigurator() = delete;

    GenStatus generate() override;
};
} // namespace gz_generator

#endif //_CONFIGURATOR_DISTANCE_BEFORE_START_H_
