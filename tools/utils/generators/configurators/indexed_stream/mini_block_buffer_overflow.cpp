/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "mini_block_buffer_overflow.hpp"

void gz_generator::InflateMiniBlockBufferOverflowConfigurator::generateMiniBlockSequence(Gen32u miniBlockCount)
{
    // Variables
    Gen32u offset              = 0u;
    Gen32u match               = 0u;
    Gen32u startSequenceLength = 0u;
    Gen32u minimalLastMatch    = 0u;

    qpl::test::random randomMatch(0u, 0u, m_seed);

    // Generate mini-blocks, which are spanned via match
    for (Gen32u mini_block = 0; mini_block < miniBlockCount - 1u; mini_block++)
    {
        // Define min length of last match
        randomMatch.set_range(MIN_MATCH, MAX_MATCH - 1u);
        minimalLastMatch    = static_cast<Gen32u>(randomMatch);

        // Define sequence length before last match
        startSequenceLength = m_miniBlockSize - (minimalLastMatch - 1u);

        // Define last match
        randomMatch.set_range(minimalLastMatch, MAX_MATCH);
        match = static_cast<Gen32u>(randomMatch);

        // Define max offset to look over mini-block via match
        m_randomOffset.set_range(MIN_MATCH, match);
        offset = static_cast<Gen32u>(m_randomOffset);

        // Generate mini-block sequence
        ConfiguratorDecompressIndex::generateMiniBlock(startSequenceLength);
        TestConfigurator::declareReference(match, offset);

        ConfiguratorDecompressIndex::getIndex();
    }

    // Generate correct mini-block
    ConfiguratorDecompressIndex::generateMiniBlock(m_miniBlockSize);
    ConfiguratorDecompressIndex::getIndex();
}
