/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef _CONFIGURATOR_INFLATE_INDEX_MATCH_SPAN_HPP_
#define _CONFIGURATOR_INFLATE_INDEX_MATCH_SPAN_HPP_

#include "index_base.hpp"

namespace  gz_generator
{
    class InflateMiniBlockBufferOverflowConfigurator: public ConfiguratorDecompressIndex
    {
        public:
            InflateMiniBlockBufferOverflowConfigurator(Gen32u seed, SpecialTestOptions &testOptions)
                    : ConfiguratorDecompressIndex(seed, testOptions) {}

            InflateMiniBlockBufferOverflowConfigurator() = delete;

            void generateMiniBlockSequence(Gen32u miniBlockCount) override;
    };
}
#endif //_CONFIGURATOR_INFLATE_INDEX_MATCH_SPAN_HPP_
