/*
 * miio_net.h
 *
 *  Created on: Nov 13, 2017
 *      Author: mashaoze
 */

#ifndef _MIIO_NET_H_
#define _MIIO_NET_H_

#include "miio_instance.h"
#include "miio_wifi_channel.h"
#include "miio_arch.h"
#include "list.h"

#define MIIO_NET_WIFI_AP_REPEATER_NUMS				4
#define MIIO_NET_WIFI_SCAN_TIME_MS_MIN				120
#define MIIO_NET_WIFI_SCAN_TIME_MS_MAX				(MIIO_NET_WIFI_SCAN_TIME_MS_MIN*5)
#define MIIO_NET_WIFI_SCAN_TIME_MS(ms) 				(((ms)<MIIO_NET_WIFI_SCAN_TIME_MS_MIN)?MIIO_NET_WIFI_SCAN_TIME_MS_MIN:((ms)>MIIO_NET_WIFI_SCAN_TIME_MS_MAX?MIIO_NET_WIFI_SCAN_TIME_MS_MAX:(ms)))

#define MIIO_NET_MONITOR_OFFLINE_COUNT_MAX(ms)		(((ms)/MIIO_OT_KPLV_TIMEOUT_MS_MIN)/2)

typedef enum{
	MIIO_NET_UNPROV = 0,
	MIIO_NET_UAP,
	MIIO_NET_DISCONNECTED,
	MIIO_NET_LOCAL,
	MIIO_NET_CLOUD,
	MIIO_NET_UPDATING,
}miio_net_state_t;

typedef enum{
	MIIO_NET_ERROR_NONE = 0,
	MIIO_NET_ERROR_AUTH_FAIL,
	MIIO_NET_ERROR_UNKNOW
}miio_net_error_t;

typedef struct{
	int (*fp)(miio_net_state_t state, miio_net_error_t error, void *ctx);
	void *ctx;
}miio_net_state_callback_t;

typedef enum {
    MIIO_WIFI_EVENT_CONFIG_ROUTER = 0,
    MIIO_WIFI_EVENT_SCAN_DONE,
    MIIO_WIFI_EVENT_STA_START,
    MIIO_WIFI_EVENT_STA_STOP,
    MIIO_WIFI_EVENT_STA_CONNECTED,
    MIIO_WIFI_EVENT_STA_DISCONNECTED,
    MIIO_WIFI_EVENT_STA_GOT_IP,
	MIIO_WIFI_EVENT_STA_LOST_IP
}miio_wifi_event_type_t;

typedef struct{
	miio_wifi_event_type_t type;
}miio_wifi_event_t;

typedef enum{
	MIIO_WIFI_STA_IDLE = 0,
	MIIO_WIFI_STA_SCANNING,
	MIIO_WIFI_STA_SCANNED,
	MIIO_WIFI_STA_CONNECTING,
	MIIO_WIFI_STA_CONNECTED,
	MIIO_WIFI_STA_GOT_IP
}miio_wifi_status_t;

typedef struct{
	const char *ssid;
	uint8_t bssid[6];
	uint8_t primary;
	uint8_t second;
	int8_t  rssi;
	uint32_t authmode;
	uint32_t pairwise_cipher;
	uint32_t group_ciper;
	uint32_t phy_mode;
}miio_ap_info_t;

typedef struct {
	list_head_t list;
	miio_ap_info_t ap_info;
	bool active;
}miio_ap_info_entry_t;
struct miio_auto_provision;


#if MIIO_PROV_STATISTIC

#define	PROV_TYPE_AP  		1
#define	PROV_TYPE_AP_NORMAL 2
#define	PROV_TYPE_AP_SAFE 	3
#define	PROV_TYPE_AUTO 		4

#define MAX_PROV_SCAN_TIMES 3
#define MAX_PROV_PASSWD_ERROR_TIMES 3

typedef struct prov_stat_one{   
    uint8_t times;	/*provision times*/
	uint8_t prov_type;  /*provision type*/
	uint8_t prov_error_code; /*provision reason code  0: success, <0, fail*/
}prov_stat_one_t;

typedef struct miio_prov_statistic{
    bool enable;
	uint32_t provision_start_time;
	uint8_t prov_stage;
	prov_stat_one_t prov_stat_cur;
	prov_stat_one_t prov_stat_tmp;
	prov_stat_one_t prov_stat_pre;
}miio_prov_statistic_t;

#endif

typedef struct miio_net{
	miio_handle_t miio_handle;
	bool factory_status;
	uint8_t mac[6];
	char host_name[33];
    bool provision_status;
	char ssid[IEEEtypes_SSID_SIZE+1];
	char password[WLAN_PSK_MAX_LENGTH+1];
	miio_wifi_channel_info_t wifi_channel_info;
	struct{
		uint32_t scan_time;
		uint8_t ichan;
		list_head_t ap_info_list;
		uint32_t repeater_count;
		uint32_t connect_count;
		miio_ap_info_t current_ap_info;
		miio_wifi_status_t sta_status;
	}wifi_connect_ctx;
	char config_type[16];
	struct{
		char bind_key[65];
		int disable_local_restore;
	}tob;
	struct{
		bool net_status_check;
		arch_os_thread_handle_t thread;
		arch_os_queue_handle_t  mq;
		uint32_t refresh_ms;
		uint32_t work_time;
		uint32_t info_time;
		uint32_t heap_size;
		uint32_t ap_time;
		uint32_t smart_config_time;
		uint32_t offline_time;
		uint32_t offline_timeout;
		uint32_t offline_count;
		uint32_t offline_count_time;
		uint32_t offline_count_stamp[10];
		uint32_t offline_count_out;
#if	MIIO_MONITOR_REBOOT_ENABLE
		uint32_t reboot_time;
#endif
#if MIIO_STATASTIC_ENABLE
		struct{
			uint32_t idle;
			uint32_t local;
			uint32_t cloud;
		}miio_times;
#endif
		uint32_t ntp_time;
		bool ntp_successed;
		bool ntp_ack_cb;  /* 1: ack callback is invoked, 0: ack callback not invoked yet */
	}monitor;
	const miio_net_state_callback_t *callbacks;
	size_t callbacks_count;
	arch_os_mutex_handle_t net_mutex;
	bool net_started;
	bool smart_config_enable;
	/**
	 * @brief auto provision
	 * To support the component of 'auto provision', Function pointers listed below should be assigned correctly
	 * wifi_scan_start
	 * wifi_scan_finish
	 * wifi_if_has_next
	 * wifi_connect
	 * wifi_disconnect
	 *
	 * @param auto_provision_enable represents whether the component of 'auto provision' would be used in the implementation of miio_net
	 * @param auto_provision represents the handle of the component
	 */
	bool auto_provision_enable;
	struct miio_auto_provision *auto_provision;
#if MIIO_PROV_STATISTIC
	miio_prov_statistic_t *prov_statistic;
#endif
	int (*wifi_scan_start)( struct miio_net *miio_net );
	int (*wifi_scan_finish)( struct miio_net *miio_net );
	int (*wifi_if_has_next)( struct miio_net *miio_net );
	int (*wifi_connect)( struct miio_net *miio_net );
	void (*wifi_disconnect)( struct miio_net *miio_net);
}miio_net_t;

int miio_net_init(miio_handle_t miio_handle, const miio_net_state_callback_t callbacks[], size_t callbacks_count);
void miio_net_set_ap(const char* ssid, const char* password, bool in_ram);
miio_net_state_t miio_net_get_state(void);
int miio_net_start(void);
int miio_net_start_async(int delay_ms);
int miio_net_restart(void);
int miio_net_restart_async(int delay_ms);
int miio_net_stop(void);
void miio_net_factory(void);
int miio_net_get_disable_local_restore(void);
int miio_net_set_disable_local_restore(int disable_local_restore);
void miio_net_set_wifi_channel(const char* cc, int priority);
int miio_net_get_wifi_channel(char *cc, size_t cc_size, int priority);
int miio_net_set_bindkey(const char *bindkey);
int miio_net_set_config_type(const char *config_type);
/* enable or disable smart config according to country code when migration */
/* SUGGESTION: use only in do_migration*/
int miio_net_switch_smart_config_onoff_in_migration(void);
int miio_net_config_router(miio_handle_t miio_handle, const char* params, size_t params_len, const char* ext, size_t ext_size);
#if MIIO_PROV_STATISTIC
bool is_prov_stat_enable();
uint8_t get_prov_stat_type(void);
void miio_net_clear_provision_stat(void);
void set_prov_start_time(void);
void increase_prov_times(void);
void set_prov_stat_stage(miio_prov_rpc_stage_t prov_stage);
void set_prov_stat_error_code(miio_prov_error_code_t error_code);
int prov_stat_error_code_process();
#endif

#if MIIO_KEY_EXCHANGE_ENABLE
int miio_net_set_bindkey_index(miio_handle_t miio_handle, const char *bindkey_index);
void miio_net_set_bind_key_ts(uint64_t utc);
#endif /* MIIO_KEY_EXCHANGE_ENABLE */

#endif
