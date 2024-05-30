/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "gen.h"
#include "igenerator.h"
#include "indexed_stream/index_no_error.hpp"
#include "indexed_stream/mini_block_buffer_underflow.hpp"
#include "indexed_stream/mini_block_buffer_overflow.hpp"
#include "indexed_stream/incorrect_block_size.hpp"

std::unique_ptr<std::stringstream> gz_generator::IndexGenerator::getStreamDescription(TestFactor &testFactor)
{
    Gen32u seed = testFactor.seed;
    SpecialTestOptions options = testFactor.specialTestOptions;

    switch (testFactor.type)
    {
        case INDEX_DECOMPRESS_NO_ERR:
        {
            ConfiguratorDecompressIndexNoErr config(seed, options);
            config.generate();

            return config.getConfig();
        }

        case INDEX_DECOMPRESS_DISTANCE_SPANS_MINIBLOCKS:
        {
            InflateMiniBlockBufferUnderflowConfigurator config(seed, options);
            config.generate();

            return config.getConfig();
        }
        case INDEX_DECOMPRESS_MATCH_SPANS_MINIBLOCKS:
        {
            InflateMiniBlockBufferOverflowConfigurator config(seed, options);
            config.generate();

            return config.getConfig();
        }

        case INDEX_DECOMPRESS_INCORRECT_BLOCK_SIZE:
        {
            InflateIndexIncorrectBlockSizeConfigurator config(seed, options);
            config.generate();

            return config.getConfig();
        }

        default:
            throw std::exception();
    }
}

GenStatus gz_generator::IndexGenerator::generate(std::vector<Gen8u> &pBinaryData,
                                                 std::vector<Gen8u> &pReferenceData,
                                                 TestFactor &factor)
{
    IndexTable *index_table_ptr = factor.specialTestOptions.index_table;

    if (index_table_ptr == nullptr)
    {
        std::cout << "Index Table pointer is NULL !";
        return GEN_TEST_FACTOR_NEEDED;
    }

    try
    {
        // Get stream description
        std::unique_ptr<std::stringstream> config = getStreamDescription(factor);

        // Stream generation
        gen_c dataGenerator(config.get(), &pBinaryData, &pReferenceData);
        dataGenerator.parse_lines();

        // Index table filling
        index_table_ptr->fillOffsets(dataGenerator.getIndexes());

        return GEN_OK;
    }
    catch (const std::exception &e)
    {
        return GEN_ERR;
    }
}
