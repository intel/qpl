/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_PROJECT_INDEX_TABLE_HPP
#define QPL_PROJECT_INDEX_TABLE_HPP

#include <cstddef>
#include <stdint.h>
#include <vector>

struct Index {
    uint32_t bit_offset;
    uint32_t crc;

    friend bool operator==(const Index& index1, const Index& index2);
};

class IndexTable {
private:
    std::vector<Index> m_indices;

    uint32_t m_blockCount         = 0;
    uint32_t m_miniBlockCount     = 0;
    uint32_t m_miniBlocksPerBlock = 0;

public:
    IndexTable(uint32_t miniBlockCount, uint32_t miniBlocksPerBlock);

    IndexTable() = default;

    /* ------ Index search index in the table ------ */
    uint32_t findHeaderBlockIndex(uint32_t miniBlockNumber) const;

    uint32_t findMiniBlockIndex(uint32_t miniBlockNumber) const;

    /* ------ Getters & Setters ------ */
    size_t size();

    Index* data();

    uint32_t getBlockCount() const;

    uint32_t getMiniBlockCount() const;

    uint32_t getMiniBlocksPerBlock() const;

    /* ------ IndexTable Service ------ */
    uint32_t getBlockNumber(uint32_t miniBlockNumber) const;

    void reset(uint32_t miniBlockCount, uint32_t miniBlocksPerBlock);

    /* ------ IndexTable Operators ------ */
    Index& operator[](const int index);

    friend bool operator==(const IndexTable& array1, const IndexTable& array2);

    friend bool operator!=(const IndexTable& array1, const IndexTable& array2);

    void fillOffsets(std::vector<uint32_t> vector);
};

#endif //QPL_PROJECT_INDEX_TABLE_HPP
