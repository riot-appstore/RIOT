/*
 * Copyright (C) 2018 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @brief       Registry module unit tests
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 *
 * @}
 */
#include <stdint.h>
#include <string.h>

#include "embUnit.h"
#include "registry.h"

#include <stdio.h>

#define TEST_APP_TREE_L1        "app"
#define TEST_APP_TREE_L2        "id"
#define TEST_APP_TREE_L3        "serial"
#define TEST_APP_TREE_LEN       3

#define TEST_GNRC_TREE_L1       "gnrc"
#define TEST_GNRC_TREE_L2       "pkt_buffer_len"
#define TEST_GNRC_TREE_LEN      2

#define TEST_SET_VALUE_VAL      "abcd"

static void test_param_lookup(void)
{
    registry_handler_t h1, h2;
    registry_handler_t *hp1, *hp2;
    h1.name = TEST_APP_TREE_L1;
    h2.name = TEST_GNRC_TREE_L1;

    registry_init();

    registry_register(&h1);
    registry_register(&h2);

    hp1 = registry_handler_lookup(h1.name);
    hp2 = registry_handler_lookup("wrong_key");

    TEST_ASSERT(hp1 == &h1);
    TEST_ASSERT_NULL(hp2);
    TEST_ASSERT_EQUAL_STRING(h1.name, hp1->name);
}

static void test_parse_name(void)
{
    char name[] = TEST_APP_TREE_L1 REGISTRY_NAME_SEPARATOR TEST_APP_TREE_L2 \
                  REGISTRY_NAME_SEPARATOR TEST_APP_TREE_L3;
    int argc;
    char *argv[REGISTRY_MAX_DIR_DEPTH];
    char *tree[] = {TEST_APP_TREE_L1, TEST_APP_TREE_L2, TEST_APP_TREE_L3};

    registry_init();

    registry_parse_name(name, &argc, argv);

    TEST_ASSERT_EQUAL_INT(TEST_APP_TREE_LEN, argc);
    for (int i = 0; i < TEST_APP_TREE_LEN; i++) {
        TEST_ASSERT_EQUAL_STRING(tree[i], argv[i]);
    }
}

static void test_parse_name_and_lookup(void)
{
    registry_handler_t h = {
        .name = TEST_APP_TREE_L1
    };
    registry_handler_t *hp;
    char name[] = TEST_APP_TREE_L1 REGISTRY_NAME_SEPARATOR TEST_APP_TREE_L2 \
                  REGISTRY_NAME_SEPARATOR TEST_APP_TREE_L3;
    int argc;
    char *argv[REGISTRY_MAX_DIR_DEPTH];

    registry_init();

    registry_register(&h);
    hp = registry_handler_parse_and_lookup(name, &argc, argv);

    TEST_ASSERT(&h == hp);
}

int _set_value_argc = 0;
char *_set_value_argv[REGISTRY_MAX_DIR_DEPTH];
char *_set_value_val;
static int _set_cb(int argc, char **argv, char *val)
{
    _set_value_argc = argc;
    _set_value_val = val;
    memcpy(_set_value_argv, argv, REGISTRY_MAX_DIR_DEPTH);
    return 0;
}

static void test_set_value(void)
{
    registry_handler_t h = {
        .name = TEST_APP_TREE_L1,
        .hndlr_set = _set_cb
    };

    char name[] = TEST_APP_TREE_L1 REGISTRY_NAME_SEPARATOR TEST_APP_TREE_L2 \
                  REGISTRY_NAME_SEPARATOR TEST_APP_TREE_L3;
    char *tree[] = {TEST_APP_TREE_L2, TEST_APP_TREE_L3};

    registry_init();
    registry_register(&h);
    registry_set_value(name, TEST_SET_VALUE_VAL);

    TEST_ASSERT_EQUAL_INT(TEST_APP_TREE_LEN - 1, _set_value_argc);
    TEST_ASSERT_EQUAL_STRING(TEST_SET_VALUE_VAL, _set_value_val);
    for (int i = 0; i < TEST_APP_TREE_LEN - 1; i++) {
        TEST_ASSERT_EQUAL_STRING(tree[i], _set_value_argv[i]);
    }
}

int _get_value_argc = 0;
char *_get_value_argv[REGISTRY_MAX_DIR_DEPTH];
char _get_value_response[] = TEST_SET_VALUE_VAL;
static char *_get_cb(int argc, char **argv, char *val, int val_len_max)
{
    (void)val;
    (void)val_len_max;
    _get_value_argc = argc;
    memcpy(_get_value_argv, argv, REGISTRY_MAX_DIR_DEPTH);
    return _get_value_response;
}

static void test_get_value(void)
{
    registry_handler_t h1 = {
        .name = TEST_APP_TREE_L1,
        .hndlr_get = _get_cb
    };

    registry_handler_t h2 = {
        .name = TEST_GNRC_TREE_L1
    };

    /* Name of the 'app' parameter to get the value */
    char name1[] = TEST_APP_TREE_L1 REGISTRY_NAME_SEPARATOR TEST_APP_TREE_L2 \
                   REGISTRY_NAME_SEPARATOR TEST_APP_TREE_L3;

    /* Name of the 'gnrc' parameter to get the value */
    char name2[] = TEST_GNRC_TREE_L1 REGISTRY_NAME_SEPARATOR TEST_GNRC_TREE_L2;

    /* Sections of the 'app' tree that should be passed to the 'get'
     * callback.
     */
    char *tree[] = {TEST_APP_TREE_L2, TEST_APP_TREE_L3};

    char *res1, *res2;

    /* Re initialize module and register two handlers */
    registry_init();
    registry_register(&h1);
    registry_register(&h2);

    /* Try to get the parameters values. This should call the 'get' callback 
     * function if any.
     */
    res1 = registry_get_value(name1, NULL, 0);
    res2 = registry_get_value(name2, NULL, 0);

    /* The 'get' callback should be called without the L1 level of the name
     * tree.
     */
    TEST_ASSERT_EQUAL_INT(TEST_APP_TREE_LEN - 1, _get_value_argc);
    TEST_ASSERT_EQUAL_STRING(TEST_SET_VALUE_VAL, res1);
    for (int i = 0; i < TEST_APP_TREE_LEN - 1; i++) {
        TEST_ASSERT_EQUAL_STRING(tree[i], _get_value_argv[i]);
    }
    /* If there is no 'get' callback function registered it should return
     * NULL.
     */
    TEST_ASSERT_NULL(res2);
}

int _commit_cb_calls = 0;
static int _commit_cb(void)
{
    _commit_cb_calls++;
    return 0;
}

static void test_commit(void)
{
    registry_handler_t h1 = {
        .name = TEST_APP_TREE_L1,
        .hndlr_commit = _commit_cb
    }, h2 = {
        .name = TEST_GNRC_TREE_L1,
        .hndlr_commit = _commit_cb
    };

    /* Re initialize module and register two handlers */
    registry_init();
    registry_register(&h1);
    registry_register(&h2);

    /* Commit callback is returning 0, this call should too */
    TEST_ASSERT_EQUAL_INT(0, registry_commit(TEST_APP_TREE_L1));
    /* The commit callback should have only been called once */
    TEST_ASSERT_EQUAL_INT(1, _commit_cb_calls);
    _commit_cb_calls = 0;

    /* Commit callback is returning 0, this call should too */
    TEST_ASSERT_EQUAL_INT(0, registry_commit(NULL));
    /* The commit callback should have only been called twice */
    TEST_ASSERT_EQUAL_INT(2, _commit_cb_calls);
}

Test *tests_registry_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_param_lookup),
        new_TestFixture(test_parse_name),
        new_TestFixture(test_parse_name_and_lookup),
        new_TestFixture(test_set_value),
        new_TestFixture(test_get_value),
        new_TestFixture(test_commit),
    };

    EMB_UNIT_TESTCALLER(registry_tests, NULL, NULL, fixtures);

    return (Test *)&registry_tests;
}

void tests_registry(void)
{
    TESTS_RUN(tests_registry_tests());
}
