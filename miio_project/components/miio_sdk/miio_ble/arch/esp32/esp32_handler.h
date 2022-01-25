/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ESP32_HANDLER_H__
#define __ESP32_HANDLER_H__

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#include "mible_def.h"
#include "arch_def.h"

int  esp32_event_init(void);
void esp32_event_lock(void);
void esp32_event_unlock(void);
int esp32_event_wait(void);
void esp32_gatts_event_handler(esp_gatts_cb_event_t event,
                    esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void esp32_gattc_event_handler(esp_gattc_cb_event_t event,
                    esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
void esp32_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
void esp32_find_connect(uint16_t conn_id, esp_bd_addr_t remote_bda);

/* Exported variables --------------------------------------------------------*/

extern esp_gatt_if_t esp32_gatts_if, esp32_gattc_if;
extern arch_gatt_t * arch_gatt_db;

#endif
