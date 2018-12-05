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
    char buf[BASE64_ESTIMATE_BYTE_SIZE(REGISTRY_MAX_VAL_LEN)];
    size_t _len = 0;
    int val_len = strlen(val_str);

    base64_decode((unsigned char *)val_str, val_len, (void *)buf, &_len);

    if (_len > sizeof(buf)) {
        return -EINVAL;
    }

    if (base64_decode((unsigned char *)val_str, val_len, (void *)buf, &_len) !=
        BASE64_SUCCESS) {
            return -EINVAL;
    }

    if ((int)_len > *len) {
        return -EINVAL;
    }
    
    memcpy(vp, (void *)buf, _len);

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
    int res;
    char temp_buf[REGISTRY_MAX_VAL_LEN];
    size_t enc_len = 0;

    base64_encode(vp, vp_len, (unsigned char *)temp_buf, &enc_len);

    if (enc_len > REGISTRY_MAX_VAL_LEN) {
        return NULL;
    }

    res = base64_encode(vp, vp_len, (unsigned char *)temp_buf, &enc_len);

    if (res != BASE64_SUCCESS || (int)enc_len > (buf_len - 1)) {
        return NULL;
    }

    memcpy(buf, (void *)temp_buf, enc_len);
    buf[enc_len] = '\0';

    return vp;
}
