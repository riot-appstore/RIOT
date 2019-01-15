#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "kernel_defines.h"
#include "clist.h"
#include "registry/registry.h"

registry_store_t *save_dst;
clist_node_t load_srcs;

static void _registry_load_cb(char *name, char *val, void *cb_arg)
{
    (void)cb_arg;
    printf("[registry_store] Setting %s to %s\n", name, val);
    registry_set_value(name, val);
}

void registry_store_init(void)
{
    load_srcs.next = NULL;
}

void registry_src_register(registry_store_t *src)
{
    clist_rpush(&load_srcs, &(src->node));
}

void registry_dst_register(registry_store_t *dst)
{
    save_dst = dst;
}

int registry_load(void)
{
    registry_store_t *src;
    clist_node_t *node = load_srcs.next;

    if (!node) {
        return -1;
    }

    do  {
        src = container_of(node, registry_store_t, node);
        src->itf->load(src, _registry_load_cb, NULL);
    } while (node != load_srcs.next);
    return 0;
}

static void _registry_dup_check_cb(char *name, char *val, void *cb_arg)
{
    registry_dup_check_arg_t *dup_arg = (registry_dup_check_arg_t *)cb_arg;

    if (strcmp(name, dup_arg->name)) {
        return;
    }
    if (!val) {
        if (!dup_arg->val || dup_arg->val[0] == '\0') {
            dup_arg->is_dup = 1;
        }
    }
    else {
        if (dup_arg->val && !strcmp(val, dup_arg->val)) {
            dup_arg->is_dup = 1;
        }
    }
}

int registry_save_one(const char *name, char *value)
{
    registry_store_t *dst = save_dst;
    registry_dup_check_arg_t dup;
    printf("[registry_store] Saving: %s = %s\n",name, value);

    if (!dst) {
        return -ENOENT;
    }

    dup.name = (char *)name;
    dup.val = value;
    dup.is_dup = 0;

    save_dst->itf->load(save_dst, _registry_dup_check_cb, &dup);

    if (dup.is_dup == 1) {
        return 0;
    }

    return dst->itf->save(dst, name, value);
}

int registry_save(void)
{
    registry_handler_t *hndlr;
    clist_node_t *node = registry_handlers.next;

    if (!node) {
        return -1;
    }

    do  {
        hndlr = container_of(node, registry_handler_t, node);
        if (hndlr->hndlr_export) {
            hndlr->hndlr_export(registry_save_one, 0, NULL, hndlr->context);
        }
    } while (node != registry_handlers.next);
    return 0;
}
