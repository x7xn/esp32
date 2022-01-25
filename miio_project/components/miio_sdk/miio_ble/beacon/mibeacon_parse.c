#include "mibeacon_parse.h"
#include "adv_beacon.h"
#include "mible_keep_alive.h"
#include "mible_cmd.h"
#include "mible_dev.h"
#include "mible_fastpair.h"
#include "mible_gateway.h"
#include "mible_prph.h"
#include "mible_rpc.h"
#include "mible_net.h"
#include "mible_hooks.h"
#include "object_delta.h"
#include "object_rule.h"
#include "mible_def.h"
#include "mible_linkage.h"
#include "mible_linkage_search.h"
#include <tinycrypt/ccm_mode.h>
#include <tinycrypt/constants.h>

#undef  MIBLE_LOG_TAG
#define MIBLE_LOG_TAG                           "mible_beacon"
#undef  LOG_LEVEL
#define LOG_LEVEL                               LOG_LEVEL_INFO

#define MIBLE_OBJECT_ID_FASTPAIR                0x0002

static int mibeacon_check_last_report(mibeacon_object_t *p_old, mibeacon_object_t *p_new,
                                      object_rule_t *p_object_rule,
                                      mibeacon_object_array_t *p_object_array);
static uint16_t mibeacon_get_min_length(uint16_t frame_ctrl, uint8_t capability);
static int mibeacon_get_cap(mibeacon_t *p_beacon, uint16_t len, uint8_t *p_cap);
static int mibeacon_get_objects(uint8_t *p_buffer, uint16_t len, mibeacon_object_array_t *p_array);
static int mibeacon_v4_decrypt(uint8_t *out, uint8_t *in, uint16_t len,
                               mibeacon_decrypt_config_t *p_cfg);
static int mibeacon_legacy_decrypt(uint8_t *out, uint8_t *in, uint16_t len,
                                   mibeacon_decrypt_config_t *p_cfg);
static void mibeacon_check_fastpair(mibeacon_info_t *p_info);
static int mibeacon_check_linkage(mibeacon_info_t *p_info);

int mibeacon_parse(mibeacon_t *p_beacon, uint16_t beacon_len, mibeacon_info_t *p_info)
{
    int ret = MIIO_OK;
    uint8_t *p_load;
    uint16_t index = 0;
    uint16_t min_len, data_len;

    if (NULL == p_beacon || NULL == p_info || beacon_len < sizeof(mibeacon_t)) {
        // LOG_WARN_TAG(MIBLE_LOG_TAG, "mibeacon parse invalid param");
        return MIIO_ERROR_PARAM;
    }

    p_info->frame_ctrl = p_beacon->frame_ctrl;
    p_info->product_id = p_beacon->product_id;
    p_info->counter = p_beacon->counter;

    p_info->version = (p_beacon->frame_ctrl & MIBEACON_VERSION_MASK) >> MIBEACON_VERSION_OFFSET;
    p_info->timestamp = arch_os_time_now();

    if (p_info->version > 5 || p_info->version < 2) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "unknown version = %d", p_info->version);
        return MIIO_ERROR_NOTFOUND;
    }

    if (p_info->version == 5 && p_beacon->frame_ctrl & MIBEACON_MESH_MASK) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "ignore mesh beacon");
        return MIIO_ERROR_PARAM;
    }

    ret = mibeacon_get_cap(p_beacon, beacon_len, &p_info->capability);
    if (MIIO_OK != ret) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Can not get capability in product %d", p_info->product_id);
        return ret;
    }

    min_len = mibeacon_get_min_length(p_beacon->frame_ctrl, p_info->capability);
    data_len = beacon_len - sizeof(mibeacon_t);
    LOG_DEBUG_TAG(MIBLE_LOG_TAG,
                  "pid = %d, version = %d; frame = %04x; min_len = %d; data_len = %d",
                  p_info->product_id, p_info->version, p_beacon->frame_ctrl, min_len, data_len);
    if (data_len < min_len) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "the len of product %d is invalid", p_info->product_id);
        return MIIO_ERROR_SIZE;
    }

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "find frame 2bytes, pid 2bytes; counter 1bytes from begin");
    LOG_DEBUG_TAG(
        MIBLE_LOG_TAG,
        "frame control: isEncrypted = %d, mac = %d, Capability = %d, Object = %d, Mesh = %d",
        p_beacon->frame_ctrl & MIBEACON_ENCRYPT_MASK, p_beacon->frame_ctrl & MIBEACON_MAC_MASK,
        p_beacon->frame_ctrl & MIBEACON_CAPABILITY_MASK,
        p_beacon->frame_ctrl & MIBEACON_OBJECT_MASK, p_beacon->frame_ctrl & MIBEACON_MESH_MASK);

    p_load = p_beacon->payload;
    if (p_beacon->frame_ctrl & MIBEACON_MAC_MASK) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "find mac, 6 bytes");
        // NOTE: ignore mac in beacon
        // memcpy((uint8_t *)p_info->mac, (uint8_t *)&p_load[index], 6);
        index += 6;
    }

    if (p_beacon->frame_ctrl & MIBEACON_CAPABILITY_MASK) {
        index += 1;
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "capability = %02x", p_info->capability);
        p_info->bond_type = (p_info->capability & CAPABILITY_BOND_MASK) >> CAPABILITY_BOND_OFFSET;
        if (BOND_TYPE_COMBO == p_info->bond_type) {
            memcpy((uint8_t *)p_info->wifi_mac, (uint8_t *)&p_load[index], 2);
            index += 2;
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "wifi_mac = %02X %02X", p_info->wifi_mac[0],
                          p_info->wifi_mac[1]);
        }

        // only for v5: capability IO
        if (p_info->version == 5 && p_info->capability & CAPABILITY_IO_MASK) {
            memcpy((uint8_t *)&p_info->io_capability, (uint8_t *)&p_load[index], 2);
            index += 2;
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "io_capability = %04x", p_info->io_capability);
        }
    }
/*
    // only for v5: mesh
    if (p_info->version == 5 && p_beacon->frame_ctrl & MIBEACON_MESH_MASK) {
        memcpy((uint8_t *)&p_info->mesh_cfg, (uint8_t *)&p_load[data_len - 2], 2);
        data_len -= 2;
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Mesh cfg = %04x", p_info->mesh_cfg);
    }
*/
    if (p_beacon->frame_ctrl & MIBEACON_ENCRYPT_MASK) {
        p_info->encrypted = 1;
        if (p_info->version >= 4 && p_info->version <= 5) {
            memcpy((uint8_t *)&p_info->mic, (uint8_t *)&p_load[data_len - 4], 4);
            data_len -= 4;
        } else if (p_info->version >= 2 && p_info->version <= 3) {
            memcpy((uint8_t *)&p_info->mic, (uint8_t *)&p_load[data_len - 1], 1);
            data_len -= 1;
        }
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "mic = %08x", p_info->mic);

        memcpy((uint8_t *)&p_info->counter + 1, (uint8_t *)&p_load[data_len - 3], 3);
        data_len -= 3;
    }
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "counter = %08x", p_info->counter);

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "obj_flag = %d, index = %d, data_len = %d",
                  p_beacon->frame_ctrl & MIBEACON_OBJECT_MASK, index, data_len);
    if (((p_beacon->frame_ctrl & MIBEACON_OBJECT_MASK) != 0 && index < data_len) ||
        ((p_beacon->frame_ctrl & MIBEACON_OBJECT_MASK) == 0 && index == data_len)) {
        p_info->object_len = data_len - index;
        memcpy((uint8_t *)p_info->object, (uint8_t *)&p_load[index], data_len - index);
        /* Check if it is for fast pair */
        mibeacon_check_fastpair(p_info);
        /* Check if it is for linkage */
        mibeacon_check_linkage(p_info);
        /* Send object to outside MCU */
        mibeacon_send_by_serial(p_info);
        /* Upload object to mijia server */
        mibeacon_upload(p_info);
    } else {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "illegal object!");
    }

    return MIIO_OK;
}

int mibeacon_upload(mibeacon_info_t *p_info)
{
#if MIBLE_GATEWAY_DISABLE

    UNUSED(mibeacon_check_last_report);
    return MIIO_OK;

#else

    int ret = MIIO_OK;
    authed_dev_t *p_authed_dev = NULL;
    unknown_dev_t *p_unknown_dev = NULL;
    denied_dev_t *p_denied_dev = NULL;
    pid_obj_id_t *p_pid_obj_id = NULL;
    unknown_pid_t *p_unknown_pid = NULL;
    object_rule_t *p_object_rule = NULL;

    keep_alive_dev_t *p_keep_alive_dev = NULL;
    mibeacon_decrypt_config_t *p_config = NULL;
    mibeacon_object_array_t *p_object_array = NULL;
    mibeacon_object_array_t *p_object_array_upload = NULL;

    if (NULL == p_info) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "mibeacon upload invalid param");
        return MIIO_ERROR_PARAM;
    }

    // 0. check wifi status
    if (!mible_net_state_check()) {
        return MIIO_ERROR_NOTREADY;
    }

    // 0.a check state of gateway
    if (!mible_gateway_enable()) {
        return MIIO_ERROR_NOTREADY;
    }

    // 1. check unknown device list
    p_unknown_dev = unknown_dev_search(p_info->mac, p_info->product_id);
    if (p_unknown_dev != NULL) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Found unknown_dev in local list");
        // ignore this beacon
        ret = MIIO_ERROR;
        goto end;
    }

    // 1.a check denied and unknown device list
    p_denied_dev = denied_dev_search(p_info->mac, p_info->product_id);
    if (p_denied_dev != NULL) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Found denied_dev in local list");
        // ignore this beacon
        ret = MIIO_ERROR;
        goto end;
    }

    // 2. check authed device list
    p_authed_dev = authed_dev_search(p_info->mac, p_info->product_id);
    if (p_authed_dev == NULL) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Not found authed_dev in local list");
        // send ble_query_dev message to server
        // NOTE: add to unknown dev list MIBEACON_UNKNOWN_TTL for avoiding multi-query
        if (MIIO_OK == mible_rpc_query_dev(p_info->mac, p_info->product_id)) {
            unknown_dev_t tmp_dev;
            memset((uint8_t *)&tmp_dev, 0, sizeof(unknown_dev_t));
            memcpy((uint8_t *)tmp_dev.mac, p_info->mac, 6);
            tmp_dev.pid = p_info->product_id;
            tmp_dev.ttl = MIBEACON_UNKNOWN_TTL;
            unknown_dev_add(&tmp_dev);
        }
        ret = MIIO_ERROR_NOTFOUND;
        goto end;
    }

    // 3. here, means the device is alive, add to keep alive list
    // Use mibeacon to check if the device is alive
    // only keep alive device which is not in denied table, still update ble event in denied table
    p_keep_alive_dev = (keep_alive_dev_t *)malloc(sizeof(keep_alive_dev_t));
    if (NULL == p_keep_alive_dev) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for keep alive dev");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memset((uint8_t *)p_keep_alive_dev, 0, sizeof(keep_alive_dev_t));
    p_keep_alive_dev->pid = p_authed_dev->pid;
    memcpy((uint8_t *)p_keep_alive_dev->mac, (uint8_t *)p_authed_dev->mac, 6);
    memcpy((uint8_t *)p_keep_alive_dev->did, (uint8_t *)p_authed_dev->did, MIBEACON_DID_LEN);
    p_keep_alive_dev->rssi = p_info->rssi;
    p_keep_alive_dev->timestamp = p_info->timestamp;
    keep_alive_dev_add(p_keep_alive_dev);

    // 4. check denied pid obj id
    p_unknown_pid = unknown_pid_search(p_info->product_id);
    if (p_unknown_pid != NULL) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Found unknown_pid in local list");
        // ignore this beacon
        ret = MIIO_ERROR;
        goto end;
    }

    // 5. check pid obj id
    p_pid_obj_id = pid_obj_id_search(p_info->product_id);
    if (p_pid_obj_id == NULL) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Not found pid_obj_id in local list");
        // send ble_query_prod to server
        // NOTE: add to denied pid list MIBEACON_UNKNOWN_TTL for avoiding mulit query
        if (MIIO_OK == mible_rpc_query_prod(p_info->product_id)) {
            unknown_pid_t tmp_pid;
            memset((uint8_t *)&tmp_pid, 0, sizeof(unknown_pid_t));
            tmp_pid.pid = p_info->product_id;
            tmp_pid.ttl = MIBEACON_UNKNOWN_TTL;
            unknown_pid_add(&tmp_pid);
        }
        ret = MIIO_ERROR_NOTFOUND;
        goto end;
    }

    // 6. if no object, return directly
    if (p_info->object_len == 0) {
        ret = MIIO_OK;
        goto end;
    }
    // here, beacon has object
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "%s find objects", p_authed_dev->did);

    // 7. check counter after check object
    if (p_info->counter == p_authed_dev->counter) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "same counter as authed dev");
        ret = MIIO_ERROR_NOTFOUND;
        goto end;
    }
    p_authed_dev->counter = p_info->counter;

    // 7.a ignore minor version and un-encrypted beacon if received encrypted beacon before
    if (p_authed_dev->already_encrypted == 1) {
        if (p_info->version < 5 || p_info->encrypted == 0) {
            ret = MIIO_ERROR;
            LOG_WARN_TAG(MIBLE_LOG_TAG, "Received un-encrypted beacon!!!");
            goto end;
        }
    }

    // 7.b Anti-replay when version is 5
    if (p_info->version == 5) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "replay and current counter = %08x %08x",
                      p_authed_dev->replay_counter, p_info->counter);
        if (p_authed_dev->replay_counter != 0 && (int32_t)(p_info->counter - p_authed_dev->replay_counter) <= 0) {
            if (p_authed_dev->replay_counter == p_info->counter) {
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Same Replay counter!!!");
            } else {
                LOG_WARN_TAG(MIBLE_LOG_TAG,
                             "Caution!!! Replay Attack!!! replay and current counter = %08x %08x",
                             p_authed_dev->replay_counter, p_info->counter);
            }
            ret = MIIO_ERROR_NOTREADY;
            goto end;
        }
    }

    // 8. decryption
    if (p_info->encrypted == 1) {
        p_config = (mibeacon_decrypt_config_t *)malloc(sizeof(mibeacon_decrypt_config_t));
        if (NULL == p_config) {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for mibeacon decrypt config");
            ret = MIIO_ERROR_NOMEM;
            goto end;
        }
        memset((uint8_t *)p_config, 0, sizeof(mibeacon_decrypt_config_t));
        p_config->frame_ctrl = p_info->frame_ctrl;
        p_config->product_id = p_info->product_id;
        p_config->counter = p_info->counter;

        p_config->mic = p_info->mic;
        memcpy((uint8_t *)p_config->mac, (uint8_t *)p_info->mac, 6);
        memcpy((uint8_t *)p_config->key, (uint8_t *)p_authed_dev->beaconkey,
                MIBEACON_BEACONKEY_LEN);

        if (p_info->version >= 4 && p_info->version <= 5) {
            ret = mibeacon_v4_decrypt(p_info->object, p_info->object, p_info->object_len, p_config);
            if (MIIO_OK != ret) {
                LOG_WARN_TAG(MIBLE_LOG_TAG, "v4 decrypt error");
            }
        } else if (p_info->version >= 2 && p_info->version <= 3) {
            ret = mibeacon_legacy_decrypt(p_info->object, p_info->object, p_info->object_len,
                                          p_config);
            if (MIIO_OK != ret) {
                LOG_WARN_TAG(MIBLE_LOG_TAG, "legacy decrypt error");
            }
        } else {
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "unknow version decryption");
            ret = MIIO_ERROR_NOTFOUND;
			goto end;
        }
        if (MIIO_OK != ret) {
            goto end;
        }

        if (p_info->version == 5) {
            // 8.a receive already encrpyted beacon
            p_authed_dev->already_encrypted = 1;
            // 8.b update replay counter
            p_authed_dev->replay_counter = p_info->counter;
        }
    }

    // 9. get all objects from beacon
    p_object_array = (mibeacon_object_array_t *)malloc(sizeof(mibeacon_object_array_t));
    if (NULL == p_object_array) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for mibeacon object array");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memset((uint8_t *)p_object_array, 0, sizeof(mibeacon_object_array_t));
    p_object_array_upload = (mibeacon_object_array_t *)malloc(sizeof(mibeacon_object_array_t));
    if (NULL == p_object_array_upload) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for mibeacon object array");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memset((uint8_t *)p_object_array_upload, 0, sizeof(mibeacon_object_array_t));
    ret = mibeacon_get_objects(p_info->object, p_info->object_len, p_object_array);
    if (MIIO_OK != ret) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Get objects format error");
        // clear all info and return
        memset((uint8_t *)p_info, 0, sizeof(mibeacon_info_t));
        goto end;
    }
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Get objects from beacon num = %d", p_object_array->num);

    // 9.1 set timestamp for all objects in the beacon
    for (int i = 0; i < p_object_array->num; i++) {
        p_object_array->object[i].timestamp = p_info->timestamp;
    }

    // 9.2. for each object in beacon
    for (int i = 0; i < p_object_array->num; i++) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "for each object: index = %d, id = %04x, len = %d",
                  i, p_object_array->object[i].id, p_object_array->object[i].data_len);
        // 9.2.1 get rule of object
        p_object_rule = object_rule_search(p_object_array->object[i].id);
        if (p_object_rule == NULL) {
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Not found object_rule for this obj_id in local list");
            continue;
        }

        // 9.2.2 for each report in authed device
        bool report_found = FALSE;
        for (int j = 0; j < p_authed_dev->report_num; j++) {
            mibeacon_object_t *p_report = &p_authed_dev->report[j];
            mibeacon_object_t *p_object = &p_object_array->object[i];
            if (p_object->id == p_report->id) {
                mibeacon_check_last_report(p_report, p_object, p_object_rule, p_object_array_upload);
                report_found = TRUE;
            }
        }
        // 9.2.3 if no report, add to array directly, add to report
        if (!report_found) {
            if (p_authed_dev->report_num >= MAX_OBJECT_ID_IN_PID) {
                continue;
            }
            memcpy((uint8_t *)&p_object_array_upload->object[p_object_array_upload->num],
                   (uint8_t *)&p_object_array->object[i], sizeof(mibeacon_object_t));
            p_object_array_upload->num ++;

            memcpy((uint8_t *)&p_authed_dev->report[p_authed_dev->report_num],
                   (uint8_t *)&p_object_array->object[i], sizeof(mibeacon_object_t));
            p_authed_dev->report_num ++;
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "add new authed device report num = %d",
                          p_authed_dev->report_num);
        }
    }

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "p_object_array_upload->num = %d", p_object_array_upload->num);
    // 9.3 add to upload list
    if (p_object_array_upload->num > 0) {
        // send ble_event message to server
        mible_rpc_report_evt(p_authed_dev, p_object_array_upload);
    }

end:
    if (NULL != p_keep_alive_dev) {
        free(p_keep_alive_dev);
        p_keep_alive_dev = NULL;
    }
    if (NULL != p_config) {
        free(p_config);
        p_config = NULL;
    }
    if (NULL != p_object_array) {
        free(p_object_array);
        p_object_array = NULL;
    }
    if (NULL != p_object_array_upload) {
        free(p_object_array_upload);
        p_object_array_upload = NULL;
    }

    return ret;

#endif
}

static void mibeacon_check_fastpair(mibeacon_info_t *p_info)
{
    mibeacon_object_array_t object_array = {0};
    int ret;

    if (0 == p_info->object_len || p_info->encrypted > 0) {
        return;
    }

    ret = mibeacon_get_objects(&p_info->object[0], p_info->object_len, &object_array);
    if (MIIO_OK != ret) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Get objects format error");
        return;
    }

    for (int index = 0; index < object_array.num; ++index) {
        if (MIBLE_OBJECT_ID_FASTPAIR == object_array.object[index].id &&
            sizeof(uint16_t) == object_array.object[index].data_len) {

            uint16_t object_data;
            memcpy(&object_data, &object_array.object[index].data[0], sizeof(object_data));
            if (mible_prph_check_object(p_info->mac, p_info->product_id, object_data)) {
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "This device and related object has existed");
                mible_hooks_fastpair_report(p_info->mac, FASTPAIR_STATE_EXISTED);
            } else {
                mible_fastpair_start(p_info->mac, p_info->product_id,
                                     object_data, p_info->rssi);
            }
            break;
        }
    }
}

int mibeacon_send_by_serial(mibeacon_info_t *p_info)
{
    mibeacon_decrypt_config_t decrypt_config;
    mibeacon_object_array_t object_array = {0};
    uint8_t object[MAX_MULTI_OBJECT_LEN] = {0};
    int ret;
    bool data_valid = false;

    if (0 == p_info->object_len) {
        return MIIO_ERROR_SIZE;
    }

    /* Check if this devie is among list */
    memset(&decrypt_config, 0xFF, sizeof(decrypt_config));
    if (MIIO_OK != mible_prph_find_key(p_info->mac, p_info->product_id, &decrypt_config.key[0])) {
        return MIIO_ERROR_NOTFOUND;
    }

    /* Check if this beacon should be encrypted */
    if (0 == p_info->encrypted && mible_prph_should_encrypted(p_info->mac, p_info->product_id)) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "The beacon of device %02x:%02x:%02x:%02x:%02x:%02x (pid %d) should be encrypted",
                        p_info->mac[5], p_info->mac[4], p_info->mac[3], p_info->mac[2],
                        p_info->mac[1], p_info->mac[0], p_info->product_id);
        return MIIO_ERROR;
    }

    /* Check if this beacon should be anti-replay */
    if (p_info->version < 5 && mible_prph_should_anti_replay(p_info->mac, p_info->product_id)) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "The beacon of device %02x:%02x:%02x:%02x:%02x:%02x (pid %d) should be anti-replay",
                        p_info->mac[5], p_info->mac[4], p_info->mac[3], p_info->mac[2],
                        p_info->mac[1], p_info->mac[0], p_info->product_id);
        return MIIO_ERROR;
    }

    if (p_info->encrypted == 1) {
        decrypt_config.frame_ctrl = p_info->frame_ctrl;
        decrypt_config.product_id = p_info->product_id;
        decrypt_config.counter = p_info->counter;
        decrypt_config.mic = p_info->mic;
        memcpy(decrypt_config.mac, p_info->mac, 6);
        if (p_info->version >= 4 && p_info->version <= 5) {
            ret = mibeacon_v4_decrypt(object, p_info->object,
                                      p_info->object_len, &decrypt_config);
            if (MIIO_OK != ret) {
                LOG_WARN_TAG(MIBLE_LOG_TAG, "Fail to decrypt with version %d", p_info->version);
                return ret;
            }
        } else if (p_info->version >= 2 && p_info->version <= 3) {
            ret = mibeacon_legacy_decrypt(object, p_info->object,
                                          p_info->object_len, &decrypt_config);
            if (MIIO_OK != ret) {
                LOG_WARN_TAG(MIBLE_LOG_TAG, "Fail to decrypt with version %d", p_info->version);
                return ret;
            }
        } else {
            LOG_WARN_TAG(MIBLE_LOG_TAG, "Decryption with unknow version %d", p_info->version);
            return MIIO_ERROR;
        }
    } else {
        memcpy(object, p_info->object, p_info->object_len);
    }

    ret = mibeacon_get_objects(object, p_info->object_len, &object_array);
    if (MIIO_OK != ret) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Get objects format error");
        return ret;
    }

    if (0 == object_array.num) {
        return MIIO_ERROR_EMPTY;
    }

    for (int index = 0; index < object_array.num; ++index) {
        if (mible_prph_check_counter(p_info->mac, p_info->product_id, object_array.object[index].id,
                                        p_info->counter, p_info->version >= 5)) {

            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Object ID %04x with counter %u",
                               object_array.object[index].id, p_info->counter);
            mible_hooks_get_object(p_info->mac, &object_array.object[index]);
            data_valid = true;
        }
    }

    /* Only if beacon is encrypted and object is valid, state of this device is stored */
    if (data_valid && 1 == p_info->encrypted) {
        mible_prph_attribute_update(p_info->mac, p_info->product_id,
                                    1 == p_info->encrypted, p_info->version >= 5);
    }

    return MIIO_OK;
}

static int mibeacon_check_last_report(mibeacon_object_t *p_old, mibeacon_object_t *p_new,
                                      object_rule_t *p_object_rule,
                                      mibeacon_object_array_t *p_object_array)
{
    int ret = MIIO_OK;
    uint32_t real_interval;

    if (NULL == p_old || NULL == p_new || NULL == p_object_rule || NULL == p_object_array
        || p_new->timestamp < p_old->timestamp) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "mibeacon check last report invalid param");
        return MIIO_ERROR_PARAM;
    }
    real_interval = p_new->timestamp - p_old->timestamp;
    LOG_DEBUG_TAG(MIBLE_LOG_TAG,
                  "object id = %04x, check report timestamp old = %u, new = %u, real interval = %u, rule = %u",
                  p_object_rule->obj_id, p_old->timestamp, p_new->timestamp, real_interval,
                  p_object_rule->interval);

    // real delta;
    bool real_delta = mibeacon_check_object_delta(p_old, p_new, p_object_rule->delta);

    if (real_interval >= p_object_rule->interval && real_delta) {
        memcpy((uint8_t *)&p_object_array->object[p_object_array->num], (uint8_t *)p_new,
               sizeof(mibeacon_object_t));
        p_object_array->num++;
        memcpy((uint8_t *)p_old, (uint8_t *)p_new, sizeof(mibeacon_object_t));
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Valid interval and delta");
    }

    if (real_interval < p_object_rule->interval) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "invalid report time interval");
    }
    if (false == real_delta) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "invalid report data delta");
    }
    return ret;
}

static uint16_t mibeacon_get_min_length(uint16_t frame_ctrl, uint8_t capability)
{
    uint16_t len = 0;
    uint8_t bond_type;
    uint8_t version = (frame_ctrl & MIBEACON_VERSION_MASK) >> MIBEACON_VERSION_OFFSET;

    if (frame_ctrl & MIBEACON_MAC_MASK) {
        len += 6;
    }

    if (frame_ctrl & MIBEACON_CAPABILITY_MASK) {
        len += 1;
        bond_type = (capability & CAPABILITY_BOND_MASK) >> CAPABILITY_BOND_OFFSET;
        if (BOND_TYPE_COMBO == bond_type) {
            len += 2;
        }

        // only for v5: capability io
        if (version == 5 && capability & CAPABILITY_IO_MASK) {
            len += 2;
        }
    }

    if (frame_ctrl & MIBEACON_ENCRYPT_MASK) {
        if (version >= 4 && version <= 5) {
            len += 3 + 4;  // 3 byte counter + 4 byte MIC
        } else if (version >= 2 && version <= 3) {
            len += 3 + 1;  // 3 byte counter + 1 byte MIC ?
        }
    }

    // only for v5: mesh
    if (version == 5 && frame_ctrl & MIBEACON_MESH_MASK) {
        len += 2;
    }

    return len;
}

static int mibeacon_get_cap(mibeacon_t *p_beacon, uint16_t len, uint8_t *p_cap)
{
    int ret = MIIO_OK;
    uint16_t index = 0;

    if (NULL == p_beacon || NULL == p_cap || len < sizeof(mibeacon_t)) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "mibeacon get cap invalid param");
        return MIIO_ERROR_PARAM;
    }

    if (p_beacon->frame_ctrl & MIBEACON_MAC_MASK) {
        index += 6;
    }

    if (p_beacon->frame_ctrl & MIBEACON_CAPABILITY_MASK) {
        if (len >= sizeof(mibeacon_t) + index + 1) {
            uint8_t *p_load = (uint8_t *)p_beacon->payload;
            *p_cap = p_load[index];
            ret = MIIO_OK;
        }
    } else if (len >= sizeof(mibeacon_t) + index) {
        *p_cap = 0;
        ret = MIIO_OK;
    } else {
        return MIIO_ERROR_NOTFOUND;
    }

    return ret;
}

static int mibeacon_get_objects(uint8_t *p_buffer, uint16_t len, mibeacon_object_array_t *p_array)
{
    int ret = MIIO_OK;
    uint8_t obj_len = 0;
    mibeacon_object_t *p_object = (mibeacon_object_t *)malloc(sizeof(mibeacon_object_t));
    if (NULL == p_object) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for mibeacon object");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }

    if (NULL == p_buffer || len == 0 || NULL == p_array) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "mibeacon get objects invalid param");
        ret = MIIO_ERROR_PARAM;
        goto end;
    }

    memset((uint8_t *)p_array, 0, sizeof(mibeacon_object_array_t));

    while(len > 0) {
        if (p_array->num >= MAX_OBJECT_IN_BEACON) {
            ret = MIIO_ERROR_FULL;
            goto end;
        }
        // p_object = (mibeacon_object_t *)p_buffer;
        memset((uint8_t *)p_object, 0, sizeof(mibeacon_object_t));
        memcpy((uint8_t *)&p_object->id, p_buffer, sizeof(uint16_t));
        p_buffer += sizeof(uint16_t);
        memcpy((uint8_t *)&p_object->data_len, p_buffer, sizeof(uint8_t));
        p_buffer += sizeof(uint8_t);
        if (p_object->data_len > MAX_DATA_LEN_IN_OBJECT) {
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "check object single length error");
            ret = MIIO_ERROR_SIZE;
            goto end;
        }
        memcpy((uint8_t *)&p_object->data[0], p_buffer, sizeof(uint8_t) * p_object->data_len);
        p_buffer += (sizeof(uint8_t) * p_object->data_len);

        obj_len = 3 + p_object->data_len;
        if (len < obj_len) {
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "check object total length error");
            ret = MIIO_ERROR_SIZE;
            goto end;
        }

        memcpy((uint8_t *)&p_array->object[p_array->num], (uint8_t *)p_object, sizeof(mibeacon_object_t));
        len -= obj_len;
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Get object from beacon index = %d; id = %04x; len = %d; data = ",
                      p_array->num, p_array->object[p_array->num].id,
                      p_array->object[p_array->num].data_len);
        // arch_dump_hex(p_array->object[p_array->num].data, p_array->object[p_array->num].data_len, NULL);
        p_array->num++;
    }

end:
    if (NULL != p_object) {
        free(p_object);
        p_object = NULL;
    }

    return ret;
}

static int mibeacon_v4_decrypt(uint8_t *out, uint8_t *in, uint16_t len,
                               mibeacon_decrypt_config_t *p_cfg)
{
    int ret = MIIO_OK;
    uint8_t *buffer = NULL;
    struct tc_ccm_mode_struct mode;
    uint8_t adddata[] = {0x11};

    buffer = (uint8_t *)malloc(sizeof(uint8_t) * (len + 4));
    if (buffer == NULL) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for decrypt");
        return MIIO_ERROR_NOMEM;
    }
    memcpy(buffer, in, len);
    memcpy(buffer + len, &p_cfg->mic, 4);

    uint8_t nonce[12];
    memcpy(nonce, p_cfg->mac, 6);
    memcpy(nonce + 6, &p_cfg->product_id, 2);
    memcpy(nonce + 8, &p_cfg->counter, 4);

    ret = tc_ccm_config(&mode, p_cfg->key, nonce, sizeof(nonce), 4);
    if (TC_CRYPTO_SUCCESS != ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "%s -- fail to config", __func__);
        ret = MIIO_ERROR;
        goto end;
    }

    ret = tc_ccm_decryption_verification(out, len, adddata, sizeof(adddata), buffer, len + 4, &mode);
    if (TC_CRYPTO_SUCCESS != ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "%s -- fail to decrypt", __func__);
        ret = MIIO_ERROR;
        goto end;
    }
    ret = MIIO_OK;

end:
    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }

    return ret;
}

static int mibeacon_legacy_decrypt(uint8_t *out, uint8_t *in, uint16_t len,
                                   mibeacon_decrypt_config_t *p_cfg)
{
    int ret = MIIO_OK;
    uint8_t *buffer = NULL;
    struct tc_ccm_mode_struct mode;
    uint8_t adddata = 0x11;

    buffer = (uint8_t *)malloc(sizeof(uint8_t) * (len + 1));
    if (buffer == NULL) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for legacy decrypt");
        return MIIO_ERROR_NOMEM;
    }
    memcpy(buffer, in, len);
    memcpy(buffer + len, &p_cfg->mic, 1);

    p_cfg->key[15] = p_cfg->key[11];
    p_cfg->key[14] = p_cfg->key[10];
    p_cfg->key[13] = p_cfg->key[9];
    p_cfg->key[12] = p_cfg->key[8];
    p_cfg->key[11] = p_cfg->key[7];
    p_cfg->key[10] = p_cfg->key[6];
    p_cfg->key[6] = 0x8D;
    p_cfg->key[7] = 0x3D;
    p_cfg->key[8] = 0x3C;
    p_cfg->key[9] = 0x97;

    uint8_t nonce[13];
    memcpy(nonce, &p_cfg->frame_ctrl, 2);
    memcpy(nonce + 2, &p_cfg->product_id, 2);
    memcpy(nonce + 4, &p_cfg->counter, 4);
    memcpy(nonce + 8, p_cfg->mac, 5);

    ret = tc_ccm_config(&mode, p_cfg->key, nonce, sizeof(nonce), 1);
    if (TC_CRYPTO_SUCCESS != ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "%s -- fail to config", __func__);
        ret = MIIO_ERROR;
        goto end;
    }

    ret = tc_ccm_decryption_verification(out, len, &adddata, sizeof(adddata), buffer, len + 1, &mode);
    if (TC_CRYPTO_SUCCESS != ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "%s -- fail to decrypt", __func__);
        ret = MIIO_ERROR;
        goto end;
    }
    ret = MIIO_OK;

end:
    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }

    return ret;
}

static int mibeacon_check_linkage(mibeacon_info_t *p_info)
{
    mibeacon_decrypt_config_t decrypt_config;
    mibeacon_object_array_t object_array = {0};
    uint8_t object[MAX_MULTI_OBJECT_LEN] = {0};
    int ret;
    bool data_valid = false;

    if (0 == p_info->object_len) {
        return MIIO_ERROR_SIZE;
    }

    mible_linkage_search_result_update(p_info);

    /* Check state of this devie */
    if (MIIO_OK != mible_linkage_check_state(p_info->mac, p_info->product_id)) {
        return MIIO_ERROR;
    }

    /* Check if this beacon should be encrypted */
    if (0 == p_info->encrypted && mible_linkage_should_encrypted(p_info->mac, p_info->product_id)) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "The beacon of device %02x:%02x:%02x:%02x:%02x:%02x (pid %d) should be encrypted",
                        p_info->mac[5], p_info->mac[4], p_info->mac[3], p_info->mac[2],
                        p_info->mac[1], p_info->mac[0], p_info->product_id);
        return MIIO_ERROR;
    }

    /* Check if this beacon should be anti-replay */
    if (p_info->version < 5 && mible_linkage_should_anti_replay(p_info->mac, p_info->product_id)) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "The beacon of device %02x:%02x:%02x:%02x:%02x:%02x (pid %d) should be anti-replay",
                        p_info->mac[5], p_info->mac[4], p_info->mac[3], p_info->mac[2],
                        p_info->mac[1], p_info->mac[0], p_info->product_id);
        return MIIO_ERROR;
    }

    if (p_info->encrypted == 1) {
        memset(&decrypt_config, 0xFF, sizeof(decrypt_config));
        if (MIIO_OK != mible_linkage_get_key(p_info->mac, p_info->product_id, &decrypt_config.key[0])) {
            return MIIO_ERROR_NOTFOUND;
        }
        decrypt_config.frame_ctrl = p_info->frame_ctrl;
        decrypt_config.product_id = p_info->product_id;
        decrypt_config.counter = p_info->counter;
        decrypt_config.mic = p_info->mic;
        memcpy(decrypt_config.mac, p_info->mac, 6);
        if (p_info->version >= 4 && p_info->version <= 5) {
            ret = mibeacon_v4_decrypt(object, p_info->object,
                                      p_info->object_len, &decrypt_config);
            if (MIIO_OK != ret) {
                LOG_WARN_TAG(MIBLE_LOG_TAG, "Fail to decrypt with version %d", p_info->version);
                return ret;
            }
        } else if (p_info->version >= 2 && p_info->version <= 3) {
            ret = mibeacon_legacy_decrypt(object, p_info->object,
                                          p_info->object_len, &decrypt_config);
            if (MIIO_OK != ret) {
                LOG_WARN_TAG(MIBLE_LOG_TAG, "Fail to decrypt with version %d", p_info->version);
                return ret;
            }
        } else {
            LOG_WARN_TAG(MIBLE_LOG_TAG, "Decryption with unknow version %d", p_info->version);
            return MIIO_ERROR;
        }
    } else {
        memcpy(object, p_info->object, p_info->object_len);
    }

    ret = mibeacon_get_objects(object, p_info->object_len, &object_array);
    if (MIIO_OK != ret) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Get objects format error");
        return ret;
    }

    if (0 == object_array.num) {
        return MIIO_ERROR_EMPTY;
    }

    for (int index = 0; index < object_array.num; ++index) {
        if (mible_linkage_check_counter(p_info->mac, p_info->product_id,
                                        p_info->counter, p_info->version >= 5)) {

            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Object ID %04x with counter %u",
                               object_array.object[index].id, p_info->counter);
            if (mible_linkage_check_last_report(p_info->mac, p_info->product_id,
                                                 object_array.object[index].id)) {

                //mible_hooks_get_object(p_info->mac, &object_array.object[index]);
                mible_hooks_linkage_event_report(p_info->mac, p_info->product_id, &object_array.object[index]);
                
                mible_linkage_object_timestamp_update(p_info->mac, p_info->product_id,
                                                 object_array.object[index].id, arch_os_time_now());
                data_valid = true;
            }
        }
    }

    /* Only if beacon is encrypted and object is valid, state of this device is stored */
    if (data_valid && 1 == p_info->encrypted) {
        mible_linkage_attribute_update(p_info->mac, p_info->product_id,
                                    1 == p_info->encrypted, p_info->version >= 5);
    }

    return MIIO_OK;
}

