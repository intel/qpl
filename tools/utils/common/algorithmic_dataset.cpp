/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tools
 */

#include "algorithmic_dataset.hpp"

namespace qpl::tools {
// List of files we don't want to appear in algorithmic testing
const std::string excluded_files[] = {"bib_eobs_bfinal", "gen_all_ll.bin", "gen_all_ll.def", "gen_large_dist.bin",
                                      "gen_large_dist.def"};

algorithmic_dataset_t::algorithmic_dataset_t(const std::string& path) : dataset_t(path) {
    // Erase all files we don't want to be in algorithmic dataset
    for (auto& value : excluded_files) {
        data_.erase(value);
    }
}
} // namespace qpl::tools
