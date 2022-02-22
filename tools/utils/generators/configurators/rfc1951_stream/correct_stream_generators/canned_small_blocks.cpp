/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "canned_small_blocks.hpp"

GenStatus gz_generator::CannedSmallBlocksConfigurator::generate()
{
    Gen32u currentLiteral = 0;

    for (Gen32u maxReferencesInBlock = 0; maxReferencesInBlock < 4u; maxReferencesInBlock++)
    {
        for (Gen32u maxLiteralsCurrentBlock = 0; maxLiteralsCurrentBlock < 4u; maxLiteralsCurrentBlock++)
        {

            TestConfigurator::declareDynamicBlock();

            //generate constant literal sequence
            for (Gen32u literals = 0; literals < maxLiteralsCurrentBlock; literals++)
            {
                TestConfigurator::declareLiteral(currentLiteral++);
            }

            //generate references to literal sequence
            for (Gen32u lookUp = 0; lookUp < maxLiteralsCurrentBlock; lookUp++)
            {
                Gen32u match = lookUp + MIN_MATCH;
                Gen32u offset = lookUp + 1u;
                TestConfigurator::declareReference(match, offset);
            }
        }
    }

    TestConfigurator::declareFinishBlock();

    return GEN_OK;
}