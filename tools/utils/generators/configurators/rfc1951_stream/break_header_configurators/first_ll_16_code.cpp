#include "first_ll_16_code.hpp"

/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/



GenStatus gz_generator::FirstLiteralLengthCode16Configurator::generate()
{
    Gen32u* pLiteralLengthCodesTable = nullptr;

    qpl::test::random rand (4u, 7u, m_seed);

    Gen8u  repeatingCount = static_cast<Gen8u>(rand);

    pLiteralLengthCodesTable = new Gen32u[DEFAULT_LL_TABLE_LENGTH];

    TestConfigurator::makeRandomLengthCodesTable(pLiteralLengthCodesTable, DEFAULT_LL_TABLE_LENGTH,
                                                 MAX_LL_CODE_BIT_LENGTH);

    for (Gen8u i = 1u; i < repeatingCount; i++)
    {
        pLiteralLengthCodesTable[i] = pLiteralLengthCodesTable[0u];
    }

    TestConfigurator::declareDynamicBlock();
    TestConfigurator::declareVectorToken(LL_VECTOR, pLiteralLengthCodesTable, DEFAULT_LL_TABLE_LENGTH);
    TestConfigurator::declareVectorToken(LL_ENCODED_VECTOR, pLiteralLengthCodesTable, DEFAULT_LL_TABLE_LENGTH);
    TestConfigurator::declareTestToken(4u, 0u);

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();
     
    delete[] pLiteralLengthCodesTable;

    return GEN_OK;
}