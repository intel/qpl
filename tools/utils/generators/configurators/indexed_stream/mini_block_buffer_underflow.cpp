/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "mini_block_buffer_underflow.hpp"

void gz_generator::InflateMiniBlockBufferUnderflowConfigurator::generateMiniBlockSequence(Gen32u miniBlockCount)
{
    // Variables
    Gen32u match               = 0U;
    Gen32u offset              = 0U;
    Gen32u startSequenceLength = 0U;
    qpl::test::random random(0U, 0U, m_seed);

    // Generate correct mini blocks
    ConfiguratorDecompressIndex::generateMiniBlock(m_miniBlockSize);
    ConfiguratorDecompressIndex::getIndex();

    // Generate mini blocks, which are spanned via offset
    for (Gen32u mini_block = 1U; mini_block < miniBlockCount; mini_block++)
    {
        match = static_cast<Gen32u>(m_randomMatch);
        m_randomOffset.set_range(match, GEN_MIN(m_miniBlockSize, MAX_OFFSET));
        offset = static_cast<Gen32u>(m_randomOffset);

        // Set starting sequence length
        random.set_range(MIN_MATCH, offset - 1U);
        startSequenceLength = static_cast<Gen32u>(random);

        // Generate mini block
        ConfiguratorDecompressIndex::generateMiniBlock(startSequenceLength);
        TestConfigurator::declareReference(match, offset);
        ConfiguratorDecompressIndex::generateMiniBlock(m_miniBlockSize - startSequenceLength - match);

        ConfiguratorDecompressIndex::getIndex();
    }
}