/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef _CONFIGURATOR_INFLATE_INDEX_ERROR_228_HPP_
#define _CONFIGURATOR_INFLATE_INDEX_ERROR_228_HPP_

#include "index_base.hpp"

namespace gz_generator
{
    class InflateIndexIncorrectBlockSizeConfigurator : public ConfiguratorDecompressIndex
    {
        public:
            InflateIndexIncorrectBlockSizeConfigurator(Gen32u seed, SpecialTestOptions &testOptions)
                    : ConfiguratorDecompressIndex(seed, testOptions) {}

            InflateIndexIncorrectBlockSizeConfigurator() = delete;

            GenStatus generate() override;

            void generateMiniBlockSequence(Gen32u miniBlockCount) override{}
    };
}

#endif //_CONFIGURATOR_INFLATE_INDEX_ERROR_228_HPP_
