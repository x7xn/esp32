/*
 * light.h
 *
 *  Created on: 2019��4��18��
 *      Author: Administrator
 */

#ifndef LIGHT_H_
#define LIGHT_H_

#include "stdint.h"
#include "miio_instance.h"
#include "miio_arch.h"
#include "jsmi.h"
#include "../../esp-idf/components/driver/include/driver/ledc.h"
#include "../../esp-idf/components/driver/include/driver/gpio.h"
#include "../../esp-idf/components/driver/include/driver/uart.h"

#define	AIRCONDITION_SAVE_NAME_SPACE	"air_info"		//�洢�ռ���
#define	AIRCONDITION_KEY_ADDR			"air_addr"		//KEY-�յ����ص�ַ
#define AIRCONDITION_KEY_POWER			"air_powercnt"	//KEY-�ϵ����

#define LIGHT_SAVE_NAME			"le_wyfan"		//�洢�ռ���
#define LIGHT_KEY_STATE			"le_infor"		//KEY-״̬
#define LIGHT_KEY_USER			"le_user"		//KEY-״̬
#define LIGHT_KEY_POWER			"le_powercnt"	//KEY-���ٷֶ�
#define LIGHT_KEY_MATCH			"le_match"		//KEY-ң����
#define LIGHT_KEY_SCEN			"le_SCEN"		//KEY-�龰
#define LIGHT_KEY_PONSETBIT		"le_ponsetbit"	//KEY-�ֶο��ر�־

//@AIRCONDITION_CMD_TYPE_MASK �յ���������
#define AIRCONDITION_CMD_SW		((uint8_t)0X01)			//�յ�����-���ƿ���
#define AIRCONDITION_CMD_MODE	((uint8_t)0X02)			//�յ�����-����ģʽ
#define AIRCONDITION_CMD_FAN	((uint8_t)0X04)			//�յ�����-���Ʒ���
#define AIRCONDITION_CMD_TEMP	((uint8_t)0X08)			//�յ�����-�����¶�

//@AIRCONDITION_NOTIFY_TYPE_MASK �յ��ϱ�����
#define AIRCONDITION_NOTIFY_SW		((uint8_t)0X01)			//�յ��ϱ�-����
#define AIRCONDITION_NOTIFY_MODE	((uint8_t)0X02)			//�յ��ϱ�-ģʽ
#define AIRCONDITION_NOTIFY_FAN		((uint8_t)0X04)			//�յ��ϱ�-����
#define AIRCONDITION_NOTIFY_TEMP	((uint8_t)0X08)			//�յ��ϱ�-�¶�
#define AIRCONDITION_NOTIFY_FAULT	((uint8_t)0X10)			//�յ��ϱ�-������
#define AIRCONDITION_NOTIFY_NAME	((uint8_t)0X20)			//�յ��ϱ�-name
#define AIRCONDITION_NOTIFY_GROUP	((uint8_t)0X40)			//�յ��ϱ�-GROUP
#define AIRCONDITION_NOTIFY_NAME1	((uint8_t)0X80)			//�յ��ϱ�-name


#pragma pack(1)

/*
 * �յ��ڴ��д�ṹ��
 */
typedef struct _aircondition_storage_info
{
	int   	aircondition_count;		// �յ����ؼ�⵽�Ŀյ�����
	uint16_t  	aircondition_addr[17];		// �յ����ص�ַ
} aircondition_storage_info_t;

typedef struct
{
	uint8_t name_save_f;//�����־λ
	uint16_t name_save_timer;//�����ʱ
	char group_name[38];
}aircondition_name_t;

/*
 * APP�˿յ��ڴ��д�ṹ��
 */
 /*
typedef struct _aircondition_app_storage_info
{
	aircondition_name_t  	aircondition_name[16];		// �յ�name����
} aircondition_app_storage_info_t;
*/
/*
 * ��״̬�Ľṹ��
 */
typedef struct _lewyfan_device_infor
{
	uint8_t   	light_sw;				// ���� 	1�� 0��
	uint16_t  	light_brightness;		// ����	0-1000
	uint16_t  	light_templatetrue;		// ɫ��	3000-6400

	uint8_t 	light_rcolor;			// R
	uint8_t 	light_gcolor;			// G
	uint8_t 	light_bcolor;			// B
	uint8_t 	light_rgbmode;			// RGBģʽ    0-wy����ģʽ  1-RGB����ģʽ
	uint8_t 	power_state;			// �ϵ翪��״̬
	uint8_t 	user_mode;				// �ϵ��Ƿ���û�����״̬
	uint16_t 	fade_on;				// ����/����
	uint16_t 	fade_off;				// ����/����
	uint16_t 	fade_scen;				// ����/����
	uint8_t 	smart_switch;			// �趯����
	uint8_t 	remote_switch;			// ң��������
	uint16_t  	lowest_brightness;		// �������
#define APP_LOCK_RSTSET	0x01
#define APP_LOCK_PERSET	0x02
#define APP_LOCK_REMOTE 0x04
	uint8_t		lock_state;				// ��������0x01 ��������0x02

	uint8_t 	modetime;				// ģʽʱ��
	uint8_t 	weakebr;				// ����ģʽ��������
	uint8_t 	sleepsbr;				// ����ģʽ��ʼ����

} lewyfan_device_infor_t;

/*
 * �յ�״̬�Ľṹ��
 */
typedef struct _aircondition_device_infor
{
	uint8_t		aircondition_addr[2];			//�յ����ص�ַ
	
	uint8_t   	aircondition_sw;				// ����״̬ 	1�� 0��
	uint8_t 	aircondition_mode;				// �յ�ģʽ     1-����   2-���� 3-�ͷ� 4-��ʪ
	uint8_t  	aircondition_set_templatetrue;	//�����ȶ�	16-30��
	uint8_t 	aircondition_fault;				//�յ�������
	uint8_t 	aircondition_fanspeed;			//�յ����� 0-�Զ� 1-���� 2-���� 3-����
	
	uint8_t 	aircondition_local_templatetrue;//�յ����ı����¶�
	uint8_t 	aircondition_slave_master;		//�յ�������
	//uint8_t 	aircondition_communicate_status;//�յ�ͨ��״̬
	uint8_t		aircondition_control_f;			//�յ���Ҫ���Ƶ����ݱ�־λ @refer AIRCONDITION_CMD_TYPE_MASK
	uint8_t		aircondition_update_f;			//�յ�״̬�Ӵ����и��µı�־λ @refer AIRCONDITION_CMD_TYPE_MASK
	uint8_t		aircondition_notify_f;			//�յ���Ҫ���µ�ƽ̨�����ݱ�־λ @refer AIRCONDITION_NOTIFY_TYPE_MASK
} aircondition_device_infor_t;


/*
 * ���ڱ����û���APP�����õĵ�״̬
 */
typedef struct _lewyfan_device_userinfor
{
uint16_t  light_brightness;
uint16_t  light_templatetrue;

uint8_t light_rcolor;
uint8_t light_gcolor;
uint8_t light_bcolor;
uint8_t light_rgbmode;
}lewyfan_device_userinfor_t;
#pragma pack()

void light_ResetData(uint8_t flag);
void light_startinit(void);
int  light_task_start(miio_handle_t handle);

//S2 get
uint8_t		 light_getlight_sw(void);
unsigned int light_getlight_brightness(void);
unsigned int light_getlight_templatetrue(void);
unsigned int light_getlight_rgbcolor(void);
unsigned int light_getlight_rgbmode(void);
unsigned int light_getlight_smartswitch(void);
uint8_t		 light_getlight_sleep(void);
uint8_t		 light_getlight_weakup(void);
unsigned int light_getlight_modetime(void);
unsigned int light_getlight_weakebr(void);
unsigned int light_getlight_sleepsbr(void);
unsigned int light_getlight_powerstate(void);
//unsigned int aircondition_get_fan(void);
//unsigned int aircondition_get_sw(void);
#if 0
unsigned int aircondition_get_mode(void);
unsigned int aircondition_get_fault(void);
unsigned int aircondition_get_temp(void);
unsigned int aircondition_get_group(void);
char* 		aircondition_get_groupname(uint8_t group_id);
unsigned int aircondition_get_notify_name_f(void);
unsigned int aircondition_get_connected_num(void);
#endif

//S2 set
void 		 light_setlight_sw(uint8_t sw);
void 		 light_setlight_brightness(unsigned int brightness);
void 		 light_setlight_templatetrue(unsigned int templatetrue);
void 		 light_setlight_rgbcolor(unsigned int rgbcolor);
void 		 light_setlight_rgbmode(unsigned int rgbcolor);
void 		 light_setlight_smartswitch(unsigned int state);
void 		 light_setlight_sleep(uint8_t sw);
void 		 light_setlight_weakup(uint8_t sw);
void 		 light_setlight_powerstate(unsigned int state);


//S4 get
unsigned int light_getlight_usermode(void);
unsigned int light_getlight_fadeon(void);
unsigned int light_getlight_fadeoff(void);
unsigned int light_getlight_fadescen(void);
unsigned int light_getlight_nightswitch(void);
unsigned int light_getlight_timeroff(void);
unsigned int light_getlight_lowestbrightness(void);
unsigned int light_getlight_lockstate(void);



//S4 set
void 		 light_setlight_usermode(unsigned int state);
void 		 light_setlight_fadeon(unsigned int state);
void 		 light_setlight_fadeoff(unsigned int state);
void 		 light_setlight_fadescen(unsigned int state);
void 		 light_setlight_nightswitch(unsigned int state);
void 		 light_setlight_timeroff(unsigned int state);
void 		 light_setlight_lowestbrightness(unsigned int state);
void 		 light_setlight_lockstate(unsigned int state);

//action
void 		 light_setlight_modetime(unsigned int state);
void 		 light_setlight_weakebr(unsigned int state);
void 		 light_setlight_sleepsbr(unsigned int state);



//S3
void 		 light_setlight_onoff(uint8_t sw);
void 		 light_setlight_bradd(uint8_t sw);
void 		aircondition_set_mode(unsigned int mode);

void 		 light_setlight_brdec(uint8_t sw);
void 		 light_setlight_brsw(uint8_t sw);

#if 0
void 		aircondition_set_temp(unsigned int temp);
void 		aircondition_set_fanspeed(unsigned int fanspeed);
void 		aircondition_set_sw(uint8_t sw);
void 		aircondition_set_mode(unsigned int mode);
void 		aircondition_set_group(uint8_t sw);
#endif
//void 		aircondition_set_groupname(uint8_t group_id,char* groupname,uint32_t groupname_len);
void 		aircondition_set_notify_name(uint8_t group_value);
void 		aircondition_set_sw_toggle(void);


void 		 light_setlight_onbrsw(uint8_t sw);

void 		 light_setlight_tmadd(uint8_t sw);
void 		 light_setlight_tmdec(uint8_t sw);
void 		 light_setlight_tmsw(uint8_t sw);
void 		 light_setlight_ontmsw(uint8_t sw);

void 		 light_setlight_rgbsw(uint8_t sw);
void 		 light_setlight_onrgbsw(uint8_t sw);


//S8
unsigned int light_getlight_remoteswitch(void);
unsigned int light_getlight_remotetype(unsigned int islow);
unsigned int light_getlight_remotemac(unsigned int remotenum);
unsigned int light_getlight_remotecmd(void);

void		 light_setlight_remoteswitch(unsigned int state);
void		 light_setlight_remoteset(unsigned int state);











uint8_t  	 GetCRC(uint8_t *buf,uint8_t len);
uint8_t 	 GetLeiShiCRC(uint8_t *buf, uint8_t len);

int  light_blesend(uint8_t *pdata,uint8_t plen);
int  light_blesendstop(void);
void light_blename_send(uint8_t *name,uint8_t len);

#endif /* LIGHT_H_ */
