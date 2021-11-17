#include <stdio.h>
#include <driver/gpio.h>
#include <driver/uart.h>
#include "mesh_gw_uart_parser.h"
#include "device_uart.h"
#include "mesh_gw_handler.h"

#define UART1_TXD1_PIN		17
#define UART1_RXD1_PIN		16

//UART1
#define UART1_RX1_BUF_SIZE	256
#define UART1_TX1_BUF_SIZE	160


static volatile TickType_t xUartCommunicationTickCount = 0;//串口通信时间


/** 
 * @brief:   [通过串口发送数据]
 * @Author:   xunuo
 * @DateTime: 2021年11月6日T16:53:46+0800
 * @param-send_len[IN]:                 [要发送的数据的长度]
 * @param-send_data[IN]:                [要发送的数据的]
 */
void device_uart_send_data(unsigned int send_len,unsigned char* send_data)
{
	
	uart_write_bytes(UART_NUM_1,(const char *)send_data,send_len);
}

/** 
 * @brief:   [串口发送任务处理]
 * @Author:   xunuo
 * @DateTime: 2021年11月8日T15:50:18+0800
 * @param-void[]:                     [description]
 */
void uart1_tx_task_handle(void)
{
	//获取当前时间
	TickType_t xCurrentTickCount = xTaskGetTickCount(); 

	//判断是否有溢出
	if(xCurrentTickCount < xUartCommunicationTickCount)
	{
		xUartCommunicationTickCount = 0;
	}
}

/*
*串口数据处理
*@param-len[IN]:	从串口中读取到的数据的长度
*@param-data[IN]:	从串口中读取到的数据首地址
*@return:
*/
uint32_t uart1_rx_data(uint32_t len,uint8_t *data)
{
	uint16_t result_code = 4;
	
#if 0
	if(len < 5)
	{
		return len;
	}
	
	if(data[0] != 0x55)
	{
		return 1;
	}

	if(data[1] != 0xaa)
	{
		return 2;
	}

	if(data[2])
	{
		return 3;
	}

	switch(data[3])
	{
		/*设备状态查询回复/上报*/
		case 0x0d:
			update_sub_device_status(uint8_t * sub_device_info, uint8_t info_len)
			if(len >= 6)
			{
				//sub_device_info_update(data[4],data[5]);
				result_code = 6;
			}
			else
			{
				result_code = 4;
			}
			break;
		
		/*设备添加进来*/
		case 0x08:
			if(len >= 6)
			{
				printf("cmd_ok\r\n");
				add_sub_device(data[4],data[5]);
				result_code = 6;
			}
			else
			{
				result_code = 4;
			}
			break;
			
		/*设备删除*/
		case 0x03:
			delete_sub_device(data[4],data[5]);
			break;

		case 0x04:
			break;

		case 0x05:
			break;
			

		default:
			break;
		
	}
#endif

	return result_code;
}

/*
	//获取当前时间
	TickType_t xCurrentTickCount = xTaskGetTickCount(); 

	//判断是否有溢出
	if(xCurrentTickCount < xUartCommunicationTickCount)
	{
		xUartCommunicationTickCount = 0;
		//xUartQueryTickCount = 0;
		return;
	}
*/

/*
* 串口接收任务
*/
void uart1_rx_task_perform()
{
	//uint8_t* rx_data = (uint8_t*) malloc(UART1_RX1_BUF_SIZE + 1);       //分配内存，用于串口接收
	uint8_t rx_data[UART1_RX1_BUF_SIZE+1] = {0};
	uint16_t rxi = 0;
	uint32_t rx_len = 0;
	uint32_t offset = 0;

	//while (1)
	//{
		//获取串口1接收的数据
		const int rxBytes = uart_read_bytes(UART_NUM_1, rx_data, UART1_RX1_BUF_SIZE, 100 / portTICK_RATE_MS);

		
		if (rxBytes > 0)
		{
			
			//uart_write_bytes(UART_NUM_1,(const char *) rx_data, rxBytes);
			//更新通信时间
			xUartCommunicationTickCount = xTaskGetTickCount();
			//rx_len  = rxBytes;
			//offset = 0;

			//rx_data[rxBytes] = 0;//在串口接收的数据增加结束符

			//将接收到的数据发出去
			printf("UART1 RX:%d\r\n",rxBytes);
			
			for(rxi = 0;rxi < rxBytes; rxi++)
			{
				printf("%02x ",rx_data[rxi]);
			}
			
			printf("\r\n");
			
			//解析
			while(1)
			{
				//offset += uart1_rx_data(rx_len,&rx_data[offset]);
				offset += mesh_gw_uart_data_parsing(&rx_data[offset], rx_len);
				
				if(offset >= rx_len)break;
			}

			
			

		}
	//}
	//free(rx_data);        //释放申请的内存
}

/** 
 * @brief:   [初始化串口资源,用于和mesh gw模块通信]
 * @Author:   xunuo
 * @DateTime: 2021年11月6日T16:52:24+0800
 * @param-void[]:                     [description]
 */
void device_uart_init(void)
{
	//串口配置结构体
	uart_config_t uart1_config;

	//串口参数配置->uart1
	uart1_config.baud_rate = 115200; //波特率
	uart1_config.data_bits = UART_DATA_8_BITS;			//数据位
	uart1_config.parity = UART_PARITY_DISABLE; 			//校验位
	uart1_config.stop_bits = UART_STOP_BITS_1;			//停止位
	uart1_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;	//硬件流控
	uart_param_config(UART_NUM_1, &uart1_config);		//设置串口
	//IO映射-> T:IO17 ?R:IO16
	uart_set_pin(UART_NUM_1, UART1_TXD1_PIN, UART1_RXD1_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	//注册串口服务即使能+设置缓存区大小
	uart_driver_install(UART_NUM_1, UART1_RX1_BUF_SIZE * 2, UART1_TX1_BUF_SIZE * 2, 0, NULL,0);

	//创建串口1接收任务
	//xTaskCreate(uart1_rx_task_perform, "uart1_rx_task_perform", 1024*2, NULL, configMAX_PRIORITIES, NULL);
	
	//printf("device uart init\r\n");
	
}

uint32_t get_device_uart_communication_time(void)
{
	return xUartCommunicationTickCount;
}
