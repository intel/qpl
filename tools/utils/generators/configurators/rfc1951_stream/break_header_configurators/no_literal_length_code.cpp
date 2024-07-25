/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "no_literal_length_code.hpp"

GenStatus gz_generator::NoLiteralLengthCodeConfigurator::generate() {
    Gen32u* pLiteralLengthCodesTable = nullptr;
    Gen32u* pCodeLengthCodesTable    = nullptr;

    // Indices 257-285 are the literal length codes
    // Stop at 285 so that not every LL code is 0, which is a different test with a different error code
    qpl::test::random rand(257U, 285U, m_seed);
    Gen32u            numberMissedCodes = 0U;

    pLiteralLengthCodesTable = new Gen32u[DEFAULT_LL_TABLE_LENGTH];
    pCodeLengthCodesTable    = new Gen32u[DEFAULT_CL_TABLE_LENGTH];

    // Set default max LL code bit length to 14 bits (MAX_LL_CODE_BIT_LENGTH - 1u)
    makeRandomLengthCodesTable(pLiteralLengthCodesTable, DEFAULT_LL_TABLE_LENGTH, MAX_LL_CODE_BIT_LENGTH - 1U);
    makeRandomLengthCodesTable(pCodeLengthCodesTable, DEFAULT_CL_TABLE_LENGTH, MAX_CL_CODE_BIT_LENGTH);

    numberMissedCodes = static_cast<Gen32u>(rand);

    // Set the max LL code bit length to 15 bits for all missing codes so that all missing codes will end up
    // at the end of the sorted list of all LL code lengths. This will make sure that the non-missing codes
    // will have valid distance codes
    for (Gen32u i = 0U; i < numberMissedCodes; i++) {
        pLiteralLengthCodesTable[i] = MAX_LL_CODE_BIT_LENGTH;
    }

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
