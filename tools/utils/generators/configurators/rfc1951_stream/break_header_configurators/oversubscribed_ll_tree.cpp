/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "oversubscribed_ll_tree.hpp"

GenStatus gz_generator::OversubscribedLiteralLengthsTreeConfigurator::generate() {
    //    Gen32u* pLiteralLengthsTable = nullptr;
    Gen32u literalLength = 0U;

    qpl::test::random randomLiteralLength(0U, DEFAULT_LL_TABLE_LENGTH - 1U, m_seed);

    std::vector<Gen32u> literalLengthsTable(DEFAULT_LL_TABLE_LENGTH, 0U);

    TestConfigurator::makeRandomLengthCodesTable(literalLengthsTable.data(), DEFAULT_LL_TABLE_LENGTH,
                                                 MAX_LL_CODE_BIT_LENGTH);

    TestConfigurator::declareDynamicBlock();
    TestConfigurator::declareVectorToken(LL_VECTOR, literalLengthsTable.data(), DEFAULT_LL_TABLE_LENGTH);

    do { //NOLINT(cppcoreguidelines-avoid-do-while)
        literalLength = static_cast<Gen32u>(randomLiteralLength);
    } while (literalLengthsTable[literalLength] == 1U);
    literalLengthsTable[literalLength]--;

    TestConfigurator::declareVectorToken(LL_ENCODED_VECTOR, literalLengthsTable.data(), DEFAULT_LL_TABLE_LENGTH);

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();

    return GEN_OK;
}
