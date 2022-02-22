/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "common.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * Data, size_t Size) {
    extract_test_case<qpl::little_endian_packed_array>(Data, Size);

    return 0;
}
