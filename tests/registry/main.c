/*
 * Copyright (C) 2013 Kaspar Schleiser <kaspar@schleiser.de>
 * Copyright (C) 2013 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @file
 * @brief       shows how to set up own and use the system shell commands.
 *              By typing help in the serial console, all the supported commands
 *              are listed.
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Zakaria Kasmi <zkasmi@inf.fu-berlin.de>
 *
 */

#include <stdio.h>
#include <string.h>

#include "shell.h"
//#include "registry/registry_store_dummy.h"
#include "registry/registry_store_eeprom.h"

registry_eeprom_t registry_eeprom_storage;

int test_opt1 = 0;
int test_opt2 = 1;

char *get_handler(int argc, char **argv, char *val, int val_len_max)
{
    if (argc) {
        if (!strcmp("opt1", argv[0])) {
        return registry_str_from_value(REGISTRY_TYPE_INT8, (void *)&test_opt1,
                                       val, val_len_max);
        }
        else if (!strcmp("opt2", argv[0])) {
            return registry_str_from_value(REGISTRY_TYPE_INT8, (void *)&test_opt2,
                                        val, val_len_max);
        }
    }
    return NULL;
}

int set_handler(int argc, char **argv, char *val)
{
    if (argc) {
        printf("[set_handler] Setting %s to %s\n", argv[0], val);
        if (!strcmp("opt1", argv[0])) {
            return registry_value_from_str(val, REGISTRY_TYPE_INT8,
                                           (void *) &test_opt1, 0);
        }
        else if (!strcmp("opt2", argv[0])) {
            return registry_value_from_str(val, REGISTRY_TYPE_INT8,
                                           (void *) &test_opt2, 0);
        }
    }
    return -1;
}

int export_handler(int (*export_func)(const char *name, char *val), int argc,
                   char **argv)
{
    (void)argv;
    (void)argc;
    char buf[10];
    int buf_len = 10;

    if (!argc) {
        // We have to print every parameter
        registry_str_from_value(REGISTRY_TYPE_INT8, (void *)&test_opt1, buf,
                                buf_len);
        export_func("app/opt1", buf);
        registry_str_from_value(REGISTRY_TYPE_INT8, (void *)&test_opt2, buf,
                                buf_len);
        export_func("app/opt2", buf);
        return 0;
    }
    else {
        if (!strcmp("opt1", argv[0])) {
            registry_str_from_value(REGISTRY_TYPE_INT8, (void *)&test_opt1, buf,
                                    buf_len);
            export_func("app/opt1", buf);
            return 0;
        }
        else if (!strcmp("opt2", argv[0])) {
            registry_str_from_value(REGISTRY_TYPE_INT8, (void *)&test_opt2, buf,
                                    buf_len);
            export_func("app/opt2", buf);
            return 0;
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
        puts("ERROR: Parameter does not exist");
        return 1;
    }
    printf("%s\n", buf);
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
    puts("Dumping storage...");
    registry_eeprom_storage.store.itf->load(&registry_eeprom_storage.store,
                                           _dump_cb, NULL);
    return 0;
}

static const shell_command_t shell_commands[] = {
    { "get", "get a parameter value", cmd_get },
    { "set", "set a parameter value", cmd_set },
    { "list", "list parameters", cmd_list },
    { "save", "save all parameters", cmd_save },
    { "load", "load stored configurations", cmd_load },
    { "dump", "dumps everything in storage", cmd_dump },
    { NULL, NULL, NULL }
};

int main(void)
{
    registry_init();
    registry_register(&handler);

    registry_eeprom_src(&registry_eeprom_storage);
    registry_eeprom_dst(&registry_eeprom_storage);

    registry_load();

    printf("test_shell.\n");


    /* define buffer to be used by the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];

    /* define own shell commands */
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
