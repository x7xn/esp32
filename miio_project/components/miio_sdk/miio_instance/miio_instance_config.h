#ifndef _MIIO_INSTANCE_CONFIG_H_
#define _MIIO_INSTANCE_CONFIG_H_

#include "miio_version.h"

#define MIBLE_ENABLE					1

//����0-100
#define HUAYI_WY_TYPE_NORMAL			0	// ���ջ���������
#define HUAYI_WY_TYPE_ARIES				1	// ���ջ���-������
#define HUAYI_WY_TYPE_PISCE				2	// ���ջ���-˫����
#define HUAYI_WY_TYPE_VIRGO				3	// ���ջ���-��Ů��
#define HUAYI_WY_TYPE_TAURUS			4	// ���ջ���-��ţ��
#define HUAYI_WY_TYPE_PEGASUS			5	// ���ջ���-������
#define HUAYI_WY_TYPE_AQUARIUS          6	// ˮƿ�Ǻ�������    	������빦��
#define HUAYI_WY_TYPE_GENERAL			7   // ������ͨ��		������빦��
#define HUAYI_WYRGB_AUQARIUS			8	// ˮƿ�ǲ���

//��ʽ100-150
#define LESHI_WY_TYPE					100	// ��ʽ˫ɫ�龰�� ��׼
#define LESHI_WY_TYPE_RGB				101	// ��ʽRGB�ƴ�
#define RBE_WY_TYPE						102 // ��ض������ܵ�
#define LESHI_WY_TYPE_DBS				103 // ��ʽ-�龰WIFI˫ɫ�� -- �°�������
#define LESHI_WY_TYPE_QJ				104 // ��ʽ˫ɫ�龰�� -- wifi x��
#define LESHI_WY_TYPE_WY0B02			105 // ��ʽWY��ģ��
#define NMN_WY_TYPE_WY0A02				106 // ŵ�����龰ɫ�µ�+˧�Ʊ�����������
#define NIOYE_WYRGB_TYPE				107	// NIOYEȫ��LED��
#define LESHI_WYRGB_TYPE				108 // ��ʽWYRGB
#define HBC_WY_TYPE_WY0A01				109 // �Ƶ����ܵ�
#define ODDS_WY_TYPE_WY0A01				110 // ŷ�۵���

//��ʿ200-250
#define LEISHI_RGB_TYPE_1				200	// ��ʿ���ܲʹ�ƴ�
#define LEISHI_RGB_TYPE_2				201	// ��ʿ���ܲʹ����
#define LEISHI_WY_TYPE_1				202	// ��ʿ����WY��
#define LEISHI_WY_TYPE_2				203	// ��ʿ����WY��-W����Y��ɫ
#define LEISHI_WY_TYPE_3				204	// ��ʿ��������-W����Y��ɫ

//��������
#define GUANGYUN_WY_TYPE_1				300 //��������



//��ʽV4 WY
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
