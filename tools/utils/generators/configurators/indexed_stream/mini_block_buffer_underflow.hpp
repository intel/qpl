/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef _CONFIGURATOR_DECOMPRESS_INDEX_BAD_DISTANCE_HPP_
#define _CONFIGURATOR_DECOMPRESS_INDEX_BAD_DISTANCE_HPP_

#include "index_base.hpp"

namespace gz_generator
{
    class InflateMiniBlockBufferUnderflowConfigurator: public ConfiguratorDecompressIndex
    {
        public:
            InflateMiniBlockBufferUnderflowConfigurator(Gen32u seed, SpecialTestOptions testOptions)
                    : ConfiguratorDecompressIndex(seed, testOptions) {}

            InflateMiniBlockBufferUnderflowConfigurator() = delete;

            void generateMiniBlockSequence(Gen32u miniBlockCount) override;
    };
}

#endif //_CONFIGURATOR_DECOMPRESS_INDEX_BAD_DISTANCE_HPP_
