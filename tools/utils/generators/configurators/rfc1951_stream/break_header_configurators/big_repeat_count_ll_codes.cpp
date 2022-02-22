/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "big_repeat_count_ll_codes.h"

GenStatus gz_generator::BigRepeatCountLiteralLengthCodesConfigurator::generate()
{
    Gen32u *pLiteralLengthCodesTable  = NULL;
    Gen32u  code_length                = 0u;
    Gen32u  repeatedCode              = 0u;
    Gen32u  repeatingCount            = 0u;
    Gen32u  maximumLiteralCode        = 0u;
    Gen32u  writtenLiteralLengthCodes = 0u;

    qpl::test::random random(0u, 0u, m_seed);

    if (0.25f > static_cast<float>(m_random))
    {
        random.set_range(1u, 8u);
        code_length = static_cast<Gen32u>(random);
        random.set_range(4u, 7u);
        repeatingCount = static_cast<Gen32u>(random);
        maximumLiteralCode = 284u;
    }
    else
    {
        code_length = 0u;
        random.set_range(3u, 138u);
        repeatingCount = static_cast<Gen32u>(random);
        maximumLiteralCode = 285u;
    }

    if (0.5f > static_cast<float>(m_random))
    {
        random.set_range(DEFAULT_LL_TABLE_LENGTH + 1u - repeatingCount, maximumLiteralCode);
    }
    else
    {
        random.set_range(GEN_MAX(245, DEFAULT_LL_TABLE_LENGTH + 1u - repeatingCount), maximumLiteralCode);
    }

    repeatedCode = static_cast<Gen32u>(random);

    pLiteralLengthCodesTable = new Gen32u[repeatedCode + repeatingCount];

    {
        Gen32u code = 0u;
        random.set_range(9u, 15u);
        for (code; code < repeatedCode; code++)
        {
            pLiteralLengthCodesTable[code] = static_cast<Gen32u>(random);
        }
        for (code; code < repeatedCode + repeatingCount; code++)
        {
            pLiteralLengthCodesTable[code] = code_length;
        }
        writtenLiteralLengthCodes = code;
    }

    TestConfigurator::declareDynamicBlock();
    TestConfigurator::declareVectorToken(LL_ENCODED_VECTOR, pLiteralLengthCodesTable, writtenLiteralLengthCodes);
    TestConfigurator::declareLiteral(0u);

    TestConfigurator::writeRandomBlock();
    TestConfigurator::declareFinishBlock();

    delete[] pLiteralLengthCodesTable;

    return GEN_OK;
}


