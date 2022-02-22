/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <cstring>
#include "huffman_only.hpp"

GenStatus gz_generator::HuffmanOnlyNoErrorConfigurator::generate()
{
    std::vector<Gen32u> pCodeLengthsTable(LITERALS_HIGH_BORDER + 1u);
    std::vector<Gen32u> pLiteralSequence;


    TestConfigurator::makeRandomLengthCodesTable(pCodeLengthsTable.data(),
                                                 (Gen32u)pCodeLengthsTable.size(),
                                                 MAX_LL_CODE_BIT_LENGTH);

    HuffmanOnlyNoErrorConfigurator::saveHuffmanTable(pCodeLengthsTable);

    pLiteralSequence = HuffmanOnlyNoErrorConfigurator::generateLiteralSequences(pCodeLengthsTable);

    HuffmanOnlyNoErrorConfigurator::declareRawBlock();
    TestConfigurator::declareVectorToken(LL_VECTOR, pCodeLengthsTable.data(), (Gen32u)pCodeLengthsTable.size());

    HuffmanOnlyNoErrorConfigurator::declareAllLiterals(pLiteralSequence);

    return GEN_OK;
}

void gz_generator::HuffmanOnlyNoErrorConfigurator::saveHuffmanTable(std::vector<Gen32u> &pCodeLengthsTable)
{
    std::vector<Gen32u> pHuffmanCodes = HuffmanOnlyNoErrorConfigurator::computeHuffmanCodes(pCodeLengthsTable);
    HuffmanOnlyNoErrorConfigurator::buildDecompressionTable(pCodeLengthsTable, pHuffmanCodes);
}

void gz_generator::HuffmanOnlyNoErrorConfigurator::declareRawBlock()
{
    *m_config << "block raw\n";
    *m_config << "no_eob\n";
}

std::vector<Gen32u> gz_generator::HuffmanOnlyNoErrorConfigurator::generateLiteralSequences(
        std::vector<Gen32u> &pCodeLengthsTable)
{
    Gen32u  literalCount = 0u;
    std::vector<Gen32u> pLiteralSequence;

    m_randomTokenCount.set_range(128u, 512u);
    literalCount = static_cast<Gen32u>(m_randomTokenCount);

    while (pLiteralSequence.size() < literalCount) {
        if (pLiteralSequence.size() < 4u || (0.8f > static_cast<float>(m_random))) {
            pLiteralSequence.push_back(static_cast<Gen32u>(m_randomLiteralCode));
        } else {
            Gen32u match;
            Gen32u offset;

            m_randomOffset.set_range(1u, GEN_MIN(MAX_OFFSET, (uint32_t) pLiteralSequence.size()));
            m_randomMatch.set_range(4u, 7u);
            offset = static_cast<Gen32u>(m_randomOffset);
            match = static_cast<Gen32u>(m_randomMatch);

            for (Gen32u repeat = 0; repeat < match; repeat++) {
                pLiteralSequence.push_back(pLiteralSequence[pLiteralSequence.size() - offset]);
            }
        }
    }

    {
        Gen32u streamBitLength  = 0u;
        Gen32u minLength        = 0u;
        Gen32u literal          = 0u;

        streamBitLength = HuffmanOnlyNoErrorConfigurator::calculateStreamBitLength(pLiteralSequence,
                                                                                   pCodeLengthsTable);
        streamBitLength &= 0xFF;

        minLength = *std::min_element(pCodeLengthsTable.begin(), pCodeLengthsTable.end());

        if (0u != streamBitLength)
        {
            while( 16u >= streamBitLength + minLength)
            {
                literal = static_cast<Gen32u>(m_randomLiteralCode);
                pLiteralSequence.push_back(literal);
                streamBitLength = (streamBitLength + pCodeLengthsTable[literal]) & 0xFF;

                if ( 0u == streamBitLength)
                {
                    break;
                }
            }
        }
    }
    return  pLiteralSequence;
}

Gen32u gz_generator::HuffmanOnlyNoErrorConfigurator::calculateStreamBitLength(std::vector<Gen32u> &pLiterals,
                                                                              std::vector<Gen32u> &pLiteralLengthsTable)
{
    Gen32u streamBitLength = 0;
    for (const auto &literal : pLiterals)
    {
        streamBitLength += pLiteralLengthsTable[literal];
    }
    return  streamBitLength;
}

void gz_generator::HuffmanOnlyNoErrorConfigurator::declareAllLiterals(std::vector<Gen32u> &pLiteralVector)
{
    for (const auto &literal : pLiteralVector)
    {
        TestConfigurator::declareLiteral(literal);
    }
}

std::vector<Gen32u> gz_generator::HuffmanOnlyNoErrorConfigurator::computeHuffmanCodes(std::vector<Gen32u> &pLiteralLengthsTable)
{
    Gen32u maxCodeLength    = 0u;

    std::vector<Gen32u> pBitLengthCountTable(MAX_LL_CODE_BIT_LENGTH + 1u);
    std::fill(pBitLengthCountTable.begin(), pBitLengthCountTable.end(), 0);
    std::vector<Gen32u> next_code(MAX_LL_CODE_BIT_LENGTH + 1u);
    std::fill(next_code.begin(), next_code.end(), 0);
    std::vector<Gen32u> huffmanCodes;

    //count number of equal bit lengths
    for (const auto &literalLength : pLiteralLengthsTable)
    {
        pBitLengthCountTable[literalLength]++;
        maxCodeLength = (literalLength > maxCodeLength) ? literalLength : maxCodeLength;
    }

    {
        Gen32u huffmanCodeValue = 0;
        for (Gen32u bits = 1; bits <= maxCodeLength; bits++)
        {
            huffmanCodeValue = (huffmanCodeValue + pBitLengthCountTable[bits - 1]) << 1;
            next_code[bits] = huffmanCodeValue;
        }
    }

    for (Gen32u i = 0; i < pLiteralLengthsTable.size(); i++)
    {
        Gen32u literalLength = pLiteralLengthsTable[i];

        if ( 0u == literalLength)
        {
            huffmanCodes.push_back(0u);
        }
        else
        {
            huffmanCodes.push_back(next_code[literalLength]);
            next_code[literalLength]++;
        }
    }

    return huffmanCodes;
}

void gz_generator::HuffmanOnlyNoErrorConfigurator::buildDecompressionTable(std::vector<Gen32u> &pLiteralLengthCodesTable,
                                                                           std::vector<Gen32u> &pHuffmanCodes)
{
    // Variables
    Gen32u emptyPosition = 0u;
    Gen32u startPosition = 0u;

    std::vector<GzHuffmanTriplet> tmpTriplets(256u);
    std::memset(&m_huffmanTable, 0u, sizeof(GenDecompressionHuffmanTable));

    // Prepare triplets
    for (Gen16u i = 0u; i < 256u; i++)
    {
        tmpTriplets[i].code  = pHuffmanCodes[i];
        tmpTriplets[i].len   = pLiteralLengthCodesTable[i];
        tmpTriplets[i].index = i;
	}
    
    // Calculate code lengths histogram
    std::for_each(tmpTriplets.begin(), tmpTriplets.end(), [&](const GzHuffmanTriplet &item)
    {
        m_huffmanTable.number_of_codes[item.len - 1u]++;
    });

    // Calculate first codes
    for (Gen32u i = 1u; i <= 15u; i++)
    {
        if (m_huffmanTable.number_of_codes[i - 1u] == 0u)
        {
            continue;
        }

        std::vector<GzHuffmanTriplet> filtered;

        // Filtering by code length
        std::copy_if(tmpTriplets.begin(),
                     tmpTriplets.end(),
                     std::back_inserter(filtered),
                     [&](const GzHuffmanTriplet triplet)
                     {
                         return triplet.len == i;
                     });

        // Sorting to get right order for mapping table (charToSortedCode)
        std::sort(filtered.begin(), filtered.end(), [](const GzHuffmanTriplet a, const GzHuffmanTriplet b)
        {
            return a.code < b.code;
        });

        m_huffmanTable.first_codes[i - 1u]        = filtered[0u].code;
        m_huffmanTable.first_table_indexes[i - 1u] = emptyPosition;

        // Writing in sorted order
        startPosition = emptyPosition;
        while (emptyPosition < (startPosition + filtered.size()))
        {
            m_huffmanTable.index_to_char[emptyPosition] = filtered[emptyPosition - startPosition].index;
            emptyPosition++;
        }
    }
}
