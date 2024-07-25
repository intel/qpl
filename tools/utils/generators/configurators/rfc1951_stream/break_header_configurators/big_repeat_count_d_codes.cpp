/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "big_repeat_count_d_codes.hpp"

GenStatus gz_generator::BigRepeatCountDistanceLengthCodesConfigurator::generate() {
    Gen32u* pDistanceLengthCodesTable  = nullptr;
    Gen32u  code_length                = 0U;
    Gen32u  repeatedCode               = 0U;
    Gen32u  repeatingCount             = 0U;
    Gen32u  maximumDistanceCode        = 0U;
    Gen32u  writtenDistanceLengthCodes = 0U;

    qpl::test::random random(0U, 0U, m_seed);

    if (0.25F > static_cast<float>(m_random)) {
        random.set_range(1U, 8U);
        code_length = static_cast<Gen32u>(random);
        random.set_range(4U, 7U);
        repeatingCount      = static_cast<Gen32u>(random);
        maximumDistanceCode = 28U;
    } else {
        code_length = 0U;
        random.set_range(3U, 29U);
        repeatingCount      = static_cast<Gen32u>(random);
        maximumDistanceCode = 29U;
    }

    random.set_range(GEN_MAX(5U, 31U - repeatingCount), maximumDistanceCode);
    repeatedCode = static_cast<Gen32u>(random);

    pDistanceLengthCodesTable = new Gen32u[repeatedCode + repeatingCount];

    {
        Gen32u code = 0U;
        random.set_range(9U, 15U);
        for (code = code; code < repeatedCode; code++) {
            pDistanceLengthCodesTable[code] = static_cast<Gen32u>(random);
        }
        for (code = code; code < repeatedCode + repeatingCount; code++) {
            pDistanceLengthCodesTable[code] = code_length;
        }
        writtenDistanceLengthCodes = code;
    }

    TestConfigurator::declareDynamicBlock();
    TestConfigurator::declareVectorToken(D_ENCODED_VECTOR, pDistanceLengthCodesTable, writtenDistanceLengthCodes);
    TestConfigurator::declareLiteral(0U);

    TestConfigurator::writeRandomBlock();
    TestConfigurator::declareFinishBlock();

    delete[] pDistanceLengthCodesTable;

    return GEN_OK;
}
