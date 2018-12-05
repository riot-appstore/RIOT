#include "registry.h"

typedef struct {
    registry_store_t store;
    const char *file_name;
} registry_file_t;

/* File Registry Storage */
int registry_file_src(registry_file_t *file);

int registry_file_dst(registry_file_t *file);
