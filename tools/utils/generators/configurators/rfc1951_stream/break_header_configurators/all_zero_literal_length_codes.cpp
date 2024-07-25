/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "all_zero_literal_length_codes.hpp"

GenStatus gz_generator::AllZeroLiteralLengthCodesConfigurator::generate() {
    Gen32u* pLiteralLengthCodesTable = nullptr;
    Gen32u* pCodeLengthCodesTable    = nullptr;

    const Gen32u numberMissedCodes = 286U;

    pLiteralLengthCodesTable = new Gen32u[DEFAULT_LL_TABLE_LENGTH];
    pCodeLengthCodesTable    = new Gen32u[DEFAULT_CL_TABLE_LENGTH];

    makeRandomLengthCodesTable(pLiteralLengthCodesTable, DEFAULT_LL_TABLE_LENGTH, MAX_LL_CODE_BIT_LENGTH);
    makeRandomLengthCodesTable(pCodeLengthCodesTable, DEFAULT_CL_TABLE_LENGTH, MAX_CL_CODE_BIT_LENGTH);

    TestConfigurator::declareDynamicBlock();
    TestConfigurator::declareVectorToken(LL_VECTOR, pLiteralLengthCodesTable, DEFAULT_LL_TABLE_LENGTH);

    for (Gen32u i = 0U; i < numberMissedCodes; i++) {
        pLiteralLengthCodesTable[i] = 0U;
    }

    TestConfigurator::declareVectorToken(LL_ENCODED_VECTOR, pLiteralLengthCodesTable, DEFAULT_LL_TABLE_LENGTH);
    TestConfigurator::declareVectorToken(CL_VECTOR_ALT, pCodeLengthCodesTable, DEFAULT_CL_TABLE_LENGTH);

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();

    delete[] pLiteralLengthCodesTable;
    delete[] pCodeLengthCodesTable;

    return GEN_OK;
}
