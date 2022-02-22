/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef __GZ_GENERATOR_H__
#define __GZ_GENERATOR_H__

#include "gendefs.hpp"
#include <memory>

namespace gz_generator
{
    class InflateGenerator
    {
        protected:
            std::unique_ptr<std::stringstream> getStreamDescription(TestFactor &testFactor);

        public:

            GenStatus
            generate(std::vector<Gen8u> &pBinaryData, std::vector<Gen8u> &pReferenceData, TestFactor &factor);
    };

    class IndexGenerator
    {
        protected:
            std::unique_ptr<std::stringstream> getStreamDescription(TestFactor &testFactor);

        public:
            GenStatus
            generate(std::vector<Gen8u> &pBinaryData, std::vector<Gen8u> &pReferenceData, TestFactor &factor);
    };
}
#endif //__GZ_GENERATOR_H__