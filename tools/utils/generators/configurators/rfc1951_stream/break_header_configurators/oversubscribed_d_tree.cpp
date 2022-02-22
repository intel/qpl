/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "oversubscribed_d_tree.hpp"

GenStatus gz_generator::OversubscribedDistanceTreeConfigurator::generate()
{
    Gen32u  distanceLengthCode;

    qpl::test::random randomDistanceLengthCode(0u, 29u, m_seed);

    std::vector<Gen32u> distanceLengthsTable(DEFAULT_D_TABLE_LENGTH, 0);

    TestConfigurator::makeRandomLengthCodesTable(distanceLengthsTable.data(),
                                                 DEFAULT_D_TABLE_LENGTH,
                                                 MAX_D_CODE_BIT_LENGTH);

    TestConfigurator::declareDynamicBlock();
    TestConfigurator::declareVectorToken(D_VECTOR, distanceLengthsTable.data(), DEFAULT_D_TABLE_LENGTH);

    do {
        distanceLengthCode = static_cast<Gen32u>(randomDistanceLengthCode);
    }
    while (distanceLengthsTable[distanceLengthCode] == 1u);
    distanceLengthsTable[distanceLengthCode]--;

    TestConfigurator::declareVectorToken(D_ENCODED_VECTOR, distanceLengthsTable.data(), DEFAULT_D_TABLE_LENGTH);

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();

    return GEN_OK;

}

