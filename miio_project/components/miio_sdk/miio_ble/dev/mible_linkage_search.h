/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MIBLE_LINKAGE_SEARCH_H__
#define __MIBLE_LINKAGE_SEARCH_H__

/* Includes ------------------------------------------------------------------*/

#include "mible_def.h"
#include "mibeacon_def.h"

/* Private define ------------------------------------------------------------*/

#define MIBLE_LINKAGE_SEARCH_MAX_NUM                   10
#define MIBLE_LINKAGE_DEFAULT_RSSI                     (-127)

/* Private typedef -----------------------------------------------------------*/

typedef struct {
    mible_addr_t address;
    uint16_t product_id;
    int8_t rssi;
} search_device_t;

typedef struct {
    uint16_t cur_num;
    search_device_t target[MIBLE_LINKAGE_SEARCH_MAX_NUM];
} search_target_array_t;

/* Exported functions --------------------------------------------------------*/

int mible_linkage_search_init(void);
int mible_linkage_search_start(search_target_array_t *target_array, uint32_t timeout);
int mible_linkage_search_result_update(mibeacon_info_t *p_info);

#endif
