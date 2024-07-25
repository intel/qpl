/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef _CONFIGURATOR_INVALID_BLOCK_H_
#define _CONFIGURATOR_INVALID_BLOCK_H_

#include "base_configurator.hpp"

namespace gz_generator {
class InvalidBlockTypeConfigurator : public TestConfigurator {
public:
    InvalidBlockTypeConfigurator(Gen32u seed) : TestConfigurator(seed) {}

    InvalidBlockTypeConfigurator() = delete;

    GenStatus generate() override;
};
} // namespace gz_generator

#endif //_CONFIGURATOR_INVALID_BLOCK_H_
