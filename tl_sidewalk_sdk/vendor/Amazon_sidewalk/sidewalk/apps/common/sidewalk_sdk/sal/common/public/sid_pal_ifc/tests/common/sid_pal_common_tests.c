/*
 * Copyright 2022-2024 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <sid_pal_common_ifc.h>
#include <unity.h>
#include <unity_fixture.h>

// Entire file is gated by this build flag, as not all platforms use this PAL
#if SID_PAL_COMMON_TESTS_ENABLED

TEST_GROUP(COMMON);

TEST_SETUP(COMMON)
{
}

TEST_TEAR_DOWN(COMMON)
{
}

TEST_GROUP_RUNNER(COMMON)
{
    RUN_TEST_CASE(COMMON, test_common_init)
}

TEST(COMMON, test_common_init)
{
    platform_specific_init_parameters_t platform_init_parameters = {};
    const sid_error_t ret = sid_pal_common_init(&platform_init_parameters);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
}

void sid_pal_common_tests_run(void)
{
    RUN_TEST_GROUP(COMMON);
}

#endif  // SID_PAL_COMMON_TESTS_ENABLED
