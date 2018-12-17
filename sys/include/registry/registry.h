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

/**
 * @brief Data types of the registry
 */
typedef enum {
    REGISTRY_TYPE_NONE = 0,  /**< No type specified */
    REGISTRY_TYPE_INT8,      /**< 8-bits integer */
    REGISTRY_TYPE_INT16,     /**< 16-bits integer */
    REGISTRY_TYPE_INT32,     /**< 32-bits integer */
    REGISTRY_TYPE_INT64,     /**< 64-bits integer */
    REGISTRY_TYPE_STRING,    /**< String */
    REGISTRY_TYPE_BYTES,     /**< Binary data */
    REGISTRY_TYPE_FLOAT,     /**< Float */
    REGISTRY_TYPE_DOUBLE,    /**< Double precision */
    REGISTRY_TYPE_BOOL,      /**< Boolean */
} registry_type_t;

/**
 * @brief Prototype of a callback function for the load action of a store
 * interface
 */
typedef void (*load_cb_t)(char *name, char *val, void *cb_arg);

/**
 * @brief Descriptor used to check duplications in store facilities
 */
typedef struct {
    char *name;
    char *val;
    int is_dup;
} registry_dup_check_arg_t;

/**
 * @brief Store facility descriptor
 */
typedef struct {
    clist_node_t node;
    const struct registry_store_itf *itf;
} registry_store_t;

/**
 * @brief Store facility interface.
 * All store facilities should, at least, implement the load and save actions.
 */
typedef struct registry_store_itf {
    int (*load)(registry_store_t *store, load_cb_t cb, void *cb_arg);
    int (*save_start)(registry_store_t *store);
    int (*save)(registry_store_t *store, const char *name, const char *value);
    int (*save_end)(registry_store_t *store);
} registry_store_itf_t;

/**
 * @brief Handler for configuration groups. Each configuration group should
 * register a handler using the @ref registry_register() function.
 * A handler provides the pointer to get, set, commit and export configuration
 * parameters.
 */
typedef struct {
    clist_node_t node; /**< Linked list node */
    char *name; /**< String representing the configuration group */

    /**
     * @brief Handler to get the current value of a configuration parameter
     * from the configuration group. The handler should return the current value
     * of the configuration parameter as a string copied to val.
     * 
     * @param[in] argc Number of elements in @p argv
     * @param[in] argv Parsed string representing the configuration parameter.
     * @param[out] val Buffer to return the current value
     * @param[in] val_len_max Size of @p val
     * @param[in] context Context of the handler
     * @return Pointer to the buffer containing the current value, NULL on
     * failure
     * 
     * @note The strings passed in @p argv do not contain the string of the name
     * of the configuration group. E.g. If a parameter name is 'group/foo/var'
     * and the name of the group is 'group', argv will contain 'foo' and 'var'.
     */
    char *(*hndlr_get)(int argc, char **argv, char *val, int val_len_max,
                       void *context);

    /**
     * @brief Handler to set a the value of a configuration parameter.
     * The value will be passed as a string in @p val.
     * 
     * @param[in] argc Number of elements in @p argv
     * @param[in] argv Parsed string representing the configuration parameter.
     * @param[out] val Buffer containing the string of the new value
     * @param[in] context Context of the handler
     * @return 0 on success, non-zero on failure
     * 
     * @note The strings passed in @p argv do not contain the string of the name
     * of the configuration group. E.g. If a parameter name is 'group/foo/var'
     * and the name of the group is 'group', argv will contain 'foo' and 'var'.
     */
    int (*hndlr_set)(int argc, char **argv, char *val, void *context);

    /**
     * @brief Handler to apply (commit) the configuration parameters of the
     * group, called once all configurations are loaded from storage.
     * This is useful when a special logic is needed to apply the parameters
     * (e.g. when dependencies exist). This handler should also be called after
     * setting the value of a configuration parameter.
     * 
     * @param[in] context Context of the handler
     * @return 0 on success, non-zero on failure
     */
    int (*hndlr_commit)(void *context);

    /**
     * @brief Handler to export the configuration parameters of the group.
     * The handler must call the @p export_func function with the name and the
     * string representing the value of the requested configuration parameter
     * defined by @p argv.
     * 
     * If @p argc is 0 then the handler should call the @p export_func for every
     * configuration parameter of the group.
     * 
     * @param[in] export_func Export function that should be called by the
     * handler.
     * @param[in] argc Number of elements in @p argv
     * @param[in] argv Parsed string representing the configuration parameter.
     * @param[in] context Context of the handler
     * @return 0 on success, non-zero on failure
     *
     * @note The strings passed in @p argv do not contain the string of the name
     * of the configuration group. E.g. If a parameter name is 'group/foo/var'
     * and the name of the group is 'group', argv will contain 'foo' and 'var'.
     */
    int (*hndlr_export)(int (*export_func)(const char *name, char *val),
                        int argc, char **argv, void *context);

    void *context; /**< Optional context used by the handlers */
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
 * @return Pointer to the handlers. NULL if not found.
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
 * @return Pointer to the handlers. NULL if not found.
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
 * @param[in] name String of the name of the parameter to be set
 * @param[in] val_str New value for the parameter
 * @return -EINVAL if handlers could not be found, otherwise returns the
 *             value of the set handler function.
 */
int registry_set_value(char *name, char *val_str);

/**
 * @brief Gets the current value of a parameter that belongs to a configuration
 *        group, identified by @p name.
 * 
 * @param[in] name String of the name of the parameter to get the value of
 * @param[out] buf Pointer to a buffer to store the current value
 * @param[in] buf_len Length of the buffer to store the current value
 * @return Pointer to the beginning of the buffer
 */
char *registry_get_value(char *name, char *buf, int buf_len);

/**
 * @brief If a @p name is passed it calls the commit handler for that
 *        configuration group. If no @p name is passed the commit handler is
 *        called for every registered configuration group.
 * 
 * @param[in] name Name of the configuration group to commit the changes (can
 * be NULL).
 * @return 0 on success, -EINVAL if the group has not implemented the commit
 * function. 
 */
int registry_commit(char *name);

/**
 * @brief Convenience function to parse a configuration parameter value from
 * a string. The type of the parameter must be known and must not be `bytes`.
 * To parse the string to bytes @ref registry_bytes_from_str() function must be
 * used.
 * 
 * @param[in] val_str Pointer of the string containing the value
 * @param[in] type Type of the parameter to be parsed
 * @param[out] vp Pointer to store the parsed value
 * @param[in] maxlen Maximum length of the output buffer when the type of the
 * parameter is string.
 * @return 0 on success, non-zero on failure
 */
int registry_value_from_str(char *val_str, registry_type_t type, void *vp,
                            int maxlen);

/**
 * @brief Convenience function to parse a configuration parameter value of
 * `bytes` type from a string.
 * 
 * @param[in] val_str Pointer of the string containing the value
 * @param[out] vp Pointer to store the parsed value
 * @param len Length of the output buffer
 * @return 0 on success, non-zero on failure
 */
int registry_bytes_from_str(char *val_str, void *vp, int *len);

/**
 * @brief Convenience function to transform a configuration parameter value into
 * a string, when the parameter is not of `bytes` type, in this case
 * @ref registry_str_from_bytes() should be used. This is used for example to
 * implement the `get` or `export` handlers.
 * 
 * @param[in] type Type of the parameter to be converted
 * @param[in] vp Pointer to the value to be converted
 * @param[out] buf Buffer to store the output string
 * @param[in] buf_len Length of @p buf
 * @return Pointer to the output string
 */
char *registry_str_from_value(registry_type_t type, void *vp, char *buf,
                              int buf_len);

/**
 * @brief Convenience function to transform a configuration parameter value of
 * `bytes` type into a string. This is used for example to implement the `get`
 * or `export` handlers.
 * 
 * @param[in] vp Pointer to the value to be converted
 * @param[in] vp_len Length of @p vp
 * @param[out] buf Buffer to store the output string
 * @param[in] buf_len Length of @p buf
 * @return Pointer to the output string
 */
char *registry_str_from_bytes(void *vp, int vp_len, char *buf, int buf_len);

/**
 * @brief Load all configuration parameters from the registered storage
 * facilities.
 * 
 * @note This should be called after the storage facilities were registered.
 * 
 * @return 0 on success, non-zero on failure 
 */
int registry_load(void);

/**
 * @brief Save all configuration parameters of every configuration group to the
 * registered storage facility.
 * 
 * @return 0 on success, non-zero on failure
 */
int registry_save(void);

/**
 * @brief Save an specific configuration paramter to the registered storage
 * facility, with the provided value (@p val).
 * 
 * @param[in] name String representing the configuration parameter
 * @param[in] val String representing the value of the configuration parameter
 * @return 0 on success, non-zero on failure
 */
int registry_save_one(const char *name, char *val);

/**
 * @brief Export an specific or all configuration parameters using the
 * @p export_func function. If name is NULL then @p export_func is called for
 * every configuration parameter on each configuration group.
 * 
 * @param[in] export_func Exporting function call with the name and current
 * value of an specific or all configuration parameters
 * @param[in] name String representing the configuration parameter. Can be NULL.
 * @return 0 on success, non-zero on failure
 */
int registry_export(int (*export_func)(const char *name, char *val),
                    char *name);

#endif /* REGISTRY_H */
