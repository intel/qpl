/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_PROJECT_CONFIGURATOR_OVERSUBSCRIBED_LL_TREE_HPP
#define QPL_PROJECT_CONFIGURATOR_OVERSUBSCRIBED_LL_TREE_HPP

#include "base_configurator.hpp"

namespace gz_generator
{
    class OversubscribedLiteralLengthsTreeConfigurator: public TestConfigurator
    {
        public:
            OversubscribedLiteralLengthsTreeConfigurator(Gen32u seed)
                : TestConfigurator(seed){}

            OversubscribedLiteralLengthsTreeConfigurator() = delete;

            GenStatus generate() override;
    };
}

#endif //QPL_PROJECT_CONFIGURATOR_OVERSUBSCRIBED_LL_TREE_HPP
