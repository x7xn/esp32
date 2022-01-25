

#ifndef _LETSNETWORKING_H
#define _LETSNETWORKING_H

/* Includes  -------------------------------------------------------------------*/
#include "stdint.h"
#include "light_drivers.h"

/****************************************/
#define HEADER_VERSION		0//第一版

#define HEADER_LESHI		0

#define HEADER_MESH       		0//组网通信帧
#define HEADER_REMOTE     		1//遥控开关帧
#define HEADER_PASSSW     		2//无源开关帧
#define HEADER_PASSSW_NONET     3//无源开关帧

#define PASSIVESW_TYPE    	0xc1
#define PASSIVESW_HEAD_V	0		// V1版本的遥控器帧
#define PASSIVESW_HEAD_T	2		// 无源开关帧
#define PASSIVESW_K2		0x01	//K2-4键回弹
#define PASSIVESW_K3		0x81	//K3-3键回弹
#define PASSIVESW_G1		0x82	//G1-单键回弹

#define PASSIVESW_PKEY		0		//参数0--按键值
#define PASSIVESW_VKEY		0x7F	//参数0--按键值
#define PASSIVESW_VIO		0x80	//参数0--按键值
#define PASSIVESW_ONDB      0x81    //双侧开关参数
#define PASSIVESW_ONSG		0xC1	//单键开关参数
#define PASSIVESW_TIME		25		//无源开关间隔

#define REMOTE_TYPE    		0xFE	// 遥控器类型
#define REMOTE_MAXLEN		18		// 遥控器包参数长度位置
#define REMOTE_LMAXLEN		11		// 雷士遥控器包参数长度位置
#define REMOTE_HEAD_V		0		// V1版本的遥控器帧
#define REMOTE_HEAD_T		1		// 遥控帧
#define REMOTE_CLEN			10		// 无参数时包长度-不包括CRC

#define REMOTE_COLORSEL		0
#define REMOTE_COLORGREA	3

#define REMOTE_WCOLOR		0
#define REMOTE_YCOLOR		1
#define REMOTE_MAXCOLOR		0x80
#define REMOTE_SCENESEL     0 /* Scene */
#define REMOTE_PERON        0 /* 随意开关使用 */

/* 幀頭 */


/* defines  -------------------------------------------------------------------*/

#define      OEM_DRIVER_TYPE           LETS_OEM_CENTER          //设备类型--网关
#define      OEM_DRIVER_LAYER          LETS_LAYER_CENTER        //网关初始化层
#define      OEM_DRIVER_CHECK(type)   (OEM_DRIVER_TYPE != type) //驱动类型检查

#define      OEM_DRIVER_PACK_LENGTH                  6          //设备信息长度
#define      OEM_INFOR_PACK_LENGTH                   12         //设备信息+主机信息

#define      LETS_LAMP_MAX                           32         //最大连接端点
#define      LETS_LAMP_INFOR                         6          //灯信息
#define      LETS_LAMP_PACK                          12         //灯信息包:LETS_LAMP_INFOR + OEM_DRIVER_PACK_LENGTH

#define      LETS_PACK_DATABLE                       11         //数据包部分
#define      LETS_PACK_LENBLE                        27         //数据包部分

#define      LETS_PASSIVE_SW_LENCRC                  14         //
#define      LETS_PASSIVE_SW_LENGHT                  15         //无源开关遥控器长度

#define      LETS_PACK_ACKWAIT                       3          //应答延迟
#define      LETS_PACK_SENTCNT                       4          //发送频道4个

#define      LETS_LAYER_OTHERCNT                     8          //广播/连接主机-8次中转后
#define      LETS_BROADCAST_CNT                      16         //广播/连接主机-16次数
#define      LETS_BROADCAST_WAIT                     500        //500ms延迟-防止数据碰撞

#define      LETS_RF_WROK                            10000      //10S
#define      LETS_RF_WAKEUP                          16         //16MS--唤醒时间15-8=8MS
#define      LETS_RF_SLEEP                           8          // 8MS--休眠时间8MS

#define      LETS_BROADCAST_ID                       0xffffffff             //广播目标--广播ID
/* type  ----------------------------------------------------------------------*/
//联合体由于数据大小端问题--所有变量必须使用字节声明
//八位机特别处理
//设备信息
#pragma pack(1)
typedef union
{
	struct
	{
		uint8_t    Type;      //设备类型
		uint32_t   ID;        //设备ID
		uint8_t    Layer;     //设备在级别
	};
	uint8_t pack[OEM_DRIVER_PACK_LENGTH];
}OEM_DRIVER_PACK;

typedef struct
{
  uint8_t    mType:3,
         	 mVer:3,
         	 mCom:2;
}Head_BIT;
typedef union
{
#if(LIGHT_REMOTE_LEISHI == 1)
	struct//遥控器类型
	{
		Head_BIT	Head;    	//1byte 帧头
		uint8_t 	PackId;     //1byte
		uint32_t	RemoteID;   //4bytes
		uint8_t	 	LampID;     //1byte
		uint8_t		Type;       //1byte
		uint8_t 	Cmd;        //1byte
		uint8_t		Len;        //1byte
		uint8_t 	Pter[8];    //10+8bytes
	};

#else
	struct//遥控器类型
	{
		Head_BIT	Head;    	//1byte 帧头
		uint32_t	RemoteID;   //4bytes
		uint8_t		Type;       //1byte
		uint8_t	 	LampID;     //1byte
		uint8_t 	PackId;     //1byte
		uint8_t 	Cmd;        //1byte
		uint8_t		Len;        //1byte
		uint8_t 	Pter[8];    //10+8bytes
	};
#endif

	struct//无源开关类型
	{
		Head_BIT		pHead;    	/*帧头 */
		uint32_t 		pID;
		uint8_t	 		pSubType;
		uint8_t 		pPter[12];   //6+12
	};
	uint8_t pack[REMOTE_MAXLEN];
} RemoteType;

typedef union
{
	struct //雷士遥控器类型
	{
		uint8_t			UUF;        //1byte 雷士遥控标识，00
		uint8_t			ID1;        //1byte
		uint8_t			ID2;        //1byte
		uint8_t			ID3;        //1byte
		uint8_t			LCmd;       //1byte
		uint8_t			LPackId;    //1byte
		uint8_t			LLampID;    //1byte
		uint8_t			LPter[3];   //1bytes
		uint8_t			Lcrc;       //1bytes
	};
	uint8_t pack[REMOTE_LMAXLEN];
} LeiShiRemoteType;

typedef union
{
	struct
	{
		Head_BIT		pHead;    /* 帧头 */
		OEM_DRIVER_PACK dDriver; /* destination Device 6bytes */
		OEM_DRIVER_PACK sDriver; /* Device 6bytes */
		uint8_t mCmd;
		uint8_t mData[LETS_PACK_DATABLE];         	 /* Device data 11Bytes */
		uint8_t mCRC;
		uint8_t mPackIdNext;          /* Pack ID + Txcnt */
	};
	uint8_t pack[LETS_PACK_LENBLE];
} RF_BLE_PACK;

#pragma pack()

#endif /* _LETSNETWORKING_H */


