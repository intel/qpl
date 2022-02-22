/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_DECOMPRESS_INDEX_H
#define QPL_DECOMPRESS_INDEX_H

#include "base_configurator.hpp"

namespace  gz_generator
{
    static constexpr Gen32u OWN_GEN_MIN_OFFSET = 1u;    // Minimum offset in mini-block

    class ConfiguratorDecompressIndex: public TestConfigurator
    {
        protected:
            float             m_literalProbability;
            const Gen32u      m_miniBlockSize;
            IndexTable *m_pIndexTable;

            void getIndex();
            void generateMiniBlock(Gen32u mini_block_size);

        public:
            ConfiguratorDecompressIndex(Gen32u seed, SpecialTestOptions &testOptions)
                : TestConfigurator(seed),
                  m_pIndexTable(testOptions.index_table),
                  m_miniBlockSize(testOptions.mini_block_size)
                {
                    m_literalProbability = static_cast<float>(m_random);
                }

            ConfiguratorDecompressIndex() = delete;

            GenStatus generate() override;

        private:
            virtual void generateMiniBlockSequence(Gen32u miniBlockCount) = 0;
    };
}
#endif //QPLDECOMPRESS_INDEX_H
