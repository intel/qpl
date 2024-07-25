/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "gen.h"
#include "igenerator.h"
#include "rfc1951_stream/break_block_configurators/bad_distance.hpp"
#include "rfc1951_stream/break_block_configurators/bad_stored_length.hpp"
#include "rfc1951_stream/break_block_configurators/distance_before_start.hpp"
#include "rfc1951_stream/break_block_configurators/unallowable_d_code.hpp"
#include "rfc1951_stream/break_block_configurators/unallowable_ll_code.hpp"
#include "rfc1951_stream/break_header_configurators/all_zero_literal_length_codes.hpp"
#include "rfc1951_stream/break_header_configurators/big_repeat_count_d_codes.hpp"
#include "rfc1951_stream/break_header_configurators/big_repeat_count_ll_codes.h"
#include "rfc1951_stream/break_header_configurators/cl_codes_span_single_table.hpp"
#include "rfc1951_stream/break_header_configurators/first_d_16_code.hpp"
#include "rfc1951_stream/break_header_configurators/first_ll_16_code.hpp"
#include "rfc1951_stream/break_header_configurators/invalid_block.hpp"
#include "rfc1951_stream/break_header_configurators/large_header.hpp"
#include "rfc1951_stream/break_header_configurators/many_distance_codes.hpp"
#include "rfc1951_stream/break_header_configurators/many_distance_codes_v2.hpp"
#include "rfc1951_stream/break_header_configurators/many_ll_codes.hpp"
#include "rfc1951_stream/break_header_configurators/no_literal_length_code.hpp"
#include "rfc1951_stream/break_header_configurators/oversubscribed_cl_tree.hpp"
#include "rfc1951_stream/break_header_configurators/oversubscribed_d_tree.hpp"
#include "rfc1951_stream/break_header_configurators/oversubscribed_ll_tree.hpp"
#include "rfc1951_stream/break_header_configurators/undefined_cl_code.hpp"
#include "rfc1951_stream/correct_stream_generators/canned_large_ll_table.hpp"
#include "rfc1951_stream/correct_stream_generators/canned_small_blocks.hpp"
#include "rfc1951_stream/correct_stream_generators/dynamic_block_no_err.hpp"
#include "rfc1951_stream/correct_stream_generators/fixed_block_no_err.hpp"
#include "rfc1951_stream/correct_stream_generators/huffman_only.hpp"
#include "rfc1951_stream/correct_stream_generators/stored_block_no_err.hpp"

std::unique_ptr<std::stringstream> gz_generator::InflateGenerator::getStreamDescription(TestFactor& testFactor) {
    const Gen32u seed = testFactor.seed;

    switch (testFactor.type) {
        // No Errors Test Configurators
        case NO_ERR_DYNAMIC_BLOCK: {
            DynamicBlockNoErrorConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        case NO_ERR_STORED_BLOCK: {
            StoredBlockNoErrorConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }
        case NO_ERR_FIXED_BLOCK: {
            FixedBlockNoErrorConfigurator config(seed);
            config.generate();
            return config.getConfig();
        }

        case NO_ERR_HUFFMAN_ONLY: {
            HuffmanOnlyNoErrorConfigurator config(seed, testFactor.specialTestOptions.decompression_huffman_table,
                                                  testFactor.specialTestOptions.is_aecs_format2_expected);
            config.generate();

            return config.getConfig();
        }

        //No errors Canned Tests
        case CANNED_SMALL: {
            CannedSmallBlocksConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        case CANNED_LARGE_LL: {
            CannedLargeLiteralLengthTableConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        // Broken Header Test Configurators
        case LARGE_HEADER: {
            LargeHeaderConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        case UNDEFINED_CL_CODE: {
            UndefinedCodeLengthCodeConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        case FIRST_LL_CODE_16: {
            FirstLiteralLengthCode16Configurator config(seed);
            config.generate();

            return config.getConfig();
        }

        case FIRST_D_CODE_16: {
            FirstDistanceLengthCode16Configurator config(seed);
            config.generate();

            return config.getConfig();
        }

        case NO_LL_CODE: {
            NoLiteralLengthCodeConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        case ALL_ZERO_LL_CODE: {
            AllZeroLiteralLengthCodesConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        case CL_CODES_SPAN_LL_WITH_D: {
            StradingCodeLengthCodesConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        case TOO_MANY_D_CODES: {
            ManyDistanceCodesConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        case TOO_MANY_D_CODES_V2: {
            ManyDistanceCodesConfiguratorVersion2 config(seed);
            config.generate();

            return config.getConfig();
        }

        case TOO_MANY_LL_CODES: {
            ManyLiteralLengthCodesConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        case OVERSUBSCRIBED_CL_TREE: {
            OversubscribedCodeLengthsTreeConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        case OVERSUBSCRIBED_D_TREE: {
            OversubscribedDistanceTreeConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        case OVERSUBSCRIBED_LL_TREE: {
            OversubscribedLiteralLengthsTreeConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        case INVALID_BLOCK_TYPE: {
            InvalidBlockTypeConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        case BIG_REPEAT_COUNT_LL: {
            BigRepeatCountLiteralLengthCodesConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        case BIG_REPEAT_COUNT_D: {
            BigRepeatCountDistanceLengthCodesConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        // Broken Block Test Configurators
        case BAD_STORED_LEN: {
            BadStoredLengthConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        case BAD_DIST: {
            BadDistanceConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        case DIST_BEFORE_START: {
            DistanceCodeBeforeStartConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        case BLOCK_CONTAIN_UNALLOWABLE_LL_CODE: {
            UnallowableLiteralLengthCodeConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        case BLOCK_CONTAIN_UNALLOWABLE_D_CODE: {
            UnallowableDistanceCodeConfigurator config(seed);
            config.generate();

            return config.getConfig();
        }

        default: throw std::exception();
    }
}

GenStatus gz_generator::InflateGenerator::generate(std::vector<Gen8u>& pBinaryData, std::vector<Gen8u>& pReferenceData,
                                                   TestFactor& factor) {
    try {
        // Get stream description
        const std::unique_ptr<std::stringstream> config = getStreamDescription(factor);

        // Stream generation
        gen_c dataGenerator(config.get(), &pBinaryData, &pReferenceData);
        dataGenerator.parse_lines();

        return GEN_OK;
    } catch (const std::exception& e) { return GEN_ERR; }
}
