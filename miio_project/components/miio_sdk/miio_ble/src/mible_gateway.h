/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MIBLE_GATEWAY_H__
#define __MIBLE_GATEWAY_H__

/* Includes ------------------------------------------------------------------*/

#include "mible_def.h"
#include "mible_gap.h"
#include "mible_gatts.h"
#include "mible_gattc.h"
#include "jsmi.h"

/* Exported constants --------------------------------------------------------*/

#ifndef MIBLE_GATEWAY_DISABLE
    #define MIBLE_GATEWAY_DISABLE                         0
#endif

#define COMMON_LIB_VERSION_MAJOR                2
#define COMMON_LIB_VERSION_MINOR                1
#define COMMON_LIB_VERSION_REVISION             1
#define COMMON_LIB_VERSION_BUILD                0019
#if MIBLE_GATEWAY_DISABLE
#define COMMON_LIB_FEATURE                      "BLE-RC"
#else
#define COMMON_LIB_FEATURE                      "BLE-CG"
#endif

#define STR_VAL(str)  #str
#define CONCAT_VERSION(x,y,z,u,v) STR_VAL(x) "." STR_VAL(y) "." STR_VAL(z) "_" STR_VAL(u) "-" v

#define MIBLE_LIB_VERSION CONCAT_VERSION(COMMON_LIB_VERSION_MAJOR, COMMON_LIB_VERSION_MINOR,\
                    COMMON_LIB_VERSION_REVISION, COMMON_LIB_VERSION_BUILD,COMMON_LIB_FEATURE)

#define MIBLE_GATEWAY_BACKOFF_MAX_RETRY_TIMES            (5)
#define MIBLE_GATEWAY_BACKOFF_MAX_VALUE                  (300)
#define MIBLE_GATEWAY_BACKOFF_FIXED_VALUE                (12 * 60 * 60)

/* Exported types ------------------------------------------------------------*/

typedef struct {
    uint16_t product_id;
    uint16_t version;
} mible_config_t;

typedef union {
    mible_task_post_t task;
    mible_beacon_t mible_beacon;
    mible_band_t mible_band;
    mible_connect_t connect;
    mible_disconnect_t disconnect;
    mible_gatts_write_t gatts_write;
    mible_gattc_found_t gattc_found;
    mible_gattc_write_t gattc_write;
    mible_gattc_notify_t gattc_notify;
    mible_gattc_read_t gattc_read;
} mible_event_t;

typedef enum {
    MIBLE_GATEWAY_STATE_DISABLE,
    MIBLE_GATEWAY_STATE_ENABLE,
    MIBLE_GATEWAY_STATE_UNKNOWN = 0X80,
    MIBLE_GATEWAY_STATE_PENDING,
} mible_gateway_state_t;

typedef struct {
    uint32_t last_timestamp;
    uint32_t back_off;
    mible_gateway_state_t cur_state;
    mible_gateway_state_t user_state;
    uint16_t retry_times;
    int8_t proximity_rssi;
    uint8_t query_num;
} mible_gateway_t;

typedef struct {
    mible_gateway_state_t user_state;
    uint8_t reserved[7];
} mible_param_t;

/* Exported variables --------------------------------------------------------*/

extern mible_config_t gateway_config;
extern mible_gateway_t gateway_state;

/* Exported functions --------------------------------------------------------*/

int mible_gateway_init(mible_config_t * p_config);
uint8_t mible_gateway_get_state(void);
int mible_gateway_start(void);
int mible_gateway_stop(void);
int mible_gateway_restart(void);
int mible_event_send(uint16_t type, mible_event_t * p_evt);
int mible_gateway_update(mible_config_t * p_config);
int mible_gateway_restore(void);
uint32_t mible_gateway_dump(char *buf, uint32_t buf_size);
bool mible_gateway_enable(void);
int  gateway_auth_callback(uint16_t conn_handle, int status);
int  gateway_enable_handler(jsmi_parser_t *parser, void *parent_key);

#endif
