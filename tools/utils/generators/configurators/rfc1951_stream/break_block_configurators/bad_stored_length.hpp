/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef _CONFIGURATOR_BAD_STORED_LENGTH_HPP_
#define _CONFIGURATOR_BAD_STORED_LENGTH_HPP_

#include "base_configurator.hpp"

namespace gz_generator {
class BadStoredLengthConfigurator : public TestConfigurator {
public:
    BadStoredLengthConfigurator(Gen32u seed) : TestConfigurator(seed) {}

    BadStoredLengthConfigurator() = delete;

    GenStatus generate() override;
};
} // namespace gz_generator

#endif //_CONFIGURATOR_BAD_STORED_LENGTH_HPP_
