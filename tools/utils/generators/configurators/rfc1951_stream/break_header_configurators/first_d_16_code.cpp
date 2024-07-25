/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "first_d_16_code.hpp"

GenStatus gz_generator::FirstDistanceLengthCode16Configurator::generate() {
    Gen32u*           pDistanceLengthCodesTable = nullptr;
    qpl::test::random rand(4U, 7U, m_seed);

    const Gen8u repeatingCount = static_cast<Gen8u>(rand);

    pDistanceLengthCodesTable = new Gen32u[DEFAULT_D_TABLE_LENGTH];

    TestConfigurator::makeRandomLengthCodesTable(pDistanceLengthCodesTable, DEFAULT_D_TABLE_LENGTH,
                                                 MAX_D_CODE_BIT_LENGTH);

    for (Gen8u i = 1U; i < repeatingCount; i++) {
        pDistanceLengthCodesTable[i] = pDistanceLengthCodesTable[0];
    }

    TestConfigurator::declareDynamicBlock();
    TestConfigurator::declareVectorToken(D_VECTOR, pDistanceLengthCodesTable, DEFAULT_D_TABLE_LENGTH);
    TestConfigurator::declareVectorToken(D_ENCODED_VECTOR, pDistanceLengthCodesTable, DEFAULT_D_TABLE_LENGTH);
    TestConfigurator::declareTestToken(4U, 1U);

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();

    delete[] pDistanceLengthCodesTable;
    return GEN_OK;
}
