/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "oversubscribed_ll_tree.hpp"

GenStatus gz_generator::OversubscribedLiteralLengthsTreeConfigurator::generate()
{
//    Gen32u* pLiteralLengthsTable = nullptr;
    Gen32u  literalLength;

    qpl::test::random randomLiteralLength(0u, DEFAULT_LL_TABLE_LENGTH - 1u, m_seed);

    std::vector<Gen32u> literalLengthsTable(DEFAULT_LL_TABLE_LENGTH, 0);

    TestConfigurator::makeRandomLengthCodesTable(literalLengthsTable.data(),
                                                 DEFAULT_LL_TABLE_LENGTH,
                                                 MAX_LL_CODE_BIT_LENGTH);

    TestConfigurator::declareDynamicBlock();
    TestConfigurator::declareVectorToken(LL_VECTOR, literalLengthsTable.data(), DEFAULT_LL_TABLE_LENGTH);

    do {
        literalLength = static_cast<Gen32u>(randomLiteralLength);
    }
    while (literalLengthsTable[literalLength] == 1u);
    literalLengthsTable[literalLength]--;

    TestConfigurator::declareVectorToken(LL_ENCODED_VECTOR, literalLengthsTable.data(), DEFAULT_LL_TABLE_LENGTH);

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();

    return GEN_OK;
}



