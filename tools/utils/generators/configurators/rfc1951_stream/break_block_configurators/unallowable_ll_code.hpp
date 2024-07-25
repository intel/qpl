/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef _CONFIGURATOR_UNALLOWABLE_LL_CODE_H_
#define _CONFIGURATOR_UNALLOWABLE_LL_CODE_H_

#include "base_configurator.hpp"

namespace gz_generator {
class UnallowableLiteralLengthCodeConfigurator : public TestConfigurator {
public:
    UnallowableLiteralLengthCodeConfigurator(Gen32u seed) : TestConfigurator(seed) {}

    UnallowableLiteralLengthCodeConfigurator() = delete;

    GenStatus generate() override;
};
} // namespace gz_generator

#endif //_CONFIGURATOR_UNALLOWABLE_LL_CODE_H_
