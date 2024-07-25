/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_PROJECT_CONFIGURATOR_OVERSUBSCRIBED_D_TREE_HPP
#define QPL_PROJECT_CONFIGURATOR_OVERSUBSCRIBED_D_TREE_HPP

#include "base_configurator.hpp"

namespace gz_generator {
class OversubscribedDistanceTreeConfigurator : public TestConfigurator {
public:
    OversubscribedDistanceTreeConfigurator(Gen32u seed) : TestConfigurator(seed) {}

    OversubscribedDistanceTreeConfigurator() = delete;

    GenStatus generate() override;
};
} // namespace gz_generator

#endif //QPL_PROJECT_CONFIGURATOR_OVERSUBSCRIBED_D_TREE_HPP
