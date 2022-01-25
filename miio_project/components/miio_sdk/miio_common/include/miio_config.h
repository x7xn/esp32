/**
* @file    miio_config.h
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

#ifndef _MIIO_CONFIG_H_
#define _MIIO_CONFIG_H_

#define	MIIO_MODEL_SIZE_MAX				(24)
#define MIIO_COUNTRY_DOMAIN_SIZE_MAX	(3)
#define MIIO_DOMAIN_SIZE_MAX			(64)
#define MIIO_FIXED_IP_NUM_MAX			(4)
#define MIIO_INFO_KEY_LEN_MAX			(32)
#define MIIO_INFO_VALUE_LEN_MAX    		(900)
#define MIIO_INFO_INTERVAL_MS			(30*60*1000)
#define MIIO_EVENT_VALUE_COUNT_MAX		(16)
#define MIIO_EVENT_LIST_LEN_MAX			(16)
#define MIIO_EVENT_TIMEOUT_MS          	(30*60*1000)
#define MIIO_PROPERTY_COOLDOWN_TIME_MS	(1*1000)
#define MIIO_RPC_TIMEOUT_MS				(4*1000)
#define MIIO_RPC_UP_SESSION_NUM_MAX		(6)
#define MIIO_RPC_DN_SESSION_NUM_MAX		(6)
#ifndef MIIO_HTTPC_TIMEOUT_MS
#define MIIO_HTTPC_TIMEOUT_MS			(30*1000)
#endif
#define MIIO_URL_SIZE_MAX				(256)
#define MIIO_OT_STACK_SIZE				(6*1024)
#define MIIO_OT_SYNC_TIME_MS_MAX		(30*60*1000)
#define MIIO_OT_SYNC_TIME_MS_MIN		(60*1000)
#define MIIO_OT_KPLV_TIMEOUT_MS_MAX		(4*60*1000)
#define MIIO_OT_KPLV_TIMEOUT_MS_MIN		(45*1000)
#define MIIO_SIGN_AN1_LEN_MAX			(128)
#define MIIO_OTA_RETRY_MAX				(3)
#define MIIO_D0_BLOCK_MS_DEFAULT		(10*1000)
#define MIIO_AUTO_OTA_TIME_MS			(60*1000)

#ifndef MIIO_SOCKET_R_TIMEOUT_MS_MAX
#define MIIO_SOCKET_R_TIMEOUT_MS_MAX	(100)
#endif
#ifndef MIIO_SOCKET_W_TIMEOUT_MS_MAX
#define MIIO_SOCKET_W_TIMEOUT_MS_MAX	(4*1000)
#endif
#ifndef MIIO_PER_RW_TIMEOUT_MS_MAX
#define MIIO_PER_RW_TIMEOUT_MS_MAX		(50)
#endif
#ifndef MIIO_PER_RW_TIMEOUT_BYTES_MAX
#define MIIO_PER_RW_TIMEOUT_BYTES_MAX	(1)
#endif
#ifndef MIIO_RPC_ACL_ENABLE
#define MIIO_RPC_ACL_ENABLE             (1)
#endif
#ifndef MIIO_OTS_ENABLE
#define MIIO_OTS_ENABLE					1
#endif
#ifndef MIIO_RECONN_RANDOM_ENABLE
#define MIIO_RECONN_RANDOM_ENABLE		1
#endif
#ifndef MIIO_OTS_PSK_ENABLE
#define MIIO_OTS_PSK_ENABLE				1
#endif
#ifndef MIIO_OTS_PACKET_SIZE_MAX
#define MIIO_OTS_PACKET_SIZE_MAX		(2048)
#endif
#ifndef MIIO_HTTPC_TLS_ENABLE
#define MIIO_HTTPC_TLS_ENABLE			1
#endif
#ifndef MIIO_HTTPDNS_ENABLE
#define MIIO_HTTPDNS_ENABLE				1
#endif

#define MIIO_RECONN_NUM_MAX			(8)
#define MIIO_RPC_RETRY_INTERVAL_MS				(3*1000)

#ifndef MIIO_HTTPDNS_TIMEOUT_MS
#define MIIO_HTTPDNS_TIMEOUT_MS			(10*1000)
#endif
#ifndef MIIO_DEVICE_CERT_ENABLE
#define MIIO_DEVICE_CERT_ENABLE			0
#endif
#ifndef MIIO_DLOG_ENABLE
#define MIIO_DLOG_ENABLE				1
#endif
#ifndef MIIO_STATASTIC_ENABLE
#define MIIO_STATASTIC_ENABLE			1
#endif
#ifndef MIIO_PROV_STATISTIC
#define MIIO_PROV_STATISTIC				0
#endif
#ifndef MIIO_OTC_TEST_ENALBE
#define MIIO_OTC_TEST_ENALBE			1
#endif
#ifndef MIIO_KPLV_ADJUST_ENABLE
#define MIIO_KPLV_ADJUST_ENABLE			1
#endif

#ifndef MIIO_IPV6_ENABLE
#define MIIO_IPV6_ENABLE                0
#endif

#ifndef MIIO_LINUX_PLATFORM
#define MIIO_LINUX_PLATFORM				0
#endif

#ifndef MIIO_AUTO_OTA_ENABLE
#define MIIO_AUTO_OTA_ENABLE			0
#endif

#ifndef MIIO_OTA_STATISTIC_ENABLE
#define MIIO_OTA_STATISTIC_ENABLE		1
#endif


#endif /* _MIIO_CONFIG_H_ */
