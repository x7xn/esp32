/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stddef.h>
#include "mible_gateway.h"
#include "mible_slave.h"
#include "mible_net.h"
#include "mible_band.h"
#include "mible_rpc.h"
#include "arch_dbg.h"
#include "arch_os.h"
#include "arch_api.h"
#include "miio_net.h"
#include "jsmi.h"

/* Private define ------------------------------------------------------------*/

#undef  MIBLE_LOG_TAG
#define MIBLE_LOG_TAG                                        "mible_net"
#undef  LOG_LEVEL
#define LOG_LEVEL                                            LOG_LEVEL_INFO

#define NET_CONFIG_USER_ID                                   0x00
#define NET_CONFIG_WIFI_SSID                                 0x01
#define NET_CONFIG_WIFI_PASSWORD                             0x02
#define NET_CONFIG_CMD_RETRY                                 0x03
#define NET_CONFIG_CMD_RESTORE                               0x04
#define NET_CONFIG_UTC                                       0x05
#define NET_CONFIG_DOMAIN                                    0x06
#define NET_CONFIG_TIME_ZONE                                 0x07
#define NET_CONFIG_TYPE                                      0x08
#define NET_CONFIG_BIND_KEY                                  0x09
#define NET_CONFIG_COUNTRY_CODE                              0x0A

#define NET_CONFIG_BIND_INDEX                                0x0D
#define NET_CONFIG_BIND_TIMESTAMP                            0x0E

#define NET_CONFIG_USER_ID_LEN                               8
#define NET_CONFIG_WIFI_SSID_LEN                             32
#define NET_CONFIG_WIFI_PASSWORD_LEN                         64
#define NET_CONFIG_UTC_LEN                                   4
#define NET_CONFIG_DOMAIN_LEN                                3
#define NET_CONFIG_TIME_ZONE_LEN                             32
#define NET_CONFIG_TYPE_LEN                                  32
#define NET_CONFIG_BIND_KEY_LEN                              64
#define NET_CONFIG_COUNTRY_CODE_LEN                          3
#define NET_CONFIG_BIND_INDEX_LEN                            64
#define NET_CONFIG_BIND_TIMESTAMP_LEN                        8


#define NET_CONFIG_REPORT_PERIOD                             1000

/* Private typedef -----------------------------------------------------------*/

typedef struct {
    uint8_t user_id[NET_CONFIG_USER_ID_LEN];
    uint8_t wifi_ssid[NET_CONFIG_WIFI_SSID_LEN + 1];
    uint8_t wifi_password[NET_CONFIG_WIFI_PASSWORD_LEN + 1];
    uint8_t utc[NET_CONFIG_UTC_LEN];
    uint8_t domain[NET_CONFIG_DOMAIN_LEN];
    uint8_t time_zone[NET_CONFIG_TIME_ZONE_LEN + 1];
    uint8_t type[NET_CONFIG_TYPE_LEN + 1];
    uint8_t bind_key[NET_CONFIG_BIND_KEY_LEN + 1];
    uint8_t code[NET_CONFIG_COUNTRY_CODE_LEN];
    uint8_t bind_index[NET_CONFIG_BIND_INDEX_LEN + 1];
    uint8_t bind_ts[NET_CONFIG_BIND_TIMESTAMP_LEN];
    uint8_t offset;
} net_config_t;

/* Private function prototypes -----------------------------------------------*/

static void net_config_execute(void *arg);
static void net_config_ready(void *arg);
static void net_config_stop(void *arg);
static void net_state_report(void *arg);

static int do_start_search_band(miio_rpc_delegate_arg_t *req_arg,
                                        miio_fp_rpc_delegate_ack_t ack, void* ctx);
static int do_get_band_list(miio_rpc_delegate_arg_t *req_arg,
                                        miio_fp_rpc_delegate_ack_t ack, void* ctx);
static void net_timeout_callback(arch_os_timer_handle_t handle);

/* Private variables ---------------------------------------------------------*/

static net_config_t *net_info;
static arch_os_mutex_handle_t net_mutex_handle;
static mible_net_state_t net_state;
static miio_handle_t miio_handle;
static const mible_net_callbacks_t *mible_net_callbacks;
static arch_os_timer_handle_t net_timer_handle;

/* Exported functions --------------------------------------------------------*/

int mible_net_init(void)
{
    int ret;

    ret = arch_os_mutex_create(&net_mutex_handle);
    if (ret) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Init net mutex failed (%d)", ret);
        return ret;
    }

    ret = arch_os_timer_create(&net_timer_handle, "net", NET_CONFIG_REPORT_PERIOD,
                               net_timeout_callback, NULL,
                               ARCH_OS_TIMER_PERIODIC, ARCH_OS_TIMER_NO_ACTIVATE);
    if (ret) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Init net timer failed (%d)", ret);
        return ret;
    }

    do_start_search_band(NULL, NULL, NULL);
    do_get_band_list(NULL, NULL, NULL);

    net_state = NET_CONFIG_STATE_INVALID;
    mible_net_callbacks = NULL;
    net_info = NULL;
    miio_handle = NULL;

    return MIIO_OK;
}

int mible_net_register(miio_handle_t handle, const mible_net_callbacks_t* p_callbacks)
{
    if (NULL == handle || NULL == p_callbacks) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "mible_net_register invalid param");
        return MIIO_ERROR_PARAM;
    }

    miio_handle = handle;
    mible_net_callbacks = p_callbacks;

    return MIIO_OK;
}

mible_net_state_t mible_net_state_get(void)
{
    return net_state;
}

bool mible_net_state_check(void)
{
    return NET_CONFIG_STATE_COMPLETED == net_state;
}

bool mible_net_beacon_check(void)
{
    return NET_CONFIG_STATE_DISCONNECTED == net_state;
}

bool mible_net_stop_check(void)
{
    return NET_CONFIG_STATE_INVALID == net_state;
}

int mible_net_state_callback(miio_net_state_t state, miio_net_error_t error, void *ctx)
{
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Get wifi state: %d", state);
    switch (state) {
        case MIIO_NET_CLOUD:
            if (MIIO_NET_ERROR_NONE == error) {
                net_state = NET_CONFIG_STATE_COMPLETED;
                mible_event_t event;
                event.task.func = net_config_ready;
                event.task.arg = NULL;
                if (MIIO_OK != mible_event_send(MIBLE_EVT_TASK_POST, &event)) {
                    LOG_WARN_TAG(MIBLE_LOG_TAG, "net config complete cannot send event");
                }                
                uint8_t state = (MIBLE_GATEWAY_STATE_UNKNOWN == gateway_state.user_state)?
                                gateway_state.cur_state : gateway_state.user_state;
                mible_rpc_mesh_switch(state);
            }
            break;
        case MIIO_NET_LOCAL:
            if (MIIO_NET_ERROR_NONE == error) {
                net_state = NET_CONFIG_STATE_CONNECTED;
            }
            break;
        case MIIO_NET_DISCONNECTED:
            if (MIIO_NET_ERROR_AUTH_FAIL == error) {
                net_state = NET_CONFIG_STATE_ERR_WRONG;
            } else {
                net_state = NET_CONFIG_STATE_CONNECTING;
                if (MIIO_OK == mible_gap_scan(false)) {
                    if(MIIO_OK != mible_gap_scan(true))
                    {
                        return MIIO_ERROR;
                    }
                }
                else
                {
                    return MIIO_ERROR;
                }
            }
            break;
        case MIIO_NET_UNPROV: {
            net_state = NET_CONFIG_STATE_INVALID;
            mible_event_t event;
            event.task.func = net_config_stop;
            event.task.arg = NULL;
            if (MIIO_OK != mible_event_send(MIBLE_EVT_TASK_POST, &event)) {
                LOG_WARN_TAG(MIBLE_LOG_TAG, "net config complete send event");
            }
            break;
        }
        case MIIO_NET_UAP: {
            net_state = NET_CONFIG_STATE_DISCONNECTED;
            mible_event_t event;
            event.task.func = net_config_ready;
            event.task.arg = NULL;
            if (MIIO_OK != mible_event_send(MIBLE_EVT_TASK_POST, &event)) {
                LOG_WARN_TAG(MIBLE_LOG_TAG, "net config start send event");
            }
            break;
        }
        default:
            break;
    }

    return MIIO_OK;
}

void mible_net_construct(uint8_t *data, int length)
{
    int data_len = length - 2;    /* left length and opcode */

    if (NULL == data || data_len < 0) {
        return;
    }

    arch_os_mutex_get(net_mutex_handle, ARCH_OS_WAIT_FOREVER);

    if (NET_CONFIG_USER_ID == data[1] &&       /* opcode */
        NET_CONFIG_USER_ID_LEN == data_len &&
        NET_CONFIG_USER_ID_LEN == data[0]) {   /* left length */

        if (NULL != net_info) {
            LOG_WARN_TAG(MIBLE_LOG_TAG, "Another workflow is running");
            goto end;
        }
        net_info = (net_config_t *)malloc(sizeof(net_config_t));
        if (NULL == net_info) {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for net construct workflow");
            goto end;
        }
        memset(net_info, 0, sizeof(net_config_t));
        memcpy(&net_info->user_id[0], &data[2], NET_CONFIG_USER_ID_LEN);
        goto end;
    }

    if (NULL == net_info) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "The workflow has not started");
        goto end;
    }

    switch (data[1]) {   /* opcode */
        case NET_CONFIG_WIFI_SSID:
            if ((net_info->offset + data_len) > NET_CONFIG_WIFI_SSID_LEN) {
                LOG_WARN_TAG(MIBLE_LOG_TAG, "Exceed SSID max len");
                break;
            }
            memcpy(&net_info->wifi_ssid[net_info->offset], &data[2], data_len);
            net_info->offset += data_len;
            if (data[0] == data_len) {    /* left length */
                net_info->wifi_ssid[net_info->offset] = '\0';
                net_info->offset = 0;
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "WiFi SSID is completed");
            }
            break;
        case NET_CONFIG_WIFI_PASSWORD:
            if ((net_info->offset + data_len) > NET_CONFIG_WIFI_PASSWORD_LEN) {
                LOG_WARN_TAG(MIBLE_LOG_TAG, "Exceed password max len");
                break;
            }
            memcpy(&net_info->wifi_password[net_info->offset], &data[2], data_len);
            net_info->offset += data_len;
            if (data[0] == data_len) {    /* left length */
                net_info->wifi_password[net_info->offset] = '\0';
                net_info->offset = 0;
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "WiFi password is completed");
                mible_event_t event;
                event.task.arg = NULL;
                event.task.func = net_config_execute;
                mible_event_send(MIBLE_EVT_TASK_POST, &event);
            }
            break;
        case NET_CONFIG_CMD_RETRY: {
            mible_event_t event;
            event.task.arg = NULL;
            event.task.func = net_config_execute;
            mible_event_send(MIBLE_EVT_TASK_POST, &event);
            break;
        }
        case NET_CONFIG_CMD_RESTORE:
            mible_gateway_restore();
            if (net_info) {
                free(net_info);
                net_info = NULL;
            }
            if (miio_handle && mible_net_callbacks && mible_net_callbacks->restore_callback) {
                mible_net_callbacks->restore_callback(miio_handle);
            }
            break;
        case NET_CONFIG_UTC:
            if (NET_CONFIG_UTC_LEN == data_len &&
                NET_CONFIG_UTC_LEN == data[0]) {    /* left length */

                memcpy(&net_info->utc[0], &data[2], data_len);
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "UTC is completed");
            } else {
                /* set UTC as INT32_MAX */
                int32_t temp = INT32_MAX;
                memcpy(&net_info->utc[0], &temp, NET_CONFIG_UTC_LEN);
                LOG_WARN_TAG(MIBLE_LOG_TAG, "UTC length is invalid");
            }
            break;
        case NET_CONFIG_DOMAIN:
            if (NET_CONFIG_DOMAIN_LEN == data_len &&
                NET_CONFIG_DOMAIN_LEN == data[0]) {    /* left length */

                memcpy(&net_info->domain[0], &data[2], data_len);
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Domain is completed");
            }
            break;
        case NET_CONFIG_TIME_ZONE:
            if ((net_info->offset + data_len) > NET_CONFIG_TIME_ZONE_LEN) {
                LOG_WARN_TAG(MIBLE_LOG_TAG, "Exceed time zone max len");
                break;
            }
            memcpy(&net_info->time_zone[net_info->offset], &data[2], data_len);
            net_info->offset += data_len;
            if (data[0] == data_len) {    /* left length */
                net_info->time_zone[net_info->offset] = '\0';
                net_info->offset = 0;
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Time zone is completed");
            }
            break;
        case NET_CONFIG_TYPE:
            if ((net_info->offset + data_len) > NET_CONFIG_TYPE_LEN) {
                LOG_WARN_TAG(MIBLE_LOG_TAG, "Exceed config type max len");
                break;
            }
            memcpy(&net_info->type[net_info->offset], &data[2], data_len);
            net_info->offset += data_len;
            if (data[0] == data_len) {    /* left length */
                net_info->type[net_info->offset] = '\0';
                net_info->offset = 0;
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Config type is completed");
            }
            break;
        case NET_CONFIG_BIND_KEY:
            if ((net_info->offset + data_len) > NET_CONFIG_BIND_KEY_LEN) {
                LOG_WARN_TAG(MIBLE_LOG_TAG, "Exceed bind key max len");
                break;
            }
            memcpy(&net_info->bind_key[net_info->offset], &data[2], data_len);
            net_info->offset += data_len;
            if (data[0] == data_len) {    /* left length */
                net_info->bind_key[net_info->offset] = '\0';
                net_info->offset = 0;
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Bind key is completed");
            }
            break;
        case NET_CONFIG_COUNTRY_CODE:
            if (NET_CONFIG_COUNTRY_CODE_LEN == data_len &&
                NET_CONFIG_COUNTRY_CODE_LEN == data[0]) {    /* left length */

                memcpy(&net_info->code[0], &data[2], data_len);
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Country code is completed");
            }
            break;
        default:
            /* unknown opcode */
            LOG_WARN_TAG(MIBLE_LOG_TAG, "The workflow can not deal with %d", data[1]);
            break;
    }

end:
    arch_os_mutex_put(net_mutex_handle);
}

static int mible_parse_TLV(uint8_t *data)
{
    if (NULL == data) {
        return 0;
    }

    int type = data[0];
    int len  = data[1];
    uint8_t* value = data+2;

    
    if (NET_CONFIG_USER_ID == type)
    {
        if(NET_CONFIG_USER_ID_LEN == len)
        {
            memcpy(&net_info->user_id[0], value, NET_CONFIG_USER_ID_LEN);
            //arch_dump_hex(net_info->user_id, NET_CONFIG_USER_ID_LEN, NULL);
            return NET_CONFIG_USER_ID_LEN+2;
        }
        else
        {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "NET_CONFIG_USER_ID_LEN error !");
            return 0;
        }
    }
        
    else if (NET_CONFIG_WIFI_SSID == type)
    {
        if ((net_info->offset + len) > NET_CONFIG_WIFI_SSID_LEN) {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "Exceed SSID max len");
            return 0;
        }

        memcpy(&net_info->wifi_ssid[net_info->offset], value, len);
        net_info->offset += len;
        net_info->wifi_ssid[net_info->offset] = '\0';
        net_info->offset = 0;
        //arch_dump_hex(net_info->wifi_ssid, len, NULL);
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "WiFi SSID is completed");

        return len+2;
    }
    else if (NET_CONFIG_WIFI_PASSWORD == type)
    {
        if ((net_info->offset + len) > NET_CONFIG_WIFI_PASSWORD_LEN) {
            LOG_WARN_TAG(MIBLE_LOG_TAG, "Exceed password max len");
            return 0;
        }

        memcpy(&net_info->wifi_password[net_info->offset], value, len);
        net_info->offset += len;
        net_info->wifi_password[net_info->offset] = '\0';
        net_info->offset = 0;
        //arch_dump_hex(net_info->wifi_password, len, NULL);
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "WiFi password is completed");

        return len+2;
     }

/*
        case NET_CONFIG_CMD_RETRY: {
            mible_event_t event;
            event.task.arg = NULL;
            event.task.func = net_config_execute;
            mible_event_send(MIBLE_EVT_TASK_POST, &event);
            break;
        }
        case NET_CONFIG_CMD_RESTORE:
            mible_gateway_restore();
            if (net_info) {
                free(net_info);
                net_info = NULL;
            }
            if (miio_handle && mible_net_callbacks && mible_net_callbacks->restore_callback) {
                mible_net_callbacks->restore_callback(miio_handle);
            }
            break;
*/
    else if (NET_CONFIG_UTC == type)
    {
        if (NET_CONFIG_UTC_LEN == len ) {    /* left length */
            memcpy(&net_info->utc[0], value, len);
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "UTC is completed");
        } else {
            /* set UTC as INT32_MAX */
            int32_t temp = INT32_MAX;
            memcpy(&net_info->utc[0], &temp, NET_CONFIG_UTC_LEN);
            LOG_WARN_TAG(MIBLE_LOG_TAG, "UTC length is invalid");
        }
        //arch_dump_hex(net_info->utc, NET_CONFIG_UTC_LEN, NULL);
        return len+2;
    }
            
    else if (NET_CONFIG_DOMAIN == type)
    {
        if (NET_CONFIG_DOMAIN_LEN == len ) {    /* left length */
            memcpy(&net_info->domain[0], value, len);
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Domain is completed");
            //arch_dump_hex(net_info->utc, NET_CONFIG_UTC_LEN, NULL);
            return len+2;
        }
        else
        {
            LOG_WARN_TAG(MIBLE_LOG_TAG, "Domain length is invalid");
            return 0;
        }
    }

    else if (NET_CONFIG_TIME_ZONE == type)
    {
        if ((net_info->offset + len) > NET_CONFIG_TIME_ZONE_LEN) {
            LOG_WARN_TAG(MIBLE_LOG_TAG, "Exceed time zone max len");
            return 0;
        }
        memcpy(&net_info->time_zone[net_info->offset], value, len);
        net_info->offset += len;
        net_info->time_zone[net_info->offset] = '\0';
        net_info->offset = 0;
        //arch_dump_hex(net_info->time_zone, len, NULL);
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Time zone is completed");
        return len+2;
    }

    else if (NET_CONFIG_TYPE == type)
    {
        if ((net_info->offset + len) > NET_CONFIG_TYPE_LEN) {
            LOG_WARN_TAG(MIBLE_LOG_TAG, "Exceed config type max len");
            return 0;
        }
        memcpy(&net_info->type[net_info->offset], value, len);
        net_info->offset += len;
        net_info->type[net_info->offset] = '\0';
        net_info->offset = 0;
        //arch_dump_hex(net_info->type, len, NULL);
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Config type is completed");
        return len+2;
    }

    else if (NET_CONFIG_BIND_KEY == type)
    {
        if ((net_info->offset + len) > NET_CONFIG_BIND_KEY_LEN) {
            LOG_WARN_TAG(MIBLE_LOG_TAG, "Exceed bind key max len");
            return 0;
        }
        memcpy(&net_info->bind_key[net_info->offset], value, len);
        net_info->offset += len;
        net_info->bind_key[net_info->offset] = '\0';
        net_info->offset = 0;
        //arch_dump_hex(net_info->bind_key, len, NULL);
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Bind key is completed");
        return len+2;
    }

    else if (NET_CONFIG_COUNTRY_CODE == type)
    {
        //Android send 2 bytes, iOS send 3 bytes
        if (NET_CONFIG_COUNTRY_CODE_LEN-1 == len || NET_CONFIG_COUNTRY_CODE_LEN == len) {    /* left length */
            memcpy(&net_info->code[0], value, len);
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Country code is completed");
            //arch_dump_hex(net_info->code, len, NULL);
            return len+2;
        }
        else
        {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "NET_CONFIG_COUNTRY_CODE_LEN error");
            return 0;
        }
    }

    else if(NET_CONFIG_BIND_INDEX == type)
    {
        if ((net_info->offset + len) > NET_CONFIG_BIND_INDEX_LEN) {
            LOG_WARN_TAG(MIBLE_LOG_TAG, "Exceed bind index max len");
            return 0;
        }
        memcpy(&net_info->bind_index[net_info->offset], value, len);
        net_info->offset += len;
        net_info->bind_index[net_info->offset] = '\0';
        net_info->offset = 0;
        //arch_dump_hex(net_info->bind_index, len, NULL);
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Bind index is completed");
        return len+2;
    }

    else if(NET_CONFIG_BIND_TIMESTAMP == type)
    {
        if(NET_CONFIG_BIND_TIMESTAMP_LEN == len)
        {
            memcpy(&net_info->bind_ts[0], value, NET_CONFIG_BIND_TIMESTAMP_LEN);
            //arch_dump_hex(net_info->bind_ts, NET_CONFIG_BIND_TIMESTAMP_LEN, NULL);
            return NET_CONFIG_BIND_TIMESTAMP_LEN+2;
        }
        else
        {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "NET_CONFIG_BIND_TIMESTAMP_LEN error !");
            return 0;
        }
    }

    else
    {
        /* unknown opcode */
        LOG_WARN_TAG(MIBLE_LOG_TAG, "The workflow can not deal with opcode %d, len = %d", type, len);

        //For future feature, we should deal with unknown opcode, not return error
        return len+2;
    }

}
void mible_net_construct_from_package_data(uint8_t *data, int length)
{
    if (NULL == data || length <= 0) {
        return;
    }

    arch_os_mutex_get(net_mutex_handle, ARCH_OS_WAIT_FOREVER);

    int index = 0;

    if (NULL != net_info) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Another workflow is running");
        goto end;
    }
    net_info = (net_config_t *)malloc(sizeof(net_config_t));
    if (NULL == net_info) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for net construct workflow");
        goto end;
    }
    memset(net_info, 0, sizeof(net_config_t));

    while(index < length)
    {
        int ret = mible_parse_TLV(data+index);
        if(0 == ret)
        {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "mible_net_construct_from_TLV error");
            goto end;
        }
        else
        {
            index += ret;
            if(index == length)
            {
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "mible_parse_TLV OK");
                break;
            }
        }
    }

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "send event to do net_config_execute");
    mible_event_t event;
    event.task.arg = NULL;
    event.task.func = net_config_execute;
    mible_event_send(MIBLE_EVT_TASK_POST, &event);    

end:
    arch_os_mutex_put(net_mutex_handle);
}


miio_handle_t mible_net_handle_get(void)
{
    return miio_handle;
}

static void net_config_execute(void *arg)
{
    mible_net_config_params_t net_config_params = {0};

    arch_os_mutex_get(net_mutex_handle, ARCH_OS_WAIT_FOREVER);

    if (mible_net_state_check()) {
        LOG_INFO_TAG(MIBLE_LOG_TAG, "The wifi system is ready");
        if (net_info) {
            free(net_info);
            net_info = NULL;
        }
        goto end;
    }

    if (NULL == net_info) {
        goto end;
    }

    /* Set user ID */
    memcpy(&net_config_params.uid, net_info->user_id, NET_CONFIG_USER_ID_LEN);

    /* Set gmt offset */
    memcpy(&net_config_params.gmt_offset, net_info->utc, sizeof(net_config_params.gmt_offset));
    if(net_config_params.gmt_offset < (-12)*60*60 || net_config_params.gmt_offset > 12*60*60) {
        net_config_params.gmt_offset = 8*60*60;     //default beijing time.
    }

    /* Set country domain */
    if ((NET_CONFIG_DOMAIN_LEN - 1) == strlen((char *)net_info->domain)) {
        net_config_params.country_domain = (char *)net_info->domain;
    } else {
        net_config_params.country_domain = NULL;
    }

    /* Set wifi channel */
    if ((NET_CONFIG_COUNTRY_CODE_LEN - 1) == strlen((char *)net_info->code)) {
        net_config_params.wifi_channel.cc = (char *)net_info->code;
    } else {
        net_config_params.wifi_channel.cc = NULL;
    }

    /* Set SSID and password */
    if (strlen((char *)net_info->wifi_ssid) > 0) {
        net_config_params.ap.ssid = (char *)net_info->wifi_ssid;
        LOG_INFO_TAG(MIBLE_LOG_TAG, "Start to connect AP (%s)", (char *)net_info->wifi_ssid);
    } else {
        net_config_params.ap.ssid = NULL;
        LOG_WARN_TAG(MIBLE_LOG_TAG, "invalid ssid");
        net_state = NET_CONFIG_STATE_ERR_UNKNOWN;
        goto end;
    }
    if (strlen((char *)net_info->wifi_password) > 0) {
        net_config_params.ap.password = (char *)net_info->wifi_password;
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "password -- %s", (char *)net_info->wifi_password);
    } else {
        net_config_params.ap.password = NULL;
    }

    /* Set Bind Key */
    if (strlen((char *)net_info->bind_key) > 0) {
        net_config_params.bindkey = (char *)net_info->bind_key;
    } else {
        net_config_params.bindkey = NULL;
    }

    /* Set config type */
    if (strlen((char *)net_info->type) > 0) {
        net_config_params.config_type = (char *)net_info->type;
    } else {
        net_config_params.config_type = NULL;
    }

#if MIIO_KEY_EXCHANGE_ENABLE
    /* Set Bind index */
    if (strlen((char *)net_info->bind_index) > 0) {
        net_config_params.bind_idx = (char *)net_info->bind_index;
    } else {
        net_config_params.bind_idx = NULL;
    }

    /* Set Bind ts */
    memcpy(&net_config_params.bind_ts, net_info->bind_ts, NET_CONFIG_BIND_TIMESTAMP_LEN);
#endif

    /* Callback */
    if (miio_handle && mible_net_callbacks && mible_net_callbacks->config_callback) {
        mible_net_callbacks->config_callback(miio_handle, &net_config_params);
    }

    /* Start to report state */
    if (MIIO_OK != arch_os_timer_is_active(net_timer_handle)) {
        arch_os_timer_activate(net_timer_handle);
    }

end:
    arch_os_mutex_put(net_mutex_handle);
}

static void net_state_report(void *arg)
{
    if (NET_CONFIG_STATE_INVALID == net_state) {
        return;
    }

    uint8_t tmp = net_state;
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Sending net current state: %d", tmp);
    mible_wifi_report(tmp);
    switch (tmp) {
        case NET_CONFIG_STATE_COMPLETED:
        case NET_CONFIG_STATE_ERR_UNKNOWN:
        case NET_CONFIG_STATE_ERR_WRONG:
            arch_os_timer_deactivate(net_timer_handle);
            break;
        default:
            break;
    }
}

static void net_config_ready(void *arg)
{
    /* When wifi is in AP mode or has connected server */
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "WiFi is ready and current state is %d", net_state);
    /* clear buffer */
    arch_os_mutex_get(net_mutex_handle, ARCH_OS_WAIT_FOREVER);
    if (net_info) {
        free(net_info);
        net_info = NULL;
    }
    arch_os_mutex_put(net_mutex_handle);
    /* Refresh the scan parameters */
    if (MIIO_OK == mible_gap_scan(false)) {
        mible_gap_scan(true);
    }
    /* Start sending mibeacon */
    mible_beacon_start();
}

static void net_config_stop(void *arg)
{
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "WiFi config stop, then stop sending mibeacon");
    arch_os_mutex_get(net_mutex_handle, ARCH_OS_WAIT_FOREVER);
    if (net_info) {
        free(net_info);
        net_info = NULL;
    }
    arch_os_mutex_put(net_mutex_handle);

    mible_beacon_stop();
}

static void net_timeout_callback(arch_os_timer_handle_t handle)
{
    mible_event_t event;

    event.task.func = net_state_report;
    event.task.arg = NULL;
    mible_event_send(MIBLE_EVT_TASK_POST, &event);
}

static int do_start_search_band(miio_rpc_delegate_arg_t *req_arg,
                                miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
    int value = -1;

    if (NULL == req_arg || NULL == ack) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "do_start_search_band invalid param");
        return MIIO_ERROR_PARAM;
    }

    if (MIIO_DELEGATE_JSON == req_arg->type) {
        jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(req_arg->pload, req_arg->pload_len);
        if(MIIO_OK == jsmi_parse_start(&jsmi_parser)) {
            // params
            jsmi_tok_path_t path[] = {
                {
                    .key = "params",
                    .type = JSMN_ARRAY
                },
                {
                    .key = (void*)0,
                    .type = JSMN_PRIMITIVE
                }
            };
            int ret = jsmi_get_value_sint(&jsmi_parser, NULL, path,
                                            NELEMENTS(path), &value);
            if(MIIO_OK != ret){
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "\"params\" not found\r");
            } else {
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Get scan window %d", value);
            }
        }
        jsmi_parse_finish(&jsmi_parser);
    }

    if (value > 0) {
        mible_band_set(value);
        miio_rpc_delegate_ack_ok(req_arg, ack, ctx);
    } else {
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
                                    MIIO_OT_ERR_INFO_PARAM_INVALID, ack, ctx);
    }

    return MIIO_OK;
}
MIIO_RPC(bleStartSearchBand, do_start_search_band, "Hello, MIoT");

static int do_get_band_list(miio_rpc_delegate_arg_t *req_arg,
                            miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
    if (NULL == req_arg || NULL == ack) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "do_get_band_list invalid param");
        return MIIO_ERROR_PARAM;
    }

    return mible_band_rpc_delegate_ack(req_arg, ack, ctx);
}
MIIO_RPC(bleGetNearbyBandList, do_get_band_list, "Hello, MIoT");

static int do_ble_gateway_enable(miio_rpc_delegate_arg_t *req_arg,
                            miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
    int ret = MIIO_ERROR_PARAM;

    if (NULL == req_arg || NULL == ack) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "do_ble_gateway_enable invalid param");
        return ret;
    }

    if (MIIO_DELEGATE_JSON == req_arg->type) {
        jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(req_arg->pload, req_arg->pload_len);
        if(MIIO_OK == jsmi_parse_start(&jsmi_parser)) {
            ret = gateway_enable_handler(&jsmi_parser, "params");
        }
        jsmi_parse_finish(&jsmi_parser);
    }

    if (MIIO_OK == ret) {
        miio_rpc_delegate_ack_ok(req_arg, ack, ctx);
    } else {
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
                                    MIIO_OT_ERR_INFO_PARAM_INVALID, ack, ctx);
    }

    return ret;
}
MIIO_RPC(ble_gateway_enable, do_ble_gateway_enable, "Hello, MIoT");

