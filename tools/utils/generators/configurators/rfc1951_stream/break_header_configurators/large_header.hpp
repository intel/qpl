/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef _CONFIGURATOR_LARGE_HEADER_HPP_
#define _CONFIGURATOR_LARGE_HEADER_HPP_

#include "base_configurator.hpp"

namespace gz_generator {
class LargeHeaderConfigurator : public TestConfigurator {
public:
    LargeHeaderConfigurator(Gen32u seed) : TestConfigurator(seed) {}

    LargeHeaderConfigurator() = delete;

    GenStatus generate() override;

private:
    inline int caclulateNumberLiteralLengthsCodesEqualTo13(Gen32u headerHighBitBorder, Gen32u header_bit_size) {
        return (int)((headerHighBitBorder - header_bit_size) / 2U);
    }

    GenStatus writeLiteralLengthsCodesForSmallHeader(Gen32u header_bit_size, Gen32u* pPossibleLiteralCount);
    GenStatus writeLiteralLengthCodes(Gen32u header_bit_size, Gen32u* pPossibleLiteralCount);
    GenStatus writeBlock(Gen32u blockByteSize);
};
} // namespace gz_generator

#endif //_CONFIGURATOR_LARGE_HEADER_HPP_
