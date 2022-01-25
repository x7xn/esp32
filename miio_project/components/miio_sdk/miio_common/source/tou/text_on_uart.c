
#include "text_on_uart.h"
#include "miio_arch.h"

#undef MIIO_LOG_TAG
#define MIIO_LOG_TAG			"tou"

#define MSG_BUF_LEN 			(800)
#define END_CHAR 				'\r'

typedef struct tou_struct {
	void * sobj;
    int rx_byte_count;
    bool echo;
    bool quit_pending;
    int uart_num;
}*tou;


void* tou_create(void *arg)
{

    tou self = malloc(sizeof(struct tou_struct));
    if(!self)
        return NULL;

    self->uart_num = (int)arg;
	self->rx_byte_count = 0;
	self->echo = false;
	self->quit_pending = false;

    serial_open(self->uart_num);

    return (void*)self;
}


int tou_recv_byte(void *handle, unsigned char *c, int timeout_ms)
{
	tou self = (tou)handle;
	return serial_read(self->uart_num, c, 1, timeout_ms);
}


void tou_send_byte(void *handle, unsigned char c)
{
	tou self = (tou)handle;

	serial_write(self->uart_num, (unsigned char*)&c, 1);
}


int tou_send_without_end_char(void *handle, const char * str)
{
	tou self = (tou)handle;

    int n = strlen(str);

    serial_write (self->uart_num, (const unsigned char*)str, n);

    return n;
}


int tou_pend_til_recv(void *handle, char * buf, int buf_size, int idle_timeout_ms)
{
	if(buf_size <= 0){
		return 0;
	}

	tou self = (tou)handle;

    if(self->echo)
        tou_send_without_end_char(self, "\n# ");

    self->rx_byte_count = 0;

    int timeout_ms = 0;

    do{
    	unsigned char* read_buf = (unsigned char *)(buf+self->rx_byte_count);
    	int read_buf_size = MIN(MIIO_PER_RW_TIMEOUT_BYTES_MAX, buf_size-self->rx_byte_count);

        int read_bytes = serial_read(self->uart_num, read_buf, read_buf_size, MIIO_PER_RW_TIMEOUT_MS_MAX);

        if(self->echo || read_bytes > 0){
        	timeout_ms = 0;
        }
        else{
        	timeout_ms += MIIO_PER_RW_TIMEOUT_MS_MAX;
        	if(timeout_ms >= idle_timeout_ms){
        		self->rx_byte_count = 0;
        		goto recv_exit;
        	}
        }

        for(int i=0; i < read_bytes; i++) {
        	char c = read_buf[i];
            if (END_CHAR == c) {
            	if(self->echo) {
            		tou_send_without_end_char(self, "\r\n");
            	}
                goto recv_exit;
            }
            else if(0x08 == c || 0x7f == c) {
                if(self->rx_byte_count > 0) {
                	self->rx_byte_count--;
					if(self->echo){
						char delete_show[4] = {0x08,' ',0x08,'\0'};
						tou_send_without_end_char(self, delete_show);
					}
				}

            	read_bytes--;
				for(int j=i; j<read_bytes; j++){
					read_buf[j] = read_buf[j+1];
				}
				read_buf[read_bytes] = '\0';
				i--;
            }
            else {
                if(c > 31 && c < 127 && !(self->rx_byte_count == 0 && c == ' ')){
                	if(self->echo){
						tou_send_byte(self, c);
					}
                    self->rx_byte_count++;
                    if (self->rx_byte_count >= buf_size) {
						self->rx_byte_count = 0;
						goto recv_exit;
					}
                }
                else{
                	read_bytes--;
					for(int j=i; j<read_bytes; j++){
						read_buf[j] = read_buf[j+1];
					}
					read_buf[read_bytes] = '\0';
					i--;
                }
            }
        }
    } while(!self->quit_pending);

    if(self->quit_pending){
    	self->rx_byte_count = 0;
    }

recv_exit:

    buf[self->rx_byte_count] = '\0';

    if(self->rx_byte_count){
    	LOG_DEBUG_TAG(MIIO_LOG_TAG, "recv=%s", buf);
    }

    return self->rx_byte_count;
}

void tou_quit_pending(void *handle)
{
	tou self = (tou)handle;
	if(self)
		self->quit_pending = true;
}

void tou_enable_pending(void *handle){
	tou self = (tou)handle;
	if(self)
		self->quit_pending = false;
}


int tou_send(void *handle, const char *str)
{
	tou self = (tou)handle;

    int n = strlen(str);

    if(n){

		serial_write(self->uart_num, (const unsigned char*)str, n);

		if(self->echo){
			unsigned char c[2] = {'\r','\n'};
			serial_write(self->uart_num, c, 2);
		}
		else{
			unsigned char c = END_CHAR;
			serial_write(self->uart_num, &c, 1);
		}

		LOG_DEBUG_TAG(MIIO_LOG_TAG, "send=%s", str);
    }

    return n;
}


int tou_set_echo(void *handle, const char* arg)
{
	tou self = (tou)handle;
	if(strcmp(arg, "on") == 0) {
		self->echo = true;
		return 0;
	} else if(strcmp(arg,"off") == 0) {
		self->echo = false;
		return 0;
	}

	return -1;
}



void tou_destroy(void **handle)
{
    if(!handle || !(*handle))
        return;

    tou self = (tou)(*handle);

    serial_close(self->uart_num);

    free(*handle);
    *handle = NULL;
}


