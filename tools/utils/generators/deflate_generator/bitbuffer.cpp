/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "bitbuffer.h"

gz_generator::BitBuffer::BitBuffer(std::vector<uint8_t>* data)
    : m_totalBitWritten(0U), m_bigEndian16(false), m_data(data) {
    m_buffer = {0U, 0U};
}

void gz_generator::BitBuffer::write(uint64_t value, uint32_t bit_count) {
    uint32_t bitToProcess = 0U;
    m_totalBitWritten += bit_count;

    bitToProcess = m_buffer.bit_length;
    value <<= bitToProcess;
    m_buffer.value |= value;
    bitToProcess += bit_count;

    if (m_bigEndian16) {
        while (bitToProcess >= 16U) {
            uint16_t word = 0U;
            word          = (uint16_t)(m_buffer.value);
            word          = bitFlip(word);
            m_data->push_back((char)word);
            m_data->push_back((char)(word >> 8U));
            m_buffer.value >>= 16U;
            bitToProcess -= 16U;
        }
    } else {
        while (bitToProcess >= 8U) {
            m_data->push_back((char)(m_buffer.value));
            m_buffer.value >>= 8U;
            bitToProcess -= 8U;
        }
    }
    m_buffer.bit_length = bitToProcess;
}

void gz_generator::BitBuffer::flush(uint32_t pad) {
    uint32_t code_length = 0U;

    if (m_bigEndian16) {
        code_length = (0U - m_buffer.bit_length) & 15U;
    } else {
        code_length = (0U - m_buffer.bit_length) & 7U;
    }
    const uint32_t code = pad & ((1U << code_length) - 1U);

    write(code, code_length);
}

void gz_generator::BitBuffer::padToByte() {
    if (m_bigEndian16) {
        std::cerr << "BitBuffer::padToByte is not supported with BE16\n";
        throw std::exception();
    }

    if (m_buffer.bit_length != 0U) {
        assert(m_buffer.bit_length < 8U);
        write(0U, 8U - m_buffer.bit_length);
    }
}

uint32_t gz_generator::BitBuffer::getBitsWritten() const {
    return m_totalBitWritten;
}

uint16_t gz_generator::BitBuffer::bitFlip(uint16_t word) {
    word = (word >> 8U) | (word << 8U);
    word = ((word >> 4U) & 0x0F0FU) | ((word << 4U) & 0xF0F0U);
    word = ((word >> 2U) & 0x3333U) | ((word << 2U) & 0xCCCCU);
    word = ((word >> 1U) & 0x5555) | ((word << 1U) & 0xAAAAU);
    return word;
}

void gz_generator::BitBuffer::setBigEndian16(bool value) {
    m_bigEndian16 = value;
}
