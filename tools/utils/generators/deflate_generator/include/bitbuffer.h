/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef __BITBUF2_H__
#define __BITBUF2_H__

#include <assert.h>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <vector>

namespace gz_generator {
class BitBuffer {
public:
    explicit BitBuffer(std::vector<uint8_t>* data);

    BitBuffer() = delete;

    BitBuffer(const BitBuffer& buffer) = delete;

    void flush(uint32_t pad = 0);

    void write(uint64_t value, uint32_t bit_count);

    void padToByte();

    static uint16_t bitFlip(uint16_t word);

    BitBuffer& operator=(const BitBuffer& buffer) = delete;

    uint32_t getBitsWritten() const;

    void setBigEndian16(bool value);

private:
    bool     m_bigEndian16;
    uint32_t m_totalBitWritten;
    struct {
        uint64_t value;
        uint32_t bit_length;
    } m_buffer;

    std::vector<uint8_t>* m_data;
};
} // namespace gz_generator

#endif // __BITBUF2_H__
