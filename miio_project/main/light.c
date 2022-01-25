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
		.user_mode    = 0,// �ϵ�����״̬
		.smart_switch = 0,// �趯����
		.remote_switch= 0,// ң��������
		.fade_on  =LIGHT_FADE_DEFAULT,				// ����/����
		.fade_off =LIGHT_FADE_DEFAULT,				// ����/����
		.fade_scen=LIGHT_FADE_DEFAULT,				// ����/����
		.lowest_brightness=0,						// �������
		.lock_state=0,								// ��������0x01 ��������0x02

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
 * ��λ�ƹ����
 * flag:
 * 0--ϵͳ�״��ϵ�
 * 1--APPɾ��
 * 2--8������
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
	if(flag != 1)lewyfan_infor.lock_state  &= (~APP_LOCK_REMOTE);//���ң����+�龰����

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
 * ���ȵƳ�ʼ��
 * miio_instance_create�е���
 */
void light_startinit(void)
{
	LOG_INFO_TAG("light","light_startinit\r\n");

	/* �ײ�������ʼ�� */
	light_driver_start();

	/* ��״̬���ֳ�ʼ�� */
	//light_remote_start();

	aircondition_remote_start();
	
	//light_scne_init();
}

/*
 * ���ȵ�������
 */
static void* light_task(void* args)
{
	/* ���������ź��� */
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
 * ���ȵ�����������
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
 * ��ȡ�ƿ���״̬
 */
uint8_t light_getlight_sw(void)
{
	return lewyfan_infor.light_sw;
}

/*
 * ��ȡ������״̬
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
 * ��ȡ��ɫ��״̬
 */
unsigned int light_getlight_templatetrue(void)
{
	return lewyfan_infor.light_templatetrue;
}

/*
 * ��ȡ��rgb״̬
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
 * ��ȡ��rgb mode״̬
 */
unsigned int light_getlight_rgbmode(void)
{
	return lewyfan_infor.light_rgbmode;
}
/*
 * ��ȡ�鶯����
 * */
unsigned int light_getlight_smartswitch(void)
{
	return lewyfan_infor.smart_switch;
}
//��ȡ˯��ģʽ
uint8_t light_getlight_sleep(void)
{
	return (lewyfan_infor.light_rgbmode == LIGHT_CD_SLEEP);
}
//��ȡ����ģʽ
uint8_t light_getlight_weakup(void)
{
	return (lewyfan_infor.light_rgbmode == LIGHT_CD_WEAKUP);
}
//��ȡ����ʱ��
unsigned int light_getlight_modetime(void)
{
	return lewyfan_infor.modetime;
}
//��ȡ����ģʽ��������
unsigned int light_getlight_weakebr(void)
{
	return lewyfan_infor.weakebr;
}
//��ȡ����ģʽ��ʼ����
unsigned int light_getlight_sleepsbr(void)
{
	return lewyfan_infor.sleepsbr;
}
/*
 * ��ȡ�ϵ�Ĭ��״̬
 * */
unsigned int light_getlight_powerstate(void)
{
	return lewyfan_infor.power_state;
}

/*
 * ��ȡ�Ƿ������ϵ�Ĭ��״̬
 * */
unsigned int light_getlight_usermode(void)
{
	return lewyfan_infor.user_mode;
}

/*
 * ��ȡ���俪����
 * */
unsigned int light_getlight_fadeon(void)
{
	return lewyfan_infor.fade_on;
}

/*
 * ��ȡ����ز���
 * */
unsigned int light_getlight_fadeoff(void)
{
	return lewyfan_infor.fade_off;
}

/*
 * ��ȡ�龰�л�����
 * */
unsigned int light_getlight_fadescen(void)
{
	return lewyfan_infor.fade_scen;
}

/*
 * ��ȡ��ҹ��ģʽ
 * */
unsigned int light_getlight_nightswitch(void)
{
	return remote_sys.mLight_night;
}

/*
 * ��ȡ��ʱ�ز���
 * */
unsigned int light_getlight_timeroff(void)
{
	return (remote_sys.mLight_TimerOff*LIGNT_TIMER_TICK/1000/60);
}

/*
 * ��ȡ����������ò���
 * */
unsigned int light_getlight_lowestbrightness(void)
{
	return lewyfan_infor.lowest_brightness;
}

/*
 * ��ȡ����״̬����
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
 * ���õƿ���״̬
 *@param-sw[IN]:Ҫ���õ�״̬
 */
 #if 1
void light_setlight_sw(uint8_t sw)
{
	Remote_update_light_sw = (uint8_t)sw;//��¼����״̬
	lewyfan_infor.light_sw    = sw;
	if(lewyfan_infor.light_sw)remote_sys.mLight_mode = M_LIGHT_ON;
	else					  remote_sys.mLight_mode = M_LIGHT_OFF;

	//֧����ҹģʽ
#if(LIGHT_NIGHT_SWITCH)
	//��ҹģʽ����
	if(remote_sys.mLight_night)
	{
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MIN;//��������Ϊ���
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;//ɫ������Ϊ���
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
	remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
}
#endif

/*
 * ���õ�����״̬
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
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
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
	remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
}

/*
 * ���õ�ɫ��״̬
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
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
		remote_sys.mRemote_SaveUp   = 1;
	}
#endif
	Remote_update_light_templatetrue = templatetrue;
	remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
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
 * ���õ�RGB״̬
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
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
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

	remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
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
 * ���õ�RGB mode״̬
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
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
		remote_sys.mRemote_SaveUp   = 1;
	}
#endif
	Remote_update_light_rgbmode = rgbcolor;
	lewyfan_infor.light_rgbmode = rgbcolor;
	remote_sys.mRemote_stimer   = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
	remote_sys.mRemote_SaveUp   = 1;
	switch(rgbcolor)
	{
	/*
	��ܰģʽ��		ɫ�£�30% / ���ȣ�100%
	���ӣ�ӰԺ��ģʽ��ɫ�£�50% / ���ȣ�1%
	�Ķ�ģʽ��		ɫ�£�70% / ���ȣ�100%
	����ģʽ��		ɫ�£�100% / ���ȣ�100%
	*/
	case LIGHT_WY_NORMAL://				0	//ɫ��ģʽ
	case LIGHT_RGB_NORMAL://			1	//�ʹ�ģʽ
		ret = 1;
		break;
	case LIGHT_RGB_COLOR:				// �ʹ�ģʽ
		lewyfan_infor.light_sw         = 1;
		remote_sys.mLight_mode         = M_LIGHT_ON;
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX;
		remote_sys.mRemote_SaveUp   = 0;
		/* no break */
	case LIGHT_RGB_FADE://				2	//�߲ʽ���
		remote_sys.mRemote_SaveUp   = 0;
		break;
	case LIGHT_RGB_JUMP://				3	//�߲�
		remote_sys.mRemote_SaveUp   = 0;
		break;
	case LIGHT_CD_SUNDAY://				4	//�չ�ģʽ
		lewyfan_infor.light_sw           = 1;
		remote_sys.mLight_mode           = M_LIGHT_ON;
		lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MAX;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*9/10;
		ret = 1;
		break;
	case LIGHT_CD_NIGHT://				5	//�¹�ģʽ
		lewyfan_infor.light_sw           = 1;
		remote_sys.mLight_mode         = M_LIGHT_ON;
		lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MIN;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
		ret = 1;
		break;
	case LIGHT_CD_WARMTH://				6	//��ܰģʽ
		lewyfan_infor.light_sw         = 1;
		remote_sys.mLight_mode         = M_LIGHT_ON;
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*3/10;
		ret = 1;
		break;
	case LIGHT_CD_TVMODE://				7	//����ģʽ
		lewyfan_infor.light_sw         = 1;
		remote_sys.mLight_mode         = M_LIGHT_ON;
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX*1/100;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*5/10;
		ret = 1;
		break;
	case LIGHT_CD_READING://			8	//�Ķ�ģʽ
		lewyfan_infor.light_sw         = 1;
		remote_sys.mLight_mode         = M_LIGHT_ON;
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*7/10;
		ret = 1;
		break;
	case LIGHT_CD_CPMPUTER://			9	//����ģʽ
		lewyfan_infor.light_sw         = 1;
		remote_sys.mLight_mode         = M_LIGHT_ON;
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN);
		ret = 1;
		break;
	case LIGHT_CD_HOST://				10	//���ģʽ
		lewyfan_infor.light_sw         = 1;
		remote_sys.mLight_mode         = M_LIGHT_ON;
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*5/10;
		ret = 1;
		break;
	case LIGHT_CD_ENTER://				11	//����ģʽ
		lewyfan_infor.light_sw         = 1;
		remote_sys.mLight_mode         = M_LIGHT_ON;
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX/2;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*3/10;
		ret = 1;
		break;
	case LIGHT_CD_WEAKUP://				12	//�峿���� 30����  ɫ�¡� 0%-90%��  ���ȡ�1-95%��
		remote_sys.mLight_ModeTimer      = (1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK);
		lewyfan_infor.light_sw           = 1;
		remote_sys.mLight_mode           = M_LIGHT_ON;
		lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MIN;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
		ret = 1;
		break;
	case LIGHT_CD_DUSK://				13	//�ƻ����� 30����  ɫ�¡� 50%-100%��  ���ȡ�1-100%��
		remote_sys.mLight_ModeTimer      = (1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK);
		lewyfan_infor.light_sw           = 1;
		remote_sys.mLight_mode           = M_LIGHT_ON;
		lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MIN;
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*5/10;
		ret = 1;
		break;
	case LIGHT_CD_SLEEP://				14	//ҹ������30����  ɫ�¡� 95%->0%��  ���ȡ�95-0%��
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
 * �����鶯����
 */
void light_setlight_smartswitch(unsigned int state)
{
#if(LIGHT_SMARTSWITCH_ENABLE == 1)
	remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
	lewyfan_infor.smart_switch   = state;
	Remote_update_smart_switch   = state;
#endif
}

//˯��ģʽ
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

	remote_sys.mRemote_stimer   = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
	remote_sys.mRemote_SaveUp   = 1;
}
//����ģʽ
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
	remote_sys.mRemote_stimer   = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
	remote_sys.mRemote_SaveUp   = 1;
}
//ģʽʱ��
void light_setlight_modetime(unsigned int state)
{
	LOG_INFO("light_setlight_modetime: %d\n", state);
	if(lewyfan_infor.light_rgbmode == LIGHT_CD_WEAKUP || lewyfan_infor.light_rgbmode == LIGHT_CD_SLEEP)
	{
		lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
		remote_sys.mRemote_stimer   = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
		remote_sys.mRemote_SaveUp   = 1;
	}
	Remote_update_light_modetime = state;
	lewyfan_infor.modetime = state;
}
//������������
void light_setlight_weakebr(unsigned int state)
{
	LOG_INFO("light_setlight_weakebr: %d\n", state);
	if(lewyfan_infor.light_rgbmode == LIGHT_CD_WEAKUP)
	{
		lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
		remote_sys.mRemote_stimer   = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
		remote_sys.mRemote_SaveUp   = 1;
	}
	if(state >= LIGHT_BRIGHTNESS_MIN)
	{
		lewyfan_infor.weakebr = state;
		Remote_update_light_weakebr = state;
	}
}
//˯��ģʽ��ʼ����
void light_setlight_sleepsbr(unsigned int state)
{
	LOG_INFO("light_setlight_sleepsbr: %d\n", state);
	if(lewyfan_infor.light_rgbmode == LIGHT_CD_SLEEP)
	{
		lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
		remote_sys.mRemote_stimer   = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
		remote_sys.mRemote_SaveUp   = 1;
	}
	if(state >= LIGHT_BRIGHTNESS_MIN)
	{
		lewyfan_infor.sleepsbr = state;
		Remote_update_light_sleepsbr = state;
	}
}

/*
 * ���õ��ϵ�״̬
 */
void light_setlight_powerstate(unsigned int state)
{
	Remote_update_powerstate   = state;
	remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
	lewyfan_infor.power_state = state;
}

/*
 * ���õ�
 */
void light_setlight_usermode(unsigned int state)
{
	if(lewyfan_infor.light_sw == 0)return;
	if(lewyfan_infor.power_state == POWER_STATE_ON)
	{
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
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
 * ���ý��俪
 */
void light_setlight_fadeon(unsigned int state)
{
#if(LIGHT_FADE_ON == 1 && LIGHT_FADE_SWITCH == 1)
	remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
	lewyfan_infor.fade_on   = state;
	Remote_update_fadeon    = state;
#endif
}

/*
 * ���ý����
 */
void light_setlight_fadeoff(unsigned int state)
{
#if(LIGHT_FADE_ON == 1 && LIGHT_FADE_SWITCH == 1)
	remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
	lewyfan_infor.fade_off   = state;
	Remote_update_fadeoff    = state;
#endif
}

/*
 * ���ý����龰
 */
void light_setlight_fadescen(unsigned int state)
{
#if(LIGHT_FADE_ON == 1 && LIGHT_FADE_SWITCH == 1)
	remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
	lewyfan_infor.fade_scen   = state;
	Remote_update_fadescen    = state;
#endif
}


/*
 * ������ҹ��ģʽ
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
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
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
 * ���ö�ʱ��
 */
void light_setlight_timeroff(unsigned int state)
{
	if(lewyfan_infor.light_sw)
	{
		remote_sys.mLight_TimerOff = state*60*1000/LIGNT_TIMER_TICK;
	}else{
		lewyfan_infor.light_sw = 1;
		remote_sys.mLight_TimerOff = state*60*1000/LIGNT_TIMER_TICK;

		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
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
 * �����������
 */
void light_setlight_lowestbrightness(unsigned int state)
{
#if(LIGHT_FADE_ON == 1 && LIGHT_FADE_SWITCH == 1)
	remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
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
 * ��������״̬
 */
void light_setlight_lockstate(unsigned int state)
{
#if(LIGHT_FADE_ON == 1 && LIGHT_FADE_SWITCH == 1)
	remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
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
//����+rgb�л�
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
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
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

		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
		remote_sys.mRemote_SaveUp  = 1;
	}else{
		lewyfan_infor.light_sw = 1;
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
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

//RGB�л�
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
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
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

		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
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


//����+ɫ���л�
void light_setlight_ontmsw(uint8_t sw)
{
#if(LIGHT_NIGHT_SWITCH)
	if(remote_sys.mLight_night)
	{
		unsigned char savesw;
		savesw = lewyfan_infor.light_sw;
		arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
		lewyfan_infor.light_sw = savesw;
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
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
		if(lewyfan_infor.light_templatetrue == LIGHT_TEMPLATETRUE_MAX)//�׹�-���ƹ�
		{
			lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
		}else
		if(lewyfan_infor.light_templatetrue == LIGHT_TEMPLATETRUE_MIN)//��-����
		{
			lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MID;
		}
		else
		{
			lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MAX;//���Թ�-���׹�
		}
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
		remote_sys.mRemote_SaveUp  = 1;
	}else{
		lewyfan_infor.light_sw = 1;
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
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

//ɫ���л�
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
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
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
		if(lewyfan_infor.light_templatetrue == LIGHT_TEMPLATETRUE_MAX)//�׹�-���ƹ�
		{
			lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
		}else
		if(lewyfan_infor.light_templatetrue == LIGHT_TEMPLATETRUE_MIN)//��-����
		{
			lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MID;
		}
		else
		{
			lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MAX;//���Թ�-���׹�
		}
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
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
//ɫ��-%
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
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
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
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
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
//ɫ��++
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
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
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
			remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
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


//����+�����л�=���<<>>���
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
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
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
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
		remote_sys.mRemote_SaveUp  = 1;
	}else{
		lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
		lewyfan_infor.light_sw = 1;
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
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
//�����л�=���<<>>���
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
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
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
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
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
	//���±�����Ϣ-���ڴ洢�Ϳ���
	update_service_aircondition_mode(m_current_group,mode);
	//remote_sys.mRemote_SaveUp	= 1;
	//remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
}

#endif

void aircondition_set_sw_toggle(void)
{
	//��ȡ��״̬
	uint8_t sw = get_service_aircondition_sw();

	//������״̬
	sw = !sw;
	
	//���±�����Ϣ-���ڴ洢�Ϳ���
	update_service_aircondition_sw(sw);
	
	//arch_printf("aircondition_set_sw_toggle:%d,0x%02x\r\n",m_current_group,sw);
	//���±�����Ϣ-���ڴ洢�Ϳ���
}

#if 0
void aircondition_set_sw(uint8_t sw)
{
	
	//arch_printf("aircondition_set_sw:%d,0x%02x\r\n",m_current_group,sw);
	//���±�����Ϣ-���ڴ洢�Ϳ���
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
	
	update_service_aircondition_group(m_current_group);//����notify
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



//����-%
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
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
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
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
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
//����++
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
		remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
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
			remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
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
//�����л�
void light_setlight_onoff(uint8_t sw)
{
	remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
	remote_sys.mRemote_SaveUp  = 1;

	//��ǰ״̬Ϊ��
	if(lewyfan_infor.light_sw == 0)
	{
		//����״̬Ϊ��
		lewyfan_infor.light_sw = 1;
		remote_sys.mLight_mode = M_LIGHT_ON;
	}
	//��ǰ״̬Ϊ��
	else
	{
		//����״̬Ϊ��
		lewyfan_infor.light_sw = 0;
		remote_sys.mLight_mode = M_LIGHT_OFF;
	}

	//���֧����ҹģʽ
#if(LIGHT_NIGHT_SWITCH)
	if(remote_sys.mLight_night && lewyfan_infor.light_sw)//��ҹģʽ�������ҵ�ǰ�Ƶ�״̬Ӧ��Ϊ ��
	{
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MIN;//���Ƶ����ȵ��������
		lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;//���Ƶ�ɫ�µ��������
	}
#endif

	//��Ӧ����Ϊ�ر�
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
 * ��ȡң����״̬
 */
unsigned int light_getlight_remoteswitch(void)
{
	return lewyfan_infor.remote_switch;
}


/*
 * ��ȡң��������
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
 * ��ȡң����mac
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
 * ����ң����״̬
 */
void light_setlight_remoteswitch(unsigned int state)
{
#if(LIGHT_REMOTE_NETSWITCH)
	Remote_update_remoteswitch  = state;
	lewyfan_infor.remote_switch = state;
	remote_sys.mRemote_stimer   = LIGHT_REMOTE_STIMER;//���������ӳٱ���ʱ��
	if(lewyfan_infor.remote_switch)
	{
		remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//���������ӳٱ���ʱ��
		remote_sys.mRemote_SaveUp  = 1;
	}
#endif
}

/*
 * ң��������
 */
unsigned int light_getlight_remotecmd(void)
{
	return Remote_update_light_cmd;
}

/*
 * ң��������
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


