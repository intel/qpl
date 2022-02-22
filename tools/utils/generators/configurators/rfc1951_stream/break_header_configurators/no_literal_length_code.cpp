/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "no_literal_length_code.hpp"

GenStatus gz_generator::NoLiteralLengthCodeConfigurator::generate()
{
    Gen32u* pLiteralLengthCodesTable = nullptr;
    Gen32u* pCodeLengthCodesTable = nullptr;

    qpl::test::random rand (257u, 286u, m_seed);
    Gen32u numberMissedCodes = 0u;

    pLiteralLengthCodesTable = new Gen32u[DEFAULT_LL_TABLE_LENGTH];
    pCodeLengthCodesTable    = new Gen32u[DEFAULT_CL_TABLE_LENGTH];

    makeRandomLengthCodesTable(pLiteralLengthCodesTable, DEFAULT_LL_TABLE_LENGTH, MAX_LL_CODE_BIT_LENGTH);
    makeRandomLengthCodesTable(pCodeLengthCodesTable, DEFAULT_CL_TABLE_LENGTH, MAX_CL_CODE_BIT_LENGTH);

    TestConfigurator::declareDynamicBlock();
    TestConfigurator::declareVectorToken(LL_VECTOR, pLiteralLengthCodesTable, DEFAULT_LL_TABLE_LENGTH);

    numberMissedCodes = static_cast<Gen32u>(rand);

    for (Gen32u i = 0u; i < numberMissedCodes; i++)
    {
        pLiteralLengthCodesTable[i] = 0u;
    }

    TestConfigurator::declareVectorToken(LL_ENCODED_VECTOR, pLiteralLengthCodesTable, DEFAULT_LL_TABLE_LENGTH);
    TestConfigurator::declareVectorToken(CL_VECTOR_ALT, pCodeLengthCodesTable, DEFAULT_CL_TABLE_LENGTH);

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();

    delete[] pLiteralLengthCodesTable;
    delete[] pCodeLengthCodesTable;

    return GEN_OK;
}
