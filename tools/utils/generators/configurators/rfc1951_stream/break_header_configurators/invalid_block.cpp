/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "invalid_block.hpp"

GenStatus gz_generator::InvalidBlockTypeConfigurator::generate() {
    TestConfigurator::writeRandomBlock();
    TestConfigurator::declareInvalidBlock();
    TestConfigurator::writeRandomBlock();

    TestConfigurator::writeRandomHuffmanBlock();
    TestConfigurator::declareFinishBlock();

    return GEN_OK;
}