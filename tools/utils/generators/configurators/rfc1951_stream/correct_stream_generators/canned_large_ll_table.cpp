/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "canned_large_ll_table.hpp"

#define RESET_ON_LIMIT(value, valueLimit, baseValue) (((value) >= (valueLimit)) ? (baseValue) : (value))

GenStatus gz_generator::CannedLargeLiteralLengthTableConfigurator::generate() {
    Gen32u match      = 0U;
    Gen32u offset     = 1U;
    Gen32u literal    = 0U;
    Gen32u hack_258   = 0U;
    Gen32u matchIndex = 0U;

    std::vector<Gen32u> matches = {3U,  4U,  5U,  6U,  7U,  8U,  9U,  10U, 11U,  13U,  15U,  17U,  19U,  23U, 27U,
                                   31U, 35U, 43U, 51U, 59U, 67U, 83U, 99U, 115U, 131U, 163U, 195U, 227U, 258U};

    for (Gen32u currentReferenceCount = 26U; currentReferenceCount <= matches.size(); currentReferenceCount++) {
        for (Gen32u currentLiteralCount = 253U; currentLiteralCount <= LITERALS_HIGH_BORDER; currentLiteralCount++) {
            TestConfigurator::declareDynamicBlock();

            for (Gen32u i = 0; i < currentLiteralCount; i++) {
                TestConfigurator::declareLiteral(literal);
                literal++;
                literal = RESET_ON_LIMIT(literal, LITERALS_HIGH_BORDER, 0U);
            }

            for (Gen32u i = 0; i < currentReferenceCount; i++) {
                match = matches[matchIndex];
                matchIndex++;
                matchIndex = RESET_ON_LIMIT(matchIndex, matches.size(), 0U);

                if (match == MAX_MATCH) {
                    match += hack_258;
                    hack_258 ^= 1;
                }

                TestConfigurator::declareReference(match, offset);
                offset++;
                offset = RESET_ON_LIMIT(offset, 4U, 1U);
            }
        }
    }
    TestConfigurator::declareFinishBlock();

    return GEN_OK;
}