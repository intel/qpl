/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "many_distance_codes.hpp"

GenStatus gz_generator::ManyDistanceCodesConfigurator::generate() {
    Gen32u* pDistanceLengthCodesTable    = nullptr;
    Gen32u  valueToRepeat                = 0U;
    Gen32u  distanceLengthCodesTableSize = DEFAULT_D_TABLE_LENGTH;
    Gen32u  testParameter                = 0U;

    qpl::test::random random(0U, 0U, m_seed);

    pDistanceLengthCodesTable = new Gen32u[distanceLengthCodesTableSize];
    TestConfigurator::makeRandomLengthCodesTable(pDistanceLengthCodesTable, distanceLengthCodesTableSize, 14U);

    random.set_range(1U, 25U);
    const Gen32u realCodesCount = static_cast<Gen32u>(random);
    random.set_range(3U, 30U - realCodesCount);
    const Gen32u repeatingCodesCount = static_cast<Gen32u>(random);

    if (3U != repeatingCodesCount) { valueToRepeat = 15U * static_cast<Gen32u>(m_random); }

    for (Gen32u repeatedCode = 0U; repeatedCode < repeatingCodesCount; repeatedCode++) {
        const Gen32u distanceTableIndex               = (realCodesCount - 1U) + repeatedCode;
        pDistanceLengthCodesTable[distanceTableIndex] = valueToRepeat;
    }

    distanceLengthCodesTableSize = realCodesCount + repeatingCodesCount - 1U;

    if (0U == valueToRepeat) {
        random.set_range(1U, repeatingCodesCount - 1U);
        testParameter = static_cast<Gen32u>(random);
    } else {
        do { //NOLINT(cppcoreguidelines-avoid-do-while)
            random.set_range(1U, repeatingCodesCount - 2U);
            testParameter = static_cast<Gen32u>(random);
        } while (ManyDistanceCodesConfigurator::breakInRun(repeatingCodesCount, repeatingCodesCount - testParameter));
    }
    TestConfigurator::declareDynamicBlock();
    TestConfigurator::declareVectorToken(D_ENCODED_VECTOR, pDistanceLengthCodesTable, distanceLengthCodesTableSize);
    TestConfigurator::declareVectorToken(D_VECTOR, pDistanceLengthCodesTable, distanceLengthCodesTableSize);
    TestConfigurator::declareTestToken(2U, testParameter);

    TestConfigurator::writeRandomBlock();
    TestConfigurator::declareFinishBlock();

    delete[] pDistanceLengthCodesTable;

    return GEN_OK;
}

bool gz_generator::ManyDistanceCodesConfigurator::breakInRun(Gen32u a, Gen32u b) //TODO NAMING!!
{
    a--;
    b--;
    while (b > 0U) {
        if (a < 3U) {
            a--;
            b--;
            continue;
        }
        if (a <= 6U) {
            b -= a;
            break;
        }
        a -= 6U;
        b -= 6U;
    }
    return b == 0U;
}
