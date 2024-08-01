/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "oversubscribed_d_tree.hpp"

GenStatus gz_generator::OversubscribedDistanceTreeConfigurator::generate() {
    Gen32u distanceLengthCode = 0U;

    qpl::test::random randomDistanceLengthCode(0U, 29U, m_seed);

    std::vector<Gen32u> distanceLengthsTable(DEFAULT_D_TABLE_LENGTH, 0U);

    TestConfigurator::makeRandomLengthCodesTable(distanceLengthsTable.data(), DEFAULT_D_TABLE_LENGTH,
                                                 MAX_D_CODE_BIT_LENGTH);

    TestConfigurator::declareDynamicBlock();
    TestConfigurator::declareVectorToken(D_VECTOR, distanceLengthsTable.data(), DEFAULT_D_TABLE_LENGTH);

    do { //NOLINT(cppcoreguidelines-avoid-do-while)
        distanceLengthCode = static_cast<Gen32u>(randomDistanceLengthCode);
    } while (distanceLengthsTable[distanceLengthCode] == 1U);
    distanceLengthsTable[distanceLengthCode]--;

    TestConfigurator::declareVectorToken(D_ENCODED_VECTOR, distanceLengthsTable.data(), DEFAULT_D_TABLE_LENGTH);

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();

    return GEN_OK;
}
