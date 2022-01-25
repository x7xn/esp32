/******************************************************************************
 * light function used here.
 *
 * FileName: light_scen.c
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
#include "LeCmd.h"
#include "LetsNetwork.h"
#include "light.h"
#include "light_remote.h"
#include "light_drivers.h"
#include "light_scen.h"

extern lewyfan_device_infor_t  lewyfan_infor;
extern      remote_task_t   remote_sys;

scen_data_t mLightScen[SCNE_MAXSIZE]={0};

void light_scne_init(void)
{
	//设备信息
	if(arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_SCEN, (void*)&mLightScen, sizeof(scen_data_t)*SCNE_MAXSIZE) <= 0)
	{
		LOG_INFO_TAG("light_remote", "mLightScen read false...");
	}
}

/*
 *
 *
 * */
void light_scne_save(uint8_t lid, uint32_t rid,uint8_t scen)
{
	uint8_t i,j,next;

	//arch_printf("light_remote light_scne_save...\r\n");
	if(lid == 0)lid = 1;
	for(i=0;i<SCNE_MAXSIZE;i++)
	{
		if(mLightScen[i].use)
		{
			if(mLightScen[i].lid == lid &&
			   mLightScen[i].rid == rid &&
			   mLightScen[i].scenum == scen)
			{
				break;
			}
		}
	}
	if(i >= SCNE_MAXSIZE)//没有存储过这个情景
	{
		for(i=0;i<SCNE_MAXSIZE;i++)//查看是否有空的情景位置
		{
			if(mLightScen[i].use == 0)break;
		}
		if(i >= SCNE_MAXSIZE)//没有空的存储位置
		{
			i = mLightScen[0].next;

			next = i+1;
			if(next >= SCNE_MAXSIZE)next=0;//下一个覆盖点
			for(j=0; j<SCNE_MAXSIZE; j++)mLightScen[j].next = next;
		}else{
			mLightScen[i].next = 0;
		}
	}
	mLightScen[i].use    = 1;
	mLightScen[i].rid    = rid;
	mLightScen[i].lid    = lid;
	mLightScen[i].scenum = scen;

	//arch_printf("mLightScen num = %d,rid=%08x,lid=%d,scen=%d...\r\n",i,rid,lid,scen);

	if(lewyfan_infor.light_sw)mLightScen[i].bdata |=   SCEN_BIT_LIGHTSW;
	else					  mLightScen[i].bdata &= (~SCEN_BIT_LIGHTSW);

	mLightScen[i].brightness = lewyfan_infor.light_brightness;
	mLightScen[i].templater  = lewyfan_infor.light_templatetrue;
	mLightScen[i].rcolor     = lewyfan_infor.light_rcolor;
	mLightScen[i].gcolor     = lewyfan_infor.light_gcolor;
	mLightScen[i].bcolor     = lewyfan_infor.light_bcolor;
	mLightScen[i].rgbmode    = lewyfan_infor.light_rgbmode;

	arch_psm_set_value(LIGHT_SAVE_NAME, LIGHT_KEY_SCEN, (void*)&mLightScen, sizeof(scen_data_t)*SCNE_MAXSIZE);
}

/*
 *
 *
 * */
void light_scne_read(uint8_t lid, uint32_t rid,uint8_t scen,uint8_t onoff)
{
	uint8_t i;

	//arch_printf("light_remote light_scne_read...\r\n");
	if(lid == 0)lid = 1;
	for(i=0;i<SCNE_MAXSIZE;i++)
	{
		if(mLightScen[i].use)
		{
			if(mLightScen[i].lid == lid &&
			   mLightScen[i].rid == rid &&
			   mLightScen[i].scenum == scen)
			{
				//arch_printf("mLightScen num = %d,rid=%08x,lid=%d,scen=%d...\r\n",i,rid,lid,scen);
				if((mLightScen[i].bdata & SCEN_BIT_LIGHTSW) && onoff != 0)
				{
					lewyfan_infor.light_sw = 1;
					remote_sys.mLight_mode = M_LIGHT_ON;
				}
				else{
					lewyfan_infor.light_sw = 0;
					remote_sys.mLight_mode = M_LIGHT_OFF;
				}


				lewyfan_infor.light_brightness   = mLightScen[i].brightness;
				lewyfan_infor.light_templatetrue = mLightScen[i].templater;
				lewyfan_infor.light_rgbmode      = mLightScen[i].rgbmode;
				lewyfan_infor.light_rcolor       = mLightScen[i].rcolor;
				lewyfan_infor.light_gcolor       = mLightScen[i].gcolor;
				lewyfan_infor.light_bcolor       = mLightScen[i].bcolor;

				if(lewyfan_infor.light_templatetrue < LIGHT_TEMPLATETRUE_MIN)lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
				if(lewyfan_infor.light_templatetrue > LIGHT_TEMPLATETRUE_MAX)lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MAX;

				if(lewyfan_infor.light_brightness < LIGHT_BRIGHTNESS_MIN)lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MIN;
				if(lewyfan_infor.light_brightness > LIGHT_BRIGHTNESS_MAX)lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX;

				if(lewyfan_infor.light_sw)
				{
					light_driver_fade(lewyfan_infor.light_brightness,
									  lewyfan_infor.light_templatetrue,
									  lewyfan_infor.light_rcolor,
									  lewyfan_infor.light_gcolor,
									  lewyfan_infor.light_bcolor);
				}else{
					light_driver_fade(0,
									  lewyfan_infor.light_templatetrue,
									  lewyfan_infor.light_rcolor,
									  lewyfan_infor.light_gcolor,
									  lewyfan_infor.light_bcolor);
				}
				break;
			}
		}
	}
	if(i >= SCNE_MAXSIZE && scen <= 7)//没有存储过这个情景
	{
		remote_sys.mLight_mode = M_LIGHT_ON;
		lewyfan_infor.light_sw = 1;
		switch(scen)//默认情景
		{
			case 0:
				lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
				lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MAX;
				break;
			case 1:
				lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MID;
				lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MAX;
				break;
			case 2:
				lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MAX;
				lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MAX;
				break;
			case 3:
				lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
				lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MIN;
				break;
			case 4:
				lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MID;
				lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MIN;
				break;
			case 5:
				lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MAX;
				lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MIN;
				break;
			case 6:
				lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MAX;
				lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MAX>>1;
				break;
			case 7:
				lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
				lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MAX>>1;
				break;
			default:
				break;
		}

		if(lewyfan_infor.light_sw)
		{
			light_driver_fade(lewyfan_infor.light_brightness,
							  lewyfan_infor.light_templatetrue,
							  lewyfan_infor.light_rcolor,
							  lewyfan_infor.light_gcolor,
							  lewyfan_infor.light_bcolor);
		}else{
			light_driver_fade(0,
							  lewyfan_infor.light_templatetrue,
							  lewyfan_infor.light_rcolor,
							  lewyfan_infor.light_gcolor,
							  lewyfan_infor.light_bcolor);
		}
	}
}


