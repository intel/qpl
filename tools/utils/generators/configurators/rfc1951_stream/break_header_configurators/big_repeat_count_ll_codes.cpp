/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "big_repeat_count_ll_codes.h"

GenStatus gz_generator::BigRepeatCountLiteralLengthCodesConfigurator::generate() {
    Gen32u* pLiteralLengthCodesTable  = NULL;
    Gen32u  code_length               = 0U;
    Gen32u  repeatedCode              = 0U;
    Gen32u  repeatingCount            = 0U;
    Gen32u  maximumLiteralCode        = 0U;
    Gen32u  writtenLiteralLengthCodes = 0U;

    qpl::test::random random(0U, 0U, m_seed);

    if (0.25F > static_cast<float>(m_random)) {
        random.set_range(1U, 8U);
        code_length = static_cast<Gen32u>(random);
        random.set_range(4U, 7U);
        repeatingCount     = static_cast<Gen32u>(random);
        maximumLiteralCode = 284U;
    } else {
        code_length = 0U;
        random.set_range(3U, 138U);
        repeatingCount     = static_cast<Gen32u>(random);
        maximumLiteralCode = 285U;
    }

    if (0.5F > static_cast<float>(m_random)) {
        random.set_range(DEFAULT_LL_TABLE_LENGTH + 1U - repeatingCount, maximumLiteralCode);
    } else {
        random.set_range(GEN_MAX(245, DEFAULT_LL_TABLE_LENGTH + 1U - repeatingCount), maximumLiteralCode);
    }

    repeatedCode = static_cast<Gen32u>(random);

    pLiteralLengthCodesTable = new Gen32u[repeatedCode + repeatingCount];

    {
        Gen32u code = 0U;
        random.set_range(9U, 15U);
        for (code = code; code < repeatedCode; code++) {
            pLiteralLengthCodesTable[code] = static_cast<Gen32u>(random);
        }
        for (code = code; code < repeatedCode + repeatingCount; code++) {
            pLiteralLengthCodesTable[code] = code_length;
        }
        writtenLiteralLengthCodes = code;
    }

    TestConfigurator::declareDynamicBlock();
    TestConfigurator::declareVectorToken(LL_ENCODED_VECTOR, pLiteralLengthCodesTable, writtenLiteralLengthCodes);
    TestConfigurator::declareLiteral(0U);

    TestConfigurator::writeRandomBlock();
    TestConfigurator::declareFinishBlock();

    delete[] pLiteralLengthCodesTable;

    return GEN_OK;
}
