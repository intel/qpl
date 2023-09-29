/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "incorrect_block_size.hpp"

GenStatus gz_generator::InflateIndexIncorrectBlockSizeConfigurator::generate()
{
    Gen32u miniBlockCount = m_pIndexTable->getMiniBlockCount();
    Gen32u miniBlocksPerBlock = m_pIndexTable->getMiniBlocksPerBlock();
    Gen32u mini_block_size = m_miniBlockSize;

    Gen32s delta = 0;
    qpl::test::random randomDelta(-8, 8, m_seed);

    while ((delta = static_cast<Gen32s>(randomDelta)) == 0u);

    // Generate first block
    TestConfigurator::declareDynamicBlock();

    for (Gen32u mini_block = 0; miniBlockCount < miniBlocksPerBlock - 1u; mini_block++)
    {
        ConfiguratorDecompressIndex::generateMiniBlock(mini_block_size);
    }

    if (delta < 0)
    {
        ConfiguratorDecompressIndex::generateMiniBlock(mini_block_size + delta);
    }
    else
    {
        ConfiguratorDecompressIndex::generateMiniBlock(mini_block_size);
        ConfiguratorDecompressIndex::generateMiniBlock(delta);
    }

    ConfiguratorDecompressIndex::getIndex();

    // Generate second block
    TestConfigurator::declareRandomBlock();

    TestConfigurator::writeRandomLiteralSequence(100u);

    ConfiguratorDecompressIndex::getIndex();

    // End Stream
    ConfiguratorDecompressIndex::declareFinishBlock();

    // We Must break table table specification
    m_pIndexTable->reset(2, 1);

    return GEN_OK;

}

