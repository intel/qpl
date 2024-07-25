/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tools
 */

#ifndef QPL_ALGORITHMIC_DATASET_HPP_
#define QPL_ALGORITHMIC_DATASET_HPP_

#include "dataset.hpp"

namespace qpl::tools {
class algorithmic_dataset_t : public dataset_t {
public:
    explicit algorithmic_dataset_t(const std::string& path);
};
} // namespace qpl::tools
#endif //QPL_ALGORITHMIC_DATASET_H
