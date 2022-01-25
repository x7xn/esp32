/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DEV_PRPH_H__
#define __DEV_PRPH_H__

/* Includes ------------------------------------------------------------------*/

#include "arch_def.h"
#include "mible_def.h"

/* Private typedef -----------------------------------------------------------*/

typedef struct {
    mible_addr_t address;
    uint16_t object_id;
    uint8_t beacon_key[MISERVICE_CHAR_BEACONKEY_LEN];
    uint16_t product_id;
    uint32_t frame_counter;
    uint32_t time_stamp;
} mible_prph_t;

/* Exported functions --------------------------------------------------------*/

int mible_prph_init(void);
int mible_prph_reset(void);
int mible_prph_add(mible_addr_t address, uint16_t product_id,
                    uint16_t object_id, uint8_t * p_key);
int mible_prph_delete(mible_addr_t address, uint16_t object_id);
int mible_prph_find_key(mible_addr_t address, uint16_t product_id,
                                    uint8_t *p_key);
bool mible_prph_check_object(mible_addr_t address, uint16_t product_id,
                                    uint16_t object_id);
bool mible_prph_check_counter(mible_addr_t address, uint16_t product_id,
                            uint16_t object_id, uint32_t frame_counter, bool anti_replay);
bool mible_prph_should_encrypted(mible_addr_t address, uint16_t product_id);
bool mible_prph_should_anti_replay(mible_addr_t address, uint16_t product_id);
void mible_prph_attribute_update(mible_addr_t address, uint16_t product_id,
                                bool encrypted, bool anti_replay);
void mible_prph_sync(void);
uint32_t mible_prph_dump(char *buf, uint32_t buf_size);

#endif
