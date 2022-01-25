/************************************************************
 * Copyright (C), 2015-2016, Lelight Technology Ltd.  http://www.lelight.cn
 * All right reserved.
 * FileName:
 * Author:
 * Version:
 * Date:
 * Description:
 * Other:
 * History:
 *
 * 1.Author:  WHY
 *   Version: V1.1
 *   Data:    2016/3/4
 *
*************************************************************/
#ifndef  _LETS_CMDLIST_H_
#define  _LETS_CMDLIST_H_


/* enum  ---------------------------------------------------------------------*/

typedef enum
{
	M_LIGHT_PON = 0,
	M_LIGHT_OFF,
	M_LIGHT_ON,
}Lamp_State_Typedef;

typedef enum
{
	LAMP_ONLINE = 0,
	LAMP_OFFLINE,
}Lamp_RfState_Typedef;


typedef enum
{
	WIFI_IDLE = 0,
	WIFI_AP,      //WIFI--AP模式
	WIFI_BUSY,    //WIFI--连接中
	WIFI_READY,   //WIFI--连接上服务器
}WIFI_STATE;

typedef enum
{
	WIFI_SHOW_AP    = 1000,    //WIFI--AP模式
	WIFI_SHOW_BUSY  = 200,     //WIFI--连接中
	WIFI_SHOW_READY = 65535,   //WIFI--连接上服务器
}WIFI_STATE_Show_Typedef;

typedef enum
{
	SHOW_IDLE = 0,
	SHOW_CNT1 = 2,
	SHOW_CNT2 = 4,
	SHOW_CNT3 = 6,
	SHOW_CNT4 = 8,
	SHOW_CNT10 = 20,
	SHOW_CNT20 = 40,
	SHOW_CNT45 = 90,
}Lamp_Show_Typedef;

typedef enum
{
	/* center controller, gateway */
	    LETS_OEM_CENTER              = 0x00,
	    /* WY light */
	    LETS_OEM_WY_LIGHT            = 0x01,
		LETS_OEM_Remark_2            = 0x02,
	    LETS_OEM_RGB                 = 0x03,
	    LETS_OEM_WY_RGB              = 0x04,
	    LETS_OEM_SINGLE_LIGHT        = 0x05,
	    LETS_OEM_SUB                 = 0x06,
	    LETS_OEM_WY_RGB_MUSIC        = 0x07,
	    LETS_OEM_SENSOR              = 0x08,
	    LETS_OEM_WY_RGB_MUSIC_SUB    = 0x09,   //WYRGB音乐+加辅灯
	    LETS_OEM_RGB_MUSIC           = 0x0a,   //RGB音乐灯
	    LETS_OEM_SG_SUB              = 0x0b,   //单色灯+辅灯
	    LETS_OEM_SENSOR_LIGHT        = 0x0c,
	    LETS_OEM_W_RGB               = 0x0d,
	    LETS_OEM_BEDROOM_LIGHT       = 0x0e,
	    LETS_OEM_SWITCH              = 0x0f,
	    LETS_OEM_GROUPED_LIGHT       = 0x10,    //组合灯
	    LETS_OEM_Remark_LIGHT        = 0x11,    //保留
	    LETS_OEM_WYFAN_LIGHT         = 0x12,

		LETS_OEM_CURTAIN_SW			 = 0xb0,	//门帘控制器---从这里开始设备不受灯控制指令
		LETS_OEM_SOCKET_SW			 = 0xb1,	//插座控制器


	    LETS_OEM_DOORSENSOR          = 0xc0,
	    LETS_OEM_PASSIVE_SW          = 0xc1,
		LETS_OEM_INDOORSENSOR        = 0xc2,
	    LETS_OEM_END                 = 0xcf,

	    /* all Device */
	    LETS_OEM_ALL                 = 0xFF,
}LETS_OEM_Type;
#define IS_LETS_OEM_TYPE(PERIPH) ((PERIPH) <= LETS_OEM_END&&\
                                  (PERIPH) >= LETS_OEM_WYLIGHT)

typedef enum
{
	//设备所在层
	LETS_LAYER_CENTER   = 0x00,       //总控制器层
	LETS_LAYER_SUB      = 0x01,       //子层
	LETS_LAYER_OTH      = 0x02,       //其他层
	LETS_LAYER_ENDPOINT = 0xff,       //端点初始化层
}LETS_LAYER_Typedef;

#define IS_LETS_OEM_LAYER(PERIPH) ((PERIPH) <= LETS_LAYER_OTH&&\
                                  (PERIPH) >= LETS_LAYER_SUB)

typedef enum
{
	LETS_NET_BROADCAST = 0,
	LETS_NET_READY,
	LETS_NET_GETHOST,
	LETS_NET_TXACK
}LETS_NET_Typedef;//网络状态

typedef enum
{
	LETS_SB_TX = 0,
	LETS_SB_WTXC,
	LETS_SB_WACK,
	LETS_SB_NODO,
} LETS_SB_Typedef;//子状态

/* Lamp cmd */
typedef enum
{
    LCMD_ON               = 0x00,        //open lamp
    LCMD_OFF              = 0x01,        //close lamp
    LCMD_CON_ADD          = 0x02,        //亮度+
    LCMD_CON_DEC          = 0x03,        //亮度-
    LCMD_COL_ADD          = 0x04,        //颜色+
    LCMD_COL_DEC          = 0x05,        //颜色-
    LCMD_RGB_SWITCH       = 0x06,        //RGB跳变
    LCMD_RGB_CYC          = 0x07,        //RGB循环渐变
    LCMD_BRIGHTNESS       = 0x08,        //亮度设置
    LCMD_WY_RGB           = 0x09,        //WY和RGB开关
    LCMD_STOP             = 0x0a,        //调光调色停止

    LCMD_SYNC             = 0x0b,        //分段同步
    LCMD_COLORSEL         = 0x0c,        //颜色档位选择
    LCMD_COLORK           = 0x0d,        //色温设置
    LCMD_KEYSCENE         = 0x0e,        //一键情景0x0E-使用
    LCMD_SUB_ON           = 0x0f,        //Auxiliary lamp open
    LCMD_SUB_OFF          = 0x10,        //Auxiliary lamp close
    LCMD_RGB_CYCMODEF     = 0x11,        //RGB Breathing pattern
    LCMD_NIGHT            = 0x12,        //小夜灯
    LCMD_TOGGLE           = 0x13,        //开关灯切换
    LCMD_RGBMUSIC         = 0x14,        //RGB music mode
    LCMD_SPEECH	          = 0x15,        //小二使能
    LCMD_RGB_COLORK       = 0x16,        //RGB颜色设置
    LCMD_RGB_SHADE        = 0x17,        //RGB颜色深浅设置
    LCMD_MODE_SELECT      = 0x18,        //晶美模式选择
    LCMD_MODE_SPEED       = 0x19,        //晶美模式变化速度
    LCMD_TIME_OFF    	  = 0x1a,        //延时关灯
    LCMD_BRI_COLOR        = 0x1b,        //亮度色温设置
    LCMD_BLE_CTL          = 0x1c,        //蓝牙设置
    LCMD_GROUP_SEL   	  = 0x1d,        //分组灯-分组控制
    LCMD_BREATH_SYNC	  = 0x1e,        //RGB呼吸模式同步
    LCMD_JMMODE_SYNC      = 0x1f,        //晶美模式同步
    LCMD_RGB_JUMPMODE     = 0x20,        //RGB跳变模式
	LCMD_WINDSPEED_SET	  = 0x21,        //
	LCMD_WINDTIME_SET	  = 0x22,        //
	LCMD_VOLUME		      = 0x23,        //
	LCMD_SCENE_CONTROL	  = 0x24,        //
	LCMD_WIRELESS_SENSOR  = 0x25,        //
	LCMD_WIFI_STATION     = 0x26,        //
	LCMD_24G_MATCH		  = 0x27,        //
	LCMD_ONEBRIGHTNESS	  = 0x28,        //
	LCMD_KENUP			  = 0x29,        //
	LCMD_RESET_GETEWAY    = 0x2F,		 //一键重置配网信息
	LCMD_LOCK			  = 0x36,		 //0-解锁，1-锁定
	LCMD_BRMODE			  = 0x38,		 //亮度档位，参数=1加，参数=2减

    LCMD_LON  	          = 0x80,        //长按开
    LCMD_LOFF 	          = 0x81,        //长按关
    LCMD_SETSCENE         = 0x8E,        //一键情景-设置

    LCMD_RGBINIT          = 0x91,        //RGB颜色初始化

    LCMD_UPSTATE          = 0xed,        /* Group control OFF/ON */
    LCMD_UPBNESS          = 0xee,        /* Group control brightness */
    LCMD_UPCOLOR          = 0xef,        /* Group control color */

    LCMD_PROMPT0          = 0xf0,        //设备提示0（闪烁1次）
    LCMD_PROMPT1          = 0xf1,        //设备提示1
    LCMD_PROMPT2          = 0xf2,        //设备提示2
    LCMD_PROMPT3          = 0xf3,        //设备提示3

    LCMD_ACK              = 0xf8,        //灯应答
    LCMD_UPLAMP           = 0xF9,        //更新灯状态
    LCMD_DELELAMP         = 0xFA,        //删除绑定
    LCMD_PTEST			  = 0xFD,		 //产测命令
    LCMD_TEST             = 0xFE,        //测试指令
    LCMD_NULL             = 0xFF,        //Null
}LETS_LCMD_Typedef;


#endif  //_LETS_CMDLIST_H_
