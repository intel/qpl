/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "unallowable_d_code.hpp"

GenStatus gz_generator::UnallowableDistanceCodeConfigurator::generate() {
    Gen32u match           = 0U;
    Gen32u offset          = 0U;
    Gen32u literalsEncoded = 0U;

    m_randomTokenCount.set_range(1U, 50U);

    TestConfigurator::declareFixedBlock();
    if (0.9F > static_cast<float>(m_random)) {
        literalsEncoded = TestConfigurator::writeRandomReferenceSequence(static_cast<Gen32u>(m_randomTokenCount));
    }
    m_randomMatch.set_range(MIN_MATCH, GEN_MIN(GEN_MAX(literalsEncoded, MIN_MATCH), MAX_MATCH));
    m_randomOffset.set_range(30U, 31U);

    match  = static_cast<Gen32u>(m_randomMatch);
    offset = DEFAULT_MAX_OFFSET + static_cast<Gen32u>(m_randomOffset);
    literalsEncoded += match;

    TestConfigurator::declareReference(match, offset);

    if (0.9F > static_cast<float>(m_random)) {
        TestConfigurator::writeRandomReferenceSequence(static_cast<Gen32u>(m_randomTokenCount), literalsEncoded);
    }

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();

    return GEN_OK;
}