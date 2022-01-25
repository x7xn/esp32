#include "mible_dev.h"
#include "adv_beacon.h"
#include "arch_api.h"
#include "list/list.h"
#include "mible_fastpair.h"
#include "mible_keep_alive.h"
#include "mible_prph.h"
#include "mible_rpc.h"
#include "mible_band.h"
#include "mible_net.h"
#include "object_rule.h"
#include "mible_def.h"
#include "mible_linkage.h"
#include "mible_linkage_search.h"

#undef  MIBLE_LOG_TAG
#define MIBLE_LOG_TAG                             "mible_dev"
#undef  LOG_LEVEL
#define LOG_LEVEL                                 LOG_LEVEL_INFO

arch_os_mutex_handle_t mibeacon_list_mutex;
static arch_os_timer_handle_t one_second_timer;
static arch_os_timer_handle_t rpc_upload_timer;

static void one_second_timeout_callback(void *arg);
static void one_second_handler(void *arg);
static void rpc_upload_timeout_callback(void *arg);
static void rpc_upload_handler(void *arg);

typedef struct {
    authed_dev_t dev;
    struct list_head list;
} authed_dev_list_t;

static authed_dev_list_t authed_dev_list;
static uint8_t authed_dev_list_count;

typedef struct {
    unknown_dev_t dev;
    struct list_head list;
} unknown_dev_list_t;

static unknown_dev_list_t unknown_dev_list;
static uint8_t unknown_dev_list_count;

typedef struct {
    denied_dev_t dev;
    struct list_head list;
} denied_dev_list_t;

static denied_dev_list_t denied_dev_list;
static uint8_t denied_dev_list_count;

int authed_dev_init(void)
{
    int ret = MIIO_OK;

    INIT_LIST_HEAD(&authed_dev_list.list);
    authed_dev_list_count = 0;

    return ret;
}

int unknown_dev_init(void)
{
    int ret = MIIO_OK;

    INIT_LIST_HEAD(&unknown_dev_list.list);
    unknown_dev_list_count = 0;

    return ret;
}

int denied_dev_init(void)
{
    int ret = MIIO_OK;

    INIT_LIST_HEAD(&denied_dev_list.list);
    denied_dev_list_count = 0;

    return ret;
}

int authed_dev_add(authed_dev_t *p_dev)
{
    int ret = MIIO_OK;
    authed_dev_list_t *tmp;
    struct list_head *pos, *q;

    if (NULL == p_dev) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "authed_dev_add invalid param");
        return MIIO_ERROR_PARAM;
    }

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);

	list_for_each_safe(pos, q, &authed_dev_list.list) {
		tmp = list_entry(pos, authed_dev_list_t, list);
        if (tmp->dev.pid == p_dev->pid && 0 == memcmp(tmp->dev.mac, p_dev->mac, 6)) {
            /* same device, check if the token, did and key is same */
            if (0 != memcmp(tmp->dev.token, p_dev->token, MIBEACON_TOKEN_LEN) ||
                0 != memcmp(tmp->dev.beaconkey, p_dev->beaconkey, MIBEACON_BEACONKEY_LEN) ||
                0 != strcmp((const char *)tmp->dev.did, (const char *)p_dev->did)) {

                /* The device might have been bonded again, clear info first */
                list_del(pos);
                free(tmp);
                authed_dev_list_count --;
                break;
            } else {
                /* same info about this device, just update its TTL */
                tmp->dev.ttl = p_dev->ttl;
                goto end;
            }
        }
	}

    if (authed_dev_list_count >= MAX_AUTHED_DEV_LIST_COUNT) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "authed dev table is full");
        ret = MIIO_ERROR_FULL;
        goto end;
    }

    tmp = (authed_dev_list_t *)malloc(sizeof(authed_dev_list_t));
    if (tmp == NULL) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for authed_dev_list");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memcpy((uint8_t *)&tmp->dev, (uint8_t *)p_dev, sizeof(authed_dev_t));
    list_add_tail(&(tmp->list), &(authed_dev_list.list));
    authed_dev_list_count ++;

end:
    arch_os_mutex_put(mibeacon_list_mutex);

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "authed_dev_add count = %d", authed_dev_list_count);

    return ret;
}

int unknown_dev_add(unknown_dev_t *p_dev)
{
    int ret = MIIO_OK;
    unknown_dev_list_t *tmp;
    struct list_head *pos, *q;

    if (NULL == p_dev) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "unknown_dev_add invalid param");
        return MIIO_ERROR_PARAM;
    }

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);

	list_for_each_safe(pos, q, &unknown_dev_list.list) {
		tmp = list_entry(pos, unknown_dev_list_t, list);
        if (tmp->dev.pid == p_dev->pid && 0 == memcmp(tmp->dev.mac, p_dev->mac, 6)) {
            memcpy((uint8_t *)&tmp->dev, (uint8_t *)p_dev, sizeof(unknown_dev_t));
            goto end;
        }
	}

    if (unknown_dev_list_count >= MAX_UNKNOWN_DEV_LIST_COUNT) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "unknown dev table is full");
        ret = MIIO_ERROR_FULL;
        goto end;
    }

    tmp = (unknown_dev_list_t *)malloc(sizeof(unknown_dev_list_t));
    if (tmp == NULL) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for unknown_dev_list");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memcpy((uint8_t *)&tmp->dev, (uint8_t *)p_dev, sizeof(unknown_dev_t));
    list_add_tail(&(tmp->list), &(unknown_dev_list.list));
    unknown_dev_list_count ++;

end:
    arch_os_mutex_put(mibeacon_list_mutex);

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "unknown_dev_add count = %d", unknown_dev_list_count);

    return ret;
}

int denied_dev_add(denied_dev_t *p_dev)
{
    int ret = MIIO_OK;
    denied_dev_list_t *tmp;
    struct list_head *pos, *q;

    if (NULL == p_dev) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "denied_dev_add invalid param");
        return MIIO_ERROR_PARAM;
    }

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);

	list_for_each_safe(pos, q, &denied_dev_list.list) {
		tmp = list_entry(pos, denied_dev_list_t, list);
        if (tmp->dev.pid == p_dev->pid && 0 == memcmp(tmp->dev.mac, p_dev->mac, 6)) {
            memcpy((uint8_t *)&tmp->dev, (uint8_t *)p_dev, sizeof(denied_dev_t));
            goto end;
        }
	}

    if (denied_dev_list_count >= MAX_DENIED_DEV_LIST_COUNT) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "denied dev table is full");
        ret = MIIO_ERROR_FULL;
        goto end;
    }

    tmp = (denied_dev_list_t *)malloc(sizeof(denied_dev_list_t));
    if (tmp == NULL) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for denied_dev_list");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memcpy((uint8_t *)&tmp->dev, (uint8_t *)p_dev, sizeof(denied_dev_t));
    list_add_tail(&(tmp->list), &(denied_dev_list.list));
    denied_dev_list_count ++;

end:
    arch_os_mutex_put(mibeacon_list_mutex);

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "denied_dev_add count = %d", denied_dev_list_count);

    return ret;
}

int authed_dev_delete(uint8_t *mac, uint16_t pid)
{
    int ret = MIIO_OK;
    authed_dev_list_t *tmp;
    struct list_head *pos, *q;

    if (NULL == mac) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "authed_dev_delete invalid param");
        return MIIO_ERROR_PARAM;
    }

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &authed_dev_list.list) {
		tmp = list_entry(pos, authed_dev_list_t, list);
        if (tmp->dev.pid == pid && 0 == memcmp(tmp->dev.mac, mac, 6)) {
            list_del(pos);
            free(tmp);
            authed_dev_list_count --;
            goto end;
        }
	}

end:
    arch_os_mutex_put(mibeacon_list_mutex);

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "authed_dev_delete count = %d", authed_dev_list_count);

    return ret;
}

int unknown_dev_delete(uint8_t *mac, uint16_t pid)
{
    int ret = MIIO_OK;
    unknown_dev_list_t *tmp;
    struct list_head *pos, *q;

    if (NULL == mac) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "unknown_dev_delete invalid param");
        return MIIO_ERROR_PARAM;
    }

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &unknown_dev_list.list) {
		tmp = list_entry(pos, unknown_dev_list_t, list);
        if (tmp->dev.pid == pid && 0 == memcmp(tmp->dev.mac, mac, 6)) {
            list_del(pos);
            free(tmp);
            unknown_dev_list_count --;
            goto end;
        }
	}

end:
    arch_os_mutex_put(mibeacon_list_mutex);

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "unknown_dev_delete count = %d", unknown_dev_list_count);

    return ret;
}

int denied_dev_delete(uint8_t *mac, uint16_t pid)
{
    int ret = MIIO_OK;
    denied_dev_list_t *tmp;
    struct list_head *pos, *q;

    if (NULL == mac) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "denied_dev_delete invalid param");
        return MIIO_ERROR_PARAM;
    }

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &denied_dev_list.list) {
		tmp = list_entry(pos, denied_dev_list_t, list);
        if (tmp->dev.pid == pid && 0 == memcmp(tmp->dev.mac, mac, 6)) {
            list_del(pos);
            free(tmp);
            denied_dev_list_count --;
            goto end;
        }
	}

end:
    arch_os_mutex_put(mibeacon_list_mutex);

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "denied_dev_delete count = %d", denied_dev_list_count);

    return ret;
}

authed_dev_t *authed_dev_search(uint8_t *mac, uint16_t pid)
{
    authed_dev_list_t *tmp, *result = NULL;
    struct list_head *pos, *q;

    if (NULL == mac) {
        return NULL;
    }

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &authed_dev_list.list) {
		tmp = list_entry(pos, authed_dev_list_t, list);
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

unknown_dev_t *unknown_dev_search(uint8_t *mac, uint16_t pid)
{
    unknown_dev_list_t *tmp, *result = NULL;
    struct list_head *pos, *q;

    if (NULL == mac) {
        return NULL;
    }

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &unknown_dev_list.list) {
		tmp = list_entry(pos, unknown_dev_list_t, list);
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

denied_dev_t * denied_dev_search(uint8_t *mac, uint16_t pid)
{
    denied_dev_list_t *tmp, *result = NULL;
    struct list_head *pos, *q;

    if (NULL == mac) {
        return NULL;
    }

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &denied_dev_list.list) {
		tmp = list_entry(pos, denied_dev_list_t, list);
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

int authed_dev_print(void)
{
    int ret = MIIO_OK;
    authed_dev_list_t *tmp;
    struct list_head *pos, *q;
    int index = 0;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "authed_dev_list_count = %d", authed_dev_list_count);
	list_for_each_safe(pos, q, &authed_dev_list.list) {
        tmp = list_entry(pos, authed_dev_list_t, list);
        UNUSED(tmp->dev.pid);
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "authed_dev_list: index = %d, pid = %d, report_num = %d",
                      index, tmp->dev.pid, tmp->dev.report_num);
        index++;
    }
    arch_os_mutex_put(mibeacon_list_mutex);

    return ret;
}

int unknown_dev_print(void)
{
    int ret = MIIO_OK;
    unknown_dev_list_t *tmp;
    struct list_head *pos, *q;
    int index = 0;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "unknown_dev_list_count = %d", unknown_dev_list_count);
	list_for_each_safe(pos, q, &unknown_dev_list.list) {
        tmp = list_entry(pos, unknown_dev_list_t, list);
        UNUSED(tmp->dev.pid);
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "unknown_dev_list: index = %d, pid = %d", index,
                      tmp->dev.pid);
        index++;
    }
    arch_os_mutex_put(mibeacon_list_mutex);

    return ret;
}

int denied_dev_print(void)
{
    int ret = MIIO_OK;
    denied_dev_list_t *tmp;
    struct list_head *pos, *q;
    int index = 0;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "denied_dev_list_count = %d", denied_dev_list_count);
	list_for_each_safe(pos, q, &denied_dev_list.list) {
        tmp = list_entry(pos, denied_dev_list_t, list);
        UNUSED(tmp->dev.pid);
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "denied_dev_list: index = %d, pid = %d", index,
                      tmp->dev.pid);
        index++;
    }
    arch_os_mutex_put(mibeacon_list_mutex);

    return ret;
}

int authed_dev_decrease_ttl(void)
{
    int ret = MIIO_OK;
    authed_dev_list_t *tmp = NULL;
    struct list_head *pos, *q;

    bool in_advance_flag = FALSE;
    uint8_t in_advance_mac[6];
    uint16_t in_advance_pid;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &authed_dev_list.list) {
		tmp = list_entry(pos, authed_dev_list_t, list);
        if (tmp->dev.ttl > 0) {
            tmp->dev.ttl--;
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "authed_dev_decrease_ttl pid = %d, ttl = %d", tmp->dev.pid,
                          tmp->dev.ttl);

            // ask for server in advance
            if (tmp->dev.ttl == MIBEACON_IN_ADVANCE_TTL) {
                in_advance_flag = TRUE;
                memcpy(in_advance_mac, tmp->dev.mac, 6);
                in_advance_pid = tmp->dev.pid;
            }
        } else {
            // delete the device from list
            list_del(pos);
            free(tmp);
            authed_dev_list_count--;
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "authed_dev_decrease_ttl delete count = %d",
                          authed_dev_list_count);
        }
    }
    arch_os_mutex_put(mibeacon_list_mutex);

    if (in_advance_flag) {
        mible_rpc_query_dev(in_advance_mac, in_advance_pid);
    }

    return ret;
}

int unknown_dev_decrease_ttl(void)
{
    int ret = MIIO_OK;
    unknown_dev_list_t *tmp;
    struct list_head *pos, *q;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &unknown_dev_list.list) {
		tmp = list_entry(pos, unknown_dev_list_t, list);
        if (tmp->dev.ttl > 0) {
            tmp->dev.ttl --;
        } else {
            list_del(pos);
            free(tmp);
            unknown_dev_list_count --;
        }
	}
    arch_os_mutex_put(mibeacon_list_mutex);

    return ret;
}

int denied_dev_decrease_ttl(void)
{
    int ret = MIIO_OK;
    denied_dev_list_t *tmp;
    struct list_head *pos, *q;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &denied_dev_list.list) {
		tmp = list_entry(pos, denied_dev_list_t, list);
        if (tmp->dev.ttl > 0) {
            tmp->dev.ttl --;
        } else {
            list_del(pos);
            free(tmp);
            denied_dev_list_count --;
        }
    }
    arch_os_mutex_put(mibeacon_list_mutex);

    return ret;
}


int mible_dev_rule_init(void)
{
    int ret = MIIO_OK;

    ret = arch_os_mutex_create(&mibeacon_list_mutex);
    if (MIIO_OK != ret) {
        return ret;
    }

    authed_dev_init();
    unknown_dev_init();
    denied_dev_init();

    keep_alive_dev_init();

    object_rule_init();
    pid_obj_id_init();
    unknown_pid_init();

    rpc_upload_init();
    mible_band_init();
    mible_linkage_search_init();

    return ret;
}

int mibeacon_timer_init(void)
{
    int ret = MIIO_OK;

    ret = arch_os_timer_create(&one_second_timer, "One second",
                               MIBEACON_ONE_SECOND_MS,
                               one_second_timeout_callback,
                               NULL,
                               ARCH_OS_TIMER_PERIODIC,
                               ARCH_OS_TIMER_AUTO_ACTIVATE);
    if (ret != MIIO_OK) {
        return ret;
    }

    ret = arch_os_timer_create(&rpc_upload_timer, "RPC upload",
                               MIBEACON_RPC_UPLOAD_MS,
                               rpc_upload_timeout_callback,
                               NULL,
                               ARCH_OS_TIMER_PERIODIC,
                               ARCH_OS_TIMER_AUTO_ACTIVATE);
    if (ret != MIIO_OK) {
        return ret;
    }

    return ret;
}

int mibeacon_timer_deinit(void)
{
	int ret = MIIO_OK;

    if(NULL != one_second_timer)
    {
        ret = arch_os_timer_delete(one_second_timer);
        if (ret != MIIO_OK) {
            return ret;
        }
        one_second_timer = NULL;
    }

    if(NULL != one_second_timer)
    {
        ret = arch_os_timer_delete(rpc_upload_timer);
        if (ret != MIIO_OK) {
            return ret;
        }
        rpc_upload_timer = NULL;
    }

    return ret;
}

static void one_second_timeout_callback(void *arg)
{
    mible_event_t event;

    event.task.func = one_second_handler;
    event.task.arg = arg;
    if (MIIO_OK != mible_event_send(MIBLE_EVT_TASK_POST, &event)) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "one second cannot send event");
    }
}

static void one_second_handler(void *arg)
{
    authed_dev_decrease_ttl();
    unknown_dev_decrease_ttl();
    denied_dev_decrease_ttl();

    // there is no ttl in rule
    pid_obj_id_decrease_ttl();
    unknown_pid_decrease_ttl();
    keep_alive_decrease_ttl();

    // Data Synchronism
    mible_prph_sync();
    mible_linkage_sync();
    // Refresh pending devices
    mible_fastpair_refresh();
    // Refresh the mi band list
    mible_band_refresh();
}

static void rpc_upload_timeout_callback(void *arg)
{
    mible_event_t event;

    event.task.func = rpc_upload_handler;
    event.task.arg = arg;
    if (MIIO_OK != mible_event_send(MIBLE_EVT_TASK_POST, &event)) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "RPC upload cannot send event");
    }
}

static void rpc_upload_handler(void *arg)
{
    get_first_rpc_and_upload();
}

#define DEV_NOTIFY_NOT_FOUND   0
#define DEV_NOTIFY_OK          1
#define DEV_NOTIFY_UNKNOWN_ERR 2
#define DEV_NOTIFY_DENIED_ERR  3
#define DEV_NOTIFY_AUTHED_ERR  4

int mible_dev_bind_action(uint8_t* addr, uint16_t pdid)
{
    authed_dev_t *p_authed_dev = NULL;
    unknown_dev_t *p_unknown_dev = NULL;
    denied_dev_t *p_denied_dev = NULL;
    keep_alive_dev_t* p_keep_alive_dev = NULL;

    p_keep_alive_dev = keep_alive_dev_search(addr, pdid);
    if(NULL != p_keep_alive_dev)
    {
        if(MIIO_OK != keep_alive_dev_delete(addr, pdid))
        {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "keep_alive_dev_delete err!");
        }
    }

	p_unknown_dev = unknown_dev_search(addr, pdid);
	if(p_unknown_dev != NULL)
	{
        if(MIIO_OK != unknown_dev_delete(addr, pdid))
        {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "unknown_dev_delete err!");
            return DEV_NOTIFY_UNKNOWN_ERR;
        }
        return DEV_NOTIFY_OK;
	}

	p_denied_dev = denied_dev_search(addr, pdid);
	if(p_denied_dev != NULL)
	{
        if(MIIO_OK != denied_dev_delete(addr, pdid))
        {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "denied_dev_delete err!");
            return DEV_NOTIFY_DENIED_ERR;
        }
        return DEV_NOTIFY_OK;
	}

    p_authed_dev = authed_dev_search(addr, pdid);
    if(p_authed_dev != NULL)
    {
        return DEV_NOTIFY_OK;
    }

    return DEV_NOTIFY_NOT_FOUND;
}

int mible_dev_unbind_action(uint8_t* addr, uint16_t pdid)
{
    authed_dev_t *p_authed_dev = NULL;
    keep_alive_dev_t* p_keep_alive_dev = NULL;

    p_keep_alive_dev = keep_alive_dev_search(addr, pdid);
    if(NULL != p_keep_alive_dev)
    {
        if(MIIO_OK != keep_alive_dev_delete(addr, pdid))
        {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "keep_alive_dev_delete err!");
        }
    }

    p_authed_dev = authed_dev_search(addr, pdid);
    if(NULL != p_authed_dev)
    {
        if(MIIO_OK != authed_dev_delete(addr, pdid))
        {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "authed_dev_delete err!");
            return DEV_NOTIFY_AUTHED_ERR;
        }
        return DEV_NOTIFY_OK;
    }

    return DEV_NOTIFY_NOT_FOUND;
}

