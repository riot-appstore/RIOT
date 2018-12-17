/*
 * Copyright (C) 2018 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_registry Registry
 * @ingroup     sys
 * @brief       RIOT Registry module for handling runtime configurations
 * 
 * @{
 * @file
 * 
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 *
 * @}
 */

#ifndef REGISTRY_H
#define REGISTRY_H

#include <stdint.h>
#include "clist.h"

/**
 * @brief Separator character to define hierarchy in configurations names.
 */
#define REGISTRY_NAME_SEPARATOR    '/'

/**
 * @brief Maximum amount of levels of hierarchy in configurations names.
 */
#define REGISTRY_MAX_DIR_DEPTH     8

/**
 * @brief Maximum amount of characters per level in configurations names.
 */
#define REGISTRY_MAX_DIR_NAME_LEN  64

/**
 * @brief Maximum length of a value when converted to string
 */
#define REGISTRY_MAX_VAL_LEN       64

/**
 * @brief Maximum length of a configuration name.
 * @{
 */
#define REGISTRY_MAX_NAME_LEN      ((REGISTRY_MAX_DIR_NAME_LEN * \
                                    REGISTRY_MAX_DIR_DEPTH) + \
                                    (REGISTRY_MAX_DIR_DEPTH - 1))
/** @} */

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
    char *(*hndlr_get)(int argc, char **argv, char *val, int val_len_max,
                       void *context);
    int (*hndlr_set)(int argc, char **argv, char *val, void *context);
    int (*hndlr_commit)(void *context);
    int (*hndlr_export)(int (*export_func)(const char *name, char *val),
                        int argc, char **argv, void *context);
    void *context;
} registry_handler_t;

extern clist_node_t registry_handlers;

/**
 * @brief Initializes the RIOT Registry and the store modules.
 */
void registry_init(void);

/**
 * @brief Initializes the store module.
 */
void registry_store_init(void);

/**
 * @brief Registers a new group of handlers for a configuration group.
 * 
 * @param[in] handler Pointer to the handlers structure.
 */
void registry_register(registry_handler_t *handler);

/**
 * @brief Registers a new store as a source of configurations. Multiple stores
 *        can be configured as sources at the same time. Configurations will be
 *        loaded from all of them.
 * 
 * @param[in] src Pointer to the store to register as source.
 */
void registry_src_register(registry_store_t *src);

/**
 * @brief Registers a new store as a destination for saving configurations.
 *        Only one store can be registered as destination at a time. If a
 *        previous store had been registered before it will be replaced by the
 *        new one.
 * 
 * @param[in] dst Pointer to the store to register
 */
void registry_dst_register(registry_store_t *dst);

/**
 * @brief Searches through the registered handlers of configurations by name.
 * 
 * @note The name of a group of configurations should not contain the
 * @ref REGISTRY_NAME_SEPARATOR.
 * 
 * @param[in] name String containing the name to look for.
 * @return registry_handler_t* Pointer to the handlers. NULL if not found.
 */
registry_handler_t *registry_handler_lookup(char *name);

/**
 * @brief Parses the name of a configuration parameter by spliting it into the
 *        hierarchy levels and then looks for its handlers.
 * 
 * @param[in] name Name of the parameter to be parsed
 * @param[out] name_argc Pointer to store the amount of parts of the name after
 *             splitting.
 * @param[out] name_argv Array of pointers to the parts that compose the name
 *             after splitting.
 * @return registry_handler_t* Pointer to the handlers. NULL if not found.
 */
registry_handler_t *registry_handler_parse_and_lookup(char *name,
                                                      int *name_argc,
                                                      char *name_argv[]);

/**
 * @brief Parses the name of a configuration parameter by splitting it into the
 *        hierarchy levels.
 * 
 * @param[in] name Name of the parameter to be parsed
 * @param[out] name_argc Pointer to store the amount of parts of the name after
 *             splitting.
 * @param[out] name_argv Array of pointers to the parts that compose the name
 *             after splitting.
 */
void registry_parse_name(char *name, int *name_argc, char *name_argv[]);

/**
 * @brief Sets the value of a parameter that belongs to a configuration group.
 * 
 * @param[in] name Name of the parameter to be set
 * @param[in] val_str New value for the parameter
 * @return int EINVAL if handlers could not be found. Otherwise returns the
 *             value of the set handler function.k
 */
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

#endif /* REGISTRY_H */
