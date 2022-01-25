/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include "util.h"
#include "arch_def.h"
#include "arch_psm.h"
#include "mible_def.h"
#include "mible_linkage.h"
#include "list/list.h"
#include "mible_hooks.h"

#undef  MIBLE_LOG_TAG
#define MIBLE_LOG_TAG                                     "mible_linkage"
#undef  LOG_LEVEL
#define LOG_LEVEL                                         LOG_LEVEL_INFO

/* Private typedef -----------------------------------------------------------*/

typedef struct {
    uint32_t cur_num;
    linkage_device_t linkage[MIBLE_LINKAGE_MAX_NUM];
    arch_os_mutex_handle_t mutex;
} mible_linkage_table_t;

typedef struct {
    list_head_t node;
    mible_addr_t address;
    uint16_t product_id;
    bool encrypted;
    bool anti_replay;
} mible_linkage_attribute_t;

/* Private variables ---------------------------------------------------------*/

static mible_linkage_table_t linkage_table;
static bool need_sync = false;
static list_head_t linkage_hdr;

/* Private function prototypes -----------------------------------------------*/


/* Exported functions --------------------------------------------------------*/

int mible_linkage_init(void)
{
    int ret;
    mible_linkage_table_t *p_linkage_table = NULL;

    ret = arch_os_mutex_create(&linkage_table.mutex);
    if (MIIO_OK != ret) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "fail to create mutex (err %d)", ret);
        return ret;
    }

    p_linkage_table = (mible_linkage_table_t *)malloc(sizeof(mible_linkage_table_t));
    if (NULL == p_linkage_table) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for linkage table.");
        return MIIO_ERROR_NOMEM;
    }

    ret = arch_psm_get_value("ble", "linkage_rule", p_linkage_table, sizeof(mible_linkage_table_t));

    arch_os_mutex_get(linkage_table.mutex, ARCH_OS_WAIT_FOREVER);
    if (sizeof(mible_linkage_table_t) == ret) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Found valid device table.");
        linkage_table.cur_num = p_linkage_table->cur_num;
        memcpy(&linkage_table.linkage[0], &p_linkage_table->linkage[0], sizeof(linkage_device_t) * MIBLE_LINKAGE_MAX_NUM);

        // add init for object timestamp
        linkage_device_t tmp;
        memset(&tmp, 0xFF, sizeof(linkage_device_t));
        for(int i=0; i<MIBLE_LINKAGE_MAX_NUM; i++)
        {
            if (0 != memcmp(&tmp, &linkage_table.linkage[i], sizeof(linkage_device_t))) 
            {
                for(int j=0; j<MIBLE_LINKAGE_OBJECT_MAX_NUM; j++)
                {
                    if(0xFFFF == linkage_table.linkage[i].object[j].object_id)
                    {
                        continue;
                    }
                    else
                    {
                        linkage_table.linkage[i].object[j].timestamp = 0;
                    }
                }
            }
        }
    } else {
        linkage_table.cur_num = 0;
        memset(&linkage_table.linkage[0], 0xFF, sizeof(linkage_device_t) * MIBLE_LINKAGE_MAX_NUM);
    }
    INIT_LIST_HEAD(&linkage_hdr);
    arch_os_mutex_put(linkage_table.mutex);

    if (p_linkage_table) {
        free(p_linkage_table);
    }

    //mible_hooks_linkage_rule_report();

    return MIIO_OK;
}

int mible_linkage_add(linkage_device_t *device)
{
    int index, ret = MIIO_ERROR_FULL;
    linkage_device_t tmp;

    if (MIBLE_LINKAGE_MAX_NUM == linkage_table.cur_num) {
        return ret;
    }

    if (device == NULL) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "mible_linkage_add, Invalid parameter");
        return MIIO_ERROR_PARAM;
    }

    memset(&tmp, 0xFF, sizeof(linkage_device_t));

    arch_os_mutex_get(linkage_table.mutex, ARCH_OS_WAIT_FOREVER);

    for (index = 0; index < MIBLE_LINKAGE_MAX_NUM; ++index) {
        // compare mac and pid
        if (0 == memcmp(linkage_table.linkage[index].address, device->address, sizeof(mible_addr_t)) &&
            device->product_id == linkage_table.linkage[index].product_id) {

            /* For a existed device, update its beacon key, frame_counter and object*/
            memcpy(linkage_table.linkage[index].beaconkey, device->beaconkey, MIBEACON_BEACONKEY_LEN);
            memcpy(linkage_table.linkage[index].object, device->object, sizeof(linkage_object_t) * MIBLE_LINKAGE_OBJECT_MAX_NUM);
            linkage_table.linkage[index].frame_counter = 0;
            //need_sync = true;
            ret = arch_psm_set_value("ble", "linkage_rule", &linkage_table, sizeof(linkage_table));
            if (sizeof(linkage_table) == ret) {
	            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "arch_psm_set_value OK");
	            ret = MIIO_OK;
            } else {
	            LOG_ERROR_TAG(MIBLE_LOG_TAG, "arch_psm_set_value error");
	            ret = MIIO_ERROR_SIZE;
            }
            goto end;
        }
    }

    for (index = 0; index < MIBLE_LINKAGE_MAX_NUM; ++index) {
        if (0 == memcmp(&tmp, &linkage_table.linkage[index], sizeof(linkage_device_t))) {
            memcpy(&linkage_table.linkage[index], device, sizeof(linkage_device_t));
            ++linkage_table.cur_num;
            //need_sync = true;
            ret = arch_psm_set_value("ble", "linkage_rule", &linkage_table, sizeof(linkage_table));
            if (sizeof(linkage_table) == ret) {
	            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "arch_psm_set_value OK");
	            ret = MIIO_OK;
            } else {
	            LOG_ERROR_TAG(MIBLE_LOG_TAG, "arch_psm_set_value error");
	            ret = MIIO_ERROR_SIZE;
            }
            goto end;
        }
    }

end:
    arch_os_mutex_put(linkage_table.mutex);

    mible_hooks_linkage_rule_report();

    return ret;
}

int mible_linkage_delete(linkage_device_t *device)
{
    int index, ret = MIIO_ERROR_NOTFOUND;
    mible_linkage_attribute_t *p_attribute, *p_tmp;

    if (0 == linkage_table.cur_num) {
        return ret;
    }

    if (device == NULL) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "mible_linkage_delete, Invalid parameter");
        return MIIO_ERROR_PARAM;
    }

    arch_os_mutex_get(linkage_table.mutex, ARCH_OS_WAIT_FOREVER);

    for (index = 0; index < MIBLE_LINKAGE_MAX_NUM; ++index) {
        // compare mac and pid
        if (0 == memcmp(linkage_table.linkage[index].address, device->address, sizeof(mible_addr_t))
         && linkage_table.linkage[index].product_id == device->product_id ) {
            memset(&linkage_table.linkage[index], 0xFF, sizeof(linkage_device_t));
            --linkage_table.cur_num;
            //need_sync = true;
            ret = arch_psm_set_value("ble", "linkage_rule", &linkage_table, sizeof(linkage_table));
            if (sizeof(linkage_table) == ret) {
	            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "arch_psm_set_value OK");
	            ret = MIIO_OK;
            } else {
	            LOG_ERROR_TAG(MIBLE_LOG_TAG, "arch_psm_set_value error");
	            ret = MIIO_ERROR_SIZE;
            }
            break;
        }
    }

    list_for_each_entry_safe(p_attribute, p_tmp, &linkage_hdr, node, mible_linkage_attribute_t) {
        if (0 == memcmp(p_attribute->address, device->address, sizeof(mible_addr_t))) {

            list_del(&p_attribute->node);
            free(p_attribute);
            break;
        }
    }

    arch_os_mutex_put(linkage_table.mutex);

    mible_hooks_linkage_rule_report();

    return ret;
}

int mible_linkage_get_specified_device(linkage_device_t *device)
{
    int ret = MIIO_ERROR_NOTFOUND;
    int index;

    if (device == NULL) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "mible_linkage_get_specified_device, Invalid parameter");
        return MIIO_ERROR_PARAM;
    }

    arch_os_mutex_get(linkage_table.mutex, ARCH_OS_WAIT_FOREVER);

    for (index = 0; index < MIBLE_LINKAGE_MAX_NUM; ++index) {
        // compare mac and pid
        if (0 == memcmp(linkage_table.linkage[index].address, device->address, sizeof(mible_addr_t)) && 
            device->product_id == linkage_table.linkage[index].product_id) {

            memcpy(device, &linkage_table.linkage[index], sizeof(linkage_device_t));
            ret = MIIO_OK;
            break;
        }
    }

    arch_os_mutex_put(linkage_table.mutex);

    return ret;
}

int mible_linkage_get_all(linkage_device_array_t *device_list)
{
    int index;
    linkage_device_t tmp;
    uint16_t list_index = 0;

    if (device_list == NULL) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "mible_linkage_get_all, Invalid parameter");
        return MIIO_ERROR_PARAM;
    }

    memset(&tmp, 0xFF, sizeof(linkage_device_t));

    arch_os_mutex_get(linkage_table.mutex, ARCH_OS_WAIT_FOREVER);

    for (index = 0; index < MIBLE_LINKAGE_MAX_NUM; ++index) {
        if (0 != memcmp(&tmp, &linkage_table.linkage[index], sizeof(linkage_device_t))) {
            memcpy(&device_list->device[list_index], &linkage_table.linkage[index], sizeof(linkage_device_t));
            list_index++;
        }
    }
    device_list->cur_num = list_index;

    arch_os_mutex_put(linkage_table.mutex);

    return MIIO_OK;
}

int mible_linkage_reset(void)
{
    mible_linkage_attribute_t *p_attribute, *p_tmp;

    arch_os_mutex_get(linkage_table.mutex, ARCH_OS_WAIT_FOREVER);

    linkage_table.cur_num = 0;
    memset(&linkage_table.linkage[0], 0xFF, sizeof(linkage_device_t) * MIBLE_LINKAGE_MAX_NUM);
    //need_sync = true;
    int ret = arch_psm_set_value("ble", "linkage_rule", &linkage_table, sizeof(linkage_table));
    if (sizeof(linkage_table) == ret) {
	    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "arch_psm_set_value OK");
	    ret = MIIO_OK;
    } else {
	    LOG_ERROR_TAG(MIBLE_LOG_TAG, "arch_psm_set_value error");
	    ret = MIIO_ERROR_SIZE;
    }

    list_for_each_entry_safe(p_attribute, p_tmp, &linkage_hdr, node, mible_linkage_attribute_t) {
        list_del(&p_attribute->node);
        free(p_attribute);
    }

    arch_os_mutex_put(linkage_table.mutex);

    mible_hooks_linkage_rule_report();

    return ret;
}

int mible_linkage_enable(linkage_device_t *device)
{
    int index, ret = MIIO_ERROR_NOTFOUND;

    arch_os_mutex_get(linkage_table.mutex, ARCH_OS_WAIT_FOREVER);

    for (index = 0; index < MIBLE_LINKAGE_MAX_NUM; ++index) {
        // compare mac and pid
        if (0 == memcmp(linkage_table.linkage[index].address, device->address, sizeof(mible_addr_t)) && 
            device->product_id == linkage_table.linkage[index].product_id) {

            linkage_table.linkage[index].enabled = device->enabled;
            //need_sync = true;
            ret = arch_psm_set_value("ble", "linkage_rule", &linkage_table, sizeof(linkage_table));
            if (sizeof(linkage_table) == ret) {
	            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "arch_psm_set_value OK");
	            ret = MIIO_OK;
            } else {
	            LOG_ERROR_TAG(MIBLE_LOG_TAG, "arch_psm_set_value error");
	            ret = MIIO_ERROR_SIZE;
            }
            goto end;
        }
    }

end:
    arch_os_mutex_put(linkage_table.mutex);

    mible_hooks_linkage_rule_report();

    return ret;
}

int mible_linkage_enable_all(linkage_function_state_t state)
{
    int index, ret = MIIO_OK;
    linkage_device_t tmp;

    memset(&tmp, 0xFF, sizeof(linkage_device_t));

    arch_os_mutex_get(linkage_table.mutex, ARCH_OS_WAIT_FOREVER);

    for (index = 0; index < MIBLE_LINKAGE_MAX_NUM; ++index) {
        if (0 != memcmp(&tmp, &linkage_table.linkage[index], sizeof(linkage_device_t))) {
            linkage_table.linkage[index].enabled = state;
        }
    }

	ret = arch_psm_set_value("ble", "linkage_rule", &linkage_table, sizeof(linkage_table));
    if (sizeof(linkage_table) == ret) {
	    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "arch_psm_set_value OK");
	    ret = MIIO_OK;
    } else {
	    LOG_ERROR_TAG(MIBLE_LOG_TAG, "arch_psm_set_value error");
	    ret = MIIO_ERROR_SIZE;
    }

    arch_os_mutex_put(linkage_table.mutex);

    mible_hooks_linkage_rule_report();

    return ret;
}

uint32_t mible_linkage_rule_dump(char *buf, uint32_t buf_size)
{
    linkage_device_t tmp;
    uint32_t n = 0;

    memset(&tmp, 0xFF, sizeof(linkage_device_t));

    n += snprintf_safe(buf + n, buf_size - n, "ble_linkage_rule");
    arch_os_mutex_get(linkage_table.mutex, ARCH_OS_WAIT_FOREVER);

    for (int index = 0; index < MIBLE_LINKAGE_MAX_NUM && linkage_table.cur_num > 0; ++index) {
        if (0 != memcmp(&tmp, &linkage_table.linkage[index], sizeof(linkage_device_t))) {
            //MAC
            n += snprintf_safe(buf + n, buf_size - n, " %02X:%02X:%02X:%02X:%02X:%02X",
                        linkage_table.linkage[index].address[5], linkage_table.linkage[index].address[4],
                        linkage_table.linkage[index].address[3], linkage_table.linkage[index].address[2],
                        linkage_table.linkage[index].address[1], linkage_table.linkage[index].address[0]);
            //PID
            n += snprintf_safe(buf + n, buf_size - n, " %d", linkage_table.linkage[index].product_id);
            //func_tpye
            n += snprintf_safe(buf + n, buf_size - n, " %d", linkage_table.linkage[index].function_type);
            //enable
            n += snprintf_safe(buf + n, buf_size - n, " %d", linkage_table.linkage[index].enabled);
            //object_num
            int object_num = 0;
            for(int object_idx = 0; object_idx < MIBLE_LINKAGE_OBJECT_MAX_NUM; ++object_idx)
            {
                if(0xFFFF == linkage_table.linkage[index].object[object_idx].object_id)
                {
                    continue;
                }
                else
                {
                    object_num++;
                }
            }
            n += snprintf_safe(buf + n, buf_size - n, " %d", object_num);
            
            //object
            for(int object_idx = 0; object_idx < MIBLE_LINKAGE_OBJECT_MAX_NUM; ++object_idx)
            {
                if(0xFFFF != linkage_table.linkage[index].object[object_idx].object_id)
                {
                    //object id
                    n += snprintf_safe(buf + n, buf_size - n, " %d", linkage_table.linkage[index].object[object_idx].object_id);
                    //value type
                    n += snprintf_safe(buf + n, buf_size - n, " %d", linkage_table.linkage[index].object[object_idx].value_type);
                }
            }
        }
    }

    arch_os_mutex_put(linkage_table.mutex);

    return n;
}

int mible_linkage_check_state(mible_addr_t address, uint16_t product_id)
{
    int index, ret = MIIO_ERROR_NOTFOUND;

    if (0 == linkage_table.cur_num) {
        return MIIO_ERROR_NOTFOUND;
    }

    arch_os_mutex_get(linkage_table.mutex, ARCH_OS_WAIT_FOREVER);

    for (index = 0; index < MIBLE_LINKAGE_MAX_NUM; ++index) {
        if (0 == memcmp(linkage_table.linkage[index].address, address, sizeof(mible_addr_t)) &&
            product_id == linkage_table.linkage[index].product_id) {

            if (linkage_table.linkage[index].enabled == LINKAGE_FUNCTION_ENABLE) {
                ret = MIIO_OK;
            } else {
                ret = MIIO_ERROR_NOTREADY;
            }
            break;
        }
    }

    arch_os_mutex_put(linkage_table.mutex);

    return ret;
}

int mible_linkage_get_key(mible_addr_t address, uint16_t product_id,
                                    uint8_t *p_key)
{
    int index, ret = MIIO_ERROR_NOTFOUND;

    if (NULL == p_key) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Invalid parameter");
        return MIIO_ERROR_PARAM;
    }

    if (0 == linkage_table.cur_num) {
        return MIIO_ERROR_NOTFOUND;
    }

    arch_os_mutex_get(linkage_table.mutex, ARCH_OS_WAIT_FOREVER);

    for (index = 0; index < MIBLE_LINKAGE_MAX_NUM; ++index) {
        if (0 == memcmp(linkage_table.linkage[index].address, address, sizeof(mible_addr_t)) &&
            product_id == linkage_table.linkage[index].product_id) {

            ret = MIIO_OK;
            memcpy(p_key, linkage_table.linkage[index].beaconkey, MIBEACON_BEACONKEY_LEN);
            break;
        }
    }

    arch_os_mutex_put(linkage_table.mutex);

    return ret;
}

bool mible_linkage_check_last_report(mible_addr_t address, uint16_t product_id,
                                                    uint16_t object_id)
{
    bool ret = false;

    if (0 == linkage_table.cur_num) {
        return ret;
    }

    arch_os_mutex_get(linkage_table.mutex, ARCH_OS_WAIT_FOREVER);

    for (int index = 0; index < MIBLE_LINKAGE_MAX_NUM; ++index) {
        if (0 == memcmp(linkage_table.linkage[index].address, address, sizeof(mible_addr_t)) &&
            product_id == linkage_table.linkage[index].product_id) {

            for (int i = 0; i < MIBLE_LINKAGE_OBJECT_MAX_NUM; i++) {
                if (object_id == linkage_table.linkage[index].object[i].object_id) {
                    if ((int32_t)(arch_os_time_now() - linkage_table.linkage[index].object[i].timestamp - linkage_table.linkage[index].interval) > 0) {
                        ret = true;
                        break;
                    }
                }
            }
            break;
        }
    }

    arch_os_mutex_put(linkage_table.mutex);

    return ret;
}

bool mible_linkage_check_counter(mible_addr_t address, uint16_t product_id,
                                uint32_t frame_counter, bool anti_replay)
{
    int index;
    bool ret = false;

    if (0 == linkage_table.cur_num) {
        return ret;
    }

    arch_os_mutex_get(linkage_table.mutex, ARCH_OS_WAIT_FOREVER);

    for (index = 0; index < MIBLE_LINKAGE_MAX_NUM; ++index) {
        if (0 == memcmp(linkage_table.linkage[index].address, address, sizeof(mible_addr_t)) &&
            (product_id == linkage_table.linkage[index].product_id)) {

            if (frame_counter == linkage_table.linkage[index].frame_counter) {
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Same counter, ignore it.");
            } else if (anti_replay) {
                /* The default frame counter in table is 0, in case new counter is bigger than 0x80000000 */
                if (0 == linkage_table.linkage[index].frame_counter ||
                    (int32_t)(frame_counter - linkage_table.linkage[index].frame_counter) > 0) {

                    linkage_table.linkage[index].frame_counter = frame_counter;
                    need_sync = true;
                    ret = true;
                } else {
                    LOG_WARN_TAG(MIBLE_LOG_TAG, "The counter should be bigger");
                }
            } else {
                /* The counter is different, then should be handled */
                linkage_table.linkage[index].frame_counter = frame_counter;
                need_sync = true;
                ret = true;
            }
            break;
        }
    }

    arch_os_mutex_put(linkage_table.mutex);

    return ret;
}

void mible_linkage_sync(void)
{
    static uint32_t ts_last = 0;
    int ret;

    if (need_sync &&
        (int32_t)(arch_os_time_now() - ts_last - MIBLE_LINKAGE_SYNC_ITVL) > 0) {

        arch_os_mutex_get(linkage_table.mutex, ARCH_OS_WAIT_FOREVER);
        ret = arch_psm_set_value("ble", "linkage_rule", &linkage_table, sizeof(linkage_table));
        if (sizeof(linkage_table) == ret) {
            need_sync = false;
            ts_last = arch_os_time_now();
        }
        arch_os_mutex_put(linkage_table.mutex);
    }
}

bool mible_linkage_should_encrypted(mible_addr_t address, uint16_t product_id)
{
    bool ret = false;
    mible_linkage_attribute_t *p_attribute;

    arch_os_mutex_get(linkage_table.mutex, ARCH_OS_WAIT_FOREVER);
    list_for_each_entry(p_attribute, &linkage_hdr, node, mible_linkage_attribute_t) {
        if (0 == memcmp(p_attribute->address, address, sizeof(mible_addr_t)) &&
            p_attribute->product_id == product_id) {

            ret = p_attribute->encrypted;
            break;
        }
    }
    arch_os_mutex_put(linkage_table.mutex);

    return ret;
}

bool mible_linkage_should_anti_replay(mible_addr_t address, uint16_t product_id)
{
    bool ret = false;
    mible_linkage_attribute_t *p_attribute;

    arch_os_mutex_get(linkage_table.mutex, ARCH_OS_WAIT_FOREVER);
    list_for_each_entry(p_attribute, &linkage_hdr, node, mible_linkage_attribute_t) {
        if (0 == memcmp(p_attribute->address, address, sizeof(mible_addr_t)) &&
            p_attribute->product_id == product_id) {

            ret = p_attribute->anti_replay;
            break;
        }
    }
    arch_os_mutex_put(linkage_table.mutex);

    return ret;
}

void mible_linkage_attribute_update(mible_addr_t address, uint16_t product_id,
                                bool encrypted, bool anti_replay)
{
    mible_linkage_attribute_t *p_attribute;

    arch_os_mutex_get(linkage_table.mutex, ARCH_OS_WAIT_FOREVER);
    list_for_each_entry(p_attribute, &linkage_hdr, node, mible_linkage_attribute_t) {
        if (0 == memcmp(p_attribute->address, address, sizeof(mible_addr_t)) &&
            p_attribute->product_id == product_id) {

            p_attribute->encrypted = encrypted;
            p_attribute->anti_replay = anti_replay;
            goto end;
        }
    }

    p_attribute = (mible_linkage_attribute_t *)malloc(sizeof(mible_linkage_attribute_t));
    if (NULL == p_attribute) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "No memory for linkage attribute");
        goto end;
    }

    memcpy(p_attribute->address, address, sizeof(mible_addr_t));
    p_attribute->product_id = product_id;
    p_attribute->encrypted = encrypted;
    p_attribute->anti_replay = anti_replay;
    list_add_tail(&p_attribute->node, &linkage_hdr);

end:
    arch_os_mutex_put(linkage_table.mutex);
}

void mible_linkage_object_timestamp_update(mible_addr_t address, uint16_t product_id,
                                            uint16_t object_id, uint32_t timestamp)
{
    if (0 == linkage_table.cur_num) {
        return;
    }

    arch_os_mutex_get(linkage_table.mutex, ARCH_OS_WAIT_FOREVER);

    for (int index = 0; index < MIBLE_LINKAGE_MAX_NUM; ++index) {
        if (0 == memcmp(linkage_table.linkage[index].address, address, sizeof(mible_addr_t)) &&
            product_id == linkage_table.linkage[index].product_id) {

            for (int i = 0; i < MIBLE_LINKAGE_OBJECT_MAX_NUM; i++) {
                if (object_id == linkage_table.linkage[index].object[i].object_id) {
                    linkage_table.linkage[index].object[i].timestamp = timestamp;
                    break;
                }
            }
            break;
        }
    }

    arch_os_mutex_put(linkage_table.mutex);
}

