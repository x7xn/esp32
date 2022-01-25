#ifndef _MIIO_INSTANCE_CONFIG_H_
#define _MIIO_INSTANCE_CONFIG_H_

#include "miio_version.h"

#define MIBLE_ENABLE					1

//华艺0-100
#define HUAYI_WY_TYPE_NORMAL			0	// 华艺慧作吸顶灯
#define HUAYI_WY_TYPE_ARIES				1	// 华艺慧作-白羊星
#define HUAYI_WY_TYPE_PISCE				2	// 华艺慧作-双鱼星
#define HUAYI_WY_TYPE_VIRGO				3	// 华艺慧作-处女星
#define HUAYI_WY_TYPE_TAURUS			4	// 华艺慧作-金牛星
#define HUAYI_WY_TYPE_PEGASUS			5	// 华艺慧作-天马星
#define HUAYI_WY_TYPE_AQUARIUS          6	// 水瓶星和射手星    	有免对码功能
#define HUAYI_WY_TYPE_GENERAL			7   // 吸吊灯通用		有免对码功能
#define HUAYI_WYRGB_AUQARIUS			8	// 水瓶星彩泡

//乐式100-150
#define LESHI_WY_TYPE					100	// 乐式双色情景灯 标准
#define LESHI_WY_TYPE_RGB				101	// 乐式RGB灯带
#define RBE_WY_TYPE						102 // 瑞柏尔的智能灯
#define LESHI_WY_TYPE_DBS				103 // 乐式-情景WIFI双色灯 -- 德佰仕驱动
#define LESHI_WY_TYPE_QJ				104 // 乐式双色情景灯 -- wifi x版
#define LESHI_WY_TYPE_WY0B02			105 // 乐式WY新模板
#define NMN_WY_TYPE_WY0A02				106 // 诺曼尼情景色温灯+帅灯堡智能吸顶灯
#define NIOYE_WYRGB_TYPE				107	// NIOYE全彩LED灯
#define LESHI_WYRGB_TYPE				108 // 乐式WYRGB
#define HBC_WY_TYPE_WY0A01				109 // 黄的智能灯
#define ODDS_WY_TYPE_WY0A01				110 // 欧帝灯饰

//雷士200-250
#define LEISHI_RGB_TYPE_1				200	// 雷士智能彩光灯带
#define LEISHI_RGB_TYPE_2				201	// 雷士智能彩光灯泡
#define LEISHI_WY_TYPE_1				202	// 雷士智能WY灯
#define LEISHI_WY_TYPE_2				203	// 雷士智能WY灯-W调光Y调色
#define LEISHI_WY_TYPE_3				204	// 雷士智能面板灯-W调光Y调色

//广云物联
#define GUANGYUN_WY_TYPE_1				300 //广云物联



//乐式V4 WY
#define MIIO_INSTANCE_MODEL				"xhuan.aircondition.ac01"
#define MIBLE_PRODUCT_ID                6893
#define HUAYI_WY_TYPE					LESHI_WY_TYPE
#define MIIO_APP_VERSION_NUMBER			2


#define MIIO_COMMANDS_ENABLE			0
#ifndef MIIO_INSTANCE_LX2_ADAPT
#define MIIO_INSTANCE_LX2_ADAPT			0
#endif
#if MIIO_INSTANCE_LX2_ADAPT
#define MIIO_PSM_ENABLE					1
#else
#define MIIO_PSM_ENABLE					0
#endif

#define MIIO_LOG_ANSI_COLOR_ENABLE		1
#define MIIO_HANDSHAKE_ENABLE			1
#define MIIO_HANDSHAKE_TEST				0
#define MIIO_CONFIG_ROUTER_SAFE_ENABLE	1
#define MIIO_CONFIG_ROUTER_SAFE_TEST	0

#define MIIO_MONITOR_REBOOT_ENABLE				0
#define MIIO_MONITOR_INTERVAL_S					(1)
#define MIIO_MONITOR_INFO_TIMEOUT_S				(10)
#define MIIO_MONITOR_OFFLINE_TIMEOUT_S			(15*60)
#define MIIO_MONITOR_AP_TIMEOUT_S				(30*60)
#define MIIO_MONITOR_STA_CONNECT_INTEVAL_S 		(5)
#define MIIO_MONITOR_REBOOT_TIMEOUT_S			(60*60)
#define MIIO_MONITOR_NTP_INTEVAL_S 				(15)

#define MIIO_NET_SMART_CONFIG_RETRY				(4)
#define MIIO_NET_SMART_CONFIG_TIMEOUT_S			(30)

#define MIIO_NET_INDICATOR_ENABLE				0
#define MIIO_NET_INDICATOR_LED_SINGLE			1
#define MIIO_NET_INDICATOR_LED_PIN_BLUE			14
#if !MIIO_NET_INDICATOR_LED_SINGLE
#define MIIO_NET_INDICATOR_LED_PIN_YELLOW		12
#endif
#define MIIO_NET_INDICATOR_LED_COMMON_CATHODE	0

#if MIIO_AUTO_OTA_ENABLE
#ifndef MIIO_AUTO_OTA_ABILITY
#define MIIO_AUTO_OTA_ABILITY					0
#endif
#endif

#endif
