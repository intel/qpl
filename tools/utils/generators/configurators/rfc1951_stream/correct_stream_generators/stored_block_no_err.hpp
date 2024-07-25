/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef _CONFIGURATOR_STATIC_BLOCK_NO_ERR_HPP_
#define _CONFIGURATOR_STATIC_BLOCK_NO_ERR_HPP_

#include "base_configurator.hpp"

namespace gz_generator {
class StoredBlockNoErrorConfigurator : public TestConfigurator {
public:
    StoredBlockNoErrorConfigurator(Gen32u seed) : TestConfigurator(seed) {}

    StoredBlockNoErrorConfigurator() = delete;

    GenStatus generate() override;
};
} // namespace gz_generator

#endif //QPL_PROJECT_CONFIGURATOR_STATIC_BLOCK_NO_ERR_H
