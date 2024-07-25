/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_PROJECT_CONFIGURATOR_CL_CODES_SPAN_SINGLE_TABLE_HPP
#define QPL_PROJECT_CONFIGURATOR_CL_CODES_SPAN_SINGLE_TABLE_HPP

#include "base_configurator.hpp"

namespace gz_generator {
class StradingCodeLengthCodesConfigurator : public TestConfigurator {
public:
    StradingCodeLengthCodesConfigurator(Gen32u seed) : TestConfigurator(seed) {}

    StradingCodeLengthCodesConfigurator() = delete;

    GenStatus generate() override;
};
} // namespace gz_generator

#endif //QPL_PROJECT_CONFIGURATOR_CL_CODES_SPAN_SINGLE_TABLE_HPP
