/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef __GZ_GENERATOR_H__
#define __GZ_GENERATOR_H__

#include <memory>
#include <vector>

#include "gendefs.hpp"

namespace gz_generator {
class InflateGenerator {
protected:
    static std::unique_ptr<std::stringstream> getStreamDescription(TestFactor& testFactor);

public:
    static GenStatus generate(std::vector<Gen8u>& pBinaryData, std::vector<Gen8u>& pReferenceData, TestFactor& factor);
};
} // namespace gz_generator
#endif //__GZ_GENERATOR_H__