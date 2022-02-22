/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "unallowable_d_code.hpp"

GenStatus gz_generator::UnallowableDistanceCodeConfigurator::generate()
{
    Gen32u match            = 0u;
    Gen32u offset           = 0u;
    Gen32u literalsEncoded  = 0u;

    m_randomTokenCount.set_range(1u, 50u);

    TestConfigurator::declareFixedBlock();
    if (0.9f > static_cast<float> (m_random))
    {
        literalsEncoded = TestConfigurator::writeRandomReferenceSequence(static_cast<Gen32u> (m_randomTokenCount));
    }
    m_randomMatch.set_range(MIN_MATCH, GEN_MIN(GEN_MAX(literalsEncoded, MIN_MATCH), MAX_MATCH));
    m_randomOffset.set_range(30u, 31u);

    match = static_cast<Gen32u>(m_randomMatch);
    offset = DEFAULT_MAX_OFFSET + static_cast<Gen32u>(m_randomOffset);
    literalsEncoded += match;

    TestConfigurator::declareReference(match, offset);

    if(0.9f > static_cast<float> (m_random))
    {
        TestConfigurator::writeRandomReferenceSequence(static_cast<Gen32u> (m_randomTokenCount), literalsEncoded);
    }

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();

    return GEN_OK;
}