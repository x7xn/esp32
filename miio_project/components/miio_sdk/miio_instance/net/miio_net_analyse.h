/*
 * miio_net_analyse.h
 *
 *  Created on: Dec 17, 2019
 *      Author: meijian
 */

#ifndef _MIIO_NET_ANALYSE_H_
#define _MIIO_NET_ANALYSE_H_

#include <stdbool.h>
#include "miio_net.h"

#define MAGIC "miio_client"
#define MAGIC_LEN 12
#define MTU_LEN 1500

#define DNS_CHECK_OK							(0) /* there is no probrem */
#define DNS_CHECK_FAKE							(1) /* dns hijack  */

#define PING_OK									(1)
#define SOCKET_ERRO								(-1)
#define PING_NO_REPLY							(-2)
#define IP_ERROR								(-3) /* a generic error happens */
#define NO_PING_PRIVILEGE						(-1000)

struct icmp_echo {
	// header
	uint8_t type;
	uint8_t code;
	uint16_t checksum;

	uint16_t ident;
	uint16_t seq;

	// data
	uint64_t sending_ts;
	char magic[MAGIC_LEN];
};

typedef enum net_status {
	NET_STATUS_UNKOWN=0,
	NET_STATUS_LOCAL_IP_GET_FAIL = 5,		/* can not get ip */
	NET_STATUS_LOCAL_NETWORK_FAIL = 10,		/* can not ping gateway */
	NET_STATUS_DNS_GET_FAIL = 15,			/* dns get fail */
	NET_STATUS_PUBLIC_NETWORK_FAIL = 20,	/* can not ntp miio */
	NET_STATUS_SERVER_IP_NOK = 30,			/* ntp ok */
	NET_STATUS_STA_DISCONNECTED = 40,
	NET_STATUS_NETWORK_FAIL = 50,			/* ntp not sent, lack of ping test results in incapability to tell local network fail from public network fail */
} net_status_e;

typedef struct {
	net_status_e reason;
    int sta_disconnect_reason;
	uint32_t start_time;
	char dev_ip[16];
	char gateway[16];
	char netmask[16];
	char rssi[10];
} miio_offline_info_t;

typedef struct {
	arch_os_mutex_handle_t offline_mutex;
	bool report_enable;
	bool analyse_start;
	miio_offline_info_t info;
} miio_offline_t;

int miio_offline_init();
void miio_offline_open();
void miio_offline_close();
void miio_offline_record(miio_net_t *miio_net, net_status_e reason, int sta_disconnect_reason);
void miio_offline_report(miio_handle_t handle);
void miio_online_stats_gather(int8_t rssi);

#endif /* _MIIO_NET_ANALYSE_H_ */
