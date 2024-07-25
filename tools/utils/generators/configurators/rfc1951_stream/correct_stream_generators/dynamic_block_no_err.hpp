/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef _CONFIGURATOR_DYNAMIC_BLOCK_NO_ERR_HPP_
#define _CONFIGURATOR_DYNAMIC_BLOCK_NO_ERR_HPP_

#include "base_configurator.hpp"

namespace gz_generator {
class DynamicBlockNoErrorConfigurator : public TestConfigurator {
public:
    DynamicBlockNoErrorConfigurator(Gen32u seed) : TestConfigurator(seed) {}

    DynamicBlockNoErrorConfigurator() = delete;

    GenStatus generate() override;
};
} // namespace gz_generator

#endif //_DYNAMIC_BLOCK_NO_ERR_HPP_
