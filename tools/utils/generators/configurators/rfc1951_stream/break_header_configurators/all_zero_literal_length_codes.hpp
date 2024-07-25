/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_PROJECT_CONFIGURATOR_ALL_ZERO_LITERAL_LENGTH_CODE_HPP
#define QPL_PROJECT_CONFIGURATOR_ALL_ZERO_LITERAL_LENGTH_CODE_HPP

#include "base_configurator.hpp"

namespace gz_generator {
class AllZeroLiteralLengthCodesConfigurator : public TestConfigurator {
    // Not a seed dependent test, but here for consistency with other tests
public:
    AllZeroLiteralLengthCodesConfigurator(Gen32u seed) : TestConfigurator(seed) {}

    AllZeroLiteralLengthCodesConfigurator() = delete;

    GenStatus generate() override;
};

} // namespace gz_generator

#endif //QPL_PROJECT_CONFIGURATOR_ALL_ZERO_LITERAL_LENGTH_CODE_HPP
