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
 * @brief       Registry module
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 *
 * @}
 */

#ifndef REGISTRY_H
#define REGISTRY_H

#include <stdint.h>
#include "clist.h"

#define REGISTRY_NAME_SEPARATOR    "/"
#define REGISTRY_MAX_DIR_DEPTH     8
#define REGISTRY_VAL_SEPARATOR     '\0'
#define REGISTRY_MAX_NAME_LEN       64
#define REGISTRY_MAX_VAL_LEN        64

typedef enum {
    REGISTRY_TYPE_NONE = 0,
    REGISTRY_TYPE_INT8,
    REGISTRY_TYPE_INT16,
    REGISTRY_TYPE_INT32,
    REGISTRY_TYPE_INT64,
    REGISTRY_TYPE_STRING,
    REGISTRY_TYPE_BYTES,
    REGISTRY_TYPE_FLOAT,
    REGISTRY_TYPE_DOUBLE,
    REGISTRY_TYPE_BOOL,
} registry_type_t;

typedef void (*load_cb_t)(char *name, char *val, void *cb_arg);

typedef struct {
    char *name;
    char *val;
    int is_dup;
} registry_dup_check_arg_t;

typedef struct {
    clist_node_t node;
    const struct registry_store_itf *itf;
} registry_store_t;

typedef struct registry_store_itf {
    int (*load)(registry_store_t *store, load_cb_t cb, void *cb_arg);
    int (*save_start)(registry_store_t *store);
    int (*save)(registry_store_t *store, const char *name, const char *value);
    int (*save_end)(registry_store_t *store);
} registry_store_itf_t;

typedef struct {
    clist_node_t node;
    char *name;
    char *(*hndlr_get)(int argc, char **argv, char *val, int val_len_max);
    int (*hndlr_set)(int argc, char **argv, char *val);
    int (*hndlr_commit)(void);
    int (*hndlr_export)(int (*export_func)(const char *name, char *val),
                        int argc, char **argv);
} registry_handler_t;

extern clist_node_t registry_handlers;

void registry_init(void);

void registry_store_init(void);

void registry_register(registry_handler_t *handler);

void registry_src_register(registry_store_t *src);

void registry_dst_register(registry_store_t *dst);

registry_handler_t *registry_handler_lookup(char *name);

registry_handler_t *registry_handler_parse_and_lookup(char *name,
                                                      int *name_argc,
                                                      char *name_argv[]);

int registry_parse_name(char *name, int *name_argc, char *name_argv[]);

int registry_set_value(char *name, char *val_str);

char *registry_get_value(char *name, char *buf, int buf_len);

int registry_commit(char *name);

int registry_value_from_str(char *val_str, registry_type_t type, void *vp,
                            int maxlen);

int registry_bytes_from_str(char *val_str, void *vp, int *len);

char *registry_str_from_value(registry_type_t type, void *vp, char *buf,
                              int buf_len);

char *registry_str_from_bytes(void *vp, int vp_len, char *buf, int buf_len);

int registry_load(void);

int registry_save(void);

int registry_save_one(const char *name, char *val);

int registry_export(int (*export_func)(const char *name, char *val),
                    char *name);

#endif
