/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <iostream>
#include <string>
#include <vector>

#include "qpl/cpp_api/operations/other/copy_operation.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    std::vector<uint8_t> destination(Size, 4);

    // Performing an operation
    auto copy_operation = qpl::copy_operation();

    auto result = qpl::execute(copy_operation,
                               Data,
                               Data + Size,
                               destination.begin(),
                               destination.end());

    // Handle extract result
    result.handle([&Size](uint32_t copy_size) -> void {
                      // Check if everything was alright
                      if (copy_size != Size) {
                          throw std::runtime_error("Incorrect value was chosen while operation performing.");
                      }
                  },
                  [](uint32_t status) -> void {
                  });

    return 0;
}
