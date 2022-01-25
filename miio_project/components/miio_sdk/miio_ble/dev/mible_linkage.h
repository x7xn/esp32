/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MIBLE_LINKAGE_H__
#define __MIBLE_LINKAGE_H__

/* Includes ------------------------------------------------------------------*/

#include "arch_def.h"
#include "mible_def.h"
#include "mibeacon_def.h"

/* Private define ------------------------------------------------------------*/

#define MIBLE_LINKAGE_MAX_NUM                          5
#define MIBLE_LINKAGE_SYNC_ITVL                        60
#define MIBLE_LINKAGE_OBJECT_MAX_NUM                   5

/* Private typedef -----------------------------------------------------------*/

typedef enum {
    LINKAGE_FUNCTION_DISABLE=0,
    LINKAGE_FUNCTION_ENABLE,
}linkage_function_state_t;

typedef struct {
    uint16_t object_id;
    uint8_t value_type;
    uint32_t timestamp;
} linkage_object_t;

typedef struct {
    mible_addr_t address;
    uint16_t product_id;
    uint16_t function_type;
    linkage_function_state_t enabled;
    uint8_t beaconkey[MIBEACON_BEACONKEY_LEN];
    linkage_object_t object[MIBLE_LINKAGE_OBJECT_MAX_NUM];
    uint32_t interval;
    uint32_t frame_counter;
} linkage_device_t;

typedef struct {
    uint16_t cur_num;
    linkage_device_t device[MIBLE_LINKAGE_MAX_NUM];
} linkage_device_array_t;

/* Exported functions --------------------------------------------------------*/

int mible_linkage_init(void);
int mible_linkage_add(linkage_device_t *device);
int mible_linkage_delete(linkage_device_t *device);
int mible_linkage_reset(void);
int mible_linkage_get_specified_device(linkage_device_t *device);
int mible_linkage_get_all(linkage_device_array_t *device_list);
int mible_linkage_enable(linkage_device_t *device);
int mible_linkage_enable_all(linkage_function_state_t state);
int mible_linkage_check_state(mible_addr_t address, uint16_t product_id);
int mible_linkage_get_key(mible_addr_t address, uint16_t product_id,
                                    uint8_t *p_key);
bool mible_linkage_check_last_report(mible_addr_t address, uint16_t product_id,
                                                    uint16_t object_id);
bool mible_linkage_check_counter(mible_addr_t address, uint16_t product_id,
                                uint32_t frame_counter, bool anti_replay);
bool mible_linkage_should_encrypted(mible_addr_t address, uint16_t product_id);
bool mible_linkage_should_anti_replay(mible_addr_t address, uint16_t product_id);
void mible_linkage_attribute_update(mible_addr_t address, uint16_t product_id,
                                bool encrypted, bool anti_replay);
void mible_linkage_object_timestamp_update(mible_addr_t address, uint16_t product_id,
                                            uint16_t object_id, uint32_t timestamp);
void mible_linkage_sync(void);
uint32_t mible_linkage_rule_dump(char *buf, uint32_t buf_size);

#endif
