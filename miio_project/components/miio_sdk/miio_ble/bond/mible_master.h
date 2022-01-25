/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BOND_MASTER_H__
#define __BOND_MASTER_H__

/* Exported functions --------------------------------------------------------*/

int mible_master_init(void);
int mible_master_connect(uint16_t conn_handle, uint16_t product_id);
int mible_master_disconnect(uint16_t conn_handle);
int mible_master_found(uint16_t conn_handle);
int mible_master_write(uint16_t conn_handle, uint16_t handle);
int mible_master_notify(uint16_t conn_handle, uint16_t handle,
                                               void * buffer, uint16_t length);
int mible_master_read(uint16_t conn_handle, uint16_t handle,
                                               void * buffer, uint16_t length);
uint32_t master_opcode_send(uint16_t conn_handle, uint32_t status);

#endif
