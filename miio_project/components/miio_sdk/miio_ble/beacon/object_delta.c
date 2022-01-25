#include "adv_beacon.h"
#include "math.h"
#include "mibeacon_def.h"
#include "object_delta.h"

#undef  MIBLE_LOG_TAG
#define MIBLE_LOG_TAG                           "mible_beacon"
#undef  LOG_LEVEL
#define LOG_LEVEL                               LOG_LEVEL_INFO

static uint16_t FromByte2Uint16(uint8_t *bs)
{
    return ((uint16_t)bs[0] | ((uint16_t)bs[1] << 8));
}

static uint32_t FromByte2Uint32(uint8_t *bs)
{
    return ((uint32_t)bs[0] | ((uint32_t)bs[1] << 8) | ((uint32_t)bs[2] << 16)
            | ((uint32_t)bs[3] << 24));
}

static int32_t FromByte2Int32(uint8_t *bs)
{
    return ((int32_t)bs[0] | ((int32_t)bs[1] << 8) | ((int32_t)bs[2] << 16)
            | ((int32_t)bs[3] << 24));
}

bool mibeacon_check_object_delta(mibeacon_object_t *p_old, mibeacon_object_t *p_new, uint32_t delta)
{
    if (NULL == p_old || NULL == p_new || p_old->id != p_new->id) {
        return FALSE;
    }

    switch (p_old->id) {
        case OBJECT_ID_EVT_LOCK: {
            if (0 != memcmp((uint8_t *)p_old->data, (uint8_t *)p_new->data, 6)) {
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Lock first 6 bytes different");
                return TRUE;
            }
            uint32_t old_ts = FromByte2Uint32(&p_old->data[6]);
            uint32_t new_ts = FromByte2Uint32(&p_new->data[6]);
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Lock check delta; old = %08x, new = %08x, rule = %d",
                          old_ts, new_ts, delta);
            if ((new_ts - old_ts) >= delta) {
                return TRUE;
            } else {
                return FALSE;
            }
        }
        case OBJECT_ID_EVT_LOCK_NEW: {
            if (0 != memcmp((uint8_t *)p_old->data, (uint8_t *)p_new->data, 5)) {
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Lock new first 5 bytes different");
                return TRUE;
            }
            uint32_t old_ts = FromByte2Uint32(&p_old->data[5]);
            uint32_t new_ts = FromByte2Uint32(&p_new->data[5]);
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Lock new check delta; old = %08x, new = %08x, rule = %d",
                          old_ts, new_ts, delta);
            if ((new_ts - old_ts) >= delta) {
                return TRUE;
            } else {
                return FALSE;
            }
        }
        case OBJECT_ID_EVT_FINGER: {
            if (0 != memcmp((uint8_t *)p_old->data, (uint8_t *)p_new->data, 4)) {
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "FINGER first 4 bytes different");
                return TRUE;
            }
            int32_t old_value = FromByte2Int32(&p_old->data[4]);
            int32_t new_value = FromByte2Int32(&p_new->data[4]);
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "FINGER check delta; old = %08x, new = %08x, rule = %d",
                          old_value, new_value, delta);
            if (abs(new_value - old_value) >= delta) {
                return TRUE;
            } else {
                return FALSE;
            }
        }
        case OBJECT_ID_ATT_HM_BAND: {
            uint16_t old_steps = FromByte2Uint16(&p_old->data[0]);
            uint16_t new_steps = FromByte2Uint16(&p_new->data[0]);
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Band check delta; old = %d, new = %d, rule = %d",
                          old_steps, new_steps, delta);
            if ((new_steps - old_steps) >= delta) {
                return TRUE;
            } else {
                return FALSE;
            }
        }
        default:
            break;
    }

    if (p_old->data_len > 4
        && 0 != memcmp((uint8_t *)p_old->data, (uint8_t *)p_new->data, p_old->data_len - 4)) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Common check delta; data_len - 4 bytes different");
        return TRUE;
    } else {
        uint16_t index = (p_old->data_len > 4) ? (p_old->data_len - 4) : 0;
        int32_t old_value = FromByte2Int32(&p_old->data[index]);
        int32_t new_value = FromByte2Int32(&p_new->data[index]);
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Common check delta; old = %08x, new = %08x, rule = %d",
                      old_value, new_value, delta);
        if (abs(new_value - old_value) >= delta) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
}

