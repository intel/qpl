/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_PROJECT_CONFIGURATOR_CANNED_LARGE_LL_TABLE_HPP
#define QPL_PROJECT_CONFIGURATOR_CANNED_LARGE_LL_TABLE_HPP

#include "base_configurator.hpp"

namespace gz_generator {
class CannedLargeLiteralLengthTableConfigurator : public TestConfigurator {
public:
    CannedLargeLiteralLengthTableConfigurator(Gen32u seed) : TestConfigurator(seed) {} // need to redesign base class

    CannedLargeLiteralLengthTableConfigurator() = delete;

    GenStatus generate() override;
};
} // namespace gz_generator

#endif //QPL_PROJECT_CONFIGURATOR_CANNED_LARGE_LL_TABLE_HPP
