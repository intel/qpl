/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "bad_stored_length.hpp"

GenStatus gz_generator::BadStoredLengthConfigurator::generate()
{   
    Gen32u testGroup;
    qpl::test::random randomTestGroup;

    TestConfigurator::declareStoredBlock();

    if (static_cast<float>(m_random) < 0.67f)
    {
        m_randomTokenCount.set_range(0u, 32u);
    }
    else
    {
        m_randomTokenCount.set_range(0u, 0xFFFF);
    }

    TestConfigurator::writeRandomLiteralSequence((Gen32u)m_randomTokenCount);

    if (static_cast<float>(m_random) < 0.5f)
    {
        randomTestGroup.set_range(0u, 15u);
        testGroup = 1u << static_cast<Gen32u>(randomTestGroup);
    }
    else
    {
        randomTestGroup.set_range(0u, 0xFFFF);
        testGroup = static_cast<Gen32u>(randomTestGroup);
    }

    TestConfigurator::declareTestToken(8u, testGroup);

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();

    return GEN_OK;
}