/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tools
 */

#include "dataset.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace qpl::tools {
dataset_t::dataset_t(const std::string& path) {
    auto real_path = canonical(absolute(std::filesystem::path(path.c_str())));
    path_          = real_path.string();

    for (const auto& entry : std::filesystem::directory_iterator(real_path)) {
        if (entry.is_symlink()) {
            std::cerr << "Skip this file because it is a symlink. Path=" << path_ << "/"
                      << entry.path().filename().string() << '\n';
        } else {
            std::ifstream file(entry.path(), std::ios::in | std::ios::binary);
            auto data = std::vector<uint8_t>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

            data_.insert(std::make_pair(entry.path().filename().string(), std::move(data)));
        }
    }
}

auto dataset_t::operator[](const std::string& file) const -> const std::vector<uint8_t>& {
    auto value = data_.find(file);

    if (value == data_.end()) { throw std::runtime_error("Couldn't open input file. Path=" + path_ + "/" + file); }

    return value->second;
}

auto dataset_t::get_data() const -> const dataset_data_t& {
    return this->data_;
}

std::ostream& operator<<(std::ostream& out, const dataset_t& dataset) {
    out << "Used dataset: " << dataset.path_ << '\n';

    for (auto& file : dataset.data_) {
        out << "--- " << file.first << '\n';
    }

    return out;
}

size_t dataset_t::size() const noexcept {
    return data_.size();
}
} // namespace qpl::tools
