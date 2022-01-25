#ifndef MIBLE_TYPE_H__
#define MIBLE_TYPE_H__

// Copyright [2017] [Beijing Xiaomi Mobile Software Co., Ltd]
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//#include "mible_port.h"

#define MIBLE_GAP_EVT_BASE   0x00
#define MIBLE_GATTS_EVT_BASE 0x40
#define MIBLE_GATTC_EVT_BASE 0x80


typedef uint8_t mible_addr_t[6];

typedef uint32_t mible_cfm_t;

typedef struct {
    uint16_t begin_handle;
    uint16_t end_handle;
} mible_handle_range_t;

typedef enum {
    MIBLE_ADDRESS_TYPE_PUBLIC, // public address
    MIBLE_ADDRESS_TYPE_RANDOM, // random address
} mible_addr_type_t;

/* GAP related */
typedef enum {
    MIBLE_SCAN_TYPE_PASSIVE,  // passive scanning
    MIBLE_SCAN_TYPE_ACTIVE,   // active scanning
} mible_gap_scan_type_t;

typedef struct {
    uint16_t scan_interval;                   // Range: 0x0004 to 0x4000 Time = N * 0.625 msec Time Range: 2.5 msec to 10.24 sec
    uint16_t scan_window;                     // Range: 0x0004 to 0x4000 Time = N * 0.625 msec Time Range: 2.5 msec to 10.24 seconds
    uint16_t timeout;                         // Scan timeout between 0x0001 and 0xFFFF in seconds, 0x0000 disables timeout.
} mible_gap_scan_param_t;

typedef enum {
    MIBLE_ADV_TYPE_CONNECTABLE_UNDIRECTED,      // ADV_IND
    MIBLE_ADV_TYPE_SCANNABLE_UNDIRECTED,        // ADV_SCAN_IND
    MIBLE_ADV_TYPE_NON_CONNECTABLE_UNDIRECTED,  // ADV_NONCONN_INC
} mible_gap_adv_type_t;

typedef struct {
    uint16_t adv_interval_min;               // Range: 0x0020 to 0x4000  Time = N * 0.625 msec Time Range: 20 ms to 10.24 sec
    uint16_t adv_interval_max;               // Range: 0x0020 to 0x4000  Time = N * 0.625 msec Time Range: 20 ms to 10.24 sec
    mible_gap_adv_type_t adv_type;
    
    struct {
        uint8_t ch_37_off : 1;  /**< Setting this bit to 1 will turn off advertising on channel 37 */
        uint8_t ch_38_off : 1;  /**< Setting this bit to 1 will turn off advertising on channel 38 */
        uint8_t ch_39_off : 1;  /**< Setting this bit to 1 will turn off advertising on channel 39 */
    } ch_mask;
} mible_gap_adv_param_t;

typedef enum {
    ADV_DATA,           // advertising data
    SCAN_RSP_DATA,      // response data from active scanning
} mible_gap_adv_data_type_t;

typedef struct {
    mible_addr_t peer_addr;
    mible_addr_type_t addr_type;
    mible_gap_adv_data_type_t adv_type;
    int8_t rssi;
    uint8_t data[31];
    uint8_t data_len;
} mible_gap_adv_report_t;

typedef enum {
    CONNECTION_TIMEOUT = 1,
    REMOTE_USER_TERMINATED,
    LOCAL_HOST_TERMINATED
} mible_gap_disconnect_reason_t;

typedef struct {
    uint16_t min_conn_interval;    // Range: 0x0006 to 0x0C80, Time = N * 1.25 msec, Time Range: 7.5 msec to 4 seconds.
    uint16_t max_conn_interval;    // Range: 0x0006 to 0x0C80, Time = N * 1.25 msec, Time Range: 7.5 msec to 4 seconds.
    uint16_t slave_latency;        // Range: 0x0000 to 0x01F3
    uint16_t conn_sup_timeout;     // Range: 0x000A to 0x0C80, Time = N * 10 msec, Time Range: 100 msec to 32 seconds
} mible_gap_conn_param_t;

typedef enum {
    MIBLE_GAP_INVALID,
    MIBLE_GAP_PERIPHERAL,
    MIBLE_GAP_CENTRAL,
} mible_gap_role_t;

typedef struct {
    mible_addr_t peer_addr;
    mible_addr_type_t type;
    mible_gap_role_t role;
    mible_gap_conn_param_t conn_param;
} mible_gap_connect_t;

typedef struct {
    mible_gap_disconnect_reason_t reason;
} mible_gap_disconnect_t;

typedef struct {
    mible_gap_conn_param_t conn_param;
} mible_gap_connect_update_t;

typedef struct {
    uint16_t conn_handle;
    union {
        mible_gap_connect_t connect;
        mible_gap_disconnect_t disconnect;
        mible_gap_adv_report_t report;
        mible_gap_connect_update_t update_conn;
    };
} mible_gap_evt_param_t;

typedef enum {
    MIBLE_GAP_EVT_CONNECTED = MIBLE_GAP_EVT_BASE, /**< Generated when a connection is established.*/
    MIBLE_GAP_EVT_DISCONNECT, /**< Generated when a connection is terminated.*/
    MIBLE_GAP_EVT_CONN_PARAM_UPDATED,
    MIBLE_GAP_EVT_ADV_REPORT,
} mible_gap_evt_t;

/*GATTS related*/

// GATTS database
typedef struct {
    uint32_t type;                                     // MIBLE_UUID_16 = 0    MIBLE_UUID_128 = 1
    union {
        uint16_t uuid16;
        uint8_t uuid128[16];
    };
} mible_uuid_t;

typedef enum {
    MIBLE_PRIMARY_SERVICE = 1,
    MIBLE_SECONDARY_SERVICE,
} mible_gatts_service_t;








typedef enum {
    MIBLE_BROADCAST           = 0x01,
    MIBLE_READ                = 0x02,
    MIBLE_WRITE_WITHOUT_RESP  = 0x04,
    MIBLE_WRITE               = 0x08,
    MIBLE_NOTIFY              = 0x10,
    MIBLE_INDICATE            = 0x20,
    MIBLE_AUTH_SIGNED_WRITE   = 0x40,
} mible_gatts_char_property;

typedef enum {
    MIBLE_GATTS_EVT_WRITE = MIBLE_GATTS_EVT_BASE,      // When this event is called, the characteristic has been modified.
    MIBLE_GATTS_EVT_READ_PERMIT_REQ,                   // If charicteristic's rd_auth = TRUE, this event will be generated.
    MIBLE_GATTS_EVT_WRITE_PERMIT_REQ,                  // If charicteristic's wr_auth = TRUE, this event will be generated, meanwhile the char value hasn't been modified. mible_gatts_rw_auth_reply().
    MIBLE_GATTS_EVT_IND_CONFIRM
} mible_gatts_evt_t;



/*
 * MIBLE_GATTS_EVT_READ_PERMIT_REQ event callback parameters
 * NOTE: Stack SHOULD decide to reply the char value or refuse according to [permit]
 * */
typedef struct {
    uint16_t value_handle;  // char value handle 
} mible_gatts_read_t;



/*GATTC related*/

/*
 * GATTC event
 * */
typedef enum {
    // this event generated in responses to a discover_primary_service procedure.
    MIBLE_GATTC_EVT_PRIMARY_SERVICE_DISCOVER_RESP = MIBLE_GATTC_EVT_BASE,
    // this event generated in responses to a discover_charicteristic_by_uuid
    // procedure.
    MIBLE_GATTC_EVT_CHR_DISCOVER_BY_UUID_RESP,
    // this event generated in responses to a discover_char_clt_cfg_descriptor
    // procedure.
    MIBLE_GATTC_EVT_CCCD_DISCOVER_RESP,
    // this event generated in responses to a read_charicteristic_value_by_uuid
    // procedure.
    MIBLE_GATTC_EVT_READ_CHAR_VALUE_BY_UUID_RESP,
    // this event generated in responses to a
    // write_charicteristic_value_with_response procedure.
    MIBLE_GATTC_EVT_WRITE_RESP,
    // this event is generated when peer gatts device send a notification. 
    MIBLE_GATTC_EVT_NOTIFICATION,
    // this event is generated when peer gatts device send a indication. 
    MIBLE_GATTC_EVT_INDICATION,
    // this event generated in responses to a discover_charicteristic procedure.
    MIBLE_GATTC_EVT_CHR_DISCOVER_RESP,
} mible_gattc_evt_t;




/*
 * MIBLE_GATTC_EVT_NOTIFICATION or MIBLE_GATTC_EVT_INDICATION event callback parameters
 *  */
typedef struct {
    uint16_t handle;
    uint8_t  len;
    uint8_t  *pdata;
} mible_gattc_notification_or_indication_t;



/* TIMER related */
typedef void (*mible_timer_handler)(void*);

typedef enum {
    MIBLE_TIMER_SINGLE_SHOT,
    MIBLE_TIMER_REPEATED,
} mible_timer_mode;

/* IIC related */
typedef enum {
    IIC_100K = 1,
    IIC_400K,
} iic_freq_t;

typedef struct {
    uint8_t scl_port;
    uint8_t scl_pin;
    uint8_t scl_extra_conf;
    uint8_t sda_port;
    uint8_t sda_pin;
    uint8_t sda_extra_conf;
    iic_freq_t freq;
} iic_config_t;

typedef enum {
    IIC_EVT_XFER_DONE,
    IIC_EVT_ADDRESS_NACK,
    IIC_EVT_DATA_NACK
} iic_event_t;

typedef enum {
    MI_SUCCESS      = 0x00,
    MI_ERR_INTERNAL,
    MI_ERR_NOT_FOUND,
    MI_ERR_NO_EVENT,
    MI_ERR_NO_MEM,
    MI_ERR_INVALID_ADDR,     // Invalid pointer supplied
    MI_ERR_INVALID_PARAM,    // Invalid parameter(s) supplied.
    MI_ERR_INVALID_STATE,    // Invalid state to perform operation.
    MI_ERR_INVALID_LENGTH,
    MI_ERR_DATA_SIZE,
    MI_ERR_TIMEOUT,
    MI_ERR_BUSY,
    MI_ERR_RESOURCES,
    MIBLE_ERR_INVALID_CONN_HANDLE,
    MIBLE_ERR_ATT_INVALID_ATT_HANDLE,
    MIBLE_ERR_GAP_INVALID_BLE_ADDR,
    MIBLE_ERR_GATT_INVALID_ATT_TYPE,
    MIBLE_ERR_UNKNOWN, // other ble stack errors
} mible_status_t;



#endif
