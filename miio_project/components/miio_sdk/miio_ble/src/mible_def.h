#ifndef __MIBLE_TYPEDEF_H__
#define __MIBLE_TYPEDEF_H__

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>

#include "arch_os.h"
#include "arch_dbg.h"

/* Exported macro ------------------------------------------------------------*/

#define MIBLE_EVT_STACK_ENABLE                                    0x0000
#define MIBLE_EVT_TASK_POST                                       0x0001

#define MIBLE_EVT_GATTS_PERUSER                                   0x0100
#define MIBLE_EVT_GATTC_PERUSER                                   0x0200
#define MIBLE_EVT_GAP_PERUSER                                     0x0300

#define MIBLE_HANDLE_INVALID                                      0xFFFF
#define MIBLE_CONNECTION_CENTRAL_NUM                              4

#ifndef NULL
#define NULL 0
#endif

/* Exported types ------------------------------------------------------------*/

typedef uint8_t mible_addr_t[6];

typedef struct {
    void (* func)(void *);
    void * arg;
} mible_task_post_t;

#endif
