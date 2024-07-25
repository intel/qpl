/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "distance_before_start.hpp"

GenStatus gz_generator::DistanceCodeBeforeStartConfigurator::generate() {
    Gen32u offset          = 0;
    Gen32u match           = 0;
    Gen32u literalsEncoded = 0;

    m_randomTokenCount.set_range(0U, 32U);
    TestConfigurator::declareRandomHuffmanBlock();

    literalsEncoded =
            TestConfigurator::writeRandomReferenceSequence(static_cast<Gen32u>(m_randomTokenCount), 0U, 3800U);
    if (0.5F > static_cast<float>(m_random)) {
        m_randomOffset.set_range(1U, 16U);
        offset = literalsEncoded + static_cast<Gen32u>(m_randomOffset);
    } else {
        m_randomOffset.set_range(literalsEncoded + 1U, 4096U);
        offset = static_cast<Gen32u>(m_randomOffset);
    }

    m_randomMatch.set_range(MIN_MATCH, GEN_MIN(literalsEncoded, MAX_MATCH));
    match = static_cast<Gen32u>(m_randomMatch);

    TestConfigurator::declareReference(match, offset);

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();

    return GEN_OK;
}
