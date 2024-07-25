/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "large_header.hpp"

#include <cmath>

#define BITS_IN_QWORD 64U
#define BITS_IN_BYTE  8U

GenStatus gz_generator::LargeHeaderConfigurator::generate() {
    GenStatus status               = GEN_OK;
    Gen32u    startingSize         = 0U;
    Gen32u    startingQWords       = 0U;
    Gen32u    startingBits         = 0U;
    Gen32u    endingSize           = 0U;
    Gen32u    endingBits           = 0U;
    Gen32u    middleQWords         = 0U;
    Gen32u    middleBytes          = 0U;
    Gen32u    possibleLiteralCount = 0U;

    qpl::test::random random(0U, 0U, m_seed);

    random.set_range(1U, 8U);

    startingSize = 8U * static_cast<Gen32u>(random);
    endingSize   = 8U * static_cast<Gen32u>(random);

    if (0.99F > static_cast<float>(m_random)) {
        random.set_range(3U, 34U);
        startingQWords = static_cast<Gen32u>(random);
        startingBits   = BITS_IN_QWORD * startingQWords;

        random.set_range(0U, startingSize - 1U);
        startingBits += static_cast<Gen32u>(random);

        status = LargeHeaderConfigurator::writeBlock(startingBits);
        if (status) { return status; }
        startingBits &= BITS_IN_QWORD - 1U;
    } else {
        startingBits = 0U;
    }

    startingBits = startingSize - startingBits;

    if (0.5F > static_cast<float>(m_random)) {
        random.set_range(0U, 7U);
    } else {
        random.set_range(0U, endingSize - 1U);
    }

    endingBits = static_cast<Gen32u>(random) + 1U;

    middleQWords = 32U;
    random.set_range(middleQWords, 8U * middleQWords);
    middleBytes = static_cast<Gen32u>(random);

    TestConfigurator::declareDynamicBlock();

    status = LargeHeaderConfigurator::writeLiteralLengthCodes(startingBits + middleBytes * BITS_IN_BYTE + endingBits,
                                                              &possibleLiteralCount);

    if (status) { return status; }

    m_randomLiteralCode.set_range(0U, possibleLiteralCount - 1U);
    {
        Gen32u              literalCount = 0U;
        std::vector<Gen32u> pLiteralVector;

        random.set_range(0U, 8U);
        literalCount = static_cast<Gen32u>(m_random);
        if (literalCount != 0U) {
            for (Gen32u literal = 0U; literal < literalCount; literal++) {
                pLiteralVector.push_back(static_cast<Gen32u>(m_randomLiteralCode));
            }
            TestConfigurator::declareVectorToken(L_VECTOR, pLiteralVector.data(), (Gen32u)pLiteralVector.size());

            *m_config << "l 0*" + std::to_string(literalCount) + "\n";
        }
    }

    TestConfigurator::declareFinishBlock();
    return GEN_OK;
}

GenStatus gz_generator::LargeHeaderConfigurator::writeLiteralLengthCodes(Gen32u  header_bit_size,
                                                                         Gen32u* pPossibleLiteralCount) {
    Gen32u num13 = 0U; //TODO Naming!!!
    Gen32u x     = 0U; //TODO Naming!!!
    Gen32u cl13  = 0U; //TODO Naming!!!

    std::vector<Gen32u> pLiteralLengthCodes;
    std::vector<Gen32u> pDictanceLengthCodes = {14U, 15U, 14U, 15U, 14U, 15U, 14U, 15U,
                                                14U, 15U, 14U, 15U, 14U, 15U, 14U, 15U};

    std::vector<Gen32u> pCodeLengthCodes = {0U, 5U, 7U, 0U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U};

    //    Some compilers support case ranges like case x ... y as an extension to the C++ language.
    //    GCC 4.9, Clang 3.5.1 and Intel C/C++ Compiler 13.0.1 seem to support it
    //    (tried on http://gcc.godbolt.org/). On the other hand, Visual C++ 19 doesn't (tried on http://webcompiler.cloudapp.net/).

    if (2283U < header_bit_size) {
        return GEN_ERR;
    } else if (2002U <= header_bit_size) {
        num13 = caclulateNumberLiteralLengthsCodesEqualTo13(2285U, header_bit_size);
        x     = 7U;
    } else if (1845U <= header_bit_size) {
        num13 = caclulateNumberLiteralLengthsCodesEqualTo13(2128U, header_bit_size);
        x     = 6U;
    } else if (1688U <= header_bit_size) {
        num13 = caclulateNumberLiteralLengthsCodesEqualTo13(1971U, header_bit_size);
        x     = 5U;
    } else if (1531U <= header_bit_size) {
        num13 = caclulateNumberLiteralLengthsCodesEqualTo13(1814U, header_bit_size);
        x     = 4U;
    } else if (1374U <= header_bit_size) {
        num13 = caclulateNumberLiteralLengthsCodesEqualTo13(1657U, header_bit_size);
        x     = 3U;
    } else if (1217U <= header_bit_size) {
        num13 = caclulateNumberLiteralLengthsCodesEqualTo13(1500U, header_bit_size);
        x     = 2U;
    } else if (1060U <= header_bit_size) {
        num13 = caclulateNumberLiteralLengthsCodesEqualTo13(1343U, header_bit_size);
        x     = 1U;
    } else {
        return LargeHeaderConfigurator::writeLiteralLengthsCodesForSmallHeader(header_bit_size, pPossibleLiteralCount);
    }

    *pPossibleLiteralCount = LITERALS_HIGH_BORDER + 1U;

    cl13 = 6U - ((int)(pow(header_bit_size, x)) & 1U);

    pLiteralLengthCodes.push_back(13U);
    pLiteralLengthCodes.push_back(12U);

    {
        Gen32u code = 1U;
        for (; code < num13; code++) {
            pLiteralLengthCodes.push_back(13U);
            pLiteralLengthCodes.push_back(15U);
        }
        for (code = code; code < 143U; code++) {
            pLiteralLengthCodes.push_back(14U);
            pLiteralLengthCodes.push_back(15U);
        }
    }
    pCodeLengthCodes[0U] = cl13;
    pCodeLengthCodes[3U] = x;

    TestConfigurator::declareVectorToken(LL_VECTOR, pLiteralLengthCodes.data(), (int)pLiteralLengthCodes.size());
    TestConfigurator::declareVectorToken(D_VECTOR, pDictanceLengthCodes.data(), (int)pDictanceLengthCodes.size());
    TestConfigurator::declareVectorToken(D_VECTOR, pDictanceLengthCodes.data(), (int)pDictanceLengthCodes.size() - 2U);
    TestConfigurator::declareVectorToken(CL_VECTOR, pCodeLengthCodes.data(), (int)pDictanceLengthCodes.size());

    return GEN_OK;
}

GenStatus gz_generator::LargeHeaderConfigurator::writeLiteralLengthsCodesForSmallHeader(Gen32u  header_bit_size,
                                                                                        Gen32u* pPossibleLiteralCount) {
    Gen32u cl12      = 0U;
    Gen32u num1314   = 0U;
    Gen32u n1        = 0U;
    Gen32u n2        = 0U;
    Gen32u zeroCount = 0U;

    std::vector<Gen32u> pLiteralLengthCodes;
    std::vector<Gen32u> pCodeLengthCodes = {7U, 0U, 6U, 6U, 7U, 7U};

    if (130U > header_bit_size || 1520U < header_bit_size) { return GEN_ERR; }

    header_bit_size -= 118U;

    num1314 = (int)(header_bit_size / 6U);
    cl12    = 2U + header_bit_size + num1314 * 6U;

    zeroCount = 255U - num1314;

    *pPossibleLiteralCount = num1314;

    {
        Gen32u code = 1U;
        for (; code < num1314; code += 2U) {
            pLiteralLengthCodes.push_back(13U);
            pLiteralLengthCodes.push_back(14U);
        }
        if (num1314 & 1U) { pLiteralLengthCodes.push_back(13U); }

        n1 = (int)(zeroCount / 2U);
        n2 = zeroCount - n1;

        for (Gen32u i = 0U; i < n1; i++) {
            pLiteralLengthCodes.push_back(0U);
        }

        pLiteralLengthCodes.push_back(15U);

        for (Gen32u i = 0U; i < n2; i++) {
            pLiteralLengthCodes.push_back(0U);
        }

        pLiteralLengthCodes.push_back(12U);
    }

    pCodeLengthCodes[1U] = cl12;

    TestConfigurator::declareVectorToken(LL_VECTOR, pLiteralLengthCodes.data(), (int)pLiteralLengthCodes.size());
    TestConfigurator::declareVectorToken(CL_VECTOR, pCodeLengthCodes.data(), (int)pCodeLengthCodes.size());

    return GEN_OK;
}

GenStatus gz_generator::LargeHeaderConfigurator::writeBlock(Gen32u blockByteSize) {
    std::vector<Gen32u> pLiteralVector;
    *m_config << "block\nll_lens 0 1 2 3 4 5 6 7 8 0*247 8\n";

    if (blockByteSize == 136U) { return GEN_ERR; }

    blockByteSize -= 136U;

    while (blockByteSize >= 8U) {
        pLiteralVector.push_back(8U);
        blockByteSize -= 8U;
    }
    if (blockByteSize > 0U) { pLiteralVector.push_back(blockByteSize); }

    TestConfigurator::declareVectorToken(L_VECTOR, pLiteralVector.data(), (Gen32u)pLiteralVector.size());

    return GEN_OK;
}
