/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "mini_block_buffer_overflow.hpp"

void gz_generator::InflateMiniBlockBufferOverflowConfigurator::generateMiniBlockSequence(Gen32u miniBlockCount)
{
    // Variables
    Gen32u offset              = 0U;
    Gen32u match               = 0U;
    Gen32u startSequenceLength = 0U;
    Gen32u minimalLastMatch    = 0U;

    qpl::test::random randomMatch(0U, 0U, m_seed);

    // Generate mini-blocks, which are spanned via match
    for (Gen32u mini_block = 0; mini_block < miniBlockCount - 1U; mini_block++)
    {
        // Define min length of last match
        randomMatch.set_range(MIN_MATCH, MAX_MATCH - 1U);
        minimalLastMatch    = static_cast<Gen32u>(randomMatch);

        // Define sequence length before last match
        startSequenceLength = m_miniBlockSize - (minimalLastMatch - 1U);

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
