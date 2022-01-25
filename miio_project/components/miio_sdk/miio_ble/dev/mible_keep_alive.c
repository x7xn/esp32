#include "mible_keep_alive.h"
#include "adv_beacon.h"
#include "list/list.h"
#include "mible_dev.h"
#include "mible_rpc.h"
#include "object_rule.h"
#include "mible_def.h"
#include "util.h"

#undef  MIBLE_LOG_TAG
#define MIBLE_LOG_TAG                             "mible_keepalive"
#undef  LOG_LEVEL
#define LOG_LEVEL                                 LOG_LEVEL_INFO

extern arch_os_mutex_handle_t mibeacon_list_mutex;

typedef struct {
	keep_alive_dev_t dev;
	struct list_head list;
} keep_alive_dev_list_t;

static keep_alive_dev_list_t keep_alive_dev_list;
static uint8_t keep_alive_dev_list_count;

static keep_alive_config_t keep_alive_config;

int keep_alive_dev_init(void)
{
    int ret = MIIO_OK;

    INIT_LIST_HEAD(&keep_alive_dev_list.list);
    keep_alive_dev_list_count = 0;

    keep_alive_config.interval = KEEP_ALIVE_DEFAULT_INTERVAL;
    keep_alive_config.delta = KEEP_ALIVE_DEFAULT_DELTA;
    keep_alive_config.ttl = KEEP_ALIVE_DEFAULT_INTERVAL;

    return ret;
}

int keep_alive_set_config(uint32_t interval, uint32_t delta)
{
    int ret = MIIO_OK;

    keep_alive_config.interval = interval;
    keep_alive_config.delta = delta;
    /* keep ttl if ttl is smaller than interval */
    if (keep_alive_config.ttl > interval)
        keep_alive_config.ttl = interval;

    return ret;
}

int keep_alive_dev_add(keep_alive_dev_t *p_dev)
{
    int ret = MIIO_OK;
    keep_alive_dev_list_t *tmp;
    struct list_head *pos, *q;
    bool is_new = false;

    if (NULL == p_dev) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "keep_alive_dev_add invalid param");
        return MIIO_ERROR_PARAM;
    }

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);

	list_for_each_safe(pos, q, &keep_alive_dev_list.list) {
		tmp = list_entry(pos, keep_alive_dev_list_t, list);
        if (tmp->dev.pid == p_dev->pid && 0 == memcmp(tmp->dev.mac, p_dev->mac, 6)) {
            memcpy((uint8_t *)&tmp->dev, (uint8_t *)p_dev, sizeof(keep_alive_dev_t));
            goto end;
        }
	}

    if (keep_alive_dev_list_count >= MAX_KEEP_ALIVE_DEV_LIST_COUNT) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "keep alive dev table is full");
        ret = MIIO_ERROR_FULL;
        goto end;
    }

    tmp = (keep_alive_dev_list_t *)malloc(sizeof(keep_alive_dev_list_t));
    if (tmp == NULL) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for keep_alive_dev_list");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memcpy((uint8_t *)&tmp->dev, (uint8_t *)p_dev, sizeof(keep_alive_dev_t));
    list_add_tail(&(tmp->list), &(keep_alive_dev_list.list));
    keep_alive_dev_list_count ++;
    is_new = true;

end:
    arch_os_mutex_put(mibeacon_list_mutex);

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "keep_alive_dev_add count = %d", keep_alive_dev_list_count);

    if (is_new) {
        /* This new device is added to list, then upload to server right now */
        ret = mible_rpc_single_alive(p_dev);
    }

    return ret;
}

int keep_alive_dev_delete(uint8_t *mac, uint16_t pid)
{
    int ret = MIIO_OK;
    keep_alive_dev_list_t *tmp;
    struct list_head *pos, *q;

    if (NULL == mac) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "keep_alive_dev_delete invalid param");
        return MIIO_ERROR_PARAM;
    }

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &keep_alive_dev_list.list) {
		tmp = list_entry(pos, keep_alive_dev_list_t, list);
        if (tmp->dev.pid == pid && 0 == memcmp(tmp->dev.mac, mac, 6)) {
            list_del(pos);
            free(tmp);
            keep_alive_dev_list_count --;
            goto end;
        }
	}

end:
    arch_os_mutex_put(mibeacon_list_mutex);

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "keep_alive_dev_delete count = %d", keep_alive_dev_list_count);

    return ret;
}

keep_alive_dev_t *keep_alive_dev_search(uint8_t *mac, uint16_t pid)
{
    keep_alive_dev_list_t *tmp, *result = NULL;
    struct list_head *pos, *q;

    if (NULL == mac) {
        return NULL;
    }

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &keep_alive_dev_list.list) {
		tmp = list_entry(pos, keep_alive_dev_list_t, list);
        if (tmp->dev.pid == pid && 0 == memcmp(tmp->dev.mac, mac, 6)) {
            result = tmp;
            goto end;
        }
	}

end:
    arch_os_mutex_put(mibeacon_list_mutex);

    if (result != NULL)
        return &result->dev;
    return NULL;
}

int keep_alive_dev_print(void)
{
    int ret = MIIO_OK;
    keep_alive_dev_list_t *tmp;
    struct list_head *pos, *q;
    int index = 0;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &keep_alive_dev_list.list) {
        tmp = list_entry(pos, keep_alive_dev_list_t, list);
        UNUSED(tmp->dev.pid);
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Found keep alive dev index = %d, pid = %d", index,
                      tmp->dev.pid);
        index++;
    }
    arch_os_mutex_put(mibeacon_list_mutex);

    return ret;
}

uint8_t keep_alive_dev_get_count(void)
{
    return keep_alive_dev_list_count;
}

int keep_alive_dev_get_string(jsmi_composer_t *composer, uint8_t start, uint8_t end)
{
    keep_alive_dev_list_t *tmp;
    struct list_head *pos, *q;
    uint8_t index = 0;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &keep_alive_dev_list.list) {
        if (index >= start && index < end) {
            tmp = list_entry(pos, keep_alive_dev_list_t, list);

            jsmi_set_value_object_begin(composer);
                jsmi_set_key_value_str(composer, "did", (char *)tmp->dev.did, strlen((char *)tmp->dev.did));
                jsmi_set_key_value_sint(composer, "rssi", tmp->dev.rssi);
            jsmi_set_value_object_end(composer);
        }
        index ++;
    }
    arch_os_mutex_put(mibeacon_list_mutex);

    return MIIO_OK;
}

int keep_alive_decrease_ttl()
{
    int ret = MIIO_OK;
    keep_alive_dev_list_t *tmp;
    struct list_head *pos, *q;
    uint32_t cur_time = arch_os_time_now();

    keep_alive_config.ttl --;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &keep_alive_dev_list.list) {
		tmp = list_entry(pos, keep_alive_dev_list_t, list);
        // if timestamp is too old, delete it
        if (cur_time > tmp->dev.timestamp + keep_alive_config.delta) {
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Too old, Found keep alive dev and delete pid = %d",
                          tmp->dev.pid);
            list_del(pos);
            free(tmp);
            keep_alive_dev_list_count--;
        }
	}
    arch_os_mutex_put(mibeacon_list_mutex);

    if (keep_alive_config.ttl == 0) {
        keep_alive_config.ttl = keep_alive_config.interval;

        // get all keep alive dev and send ble_keep_alive message to server
        keep_alive_dev_print();
        mible_rpc_keep_alive();
    }

    return ret;
}
