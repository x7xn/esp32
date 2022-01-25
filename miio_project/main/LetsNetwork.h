

#ifndef _LETSNETWORKING_H
#define _LETSNETWORKING_H

/* Includes  -------------------------------------------------------------------*/
#include "stdint.h"
#include "light_drivers.h"

/****************************************/
#define HEADER_VERSION		0//��һ��

#define HEADER_LESHI		0

#define HEADER_MESH       		0//����ͨ��֡
#define HEADER_REMOTE     		1//ң�ؿ���֡
#define HEADER_PASSSW     		2//��Դ����֡
#define HEADER_PASSSW_NONET     3//��Դ����֡

#define PASSIVESW_TYPE    	0xc1
#define PASSIVESW_HEAD_V	0		// V1�汾��ң����֡
#define PASSIVESW_HEAD_T	2		// ��Դ����֡
#define PASSIVESW_K2		0x01	//K2-4���ص�
#define PASSIVESW_K3		0x81	//K3-3���ص�
#define PASSIVESW_G1		0x82	//G1-�����ص�

#define PASSIVESW_PKEY		0		//����0--����ֵ
#define PASSIVESW_VKEY		0x7F	//����0--����ֵ
#define PASSIVESW_VIO		0x80	//����0--����ֵ
#define PASSIVESW_ONDB      0x81    //˫�࿪�ز���
#define PASSIVESW_ONSG		0xC1	//�������ز���
#define PASSIVESW_TIME		25		//��Դ���ؼ��

#define REMOTE_TYPE    		0xFE	// ң��������
#define REMOTE_MAXLEN		18		// ң��������������λ��
#define REMOTE_LMAXLEN		11		// ��ʿң��������������λ��
#define REMOTE_HEAD_V		0		// V1�汾��ң����֡
#define REMOTE_HEAD_T		1		// ң��֡
#define REMOTE_CLEN			10		// �޲���ʱ������-������CRC

#define REMOTE_COLORSEL		0
#define REMOTE_COLORGREA	3

#define REMOTE_WCOLOR		0
#define REMOTE_YCOLOR		1
#define REMOTE_MAXCOLOR		0x80
#define REMOTE_SCENESEL     0 /* Scene */
#define REMOTE_PERON        0 /* ���⿪��ʹ�� */

/* ���^ */


/* defines  -------------------------------------------------------------------*/

#define      OEM_DRIVER_TYPE           LETS_OEM_CENTER          //�豸����--����
#define      OEM_DRIVER_LAYER          LETS_LAYER_CENTER        //���س�ʼ����
#define      OEM_DRIVER_CHECK(type)   (OEM_DRIVER_TYPE != type) //�������ͼ��

#define      OEM_DRIVER_PACK_LENGTH                  6          //�豸��Ϣ����
#define      OEM_INFOR_PACK_LENGTH                   12         //�豸��Ϣ+������Ϣ

#define      LETS_LAMP_MAX                           32         //������Ӷ˵�
#define      LETS_LAMP_INFOR                         6          //����Ϣ
#define      LETS_LAMP_PACK                          12         //����Ϣ��:LETS_LAMP_INFOR + OEM_DRIVER_PACK_LENGTH

#define      LETS_PACK_DATABLE                       11         //���ݰ�����
#define      LETS_PACK_LENBLE                        27         //���ݰ�����

#define      LETS_PASSIVE_SW_LENCRC                  14         //
#define      LETS_PASSIVE_SW_LENGHT                  15         //��Դ����ң��������

#define      LETS_PACK_ACKWAIT                       3          //Ӧ���ӳ�
#define      LETS_PACK_SENTCNT                       4          //����Ƶ��4��

#define      LETS_LAYER_OTHERCNT                     8          //�㲥/��������-8����ת��
#define      LETS_BROADCAST_CNT                      16         //�㲥/��������-16����
#define      LETS_BROADCAST_WAIT                     500        //500ms�ӳ�-��ֹ������ײ

#define      LETS_RF_WROK                            10000      //10S
#define      LETS_RF_WAKEUP                          16         //16MS--����ʱ��15-8=8MS
#define      LETS_RF_SLEEP                           8          // 8MS--����ʱ��8MS

#define      LETS_BROADCAST_ID                       0xffffffff             //�㲥Ŀ��--�㲥ID
/* type  ----------------------------------------------------------------------*/
//�������������ݴ�С������--���б�������ʹ���ֽ�����
//��λ���ر���
//�豸��Ϣ
#pragma pack(1)
typedef union
{
	struct
	{
		uint8_t    Type;      //�豸����
		uint32_t   ID;        //�豸ID
		uint8_t    Layer;     //�豸�ڼ���
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
	struct//ң��������
	{
		Head_BIT	Head;    	//1byte ֡ͷ
		uint8_t 	PackId;     //1byte
		uint32_t	RemoteID;   //4bytes
		uint8_t	 	LampID;     //1byte
		uint8_t		Type;       //1byte
		uint8_t 	Cmd;        //1byte
		uint8_t		Len;        //1byte
		uint8_t 	Pter[8];    //10+8bytes
	};

#else
	struct//ң��������
	{
		Head_BIT	Head;    	//1byte ֡ͷ
		uint32_t	RemoteID;   //4bytes
		uint8_t		Type;       //1byte
		uint8_t	 	LampID;     //1byte
		uint8_t 	PackId;     //1byte
		uint8_t 	Cmd;        //1byte
		uint8_t		Len;        //1byte
		uint8_t 	Pter[8];    //10+8bytes
	};
#endif

	struct//��Դ��������
	{
		Head_BIT		pHead;    	/*֡ͷ */
		uint32_t 		pID;
		uint8_t	 		pSubType;
		uint8_t 		pPter[12];   //6+12
	};
	uint8_t pack[REMOTE_MAXLEN];
} RemoteType;

typedef union
{
	struct //��ʿң��������
	{
		uint8_t			UUF;        //1byte ��ʿң�ر�ʶ��00
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
		Head_BIT		pHead;    /* ֡ͷ */
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


