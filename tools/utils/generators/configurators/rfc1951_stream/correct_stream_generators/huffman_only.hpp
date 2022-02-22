/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef _CONFIGURATOR_HUFFMAN_ONLY_HPP_
#define _CONFIGURATOR_HUFFMAN_ONLY_HPP_

#include "base_configurator.hpp"

/*Based on dcmp_tokengen7.pl*/
namespace gz_generator
{
    struct GzHuffmanTriplet
    {
        Gen32u index;
        Gen32u len;
        Gen32u code;
    };

    class HuffmanOnlyNoErrorConfigurator: public TestConfigurator
    {
        GenDecompressionHuffmanTable &m_huffmanTable;
        public:
            HuffmanOnlyNoErrorConfigurator(Gen32u seed, GenDecompressionHuffmanTable &table)
                : TestConfigurator(seed),
                  m_huffmanTable(table){}

            HuffmanOnlyNoErrorConfigurator() = delete;

            GenStatus generate() override;

        private:
            std::vector<Gen32u> generateLiteralSequences(std::vector<Gen32u> &pLiteralLengthCodesTable);
            std::vector<Gen32u> computeHuffmanCodes(std::vector<Gen32u> &pLiteralLengthsTable);

            Gen32u calculateStreamBitLength(std::vector<Gen32u> &pLiteralsSequence, std::vector<Gen32u> &pLiteralLengths);

            void buildDecompressionTable(std::vector<Gen32u> &pLiteralLengthCodesTable,
                                         std::vector<Gen32u> &pHuffmanCodes);
            void declareAllLiterals(std::vector<Gen32u> &pLiteralVector);
            void saveHuffmanTable(std::vector<Gen32u> &pTestHuffmantable);
            void declareRawBlock();
    };
}

#endif //_CONFIGURATOR_HUFFMAN_ONLY_HPP_
