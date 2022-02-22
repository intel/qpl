/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "oversubscribed_cl_tree.hpp"

GenStatus gz_generator::OversubscribedCodeLengthsTreeConfigurator::generate()
{
    Gen32u  code_length;

    qpl::test::random randomCodeLength(0u, DEFAULT_CL_TABLE_LENGTH - 1u, m_seed);

    std::vector<Gen32u> codeLengthsTable(DEFAULT_CL_TABLE_LENGTH, 0);

    makeRandomLengthCodesTable(codeLengthsTable.data(), DEFAULT_CL_TABLE_LENGTH, MAX_CL_CODE_BIT_LENGTH);

    TestConfigurator::declareDynamicBlock();
    TestConfigurator::declareVectorToken(CL_VECTOR_ALT, codeLengthsTable.data(), DEFAULT_CL_TABLE_LENGTH);

    do {
        code_length = static_cast<Gen32u>(randomCodeLength);
    }
    while (codeLengthsTable[code_length] == 1u);
    codeLengthsTable[code_length]--;

    TestConfigurator::declareVectorToken(CL_ENCODED_VECTOR, codeLengthsTable.data(), DEFAULT_CL_TABLE_LENGTH);

    writeRandomHuffmanBlock();
    declareFinishBlock();

    return GEN_OK;
}