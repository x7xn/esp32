/*
 * LIGHT_REMOTE_H_
 *
 *  Created on: 2019年4月18日
 *      Author: Administrator
 */

#ifndef LIGHT_REMOTE_H_
#define LIGHT_REMOTE_H_

#include "stdint.h"
#include "miio_instance.h"
#include "miio_arch.h"
#include "jsmi.h"

#define WIFI_RESET_POWER	1//5次快速开关重置
#define WIFI_RESET_APP		2//APP删除重置
#define WIFI_RESET_WAIT		3//重启状态，不允许调光，预防闪烁

#define POWER_STATE_ON		1
#define POWER_STATE_SAVE	0

#define SYS_PONSETBIT       0x01/* Section switch */

//#define AIRCONDITION_DEFAULT_NAME		"name"/*空调name属性默认值*/


#pragma pack(1)

typedef struct _remote_match_t
{
	uint8_t  use;
	uint8_t	 lid;
	uint32_t rid;
	uint8_t  type;
	uint8_t  num;
} remote_match_t;


typedef struct _remote_task_t
{
	uint16_t        mLight_TestTimer;
	uint8_t			mLight_test;
	uint8_t			mLight_save;//保存上电状态
	uint8_t			mLight_night;//起夜灯模式
	uint8_t         mLight_online;
	uint8_t 		mLight_mode;
	uint16_t 		mLight_powercnt;
	uint16_t 		mLight_powertim;
	uint16_t 		mLight_matchtimer;
	uint16_t 		mLight_showtimr;
	uint16_t 		mLight_flashcnt;
	uint16_t 		mLight_fadecnt;
	uint16_t	    mLight_fadetimer;//灯渐变时间
	uint32_t		mLight_fadedelay;
	uint32_t		mLight_SwCloseTimer;
	uint32_t		mLight_TimerOff;
	uint32_t		mLight_ModeTimer;
	uint16_t		mLight_PWMmin;
	uint8_t			mLight_ConstantPower;  // 1-恒定功率  0-叠加功率
	uint8_t			mLight_ACenable;


	uint8_t			mRemote_LeiShi;
	uint8_t			mRemote_message;
	uint8_t			mRemote_match;
	uint16_t		mRemote_stimer;
	uint8_t			mRemote_matchcnt;
	uint8_t			mRemote_clearcnt;
	uint8_t			mRemote_CmdMode;				//  0->清除调光
	uint8_t			mRemote_CmdUp;
	uint8_t         mRemote_CmdFade;
	uint8_t			mRemote_SaveUp;
	uint16_t        mRemote_CmdDelay;
	uint16_t		mRemote_CycTimer;
	uint16_t		mRemote_StateTime;
	uint16_t		mRemote_KeyTime;
	uint8_t			mRemote_UpOff;
	uint8_t			mRemote_OneKeySence;
	uint8_t			mRemote_MacUpdate;

	uint8_t			mRGB_cnt;

	uint8_t         Other;							// 开关分段 -->SYS_PONSETBIT
	uint16_t	    mUpdateTimer;
	uint8_t			pX;
} remote_task_t;

typedef struct _aircondition_remote_task_t
{
	uint8_t			m_wifi_is_config_f;//是否已配网
	uint8_t         mLight_online;//在线状态
	uint8_t 		mLight_mode;//当前状态
	uint16_t 		mLight_powercnt;//快速开关计数
	uint16_t 		mLight_powertim;//快速上电时间

	uint16_t		mRemote_stimer;//存储更改计时器
	uint16_t		mRemote_SaveUp_f;//
	uint16_t	    mpPassiveUpdateTimer;//上报数据至平台的计时器
	
	//uint16_t	    mpInitiativeUpdateTimer;//上报数据至平台的计时器
} aircondition_remote_task_t;

#pragma pack()

void light_remote_updateCmd(uint8_t cmd,uint8_t per1,uint8_t per2,uint8_t per3);
void light_remote_appset(uint32_t remoteid);
void light_remote_update(void);
void light_remote_start(void);
void light_remote_task(void);
void RePowerOnInit(void);		// 墙壁开关分段

void update_service_aircondition_sw(uint8_t sw);
void update_service_aircondition_fanSpeed(uint8_t fan_speed);
void update_service_aircondition_mode(uint8_t mode);
void update_service_aircondition_SetTemp(uint8_t set_temp);
void update_service_aircondition_name(uint8_t aircondition_index,char* name,uint32_t name_len);
void update_service_aircondition_notifyname(bool notify_flag);
void update_service_aircondition_group(uint8_t group_value);

int get_service_aircondition_group(void);
int get_service_aircondition_mode(void);
int get_service_aircondition_temp(void);
bool get_service_aircondition_sw(void);
int get_service_aircondition_fanspeed(void);
int get_service_aircondition_fault(void);
char* get_service_aircondition_name(uint8_t aircondition_index);
bool get_notify_name_flag(void);
int get_service_aircondition_num(void);

uint8_t get_wifi_connect_status(void);
uint8_t get_allow_net_status(void);
void enable_connect_net(void);

void aircondition_remote_start(void);
void aircondition_remote_task(void);


void update_aircondition_status_for_notify(aircondition_device_infor_t* aircondition_info,uint8_t aircondition_index);
void update_aircondition_connect_for_save(aircondition_device_infor_t* aircondition_info,uint8_t aircondition_count);


#endif /* LIGHT_REMOTE_H_ */
