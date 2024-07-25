/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_PROJECT_CONFIGURATOR_FIRST_LL_16_CODE_HPP
#define QPL_PROJECT_CONFIGURATOR_FIRST_LL_16_CODE_HPP

#include "base_configurator.hpp"

namespace gz_generator {
class FirstLiteralLengthCode16Configurator : public TestConfigurator {
public:
    FirstLiteralLengthCode16Configurator(Gen32u seed) : TestConfigurator(seed) {}

    FirstLiteralLengthCode16Configurator() = delete;

    GenStatus generate() override;
};
} // namespace gz_generator

#endif //QPL_PROJECT_CONFIGURATOR_FIRST_LL_16_CODE_HPP
