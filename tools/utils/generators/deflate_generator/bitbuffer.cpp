/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "bitbuffer.h"

gz_generator::BitBuffer::BitBuffer(std::vector<uint8_t> *data)
    : m_data(data)
{
    m_buffer = {0u,0u};
    m_totalBitWritten = 0u;
    m_bigEndian16 = false;
}

void gz_generator::BitBuffer::write(uint64_t value, uint32_t bit_count)
{
    uint32_t bitToProcess;
    m_totalBitWritten += bit_count;

    bitToProcess =  m_buffer.bit_length;
    value <<= bitToProcess;
    m_buffer.value |= value;
    bitToProcess += bit_count;

    if (m_bigEndian16) {
        while (bitToProcess >= 16u)
        {
            uint16_t word;
            word = (uint16_t)(m_buffer.value);
            word = bitFlip(word);
            m_data->push_back((char)word);
            m_data->push_back((char)(word >> 8u));
            m_buffer.value >>= 16u;
            bitToProcess -= 16u;
        }
    }
    else
    {
        while (bitToProcess >= 8u)
        {
            m_data->push_back((char)(m_buffer.value));
            m_buffer.value >>= 8u;
            bitToProcess -= 8u;
        }
    }
    m_buffer.bit_length = bitToProcess;
}

void gz_generator::BitBuffer::flush(uint32_t pad)
{
    uint32_t code;
    uint32_t code_length;

    if (m_bigEndian16)
    {
        code_length = (0u - m_buffer.bit_length) & 15u;
    }
    else
    {
        code_length = (0u - m_buffer.bit_length) & 7u;
    }
    code = pad & ((1u << code_length) - 1u);

    write(code, code_length);
}

void gz_generator::BitBuffer::padToByte()
{
    if (m_bigEndian16)
    {
        std::cerr << "BitBuffer::padToByte is not supported with BE16" << std::endl;
        throw std::exception();
    }

    if (m_buffer.bit_length != 0u)
    {
        assert(m_buffer.bit_length < 8u);
        write(0u, 8u - m_buffer.bit_length);
    }
}

uint32_t gz_generator::BitBuffer::getBitsWritten()
{
    return m_totalBitWritten;
}

uint16_t gz_generator::BitBuffer::bitFlip(uint16_t word)
{
    word = (word >> 8u) | (word << 8u);
    word = ((word >> 4u) & 0x0F0Fu) | ((word << 4u) & 0xF0F0u);
    word = ((word >> 2u) & 0x3333u) | ((word << 2u) & 0xCCCCu);
    word = ((word >> 1u) & 0x5555) | ((word << 1u) & 0xAAAAu);
    return word;
}

void gz_generator::BitBuffer::setBigEndian16(bool value)
{
    void setBigEndian16(bool value);
}
