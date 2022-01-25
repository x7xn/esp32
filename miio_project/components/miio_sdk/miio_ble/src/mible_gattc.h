#ifndef __MIBLE_GATTC_H__
#define __MIBLE_GATTC_H__

/* Includes ------------------------------------------------------------------*/

#include "mible_def.h"

/* Exported types ------------------------------------------------------------*/

typedef struct {
    uint16_t conn_handle;
} mible_gattc_found_t;

typedef struct {
    uint16_t conn_handle;
    uint16_t handle;
} mible_gattc_write_t;

typedef struct {
    uint16_t conn_handle;
    uint16_t handle;
    uint8_t data[20];
    uint8_t len;
} mible_gattc_notify_t;

typedef struct {
    uint16_t conn_handle;
    uint16_t handle;
    uint8_t data[20];
    uint8_t len;
} mible_gattc_read_t;

/* Exported constants --------------------------------------------------------*/

#define MIBLE_EVT_GATTC_FOUND                       MIBLE_EVT_GATTC_PERUSER + 0
#define MIBLE_EVT_GATTC_WRITE                       MIBLE_EVT_GATTC_PERUSER + 1
#define MIBLE_EVT_GATTC_NOTIFY                      MIBLE_EVT_GATTC_PERUSER + 2
#define MIBLE_EVT_GATTC_READ                        MIBLE_EVT_GATTC_PERUSER + 3

/* Exported functions --------------------------------------------------------*/

int mible_gattc_init(void);
int mible_gattc_found_handler(mible_gattc_found_t * p_found);
int mible_gattc_write_handler(mible_gattc_write_t * p_write);
int mible_gattc_notify_handler(mible_gattc_notify_t * p_notify);
int mible_gattc_read_handler(mible_gattc_read_t * p_read);

#endif
