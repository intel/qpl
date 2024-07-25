/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_PROJECT_CONFIGURATOR_DYNAMIC_SMALL_HPP
#define QPL_PROJECT_CONFIGURATOR_DYNAMIC_SMALL_HPP

#include "base_configurator.hpp"

namespace gz_generator {
class CannedSmallBlocksConfigurator : public TestConfigurator {
public:
    CannedSmallBlocksConfigurator(Gen32u seed) : TestConfigurator(seed) {}

    CannedSmallBlocksConfigurator() = delete;

    GenStatus generate() override;
};
} // namespace gz_generator

#endif //QPL_PROJECT_CONFIGURATOR_DYNAMIC_SMALL_HPP
