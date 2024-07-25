/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_PROJECT_CONFIGURATOR_BIG_REPEAT_COUNT_LL_CODES_H
#define QPL_PROJECT_CONFIGURATOR_BIG_REPEAT_COUNT_LL_CODES_H

#include "base_configurator.hpp"

namespace gz_generator {
class BigRepeatCountLiteralLengthCodesConfigurator : public TestConfigurator {
public:
    BigRepeatCountLiteralLengthCodesConfigurator(Gen32u seed) : TestConfigurator(seed) {}

    BigRepeatCountLiteralLengthCodesConfigurator() = delete;

    GenStatus generate() override;
};
} // namespace gz_generator

#endif //QPL_PROJECT_CONFIGURATOR_BIG_REPEAT_COUNT_LL_CODES_H
