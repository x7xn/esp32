#include <stdio.h>
#include <esp_log.h>
#include "mesh_gw_handler.h"
#include "mesh_gw_uart_parser.h"

#define PROTOCOL_FIXED_HEAD0	((uint8_t) 0X55)/*<固定起始头0>*/
#define PROTOCOL_FIXED_HEAD1	((uint8_t) 0Xaa)/*<固定起始头1>*/
#define PROTOCOL_VER			((uint8_t) 0X00)/*<版本>*/
#define PROTOCOL_CMD_UPDATE		((uint8_t) 0X0D)/*<子设备状态上报<->*/
#define PROTOCOL_CMD_CONTROL	((uint8_t) 0X0C)/*<子设备状态控制->>*/
#define PROTOCOL_CMD_HEARTBEAT	((uint8_t) 0X0A)/*<子设备心跳检测->>*/
#define PROTOCOL_CMD_QUERY		((uint8_t) 0X0B)/*<子设备状态查询<->*/

static const char *TAG = "UART PARASER";

#pragma pack(1)

/*<协议数据包>*/
typedef struct{
	uint8_t fixed_head[2];/*<帧头>*/
	uint8_t ver;/*<版本>*/
	uint8_t cmd;/*<命令字>*/
	uint8_t data_len[2];/*<数据长度>*/
	uint8_t* data;/*<数据>*/
	uint8_t checksum;/*<校验位>*/
}protocol_package_t;

#pragma pack()


uint8_t calculate_checksum_result(uint8_t* source_data,uint8_t calculate_len)
{
	uint16_t i = 0;
	uint8_t result_code = 0;

	for(; i < calculate_len; i++)
	{
		result_code += source_data[i];
	}

	return result_code;
}
/** 
 * @brief:   [解析乐式网关模块串口数据]
 * @Author:   xunuo
 * @DateTime: 2021年11月13日T17:04:08+0800
 * @param-source_data[IN]:              [串口数据]
 * @param-data_len[IN]:                 [数据长度]
 * @return:                            [0-数据未接收完毕，需要等待 其它-已解析的数据长度]
 */
uint16_t mesh_gw_uart_data_parsing(uint8_t* source_data,uint16_t data_len)
{
	protocol_package_t* temp_package = (protocol_package_t*)source_data;
	uint16_t temp_data_len = (temp_package->data_len[0] << 8 | temp_package->data_len[1]); 

	//帧头判断
	if(temp_package->fixed_head[0] != PROTOCOL_FIXED_HEAD0)
	{
		ESP_LOGI(TAG,"fixed head0 error\r\n");
		return 1;
	}
	
	if(temp_package->fixed_head[1] != PROTOCOL_FIXED_HEAD1)
	{
		ESP_LOGI(TAG,"fixed head1 error\r\n");
		return 2;
	}
	
	//版本判断
	if(temp_package->ver != PROTOCOL_VER)
	{
		ESP_LOGI(TAG,"ver error\r\n");
		return 3;
	}

	//命令字判断
	if((temp_package->cmd != PROTOCOL_CMD_UPDATE) && (temp_package->cmd != PROTOCOL_CMD_HEARTBEAT) && (temp_package->cmd != PROTOCOL_CMD_QUERY))
	{
		
		ESP_LOGI(TAG,"cmd error\r\n");
		return 4;
	}

	//数据长度判断
	if((temp_data_len + 7) < data_len)
	{
		
		ESP_LOGI(TAG,"data_len error\r\n");
		return 5;
	}

	//校验位判断
	if(calculate_checksum_result(source_data,temp_data_len+6) != source_data[temp_data_len+6])
	{
		ESP_LOGI(TAG,"check error\r\n");
		return 5;
	}

	//数据解析
	switch(temp_package->cmd)
	{
		//状态更新
		case PROTOCOL_CMD_UPDATE:
			
			ESP_LOGI(TAG,"check update\r\n");
			//temp_package->data = &source_data[6];
			//数据长度不合法 or 子设备ID长度不合法
			if((temp_data_len < 16) || (source_data[6] != 0x0a))
			{
				return 5;
			}
			
			update_sub_device_status(&source_data[7],temp_data_len-1);
			return (temp_data_len+7);
			break;
			
		case PROTOCOL_CMD_QUERY:
			break;
			
		case PROTOCOL_CMD_HEARTBEAT:
			break;
	}

	return 5;
}

