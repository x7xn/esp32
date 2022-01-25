/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include "util.h"
#include "arch_def.h"
#include "arch_psm.h"
#include "mible_def.h"
#include "mible_prph.h"
#include "mibeacon_def.h"
#include "list/list.h"

#undef  MIBLE_LOG_TAG
#define MIBLE_LOG_TAG                                     "mible_prph"
#undef  LOG_LEVEL
#define LOG_LEVEL                                         LOG_LEVEL_INFO

/* Private define ------------------------------------------------------------*/

#define MIBLE_PERIPHERAL_MAX_NUM                          10
#define MIBLE_PERIPHERAL_SYNC_ITVL                        60

/* Private typedef -----------------------------------------------------------*/

typedef struct {
    uint32_t cur_num;
    mible_prph_t prph[MIBLE_PERIPHERAL_MAX_NUM];
    arch_os_mutex_handle_t mutex;
} mible_prph_table_t;

typedef struct {
    list_head_t node;
    mible_addr_t address;
    uint16_t product_id;
    bool encrypted;
    bool anti_replay;
} mible_prph_attribute_t;

/* Private variables ---------------------------------------------------------*/

static mible_prph_table_t prph_table;
static bool need_sync = false;
static list_head_t prph_hdr;

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

int mible_prph_init(void)
{
    int ret;
    mible_prph_table_t *p_prph_table = NULL;

    ret = arch_os_mutex_create(&prph_table.mutex);
    if (MIIO_OK != ret) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "fail to create mutex (err %d)", ret);
        return ret;
    }

    p_prph_table = (mible_prph_table_t *)malloc(sizeof(mible_prph_table_t));
    if (NULL == p_prph_table) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for prph table.");
        return MIIO_ERROR_NOMEM;
    }

    ret = arch_psm_get_value("ble", "event_rule", p_prph_table, sizeof(mible_prph_table_t));

    arch_os_mutex_get(prph_table.mutex, ARCH_OS_WAIT_FOREVER);
    if (sizeof(mible_prph_table_t) == ret) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Found valid device table.");
        prph_table.cur_num = p_prph_table->cur_num;
        memcpy(&prph_table.prph[0], &p_prph_table->prph[0], sizeof(mible_prph_t) * MIBLE_PERIPHERAL_MAX_NUM);
    } else {
        prph_table.cur_num = 0;
        memset(&prph_table.prph[0], 0xFF, sizeof(mible_prph_t) * MIBLE_PERIPHERAL_MAX_NUM);
    }
    INIT_LIST_HEAD(&prph_hdr);
    arch_os_mutex_put(prph_table.mutex);

    if (p_prph_table) {
        free(p_prph_table);
    }

    return MIIO_OK;
}

int mible_prph_reset(void)
{
    mible_prph_attribute_t *p_attribute, *p_tmp;

    arch_os_mutex_get(prph_table.mutex, ARCH_OS_WAIT_FOREVER);

    prph_table.cur_num = 0;
    memset(&prph_table.prph[0], 0xFF, sizeof(mible_prph_t) * MIBLE_PERIPHERAL_MAX_NUM);
    //need_sync = true;
    int ret = arch_psm_set_value("ble", "event_rule", &prph_table, sizeof(prph_table));
    if (sizeof(prph_table) == ret) {
        ret = MIIO_OK;
    } else {
        ret  = MIIO_ERROR_SIZE;
    }

    list_for_each_entry_safe(p_attribute, p_tmp, &prph_hdr, node, mible_prph_attribute_t) {
        list_del(&p_attribute->node);
        free(p_attribute);
    }

    arch_os_mutex_put(prph_table.mutex);

    return ret;
}

uint32_t mible_prph_dump(char *buf, uint32_t buf_size)
{
    mible_prph_t tmp;
    uint32_t n = 0;
    bool label = false;

    memset(&tmp, 0xFF, sizeof(mible_prph_t));

    n += snprintf_safe(buf + n, buf_size - n, "[");
    arch_os_mutex_get(prph_table.mutex, ARCH_OS_WAIT_FOREVER);

    for (int index = 0; index < MIBLE_PERIPHERAL_MAX_NUM && prph_table.cur_num > 0; ++index) {
        if (0 != memcmp(&tmp, &prph_table.prph[index], sizeof(mible_prph_t))) {
            if (label) {
                n += snprintf_safe(buf + n, buf_size - n, ",");
            }
            n += snprintf_safe(buf + n, buf_size - n, "{\"mac\":\"");
            n += snprintf_hex(buf + n, buf_size - n,
                              prph_table.prph[index].address, 6, 0);
            n += snprintf_safe(buf + n, buf_size - n, "\",");
            n += snprintf_safe(buf + n, buf_size - n, "\"evtid\":%d,",
                              prph_table.prph[index].object_id);
            n += snprintf_safe(buf + n, buf_size - n, "\"pid\":%d,",
                              prph_table.prph[index].product_id);
            n += snprintf_safe(buf + n, buf_size - n, "\"beaconkey\":\"");
            n += snprintf_hex(buf + n, buf_size - n, prph_table.prph[index].beacon_key,
                              MISERVICE_CHAR_BEACONKEY_LEN, 0);
            n += snprintf_safe(buf + n, buf_size - n, "\"}");
            label = true;
        }
    }

    arch_os_mutex_put(prph_table.mutex);
    n += snprintf_safe(buf + n, buf_size - n, "]");

    return n;
}

int mible_prph_add(mible_addr_t address, uint16_t product_id,
                    uint16_t object_id, uint8_t * p_key)
{
    int index, ret = MIIO_ERROR_FULL;
    mible_prph_t tmp;

    if (MIBLE_PERIPHERAL_MAX_NUM == prph_table.cur_num) {
        return ret;
    }

    memset(&tmp, 0xFF, sizeof(mible_prph_t));

    arch_os_mutex_get(prph_table.mutex, ARCH_OS_WAIT_FOREVER);

    for (index = 0; index < MIBLE_PERIPHERAL_MAX_NUM; ++index) {
        if (0 == memcmp(prph_table.prph[index].address, address, sizeof(mible_addr_t)) &&
            object_id == prph_table.prph[index].object_id &&
            product_id == prph_table.prph[index].product_id) {

            /* For a existed device, update its beacon key and restore its counter */
            memcpy(prph_table.prph[index].beacon_key, p_key, MISERVICE_CHAR_BEACONKEY_LEN);
            prph_table.prph[index].frame_counter = 0;
            //need_sync = true;
            ret = arch_psm_set_value("ble", "event_rule", &prph_table, sizeof(prph_table));
            if (sizeof(prph_table) == ret) {
                ret = MIIO_OK;
            } else {
                ret  = MIIO_ERROR_SIZE;
            }
            goto end;
        }
    }

    for (index = 0; index < MIBLE_PERIPHERAL_MAX_NUM; ++index) {
        if (0 == memcmp(&tmp, &prph_table.prph[index], sizeof(mible_prph_t))) {
            memcpy(prph_table.prph[index].address, address, sizeof(mible_addr_t));
            memcpy(prph_table.prph[index].beacon_key, p_key, MISERVICE_CHAR_BEACONKEY_LEN);
            prph_table.prph[index].object_id = object_id;
            prph_table.prph[index].product_id = product_id;
            /* The default counter is 0, then it will be updated by beacon handler */
            prph_table.prph[index].frame_counter = 0;
            ++prph_table.cur_num;
            //need_sync = true;
            ret = arch_psm_set_value("ble", "event_rule", &prph_table, sizeof(prph_table));
            if (sizeof(prph_table) == ret) {
                ret = MIIO_OK;
            } else {
                ret  = MIIO_ERROR_SIZE;
            }
            goto end;
        }
    }

end:
    arch_os_mutex_put(prph_table.mutex);

#if (defined(MIBLE_LEGACY_ENABLE) && MIBLE_LEGACY_ENABLE)
    if (MIIO_OK == ret) {
        void mible_legacy_cmd_insert(const char *command);
        mible_legacy_cmd_insert("ble_event list");
    }
#endif

    return ret;
}

int mible_prph_delete(mible_addr_t address, uint16_t object_id)
{
    int index, ret = MIIO_ERROR_NOTFOUND;
    mible_prph_attribute_t *p_attribute, *p_tmp;

    if (0 == prph_table.cur_num) {
        return ret;
    }

    arch_os_mutex_get(prph_table.mutex, ARCH_OS_WAIT_FOREVER);

    for (index = 0; index < MIBLE_PERIPHERAL_MAX_NUM; ++index) {
        if (0 == memcmp(prph_table.prph[index].address, address, sizeof(mible_addr_t)) &&
            object_id == prph_table.prph[index].object_id) {

            memset(&prph_table.prph[index], 0xFF, sizeof(mible_prph_t));
            --prph_table.cur_num;
            //need_sync = true;
            //ret = MIIO_OK;
            ret = arch_psm_set_value("ble", "event_rule", &prph_table, sizeof(prph_table));
            if (sizeof(prph_table) == ret) {
                ret = MIIO_OK;
            } else {
                ret  = MIIO_ERROR_SIZE;
            }
            break;
        }
    }

    list_for_each_entry_safe(p_attribute, p_tmp, &prph_hdr, node, mible_prph_attribute_t) {
        if (0 == memcmp(p_attribute->address, address, sizeof(mible_addr_t))) {
            list_del(&p_attribute->node);
            free(p_attribute);
            break;
        }
    }

    arch_os_mutex_put(prph_table.mutex);

    return ret;
}

int mible_prph_find_key(mible_addr_t address, uint16_t product_id,
                                    uint8_t *p_key)
{
    int index, ret = MIIO_ERROR_NOTFOUND;

    if (NULL == p_key) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Invalid parameter");
        return MIIO_ERROR_PARAM;
    }

    if (0 == prph_table.cur_num) {
        return MIIO_ERROR_NOTFOUND;
    }

    arch_os_mutex_get(prph_table.mutex, ARCH_OS_WAIT_FOREVER);
    /* Xiaomi band is a exception, the PID might be wrong */
    for (index = 0; index < MIBLE_PERIPHERAL_MAX_NUM; ++index) {
        if (0 == memcmp(prph_table.prph[index].address, address, sizeof(mible_addr_t)) &&
            (product_id == prph_table.prph[index].product_id || MIBLE_BAND_PID == product_id)) {

            ret = MIIO_OK;
            memcpy(p_key, prph_table.prph[index].beacon_key, MISERVICE_CHAR_BEACONKEY_LEN);
            break;
        }
    }

    arch_os_mutex_put(prph_table.mutex);

    return ret;
}

bool mible_prph_check_object(mible_addr_t address, uint16_t product_id,
                                            uint16_t object_id)
{
    int index;

    if (0 == prph_table.cur_num) {
        return false;
    }

    arch_os_mutex_get(prph_table.mutex, ARCH_OS_WAIT_FOREVER);

    for (index = 0; index < MIBLE_PERIPHERAL_MAX_NUM; ++index) {
        if (0 == memcmp(prph_table.prph[index].address, address, sizeof(mible_addr_t)) &&
            product_id == prph_table.prph[index].product_id &&
            object_id == prph_table.prph[index].object_id) {

            break;
        }
    }

    arch_os_mutex_put(prph_table.mutex);

    return (index < MIBLE_PERIPHERAL_MAX_NUM);
}

bool mible_prph_check_counter(mible_addr_t address, uint16_t product_id,
                            uint16_t object_id, uint32_t frame_counter, bool anti_replay)
{
    int index;
    bool ret = false;

    if (0 == prph_table.cur_num) {
        return ret;
    }

    arch_os_mutex_get(prph_table.mutex, ARCH_OS_WAIT_FOREVER);
    /* Xiaomi band is a exception, the PID might be wrong */
    for (index = 0; index < MIBLE_PERIPHERAL_MAX_NUM; ++index) {
        if (0 == memcmp(prph_table.prph[index].address, address, sizeof(mible_addr_t)) &&
            (product_id == prph_table.prph[index].product_id || MIBLE_BAND_PID == product_id) &&
            object_id == prph_table.prph[index].object_id) {

            if (frame_counter == prph_table.prph[index].frame_counter) {
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Same counter, ignore it.");
            } else if (anti_replay) {
                /* The default frame counter in table is 0, in case new counter is bigger than 0x80000000 */
                if (0 == prph_table.prph[index].frame_counter ||
                    (int32_t)(frame_counter - prph_table.prph[index].frame_counter) > 0) {

                    prph_table.prph[index].frame_counter = frame_counter;
                    need_sync = true;
                    ret = true;
                } else {
                    LOG_WARN_TAG(MIBLE_LOG_TAG, "The counter should be bigger");
                }
            } else {
                /* The counter is different, then should be handled */
                prph_table.prph[index].frame_counter = frame_counter;
                need_sync = true;
                ret = true;
            }
            break;
        }
    }

    arch_os_mutex_put(prph_table.mutex);

    return ret;
}

void mible_prph_sync(void)
{
    static uint32_t ts_last = 0;
    int ret;

    if (need_sync &&
        (int32_t)(arch_os_time_now() - ts_last - MIBLE_PERIPHERAL_SYNC_ITVL) > 0) {

        arch_os_mutex_get(prph_table.mutex, ARCH_OS_WAIT_FOREVER);
        ret = arch_psm_set_value("ble", "event_rule", &prph_table, sizeof(prph_table));
        if (sizeof(prph_table) == ret) {
            need_sync = false;
            ts_last = arch_os_time_now();
        }
        arch_os_mutex_put(prph_table.mutex);
    }
}

bool mible_prph_should_encrypted(mible_addr_t address, uint16_t product_id)
{
    bool ret = false;
    mible_prph_attribute_t *p_attribute;

    arch_os_mutex_get(prph_table.mutex, ARCH_OS_WAIT_FOREVER);
    list_for_each_entry(p_attribute, &prph_hdr, node, mible_prph_attribute_t) {
        if (0 == memcmp(p_attribute->address, address, sizeof(mible_addr_t)) &&
            p_attribute->product_id == product_id) {

            ret = p_attribute->encrypted;
            break;
        }
    }
    arch_os_mutex_put(prph_table.mutex);

    return ret;
}

bool mible_prph_should_anti_replay(mible_addr_t address, uint16_t product_id)
{
    bool ret = false;
    mible_prph_attribute_t *p_attribute;

    arch_os_mutex_get(prph_table.mutex, ARCH_OS_WAIT_FOREVER);
    list_for_each_entry(p_attribute, &prph_hdr, node, mible_prph_attribute_t) {
        if (0 == memcmp(p_attribute->address, address, sizeof(mible_addr_t)) &&
            p_attribute->product_id == product_id) {

            ret = p_attribute->anti_replay;
            break;
        }
    }
    arch_os_mutex_put(prph_table.mutex);

    return ret;
}

void mible_prph_attribute_update(mible_addr_t address, uint16_t product_id,
                                bool encrypted, bool anti_replay)
{
    mible_prph_attribute_t *p_attribute;

    arch_os_mutex_get(prph_table.mutex, ARCH_OS_WAIT_FOREVER);
    list_for_each_entry(p_attribute, &prph_hdr, node, mible_prph_attribute_t) {
        if (0 == memcmp(p_attribute->address, address, sizeof(mible_addr_t)) &&
            p_attribute->product_id == product_id) {

            p_attribute->encrypted = encrypted;
            p_attribute->anti_replay = anti_replay;
            goto end;
        }
    }

    p_attribute = (mible_prph_attribute_t *)malloc(sizeof(mible_prph_attribute_t));
    if (NULL == p_attribute) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "No memory for peripheral attribute");
        goto end;
    }

    memcpy(p_attribute->address, address, sizeof(mible_addr_t));
    p_attribute->product_id = product_id;
    p_attribute->encrypted = encrypted;
    p_attribute->anti_replay = anti_replay;
    list_add_tail(&p_attribute->node, &prph_hdr);

end:
    arch_os_mutex_put(prph_table.mutex);
}
