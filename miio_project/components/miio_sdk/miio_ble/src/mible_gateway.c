/* Includes ------------------------------------------------------------------*/

#include "util.h"
#include "mible_gateway.h"
#include "arch_api.h"
#include "arch_os.h"
#include "arch_psm.h"
#include "adv_beacon.h"
#include "mible_dev.h"
#include "mible_slave.h"
#include "mible_net.h"
#include "mible_cmd.h"
#include "mible_prph.h"
#include "mible_fastpair.h"
#include "mible_band.h"
#include "mible_hooks.h"
#include "mible_rpc.h"
#include "mible_linkage.h"
#include "mible_wifi_access_config.h"

#undef  MIBLE_LOG_TAG
#define MIBLE_LOG_TAG                              "mible_gateway"
#undef  LOG_LEVEL
#define LOG_LEVEL                                  LOG_LEVEL_INFO

/* Private define ------------------------------------------------------------*/

#define GATEWAY_EVENT_NUM                          20
#define GATEWAY_PENDING_TIMEOUT                    30
#define GATEWAY_UPDATE_TIMEOUT                     3600

/* Private typedef -----------------------------------------------------------*/

typedef struct {
    uint16_t type;
    mible_event_t event;
} gateway_event_t;

/* Private variables ---------------------------------------------------------*/

static arch_os_thread_handle_t thread_handle;
static arch_os_queue_handle_t queue_handle = NULL;
static uint8_t gateway_isstart = 0;

static uint8_t qr_code[16] = {
0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xa0,0xaa,0xab,0xac,0xad,0xae,0xaf,
};

/* Exported variables --------------------------------------------------------*/

mible_config_t gateway_config;
mible_gateway_t gateway_state;

/* Private function prototypes -----------------------------------------------*/

static void* gateway_evt_handler(void *arg);
static void gateway_beacon_update(void * arg);

/* Exported functions --------------------------------------------------------*/

void mi_schd_event_handler(schd_evt_t *p_event)
{
    LOG_INFO_TAG(MIBLE_LOG_TAG, "USER CUSTOM CALLBACK RECV EVT ID %d\n", p_event->id);
    switch (p_event->id) {
    case SCHD_EVT_OOB_REQUEST:
        LOG_INFO_TAG(MIBLE_LOG_TAG, "App selected IO cap is 0x%04X\n", p_event->data.IO_capability);
        switch (p_event->data.IO_capability) {
        case 0x0080:
            mi_schd_oob_rsp(qr_code, 16);
            LOG_INFO_TAG(MIBLE_LOG_TAG, "Please scan QR code label.\n");
            break;

        default:
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "Selected IO cap is not supported.\n");
            //mible_gap_disconnect(0);
        }
        break;

    case SCHD_EVT_KEY_DEL_SUCC:
        // device has been reset, restart adv mibeacon contains IO cap.
        //advertising_init(0);
        break;

    default:
        break;
    }
}


int mible_gateway_init(mible_config_t * p_config)
{
    int ret;
    int8_t rssi;
    mible_param_t parameter;
    uint16_t version;
    arch_info_t gateway_info;

    if (NULL == p_config || p_config->version >= 10000) {
        return MIIO_ERROR_PARAM;
    }
    gateway_config = *p_config;

    ret = arch_info_restore(&gateway_info);
    if (MIIO_OK == ret) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Found valid product ID %d", gateway_info.product_id);
        gateway_config.product_id = gateway_info.product_id;
    }

    ret = arch_psm_get_value("ble", "version", &version, sizeof(version));
    if (sizeof(version) == ret) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Found valid version %04d", version);
        gateway_config.version = version;
    }

    ret = arch_psm_get_value("ble", "rssi", &rssi, sizeof(rssi));
    if (sizeof(rssi) == ret) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Found valid rssi %d", rssi);
        gateway_state.proximity_rssi = rssi;
    } else {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Use default rssi");
        gateway_state.proximity_rssi = MIBEACON_PROXIMITY_RSSI_DEFAULT;
    }

    ret = arch_psm_get_value("ble", "param", &parameter, sizeof(parameter));
    if (sizeof(parameter) == ret) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Found valid parameter: user_state %d", parameter.user_state);
        gateway_state.user_state = parameter.user_state;
    } else {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Use default config");
        gateway_state.user_state = MIBLE_GATEWAY_STATE_UNKNOWN;
    }
    gateway_state.cur_state = MIBLE_GATEWAY_STATE_UNKNOWN;
    gateway_state.last_timestamp = arch_os_time_now();
    gateway_state.retry_times = 0;
    gateway_state.back_off = 0;

    ret = mible_hooks_init();
    if (MIIO_OK != ret) {
        return ret;
    }

    ret = mible_dev_rule_init();
    if (MIIO_OK != ret) {
        return ret;
    }

    ret = mible_prph_init();
    if (MIIO_OK != ret) {
        return ret;
    }

    ret = mibeacon_timer_init();
    if (MIIO_OK != ret) {
        return ret;
    }
#if MIIO_COMMANDS_ENABLE
    ret = mible_cmd_init();
    if (MIIO_OK != ret) {
        return ret;
    }
#endif
    ret = mible_fastpair_init();
    if (MIIO_OK != ret) {
        return ret;
    }

    ret = mible_linkage_init();
    if (MIIO_OK != ret) {
        return ret;
    }

    ret = mible_net_init();
    if (MIIO_OK != ret) {
        return ret;
    }

    ret = arch_os_queue_create(&queue_handle, GATEWAY_EVENT_NUM,
                               sizeof(gateway_event_t));
    if (MIIO_OK != ret) {
        return ret;
    }

    ret = arch_stack_enable();
    if (MIIO_OK != ret) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Init BLE stack failed (%d)", ret);
        return ret;
    }

    mi_scheduler_init(10, mi_schd_event_handler, NULL);
    //mi_scheduler_start(SYS_KEY_RESTORE);

    return MIIO_OK;
}

uint8_t mible_gateway_get_state(void)
{
    return gateway_isstart;
}

int mible_gateway_start(void)
{
    int ret;

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "The stack is enabled");
    if(thread_handle != NULL){
        LOG_WARN_TAG(MIBLE_LOG_TAG, "ble_gateway task already exist");
        return MIIO_OK;
    }

    //set gateway flag
    gateway_isstart = 1;

    
#if MIBLE_GATEWAY_DISABLE       //only remote control
    ret = arch_os_thread_create(&thread_handle, "ble_gateway",
                                gateway_evt_handler,
                                2048, NULL, ARCH_OS_PRIORITY_DEFAULT);
#else
    ret = arch_os_thread_create(&thread_handle, "ble_gateway",
                                gateway_evt_handler,
                                4096, NULL, ARCH_OS_PRIORITY_DEFAULT);
#endif
    if (MIIO_OK != ret) {
        return ret;
    }

    return MIIO_OK;
}

int mible_gateway_stop(void)
{
    int ret = MIIO_OK;

    if(0 == gateway_isstart){
        LOG_WARN_TAG(MIBLE_LOG_TAG, "BLE stack is stoped!!");
        return MIIO_OK;
    }

    mible_gap_deinit();
    mible_slave_deinit();
    mible_gattc_init();
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "The profile is deinited");

    ret = arch_stack_disable();
    if (MIIO_OK != ret) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Deinit BLE stack failed (%d)", ret);
        return ret;
    }
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "The stack is disabled");

    ret = mibeacon_timer_deinit();
    if (MIIO_OK != ret) {
        return ret;
    }
    
    ret = arch_os_thread_delete(thread_handle);
    if (MIIO_OK != ret) {
        return ret;
    }
    thread_handle = NULL;
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "ble_gateway task delete succ!!");

    gateway_isstart = 0;
    return MIIO_OK;
}

int mible_gateway_restart(void)
{
    int ret = MIIO_OK;

    if(gateway_isstart){
        LOG_WARN_TAG(MIBLE_LOG_TAG, "BLE stack is started!!");
        return MIIO_OK;
    }
    
    ret = mibeacon_timer_init();
    if (MIIO_OK != ret) {
        return ret;
    }
    
    ret = arch_stack_enable();
    if (MIIO_OK != ret) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Init BLE stack failed (%d)", ret);
        return ret;
    }
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "The stack is restarted");

    gateway_isstart = 1;
    return MIIO_OK;
}


int mible_gateway_update(mible_config_t * p_config)
{
    mible_event_t event;

    if (NULL == p_config) {
        return MIIO_ERROR_PARAM;
    }

    if (0 == memcmp(p_config, &gateway_config, sizeof(mible_config_t))) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Same values, no need to update");
        return MIIO_OK;
    }

    gateway_config = *p_config;
    event.task.func = gateway_beacon_update;
    event.task.arg = NULL;
    if (MIIO_OK != mible_event_send(MIBLE_EVT_TASK_POST, &event)) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Can not send beacon update event");
        return MIIO_ERROR_NOTREADY;
    }

    return MIIO_OK;
}

int mible_gateway_restore(void)
{
    //stop timer before erasing flash to avoid write flash after erasing
    int ret = mibeacon_timer_deinit();
    if (MIIO_OK != ret) {
        return ret;
    }

    arch_psm_erase_key("ble", "config");
    arch_psm_erase_key("ble", "param");
    arch_psm_erase_key("ble", "event_rule");
    arch_psm_erase_key("ble", "linkage_rule");

    return MIIO_OK;
}

uint32_t mible_gateway_dump(char *buf, uint32_t buf_size)
{
    uint32_t n = 0;

    n += snprintf_safe(buf + n, buf_size - n, "[");
    n += snprintf_safe(buf + n, buf_size - n, "\"product id\":%d,",
                       gateway_config.product_id);
    n += snprintf_safe(buf + n, buf_size - n, "\"version\":%s_%04u",
                       miio_instance_version(), gateway_config.version);
    n += snprintf_safe(buf + n, buf_size - n, "]");

    return n;
}

bool mible_gateway_enable(void)
{
    if(gateway_state.user_state == MIBLE_GATEWAY_STATE_DISABLE) {
        return false;
    }

    switch (gateway_state.cur_state) {
    case MIBLE_GATEWAY_STATE_ENABLE:
        if ((int32_t)(arch_os_time_now() - gateway_state.last_timestamp - GATEWAY_UPDATE_TIMEOUT) > 0) {
            mible_rpc_gateway_enable();
        }
        return true;
    case MIBLE_GATEWAY_STATE_DISABLE:
        return false;
    case MIBLE_GATEWAY_STATE_UNKNOWN:
        if ((int32_t)(arch_os_time_now() - gateway_state.last_timestamp - gateway_state.back_off) > 0) {
            mible_rpc_gateway_enable();
        }
        return true;
    case MIBLE_GATEWAY_STATE_PENDING:
        if ((int32_t)(arch_os_time_now() - gateway_state.last_timestamp - GATEWAY_PENDING_TIMEOUT) > 0) {
            mible_rpc_gateway_enable();
        }
        return true;
    default:
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Unknown gateway state %d", gateway_state.cur_state);
        return true;
    }
}

int mible_event_send(uint16_t type, mible_event_t * p_evt)
{
    gateway_event_t tmp;

    if (NULL == queue_handle) {
        return MIIO_ERROR_NOTREADY;
    }

    memset(&tmp, 0, sizeof(tmp));
    tmp.type = type;
    if (p_evt) {
        tmp.event = *p_evt;
    }

    if (MIIO_OK != arch_os_queue_send(queue_handle, &tmp, 0)) {
        return MIIO_ERROR_FULL;
    }

    return MIIO_OK;
}

static void* gateway_evt_handler(void *arg)
{
    int ret;
    gateway_event_t tmp;

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Init GAP, GATTS and GATTC");
    mible_gap_init();
    mible_gatts_init();
    mible_gattc_init();

    while (true) {
        ret = arch_os_queue_recv(queue_handle, &tmp, ARCH_OS_WAIT_FOREVER);
        if (MIIO_OK != ret) {
            LOG_WARN_TAG(MIBLE_LOG_TAG, "Can not receive any event");
            continue;
        }

        switch (tmp.type) {
            case MIBLE_EVT_TASK_POST:
                if (tmp.event.task.func) {
                    tmp.event.task.func(tmp.event.task.arg);
                }
                break;
            case MIBLE_EVT_GAP_BEACON:
                mible_beacon_handler(&tmp.event.mible_beacon);
                break;
            case MIBLE_EVT_GAP_BAND:
                mible_band_handler(&tmp.event.mible_band);
                break;
            case MIBLE_EVT_GAP_CONNECT:
                mible_connect_handler(&tmp.event.connect);
                break;
            case MIBLE_EVT_GAP_DISCONNECT:
                mible_disconnect_handler(&tmp.event.disconnect);
                break;
            case MIBLE_EVT_GATTS_WRITE:
                mible_gatts_write_handler(&tmp.event.gatts_write);
                break;
            case MIBLE_EVT_GATTC_FOUND:
                mible_gattc_found_handler(&tmp.event.gattc_found);
                break;
            case MIBLE_EVT_GATTC_WRITE:
                mible_gattc_write_handler(&tmp.event.gattc_write);
                break;
            case MIBLE_EVT_GATTC_NOTIFY:
                mible_gattc_notify_handler(&tmp.event.gattc_notify);
                break;
            case MIBLE_EVT_GATTC_READ:
                mible_gattc_read_handler(&tmp.event.gattc_read);
                break;
            default:
                break;
        }
    }

    return NULL;
}

static void gateway_beacon_update(void * arg)
{
    arch_info_t gateway_info;

    /* Save product ID into flash */
    gateway_info.product_id = gateway_config.product_id;
    arch_info_save(&gateway_info);
    /* Save version into PSM */
    arch_psm_set_value("ble", "version", &gateway_config.version,
                        sizeof(gateway_config.version));

    mible_beacon_start();
}

int gateway_auth_callback(uint16_t conn_handle, int status)
{
    mible_connect_t *p_connect;

    p_connect = mible_find_connection_by_handle(conn_handle);
    if (NULL == p_connect) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Cannot find this device (%04x)", conn_handle);
        return MIIO_ERROR_NOTFOUND;
    }

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Auth with peer node %02X:%02X:%02X:%02X:%02X:%02X %s",
          p_connect->address[5], p_connect->address[4], p_connect->address[3],
          p_connect->address[2], p_connect->address[1], p_connect->address[0],
          (MIIO_OK == status) ? "succ" : "fail");

    if (MIIO_OK != status) {
        arch_gap_disconnect(conn_handle);
    }

    return MIIO_OK;
}

int gateway_enable_handler(jsmi_parser_t *parser, void *parent_key)
{
    int ret;

    if (NULL == parser || NULL == parent_key) {
        return MIIO_ERROR_PARAM;
    }

    uint8_t state;
    jsmi_tok_path_t state_path[] = {
        {
            .key = parent_key,
            .type = JSMN_OBJECT
        },
        {
            .key = "state",
            .type = JSMN_PRIMITIVE
        }
    };
    ret = jsmi_get_value_uint8(parser, NULL, state_path, NELEMENTS(state_path), &state);
    if (MIIO_OK != ret) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "parse state fail (err %d)", ret);
        return ret;
    }
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "state value %u", state);
    gateway_state.cur_state = state;

    int32_t rssi;
    jsmi_tok_path_t rssi_path[] = {
        {
            .key = parent_key,
            .type = JSMN_OBJECT
        },
        {
            .key = "rssi",
            .type = JSMN_PRIMITIVE
        }
    };
    ret = jsmi_get_value_sint32(parser, NULL, rssi_path, NELEMENTS(rssi_path), &rssi);
    if (MIIO_OK == ret) {
        if (rssi < INT8_MIN || rssi > INT8_MAX) {
            LOG_WARN_TAG(MIBLE_LOG_TAG, "RSSI %d is invalid", rssi);
            return MIIO_ERROR_PARAM;
        }
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "rssi value %d", rssi);
        //update proximity_rssi
        if (gateway_state.proximity_rssi != rssi) {
            gateway_state.proximity_rssi = rssi;
            arch_psm_set_value("ble", "rssi", &gateway_state.proximity_rssi, sizeof(gateway_state.proximity_rssi));
            mible_beacon_start();
        } else {
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "same RSSI, so ignore it");
        }
    } else {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "parse RSSI fail (err %d)", ret);
    }
    
    mible_rpc_mesh_switch((MIBLE_GATEWAY_STATE_UNKNOWN == gateway_state.user_state)?
                           gateway_state.cur_state : gateway_state.user_state);

    return MIIO_OK;
}


