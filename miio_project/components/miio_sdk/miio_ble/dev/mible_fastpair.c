/* Includes ------------------------------------------------------------------*/

#include "mible_fastpair.h"
#include "mible_prph.h"
#include "mible_cmd.h"
#include "mible_hooks.h"
#include "arch_api.h"
#include "arch_os.h"
#include "list/list.h"

#undef  MIBLE_LOG_TAG
#define MIBLE_LOG_TAG                                 "mible_fastpair"
#undef  LOG_LEVEL
#define LOG_LEVEL                                     LOG_LEVEL_INFO

/* Private define ------------------------------------------------------------*/

#define MIBLE_FASTPAIR_TIMEOUT                        10000
#define MIBLE_FASTPAIR_DATABASE_MAX                   10
#define MIBLE_FASTPAIR_DATABASE_REFRESH               10

/* Private typedef -----------------------------------------------------------*/

typedef struct {
    mible_addr_t address;
    uint16_t product_id;
    uint16_t object_id;
    arch_os_timer_handle_t timer_handle;
} mible_fastpair_t;

typedef struct {
    list_head_t node;
    mible_addr_t address;
    uint16_t product_id;
    uint16_t object_id;
    bool permit;
    uint32_t timestamp;
} mible_fastpair_db_t;

/* Private variables ---------------------------------------------------------*/

static mible_fastpair_t *session = NULL;
static list_head_t fastpair_hdr;
static arch_os_mutex_handle_t fastpair_mutex;
static uint16_t current_num;

/* Private variables ---------------------------------------------------------*/

static void fastpair_timeout_callback(arch_os_timer_handle_t handle);
static void fastpair_restore(void * arg);
static bool fastpair_db_check(mible_addr_t address, uint16_t product_id,
                                      uint16_t object_id, int8_t rssi);

/* Exported functions --------------------------------------------------------*/

int mible_fastpair_init(void)
{
    current_num = 0;
    INIT_LIST_HEAD(&fastpair_hdr);

    return arch_os_mutex_create(&fastpair_mutex);
}

int mible_fastpair_start(mible_addr_t address, uint16_t product_id,
                            uint16_t object_id, int8_t rssi)
{
    int ret;

    if (NULL != session) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "BLE fast pair is ongoing.");
        return MIIO_ERROR_BUSY;
    }

    if (!fastpair_db_check(address, product_id, object_id, rssi)) {
        return MIIO_ERROR;
    }

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Start fast pair with %02x:%02x:%02x:%02x:%02x:%02x",
        address[5], address[4], address[3], address[2], address[1], address[0]);

    session = (mible_fastpair_t *)malloc(sizeof(mible_fastpair_t));
    if (NULL == session) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for mible_fastpair");
        return MIIO_ERROR_NOMEM;
    }

    memcpy(session->address, address, sizeof(mible_addr_t));
    session->product_id = product_id;
    session->object_id = object_id;

    ret = arch_os_timer_create(&session->timer_handle, "fastpair",
                               MIBLE_FASTPAIR_TIMEOUT,
                               fastpair_timeout_callback, NULL,
                               ARCH_OS_TIMER_ONE_SHOT, ARCH_OS_TIMER_AUTO_ACTIVATE);
    if (MIIO_OK != ret) {
        return ret;
    }

    ret = mible_gap_scan(false);
    if (MIIO_OK != ret) {
        return ret;
    }

    ret = arch_gap_connect(session->address);
    if (MIIO_OK != ret) {
        return ret;
    }

    return MIIO_OK;
}

uint16_t mible_fastpair_get_pid(void)
{
    return (NULL == session) ? 0 : session->product_id;
}

int mible_fastpair_complete(uint16_t conn_handle, uint8_t *p_key, bool isSecureFastpair)
{
    mible_connect_t *p_connect;

    if (NULL == session) {
        return MIIO_ERROR_NOTREADY;
    }

    p_connect = mible_find_connection_by_handle(conn_handle);
    if (NULL == p_connect) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Cannot find this device (%04x)", conn_handle);
        return MIIO_ERROR_NOTFOUND;
    }

    if (0 != memcmp(p_connect->address, session->address, sizeof(mible_addr_t)) || 
        MIBLE_LOCAL_ROLE_MASTER != p_connect->local_role) {

        LOG_WARN_TAG(MIBLE_LOG_TAG, "This is not the target");
        return MIIO_ERROR_PARAM;
    }

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Succeed in adding %02x:%02x:%02x:%02x:%02x:%02x as peripheral",
              session->address[5], session->address[4], session->address[3],
              session->address[2], session->address[1], session->address[0]);

    if (MIIO_OK == mible_prph_add(session->address, session->product_id,
                                  session->object_id, p_key)) {

        mible_hooks_fastpair_report(p_connect->address, FASTPAIR_STATE_SUCC);
    } else {
        mible_hooks_fastpair_report(p_connect->address, FASTPAIR_STATE_FAIL);
    }

    if(NULL != session->timer_handle)
    {
        arch_os_timer_delete(session->timer_handle);
    }

    free(session);
    session = NULL;

    if(!isSecureFastpair)
    {
        LOG_INFO_TAG(MIBLE_LOG_TAG, "mible_fastpair_complete, going to arch_gap_disconnect");
        arch_gap_disconnect(conn_handle);

        LOG_INFO_TAG(MIBLE_LOG_TAG, "recover mible_gap_scan");
        mible_gap_scan(true);
    }

    return MIIO_OK;
}

int mible_fastpair_update(mible_addr_t address, uint16_t product_id,
                                            uint16_t object_id, bool permit)
{
    mible_fastpair_db_t *fastpair_db;
    bool found = false;

    arch_os_mutex_get(fastpair_mutex, ARCH_OS_WAIT_FOREVER);

    list_for_each_entry(fastpair_db, &fastpair_hdr, node, mible_fastpair_db_t) {
        if (0 == memcmp(fastpair_db->address, address, sizeof(mible_addr_t)) &&
                    product_id == fastpair_db->product_id &&
                    object_id == fastpair_db->object_id) {

            fastpair_db->timestamp = arch_os_time_now();
            fastpair_db->permit = permit;
            found = true;
            break;
        }
    }

    if (!found) {
        fastpair_db = (mible_fastpair_db_t *)malloc(sizeof(mible_fastpair_db_t));
        if (NULL != fastpair_db) {
            memcpy(fastpair_db->address, address, sizeof(mible_addr_t));
            fastpair_db->product_id = product_id;
            fastpair_db->object_id = object_id;
            fastpair_db->permit = permit;
            fastpair_db->timestamp = arch_os_time_now();
            list_add_tail(&fastpair_db->node, &fastpair_hdr);
            ++current_num;
        } else {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for fast pair db");
        }
    }

    while (current_num > MIBLE_FASTPAIR_DATABASE_MAX) {
        fastpair_db = list_first_entry(&fastpair_hdr, mible_fastpair_db_t, node);
        list_del(&fastpair_db->node);
        free(fastpair_db);
        --current_num;
    }

    arch_os_mutex_put(fastpair_mutex);

    return MIIO_OK;
}

int mible_fastpair_refresh(void)
{
    mible_fastpair_db_t *fastpair_db, *tmp;
    uint32_t timestamp = arch_os_time_now();

    arch_os_mutex_get(fastpair_mutex, ARCH_OS_WAIT_FOREVER);

    list_for_each_entry_safe(fastpair_db, tmp, &fastpair_hdr, node, mible_fastpair_db_t) {
        if ((int32_t)(timestamp - fastpair_db->timestamp - MIBLE_FASTPAIR_DATABASE_REFRESH) > 0) {
            list_del(&fastpair_db->node);
            free(fastpair_db);
            --current_num;
        } else {
            break;
        }
    }

    arch_os_mutex_put(fastpair_mutex);

    return MIIO_OK;
}

static void fastpair_timeout_callback(arch_os_timer_handle_t handle)
{
    mible_event_t event;

    event.task.func = fastpair_restore;
    event.task.arg = NULL;
    if (MIIO_OK != mible_event_send(MIBLE_EVT_TASK_POST, &event)) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "fastpair restore cannot send event");
    }
}

static void fastpair_restore(void * arg)
{
    mible_connect_t *p_connect;

    if (NULL == session) {
        return;
    }

    if(NULL != session->timer_handle)
    {
        arch_os_timer_delete(session->timer_handle);
    }

    p_connect = mible_find_connection_by_address(session->address);
    if (NULL != p_connect && MIBLE_LOCAL_ROLE_MASTER == p_connect->local_role) {
        arch_gap_disconnect(p_connect->conn_handle);
        LOG_WARN_TAG(MIBLE_LOG_TAG, "fastpair timeout, disconnect with 0x%04x", p_connect->conn_handle);
    } else {
        arch_gap_cancel_connection();
        LOG_WARN_TAG(MIBLE_LOG_TAG, "fastpair timeout, stop trying");
    }
    mible_hooks_fastpair_report(session->address, FASTPAIR_STATE_TIMEOUT);

    free(session);
    session = NULL;

    mible_gap_scan(true);
}

static bool fastpair_db_check(mible_addr_t address, uint16_t product_id,
                                            uint16_t object_id, int8_t rssi)
{
    mible_fastpair_db_t *fastpair_db;
    bool handle = false, found = false;
    uint32_t timestamp = arch_os_time_now();

    arch_os_mutex_get(fastpair_mutex, ARCH_OS_WAIT_FOREVER);

    list_for_each_entry(fastpair_db, &fastpair_hdr, node, mible_fastpair_db_t) {
        if (0 == memcmp(fastpair_db->address, address, sizeof(mible_addr_t)) &&
                    product_id == fastpair_db->product_id &&
                    object_id == fastpair_db->object_id) {

            found = true;
            handle = fastpair_db->permit;
            break;
        }
    }

    if (!found) {
        fastpair_db = (mible_fastpair_db_t *)malloc(sizeof(mible_fastpair_db_t));
        if (NULL != fastpair_db) {
            memcpy(fastpair_db->address, address, sizeof(mible_addr_t));
            fastpair_db->product_id = product_id;
            fastpair_db->object_id = object_id;
            fastpair_db->permit = false;
            fastpair_db->timestamp = timestamp;
            list_add_tail(&fastpair_db->node, &fastpair_hdr);
            ++current_num;
        } else {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for fast pair db");
        }
    }

    while (current_num > MIBLE_FASTPAIR_DATABASE_MAX) {
        fastpair_db = list_first_entry(&fastpair_hdr, mible_fastpair_db_t, node);
        list_del(&fastpair_db->node);
        free(fastpair_db);
        --current_num;
    }

    arch_os_mutex_put(fastpair_mutex);

    if (!found) {
        /* Cannot find this device and check with app */
        mible_hooks_fastpair_check(address, product_id, object_id, rssi);
    }

    return handle;
}

