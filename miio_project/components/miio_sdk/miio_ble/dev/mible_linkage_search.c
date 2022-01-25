/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include "mible_rpc.h"
#include "jsmi.h"
#include "util.h"
#include "arch_def.h"
#include "arch_psm.h"
#include "mible_def.h"
#include "mible_net.h"
#include "mible_gateway.h"
#include "mible_linkage_search.h"
#include "list/list.h"

#undef  MIBLE_LOG_TAG
#define MIBLE_LOG_TAG                                     "mible_linkage_search"
#undef  LOG_LEVEL
#define LOG_LEVEL                                         LOG_LEVEL_INFO

/* Private typedef -----------------------------------------------------------*/

typedef struct {
    list_head_t node;
    search_device_t device;
    bool found;
    uint32_t timestamp;
} mible_linkage_search_t;

/* Private variables ---------------------------------------------------------*/

static list_head_t search_hdr;
static uint16_t search_target_num, search_found_num;
static bool search_enable;
static arch_os_mutex_handle_t search_mutex;
static arch_os_timer_handle_t search_timer_handle;

/* Private function prototypes -----------------------------------------------*/

static int linkage_search_stop(void);
static void linkage_search_reset(void);
static int linkage_search_target_device_add(search_device_t *search_device);
static int linkage_search_start(uint32_t timeout);
static void linkage_search_result_get(search_target_array_t *search_result);
static void linkage_search_result_notify(void *timestamp);
static void linkage_search_timeout_callback(arch_os_timer_handle_t handle);

/* Exported functions --------------------------------------------------------*/

int mible_linkage_search_init(void)
{
    INIT_LIST_HEAD(&search_hdr);
    search_target_num = 0;
    search_found_num = 0;
    search_enable = false;

    return arch_os_mutex_create(&search_mutex);
}

int mible_linkage_search_start(search_target_array_t *target_array, uint32_t timeout)
{
    int ret = MIIO_ERROR;

    if (NULL == target_array) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "linkage search invalid param");
        return MIIO_ERROR_PARAM;
    }

    arch_os_mutex_get(search_mutex, ARCH_OS_WAIT_FOREVER);

    linkage_search_stop();
    linkage_search_reset();

    for (int i = 0; i < target_array->cur_num; ++i) {
        ret = linkage_search_target_device_add(&target_array->target[i]);
        if (MIIO_OK != ret) {
            goto end;
        }
    }

    ret = linkage_search_start(timeout);

end:
    arch_os_mutex_put(search_mutex);

    return ret;
}

int mible_linkage_search_result_update(mibeacon_info_t *p_info)
{
    int ret = MIIO_ERROR_NOTREADY;
    mible_linkage_search_t *p_search;

    if (NULL == p_info) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "linkage search result invalid param");
        return MIIO_ERROR_PARAM;
    }

    arch_os_mutex_get(search_mutex, ARCH_OS_WAIT_FOREVER);

    if (!search_enable) {
        goto end;
    }

    if(0 == search_target_num){
	    goto end;
    }
    
    list_for_each_entry(p_search, &search_hdr, node, mible_linkage_search_t) {
        if ((0 == memcmp(p_search->device.address, p_info->mac, sizeof(mible_addr_t))) &&
            (p_search->device.product_id == p_info->product_id)) {
            p_search->device.rssi = p_info->rssi;
            p_search->timestamp = arch_os_time_now();
            if (!p_search->found) {
                p_search->found = true;
                search_found_num++;
            }
            break;
        }
    }

    if (search_found_num == search_target_num) {
        search_target_array_t search_result;

        memset(&search_result, 0xFF, sizeof(search_result));
        search_result.cur_num = 0;

        linkage_search_stop();
        linkage_search_result_get(&search_result);
        linkage_search_reset();
        mible_rpc_search_result_evt(&search_result);
    }
    
    ret = MIIO_OK;

end:
    arch_os_mutex_put(search_mutex);

    return ret;
}

static int linkage_search_stop(void)
{
    int ret = MIIO_ERROR;

    search_enable = false;

    if(search_timer_handle == NULL){
        ret = arch_os_timer_create(&search_timer_handle, "Linkage search",
                               ARCH_OS_WAIT_FOREVER,
                               linkage_search_timeout_callback,
                               NULL,
                               ARCH_OS_TIMER_ONE_SHOT,
                               ARCH_OS_TIMER_AUTO_ACTIVATE);
        if(ret < 0){
            LOG_WARN_TAG(MIBLE_LOG_TAG,"linkage search timer init error");
        }else{
            LOG_INFO_TAG(MIBLE_LOG_TAG,"set linkage search timer:%p", search_timer_handle);
        }
    }else{
        if((ret = arch_os_timer_change(search_timer_handle, ARCH_OS_WAIT_FOREVER)) < 0){
            LOG_WARN_TAG(MIBLE_LOG_TAG, "modify linkage search timer error ret %d", ret);
        }else{
            LOG_INFO_TAG(MIBLE_LOG_TAG,"stop linkage search timer: %p", search_timer_handle);
        }
    }

    return ret;
}

static void linkage_search_reset(void)
{
    mible_linkage_search_t *p_search, *p_tmp;

    list_for_each_entry_safe(p_search, p_tmp, &search_hdr, node, mible_linkage_search_t) {
        list_del(&p_search->node);
        free(p_search);
    }
    search_target_num = 0;
    search_found_num = 0;
}

static int linkage_search_target_device_add(search_device_t *target)
{
    int ret = MIIO_ERROR_FULL;
    mible_linkage_search_t *p_search;

    if (MIBLE_LINKAGE_SEARCH_MAX_NUM <= search_target_num) {
        return ret;
    }

    list_for_each_entry(p_search, &search_hdr, node, mible_linkage_search_t) {
        if ((0 == memcmp(p_search->device.address, target->address, sizeof(mible_addr_t)))
            && (p_search->device.product_id == target->product_id)) {
            ret = MIIO_OK;
            goto end;
        }
    }

    p_search = (mible_linkage_search_t *)malloc(sizeof(mible_linkage_search_t));
    if (NULL == p_search) {
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }

    memset(p_search, 0, sizeof(mible_linkage_search_t));
    memcpy(&p_search->device, target, sizeof(search_device_t));
    p_search->found = false;
    p_search->timestamp = arch_os_time_now();
    list_add_tail(&p_search->node, &search_hdr);
    search_target_num++;
    ret = MIIO_OK;

end:
    return ret;
}

static int linkage_search_start(uint32_t timeout)
{
    int ret = MIIO_ERROR;

    if (search_target_num == 0) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "no search target, cannot start search");
        return MIIO_ERROR_EMPTY;
    }

    if(search_timer_handle == NULL){
        ret = arch_os_timer_create(&search_timer_handle, "Linkage search",
                               timeout * 1000,
                               linkage_search_timeout_callback,
                               NULL,
                               ARCH_OS_TIMER_ONE_SHOT,
                               ARCH_OS_TIMER_AUTO_ACTIVATE);
        if(ret < 0){
            LOG_WARN_TAG(MIBLE_LOG_TAG, "linkage search timer init error");
        }else{
            LOG_INFO_TAG(MIBLE_LOG_TAG, "set linkage search timer:%p", search_timer_handle);
        }
    }else{
        if((ret = arch_os_timer_change(search_timer_handle, timeout * 1000)) < 0){
            LOG_WARN_TAG(MIBLE_LOG_TAG, "modify linkage search timer error ret %d", ret);
        }else{
            LOG_INFO_TAG(MIBLE_LOG_TAG, "start linkage search timer: %p", search_timer_handle);
        }
    }

    if (MIIO_OK == ret) {
        search_enable = true;
    }

    return ret;
}

static void linkage_search_result_get(search_target_array_t *search_result)
{
    mible_linkage_search_t *p_search;

    if (list_empty(&search_hdr)) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "search result is empty");
        return;
    }

    list_for_each_entry(p_search, &search_hdr, node, mible_linkage_search_t) {
        memcpy(&search_result->target[search_result->cur_num], &p_search->device, sizeof(search_device_t));
        search_result->cur_num++;
    }
}

static void linkage_search_result_notify(void *search_result)
{
    search_target_array_t *result;

    if (NULL == search_result) {
        return;
    }

    result = search_result;
    mible_rpc_search_result_evt(result);

    if (NULL != result) {
        free(result);
        result = NULL;
    }
}

static void linkage_search_timeout_callback(arch_os_timer_handle_t handle)
{
    mible_event_t event;
    search_target_array_t *search_result = (search_target_array_t *)malloc(sizeof(search_target_array_t));

    if (NULL == search_result) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for search result buffer");;
        return;
    }

    memset(search_result, 0xFF, sizeof(search_target_array_t));
    search_result->cur_num = 0;

    arch_os_mutex_get(search_mutex, ARCH_OS_WAIT_FOREVER);

    linkage_search_stop();
    linkage_search_result_get(search_result);
    linkage_search_reset();

    arch_os_mutex_put(search_mutex);

    event.task.func = linkage_search_result_notify;
    event.task.arg = search_result;
    if (MIIO_OK != mible_event_send(MIBLE_EVT_TASK_POST, &event)) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "linkage_search_timeout_callback cannot send event");
    }
}


