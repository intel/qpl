/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_PROJECT_CONFIGURATOR_NO_LITERAL_LENGTH_CODE_HPP
#define QPL_PROJECT_CONFIGURATOR_NO_LITERAL_LENGTH_CODE_HPP

#include "base_configurator.hpp"

namespace gz_generator {
class NoLiteralLengthCodeConfigurator : public TestConfigurator {
public:
    NoLiteralLengthCodeConfigurator(Gen32u seed) : TestConfigurator(seed) {}

    NoLiteralLengthCodeConfigurator() = delete;

    GenStatus generate() override;
};

} // namespace gz_generator

#endif //QPL_PROJECT_CONFIGURATOR_NO_LITERAL_LENGTH_CODE_HPP
