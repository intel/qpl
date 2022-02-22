/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "distance_before_start.hpp"

GenStatus gz_generator::DistanceCodeBeforeStartConfigurator::generate()
{
    Gen32u offset = 0;
    Gen32u match = 0;
    Gen32u literalsEncoded = 0;

    m_randomTokenCount.set_range(0u, 32u);
    TestConfigurator::declareRandomHuffmanBlock();

    literalsEncoded = TestConfigurator::writeRandomReferenceSequence(static_cast<Gen32u> (m_randomTokenCount),
                                                                     0u,
                                                                     3800u);
    if (0.5f > static_cast<float>(m_random))
    {
        m_randomOffset.set_range(1u, 16u);
        offset = literalsEncoded + static_cast<Gen32u>(m_randomOffset);
    }
    else
    {
        m_randomOffset.set_range(literalsEncoded + 1u, 4096u);
        offset = static_cast<Gen32u>(m_randomOffset);
    }

    m_randomMatch.set_range(MIN_MATCH, GEN_MIN(literalsEncoded, MAX_MATCH));
    match = static_cast<Gen32u>(m_randomMatch);

    TestConfigurator::declareReference(match, offset);

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();

    return GEN_OK;
}
