/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "many_distance_codes_v2.hpp"

GenStatus gz_generator::ManyDistanceCodesConfiguratorVersion2::generate() {
    const Gen32u extraCodesCount = 1U + static_cast<Gen32u>(m_random);
    const Gen32u total_codes     = DEFAULT_D_TABLE_LENGTH + extraCodesCount;

    std::vector<Gen32u> distanceLengthCodesTable(total_codes, 0);

    TestConfigurator::makeRandomLengthCodesTable(distanceLengthCodesTable.data(), total_codes, MAX_D_CODE_BIT_LENGTH);

    TestConfigurator::declareExtraLengths();
    TestConfigurator::declareDynamicBlock();

    TestConfigurator::declareVectorToken(D_ENCODED_VECTOR, distanceLengthCodesTable.data(), total_codes);

    TestConfigurator::declareLiteral(1U);
    TestConfigurator::declareLiteral(1U);
    TestConfigurator::declareLiteral(1U);

    TestConfigurator::writeRandomBlock();
    TestConfigurator::declareFinishBlock();

    return GEN_OK;
}