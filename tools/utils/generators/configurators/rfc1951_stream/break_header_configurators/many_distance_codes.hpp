/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef _CONFIGURATOR_MANY_DISTANCE_CODES_HPP_
#define _CONFIGURATOR_MANY_DISTANCE_CODES_HPP_

#include "base_configurator.hpp"

namespace gz_generator {
class ManyDistanceCodesConfigurator : public TestConfigurator {
public:
    ManyDistanceCodesConfigurator(Gen32u seed) : TestConfigurator(seed) {}

    ManyDistanceCodesConfigurator() = delete;

    GenStatus generate() override;

    /**
             * return true if given POINT in run of length N is a breakpoint between sequences
             */
    static bool breakInRun(Gen32u a, Gen32u b);
};
} // namespace gz_generator

#endif //_MANY_DISTANCE_CODES_HPP_
