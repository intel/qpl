/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include <stdexcept>
#include <ops/dispatcher.hpp>

namespace bench::ops
{
template <api_e api, path_e path>
using deflate_t   = typename api_dispatcher_t<api, path, operation_e::deflate>::impl_t;
template <api_e api, path_e path>
using inflate_t = typename api_dispatcher_t<api, path, operation_e::inflate>::impl_t;
}
