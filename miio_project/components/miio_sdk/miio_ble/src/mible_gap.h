/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MIBLE_GAP_H__
#define __MIBLE_GAP_H__

/* Includes ------------------------------------------------------------------*/

#include "mible_def.h"

/* Exported types ------------------------------------------------------------*/

typedef struct {
    mible_addr_t address;
    uint8_t len;
    uint8_t data[31];
    int8_t rssi;
} mible_beacon_t;

typedef struct {
    mible_addr_t address;
    uint16_t step;
    uint8_t sleep;
    int8_t rssi;
    uint32_t ext_data;
} mible_band_t;

typedef enum {
    MIBLE_LOCAL_ROLE_MASTER,
    MIBLE_LOCAL_ROLE_SLAVE,
} mible_local_role_t;

typedef struct {
    mible_addr_t address;
    uint16_t conn_handle;
    mible_local_role_t local_role;
} mible_connect_t;

typedef struct {
    uint16_t conn_handle;
} mible_disconnect_t;

/* Exported constants --------------------------------------------------------*/

#define MIBLE_EVT_GAP_BEACON                          MIBLE_EVT_GAP_PERUSER + 0
#define MIBLE_EVT_GAP_BAND                            MIBLE_EVT_GAP_PERUSER + 1
#define MIBLE_EVT_GAP_CONNECT                         MIBLE_EVT_GAP_PERUSER + 2
#define MIBLE_EVT_GAP_DISCONNECT                      MIBLE_EVT_GAP_PERUSER + 3

/* Exported functions --------------------------------------------------------*/

int mible_gap_init(void);
int mible_gap_deinit(void);
int mible_beacon_start(void);
int mible_beacon_stop(void);
int mible_connect_handler(mible_connect_t *p_connect);
int mible_disconnect_handler(mible_disconnect_t *p_disconnect);
mible_connect_t * mible_find_connection_by_handle(uint16_t conn_handle);
mible_connect_t * mible_find_connection_by_address(mible_addr_t address);
int mible_gap_scan(bool enable);

#endif
