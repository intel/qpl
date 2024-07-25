/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "dynamic_block_no_err.hpp"

GenStatus gz_generator::DynamicBlockNoErrorConfigurator::generate() {
    TestConfigurator::declareDynamicBlock();
    m_randomTokenCount.set_range(5000, 10000);
    TestConfigurator::writeRandomReferenceSequence(static_cast<Gen8u>(m_randomTokenCount));
    TestConfigurator::declareFinishBlock();

    return GEN_OK;
}
