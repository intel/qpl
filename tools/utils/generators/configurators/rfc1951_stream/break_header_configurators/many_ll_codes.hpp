/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef _CONFIGURATOR_MANY_LITERAL_LENGTH_CODES_HPP_
#define _CONFIGURATOR_MANY_LITERAL_LENGTH_CODES_HPP_

#include "base_configurator.hpp"

namespace gz_generator {
class ManyLiteralLengthCodesConfigurator : public TestConfigurator {
public:
    ManyLiteralLengthCodesConfigurator(Gen32u seed) : TestConfigurator(seed) {}

    ManyLiteralLengthCodesConfigurator() = delete;

    GenStatus generate() override;
};
} // namespace gz_generator

#endif //_MANY_DISTANCE_CODES_HPP_
