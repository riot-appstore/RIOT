#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "registry/registry.h"
#include "fmt.h"
#include "base64.h"

int registry_value_from_str(char *val_str, registry_type_t type, void *vp,
                            int maxlen)
{
    int32_t val = 0;
    char *eptr = 0;

    if (!val_str) {
        goto err;
    }

    switch (type) {
        case REGISTRY_TYPE_INT8:
        case REGISTRY_TYPE_INT16:
        case REGISTRY_TYPE_INT32:
        case REGISTRY_TYPE_BOOL:
            val = strtol(val_str, &eptr, 0);
            if (*eptr != REGISTRY_VAL_SEPARATOR) {
                goto err;
            }
            if (type == REGISTRY_TYPE_BOOL) {
                if (val < 0 || val > 1) {
                    goto err;
                }
                *(bool *)vp = val;
            }
            else if (type == REGISTRY_TYPE_INT8) {
                if (val < INT8_MIN || val > INT8_MAX) {
                    goto err;
                }
                *(int8_t *)vp = val;
            }
            else if (type == REGISTRY_TYPE_INT32) {
                *(int32_t *)vp = val;
            }
            break;
        case REGISTRY_TYPE_INT64:
            // TODO implement fmt 64 scan function
            puts("Not implemented");
            break;
        case REGISTRY_TYPE_STRING:
            val = strlen(val_str);
            if (val + 1 > maxlen) {
                goto err;
            }
            strcpy(vp, val_str);
            break;
        default:
            goto err;
    }
    return 0;
err:
    return -EINVAL;
}

int registry_bytes_from_str(char *val_str, void *vp, int *len)
{
    int err;

    err = base64_decode((const unsigned char *)val_str, strlen(val_str), vp,
                        (size_t *)len);

    if (err) {
        return -1;
    }
    return 0;
}

char *registry_str_from_value(registry_type_t type, void *vp, char *buf,
                              int buf_len)
{
    int32_t val = 0;

    if (type == REGISTRY_TYPE_STRING) {
        return vp;
    }

    switch(type) {
        case REGISTRY_TYPE_INT8:
        case REGISTRY_TYPE_INT16:
        case REGISTRY_TYPE_INT32:
        case REGISTRY_TYPE_BOOL:
            if (type == REGISTRY_TYPE_BOOL) {
                val = *(bool *)vp;
            }
            else if (type == REGISTRY_TYPE_INT8) {
                val = *(int8_t *)vp;
            }
            else if (type == REGISTRY_TYPE_INT16) {
                val = *(int16_t *)vp;
            }
            else if (type == REGISTRY_TYPE_INT32) {
                val = *(int32_t *)vp;
            }
            snprintf(buf, buf_len, "%ld", (long)val);
            return buf;
        case REGISTRY_TYPE_INT64:
            // TODO implement in fmt
            puts("Not implemented in fmt yet");
            return NULL;
        default:
            return NULL;
    }
}

char *registry_str_from_bytes(void *vp, int vp_len, char *buf, int buf_len)
{
    size_t enc_len = (size_t) buf_len;
    int err;

    err = base64_encode(vp, vp_len, (unsigned char *)buf, &enc_len);

    if (err) {
        return NULL;
    }
    return buf;
}
