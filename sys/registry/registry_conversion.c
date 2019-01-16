#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "registry/registry.h"
#include "base64.h"

#if defined(CONFIG_REGISTRY_USE_INT64)
#include "fmt.h"

static int64_t dec_to_s64(char *p_str, char **e_ptr)
{
    uint64_t val = 0U;
    uint64_t prev_val = 0U;
    bool neg = false;
    int digit;

    if (*p_str == '-') {
        neg = true;
        p_str++;
    } else if (*p_str == '+') {
        p_str++;
    }

    while (1) {
        if (*p_str >= '0' && *p_str <= '9') {
            digit = *p_str - '0';
        } else {
            break;
        }

        val *= 10;
        val += digit;

        if (val < prev_val) {
            break;
        }

        prev_val = val;
        p_str++;
    }

    if (e_ptr != 0) {
        *e_ptr = p_str;
    }

    if (neg) {
        return -val;
    } else {
        return val;
    }
}
#endif /* CONFIG_REGISTRY_USE_INT64 */

int registry_value_from_str(char *val_str, registry_type_t type, void *vp,
                            int maxlen)
{
#if defined(CONFIG_REGISTRY_USE_INT64)
    int64_t val = 0;
#else /* CONFIG_REGISTRY_USE_INT64 */
    int32_t val = 0;
#endif

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
            if (*eptr != '\0') {
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
        case REGISTRY_TYPE_STRING:
            val = strlen(val_str);
            if (val + 1 > maxlen) {
                goto err;
            }
            strcpy(vp, val_str);
            break;
#if defined(CONFIG_REGISTRY_USE_INT64)
        case REGISTRY_TYPE_INT64:
            val = dec_to_s64(val_str, &eptr);
            if (*eptr != '\0') {
                goto err;
            }
            *(int64_t *)vp = val;
            break;
#endif /* CONFIG_REGISTRY_USE_INT64 */
        default:
            goto err;
    }
    return 0;
err:
    return -EINVAL;
}

int registry_bytes_from_str(char *val_str, void *vp, int *len)
{
    char buf[BASE64_ESTIMATE_DECODE_SIZE(REGISTRY_MAX_VAL_LEN)];
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
    *len = (int)_len;

    return 0;
}

char *registry_str_from_value(registry_type_t type, void *vp, char *buf,
                              int buf_len)
{
#if defined(CONFIG_REGISTRY_USE_INT64)
    int64_t val = 0;
    int len;
#else /* CONFIG_REGISTRY_USE_INT64 */
    int32_t val = 0;
#endif

    if (type == REGISTRY_TYPE_STRING) {
        return vp;
    }

    switch(type) {
        case REGISTRY_TYPE_INT8:
            val = *(int8_t *)vp;
            break;
        case REGISTRY_TYPE_INT16:
            val = *(int8_t *)vp;
            break;
        case REGISTRY_TYPE_INT32:
            val = *(int32_t *)vp;
            break;
        case REGISTRY_TYPE_BOOL:
            val = *(bool *)vp;
            break;
#if defined(CONFIG_REGISTRY_USE_INT64)
        case REGISTRY_TYPE_INT64:
            val = *(int64_t *)vp;
            len = fmt_s64_dec(NULL, val);
            if (len > buf_len - 1) {
                return NULL;
            }
            fmt_s64_dec(buf, val);
            buf[len] = '\0';
            return buf;
#endif /* CONFIG_REGISTRY_USE_INT64 */
        default:
            return NULL;
    }
    snprintf(buf, buf_len, "%lld", val);
    return buf;
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
