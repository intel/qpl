/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef _CONFIGURATOR_UNALLOWABLE_D_CODE_H_
#define _CONFIGURATOR_UNALLOWABLE_D_CODE_H_

#include "base_configurator.hpp"

namespace gz_generator {
class UnallowableDistanceCodeConfigurator : public TestConfigurator {
public:
    UnallowableDistanceCodeConfigurator(Gen32u seed) : TestConfigurator(seed) {}

    UnallowableDistanceCodeConfigurator() = delete;

    GenStatus generate() override;
};
} // namespace gz_generator

#endif //_CONFIGURATOR_UNALLOWABLE_D_CODE_H_
