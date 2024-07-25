/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "base_configurator.hpp"

#ifndef __CONFIGURATOR_FACTORY_H__
#define __CONFIGURATOR_FACTORY_H__

namespace gz_generator {
class InflateTestConfiguratorFactory {
public:
    GenStatus createConfigurator(ITestConfigurator*& pConfigurator, TestFactor& factor);
};

class IndexTestConfiguratorFactory {
public:
    GenStatus createConfigurator(ITestConfigurator*& pConfigurator, TestFactor& factor);
};

} // namespace gz_generator
#endif //__CONFIGURATOR_FACTORY_H__
