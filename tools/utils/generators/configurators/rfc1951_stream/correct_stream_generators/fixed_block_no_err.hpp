/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef _CONFIGURATOR_FIXED_BLOCK_NO_ERR_HPP_
#define _CONFIGURATOR_FIXED_BLOCK_NO_ERR_HPP_

#include "base_configurator.hpp"

namespace gz_generator {
class FixedBlockNoErrorConfigurator : public TestConfigurator {
public:
    FixedBlockNoErrorConfigurator(Gen32u seed) : TestConfigurator(seed) {}

    FixedBlockNoErrorConfigurator() = delete;

    GenStatus generate() override;
};
} // namespace gz_generator

#endif //_FIXED_BLOCK_NO_ERR_HPP_
