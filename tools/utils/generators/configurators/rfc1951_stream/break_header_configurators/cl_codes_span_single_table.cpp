#include "cl_codes_span_single_table.hpp"

/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

GenStatus gz_generator::StradingCodeLengthCodesConfigurator::generate()
{
    Gen32u* pLiteralLengthCodesTable = nullptr;
    Gen32u* pDistanceLengthCodesTable = nullptr;

    Gen32u insertedValue        = 0u;
    Gen32u totalValues          = 0u;
    Gen32u firstValuesCount     = 0u;
    Gen32u secondValuesCount    = 0u;

    qpl::test::random rangeRandom(0u, 0u, m_seed);

    pLiteralLengthCodesTable  = new Gen32u[DEFAULT_LL_TABLE_LENGTH];
    pDistanceLengthCodesTable = new Gen32u[DEFAULT_D_TABLE_LENGTH];

    TestConfigurator::makeRandomLengthCodesTable(pLiteralLengthCodesTable, DEFAULT_LL_TABLE_LENGTH, 14u);
    TestConfigurator::makeRandomLengthCodesTable(pDistanceLengthCodesTable, DEFAULT_D_TABLE_LENGTH, 14u);
    
    if (static_cast<float>(m_random) < 0.33f)
    {
        insertedValue = 15u;
        rangeRandom.set_range(4u, 7u);
        totalValues = static_cast<Gen32u>(rangeRandom);
        rangeRandom.set_range(2u, totalValues - 1u);
        firstValuesCount = static_cast<Gen32u>(rangeRandom);
    } 
    else if (static_cast<float>(m_random) < 0.5f)
    {
        rangeRandom.set_range(3u, 10u);
        totalValues = static_cast<Gen32u>(rangeRandom);
        rangeRandom.set_range(1u, totalValues - 1u);
        firstValuesCount = static_cast<Gen32u>(rangeRandom);
    }
    else
    {
        rangeRandom.set_range(11u, 138u);
        totalValues = static_cast<Gen32u>(rangeRandom);
        rangeRandom.set_range(1u, totalValues - 1u);
        firstValuesCount = static_cast<Gen32u>(rangeRandom);
        firstValuesCount = ((totalValues - firstValuesCount) > 29u) ?
                            totalValues - 29u :
                            firstValuesCount;
    }
    secondValuesCount = totalValues - firstValuesCount;

    for(Gen32u literal = 286u - firstValuesCount; literal < 286u; literal++)
    {
        pLiteralLengthCodesTable[literal] = insertedValue;
    }

    for(Gen32u distance = 0u; distance < secondValuesCount; distance++)
    {
        pDistanceLengthCodesTable[distance] = insertedValue;
    }


    TestConfigurator::declareDynamicBlock();
    TestConfigurator::declareVectorToken(LL_ENCODED_VECTOR, pLiteralLengthCodesTable, DEFAULT_LL_TABLE_LENGTH);

    pLiteralLengthCodesTable[256] = (pLiteralLengthCodesTable[256] == 0u) ? 15u : pLiteralLengthCodesTable[256];
    TestConfigurator::declareVectorToken(LL_VECTOR, pLiteralLengthCodesTable, DEFAULT_LL_TABLE_LENGTH);
    TestConfigurator::declareVectorToken(D_ENCODED_VECTOR, pDistanceLengthCodesTable, DEFAULT_D_TABLE_LENGTH);
    TestConfigurator::declareVectorToken(D_VECTOR, pDistanceLengthCodesTable, DEFAULT_D_TABLE_LENGTH);

    TestConfigurator::declareTestToken(1u);

    TestConfigurator::writeRandomHuffmanBlock(1u);
    TestConfigurator::declareFinishBlock();

    delete[] pLiteralLengthCodesTable;
    delete[] pDistanceLengthCodesTable;

    return GEN_OK;
}