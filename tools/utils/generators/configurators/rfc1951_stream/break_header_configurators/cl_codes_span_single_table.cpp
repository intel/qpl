#include "cl_codes_span_single_table.hpp"

/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

GenStatus gz_generator::StradingCodeLengthCodesConfigurator::generate() {
    Gen32u* pLiteralLengthCodesTable  = nullptr;
    Gen32u* pDistanceLengthCodesTable = nullptr;

    Gen32u insertedValue     = 0U;
    Gen32u totalValues       = 0U;
    Gen32u firstValuesCount  = 0U;
    Gen32u secondValuesCount = 0U;

    qpl::test::random rangeRandom(0U, 0U, m_seed);

    pLiteralLengthCodesTable  = new Gen32u[DEFAULT_LL_TABLE_LENGTH];
    pDistanceLengthCodesTable = new Gen32u[DEFAULT_D_TABLE_LENGTH];

    TestConfigurator::makeRandomLengthCodesTable(pLiteralLengthCodesTable, DEFAULT_LL_TABLE_LENGTH, 14U);
    TestConfigurator::makeRandomLengthCodesTable(pDistanceLengthCodesTable, DEFAULT_D_TABLE_LENGTH, 14U);

    if (static_cast<float>(m_random) < 0.33F) {
        insertedValue = 15U;
        rangeRandom.set_range(4U, 7U);
        totalValues = static_cast<Gen32u>(rangeRandom);
        rangeRandom.set_range(2U, totalValues - 1U);
        firstValuesCount = static_cast<Gen32u>(rangeRandom);
    } else if (static_cast<float>(m_random) < 0.5F) {
        rangeRandom.set_range(3U, 10U);
        totalValues = static_cast<Gen32u>(rangeRandom);
        rangeRandom.set_range(1U, totalValues - 1U);
        firstValuesCount = static_cast<Gen32u>(rangeRandom);
    } else {
        rangeRandom.set_range(11U, 138U);
        totalValues = static_cast<Gen32u>(rangeRandom);
        rangeRandom.set_range(1U, totalValues - 1U);
        firstValuesCount = static_cast<Gen32u>(rangeRandom);
        firstValuesCount = ((totalValues - firstValuesCount) > 29U) ? totalValues - 29U : firstValuesCount;
    }
    secondValuesCount = totalValues - firstValuesCount;

    for (Gen32u literal = 286U - firstValuesCount; literal < 286U; literal++) {
        pLiteralLengthCodesTable[literal] = insertedValue;
    }

    for (Gen32u distance = 0U; distance < secondValuesCount; distance++) {
        pDistanceLengthCodesTable[distance] = insertedValue;
    }

    TestConfigurator::declareDynamicBlock();
    TestConfigurator::declareVectorToken(LL_ENCODED_VECTOR, pLiteralLengthCodesTable, DEFAULT_LL_TABLE_LENGTH);

    pLiteralLengthCodesTable[256] = (pLiteralLengthCodesTable[256] == 0U) ? 15U : pLiteralLengthCodesTable[256];
    TestConfigurator::declareVectorToken(LL_VECTOR, pLiteralLengthCodesTable, DEFAULT_LL_TABLE_LENGTH);
    TestConfigurator::declareVectorToken(D_ENCODED_VECTOR, pDistanceLengthCodesTable, DEFAULT_D_TABLE_LENGTH);
    TestConfigurator::declareVectorToken(D_VECTOR, pDistanceLengthCodesTable, DEFAULT_D_TABLE_LENGTH);

    TestConfigurator::declareTestToken(1U);

    TestConfigurator::writeRandomHuffmanBlock(1U);
    TestConfigurator::declareFinishBlock();

    delete[] pLiteralLengthCodesTable;
    delete[] pDistanceLengthCodesTable;

    return GEN_OK;
}