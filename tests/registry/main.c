/*
 * Copyright (C) 2018 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @file
 * @brief       Shows the use of the RIOT registry for handling runtime
 *              configurations using multiple store options.
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */

#include <stdio.h>
#include <string.h>
#include "shell.h"
#include "board.h"
#include "registry/registry.h"
#include "registry/store/registry_store.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#ifdef MODULE_REGISTRY_STORE_DUMMY
registry_dummy_t registry_dummy_storage;
#endif /* MODULE_REGISTRY_STORE_DUMMY */

#ifdef MODULE_REGISTRY_STORE_EEPROM
registry_eeprom_t registry_eeprom_storage;
#endif /* MODULE_REGISTRY_STORE_EEPROM */

#ifdef MODULE_REGISTRY_STORE_FILE

#include "mtd.h"
#include "fs/fatfs.h"
/* Flash mount point */
#define FLASH_MOUNT_POINT   "/sda"

static fatfs_desc_t fs_desc = {
    .vol_idx = 0
};

/* provide mtd devices for use within diskio layer of fatfs */
mtd_dev_t *fatfs_mtd_devs[FF_VOLUMES];

#define FS_DRIVER fatfs_file_system

/* this structure defines the vfs mount point:
 *  - fs field is set to the file system driver
 *  - mount_point field is the mount point name
 *  - private_data depends on the underlying file system. For both spiffs and
 *  littlefs, it needs to be a pointer to the file system descriptor */
static vfs_mount_t flash_mount = {
    .fs = &FS_DRIVER,
    .mount_point = FLASH_MOUNT_POINT,
    .private_data = &fs_desc,
};

registry_file_t registry_file_storage = {
    .file_name="/sda/reg"
};
#endif /* MODULE_REGISTRY_STORE_FILE */

#ifndef BYTES_LENGTH
#define BYTES_LENGTH    16
#endif

/* These are the 'configuration parameters' */
int test_opt1 = 0;
int test_opt2 = 1;
unsigned char test_bytes[BYTES_LENGTH] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                          0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                          0xAA, 0xAA, 0xAA, 0xAA};

char *get_handler(int argc, char **argv, char *val, int val_len_max, void *ctx)
{
    (void)ctx;

    if (argc) {
        if (!strcmp("opt1", argv[0])) {
        return registry_str_from_value(REGISTRY_TYPE_INT8, (void *)&test_opt1,
                                       val, val_len_max);
        }
        else if (!strcmp("opt2", argv[0])) {
            return registry_str_from_value(REGISTRY_TYPE_INT8, (void *)&test_opt2,
                                        val, val_len_max);
        }
        else if (!strcmp("bytes", argv[0])) {
            return registry_str_from_bytes((void *)test_bytes, BYTES_LENGTH,
                                           val, val_len_max);

        }
    }
    return NULL;
}

int set_handler(int argc, char **argv, char *val, void *ctx)
{
    (void)ctx;
    const char buf[BYTES_LENGTH];
    int res = 0;
    int len = sizeof(test_bytes);

    if (argc) {
        DEBUG("[set_handler] Setting %s to %s\n", argv[0], val);
        if (!strcmp("opt1", argv[0])) {
            return registry_value_from_str(val, REGISTRY_TYPE_INT8,
                                           (void *) &test_opt1, 0);
        }
        else if (!strcmp("opt2", argv[0])) {
            return registry_value_from_str(val, REGISTRY_TYPE_INT8,
                                           (void *) &test_opt2, 0);
        }
        else if (!strcmp("bytes", argv[0])) {
            res = registry_bytes_from_str(val, (void *)buf, &len);
            if (res) {
                DEBUG("[set_handler] Error while parsing base64");
                return res;
            }

            if (len != BYTES_LENGTH) {
                DEBUG("[set_handler] Error less bytes than needed were passed");
                return -1;
            }
            memcpy((void *)test_bytes, (void *)buf, BYTES_LENGTH);
        }
    }
    return -1;
}

int export_handler(int (*export_func)(const char *name, char *val), int argc,
                   char **argv, void *ctx)
{
    (void)argv;
    (void)argc;
    (void)ctx;
    char buf[REGISTRY_MAX_VAL_LEN];

    if (!argc) {
        // We have to print every parameter
        registry_str_from_value(REGISTRY_TYPE_INT8, (void *)&test_opt1, buf,
                                sizeof(buf));
        export_func("app/opt1", buf);
        registry_str_from_value(REGISTRY_TYPE_INT8, (void *)&test_opt2, buf,
                                sizeof(buf));
        export_func("app/opt2", buf);
        registry_str_from_bytes((void *)test_bytes, BYTES_LENGTH, buf,
                                sizeof(buf));
        export_func("app/bytes", buf);
        return 0;
    }
    else {
        if (!strcmp("opt1", argv[0])) {
            registry_str_from_value(REGISTRY_TYPE_INT8, (void *)&test_opt1, buf,
                                    sizeof(buf));
            export_func("app/opt1", buf);
            return 0;
        }
        else if (!strcmp("opt2", argv[0])) {
            registry_str_from_value(REGISTRY_TYPE_INT8, (void *)&test_opt2, buf,
                                    sizeof(buf));
            export_func("app/opt2", buf);
            return 0;
        }
        else if (!strcmp("bytes", argv[0])) {
            registry_str_from_bytes((void *)test_bytes, BYTES_LENGTH, buf,
                                    sizeof(buf));
            export_func("app/bytes", buf);
        }
    }
    return 1;
}


int print_parameter(const char *name, char *val)
{
    printf("%s = %s\n", name, val);
    return 0;
}

registry_handler_t handler = {
    .name = "app",
    .hndlr_get = get_handler,
    .hndlr_set = set_handler,
    .hndlr_export = export_handler
};

int cmd_get(int argc, char **argv)
{
    char buf[REGISTRY_MAX_VAL_LEN];
    int buf_len = REGISTRY_MAX_VAL_LEN;
    char *res;

    if (argc != 2) {
        (void) argv;
        printf("usage: %s <param>\n", argv[0]);
        return 1;
    }

    res = registry_get_value(argv[1], buf, buf_len);

    if (res == NULL) {
        DEBUG("Error: Parameter does not exist\n");
        return 1;
    }
    DEBUG("%s\n", buf);
    return 0;
}

int cmd_list(int argc, char **argv)
{
    if (argc == 1) {
        registry_export(print_parameter, NULL);
    }
    else if (argc == 2) {
        registry_export(print_parameter, argv[1]);
    }
    else {
        printf("usage: %s [module:parameter]\n", argv[0]);
    }
    return 0;
}

int cmd_set(int argc, char **argv)
{
    if (argc != 3) {
        printf("usage: %s <param> <value>\n", argv[0]);
        return 1;
    }
    return registry_set_value(argv[1], argv[2]);
}

int cmd_save(int argc, char **argv)
{
    if (argc != 1) {
        printf("usage: %s\n", argv[0]);
        return 1;
    }
    return registry_save();
}

int cmd_load(int argc, char **argv)
{
    if (argc != 1) {
        printf("usage: %s\n", argv[0]);
        return 1;
    }
    return registry_load();
}

static void _dump_cb(char *name, char *val, void *cb_arg)
{
    (void)cb_arg;
    printf("%s \t %s\n", name, val);
}

int cmd_dump(int argc, char **argv)
{
    if (argc != 1) {
        printf("usage: %s\n", argv[0]);
        return 1;
    }
    DEBUG("Dumping storage...\n");

    /* This is just for debuging pourposes, storage facilities are not mean to
     * be called directly */
#if defined(MODULE_REGISTRY_STORE_DUMMY)
    registry_dummy_storage.store.itf->load(&registry_dummy_storage.store,
                                           _dump_cb, NULL);
#elif defined(MODULE_REGISTRY_STORE_EEPROM)
    registry_eeprom_storage.store.itf->load(&registry_eeprom_storage.store,
                                           _dump_cb, NULL);
#elif defined(MODULE_REGISTRY_STORE_FILE)
    registry_file_storage.store.itf->load(&registry_file_storage.store,
                                           _dump_cb, NULL);
#else
    printf("ERROR: No store defined\n");
    return 1;
#endif
    return 0;
}

static int cmd_test_bytes(int argc, char **argv) {
    if (argc != 1) {
        printf("usage: %s\n", argv[0]);
        return 1;
    }
    printf("Sizeof test_bytes: %d\n", sizeof(test_bytes));
    for (unsigned int i = 0; i < sizeof(test_bytes); i++) {
        printf("%2x ",test_bytes[i]);
    }
    puts("");
    return 0;
}

static const shell_command_t shell_commands[] = {
    { "get", "get a parameter value", cmd_get },
    { "set", "set a parameter value", cmd_set },
    { "list", "list parameters", cmd_list },
    { "save", "save all parameters", cmd_save },
    { "load", "load stored configurations", cmd_load },
    { "dump", "dumps everything in storage", cmd_dump },
    { "test_bytes", "dumps the bytes variable in HEX", cmd_test_bytes },
    { NULL, NULL, NULL }
};

int main(void)
{

    registry_init();
    registry_register(&handler);

#if defined(MODULE_REGISTRY_STORE_DUMMY)
    DEBUG("Using dummy registry store\n");
    registry_dummy_src(&registry_dummy_storage);
    registry_dummy_dst(&registry_dummy_storage);
#elif defined(MODULE_REGISTRY_STORE_EEPROM)
    DEBUG("Using EEPROM registry store\n");
    registry_eeprom_src(&registry_eeprom_storage);
    registry_eeprom_dst(&registry_eeprom_storage);
#elif defined(MODULE_REGISTRY_STORE_FILE)
    DEBUG("Using File registry store\n");
#if MODULE_MTD_SDCARD
    fatfs_mtd_devs[0] = MTD_0;
#endif /* MODULE_MTD_SDCARD */

    if (vfs_mount(&flash_mount) < 0) {
        DEBUG("[registry_file_src] Failed to mount FS\n");
    } 
    registry_file_src(&registry_file_storage);
    registry_file_dst(&registry_file_storage);
#else
#error "You should choose a store for registry"
#endif

    registry_load();

    /* define buffer to be used by the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];

    /* define own shell commands */
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
