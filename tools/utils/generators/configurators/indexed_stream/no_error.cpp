/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "index_no_error.hpp"

void gz_generator::ConfiguratorDecompressIndexNoErr::generateMiniBlockSequence(Gen32u miniBlockCount)
{
    for (Gen32u mini_block = 0; mini_block < miniBlockCount; mini_block++)
    {
        ConfiguratorDecompressIndex::generateMiniBlock(m_miniBlockSize);
        ConfiguratorDecompressIndex::getIndex();
    }
}

