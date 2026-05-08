/*
 * Copyright 2021-2024 Amazon.com, Inc. or its affiliates.  All rights reserved.
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

#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_storage_kv_internal_group_ids.h>
#include <storage_kv_keys.h>

#include <unity.h>
#include <unity_fixture.h>
#include <string.h>

#define DEV_ID_SZ 5
#define DEV_VER_SZ 2
#define PROT_VER_SZ 2

TEST_GROUP(STORAGE_KV);

TEST_SETUP(STORAGE_KV)
{
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, sid_pal_storage_kv_init());
}

TEST_TEAR_DOWN(STORAGE_KV)
{
    sid_error_t ret;

    ret = sid_pal_storage_kv_group_delete(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
}

TEST_GROUP_RUNNER(STORAGE_KV)
{
    RUN_TEST_CASE(STORAGE_KV, test_record_set_get);
    RUN_TEST_CASE(STORAGE_KV, test_record_set_twice);
    RUN_TEST_CASE(STORAGE_KV, test_record_set_delete_set);
    RUN_TEST_CASE(STORAGE_KV, test_record_get_error);
    RUN_TEST_CASE(STORAGE_KV, test_record_delete);
    RUN_TEST_CASE(STORAGE_KV, test_record_delete_error);
    RUN_TEST_CASE(STORAGE_KV, test_record_group_delete);
    RUN_TEST_CASE(STORAGE_KV, test_record_group_delete_error);
    RUN_TEST_CASE(STORAGE_KV, test_record_set_invalid_args);
}

/* Helper function to populate records to test deletion */
static void set_record()
{
    sid_error_t ret;
    uint8_t id[DEV_ID_SZ] = {0x99, 0x88};

    ret = sid_pal_storage_kv_record_set(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DBG_DEV_ID, id,
                                        DEV_ID_SZ);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
}

static uint32_t roundup_to_word(uint32_t v)
{
    return (v + 3u) / 4u * 4u;
}

/* Test basic record set and get */
TEST(STORAGE_KV, test_record_set_get)
{
    sid_error_t ret;
    uint8_t dev_id[DEV_ID_SZ] = {0xfe, 0xed, 0xfa, 0xce, 0xa1};
    uint8_t id[DEV_ID_SZ];
    uint8_t dev_ver[DEV_VER_SZ] = {0x5, 0x6};
    uint8_t ver[DEV_VER_SZ];
    uint32_t len;

    ret = sid_pal_storage_kv_record_set(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DBG_DEV_ID, dev_id,
                                        DEV_ID_SZ);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

    ret = sid_pal_storage_kv_record_set(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DEV_VERSION, dev_ver,
                                        DEV_VER_SZ);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

    ret = sid_pal_storage_kv_record_get_len(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DBG_DEV_ID, &len);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
    TEST_ASSERT_GREATER_OR_EQUAL(DEV_ID_SZ, len);
    TEST_ASSERT_LESS_OR_EQUAL(roundup_to_word(DEV_ID_SZ), len);

    ret = sid_pal_storage_kv_record_get(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DBG_DEV_ID, id, len);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
    TEST_ASSERT_EQUAL(0, memcmp(dev_id, id, DEV_ID_SZ));

    ret =
        sid_pal_storage_kv_record_get_len(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DEV_VERSION, &len);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
    TEST_ASSERT_GREATER_OR_EQUAL(DEV_VER_SZ, len);
    TEST_ASSERT_LESS_OR_EQUAL(roundup_to_word(DEV_VER_SZ), len);

    ret = sid_pal_storage_kv_record_get(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DEV_VERSION, ver,
                                        DEV_VER_SZ);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
    TEST_ASSERT_EQUAL(0, memcmp(dev_ver, ver, DEV_VER_SZ));
}

/* Test setting record twice updates the record */
TEST(STORAGE_KV, test_record_set_twice)
{
    sid_error_t ret;
    uint8_t dev_id[DEV_ID_SZ] = {0xfe, 0xed, 0xfa, 0xce, 0xa1};
    uint8_t dev_id2[DEV_ID_SZ] = {0xca, 0xfe, 0xf0, 0x0d, 0x5e};
    uint8_t id[DEV_ID_SZ];
    uint32_t len;

    ret = sid_pal_storage_kv_record_set(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DBG_DEV_ID, dev_id,
                                        DEV_ID_SZ);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

    ret = sid_pal_storage_kv_record_set(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DBG_DEV_ID, dev_id2,
                                        DEV_ID_SZ);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

    ret = sid_pal_storage_kv_record_get_len(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DBG_DEV_ID, &len);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
    TEST_ASSERT_GREATER_OR_EQUAL(DEV_ID_SZ, len);
    TEST_ASSERT_LESS_OR_EQUAL(roundup_to_word(DEV_ID_SZ), len);

    ret = sid_pal_storage_kv_record_get(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DBG_DEV_ID, id, len);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
    TEST_ASSERT_EQUAL(0, memcmp(dev_id2, id, DEV_ID_SZ));
}

/* Test re-setting record after a delete */
TEST(STORAGE_KV, test_record_set_delete_set)
{
    sid_error_t ret;
    uint8_t dev_id[DEV_ID_SZ] = {0xfe, 0xed, 0xfa, 0xce, 0xa1};
    uint8_t dev_id2[DEV_ID_SZ] = {0xca, 0xfe, 0xf0, 0x0d, 0x5e};
    uint8_t id[DEV_ID_SZ];
    uint32_t len;

    ret = sid_pal_storage_kv_record_set(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DBG_DEV_ID, dev_id,
                                        DEV_ID_SZ);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

    ret = sid_pal_storage_kv_record_delete(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DBG_DEV_ID);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

    ret = sid_pal_storage_kv_record_set(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DBG_DEV_ID, dev_id2,
                                        DEV_ID_SZ);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

    ret = sid_pal_storage_kv_record_get_len(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DBG_DEV_ID, &len);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
    TEST_ASSERT_GREATER_OR_EQUAL(DEV_ID_SZ, len);
    TEST_ASSERT_LESS_OR_EQUAL(roundup_to_word(DEV_ID_SZ), len);

    ret = sid_pal_storage_kv_record_get(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DBG_DEV_ID, id, len);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
    TEST_ASSERT_EQUAL(0, memcmp(dev_id2, id, DEV_ID_SZ));
}

/* Test reading non-existent record */
TEST(STORAGE_KV, test_record_get_error)
{
    sid_error_t ret;
    uint8_t ver[DEV_VER_SZ];

    ret = sid_pal_storage_kv_record_get(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_PROTOCOL_VERSION, ver,
                                        PROT_VER_SZ);
    TEST_ASSERT_EQUAL(SID_ERROR_NOT_FOUND, ret);
}

/* Test record is properly deleted */
TEST(STORAGE_KV, test_record_delete)
{
    sid_error_t ret;
    uint8_t id[DEV_ID_SZ];

    set_record();

    ret = sid_pal_storage_kv_record_delete(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DBG_DEV_ID);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

    ret = sid_pal_storage_kv_record_get(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DBG_DEV_ID, id,
                                        DEV_ID_SZ);
    TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);
}

/* Test deleting non-existent record */
TEST(STORAGE_KV, test_record_delete_error)
{
    sid_error_t ret;

    ret = sid_pal_storage_kv_record_delete(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_PROTOCOL_VERSION);
    TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);
}

/* Test deleting a group removes records */
TEST(STORAGE_KV, test_record_group_delete)
{
    sid_error_t ret;
    uint8_t id[DEV_ID_SZ];

    set_record();

    ret = sid_pal_storage_kv_group_delete(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

    ret = sid_pal_storage_kv_record_get(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DBG_DEV_ID, id,
                                        DEV_ID_SZ);
    TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);
}

/* Test deleting group twice */
TEST(STORAGE_KV, test_record_group_delete_error)
{
    sid_error_t ret;

    set_record();

    ret = sid_pal_storage_kv_group_delete(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);

    ret = sid_pal_storage_kv_group_delete(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID);
    TEST_ASSERT_EQUAL(SID_ERROR_NONE, ret);
}

/* Test setting with invalid args */
TEST(STORAGE_KV, test_record_set_invalid_args)
{
    uint8_t id[DEV_ID_SZ] = {0};
    sid_error_t ret =
        sid_pal_storage_kv_record_set(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DBG_DEV_ID, id, 0);
    TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);

    ret = sid_pal_storage_kv_record_set(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID, STORAGE_KV_DBG_DEV_ID, NULL,
                                        sizeof(id));
    TEST_ASSERT_NOT_EQUAL(SID_ERROR_NONE, ret);
}

int sid_pal_storage_kv_tests_run(void)
{
    RUN_TEST_GROUP(STORAGE_KV);

    return 0;
}
