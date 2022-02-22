/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/cpp_api/util/exceptions.hpp"

namespace qpl {

exception::exception(const char *message) noexcept
        : message_(message) {
    // Empty constructor
}

[[nodiscard]] auto exception::what() const noexcept -> const char * {
    return message_;
}

invalid_argument_exception::invalid_argument_exception(const char *message) noexcept
        : exception(message) {
    // Empty constructor
}

operation_process_exception::operation_process_exception(const char *message) noexcept
        : exception(message) {
    // Empty constructor
}

memory_overflow_exception::memory_overflow_exception(const char *message) noexcept
        : invalid_argument_exception(message) {
    // Empty constructor
}

memory_underflow_exception::memory_underflow_exception(const char *message) noexcept
        : operation_process_exception(message) {
    // Empty constructor
}

invalid_data_exception::invalid_data_exception(const char *message) noexcept
        : operation_process_exception(message) {
    // Empty constructor
}

invalid_compression_parameter_exception::invalid_compression_parameter_exception(const char *message) noexcept
        : operation_process_exception(message) {
    // Empty constructor
}

short_destination_exception::short_destination_exception(const char *message) noexcept
        : memory_underflow_exception(message) {
    // Empty constructor
}

} // namespace qpl
