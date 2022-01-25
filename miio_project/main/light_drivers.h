/*
 * light_driver.h
 *
 *  Created on: 2019年4月18日
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

//基本宏定义
//每运行一次花费10ms的时间
#define  LIGNT_TIMER_TICK		10//10ms节拍器

#define  LIGHT_REMOTE_MATCHMAX				8							//最多配对遥控器数量
#define  LIGHT_REMOTE_ALL					0							//总控
#define  LIGHT_REMOTE_ONEKEY_SWITCH_TIME	(4*1000/LIGNT_TIMER_TICK)	//4S
#define  LIGHT_REMOTE_ONEKEY_TIME   		(500/LIGNT_TIMER_TICK)	  	//单键控制，在接收到按下动作后，如果500ms内接收到松开指令，则调光
//#define  LIGHT_REMOTE_STIMER				(3*1000/LIGNT_TIMER_TICK)	//保存时间3S
#define  LIGHT_REMOTE_STIMER				(5*1000/LIGNT_TIMER_TICK)	//保存时间5S
#define  LIGHT_REMOTE_CYCTIMER				(20*1000/LIGNT_TIMER_TICK)	//循环调光最大时间20S
#define  LIGHT_SCENENUMBER   				16 							//最大存储情景数
//#define  LIGHT_RESET_TIMER					(6*1000/LIGNT_TIMER_TICK)	//上电6S内，快速上电时间
#define  LIGHT_RESET_TIMER					(6*100/LIGNT_TIMER_TICK)	//上电6S内，快速上电时间

#define  LIGHT_SWITCH_TIMER					(30*1000/LIGNT_TIMER_TICK)	//上电30S内，快速分段开关时间
#define  LIGHT_FLASH_FRE 					(1000/LIGNT_TIMER_TICK)	  	//1S，渐变和跳变间隔时间
#define  LIGHT_TEST_TIMER					(6000/LIGNT_TIMER_TICK)		//上电5S内产测模式

#define  LIGHT_AC_CTIMER					(80/LIGNT_TIMER_TICK)		//80ms,AC检测间隔时间
#define  LIGHT_AC_FRE						2							//AC间隔时间内测到的最低频率

#define  LIGHT_FADE_NO						0						// 立开时间
#define  LIGHT_FADE_TIME    				(1000/LIGNT_TIMER_TICK)	// 渐变开最大时间
#define  LIGHT_FADE_DEFAULT					10						// 渐变默认时间1S=10*100MS
#define  LIGHY_FADE_UNIT					100						// 渐变时间单位100MS

#define  LIGHT_REMOTE_SECEN					128

#define  LIGHT_MODE_TIMER					30	//睡眠-唤醒，渐变时间-分钟
#define  LIGHT_WEAK_EBR						100	//唤醒最后亮度%
#define  LIGHT_SLEEP_SBR					50	//睡眠开始亮度%

//华艺部分======================================================================
#if(HUAYI_WY_TYPE == HUAYI_WY_TYPE_GENERAL)	//吸吊灯通用		有免对码功能
// 吸吊灯通用  	GPIO4悬空  3K  叠加功率 15%   有AC		WY相同
//          GPIO4下拉  16K 叠恒功率   3%  无AC		WY互补
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		0					//RGB是否打开
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			0 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			0 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	0 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		0 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				0 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			4 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			0 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		0 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(30*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			800 // 开关灯加速最大时间 雷士的300  其他800
#define  LIGHT_SWITCH_BR_DEV		100	// 调光速度越小越快     雷士 60        其他100
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			1   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*15/100)
#define  LIGHT_CONSTANT_P1		0 // 1-恒定功率  【0-叠加功率】
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	16000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// 下拉后的最低亮度 3%
#define  LIGHT_CONSTANT_P2		1 // 【1-恒定功率】  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===============================================================
#elif(HUAYI_WY_TYPE == HUAYI_WY_TYPE_AQUARIUS)// 水瓶星和射手星    	有免对码功能
//华艺慧作水瓶星双色灯  Aquarius
/* 射手星和水瓶星    共用同一配置
 * GPIO4 悬空           3KHZ、非隔离、恒功率、15%、无AC检测、小夜灯为黄光15%
 * GPIO4 下拉10K:3Khz，W亮度3%~100%，色温0~100 无AC检测，叠加功率  W亮度Y色温
 * 短按亮度的亮度等级为1%、20%、50%、100%
 * 2019/11/26 新增宏  1开启  用于判断是不是使用 APP可调节1-100PWM做出相应的变化
 */
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		0					//RGB是否打开
#define  LIGHT_WY_WBR_YCT		1					//W调光Y调色
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			0 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			0 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	0 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		0 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				0 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			4 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			0 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		0 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(30*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			800 // 开关灯加速最大时间 雷士的300  其他800
#define  LIGHT_SWITCH_BR_DEV		100	// 调光速度越小越快     雷士 60        其他100
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			1   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*15/100)
#define  LIGHT_CONSTANT_P1		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		0 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	3000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// 下拉后的最低亮度 3%
#define  LIGHT_CONSTANT_P2		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//==============================================================================
//华艺慧作白羊星双色灯
#elif(HUAYI_WY_TYPE == HUAYI_WY_TYPE_ARIES)
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		0					//RGB是否打开
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			0 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			0 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	0 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		0 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				0 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			4 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			0 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		0 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(30*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			800 // 开关灯加速最大时间 雷士的300  其他800
#define  LIGHT_SWITCH_BR_DEV		100	// 调光速度越小越快     雷士 60        其他100
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			0   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*10/100)
#define  LIGHT_CONSTANT_P1		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	600
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*12/100)
#define  LIGHT_CONSTANT_P2		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		1 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================
//华艺慧作双鱼星双色灯
#elif(HUAYI_WY_TYPE == HUAYI_WY_TYPE_PISCE)
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		0					//RGB是否打开
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			0 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			0 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	0 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		0 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				0 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			4 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			0 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		0 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(30*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			800 // 开关灯加速最大时间 雷士的300  其他800
#define  LIGHT_SWITCH_BR_DEV		100	// 调光速度越小越快     雷士 60        其他100
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			0   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*10/100)
#define  LIGHT_CONSTANT_P1		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	16000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P2		1 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================
//华艺慧作室女星双色灯
#elif(HUAYI_WY_TYPE == HUAYI_WY_TYPE_VIRGO)
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		0					//RGB是否打开
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			0 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			0 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	0 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		0 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				0 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			4 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			0 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		0 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(30*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			800 // 开关灯加速最大时间 雷士的300  其他800
#define  LIGHT_SWITCH_BR_DEV		100	// 调光速度越小越快     雷士 60        其他100
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			0   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*10/100)
#define  LIGHT_CONSTANT_P1		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	16000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P2		1 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================
//华艺----
#elif(HUAYI_WY_TYPE == HUAYI_WY_TYPE_TAURUS)
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		0					//RGB是否打开
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			0 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			0 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	0 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		0 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				0 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			4 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			0 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		0 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(30*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			800 // 开关灯加速最大时间 雷士的300  其他800
#define  LIGHT_SWITCH_BR_DEV		100	// 调光速度越小越快     雷士 60        其他100
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			0   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*10/100)
#define  LIGHT_CONSTANT_P1		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	16000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P2		1 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================
//华艺
#elif(HUAYI_WY_TYPE == HUAYI_WY_TYPE_PEGASUS)
// GPIO4悬空  3K 10%-100%  有AC  同相输出    叠加功率
// GPIO4下拉  16K 3%-100%  无AC  互补输出        恒功率
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		0					//RGB是否打开
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			0 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			0 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	0 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		0 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				0 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			4 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			0 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		0 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(30*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			800 // 开关灯加速最大时间 雷士的300  其他800
#define  LIGHT_SWITCH_BR_DEV		100	// 调光速度越小越快     雷士 60        其他100
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			0   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*10/100)
#define  LIGHT_CONSTANT_P1		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	16000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P2		1 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================
//华艺水瓶星彩光灯泡
#elif(HUAYI_WY_TYPE == HUAYI_WYRGB_AUQARIUS)
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		1					//RGB是否打开
#define  LIGHT_WY_WBR_YCT		0					//W调光Y调色
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			0 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			1 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	0 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		1 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				0 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			4 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			0 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		0 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(30*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			300 // 开关灯加速最大时间 雷士的300  其他800
#define  LIGHT_SWITCH_BR_DEV		60	// 调光速度越小越快     雷士 60        其他100
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			0   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	8000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		1 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		0 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	3000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// 下拉后的最低亮度 3%
#define  LIGHT_CONSTANT_P2		1 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================

//===========================================================================
/*
雷------	士------	照------	明
雷------	士------	照------	明
雷------	士------	照------	明
*/
//===========================================================================

// 雷士智能彩光灯带
#elif(HUAYI_WY_TYPE == LEISHI_RGB_TYPE_1)
#define  LIGHT_REMOTE_LEISHI	0					//雷士专用遥控器
#define  LIGHT_WY_PWMON			0					//WY是否打开
#define  LIGHT_RGB_PWMON		1					//RGB是否打开
#define  LIGHT_WY_WBR_YCT		0					//W调光Y调色
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			0 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			1 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	0 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		1 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				1 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			8 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			1 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		1 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			300 // 开关灯加速最大时间 雷士的300  其他800
#define  LIGHT_SWITCH_BR_DEV		60	// 调光速度越小越快     雷士 60        其他100
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			0   // 是否支持产测遥控器免对码
#define  LIGHT_REPOWER_SAVE			1	// 上电记忆有分段

//P1
#define  LIGHT_FRE_P1          	8000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		1 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		0 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	3000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// 下拉后的最低亮度 3%
#define  LIGHT_CONSTANT_P2		1 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================

//雷士智能彩光灯泡
#elif(HUAYI_WY_TYPE == LEISHI_RGB_TYPE_2)
#define  LIGHT_REMOTE_LEISHI	0					//雷士专用遥控器
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		1					//RGB是否打开
#define  LIGHT_WY_WBR_YCT		0					//W调光Y调色
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			0 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			1 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	0 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		1 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				1 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			8 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			1 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		1 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			300 // 开关灯加速最大时间 雷士的300  其他800
#define  LIGHT_SWITCH_BR_DEV		60	// 调光速度越小越快     雷士 60        其他100
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			0   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	8000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		1 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		0 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	3000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// 下拉后的最低亮度 3%
#define  LIGHT_CONSTANT_P2		1 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================

// 雷士智能WY灯
#elif(HUAYI_WY_TYPE == LEISHI_WY_TYPE_1)
#define  LIGHT_REMOTE_LEISHI	0					//雷士专用遥控器
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		0					//RGB是否打开
#define  LIGHT_WY_WBR_YCT		0					//W调光Y调色
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			0 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			0 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	0 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		1 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				1 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			8 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			1 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		1 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			300 // 开关灯加速最大时间 雷士的300  其他800
#define  LIGHT_SWITCH_BR_DEV		60	// 调光速度越小越快     雷士 60        其他100
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			0   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	600
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// 下拉后的最低亮度 3%
#define  LIGHT_CONSTANT_P2		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		1 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================

// 雷士智能WY灯-W调光Y调色
#elif(HUAYI_WY_TYPE == LEISHI_WY_TYPE_2)
#define  LIGHT_REMOTE_LEISHI	0					//雷士专用遥控器
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		0					//RGB是否打开
#define  LIGHT_WY_WBR_YCT		1					//W调光Y调色
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			0 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			0 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	0 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		1 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				1 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			8 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			1 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		1 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			300 // 开关灯加速最大时间 雷士的300  其他800
#define  LIGHT_SWITCH_BR_DEV		60	// 调光速度越小越快     雷士 60        其他100
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			0   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		1 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	600
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// 下拉后的最低亮度 3%
#define  LIGHT_CONSTANT_P2		1 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		1 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================
// 雷士智能面板灯-W调光Y调色-W调光Y调色
#elif(HUAYI_WY_TYPE == LEISHI_WY_TYPE_3)
#define  LIGHT_REMOTE_LEISHI	0					//雷士专用遥控器
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		0					//RGB是否打开
#define  LIGHT_WY_WBR_YCT		1					//W调光Y调色
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			1 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			1 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	1 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		1 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				1 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			8 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			1 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		1 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			300 // 开关灯加速最大时间 雷士的300  其他800
#define  LIGHT_SWITCH_BR_DEV		60	// 调光速度越小越快     雷士 60        其他100
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			0   // 是否支持产测遥控器免对码


//P1
#define  LIGHT_FRE_P1          	5000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*5/100)
#define  LIGHT_CONSTANT_P1		1 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	600
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*5/100)	// 下拉后的最低亮度 3%
#define  LIGHT_CONSTANT_P2		1 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		1 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21


//===========================================================================
/*
乐------	式------	照------	明
乐------	式------	照------	明
乐------	式------	照------	明
*/
//===========================================================================

#elif(HUAYI_WY_TYPE == LESHI_WY_TYPE)
//乐式双色灯
// 1、乐式标准小米情景双色灯WIFI 参数:
// IO4悬空                    :	3KHZ--WY--叠加功率--有AC--最低亮度10%--小夜灯为黄光10%--带分段(乐式标准)--遥控渐开--乐式对码方式
// IO4下拉电阻10k :	16KHZ--WY--恒功率--无AC--最低亮度3%--小夜灯为黄光10%--带分段(乐式标准)--遥控渐开--乐式对码方式
#define  LIGHT_WY_PWMON				1					// WY是否打开
#define  LIGHT_RGB_PWMON			0					// RGB是否打开
#define  LIGHT_BIT_DUTY				LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX				(unsigned int)0x0FFF//

//新增
#define  LIGHT_PROSET_SWITCH		1 // 专业设置
#define  LIGHT_NIGHT_SWITCH			1 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			1 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		1 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	1 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		1 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				1 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			8 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_RESET_CNT_LOCK		20// 快速开关N次-锁定后次数
#define  LIGHT_MATCH_CMDON			1 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		1 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			800 // 开关灯加速最大时间 雷士的300-其他800
#define  LIGHT_SWITCH_BR_DEV		100	// 雷士调光速度 60        其他速度100  越小越快
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			1   // 是否支持产测遥控器免对码

//P1 IO4悬空
#define  LIGHT_FRE_P1          		3000
#define  LIGHT_PWM_MIN_P1			(LIGHT_PWM_MAX*10/100)//悬空最低亮度10%
#define  LIGHT_CONSTANT_P1			0 					// 1-恒定功率  【0-叠加功率】
#define  LIGHT_AC_ENABLE_P1			1 					// 1-AC	 0-NOAC
//P2 IO4下拉电阻10k
#define  LIGHT_FRE_P2        		16000
#define  LIGHT_PWM_MIN_P2			(LIGHT_PWM_MAX*3/100)//下拉后的最低亮度 3%
#define  LIGHT_CONSTANT_P2			1 					// 【1-恒定功率】  0-叠加功率
#define  LIGHT_AC_ENABLE_P2			0 					// 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================
#elif(HUAYI_WY_TYPE == LESHI_WY_TYPE_QJ)
//乐式双色灯
// 1、乐式标准小米情景WIFI双色灯的--程序修改，
// IO4下拉电阻10k :模式修改成16KHZ ,3% ,无AC ,恒功率 版本  ，IO悬空参数不变 。
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		0					//RGB是否打开
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			0 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			0 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	0 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		1 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				1 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			8 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			1 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		1 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			800 // 开关灯加速最大时间 雷士的300-其他800
#define  LIGHT_SWITCH_BR_DEV		100	// 雷士调光速度 60        其他速度100  越小越快
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			0   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*10/100)
#define  LIGHT_CONSTANT_P1		0 // 1-恒定功率  【0-叠加功率】
#define  LIGHT_AC_ENABLE_P1		0 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	16000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// 下拉后的最低亮度 3%
#define  LIGHT_CONSTANT_P2		1 // 【1-恒定功率】  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================
#elif(HUAYI_WY_TYPE == RBE_WY_TYPE)
// 瑞柏尔智能灯
// 增加分段功能   重置次数8次
// IO4下拉电阻10k :模式修改成16KHZ ,3% ,无AC ,恒功率 版本  ，IO悬空参数不变 。
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		0					//RGB是否打开
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			1 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			0 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	1 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		1 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				1 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			8 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			1 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		1 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			800 // 开关灯加速最大时间 雷士的300  其他800
#define  LIGHT_SWITCH_BR_DEV		100	// 调光速度越小越快     雷士 60        其他100
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			1   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*10/100)
#define  LIGHT_CONSTANT_P1		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	16000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P2		1 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================
#elif(HUAYI_WY_TYPE == LESHI_WY_TYPE_DBS)
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		0					//RGB是否打开
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			1 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			1 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	1 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		1 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				1 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			8 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			1 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		1 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			800 // 开关灯加速最大时间 雷士的300-其他800
#define  LIGHT_SWITCH_BR_DEV		150	// 雷士调光速度 60        其他速度100  越小越快
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			1   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	1000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*4/100)	// 下拉后的最低亮度 3%
#define  LIGHT_CONSTANT_P2		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC
#define  LIGHT_PWM_REVERSEL_P2	1 //PWM反转

//SW
#define  LIGHT_SW_GPIO        	17

//===========================================================================
// 乐式WYRGB
#elif(HUAYI_WY_TYPE == LESHI_WYRGB_TYPE)
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		1					//RGB是否打开
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			0 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			1 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	0 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		1 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				1 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			8 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			1 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		1 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			800 // 开关灯加速最大时间 雷士的300-其他800
#define  LIGHT_SWITCH_BR_DEV		100	// 雷士调光速度 60        其他速度100  越小越快
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			1   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	8000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		1 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		0 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	3000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// 下拉后的最低亮度 3%
#define  LIGHT_CONSTANT_P2		1 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21
//===========================================================================
// 乐式彩光灯带
#elif(HUAYI_WY_TYPE == LESHI_WY_TYPE_RGB)
#define  LIGHT_WY_PWMON			0					//WY是否打开
#define  LIGHT_RGB_PWMON		1					//RGB是否打开
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			0 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			1 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	0 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		1 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				1 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			8 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			1 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		1 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			800 // 开关灯加速最大时间 雷士的300-其他800
#define  LIGHT_SWITCH_BR_DEV		100	// 雷士调光速度 60        其他速度100  越小越快
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			0   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	8000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		0 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	3000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// 下拉后的最低亮度 3%
#define  LIGHT_CONSTANT_P2		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21


//===========================================================================
// NIOYE全彩LED灯
#elif(HUAYI_WY_TYPE == NIOYE_WYRGB_TYPE)
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		1					//RGB是否打开
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			0 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			1 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	0 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		1 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				1 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			8 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			1 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		1 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			800 // 开关灯加速最大时间 雷士的300-其他800
#define  LIGHT_SWITCH_BR_DEV		100	// 雷士调光速度 60        其他速度100  越小越快
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			1   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	2000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*19/100)
#define  LIGHT_CONSTANT_P1		1 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		0 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	3000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// 下拉后的最低亮度 3%
#define  LIGHT_CONSTANT_P2		1 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21


//===========================================================================
// 乐式WY新模板
/*
1、使用乐式WiFi公版开发-三期公版UI开发 （支持模式功能，支持灵动开关，默认设置等功能）
2、程序参数：16KHz / 3000级、隔离、双色、恒功率、3%（PWM输出4mA)   有AC检测  。
3、GPIO4 下拉10K为：16KHz / 3000级、隔离、双色、叠加功率、3%（PWM输出4mA)   有AC检测
4、上电默认为开灯的。
*/
#elif(HUAYI_WY_TYPE == LESHI_WY_TYPE_WY0B02)
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		0					//RGB是否打开
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			1 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			1 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		1 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	1 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		1 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				1 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			8 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			1 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		1 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			800 // 开关灯加速最大时间 雷士的300-其他800
#define  LIGHT_SWITCH_BR_DEV		100	// 雷士调光速度 60        其他速度100  越小越快
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			0   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	16000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*2/100)//3%
#define  LIGHT_CONSTANT_P1		1 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	16000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*2/100)//3%
#define  LIGHT_CONSTANT_P2		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		1 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21
//===========================================================================
//欧帝灯饰
#elif(HUAYI_WY_TYPE == ODDS_WY_TYPE_WY0A01)//
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		0					//RGB是否打开
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			1 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			1 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	1 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		1 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				1 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			8 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			1 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		1 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			800 // 开关灯加速最大时间 雷士的300-其他800
#define  LIGHT_FADE_ONOFF_TIMER		1000
#define  LIGHT_SWITCH_BR_DEV		60	// 雷士调光速度 60        其他速度100  越小越快
#define  LIGHT_SWITCH_BR_DMI		2 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			1   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	800
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*4/100)	// 下拉后的最低亮度 3%
#define  LIGHT_CONSTANT_P2		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC
#define  LIGHT_PWM_REVERSEL_P2	1 //PWM反转

//SW
#define  LIGHT_SW_GPIO        	17

//===========================================================================
//皇的智能灯
#elif(HUAYI_WY_TYPE == HBC_WY_TYPE_WY0A01)//
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		0					//RGB是否打开
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			1 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			1 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	1 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		1 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				1 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			8 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			1 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		1 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			800 // 开关灯加速最大时间 雷士的300-其他800
#define  LIGHT_FADE_ONOFF_TIMER		1000
#define  LIGHT_SWITCH_BR_DEV		60	// 雷士调光速度 60        其他速度100  越小越快
#define  LIGHT_SWITCH_BR_DMI		2 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			1   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	800
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*4/100)	// 下拉后的最低亮度 3%
#define  LIGHT_CONSTANT_P2		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC
#define  LIGHT_PWM_REVERSEL_P2	1 //PWM反转

//SW
#define  LIGHT_SW_GPIO        	17

//===========================================================================
//诺曼尼情景色温灯+帅灯堡智能吸顶灯
#elif(HUAYI_WY_TYPE == NMN_WY_TYPE_WY0A02)//
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		0					//RGB是否打开
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			1 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			1 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	1 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		1 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				1 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			8 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			1 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		1 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			800 // 开关灯加速最大时间 雷士的300-其他800
#define  LIGHT_SWITCH_BR_DEV		150	// 雷士调光速度 60        其他速度100  越小越快
#define  LIGHT_FADE_ONOFF_TIMER		1000
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			1   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	1000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*4/100)	// 下拉后的最低亮度 3%
#define  LIGHT_CONSTANT_P2		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC
#define  LIGHT_PWM_REVERSEL_P2	1 //PWM反转

//SW
#define  LIGHT_SW_GPIO        	17

//===========================================================================
//广云物联
//广云物联
//广云物联
//广云物联
//广云物联
//广云物联
#elif(HUAYI_WY_TYPE == GUANGYUN_WY_TYPE_1)
#define  LIGHT_WY_PWMON			1					//WY是否打开
#define  LIGHT_RGB_PWMON		0					//RGB是否打开
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			0 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			0 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	0 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		0 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				1 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			8 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			1 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		1 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			1	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			800 // 开关灯加速最大时间 雷士的300-其他800
#define  LIGHT_SWITCH_BR_DEV		100	// 雷士调光速度 60        其他速度100  越小越快
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			0   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*3/100)
#define  LIGHT_CONSTANT_P1		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		0 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	3000
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// 下拉后的最低亮度 3%
#define  LIGHT_CONSTANT_P2		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		0 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21

//===========================================================================
#else
#define  LIGHT_WY_PWMON			1						//WY是否打开
#define  LIGHT_RGB_PWMON		0						//RGB是否打开
#define  LIGHT_BIT_DUTY			LEDC_TIMER_12_BIT
#define  LIGHT_PWM_MAX			(unsigned int)0x0FFF//

//新增
#define  LIGHT_NIGHT_SWITCH			1 // 起夜灯模式是否支持
#define  LIGHT_MODE_SWITCH			1 // 灯色温模式/RGB模式控制是否支持
#define  LIGHT_REMOTE_NETSWITCH		0 // 是否支持遥控器网络上报
#define  LIGHT_SMARTSWITCH_ENABLE	0 // 是否支持灵动开关
#define  LIGHT_POWER_DEFAULT		1 // 是否支持上电默认状态设置
#define  LIGHT_RE_POWER				1 // 分段功能  1打开 0关闭
#define  LIGHT_RESET_CNT			8 // 快速开关N次-WIFI进入配网 华艺的5次，其他8次
#define  LIGHT_MATCH_CMDON			1 // 是否支持-开灯/关灯键对码清码
#define  LIGHT_MATCH_CMD24G			1 // 是否支持-一键对码方式
#define  LIGHT_MATCH_CLEARALL		1 // 清码是否清除所有遥控器 1清除所有
#define  LIGHT_MATCH_TIMER			(6*1000/LIGNT_TIMER_TICK)//30S对码--华艺对码时间30S,其他对码时间12S
#define  LIGHT_FADE_ON				1	// 渐开or立开   1-渐  0-立
#define  LIGHT_FADE_SWITCH			0	// LIGHT_FADE_ON = 1时,APP是否具有渐变打开和关闭功能
#define  LIGHT_FADE_MAX  			300 // 开关灯加速最大时间 雷士的300  其他800
#define  LIGHT_SWITCH_BR_DEV		60	// 调光速度越小越快     雷士 60        其他100
#define  LIGHT_SWITCH_BR_DMI		1 	// 最小调光阀值
#define  LIGHT_SWITCH_TM_DEV		15	// 乐式色温速度
#define  LIGHT_REMOTE_TEST			0   // 是否支持产测遥控器免对码

//P1
#define  LIGHT_FRE_P1          	3000
#define  LIGHT_PWM_MIN_P1		(LIGHT_PWM_MAX*10/100)
#define  LIGHT_CONSTANT_P1		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P1		1 // 1-AC	 0-NOAC
//P2
#define  LIGHT_FRE_P2        	600
#define  LIGHT_PWM_MIN_P2		(LIGHT_PWM_MAX*3/100)	// 下拉后的最低亮度 3%
#define  LIGHT_CONSTANT_P2		0 // 1-恒定功率  0-叠加功率
#define  LIGHT_AC_ENABLE_P2		1 // 1-AC	 0-NOAC

//SW
#define  LIGHT_SW_GPIO        	21
#endif





//=====================================================================
//IO口
//AC监测引脚
#define FAN_PCNT_UNIT      		PCNT_UNIT_0
#define FAN_PCNT_H_LIM_VAL      10000
#define FAN_PCNT_L_LIM_VAL     -10000
#define FAN_PCNT_THRESH1_VAL    5000
#define FAN_PCNT_THRESH0_VAL   -5000
#define FAN_PCNT_INPUT_SIG_IO   22  // Pulse Input GPIO
#define FAN_PCNT_INPUT_CTRL_IO  23  // Control GPIO HIGH=count up, LOW=count down
//类型引脚
#define LIGHT_TYPE_GPIO1		4		// GPIO4 下拉
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
#define LIGHT_TEMPLATETRUE_MIN		3000	// 暖光
#define LIGHT_TEMPLATETRUE_MAX		6400	// 冷光
#define LIGHT_TEMPLATETRUE_MID		4700	// 中性光

#define LIGHT_BRIGHTNESS_MIN		10
#define LIGHT_BRIGHTNESS_MAX		1000

//RGB
#define LIGHT_RGB_FADECNT			7//

#define LIGHT_RGB_VMAX				255
#define LIGHT_RGB_VMIN				0

#define LIGHT_WY_NORMAL				0	//色温模式
#define LIGHT_RGB_NORMAL			1	//彩光模式
#define LIGHT_RGB_FADE				2	//七彩渐变
#define LIGHT_RGB_JUMP				3	//七彩跳变
#define LIGHT_CD_SUNDAY				4	//日光模式
#define LIGHT_CD_NIGHT				5	//月光模式
#define LIGHT_RGB_COLOR				6	//七彩跳变
#define LIGHT_CD_WARMTH				7	//温馨模式
#define LIGHT_CD_TVMODE				8	//电视模式
#define LIGHT_CD_READING			9	//阅读模式
#define LIGHT_CD_CPMPUTER			10	//电脑模式
#define LIGHT_CD_HOST				11	//会客模式
#define LIGHT_CD_ENTER				12	//娱乐模式
#define LIGHT_CD_WEAKUP				13	//清晨唤醒
#define LIGHT_CD_DUSK				14	//黄昏明亮
#define LIGHT_CD_SLEEP				15	//夜晚助眠

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
