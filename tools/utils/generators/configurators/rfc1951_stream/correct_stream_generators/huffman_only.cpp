/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "huffman_only.hpp"

#include <cstring>

GenStatus gz_generator::HuffmanOnlyNoErrorConfigurator::generate() {
    std::vector<Gen32u> pCodeLengthsTable(LITERALS_HIGH_BORDER + 1U);
    std::vector<Gen32u> pLiteralSequence;

    TestConfigurator::makeRandomLengthCodesTable(pCodeLengthsTable.data(), (Gen32u)pCodeLengthsTable.size(),
                                                 MAX_LL_CODE_BIT_LENGTH);

    HuffmanOnlyNoErrorConfigurator::saveHuffmanTable(pCodeLengthsTable);

    pLiteralSequence = HuffmanOnlyNoErrorConfigurator::generateLiteralSequences(pCodeLengthsTable);

    HuffmanOnlyNoErrorConfigurator::declareRawBlock();
    TestConfigurator::declareVectorToken(LL_VECTOR, pCodeLengthsTable.data(), (Gen32u)pCodeLengthsTable.size());

    HuffmanOnlyNoErrorConfigurator::declareAllLiterals(pLiteralSequence);

    return GEN_OK;
}

void gz_generator::HuffmanOnlyNoErrorConfigurator::saveHuffmanTable(std::vector<Gen32u>& pCodeLengthsTable) {
    std::vector<Gen32u> pHuffmanCodes = HuffmanOnlyNoErrorConfigurator::computeHuffmanCodes(pCodeLengthsTable);
    HuffmanOnlyNoErrorConfigurator::buildDecompressionTable(pCodeLengthsTable, pHuffmanCodes);
}

void gz_generator::HuffmanOnlyNoErrorConfigurator::declareRawBlock() {
    *m_config << "block raw\n";
    *m_config << "no_eob\n";
}

std::vector<Gen32u>
gz_generator::HuffmanOnlyNoErrorConfigurator::generateLiteralSequences(std::vector<Gen32u>& pCodeLengthsTable) {
    Gen32u              literalCount = 0U;
    std::vector<Gen32u> pLiteralSequence;

    m_randomTokenCount.set_range(128U, 512U);
    literalCount = static_cast<Gen32u>(m_randomTokenCount);

    while (pLiteralSequence.size() < literalCount) {
        if (pLiteralSequence.size() < 4U || (0.8F > static_cast<float>(m_random))) {
            pLiteralSequence.push_back(static_cast<Gen32u>(m_randomLiteralCode));
        } else {
            Gen32u match  = 0U;
            Gen32u offset = 0U;

            m_randomOffset.set_range(1U, GEN_MIN(MAX_OFFSET, (uint32_t)pLiteralSequence.size()));
            m_randomMatch.set_range(4U, 7U);
            offset = static_cast<Gen32u>(m_randomOffset);
            match  = static_cast<Gen32u>(m_randomMatch);

            for (Gen32u repeat = 0U; repeat < match; repeat++) {
                pLiteralSequence.push_back(pLiteralSequence[pLiteralSequence.size() - offset]);
            }
        }
    }

    {
        Gen32u streamBitLength = 0U;
        Gen32u minLength       = 0U;
        Gen32u literal         = 0U;

        streamBitLength = HuffmanOnlyNoErrorConfigurator::calculateStreamBitLength(pLiteralSequence, pCodeLengthsTable);
        streamBitLength &= 0xFF;

        minLength = *std::min_element(pCodeLengthsTable.begin(), pCodeLengthsTable.end());

        if (0U != streamBitLength) {
            while (16U >= streamBitLength + minLength) {
                literal = static_cast<Gen32u>(m_randomLiteralCode);
                pLiteralSequence.push_back(literal);
                streamBitLength = (streamBitLength + pCodeLengthsTable[literal]) & 0xFF;

                if (0U == streamBitLength) { break; }
            }
        }
    }
    return pLiteralSequence;
}

Gen32u
gz_generator::HuffmanOnlyNoErrorConfigurator::calculateStreamBitLength(std::vector<Gen32u>& pLiterals,
                                                                       std::vector<Gen32u>& pLiteralLengthsTable) {
    Gen32u streamBitLength = 0U;
    for (const auto& literal : pLiterals) {
        streamBitLength += pLiteralLengthsTable[literal];
    }
    return streamBitLength;
}

void gz_generator::HuffmanOnlyNoErrorConfigurator::declareAllLiterals(std::vector<Gen32u>& pLiteralVector) {
    for (const auto& literal : pLiteralVector) {
        TestConfigurator::declareLiteral(literal);
    }
}

std::vector<Gen32u>
gz_generator::HuffmanOnlyNoErrorConfigurator::computeHuffmanCodes(std::vector<Gen32u>& pLiteralLengthsTable) {
    Gen32u maxCodeLength = 0U;

    std::vector<Gen32u> pBitLengthCountTable(MAX_LL_CODE_BIT_LENGTH + 1U);
    std::fill(pBitLengthCountTable.begin(), pBitLengthCountTable.end(), 0U);
    std::vector<Gen32u> next_code(MAX_LL_CODE_BIT_LENGTH + 1U);
    std::fill(next_code.begin(), next_code.end(), 0U);
    std::vector<Gen32u> huffmanCodes;

    //count number of equal bit lengths
    for (const auto& literalLength : pLiteralLengthsTable) {
        pBitLengthCountTable[literalLength]++;
        maxCodeLength = (literalLength > maxCodeLength) ? literalLength : maxCodeLength;
    }

    {
        Gen32u huffmanCodeValue = 0U;
        for (Gen32u bits = 1U; bits <= maxCodeLength; bits++) {
            huffmanCodeValue = (huffmanCodeValue + pBitLengthCountTable[bits - 1U]) << 1;
            next_code[bits]  = huffmanCodeValue;
        }
    }

    for (Gen32u i = 0U; i < pLiteralLengthsTable.size(); i++) {
        const Gen32u literalLength = pLiteralLengthsTable[i];

        if (0U == literalLength) {
            huffmanCodes.push_back(0U);
        } else {
            huffmanCodes.push_back(next_code[literalLength]);
            next_code[literalLength]++;
        }
    }

    return huffmanCodes;
}

/**
 * @brief Routine to construct decompression representation for Huffman Only
 *
 * @details Based on the _is_aecs_format2_expected value (that indicates AECS Format to be generated),
 * build either mapping table and first table indices array or mapping CAM.
 * Also construct number of codes and first codes arrays (used in both Formats).
 * Basic idea is to go through all bitwidths (1-15), filter out the triplets corresponding
 * to a given code lengths, sort this subset and fill data required for specified format.
 *
 * Mapping table (corresponds to AECS Format-1) is such that:
 * we store all the symbols with length 1 first (sorted), then all the symbols with length 2 (sorted), etc.
 * For a given code length i, number of such codes is number_of_codes[i],
 * region with these codes in the mapping table starts with first_table_indexes[i] offset,
 * and additionally we store first_code[i] for the code with length i that is first once sorted.
 * Table index for certain input code of length i could be computed then as:
 * first_table_indexes[i] + input code - first_code[i].
 *
 * Mapping CAM (corresponds to AECS Format-2) is such that, for each entry
 * the index is the input symbol and the value is the pair of input code length and (input code - first code),
 * stored in first 4 bits and next 4 bits respectively;
 * Therefore CAM size is exactly 265 (number of len codes without once requiring extra bits).
 * Working with Mapping CAM requires number_of_codes and first_codes arrays,
 * but doesn't require first_table_indexes (as calculating offset is not needed).
*/
void gz_generator::HuffmanOnlyNoErrorConfigurator::buildDecompressionTable(
        std::vector<Gen32u>& pLiteralLengthCodesTable, std::vector<Gen32u>& pHuffmanCodes) {
    // Variables
    Gen32u emptyPosition = 0U;
    Gen32u startPosition = 0U;
    Gen32u bitWidthIndex = 0U;

    std::vector<GzHuffmanTriplet> tmpTriplets(256U);
    std::memset(&m_huffmanTable, 0U, sizeof(GenDecompressionHuffmanTable));

    // Set format_stored
    if (_is_aecs_format2_expected) {
        m_huffmanTable.format_stored = ht_with_mapping_cam;
    } else {
        m_huffmanTable.format_stored = ht_with_mapping_table;
    }

    // Prepare triplets
    for (Gen16u i = 0U; i < 256U; i++) {
        tmpTriplets[i].code  = pHuffmanCodes[i];
        tmpTriplets[i].len   = pLiteralLengthCodesTable[i];
        tmpTriplets[i].index = i;
    }

    // Calculate code lengths histogram
    std::for_each(tmpTriplets.begin(), tmpTriplets.end(),
                  [&](const GzHuffmanTriplet& item) { m_huffmanTable.number_of_codes[item.len - 1U]++; });

    // Calculate first codes
    for (Gen32u i = 1U; i <= 15U; i++) {
        if (m_huffmanTable.number_of_codes[i - 1U] == 0U) { continue; }

        std::vector<GzHuffmanTriplet> filtered;

        // Filtering by code length
        std::copy_if(tmpTriplets.begin(), tmpTriplets.end(), std::back_inserter(filtered),
                     [&](const GzHuffmanTriplet triplet) { return triplet.len == i; });

        // Sorting to get right order for mapping table (charToSortedCode)
        std::sort(filtered.begin(), filtered.end(),
                  [](const GzHuffmanTriplet a, const GzHuffmanTriplet b) { return a.code < b.code; });

        m_huffmanTable.first_codes[i - 1U] = filtered[0U].code;

        if (_is_aecs_format2_expected) {
            bitWidthIndex = 0U;
            for (auto& item : filtered) {
                m_huffmanTable.lit_cam[item.index] = item.len | (bitWidthIndex << 4);
                bitWidthIndex++;
            }
        } else {
            m_huffmanTable.first_table_indexes[i - 1U] = emptyPosition;

            // Writing in sorted order
            startPosition = emptyPosition;
            while (emptyPosition < (startPosition + filtered.size())) {
                m_huffmanTable.index_to_char[emptyPosition] = filtered[emptyPosition - startPosition].index;
                emptyPosition++;
            }
        }
    }
}
