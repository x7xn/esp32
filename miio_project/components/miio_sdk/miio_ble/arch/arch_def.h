/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ARCH_TYPEDEF_H__
#define __ARCH_TYPEDEF_H__

/* Includes ------------------------------------------------------------------*/

#include "arch_dbg.h"

/* Exported macro ------------------------------------------------------------*/

#define MISERVICE_UUID                                            0xFE95

#define MISERVICE_CHAR_TOKEN_UUID                                 0x0001
#define MISERVICE_CHAR_PRODUCTID_UUID                             0x0002
#define MISERVICE_CHAR_VERSION_UUID                               0x0004
#define MISERVICE_CHAR_WIFICFG_UUID                               0x0005
#define MISERVICE_CHAR_CTRL_POINT_UUID                            0x0010  //lagacy for rc4 auth handle
#define MISERVICE_CHAR_DEVICE_ID_UUID                             0x0013
#define MISERVICE_CHAR_BEACONKEY_UUID                             0x0014
#define MISERVICE_CHAR_STAND_AUTH_UUID                            0x0019


#define MISERVICE_CHAR_TOKEN_LEN                                  12
#define MISERVICE_CHAR_PRODUCTID_LEN                              2
#define MISERVICE_CHAR_VERSION_LEN                                20
#define MISERVICE_CHAR_WIFICFG_LEN                                20
#define MISERVICE_CHAR_AUTHENTICATION_LEN                         4
#define MISERVICE_CHAR_SN_LEN                                     20
#define MISERVICE_CHAR_BEACONKEY_LEN                              12
#define MISERVICE_CHAR_CTRL_LEN                                   20
#define MISERVICE_CHAR_AUTH_LEN                                   20



/* Exported types ------------------------------------------------------------*/

typedef struct {
    uint16_t token_handle;
    uint16_t token_ccc_handle;
    uint16_t pid_handle;
    uint16_t version_handle;
    uint16_t wifi_handle;
    uint16_t wifi_ccc_handle;
    uint16_t ctrl_handle;
    uint16_t ctrl_ccc_handle;
    uint16_t auth_handle;
    uint16_t auth_ccc_handle;
    uint16_t stand_auth_handle;
    uint16_t stand_auth_ccc_handle;
    uint16_t did_handle;
    uint16_t key_handle;
} arch_gatt_t;

typedef struct {
    uint16_t product_id;
} arch_info_t;

typedef struct {
    uint16_t interval;
    uint16_t window;
} arch_scan_param_t;

typedef enum {
    ARCH_ADV_DATA_SERVICE,
    ARCH_ADV_DATA_MANUFACTURER,
} arch_adv_data_t;

#endif
