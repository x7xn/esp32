/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BOND_SLAVE_H__
#define __BOND_SLAVE_H__

/* Includes ------------------------------------------------------------------*/

int mible_slave_init(void);
int mible_slave_deinit(void);
int mible_slave_connect(uint16_t conn_handle);
int mible_slave_disconnect(void);
int mible_slave_write(uint16_t handle, uint8_t *buffer, uint16_t size);
int mible_wifi_report(uint8_t state);
uint32_t opcode_send(uint32_t status);

#endif
