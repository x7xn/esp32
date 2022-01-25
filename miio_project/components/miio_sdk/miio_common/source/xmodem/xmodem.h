#ifndef __XMODEM_H__
#define __XMODEM_H__


#define XMODEM_MTU 				(128)

typedef void (*fp_port_outbyte)(void* handle, unsigned char c);
typedef int  (*fp_port_inbyte)(void* handle, unsigned char *c, int timeout_ms);

typedef struct _xmodem{
	int last_error;
	struct {
	    unsigned char packetno;
	}packetno_wrap;
	struct{
		void *handle;
		fp_port_outbyte out;
		fp_port_inbyte in;
	}io;
	unsigned char xbuff[1030]; /* 128 data  + 3 head chars + 2 crc + nul */
}*xmodem;

xmodem xmodem_create(void *io_handle, fp_port_outbyte io_out, fp_port_inbyte io_in);
int xmodem_get_crc_config(xmodem x, int *crc_ret);
int xmodem_transfer_data(xmodem x, int crc,unsigned char *src, int srcsz,int xmodem_mtu);
int xmodem_transfer_end(xmodem x, int crc ,unsigned char *src, int srcsz,int xmodem_mtu);
void cancel_xmodem_transfer(xmodem x);
void xmodem_destroy(xmodem *x);



#endif

