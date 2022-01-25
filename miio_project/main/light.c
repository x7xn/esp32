/******************************************************************************
 * light function used here.
 *
 * FileName: light.c
 *
 * Description: entry file of user application
 *
 *
 * Time: 2019.4.
 *
*******************************************************************************/


/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include "miio_net.h"
//#include "../../esp-idf/components/freertos/include/freertos/semaphore.h"
#if MIBLE_ENABLE
#include "mible_gateway.h"
#include "adv_beacon.h"
#endif

#include "LeCmd.h"
#include "LetsNetwork.h"
#include "Lets_DevPublish.h"
#include "light_drivers.h"
#include "light_remote.h"
#include "light.h"
#include "light_scen.h"
#include "device/handler/S_2_Light_doChange.h"

arch_os_thread_handle_t light_handle;

SemaphoreHandle_t xSemaphore = NULL;

lewyfan_device_infor_t lewyfan_infor=
{
		.light_sw		    = 1,
		.light_brightness   = LIGHT_BRIGHTNESS_MAX,
		.light_templatetrue = 4700,

		.light_rcolor = LIGHT_RGB_VMAX,
		.light_gcolor = LIGHT_RGB_VMAX,
		.light_bcolor = LIGHT_RGB_VMAX,
#if(LIGHT_WY_PWMON)
		.light_rgbmode= LIGHT_WY_NORMAL,
#elif(LIGHT_RGB_PWMON)
		.light_rgbmode= LIGHT_RGB_NORMAL,
#else
		.light_rgbmode= LIGHT_WY_NORMAL,
#endif
		.power_state = POWER_STATE_ON,
		.user_mode    = 0,// 上电设置状态
		.smart_switch = 0,// 凌动开关
		.remote_switch= 0,// 遥控器管理
		.fade_on  =LIGHT_FADE_DEFAULT,				// 渐变/立变
		.fade_off =LIGHT_FADE_DEFAULT,				// 渐变/立变
		.fade_scen=LIGHT_FADE_DEFAULT,				// 渐变/立变
		.lowest_brightness=0,						// 最低亮度
		.lock_state=0,								// 锁定操作0x01 锁定参数0x02

		.modetime = LIGHT_MODE_TIMER,
		.weakebr  = LIGHT_WEAK_EBR,
		.sleepsbr = LIGHT_SLEEP_SBR,
};


lewyfan_device_userinfor_t lewyfan_userinfor=
{
		.light_brightness   = 1000,
		.light_templatetrue = 4700,

		.light_rcolor = LIGHT_RGB_VMAX,
		.light_gcolor = LIGHT_RGB_VMAX,
		.light_bcolor = LIGHT_RGB_VMAX,
#if(LIGHT_WY_PWMON)
		.light_rgbmode= LIGHT_WY_NORMAL,
#elif(LIGHT_RGB_PWMON)
		.light_rgbmode= LIGHT_RGB_NORMAL,
#else
		.light_rgbmode= LIGHT_WY_NORMAL,
#endif
};

RemoteType	device_remote_rxpack;
uint8_t		device_remote_rxcnt=0;
uint8_t		device_remote_rxpst=0;
uint16_t    device_power_send = 0;


RF_BLE_PACK	device_network_rxpack;
uint8_t		device_network_rxcnt=0;
uint8_t		device_network_rxlen=0;

extern      remote_task_t   remote_sys;

uint8_t  TelinkRF_ReadPack(uint8_t *dat,uint8_t len,uint8_t *mac);
void 	 EncryptDecrypt(unsigned int idx, unsigned char *buf, unsigned char len);
void 	 EncryptDecrypt_leishi(unsigned int idx, unsigned char *buf, unsigned char len);


/*
 * 复位灯光参数
 * flag:
 * 0--系统首次上电
 * 1--APP删除
 * 2--8段重置
 */
void light_ResetData(uint8_t flag)
{
	lewyfan_infor.light_sw		     = 1;
	lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MAX;
	lewyfan_infor.light_templatetrue = 4700;

	lewyfan_infor.light_rcolor = LIGHT_RGB_VMAX;
	lewyfan_infor.light_gcolor = LIGHT_RGB_VMAX;
	lewyfan_infor.light_bcolor = LIGHT_RGB_VMAX;
#if(LIGHT_WY_PWMON)
	lewyfan_infor.light_rgbmode= LIGHT_WY_NORMAL;
#elif(LIGHT_RGB_PWMON)
	lewyfan_infor.light_rgbmode= LIGHT_RGB_NORMAL;
#else
	lewyfan_infor.light_rgbmode= LIGHT_WY_NORMAL;
#endif


	if(flag == 0)lewyfan_infor.lock_state = 0;
	else
	if(flag != 1)lewyfan_infor.lock_state  &= (~APP_LOCK_REMOTE);//清除遥控器+情景锁定

	if(flag == 0 || (lewyfan_infor.lock_state & APP_LOCK_PERSET) == 0)
	{
		lewyfan_infor.power_state = POWER_STATE_ON;
		lewyfan_infor.user_mode   = 0;
		lewyfan_infor.fade_on     = LIGHT_FADE_DEFAULT;
		lewyfan_infor.fade_off    = LIGHT_FADE_DEFAULT;
		lewyfan_infor.fade_scen   = LIGHT_FADE_DEFAULT;
		lewyfan_infor.lowest_brightness = 0;
		lewyfan_infor.smart_switch= 0;
		lewyfan_infor.remote_switch=0;

		lewyfan_infor.modetime = LIGHT_MODE_TIMER;
		lewyfan_infor.weakebr  = LIGHT_WEAK_EBR;
		lewyfan_infor.sleepsbr = LIGHT_SLEEP_SBR;
	}

	arch_psm_set_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
}


/*
 * 风扇灯初始化
 * miio_instance_create中调用
 */
void light_startinit(void)
{
	LOG_INFO_TAG("light","light_startinit\r\n");

	/* 底层驱动初始化 */
	light_driver_start();

	/* 灯状态部分初始化 */
	//light_remote_start();

	aircondition_remote_start();
	
	//light_scne_init();
}

/*
 * 风扇灯主进程
 */
static void* light_task(void* args)
{
	/* 创建互斥信号量 */
	xSemaphore = xSemaphoreCreateMutex();
	//vTaskStartScheduler();
	
    //miio_handle_t handle = (miio_handle_t) args;
	arch_printf("light_task memory: %u\r\n",esp_get_free_heap_size());
	
    while(1)
    {
    	//LOG_INFO("system memory: %u\r\n",esp_get_free_heap_size());
    	//LOG_INFO_TAG("light_task", "light_task 10ms...");
    	//light_remote_task();
		aircondition_remote_task();
		aircondition_driver_task();
    	//light_driver_task();
    	arch_os_ms_sleep(30);
    }

    return (void*)0;
}

/*
 * 风扇灯主进程启动
 */
int light_task_start(miio_handle_t handle)
{
    static arch_os_thread_handle_t lightTaskHandle;

	
    return arch_os_thread_create(&lightTaskHandle, "light_task", light_task, 4096, handle, ARCH_OS_PRIORITY_DEFAULT);
}



extern uint8_t 		Remote_update_light_sw;
extern uint32_t 	Remote_update_light_brightness;
extern uint32_t 	Remote_update_light_cmd;
#if(LIGHT_WY_PWMON)
extern uint32_t 	Remote_update_light_templatetrue;
#endif

extern uint8_t 	Remote_update_light_sleepmode;
extern uint8_t 	Remote_update_light_weakupmode;
extern uint8_t 	Remote_update_light_modetime;
extern uint8_t 	Remote_update_light_sleepsbr;
extern uint8_t 	Remote_update_light_weakebr;


#if(LIGHT_RGB_PWMON)
extern uint32_t 	Remote_update_light_rgbcolor;
#endif
extern uint8_t 		Remote_update_light_rgbmode;
extern uint8_t 		Remote_update_powerstate;
extern uint8_t 		Remote_update_usermode;

#if(LIGHT_FADE_ON == 1 && LIGHT_FADE_SWITCH == 1)
extern uint16_t		Remote_update_fadeon;
extern uint16_t		Remote_update_fadeoff;
extern uint16_t		Remote_update_fadescen;
#endif

#if(LIGHT_SMARTSWITCH_ENABLE == 1)
extern uint8_t 		Remote_update_smart_switch;
#endif

#if(LIGHT_NIGHT_SWITCH == 1)
extern uint8_t 		Remote_update_night_switch;
#endif

#if(LIGHT_REMOTE_NETSWITCH)
extern uint8_t 		Remote_update_remoteswitch;
#endif

#if(LIGHT_PROSET_SWITCH == 1)
uint32_t 	Remote_update_lowest_brightness;
uint32_t 	Remote_update_lock_state;
#endif

/*
 * 获取灯开关状态
 */
uint8_t light_getlight_sw(void)
{
	return lewyfan_infor.light_sw;
}

/*
 * 获取灯亮度状态
 */
unsigned int light_getlight_brightness(void)
{
	unsigned int temp = lewyfan_infor.light_brightness/10;
	if(temp < 1)temp = 1;
	else
	if(temp > 100)temp = 100;
	return (temp);
}

/*
 * 获取灯色温状态
 */
unsigned int light_getlight_templatetrue(void)
{
	return lewyfan_infor.light_templatetrue;
}

/*
 * 获取灯rgb状态
 */
unsigned int light_getlight_rgbcolor(void)
{
	unsigned int rgbcolor;

	rgbcolor  = lewyfan_infor.light_rcolor;
	rgbcolor<<=8;
	rgbcolor |= lewyfan_infor.light_gcolor;
	rgbcolor<<=8;
	rgbcolor |= lewyfan_infor.light_bcolor;

	return rgbcolor;
}
/*
 * 获取灯rgb mode状态
 */
unsigned int light_getlight_rgbmode(void)
{
	return lewyfan_infor.light_rgbmode;
}
/*
 * 获取灵动开关
 * */
unsigned int light_getlight_smartswitch(void)
{
	return lewyfan_infor.smart_switch;
}
//获取睡眠模式
uint8_t light_getlight_sleep(void)
{
	return (lewyfan_infor.light_rgbmode == LIGHT_CD_SLEEP);
}
//获取唤醒模式
uint8_t light_getlight_weakup(void)
{
	return (lewyfan_infor.light_rgbmode == LIGHT_CD_WEAKUP);
}
//获取渐变时间
unsigned int light_getlight_modetime(void)
{
	return lewyfan_infor.modetime;
}
//获取唤醒模式最终亮度
unsigned int light_getlight_weakebr(void)
{
	return lewyfan_infor.weakebr;
}
//获取助眠模式开始亮度
unsigned int light_getlight_sleepsbr(void)
{
	return lewyfan_infor.sleepsbr;
}
/*
 * 获取上电默认状态
 * */
unsigned int light_getlight_powerstate(void)
{
	return lewyfan_infor.power_state;
}

/*
 * 获取是否设置上电默认状态
 * */
unsigned int light_getlight_usermode(void)
{
	return lewyfan_infor.user_mode;
}

/*
 * 获取渐变开参数
 * */
unsigned int light_getlight_fadeon(void)
{
	return lewyfan_infor.fade_on;
}

/*
 * 获取渐变关参数
 * */
unsigned int light_getlight_fadeoff(void)
{
	return lewyfan_infor.fade_off;
}

/*
 * 获取情景切换参数
 * */
unsigned int light_getlight_fadescen(void)
{
	return lewyfan_infor.fade_scen;
}

/*
 * 获取起夜灯模式
 * */
unsigned int light_getlight_nightswitch(void)
{
	return remote_sys.mLight_night;
}

/*
 * 获取定时关参数
 * */
unsigned int light_getlight_timeroff(void)
{
	return (remote_sys.mLight_TimerOff*LIGNT_TIMER_TICK/1000/60);
}

/*
 * 获取最低亮度设置参数
 * */
unsigned int light_getlight_lowestbrightness(void)
{
	return lewyfan_infor.lowest_brightness;
}

/*
 * 获取锁定状态参数
 * */
unsigned int light_getlight_lockstate(void)
{
	return lewyfan_infor.lock_state;
}




/***************************************************************************************
 * *************************************************************************************
 * *************************************************************************************
 * *************************************************************************************
 * *************************************************************************************
 * *************************************************************************************
 * 设置灯开关状态
 *@param-sw[IN]:要设置的状态
 */
 #if 1
void light_setlight_sw(uint8_t sw)
{
	Remote_update_light_sw = (uint8_t)sw;//记录更新状态
	lewyfan_infor.light_sw    = sw;
	if(lewyfan_infor.light_sw)remote_sys.mLight_mode = M_LIGHT_ON;
	else					  remote_sys.mLight_mode = M_LIGHT_OFF;

	//支持起夜模式
#if(LIGHT_NIGHT_SWITCH)
	//起夜模式开启
	if(remote_sys.mLight_night)
	{
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MIN;//亮度设置为最低
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;//色温设置为最低
	#if(LIGHT_WY_PWMON == 1 && LIGHT_RGB_PWMON == 0)//WY
		lewyfan_infor.light_rgbmode = LIGHT_CD_NIGHT;
	#endif
	}
#endif

	if(lewyfan_infor.light_sw == 0)
	{
		light_driver_fade(0,
						  lewyfan_infor.light_templatetrue,
						  lewyfan_infor.light_rcolor,
						  lewyfan_infor.light_gcolor,
						  lewyfan_infor.light_bcolor);
	}else{
		light_driver_fade(lewyfan_infor.light_brightness,
						  lewyfan_infor.light_templatetrue,
						  lewyfan_infor.light_rcolor,
						  lewyfan_infor.light_gcolor,
						  lewyfan_infor.light_bcolor);
	}

#if(LIGHT_WY_PWMON == 1)
	if(lewyfan_infor.light_rgbmode == LIGHT_CD_WEAKUP ||
	   lewyfan_infor.light_rgbmode == LIGHT_CD_DUSK ||
	   lewyfan_infor.light_rgbmode == LIGHT_CD_SLEEP)
	{
		lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
	}
#endif
	remote_sys.mRemote_SaveUp   = 1;
	remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
}
#endif

/*
 * 设置灯亮度状态
 */
void light_setlight_brightness(unsigned int brightness)
{
#if(LIGHT_NIGHT_SWITCH)
	if(remote_sys.mLight_night)
	{
		remote_sys.mLight_night = 0;
		unsigned char savesw;
		savesw = lewyfan_infor.light_sw;
		arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
		lewyfan_infor.light_sw = savesw;
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp   = 1;
	}
#endif
#if(LIGHT_WY_PWMON == 1 && LIGHT_RGB_PWMON == 1)//WYRGB
	if(lewyfan_infor.light_rgbmode == LIGHT_RGB_NORMAL ||
	   lewyfan_infor.light_rgbmode == LIGHT_RGB_FADE ||
	   lewyfan_infor.light_rgbmode == LIGHT_RGB_COLOR ||
	   lewyfan_infor.light_rgbmode == LIGHT_RGB_JUMP)
	{
		lewyfan_infor.light_rgbmode = LIGHT_RGB_NORMAL;
		remote_sys.mRemote_SaveUp  = 1;
	}else{
		lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
		remote_sys.mRemote_SaveUp  = 1;
	}
#elif(LIGHT_WY_PWMON == 1 && LIGHT_RGB_PWMON == 0)//WY
	lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
	remote_sys.mRemote_SaveUp  = 1;
#else
	lewyfan_infor.light_rgbmode = LIGHT_RGB_NORMAL;
	remote_sys.mRemote_SaveUp  = 1;
#endif
	Remote_update_light_brightness = brightness*10;
	lewyfan_infor.light_brightness = brightness*10;
	if(lewyfan_infor.light_sw == 0)
	{
		light_driver_fade(0,
						  lewyfan_infor.light_templatetrue,
						  lewyfan_infor.light_rcolor,
						  lewyfan_infor.light_gcolor,
						  lewyfan_infor.light_bcolor);
	}else{
		light_driver_fade(lewyfan_infor.light_brightness,
						  lewyfan_infor.light_templatetrue,
						  lewyfan_infor.light_rcolor,
						  lewyfan_infor.light_gcolor,
						  lewyfan_infor.light_bcolor);
	}
	remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
}

/*
 * 设置灯色温状态
 */
void light_setlight_templatetrue(unsigned int templatetrue)
{
#if(LIGHT_WY_PWMON)
#if(LIGHT_NIGHT_SWITCH)
	if(remote_sys.mLight_night)
	{
		remote_sys.mLight_night = 0;
		unsigned char savesw;
		savesw = lewyfan_infor.light_sw;
		arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
		lewyfan_infor.light_sw = savesw;
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp   = 1;
	}
#endif
	Remote_update_light_templatetrue = templatetrue;
	remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
	if(lewyfan_infor.light_rgbmode != LIGHT_WY_NORMAL)
	{
		lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
		remote_sys.mRemote_SaveUp   = 1;
	}
	lewyfan_infor.light_templatetrue = templatetrue;
	if(lewyfan_infor.light_sw == 0)
	{
		light_driver_fade(0,
						  lewyfan_infor.light_templatetrue,
						  lewyfan_infor.light_rcolor,
						  lewyfan_infor.light_gcolor,
						  lewyfan_infor.light_bcolor);
	}else{
		light_driver_fade(lewyfan_infor.light_brightness,
						  lewyfan_infor.light_templatetrue,
						  lewyfan_infor.light_rcolor,
						  lewyfan_infor.light_gcolor,
						  lewyfan_infor.light_bcolor);
	}
#endif
}

/*
 * 设置灯RGB状态
 */
void light_setlight_rgbcolor(unsigned int rgbcolor)
{
#if(LIGHT_RGB_PWMON)

#if(LIGHT_NIGHT_SWITCH)
	if(remote_sys.mLight_night)
	{
		remote_sys.mLight_night = 0;
		unsigned char savesw;
		savesw = lewyfan_infor.light_sw;
		arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
		lewyfan_infor.light_sw = savesw;
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp   = 1;
	}
#endif
	if(rgbcolor == 0)return;
	if(lewyfan_infor.light_rgbmode != LIGHT_RGB_NORMAL)
	{
		lewyfan_infor.light_rgbmode = LIGHT_RGB_NORMAL;
		remote_sys.mRemote_SaveUp   = 1;
	}
	Remote_update_light_rgbcolor = rgbcolor;
	lewyfan_infor.light_rcolor = 0x0000ff&(rgbcolor>>16);
	lewyfan_infor.light_gcolor = 0x0000ff&(rgbcolor>>8);
	lewyfan_infor.light_bcolor = 0x0000ff&(rgbcolor>>0);

	remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
	if(lewyfan_infor.light_sw == 0)
	{
		light_driver_fade(0,
						  lewyfan_infor.light_templatetrue,
						  lewyfan_infor.light_rcolor,
						  lewyfan_infor.light_gcolor,
						  lewyfan_infor.light_bcolor);
	}else{
		light_driver_fade(lewyfan_infor.light_brightness,
						  lewyfan_infor.light_templatetrue,
						  lewyfan_infor.light_rcolor,
						  lewyfan_infor.light_gcolor,
						  lewyfan_infor.light_bcolor);
	}
#endif
}

/*
 * 设置灯RGB mode状态
 */
void light_setlight_rgbmode(unsigned int rgbcolor)
{
	uint8_t ret=0;

#if(LIGHT_NIGHT_SWITCH)
	if(remote_sys.mLight_night)
	{
		remote_sys.mLight_night = 0;
		unsigned char savesw;
		savesw = lewyfan_infor.light_sw;
		arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
		lewyfan_infor.light_sw = savesw;
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp   = 1;
	}
#endif
	Remote_update_light_rgbmode = rgbcolor;
	lewyfan_infor.light_rgbmode = rgbcolor;
	remote_sys.mRemote_stimer   = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
	remote_sys.mRemote_SaveUp   = 1;
	switch(rgbcolor)
	{
	/*
	温馨模式：		色温：30% / 亮度：100%
	电视（影院）模式：色温：50% / 亮度：1%
	阅读模式：		色温：70% / 亮度：100%
	电脑模式：		色温：100% / 亮度：100%
	*/
	case LIGHT_WY_NORMAL://				0	//色温模式
	case LIGHT_RGB_NORMAL://			1	//彩光模式
		ret = 1;
		break;
	case LIGHT_RGB_COLOR:				// 彩光模式
		lewyfan_infor.light_sw         = 1;
		remote_sys.mLight_mode         = M_LIGHT_ON;
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX;
		remote_sys.mRemote_SaveUp   = 0;
		/* no break */
	case LIGHT_RGB_FADE://				2	//七彩渐变
		remote_sys.mRemote_SaveUp   = 0;
		break;
	case LIGHT_RGB_JUMP://				3	//七彩
		remote_sys.mRemote_SaveUp   = 0;
		break;
	case LIGHT_CD_SUNDAY://				4	//日光模式
		lewyfan_infor.light_sw           = 1;
		remote_sys.mLight_mode           = M_LIGHT_ON;
		lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MAX;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*9/10;
		ret = 1;
		break;
	case LIGHT_CD_NIGHT://				5	//月光模式
		lewyfan_infor.light_sw           = 1;
		remote_sys.mLight_mode         = M_LIGHT_ON;
		lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MIN;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
		ret = 1;
		break;
	case LIGHT_CD_WARMTH://				6	//温馨模式
		lewyfan_infor.light_sw         = 1;
		remote_sys.mLight_mode         = M_LIGHT_ON;
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*3/10;
		ret = 1;
		break;
	case LIGHT_CD_TVMODE://				7	//电视模式
		lewyfan_infor.light_sw         = 1;
		remote_sys.mLight_mode         = M_LIGHT_ON;
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX*1/100;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*5/10;
		ret = 1;
		break;
	case LIGHT_CD_READING://			8	//阅读模式
		lewyfan_infor.light_sw         = 1;
		remote_sys.mLight_mode         = M_LIGHT_ON;
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*7/10;
		ret = 1;
		break;
	case LIGHT_CD_CPMPUTER://			9	//电脑模式
		lewyfan_infor.light_sw         = 1;
		remote_sys.mLight_mode         = M_LIGHT_ON;
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN);
		ret = 1;
		break;
	case LIGHT_CD_HOST://				10	//会客模式
		lewyfan_infor.light_sw         = 1;
		remote_sys.mLight_mode         = M_LIGHT_ON;
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*5/10;
		ret = 1;
		break;
	case LIGHT_CD_ENTER://				11	//娱乐模式
		lewyfan_infor.light_sw         = 1;
		remote_sys.mLight_mode         = M_LIGHT_ON;
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX/2;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*3/10;
		ret = 1;
		break;
	case LIGHT_CD_WEAKUP://				12	//清晨唤醒 30分钟  色温【 0%-90%】  亮度【1-95%】
		remote_sys.mLight_ModeTimer      = (1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK);
		lewyfan_infor.light_sw           = 1;
		remote_sys.mLight_mode           = M_LIGHT_ON;
		lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MIN;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
		ret = 1;
		break;
	case LIGHT_CD_DUSK://				13	//黄昏明亮 30分钟  色温【 50%-100%】  亮度【1-100%】
		remote_sys.mLight_ModeTimer      = (1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK);
		lewyfan_infor.light_sw           = 1;
		remote_sys.mLight_mode           = M_LIGHT_ON;
		lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MIN;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*5/10;
		ret = 1;
		break;
	case LIGHT_CD_SLEEP://				14	//夜晚助眠30分钟  色温【 95%->0%】  亮度【95-0%】
		remote_sys.mLight_ModeTimer      = (1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK);
		lewyfan_infor.light_sw           = 1;
		remote_sys.mLight_mode           = M_LIGHT_ON;
		lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MAX*lewyfan_infor.sleepsbr/100;
		if(lewyfan_infor.light_brightness == 0)lewyfan_infor.light_brightness=1;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*95/100;
		ret = 1;
		break;

	default:
		ret = 1;
		break;
	}

	if(ret)
	{
		if(lewyfan_infor.light_sw == 0)
		{
			light_driver_fade(0,
							  lewyfan_infor.light_templatetrue,
							  lewyfan_infor.light_rcolor,
							  lewyfan_infor.light_gcolor,
							  lewyfan_infor.light_bcolor);
		}else{
			light_driver_fade(lewyfan_infor.light_brightness,
							  lewyfan_infor.light_templatetrue,
							  lewyfan_infor.light_rcolor,
							  lewyfan_infor.light_gcolor,
							  lewyfan_infor.light_bcolor);
		}
	}
}

/*
 * 设置灵动开关
 */
void light_setlight_smartswitch(unsigned int state)
{
#if(LIGHT_SMARTSWITCH_ENABLE == 1)
	remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
	lewyfan_infor.smart_switch   = state;
	Remote_update_smart_switch   = state;
#endif
}

//睡眠模式
void light_setlight_sleep(uint8_t sw)
{
	if(sw)
	{
		Remote_update_light_sleepmode    = 1;
		if(lewyfan_infor.light_rgbmode != LIGHT_CD_SLEEP)
		{
			lewyfan_infor.light_rgbmode      = LIGHT_CD_SLEEP;
			remote_sys.mLight_ModeTimer      = (1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK);
			lewyfan_infor.light_sw           = 1;
			remote_sys.mLight_mode           = M_LIGHT_ON;
			lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MAX*lewyfan_infor.sleepsbr/100;
			if(lewyfan_infor.light_brightness == 0)lewyfan_infor.light_brightness=1;
			lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*95/100;
		}
	}else{
		if(lewyfan_infor.light_rgbmode == LIGHT_CD_SLEEP)
		{
			lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
		}
		Remote_update_light_sleepmode = 0;
	}

	remote_sys.mRemote_stimer   = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
	remote_sys.mRemote_SaveUp   = 1;
}
//唤醒模式
void light_setlight_weakup(uint8_t sw)
{
	if(sw)
	{
		Remote_update_light_weakupmode   = 1;
		if(lewyfan_infor.light_rgbmode != LIGHT_CD_WEAKUP)
		{
			lewyfan_infor.light_rgbmode      = LIGHT_CD_WEAKUP;
			remote_sys.mLight_ModeTimer      = (1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK);
			lewyfan_infor.light_sw           = 1;
			remote_sys.mLight_mode           = M_LIGHT_ON;
			lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MIN;
			lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
		}
	}
	else
	{
		Remote_update_light_weakupmode  = 0;
		if(lewyfan_infor.light_rgbmode == LIGHT_CD_WEAKUP)
		{
			lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
		}
	}
	remote_sys.mRemote_stimer   = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
	remote_sys.mRemote_SaveUp   = 1;
}
//模式时间
void light_setlight_modetime(unsigned int state)
{
	LOG_INFO("light_setlight_modetime: %d\n", state);
	if(lewyfan_infor.light_rgbmode == LIGHT_CD_WEAKUP || lewyfan_infor.light_rgbmode == LIGHT_CD_SLEEP)
	{
		lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
		remote_sys.mRemote_stimer   = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp   = 1;
	}
	Remote_update_light_modetime = state;
	lewyfan_infor.modetime = state;
}
//唤醒最终亮度
void light_setlight_weakebr(unsigned int state)
{
	LOG_INFO("light_setlight_weakebr: %d\n", state);
	if(lewyfan_infor.light_rgbmode == LIGHT_CD_WEAKUP)
	{
		lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
		remote_sys.mRemote_stimer   = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp   = 1;
	}
	if(state >= LIGHT_BRIGHTNESS_MIN)
	{
		lewyfan_infor.weakebr = state;
		Remote_update_light_weakebr = state;
	}
}
//睡眠模式开始亮度
void light_setlight_sleepsbr(unsigned int state)
{
	LOG_INFO("light_setlight_sleepsbr: %d\n", state);
	if(lewyfan_infor.light_rgbmode == LIGHT_CD_SLEEP)
	{
		lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
		remote_sys.mRemote_stimer   = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp   = 1;
	}
	if(state >= LIGHT_BRIGHTNESS_MIN)
	{
		lewyfan_infor.sleepsbr = state;
		Remote_update_light_sleepsbr = state;
	}
}

/*
 * 设置灯上电状态
 */
void light_setlight_powerstate(unsigned int state)
{
	Remote_update_powerstate   = state;
	remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
	lewyfan_infor.power_state = state;
}

/*
 * 设置灯
 */
void light_setlight_usermode(unsigned int state)
{
	if(lewyfan_infor.light_sw == 0)return;
	if(lewyfan_infor.power_state == POWER_STATE_ON)
	{
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
		lewyfan_infor.user_mode   = state;
		Remote_update_usermode    = state;
		if(state)
		{
			remote_sys.mLight_flashcnt = SHOW_CNT1;
			lewyfan_userinfor.light_brightness 	 = lewyfan_infor.light_brightness;
			lewyfan_userinfor.light_templatetrue = lewyfan_infor.light_templatetrue;
			lewyfan_userinfor.light_rcolor 		 = lewyfan_infor.light_rcolor;
			lewyfan_userinfor.light_gcolor 		 = lewyfan_infor.light_gcolor;
			lewyfan_userinfor.light_bcolor 		 = lewyfan_infor.light_bcolor;
			lewyfan_userinfor.light_rgbmode 	 = lewyfan_infor.light_rgbmode;
			arch_psm_set_value(LIGHT_SAVE_NAME, LIGHT_KEY_USER, (void*)&lewyfan_userinfor, sizeof(lewyfan_userinfor));
		}
	}
}


/*
 * 设置渐变开
 */
void light_setlight_fadeon(unsigned int state)
{
#if(LIGHT_FADE_ON == 1 && LIGHT_FADE_SWITCH == 1)
	remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
	lewyfan_infor.fade_on   = state;
	Remote_update_fadeon    = state;
#endif
}

/*
 * 设置渐变关
 */
void light_setlight_fadeoff(unsigned int state)
{
#if(LIGHT_FADE_ON == 1 && LIGHT_FADE_SWITCH == 1)
	remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
	lewyfan_infor.fade_off   = state;
	Remote_update_fadeoff    = state;
#endif
}

/*
 * 设置渐变情景
 */
void light_setlight_fadescen(unsigned int state)
{
#if(LIGHT_FADE_ON == 1 && LIGHT_FADE_SWITCH == 1)
	remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
	lewyfan_infor.fade_scen   = state;
	Remote_update_fadescen    = state;
#endif
}


/*
 * 设置起夜灯模式
 */
void light_setlight_nightswitch(unsigned int state)
{
#if(LIGHT_NIGHT_SWITCH == 1)
	remote_sys.mLight_night   = state;
	Remote_update_night_switch= state;
	if(state == 0)
	{
		unsigned char savesw;
		savesw = lewyfan_infor.light_sw;
		arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
		lewyfan_infor.light_sw = savesw;
		/*
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp   = 1;
		if(lewyfan_infor.light_sw)
		{
			light_driver_fade(lewyfan_infor.light_brightness,
							  lewyfan_infor.light_templatetrue,
							  lewyfan_infor.light_rcolor,
							  lewyfan_infor.light_gcolor,
							  lewyfan_infor.light_bcolor);
		}
		*/
	}
#endif
}

/*
 * 设置定时关
 */
void light_setlight_timeroff(unsigned int state)
{
	if(lewyfan_infor.light_sw)
	{
		remote_sys.mLight_TimerOff = state*60*1000/LIGNT_TIMER_TICK;
	}else{
		lewyfan_infor.light_sw = 1;
		remote_sys.mLight_TimerOff = state*60*1000/LIGNT_TIMER_TICK;

		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp   = 1;
		if(lewyfan_infor.light_sw == 0)
		{
			light_driver_fade(0,
							  lewyfan_infor.light_templatetrue,
							  lewyfan_infor.light_rcolor,
							  lewyfan_infor.light_gcolor,
							  lewyfan_infor.light_bcolor);
		}else{
			light_driver_fade(lewyfan_infor.light_brightness,
							  lewyfan_infor.light_templatetrue,
							  lewyfan_infor.light_rcolor,
							  lewyfan_infor.light_gcolor,
							  lewyfan_infor.light_bcolor);
		}
	}
}

/*
 * 设置最低亮度
 */
void light_setlight_lowestbrightness(unsigned int state)
{
#if(LIGHT_FADE_ON == 1 && LIGHT_FADE_SWITCH == 1)
	remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
	lewyfan_infor.lowest_brightness  = state;
	Remote_update_lowest_brightness  = state;
	if(lewyfan_infor.lowest_brightness)
	{
		remote_sys.mLight_PWMmin = (LIGHT_PWM_MAX*lewyfan_infor.lowest_brightness)/LIGHT_BRIGHTNESS_MAX;
	}
	else
	{
		if(remote_sys.pX == 2)
			remote_sys.mLight_PWMmin = LIGHT_PWM_MIN_P2;
		else
			remote_sys.mLight_PWMmin = LIGHT_PWM_MIN_P1;
	}
#endif
}

/*
 * 设置锁定状态
 */
void light_setlight_lockstate(unsigned int state)
{
#if(LIGHT_FADE_ON == 1 && LIGHT_FADE_SWITCH == 1)
	remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
	lewyfan_infor.lock_state  = state;
	Remote_update_lock_state  = state;
#endif
}


const unsigned char RGB_SWITCH_TAB[7][3]={
		{0xFF,0x00,0x00},
		{0xFF,0x7F,0x00},
		{0xFF,0xFF,0x00},
		{0x00,0xFF,0x00},
		{0x00,0xFF,0xFF},
		{0x00,0x00,0xFF},
		{0x8B,0x00,0xFF}
};
//开关+rgb切换
void light_setlight_onrgbsw(uint8_t sw)
{
	unsigned int i = 0;
#if(LIGHT_NIGHT_SWITCH)
	if(remote_sys.mLight_night)
	{
		remote_sys.mLight_night = 0;
		unsigned char savesw;
		savesw = lewyfan_infor.light_sw;
		arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
		lewyfan_infor.light_sw = savesw;
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp   = 1;
	}
#endif
	if(lewyfan_infor.light_rgbmode != LIGHT_RGB_NORMAL)
	{
		lewyfan_infor.light_rgbmode = LIGHT_RGB_NORMAL;
		remote_sys.mRemote_SaveUp   = 1;
	}
	if(lewyfan_infor.light_sw)
	{
		for(i=0;i<7;i++)
		{
			if(lewyfan_infor.light_rcolor == RGB_SWITCH_TAB[i][0] &&
			   lewyfan_infor.light_gcolor == RGB_SWITCH_TAB[i][1] &&
			   lewyfan_infor.light_bcolor == RGB_SWITCH_TAB[i][2])
			{
				break;
			}
		}
		i++;
		if(i>=7)i=0;
		lewyfan_infor.light_rcolor = RGB_SWITCH_TAB[i][0];
		lewyfan_infor.light_gcolor = RGB_SWITCH_TAB[i][1];
		lewyfan_infor.light_bcolor = RGB_SWITCH_TAB[i][2];

		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp  = 1;
	}else{
		lewyfan_infor.light_sw = 1;
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp  = 1;
	}
	if(lewyfan_infor.light_sw == 0)
	{
		light_driver_fade(0,
						  lewyfan_infor.light_templatetrue,
						  lewyfan_infor.light_rcolor,
						  lewyfan_infor.light_gcolor,
						  lewyfan_infor.light_bcolor);
	}else{
		light_driver_fade(lewyfan_infor.light_brightness,
						  lewyfan_infor.light_templatetrue,
						  lewyfan_infor.light_rcolor,
						  lewyfan_infor.light_gcolor,
						  lewyfan_infor.light_bcolor);
	}
}

//RGB切换
void light_setlight_rgbsw(uint8_t sw)
{
	unsigned int i;
#if(LIGHT_NIGHT_SWITCH)
	if(remote_sys.mLight_night)
	{
		unsigned char savesw;
		savesw = lewyfan_infor.light_sw;
		arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
		lewyfan_infor.light_sw = savesw;
		remote_sys.mLight_night = 0;
		remote_sys.mRemote_SaveUp  = 1;
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
	}
#endif
	if(lewyfan_infor.light_sw)
	{
		if(lewyfan_infor.light_rgbmode != LIGHT_RGB_NORMAL)
		{
			lewyfan_infor.light_rgbmode = LIGHT_RGB_NORMAL;
			remote_sys.mRemote_SaveUp   = 1;
		}

		for(i=0;i<7;i++)
		{
			if(lewyfan_infor.light_rcolor == RGB_SWITCH_TAB[i][0] &&
			   lewyfan_infor.light_gcolor == RGB_SWITCH_TAB[i][1] &&
			   lewyfan_infor.light_bcolor == RGB_SWITCH_TAB[i][2])
			{
				break;
			}
		}
		i++;
		if(i>=7)i=0;
		lewyfan_infor.light_rcolor = RGB_SWITCH_TAB[i][0];
		lewyfan_infor.light_gcolor = RGB_SWITCH_TAB[i][1];
		lewyfan_infor.light_bcolor = RGB_SWITCH_TAB[i][2];

		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp  = 1;
		if(lewyfan_infor.light_sw == 0)
		{
			light_driver_fade(0,
							  lewyfan_infor.light_templatetrue,
							  lewyfan_infor.light_rcolor,
							  lewyfan_infor.light_gcolor,
							  lewyfan_infor.light_bcolor);
		}else{
			light_driver_fade(lewyfan_infor.light_brightness,
							  lewyfan_infor.light_templatetrue,
							  lewyfan_infor.light_rcolor,
							  lewyfan_infor.light_gcolor,
							  lewyfan_infor.light_bcolor);
		}
	}
}


//开关+色温切换
void light_setlight_ontmsw(uint8_t sw)
{
#if(LIGHT_NIGHT_SWITCH)
	if(remote_sys.mLight_night)
	{
		unsigned char savesw;
		savesw = lewyfan_infor.light_sw;
		arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
		lewyfan_infor.light_sw = savesw;
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp   = 1;
		remote_sys.mLight_night = 0;
	}
#endif
	if(lewyfan_infor.light_sw)
	{
		if(lewyfan_infor.light_rgbmode != LIGHT_WY_NORMAL)
		{
			lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
			remote_sys.mRemote_SaveUp   = 1;
		}
		if(lewyfan_infor.light_templatetrue == LIGHT_TEMPLATETRUE_MAX)//白光-》黄光
		{
			lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
		}else
		if(lewyfan_infor.light_templatetrue == LIGHT_TEMPLATETRUE_MIN)//黄-》中
		{
			lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MID;
		}
		else
		{
			lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MAX;//中性光-》白光
		}
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp  = 1;
	}else{
		lewyfan_infor.light_sw = 1;
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp  = 1;
	}
	if(lewyfan_infor.light_sw == 0)
	{
		light_driver_fade(0,
						  lewyfan_infor.light_templatetrue,
						  lewyfan_infor.light_rcolor,
						  lewyfan_infor.light_gcolor,
						  lewyfan_infor.light_bcolor);
	}else{
		light_driver_fade(lewyfan_infor.light_brightness,
						  lewyfan_infor.light_templatetrue,
						  lewyfan_infor.light_rcolor,
						  lewyfan_infor.light_gcolor,
						  lewyfan_infor.light_bcolor);
	}
}

//色温切换
void light_setlight_tmsw(uint8_t sw)
{
#if(LIGHT_NIGHT_SWITCH)
	if(remote_sys.mLight_night)
	{
		remote_sys.mLight_night = 0;
		unsigned char savesw;
		savesw = lewyfan_infor.light_sw;
		arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
		lewyfan_infor.light_sw = savesw;
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp   = 1;
	}
#endif
	if(lewyfan_infor.light_sw)
	{
		if(lewyfan_infor.light_rgbmode != LIGHT_WY_NORMAL)
		{
			lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
			remote_sys.mRemote_SaveUp   = 1;
		}
		if(lewyfan_infor.light_templatetrue == LIGHT_TEMPLATETRUE_MAX)//白光-》黄光
		{
			lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
		}else
		if(lewyfan_infor.light_templatetrue == LIGHT_TEMPLATETRUE_MIN)//黄-》中
		{
			lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MID;
		}
		else
		{
			lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MAX;//中性光-》白光
		}
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp  = 1;
		if(lewyfan_infor.light_sw == 0)
		{
			light_driver_fade(0,
							  lewyfan_infor.light_templatetrue,
							  lewyfan_infor.light_rcolor,
							  lewyfan_infor.light_gcolor,
							  lewyfan_infor.light_bcolor);
		}else{
			light_driver_fade(lewyfan_infor.light_brightness,
							  lewyfan_infor.light_templatetrue,
							  lewyfan_infor.light_rcolor,
							  lewyfan_infor.light_gcolor,
							  lewyfan_infor.light_bcolor);
		}
	}
}
//色温-%
void light_setlight_tmdec(uint8_t sw)
{
#if(LIGHT_NIGHT_SWITCH)
	if(remote_sys.mLight_night)
	{
		remote_sys.mLight_night = 0;
		unsigned char savesw;
		savesw = lewyfan_infor.light_sw;
		arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
		lewyfan_infor.light_sw = savesw;
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp   = 1;
	}
#endif
	if(lewyfan_infor.light_rgbmode != LIGHT_WY_NORMAL)
	{
		lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
		remote_sys.mRemote_SaveUp   = 1;
	}
	if(lewyfan_infor.light_sw && lewyfan_infor.light_templatetrue > LIGHT_TEMPLATETRUE_MIN)
	{
		if(lewyfan_infor.light_templatetrue > ((LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)/10))
		{
			lewyfan_infor.light_templatetrue -= ((LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)/10);
			if(lewyfan_infor.light_templatetrue < LIGHT_TEMPLATETRUE_MIN)
				lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
		}else{
			lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
		}
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp  = 1;
		if(lewyfan_infor.light_sw == 0)
		{
			light_driver_fade(0,
							  lewyfan_infor.light_templatetrue,
							  lewyfan_infor.light_rcolor,
							  lewyfan_infor.light_gcolor,
							  lewyfan_infor.light_bcolor);
		}else{
			light_driver_fade(lewyfan_infor.light_brightness,
							  lewyfan_infor.light_templatetrue,
							  lewyfan_infor.light_rcolor,
							  lewyfan_infor.light_gcolor,
							  lewyfan_infor.light_bcolor);
		}
	}
}
//色温++
void light_setlight_tmadd(uint8_t sw)
{
#if(LIGHT_NIGHT_SWITCH)
	if(remote_sys.mLight_night)
	{
		remote_sys.mLight_night = 0;
		unsigned char savesw;
		savesw = lewyfan_infor.light_sw;
		arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
		lewyfan_infor.light_sw = savesw;
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp   = 1;
	}
#endif
	if(lewyfan_infor.light_rgbmode != LIGHT_WY_NORMAL)
	{
		lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
		remote_sys.mRemote_SaveUp   = 1;
	}
	if(lewyfan_infor.light_sw)
	{
		if(lewyfan_infor.light_templatetrue < LIGHT_TEMPLATETRUE_MAX)
		{
			if((lewyfan_infor.light_templatetrue + ((LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)/10)) > LIGHT_TEMPLATETRUE_MAX)
			{
				lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MAX;
			}else{
				lewyfan_infor.light_templatetrue += ((LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)/10);
			}
			remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
			remote_sys.mRemote_SaveUp  = 1;
			if(lewyfan_infor.light_sw == 0)
			{
				light_driver_fade(0,
								  lewyfan_infor.light_templatetrue,
								  lewyfan_infor.light_rcolor,
								  lewyfan_infor.light_gcolor,
								  lewyfan_infor.light_bcolor);
			}else{
				light_driver_fade(lewyfan_infor.light_brightness,
								  lewyfan_infor.light_templatetrue,
								  lewyfan_infor.light_rcolor,
								  lewyfan_infor.light_gcolor,
								  lewyfan_infor.light_bcolor);
			}
		}
	}
}


//开关+亮度切换=最低<<>>最高
void light_setlight_onbrsw(uint8_t sw)
{
#if(LIGHT_NIGHT_SWITCH)
	if(remote_sys.mLight_night)
	{
		remote_sys.mLight_night = 0;
		unsigned char savesw;
		savesw = lewyfan_infor.light_sw;
		arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
		lewyfan_infor.light_sw = savesw;
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp   = 1;
	}
#endif
	if(lewyfan_infor.light_sw)
	{
		lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
		if(lewyfan_infor.light_brightness < (LIGHT_BRIGHTNESS_MAX/5))
		{
			lewyfan_infor.light_brightness = (LIGHT_BRIGHTNESS_MAX/5);
		}
		else
		if(lewyfan_infor.light_brightness < (LIGHT_BRIGHTNESS_MAX/2))
		{
			lewyfan_infor.light_brightness = (LIGHT_BRIGHTNESS_MAX/2);
		}
		else
		if(lewyfan_infor.light_brightness < (LIGHT_BRIGHTNESS_MAX))
		{
			lewyfan_infor.light_brightness = (LIGHT_BRIGHTNESS_MAX);
		}
		else{
			lewyfan_infor.light_brightness = (LIGHT_BRIGHTNESS_MIN);
		}
		/*
		if(lewyfan_infor.light_brightness != LIGHT_BRIGHTNESS_MAX)
		{
			lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX;
		}else{
			lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MIN;
		}
		*/
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp  = 1;
	}else{
		lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
		lewyfan_infor.light_sw = 1;
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp  = 1;
	}
	if(lewyfan_infor.light_sw == 0)
	{
		light_driver_fade(0,
						  lewyfan_infor.light_templatetrue,
						  lewyfan_infor.light_rcolor,
						  lewyfan_infor.light_gcolor,
						  lewyfan_infor.light_bcolor);
	}else{
		light_driver_fade(lewyfan_infor.light_brightness,
						  lewyfan_infor.light_templatetrue,
						  lewyfan_infor.light_rcolor,
						  lewyfan_infor.light_gcolor,
						  lewyfan_infor.light_bcolor);
	}
}
//亮度切换=最低<<>>最高
void light_setlight_brsw(uint8_t sw)
{
#if(LIGHT_NIGHT_SWITCH)
	if(remote_sys.mLight_night)
	{
		remote_sys.mLight_night = 0;
		unsigned char savesw;
		savesw = lewyfan_infor.light_sw;
		arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
		lewyfan_infor.light_sw = savesw;
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp   = 1;
	}
#endif
	if(lewyfan_infor.light_sw)
	{
		if(lewyfan_infor.light_brightness != LIGHT_BRIGHTNESS_MAX)
		{
			lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX;
		}else{
			lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MIN;
		}
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp  = 1;
		if(lewyfan_infor.light_sw == 0)
		{
			light_driver_fade(0,
							  lewyfan_infor.light_templatetrue,
							  lewyfan_infor.light_rcolor,
							  lewyfan_infor.light_gcolor,
							  lewyfan_infor.light_bcolor);
		}else{
			light_driver_fade(lewyfan_infor.light_brightness,
							  lewyfan_infor.light_templatetrue,
							  lewyfan_infor.light_rcolor,
							  lewyfan_infor.light_gcolor,
							  lewyfan_infor.light_bcolor);
		}
	}
}


void aircondition_uartTest(void)
{
	uint8_t  send_data[9] = {0x01,0x25,0x25,0xFF,0xFF,0xFF,0x99,0xC0,0x8C};
	
	uart_write_bytes(UART_NUM_1,(const char *)send_data,9);

#if 0
	//arch_printf("UART1 TX:\r\n");
	uint8_t rxi;
	for(rxi=0 ;rxi < offsetcrc; rxi++)
	{
		//arch_printf("%02x ",send_data[rxi]);
	}
	//arch_printf("\r\n");
#endif

}

/*
unsigned int aircondition_get_fan(void)
{
	uint8_t result_code = get_service_aircondition_fanspeed(m_current_group);
	
	//arch_printf("aircondition_get_fan:%d,%d\r\n",m_current_group,result_code);
	
	if(result_code > 3)
	{
		result_code = 0;
	}
	
	return result_code;
}
*/
/*
unsigned int aircondition_get_sw(void)
{
	
	uint8_t result_code = get_service_aircondition_sw(m_current_group);
	//arch_printf("aircondition_get_sw:%d,%d\r\n",m_current_group,result_code);
	result_code = result_code?1:0;
	return (result_code);
}
*/
/*
unsigned int aircondition_get_mode(void)
{
	uint8_t result_code = get_service_aircondition_mode(m_current_group);
	
	//arch_printf("aircondition_get_mode:%d,%d\r\n",m_current_group,result_code);

	if(!result_code)
	{
		result_code = 1;
	}
	else if(result_code > 4)
	{
		result_code = 4;
	}
	
	return result_code;
}
unsigned int aircondition_get_fault(void)
{
	uint8_t result_code = get_service_aircondition_fault(m_current_group);
	//arch_printf("aircondition_get_fault:%d,%d\r\n",m_current_group,result_code);
	return (result_code);
}
unsigned int aircondition_get_temp(void)
{
	uint8_t result_code = get_service_aircondition_temp(m_current_group);
	//arch_printf("aircondition_get_temp:%d,%d\r\n",m_current_group,result_code);

	if(result_code < 16)
	{
		result_code = 16;
	}
	else if(result_code > 30)
	{
		result_code = 30;
	}
	
	return result_code;
}
unsigned int aircondition_get_group(void)
{
	return m_current_group;
}
unsigned int aircondition_get_notify_name_f(void)
{
	
	return (get_notify_name_flag());
}
*/
/*
unsigned int aircondition_get_connected_num(void)
{
	uint8_t result_code = get_service_aircondition_num();

	//arch_printf("get_num:%d\r\n",result_code);
	return result_code;
}
*/

/*
char* aircondition_get_groupname(uint8_t group_id)
{
	
	static char group_name[38] = {0} ;
	if(group_id > 15)
	{
		group_id = 0;
	}

	get_service_aircondition_name(group_name,group_id);
	//arch_printf("aircondition_get_name:%d,%s\r\n",group_id,group_name);
	return (group_name);
}
*/

#if 0
void aircondition_set_temp(unsigned int temp)
{
	
	//arch_printf("aircondition_set_temp:%d,%d\r\n",m_current_group,temp);
	update_service_aircondition_SetTemp(m_current_group,temp);
}
void aircondition_set_mode(unsigned int mode)
{
	
	//arch_printf("aircondition_set_mode:%d,%d\r\n",m_current_group,mode);
	//更新本地信息-用于存储和控制
	update_service_aircondition_mode(m_current_group,mode);
	//remote_sys.mRemote_SaveUp	= 1;
	//remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
}

#endif

void aircondition_set_sw_toggle(void)
{
	//获取旧状态
	uint8_t sw = get_service_aircondition_sw();

	//更新新状态
	sw = !sw;
	
	//更新本地信息-用于存储和控制
	update_service_aircondition_sw(sw);
	
	//arch_printf("aircondition_set_sw_toggle:%d,0x%02x\r\n",m_current_group,sw);
	//更新本地信息-用于存储和控制
}

#if 0
void aircondition_set_sw(uint8_t sw)
{
	
	//arch_printf("aircondition_set_sw:%d,0x%02x\r\n",m_current_group,sw);
	//更新本地信息-用于存储和控制
	update_service_aircondition_sw(m_current_group,sw);
}

void aircondition_set_fanspeed(unsigned int fanspeed)
{
	
	//arch_printf("aircondition_set_fanspeed:%d,%d\r\n",m_current_group,fanspeed);
	update_service_aircondition_fanSpeed(m_current_group,fanspeed);
}
void aircondition_set_group(uint8_t group_value)
{
	
	//arch_printf("aircondition_set_group:%d,%d\r\n",m_current_group,group_value);
	if(m_current_group != group_value)
	{
		m_current_group = group_value;
	}
	
	update_service_aircondition_group(m_current_group);//用于notify
}
#endif

void aircondition_set_notify_name(uint8_t group_value)
{
	
	//arch_printf("aircondition_set_notify_name:%d\r\n",group_value);
	update_service_aircondition_notifyname(group_value);
}

/*
void aircondition_set_groupname(uint8_t group_id,char* groupname,uint32_t groupname_len)
{
	
	//arch_printf("aircondition_set_name:%d,%s\r\n",group_id,(const char*)groupname);
	if(group_id > 15)
	{
		return;
	}
	
	update_service_aircondition_name(group_id,groupname,groupname_len);
	//
	//aircondition_driver_control();	
}
*/



//亮度-%
void light_setlight_brdec(uint8_t sw)
{
#if(LIGHT_NIGHT_SWITCH)
	if(remote_sys.mLight_night)
	{
		remote_sys.mLight_night = 0;
		unsigned char savesw;
		savesw = lewyfan_infor.light_sw;
		arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
		lewyfan_infor.light_sw = savesw;
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp   = 1;
	}
#endif
	if(lewyfan_infor.light_sw && lewyfan_infor.light_brightness > LIGHT_BRIGHTNESS_MIN)
	{

		if(lewyfan_infor.light_brightness > (LIGHT_BRIGHTNESS_MAX/10))
		{
			lewyfan_infor.light_brightness -= (LIGHT_BRIGHTNESS_MAX/10);
			if(lewyfan_infor.light_brightness < LIGHT_BRIGHTNESS_MIN)
				lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MIN;
		}else{
			lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MIN;
		}
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp  = 1;
		if(lewyfan_infor.light_sw == 0)
		{
			light_driver_fade(0,
							  lewyfan_infor.light_templatetrue,
							  lewyfan_infor.light_rcolor,
							  lewyfan_infor.light_gcolor,
							  lewyfan_infor.light_bcolor);
		}else{
			light_driver_fade(lewyfan_infor.light_brightness,
							  lewyfan_infor.light_templatetrue,
							  lewyfan_infor.light_rcolor,
							  lewyfan_infor.light_gcolor,
							  lewyfan_infor.light_bcolor);
		}
	}
}
//亮度++
void light_setlight_bradd(uint8_t sw)
{
#if(LIGHT_NIGHT_SWITCH)
	if(remote_sys.mLight_night)
	{
		remote_sys.mLight_night = 0;
		unsigned char savesw;
		savesw = lewyfan_infor.light_sw;
		arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
		lewyfan_infor.light_sw = savesw;
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp   = 1;
	}
#endif
	if(lewyfan_infor.light_sw)
	{
		if(lewyfan_infor.light_brightness < LIGHT_BRIGHTNESS_MAX)
		{
			if((lewyfan_infor.light_brightness + (LIGHT_BRIGHTNESS_MAX/10)) > LIGHT_BRIGHTNESS_MAX)
			{
				lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX;
			}else{
				lewyfan_infor.light_brightness += (LIGHT_BRIGHTNESS_MAX/10);
			}
			remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
			remote_sys.mRemote_SaveUp  = 1;
			if(lewyfan_infor.light_sw == 0)
			{
				light_driver_fade(0,
								  lewyfan_infor.light_templatetrue,
								  lewyfan_infor.light_rcolor,
								  lewyfan_infor.light_gcolor,
								  lewyfan_infor.light_bcolor);
			}else{
				light_driver_fade(lewyfan_infor.light_brightness,
								  lewyfan_infor.light_templatetrue,
								  lewyfan_infor.light_rcolor,
								  lewyfan_infor.light_gcolor,
								  lewyfan_infor.light_bcolor);
			}
		}
	}
}



#if 1
//开关切换
void light_setlight_onoff(uint8_t sw)
{
	remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
	remote_sys.mRemote_SaveUp  = 1;

	//当前状态为关
	if(lewyfan_infor.light_sw == 0)
	{
		//更新状态为开
		lewyfan_infor.light_sw = 1;
		remote_sys.mLight_mode = M_LIGHT_ON;
	}
	//当前状态为开
	else
	{
		//更新状态为关
		lewyfan_infor.light_sw = 0;
		remote_sys.mLight_mode = M_LIGHT_OFF;
	}

	//如果支持起夜模式
#if(LIGHT_NIGHT_SWITCH)
	if(remote_sys.mLight_night && lewyfan_infor.light_sw)//起夜模式开启并且当前灯的状态应设为 开
	{
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MIN;//将灯的亮度调整到最低
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;//将灯的色温调整到最低
	}
#endif

	//灯应设置为关闭
	if(lewyfan_infor.light_sw == 0)
	{
		light_driver_fade(0,
						  lewyfan_infor.light_templatetrue,
						  lewyfan_infor.light_rcolor,
						  lewyfan_infor.light_gcolor,
						  lewyfan_infor.light_bcolor);
	}else{
		light_driver_fade(lewyfan_infor.light_brightness,
						  lewyfan_infor.light_templatetrue,
						  lewyfan_infor.light_rcolor,
						  lewyfan_infor.light_gcolor,
						  lewyfan_infor.light_bcolor);
	}
}
#endif
//S8

/*
 * 获取遥控器状态
 */
unsigned int light_getlight_remoteswitch(void)
{
	return lewyfan_infor.remote_switch;
}


/*
 * 获取遥控器类型
 */
extern remote_match_t	remote_array[LIGHT_REMOTE_MATCHMAX];
unsigned int light_getlight_remotetype(unsigned int islow)
{
	uint32_t temp;

	if(islow)
	{
		if(remote_array[3].use)temp = remote_array[3].type;
		else				   temp = 0;
		temp <<= 8;
		if(remote_array[2].use)temp |= remote_array[2].type;
		temp <<= 8;
		if(remote_array[1].use)temp |= remote_array[1].type;
		temp <<= 8;
		if(remote_array[0].use)temp |= remote_array[0].type;
	}else{
		if(remote_array[7].use)temp = remote_array[7].type;
		else				   temp = 0;
		temp <<= 8;
		if(remote_array[6].use)temp |= remote_array[6].type;
		temp <<= 8;
		if(remote_array[5].use)temp |= remote_array[5].type;
		temp <<= 8;
		if(remote_array[1].use)temp |= remote_array[4].type;
	}
	return temp;
}

/*
 * 获取遥控器mac
 */
unsigned int light_getlight_remotemac(unsigned int remotenum)
{
	uint32_t rid;
	if(remotenum > LIGHT_REMOTE_MATCHMAX)remotenum = LIGHT_REMOTE_MATCHMAX;
	remotenum -= 1;
	if(remote_array[remotenum].use)rid = remote_array[remotenum].rid;
	else						   rid = 0;

	return rid;
}

/*
 * 设置遥控器状态
 */
void light_setlight_remoteswitch(unsigned int state)
{
#if(LIGHT_REMOTE_NETSWITCH)
	Remote_update_remoteswitch  = state;
	lewyfan_infor.remote_switch = state;
	remote_sys.mRemote_stimer   = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
	if(lewyfan_infor.remote_switch)
	{
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
		remote_sys.mRemote_SaveUp  = 1;
	}
#endif
}

/*
 * 遥控器设置
 */
unsigned int light_getlight_remotecmd(void)
{
	return Remote_update_light_cmd;
}

/*
 * 遥控器设置
 */
void light_setlight_remoteset(unsigned int state)
{
#if(LIGHT_REMOTE_NETSWITCH)
	light_remote_appset(state);
#endif
}


void light_ble_rx(unsigned char *mac,unsigned char *bledata,uint8_t datalen)
{
}

extern int arch_gap_adv_lename_start(uint8_t * svc_data, uint16_t svc_data_len);
extern int arch_gap_adv_le_start(uint8_t * svc_data, uint16_t svc_data_len);
extern int arch_gap_adv_stop(void);

int light_blesend(uint8_t *pdata,uint8_t plen)
{
    int ret = MIIO_OK;
    ret = arch_gap_adv_le_start(pdata, plen);
    return ret;
}

int light_blesendstop(void)
{
    int ret = MIIO_OK;
    ret = arch_gap_adv_stop();
    return ret;
}

void light_blename_send(uint8_t *name,uint8_t len)
{
	arch_gap_adv_lename_start(name,len);
}


