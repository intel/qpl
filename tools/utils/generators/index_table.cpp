/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "index_table.hpp"

#include <algorithm>
#include <cstdint>
#include <stdexcept>

/* ------ Index search index in the table ------ */

uint32_t IndexTable::findHeaderBlockIndex(uint32_t miniBlockNumber) const {
    if (miniBlockNumber >= m_miniBlockCount) throw std::invalid_argument("No such mini block in the table");

    uint32_t blockNumber = 0U;
    // █|▒|▒|▒|▒|◙|█|▒|▒|▒|▒|◙|█|▒|▒|▒|◙ - indices[0:16]
    // █ - block header
    // ▒ - mini block
    // ◙ - EOB
    // -> current_block_header_index = previous mini block count + previous count of EOB + HEADER_BLOCK

    blockNumber = getBlockNumber(miniBlockNumber);
    return blockNumber * (m_miniBlocksPerBlock + 2U);
}

uint32_t IndexTable::findMiniBlockIndex(uint32_t miniBlockNumber) const {
    if (miniBlockNumber >= m_miniBlockCount) throw std::invalid_argument("No such mini block in the table");

    uint32_t currentHeaderIndex     = 0U;
    uint32_t miniBlockNumberInBlock = 0U;

    // █|▒|▒|▒|▒|◙|█|▒|▒|▒|▒|◙|█|▒|▒|▒|◙ - indices[0:16]
    // █ - block header
    // ▒ - mini block
    // ◙ - EOB
    // -> current_mini_block_index = current_block_header_index + HEADER_BLOCK + mini block number in current block

    currentHeaderIndex     = findHeaderBlockIndex(miniBlockNumber);
    miniBlockNumberInBlock = miniBlockNumber - getBlockNumber(miniBlockNumber) * m_miniBlocksPerBlock;
    return currentHeaderIndex + 1U + miniBlockNumberInBlock;
}

/* ------ Access to elements ------ */
uint32_t IndexTable::getBlockNumber(uint32_t miniBlockNumber) const {
    return miniBlockNumber / m_miniBlocksPerBlock;
}

/* ------ Getters & Setters ------ */

void IndexTable::reset(uint32_t miniBlockCount, uint32_t miniBlocksPerBlock) {
    m_miniBlocksPerBlock = miniBlocksPerBlock;
    m_miniBlockCount     = miniBlockCount;
    m_blockCount         = (m_miniBlockCount + m_miniBlocksPerBlock - 1U) / m_miniBlocksPerBlock;
    m_indices.resize(m_blockCount * 2U + m_miniBlockCount + 1U);
    std::fill(m_indices.begin(), m_indices.end(), Index {0, 0});
}

uint32_t IndexTable::getBlockCount() const {
    return this->m_blockCount;
}

uint32_t IndexTable::getMiniBlockCount() const {
    return this->m_miniBlockCount;
}

uint32_t IndexTable::getMiniBlocksPerBlock() const {
    return this->m_miniBlocksPerBlock;
}

size_t IndexTable::size() {
    return this->m_indices.size();
}

Index* IndexTable::data() {
    return reinterpret_cast<Index*>(this->m_indices.data());
}

/* ------ IndexTable Operators ------ */
Index& IndexTable::operator[](const int index) {
    return m_indices[index];
}

bool operator==(const IndexTable& array1, const IndexTable& array2) {
    return std::equal(array1.m_indices.begin(), array1.m_indices.end(), array2.m_indices.begin());
}

bool operator!=(const IndexTable& array1, const IndexTable& array2) {
    return !(std::equal(array1.m_indices.begin(), array1.m_indices.end(), array2.m_indices.begin()));
}

/** --------------- Constructors --------------- **/

IndexTable::IndexTable(uint32_t miniBlockCount, uint32_t miniBlocksPerBlock) {
    IndexTable::reset(miniBlockCount, miniBlocksPerBlock);
}

void IndexTable::fillOffsets(std::vector<uint32_t> vector) {
    for (auto i = 0; i < m_indices.size() && i < vector.size(); i++) {
        m_indices[i].bit_offset = vector[i];
    };
}

bool operator==(const Index& index1, const Index& index2) {
    return (index1.bit_offset == index2.bit_offset) && (index1.crc == index2.crc);
}
