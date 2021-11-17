#ifndef _DEVICE_UART_H
#define _DEVICE_UART_H





void device_uart_init(void);
void device_uart_send_data(unsigned int send_len,unsigned char* send_data);
void uart1_rx_task_perform(void);
void uart1_tx_task_handle(void);
uint32_t get_device_uart_communication_time(void);

#endif
