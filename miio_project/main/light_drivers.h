/*
 * light_driver.h
 *
 *  Created on: 2019��4��18��
 *      Author: Administrator
 */

#ifndef _LIGHT_DRIVERS_H_
#define _LIGHT_DRIVERS_H_

#include "stdint.h"
#include "miio_instance.h"
#include "miio_arch.h"
#include "jsmi.h"
#include "light.h"
#include "../../esp-idf/components/driver/include/driver/ledc.h"
#include "../../esp-idf/components/driver/include/driver/gpio.h"
#include "../../esp-idf/components/driver/include/driver/pcnt.h"
#include "../../esp-idf/components/driver/include/driver/uart.h"


//UART1
#define UART1_RX1_BUF_SIZE	169
#define UART1_TX1_BUF_SIZE	169
#define UART1_TXD1_PIN		17
#define UART1_RXD1_PIN		16

//�����궨��
//ÿ����һ�λ���10ms��ʱ��
#define  LIGNT_TIMER_TICK		10//10ms������

#define  LIGHT_REMOTE_MATCHMAX				8							//������ң��������
#define  LIGHT_REMOTE_ALL					0							//�ܿ�
#define  LIGHT_REMOTE_ONEKEY_SWITCH_TIME	(4*1000/LIGNT_TIMER_TICK)	//4S
#define  LIGHT_REMOTE_ONEKEY_TIME   		(500/LIGNT_TIMER_TICK)	  	//�������ƣ��ڽ��յ����¶��������500ms�ڽ��յ��ɿ�ָ������
//#define  LIGHT_REMOTE_STIMER				(3*1000/LIGNT_TIMER_TICK)	//����ʱ��3S
#define  LIGHT_REMOTE_STIMER				(5*1000/LIGNT_TIMER_TICK)	//����ʱ��5S
#define  LIGHT_REMOTE_CYCTIMER				(20*1000/LIGNT_TIMER_TICK)	//ѭ���������ʱ��20S
#define  LIGHT_SCENENUMBER   				16 							//���洢�龰��
//#define  LIGHT_RESET_TIMER					(6*1000/LIGNT_TIMER_TICK)	//�ϵ�6S�ڣ������ϵ�ʱ��
#define  LIGHT_RESET_TIMER					(6*100/LIGNT_TIMER_TICK)	//�ϵ�6S�ڣ������ϵ�ʱ��

#define  LIGHT_SWITCH_TIMER					(30*1000/LIGNT_TIMER_TICK)	//�ϵ�30S�ڣ����ٷֶο���ʱ��
#define  LIGHT_FLASH_FRE 					(1000/LIGNT_TIMER_TICK)	  	//1S�������������ʱ��
#define  LIGHT_TEST_TIMER					(6000/LIGNT_TIMER_TICK)		//�ϵ�5S�ڲ���ģʽ

#define  LIGHT_AC_CTIMER					(80/LIGNT_TIMER_TICK)		//80ms,AC�����ʱ��
#define  LIGHT_AC_FRE						2							//AC���ʱ���ڲ⵽�����Ƶ��

#define  LIGHT_FADE_NO						0						// ����ʱ��
#define  LIGHT_FADE_TIME    				(1000/LIGNT_TIMER_TICK)	// ���俪���ʱ��
#define  LIGHT_FADE_DEFAULT					10						// ����Ĭ��ʱ��1S=10*100MS
#define  LIGHY_FADE_UNIT					100						// ����ʱ�䵥λ100MS

#define  LIGHT_REMOTE_SECEN					128

#define  LIGHT_MODE_TIMER					30	//˯��-���ѣ�����ʱ��-����
#define  LIGHT_WEAK_EBR						100	//�����������%
#define  LIGHT_SLEEP_SBR					50	//˯�߿�ʼ����%

//���ղ���======================================================================
#if(HUAYI_WY_TYPE == HUAYI_WY_TYPE_GENERAL)	//������ͨ��		������빦��
// ������ͨ��  	GPIO4����  3K  ���ӹ��� 15%   ��AC		WY��ͬ
//          GPIO4����  16K ���㹦��   3%  ��AC		WY����
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		0					//RGB�Ƿ��
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			0 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			0 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	0 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		0 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				0 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			4 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			0 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		0 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(30*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			800 // ���صƼ������ʱ�� ��ʿ��300  ����800
#define  LIGHT_SWITCH_BR_DEV		100	// �����ٶ�ԽСԽ��     ��ʿ 60        ����100
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			1   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*15/100)
#define  LIGHT_CONSTANT_P1		0 // 1-�㶨����  ��0-���ӹ��ʡ�
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	16000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// �������������� 3%
#define  LIGHT_CONSTANT_P2		1 // ��1-�㶨���ʡ�  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===============================================================
#elif(HUAYI_WY_TYPE == HUAYI_WY_TYPE_AQUARIUS)// ˮƿ�Ǻ�������    	������빦��
//���ջ���ˮƿ��˫ɫ��  Aquarius
/* �����Ǻ�ˮƿ��    ����ͬһ����
 * GPIO4 ����           3KHZ���Ǹ��롢�㹦�ʡ�15%����AC��⡢Сҹ��Ϊ�ƹ�15%
 * GPIO4 ����10K:3Khz��W����3%~100%��ɫ��0~100 ��AC��⣬���ӹ���  W����Yɫ��
 * �̰����ȵ����ȵȼ�Ϊ1%��20%��50%��100%
 * 2019/11/26 ������  1����  �����ж��ǲ���ʹ�� APP�ɵ���1-100PWM������Ӧ�ı仯
 */
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		0					//RGB�Ƿ��
#define  LIGHT_WY_WBR_YCT		1					//W����Y��ɫ
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			0 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			0 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	0 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		0 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				0 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			4 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			0 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		0 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(30*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			800 // ���صƼ������ʱ�� ��ʿ��300  ����800
#define  LIGHT_SWITCH_BR_DEV		100	// �����ٶ�ԽСԽ��     ��ʿ 60        ����100
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			1   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*15/100)
#define  LIGHT_CONSTANT_P1		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		0 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	3000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// �������������� 3%
#define  LIGHT_CONSTANT_P2		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//==============================================================================
//���ջ���������˫ɫ��
#elif(HUAYI_WY_TYPE == HUAYI_WY_TYPE_ARIES)
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		0					//RGB�Ƿ��
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			0 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			0 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	0 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		0 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				0 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			4 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			0 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		0 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(30*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			800 // ���صƼ������ʱ�� ��ʿ��300  ����800
#define  LIGHT_SWITCH_BR_DEV		100	// �����ٶ�ԽСԽ��     ��ʿ 60        ����100
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			0   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*10/100)
#define  LIGHT_CONSTANT_P1		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	600
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*12/100)
#define  LIGHT_CONSTANT_P2		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		1 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================
//���ջ���˫����˫ɫ��
#elif(HUAYI_WY_TYPE == HUAYI_WY_TYPE_PISCE)
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		0					//RGB�Ƿ��
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			0 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			0 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	0 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		0 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				0 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			4 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			0 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		0 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(30*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			800 // ���صƼ������ʱ�� ��ʿ��300  ����800
#define  LIGHT_SWITCH_BR_DEV		100	// �����ٶ�ԽСԽ��     ��ʿ 60        ����100
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			0   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*10/100)
#define  LIGHT_CONSTANT_P1		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	16000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P2		1 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================
//���ջ�����Ů��˫ɫ��
#elif(HUAYI_WY_TYPE == HUAYI_WY_TYPE_VIRGO)
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		0					//RGB�Ƿ��
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			0 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			0 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	0 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		0 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				0 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			4 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			0 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		0 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(30*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			800 // ���صƼ������ʱ�� ��ʿ��300  ����800
#define  LIGHT_SWITCH_BR_DEV		100	// �����ٶ�ԽСԽ��     ��ʿ 60        ����100
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			0   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*10/100)
#define  LIGHT_CONSTANT_P1		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	16000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P2		1 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================
//����----
#elif(HUAYI_WY_TYPE == HUAYI_WY_TYPE_TAURUS)
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		0					//RGB�Ƿ��
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			0 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			0 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	0 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		0 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				0 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			4 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			0 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		0 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(30*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			800 // ���صƼ������ʱ�� ��ʿ��300  ����800
#define  LIGHT_SWITCH_BR_DEV		100	// �����ٶ�ԽСԽ��     ��ʿ 60        ����100
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			0   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*10/100)
#define  LIGHT_CONSTANT_P1		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	16000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P2		1 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================
//����
#elif(HUAYI_WY_TYPE == HUAYI_WY_TYPE_PEGASUS)
// GPIO4����  3K 10%-100%  ��AC  ͬ�����    ���ӹ���
// GPIO4����  16K 3%-100%  ��AC  �������        �㹦��
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		0					//RGB�Ƿ��
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			0 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			0 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	0 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		0 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				0 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			4 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			0 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		0 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(30*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			800 // ���صƼ������ʱ�� ��ʿ��300  ����800
#define  LIGHT_SWITCH_BR_DEV		100	// �����ٶ�ԽСԽ��     ��ʿ 60        ����100
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			0   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*10/100)
#define  LIGHT_CONSTANT_P1		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	16000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P2		1 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================
//����ˮƿ�ǲʹ����
#elif(HUAYI_WY_TYPE == HUAYI_WYRGB_AUQARIUS)
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		1					//RGB�Ƿ��
#define  LIGHT_WY_WBR_YCT		0					//W����Y��ɫ
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			0 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			1 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	0 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		1 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				0 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			4 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			0 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		0 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(30*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			300 // ���صƼ������ʱ�� ��ʿ��300  ����800
#define  LIGHT_SWITCH_BR_DEV		60	// �����ٶ�ԽСԽ��     ��ʿ 60        ����100
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			0   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	8000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		1 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		0 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	3000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// �������������� 3%
#define  LIGHT_CONSTANT_P2		1 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================

//===========================================================================
/*
��------	ʿ------	��------	��
��------	ʿ------	��------	��
��------	ʿ------	��------	��
*/
//===========================================================================

// ��ʿ���ܲʹ�ƴ�
#elif(HUAYI_WY_TYPE == LEISHI_RGB_TYPE_1)
#define  LIGHT_REMOTE_LEISHI	0					//��ʿר��ң����
#define  LIGHT_WY_PWMON			0					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		1					//RGB�Ƿ��
#define  LIGHT_WY_WBR_YCT		0					//W����Y��ɫ
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			0 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			1 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	0 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		1 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				1 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			8 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			1 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		1 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			300 // ���صƼ������ʱ�� ��ʿ��300  ����800
#define  LIGHT_SWITCH_BR_DEV		60	// �����ٶ�ԽСԽ��     ��ʿ 60        ����100
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			0   // �Ƿ�֧�ֲ���ң���������
#define  LIGHT_REPOWER_SAVE			1	// �ϵ�����зֶ�

//P1
#define  LIGHT_FRE_P1          	8000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		1 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		0 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	3000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// �������������� 3%
#define  LIGHT_CONSTANT_P2		1 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================

//��ʿ���ܲʹ����
#elif(HUAYI_WY_TYPE == LEISHI_RGB_TYPE_2)
#define  LIGHT_REMOTE_LEISHI	0					//��ʿר��ң����
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		1					//RGB�Ƿ��
#define  LIGHT_WY_WBR_YCT		0					//W����Y��ɫ
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			0 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			1 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	0 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		1 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				1 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			8 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			1 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		1 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			300 // ���صƼ������ʱ�� ��ʿ��300  ����800
#define  LIGHT_SWITCH_BR_DEV		60	// �����ٶ�ԽСԽ��     ��ʿ 60        ����100
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			0   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	8000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		1 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		0 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	3000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// �������������� 3%
#define  LIGHT_CONSTANT_P2		1 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================

// ��ʿ����WY��
#elif(HUAYI_WY_TYPE == LEISHI_WY_TYPE_1)
#define  LIGHT_REMOTE_LEISHI	0					//��ʿר��ң����
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		0					//RGB�Ƿ��
#define  LIGHT_WY_WBR_YCT		0					//W����Y��ɫ
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			0 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			0 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	0 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		1 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				1 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			8 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			1 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		1 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			300 // ���صƼ������ʱ�� ��ʿ��300  ����800
#define  LIGHT_SWITCH_BR_DEV		60	// �����ٶ�ԽСԽ��     ��ʿ 60        ����100
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			0   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	600
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// �������������� 3%
#define  LIGHT_CONSTANT_P2		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		1 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================

// ��ʿ����WY��-W����Y��ɫ
#elif(HUAYI_WY_TYPE == LEISHI_WY_TYPE_2)
#define  LIGHT_REMOTE_LEISHI	0					//��ʿר��ң����
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		0					//RGB�Ƿ��
#define  LIGHT_WY_WBR_YCT		1					//W����Y��ɫ
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			0 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			0 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	0 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		1 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				1 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			8 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			1 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		1 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			300 // ���صƼ������ʱ�� ��ʿ��300  ����800
#define  LIGHT_SWITCH_BR_DEV		60	// �����ٶ�ԽСԽ��     ��ʿ 60        ����100
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			0   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		1 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	600
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// �������������� 3%
#define  LIGHT_CONSTANT_P2		1 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		1 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================
// ��ʿ��������-W����Y��ɫ-W����Y��ɫ
#elif(HUAYI_WY_TYPE == LEISHI_WY_TYPE_3)
#define  LIGHT_REMOTE_LEISHI	0					//��ʿר��ң����
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		0					//RGB�Ƿ��
#define  LIGHT_WY_WBR_YCT		1					//W����Y��ɫ
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			1 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			1 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	1 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		1 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				1 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			8 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			1 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		1 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			300 // ���صƼ������ʱ�� ��ʿ��300  ����800
#define  LIGHT_SWITCH_BR_DEV		60	// �����ٶ�ԽСԽ��     ��ʿ 60        ����100
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			0   // �Ƿ�֧�ֲ���ң���������


//P1
#define  LIGHT_FRE_P1          	5000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*5/100)
#define  LIGHT_CONSTANT_P1		1 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	600
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*5/100)	// �������������� 3%
#define  LIGHT_CONSTANT_P2		1 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		1 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21


//===========================================================================
/*
��------	ʽ------	��------	��
��------	ʽ------	��------	��
��------	ʽ------	��------	��
*/
//===========================================================================

#elif(HUAYI_WY_TYPE == LESHI_WY_TYPE)
//��ʽ˫ɫ��
// 1����ʽ��׼С���龰˫ɫ��WIFI ����:
// IO4����                    :	3KHZ--WY--���ӹ���--��AC--�������10%--Сҹ��Ϊ�ƹ�10%--���ֶ�(��ʽ��׼)--ң�ؽ���--��ʽ���뷽ʽ
// IO4��������10k :	16KHZ--WY--�㹦��--��AC--�������3%--Сҹ��Ϊ�ƹ�10%--���ֶ�(��ʽ��׼)--ң�ؽ���--��ʽ���뷽ʽ
#define  LIGHT_WY_PWMON				1					// WY�Ƿ��
#define  LIGHT_RGB_PWMON			0					// RGB�Ƿ��
#define  LIGHT_BIT_DUTY				LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX				(unsigned int)0x0FFF//

//����
#define  LIGHT_PROSET_SWITCH		1 // רҵ����
#define  LIGHT_NIGHT_SWITCH			1 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			1 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		1 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	1 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		1 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				1 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			8 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_RESET_CNT_LOCK		20// ���ٿ���N��-���������
#define  LIGHT_MATCH_CMDON			1 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		1 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			800 // ���صƼ������ʱ�� ��ʿ��300-����800
#define  LIGHT_SWITCH_BR_DEV		100	// ��ʿ�����ٶ� 60        �����ٶ�100  ԽСԽ��
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			1   // �Ƿ�֧�ֲ���ң���������

//P1 IO4����
#define  LIGHT_FRE_P1          		3000
#define  LIGHT_PWM_MIN_P1			(LIGHT_PWM_MAX*10/100)//�����������10%
#define  LIGHT_CONSTANT_P1			0 					// 1-�㶨����  ��0-���ӹ��ʡ�
#define  LIGHT_AC_ENABLE_P1			1 					// 1-AC	 0-NOAC
//P2 IO4��������10k
#define  LIGHT_FRE_P2        		16000
#define  LIGHT_PWM_MIN_P2			(LIGHT_PWM_MAX*3/100)//�������������� 3%
#define  LIGHT_CONSTANT_P2			1 					// ��1-�㶨���ʡ�  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2			0 					// 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================
#elif(HUAYI_WY_TYPE == LESHI_WY_TYPE_QJ)
//��ʽ˫ɫ��
// 1����ʽ��׼С���龰WIFI˫ɫ�Ƶ�--�����޸ģ�
// IO4��������10k :ģʽ�޸ĳ�16KHZ ,3% ,��AC ,�㹦�� �汾  ��IO���ղ������� ��
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		0					//RGB�Ƿ��
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			0 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			0 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	0 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		1 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				1 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			8 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			1 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		1 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			800 // ���صƼ������ʱ�� ��ʿ��300-����800
#define  LIGHT_SWITCH_BR_DEV		100	// ��ʿ�����ٶ� 60        �����ٶ�100  ԽСԽ��
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			0   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*10/100)
#define  LIGHT_CONSTANT_P1		0 // 1-�㶨����  ��0-���ӹ��ʡ�
#define  LIGHT_AC_ENABLE_P1		0 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	16000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// �������������� 3%
#define  LIGHT_CONSTANT_P2		1 // ��1-�㶨���ʡ�  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================
#elif(HUAYI_WY_TYPE == RBE_WY_TYPE)
// ��ض����ܵ�
// ���ӷֶι���   ���ô���8��
// IO4��������10k :ģʽ�޸ĳ�16KHZ ,3% ,��AC ,�㹦�� �汾  ��IO���ղ������� ��
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		0					//RGB�Ƿ��
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			1 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			0 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	1 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		1 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				1 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			8 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			1 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		1 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			800 // ���صƼ������ʱ�� ��ʿ��300  ����800
#define  LIGHT_SWITCH_BR_DEV		100	// �����ٶ�ԽСԽ��     ��ʿ 60        ����100
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			1   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*10/100)
#define  LIGHT_CONSTANT_P1		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	16000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P2		1 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================
#elif(HUAYI_WY_TYPE == LESHI_WY_TYPE_DBS)
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		0					//RGB�Ƿ��
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			1 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			1 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	1 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		1 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				1 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			8 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			1 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		1 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			800 // ���صƼ������ʱ�� ��ʿ��300-����800
#define  LIGHT_SWITCH_BR_DEV		150	// ��ʿ�����ٶ� 60        �����ٶ�100  ԽСԽ��
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			1   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	1000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*4/100)	// �������������� 3%
#define  LIGHT_CONSTANT_P2		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC
#define  LIGHT_PWM_REVERSEL_P2	1 //PWM��ת

//SW
#define  LIGHT_SW_GPIO        	17

//===========================================================================
// ��ʽWYRGB
#elif(HUAYI_WY_TYPE == LESHI_WYRGB_TYPE)
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		1					//RGB�Ƿ��
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			0 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			1 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	0 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		1 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				1 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			8 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			1 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		1 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			800 // ���صƼ������ʱ�� ��ʿ��300-����800
#define  LIGHT_SWITCH_BR_DEV		100	// ��ʿ�����ٶ� 60        �����ٶ�100  ԽСԽ��
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			1   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	8000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		1 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		0 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	3000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// �������������� 3%
#define  LIGHT_CONSTANT_P2		1 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21
//===========================================================================
// ��ʽ�ʹ�ƴ�
#elif(HUAYI_WY_TYPE == LESHI_WY_TYPE_RGB)
#define  LIGHT_WY_PWMON			0					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		1					//RGB�Ƿ��
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			0 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			1 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	0 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		1 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				1 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			8 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			1 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		1 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			800 // ���صƼ������ʱ�� ��ʿ��300-����800
#define  LIGHT_SWITCH_BR_DEV		100	// ��ʿ�����ٶ� 60        �����ٶ�100  ԽСԽ��
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			0   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	8000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		0 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	3000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// �������������� 3%
#define  LIGHT_CONSTANT_P2		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21


//===========================================================================
// NIOYEȫ��LED��
#elif(HUAYI_WY_TYPE == NIOYE_WYRGB_TYPE)
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		1					//RGB�Ƿ��
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			0 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			1 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	0 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		1 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				1 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			8 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			1 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		1 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			800 // ���صƼ������ʱ�� ��ʿ��300-����800
#define  LIGHT_SWITCH_BR_DEV		100	// ��ʿ�����ٶ� 60        �����ٶ�100  ԽСԽ��
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			1   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	2000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*19/100)
#define  LIGHT_CONSTANT_P1		1 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		0 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	3000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// �������������� 3%
#define  LIGHT_CONSTANT_P2		1 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21


//===========================================================================
// ��ʽWY��ģ��
/*
1��ʹ����ʽWiFi���濪��-���ڹ���UI���� ��֧��ģʽ���ܣ�֧���鶯���أ�Ĭ�����õȹ��ܣ�
2�����������16KHz / 3000�������롢˫ɫ���㹦�ʡ�3%��PWM���4mA)   ��AC���  ��
3��GPIO4 ����10KΪ��16KHz / 3000�������롢˫ɫ�����ӹ��ʡ�3%��PWM���4mA)   ��AC���
4���ϵ�Ĭ��Ϊ���Ƶġ�
*/
#elif(HUAYI_WY_TYPE == LESHI_WY_TYPE_WY0B02)
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		0					//RGB�Ƿ��
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			1 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			1 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		1 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	1 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		1 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				1 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			8 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			1 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		1 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			800 // ���صƼ������ʱ�� ��ʿ��300-����800
#define  LIGHT_SWITCH_BR_DEV		100	// ��ʿ�����ٶ� 60        �����ٶ�100  ԽСԽ��
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			0   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	16000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*2/100)//3%
#define  LIGHT_CONSTANT_P1		1 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	16000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*2/100)//3%
#define  LIGHT_CONSTANT_P2		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		1 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21
//===========================================================================
//ŷ�۵���
#elif(HUAYI_WY_TYPE == ODDS_WY_TYPE_WY0A01)//
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		0					//RGB�Ƿ��
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			1 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			1 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	1 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		1 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				1 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			8 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			1 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		1 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			800 // ���صƼ������ʱ�� ��ʿ��300-����800
#define  LIGHT_FADE_ONOFF_TIMER		1000
#define  LIGHT_SWITCH_BR_DEV		60	// ��ʿ�����ٶ� 60        �����ٶ�100  ԽСԽ��
#define  LIGHT_SWITCH_BR_DMI		2 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			1   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	800
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*4/100)	// �������������� 3%
#define  LIGHT_CONSTANT_P2		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC
#define  LIGHT_PWM_REVERSEL_P2	1 //PWM��ת

//SW
#define  LIGHT_SW_GPIO        	17

//===========================================================================
//�ʵ����ܵ�
#elif(HUAYI_WY_TYPE == HBC_WY_TYPE_WY0A01)//
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		0					//RGB�Ƿ��
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			1 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			1 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	1 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		1 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				1 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			8 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			1 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		1 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			800 // ���صƼ������ʱ�� ��ʿ��300-����800
#define  LIGHT_FADE_ONOFF_TIMER		1000
#define  LIGHT_SWITCH_BR_DEV		60	// ��ʿ�����ٶ� 60        �����ٶ�100  ԽСԽ��
#define  LIGHT_SWITCH_BR_DMI		2 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			1   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	800
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*4/100)	// �������������� 3%
#define  LIGHT_CONSTANT_P2		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC
#define  LIGHT_PWM_REVERSEL_P2	1 //PWM��ת

//SW
#define  LIGHT_SW_GPIO        	17

//===========================================================================
//ŵ�����龰ɫ�µ�+˧�Ʊ�����������
#elif(HUAYI_WY_TYPE == NMN_WY_TYPE_WY0A02)//
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		0					//RGB�Ƿ��
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			1 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			1 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	1 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		1 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				1 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			8 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			1 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		1 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			800 // ���صƼ������ʱ�� ��ʿ��300-����800
#define  LIGHT_SWITCH_BR_DEV		150	// ��ʿ�����ٶ� 60        �����ٶ�100  ԽСԽ��
#define  LIGHT_FADE_ONOFF_TIMER		1000
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			1   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	1000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*4/100)	// �������������� 3%
#define  LIGHT_CONSTANT_P2		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC
#define  LIGHT_PWM_REVERSEL_P2	1 //PWM��ת

//SW
#define  LIGHT_SW_GPIO        	17

//===========================================================================
//��������
//��������
//��������
//��������
//��������
//��������
#elif(HUAYI_WY_TYPE == GUANGYUN_WY_TYPE_1)
#define  LIGHT_WY_PWMON			1					//WY�Ƿ��
#define  LIGHT_RGB_PWMON		0					//RGB�Ƿ��
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			0 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			0 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	0 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		0 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				1 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			8 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			1 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		1 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			800 // ���صƼ������ʱ�� ��ʿ��300-����800
#define  LIGHT_SWITCH_BR_DEV		100	// ��ʿ�����ٶ� 60        �����ٶ�100  ԽСԽ��
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			0   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		0 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	3000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// �������������� 3%
#define  LIGHT_CONSTANT_P2		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================
#else
#define  LIGHT_WY_PWMON			1						//WY�Ƿ��
#define  LIGHT_RGB_PWMON		0						//RGB�Ƿ��
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//����
#define  LIGHT_NIGHT_SWITCH			1 // ��ҹ��ģʽ�Ƿ�֧��
#define  LIGHT_MODE_SWITCH			1 // ��ɫ��ģʽ/RGBģʽ�����Ƿ�֧��
#define  LIGHT_REMOTE_NETSWITCH		0 // �Ƿ�֧��ң���������ϱ�
#define  LIGHT_SMARTSWITCH_ENABLE	0 // �Ƿ�֧���鶯����
#define  LIGHT_POWER_DEFAULT		1 // �Ƿ�֧���ϵ�Ĭ��״̬����
#define  LIGHT_RE_POWER				1 // �ֶι���  1�� 0�ر�
#define  LIGHT_RESET_CNT			8 // ���ٿ���N��-WIFI�������� ���յ�5�Σ�����8��
#define  LIGHT_MATCH_CMDON			1 // �Ƿ�֧��-����/�صƼ���������
#define  LIGHT_MATCH_CMD24G			1 // �Ƿ�֧��-һ�����뷽ʽ
#define  LIGHT_MATCH_CLEARALL		1 // �����Ƿ��������ң���� 1�������
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S����--���ն���ʱ��30S,��������ʱ��12S
#define  LIGHT_FADE_ON				1	// ����or����   1-��  0-��
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1ʱ,APP�Ƿ���н���򿪺͹رչ���
#define  LIGHT_FADE_MAX  			300 // ���صƼ������ʱ�� ��ʿ��300  ����800
#define  LIGHT_SWITCH_BR_DEV		60	// �����ٶ�ԽСԽ��     ��ʿ 60        ����100
#define  LIGHT_SWITCH_BR_DMI		1 	// ��С���ֵⷧ
#define  LIGHT_SWITCH_TM_DEV		15	// ��ʽɫ���ٶ�
#define  LIGHT_REMOTE_TEST			0   // �Ƿ�֧�ֲ���ң���������

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*10/100)
#define  LIGHT_CONSTANT_P1		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	600
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// �������������� 3%
#define  LIGHT_CONSTANT_P2		0 // 1-�㶨����  0-���ӹ���
#define  LIGHT_AC_ENABLE_P2		1 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21
#endif





//=====================================================================
//IO��
//AC�������
#define FAN_PCNT_UNIT      		PCNT_UNIT_0
#define FAN_PCNT_H_LIM_VAL      10000
#define FAN_PCNT_L_LIM_VAL     -10000
#define FAN_PCNT_THRESH1_VAL    5000
#define FAN_PCNT_THRESH0_VAL   -5000
#define FAN_PCNT_INPUT_SIG_IO   22  // Pulse Input GPIO
#define FAN_PCNT_INPUT_CTRL_IO  23  // Control GPIO HIGH=count up, LOW=count down
//��������
#define LIGHT_TYPE_GPIO1		4		// GPIO4 ����
#define LIGHT_TYPE_GPIO2		5

//WY
#define  LIGHT_PWM_WMAX			(LIGHT_PWM_MAX)
#define  LIGHT_PWM_YMAX			(LIGHT_PWM_MAX)
//RGB
#define  LIGHT_PWM_RMAX			(LIGHT_PWM_MAX)
#define  LIGHT_PWM_GMAX			(LIGHT_PWM_MAX)
#define  LIGHT_PWM_BMAX			(LIGHT_PWM_MAX)

//LED-W-Y-R-G-B
#define  LIGHT_TOTAL_NUM 		5
#define  LIGHT_TIMER          	LEDC_TIMER_0
#define  LIGHT_MODE           	LEDC_HIGH_SPEED_MODE

//W
#define  LIGHT_W_NUMCH			0
#define  LIGHT_W_GPIO       	19
#define  LIGHT_W_CHANNEL    	LEDC_CHANNEL_1
//Y
#define  LIGHT_Y_NUMCH			1
#define  LIGHT_Y_GPIO       	18
#define  LIGHT_Y_CHANNEL    	LEDC_CHANNEL_2
//R
#define  LIGHT_R_NUMCH			2
#define  LIGHT_R_GPIO       	16
#define  LIGHT_R_CHANNEL    	LEDC_CHANNEL_3
//G
#define  LIGHT_G_NUMCH			3
#define  LIGHT_G_GPIO       	17
#define  LIGHT_G_CHANNEL    	LEDC_CHANNEL_4
//B
#define  LIGHT_B_NUMCH			4
#define  LIGHT_B_GPIO       	26
#define  LIGHT_B_CHANNEL    	LEDC_CHANNEL_5

//WY
#define LIGHT_TEMPLATETRUE_MIN		3000	// ů��
#define LIGHT_TEMPLATETRUE_MAX		6400	// ���
#define LIGHT_TEMPLATETRUE_MID		4700	// ���Թ�

#define LIGHT_BRIGHTNESS_MIN		10
#define LIGHT_BRIGHTNESS_MAX		1000

//RGB
#define LIGHT_RGB_FADECNT			7//

#define LIGHT_RGB_VMAX				255
#define LIGHT_RGB_VMIN				0

#define LIGHT_WY_NORMAL				0	//ɫ��ģʽ
#define LIGHT_RGB_NORMAL			1	//�ʹ�ģʽ
#define LIGHT_RGB_FADE				2	//�߲ʽ���
#define LIGHT_RGB_JUMP				3	//�߲�����
#define LIGHT_CD_SUNDAY				4	//�չ�ģʽ
#define LIGHT_CD_NIGHT				5	//�¹�ģʽ
#define LIGHT_RGB_COLOR				6	//�߲�����
#define LIGHT_CD_WARMTH				7	//��ܰģʽ
#define LIGHT_CD_TVMODE				8	//����ģʽ
#define LIGHT_CD_READING			9	//�Ķ�ģʽ
#define LIGHT_CD_CPMPUTER			10	//����ģʽ
#define LIGHT_CD_HOST				11	//���ģʽ
#define LIGHT_CD_ENTER				12	//����ģʽ
#define LIGHT_CD_WEAKUP				13	//�峿����
#define LIGHT_CD_DUSK				14	//�ƻ�����
#define LIGHT_CD_SLEEP				15	//ҹ������

void light_driver_start(void);
void light_driver_task(void);
void light_driver_fade(unsigned int brightness,unsigned int templatetrue,unsigned char r,unsigned char g,unsigned char b);
void light_driver_jump(unsigned int brightness,unsigned int templatetrue,unsigned char r,unsigned char g,unsigned char b);

void light_driver_pcnt_init(void);
void light_driver_pcnt_resume(void);
void light_driver_pcnt_get(int16_t *conter);

uint32_t light_driver_getfadetimer(void);

void aircondition_driver_task(void);
void pending_query_aircondition_task(uint8_t task_type);
void pending_aircondition_uart_task(uint8_t aircondition_index,uint8_t cmd_type,aircondition_device_infor_t aircondition_info);
void update_aircondition_addr_to_driver(aircondition_storage_info_t *aircondition_device_info);

#endif /* _LIGHT_DRIVERS_H_ */
