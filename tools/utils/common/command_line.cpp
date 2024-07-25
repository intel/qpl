/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Test
 */

#include "command_line.hpp"

#include <algorithm>
#include <charconv>
#include <stdexcept>

namespace qpl::test {

CommandLine::CommandLine(char** arguments_pptr, int arguments_count) {
    for (int i = 1; i < arguments_count; i++) {
        std::string current_argument = arguments_pptr[i];

        auto equal_sign_it = std::find(current_argument.begin(), current_argument.end(), '=');

        // Check if "=" is the last or first symbol
        if ((current_argument.end() == equal_sign_it || '=' == current_argument.back()) ||
            current_argument.begin() == equal_sign_it) {
            throw std::runtime_error("Invalid command format");
        }

        // Set argument key
        const std::string argument_key = std::string(current_argument.begin(), equal_sign_it);
        std::string       argument_value;

        if (current_argument.end() == equal_sign_it) {
            argument_value = ""; // Such argument has no value
        } else {
            argument_value = std::string(equal_sign_it + 1, current_argument.end());
        }

        parsed_arguments_.insert(std::make_pair(argument_key, convertType(argument_value)));
    }
}

CommandLine::argument_t CommandLine::convertType(const std::string& parsed_argument) {
    int iValue = 0U;

    auto result = std::from_chars(parsed_argument.c_str(), parsed_argument.c_str() + parsed_argument.length(), iValue);

    if (result.ec != std::errc::invalid_argument) { return iValue; }

    if (parsed_argument == "on") {
        return true;
    } else if (parsed_argument == "off") {
        return false;
    }

    return parsed_argument;
}

template <class type>
auto CommandLine::find(const std::string& parameter_name) -> std::optional<type> {
    auto pair = parsed_arguments_.find(parameter_name);
    if constexpr (std::is_same_v<type, uint32_t>) {
        if (pair != parsed_arguments_.end() && std::holds_alternative<int32_t>(pair->second)) {
            return std::get<int32_t>(pair->second);
        }
    } else {
        if (pair != parsed_arguments_.end() && std::holds_alternative<type>(pair->second)) {
            return std::get<type>(pair->second);
        }
    }

    return std::nullopt;
}

template auto CommandLine::find<bool>(const std::string& parameter_name) -> std::optional<bool>;
template auto CommandLine::find<int32_t>(const std::string& parameter_name) -> std::optional<int32_t>;
template auto CommandLine::find<uint32_t>(const std::string& parameter_name) -> std::optional<uint32_t>;
template auto CommandLine::find<std::string>(const std::string& parameter_name) -> std::optional<std::string>;

} // namespace qpl::test
