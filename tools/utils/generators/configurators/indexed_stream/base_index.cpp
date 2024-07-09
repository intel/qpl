/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "index_base.hpp"

GenStatus gz_generator::ConfiguratorDecompressIndex::generate()
{
    // Variables
    const Gen32u miniBlockCount           = m_pIndexTable->getMiniBlockCount();
    const Gen32u miniBlocksPerBlock       = m_pIndexTable->getMiniBlocksPerBlock();
    const Gen32u blockCount               = (miniBlockCount + miniBlocksPerBlock - 1U) / miniBlocksPerBlock;
    const Gen32u remainingMiniBlockCount  = miniBlockCount % miniBlocksPerBlock;

    // Generate deflate blocks
    for(Gen32u block = 0U; block < blockCount; block++)
    {
        Gen32u currentMiniBlockCount = 0;

        // Define mini-block count per block;
        if( block != LAST_FROM(blockCount) || !remainingMiniBlockCount)
        {
            currentMiniBlockCount = miniBlocksPerBlock;
        }
        else
        {
            currentMiniBlockCount = remainingMiniBlockCount;
        }

        // Write new block
        ConfiguratorDecompressIndex::declareDynamicBlock();
        generateMiniBlockSequence(currentMiniBlockCount);
    }

    // Complete block sequence
    ConfiguratorDecompressIndex::declareFinishBlock();

    return GEN_OK;
}

void gz_generator::ConfiguratorDecompressIndex::generateMiniBlock(Gen32u mini_block_size)
{
    // Variables
    Gen32u currentLength = 0U;  // Current symbols has been encoded in the stream

    // Generate mini-block of significant size
    while (currentLength < mini_block_size)
    {
        // Write literal or reference
        if ((m_literalProbability > static_cast<float>(m_random))
            || (MIN_MATCH > currentLength)
            || (MIN_MATCH > mini_block_size - currentLength))
        {
            ConfiguratorDecompressIndex::declareLiteral(static_cast<Gen32u>(m_randomLiteralCode));
            currentLength++;
        }
        else
        {
            // Variables
            Gen32u offset    = 0U;                            // Offset for current reference token
            Gen32u match     = 0U;                            // Match for current reference token
            const Gen32u maxOffset = currentLength;                 // Offset high limit
            Gen32u maxMatch  = mini_block_size - currentLength; // Match high limit

            // Calculate the current match  (Use big matches in the middle of mini-block)
            if ((32U >= maxMatch) && (0.1F > static_cast<float>(m_random)))
            {
                // Calculate current match low limit
                const Gen32u minMatch = GEN_MAX(MIN_MATCH, maxMatch - MIN_MATCH);

                // Set match range and get current one
                m_randomMatch.set_range(minMatch, maxMatch);
                match = static_cast<Gen32u>(m_randomMatch);
            }
            else
            {
                // Calculate current match high limit
                maxMatch = GEN_MIN(MAX_MATCH, maxMatch);
                maxMatch = (0.75F > static_cast<float>(m_randomMatch)) ? GEN_MIN(32U, maxMatch): maxMatch;

                // Set match range and get current one
                m_randomMatch.set_range(MIN_MATCH, maxMatch);
                match = static_cast<Gen32u>(m_randomMatch);
            }

            // Calculate the current offset
            if ((maxOffset <= MAX_OFFSET) && (0.10F > static_cast<float>(m_randomMatch)))
            {
                // Calculate current offset low limit
                const Gen32u minOffset = GEN_MAX(OWN_GEN_MIN_OFFSET, maxOffset - MIN_MATCH);

                // Set offset range and get current one
                m_randomOffset.set_range(minOffset, maxOffset);
                offset = static_cast<Gen32u>(m_randomOffset);
            }
            else
            {
                // Set offset range and get current one
                m_randomOffset.set_range(OWN_GEN_MIN_OFFSET, GEN_MIN(MAX_OFFSET, maxOffset));
                offset = static_cast<Gen32u>(m_randomOffset);
            }

            // Check if match will span mini-block with another
            if (match > mini_block_size - currentLength)
            {
                match = mini_block_size - currentLength;

                //If match is too small use literal tokens in the end of mini-block
                if (match < MIN_MATCH)
                {
                    continue;
                }
            }

            // Write reference in to stream
            ConfiguratorDecompressIndex::declareReference(match, offset);
            currentLength += match;
        }
    }

    // Make sure that mini-block generated correctly
    if (currentLength != mini_block_size)
    {
        throw std::out_of_range("Error occurred during mini-block generation! Mini-block expected size = "
                                + std::to_string(mini_block_size)
                                + "vs summary mini-block size = "
                                + std::to_string(currentLength));
    }
}

void gz_generator::ConfiguratorDecompressIndex::getIndex()
{
    *m_config << "log 4\n";
}
