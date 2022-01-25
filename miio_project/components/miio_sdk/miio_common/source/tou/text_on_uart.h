#ifndef TEXT_ON_UART_H_
#define TEXT_ON_UART_H_


void *tou_create(void *arg);
void tou_destroy(void **handle);
int tou_pend_til_recv(void *handle, char * buf, int buf_size, int idle_timeout_ms);
void tou_quit_pending(void *handle);
int tou_send(void *handle, const char * str);
void tou_send_byte(void *handle, unsigned char c);
int tou_recv_byte(void *handle, unsigned char *c, int timeout_ms);
int tou_set_echo(void *handle, const char *arg);

#endif /* TEXT_ON_UART_H_ */
