/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "gendefs.hpp"

#ifndef __COMMON_METHODS_H__
#define __COMMON_METHODS_H__
namespace gz_generator {
class CommonMethods {
public:
    static Gen8u  bsr_32(Gen32u number);
    static void   shuffle_32u(Gen32u* vector_ptr, Gen32u vectorLength, Gen32u seed);
    static Gen32u pick(Gen32u* vector_ptr, Gen32u vectorLength, Gen32u seed);
    static Gen32u code2Match(Gen32u code, Gen32u seed);
};
} // namespace gz_generator
#endif //__COMMON_METHODS_H__