/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "bad_distance.hpp"

GenStatus gz_generator::BadDistanceConfigurator::generate()
{
    Gen32u offset           = 0u;
    Gen32u match            = 0u;
    Gen32u literalsEncoded  = 0u;

    declareRandomHuffmanBlock();

    literalsEncoded = TestConfigurator::writeRandomReferenceSequence(DEFAULT_MAX_OFFSET + 1u,
                                                                     literalsEncoded,
                                                                     DEFAULT_MAX_OFFSET + 1u);

    if( 0.5f > static_cast<float>(m_random))
    {
        m_randomOffset.set_range(1u, 16u);
        offset = MAX_OFFSET + static_cast<Gen32u>(m_randomOffset);
    }
    else
    {
        m_randomOffset.set_range(MAX_OFFSET + 1u, DEFAULT_MAX_OFFSET);
        offset = static_cast<Gen32u>(m_randomOffset);
    }

    m_randomMatch.set_range(MIN_MATCH, GEN_MIN(literalsEncoded, MAX_MATCH));
    match = static_cast<Gen32u>(m_randomMatch);

    TestConfigurator::declareReference(match, offset);

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();

    return GEN_OK;
}
