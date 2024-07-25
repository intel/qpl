/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_PROJECT_CONFIGURATOR_OVERSUBSCRIBED_CL_TREE_HPP
#define QPL_PROJECT_CONFIGURATOR_OVERSUBSCRIBED_CL_TREE_HPP

#include "base_configurator.hpp"

namespace gz_generator {
class OversubscribedCodeLengthsTreeConfigurator : public TestConfigurator {
public:
    OversubscribedCodeLengthsTreeConfigurator(Gen32u seed) : TestConfigurator(seed) {}

    OversubscribedCodeLengthsTreeConfigurator() = delete;

    GenStatus generate() override;
};
} // namespace gz_generator

#endif //QPL_PROJECT_CONFIGURATOR_OVERSUBSCRIBED_CL_TREE_HPP
