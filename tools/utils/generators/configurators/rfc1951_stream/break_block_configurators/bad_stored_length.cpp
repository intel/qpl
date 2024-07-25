/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "bad_stored_length.hpp"

GenStatus gz_generator::BadStoredLengthConfigurator::generate() {
    Gen32u            testGroup = 0U;
    qpl::test::random randomTestGroup;

    TestConfigurator::declareStoredBlock();

    if (static_cast<float>(m_random) < 0.67F) {
        m_randomTokenCount.set_range(0U, 32U);
    } else {
        m_randomTokenCount.set_range(0U, 0xFFFF);
    }

    TestConfigurator::writeRandomLiteralSequence((Gen32u)m_randomTokenCount);

    if (static_cast<float>(m_random) < 0.5F) {
        randomTestGroup.set_range(0U, 15U);
        testGroup = 1U << static_cast<Gen32u>(randomTestGroup);
    } else {
        randomTestGroup.set_range(0U, 0xFFFFU);
        testGroup = static_cast<Gen32u>(randomTestGroup);
    }

    TestConfigurator::declareTestToken(8U, testGroup);

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();

    return GEN_OK;
}