#include "adv_beacon.h"
#include "math.h"
#include "mible_band.h"
#include "mibeacon_def.h"
#include "mibeacon_parse.h"
#include "mible_gap.h"
#include "mible_gateway.h"
#include "mible_def.h"

#undef  MIBLE_LOG_TAG
#define MIBLE_LOG_TAG                           "mible_beacon"
#undef  LOG_LEVEL
#define LOG_LEVEL                               LOG_LEVEL_INFO

static int send_beacon_to_parse(mible_addr_t address, ble_adv_frgm_data_t *p_frgm_data,
                                uint8_t adv_frgm_len, int8_t rssi)
{
    mible_event_t evt;
    int ret;
    static uint16_t error_num = 0;

    memset((uint8_t *)&evt, 0, sizeof(mible_event_t));
    memcpy((uint8_t *)evt.mible_beacon.address,
            (uint8_t *)address,
            sizeof(mible_addr_t));
    evt.mible_beacon.len = adv_frgm_len - sizeof(ble_adv_frgm_data_t);
    memcpy((uint8_t *)evt.mible_beacon.data,
            (uint8_t *)p_frgm_data->payload,
            evt.mible_beacon.len);
    evt.mible_beacon.rssi = rssi;

    ret = mible_event_send(MIBLE_EVT_GAP_BEACON, (mible_event_t *)&evt);
    if (MIIO_OK != ret) {
        error_num++;
        if (MIBEACON_EVENT_ERROR_GATE == error_num) {
            LOG_WARN_TAG(MIBLE_LOG_TAG, "fail to send beacon event");
            error_num = 0;
        }
    }

    return ret;
}

int ble_adv_parse(mible_addr_t address, uint8_t *p_adv_data, uint16_t adv_size, int8_t rssi)
{
    int ret = MIIO_OK;
    int ret2 = MIIO_OK;
    ble_adv_frgm_data_t *p_frgm_data = NULL;
    uint8_t adv_frgm_len = 0;

    if (NULL == p_adv_data || 0 == adv_size || adv_size > 31) {
        LOG_WARN_TAG(MIBLE_LOG_TAG,"beacon parse invalid param");
        return MIIO_ERROR_PARAM;
    }

    // LOG_DEBUG_TAG(MIBLE_LOG_TAG, "get beacon raw data =");
    // arch_dump_hex(p_adv_data, adv_size, NULL);

    // it is band beacon, do not need to parse here
    if (MIIO_OK == mible_band_parse(address, p_adv_data, adv_size, rssi)) {
        return MIIO_OK;
    }

    p_frgm_data = ble_adv_get_frgm_data(p_adv_data,
                                        adv_size,
                                        &adv_frgm_len,
                                        MIBLE_ADTYPE_SERVICE_DATA);
    if (NULL != p_frgm_data && adv_frgm_len > sizeof(ble_adv_frgm_data_t)) {
        if (p_frgm_data->tag == MIBLE_SERVICE_UUID) {
            ret = send_beacon_to_parse(address, p_frgm_data, adv_frgm_len, rssi);
        } else {
            ret = MIIO_ERROR_NOTFOUND;
        }
    } else {
        ret = MIIO_ERROR_NOTFOUND;
    }

    p_frgm_data = NULL;
    adv_frgm_len = 0;

    p_frgm_data = ble_adv_get_frgm_data(p_adv_data,
                                        adv_size,
                                        &adv_frgm_len,
                                        MIBLE_ADTYPE_MANUFACTURER_SPECIFIC);
    if (NULL != p_frgm_data && adv_frgm_len > sizeof(ble_adv_frgm_data_t)) {
        if (p_frgm_data->tag == MIBLE_MANUFT_ID) {
            ret2 = send_beacon_to_parse(address, p_frgm_data, adv_frgm_len, rssi);
        } else {
            ret2 = MIIO_ERROR_NOTFOUND;
        }
    } else {
        ret2 = MIIO_ERROR_NOTFOUND;
    }

    if (ret)
        return ret;
    else
        return ret2;
}

bool ble_adv_check_frgm(uint8_t *p_adv_data, uint16_t adv_size, uint8_t type, uint16_t tag)
{
    uint16_t len = 0;
    ble_adv_frgm_t *p_adv;

    if (NULL == p_adv_data || 0 == adv_size || 0 == type) {
        LOG_WARN_TAG(MIBLE_LOG_TAG,"adv check fragment invalid param");
        return FALSE;
    }

    while (len < adv_size) {
        p_adv = (ble_adv_frgm_t *)(p_adv_data + len);
        if (0 == p_adv->size) {
            break;
        }
        if (type == p_adv->type && 0 == memcmp(p_adv->data, &tag, 2)) {
            return TRUE;
        }
        len += p_adv->size + 1;
    }

    return FALSE;
}

void *ble_adv_get_frgm_data(uint8_t *p_adv_data, uint16_t adv_size, uint8_t *p_seg_data_size,
                            uint8_t type)
{
    uint16_t        len = 0;
    ble_adv_frgm_t *p_adv;

    if (NULL == p_adv_data || 0 == adv_size || 0 == type) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "adv get fragment data invalid param");
        return NULL;
    }

    // check the length first!
    len = 0;
    while (len < adv_size) {
        p_adv = (ble_adv_frgm_t *)(p_adv_data + len);
        len += p_adv->size + 1;
    }
    if (len != adv_size) {
        LOG_WARN_TAG(MIBLE_LOG_TAG,"ble adv length error!!");
        arch_dump_hex(p_adv_data, adv_size, "beacon");
        *p_seg_data_size = 0;
        return NULL;
    }

    len = 0;
    while (len < adv_size) {
        p_adv = (ble_adv_frgm_t *)(p_adv_data + len);
        if (0 == p_adv->size) {
            break;
        }
        if (type == p_adv->type) {
            *p_seg_data_size = p_adv->size - 1;
            return p_adv->data;
        }
        len += p_adv->size + 1;
    }

    return NULL;
}

int mible_beacon_handler(mible_beacon_t *p_beacon)
{
    int ret = MIIO_OK;
    mibeacon_info_t *p_info;

    if (NULL == p_beacon) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "beacon handler invalid parameter");
        return MIIO_ERROR_PARAM;
    }

    p_info = (mibeacon_info_t *)malloc(sizeof(mibeacon_info_t));
    if (NULL == p_info) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for mibeacon info");
        return MIIO_ERROR_NOMEM;
    }
    memset(p_info, 0, sizeof(mibeacon_info_t));
    memcpy((uint8_t *)p_info->mac, (uint8_t *)p_beacon->address, 6);
    p_info->rssi = p_beacon->rssi;

    // LOG_DEBUG_TAG(MIBLE_LOG_TAG,"get beacon = ");
    // arch_dump_hex(p_beacon->data, p_beacon->len, NULL);

    ret = mibeacon_parse((mibeacon_t *)p_beacon->data, p_beacon->len, p_info);

    if (NULL != p_info) {
        free(p_info);
        p_info = NULL;
    }
    return ret;
}
