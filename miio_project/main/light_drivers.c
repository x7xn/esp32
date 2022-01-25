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
#include "LeCmd.h"
#include "LetsNetwork.h"
//#include "light.h"
#include "light_remote.h"
#include "light_drivers.h"


extern lewyfan_device_userinfor_t  	lewyfan_userinfor;
extern lewyfan_device_infor_t 		lewyfan_infor;
extern remote_task_t   		  		remote_sys;
extern uint8_t  	          		mReset;

//内部变量
uint8_t	  	  rPower_ACCheck	 = 1;
uint32_t	  rTimer_Second		 = 0;

uint32_t	  sLamp_brightness   = 0;				//灯亮度缓存器
uint32_t  	  sLamp_templatetrue = 3000;			//灯色温缓存器
uint32_t  	  sLamp_RValue = 0;						//灯色温缓存器
uint32_t  	  sLamp_GValue = 0;						//灯色温缓存器
uint32_t  	  sLamp_BValue = 0;						//灯色温缓存器

uint32_t	  rLamp_brightness   = 0;				//灯亮度寄存器
uint32_t  	  rLamp_templatetrue = 3000;			//灯色温寄存器
uint32_t  	  rLamp_RValue = 0;						//灯色温缓存器
uint32_t  	  rLamp_GValue = 0;						//灯色温缓存器
uint32_t  	  rLamp_BValue = 0;						//灯色温缓存器

uint32_t	  dLamp_brightness   = 1;				//灯亮度增量寄存器
uint32_t  	  dLamp_templatetrue = 1;				//灯色温增量寄存器
uint32_t  	  dLamp_RValue = 1;						//灯色温增量缓存器
uint32_t  	  dLamp_GValue = 1;						//灯色温增量缓存器
uint32_t  	  dLamp_BValue = 1;						//灯色温增量缓存器
uint32_t	  dLamp_Timer  = 1;

const uint8_t mRGM_FADE_COLOR[LIGHT_RGB_FADECNT][3]=
{
		{LIGHT_RGB_VMAX,0x00,			0x00},
		{LIGHT_RGB_VMAX,LIGHT_RGB_VMAX,	0x00},
		{0x00,			LIGHT_RGB_VMAX,	0x00},
		{0x00,			LIGHT_RGB_VMAX,	LIGHT_RGB_VMAX},
		{0x00,			0x00,			LIGHT_RGB_VMAX},
		{LIGHT_RGB_VMAX,0x00,			LIGHT_RGB_VMAX},
		{LIGHT_RGB_VMAX,LIGHT_RGB_VMAX,	LIGHT_RGB_VMAX}
};
//灯控部分PWM
ledc_timer_config_t light_timer = {
		.duty_resolution = LIGHT_BIT_DUTY, 	  // resolution of PWM duty
        .freq_hz	     = LIGHT_FRE_P1,      // frequency of PWM signal
        .speed_mode 	 = LIGHT_MODE,        // timer mode
        .timer_num 		 = LIGHT_TIMER        // timer index
        };

//灯控部分PWM
ledc_channel_config_t light_channel[LIGHT_TOTAL_NUM] = {
		{ .channel = LIGHT_W_CHANNEL, .duty = 0, .gpio_num = LIGHT_W_GPIO, .speed_mode = LIGHT_MODE, .timer_sel = LIGHT_TIMER},
		{ .channel = LIGHT_Y_CHANNEL, .duty = 0, .gpio_num = LIGHT_Y_GPIO, .speed_mode = LIGHT_MODE, .timer_sel = LIGHT_TIMER},
		{ .channel = LIGHT_R_CHANNEL, .duty = 0, .gpio_num = LIGHT_R_GPIO, .speed_mode = LIGHT_MODE, .timer_sel = LIGHT_TIMER},
		{ .channel = LIGHT_G_CHANNEL, .duty = 0, .gpio_num = LIGHT_G_GPIO, .speed_mode = LIGHT_MODE, .timer_sel = LIGHT_TIMER},
		{ .channel = LIGHT_B_CHANNEL, .duty = 0, .gpio_num = LIGHT_B_GPIO, .speed_mode = LIGHT_MODE, .timer_sel = LIGHT_TIMER}
};

//内部函数
void light_driver_set(uint32_t brightness,uint32_t templatetrue,uint8_t r,uint8_t g, uint8_t b);
void light_driver_setw(uint32_t duty);
void light_driver_sety(uint32_t duty);

void light_driver_setr(uint32_t duty);
void light_driver_setg(uint32_t duty);
void light_driver_setb(uint32_t duty);

static aircondition_device_infor_t aircondition_driver_list[17] = {0};

#define	WAIT_UART_REPLY_DURATION	((uint16_t)20)/*<����ͨ����С���ʱ��>*/
#define	WAIT_UART_QUERY_STATUS_DURATION	((uint16_t)1000)/*<��ѯ���ʱ��-����ʹ��10000 ����ʹ��1000>*/
#define	WAIT_UART_QUERY_DEVICE_DURATION	((uint16_t)500)/*<��ѯ���ʱ��-����ʹ��10000 ����ʹ��1000>*/

static volatile TickType_t xUartCommunicationTickCount = 0;//����ͨ��ʱ��

static volatile TickType_t xUartQueryTickCount = 0;//��ѯʱ���

static uint8_t m_query_aircondition_task = 0;//�յ���ѯ����  0-������ 1-��ѯ״̬ 2-��ѯ�����ӿյ��ļ��ص�ַ
static uint8_t m_aircondition_connect_count = 0;//�����ӵĿյ�����
static uint8_t m_aircondition_addr_save_f = 0;//��ȡ�����ص�ַ��Ҫ����

/*
*���ڴ��ж�ȡ���Ŀյ���Ϣ���µ�driver����
*@param-aircondition_device_info[IN]:�յ��豸��Ϣ
*/
void update_aircondition_addr_to_driver(aircondition_storage_info_t *aircondition_device_info)
{
	uint8_t i = 0;

	//���������ӵĸ���
	m_aircondition_connect_count = aircondition_device_info->aircondition_count;

	//���¶�Ӧ�Ŀյ���ַ
	for(i = 0; i < m_aircondition_connect_count;i++)
	{
		aircondition_driver_list[i].aircondition_addr[0] = aircondition_device_info->aircondition_addr[i] >> 8;
		aircondition_driver_list[i].aircondition_addr[1] = aircondition_device_info->aircondition_addr[i];
	}
}


/*
*�����ѯ����
*task_type[IN]:�������� 0-������ 1-��ѯ״̬ 2-��ѯ�����ӿյ��ļ��ص�ַ
*/
void pending_query_aircondition_task(uint8_t task_type)
{
	m_query_aircondition_task = task_type;
}

/*
*����յ����ڿ�������
*@param-aircondition_index[IN]:	Ҫ���ƵĿյ��±�
*@param-cmd_type[IN]:			Ҫ���Ƶ����� refer@aircond
*@param-aircondition_info[IN]:	Ҫ���Ƶ���������
*/
void pending_aircondition_uart_task(uint8_t aircondition_index,uint8_t cmd_type,aircondition_device_infor_t aircondition_info)
{
	uint8_t i = 0;
	
	//�ܿ�
	if(aircondition_index == 16)
	{
		//�����豸������
		for(; i < m_aircondition_connect_count; i++)
		{
			//���𴮿ڷ�������
			aircondition_driver_list[i].aircondition_control_f |= cmd_type;
		
			//����Ҫ���ƵĲ���
			if(cmd_type & AIRCONDITION_CMD_SW)
			{
				aircondition_driver_list[i].aircondition_sw = aircondition_info.aircondition_sw;
			}
			if(cmd_type & AIRCONDITION_CMD_MODE)
			{
				switch (aircondition_info.aircondition_mode)
				{
					case 1://����
						aircondition_driver_list[i].aircondition_mode = 1;
						break;
		
					case 2://����
						aircondition_driver_list[i].aircondition_mode = 8;
						break;
		
					case 3://�ͷ�
						aircondition_driver_list[i].aircondition_mode = 4;
						break;
					
					case 4://��ʪ
						aircondition_driver_list[i].aircondition_mode = 2;
						break;
		
					default:
						break;
				}
			}
			if(cmd_type & AIRCONDITION_CMD_FAN)
			{
				switch (aircondition_info.aircondition_fanspeed)
				{
					case 0://�Զ�
						aircondition_driver_list[i].aircondition_fanspeed = 0;
						break;
		
					case 1://����
						aircondition_driver_list[i].aircondition_fanspeed = 4;
						break;
		
					case 2://����
						aircondition_driver_list[i].aircondition_fanspeed = 2;
						break;
					
					case 3://����
						aircondition_driver_list[i].aircondition_fanspeed = 1;
						break;
		
					default:
						break;
				}
			}
			if(cmd_type & AIRCONDITION_CMD_TEMP)
			{
				aircondition_driver_list[i].aircondition_set_templatetrue = aircondition_info.aircondition_set_templatetrue;
			}
		}
	}
	else
	{
		//���𴮿ڷ�������
		aircondition_driver_list[aircondition_index].aircondition_control_f |= cmd_type;
	
		//����Ҫ���ƵĲ���
		if(cmd_type & AIRCONDITION_CMD_SW)
		{
			aircondition_driver_list[aircondition_index].aircondition_sw = aircondition_info.aircondition_sw;
		}
		if(cmd_type & AIRCONDITION_CMD_MODE)
		{
			switch (aircondition_info.aircondition_mode)
			{
				case 1://����
					aircondition_driver_list[aircondition_index].aircondition_mode = 1;
					break;
	
				case 2://����
					aircondition_driver_list[aircondition_index].aircondition_mode = 8;
					break;
	
				case 3://�ͷ�
					aircondition_driver_list[aircondition_index].aircondition_mode = 4;
					break;
				
				case 4://��ʪ
					aircondition_driver_list[aircondition_index].aircondition_mode = 2;
					break;
	
				default:
					break;
			}
		}
		if(cmd_type & AIRCONDITION_CMD_FAN)
		{
			switch (aircondition_info.aircondition_fanspeed)
			{
				case 0://�Զ�
					aircondition_driver_list[aircondition_index].aircondition_fanspeed = 0;
					break;
	
				case 1://����
					aircondition_driver_list[aircondition_index].aircondition_fanspeed = 4;
					break;
	
				case 2://����
					aircondition_driver_list[aircondition_index].aircondition_fanspeed = 2;
					break;
				
				case 3://����
					aircondition_driver_list[aircondition_index].aircondition_fanspeed = 1;
					break;
	
				default:
					break;
			}
		}
		if(cmd_type & AIRCONDITION_CMD_TEMP)
		{
			aircondition_driver_list[aircondition_index].aircondition_set_templatetrue = aircondition_info.aircondition_set_templatetrue;
		}
	}
	
}

/*
*�����У�飬�������
*@param-source_data[IN]:	��Ҫ����У��͵�����
*@param-data_len[IN]:		��Ҫ����ĳ���
*@return:					����õ���У��ֵ
*/
uint8_t calculate_uart_checksum(uint8_t*source_data,uint8_t data_len)
{
	uint8_t i = 0;
	uint8_t checksum = 0;

	for(; i < data_len;i++)
	{
		checksum += source_data[i];
	}

	return checksum;
}

/*
*�������ָ��
*@return: 0-�����ڰ��ָ�� ����-���ָ��
*/
uint8_t parse_board_instruction(uint8_t *source_data)
{
	uint8_t detect_instruction[9] = {0x01,0xA5,0x66,0xFF,0xFF,0xFF,0x88,0x5A,0x8F};
	uint8_t i = 0;

	for(i = 0; i < 9; i++)
	{
		if(source_data[i] != detect_instruction[i])
		{
			return 0;
		}
	}

	return 1;
}


/*
*���ָ��ظ�
*ack_type:0-���ָ�� 1-����ָ�� 2-��������ָ�� 3-��ѯ����ָ��
*/
void aircondition_driver_uartAck(uint8_t ack_type)
{
	//���ָ��ظ�
	uint8_t  board_instruction[9] = {0x01,0x25,0x25,0xFF,0xFF,0xFF,0x99,0xC0,0x8C};
	//����ָ��ظ�
	uint8_t  clear_net[9] = {0x01,0xA2,0x55,0xFF,0xFF,0xFF,0xF5,0x9F,0x1D};
	//��������ָ��
	uint8_t  enable_net_ok[9] = {0x01,0xA2,0xAA,0xFF,0xFF,0xFF,0x4A,0xCA,0xB9};
	//��������ָ��
	uint8_t  enable_net_fail[9] = {0x01,0xA3,0xAA,0xFF,0xFF,0xFF,0x4B,0x0A,0xA8};
	//��ѯ����״̬
	uint8_t  query_net_ok[9] = {0x01,0xA0,0xAA,0x01,0x01,0x01,0x4E,0xDA,0xE0};
	//��ѯ����״̬
	uint8_t  query_net_fail[9] = {0x01,0xA0,0xAA,0x00,0x00,0x00,0x4B,0x4B,0x4F};

	switch(ack_type)
	{
		//���ָ��
		case 0:
			uart_write_bytes(UART_NUM_1,(const char *)board_instruction,9);
			break;
		//����ָ��
		case 1:
			uart_write_bytes(UART_NUM_1,(const char *)clear_net,7);
			enable_connect_net();
			break;
			
		//��������ָ��
		case 2:
			if(get_allow_net_status())
			{
				uart_write_bytes(UART_NUM_1,(const char *)enable_net_ok,7);
				enable_connect_net();
			}
			else
			{
				uart_write_bytes(UART_NUM_1,(const char *)enable_net_fail,7);
			}
			break;
		//��ѯ����״ָ̬��
		case 3:
			//��ȡ������Ϣ
			if(get_wifi_connect_status())
			{
				uart_write_bytes(UART_NUM_1,(const char *)query_net_ok,7);
			}
			else
			{
				uart_write_bytes(UART_NUM_1,(const char *)query_net_fail,7);
			}
			break;
		
		default:
			break;
	}

#if 0
	//arch_printf("UART1 TX:\r\n");
	uint8_t rxi;
	for(rxi=0 ;rxi < offsetcrc; rxi++)
	{
		//arch_printf("%02x ",board_instruction[rxi]);
	}
	//arch_printf("\r\n");
#endif

}

/*
*��������ָ��
*type: 1-����ָ�� 2-�������� 3-��ѯ����
*@return: 0-����������ָ�� ����-����ָ��
*/
uint8_t reset_net_instruction(uint8_t *source_data,uint8_t type)
{
	uint8_t clear_net[9] = {0x01,0xA1,0x55,0xFF,0xFF,0xFF,0xF4,0x5E,0xEE};
	uint8_t enable_config_net[9] = {0x01,0xA1,0xAA,0xFF,0xFF,0xFF,0x49,0x8A,0x8B};
	uint8_t query_net_sta[9] = {0x01,0xA0,0xAA,0xFF,0xFF,0xFF,0x48,0x4A,0x9A};
	uint8_t i = 0;

	//��������
	if(type == 1)
	{
		for(i = 0; i < 7; i++)
		{
			if(source_data[i] != clear_net[i])
			{
				return 0;
			}
		}
	}
	//��������
	else if(type == 2)
	{
		for(i = 0; i < 7; i++)
		{
			if(source_data[i] != enable_config_net[i])
			{
				return 0;
			}
		}
	}
	//��ѯ����״̬
	else if(type == 3)
	{
		for(i = 0; i < 7; i++)
		{
			if(source_data[i] != query_net_sta[i])
			{
				return 0;
			}
		}
	}
	else
	{
		return 0;
	}

	return 1;
}

/*
*���ݴ��ڷ��ص����ݸ��¿յ���״̬
*@param-aircondition_status[IN]:�յ�����
*@param-aircondition_count[IN]:�յ�����
*/
void update_aircondition_status_from_uart(uint8_t* aircondition_status,uint8_t aircondition_count)
{
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t n = 0;

	for(n = 0; n < aircondition_count; n++)
	{
		//����Ҫ�Աȼ��ص�ַ
		if(m_query_aircondition_task == 2)
		{
			//arch_printf("device_from_uart\r\n");
			//���¿յ�״̬
			aircondition_driver_list[n].aircondition_addr[0] = aircondition_status[i];
			aircondition_driver_list[n].aircondition_addr[1] = aircondition_status[i+1];
			aircondition_driver_list[n].aircondition_sw = aircondition_status[i+2];
			aircondition_driver_list[n].aircondition_set_templatetrue = aircondition_status[i+3];
			aircondition_driver_list[n].aircondition_mode = aircondition_status[i+4];
			aircondition_driver_list[n].aircondition_fanspeed = aircondition_status[i+5];
			aircondition_driver_list[n].aircondition_local_templatetrue= aircondition_status[i+6];
			aircondition_driver_list[n].aircondition_fault = aircondition_status[i+7];
			aircondition_driver_list[n].aircondition_slave_master= aircondition_status[i+9];

			m_aircondition_connect_count++;//���������Ӹ���
			m_aircondition_addr_save_f = 1;//���𱣴�յ���ַ��־λ
			aircondition_driver_list[n].aircondition_update_f = 0x01;//����״̬Ҳ�ϱ�
		}
		else
		{
			for(j = 0; j < 16; j++)
			{
				//���ݼ��ص�ַ�ҵ���Ӧ�յ�
				if((aircondition_status[i] == aircondition_driver_list[j].aircondition_addr[0]) && (aircondition_status[i+1] == aircondition_driver_list[j].aircondition_addr[1]))
				{
					//arch_printf("status_from_uart\r\n");
					if(!aircondition_driver_list[j].aircondition_control_f)
					{
						//���¿յ�״̬
						aircondition_driver_list[j].aircondition_sw = aircondition_status[i+2];
						aircondition_driver_list[j].aircondition_set_templatetrue = aircondition_status[i+3];
						aircondition_driver_list[j].aircondition_mode = aircondition_status[i+4];
						aircondition_driver_list[j].aircondition_fanspeed = aircondition_status[i+5];
						aircondition_driver_list[j].aircondition_local_templatetrue= aircondition_status[i+6];
						aircondition_driver_list[j].aircondition_fault = aircondition_status[i+7];
						aircondition_driver_list[j].aircondition_slave_master= aircondition_status[i+9];
						aircondition_driver_list[j].aircondition_update_f = 0x01;
					}
				}
				
			}
		}
		
		i+=10;
	}
}
/*
*�״�ʹ�ã���ȡ���ӵ��ڻ�����Ϣ��������ַ���������
*@param-aircondition_status[IN]:�����л�ȡ���Ļظ���Ϣ
*@param-aircondition_count[IN]:��ȡ���Ŀյ��ĸ���
*/
/*void update_aircondition_communication_status(uint8_t* aircondition_status,uint8_t aircondition_count)
{
	uint8_t i = 0;
	uint8_t n = 0;

	for(; n < aircondition_count; n++)
	{
		//��ȡ�յ���ַ
		aircondition_driver_list[n].aircondition_addr[0] = aircondition_status[i];
		aircondition_driver_list[n].aircondition_addr[1] = aircondition_status[i+1];
		aircondition_driver_list[n].aircondition_communicate_status = aircondition_status[i+2];
		
		i+=3;
	}

	//������Ϊ0
	if(aircondition_count)
	{
		//������Ϣ�������
		update_aircondition_addr_info(aircondition_driver_list,aircondition_count);
	}
}
*/
/*
*�������ݴ���
*@param-len[IN]:	�Ӵ����ж�ȡ�������ݵĳ���
*@param-data[IN]:	�Ӵ����ж�ȡ���������׵�ַ
*@return:
*/
uint32_t uart1_rx_data(uint32_t len,uint8_t *data)
{
	uint8_t  cmd = 0;
	uint16_t dlen = 0;
	uint16_t result_code = 0;
	/*
	*->���ָ�		01 A5 66 FF FF FF 88 5A 8F 
	*<-���ָ��ظ��� 01 25 25 FF FF FF 99 C0 8C 
	*
	*���ã����������Ϣ������ģ�飬������������
	*����->ģ��
	*01 A1 55 FF FF FF F4 5E EE 
	*ģ��->����
	*�ɹ�
	*01 A2 55 FF FF FF F5
	*ʧ��
	*01 A3 55 FF FF FF F6

	*��������ָ��
	*01 A1 AA FF FF FF 49 8A 8B 
	*

	*��ѯ����״̬
	*01 A0 AA FF FF FF 48 4A 9A 
	
	*->�յ�״̬: 		01 50 ff air_condition_count {air_condition_0_status 10bytes} {air_condition_1_status 10bytes}
	*... {air_condition_n_status 10bytes} checksum
	*{air_condition_0_status 10bytes}
	*�������ַ�����ڻ���ַ��������״̬���������¶ȡ���ģʽ�� �����١� �������¶ȡ� �������롿 ������ �����ӻ���
	*
	*
	*->�յ�����״̬	01 50 02 air_condition_count {air_condition_0_status 3bytes} {air_condition_1_status 3bytes}
	*...{air_condition_n_status 3bytes} checksum
	*
	*{air_condition_n_status 3bytes}
	*�������ַ�����ڻ���ַ��������״̬��
	*/

	
	//���ݳ���С�ڿɽ�������С����,��������
	if(len < 7)
	{
		return len;
	}

	if(data[0] != 0x01)return 1;

	if((data[1] != 0xa5) && (data[1] != 0xa1)  && (data[1] != 0xa0) && (data[1] != 0x50))return 2;

	cmd = data[1];

	switch(cmd)
	{
			//���ָ��
		case 0xa5:
			if(parse_board_instruction(data))
			{
				//�ظ�
				aircondition_driver_uartAck(0);
				result_code = 9;
			}
			else
			{
				result_code = 2;
			}
			break;
			
			//����ָ��
			//��������ָ��
		case 0xa1:
			if(reset_net_instruction(data,1))
			{
				//�ظ�
				aircondition_driver_uartAck(1);
				result_code = 7;
			}
			else if(reset_net_instruction(data,2))
			{
				//�ظ�
				aircondition_driver_uartAck(2);
				result_code = 7;
			}
			else
			{
				result_code = 2;
			}
			break;
	
			//��ѯ����״̬
		case 0xa0:
			if(reset_net_instruction(data,3))
			{
				//�ظ�
				aircondition_driver_uartAck(3);
				result_code = 7;
			}
			else
			{
				result_code = 2;
			}
			break;

			//��ѯ�յ�״̬
		case 0x50:
			//״̬��ѯ�ظ�
			if(data[2] == 0xff)
			{
				//�յ��������Ϸ�
				if(!data[3] ||(data[3] > 16))
				{
					result_code = 4;
				}
				else
				{
					dlen = data[3]*10+5;

					//���ݳ��Ȳ��Ϸ�
					if(len < dlen)
					{
						result_code = 3;
					}
					else
					{
						//�ж�У��λ
						if(calculate_uart_checksum(data,dlen-1) == data[len-1])
						{
							//���¶�Ӧ�յ�����
							update_aircondition_status_from_uart(&data[4],data[3]);
							result_code = dlen;
						}
						else
						{
							result_code = 3;
						}
					}
				}
			}
			#if 0
			//���߲�ѯ�ظ�
			else if(data[2] == 0x02)
			{
				//�յ��������Ϸ�
				if(!data[3] ||(data[3] > 16))
				{
					result_code = 4;
				}
				else
				{
					//�յ��������Ϸ�
					if(!data[3] ||(data[3] > 16))
					{
						result_code = 4;
					}
					else
					{
						dlen = data[3]*3+5;

						//���ݳ��Ȳ��Ϸ�
						if(len < dlen)
						{
							result_code = 3;
						}
						else
						{
							//�ж�У��λ
							if(calculate_uart_checksum(data,dlen-1) == data[len-1])
							{
								//���¶�Ӧ�յ�����
								update_aircondition_communication_status(&data[4],data[3]);
								result_code = dlen;
							}
							else
							{
								result_code = 3;
							}
						}
					}
				}
			}
			#endif
			else
			{
				result_code = 2;
			}
			break;
		
		default:
			break;
	}
	
	
	return result_code;
}

/*
*���ڷ�������
*/
void uart_tx_task(void)
{
	uint8_t i = 0;
	uint8_t send_len = 1;
	uint8_t send_data[10] = {0x01,0x50,0xff,0xff,0xff,0xff,0x4d,};

	//��ȡ��ǰʱ��
	TickType_t xCurrentTickCount = xTaskGetTickCount(); 

	//�ж��Ƿ������
	if(xCurrentTickCount < xUartCommunicationTickCount)
	{
		xUartCommunicationTickCount = 0;
		xUartQueryTickCount = 0;
		return;
	}
	
	//�����ϴ�ͨ��ʱ���ѳ�����С��� 
	if(xCurrentTickCount > (xUartCommunicationTickCount + WAIT_UART_REPLY_DURATION))
	{
		//��ǰ����Ҫִ�еĿ�������
		for(i = 0; i < m_aircondition_connect_count; i++)
		{
			if(aircondition_driver_list[i].aircondition_control_f)
			{
				/*
				//�������
				if(aircondition_driver_list[i].aircondition_control_f >= 0x0f)
				{
					send_data[send_len++] = 0x60;//function_code
					send_data[send_len++] = aircondition_driver_list[i].aircondition_sw;//����״̬
					send_data[send_len++] = aircondition_driver_list[i].aircondition_set_templatetrue;//�����¶�
					send_data[send_len++] = aircondition_driver_list[i].aircondition_mode;//ģʽ
					send_data[send_len++] = aircondition_driver_list[i].aircondition_fanspeed;//����
					send_data[send_len++] = 0x01;//device_count
					send_data[send_len++] = aircondition_driver_list[i].aircondition_addr[0];//���ص�ַH
					send_data[send_len++] = aircondition_driver_list[i].aircondition_addr[1];//���ص�ַL
					send_data[send_len] = calculate_uart_checksum(send_data, send_len);

					
					uart_write_bytes(UART_NUM_1,(const char *) send_data, send_len+1);
					//����ͨ��ʱ��
					xUartCommunicationTickCount = xCurrentTickCount;
					//��������־λ
					aircondition_driver_list[i].aircondition_control_f = 0;
				}
				else 
					*/
				if(aircondition_driver_list[i].aircondition_control_f & AIRCONDITION_CMD_SW)
				{
					
					send_data[send_len++] = 0x31;//function_code
					send_data[send_len++] = aircondition_driver_list[i].aircondition_sw;//����״̬
					send_data[send_len++] = 0x01;//device_count
					send_data[send_len++] = aircondition_driver_list[i].aircondition_addr[0];//���ص�ַH
					send_data[send_len++] = aircondition_driver_list[i].aircondition_addr[1];//���ص�ַL
					send_data[send_len] = calculate_uart_checksum(send_data, send_len);
					
					
					uart_write_bytes(UART_NUM_1,(const char *) send_data, send_len+1);
					//����ͨ��ʱ��
					xUartCommunicationTickCount = xCurrentTickCount;
					//��������־λ
					aircondition_driver_list[i].aircondition_control_f &= (~AIRCONDITION_CMD_SW);
				}
				else if(aircondition_driver_list[i].aircondition_control_f & AIRCONDITION_CMD_MODE)
				{
					
					send_data[send_len++] = 0x33;//function_code
					send_data[send_len++] = aircondition_driver_list[i].aircondition_mode;//ģʽ
					send_data[send_len++] = 0x01;//device_count
					send_data[send_len++] = aircondition_driver_list[i].aircondition_addr[0];//���ص�ַH
					send_data[send_len++] = aircondition_driver_list[i].aircondition_addr[1];//���ص�ַL
					send_data[send_len] = calculate_uart_checksum(send_data, send_len);
					
					
					uart_write_bytes(UART_NUM_1,(const char *) send_data, send_len+1);
					//����ͨ��ʱ��
					xUartCommunicationTickCount = xCurrentTickCount;
					//��������־λ
					aircondition_driver_list[i].aircondition_control_f &= (~AIRCONDITION_CMD_MODE);
				}
				else if(aircondition_driver_list[i].aircondition_control_f & AIRCONDITION_CMD_TEMP)
				{
					
					send_data[send_len++] = 0x32;//function_code
					send_data[send_len++] = aircondition_driver_list[i].aircondition_set_templatetrue;//�����¶�
					send_data[send_len++] = 0x01;//device_count
					send_data[send_len++] = aircondition_driver_list[i].aircondition_addr[0];//���ص�ַH
					send_data[send_len++] = aircondition_driver_list[i].aircondition_addr[1];//���ص�ַL
					send_data[send_len] = calculate_uart_checksum(send_data, send_len);
					
					
					uart_write_bytes(UART_NUM_1,(const char *) send_data, send_len+1);
					//����ͨ��ʱ��
					xUartCommunicationTickCount = xCurrentTickCount;
					//��������־λ
					aircondition_driver_list[i].aircondition_control_f &= (~AIRCONDITION_CMD_TEMP);
				}
				else if(aircondition_driver_list[i].aircondition_control_f & AIRCONDITION_CMD_FAN)
				{
					
					send_data[send_len++] = 0x34;//function_code
					send_data[send_len++] = aircondition_driver_list[i].aircondition_fanspeed;//����
					send_data[send_len++] = 0x01;//device_count
					send_data[send_len++] = aircondition_driver_list[i].aircondition_addr[0];//���ص�ַH
					send_data[send_len++] = aircondition_driver_list[i].aircondition_addr[1];//���ص�ַL
					send_data[send_len] = calculate_uart_checksum(send_data, send_len);
					
					
					uart_write_bytes(UART_NUM_1,(const char *) send_data, send_len+1);
					//����ͨ��ʱ��
					xUartCommunicationTickCount = xCurrentTickCount;
					//��������־λ
					aircondition_driver_list[i].aircondition_control_f &= (~AIRCONDITION_CMD_FAN);
				}

				return;
			}
		}

		//��δ��ȡ�����ص�ַ����ѯ
		if(m_query_aircondition_task == 2)
		{
			//��ѯʱ�䵽��
			if(xCurrentTickCount > (xUartQueryTickCount + WAIT_UART_QUERY_DEVICE_DURATION))
			{
				uart_write_bytes(UART_NUM_1,(const char *) send_data, 7);
				//����ͨ��ʱ��
				xUartCommunicationTickCount = xCurrentTickCount;
				//���²�ѯʱ��
				xUartQueryTickCount = xCurrentTickCount;
			}
		}
		//�ѻ�ȡ�����ص�ַ�ǣ���ѯ״̬��Ϣ
		else
		{
			//��ѯʱ�䵽��
			if(xCurrentTickCount > (xUartQueryTickCount + WAIT_UART_QUERY_STATUS_DURATION))
			{
				uart_write_bytes(UART_NUM_1,(const char *) send_data, 7);
				//����ͨ��ʱ��
				xUartCommunicationTickCount = xCurrentTickCount;
				//���²�ѯʱ��
				xUartQueryTickCount = xCurrentTickCount;
			}
		}
		
	}

	//�յ�״̬�Ӵ����л�ȡ������,������״̬��ǰһ״̬�Աȣ�������״̬�����ϱ�
	if(m_aircondition_addr_save_f)
	{
		m_query_aircondition_task = 1;//������Ҫ���ٲ�ѯ
		//arch_printf("addr_to_remote\r\n");			
		update_aircondition_connect_for_save(aircondition_driver_list,m_aircondition_connect_count);

		//������±�־λ
		m_aircondition_addr_save_f = 0;
	}
	
	for(i = 0; i < m_aircondition_connect_count; i++)
	{	
		
		if(aircondition_driver_list[i].aircondition_update_f)
		{
			//arch_printf("sta_to_remote\r\n");
			update_aircondition_status_for_notify(&aircondition_driver_list[i],i);

			//������±�־λ
			aircondition_driver_list[i].aircondition_update_f = 0;
		}
	}
	
}

/*
* ���ڽ�������
*/
void uart1_rx_task()
{
	//uint8_t* data = (uint8_t*) malloc(UART1_RX1_BUF_SIZE + 1);       //�����ڴ棬���ڴ��ڽ���
	uint8_t data[UART1_RX1_BUF_SIZE+1] = {0};
	//uint16_t rxi = 0;
	uint32_t rdlen = 0;
	uint32_t offset = 0;

	//while (1)
	//{
		//��ȡ����1���յ�����
		const int rxBytes = uart_read_bytes(UART_NUM_1, data, UART1_RX1_BUF_SIZE, 100 / portTICK_RATE_MS);

		//uart_write_bytes(UART_NUM_1,(const char *) data, rxBytes);
		
		if (rxBytes > 0)
		{
			//����ͨ��ʱ��
			xUartCommunicationTickCount = xTaskGetTickCount();
			rdlen  = rxBytes;
			offset = 0;
			
			while(1)
			{
				offset += uart1_rx_data(rdlen,&data[offset]);
				
				if(offset >= rdlen)break;
			}

			/*
			data[rxBytes] = 0;//�ڴ��ڽ��յ��������ӽ�����

			//�����յ������ݷ���ȥ
			//arch_printf("UART1 RX:\r\n");
			
			for(rxi = 0;rxi < rxBytes; rxi++)
			{
				//arch_printf("%02x ",data[rxi]);
			}
			
			//arch_printf("\r\n");
			*/

		}
	//}
	//free(data);        //�ͷ�������ڴ�
}

void aircondition_driver_task(void)
{
	//����
	uart_tx_task();
	
	//���մ���
	uart1_rx_task();
}
/*
 * light_driver_task
 * 10MS执行一�?
 */
uint8_t rFanCheck = 5;

void light_driver_task(void)
{
	uint32_t value,dev;
	uint32_t temp1,temp2;
	int16_t acfre;
#if(LIGHT_RGB_PWMON)
	uint8_t r,g,b;
#endif
	//aircondition_driver_task();

	if(remote_sys.mLight_ModeTimer)remote_sys.mLight_ModeTimer--;
	if(remote_sys.mLight_fadedelay)remote_sys.mLight_fadedelay--;
	if(remote_sys.mLight_SwCloseTimer)
	{
		if(--remote_sys.mLight_SwCloseTimer == 0)
		{
			gpio_set_level(LIGHT_SW_GPIO, 0);
		}
	}

	// AC���ʹ��
	if(remote_sys.mLight_ACenable)
	{
		// 上电超过30秒后 AC无效   上电30秒内 �?0ms会进来检测一�?
		//AC����ʱʱ�䵽��
		if(++rTimer_Second == LIGHT_AC_CTIMER)//0.2S = 50*0.2 = 10
		{
			rTimer_Second = 0;//��ʱ��0��Ϊ��һ�εļ�ʱ��׼��
			light_driver_pcnt_get(&acfre);//��ȡPCNT
			light_driver_pcnt_resume();//����PCNT��Ϊ��һ����׼��
			if(rPower_ACCheck)
			{
				rPower_ACCheck--;
				if(acfre <= LIGHT_AC_FRE)//��ʱ�����ڲ⵽��Ƶ�ʵ������Ƶ��
				{
					remote_sys.mLight_ACenable = 0;
					//arch_printf("Disable AC\r\n");
				}
			}
			if(acfre < LIGHT_AC_FRE)//50HZ  掉电
			{
				if(rFanCheck == 5)//上电开始没脉冲  			//第一次上�?
				{

				}else{
					if(rFanCheck == 1)//分段掉电处理		//分段
					{
						rFanCheck = 0;
						//arch_printf("PowerDown=%d\r\n",acfre);
						if(rLamp_brightness)
						{
							light_driver_jump(0,
											  lewyfan_infor.light_templatetrue,
											  lewyfan_infor.light_rcolor,
											  lewyfan_infor.light_gcolor,
											  lewyfan_infor.light_bcolor);
						}
					}
				}

			}
			else
			{	// 上电
				if(rFanCheck == 0)//分段上电处理
				{
#if(LIGHT_NIGHT_ENABLE)
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
					//arch_printf("PowerUp=%d\r\n",acfre);
#if(LIGHT_SMARTSWITCH_ENABLE)
					if(lewyfan_infor.smart_switch)//灵动开关打开
					{
						if(lewyfan_infor.light_sw)lewyfan_infor.light_sw = 0;
						else					  lewyfan_infor.light_sw = 1;
					}else
#endif
					{
						if(lewyfan_infor.power_state == POWER_STATE_SAVE)//掉电保户
						{
							RePowerOnInit();
							arch_psm_set_value(LIGHT_SAVE_NAME, LIGHT_KEY_PONSETBIT, (void*)&remote_sys.Other, sizeof(remote_sys.Other));
						}else{
							lewyfan_infor.light_sw = 1;
							if(lewyfan_infor.user_mode)
							{
								arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_USER, (void*)&lewyfan_userinfor, sizeof(lewyfan_userinfor));
								lewyfan_infor.light_brightness 	 = lewyfan_userinfor.light_brightness;
								lewyfan_infor.light_templatetrue = lewyfan_userinfor.light_templatetrue;
								lewyfan_infor.light_rcolor 		 = lewyfan_userinfor.light_rcolor;
								lewyfan_infor.light_gcolor 		 = lewyfan_userinfor.light_gcolor;
								lewyfan_infor.light_bcolor 		 = lewyfan_userinfor.light_bcolor;
								lewyfan_infor.light_rgbmode 	 = lewyfan_userinfor.light_rgbmode;
							}else{
								RePowerOnInit();
								arch_psm_set_value(LIGHT_SAVE_NAME, LIGHT_KEY_PONSETBIT, (void*)&remote_sys.Other, sizeof(remote_sys.Other));
							}
						}
					}
					remote_sys.mLight_powercnt++;
					if(remote_sys.mLight_powercnt == LIGHT_RESET_CNT)//快速开�?�?WIFI进入配网
					{
						if(remote_sys.mLight_powercnt > LIGHT_RESET_CNT_LOCK || (lewyfan_infor.lock_state & APP_LOCK_RSTSET) == 0)
						{
							mReset = WIFI_RESET_POWER;
							remote_sys.mLight_powercnt = 0;
							remote_sys.mLight_fadecnt  = SHOW_CNT3;
							remote_sys.mLight_showtimr = 20;
							light_ResetData(2);
						}
					}

					if(WIFI_RESET_POWER != mReset)
					{
						light_driver_jump(lewyfan_infor.light_brightness,
										  lewyfan_infor.light_templatetrue,
										  lewyfan_infor.light_rcolor,
										  lewyfan_infor.light_gcolor,
										  lewyfan_infor.light_bcolor);
					}

					arch_psm_set_value(LIGHT_SAVE_NAME, LIGHT_KEY_POWER, (void*)&remote_sys.mLight_powercnt, sizeof(remote_sys.mLight_powercnt));
					remote_sys.mLight_mode      = M_LIGHT_PON;
					remote_sys.mLight_TestTimer = LIGHT_TEST_TIMER;
					remote_sys.mLight_powertim  = LIGHT_RESET_TIMER;
					remote_sys.mLight_matchtimer= LIGHT_MATCH_TIMER;
					remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
					remote_sys.mRemote_SaveUp  = 1;
				}
				rFanCheck = 1;
			}
		}
	}
	//灯控部分--100MS
	if(remote_sys.mLight_fadedelay)return;
	//缓存开�?
	if(sLamp_brightness     != rLamp_brightness
#if(LIGHT_WY_PWMON)
	   ||sLamp_templatetrue != rLamp_templatetrue
#endif
#if(LIGHT_RGB_PWMON)
	   ||sLamp_RValue != rLamp_RValue
	   ||sLamp_GValue != rLamp_GValue
	   ||sLamp_BValue != rLamp_BValue
#endif
	   )
	{
		remote_sys.mLight_fadedelay = dLamp_Timer;
		//亮度+
		if(rLamp_brightness < sLamp_brightness)
		{
			rLamp_brightness += dLamp_brightness;
			if(rLamp_brightness > sLamp_brightness)rLamp_brightness = sLamp_brightness;
		}
		else
		//亮度-
		if(rLamp_brightness > sLamp_brightness)
		{
			if(rLamp_brightness >= (sLamp_brightness + dLamp_brightness))
				rLamp_brightness -= dLamp_brightness;
			else
				rLamp_brightness = sLamp_brightness;
		}
#if(LIGHT_WY_PWMON)
		//WY+
		if(rLamp_templatetrue < sLamp_templatetrue)
		{
			rLamp_templatetrue += dLamp_templatetrue;
			if(rLamp_templatetrue > sLamp_templatetrue)rLamp_templatetrue = sLamp_templatetrue;
		}
		else
		//WY-
		if(rLamp_templatetrue > sLamp_templatetrue)
		{
			if(rLamp_templatetrue >= (sLamp_templatetrue + dLamp_templatetrue))
				rLamp_templatetrue -= dLamp_templatetrue;
			else
				rLamp_templatetrue = sLamp_templatetrue;
		}
#endif
#if(LIGHT_RGB_PWMON)
		//r+
		if(rLamp_RValue < sLamp_RValue)
		{
			rLamp_RValue += dLamp_RValue;
			if(rLamp_RValue > sLamp_RValue)rLamp_RValue = sLamp_RValue;
		}
		else
		//r-
		if(rLamp_RValue > sLamp_RValue)
		{
			if(rLamp_RValue >= (sLamp_RValue + dLamp_RValue))
				rLamp_RValue -= dLamp_RValue;
			else
				rLamp_RValue = sLamp_RValue;
		}

		//g+
		if(rLamp_GValue < sLamp_GValue)
		{
			rLamp_GValue += dLamp_GValue;
			if(rLamp_GValue > sLamp_GValue)rLamp_GValue = sLamp_GValue;
		}
		else
		//g-
		if(rLamp_GValue > sLamp_GValue)
		{
			if(rLamp_GValue >= (sLamp_GValue + dLamp_GValue))
				rLamp_GValue -= dLamp_GValue;
			else
				rLamp_GValue = sLamp_GValue;
		}

		//b+
		if(rLamp_BValue < sLamp_BValue)
		{
			rLamp_BValue += dLamp_BValue;
			if(rLamp_BValue > sLamp_BValue)rLamp_BValue = sLamp_BValue;
		}
		else
		//b-
		if(rLamp_BValue > sLamp_BValue)
		{
			if(rLamp_BValue >= (sLamp_BValue + dLamp_BValue))
				rLamp_BValue -= dLamp_BValue;
			else
				rLamp_BValue = sLamp_BValue;
		}
#endif

		light_driver_set(rLamp_brightness,rLamp_templatetrue,rLamp_RValue,rLamp_GValue,rLamp_BValue);
		return;
	}

	// 色温/亮度调节函数处理
	if(remote_sys.mRemote_CmdMode)
	{
		switch(remote_sys.mRemote_CmdMode)
		{
		case LCMD_ONEBRIGHTNESS: // //华艺一键调�?0x01：开�?0x02：停�?
			if(remote_sys.mRemote_CmdDelay)return;
			value = lewyfan_infor.light_brightness;
			dev   = lewyfan_infor.light_brightness/LIGHT_SWITCH_BR_DEV;
			if(dev == 0)dev = 1;

			if(remote_sys.mRemote_CmdUp)
			{
				value += dev;
				if (value >= LIGHT_BRIGHTNESS_MAX)
				{
					value = LIGHT_BRIGHTNESS_MAX;
					remote_sys.mRemote_CmdDelay= 1000/LIGNT_TIMER_TICK;
					remote_sys.mRemote_CmdUp   = 0;
				}
				lewyfan_infor.light_brightness = value;
				light_driver_jump(lewyfan_infor.light_brightness,
								  lewyfan_infor.light_templatetrue,
								  lewyfan_infor.light_rcolor,
								  lewyfan_infor.light_gcolor,
								  lewyfan_infor.light_bcolor);
			}else
			{
				if(value <= dev)
				{
					value = LIGHT_BRIGHTNESS_MIN;
					remote_sys.mRemote_CmdDelay= 1000/LIGNT_TIMER_TICK;
					remote_sys.mRemote_CmdUp   = 1;
				}
				else
				{
					value -= dev;
				}
				lewyfan_infor.light_brightness = value;
				light_driver_jump(lewyfan_infor.light_brightness,
								  lewyfan_infor.light_templatetrue,
								  lewyfan_infor.light_rcolor,
								  lewyfan_infor.light_gcolor,
								  lewyfan_infor.light_bcolor);
			}
			break;
		case LCMD_CON_ADD://BR+
			value = lewyfan_infor.light_brightness;
			dev   = lewyfan_infor.light_brightness/LIGHT_SWITCH_BR_DEV;
			if(dev < LIGHT_SWITCH_BR_DMI)dev = LIGHT_SWITCH_BR_DMI;
			if(value < LIGHT_BRIGHTNESS_MAX)
			{
				value += dev;
				if(value > LIGHT_BRIGHTNESS_MAX)
				{
					value = LIGHT_BRIGHTNESS_MAX;
					remote_sys.mRemote_CmdMode = 0;
				}
				lewyfan_infor.light_brightness = value;
				light_driver_jump(lewyfan_infor.light_brightness,
								  lewyfan_infor.light_templatetrue,
								  lewyfan_infor.light_rcolor,
								  lewyfan_infor.light_gcolor,
								  lewyfan_infor.light_bcolor);
			}
			else{
				remote_sys.mRemote_CmdMode = 0;
			}

			break;
		case LCMD_CON_DEC://BR-
			value = lewyfan_infor.light_brightness;
			dev   = lewyfan_infor.light_brightness/LIGHT_SWITCH_BR_DEV;
			if(dev < LIGHT_SWITCH_BR_DMI)dev = LIGHT_SWITCH_BR_DMI;
			if(value > LIGHT_BRIGHTNESS_MIN)
			{
				if(value > (LIGHT_BRIGHTNESS_MIN+dev))
				{
					value -= dev;
				}else{
					value = LIGHT_BRIGHTNESS_MIN;
					remote_sys.mRemote_CmdMode = 0;
				}
				lewyfan_infor.light_brightness = value;
				light_driver_jump(lewyfan_infor.light_brightness,
								  lewyfan_infor.light_templatetrue,
								  lewyfan_infor.light_rcolor,
								  lewyfan_infor.light_gcolor,
								  lewyfan_infor.light_bcolor);
			}
			else{
				remote_sys.mRemote_CmdMode = 0;
			}
			break;

		case LCMD_COLORK: //华艺一键调�?0x01：开�?0x02：停�?
			if(remote_sys.mRemote_CmdDelay)return;
			value = lewyfan_infor.light_templatetrue;
			dev = LIGHT_SWITCH_TM_DEV;

			if(remote_sys.mRemote_CmdFade)
			{
				value += dev;
				if(value > LIGHT_TEMPLATETRUE_MAX)
				{
					value = LIGHT_TEMPLATETRUE_MAX;
					remote_sys.mRemote_CmdDelay= 1000/LIGNT_TIMER_TICK;
					remote_sys.mRemote_CmdFade   = 0;
				}
				lewyfan_infor.light_templatetrue = value;
				light_driver_jump(lewyfan_infor.light_brightness,
								  lewyfan_infor.light_templatetrue,
								  lewyfan_infor.light_rcolor,
								  lewyfan_infor.light_gcolor,
								  lewyfan_infor.light_bcolor);
			}
			else
			{
				if(value > (LIGHT_TEMPLATETRUE_MIN+dev))
				{
					value -= dev;
				}else{
					value = LIGHT_TEMPLATETRUE_MIN;
					remote_sys.mRemote_CmdDelay= 1000/LIGNT_TIMER_TICK;
					remote_sys.mRemote_CmdFade   = 1;
				}
				lewyfan_infor.light_templatetrue = value;
				light_driver_jump(lewyfan_infor.light_brightness,
								  lewyfan_infor.light_templatetrue,
								  lewyfan_infor.light_rcolor,
								  lewyfan_infor.light_gcolor,
								  lewyfan_infor.light_bcolor);
			}
			break;
		case LCMD_COL_ADD://TM+
			value = lewyfan_infor.light_templatetrue;
			dev = LIGHT_SWITCH_TM_DEV;
			if(value < LIGHT_TEMPLATETRUE_MAX)
			{
				value += dev;
				if(value > LIGHT_TEMPLATETRUE_MAX)
				{
					value = LIGHT_TEMPLATETRUE_MAX;
					remote_sys.mRemote_CmdMode = 0;
				}
				lewyfan_infor.light_templatetrue = value;
				light_driver_jump(lewyfan_infor.light_brightness,
								  lewyfan_infor.light_templatetrue,
								  lewyfan_infor.light_rcolor,
								  lewyfan_infor.light_gcolor,
								  lewyfan_infor.light_bcolor);
			}
			else{
				remote_sys.mRemote_CmdMode = 0;
			}
			break;
		case LCMD_COL_DEC://TM-
			value = lewyfan_infor.light_templatetrue;
			dev = LIGHT_SWITCH_TM_DEV;
			if(value > LIGHT_TEMPLATETRUE_MIN)
			{
				if(value > (LIGHT_TEMPLATETRUE_MIN+dev))
				{
					value -= dev;
				}else{
					value = LIGHT_TEMPLATETRUE_MIN;
					remote_sys.mRemote_CmdMode = 0;
				}
				lewyfan_infor.light_templatetrue = value;
				light_driver_jump(lewyfan_infor.light_brightness,
								  lewyfan_infor.light_templatetrue,
								  lewyfan_infor.light_rcolor,
								  lewyfan_infor.light_gcolor,
								  lewyfan_infor.light_bcolor);
			}
			else{
				remote_sys.mRemote_CmdMode = 0;
			}
			break;
		default:
			remote_sys.mRemote_CmdMode = 0;
			break;
		}
		remote_sys.mLight_fadedelay = 0;
		return;
	}

#if(LIGHT_MODE_SWITCH == 1)
	switch(lewyfan_infor.light_rgbmode)
	{
	#if(LIGHT_RGB_PWMON)
		//RGB模式调节--流光模式 //RGB模式调节--呼吸模式
		case LIGHT_RGB_FADE:
		case LIGHT_RGB_JUMP:
		case LIGHT_RGB_COLOR:
			if(lewyfan_infor.light_sw)
			{
				if(remote_sys.mRGB_cnt >= LIGHT_RGB_FADECNT)remote_sys.mRGB_cnt=0;
				r = mRGM_FADE_COLOR[remote_sys.mRGB_cnt][0];
				g = mRGM_FADE_COLOR[remote_sys.mRGB_cnt][1];
				b = mRGM_FADE_COLOR[remote_sys.mRGB_cnt][2];

				lewyfan_infor.light_rcolor = r;
				lewyfan_infor.light_gcolor = g;
				lewyfan_infor.light_bcolor = b;

				if(lewyfan_infor.light_rgbmode == LIGHT_RGB_JUMP)
				{
					if(lewyfan_infor.fade_scen == 0)
					{
						remote_sys.mLight_fadedelay = (LIGHT_FADE_TIME);
					}
					else{
						remote_sys.mLight_fadedelay = (LIGHY_FADE_UNIT*lewyfan_infor.fade_scen)/LIGNT_TIMER_TICK;;
					}
					light_driver_jump(lewyfan_infor.light_brightness,
									  lewyfan_infor.light_templatetrue,
									  lewyfan_infor.light_rcolor,
									  lewyfan_infor.light_gcolor,
									  lewyfan_infor.light_bcolor);
				}else{
					if(lewyfan_infor.fade_scen == 0)
					{
						remote_sys.mLight_fadetimer = LIGHT_FADE_TIME;
					}
					light_driver_fade(lewyfan_infor.light_brightness,
									  lewyfan_infor.light_templatetrue,
									  lewyfan_infor.light_rcolor,
									  lewyfan_infor.light_gcolor,
									  lewyfan_infor.light_bcolor);
				}
				remote_sys.mRGB_cnt++;
			}
			break;
	#endif
	#if(LIGHT_WY_PWMON)
		case LIGHT_CD_WEAKUP://清晨唤醒 30分钟  色温�?0%-90%�? 亮度�?-95%�?
			if(remote_sys.mLight_ModeTimer % (1000/LIGNT_TIMER_TICK) == 0)//1S渐变一�?30*60 = 1800S
			{
				//色温
				temp1 = LIGHT_TEMPLATETRUE_MIN;
				temp2 = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*9/10;
				value = temp1 + ((temp2-temp1)*((1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK)-remote_sys.mLight_ModeTimer)/(1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK));
				lewyfan_infor.light_templatetrue = value;

				//亮度
				temp1 = LIGHT_BRIGHTNESS_MIN;
				temp2 = LIGHT_BRIGHTNESS_MAX*lewyfan_infor.weakebr/100;
				value = temp1 + ((temp2-temp1)*((1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK)-remote_sys.mLight_ModeTimer)/(1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK));
				lewyfan_infor.light_brightness = value;

				light_driver_jump(lewyfan_infor.light_brightness,
								  lewyfan_infor.light_templatetrue,
								  lewyfan_infor.light_rcolor,
								  lewyfan_infor.light_gcolor,
								  lewyfan_infor.light_bcolor);
				if(remote_sys.mLight_ModeTimer == 0 || value >= temp2)
				{
					lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
					remote_sys.mRemote_stimer   = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
					remote_sys.mRemote_SaveUp   = 1;
				}
			}
			break;
		case LIGHT_CD_DUSK://黄昏明亮 30分钟  色温�?50%-100%�? 亮度�?-100%�?
			if(remote_sys.mLight_ModeTimer % (1000/LIGNT_TIMER_TICK) == 0)//1S渐变一�?30*60 = 1800S
			{
				//色温
				temp1 = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*5/10;
				temp2 = LIGHT_TEMPLATETRUE_MAX;
				value = temp1 + ((temp2-temp1)*((1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK)-remote_sys.mLight_ModeTimer)/(1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK));
				lewyfan_infor.light_templatetrue = value;

				//亮度
				temp1 = LIGHT_BRIGHTNESS_MIN;
				temp2 = LIGHT_BRIGHTNESS_MAX*9/10;
				value = temp1 + ((temp2-temp1)*((1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK)-remote_sys.mLight_ModeTimer)/(1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK));
				lewyfan_infor.light_brightness = value;

				light_driver_jump(lewyfan_infor.light_brightness,
								  lewyfan_infor.light_templatetrue,
								  lewyfan_infor.light_rcolor,
								  lewyfan_infor.light_gcolor,
								  lewyfan_infor.light_bcolor);
				if(remote_sys.mLight_ModeTimer == 0)
				{
					lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
					remote_sys.mRemote_stimer   = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
					remote_sys.mRemote_SaveUp   = 1;
				}
			}
			break;
		case LIGHT_CD_SLEEP://夜晚助眠30分钟  色温�?95%->0%�? 亮度�?5-0%�?
			if(remote_sys.mLight_ModeTimer % (1000/LIGNT_TIMER_TICK) == 0)//1S渐变一�?30*60 = 1800S
			{
				//色温
				temp1 = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*95/100;
				temp2 = LIGHT_TEMPLATETRUE_MIN;
				value = temp1 - ((temp1-temp2)*((1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK) - remote_sys.mLight_ModeTimer)/(1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK));
				lewyfan_infor.light_templatetrue = value;

				//亮度
				temp1 = LIGHT_BRIGHTNESS_MAX*lewyfan_infor.sleepsbr/100;
				temp2 = LIGHT_BRIGHTNESS_MIN;
				value = temp1 - ((temp1-temp2)*((1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK)-remote_sys.mLight_ModeTimer)/(1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK));
				lewyfan_infor.light_brightness = value;

				if(remote_sys.mLight_ModeTimer == 0)//结束
				{
					lewyfan_infor.light_sw      = 0;
					lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
					remote_sys.mRemote_stimer   = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
					remote_sys.mRemote_SaveUp   = 1;
					light_driver_jump(0,
									  lewyfan_infor.light_templatetrue,
									  lewyfan_infor.light_rcolor,
									  lewyfan_infor.light_gcolor,
									  lewyfan_infor.light_bcolor);
				}
				else
				{
					light_driver_jump(lewyfan_infor.light_brightness,
									  lewyfan_infor.light_templatetrue,
									  lewyfan_infor.light_rcolor,
									  lewyfan_infor.light_gcolor,
									  lewyfan_infor.light_bcolor);
				}
			}
			break;
	#endif
		default:
			break;
	}
#endif
}


/*
 * light_driver_fade
 * 开�? sw=true 关灯:sw=false
 * 亮度：br(1-100)
 * 色温：tm(3000-6400)
 */
void light_driver_fade(uint32_t brightness,uint32_t templatetrue,uint8_t r,uint8_t g,uint8_t b)
{
	uint32_t dmax = 0;
	LOG_INFO_TAG("light_fade","br=%d,tp=%d,r=%d,g=%d,b=%d..",brightness, templatetrue,r, g, b);

	//ɫ�²��Ϸ�������£�ֱ��ȡ��ֵ
	if(templatetrue < LIGHT_TEMPLATETRUE_MIN)templatetrue = LIGHT_TEMPLATETRUE_MIN;
	if(templatetrue > LIGHT_TEMPLATETRUE_MAX)templatetrue = LIGHT_TEMPLATETRUE_MAX;

	//���Ȳ��Ϸ���ֱ��ȡ��ֵ
	if(brightness > LIGHT_BRIGHTNESS_MAX)brightness = LIGHT_BRIGHTNESS_MAX;

	//remote_sys.mLight_fadetiemr = LIGHT_FADE_TIME;
	sLamp_brightness   = brightness;
	sLamp_templatetrue = templatetrue;

	sLamp_RValue = r;
	sLamp_GValue = g;
	sLamp_BValue = b;

	if(sLamp_brightness != 0 && rLamp_brightness == 0)//开�?
	{
		if(lewyfan_infor.fade_on == 0)
		{
			light_driver_jump(brightness,templatetrue,r,g,b);
		}
		else
		{
			dLamp_Timer = (LIGHY_FADE_UNIT*lewyfan_infor.fade_on)/LIGNT_TIMER_TICK;//渐变时间-->>节拍数据
			//亮度
			if(sLamp_brightness > rLamp_brightness)
				dLamp_brightness = (sLamp_brightness - rLamp_brightness);
			else
				dLamp_brightness = (rLamp_brightness - sLamp_brightness);
			if(dLamp_brightness > dmax)dmax = dLamp_brightness;

#if(LIGHT_WY_PWMON)
			//WY----------
			rLamp_templatetrue = sLamp_templatetrue;
#endif

#if(LIGHT_RGB_PWMON)
			//R----------
			rLamp_RValue = sLamp_RValue;

			//g-----------
			rLamp_GValue = sLamp_GValue;

			//b-----------
			rLamp_BValue = sLamp_BValue;
#endif

			if(dmax >= dLamp_Timer)//增量大于总节拍数 例如dmax = 1000 渐变时间的总节拍dLamp_Timer = 100
			{
				dLamp_brightness   = dLamp_brightness  /dLamp_Timer;
#if(LIGHT_WY_PWMON)
				dLamp_templatetrue = dLamp_templatetrue/dLamp_Timer;
#endif
#if(LIGHT_RGB_PWMON)
				dLamp_RValue 	   = dLamp_RValue	   /dLamp_Timer;
				dLamp_GValue	   = dLamp_GValue	   /dLamp_Timer;
				dLamp_BValue 	   = dLamp_BValue	   /dLamp_Timer;
#endif
				dLamp_Timer 	   = 1;
#if(LIGHT_WY_PWMON)
				if(dLamp_brightness   == 0)dLamp_brightness = 1;
#endif
				if(dLamp_templatetrue == 0)dLamp_templatetrue = 1;
#if(LIGHT_RGB_PWMON)
				if(dLamp_RValue == 0)dLamp_RValue = 1;
				if(dLamp_GValue == 0)dLamp_GValue = 1;
				if(dLamp_BValue == 0)dLamp_BValue = 1;
#endif
			}
			else				  //增量小于总节拍数 例如dmax = 10  渐变时间的总节拍dLamp_Timer = 100
			{
				dLamp_brightness   = 1;
#if(LIGHT_WY_PWMON)
				dLamp_templatetrue = 1;
#endif
#if(LIGHT_RGB_PWMON)
				dLamp_RValue 	   = 1;
				dLamp_GValue	   = 1;
				dLamp_BValue 	   = 1;
#endif
				dLamp_Timer 	   = dLamp_Timer/dmax;
			}
			rLamp_brightness++;
			light_driver_set(rLamp_brightness,rLamp_templatetrue,rLamp_RValue,rLamp_GValue,rLamp_BValue);
		}
	}
	else
	if(sLamp_brightness == 0 && rLamp_brightness != 0)//关灯
	{
		if(lewyfan_infor.fade_off == 0)
		{
			light_driver_jump(brightness,templatetrue,r,g,b);
		}
		else
		{
			dLamp_Timer = (LIGHY_FADE_UNIT*lewyfan_infor.fade_off)/LIGNT_TIMER_TICK;//渐变时间-->>节拍数据
			//亮度
			if(sLamp_brightness > rLamp_brightness)
				dLamp_brightness = (sLamp_brightness - rLamp_brightness);
			else
				dLamp_brightness = (rLamp_brightness - sLamp_brightness);
			if(dLamp_brightness > dmax)dmax = dLamp_brightness;

#if(LIGHT_WY_PWMON)
			//WY----------
			if(sLamp_templatetrue > rLamp_templatetrue)
				dLamp_templatetrue = (sLamp_templatetrue - rLamp_templatetrue);
			else
				dLamp_templatetrue = (rLamp_templatetrue - sLamp_templatetrue);
			if(dLamp_templatetrue > dmax)dmax = dLamp_templatetrue;
#endif

#if(LIGHT_RGB_PWMON)
			//R----------
			if(sLamp_RValue > rLamp_RValue)
				dLamp_RValue = (sLamp_RValue - rLamp_RValue);
			else
				dLamp_RValue = (rLamp_RValue - sLamp_RValue);
			if(dLamp_RValue > dmax)dmax = dLamp_RValue;

			//g-----------
			if(sLamp_GValue > rLamp_GValue)
				dLamp_GValue = (sLamp_GValue - rLamp_GValue);
			else
				dLamp_GValue = (rLamp_GValue - sLamp_GValue);
			if(dLamp_GValue > dmax)dmax = dLamp_GValue;

			//b-------------
			if(sLamp_BValue > rLamp_BValue)
				dLamp_BValue = (sLamp_BValue - rLamp_BValue);
			else
				dLamp_BValue = (rLamp_BValue - sLamp_BValue);
			if(dLamp_BValue > dmax)dmax = dLamp_BValue;
#endif

			if(dmax >= dLamp_Timer)//增量大于总节拍数 例如dmax = 1000 渐变时间的总节拍dLamp_Timer = 100
			{
				dLamp_brightness   = dLamp_brightness  /dLamp_Timer;
#if(LIGHT_WY_PWMON)
				dLamp_templatetrue = dLamp_templatetrue/dLamp_Timer;
#endif
#if(LIGHT_RGB_PWMON)
				dLamp_RValue 	   = dLamp_RValue	   /dLamp_Timer;
				dLamp_GValue	   = dLamp_GValue	   /dLamp_Timer;
				dLamp_BValue 	   = dLamp_BValue	   /dLamp_Timer;
#endif
				dLamp_Timer 	   = 1;
				if(dLamp_brightness   == 0)dLamp_brightness = 1;
#if(LIGHT_WY_PWMON)
				if(dLamp_templatetrue == 0)dLamp_templatetrue = 1;
#endif
#if(LIGHT_RGB_PWMON)
				if(dLamp_RValue == 0)dLamp_RValue = 1;
				if(dLamp_GValue == 0)dLamp_GValue = 1;
				if(dLamp_BValue == 0)dLamp_BValue = 1;
#endif
			}
			else				  //增量小于总节拍数 例如dmax = 10  渐变时间的总节拍dLamp_Timer = 100
			{
				dLamp_brightness   = 1;
#if(LIGHT_WY_PWMON)
				dLamp_templatetrue = 1;
#endif
#if(LIGHT_RGB_PWMON)
				dLamp_RValue 	   = 1;
				dLamp_GValue	   = 1;
				dLamp_BValue 	   = 1;
#endif
				dLamp_Timer 	   = dLamp_Timer/dmax;
			}
		}
	}
	else//调节
	{
		if(remote_sys.mLight_fadetimer)
		{
			dLamp_Timer = remote_sys.mLight_fadetimer;
			remote_sys.mLight_fadetimer = 0;
		}
		else{
			dLamp_Timer = LIGHY_FADE_UNIT*lewyfan_infor.fade_scen/LIGNT_TIMER_TICK;
		}
		if(dLamp_Timer == 0)
		{
			light_driver_jump(brightness,templatetrue,r,g,b);
		}
		else
		{
			//亮度
			if(sLamp_brightness > rLamp_brightness)
				dLamp_brightness = (sLamp_brightness - rLamp_brightness);
			else
				dLamp_brightness = (rLamp_brightness - sLamp_brightness);
			if(dLamp_brightness > dmax)dmax = dLamp_brightness;
#if(LIGHT_WY_PWMON)
			//WY----------
			if(sLamp_templatetrue > rLamp_templatetrue)
				dLamp_templatetrue = (sLamp_templatetrue - rLamp_templatetrue);
			else
				dLamp_templatetrue = (rLamp_templatetrue - sLamp_templatetrue);
			if(dLamp_templatetrue > dmax)dmax = dLamp_templatetrue;
#endif

#if(LIGHT_RGB_PWMON)
			//R----------
			if(sLamp_RValue > rLamp_RValue)
				dLamp_RValue = (sLamp_RValue - rLamp_RValue);
			else
				dLamp_RValue = (rLamp_RValue - sLamp_RValue);
			if(dLamp_RValue > dmax)dmax = dLamp_RValue;

			//g-----------
			if(sLamp_GValue > rLamp_GValue)
				dLamp_GValue = (sLamp_GValue - rLamp_GValue);
			else
				dLamp_GValue = (rLamp_GValue - sLamp_GValue);
			if(dLamp_GValue > dmax)dmax = dLamp_GValue;

			//b-------------
			if(sLamp_BValue > rLamp_BValue)
				dLamp_BValue = (sLamp_BValue - rLamp_BValue);
			else
				dLamp_BValue = (rLamp_BValue - sLamp_BValue);
			if(dLamp_BValue > dmax)dmax = dLamp_BValue;
#endif

			if(dmax >= dLamp_Timer)//增量大于总节拍数 例如dmax = 1000 渐变时间的总节拍dLamp_Timer = 100
			{
				dLamp_brightness   = dLamp_brightness  /dLamp_Timer;

#if(LIGHT_WY_PWMON)
				dLamp_templatetrue = dLamp_templatetrue/dLamp_Timer;
#endif

#if(LIGHT_RGB_PWMON)
				dLamp_RValue 	   = dLamp_RValue	   /dLamp_Timer;
				dLamp_GValue	   = dLamp_GValue	   /dLamp_Timer;
				dLamp_BValue 	   = dLamp_BValue	   /dLamp_Timer;
#endif
				dLamp_Timer 	   = 1;
				if(dLamp_brightness   == 0)dLamp_brightness = 1;

#if(LIGHT_WY_PWMON)
				if(dLamp_templatetrue == 0)dLamp_templatetrue = 1;
#endif
#if(LIGHT_RGB_PWMON)
				if(dLamp_RValue == 0)dLamp_RValue = 1;
				if(dLamp_GValue == 0)dLamp_GValue = 1;
				if(dLamp_BValue == 0)dLamp_BValue = 1;
#endif
			}
			else				  //增量小于总节拍数 例如dmax = 10  渐变时间的总节拍dLamp_Timer = 100
			{
				dLamp_brightness   = 1;

#if(LIGHT_WY_PWMON)
				dLamp_templatetrue = 1;
#endif
#if(LIGHT_RGB_PWMON)
				dLamp_RValue 	   = 1;
				dLamp_GValue	   = 1;
				dLamp_BValue 	   = 1;
#endif
				dLamp_Timer 	   = dLamp_Timer/dmax;
			}
		}
	}
	LOG_INFO_TAG("light_fade","DT=%d,DBR=%d,DTM=%d,DR=%d,DG=%d,DB=%d",dLamp_Timer,dLamp_brightness,dLamp_templatetrue,dLamp_RValue,dLamp_GValue,dLamp_BValue);
}
void light_driver_jump(uint32_t brightness,uint32_t templatetrue,uint8_t r,uint8_t g,uint8_t b)
{
	LOG_INFO_TAG("light_jump","br=%d,tp=%d,r=%d,g=%d,b=%d...", brightness, templatetrue,r, g, b);
	//ɫ�²��Ϸ���ֱ��ȡ��ֵ
	if(templatetrue < LIGHT_TEMPLATETRUE_MIN)templatetrue = LIGHT_TEMPLATETRUE_MIN;
	if(templatetrue > LIGHT_TEMPLATETRUE_MAX)templatetrue = LIGHT_TEMPLATETRUE_MAX;

	//亮度
	//���Ȳ��Ϸ�ֱ��ȡ��ֵ
	if(brightness > LIGHT_BRIGHTNESS_MAX)brightness = LIGHT_BRIGHTNESS_MAX;

	sLamp_brightness   = brightness;
	sLamp_templatetrue = templatetrue;

	sLamp_RValue = r;
	sLamp_GValue = g;
	sLamp_BValue = b;

	rLamp_brightness   = brightness;
	rLamp_templatetrue = templatetrue;

	rLamp_RValue = r;
	rLamp_GValue = g;
	rLamp_BValue = b;
	//��װ���������Ƶ�״̬
    light_driver_set(rLamp_brightness,rLamp_templatetrue,rLamp_RValue,rLamp_GValue,rLamp_BValue);
}

/*
 * light_driver_set
 * 开�? sw=true 关灯:sw=false
 * 亮度：br(1-100)
 * 色温：tm(3000-6400)
 */
void light_driver_set(uint32_t brightness,uint32_t templatetrue,uint8_t r,uint8_t g, uint8_t b)
{
	//uint32_t minv;
#if(LIGHT_WY_PWMON)
	uint32_t ycolor=0,wcolor=0;
	uint32_t yvalue=0,wvalue=0;
#endif

#if(LIGHT_RGB_PWMON)
	uint32_t rcolor=0,gcolor=0,bcolor=0;
#endif

	if(mReset == WIFI_RESET_WAIT)return;

	//LOG_INFO_TAG("light_driver_set","br=%d,tp=%d,r=%d,g=%d,b=%d", brightness, templatetrue,r,g,b);
	//���Ȳ�Ϊ0
	if(brightness)
	{
	#if(LIGHT_WY_PWMON)
		if(templatetrue < LIGHT_TEMPLATETRUE_MIN)templatetrue = LIGHT_TEMPLATETRUE_MIN;
		if(templatetrue > LIGHT_TEMPLATETRUE_MAX)templatetrue = LIGHT_TEMPLATETRUE_MAX;

	#if(LIGHT_WY_WBR_YCT)//W调光Y调色
		// 调光
		wvalue = ((LIGHT_PWM_WMAX - remote_sys.mLight_PWMmin)*brightness)/LIGHT_BRIGHTNESS_MAX;
		wvalue += remote_sys.mLight_PWMmin;
		//调色
		yvalue = (LIGHT_PWM_WMAX) * (templatetrue - LIGHT_TEMPLATETRUE_MIN) / (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN);
	#else

		if(remote_sys.mLight_ConstantPower == 0)
		{	// 叠加功率
			if(templatetrue < LIGHT_TEMPLATETRUE_MID)
			{
				ycolor = (LIGHT_PWM_WMAX-remote_sys.mLight_PWMmin);
				wcolor = (LIGHT_PWM_WMAX-remote_sys.mLight_PWMmin) * (templatetrue - LIGHT_TEMPLATETRUE_MIN) / (LIGHT_TEMPLATETRUE_MID-LIGHT_TEMPLATETRUE_MIN);
			}else{
				ycolor = (LIGHT_PWM_YMAX-remote_sys.mLight_PWMmin) * (LIGHT_TEMPLATETRUE_MAX - templatetrue) / (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MID);
				wcolor = (LIGHT_PWM_YMAX-remote_sys.mLight_PWMmin);
			}
		}
		else{
			ycolor =(LIGHT_PWM_YMAX - remote_sys.mLight_PWMmin)*(LIGHT_TEMPLATETRUE_MAX - templatetrue) / (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN);
			wcolor =(LIGHT_PWM_WMAX - remote_sys.mLight_PWMmin)*(templatetrue - LIGHT_TEMPLATETRUE_MIN) / (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN);

		}

		yvalue = ycolor*brightness/LIGHT_BRIGHTNESS_MAX;
		wvalue = wcolor*brightness/LIGHT_BRIGHTNESS_MAX;

		if(templatetrue != LIGHT_TEMPLATETRUE_MIN)//限制单边最小PWM
		{
			wvalue += remote_sys.mLight_PWMmin;
		}
		if(templatetrue != LIGHT_TEMPLATETRUE_MAX)//限制单边最小PWM
		{
			yvalue += remote_sys.mLight_PWMmin;
		}
	#endif//
		if(LIGHT_PWM_MAX ==wvalue)wvalue=wvalue+1;
		if(LIGHT_PWM_MAX ==yvalue)yvalue=yvalue+1;
	#endif//LIGHT_WY_ON

	#if(LIGHT_RGB_PWMON)
		rcolor = (LIGHT_PWM_RMAX - remote_sys.mLight_PWMmin)*r/LIGHT_RGB_VMAX;
		gcolor = (LIGHT_PWM_GMAX - remote_sys.mLight_PWMmin)*g/LIGHT_RGB_VMAX;
		bcolor = (LIGHT_PWM_BMAX - remote_sys.mLight_PWMmin)*b/LIGHT_RGB_VMAX;

		rcolor = rcolor*brightness/LIGHT_BRIGHTNESS_MAX;
		gcolor = gcolor*brightness/LIGHT_BRIGHTNESS_MAX;
		bcolor = bcolor*brightness/LIGHT_BRIGHTNESS_MAX;

		if(r)rcolor += remote_sys.mLight_PWMmin;
		if(g)gcolor += remote_sys.mLight_PWMmin;
		if(b)bcolor += remote_sys.mLight_PWMmin;
		if(LIGHT_PWM_RMAX ==rcolor)rcolor=rcolor+1;
		if(LIGHT_PWM_GMAX ==gcolor)gcolor=gcolor+1;
		if(LIGHT_PWM_BMAX ==bcolor)bcolor=bcolor+1;
	#endif//LIGHT_RGB_PWMON
		gpio_set_level(LIGHT_SW_GPIO, 1);
		remote_sys.mLight_SwCloseTimer = 0;
	}
	else
	{
	#if(LIGHT_WY_WBR_YCT && LIGHT_WY_PWMON)//W调光Y调色
		//调色
		yvalue = (LIGHT_PWM_WMAX) * (templatetrue - LIGHT_TEMPLATETRUE_MIN) / (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN);
	#endif
		remote_sys.mLight_SwCloseTimer = (1500/LIGNT_TIMER_TICK);
	}

#if(LIGHT_WY_PWMON && LIGHT_RGB_PWMON)
	if(lewyfan_infor.light_rgbmode != LIGHT_RGB_NORMAL &&
	   lewyfan_infor.light_rgbmode != LIGHT_RGB_FADE &&
	   lewyfan_infor.light_rgbmode != LIGHT_RGB_COLOR &&
	   lewyfan_infor.light_rgbmode != LIGHT_RGB_JUMP)
	{
		rcolor = 0;
		gcolor = 0;
		bcolor = 0;

		sLamp_RValue = 0;
		sLamp_GValue = 0;
		sLamp_BValue = 0;

		rLamp_RValue = 0;
		rLamp_GValue = 0;
		rLamp_BValue = 0;

	}else{
		wvalue = 0;
		yvalue = 0;

		sLamp_templatetrue = 0;
		rLamp_templatetrue = 0;
	}
#endif
//��Ӧ����֧�֣��ı�ռ�ձȣ�ʵ�����Ⱥ���ɫ����
#if(LIGHT_WY_PWMON)
	light_driver_setw(wvalue);
	light_driver_sety(yvalue);
#endif
#if(LIGHT_RGB_PWMON)
	light_driver_setr(rcolor);
	light_driver_setg(gcolor);
	light_driver_setb(bcolor);
#endif
}

static uint32_t wDuty = 0;
static uint32_t yDuty = 0;
static uint32_t rDuty = 0;
static uint32_t gDuty = 0;
static uint32_t bDuty = 0;
/*
 * 设置白光PWM
 */

void light_driver_setw(uint32_t duty)
{
	//LOG_INFO_TAG("setw","duty=%d", duty);
	if(duty == wDuty)return;
	if(wDuty == 0)
	{
#if(LIGHT_PWM_REVERSEL_P2)//
		if(light_timer.freq_hz == LIGHT_FRE_P2)//下拉反转
		{
			//ledc_set_duty_and_update(light_channel[LIGHT_W_NUMCH].speed_mode, light_channel[LIGHT_W_NUMCH].channel, LIGHT_PWM_MAX-remote_sys.mLight_PWMmin,0);
		}else{
			//ledc_set_duty_and_update(light_channel[LIGHT_W_NUMCH].speed_mode, light_channel[LIGHT_W_NUMCH].channel, remote_sys.mLight_PWMmin,0);
		}
#else
		//ledc_set_duty_and_update(light_channel[LIGHT_W_NUMCH].speed_mode, light_channel[LIGHT_W_NUMCH].channel, remote_sys.mLight_PWMmin,0);
#endif
	}
	wDuty = duty;

#if(LIGHT_PWM_REVERSEL_P2)//
	if(light_timer.freq_hz == LIGHT_FRE_P2)//下拉反转
	{
		if(duty > LIGHT_PWM_MAX)duty = 0;
		else					duty = LIGHT_PWM_MAX-duty;
		if(duty == LIGHT_PWM_MAX)duty++;
	}
	//ledc_set_duty_and_update(light_channel[LIGHT_W_NUMCH].speed_mode, light_channel[LIGHT_W_NUMCH].channel, duty,0);
#else
	//ledc_set_duty_and_update(light_channel[LIGHT_W_NUMCH].speed_mode, light_channel[LIGHT_W_NUMCH].channel, duty,0);
#endif
	//ledc_fade_start		   (light_channel[LIGHT_W_NUMCH].speed_mode, light_channel[LIGHT_W_NUMCH].channel, LEDC_FADE_NO_WAIT);
}


/*
 * 设置黄光PWM
 */
void light_driver_sety(uint32_t duty)
{
	//LOG_INFO_TAG("sety","duty=%d", duty);
	if(duty == yDuty)return;

	// 互补判断 >=16K才需要互补输�?
#if(LIGHT_WY_WBR_YCT)//W调光Y调色
#else
	if(light_timer.freq_hz >= 16000) // 16000
	{
		uint32_t yv=yDuty;
		uint32_t wv=wDuty;
		if(yv > LIGHT_PWM_MAX)yv -= 1;
		if(wv > LIGHT_PWM_MAX)wv -= 1;

		if((yv < (LIGHT_PWM_MAX>>1) && wv < (LIGHT_PWM_MAX>>1)) || remote_sys.mLight_ConstantPower)
		{
			//ledc_set_duty_with_hpoint(light_channel[LIGHT_Y_NUMCH].speed_mode, light_channel[LIGHT_Y_NUMCH].channel, yv, (LIGHT_PWM_MAX>>1));
		}
		else
		{
			//if(wv < LIGHT_PWM_MAX)
			//ledc_set_duty_with_hpoint(light_channel[LIGHT_Y_NUMCH].speed_mode, light_channel[LIGHT_Y_NUMCH].channel, yv, wv);
		}
	}
#endif
	if(yDuty == 0)
	{
#if(LIGHT_PWM_REVERSEL_P2)//
		if(light_timer.freq_hz == LIGHT_FRE_P2)//下拉反转
		{
			//ledc_set_duty_and_update(light_channel[LIGHT_Y_NUMCH].speed_mode, light_channel[LIGHT_Y_NUMCH].channel, LIGHT_PWM_MAX - remote_sys.mLight_PWMmin,0);
		}else{
			//ledc_set_duty_and_update(light_channel[LIGHT_Y_NUMCH].speed_mode, light_channel[LIGHT_Y_NUMCH].channel, remote_sys.mLight_PWMmin,0);
		}
#else
		//ledc_set_duty_and_update(light_channel[LIGHT_Y_NUMCH].speed_mode, light_channel[LIGHT_Y_NUMCH].channel, remote_sys.mLight_PWMmin,0);
#endif
	}
	yDuty = duty;
#if(LIGHT_PWM_REVERSEL_P2)//
	if(light_timer.freq_hz == LIGHT_FRE_P2)//下拉反转
	{
		if(duty > LIGHT_PWM_MAX)duty = 0;
		else					duty = LIGHT_PWM_MAX-duty;
		if(duty == LIGHT_PWM_MAX)duty++;
	}
	//ledc_set_duty_and_update(light_channel[LIGHT_Y_NUMCH].speed_mode, light_channel[LIGHT_Y_NUMCH].channel, duty,0);
#else
	//ledc_set_duty_and_update(light_channel[LIGHT_Y_NUMCH].speed_mode, light_channel[LIGHT_Y_NUMCH].channel, duty,0);
#endif
	//ledc_fade_start		   (light_channel[LIGHT_Y_NUMCH].speed_mode, light_channel[LIGHT_Y_NUMCH].channel, LEDC_FADE_NO_WAIT);
}
/*
 * 设置RPWM
 */
void light_driver_setr(uint32_t duty)
{
	//LOG_INFO_TAG("setr","duty=%d", duty);
	if(duty == rDuty)return;
	if(rDuty == 0)
	{
		//ledc_set_duty_and_update(light_channel[LIGHT_R_NUMCH].speed_mode, light_channel[LIGHT_R_NUMCH].channel, remote_sys.mLight_PWMmin,0);
	}
	rDuty = duty;
	//ledc_set_duty_and_update(light_channel[LIGHT_R_NUMCH].speed_mode, light_channel[LIGHT_R_NUMCH].channel, duty,0);
	//ledc_fade_start		    (light_channel[LIGHT_R_NUMCH].speed_mode, light_channel[LIGHT_R_NUMCH].channel, LEDC_FADE_NO_WAIT);
}

/*
 * 设置GPWM
 */
void light_driver_setg(uint32_t duty)
{
	//LOG_INFO_TAG("setg","duty=%d", duty);
	if(duty == gDuty)return;

	if(gDuty == 0)
	{
		//ledc_set_duty_and_update(light_channel[LIGHT_G_NUMCH].speed_mode, light_channel[LIGHT_G_NUMCH].channel, remote_sys.mLight_PWMmin,0);
	}
	gDuty = duty;
	//ledc_set_duty_and_update(light_channel[LIGHT_G_NUMCH].speed_mode, light_channel[LIGHT_G_NUMCH].channel, duty,0);
	//ledc_fade_start		    (light_channel[LIGHT_G_NUMCH].speed_mode, light_channel[LIGHT_G_NUMCH].channel, LEDC_FADE_NO_WAIT);
}

/*
 * 设置RPWM
 */
void light_driver_setb(uint32_t duty)
{
	//LOG_INFO_TAG("setb","duty=%d", duty);
	if(duty == bDuty)return;
	if(bDuty == 0)
	{
		//ledc_set_duty_and_update(light_channel[LIGHT_B_NUMCH].speed_mode, light_channel[LIGHT_B_NUMCH].channel, remote_sys.mLight_PWMmin,0);
	}
	bDuty = duty;
	//ledc_set_duty_and_update(light_channel[LIGHT_B_NUMCH].speed_mode, light_channel[LIGHT_B_NUMCH].channel, duty,0);
	//ledc_fade_start		    (light_channel[LIGHT_B_NUMCH].speed_mode, light_channel[LIGHT_B_NUMCH].channel, LEDC_FADE_NO_WAIT);
}

/*
*�յ����� ����ͨ�ų�ʼ��
*/
void aircondition_driver_uartinit(void)
{
	//�������ýṹ��
	uart_config_t uart1_config;

	//���ڲ�������->uart1
	uart1_config.baud_rate = 9600; //������
	uart1_config.data_bits = UART_DATA_8_BITS; 			//����λ
	uart1_config.parity = UART_PARITY_EVEN; 			//У��λ
	uart1_config.stop_bits = UART_STOP_BITS_1; 			//ֹͣλ
	uart1_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE; 	//Ӳ������
	uart_param_config(UART_NUM_1, &uart1_config); 		//���ô���
	//IOӳ��-> T:IO17 ?R:IO16
	uart_set_pin(UART_NUM_1, UART1_TXD1_PIN, UART1_RXD1_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	//ע�ᴮ�ڷ���ʹ��+���û�������С
	uart_driver_install(UART_NUM_1, UART1_RX1_BUF_SIZE * 2, UART1_TX1_BUF_SIZE * 2, 0, NULL,0);

	//��������1��������
	//xTaskCreate(uart1_rx_task, "uart1_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
	
	//����1���ݷ��Ͳ���
	//aircondition_driver_uartAck();
}

/*
 * 风扇灯初始化
 */
void light_driver_start(void)
{
	LOG_INFO("light_driver_start\r\n");
	light_driver_pcnt_init();
	aircondition_driver_uartinit();

    if(gpio_get_level(LIGHT_TYPE_GPIO1) == 0)	// 判断GPIO4 是否下拉    下拉
	{
		light_timer.freq_hz      = LIGHT_FRE_P2;
		remote_sys.mLight_ConstantPower = LIGHT_CONSTANT_P2;
		remote_sys.mLight_ACenable = LIGHT_AC_ENABLE_P2;
		remote_sys.mLight_PWMmin   = LIGHT_PWM_MIN_P2;
		remote_sys.pX			   = 2;
	}
	else	// 悬空
	{
		light_timer.freq_hz      = LIGHT_FRE_P1;
		remote_sys.mLight_ConstantPower = LIGHT_CONSTANT_P1;
		remote_sys.mLight_ACenable = LIGHT_AC_ENABLE_P1;
		remote_sys.mLight_PWMmin   = LIGHT_PWM_MIN_P1;
		remote_sys.pX			   = 1;
	}
#if(LIGHT_WY_PWMON)
    light_channel[LIGHT_Y_NUMCH].hpoint = (LIGHT_PWM_MAX>>1);
#endif
    // Set configuration of timer0 for high speed channels  设置timer0配置为高速通道
    ledc_timer_config(&light_timer);

    int ch;
    // Set LED Controller with previously prepared configuration   设置LED控制器与预先准备的配�?
#if(LIGHT_WY_PWMON)
    for (ch = LIGHT_W_NUMCH; ch <= LIGHT_Y_NUMCH; ch++) {
	#if(LIGHT_PWM_REVERSEL_P2)//
    	if(light_timer.freq_hz == LIGHT_FRE_P2)//下拉反转
    	{
    		light_channel[ch].duty = LIGHT_PWM_MAX;
    	}
	#endif
        //ledc_channel_config(&light_channel[ch]);
    }
#endif
#if(LIGHT_RGB_PWMON)
    for (ch = LIGHT_R_NUMCH; ch <= LIGHT_B_NUMCH; ch++) {
        //ledc_channel_config(&light_channel[ch]);
    }
#endif

	// Initialize fade service.
	ledc_fade_func_install(0);

	gpio_config_t light_sw_ioconf={.pin_bit_mask=(1<<LIGHT_SW_GPIO), .mode=GPIO_MODE_OUTPUT, .pull_up_en=1, .pull_down_en=0};
	gpio_config(&light_sw_ioconf);
}



/* Initialize PCNT functions:				初始化PCNT函数
 *  - configure and initialize PCNT			配置和初始化PCNT
 *  - set up the input filter				设置输入过滤�?
 *  - set up the counter events to watch	设置要观察的计数器事�?
 */
void light_driver_pcnt_init(void)
{
	// gpio_config_t 配置GPIO参数的结构体
    gpio_config_t light_type1_ioconf={
    		.pin_bit_mask=(1<<LIGHT_TYPE_GPIO1),
    		.mode=GPIO_MODE_INPUT,
    		.pull_up_en=1,
    		.pull_down_en=0};
    // gpio_config 注册GPIO
    gpio_config(&light_type1_ioconf);

    gpio_config_t light_type2_ioconf={
    		.pin_bit_mask=(1<<LIGHT_TYPE_GPIO2),
    		.mode=GPIO_MODE_INPUT,
    		.pull_up_en=1,
    		.pull_down_en=0};
    gpio_config(&light_type2_ioconf);


// 电平信号捕捉�?  用于AC检�?  用到22 23�?
    /* Prepare configuration for the PCNT unit */
    /* 单通道的脉冲计数器配置 */
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = FAN_PCNT_INPUT_SIG_IO,
        .ctrl_gpio_num = (-1),
        .channel = PCNT_CHANNEL_0,
        .unit = FAN_PCNT_UNIT,
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge
        .neg_mode = PCNT_COUNT_INC,   // Keep the counter value on the negative edge
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_KEEP, // Reverse counting direction if low
        .hctrl_mode = PCNT_MODE_KEEP,    // Keep the primary counter mode if high
        // Set the maximum and minimum limit values to watch
        .counter_h_lim = FAN_PCNT_H_LIM_VAL,
        .counter_l_lim = FAN_PCNT_L_LIM_VAL,
    };
    /* Initialize PCNT unit */
    pcnt_unit_config(&pcnt_config);

    /* Initialize PCNT's counter */
    pcnt_counter_pause(FAN_PCNT_UNIT);
    pcnt_counter_clear(FAN_PCNT_UNIT);

    /* Everything is set up, now go to counting */
    pcnt_counter_resume(FAN_PCNT_UNIT);
    //pcnt_get_counter_value(PCNT_TEST_UNIT, &count);

    gpio_config_t light_pcnt_ioconf={.pin_bit_mask=(1<<FAN_PCNT_INPUT_CTRL_IO), .mode=GPIO_MODE_INPUT, .pull_up_en=1, .pull_down_en=0};
    gpio_config(&light_pcnt_ioconf);

    /*
    gpio_config_t light_pcnt_ioconf={.pin_bit_mask=(1<<FAN_PCNT_INPUT_SIG_IO), .mode=GPIO_MODE_INPUT, .pull_up_en=0, .pull_down_en=1};
    gpio_config(&light_pcnt_ioconf);
    */
}

void light_driver_pcnt_resume(void)
{
	pcnt_counter_clear(FAN_PCNT_UNIT);
}

void light_driver_pcnt_get(int16_t *conter)
{
	pcnt_get_counter_value(FAN_PCNT_UNIT,conter);
}


