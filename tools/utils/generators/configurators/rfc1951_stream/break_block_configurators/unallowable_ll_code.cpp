/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "unallowable_ll_code.hpp"

/**
 * @brief Generate stream where literal is more 285 and uses reserved symbols 286 and 287
 * @return
 */
GenStatus gz_generator::UnallowableLiteralLengthCodeConfigurator::generate() {
    Gen32u            literalsEncoded = 0;
    qpl::test::random randomUndefinedLiteral(286, 287, m_seed);

    m_randomTokenCount.set_range(1, 50);

    TestConfigurator::declareFixedBlock();

    if (0.9F > static_cast<float>(m_random)) {
        literalsEncoded = TestConfigurator::writeRandomReferenceSequence(static_cast<Gen32u>(m_randomTokenCount));
    }
    TestConfigurator::declareLiteral(static_cast<Gen32u>(randomUndefinedLiteral));
    literalsEncoded += 1;

    if (0.9F > static_cast<float>(m_random)) {
        TestConfigurator::writeRandomReferenceSequence(static_cast<Gen32u>(m_randomTokenCount), literalsEncoded);
    }

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();
    return GEN_OK;
}