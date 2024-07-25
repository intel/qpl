/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_PROJECT_CONFIGURATOR_BIG_REPEAT_COUNT_D_CODES_HPP
#define QPL_PROJECT_CONFIGURATOR_BIG_REPEAT_COUNT_D_CODES_HPP

#include "base_configurator.hpp"

namespace gz_generator {
class BigRepeatCountDistanceLengthCodesConfigurator : public TestConfigurator {
public:
    BigRepeatCountDistanceLengthCodesConfigurator(Gen32u seed) : TestConfigurator(seed) {}

    BigRepeatCountDistanceLengthCodesConfigurator() = delete;

    GenStatus generate() override;
};
} // namespace gz_generator

#endif //QPL_PROJECT_CONFIGURATOR_BIG_REPEAT_COUNT_D_CODES_HPP
