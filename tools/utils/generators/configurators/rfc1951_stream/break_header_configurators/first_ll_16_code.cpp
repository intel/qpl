#include "first_ll_16_code.hpp"

/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

GenStatus gz_generator::FirstLiteralLengthCode16Configurator::generate() {
    Gen32u* pLiteralLengthCodesTable = nullptr;

    qpl::test::random rand(4U, 7U, m_seed);

    const Gen8u repeatingCount = static_cast<Gen8u>(rand);

    pLiteralLengthCodesTable = new Gen32u[DEFAULT_LL_TABLE_LENGTH];

    TestConfigurator::makeRandomLengthCodesTable(pLiteralLengthCodesTable, DEFAULT_LL_TABLE_LENGTH,
                                                 MAX_LL_CODE_BIT_LENGTH);

    for (Gen8u i = 1U; i < repeatingCount; i++) {
        pLiteralLengthCodesTable[i] = pLiteralLengthCodesTable[0U];
    }

    TestConfigurator::declareDynamicBlock();
    TestConfigurator::declareVectorToken(LL_VECTOR, pLiteralLengthCodesTable, DEFAULT_LL_TABLE_LENGTH);
    TestConfigurator::declareVectorToken(LL_ENCODED_VECTOR, pLiteralLengthCodesTable, DEFAULT_LL_TABLE_LENGTH);
    TestConfigurator::declareTestToken(4U, 0U);

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();

    delete[] pLiteralLengthCodesTable;

    return GEN_OK;
}
