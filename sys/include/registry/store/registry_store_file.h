#ifndef REGISTRY_STORE_FILE_H
#define REGISTRY_STORE_FILE_H

#include "registry/registry.h"

#define REGISTRY_EXTRA_LEN         3 // '=' '\0\n'

typedef struct {
    registry_store_t store;
    const char *file_name;
} registry_file_t;

/* File Registry Storage */
int registry_file_src(registry_file_t *file);

int registry_file_dst(registry_file_t *file);

#endif /* REGISTRY_STORE_FILE_H */
