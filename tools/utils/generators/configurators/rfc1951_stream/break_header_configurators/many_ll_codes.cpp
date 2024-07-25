/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "many_ll_codes.hpp"

GenStatus gz_generator::ManyLiteralLengthCodesConfigurator::generate() {
    const Gen32u extraLengthCodesCount = 1U + static_cast<Gen32u>(m_random);
    const Gen32u total_codes           = DEFAULT_LL_TABLE_LENGTH + extraLengthCodesCount;

    std::vector<Gen32u> literalLengthCodesTable(total_codes, 0);

    TestConfigurator::makeRandomLengthCodesTable(literalLengthCodesTable.data(), total_codes, MAX_LL_CODE_BIT_LENGTH);

    TestConfigurator::declareExtraLengths();
    TestConfigurator::declareDynamicBlock();

    TestConfigurator::declareVectorToken(LL_ENCODED_VECTOR, literalLengthCodesTable.data(), total_codes);

    TestConfigurator::declareLiteral(1U);
    TestConfigurator::declareLiteral(1U);
    TestConfigurator::declareLiteral(1U);

    TestConfigurator::writeRandomBlock();
    TestConfigurator::declareFinishBlock();

    return GEN_OK;
}
