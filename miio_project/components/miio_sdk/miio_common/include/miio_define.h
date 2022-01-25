/**
* @file    miio_define.h
* @author  mashaoze
* @date    2017
* @par     Copyright (c):
*
*    Copyright 2017 MIoT,MI
*
*    Licensed under the Apache License, Version 2.0 (the "License");
*    you may not use this file except in compliance with the License.
*    You may obtain a copy of the License at
*
*        http://www.apache.org/licenses/LICENSE-2.0
*
*    Unless required by applicable law or agreed to in writing, software
*    distributed under the License is distributed on an "AS IS" BASIS,
*    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*    See the License for the specific language governing permissions and
*    limitations under the License.
*/
#ifndef __MIIO_DEFINE_H__
#define __MIIO_DEFINE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "miio_version.h"
#include "miio_config.h"

#ifndef __cplusplus

#ifndef BOOL
#define BOOL            	bool
#endif

#ifndef TRUE
#define TRUE            	true
#endif

#ifndef FALSE
#define FALSE           	false
#endif

#endif

#ifndef NULL
#define NULL 				((void *)0)
#endif

#define MIIO_BASE64_SIZE(len)		(((len)+2)/3*4+1)

// ------------ADDON AND RPC----------------------
typedef struct miio_addon_symbol_s {
	const char* name;
	const char* full_name;
	void* value;
	const char* tip;
#if MIIO_RPC_ACL_ENABLE
	/**
	 * @see Access Mask https://xiaomi.f.mioffice.cn/docs/dock4Xs4lHT44ektEIeARTmJM7d 
	 */
	const uint8_t acm;
#endif
}miio_addon_symbol_t;


#define _miio_addon_entry_declare(_addon, _name)				\
	miio_addon_symbol_t _miio_addon_list_2_##_addon##_2_##_name	\
		__attribute__((aligned(4)))								\
		__attribute__((unused, section(".miio_addon_list_2_"#_addon"_2_"#_name)))
#define miio_addon_entry_declare(_addon, _name)			_miio_addon_entry_declare(_addon, _name)

#define _miio_addon_entry_get(_addon, _name)					\
({																\
	extern miio_addon_symbol_t _miio_addon_list_2_##_addon##_2_##_name;	\
	miio_addon_symbol_t *_entry_result =						\
		&_miio_addon_list_2_##_addon##_2_##_name;				\
	_entry_result;												\
})
#define miio_addon_entry_get(_addon, _name)				_miio_addon_entry_get(_addon, _name)

#define _miio_addon_entry_start(_addon)							\
({																\
	static char start[0] 										\
		__attribute__((aligned(4)))								\
		__attribute__((unused, section(".miio_addon_list_2_"#_addon"_1")));	\
	(miio_addon_symbol_t *)&start;								\
})
#define miio_addon_entry_start(_addon)					_miio_addon_entry_start(_addon)

#define _miio_addon_entry_end(_addon)							\
({																\
	static char end[0] 											\
		__attribute__((aligned(4)))								\
		__attribute__((unused, section(".miio_addon_list_2_"#_addon"_3")));	\
	(miio_addon_symbol_t *)&end;								\
})
#define miio_addon_entry_end(_addon)						_miio_addon_entry_end(_addon)

#define miio_addon_entry_count(_addon)							\
({																\
	miio_addon_symbol_t *start = miio_addon_entry_start(_addon);\
	miio_addon_symbol_t *end = miio_addon_entry_end(_addon);	\
	unsigned int _miio_addon_result = end - start;				\
	_miio_addon_result;											\
})
#if MIIO_RPC_ACL_ENABLE
#define __miio_addon_acm_assignment(_acm) , .acm = (_acm) & 0B11
#else
#define __miio_addon_acm_assignment(_acm)
#endif

#define _miio_addon_value_complete(_addon, _name, _value, _tip, _acm){.name = #_name, .full_name = #_addon"."#_name, .value = _value, .tip = _tip __miio_addon_acm_assignment(_acm)}
#define miio_addon_value_complete(_addon, _name, _value, _tip, _acm)	_miio_addon_value_complete(_addon, _name, _value, _tip, _acm)

#define miio_addon_entry_complete(_addon, _name, _value, _tip, _acm)	\
	miio_addon_entry_declare(_addon, _name) = miio_addon_value_complete(_addon, _name, _value, _tip, _acm)

#define _STRING(name)											#name
#define MIIO_ADDON_NAME_STR(name)								_STRING(name)
#define MIIO_RPC_ADDON_NAME										miIO
#define MIIO_RPC_USER_ADDON_NAME								user
/**
 * example:
 * >1. MIIO_RPC_ACL(hello, do_hello, NULL, ( MIIO_RPC_ACM_NONE ) )
 * 		This macro defines an rpc named "hello", regardless of whether 
 * 		the target device is in the "PROVISIONED" state, the RPC can be
 * 		executed by any controller
 * 
 * >2. MIIO_RPC_ACL(hello, do_hello, NULL, ( MIIO_RPC_ACM_PROVISIONED ) )
 * 		This macro defined a rpc named 'hello' which can be executed 
 * 		by any controller in the constraintly condition of the target 
 * 		device is in the state of 'PROVISIONED'
 * 
 * >3. MIIO_RPC_ACL(hello, do_hello, NULL, ( MIIO_RPC_ACM_PROVISIONED & MIIO_RPC_ACM_PSK ) )
 * 		This macro defines an rpc named "hello" which can be executed 
 * 		by the controller identified by psk in the constraintly condition
 * 		of the device is in the state of 'PROVISIONED'
 * 
 * >4. MIIO_RPC(hello, do_hello, NULL)
 * 		This macro is equal to 'MIIO_RPC_ACL(hello, do_hello, NULL, ( MIIO_RPC_ACM_PROVISIONED)  )'
 */
#define MIIO_RPC_ACM_NONE			( 0B11 )
#define MIIO_RPC_ACM_PROVISIONED 	( 0B10 )
#define MIIO_RPC_ACM_PSK 			( 0B01 )
#define MIIO_RPC_ACM_TEST(acl, acm)	( ( ( (acl) | (acm) ) & 0B11 ) == 0B11)

#define MIIO_RPC(_name, _rpc, _tip)								miio_addon_entry_complete(MIIO_RPC_ADDON_NAME, _name, _rpc, _tip, ( MIIO_RPC_ACM_PROVISIONED) )
#define MIIO_RPC_ACM(_name, _rpc, _tip, _acm)					miio_addon_entry_complete(MIIO_RPC_ADDON_NAME, _name, _rpc, _tip, ( _acm ) )
#define MIIO_RPC_USER(_name, _rpc, _tip)						miio_addon_entry_complete(MIIO_RPC_USER_ADDON_NAME, _name, _rpc, _tip, ( MIIO_RPC_ACM_PROVISIONED ) )
#define MIIO_RPC_USER_ACM(_name, _rpc, _tip, _acm)				miio_addon_entry_complete(MIIO_RPC_USER_ADDON_NAME, _name, _rpc, _tip, ( _acm ) )

// ---------------- CONST MACRO -----------------

#define MIIO_OK							0					/* There is no error 						*/
#define MIIO_ERROR						(-1)				/* A generic error happens 					*/
#define MIIO_ERROR_TIMEOUT				(-2)				/* Timed out 								*/
#define MIIO_ERROR_FULL					(-3)				/* The resource is full						*/
#define MIIO_ERROR_EMPTY				(-4)				/* The resource is empty 					*/
#define MIIO_ERROR_NOMEM				(-5)				/* No memory								*/
#define MIIO_ERROR_NOSYS				(-6)				/* No system 								*/
#define MIIO_ERROR_BUSY					(-7)				/* Busy										*/
#define MIIO_ERROR_TRYOUT				(-8)				/* try enough times							*/
#define MIIO_ERROR_NOTFOUND				(-9)
#define MIIO_ERROR_PARAM				(-10)
#define MIIO_ERROR_SIZE					(-11)
#define MIIO_ERROR_NOTREADY				(-12)

#define MIIO_ITERATOR_CONTINUE 			MIIO_OK
#define MIIO_ITERATOR_ABORT 			MIIO_ERROR
typedef int (* miio_fp_iterator_t)(void* item, void* args);

typedef  void*  miio_handle_t;

typedef enum{
	MIIO_DELEGATE_JSON = 0
}miio_delegate_type_t;

#define MIIO_DELEGATE_ATUH_NONE			(0)
#define MIIO_DELEGATE_AUTH_BIT(x)		(1<<(x))
#define MIIO_DELEGATE_AUTH_BIT_LOCAL	MIIO_DELEGATE_AUTH_BIT(0)
#define MIIO_DELEGATE_AUTH_BIT_CLOUD	MIIO_DELEGATE_AUTH_BIT(1)

typedef struct{
	unsigned int id;
	unsigned int timestamp; //UTC time
	unsigned int tick; // ms
	miio_delegate_type_t type;
	int auth;
	size_t pload_len;
	void *pload;
	uint8_t authcode;
}miio_rpc_delegate_arg_t;

typedef int (*miio_fp_rpc_delegate_ack_t)(miio_rpc_delegate_arg_t* arg, void* ctx);

typedef struct{
	int timeout;
	miio_rpc_delegate_arg_t arg;
	miio_fp_rpc_delegate_ack_t ack;
	void* ack_ctx;
}miio_rpc_delegate_context_t;

typedef enum{
	MIIO_MODE_IDLE = 0,
	MIIO_MODE_LOCAL,
	MIIO_MODE_CLOUD
}miio_mode_t;

typedef struct{
	void *identity;
	int (*get_did)(void *identity, uint64_t *did);
#if MIIO_KEY_EXCHANGE_ENABLE
    int (*get_new_did)(void *identity, uint64_t *did);
    int (*get_new_psk)(void *identity, uint8_t psk[16]);
#endif
	int (*get_psk)(void *identity, uint8_t psk[16]);
	int (*get_token)(void *identity, uint8_t token[16]);
	int (*update_token)(void *identity, uint8_t token[16]);
	int (*get_random)(void *identity, uint8_t *randoms, size_t randoms_len);
	const unsigned char* (*get_root_cert_pem)(void);
	const unsigned char* (*get_device_cert_pem)(void *identity);
	int (*get_sign_suites)(int sign_suites[], size_t sign_suites_nums);
	int (*get_sign)(void *identity, int sign_suite, void *sign_params, void *data, size_t data_len, uint8_t *sign_out, size_t *sign_out_len, int base64);
}miio_ciphers_interface_t;

#if MIIO_PROV_STATISTIC

#define PROVISION_PSM_STR "provision_stat"

typedef enum{
	PROV_ERROR_NONE = 0,
	PROV_ERROR_UNKNOWN = 1,
	PROV_ERROR_PROBE_LOST = 2,
	PROV_ERROR_ONLY_PROBE_RECEIVED = 3,
	PROV_ERROR_COMMON_PROBE_ACK_FAIL = 4,
	PROV_ERROR_COMMON_RPC_DECRYPT_FAIL = 5,
	PROV_ERROR_CONFIG_ROUTER_PARAM_INVAILD = 6,
	PROV_ERROR_HANDSHAKE_HELLO_FAIL = 7, 
	PROV_ERROR_HANDSHAKE_ECDH_RPC_LOST = 8,
	PROV_ERROR_HANDSHAKE_ECDH_FAIL = 9,
	PROV_ERROR_CONFIG_ROUTER_SAFE_RPC_LOST = 10,
	PROV_ERROR_SAFE_DECRYPT_FAIL = 11,	
	PROV_ERROR_AP_NOT_FOUND = 12,
	PROV_ERROR_PASSWD_ERROR = 13,
	PROV_ERROR_STA_DHCP_FAIL = 14,
	PROV_ERROR_DNS_FAIL = 15,
	PROV_ERROR_OT_CONNECT_FAIL = 16,
	PROV_ERROR_OT_LOGIN_FAIL = 17,
}miio_prov_error_code_t;

typedef enum{
	PROV_STAGE_INIT = 0,
	PROV_STAGE_APSTA_WIFI_CONNECT = 1,  /*auto_provision mode, sta connect to router*/
	PROV_STAGE_AP_WIFI_CONNECT = 2,		/*ap mode, app connect to device ap*/
	PROV_STAGE_PROBE = 3,
	PROV_STAGE_INFO = 4,
	PROV_STAGE_CONFIG_ROUTER = 5,
	PROV_STAGE_HANDSHAKE_HELLO = 6,
	PROV_STAGE_HANDSHAKE_ECDH = 7,
	PROV_STAGE_CONFIG_ROUTER_SAFE = 8,
	PROV_STAGE_DEVICE_WIFI_CONNECT = 9,	/*after config_router, device connect to router*/
	PROV_STAGE_CLOUD_CONNECT_START = 10,
	PROV_STAGE_HTTPDNS = 11,
	PROV_STAGE_LOCALDNS = 12,
	PROV_STAGE_OT_CONNECTED = 13,
	PROV_STAGE_OT_LOGIN = 14,
	PROV_STAGE_ONLINE = 15,
}miio_prov_rpc_stage_t;

#endif

typedef struct miio_configs{
	char model[MIIO_MODEL_SIZE_MAX];
	uint64_t uid;
	char country_domain[MIIO_COUNTRY_DOMAIN_SIZE_MAX];
	int gmt_offset;
	const miio_ciphers_interface_t *ciphers_if;
#if MIIO_AUTO_OTA_ENABLE
	bool app_auto_ota_ability;
	bool mcu_auto_ota_ability;
#endif
}miio_configs_t;


typedef enum{
	MIIO_HTTPC_REQ_TYPE_GET = 0,
	MIIO_HTTPC_REQ_TYPE_POST
}miio_httpc_req_type_t;

typedef enum{
	HTTPC_DNS_FAILED = 1,
	HTTPC_TLS_TCP_OPEN_FAILED,
	HTTPC_TLS_TCP_READ_FAILED,
	HTTPC_TLS_TCP_WRITE_FAILED,
	HTTPC_IF_WRITE_FAILED,
	HTTPC_IF_READ_FAILED,
	HTTPC_IF_CLOSE_FAILED,
	HTTPC_IF_OPEN_FAILED,
	HTTPC_HEAD_CHECK_FAILED,
	HTTPC_BODY_RECEIVING_TIMEOUT,
	HTTPC_HEAD_RECEIVING_TIMEOUT,
	HTTPC_NO_INOUT_INTERFACE,
	HTTPC_UNKNOWN_FAILED
}miio_httpc_err_code_t;

typedef struct {
	const char* url;
	const char* hostname;
	char hostip[64];
	uint16_t port;
	miio_httpc_req_type_t req_type;
	int partial_enable;
	int partial_size;
	const unsigned char* root_cert_pem;
	uint32_t timeout_ms;
}miio_httpc_req_params_t;

typedef struct{
	miio_httpc_err_code_t err_code;
	char hostip[64];
}miio_httpc_statistic_t;

typedef struct{
	const unsigned char* root_cert_pem;
	bool partial_enable;
	char hostip[64];
	char url[MIIO_URL_SIZE_MAX];
}miio_download_params_t;

typedef struct{
	int (*fp_open)(void* ctx, void* arg);					//return negative means error
	int (*fp_read)(void* ctx, uint8_t* out, size_t len);	//return byte num, negative means error
	int (*fp_write)(void* ctx, uint8_t* in, size_t len);	//return negative means error
	int (*fp_close)(void* ctx);								//return negative means error
	int (*fp_error)(void* ctx, void* msg);
	int (*fp_statistics)(void* ctx, void* statistics);
}miio_stream_io_if_t;


typedef void* miio_ota_task_handle_t;

typedef enum{
	MIIO_OTA_TASK_IDLE = 0,
	MIIO_OTA_TASK_WAITING,
	MIIO_OTA_TASK_STARTING,
	MIIO_OTA_TASK_DOWNLOADING,
	MIIO_OTA_TASK_DOWNLOADED,
	MIIO_OTA_TASK_INSTALLING,
	MIIO_OTA_TASK_INSTALLED,
	MIIO_OTA_TASK_FAILED
}miio_ota_task_state_t;

typedef struct{
	int (*fp_init)(void *handle, void* ctx, miio_delegate_type_t params_type, const void* params, size_t params_len);	//return negative means error
	int (*fp_version)(void *handle, void* ctx, miio_delegate_type_t type, void *composer);	//return byte num, negative means error
	int (*fp_open)(void *handle,  void* ctx, size_t content_len);		//return negative means error
	int (*fp_write)(void *handle, void* ctx, uint32_t offset, uint8_t* in, size_t len);	//return byte num, negative means error
	int (*fp_close)(void *handle, void* ctx);							//return negative means error
	int (*fp_error)(void *handle, void* ctx);
	int (*fp_deinit)(void *handle, void* ctx);
}miio_ota_task_hook_if_t;

typedef int (*miio_fp_info_hook_t)(miio_handle_t handle, miio_delegate_type_t type, void *composer, void* ctx);
typedef int (*miio_fp_ext_rpc_hook_t)(miio_handle_t handle, void* ctx);
typedef int (*miio_fp_online_hook_t)(miio_handle_t handle, void* ctx);
typedef int (*miio_fp_offline_hook_t)(miio_handle_t handle, void* ctx);
typedef int (*miio_fp_restore_hook_t)(miio_handle_t handle, void* ctx);
typedef int (*miio_fp_reboot_hook_t)(miio_handle_t handle, void* ctx);
typedef int (*miio_fp_statistics_hook_t)(miio_handle_t handle, miio_delegate_type_t type, void *composer, void* ctx);
typedef int (*miio_fp_ota_task_hook_t)(miio_handle_t handle, const char* ota_task_name, miio_ota_task_state_t ota_state, int ota_progress, void* ctx);
#if MIIO_AUTO_OTA_ENABLE
typedef int (*miio_fp_auto_ota_hook_t)(miio_handle_t handle, int *device_state, void* ctx);
#endif /* MIIO_AUTO_OTA_ENABLE */
#if MIIO_PROV_STATISTIC
typedef int (*miio_fp_provision_stat_hook_t)(miio_handle_t handle, miio_prov_rpc_stage_t prov_stage, miio_prov_error_code_t error_code, void *ctx);
#endif

typedef struct{
	miio_fp_online_hook_t online;
	miio_fp_offline_hook_t offline;
	miio_fp_info_hook_t info;
	miio_fp_ext_rpc_hook_t ext_rpc;
	miio_fp_restore_hook_t restore;
	miio_fp_reboot_hook_t reboot;
	miio_fp_statistics_hook_t statistics;
	miio_fp_ota_task_hook_t ota_task;
#if MIIO_AUTO_OTA_ENABLE
	miio_fp_auto_ota_hook_t auto_ota;
#endif /* MIIO_AUTO_OTA_ENABLE */
#if MIIO_PROV_STATISTIC
	miio_fp_provision_stat_hook_t prov_stat;
#endif
	void *ctx;
}miio_hooks_t;
////////////////////////////////////////////
//sys error code:	-32767 < x < -30000
////////////////////////////////////////////
#define MIIO_OT_ERR_CODE_MIN				(-32767)
#define MIIO_OT_ERR_CODE_MAX				(-30000)
#define MIIO_OT_ERR_CODE_R(e)				(MIIO_OT_ERR_CODE_MIN+abs(e))
#define MIIO_OT_ERR_CODE_L(e)				(MIIO_OT_ERR_CODE_MAX-abs(e))
#define MIIO_OT_ERR_CODE_FILTER(e)			(((e) > MIIO_OT_ERR_CODE_MIN && (e) < MIIO_OT_ERR_CODE_MAX) ? (e) : MIIO_OT_ERR_CODE_MAX)


#define MIIO_OT_ERR_INFO_UNDEF_ERROR			"sys undefined error."
#define MIIO_OT_ERR_CODE_UNDEF_ERROR			MIIO_OT_ERR_CODE_L(0)

#define MIIO_OT_ERR_INFO_RESP_INVALID			"resp invalid."
#define MIIO_OT_ERR_CODE_RESP_INVALID			MIIO_OT_ERR_CODE_L(1)

#define MIIO_OT_ERR_INFO_TRYOUT					"try out."
#define MIIO_OT_ERR_CODE_TRYOUT					MIIO_OT_ERR_CODE_L(11)

#define MIIO_OT_ERR_INFO_BUSY					"busy."
#define MIIO_OT_ERR_CODE_BUSY					MIIO_OT_ERR_CODE_L(12)

#define MIIO_OT_ERR_INFO_OFFLINE				"offline."
#define MIIO_OT_ERR_CODE_OFFLINE				MIIO_OT_ERR_CODE_L(13)

#define MIIO_OT_ERR_INFO_NOT_SYNC				"not sync."
#define MIIO_OT_ERR_CODE_NOT_SYNC				MIIO_OT_ERR_CODE_L(14)

#define MIIO_OT_ERR_INFO_SERVICE_NOT_AVAILABLE	"service not available."
#define MIIO_OT_ERR_CODE_SERVICE_NOT_AVAILABLE	MIIO_OT_ERR_CODE_L(20)

#define MIIO_OT_ERR_INFO_REQ_ERROR				"req error."
#define MIIO_OT_ERR_CODE_REQ_ERROR				MIIO_OT_ERR_CODE_L(2600)

#define MIIO_OT_ERR_INFO_METHOD_INVALID			"method not found."
#define MIIO_OT_ERR_CODE_METHOD_INVALID			MIIO_OT_ERR_CODE_L(2601)

#define MIIO_OT_ERR_INFO_PARAM_INVALID			"Invalid param."
#define MIIO_OT_ERR_CODE_PARAM_INVALID			MIIO_OT_ERR_CODE_L(2602)

#if MIIO_AUTO_OTA_ENABLE
////////////////////////////////////////////
// auto ota state code
////////////////////////////////////////////
#define MIIO_AUTO_OTA_READY					(0)
#define MIIO_AUTO_OTA_BUSY					(1)
#endif /* MIIO_AUTO_OTA_ENABLE */

////////////////////////////////////////////
//ota error code:	-33100 < x < -33000
////////////////////////////////////////////
#define MIIO_OTA_ERR_CODE_MIN				(-33100)
#define MIIO_OTA_ERR_CODE_MAX				(-33000)
#define MIIO_OTA_ERR_CODE_R(e)				(MIIO_OTA_ERR_CODE_MIN+abs(e))
#define MIIO_OTA_ERR_CODE_L(e)				(MIIO_OTA_ERR_CODE_MAX-abs(e))
#define MIIO_OTA_ERR_CODE_FILTER(e)			(((e) > MIIO_OTA_ERR_CODE_MIN && (e) < MIIO_OTA_ERR_CODE_MAX) ? (e) : MIIO_OTA_ERR_CODE_MAX)
#define MIIO_OTA_ERR_CODE_CHECK(e)			((e) > MIIO_OTA_ERR_CODE_MIN && (e) < MIIO_OTA_ERR_CODE_MAX)

#define MIIO_OTA_ERR_INFO_DOWN				"down error"
#define MIIO_OTA_ERR_CODE_DOWN				MIIO_OTA_ERR_CODE_L(1)

#define MIIO_OTA_ERR_INFO_DNS				"dns error"
#define MIIO_OTA_ERR_CODE_DNS				MIIO_OTA_ERR_CODE_L(2)

#define MIIO_OTA_ERR_INFO_CONNECT			"connect error"
#define MIIO_OTA_ERR_CODE_CONNECT			MIIO_OTA_ERR_CODE_L(3)

#define MIIO_OTA_ERR_INFO_DISCONNECT		"disconnect"
#define MIIO_OTA_ERR_CODE_DISCONNECT		MIIO_OTA_ERR_CODE_L(4)

#define MIIO_OTA_ERR_INFO_INSTALL			"install error"
#define MIIO_OTA_ERR_CODE_INSTALL			MIIO_OTA_ERR_CODE_L(5)

#define MIIO_OTA_ERR_INFO_CANCEL			"cancel"
#define MIIO_OTA_ERR_CODE_CANCEL			MIIO_OTA_ERR_CODE_L(6)

#define MIIO_OTA_ERR_INFO_LOW_ENERGY		"low energy"
#define MIIO_OTA_ERR_CODE_LOW_ENERGY		MIIO_OTA_ERR_CODE_L(7)

#if MIIO_AUTO_OTA_ENABLE
#define MIIO_OTA_ERR_INFO_INSTALL_BUSY		"install busy"
#define MIIO_OTA_ERR_CODE_INSTALL_BUSY		MIIO_OTA_ERR_CODE_L(8)
#endif /* MIIO_AUTO_OTA_ENABLE */

#if MIIO_OTA_STATISTIC_ENABLE
#define MIIO_OTA_STATISTIC_CODE				20
#define MIIO_OTA_ERR_INFO_UNKNOWN			"unknown"
#define MIIO_OTA_ERR_CODE_UNKNOWN			MIIO_OTA_ERR_CODE_L(MIIO_OTA_STATISTIC_CODE)

#define MIIO_OTA_ERR_INFO_HTTP_HEAD			"http head error"
#define MIIO_OTA_ERR_CODE_HTTP_HEAD			MIIO_OTA_ERR_CODE_L(MIIO_OTA_STATISTIC_CODE+1)

#define MIIO_OTA_ERR_INFO_DOWN_INIT			"down init error"
#define MIIO_OTA_ERR_CODE_DOWN_INIT			MIIO_OTA_ERR_CODE_L(MIIO_OTA_STATISTIC_CODE+2)

#define MIIO_OTA_ERR_INFO_DOWN_TIMEOUT		"down timeout"
#define MIIO_OTA_ERR_CODE_DOWN_TIMEOUT		MIIO_OTA_ERR_CODE_L(MIIO_OTA_STATISTIC_CODE+3)

#define MIIO_OTA_ERR_INFO_DOWN_WRITE		"down write error"
#define MIIO_OTA_ERR_CODE_DOWN_WRITE		MIIO_OTA_ERR_CODE_L(MIIO_OTA_STATISTIC_CODE+4)

#define MIIO_OTA_ERR_INFO_DOWN_VERIFY		"down verify error"
#define MIIO_OTA_ERR_CODE_DOWN_VERIFY		MIIO_OTA_ERR_CODE_L(MIIO_OTA_STATISTIC_CODE+5)
#endif

////////////////////////////////////////////
//user error code:  -10000 <= x <= -5000
////////////////////////////////////////////
#define MIIO_USER_ERR_CODE_MIN					(-10000)
#define MIIO_USER_ERR_CODE_MAX					(-5000)
#define MIIO_USER_ERR_CODE_R(e)					(MIIO_USER_ERR_CODE_MIN+abs(e))
#define MIIO_USER_ERR_CODE_L(e)					(MIIO_USER_ERR_CODE_MAX-abs(e))

#define MIIO_USER_ERR_CODE_FILTER(e)			(((e) >= MIIO_USER_ERR_CODE_MIN && (e) <= MIIO_USER_ERR_CODE_MAX) ? (e) : MIIO_USER_ERR_CODE_MIN)

//user defined error
#define MIIO_USER_ERR_INFO_UNDEF_ERROR			"user undefined error"
#define MIIO_USER_ERR_CODE_UNDEF_ERROR			MIIO_USER_ERR_CODE_R(0)

//user down command timeout
#define MIIO_USER_ERR_INFO_ACK_TIMEOUT			"user ack timeout"
#define MIIO_USER_ERR_CODE_ACK_TIMEOUT			MIIO_USER_ERR_CODE_R(1)

#define MIIO_USER_ERR_INFO_ACK_INVALID			"user ack invalid"
#define MIIO_USER_ERR_CODE_ACK_INVALID			MIIO_USER_ERR_CODE_R(2)

#define MIIO_USER_ERR_PROP_CODE                 (-4004)

#endif
