/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "many_distance_codes.hpp"

GenStatus gz_generator::ManyDistanceCodesConfigurator::generate()
{
    Gen32u* pDistanceLengthCodesTable       = nullptr;
    Gen32u  realCodesCount                  = 0u;
    Gen32u  repeatingCodesCount             = 0u;
    Gen32u  valueToRepeat                   = 0u;
    Gen32u  distanceLengthCodesTableSize    = DEFAULT_D_TABLE_LENGTH;
    Gen32u  testParameter;

    qpl::test::random random(0u, 0u, m_seed);

    pDistanceLengthCodesTable = new Gen32u[distanceLengthCodesTableSize];
    TestConfigurator::makeRandomLengthCodesTable(pDistanceLengthCodesTable, distanceLengthCodesTableSize, 14u);

    random.set_range(1u, 25u);
    realCodesCount = static_cast<Gen32u>(random);
    random.set_range(3u, 30u - realCodesCount);
    repeatingCodesCount = static_cast<Gen32u>(random);

    if (3u != repeatingCodesCount)
    {
        valueToRepeat = 15u * static_cast<Gen32u>(m_random);
    }

    for (Gen32u repeatedCode = 0u ; repeatedCode < repeatingCodesCount; repeatedCode++)
    {
        Gen32u distanceTableIndex = (realCodesCount - 1) + repeatedCode;
        pDistanceLengthCodesTable[distanceTableIndex] = valueToRepeat;
    }

    distanceLengthCodesTableSize = realCodesCount + repeatingCodesCount - 1u;

    if (0u == valueToRepeat)
    {
        random.set_range(1u, repeatingCodesCount - 1u);
        testParameter = static_cast<Gen32u>(random);
    }
    else
    {
        do
        {
            random.set_range(1u, repeatingCodesCount - 2u);
            testParameter = static_cast<Gen32u>(random);
        } while(ManyDistanceCodesConfigurator::breakInRun(repeatingCodesCount, repeatingCodesCount - testParameter));
    }
    TestConfigurator::declareDynamicBlock();
    TestConfigurator::declareVectorToken(D_ENCODED_VECTOR, pDistanceLengthCodesTable, distanceLengthCodesTableSize);
    TestConfigurator::declareVectorToken(D_VECTOR, pDistanceLengthCodesTable, distanceLengthCodesTableSize);
    TestConfigurator::declareTestToken(2u, testParameter);

    TestConfigurator::writeRandomBlock();
    TestConfigurator::declareFinishBlock();

    delete[] pDistanceLengthCodesTable;

    return GEN_OK;
}

bool gz_generator::ManyDistanceCodesConfigurator::breakInRun(Gen32u a, Gen32u b) //TODO NAMING!!
{
    a--;
    b--;
    while (b > 0)
    {
        if (a < 3)
        {
            a--;
            b--;
            continue;
        }
        if (a <= 6)
        {
            b -= a;
            break;
        }
        a -= 6;
        b -= 6;
    }
    return (b == 0) ? true : false;
}