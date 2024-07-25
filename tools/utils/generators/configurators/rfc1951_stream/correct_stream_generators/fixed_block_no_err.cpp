/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "fixed_block_no_err.hpp"

GenStatus gz_generator::FixedBlockNoErrorConfigurator::generate() {
    TestConfigurator::declareFixedBlock();
    m_randomTokenCount.set_range(5000, 10000);
    TestConfigurator::writeRandomReferenceSequence(static_cast<Gen8u>(m_randomTokenCount));
    TestConfigurator::declareFinishBlock();

    return GEN_OK;
}