/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "big_repeat_count_d_codes.hpp"

GenStatus gz_generator::BigRepeatCountDistanceLengthCodesConfigurator::generate()
{
    Gen32u *pDistanceLengthCodesTable  = nullptr;
    Gen32u  code_length                 = 0u;
    Gen32u  repeatedCode               = 0u;
    Gen32u  repeatingCount             = 0u;
    Gen32u  maximumDistanceCode        = 0u;
    Gen32u  writtenDistanceLengthCodes = 0u;

    qpl::test::random random(0u, 0u, m_seed);

    if (0.25f > static_cast<float>(m_random))
    {
        random.set_range(1u, 8u);
        code_length = static_cast<Gen32u>(random);
        random.set_range(4u, 7u);
        repeatingCount = static_cast<Gen32u>(random);
        maximumDistanceCode = 28u;
    }
    else
    {
        code_length = 0u;
        random.set_range(3u, 29u);
        repeatingCount = static_cast<Gen32u>(random);
        maximumDistanceCode = 29u;
    }

    random.set_range(GEN_MAX(5u, 31u - repeatingCount), maximumDistanceCode);
    repeatedCode = static_cast<Gen32u>(random);

    pDistanceLengthCodesTable = new Gen32u[repeatedCode + repeatingCount];

    {
        Gen32u code = 0u;
        random.set_range(9u, 15u);
        for (code = code; code < repeatedCode; code++)
        {
            pDistanceLengthCodesTable[code] = static_cast<Gen32u>(random);
        }
        for (code = code; code < repeatedCode + repeatingCount; code++)
        {
            pDistanceLengthCodesTable[code] = code_length;
        }
        writtenDistanceLengthCodes = code;
    }

    TestConfigurator::declareDynamicBlock();
    TestConfigurator::declareVectorToken(D_ENCODED_VECTOR, pDistanceLengthCodesTable, writtenDistanceLengthCodes);
    TestConfigurator::declareLiteral(0u);

    TestConfigurator::writeRandomBlock();
    TestConfigurator::declareFinishBlock();

    delete[] pDistanceLengthCodesTable;

    return GEN_OK;
}
