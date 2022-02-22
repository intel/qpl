/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "undefined_cl_code.hpp"

GenStatus gz_generator::UndefinedCodeLengthCodeConfigurator::generate()
{
    Gen32u* pLiteralLengthTable  = nullptr;
    Gen32u* pDistanceLengthTable = nullptr;
    Gen32u* code_length_table_ptr     = nullptr;

    Gen32u dataTokenCount        = 0; // The tokens that directly describe data stream

    qpl::test::random randomLLCode(0u, 285u, m_seed);
    qpl::test::random randomMatchCode(257u, 285u, m_seed);
    qpl::test::random randomDistanceCode(0, 29u, m_seed);
    qpl::test::random randomControlLiteralCode(0, 10u, m_seed);
    qpl::test::random randomSmallMatch(3u, 10u, m_seed);
    qpl::test::random randomLargeMatch(11u, 258u, m_seed);
    qpl::test::random randomNoneControlLiteralCode(11u, 258u, m_seed);

    pLiteralLengthTable  = new Gen32u[DEFAULT_LL_TABLE_LENGTH];
    pDistanceLengthTable = new Gen32u[DEFAULT_D_TABLE_LENGTH];
    code_length_table_ptr     = new Gen32u[DEFAULT_CL_TABLE_LENGTH];

    TestConfigurator::makeRandomLengthCodesTable(pLiteralLengthTable, DEFAULT_LL_TABLE_LENGTH, 13u);
    TestConfigurator::makeRandomLengthCodesTable(pDistanceLengthTable, DEFAULT_D_TABLE_LENGTH, 13u);
    TestConfigurator::makeRandomLengthCodesTable(code_length_table_ptr, 16u, 7u);

    code_length_table_ptr[15] = 7u;
    code_length_table_ptr[16] = 0u;
    code_length_table_ptr[17] = 0u;
    code_length_table_ptr[18] = 0u;

    for (Gen32u i = 3u; i < DEFAULT_LL_TABLE_LENGTH; i += 4u)
    {
        pLiteralLengthTable[i] = 14u;
    }

    for (Gen32u i = 3u; i < DEFAULT_D_TABLE_LENGTH; i+=4u)
    {
        pDistanceLengthTable[i] = 14u;
    }

    TestConfigurator::declareDynamicBlock();

    if (static_cast<float>(m_random) < 0.5f) // wreck D code
    {   
        pDistanceLengthTable[static_cast<Gen8u>(randomDistanceCode)] = 15u;
    }
    else //wreck LL code
    {
        float random = static_cast<float>(m_random);
        if (random > 0.66f)
        {
            pLiteralLengthTable[static_cast<Gen8u>(randomLLCode)] = 15u;
        }
        else if (random > 0.33f)
        {
            pLiteralLengthTable[static_cast<Gen8u>(randomControlLiteralCode)] = 15u;
        }
        else if (random > 0.05f)
        {
            pLiteralLengthTable[static_cast<Gen8u>(randomDistanceCode)] = 15u;
        }
        else
        {
            pLiteralLengthTable[EOB_CODE] = 15u;
        }
    }

    TestConfigurator::declareVectorToken(LL_VECTOR, pLiteralLengthTable, DEFAULT_LL_TABLE_LENGTH);
    TestConfigurator::declareVectorToken(D_VECTOR, pDistanceLengthTable, DEFAULT_D_TABLE_LENGTH);
    TestConfigurator::declareVectorToken(CL_VECTOR_ALT, code_length_table_ptr, DEFAULT_CL_TABLE_LENGTH);

    code_length_table_ptr[15] = 0u;

    TestConfigurator::declareVectorToken(CL_ENCODED_VECTOR, code_length_table_ptr, DEFAULT_CL_TABLE_LENGTH);

    if (static_cast<float>(m_random) >= 0.05f)
    {
        dataTokenCount = static_cast<Gen8u>(m_randomTokenCount);
    }

    TestConfigurator::writeRandomReferenceSequence(dataTokenCount);

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();

    delete[] pLiteralLengthTable;
    delete[] pDistanceLengthTable;
    delete[] code_length_table_ptr;
    
    return GEN_OK;
}
