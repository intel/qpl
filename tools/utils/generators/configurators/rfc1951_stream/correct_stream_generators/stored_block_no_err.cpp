/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "stored_block_no_err.hpp"

GenStatus gz_generator::StoredBlockNoErrorConfigurator::generate() {
    TestConfigurator::writeRandomStoredBlock();
    TestConfigurator::declareFinishBlock();

    return GEN_OK;
}