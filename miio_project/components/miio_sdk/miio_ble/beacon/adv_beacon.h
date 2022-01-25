#ifndef __ADV_BEACON_H__
#define __ADV_BEACON_H__

#include "mible_gap.h"

/*
 * Definition for BLE advertising type
 */
#define MIBLE_ADTYPE_SERVICE_DATA                 0x16  //!< Service Data
#define MIBLE_ADTYPE_MANUFACTURER_SPECIFIC        0xFF  //!< Manufacturer Specific Data: first 2 octets contain the Company Identifier Code followed by the additional manufacturer specific data

#define MIBLE_SERVICE_UUID                        0xFE95
#define MIBLE_MANUFT_ID                           0x038F

#define HUAMI_MANUFT_ID                           0x0157
#define HUAMI_SERVICE_UUID                        0xfee0
#define SIG_WEIGHT_SERVICE_UUID                   0x181D

#define MIBAND_DATA_TYPE_REGULAR                  0x00
#define MIBAND_DATA_TYPE_EVENT                    0x01
#define MIBAND_DATA_TYPE_EXTENDED                 0x02


#define MIBEACON_PROXIMITY_TYPE                   0x10
#define MIBEACON_PROXIMITY_LENGTH                 10
#define MIBEACON_PROXIMITY_RSSI_DEFAULT           (-127)

struct _ble_adv_frgm {
    uint8_t size;
    uint8_t type;
    uint8_t data[0];
} __attribute__((packed));
typedef struct _ble_adv_frgm ble_adv_frgm_t;

struct _ble_adv_frgm_data {
    uint16_t tag;
    uint8_t payload[0];
} __attribute__((packed));
typedef struct _ble_adv_frgm_data ble_adv_frgm_data_t;

bool ble_adv_check_frgm(uint8_t *p_adv_data, uint16_t adv_size, uint8_t type, uint16_t tag);
void *ble_adv_get_frgm_data(uint8_t *p_adv_data, uint16_t adv_size, uint8_t *p_seg_data_size,
                            uint8_t type);

int ble_adv_parse(mible_addr_t address, uint8_t *p_adv_data, uint16_t adv_size, int8_t rssi);
int mible_beacon_handler(mible_beacon_t *p_beacon);

#endif
