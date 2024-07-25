/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tools
 */

#ifndef QPL_TESTS_UTILS_COMMON_DATASET_HPP_
#define QPL_TESTS_UTILS_COMMON_DATASET_HPP_

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace qpl::tools {

class dataset_t {
    using dataset_data_t = std::unordered_map<std::string, std::vector<uint8_t>>;

public:
    explicit dataset_t(const std::string& path);

    [[nodiscard]] auto operator[](const std::string& key) const -> const std::vector<uint8_t>&;

    friend std::ostream& operator<<(std::ostream& out, const dataset_t& dataset);

    [[nodiscard]] size_t size() const noexcept;

    [[nodiscard]] auto get_data() const -> const dataset_data_t&;

protected:
    dataset_data_t data_ {};
    std::string    path_;
};
} // namespace qpl::tools

#endif //QPL_TESTS_UTILS_COMMON_DATASET_HPP_
