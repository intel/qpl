/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "deflate_fuzz_test.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * Data, size_t Size) {
    return deflate_fuzz(Data, Size, qpl::fixed_mode, qpl::default_level);
}
