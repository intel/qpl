/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "gtest/gtest.h"

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tests
 */

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    int status = RUN_ALL_TESTS();

    return status;
}
