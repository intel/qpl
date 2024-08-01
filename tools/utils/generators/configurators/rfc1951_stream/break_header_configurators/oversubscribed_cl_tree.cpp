/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "oversubscribed_cl_tree.hpp"

GenStatus gz_generator::OversubscribedCodeLengthsTreeConfigurator::generate() {
    Gen32u code_length = 0U;

    qpl::test::random randomCodeLength(0U, DEFAULT_CL_TABLE_LENGTH - 1U, m_seed);

    std::vector<Gen32u> codeLengthsTable(DEFAULT_CL_TABLE_LENGTH, 0U);

    makeRandomLengthCodesTable(codeLengthsTable.data(), DEFAULT_CL_TABLE_LENGTH, MAX_CL_CODE_BIT_LENGTH);

    TestConfigurator::declareDynamicBlock();
    TestConfigurator::declareVectorToken(CL_VECTOR_ALT, codeLengthsTable.data(), DEFAULT_CL_TABLE_LENGTH);

    do { //NOLINT(cppcoreguidelines-avoid-do-while)
        code_length = static_cast<Gen32u>(randomCodeLength);
    } while (codeLengthsTable[code_length] == 1U);
    codeLengthsTable[code_length]--;

    TestConfigurator::declareVectorToken(CL_ENCODED_VECTOR, codeLengthsTable.data(), DEFAULT_CL_TABLE_LENGTH);

    writeRandomHuffmanBlock();
    declareFinishBlock();

    return GEN_OK;
}