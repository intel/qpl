/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "base_configurator.hpp"

#define GZ_DEBUG_OUTPUT

/*
 * Writers Implementation
 */

#include <algorithm>
#include <iterator>
#include <random>
#include <sstream>

void gz_generator::TestConfigurator::writeRandomBlock() {
    if (static_cast<float>(m_random) < HUFFMAX_BLOCK_PROBABILITY) {
        writeRandomHuffmanBlock();
    } else {
        writeRandomStoredBlock();
    }
}

void gz_generator::TestConfigurator::writeRandomHuffmanBlock(uint32_t minElementsCount) {
    const uint32_t maxElementsCount = 100U;

    declareRandomHuffmanBlock();
    m_randomTokenCount.set_range(minElementsCount,
                                 minElementsCount > maxElementsCount ? minElementsCount + 10 : maxElementsCount);
    writeRandomReferenceSequence(static_cast<Gen32u>(m_randomTokenCount));
}

void gz_generator::TestConfigurator::writeRandomStoredBlock() {
    m_randomTokenCount.set_range(1000U, 2000U);
    TestConfigurator::declareStoredBlock();
    writeRandomLiteralSequence(static_cast<Gen32u>(m_randomTokenCount));
}

void gz_generator::TestConfigurator::writeRandomLiteralSequence(Gen32u sequenceLength) {
    for (Gen32u symbols = 0U; symbols < sequenceLength; symbols++) {
        TestConfigurator::declareLiteral(static_cast<Gen32u>(m_randomLiteralCode));
    }
}

Gen32u gz_generator::TestConfigurator::writeRandomReferenceSequence(Gen32u sequenceLength,
                                                                    Gen32u previouslyLiteralsEncoded,
                                                                    Gen32u encodedLiteralsCountLimit) {
    uint32_t numberLiteralsEncoded = previouslyLiteralsEncoded;

    for (uint32_t symbols = 0U; (symbols < sequenceLength) && (encodedLiteralsCountLimit > numberLiteralsEncoded);
         symbols++) {
        if ((static_cast<float>(m_random) < 0.7F) || (numberLiteralsEncoded < MIN_MATCH) ||
            ((encodedLiteralsCountLimit - numberLiteralsEncoded) < MIN_MATCH)) {
            TestConfigurator::declareLiteral(static_cast<uint32_t>(m_randomLiteralCode));
            numberLiteralsEncoded++;
        } else {
            uint32_t       offset              = 0U;
            uint32_t       match               = 0U;
            const uint32_t max_available_match = GEN_MIN(encodedLiteralsCountLimit - numberLiteralsEncoded, MAX_MATCH);

            m_randomOffset.set_range(1U, GEN_MIN(numberLiteralsEncoded, MAX_OFFSET));
            m_randomMatch.set_range(MIN_MATCH, GEN_MIN(numberLiteralsEncoded, max_available_match));
            offset = static_cast<uint32_t>(m_randomOffset);
            match  = static_cast<uint32_t>(m_randomMatch);

            TestConfigurator::declareReference(match, offset);
            numberLiteralsEncoded += match;
        }
    }
    return numberLiteralsEncoded;
}

//returns the produced table length
Gen32u gz_generator::TestConfigurator::makeRandomLengthCodesTable(Gen32u* pLengthCodeTable, Gen32u lengthTableSize,
                                                                  Gen8u maxLengthCodeValue) const {
    pLengthCodeTable[0U]          = 1U;
    pLengthCodeTable[1U]          = 1U;
    Gen32u            len         = 0U;
    Gen32u            lengthCount = 2U; //num length
    Gen32u            num_max     = 0U;
    qpl::test::random rand(0U, lengthTableSize, m_seed);

    while ((lengthCount + num_max) < lengthTableSize) {
        rand.set_range(0U, lengthCount - 1U);
        const Gen8u i = static_cast<Gen32u>(rand);
        len           = pLengthCodeTable[i] + 1U;
        if (len < maxLengthCodeValue) {
            pLengthCodeTable[i]             = len;
            pLengthCodeTable[lengthCount++] = len;
        } else {
            pLengthCodeTable[i] = pLengthCodeTable[--lengthCount];
            num_max += 2U;
        }
    }

    for (Gen32u i = 0U; i < num_max; i++) {
        pLengthCodeTable[lengthCount + i] = maxLengthCodeValue;
    }

    CommonMethods::shuffle_32u(pLengthCodeTable, lengthTableSize, m_seed);

    return lengthCount + num_max;
}

/*
 * Delegates implementation
 */

void gz_generator::TestConfigurator::declareRandomBlock() {
    if (static_cast<float>(m_random) < STORED_BLOCK_PROBABILITY) {
        TestConfigurator::declareStoredBlock();
    } else {
        TestConfigurator::declareRandomHuffmanBlock();
    }
}

void gz_generator::TestConfigurator::declareRandomHuffmanBlock() {
    if (static_cast<float>(m_random) < FIXED_BLOCK_PROBABILITY) {
        TestConfigurator::declareFixedBlock();
    } else {
        TestConfigurator::declareDynamicBlock();
    }
}

void gz_generator::TestConfigurator::declareFixedBlock() {
    *m_config << "block fixed\n";
}

void gz_generator::TestConfigurator::declareDynamicBlock() {
    *m_config << "block\n";
    ;
}

void gz_generator::TestConfigurator::declareStoredBlock() {
    *m_config << "block stored\n";
}

void gz_generator::TestConfigurator::declareInvalidBlock() {
    *m_config << "block invalid\n";
}

void gz_generator::TestConfigurator::declareFinishBlock() {
    *m_config << "bfinal 1\n";
}

void gz_generator::TestConfigurator::declareLiteral(Gen32u literal) {
    *m_config << "l " << std::to_string(literal) << '\n';
}

void gz_generator::TestConfigurator::declareReference(Gen32u match, Gen32u offset) {
    *m_config << "r " << std::to_string(match) << ' ' << std::to_string(offset) + '\n';
}

void gz_generator::TestConfigurator::declareExtraLengths() {
    *m_config << "set extra_lens\n";
}

void gz_generator::TestConfigurator::declareRandomLiterals(Gen32u count) {
    *m_config << "l ? * " + std::to_string(count) + "\n";
}

void gz_generator::TestConfigurator::declareVectorToken(const VectorTokenType type, const Gen32u* vector_ptr,
                                                        const Gen32u length) {
    std::string vectorName;
    switch (type) {
        case LL_VECTOR: vectorName = "ll_lens"; break;
        case LL_ENCODED_VECTOR: vectorName = "ll_lens encoded"; break;
        case D_VECTOR: vectorName = "d_lens"; break;
        case D_ENCODED_VECTOR: vectorName = "d_lens encoded"; break;
        case CL_VECTOR: vectorName = "cl_lens"; break;
        case CL_VECTOR_ALT: vectorName = "cl_lens alt"; break;
        case CL_ENCODED_VECTOR: vectorName = "cl_lens encoded"; break;
        case L_VECTOR: vectorName = "l"; break;
        default: return;
    }

    *m_config << vectorName;

    for (Gen32u i = 0U; i < length; i++) {
        *m_config << ' ' << std::to_string(vector_ptr[i]);
    }
    *m_config << '\n';
}

void gz_generator::TestConfigurator::declareTestToken(const Gen32u testNumber) {
    *m_config << "testmode " << std::to_string(testNumber) << '\n';
}

void gz_generator::TestConfigurator::declareTestToken(const Gen32u testNumber, const Gen32u testGroup) {
    *m_config << "testmode " << std::to_string(testNumber) << " " << std::to_string(testGroup) << '\n';
}

std::unique_ptr<std::stringstream> gz_generator::TestConfigurator::getConfig() {
    //std::cout << m_config.get()->str();

    return std::move(m_config);
}
