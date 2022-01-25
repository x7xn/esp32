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

//鍐呴儴鍙橀噺
uint8_t	  	  rPower_ACCheck	 = 1;
uint32_t	  rTimer_Second		 = 0;

uint32_t	  sLamp_brightness   = 0;				//鐏寒搴︾紦瀛樺櫒
uint32_t  	  sLamp_templatetrue = 3000;			//鐏壊娓╃紦瀛樺櫒
uint32_t  	  sLamp_RValue = 0;						//鐏壊娓╃紦瀛樺櫒
uint32_t  	  sLamp_GValue = 0;						//鐏壊娓╃紦瀛樺櫒
uint32_t  	  sLamp_BValue = 0;						//鐏壊娓╃紦瀛樺櫒

uint32_t	  rLamp_brightness   = 0;				//鐏寒搴﹀瘎瀛樺櫒
uint32_t  	  rLamp_templatetrue = 3000;			//鐏壊娓╁瘎瀛樺櫒
uint32_t  	  rLamp_RValue = 0;						//鐏壊娓╃紦瀛樺櫒
uint32_t  	  rLamp_GValue = 0;						//鐏壊娓╃紦瀛樺櫒
uint32_t  	  rLamp_BValue = 0;						//鐏壊娓╃紦瀛樺櫒

uint32_t	  dLamp_brightness   = 1;				//鐏寒搴﹀閲忓瘎瀛樺櫒
uint32_t  	  dLamp_templatetrue = 1;				//鐏壊娓╁閲忓瘎瀛樺櫒
uint32_t  	  dLamp_RValue = 1;						//鐏壊娓╁閲忕紦瀛樺櫒
uint32_t  	  dLamp_GValue = 1;						//鐏壊娓╁閲忕紦瀛樺櫒
uint32_t  	  dLamp_BValue = 1;						//鐏壊娓╁閲忕紦瀛樺櫒
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
//鐏帶閮ㄥ垎PWM
ledc_timer_config_t light_timer = {
		.duty_resolution = LIGHT_BIT_DUTY, 	  // resolution of PWM duty
        .freq_hz	     = LIGHT_FRE_P1,      // frequency of PWM signal
        .speed_mode 	 = LIGHT_MODE,        // timer mode
        .timer_num 		 = LIGHT_TIMER        // timer index
        };

//鐏帶閮ㄥ垎PWM
ledc_channel_config_t light_channel[LIGHT_TOTAL_NUM] = {
		{ .channel = LIGHT_W_CHANNEL, .duty = 0, .gpio_num = LIGHT_W_GPIO, .speed_mode = LIGHT_MODE, .timer_sel = LIGHT_TIMER},
		{ .channel = LIGHT_Y_CHANNEL, .duty = 0, .gpio_num = LIGHT_Y_GPIO, .speed_mode = LIGHT_MODE, .timer_sel = LIGHT_TIMER},
		{ .channel = LIGHT_R_CHANNEL, .duty = 0, .gpio_num = LIGHT_R_GPIO, .speed_mode = LIGHT_MODE, .timer_sel = LIGHT_TIMER},
		{ .channel = LIGHT_G_CHANNEL, .duty = 0, .gpio_num = LIGHT_G_GPIO, .speed_mode = LIGHT_MODE, .timer_sel = LIGHT_TIMER},
		{ .channel = LIGHT_B_CHANNEL, .duty = 0, .gpio_num = LIGHT_B_GPIO, .speed_mode = LIGHT_MODE, .timer_sel = LIGHT_TIMER}
};

//鍐呴儴鍑芥暟
void light_driver_set(uint32_t brightness,uint32_t templatetrue,uint8_t r,uint8_t g, uint8_t b);
void light_driver_setw(uint32_t duty);
void light_driver_sety(uint32_t duty);

void light_driver_setr(uint32_t duty);
void light_driver_setg(uint32_t duty);
void light_driver_setb(uint32_t duty);

static aircondition_device_infor_t aircondition_driver_list[17] = {0};

#define	WAIT_UART_REPLY_DURATION	((uint16_t)20)/*<串口通信最小间隔时间>*/
#define	WAIT_UART_QUERY_STATUS_DURATION	((uint16_t)1000)/*<查询间隔时间-测试使用10000 最终使用1000>*/
#define	WAIT_UART_QUERY_DEVICE_DURATION	((uint16_t)500)/*<查询间隔时间-测试使用10000 最终使用1000>*/

static volatile TickType_t xUartCommunicationTickCount = 0;//串口通信时间

static volatile TickType_t xUartQueryTickCount = 0;//查询时间点

static uint8_t m_query_aircondition_task = 0;//空调查询任务  0-无任务 1-查询状态 2-查询以连接空调的集控地址
static uint8_t m_aircondition_connect_count = 0;//已连接的空调个数
static uint8_t m_aircondition_addr_save_f = 0;//获取到集控地址需要保存

/*
*将内存中读取到的空调信息更新到driver变量
*@param-aircondition_device_info[IN]:空调设备信息
*/
void update_aircondition_addr_to_driver(aircondition_storage_info_t *aircondition_device_info)
{
	uint8_t i = 0;

	//更新已连接的个数
	m_aircondition_connect_count = aircondition_device_info->aircondition_count;

	//更新对应的空调地址
	for(i = 0; i < m_aircondition_connect_count;i++)
	{
		aircondition_driver_list[i].aircondition_addr[0] = aircondition_device_info->aircondition_addr[i] >> 8;
		aircondition_driver_list[i].aircondition_addr[1] = aircondition_device_info->aircondition_addr[i];
	}
}


/*
*挂起查询任务
*task_type[IN]:任务类型 0-无任务 1-查询状态 2-查询以连接空调的集控地址
*/
void pending_query_aircondition_task(uint8_t task_type)
{
	m_query_aircondition_task = task_type;
}

/*
*挂起空调串口控制任务
*@param-aircondition_index[IN]:	要控制的空调下标
*@param-cmd_type[IN]:			要控制的命令 refer@aircond
*@param-aircondition_info[IN]:	要控制的命令内容
*/
void pending_aircondition_uart_task(uint8_t aircondition_index,uint8_t cmd_type,aircondition_device_infor_t aircondition_info)
{
	uint8_t i = 0;
	
	//总控
	if(aircondition_index == 16)
	{
		//所有设备都挂起
		for(; i < m_aircondition_connect_count; i++)
		{
			//挂起串口发送任务
			aircondition_driver_list[i].aircondition_control_f |= cmd_type;
		
			//更新要控制的参数
			if(cmd_type & AIRCONDITION_CMD_SW)
			{
				aircondition_driver_list[i].aircondition_sw = aircondition_info.aircondition_sw;
			}
			if(cmd_type & AIRCONDITION_CMD_MODE)
			{
				switch (aircondition_info.aircondition_mode)
				{
					case 1://制冷
						aircondition_driver_list[i].aircondition_mode = 1;
						break;
		
					case 2://制热
						aircondition_driver_list[i].aircondition_mode = 8;
						break;
		
					case 3://送风
						aircondition_driver_list[i].aircondition_mode = 4;
						break;
					
					case 4://除湿
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
					case 0://自动
						aircondition_driver_list[i].aircondition_fanspeed = 0;
						break;
		
					case 1://低速
						aircondition_driver_list[i].aircondition_fanspeed = 4;
						break;
		
					case 2://中速
						aircondition_driver_list[i].aircondition_fanspeed = 2;
						break;
					
					case 3://高速
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
		//挂起串口发送任务
		aircondition_driver_list[aircondition_index].aircondition_control_f |= cmd_type;
	
		//更新要控制的参数
		if(cmd_type & AIRCONDITION_CMD_SW)
		{
			aircondition_driver_list[aircondition_index].aircondition_sw = aircondition_info.aircondition_sw;
		}
		if(cmd_type & AIRCONDITION_CMD_MODE)
		{
			switch (aircondition_info.aircondition_mode)
			{
				case 1://制冷
					aircondition_driver_list[aircondition_index].aircondition_mode = 1;
					break;
	
				case 2://制热
					aircondition_driver_list[aircondition_index].aircondition_mode = 8;
					break;
	
				case 3://送风
					aircondition_driver_list[aircondition_index].aircondition_mode = 4;
					break;
				
				case 4://除湿
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
				case 0://自动
					aircondition_driver_list[aircondition_index].aircondition_fanspeed = 0;
					break;
	
				case 1://低速
					aircondition_driver_list[aircondition_index].aircondition_fanspeed = 4;
					break;
	
				case 2://中速
					aircondition_driver_list[aircondition_index].aircondition_fanspeed = 2;
					break;
				
				case 3://高速
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
*计算和校验，溢出不计
*@param-source_data[IN]:	需要计算校验和的数据
*@param-data_len[IN]:		需要计算的长度
*@return:					计算得到的校验值
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
*解析板检指令
*@return: 0-不属于板检指令 其它-板检指令
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
*板检指令回复
*ack_type:0-板检指令 1-清网指令 2-允许入网指令 3-查询入网指令
*/
void aircondition_driver_uartAck(uint8_t ack_type)
{
	//板检指令回复
	uint8_t  board_instruction[9] = {0x01,0x25,0x25,0xFF,0xFF,0xFF,0x99,0xC0,0x8C};
	//清网指令回复
	uint8_t  clear_net[9] = {0x01,0xA2,0x55,0xFF,0xFF,0xFF,0xF5,0x9F,0x1D};
	//允许入网指令
	uint8_t  enable_net_ok[9] = {0x01,0xA2,0xAA,0xFF,0xFF,0xFF,0x4A,0xCA,0xB9};
	//允许入网指令
	uint8_t  enable_net_fail[9] = {0x01,0xA3,0xAA,0xFF,0xFF,0xFF,0x4B,0x0A,0xA8};
	//查询联网状态
	uint8_t  query_net_ok[9] = {0x01,0xA0,0xAA,0x01,0x01,0x01,0x4E,0xDA,0xE0};
	//查询联网状态
	uint8_t  query_net_fail[9] = {0x01,0xA0,0xAA,0x00,0x00,0x00,0x4B,0x4B,0x4F};

	switch(ack_type)
	{
		//板检指令
		case 0:
			uart_write_bytes(UART_NUM_1,(const char *)board_instruction,9);
			break;
		//清网指令
		case 1:
			uart_write_bytes(UART_NUM_1,(const char *)clear_net,7);
			enable_connect_net();
			break;
			
		//允许入网指令
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
		//查询入网状态指令
		case 3:
			//获取联网信息
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
*解析清网指令
*type: 1-清网指令 2-允许配网 3-查询网络
*@return: 0-不属于清网指令 其它-清网指令
*/
uint8_t reset_net_instruction(uint8_t *source_data,uint8_t type)
{
	uint8_t clear_net[9] = {0x01,0xA1,0x55,0xFF,0xFF,0xFF,0xF4,0x5E,0xEE};
	uint8_t enable_config_net[9] = {0x01,0xA1,0xAA,0xFF,0xFF,0xFF,0x49,0x8A,0x8B};
	uint8_t query_net_sta[9] = {0x01,0xA0,0xAA,0xFF,0xFF,0xFF,0x48,0x4A,0x9A};
	uint8_t i = 0;

	//重置网络
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
	//允许配网
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
	//查询网络状态
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
*根据串口返回的数据更新空调的状态
*@param-aircondition_status[IN]:空调数据
*@param-aircondition_count[IN]:空调个数
*/
void update_aircondition_status_from_uart(uint8_t* aircondition_status,uint8_t aircondition_count)
{
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t n = 0;

	for(n = 0; n < aircondition_count; n++)
	{
		//不需要对比集控地址
		if(m_query_aircondition_task == 2)
		{
			//arch_printf("device_from_uart\r\n");
			//更新空调状态
			aircondition_driver_list[n].aircondition_addr[0] = aircondition_status[i];
			aircondition_driver_list[n].aircondition_addr[1] = aircondition_status[i+1];
			aircondition_driver_list[n].aircondition_sw = aircondition_status[i+2];
			aircondition_driver_list[n].aircondition_set_templatetrue = aircondition_status[i+3];
			aircondition_driver_list[n].aircondition_mode = aircondition_status[i+4];
			aircondition_driver_list[n].aircondition_fanspeed = aircondition_status[i+5];
			aircondition_driver_list[n].aircondition_local_templatetrue= aircondition_status[i+6];
			aircondition_driver_list[n].aircondition_fault = aircondition_status[i+7];
			aircondition_driver_list[n].aircondition_slave_master= aircondition_status[i+9];

			m_aircondition_connect_count++;//记下已连接个数
			m_aircondition_addr_save_f = 1;//挂起保存空调地址标志位
			aircondition_driver_list[n].aircondition_update_f = 0x01;//最新状态也上报
		}
		else
		{
			for(j = 0; j < 16; j++)
			{
				//根据集控地址找到对应空调
				if((aircondition_status[i] == aircondition_driver_list[j].aircondition_addr[0]) && (aircondition_status[i+1] == aircondition_driver_list[j].aircondition_addr[1]))
				{
					//arch_printf("status_from_uart\r\n");
					if(!aircondition_driver_list[j].aircondition_control_f)
					{
						//更新空调状态
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
*首次使用，获取连接的内机的信息，包含地址和在线情况
*@param-aircondition_status[IN]:串口中获取到的回复信息
*@param-aircondition_count[IN]:获取到的空调的个数
*/
/*void update_aircondition_communication_status(uint8_t* aircondition_status,uint8_t aircondition_count)
{
	uint8_t i = 0;
	uint8_t n = 0;

	for(; n < aircondition_count; n++)
	{
		//获取空调地址
		aircondition_driver_list[n].aircondition_addr[0] = aircondition_status[i];
		aircondition_driver_list[n].aircondition_addr[1] = aircondition_status[i+1];
		aircondition_driver_list[n].aircondition_communicate_status = aircondition_status[i+2];
		
		i+=3;
	}

	//个数不为0
	if(aircondition_count)
	{
		//更新信息到服务层
		update_aircondition_addr_info(aircondition_driver_list,aircondition_count);
	}
}
*/
/*
*串口数据处理
*@param-len[IN]:	从串口中读取到的数据的长度
*@param-data[IN]:	从串口中读取到的数据首地址
*@return:
*/
uint32_t uart1_rx_data(uint32_t len,uint8_t *data)
{
	uint8_t  cmd = 0;
	uint16_t dlen = 0;
	uint16_t result_code = 0;
	/*
	*->板检指令：		01 A5 66 FF FF FF 88 5A 8F 
	*<-板检指令回复： 01 25 25 FF FF FF 99 C0 8C 
	*
	*重置，清除配网信息，重启模块，允许重新配网
	*网关->模块
	*01 A1 55 FF FF FF F4 5E EE 
	*模块->网关
	*成功
	*01 A2 55 FF FF FF F5
	*失败
	*01 A3 55 FF FF FF F6

	*允许入网指令
	*01 A1 AA FF FF FF 49 8A 8B 
	*

	*查询入网状态
	*01 A0 AA FF FF FF 48 4A 9A 
	
	*->空调状态: 		01 50 ff air_condition_count {air_condition_0_status 10bytes} {air_condition_1_status 10bytes}
	*... {air_condition_n_status 10bytes} checksum
	*{air_condition_0_status 10bytes}
	*【外机地址】【内机地址】【开关状态】【设置温度】【模式】 【风速】 【本地温度】 【故障码】 【风向】 【主从机】
	*
	*
	*->空调在线状态	01 50 02 air_condition_count {air_condition_0_status 3bytes} {air_condition_1_status 3bytes}
	*...{air_condition_n_status 3bytes} checksum
	*
	*{air_condition_n_status 3bytes}
	*【外机地址】【内机地址】【在线状态】
	*/

	
	//数据长度小于可解析的最小长度,丢弃数据
	if(len < 7)
	{
		return len;
	}

	if(data[0] != 0x01)return 1;

	if((data[1] != 0xa5) && (data[1] != 0xa1)  && (data[1] != 0xa0) && (data[1] != 0x50))return 2;

	cmd = data[1];

	switch(cmd)
	{
			//板检指令
		case 0xa5:
			if(parse_board_instruction(data))
			{
				//回复
				aircondition_driver_uartAck(0);
				result_code = 9;
			}
			else
			{
				result_code = 2;
			}
			break;
			
			//清网指令
			//允许入网指令
		case 0xa1:
			if(reset_net_instruction(data,1))
			{
				//回复
				aircondition_driver_uartAck(1);
				result_code = 7;
			}
			else if(reset_net_instruction(data,2))
			{
				//回复
				aircondition_driver_uartAck(2);
				result_code = 7;
			}
			else
			{
				result_code = 2;
			}
			break;
	
			//查询入网状态
		case 0xa0:
			if(reset_net_instruction(data,3))
			{
				//回复
				aircondition_driver_uartAck(3);
				result_code = 7;
			}
			else
			{
				result_code = 2;
			}
			break;

			//查询空调状态
		case 0x50:
			//状态查询回复
			if(data[2] == 0xff)
			{
				//空调个数不合法
				if(!data[3] ||(data[3] > 16))
				{
					result_code = 4;
				}
				else
				{
					dlen = data[3]*10+5;

					//数据长度不合法
					if(len < dlen)
					{
						result_code = 3;
					}
					else
					{
						//判断校验位
						if(calculate_uart_checksum(data,dlen-1) == data[len-1])
						{
							//更新对应空调数据
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
			//在线查询回复
			else if(data[2] == 0x02)
			{
				//空调个数不合法
				if(!data[3] ||(data[3] > 16))
				{
					result_code = 4;
				}
				else
				{
					//空调个数不合法
					if(!data[3] ||(data[3] > 16))
					{
						result_code = 4;
					}
					else
					{
						dlen = data[3]*3+5;

						//数据长度不合法
						if(len < dlen)
						{
							result_code = 3;
						}
						else
						{
							//判断校验位
							if(calculate_uart_checksum(data,dlen-1) == data[len-1])
							{
								//更新对应空调数据
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
*串口发送任务
*/
void uart_tx_task(void)
{
	uint8_t i = 0;
	uint8_t send_len = 1;
	uint8_t send_data[10] = {0x01,0x50,0xff,0xff,0xff,0xff,0x4d,};

	//获取当前时间
	TickType_t xCurrentTickCount = xTaskGetTickCount(); 

	//判断是否有溢出
	if(xCurrentTickCount < xUartCommunicationTickCount)
	{
		xUartCommunicationTickCount = 0;
		xUartQueryTickCount = 0;
		return;
	}
	
	//距离上次通信时间已超过最小间隔 
	if(xCurrentTickCount > (xUartCommunicationTickCount + WAIT_UART_REPLY_DURATION))
	{
		//当前有需要执行的控制任务
		for(i = 0; i < m_aircondition_connect_count; i++)
		{
			if(aircondition_driver_list[i].aircondition_control_f)
			{
				/*
				//组控命令
				if(aircondition_driver_list[i].aircondition_control_f >= 0x0f)
				{
					send_data[send_len++] = 0x60;//function_code
					send_data[send_len++] = aircondition_driver_list[i].aircondition_sw;//开关状态
					send_data[send_len++] = aircondition_driver_list[i].aircondition_set_templatetrue;//设置温度
					send_data[send_len++] = aircondition_driver_list[i].aircondition_mode;//模式
					send_data[send_len++] = aircondition_driver_list[i].aircondition_fanspeed;//风速
					send_data[send_len++] = 0x01;//device_count
					send_data[send_len++] = aircondition_driver_list[i].aircondition_addr[0];//集控地址H
					send_data[send_len++] = aircondition_driver_list[i].aircondition_addr[1];//集控地址L
					send_data[send_len] = calculate_uart_checksum(send_data, send_len);

					
					uart_write_bytes(UART_NUM_1,(const char *) send_data, send_len+1);
					//更新通信时间
					xUartCommunicationTickCount = xCurrentTickCount;
					//清楚任务标志位
					aircondition_driver_list[i].aircondition_control_f = 0;
				}
				else 
					*/
				if(aircondition_driver_list[i].aircondition_control_f & AIRCONDITION_CMD_SW)
				{
					
					send_data[send_len++] = 0x31;//function_code
					send_data[send_len++] = aircondition_driver_list[i].aircondition_sw;//开关状态
					send_data[send_len++] = 0x01;//device_count
					send_data[send_len++] = aircondition_driver_list[i].aircondition_addr[0];//集控地址H
					send_data[send_len++] = aircondition_driver_list[i].aircondition_addr[1];//集控地址L
					send_data[send_len] = calculate_uart_checksum(send_data, send_len);
					
					
					uart_write_bytes(UART_NUM_1,(const char *) send_data, send_len+1);
					//更新通信时间
					xUartCommunicationTickCount = xCurrentTickCount;
					//清楚任务标志位
					aircondition_driver_list[i].aircondition_control_f &= (~AIRCONDITION_CMD_SW);
				}
				else if(aircondition_driver_list[i].aircondition_control_f & AIRCONDITION_CMD_MODE)
				{
					
					send_data[send_len++] = 0x33;//function_code
					send_data[send_len++] = aircondition_driver_list[i].aircondition_mode;//模式
					send_data[send_len++] = 0x01;//device_count
					send_data[send_len++] = aircondition_driver_list[i].aircondition_addr[0];//集控地址H
					send_data[send_len++] = aircondition_driver_list[i].aircondition_addr[1];//集控地址L
					send_data[send_len] = calculate_uart_checksum(send_data, send_len);
					
					
					uart_write_bytes(UART_NUM_1,(const char *) send_data, send_len+1);
					//更新通信时间
					xUartCommunicationTickCount = xCurrentTickCount;
					//清楚任务标志位
					aircondition_driver_list[i].aircondition_control_f &= (~AIRCONDITION_CMD_MODE);
				}
				else if(aircondition_driver_list[i].aircondition_control_f & AIRCONDITION_CMD_TEMP)
				{
					
					send_data[send_len++] = 0x32;//function_code
					send_data[send_len++] = aircondition_driver_list[i].aircondition_set_templatetrue;//设置温度
					send_data[send_len++] = 0x01;//device_count
					send_data[send_len++] = aircondition_driver_list[i].aircondition_addr[0];//集控地址H
					send_data[send_len++] = aircondition_driver_list[i].aircondition_addr[1];//集控地址L
					send_data[send_len] = calculate_uart_checksum(send_data, send_len);
					
					
					uart_write_bytes(UART_NUM_1,(const char *) send_data, send_len+1);
					//更新通信时间
					xUartCommunicationTickCount = xCurrentTickCount;
					//清楚任务标志位
					aircondition_driver_list[i].aircondition_control_f &= (~AIRCONDITION_CMD_TEMP);
				}
				else if(aircondition_driver_list[i].aircondition_control_f & AIRCONDITION_CMD_FAN)
				{
					
					send_data[send_len++] = 0x34;//function_code
					send_data[send_len++] = aircondition_driver_list[i].aircondition_fanspeed;//风速
					send_data[send_len++] = 0x01;//device_count
					send_data[send_len++] = aircondition_driver_list[i].aircondition_addr[0];//集控地址H
					send_data[send_len++] = aircondition_driver_list[i].aircondition_addr[1];//集控地址L
					send_data[send_len] = calculate_uart_checksum(send_data, send_len);
					
					
					uart_write_bytes(UART_NUM_1,(const char *) send_data, send_len+1);
					//更新通信时间
					xUartCommunicationTickCount = xCurrentTickCount;
					//清楚任务标志位
					aircondition_driver_list[i].aircondition_control_f &= (~AIRCONDITION_CMD_FAN);
				}

				return;
			}
		}

		//还未获取到集控地址，查询
		if(m_query_aircondition_task == 2)
		{
			//查询时间到达
			if(xCurrentTickCount > (xUartQueryTickCount + WAIT_UART_QUERY_DEVICE_DURATION))
			{
				uart_write_bytes(UART_NUM_1,(const char *) send_data, 7);
				//更新通信时间
				xUartCommunicationTickCount = xCurrentTickCount;
				//更新查询时间
				xUartQueryTickCount = xCurrentTickCount;
			}
		}
		//已获取到集控地址是，查询状态信息
		else
		{
			//查询时间到达
			if(xCurrentTickCount > (xUartQueryTickCount + WAIT_UART_QUERY_STATUS_DURATION))
			{
				uart_write_bytes(UART_NUM_1,(const char *) send_data, 7);
				//更新通信时间
				xUartCommunicationTickCount = xCurrentTickCount;
				//更新查询时间
				xUartQueryTickCount = xCurrentTickCount;
			}
		}
		
	}

	//空调状态从串口中获取到更新,将最新状态与前一状态对比，将更新状态进行上报
	if(m_aircondition_addr_save_f)
	{
		m_query_aircondition_task = 1;//不再需要快速查询
		//arch_printf("addr_to_remote\r\n");			
		update_aircondition_connect_for_save(aircondition_driver_list,m_aircondition_connect_count);

		//清除更新标志位
		m_aircondition_addr_save_f = 0;
	}
	
	for(i = 0; i < m_aircondition_connect_count; i++)
	{	
		
		if(aircondition_driver_list[i].aircondition_update_f)
		{
			//arch_printf("sta_to_remote\r\n");
			update_aircondition_status_for_notify(&aircondition_driver_list[i],i);

			//清除更新标志位
			aircondition_driver_list[i].aircondition_update_f = 0;
		}
	}
	
}

/*
* 串口接收任务
*/
void uart1_rx_task()
{
	//uint8_t* data = (uint8_t*) malloc(UART1_RX1_BUF_SIZE + 1);       //分配内存，用于串口接收
	uint8_t data[UART1_RX1_BUF_SIZE+1] = {0};
	//uint16_t rxi = 0;
	uint32_t rdlen = 0;
	uint32_t offset = 0;

	//while (1)
	//{
		//获取串口1接收的数据
		const int rxBytes = uart_read_bytes(UART_NUM_1, data, UART1_RX1_BUF_SIZE, 100 / portTICK_RATE_MS);

		//uart_write_bytes(UART_NUM_1,(const char *) data, rxBytes);
		
		if (rxBytes > 0)
		{
			//更新通信时间
			xUartCommunicationTickCount = xTaskGetTickCount();
			rdlen  = rxBytes;
			offset = 0;
			
			while(1)
			{
				offset += uart1_rx_data(rdlen,&data[offset]);
				
				if(offset >= rdlen)break;
			}

			/*
			data[rxBytes] = 0;//在串口接收的数据增加结束符

			//将接收到的数据发出去
			//arch_printf("UART1 RX:\r\n");
			
			for(rxi = 0;rxi < rxBytes; rxi++)
			{
				//arch_printf("%02x ",data[rxi]);
			}
			
			//arch_printf("\r\n");
			*/

		}
	//}
	//free(data);        //释放申请的内存
}

void aircondition_driver_task(void)
{
	//发送
	uart_tx_task();
	
	//接收处理
	uart1_rx_task();
}
/*
 * light_driver_task
 * 10MS鎵ц涓�娆?
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

	// AC检测使能
	if(remote_sys.mLight_ACenable)
	{
		// 涓婄數瓒呰繃30绉掑悗 AC鏃犳晥   涓婄數30绉掑唴 姣?0ms浼氳繘鏉ユ娴嬩竴娆?
		//AC检测计时时间到达
		if(++rTimer_Second == LIGHT_AC_CTIMER)//0.2S = 50*0.2 = 10
		{
			rTimer_Second = 0;//计时清0，为下一次的计时做准备
			light_driver_pcnt_get(&acfre);//获取PCNT
			light_driver_pcnt_resume();//重置PCNT，为下一次做准备
			if(rPower_ACCheck)
			{
				rPower_ACCheck--;
				if(acfre <= LIGHT_AC_FRE)//该时间间隔内测到的频率低于最低频率
				{
					remote_sys.mLight_ACenable = 0;
					//arch_printf("Disable AC\r\n");
				}
			}
			if(acfre < LIGHT_AC_FRE)//50HZ  鎺夌數
			{
				if(rFanCheck == 5)//涓婄數寮�濮嬫病鑴夊啿  			//绗竴娆′笂鐢?
				{

				}else{
					if(rFanCheck == 1)//鍒嗘鎺夌數澶勭悊		//鍒嗘
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
			{	// 涓婄數
				if(rFanCheck == 0)//鍒嗘涓婄數澶勭悊
				{
#if(LIGHT_NIGHT_ENABLE)
					if(remote_sys.mLight_night)
					{
						remote_sys.mLight_night = 0;
						unsigned char savesw;
						savesw = lewyfan_infor.light_sw;
						arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
						lewyfan_infor.light_sw = savesw;
						remote_sys.mRemote_stimer = LIGHT_REMOTE_STIMER;//鏈夊懡浠ゅ垯寤惰繜淇濆瓨鏃堕棿
						remote_sys.mRemote_SaveUp   = 1;
					}
#endif
					//arch_printf("PowerUp=%d\r\n",acfre);
#if(LIGHT_SMARTSWITCH_ENABLE)
					if(lewyfan_infor.smart_switch)//鐏靛姩寮�鍏虫墦寮�
					{
						if(lewyfan_infor.light_sw)lewyfan_infor.light_sw = 0;
						else					  lewyfan_infor.light_sw = 1;
					}else
#endif
					{
						if(lewyfan_infor.power_state == POWER_STATE_SAVE)//鎺夌數淇濇埛
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
					if(remote_sys.mLight_powercnt == LIGHT_RESET_CNT)//蹇�熷紑鍏?娆?WIFI杩涘叆閰嶇綉
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
					remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER;//鏈夊懡浠ゅ垯寤惰繜淇濆瓨鏃堕棿
					remote_sys.mRemote_SaveUp  = 1;
				}
				rFanCheck = 1;
			}
		}
	}
	//鐏帶閮ㄥ垎--100MS
	if(remote_sys.mLight_fadedelay)return;
	//缂撳瓨寮�鍏?
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
		//浜害+
		if(rLamp_brightness < sLamp_brightness)
		{
			rLamp_brightness += dLamp_brightness;
			if(rLamp_brightness > sLamp_brightness)rLamp_brightness = sLamp_brightness;
		}
		else
		//浜害-
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

	// 鑹叉俯/浜害璋冭妭鍑芥暟澶勭悊
	if(remote_sys.mRemote_CmdMode)
	{
		switch(remote_sys.mRemote_CmdMode)
		{
		case LCMD_ONEBRIGHTNESS: // //鍗庤壓涓�閿皟鍏?0x01锛氬紑濮?0x02锛氬仠姝?
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

		case LCMD_COLORK: //鍗庤壓涓�閿皟鑹?0x01锛氬紑濮?0x02锛氬仠姝?
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
		//RGB妯″紡璋冭妭--娴佸厜妯″紡 //RGB妯″紡璋冭妭--鍛煎惛妯″紡
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
		case LIGHT_CD_WEAKUP://娓呮櫒鍞ら啋 30鍒嗛挓  鑹叉俯銆?0%-90%銆? 浜害銆?-95%銆?
			if(remote_sys.mLight_ModeTimer % (1000/LIGNT_TIMER_TICK) == 0)//1S娓愬彉涓�娆?30*60 = 1800S
			{
				//鑹叉俯
				temp1 = LIGHT_TEMPLATETRUE_MIN;
				temp2 = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*9/10;
				value = temp1 + ((temp2-temp1)*((1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK)-remote_sys.mLight_ModeTimer)/(1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK));
				lewyfan_infor.light_templatetrue = value;

				//浜害
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
					remote_sys.mRemote_stimer   = LIGHT_REMOTE_STIMER/3;//鏈夊懡浠ゅ垯寤惰繜淇濆瓨鏃堕棿
					remote_sys.mRemote_SaveUp   = 1;
				}
			}
			break;
		case LIGHT_CD_DUSK://榛勬槒鏄庝寒 30鍒嗛挓  鑹叉俯銆?50%-100%銆? 浜害銆?-100%銆?
			if(remote_sys.mLight_ModeTimer % (1000/LIGNT_TIMER_TICK) == 0)//1S娓愬彉涓�娆?30*60 = 1800S
			{
				//鑹叉俯
				temp1 = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*5/10;
				temp2 = LIGHT_TEMPLATETRUE_MAX;
				value = temp1 + ((temp2-temp1)*((1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK)-remote_sys.mLight_ModeTimer)/(1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK));
				lewyfan_infor.light_templatetrue = value;

				//浜害
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
					remote_sys.mRemote_stimer   = LIGHT_REMOTE_STIMER/3;//鏈夊懡浠ゅ垯寤惰繜淇濆瓨鏃堕棿
					remote_sys.mRemote_SaveUp   = 1;
				}
			}
			break;
		case LIGHT_CD_SLEEP://澶滄櫄鍔╃湢30鍒嗛挓  鑹叉俯銆?95%->0%銆? 浜害銆?5-0%銆?
			if(remote_sys.mLight_ModeTimer % (1000/LIGNT_TIMER_TICK) == 0)//1S娓愬彉涓�娆?30*60 = 1800S
			{
				//鑹叉俯
				temp1 = LIGHT_TEMPLATETRUE_MIN + (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN)*95/100;
				temp2 = LIGHT_TEMPLATETRUE_MIN;
				value = temp1 - ((temp1-temp2)*((1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK) - remote_sys.mLight_ModeTimer)/(1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK));
				lewyfan_infor.light_templatetrue = value;

				//浜害
				temp1 = LIGHT_BRIGHTNESS_MAX*lewyfan_infor.sleepsbr/100;
				temp2 = LIGHT_BRIGHTNESS_MIN;
				value = temp1 - ((temp1-temp2)*((1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK)-remote_sys.mLight_ModeTimer)/(1000*60*lewyfan_infor.modetime/LIGNT_TIMER_TICK));
				lewyfan_infor.light_brightness = value;

				if(remote_sys.mLight_ModeTimer == 0)//缁撴潫
				{
					lewyfan_infor.light_sw      = 0;
					lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
					remote_sys.mRemote_stimer   = LIGHT_REMOTE_STIMER/3;//鏈夊懡浠ゅ垯寤惰繜淇濆瓨鏃堕棿
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
 * 寮�鐏? sw=true 鍏崇伅:sw=false
 * 浜害锛歜r(1-100)
 * 鑹叉俯锛歵m(3000-6400)
 */
void light_driver_fade(uint32_t brightness,uint32_t templatetrue,uint8_t r,uint8_t g,uint8_t b)
{
	uint32_t dmax = 0;
	LOG_INFO_TAG("light_fade","br=%d,tp=%d,r=%d,g=%d,b=%d..",brightness, templatetrue,r, g, b);

	//色温不合法的情况下，直接取阈值
	if(templatetrue < LIGHT_TEMPLATETRUE_MIN)templatetrue = LIGHT_TEMPLATETRUE_MIN;
	if(templatetrue > LIGHT_TEMPLATETRUE_MAX)templatetrue = LIGHT_TEMPLATETRUE_MAX;

	//亮度不合法，直接取阈值
	if(brightness > LIGHT_BRIGHTNESS_MAX)brightness = LIGHT_BRIGHTNESS_MAX;

	//remote_sys.mLight_fadetiemr = LIGHT_FADE_TIME;
	sLamp_brightness   = brightness;
	sLamp_templatetrue = templatetrue;

	sLamp_RValue = r;
	sLamp_GValue = g;
	sLamp_BValue = b;

	if(sLamp_brightness != 0 && rLamp_brightness == 0)//寮�鐏?
	{
		if(lewyfan_infor.fade_on == 0)
		{
			light_driver_jump(brightness,templatetrue,r,g,b);
		}
		else
		{
			dLamp_Timer = (LIGHY_FADE_UNIT*lewyfan_infor.fade_on)/LIGNT_TIMER_TICK;//娓愬彉鏃堕棿-->>鑺傛媿鏁版嵁
			//浜害
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

			if(dmax >= dLamp_Timer)//澧為噺澶т簬鎬昏妭鎷嶆暟 渚嬪dmax = 1000 娓愬彉鏃堕棿鐨勬�昏妭鎷峝Lamp_Timer = 100
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
			else				  //澧為噺灏忎簬鎬昏妭鎷嶆暟 渚嬪dmax = 10  娓愬彉鏃堕棿鐨勬�昏妭鎷峝Lamp_Timer = 100
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
	if(sLamp_brightness == 0 && rLamp_brightness != 0)//鍏崇伅
	{
		if(lewyfan_infor.fade_off == 0)
		{
			light_driver_jump(brightness,templatetrue,r,g,b);
		}
		else
		{
			dLamp_Timer = (LIGHY_FADE_UNIT*lewyfan_infor.fade_off)/LIGNT_TIMER_TICK;//娓愬彉鏃堕棿-->>鑺傛媿鏁版嵁
			//浜害
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

			if(dmax >= dLamp_Timer)//澧為噺澶т簬鎬昏妭鎷嶆暟 渚嬪dmax = 1000 娓愬彉鏃堕棿鐨勬�昏妭鎷峝Lamp_Timer = 100
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
			else				  //澧為噺灏忎簬鎬昏妭鎷嶆暟 渚嬪dmax = 10  娓愬彉鏃堕棿鐨勬�昏妭鎷峝Lamp_Timer = 100
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
	else//璋冭妭
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
			//浜害
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

			if(dmax >= dLamp_Timer)//澧為噺澶т簬鎬昏妭鎷嶆暟 渚嬪dmax = 1000 娓愬彉鏃堕棿鐨勬�昏妭鎷峝Lamp_Timer = 100
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
			else				  //澧為噺灏忎簬鎬昏妭鎷嶆暟 渚嬪dmax = 10  娓愬彉鏃堕棿鐨勬�昏妭鎷峝Lamp_Timer = 100
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
	//色温不合法，直接取阈值
	if(templatetrue < LIGHT_TEMPLATETRUE_MIN)templatetrue = LIGHT_TEMPLATETRUE_MIN;
	if(templatetrue > LIGHT_TEMPLATETRUE_MAX)templatetrue = LIGHT_TEMPLATETRUE_MAX;

	//浜害
	//亮度不合法直接取阈值
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
	//安装参数驱动灯的状态
    light_driver_set(rLamp_brightness,rLamp_templatetrue,rLamp_RValue,rLamp_GValue,rLamp_BValue);
}

/*
 * light_driver_set
 * 寮�鐏? sw=true 鍏崇伅:sw=false
 * 浜害锛歜r(1-100)
 * 鑹叉俯锛歵m(3000-6400)
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
	//亮度不为0
	if(brightness)
	{
	#if(LIGHT_WY_PWMON)
		if(templatetrue < LIGHT_TEMPLATETRUE_MIN)templatetrue = LIGHT_TEMPLATETRUE_MIN;
		if(templatetrue > LIGHT_TEMPLATETRUE_MAX)templatetrue = LIGHT_TEMPLATETRUE_MAX;

	#if(LIGHT_WY_WBR_YCT)//W璋冨厜Y璋冭壊
		// 璋冨厜
		wvalue = ((LIGHT_PWM_WMAX - remote_sys.mLight_PWMmin)*brightness)/LIGHT_BRIGHTNESS_MAX;
		wvalue += remote_sys.mLight_PWMmin;
		//璋冭壊
		yvalue = (LIGHT_PWM_WMAX) * (templatetrue - LIGHT_TEMPLATETRUE_MIN) / (LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MIN);
	#else

		if(remote_sys.mLight_ConstantPower == 0)
		{	// 鍙犲姞鍔熺巼
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

		if(templatetrue != LIGHT_TEMPLATETRUE_MIN)//闄愬埗鍗曡竟鏈�灏廝WM
		{
			wvalue += remote_sys.mLight_PWMmin;
		}
		if(templatetrue != LIGHT_TEMPLATETRUE_MAX)//闄愬埗鍗曡竟鏈�灏廝WM
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
	#if(LIGHT_WY_WBR_YCT && LIGHT_WY_PWMON)//W璋冨厜Y璋冭壊
		//璋冭壊
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
//对应功能支持，改变占空比，实现亮度和颜色渐变
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
 * 璁剧疆鐧藉厜PWM
 */

void light_driver_setw(uint32_t duty)
{
	//LOG_INFO_TAG("setw","duty=%d", duty);
	if(duty == wDuty)return;
	if(wDuty == 0)
	{
#if(LIGHT_PWM_REVERSEL_P2)//
		if(light_timer.freq_hz == LIGHT_FRE_P2)//涓嬫媺鍙嶈浆
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
	if(light_timer.freq_hz == LIGHT_FRE_P2)//涓嬫媺鍙嶈浆
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
 * 璁剧疆榛勫厜PWM
 */
void light_driver_sety(uint32_t duty)
{
	//LOG_INFO_TAG("sety","duty=%d", duty);
	if(duty == yDuty)return;

	// 浜掕ˉ鍒ゆ柇 >=16K鎵嶉渶瑕佷簰琛ヨ緭鍑?
#if(LIGHT_WY_WBR_YCT)//W璋冨厜Y璋冭壊
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
		if(light_timer.freq_hz == LIGHT_FRE_P2)//涓嬫媺鍙嶈浆
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
	if(light_timer.freq_hz == LIGHT_FRE_P2)//涓嬫媺鍙嶈浆
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
 * 璁剧疆RPWM
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
 * 璁剧疆GPWM
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
 * 璁剧疆RPWM
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
*空调网关 串口通信初始化
*/
void aircondition_driver_uartinit(void)
{
	//串口配置结构体
	uart_config_t uart1_config;

	//串口参数配置->uart1
	uart1_config.baud_rate = 9600; //波特率
	uart1_config.data_bits = UART_DATA_8_BITS; 			//数据位
	uart1_config.parity = UART_PARITY_EVEN; 			//校验位
	uart1_config.stop_bits = UART_STOP_BITS_1; 			//停止位
	uart1_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE; 	//硬件流控
	uart_param_config(UART_NUM_1, &uart1_config); 		//设置串口
	//IO映射-> T:IO17 ?R:IO16
	uart_set_pin(UART_NUM_1, UART1_TXD1_PIN, UART1_RXD1_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	//注册串口服务即使能+设置缓存区大小
	uart_driver_install(UART_NUM_1, UART1_RX1_BUF_SIZE * 2, UART1_TX1_BUF_SIZE * 2, 0, NULL,0);

	//创建串口1接收任务
	//xTaskCreate(uart1_rx_task, "uart1_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
	
	//串口1数据发送测试
	//aircondition_driver_uartAck();
}

/*
 * 椋庢墖鐏垵濮嬪寲
 */
void light_driver_start(void)
{
	LOG_INFO("light_driver_start\r\n");
	light_driver_pcnt_init();
	aircondition_driver_uartinit();

    if(gpio_get_level(LIGHT_TYPE_GPIO1) == 0)	// 鍒ゆ柇GPIO4 鏄惁涓嬫媺    涓嬫媺
	{
		light_timer.freq_hz      = LIGHT_FRE_P2;
		remote_sys.mLight_ConstantPower = LIGHT_CONSTANT_P2;
		remote_sys.mLight_ACenable = LIGHT_AC_ENABLE_P2;
		remote_sys.mLight_PWMmin   = LIGHT_PWM_MIN_P2;
		remote_sys.pX			   = 2;
	}
	else	// 鎮┖
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
    // Set configuration of timer0 for high speed channels  璁剧疆timer0閰嶇疆涓洪珮閫熼�氶亾
    ledc_timer_config(&light_timer);

    int ch;
    // Set LED Controller with previously prepared configuration   璁剧疆LED鎺у埗鍣ㄤ笌棰勫厛鍑嗗鐨勯厤缃?
#if(LIGHT_WY_PWMON)
    for (ch = LIGHT_W_NUMCH; ch <= LIGHT_Y_NUMCH; ch++) {
	#if(LIGHT_PWM_REVERSEL_P2)//
    	if(light_timer.freq_hz == LIGHT_FRE_P2)//涓嬫媺鍙嶈浆
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



/* Initialize PCNT functions:				鍒濆鍖朠CNT鍑芥暟
 *  - configure and initialize PCNT			閰嶇疆鍜屽垵濮嬪寲PCNT
 *  - set up the input filter				璁剧疆杈撳叆杩囨护鍣?
 *  - set up the counter events to watch	璁剧疆瑕佽瀵熺殑璁℃暟鍣ㄤ簨浠?
 */
void light_driver_pcnt_init(void)
{
	// gpio_config_t 閰嶇疆GPIO鍙傛暟鐨勭粨鏋勪綋
    gpio_config_t light_type1_ioconf={
    		.pin_bit_mask=(1<<LIGHT_TYPE_GPIO1),
    		.mode=GPIO_MODE_INPUT,
    		.pull_up_en=1,
    		.pull_down_en=0};
    // gpio_config 娉ㄥ唽GPIO
    gpio_config(&light_type1_ioconf);

    gpio_config_t light_type2_ioconf={
    		.pin_bit_mask=(1<<LIGHT_TYPE_GPIO2),
    		.mode=GPIO_MODE_INPUT,
    		.pull_up_en=1,
    		.pull_down_en=0};
    gpio_config(&light_type2_ioconf);


// 鐢靛钩淇″彿鎹曟崏鍣?  鐢ㄤ簬AC妫�娴?  鐢ㄥ埌22 23鑴?
    /* Prepare configuration for the PCNT unit */
    /* 鍗曢�氶亾鐨勮剦鍐茶鏁板櫒閰嶇疆 */
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


