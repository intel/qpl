/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "large_header.hpp"
#include <math.h>

#define BITS_IN_QWORD   64u
#define BITS_IN_BYTE    8u

GenStatus gz_generator::LargeHeaderConfigurator::generate()
{
    GenStatus status = GEN_OK;
    Gen32u startingSize = 0u;
    Gen32u startingQWords = 0u;
    Gen32u startingBits = 0u;
    Gen32u endingSize = 0u;
    Gen32u endingBits = 0u;
    Gen32u middleQWords = 0u;
    Gen32u middleBytes = 0u;
    Gen32u possibleLiteralCount = 0u;

    qpl::test::random random(0u, 0u, m_seed);

    random.set_range(1u, 8u);

    startingSize = 8u * static_cast<Gen32u>(random);
    endingSize = 8u * static_cast<Gen32u>(random);

    if (0.99f > static_cast<float>(m_random))
    {
        random.set_range(3u, 34u);
        startingQWords = static_cast<Gen32u>(random);
        startingBits = BITS_IN_QWORD * startingQWords;

        random.set_range(0u, startingSize - 1u);
        startingBits += static_cast<Gen32u>(random);

        status = LargeHeaderConfigurator::writeBlock(startingBits);
        if (status)
        {
            return status;
        }
        startingBits &= BITS_IN_QWORD - 1u;
    }
    else
    {
        startingBits = 0u;
        startingQWords = 0u;
    }


    startingBits = startingSize - startingBits;

    if (0.5f > static_cast<float>(m_random))
    {
        random.set_range(0u, 7u);
    }
    else
    {
        random.set_range(0u, endingSize - 1u);
    }

    endingBits = static_cast<Gen32u>(random) + 1u;


    middleQWords = 32u;
    random.set_range(middleQWords, 8u * middleQWords);
    middleBytes = static_cast<Gen32u>(random);


    TestConfigurator::declareDynamicBlock();

    status = LargeHeaderConfigurator::writeLiteralLengthCodes(startingBits + middleBytes * BITS_IN_BYTE + endingBits,
                                                     &possibleLiteralCount);

    if (status)
    {
        return status;
    }

    m_randomLiteralCode.set_range(0, possibleLiteralCount - 1u);
    {
        Gen32u literalCount = 0u;
        std::vector<Gen32u> pLiteralVector;

        random.set_range(0u, 8u);
        literalCount = static_cast<Gen32u>(m_random);
        if (literalCount != 0)
        {
            for (Gen32u literal = 0u; literal < literalCount; literal++)
            {
                pLiteralVector.push_back(static_cast<Gen32u>(m_randomLiteralCode));
            }
            TestConfigurator::declareVectorToken(L_VECTOR, pLiteralVector.data(), (Gen32u) pLiteralVector.size());

            *m_config << "l 0*" + std::to_string(literalCount) + "\n";
        }
    }

    TestConfigurator::declareFinishBlock();
    return GEN_OK;
}

GenStatus
gz_generator::LargeHeaderConfigurator::writeLiteralLengthCodes(Gen32u header_bit_size, Gen32u *pPossibleLiteralCount)
{
    Gen32u num13 = 0u;  //TODO Naming!!!
    Gen32u x = 0u;      //TODO Naming!!!
    Gen32u cl13 = 0u;   //TODO Naming!!!

    std::vector<Gen32u> pLiteralLengthCodes;
    std::vector<Gen32u> pDictanceLengthCodes = {14u, 15u, 14u, 15u, 14u, 15u, 14u, 15u, 14u, 15u, 14u, 15u, 14u, 15u,
                                                14u, 15u};

    std::vector<Gen32u> pCodeLengthCodes = {0u, 5u, 7u, 0u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u};



//    Some compilers support case ranges like case x ... y as an extension to the C++ language.
//    GCC 4.9, Clang 3.5.1 and Intel C/C++ Compiler 13.0.1 seem to support it
//    (tried on http://gcc.godbolt.org/). On the other hand, Visual C++ 19 doesn't (tried on http://webcompiler.cloudapp.net/).

    if (2283u < header_bit_size)
    {
        return GEN_ERR;
    }
    else if (2002u <= header_bit_size)
    {
        num13 = caclulateNumberLiteralLengthsCodesEqualTo13(2285u, header_bit_size);
        x = 7u;
    }
    else if (1845u <= header_bit_size)
    {
        num13 = caclulateNumberLiteralLengthsCodesEqualTo13(2128u, header_bit_size);
        x = 6u;
    }
    else if (1688u <= header_bit_size)
    {
        num13 = caclulateNumberLiteralLengthsCodesEqualTo13(1971u, header_bit_size);
        x = 5u;
    }
    else if (1531u <= header_bit_size)
    {
        num13 = caclulateNumberLiteralLengthsCodesEqualTo13(1814, header_bit_size);
        x = 4u;
    }
    else if (1374u <= header_bit_size)
    {
        num13 = caclulateNumberLiteralLengthsCodesEqualTo13(1657u, header_bit_size);
        x = 3u;
    }
    else if (1217u <= header_bit_size)
    {
        num13 = caclulateNumberLiteralLengthsCodesEqualTo13(1500u, header_bit_size);
        x = 2u;
    }
    else if (1060u <= header_bit_size)
    {
        num13 = caclulateNumberLiteralLengthsCodesEqualTo13(1343u, header_bit_size);
        x = 1u;
    }
    else
    {
        return LargeHeaderConfigurator::writeLiteralLengthsCodesForSmallHeader(header_bit_size, pPossibleLiteralCount);
    }

    *pPossibleLiteralCount = LITERALS_HIGH_BORDER + 1u;

    cl13 = 6u - ((int) (pow(header_bit_size, x)) & 1u);

    pLiteralLengthCodes.push_back(13u);
    pLiteralLengthCodes.push_back(12u);

    {
        Gen32u code = 0u;
        for (code = 1; code < num13; code++)
        {
            pLiteralLengthCodes.push_back(13u);
            pLiteralLengthCodes.push_back(15u);
        }
        for (code = code; code < 143u; code++)
        {
            pLiteralLengthCodes.push_back(14u);
            pLiteralLengthCodes.push_back(15u);
        }
    }
    pCodeLengthCodes[0] = cl13;
    pCodeLengthCodes[3] = x;

    TestConfigurator::declareVectorToken(LL_VECTOR, pLiteralLengthCodes.data(), (int) pLiteralLengthCodes.size());
    TestConfigurator::declareVectorToken(D_VECTOR, pDictanceLengthCodes.data(), (int) pDictanceLengthCodes.size());
    TestConfigurator::declareVectorToken(D_VECTOR, pDictanceLengthCodes.data(), (int) pDictanceLengthCodes.size() - 2u);
    TestConfigurator::declareVectorToken(CL_VECTOR, pCodeLengthCodes.data(), (int) pDictanceLengthCodes.size());

    return GEN_OK;
}

GenStatus gz_generator::LargeHeaderConfigurator::writeLiteralLengthsCodesForSmallHeader(Gen32u header_bit_size,
                                                                                        Gen32u *pPossibleLiteralCount)
{
    Gen32u cl12 = 0u;
    Gen32u num1314 = 0u;
    Gen32u n1 = 0u;
    Gen32u n2 = 0u;
    Gen32u zeroCount = 0u;

    std::vector<Gen32u> pLiteralLengthCodes;
    std::vector<Gen32u> pCodeLengthCodes = {7u, 0u, 6u, 6u, 7u, 7u};

    if (130u > header_bit_size || 1520u < header_bit_size)
    {
        return GEN_ERR;
    }

    header_bit_size -= 118u;

    num1314 = (int) (header_bit_size / 6u);
    cl12 = 2u + header_bit_size + num1314 * 6u;

    zeroCount = 255u - num1314;

    *pPossibleLiteralCount = num1314;

    {
        Gen32u code = 0;
        for (code = 1; code < num1314; code += 2u)
        {
            pLiteralLengthCodes.push_back(13u);
            pLiteralLengthCodes.push_back(14u);
        }
        if (num1314 & 1u)
        {
            pLiteralLengthCodes.push_back(13u);
        }

        n1 = (int) (zeroCount / 2u);
        n2 = zeroCount - n1;

        for (Gen32u i = 0; i < n1; i++)
        {
            pLiteralLengthCodes.push_back(0u);
        }

        pLiteralLengthCodes.push_back(15);

        for (Gen32u i = 0; i < n2; i++)
        {
            pLiteralLengthCodes.push_back(0u);
        }

        pLiteralLengthCodes.push_back(12u);
    }

    pCodeLengthCodes[1] = cl12;

    TestConfigurator::declareVectorToken(LL_VECTOR, pLiteralLengthCodes.data(), (int) pLiteralLengthCodes.size());
    TestConfigurator::declareVectorToken(CL_VECTOR, pCodeLengthCodes.data(), (int) pCodeLengthCodes.size());

    return GEN_OK;
}

GenStatus gz_generator::LargeHeaderConfigurator::writeBlock(Gen32u blockByteSize)
{
    std::vector<Gen32u> pLiteralVector;
    *m_config << "block\nll_lens 0 1 2 3 4 5 6 7 8 0*247 8\n";

    if (blockByteSize == 136u)
    {
        return GEN_ERR;
    }

    blockByteSize -= 136u;

    while (blockByteSize >= 8u)
    {
        pLiteralVector.push_back(8u);
        blockByteSize -= 8u;
    }
    if (blockByteSize > 0u)
    {
        pLiteralVector.push_back(blockByteSize);
    }

    TestConfigurator::declareVectorToken(L_VECTOR, pLiteralVector.data(), (Gen32u) pLiteralVector.size());

    return GEN_OK;
}
