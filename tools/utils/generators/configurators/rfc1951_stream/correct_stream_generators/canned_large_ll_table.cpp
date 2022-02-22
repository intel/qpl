/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "canned_large_ll_table.hpp"

#define RESET_ON_LIMIT( value, valueLimit, baseValue ) ( ((value) >= (valueLimit)) ? (baseValue) : (value) )

GenStatus gz_generator::CannedLargeLiteralLengthTableConfigurator::generate()
{
    Gen32u match      = 0u;
    Gen32u offset     = 1u;
    Gen32u literal    = 0u;
    Gen32u hack_258   = 0u;
    Gen32u matchIndex = 0u;

    std::vector <Gen32u> matches = {3u,   4u,   5u,   6u,   7u,
                                    8u,   9u,   10u,  11u,  13u,
                                    15u,  17u,  19u,  23u,  27u,
                                    31u,  35u,  43u,  51u,  59u,
                                    67u,  83u,  99u,  115u, 131u,
                                    163u, 195u, 227u, 258u};

    for ( Gen32u currentReferenceCount = 26u; currentReferenceCount <= matches.size(); currentReferenceCount++)
    {
        for ( Gen32u currentLiteralCount = 253u; currentLiteralCount <= LITERALS_HIGH_BORDER; currentLiteralCount++)
        {
            TestConfigurator::declareDynamicBlock();

            for (Gen32u i = 0; i < currentLiteralCount; i++)
            {
                TestConfigurator::declareLiteral(literal);
                literal++;
                literal = RESET_ON_LIMIT(literal, LITERALS_HIGH_BORDER, 0u);
            }

            for (Gen32u i = 0; i < currentReferenceCount; i++)
            {
                match = matches[matchIndex];
                matchIndex++;
                matchIndex = RESET_ON_LIMIT(matchIndex, matches.size(), 0u);

                if (match == MAX_MATCH)
                {
                    match += hack_258;
                    hack_258 ^= 1;
                }

                TestConfigurator::declareReference(match, offset);
                offset++;
                offset = RESET_ON_LIMIT(offset, 4u, 1u);
            }
        }
    }
    TestConfigurator::declareFinishBlock();

    return GEN_OK;
}