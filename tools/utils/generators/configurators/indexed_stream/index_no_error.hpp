/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef __CONFIGURATOR_DECOMPRESS_INDEX_NO_ERROR_HPP__
#define __CONFIGURATOR_DECOMPRESS_INDEX_NO_ERROR_HPP__

#include "index_base.hpp"

namespace gz_generator
{
    class ConfiguratorDecompressIndexNoErr: public ConfiguratorDecompressIndex
    {
        public:
            ConfiguratorDecompressIndexNoErr(Gen32u seed, SpecialTestOptions &testOptions)
                : ConfiguratorDecompressIndex(seed, testOptions) {}

            ConfiguratorDecompressIndexNoErr() = delete;

            void generateMiniBlockSequence(Gen32u miniBlockCount) override;
    };
}

#endif //__CONFIGURATOR_DECOMPRESS_INDEX_NO_ERROR_HPP__
