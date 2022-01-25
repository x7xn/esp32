/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MIBLE_GATTS_H__
#define __MIBLE_GATTS_H__

/* Includes ------------------------------------------------------------------*/

#include "mible_def.h"

/* Exported types ------------------------------------------------------------*/

typedef struct {
    uint8_t value[20];
    uint16_t length;
    uint16_t handle;
} mible_gatts_write_t;

/* Exported constants --------------------------------------------------------*/

#define MIBLE_EVT_GATTS_WRITE                       MIBLE_EVT_GATTS_PERUSER + 0

/* Exported functions --------------------------------------------------------*/

int mible_gatts_init(void);
int mible_gatts_write_handler(mible_gatts_write_t *p_write);

#endif
