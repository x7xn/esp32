/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FAST_PAIR_H__
#define __FAST_PAIR_H__

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
#include "mible_def.h"

/* Private typedef -----------------------------------------------------------*/

typedef enum {
    FASTPAIR_STATE_SUCC,
    FASTPAIR_STATE_FAIL,
    FASTPAIR_STATE_TIMEOUT,
    FASTPAIR_STATE_EXISTED,
} fastpair_state_t;

/* Exported functions --------------------------------------------------------*/

int mible_fastpair_init(void);
int mible_fastpair_start(mible_addr_t address, uint16_t product_id,
                            uint16_t object_id, int8_t rssi);
uint16_t mible_fastpair_get_pid(void);
int mible_fastpair_complete(uint16_t conn_handle, uint8_t *p_key, bool isSecureFastpair);
int mible_fastpair_update(mible_addr_t address, uint16_t product_id,
                            uint16_t object_id, bool permit);
int mible_fastpair_refresh(void);

#endif
