/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "bad_distance.hpp"

GenStatus gz_generator::BadDistanceConfigurator::generate() {
    Gen32u offset          = 0U;
    Gen32u match           = 0U;
    Gen32u literalsEncoded = 0U;

    declareRandomHuffmanBlock();

    literalsEncoded = TestConfigurator::writeRandomReferenceSequence(DEFAULT_MAX_OFFSET + 1U, literalsEncoded,
                                                                     DEFAULT_MAX_OFFSET + 1U);

    if (0.5F > static_cast<float>(m_random)) {
        m_randomOffset.set_range(1U, 16U);
        offset = MAX_OFFSET + static_cast<Gen32u>(m_randomOffset);
    } else {
        m_randomOffset.set_range(MAX_OFFSET + 1U, DEFAULT_MAX_OFFSET);
        offset = static_cast<Gen32u>(m_randomOffset);
    }

    m_randomMatch.set_range(MIN_MATCH, GEN_MIN(literalsEncoded, MAX_MATCH));
    match = static_cast<Gen32u>(m_randomMatch);

    TestConfigurator::declareReference(match, offset);

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();

    return GEN_OK;
}
