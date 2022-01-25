#ifndef __ARCH_API_H__
#define __ARCH_API_H__

#include "arch_def.h"
#include "arch_dbg.h"
#include "arch_config.h"
#include "mible_gateway.h"
#include "mible_net.h"

/* Exported functions --------------------------------------------------------*/

int arch_stack_enable(void);
int arch_stack_disable(void);
int arch_address_get(mible_addr_t addr);
int arch_info_save(arch_info_t *p_info);
int arch_info_restore(arch_info_t *p_info);
int arch_generate_random(uint8_t * buffer, uint16_t length);
int arch_aes_encrypt(uint8_t out[16], const uint8_t in[16], const uint8_t key[16]);

int arch_gap_adv_start(arch_adv_data_t type, uint8_t * data, uint16_t data_len);
int arch_gap_adv_stop(void);
int arch_gap_scan_start(void);
int arch_gap_scan_stop(void);
int arch_gap_connect(mible_addr_t address);
int arch_gap_cancel_connection(void);
int arch_gap_disconnect(uint16_t conn_handle);

int arch_gatts_init(arch_gatt_t * p_gatt);
int arch_gatts_value_set(uint16_t handle, void * buffer, uint16_t length);
int arch_gatts_value_notify(uint16_t conn_handle, uint16_t handle,
                                        void * buffer, uint16_t length);

int arch_gattc_search_service(uint16_t conn_handle);
int arch_gattc_get_service(uint16_t conn_handle, arch_gatt_t * p_gatt);
int arch_gattc_value_write(uint16_t conn_handle, uint16_t handle,
                                        void * buffer, uint16_t length);
int arch_gattc_value_write_no_rsp(uint16_t conn_handle, uint16_t handle,
                                        void * buffer, uint16_t length);
int arch_gattc_value_read(uint16_t conn_handle, uint16_t handle);
int arch_gattc_notify_enable(uint16_t conn_handle, uint16_t handle);
int arch_gattc_notify_enable_no_rsp(uint16_t conn_handle, uint16_t handle);

#endif
