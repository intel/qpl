/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "undefined_cl_code.hpp"

GenStatus gz_generator::UndefinedCodeLengthCodeConfigurator::generate() {
    Gen32u* pLiteralLengthTable   = nullptr;
    Gen32u* pDistanceLengthTable  = nullptr;
    Gen32u* code_length_table_ptr = nullptr;

    Gen32u dataTokenCount = 0; // The tokens that directly describe data stream

    qpl::test::random       randomLLCode(0U, 285U, m_seed);
    const qpl::test::random randomMatchCode(257U, 285U, m_seed);
    qpl::test::random       randomDistanceCode(0, 29U, m_seed);
    qpl::test::random       randomControlLiteralCode(0, 10U, m_seed);
    const qpl::test::random randomSmallMatch(3U, 10U, m_seed);
    const qpl::test::random randomLargeMatch(11U, 258U, m_seed);
    const qpl::test::random randomNoneControlLiteralCode(11U, 258U, m_seed);

    pLiteralLengthTable   = new Gen32u[DEFAULT_LL_TABLE_LENGTH];
    pDistanceLengthTable  = new Gen32u[DEFAULT_D_TABLE_LENGTH];
    code_length_table_ptr = new Gen32u[DEFAULT_CL_TABLE_LENGTH];

    TestConfigurator::makeRandomLengthCodesTable(pLiteralLengthTable, DEFAULT_LL_TABLE_LENGTH, 13U);
    TestConfigurator::makeRandomLengthCodesTable(pDistanceLengthTable, DEFAULT_D_TABLE_LENGTH, 13U);
    TestConfigurator::makeRandomLengthCodesTable(code_length_table_ptr, 16U, 7U);

    code_length_table_ptr[15] = 7U;
    code_length_table_ptr[16] = 0U;
    code_length_table_ptr[17] = 0U;
    code_length_table_ptr[18] = 0U;

    for (Gen32u i = 3U; i < DEFAULT_LL_TABLE_LENGTH; i += 4U) {
        pLiteralLengthTable[i] = 14U;
    }

    for (Gen32u i = 3U; i < DEFAULT_D_TABLE_LENGTH; i += 4U) {
        pDistanceLengthTable[i] = 14U;
    }

    TestConfigurator::declareDynamicBlock();

    if (static_cast<float>(m_random) < 0.5F) // wreck D code
    {
        pDistanceLengthTable[static_cast<Gen8u>(randomDistanceCode)] = 15U;
    } else //wreck LL code
    {
        const float random = static_cast<float>(m_random);
        if (random > 0.66F) {
            pLiteralLengthTable[static_cast<Gen8u>(randomLLCode)] = 15U;
        } else if (random > 0.33F) {
            pLiteralLengthTable[static_cast<Gen8u>(randomControlLiteralCode)] = 15U;
        } else if (random > 0.05F) {
            pLiteralLengthTable[static_cast<Gen8u>(randomDistanceCode)] = 15U;
        } else {
            pLiteralLengthTable[EOB_CODE] = 15U;
        }
    }

    TestConfigurator::declareVectorToken(LL_VECTOR, pLiteralLengthTable, DEFAULT_LL_TABLE_LENGTH);
    TestConfigurator::declareVectorToken(D_VECTOR, pDistanceLengthTable, DEFAULT_D_TABLE_LENGTH);
    TestConfigurator::declareVectorToken(CL_VECTOR_ALT, code_length_table_ptr, DEFAULT_CL_TABLE_LENGTH);

    code_length_table_ptr[15] = 0U;

    TestConfigurator::declareVectorToken(CL_ENCODED_VECTOR, code_length_table_ptr, DEFAULT_CL_TABLE_LENGTH);

    if (static_cast<float>(m_random) >= 0.05F) { dataTokenCount = static_cast<Gen8u>(m_randomTokenCount); }

    TestConfigurator::writeRandomReferenceSequence(dataTokenCount);

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();

    delete[] pLiteralLengthTable;
    delete[] pDistanceLengthTable;
    delete[] code_length_table_ptr;

    return GEN_OK;
}
