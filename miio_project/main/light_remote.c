/******************************************************************************
 * light function used here.
 *
 * FileName: light_remote.c
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
#include "esp_system.h"
#include "miio_net.h"
//#include "../../esp-idf/components/freertos/include/freertos/semaphore.h"

#if MIBLE_ENABLE
#include "mible_gateway.h"
#endif
#include "LeCmd.h"
#include "LetsNetwork.h"
#include "light.h"
#include "light_drivers.h"
#include "light_remote.h"
#include "light_scen.h"
#include "device/handler/S_2_Light_doChange.h"

#define AIRCONDITION_PASSIVE_TASK_INTERNAL		((uint16_t)(500/LIGNT_TIMER_TICK))
#define AIRCONDITION_INITIATIVE_TASK_INTERNAL	((uint16_t)(1000/LIGNT_TIMER_TICK))
#define SAVE_AIR_NAME_TIMER						((uint16_t)(1000/LIGNT_TIMER_TICK))
//#define AIRCONDITION_ALLOW_CONFIG_TIME	((uint32_t)(6000))/*<实际使用6000*30 测试使用 6000>*/


 extern SemaphoreHandle_t xSemaphore;
static volatile uint8_t m_source_is_locked = 0;
//空调内存读写信息
//uint8_t m_wait_aircondition_resp = 0;//等待空调的信息
static aircondition_storage_info_t m_aircondition_device_info = { 0,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
static aircondition_name_t m_aircondition_app_info[17] = {
	{.name_save_f = 0,.name_save_timer = 0,.group_name = {0}},{.name_save_f = 0,.name_save_timer = 0,.group_name = {0}},
	{.name_save_f = 0,.name_save_timer = 0,.group_name = {0}},{.name_save_f = 0,.name_save_timer = 0,.group_name = {0}},
	{.name_save_f = 0,.name_save_timer = 0,.group_name = {0}},{.name_save_f = 0,.name_save_timer = 0,.group_name = {0}},
	{.name_save_f = 0,.name_save_timer = 0,.group_name = {0}},{.name_save_f = 0,.name_save_timer = 0,.group_name = {0}},
	{.name_save_f = 0,.name_save_timer = 0,.group_name = {0}},{.name_save_f = 0,.name_save_timer = 0,.group_name = {0}},
	{.name_save_f = 0,.name_save_timer = 0,.group_name = {0}},{.name_save_f = 0,.name_save_timer = 0,.group_name = {0}},
	{.name_save_f = 0,.name_save_timer = 0,.group_name = {0}},{.name_save_f = 0,.name_save_timer = 0,.group_name = {0}},
	{.name_save_f = 0,.name_save_timer = 0,.group_name = {0}},{.name_save_f = 0,.name_save_timer = 0,.group_name = {0}},
	{.name_save_f = 0,.name_save_timer = 0,.group_name = {0}}
};
//空调保存信息标志位
//uint8_t	m_aircondition_save_f = 0;
//static uint8_t m_current_connected_count = 0;//当前已连接的空调个数

//预定义
//变量
uint8_t			mSave  = 0;
uint8_t  	    mReset = 0;
RemoteType		mRemote_rxpack;
remote_match_t	remote_array[LIGHT_REMOTE_MATCHMAX+1];
remote_task_t   remote_sys=
{
.mLight_TestTimer    = LIGHT_TEST_TIMER,
.mLight_test 		 = 0,
.mLight_online       = 0,
.mLight_mode         = M_LIGHT_PON,
.mLight_powercnt	 = 0,
.mLight_powertim	 = LIGHT_RESET_TIMER,
.mLight_matchtimer   = LIGHT_MATCH_TIMER,

.mLight_showtimr	 = 0,
.mLight_flashcnt	 = 0,
.mLight_fadecnt	     = 0,
.mLight_fadetimer	 = 0,
.mLight_fadedelay    = 0,
.mLight_SwCloseTimer = 0,
.mLight_ConstantPower= LIGHT_CONSTANT_P1,
.mLight_PWMmin       = LIGHT_PWM_MIN_P1,
.mLight_ACenable	 = LIGHT_AC_ENABLE_P1,
.mLight_ModeTimer    = 0,

.mRemote_message	 = 0,
.mRemote_match  	 = 0,
.mRemote_stimer 	 = 0,
.mRemote_matchcnt 	 = 0,
.mRemote_clearcnt 	 = 0,
.mRemote_CmdMode     = 0,
.mRemote_CmdUp       = 0,
.mRemote_CmdFade     = 0,
.mRemote_SaveUp      = 0,
.mRemote_CmdDelay    = 0,
.mRemote_CycTimer    = 0,
.mRemote_StateTime   = 0,
.mRemote_KeyTime     = 0,
.mRemote_UpOff       = 0,
.mRemote_OneKeySence = 0,
.mRemote_MacUpdate   = 0xFF,
.mUpdateTimer        = 0,
};

aircondition_remote_task_t   aircondition_remote_sys=
{
.m_wifi_is_config_f  = 0,
.mLight_online       = 0,
.mLight_powercnt	 = 0,
.mLight_powertim	 = LIGHT_RESET_TIMER,
.mRemote_stimer 	 = 0,
.mRemote_SaveUp_f 	 = 0,
.mpPassiveUpdateTimer        = 0,
};

//空调状态
static volatile aircondition_device_infor_t service_aircondition_infor[17] = {{{0,0},2,0,0,0,4,0,0,0,0,0},
{{0,0},2,0,0,0,4,0,0,0,0,0},{{0,0},2,0,0,0,4,0,0,0,0,0},{{0,0},2,0,0,0,4,0,0,0,0,0},{{0,0},2,0,0,0,4,0,0,0,0,0},
{{0,0},2,0,0,0,4,0,0,0,0,0},{{0,0},2,0,0,0,4,0,0,0,0,0},{{0,0},2,0,0,0,4,0,0,0,0,0},{{0,0},2,0,0,0,4,0,0,0,0,0},
{{0,0},2,0,0,0,4,0,0,0,0,0},{{0,0},2,0,0,0,4,0,0,0,0,0},{{0,0},2,0,0,0,4,0,0,0,0,0},{{0,0},2,0,0,0,4,0,0,0,0,0},
{{0,0},2,0,0,0,4,0,0,0,0,0},{{0,0},2,0,0,0,4,0,0,0,0,0},{{0,0},2,0,0,0,4,0,0,0,0,0},{{0,0},2,0,0,0,4,0,0,0,0,0}};

static volatile int m_current_group = 0;//平台当前操作的空调
static volatile bool m_current_sw = 0;
static volatile int m_current_fanspeed = 0;
static volatile int m_current_mode = 0;
static volatile int m_current_temp = 0;
static volatile int m_current_fault = 0;

static volatile int m_notify_group_id = 0;

static volatile bool m_is_notify_name_f = 0;
static uint8_t m_is_notify_name_count = 0;
static uint8_t m_is_notify_count_f = 0;

//
#define  MREMOTE_CHECK_BUF_MAX	16
uint8_t  mRemote_timer = 0;
uint8_t  mRemote_idcnt = 0;
uint32_t mRemote_idbuf[MREMOTE_CHECK_BUF_MAX] = {0};
uint8_t  mRemote_ldbuf[MREMOTE_CHECK_BUF_MAX] = {0};
uint8_t  mRemote_pdbuf[MREMOTE_CHECK_BUF_MAX] = {0};

//外部变量
extern lewyfan_device_infor_t  	   lewyfan_infor;
extern lewyfan_device_userinfor_t  lewyfan_userinfor;
extern uint8_t				       device_remote_rxcnt;
extern RemoteType			   	   device_remote_rxpack;
extern uint8_t				   	   device_remote_rxpst;
//extern miio_handle_t 		   	   g_miio_instance_handle;

//局部函数
void 	light_remote_taskpoweron(void);
void 	light_remote_taskon(void);
void 	light_remote_taskoff(void);
void 	light_remote_CheckMessage(void);
uint8_t light_remote_Check(uint32_t remote, uint8_t lamp,uint8_t type);
void 	light_remote_externpoweron(uint8_t type);
uint8_t light_remote_clear(uint8_t one);
uint8_t light_remote_match(uint8_t one);


void    light_remote_onhandler(uint8_t cmd);
void    light_remote_blesucess(void);
void    light_remote_getipsucess(void);
void    light_remote_blereset(void);
void 	light_remote_wifiap(uint8_t state);

void cancel_notify(uint8_t skip_index)
{
	uint8_t i = 0;

	for(i = 0; i < 17; i++)
	{
		if(i != skip_index)
		{
			service_aircondition_infor[i].aircondition_notify_f &= (~AIRCONDITION_NOTIFY_NAME);
		}
	}
}

#if 1
int get_service_aircondition_group(void)
{
	return m_current_group;
}

int get_service_aircondition_mode(void)
{
	if(!m_current_group)
	{
		m_current_mode = service_aircondition_infor[16].aircondition_mode;
	}
	else
	{
		m_current_mode = service_aircondition_infor[m_current_group-1].aircondition_mode;
	}

	if(!m_current_mode)
	{
		m_current_mode = 1;
	}
	else if(m_current_mode > 4)
	{
		m_current_mode = 4;
	}
	
	//arch_printf("aircondition_get_mode:%d,%d\r\n",m_current_group,result_code);

	return m_current_mode;
}

int get_service_aircondition_temp(void)
{
	if(!m_current_group)
	{
		m_current_temp = service_aircondition_infor[16].aircondition_set_templatetrue;
	}
	else
	{
		m_current_temp = service_aircondition_infor[m_current_group-1].aircondition_set_templatetrue;
	}
	
	if(!m_current_temp)
	{
		m_current_temp = 16;
	}
	else
	{
		m_current_temp -= 1;
	}
	
	return m_current_temp;
}

bool get_service_aircondition_sw(void)
{
	if(!m_current_group)
	{
		m_current_sw = (service_aircondition_infor[16].aircondition_sw?TRUE:FALSE);
	}
	else
	{
		m_current_sw = (service_aircondition_infor[m_current_group-1].aircondition_sw?TRUE:FALSE);
	}
	
	return m_current_sw;
}


int get_service_aircondition_fanspeed(void)
{
	if(!m_current_group)
	{
		m_current_fanspeed = service_aircondition_infor[16].aircondition_fanspeed;
	}
	else
	{
		m_current_fanspeed = service_aircondition_infor[m_current_group-1].aircondition_fanspeed;
	}
	
	//arch_printf("aircondition_get_fan:%d,%d\r\n",m_current_group,result_code);
	
	if(m_current_fanspeed > 3)
	{
		m_current_fanspeed = 0;
	}
	
	return m_current_fanspeed;
}

int get_service_aircondition_fault(void)
{
	if(!m_current_group)
	{
		m_current_fault = service_aircondition_infor[16].aircondition_fault;
	}
	else
	{
		m_current_fault = service_aircondition_infor[m_current_group-1].aircondition_fault;
	}
	
	
	return m_current_fault;
}
char* get_service_aircondition_name(uint8_t aircondition_index)
{

	//strncpy(output_name,m_aircondition_app_info[aircondition_index].group_name,37);
	//output_name[37] = '\0';

	if(aircondition_index > 15)
	{
		aircondition_index = 0;
	}

	//arch_printf("aircondition_get_name:%d,%s\r\n",group_id,group_name);
	return (m_aircondition_app_info[aircondition_index].group_name);

}
#endif
/*
*平台下发参数更新空调模式
*@param-aircondition_index[IN]:	要控制的空调
*@param-mode[IN]:模式
*/
void update_service_aircondition_mode(uint8_t mode)
{
	uint8_t aircondition_index = 0;
	
	if(!m_current_group)
	{	
		aircondition_index = 16;
		//更新
		service_aircondition_infor[aircondition_index].aircondition_mode = mode;
		service_aircondition_infor[aircondition_index].aircondition_notify_f |= AIRCONDITION_NOTIFY_MODE;
	}
	else
	{
		aircondition_index = m_current_group-1;
		//不相同
		if(mode != service_aircondition_infor[aircondition_index].aircondition_mode)
		{
			//更新
			service_aircondition_infor[aircondition_index].aircondition_mode = mode;
			//准备上报
			//service_aircondition_infor[aircondition_index].aircondition_notify_f |= AIRCONDITION_NOTIFY_MODE;
		}
		
		//取消其它空调的信息上报
		cancel_notify(aircondition_index);
		
		service_aircondition_infor[aircondition_index].aircondition_notify_f |= AIRCONDITION_NOTIFY_MODE;
	}
	
	//挂起串口控制任务
	pending_aircondition_uart_task(aircondition_index,AIRCONDITION_CMD_MODE,service_aircondition_infor[aircondition_index]);
}

void update_service_aircondition_sw(uint8_t sw)
{
	uint8_t aircondition_index = 0;
	//总控
	if(!m_current_group)
	{
		aircondition_index = 16;
		
		service_aircondition_infor[aircondition_index].aircondition_sw = sw;
		service_aircondition_infor[aircondition_index].aircondition_notify_f |= AIRCONDITION_NOTIFY_SW;
	}
	else
	{
		aircondition_index = m_current_group -1;
		//不相同
		if(sw != service_aircondition_infor[aircondition_index].aircondition_sw)
		{
			//更新
			service_aircondition_infor[aircondition_index].aircondition_sw = sw;
			//service_aircondition_infor[aircondition_index].aircondition_notify_f |= AIRCONDITION_NOTIFY_SW;
		}

		//取消其它空调的信息上报
		cancel_notify(aircondition_index);
		
		service_aircondition_infor[aircondition_index].aircondition_notify_f |= AIRCONDITION_NOTIFY_SW;
	}
	
	
	//挂起串口控制任务
	pending_aircondition_uart_task(aircondition_index,AIRCONDITION_CMD_SW,service_aircondition_infor[aircondition_index]);
}

void update_service_aircondition_fanSpeed(uint8_t fan_speed)
{
	uint8_t aircondition_index = 0;
	
	//当前为总控
	if(!m_current_group)
	{
		aircondition_index = 16;
		//更新
		service_aircondition_infor[aircondition_index].aircondition_fanspeed = fan_speed;
	}
	else
	{
		aircondition_index = m_current_group-1;
		//不相同
		if(fan_speed != service_aircondition_infor[aircondition_index].aircondition_fanspeed)
		{
			//更新
			service_aircondition_infor[aircondition_index].aircondition_fanspeed = fan_speed;
			
		}
		
		//取消其它空调的信息上报
		cancel_notify(aircondition_index);
		
	}
	
	service_aircondition_infor[aircondition_index].aircondition_notify_f |= AIRCONDITION_NOTIFY_FAN;
	//挂起串口控制任务
	pending_aircondition_uart_task(aircondition_index,AIRCONDITION_CMD_FAN,service_aircondition_infor[aircondition_index]);
	
}

void update_service_aircondition_SetTemp(uint8_t set_temp)
{
	uint8_t aircondition_index = 0;
	
	//总控
	if(!m_current_group)
	{
		aircondition_index = 16;
		
		//更新
		service_aircondition_infor[aircondition_index].aircondition_set_templatetrue = set_temp;
		service_aircondition_infor[aircondition_index].aircondition_notify_f |= AIRCONDITION_NOTIFY_TEMP;
	}
	else
	{
		aircondition_index = m_current_group - 1;
		//不相同
		if(set_temp != service_aircondition_infor[aircondition_index].aircondition_set_templatetrue)
		{
			//更新
			service_aircondition_infor[aircondition_index].aircondition_set_templatetrue = set_temp;
			
			//service_aircondition_infor[aircondition_index].aircondition_notify_f |= AIRCONDITION_NOTIFY_TEMP;
		}
		
		//取消其它空调的信息上报
		cancel_notify(aircondition_index);
		
		service_aircondition_infor[aircondition_index].aircondition_notify_f |= AIRCONDITION_NOTIFY_TEMP;
	}
	
	//挂起串口控制任务
	pending_aircondition_uart_task(aircondition_index,AIRCONDITION_CMD_TEMP,service_aircondition_infor[aircondition_index]);

}
//static uint8_t m_group_value_notify_f = 0;
//static uint8_t m_group_vale = 0;

void update_service_aircondition_group(uint8_t group_value)
{        
        TickType_t xLastWakeTime;

        if( xSemaphore != NULL )
        {
                // See if we can obtain the semaphore.  If the semaphore is not available
                // wait 10 ticks to see if it becomes free.
                if( xSemaphoreTake( xSemaphore, ( TickType_t ) 20 ) == pdTRUE)
                {
					//LOG_ERROR("xSemaphore1\r\n");
                        if(group_value > 16)
                        {
                                return;
                        }
						m_source_is_locked = 1;
                        m_current_group = group_value;
                        
                        //总控
                        if(!group_value)
                        {
                                
                                service_aircondition_infor[16].aircondition_notify_f |= AIRCONDITION_NOTIFY_GROUP;        
                                return;
                        }
                                
                        //取消其它空调的信息上报
                        cancel_notify(group_value-1);

                        //更新        
                        service_aircondition_infor[group_value-1].aircondition_notify_f |= AIRCONDITION_NOTIFY_GROUP;        
                        
                        //挂起当前空调状态上报标志位
                        service_aircondition_infor[group_value-1].aircondition_notify_f |= AIRCONDITION_NOTIFY_SW;        
                        service_aircondition_infor[group_value-1].aircondition_notify_f |= AIRCONDITION_NOTIFY_MODE;        
                        service_aircondition_infor[group_value-1].aircondition_notify_f |= AIRCONDITION_NOTIFY_FAN;        
                        service_aircondition_infor[group_value-1].aircondition_notify_f |= AIRCONDITION_NOTIFY_TEMP;        
                        service_aircondition_infor[group_value-1].aircondition_notify_f |= AIRCONDITION_NOTIFY_FAULT;        
        
                        xSemaphoreGive( xSemaphore );
						m_source_is_locked = 0;

                        xLastWakeTime = xTaskGetTickCount();

                        vTaskDelayUntil( &xLastWakeTime, 200/portTICK_RATE_MS );
                }
                else
                {
                        
                    //LOG_ERROR("xSemaphore error1\r\n");
					if(m_source_is_locked)
					{
					  xSemaphoreGive( xSemaphore );
					  	m_source_is_locked = 0;
					}
                }
        }
        
}



bool get_notify_name_flag(void)
{
	return m_is_notify_name_f;
}

int get_service_aircondition_num(void)
{
	return m_aircondition_device_info.aircondition_count;
}



void update_service_aircondition_notifyname(bool notify_flag)
{
	//uint8_t i = 0;

	m_is_notify_name_f = notify_flag;
	
	//开启上报
	if(notify_flag == 1)
	{
/*
		for(i=0;i < 16;i++)
		{
			service_aircondition_infor[i].aircondition_notify_f |= AIRCONDITION_NOTIFY_NAME;
		}
		m_is_notify_name_count = 16;
*/
	}
}

void save_aircondition_name(void)
{
	uint8_t i = 0;
	char key_name[11] = {'a','i','r','_','n','a','m','e','0','0','\0'};
	TickType_t xLastWakeTime = 0;
	
	//倒计时
	/*for(i = 0; i < 16; i++)
	{
		if(m_aircondition_app_info[i].name_save_timer)
		{
			m_aircondition_app_info[i].name_save_timer--;
		}
	}
	*/
	
	//for(i = 0; i < 16; i++)
	//{
		//有需要保存的信息,一次保存一条
		//if(m_aircondition_app_info[i].name_save_f)
		//{
			//时间到达
			//if(!m_aircondition_app_info[i].name_save_timer)
			//{
				////arch_printf("aircondition_remote_task\r\n");
				if( xSemaphore != NULL )
				{
					// See if we can obtain the semaphore.  If the semaphore is not available
					// wait 10 ticks to see if it becomes free.
					if( xSemaphoreTake( xSemaphore, ( TickType_t ) 20 ) == pdTRUE)
					{
						
						for(i = 0; i < 16; i++)
						{
							//有需要保存的信息,一次保存一条
							if(m_aircondition_app_info[i].name_save_f)
							{
								if(m_aircondition_app_info[i].name_save_timer)
								{
									m_aircondition_app_info[i].name_save_timer--;
								}
								
								//时间到达
								if(!m_aircondition_app_info[i].name_save_timer)
								{
									
									//LOG_ERROR("xSemaphore3\r\n");
									
									//执行保存
									key_name[8] = (i > 9)?('1'):('0');
									key_name[9] = (i > 9)?(i-10+'0'):(i+'0');
									arch_psm_set_str("air_info", key_name, m_aircondition_app_info[i].group_name);
					
									//清除保存标志位
									m_aircondition_app_info[i].name_save_f = 0;
									//arch_printf("remote save name%d:%s\r\n",i,m_aircondition_app_info[i].group_name);
									//break;
								}
							}
						}

						
						xSemaphoreGive( xSemaphore);

						xLastWakeTime = xTaskGetTickCount();

						vTaskDelayUntil( &xLastWakeTime, 200/portTICK_RATE_MS );
						//break;
					}
					else
					{   
						//LOG_ERROR("xSemaphore error3\r\n");
						//如果占用来自于set group
						if(m_source_is_locked)
						{
							xSemaphoreGive( xSemaphore );
							m_source_is_locked = 0;
						}

					}
				}
			//}
		//}
	//}
	
}

void update_service_aircondition_name(uint8_t aircondition_index,char* name,uint32_t name_len)
{
	TickType_t xLastWakeTime;
	uint8_t i = 0;

	if( xSemaphore != NULL )
	{
		// See if we can obtain the semaphore.  If the semaphore is not available
		// wait 10 ticks to see if it becomes free.
		if( xSemaphoreTake( xSemaphore, ( TickType_t ) 100 ) == pdTRUE)
		{
			// We were able to obtain the semaphore and can now access the
			// shared resource.

			// ...

			// We have finished accessing the shared resource.  Release the
			// semaphore.
			if((name_len > 37) || (aircondition_index > 15))
			{
				return;
			}
			
			//取消其它空调的信息上报
			cancel_notify(aircondition_index);
			service_aircondition_infor[aircondition_index].aircondition_notify_f |= 0xa0;
			
			//LOG_ERROR("xSemaphore0\r\n");
			//不相同
			if(strcmp(name,m_aircondition_app_info[aircondition_index].group_name) != 0)
			{
				memset(m_aircondition_app_info[aircondition_index].group_name, '\0', 37);
				//更新
				strncpy(m_aircondition_app_info[aircondition_index].group_name,name,name_len);
				
				//准备保存
				m_aircondition_app_info[aircondition_index].name_save_f = 1;
				
				//所有保存时间都后延
				for(i = 0; i < 16;i++)
				{
					m_aircondition_app_info[i].name_save_timer = SAVE_AIR_NAME_TIMER;
				}
				
				//arch_printf("name:%d update\r\n",aircondition_index);
				
				//arch_printf("name:%d update-%s\r\n",aircondition_index,m_aircondition_app_info[aircondition_index].group_name);
			}
			
			xSemaphoreGive( xSemaphore );

			xLastWakeTime = xTaskGetTickCount();

			vTaskDelayUntil( &xLastWakeTime, 200/portTICK_RATE_MS );
		}
		else
		{
			
	    	//LOG_ERROR("xSemaphore error0\r\n");
		}
	}

	
	//arch_printf("name:%d update-%s\r\n",aircondition_index,name);
}

void aircondition_name_change_notify(void)
{
	
	switch(m_notify_group_id)
	{
		case 0:
			P_2_2_Name_doChange_notify(m_aircondition_app_info[m_notify_group_id].group_name);//name
			break;
		case 1:
			P_2_3_Name_doChange_notify(m_aircondition_app_info[m_notify_group_id].group_name);//name
			break;
		case 2:
			P_2_4_Name_doChange_notify(m_aircondition_app_info[m_notify_group_id].group_name);//name
			break;
		case 3:
			P_2_5_Name_doChange_notify(m_aircondition_app_info[m_notify_group_id].group_name);//name
			break;
		case 4:
			P_2_6_Name_doChange_notify(m_aircondition_app_info[m_notify_group_id].group_name);//name
			break;
		case 5:
			P_2_7_Name_doChange_notify(m_aircondition_app_info[m_notify_group_id].group_name);//name
			break;
		case 6:
			P_2_8_Name_doChange_notify(m_aircondition_app_info[m_notify_group_id].group_name);//name
			break;
		case 7:
			P_2_9_Name_doChange_notify(m_aircondition_app_info[m_notify_group_id].group_name);//name
			break;
		case 8:
			P_2_10_Name_doChange_notify(m_aircondition_app_info[m_notify_group_id].group_name);//name
			break;
		case 9:
			P_2_11_Name_doChange_notify(m_aircondition_app_info[m_notify_group_id].group_name);//name
			break;
		case 10:
			P_2_12_Name_doChange_notify(m_aircondition_app_info[m_notify_group_id].group_name);//name
			break;
		case 11:
			P_2_13_Name_doChange_notify(m_aircondition_app_info[m_notify_group_id].group_name);//name
			break;
		case 12:
			P_2_14_Name_doChange_notify(m_aircondition_app_info[m_notify_group_id].group_name);//name
			break;
		case 13:
			P_2_15_Name_doChange_notify(m_aircondition_app_info[m_notify_group_id].group_name);//name
			break;
		case 14:
			P_2_16_Name_doChange_notify(m_aircondition_app_info[m_notify_group_id].group_name);//name
			break;
		case 15:
			P_2_17_Name_doChange_notify(m_aircondition_app_info[m_notify_group_id].group_name);//name
			break;
		
	}
}

void aircondition_remote_update(void)
{
	uint8_t notify_cnt = 0;
	uint8_t i = 0;	

	
	if(aircondition_remote_sys.mLight_online == 0)return;

	//空调个数需要上报
	if(m_is_notify_count_f)
	{
		P_2_19_NotifyNum_doChange_notify(m_aircondition_device_info.aircondition_count);
		notify_cnt++;
		m_is_notify_count_f = 0;
	}
	//name属性上报完毕
	if(m_is_notify_name_f == 1 && !m_is_notify_name_count)
	{
		//m_is_notify_name_f = 2;
		P_2_18_NotifyGroupName_doChange_notify(m_is_notify_name_f);
		notify_cnt++;
	}
	
	
	//检查有需要上报的任务
	for(i = 0; i < 17; i++)
	{
		//需要上报
		if(service_aircondition_infor[i].aircondition_notify_f)
		{
			
			//arch_printf("notify_value%d:%d\r\n",i,service_aircondition_infor[i].aircondition_notify_f);
			//需要上报name
			//name需要上报
			if(service_aircondition_infor[i].aircondition_notify_f & AIRCONDITION_NOTIFY_NAME)
			{
				
				//arch_printf("notify_name%d:%s\r\n",i,m_aircondition_app_info[i].group_name);
				
				//总控无name可报
				if(i == 16)
				{
					service_aircondition_infor[i].aircondition_notify_f &= (~AIRCONDITION_NOTIFY_NAME);
				}
				else
				{
					m_notify_group_id = i;
					aircondition_name_change_notify();
					//aircondition_name_change_notify();
					notify_cnt++;
					service_aircondition_infor[i].aircondition_notify_f &= (~AIRCONDITION_NOTIFY_NAME);
	
					if(m_is_notify_name_f == 1 && m_is_notify_name_count)
					{
						m_is_notify_name_count--;
					}
					break;
				}
	
			}

			//arch_printf("notify_value%d:%d\r\n",i,service_aircondition_infor[i].aircondition_notify_f);
			//需要上报name
			//name需要上报
			if(service_aircondition_infor[i].aircondition_notify_f & AIRCONDITION_NOTIFY_NAME1)
			{
				
				//arch_printf("notify_name%d:%s\r\n",i,m_aircondition_app_info[i].group_name);
				
				//总控无name可报
				if(i == 16)
				{
					service_aircondition_infor[i].aircondition_notify_f &= (~AIRCONDITION_NOTIFY_NAME1);
				}
				else
				{
					m_notify_group_id = i;
					aircondition_name_change_notify();
					//aircondition_name_change_notify();
					notify_cnt++;
					service_aircondition_infor[i].aircondition_notify_f &= (~AIRCONDITION_NOTIFY_NAME1);
				}
	
			}

			if(notify_cnt > 3)
			{
				//arch_printf("\r\n");
				break;
			}
#if 1
			//其余也需要上报
			if(service_aircondition_infor[i].aircondition_notify_f & AIRCONDITION_NOTIFY_GROUP)
			{
				//arch_printf("notify_group:%d\r\n",((i==16)?0:(i+1)));
				//首先上报group
				if(i == 16)
				{
					m_notify_group_id = 0;
				}
				else
				{
					m_notify_group_id = i+1;
				}
				
				P_2_1_Group_doChange_notify(m_notify_group_id);//group
				notify_cnt++;
				
				//arch_printf("group:%d",i);

				service_aircondition_infor[i].aircondition_notify_f &= (~AIRCONDITION_NOTIFY_GROUP);

			}
			
			if(notify_cnt > 3)
			{
				//arch_printf("\r\n");
				break;
			}
#endif
			//设置温度需要上报
			if(service_aircondition_infor[i].aircondition_notify_f & AIRCONDITION_NOTIFY_TEMP)
			{
				m_current_temp = ((service_aircondition_infor[i].aircondition_set_templatetrue < 16)?(16):(service_aircondition_infor[i].aircondition_set_templatetrue));

				P_3_4_SetTemp_doChange_notify(m_current_temp);//templatetrue
				notify_cnt++;
				service_aircondition_infor[i].aircondition_notify_f &= (~AIRCONDITION_NOTIFY_TEMP);

				//arch_printf(",temp:%d",service_aircondition_infor[i].aircondition_set_templatetrue);
			}
			
			if(notify_cnt > 4)
			{
				//arch_printf("\r\n");
				break;
			}
			//开关状态需要上报
			if(service_aircondition_infor[i].aircondition_notify_f & AIRCONDITION_NOTIFY_SW)
			{
				m_current_sw = service_aircondition_infor[i].aircondition_sw;
				
				P_3_1_OnOff_doChange_notify(m_current_sw);//on off
				notify_cnt++;
				service_aircondition_infor[i].aircondition_notify_f &= (~AIRCONDITION_NOTIFY_SW);
				
				//arch_printf(",sw:%d",service_aircondition_infor[i].aircondition_sw);
			}
			
			if(notify_cnt > 4)
			{
				//arch_printf("\r\n");
				break;
			}
			
			//风速需要上报
			if(service_aircondition_infor[i].aircondition_notify_f & AIRCONDITION_NOTIFY_FAN)
			{
				m_current_fanspeed = ((service_aircondition_infor[i].aircondition_fanspeed > 3)?0:service_aircondition_infor[i].aircondition_fanspeed);

				P_4_2_FanSpeed_doChange_notify(m_current_fanspeed);//fan speed
				notify_cnt++;
				service_aircondition_infor[i].aircondition_notify_f &= (~AIRCONDITION_NOTIFY_FAN);
				
				//arch_printf(",fan:%d",service_aircondition_infor[i].aircondition_fanspeed);
			}
			
			if(notify_cnt > 4)
			{
				//arch_printf("\r\n");
				break;
			}
			
			//模式需要上报
			if(service_aircondition_infor[i].aircondition_notify_f & AIRCONDITION_NOTIFY_MODE)
			{
				m_current_mode = ((service_aircondition_infor[i].aircondition_mode==0)?1:service_aircondition_infor[i].aircondition_mode);

				P_3_2_Mode_doChange_notify(m_current_mode);//mode
				notify_cnt++;
				service_aircondition_infor[i].aircondition_notify_f &= (~AIRCONDITION_NOTIFY_MODE);
				
				//arch_printf(",mode:%d",service_aircondition_infor[i].aircondition_mode);
			}
			
			if(notify_cnt > 4)
			{
				//arch_printf("\r\n");
				break;
			}
			
			//错误码需要上报
			if(service_aircondition_infor[i].aircondition_notify_f & AIRCONDITION_NOTIFY_FAULT)
			{
				m_current_fault = service_aircondition_infor[i].aircondition_fault;
				
				P_3_3_Fault_doChange_notify(m_current_fault);//fault
				notify_cnt++;
				service_aircondition_infor[i].aircondition_notify_f &= (~AIRCONDITION_NOTIFY_FAULT);

				//arch_printf(",fault:%d",service_aircondition_infor[i].aircondition_fault);
			}
			
			//arch_printf("\r\n");
			//每个空调的信息分开发送
			break;
		}
	}

#if 0
	//有发送
	if(notify_cnt)
	{
		//延长串口更新带来的数据上报时间
		aircondition_remote_sys.mpInitiativeUpdateTimer += AIRCONDITION_PASSIVE_TASK_INTERNAL;
	}
#endif
}

#if 1
void aircondition_uart_update_notify(void)
{
	uint8_t i = 0;	

	
	//检查当前分组的空调是否有需要上报的信息
	i = m_current_group;

	if(i > 16 || (!i))
	{
		return;
	}
	
	i-=1;

	
	//需要上报
	if(service_aircondition_infor[i].aircondition_update_f)
	{
		service_aircondition_infor[i].aircondition_notify_f |= service_aircondition_infor[i].aircondition_update_f;
		service_aircondition_infor[i].aircondition_notify_f |= AIRCONDITION_NOTIFY_GROUP;
		service_aircondition_infor[i].aircondition_update_f = 0;
	}

	
	//擦除其它空调上报
	cancel_notify(i);

/*
	for(i = 0; i < 16; i++)
	{
		service_aircondition_infor[i].aircondition_update_f = 0;
	}
*/
}
#endif


/*
*擦除空调的存储信息
*/
void aircondition_earse_info(uint8_t erase_mask)
{
	uint8_t i = 0;
	char key_name[11] = {'a','i','r','_','n','a','m','e','0','0','\0'};
	char default_name[15] = {'\\','u','7','a','7','a','\\','u','8','c','0','3','1','6','\0'};//"\\u7a7a\\u8c0316";
	
	//arch_printf("aircondition_earse_info\r\n");

	//擦除name
	if(erase_mask & 0x01)
	{
		//获取group_name对应的值
		for(i = 0; i < 16; i++)
		{
			key_name[8] = (i > 9)?('1'):('0');
			key_name[9] = (i > 9)?(i-10+'0'):(i+'0');

			if(i < 9)
			{
				default_name[12] = '1'+i;
				default_name[13] = '\0';
			}
			else
			{
				default_name[12] = '1';
				default_name[13] = '0'+i-9;
				
				default_name[14] = '\0';
			}
			strncpy(m_aircondition_app_info[i].group_name, default_name, strlen(default_name));
			m_aircondition_app_info[i].group_name[strlen(default_name)] = '\0';
			//arch_printf("default_name,%s: %s\r\n",key_name,m_aircondition_app_info[i].group_name);
						
			arch_psm_set_str(AIRCONDITION_SAVE_NAME_SPACE, key_name,m_aircondition_app_info[i].group_name);
			
		}
	}

	//擦除地址
	if(erase_mask & 0x02)
	{
		m_aircondition_device_info.aircondition_count = 0;
		
		//arch_printf("default_addr:");
		for(i = 0; i < 16; i++)
		{
			m_aircondition_device_info.aircondition_addr[i] = 0;
			//arch_printf("[%d]:%d,",i,m_aircondition_device_info.aircondition_addr[i]);
		}
		
		arch_psm_set_value(AIRCONDITION_SAVE_NAME_SPACE, AIRCONDITION_KEY_ADDR, (void*)&m_aircondition_device_info, sizeof(m_aircondition_device_info)) ;
		
		//arch_printf("\r\n");
		pending_query_aircondition_task(2);
		update_aircondition_addr_to_driver(&m_aircondition_device_info);
		//更新全局变量
		for(i = 0; i < 17; i++)
		{
			service_aircondition_infor[i].aircondition_fanspeed = 5;
			service_aircondition_infor[i].aircondition_fault = 0;
			service_aircondition_infor[i].aircondition_set_templatetrue = 0;
			service_aircondition_infor[i].aircondition_mode = 0;
			service_aircondition_infor[i].aircondition_sw = 2;
			service_aircondition_infor[i].aircondition_notify_f = 0;
			service_aircondition_infor[i].aircondition_update_f = 0;
		}
	}
}

//设备上电配网状态
//state=0配网中
//state=1已配网
void  light_remote_wifiap(uint8_t state)
{
	aircondition_remote_sys.m_wifi_is_config_f = state;
}

//连接路由器成功
void    light_remote_getipsucess(void)
{
	if(remote_sys.mLight_test)
	{
		remote_sys.mLight_fadecnt += SHOW_CNT45;
	}
}

//设备离线
void light_remote_offline(void)
{
	//arch_printf("light_remote_offline\r\n");
	remote_sys.mLight_online = 0;
	//light_remote_wifiap(1);

	aircondition_remote_sys.mLight_online = 0;
}


/*
*首次获取空调的信息，进行存储
*@param-aircondition_info[IN]:空调信息列表
*@param-aircondition_count[IN]:已获取到的空调的个数
*/
void update_aircondition_connect_for_save(aircondition_device_infor_t* aircondition_info,uint8_t aircondition_count)
{
	uint8_t i = 0;

	if(m_aircondition_device_info.aircondition_count != aircondition_count)
	{
		//上报空调个数
		m_is_notify_count_f = 1;
	}
	m_aircondition_device_info.aircondition_count = aircondition_count;//记下个数

	for(i = 0; i < aircondition_count; i++)
	{
		m_aircondition_device_info.aircondition_addr[i] = ((aircondition_info[i].aircondition_addr[0] << 8)|aircondition_info[i].aircondition_addr[1]);

	}

	//保存信息在内存中
	//arch_printf("air_addr_save:%d\r\n",aircondition_count);
	arch_psm_set_value("air_info", "air_addr", (void*)&m_aircondition_device_info, sizeof(m_aircondition_device_info));
}

/*
*根据从串口中获取的数据更新空调状态，并且检查是否需要上报
*@param-aircondition_info[IN]:	更新后的空调信息
*@param-aircondition_index[IN]:	需要检查更新的空调
*/
void update_aircondition_status_for_notify(aircondition_device_infor_t* aircondition_info,uint8_t aircondition_index)
{
	//开关状态不同
	if(service_aircondition_infor[aircondition_index].aircondition_sw != (aircondition_info->aircondition_sw))
	{
		//更新开关状态
		service_aircondition_infor[aircondition_index].aircondition_sw = aircondition_info->aircondition_sw;
		//挂起上报标志位
		service_aircondition_infor[aircondition_index].aircondition_update_f |= AIRCONDITION_NOTIFY_SW;
	}

	/*
	*模式		空调网关		平台
	*制冷			1		1
	*制热			8		2
	*送风			4		3
	*除湿			2		4
	*/
	//模式不同
	//最新状态为制冷
	if((aircondition_info->aircondition_mode == 1) && (service_aircondition_infor[aircondition_index].aircondition_mode != 1))
	{
		service_aircondition_infor[aircondition_index].aircondition_mode = 1;
		service_aircondition_infor[aircondition_index].aircondition_update_f |= AIRCONDITION_NOTIFY_MODE;
	}
	//最新状态为除湿
	else if((aircondition_info->aircondition_mode == 2) && (service_aircondition_infor[aircondition_index].aircondition_mode != 4))
	{
		service_aircondition_infor[aircondition_index].aircondition_mode = 4;
		service_aircondition_infor[aircondition_index].aircondition_update_f |= AIRCONDITION_NOTIFY_MODE;
	}
	//最新状态为送风
	else if((aircondition_info->aircondition_mode == 4) && (service_aircondition_infor[aircondition_index].aircondition_mode != 3))
	{
		service_aircondition_infor[aircondition_index].aircondition_mode = 3;
		service_aircondition_infor[aircondition_index].aircondition_update_f |= AIRCONDITION_NOTIFY_MODE;
	}
	//最新状态为制热
	else if((aircondition_info->aircondition_mode == 8) && (service_aircondition_infor[aircondition_index].aircondition_mode != 2))
	{
		service_aircondition_infor[aircondition_index].aircondition_mode = 2;
		service_aircondition_infor[aircondition_index].aircondition_update_f |= AIRCONDITION_NOTIFY_MODE;
	}
	
	/*
	*风速		空调网关		平台
	*自动			0		0
	*低速			4		1
	*中速			2		2
	*高速			1		3
	*/
	//风速不同
	//最新状态为自动
	if((!aircondition_info->aircondition_fanspeed) && (service_aircondition_infor[aircondition_index].aircondition_fanspeed))
	{
		service_aircondition_infor[aircondition_index].aircondition_fanspeed = 0;
		service_aircondition_infor[aircondition_index].aircondition_update_f |= AIRCONDITION_NOTIFY_FAN;
	}
	//最新状态为高速
	else if((aircondition_info->aircondition_fanspeed == 1) && (service_aircondition_infor[aircondition_index].aircondition_fanspeed != 3))
	{
		service_aircondition_infor[aircondition_index].aircondition_fanspeed = 3;
		service_aircondition_infor[aircondition_index].aircondition_update_f |= AIRCONDITION_NOTIFY_FAN;
	}
	//最新状态为中速
	else if((aircondition_info->aircondition_fanspeed == 2) && (service_aircondition_infor[aircondition_index].aircondition_fanspeed != 2))
	{
		service_aircondition_infor[aircondition_index].aircondition_fanspeed = 2;
		service_aircondition_infor[aircondition_index].aircondition_update_f |= AIRCONDITION_NOTIFY_FAN;
	}
	//最新状态为低速
	else if((aircondition_info->aircondition_fanspeed == 4) && (service_aircondition_infor[aircondition_index].aircondition_fanspeed != 1))
	{
		service_aircondition_infor[aircondition_index].aircondition_fanspeed = 1;
		service_aircondition_infor[aircondition_index].aircondition_update_f |= AIRCONDITION_NOTIFY_FAN;
	}

	//设置温度不同
	if(service_aircondition_infor[aircondition_index].aircondition_set_templatetrue != (aircondition_info->aircondition_set_templatetrue))
	{
		service_aircondition_infor[aircondition_index].aircondition_set_templatetrue = aircondition_info->aircondition_set_templatetrue;
		service_aircondition_infor[aircondition_index].aircondition_update_f |= AIRCONDITION_NOTIFY_TEMP;
	}

	//故障码不同
	if(service_aircondition_infor[aircondition_index].aircondition_fault != (aircondition_info->aircondition_fault))
	{
		service_aircondition_infor[aircondition_index].aircondition_fault = aircondition_info->aircondition_fault;
		service_aircondition_infor[aircondition_index].aircondition_update_f |= AIRCONDITION_NOTIFY_FAULT;
	}

/*
	if(service_aircondition_infor[aircondition_index].aircondition_update_f)
	{
		//arch_printf("have_update\r\n");
	}
*/
}


//设备上线
void light_remote_online(void)
{
	//arch_printf("light_remote_offline\r\n");
	
	remote_sys.mLight_online = 1;
	aircondition_remote_sys.mLight_online = 1;
	aircondition_remote_sys.m_wifi_is_config_f = 1;//已配网
	service_aircondition_infor[0].aircondition_notify_f |= 0x3f;//上报一个空调的属性点
	m_is_notify_count_f = 1;//连接个数上报
	
	aircondition_remote_update();
	//light_remote_wifiap(2);
}

//设备配网成功
void    light_remote_blesucess(void)
{
	if(remote_sys.mLight_test == 0 && mReset == 0)
	{
		//arch_printf("light_remote_blesucess\r\n");
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX;
		remote_sys.mLight_fadecnt += SHOW_CNT3;
	}
}

//设备重置
void    light_remote_blereset(void)
{
	remote_sys.mLight_online = 0;
	aircondition_remote_sys.mLight_online = 0;
	//允许配网
	//aircondition_remote_sys.mLight_config_wifi_f = 1;
	//aircondition_remote_sys.mpAllowConfigTimer = AIRCONDITION_ALLOW_CONFIG_TIME;
	aircondition_earse_info(3);
	//light_remote_wifiap(0);
	if(mReset == 0 && remote_sys.mLight_test == 0)
	{
		mReset = WIFI_RESET_APP;
		//arch_printf("light_remote_blereset\r\n");
		remote_sys.mLight_fadecnt += SHOW_CNT1;
		light_ResetData(1);
	}
}

/*
* 灯控主进程初始化
*/
void  aircondition_remote_start(void)
{
	uint16_t i = 0;
	char key_name[11] = {'a','i','r','_','n','a','m','e','0','0','\0'};
	char default_name[15] = {'\\','u','7','a','7','a','\\','u','8','c','0','3','1','6','\0'};//"\\u7a7a\\u8c0316";
	
	//arch_printf("aircondition_remote_start:%d\r\n",sizeof(m_aircondition_app_info));

	//获取group_name对应的值
	for(i = 0; i < 16; i++)
	{
		key_name[8] = (i > 9)?('1'):('0');
		key_name[9] = (i > 9)?(i-10+'0'):(i+'0');
		
		if(arch_psm_get_str(AIRCONDITION_SAVE_NAME_SPACE, key_name, m_aircondition_app_info[i].group_name, 37) <= 0)
		{
			if(i < 9)
			{
				default_name[12] = '1'+i;
				default_name[13] = '\0';
			}
			else
			{
				default_name[12] = '1';
				default_name[13] = '0'+i-9;
				default_name[14] = '\0';
			}
			
			strncpy(m_aircondition_app_info[i].group_name, default_name, strlen(default_name));
			m_aircondition_app_info[i].group_name[strlen(default_name)] = '\0';
			//arch_printf("remote_start,%d-%d: %s\r\n",strlen(default_name),strlen(m_aircondition_app_info[i].group_name),m_aircondition_app_info[i].group_name);
						
			arch_psm_set_str(AIRCONDITION_SAVE_NAME_SPACE, key_name,m_aircondition_app_info[i].group_name);
		}
		else
		{
			
			m_aircondition_app_info[i].group_name[37] = '\0';
			
			//arch_printf("remote_read,%d: %s\r\n",strlen(m_aircondition_app_info[i].group_name),m_aircondition_app_info[i].group_name);
		}
		
	}
	
	//读取空调设备的信息
	if(arch_psm_get_value(AIRCONDITION_SAVE_NAME_SPACE, AIRCONDITION_KEY_ADDR, (void*)&m_aircondition_device_info, sizeof(m_aircondition_device_info)) <= 0)
	{
		//arch_printf("air_info, read false...\r\n");
		//持续查询,等待获取空调信息，
		pending_query_aircondition_task(2);
	}
	else
	{		
		//arch_printf("air_read,count:%d\r\naddr:",m_aircondition_device_info.aircondition_count);
		for(i = 0; i < 16; i++)
		{
			//arch_printf("[%d]:%d,",i,m_aircondition_device_info.aircondition_addr[i]);
		}
		
		//arch_printf("\r\n");

		//个数不合法
		if(m_aircondition_device_info.aircondition_count > 16)
		{
			m_aircondition_device_info.aircondition_count = 16;
		}
		//还未获取到空调信息
		else if(!m_aircondition_device_info.aircondition_count)
		{
			pending_query_aircondition_task(2);
		}
		else
		{
			pending_query_aircondition_task(1);
		}

		//将信息更新到设备
		update_aircondition_addr_to_driver(&m_aircondition_device_info);
	}
	
	#if 1	
	arch_psm_get_value(AIRCONDITION_SAVE_NAME_SPACE, AIRCONDITION_KEY_POWER, (void*)&aircondition_remote_sys.mLight_powercnt, sizeof(aircondition_remote_sys.mLight_powercnt));
	aircondition_remote_sys.mLight_powercnt++;

	//快速开关8次-WIFI进入配网
	if(aircondition_remote_sys.mLight_powercnt > 8)
	{
		aircondition_remote_sys.mLight_mode = WIFI_RESET_POWER;		
		//arch_printf("reset_mode\r\n");
	}
	else
	{
		aircondition_remote_sys.mLight_mode = 2;//上电状态
		aircondition_remote_sys.mRemote_SaveUp_f = 1;
		
	}

	if(aircondition_remote_sys.mRemote_SaveUp_f)arch_psm_set_value(AIRCONDITION_SAVE_NAME_SPACE, AIRCONDITION_KEY_POWER, (void*)&aircondition_remote_sys.mLight_powercnt, sizeof(aircondition_remote_sys.mLight_powercnt));

	aircondition_remote_sys.mLight_powertim	= LIGHT_RESET_TIMER;// 10S分段开关
	
	//arch_printf("powercnt:%d\r\n",aircondition_remote_sys.mLight_powercnt);
	#endif
	//i = AIRCONDITION_PASSIVE_TASK_INTERNAL;
	//arch_printf("aircondition_remote_task time:%d\r\n",i);
	
	//arch_printf("save name timer:%d \r\n",SAVE_AIR_NAME_TIMER);
}

/*
 * 灯控主进程初始化
 */
void light_remote_start(void)
{
	LOG_INFO("light_remote_start\r\n");

	mSave = 0;
	//设备信息
	if(arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor)) <= 0)
	{
		LOG_INFO_TAG("light_remote", "le_infor read false...");
		light_ResetData(0);
	}
	else
	{
		LOG_INFO_TAG("light_remote","light: SW=%d,BR=%d,TP=%d...",(unsigned int )lewyfan_infor.light_sw, lewyfan_infor.light_brightness, lewyfan_infor.light_templatetrue);
	}

	if(lewyfan_infor.power_state > POWER_STATE_ON)
	{
		lewyfan_infor.power_state = POWER_STATE_ON;
	}
#if(LIGHT_SMARTSWITCH_ENABLE)
	if(lewyfan_infor.smart_switch)//灵动开关打开
	{
		if(lewyfan_infor.light_sw == 0)
		{
			lewyfan_infor.light_sw = 1;
		}else{
			lewyfan_infor.light_sw = 0;
		}
		mSave |= 0x01;
	}else
#endif
	if(lewyfan_infor.power_state == POWER_STATE_SAVE)//掉电保户
	{
		RePowerOnInit();
	}
	else
	if(lewyfan_infor.power_state == POWER_STATE_ON)
	{
		if(lewyfan_infor.light_sw == 0)
		{
			lewyfan_infor.light_sw = 1;
			mSave |= 0x01;
		}
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
		}
	}


	//快速开关计数
	arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_POWER, (void*)&remote_sys.mLight_powercnt, sizeof(remote_sys.mLight_powercnt));
	remote_sys.mLight_powercnt++;

	//快速开关5次-WIFI进入配网
	//快速开关5次-WIFI进入配网
	if(remote_sys.mLight_powercnt > LIGHT_RESET_CNT)
	{
		if(remote_sys.mLight_powercnt > LIGHT_RESET_CNT_LOCK || (lewyfan_infor.lock_state & APP_LOCK_RSTSET) == 0)
		{
			mReset = WIFI_RESET_POWER;
			remote_sys.mLight_fadecnt = SHOW_CNT3;
			light_ResetData(2);
		}else{
			mReset = 0;
			mSave |= 0x04;
		}
	}else{
		mReset = 0;
		mSave |= 0x04;
	}

	light_driver_jump(0,
					  lewyfan_infor.light_templatetrue,
					  lewyfan_infor.light_rcolor,
					  lewyfan_infor.light_gcolor,
					  lewyfan_infor.light_bcolor);
	if(lewyfan_infor.light_sw && mReset == 0)
	{
		light_driver_fade(lewyfan_infor.light_brightness,
					  	  lewyfan_infor.light_templatetrue,
					  	  lewyfan_infor.light_rcolor,
					  	  lewyfan_infor.light_gcolor,
					  	  lewyfan_infor.light_bcolor);
	}

	//对码遥控器信息读取
	if(arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_MATCH, (void*)&remote_array, sizeof(remote_array)) <= 0)
	{
		LOG_INFO_TAG("light_remote", "le_match read false...");
		memset((void*)&remote_array,0,sizeof(remote_array));
	}
	else
	{
		for(int i=0; i<LIGHT_REMOTE_MATCHMAX; i++)
		{
			if(remote_array[i].use)
			{
				LOG_INFO("remote rid = %08x, lid = %d\r\n",remote_array[i].rid,remote_array[i].lid);
			}
		}
	}

	if(mSave & 0x04)arch_psm_set_value(LIGHT_SAVE_NAME, LIGHT_KEY_POWER, (void*)&remote_sys.mLight_powercnt, sizeof(remote_sys.mLight_powercnt));
	if(mSave & 0x01)arch_psm_set_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
	if(mSave & 0x02)arch_psm_set_value(LIGHT_SAVE_NAME, LIGHT_KEY_PONSETBIT, (void*)&remote_sys.Other, sizeof(remote_sys.Other));

	remote_sys.mLight_mode      = M_LIGHT_PON;
	remote_sys.mLight_powertim  = LIGHT_RESET_TIMER;	// 10S分段开关
	remote_sys.mLight_matchtimer= LIGHT_MATCH_TIMER;	//
	remote_sys.mLight_night     = 0;
}
/*
*
*/
uint8_t get_wifi_connect_status(void)
{
	return aircondition_remote_sys.m_wifi_is_config_f;	
}
/*
*是否允许联网
*/
uint8_t get_allow_net_status(void)
{
	if(aircondition_remote_sys.mLight_online)
	{
		return 0;
	}
	
	return 1;
}

void enable_connect_net(void)
{
	//清除上电次数
	aircondition_remote_sys.mLight_powercnt = 0;
	//更新内存中的快速上电计数
	arch_psm_set_value(AIRCONDITION_SAVE_NAME_SPACE, AIRCONDITION_KEY_POWER, (void*)&aircondition_remote_sys.mLight_powercnt, sizeof(aircondition_remote_sys.mLight_powercnt));
	
	miio_net_stop();
	miio_net_set_ap(0, 0, 0);
	miio_net_restart();
	aircondition_remote_sys.mLight_mode = 0;
	aircondition_remote_sys.mLight_online = 0;
	
	//aircondition_remote_sys.mLight_config_wifi_f = 1;
	//重置允许配网计时器
	//aircondition_remote_sys.mpAllowConfigTimer = AIRCONDITION_ALLOW_CONFIG_TIME;
	//light_remote_wifiap(0);
	aircondition_earse_info(3);
	//arch_printf("enable_config_net-----\r\n");

	//定时30分钟
	
}
/*
*空调与平台相关处理主进程
*/
void aircondition_remote_task(void)
{
	TickType_t xLastWakeTime;
	
	if(aircondition_remote_sys.mLight_powertim)
	{
		aircondition_remote_sys.mLight_powertim--;
	}
	
	//当前处于可能重置状态，但是超时未有下一次上电
	if(aircondition_remote_sys.mLight_mode && aircondition_remote_sys.mLight_powertim == 0)
	{
		//清除上电次数
		aircondition_remote_sys.mLight_powercnt = 0;
		//更新内存中的快速上电计数
		arch_psm_set_value(AIRCONDITION_SAVE_NAME_SPACE, AIRCONDITION_KEY_POWER, (void*)&aircondition_remote_sys.mLight_powercnt, sizeof(aircondition_remote_sys.mLight_powercnt));
		//arch_printf("powercnt=0-----\r\n");
		
		//当前已进入重置
		if(aircondition_remote_sys.mLight_mode == WIFI_RESET_POWER)
		{
			miio_net_stop();
			miio_net_set_ap(0, 0, 0);
			miio_net_restart();
			aircondition_remote_sys.mLight_mode = 0;
			//light_remote_wifiap(0);
			aircondition_earse_info(3);
			//arch_printf("reset_end...\r\n");
		}
		else
		{
			light_driver_jump(0,lewyfan_infor.light_templatetrue,lewyfan_infor.light_rcolor,lewyfan_infor.light_gcolor,lewyfan_infor.light_bcolor);
			//mReset = WIFI_RESET_WAIT;
		}
		aircondition_remote_sys.mLight_mode = 0;
		/*
		arch_os_ms_sleep(1000);
		esp_restart();
		*/
	}
	
#if 0
	//更新允许配网计时器
	if(aircondition_remote_sys.mpAllowConfigTimer)
	{
		aircondition_remote_sys.mpAllowConfigTimer--;
	}	

	//允许配网时间到达且当前处于配网状态
	if(!aircondition_remote_sys.mpAllowConfigTimer && aircondition_remote_sys.mLight_config_wifi_f)
	{
		//当前仍未完成网络配置
		if(!aircondition_remote_sys.mLight_online)
		{
			//arch_printf("stop config_wifi\r\n");
			//停止配网操作
			miio_net_stop();
			//关闭配网状态
			aircondition_remote_sys.mLight_config_wifi_f = 0;
		}
	}
#endif
	
	//notify
	if(aircondition_remote_sys.mpPassiveUpdateTimer)
	{
		aircondition_remote_sys.mpPassiveUpdateTimer--;
	}

	//允许上报时间到达，进行上报
	if(aircondition_remote_sys.mpPassiveUpdateTimer == 0)
	{ 
		////arch_printf("aircondition_remote_task\r\n");
		 if( xSemaphore != NULL )
        {
                // See if we can obtain the semaphore.  If the semaphore is not available
                // wait 10 ticks to see if it becomes free.
               		if( xSemaphoreTake( xSemaphore, ( TickType_t ) 20 ) == pdTRUE)
                	{
						////LOG_ERROR("xSemaphore2\r\n");
					
                       	aircondition_remote_update();  
					
        
                        xSemaphoreGive( xSemaphore );

                        xLastWakeTime = xTaskGetTickCount();

                        vTaskDelayUntil( &xLastWakeTime, 200/portTICK_RATE_MS );
                }
                else
                {   
                    //LOG_ERROR("xSemaphore error2\r\n");
					//如果占用来自于set group
					if(m_source_is_locked)
					{
					  xSemaphoreGive( xSemaphore );
					  	m_source_is_locked = 0;
					}
					
                }
		}
		
		//aircondition_remote_sys.mpPassiveUpdateTimer = AIRCONDITION_PASSIVE_TASK_INTERNAL;//2000/LIGNT_TIMER_TICK
	}

	 aircondition_uart_update_notify();
					   
	//保存name属性
	save_aircondition_name();
#if 0
	//串口数据带来的notify
	if(aircondition_remote_sys.mpInitiativeUpdateTimer)
	{
		aircondition_remote_sys.mpInitiativeUpdateTimer--;
	}

	//允许上报时间到达，进行上报
	if(aircondition_remote_sys.mpInitiativeUpdateTimer == 0)
	{ 
		aircondition_uart_update_notify();
		aircondition_remote_sys.mpInitiativeUpdateTimer = AIRCONDITION_INITIATIVE_TASK_INTERNAL;//
	}
#endif

	
}
/*
 * 灯控主进程-没10MS运行一次
 */
void light_remote_task(void)
{

//	//arch_printf("-----light_remote_task-----\n");

	light_remote_CheckMessage();
	if (remote_sys.mRemote_message)
	{
		//arch_printf("light_remote  remote_sys.mRemote_match = %d\r\n",remote_sys.mRemote_match);
		//arch_printf("light_remote  Cmd = %d\r\n",mRemote_rxpack.Cmd);
	}
	switch(remote_sys.mLight_mode)
	{
	case M_LIGHT_PON:
		light_remote_taskpoweron();
		break;
	case M_LIGHT_ON:
		light_remote_taskon();
		break;
	case M_LIGHT_OFF:
		light_remote_taskoff();
		break;
	default:
		remote_sys.mLight_mode = M_LIGHT_ON;
		lewyfan_infor.light_sw = 1;
		if(remote_sys.mLight_fadecnt == 0 && remote_sys.mLight_flashcnt == 0)
		{
			light_driver_fade(lewyfan_infor.light_brightness,
							  lewyfan_infor.light_templatetrue,
							  lewyfan_infor.light_rcolor,
							  lewyfan_infor.light_gcolor,
							  lewyfan_infor.light_bcolor);
		}
		break;
	}
	//灯渐变
	if(remote_sys.mLight_showtimr)remote_sys.mLight_showtimr--;
	if(remote_sys.mLight_showtimr == 0)
	{
		if(remote_sys.mLight_fadecnt)
		{
			remote_sys.mLight_showtimr  = LIGHT_FLASH_FRE;
			remote_sys.mLight_fadetimer = LIGHT_FLASH_FRE>>1;
			if(remote_sys.mLight_test)
			{
				if(remote_sys.mLight_fadecnt == 1)
				{
					light_driver_fade(lewyfan_infor.light_brightness,lewyfan_infor.light_templatetrue,lewyfan_infor.light_rcolor,lewyfan_infor.light_gcolor,lewyfan_infor.light_bcolor);
				}
				else
				{
					if(remote_sys.mLight_fadecnt & 0x01)
					{
						light_driver_fade(lewyfan_infor.light_brightness,LIGHT_TEMPLATETRUE_MIN,lewyfan_infor.light_rcolor,lewyfan_infor.light_gcolor,lewyfan_infor.light_bcolor);
					}else{
						light_driver_fade(lewyfan_infor.light_brightness,LIGHT_TEMPLATETRUE_MAX,lewyfan_infor.light_rcolor,lewyfan_infor.light_gcolor,lewyfan_infor.light_bcolor);
					}
				}
			}
			else
			{
				if(remote_sys.mLight_fadecnt & 0x01)
				{
					light_driver_fade(lewyfan_infor.light_brightness,lewyfan_infor.light_templatetrue,lewyfan_infor.light_rcolor,lewyfan_infor.light_gcolor,lewyfan_infor.light_bcolor);
				}else{
					light_driver_fade(LIGHT_BRIGHTNESS_MIN,lewyfan_infor.light_templatetrue,lewyfan_infor.light_rcolor,lewyfan_infor.light_gcolor,lewyfan_infor.light_bcolor);
				}
			}
			remote_sys.mLight_fadecnt--;
		}
		else
		{
			if(mReset != 0 && remote_sys.mLight_fadecnt == 0)
			{
				remote_sys.mLight_powercnt = 0;
				arch_psm_set_value(LIGHT_SAVE_NAME, LIGHT_KEY_POWER, (void*)&remote_sys.mLight_powercnt, sizeof(remote_sys.mLight_powercnt));

				if(mReset == WIFI_RESET_POWER)
				{
					miio_net_stop();
					miio_net_set_ap(0, 0, 0);
					miio_net_restart();
					mReset = 0;
					light_remote_wifiap(0);
				}
				else{
					light_driver_jump(0,lewyfan_infor.light_templatetrue,lewyfan_infor.light_rcolor,lewyfan_infor.light_gcolor,lewyfan_infor.light_bcolor);
					mReset = WIFI_RESET_WAIT;
				}
				/*
				arch_os_ms_sleep(1000);
				esp_restart();
				*/
			}
			if(remote_sys.mLight_flashcnt)
			{
				remote_sys.mLight_showtimr  = LIGHT_FLASH_FRE;
				if(lewyfan_infor.light_sw)//开灯
				{
					if(remote_sys.mLight_flashcnt & 0x01)
					{
						light_driver_jump(lewyfan_infor.light_brightness,lewyfan_infor.light_templatetrue,lewyfan_infor.light_rcolor,lewyfan_infor.light_gcolor,lewyfan_infor.light_bcolor);
					}else{
						light_driver_jump(0								,lewyfan_infor.light_templatetrue,lewyfan_infor.light_rcolor,lewyfan_infor.light_gcolor,lewyfan_infor.light_bcolor);
					}
				}
				else{
					if(remote_sys.mLight_flashcnt & 0x01)
					{
						light_driver_jump(0								,lewyfan_infor.light_templatetrue,lewyfan_infor.light_rcolor,lewyfan_infor.light_gcolor,lewyfan_infor.light_bcolor);
					}else{
						light_driver_jump(lewyfan_infor.light_brightness,lewyfan_infor.light_templatetrue,lewyfan_infor.light_rcolor,lewyfan_infor.light_gcolor,lewyfan_infor.light_bcolor);
					}
				}
				remote_sys.mLight_flashcnt--;
				if(remote_sys.mLight_flashcnt == 0)
				{
					if(remote_sys.mLight_test)
					{
						miio_net_stop();
						miio_net_set_ap("miio_default", "0x82562647", 1);
						miio_net_start_async(500);
					}
				}
			}
		}
	}

	if(remote_sys.mRemote_stimer)
	{	
		// 每次灯状态变化都要 调用一次
		if(--remote_sys.mRemote_stimer == 0)
		{
			//设备信息  保存设备信息
			if(remote_sys.mLight_night == 0)
			{
				if(arch_psm_set_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor)) <= 0)
				{
					LOG_INFO_TAG("light_remote", "le_infor save false...");
				}
			}
			if(remote_sys.mRemote_SaveUp)
			{
				remote_sys.mRemote_SaveUp = 0;
				light_remote_update();
			}
		}
	}
	if(remote_sys.mRemote_CycTimer)
	{
		remote_sys.mRemote_CycTimer--;
		if(remote_sys.mRemote_CycTimer == 0)
		{
			if(remote_sys.mLight_mode == LCMD_ONEBRIGHTNESS)
			{
				remote_sys.mLight_mode = 0;
			}
		}
	}
	if(remote_sys.mRemote_CmdDelay)remote_sys.mRemote_CmdDelay--;
	if(remote_sys.mRemote_StateTime)
	{
		remote_sys.mRemote_StateTime--;
		if(remote_sys.mRemote_StateTime == 0)remote_sys.mRemote_OneKeySence = 0;
	}
	if(remote_sys.mRemote_KeyTime)remote_sys.mRemote_KeyTime--;
	if(remote_sys.mLight_TestTimer)remote_sys.mLight_TestTimer--;
	if(mRemote_timer){
		mRemote_timer--;
		if(mRemote_timer == 0)mRemote_idcnt = 0;
	}
	if(remote_sys.mLight_TimerOff)
	{
		remote_sys.mLight_TimerOff--;
		if(remote_sys.mLight_TimerOff == 0)
		{
			if(lewyfan_infor.light_sw)
			{
				lewyfan_infor.light_sw      = 0;
				light_driver_fade(0,
								  lewyfan_infor.light_templatetrue,
								  lewyfan_infor.light_rcolor,
								  lewyfan_infor.light_gcolor,
								  lewyfan_infor.light_bcolor);
				remote_sys.mLight_mode      = M_LIGHT_OFF;
				remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
				remote_sys.mRemote_SaveUp  = 1;
			}
		}
	}
		//上报分开
	if(remote_sys.mUpdateTimer)
	{
		remote_sys.mUpdateTimer--;
		if(remote_sys.mUpdateTimer == 0)
		{
			light_remote_update();
		}
	}
}


/*
 * 上电状态
 */
void light_remote_taskpoweron(void)
{
	uint8_t cmd;
	//上电对码和分段时间
	if(remote_sys.mLight_matchtimer)
	{
		if(--remote_sys.mLight_matchtimer == 0)
		{
			light_remote_externpoweron(1);
		}
	}

	if(remote_sys.mLight_powertim)
	{
		if(--remote_sys.mLight_powertim == 0)
		{
			light_remote_externpoweron(0);
		}
	}



	if (remote_sys.mRemote_message == 0)return; //接收到开灯信息

	cmd = mRemote_rxpack.Cmd;
	remote_sys.mRemote_CmdMode = 0; //清除调光
	switch (cmd)
	{
#if(LIGHT_MATCH_CMDON)
		case LCMD_ON:
			if (remote_sys.mRemote_match)
			{
				if(lewyfan_infor.light_sw == 0)
				{
					lewyfan_infor.light_sw = 1;
					light_driver_fade(lewyfan_infor.light_brightness,
									  lewyfan_infor.light_templatetrue,
									  lewyfan_infor.light_rcolor,
									  lewyfan_infor.light_gcolor,
									  lewyfan_infor.light_bcolor);
				}
				light_remote_clear(0);
			}
			else
			{
				light_remote_match(0);
			}
			break;
		case LCMD_OFF:
			if(lewyfan_infor.light_sw)
			{
				lewyfan_infor.light_sw = 0;
				light_driver_fade(0,
								  lewyfan_infor.light_templatetrue,
								  lewyfan_infor.light_rcolor,
								  lewyfan_infor.light_gcolor,
								  lewyfan_infor.light_bcolor);
			}
			light_remote_clear(0);
			break;
#endif
#if(LIGHT_MATCH_CMD24G)
		case LCMD_24G_MATCH://开机使用区域开按键连续3次开对码
			if (remote_sys.mRemote_match)//已经绑定还进入这个---无源清码
			{
				light_remote_clear(1);
			}else{//对码
				light_remote_match(1);
			}
			break;
#endif
		default:
			break;
	}

	remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
	remote_sys.mRemote_message = 0;
	remote_sys.mRemote_SaveUp  = 1;
}

/*
 * 开灯状态
 */
void light_remote_taskon(void)
{
	uint8_t cmd,scenesel;

	if (remote_sys.mRemote_message == 0)return; //接收到开灯信息
	cmd = mRemote_rxpack.Cmd;
	remote_sys.mRemote_CmdMode = 0; //清除调光
#if(LIGHT_WY_PWMON == 0 || LIGHT_RGB_PWMON == 1)//RGB功能
	if(lewyfan_infor.light_rgbmode == LIGHT_CD_WEAKUP ||
	   lewyfan_infor.light_rgbmode == LIGHT_CD_DUSK ||
	   lewyfan_infor.light_rgbmode == LIGHT_CD_SLEEP)
	{
		lewyfan_infor.light_rgbmode = LIGHT_RGB_NORMAL;
	}
#else
	if(lewyfan_infor.light_rgbmode == LIGHT_CD_WEAKUP ||
	   lewyfan_infor.light_rgbmode == LIGHT_CD_DUSK ||
	   lewyfan_infor.light_rgbmode == LIGHT_CD_SLEEP)
	{
		lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
	}
#endif
	switch (cmd)
	{
		case LCMD_LOCK://0-解锁，1-锁定
			if(mRemote_rxpack.Pter[REMOTE_SCENESEL] == 0)
			{
				remote_sys.mLight_flashcnt += SHOW_CNT2;
				lewyfan_infor.lock_state  &= (~APP_LOCK_REMOTE);
			}else
			if(mRemote_rxpack.Pter[REMOTE_SCENESEL] == 1)
			{
				remote_sys.mLight_flashcnt += SHOW_CNT1;
				lewyfan_infor.lock_state   |= (APP_LOCK_REMOTE);
			}
			break;
		case LCMD_DELELAMP:
			light_remote_clear(1);
			break;
		case LCMD_TEST://产测命令
			if(remote_sys.mLight_TestTimer)
			{
				lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MID;
				remote_sys.mLight_test      = 1;
				remote_sys.mLight_flashcnt += SHOW_CNT3;
			}
			break;
		case LCMD_RESET_GETEWAY://重置网络信息
			scenesel = mRemote_rxpack.Pter[REMOTE_SCENESEL];
			if(scenesel == 0 || scenesel == 1)//0-清除所有信息 1-清除网络信息 2-清除网关信息
			{
				mReset = WIFI_RESET_POWER;
				light_ResetData(scenesel);
				remote_sys.mLight_fadecnt = SHOW_CNT3;
			}
			break;
		case LCMD_KEYSCENE: //一键情景-使用
			scenesel = mRemote_rxpack.Pter[REMOTE_SCENESEL];
			if ((scenesel < LIGHT_SCENENUMBER) && (lewyfan_infor.lock_state  & (APP_LOCK_REMOTE)) == 0)
			{
				if(mRemote_rxpack.Len > 1 && mRemote_rxpack.Pter[REMOTE_SCENESEL + 1] == 0)
					light_scne_read(mRemote_rxpack.LampID,mRemote_rxpack.RemoteID,scenesel,0);
				else
					light_scne_read(mRemote_rxpack.LampID,mRemote_rxpack.RemoteID,scenesel,1);
			}
#if(LIGHT_REMOTE_NETSWITCH)
			light_remote_updateCmd(LCMD_KEYSCENE,mRemote_rxpack.Pter[0],mRemote_rxpack.Pter[1],mRemote_rxpack.Pter[2]);
#endif
#if(LIGHT_NIGHT_SWITCH)
			if(remote_sys.mLight_night)remote_sys.mLight_night = 0;
#endif
			break;
		case LCMD_SETSCENE: //一键情景-保存
			scenesel = mRemote_rxpack.Pter[REMOTE_SCENESEL];
			if (scenesel < LIGHT_SCENENUMBER)
			{
				light_scne_save(mRemote_rxpack.LampID,mRemote_rxpack.RemoteID,scenesel);
				remote_sys.mLight_flashcnt += SHOW_CNT1;
			}
#if(LIGHT_REMOTE_NETSWITCH)
			light_remote_updateCmd(LCMD_SETSCENE,mRemote_rxpack.Pter[0],mRemote_rxpack.Pter[1],mRemote_rxpack.Pter[2]);
#endif
			break;

		case LCMD_OFF://关灯键
#if(LIGHT_REMOTE_NETSWITCH)
			light_remote_updateCmd(LCMD_OFF,mRemote_rxpack.Pter[0],mRemote_rxpack.Pter[1],mRemote_rxpack.Pter[2]);
#endif
			lewyfan_infor.light_sw      = 0;
			light_driver_fade(0,
							  lewyfan_infor.light_templatetrue,
							  lewyfan_infor.light_rcolor,
							  lewyfan_infor.light_gcolor,
							  lewyfan_infor.light_bcolor);
			remote_sys.mLight_mode      = M_LIGHT_OFF;
			break;
		case LCMD_KENUP://华艺特有处理
			if (remote_sys.mRemote_StateTime)
			{
				if(remote_sys.mRemote_KeyTime > 0)
				{
					if(remote_sys.mRemote_UpOff)break;//若属于开的松开不处理
					remote_sys.mRemote_OneKeySence ++;
				}

				remote_sys.mRemote_OneKeySence &= 0x03;

				remote_sys.mLight_mode = M_LIGHT_ON;
				lewyfan_infor.light_sw = 1;

				if (remote_sys.mRemote_OneKeySence == 3)
				{
					lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MIN;
					lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MID;
				} else {
					lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MAX;
					if(remote_sys.mRemote_OneKeySence == 0)
						lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MAX;
					else
					if(remote_sys.mRemote_OneKeySence == 1)
						lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
					else
						lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MID;
				}
				//执行开灯
				if(remote_sys.mLight_fadecnt == 0 && remote_sys.mLight_flashcnt == 0)
				{
					light_driver_fade(lewyfan_infor.light_brightness,
									  lewyfan_infor.light_templatetrue,
									  lewyfan_infor.light_rcolor,
									  lewyfan_infor.light_gcolor,
									  lewyfan_infor.light_bcolor);
				}
			} else {
				//单键模式下关灯 大于4s  关灯
				if(remote_sys.mLight_fadecnt == 0 && remote_sys.mLight_flashcnt == 0)
				{
					lewyfan_infor.light_sw      = 0;
					light_driver_fade(0,
									  lewyfan_infor.light_templatetrue,
									  lewyfan_infor.light_rcolor,
									  lewyfan_infor.light_gcolor,
									  lewyfan_infor.light_bcolor);
					remote_sys.mLight_mode      = M_LIGHT_OFF;
				}
			}
			remote_sys.mRemote_StateTime = LIGHT_REMOTE_ONEKEY_SWITCH_TIME;
			break;
		case LCMD_ON: //开灯-关灯参数  随意开关特殊处理
			if (mRemote_rxpack.Pter[REMOTE_PERON] == 0 || mRemote_rxpack.Pter[REMOTE_PERON] == 2)
			{
				if(remote_sys.mLight_fadecnt == 0 && remote_sys.mLight_flashcnt == 0)
				{
					lewyfan_infor.light_sw      = 0;
					light_driver_fade(0,
									  lewyfan_infor.light_templatetrue,
									  lewyfan_infor.light_rcolor,
									  lewyfan_infor.light_gcolor,
									  lewyfan_infor.light_bcolor);
					remote_sys.mLight_mode      = M_LIGHT_OFF;
		#if(LIGHT_REMOTE_NETSWITCH)
					light_remote_updateCmd(LCMD_OFF,mRemote_rxpack.Pter[0],mRemote_rxpack.Pter[1],mRemote_rxpack.Pter[2]);
		#endif
				}
			}
			else
			if (mRemote_rxpack.Pter[REMOTE_PERON] != 0)
			{
	#if(LIGHT_REMOTE_NETSWITCH)
				light_remote_updateCmd(LCMD_ON,mRemote_rxpack.Pter[0],mRemote_rxpack.Pter[1],mRemote_rxpack.Pter[2]);
	#endif
				//单键无源开关特别处理
				if (mRemote_rxpack.Pter[REMOTE_PERON] >= PASSIVESW_ONDB)
				{
					remote_sys.mRemote_UpOff   = 0;
					remote_sys.mRemote_KeyTime = LIGHT_REMOTE_ONEKEY_TIME;

#if(HUAYI_WY_TYPE >= LESHI_WY_TYPE)
					//单键 切换
					if (remote_sys.mRemote_StateTime)
					{
			#if(LIGHT_NIGHT_SWITCH)
						if(remote_sys.mLight_night)remote_sys.mLight_night = 0;
			#endif
						//小于4s 开灯-同时切换状态
						remote_sys.mRemote_OneKeySence++;
						remote_sys.mRemote_OneKeySence &= 0x03;

						if (remote_sys.mRemote_OneKeySence == 3)
						{
							lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MIN;
							lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MID;
						} else {
							lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MAX;
							if(remote_sys.mRemote_OneKeySence == 0)
								lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MAX;
							else
							if(remote_sys.mRemote_OneKeySence == 1)
								lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
							else
								lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MID;
						}
					}
					else
					{
						if (mRemote_rxpack.Pter[REMOTE_PERON] >= PASSIVESW_ONSG)
						{
							remote_sys.mRemote_OneKeySence = 0;
							lewyfan_infor.light_sw = 0;
							remote_sys.mLight_mode = M_LIGHT_OFF;
						}
					}
					remote_sys.mRemote_StateTime = LIGHT_REMOTE_ONEKEY_SWITCH_TIME;
#endif
					light_driver_fade(lewyfan_infor.light_brightness,
									  lewyfan_infor.light_templatetrue,
									  lewyfan_infor.light_rcolor,
									  lewyfan_infor.light_gcolor,
									  lewyfan_infor.light_bcolor);
				}
				else
				{
					if(remote_sys.mLight_fadecnt == 0 && remote_sys.mLight_flashcnt == 0)
					{
						if(lewyfan_infor.light_sw == 0)
						{
							lewyfan_infor.light_sw = 1;
							light_driver_fade(lewyfan_infor.light_brightness,
											  lewyfan_infor.light_templatetrue,
											  lewyfan_infor.light_rcolor,
											  lewyfan_infor.light_gcolor,
											  lewyfan_infor.light_bcolor);
						}
					}
				}
			}
			break;
		default://开灯部分
#if(LIGHT_NIGHT_SWITCH)
			if(remote_sys.mLight_night)
			{
				unsigned char savesw;
				savesw = lewyfan_infor.light_sw;
				arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
				lewyfan_infor.light_sw = savesw;
				remote_sys.mLight_night = 0;
			}
#endif
			light_remote_onhandler(cmd);
			break;
	}

	remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
	remote_sys.mRemote_message = 0;
	remote_sys.mRemote_SaveUp  = 1;
}

/*
 * 关灯状态
 */
void light_remote_taskoff(void)
{
	uint8_t cmd,scenesel;

	if (remote_sys.mRemote_message == 0)return; //接收到开灯信息
	cmd = mRemote_rxpack.Cmd;
	remote_sys.mRemote_CmdMode = 0; //清除调光
#if(LIGHT_WY_PWMON == 0 || LIGHT_RGB_PWMON == 1)//RGB功能
	if(lewyfan_infor.light_rgbmode == LIGHT_CD_WEAKUP ||
	   lewyfan_infor.light_rgbmode == LIGHT_CD_DUSK ||
	   lewyfan_infor.light_rgbmode == LIGHT_CD_SLEEP)
	{
		lewyfan_infor.light_rgbmode = LIGHT_RGB_NORMAL;
	}
#else
	if(lewyfan_infor.light_rgbmode == LIGHT_CD_WEAKUP ||
	   lewyfan_infor.light_rgbmode == LIGHT_CD_DUSK ||
	   lewyfan_infor.light_rgbmode == LIGHT_CD_SLEEP)
	{
		lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
	}
#endif
	switch (cmd)
	{
		case LCMD_LOCK://0-解锁，1-锁定
			if(mRemote_rxpack.Pter[REMOTE_SCENESEL] == 0)
			{
				remote_sys.mLight_flashcnt += SHOW_CNT2;
				lewyfan_infor.lock_state  &= (~APP_LOCK_REMOTE);
			}else
			if(mRemote_rxpack.Pter[REMOTE_SCENESEL] == 1)
			{
				remote_sys.mLight_flashcnt += SHOW_CNT1;
				lewyfan_infor.lock_state   |= (APP_LOCK_REMOTE);
			}
			break;
		case LCMD_DELELAMP:
			light_remote_clear(1);
			break;
		case LCMD_TEST://产测命令
			if(remote_sys.mLight_TestTimer)
			{
				lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MID;
				remote_sys.mLight_mode      = M_LIGHT_ON;
				lewyfan_infor.light_sw      = 1;
				remote_sys.mLight_test      = 1;
				remote_sys.mLight_flashcnt += SHOW_CNT3;
			}
			break;
		case LCMD_RESET_GETEWAY://重置网络信息
			scenesel = mRemote_rxpack.Pter[REMOTE_SCENESEL];
			if(scenesel == 0 || scenesel == 1)//0-清除所有信息 1-清除网络信息 2-清除网关信息
			{
				mReset = WIFI_RESET_POWER;
				light_ResetData(scenesel);
				remote_sys.mLight_fadecnt = SHOW_CNT3;
			}
			break;
		case LCMD_KEYSCENE: //一键情景-使用
			scenesel = mRemote_rxpack.Pter[REMOTE_SCENESEL];
			if ((scenesel < LIGHT_SCENENUMBER)  && (lewyfan_infor.lock_state  & (APP_LOCK_REMOTE)) == 0)
			{
				if(mRemote_rxpack.Len > 1 && mRemote_rxpack.Pter[REMOTE_SCENESEL + 1] == 0)
					light_scne_read(mRemote_rxpack.LampID,mRemote_rxpack.RemoteID,scenesel,0);
				else
					light_scne_read(mRemote_rxpack.LampID,mRemote_rxpack.RemoteID,scenesel,1);
			}
#if(LIGHT_REMOTE_NETSWITCH)
			light_remote_updateCmd(LCMD_KEYSCENE,mRemote_rxpack.Pter[0],mRemote_rxpack.Pter[1],mRemote_rxpack.Pter[2]);
#endif
#if(LIGHT_NIGHT_SWITCH)
			if(remote_sys.mLight_night)remote_sys.mLight_night = 0;
#endif
			break;
		case LCMD_SETSCENE: //一键情景-保存
			scenesel = mRemote_rxpack.Pter[REMOTE_SCENESEL];
			if (scenesel < LIGHT_SCENENUMBER)
			{
				light_scne_save(mRemote_rxpack.LampID,mRemote_rxpack.RemoteID,scenesel);
				remote_sys.mLight_flashcnt += SHOW_CNT1;
			}
#if(LIGHT_REMOTE_NETSWITCH)
			light_remote_updateCmd(LCMD_SETSCENE,mRemote_rxpack.Pter[0],mRemote_rxpack.Pter[1],mRemote_rxpack.Pter[2]);
#endif
			break;
		case LCMD_NIGHT:
		case LCMD_LOFF:
#if(LIGHT_NIGHT_SWITCH)
			if(remote_sys.mLight_night)remote_sys.mLight_night = 0;
#endif
#if(LIGHT_REMOTE_NETSWITCH)
			light_remote_updateCmd(cmd,mRemote_rxpack.Pter[0],mRemote_rxpack.Pter[1],mRemote_rxpack.Pter[2]);
#endif
			remote_sys.mLight_mode = M_LIGHT_ON;
			lewyfan_infor.light_sw = 1;
	#if(HUAYI_WY_TYPE >= LESHI_WY_TYPE && HUAYI_WY_TYPE < LEISHI_RGB_TYPE_1)
	#else
			lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;	// 色温
	#endif

			lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MIN;//LAMP_BRIGHTNESSNIGHT;  亮度
			light_driver_fade(lewyfan_infor.light_brightness,
							  lewyfan_infor.light_templatetrue,
							  lewyfan_infor.light_rcolor,
							  lewyfan_infor.light_gcolor,
							  lewyfan_infor.light_bcolor);
			break;
		case LCMD_ON: //开灯-关灯参数  随意开关特殊处理
			if (mRemote_rxpack.Pter[REMOTE_PERON] != 0)
			{
#if(LIGHT_REMOTE_NETSWITCH)
				light_remote_updateCmd(LCMD_ON,mRemote_rxpack.Pter[0],mRemote_rxpack.Pter[1],mRemote_rxpack.Pter[2]);
#endif
				remote_sys.mLight_mode = M_LIGHT_ON;
				lewyfan_infor.light_sw = 1;

				if (mRemote_rxpack.Pter[REMOTE_PERON] >= PASSIVESW_ONDB)
				{
					//单键 切换
					if (remote_sys.mRemote_StateTime)
					{
			#if(LIGHT_NIGHT_SWITCH)
						if(remote_sys.mLight_night)remote_sys.mLight_night = 0;
			#endif
						//小于4s 开灯-同时切换状态
						remote_sys.mRemote_OneKeySence++;
						remote_sys.mRemote_OneKeySence &= 0x03;

						if (remote_sys.mRemote_OneKeySence == 3)
						{
							lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MIN;
							lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MID;
						} else {
							lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MAX;
							if(remote_sys.mRemote_OneKeySence == 0)
								lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MAX;
							else
							if(remote_sys.mRemote_OneKeySence == 1)
								lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
							else
								lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MID;
						}
					}
					remote_sys.mRemote_UpOff     = 1;
					remote_sys.mRemote_KeyTime   = LIGHT_REMOTE_ONEKEY_TIME;
					remote_sys.mRemote_StateTime = LIGHT_REMOTE_ONEKEY_SWITCH_TIME;
				}
#if(LIGHT_NIGHT_SWITCH)
				if(remote_sys.mLight_night)
				{
					lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MIN;
					lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
					remote_sys.mRemote_SaveUp   = 1;
				#if(LIGHT_WY_PWMON == 1 && LIGHT_RGB_PWMON == 0)//WY
					lewyfan_infor.light_rgbmode = LIGHT_CD_NIGHT;
				#endif
				}
#endif
				light_driver_fade(lewyfan_infor.light_brightness,
								  lewyfan_infor.light_templatetrue,
								  lewyfan_infor.light_rcolor,
								  lewyfan_infor.light_gcolor,
								  lewyfan_infor.light_bcolor);
			}else{
#if(LIGHT_REMOTE_NETSWITCH)
			light_remote_updateCmd(LCMD_OFF,mRemote_rxpack.Pter[0],mRemote_rxpack.Pter[1],mRemote_rxpack.Pter[2]);
#endif
			}
			break;
		default:
#if(LIGHT_REMOTE_NETSWITCH)
			light_remote_updateCmd(cmd,mRemote_rxpack.Pter[0],mRemote_rxpack.Pter[1],mRemote_rxpack.Pter[2]);
#endif
			break;
	}

	remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER;//有命令则延迟保存时间
	remote_sys.mRemote_message = 0;
	remote_sys.mRemote_SaveUp  = 1;
}



/************************************************************
Description: Lamp ON message handler
Parameters: cmd
Return: None
Other: None
************************************************************/
void light_remote_onhandler(uint8_t cmd)
{
	uint8_t  colorsel;
	uint8_t  wcolor, ycolor;
	uint32_t templatetrue,brightness;


#if(LIGHT_REMOTE_NETSWITCH)
	light_remote_updateCmd(cmd,mRemote_rxpack.Pter[0],mRemote_rxpack.Pter[1],mRemote_rxpack.Pter[2]);
#endif
	switch (cmd)
	{
		case LCMD_TIME_OFF:
			remote_sys.mLight_TimerOff   = mRemote_rxpack.Pter[0];
			remote_sys.mLight_TimerOff <<= 8;
			remote_sys.mLight_TimerOff  |= mRemote_rxpack.Pter[1];
			remote_sys.mLight_flashcnt += SHOW_CNT1;
			remote_sys.mLight_TimerOff *= (1000/LIGNT_TIMER_TICK);
			return;
		case LCMD_LON:
			lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX;
			break;
		case LCMD_ONEBRIGHTNESS:
			if(mRemote_rxpack.Pter[REMOTE_COLORSEL] == 1)
			{
				remote_sys.mRemote_CmdMode  = cmd;
				remote_sys.mRemote_CycTimer = LIGHT_REMOTE_CYCTIMER;
			}
			else{
				remote_sys.mRemote_CmdMode   = 0;
			}

			break;
		case LCMD_NIGHT:
		case LCMD_LOFF:
#if(HUAYI_WY_TYPE >= LESHI_WY_TYPE && HUAYI_WY_TYPE < LEISHI_RGB_TYPE_1)
#else
			lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
#endif
			lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MIN;//LAMP_BRIGHTNESSNIGHT;
			break;
#if(LIGHT_WY_PWMON)
		case LCMD_COL_ADD://色温调节
		case LCMD_COL_DEC://色温调节
			lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
			/* no break */
#endif
			/* no break */
		case LCMD_CON_ADD://灯亮度调节
		case LCMD_CON_DEC://灯亮度调节
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
			remote_sys.mRemote_CmdMode = cmd;
			return;

#if(LIGHT_WY_PWMON)
  #if(HUAYI_WY_TYPE >= LESHI_WY_TYPE)
  // 乐式默认色温切换 中性光-白光-黄光
		case LCMD_COLORSEL://色温档位选择				(2019/11/7 修复色温的错误 注释的为乐式标准)
			colorsel = mRemote_rxpack.Pter[REMOTE_COLORSEL];
			lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
			if (colorsel < REMOTE_COLORGREA)
			{
#if(HUAYI_WY_TYPE >= LEISHI_RGB_TYPE_1)
				lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MAX;
#endif
				if(colorsel == 0)lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MID;
				else
				if(colorsel == 1)lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MAX;
				else			 lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
			}else{
				lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MAX>>1;
				lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MID;
			}
			break;
  #else//华艺 白光-黄光-中性光
		case LCMD_COLORSEL://色温档位选择
			lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
			colorsel = mRemote_rxpack.Pter[REMOTE_COLORSEL];
			if (colorsel < REMOTE_COLORGREA)
			{
				lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MAX;
				if(colorsel == 0)lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MAX;	// LIGHT_TEMPLATETRUE_MID
				else
				if(colorsel == 1)lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;	// LIGHT_TEMPLATETRUE_MAX
				else			 lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MID;	// LIGHT_TEMPLATETRUE_MIN
			}else{
				lewyfan_infor.light_brightness   = LIGHT_BRIGHTNESS_MAX>>1;
				lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MID;
			}
			break;
  #endif
		case LCMD_COLORK://色温设置
			lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
			wcolor = mRemote_rxpack.Pter[REMOTE_WCOLOR];
			ycolor = mRemote_rxpack.Pter[REMOTE_YCOLOR];
			if ((wcolor + ycolor) >= REMOTE_MAXCOLOR)
			{
				if(wcolor >= REMOTE_MAXCOLOR)
				{
					if(ycolor > REMOTE_MAXCOLOR)ycolor = REMOTE_MAXCOLOR;
					templatetrue  = ((LIGHT_TEMPLATETRUE_MAX-LIGHT_TEMPLATETRUE_MID)*(REMOTE_MAXCOLOR-ycolor))/REMOTE_MAXCOLOR;
					templatetrue += LIGHT_TEMPLATETRUE_MID;
					lewyfan_infor.light_templatetrue = templatetrue;
				}else{
					templatetrue  = ((LIGHT_TEMPLATETRUE_MID-LIGHT_TEMPLATETRUE_MIN)*(REMOTE_MAXCOLOR-wcolor))/REMOTE_MAXCOLOR;
					templatetrue  = LIGHT_TEMPLATETRUE_MID - templatetrue;
					lewyfan_infor.light_templatetrue = templatetrue;
				}
			}
			break;
#endif
		case LCMD_BRIGHTNESS://亮度设置
			brightness   = mRemote_rxpack.Pter[REMOTE_WCOLOR];
			brightness <<= 8;
			brightness  |= mRemote_rxpack.Pter[REMOTE_YCOLOR];

			if (brightness < LIGHT_BRIGHTNESS_MIN)
			{
				brightness = LIGHT_BRIGHTNESS_MIN;
			}
			else if (brightness > LIGHT_BRIGHTNESS_MAX)
			{
				brightness = LIGHT_BRIGHTNESS_MAX;
			}

			lewyfan_infor.light_brightness = brightness;
			break;
		case LCMD_BRMODE://
#if(HUAYI_WY_TYPE == HUAYI_WY_TYPE_GENERAL || HUAYI_WY_TYPE == HUAYI_WY_TYPE_AQUARIUS)
#define LCMD_BRMODE_0	(LIGHT_BRIGHTNESS_MIN)			// 1%
#define LCMD_BRMODE_1	(LIGHT_BRIGHTNESS_MAX*2/10)		// 20%
#define LCMD_BRMODE_2	(LIGHT_BRIGHTNESS_MAX>>1)		// 50%
#define LCMD_BRMODE_3	(LIGHT_BRIGHTNESS_MAX)			// 100%
#else// 100%->70%->50->1%
#define LCMD_BRMODE_0	(LIGHT_BRIGHTNESS_MIN)			// 1%
#define LCMD_BRMODE_1	(LIGHT_BRIGHTNESS_MAX>>1)		// 50%
#define LCMD_BRMODE_2	(LIGHT_BRIGHTNESS_MAX*7/10)		// 70%
#define LCMD_BRMODE_3	(LIGHT_BRIGHTNESS_MAX)			// 100%
#endif
			colorsel = mRemote_rxpack.Pter[REMOTE_COLORSEL];
			if(colorsel == 1)//档位+
			{
				if(lewyfan_infor.light_brightness < LCMD_BRMODE_3)
				{
					if(lewyfan_infor.light_brightness < LCMD_BRMODE_2)
					{
						if(lewyfan_infor.light_brightness < LCMD_BRMODE_1)
						{
							lewyfan_infor.light_brightness = LCMD_BRMODE_1;
						}
						else
						{
							lewyfan_infor.light_brightness = LCMD_BRMODE_2;
						}
					}
					else
					{
						lewyfan_infor.light_brightness = LCMD_BRMODE_3;
					}
				}
			}
			else
			if(colorsel == 2)
			{
				if(lewyfan_infor.light_brightness > LCMD_BRMODE_0)
				{
					if(lewyfan_infor.light_brightness > LCMD_BRMODE_1)
					{
						if(lewyfan_infor.light_brightness > LCMD_BRMODE_2)
						{
							lewyfan_infor.light_brightness = LCMD_BRMODE_2;
						}
						else
						{
							lewyfan_infor.light_brightness = LCMD_BRMODE_1;
						}
					}
					else//
					{
						lewyfan_infor.light_brightness = LCMD_BRMODE_0;
					}
				}
			}
			break;
#if(LIGHT_RGB_PWMON)
		case LCMD_RGB_COLORK://RGB设置
			lewyfan_infor.light_rgbmode = LIGHT_RGB_NORMAL;
			if ((mRemote_rxpack.Pter[0] + mRemote_rxpack.Pter[1] + mRemote_rxpack.Pter[2]) >= 255)
			{
				lewyfan_infor.light_rcolor = mRemote_rxpack.Pter[0];
				lewyfan_infor.light_gcolor = mRemote_rxpack.Pter[1];
				lewyfan_infor.light_bcolor = mRemote_rxpack.Pter[2];
			}
			break;

		case LCMD_RGBINIT://RGB重置
			lewyfan_infor.light_rgbmode = LIGHT_RGB_NORMAL;
			lewyfan_infor.light_rcolor = 255;
			lewyfan_infor.light_gcolor = 0;
			lewyfan_infor.light_bcolor = 0;
			break;
		case LCMD_RGB_CYCMODEF: //RGB呼吸模式
			if(mRemote_rxpack.Pter[0])
			{
				lewyfan_infor.light_rgbmode = LIGHT_RGB_FADE;
				return;
			}
			else
				lewyfan_infor.light_rgbmode = LIGHT_RGB_NORMAL;
			break;

		case LCMD_RGB_JUMPMODE:
			if(mRemote_rxpack.Pter[0])
			{
				lewyfan_infor.light_rgbmode = LIGHT_RGB_JUMP;
				return;
			}
			else
			lewyfan_infor.light_rgbmode = LIGHT_RGB_NORMAL;
			break;
#endif
		default:
			return;
	}

	if(remote_sys.mLight_fadecnt == 0 && remote_sys.mLight_flashcnt == 0)
	{
		light_driver_fade(lewyfan_infor.light_brightness,
						  lewyfan_infor.light_templatetrue,
						  lewyfan_infor.light_rcolor,
						  lewyfan_infor.light_gcolor,
						  lewyfan_infor.light_bcolor);
	}
}

/*
 * 遥控包处理
 */
void light_remote_CheckMessage(void)
{
	RemoteType pack;
	uint8_t i,key_value = 0;

	if (device_remote_rxcnt == 0)return;
	device_remote_rxcnt = 0;

	pack = device_remote_rxpack;

	////arch_printf("light_remote  Cmd = %d\r\n",pack.Cmd);

	if (pack.Head.mType == PASSIVESW_HEAD_T)//无源开关帧-组网类型
	{
		return;
	}

	if (pack.Head.mType == HEADER_PASSSW_NONET) //无源开关帧-遙控類型
	{
		pack.Head.mType = REMOTE_HEAD_T;
		pack.Head.mVer  = REMOTE_HEAD_V;
		if (pack.pSubType >= PASSIVESW_G1) //单侧无源开关-使用单键对码与清码-特别处理
		{
			pack.Pter[0] = PASSIVESW_ONSG; //开灯键作为对码与清码功能
		}
		else
		{
			pack.Pter[0] = PASSIVESW_ONDB; //开灯键作为对码与清码功能
		}
		pack.Type = LETS_OEM_PASSIVE_SW;
		key_value = pack.pPter[PASSIVESW_PKEY];
		pack.LampID = key_value & PASSIVESW_VKEY; //组
		if (key_value & PASSIVESW_VIO) //开杆--I侧与O侧
		{
			pack.Cmd = LCMD_ON;
		}
		else
		{
			pack.Cmd = LCMD_OFF;
		}
		pack.Len = 1;
		pack.PackId = mRemote_rxpack.PackId+1;
		//pack.Pter[pack.Len] = GetCRC(pack.pack, pack.Len + REMOTE_CLEN); //校验
		device_remote_rxpst = PASSIVESW_TIME;
	}

	//程序加入 产测遥控ID
	#if(LIGHT_REMOTE_TEST)
	if(pack.RemoteID == 0xFFFFFFFF){	// 测试专用ID
		remote_sys.mLight_mode = M_LIGHT_ON;
		mRemote_rxpack  = pack;
		remote_sys.mRemote_message = 1;
		remote_sys.mRemote_match = 9;
		return;
	}
	#endif

	remote_sys.mRemote_match = light_remote_Check(pack.RemoteID, pack.LampID,pack.Type);//检测接收的遥控ID是否已经绑定
	if (remote_sys.mRemote_match)
	{
		if(mRemote_idcnt)
		{
			for(i=0;i<mRemote_idcnt;i++)
			{
				if(mRemote_idbuf[i] == pack.RemoteID &&
				   mRemote_ldbuf[i] == pack.LampID &&
				   mRemote_pdbuf[i] == pack.PackId)
				{
					mRemote_timer = 200;//10ms unix
					//arch_printf("light_remote  last pack\r\n");
					return;
				}
			}
		}
		//转发
		if(mRemote_idcnt >= MREMOTE_CHECK_BUF_MAX)
		{
			for(i=0;i<(MREMOTE_CHECK_BUF_MAX-1);i++)
			{
				mRemote_idbuf[i] = mRemote_idbuf[i+1];
				mRemote_ldbuf[i] = mRemote_ldbuf[i+1];
				mRemote_pdbuf[i] = mRemote_pdbuf[i+1];
			}
			mRemote_idbuf[i] = pack.RemoteID;
			mRemote_ldbuf[i] = pack.LampID;
			mRemote_pdbuf[i] = pack.PackId;
		}
		else{
			mRemote_idbuf[mRemote_idcnt] = pack.RemoteID;
			mRemote_ldbuf[mRemote_idcnt] = pack.LampID;
			mRemote_pdbuf[mRemote_idcnt] = pack.PackId;
			mRemote_idcnt++;
		}
		////arch_printf("light_remote  RemoteID=%08x,LampID=%d,PackId=%d\r\n",pack.RemoteID,pack.LampID,pack.PackId);
		mRemote_timer = 200;//10ms unix
	}

	if (remote_sys.mLight_mode == M_LIGHT_PON)
	{
		if (remote_sys.mRemote_match)//检测到遥控器已经绑定
		{
#if(LIGHT_MATCH_CMDON && LIGHT_MATCH_CMD24G)
			if((pack.Cmd == LCMD_OFF) ||
			  ((pack.Cmd == LCMD_ON) && (pack.Pter[0] >= PASSIVESW_ONSG))||
			  ((pack.Cmd == LCMD_24G_MATCH) && (pack.Pter[0] == 2)))//进入解除配对
			{

			}
			else
			{
				light_remote_externpoweron(1);
			}
#else
#if(LIGHT_MATCH_CMDON)
			if((pack.Cmd == LCMD_OFF) ||\
			  ((pack.Cmd == LCMD_ON) && (pack.Pter[0] >= PASSIVESW_ONSG)))//进入解除配对
			{

			}
			else
			{
				light_remote_externpoweron(1);
			}
#endif
#if(LIGHT_MATCH_CMD24G)
			if((pack.Cmd == LCMD_24G_MATCH) && (pack.Pter[0] == 2))//进入解除配对
			{

			}
			else
			{
				light_remote_externpoweron(1);
			}
#endif
#endif
			mRemote_rxpack  = pack;
			remote_sys.mRemote_message = 1;
		}
		else//进入配对
		{
#if(LIGHT_MATCH_CMDON && LIGHT_MATCH_CMD24G)
			if((pack.Cmd == LCMD_ON) ||
			   (pack.Cmd == LCMD_24G_MATCH && pack.Pter[0] == 1))//进入解除配对
			{
				mRemote_rxpack  = pack;
				remote_sys.mRemote_message = 1;
			}
#else
#if(LIGHT_MATCH_CMDON)
			if (pack.Cmd == LCMD_ON)//配对
			{
				mRemote_rxpack  = pack;
				remote_sys.mRemote_message = 1;
			}
#endif
#if(LIGHT_MATCH_CMD24G)
			if (pack.Cmd == LCMD_24G_MATCH && pack.Pter[0] == 1)//配对
			{
				mRemote_rxpack  = pack;
				remote_sys.mRemote_message = 1;
			}
#endif
#endif
		}
	}
	else
	{
		if (remote_sys.mRemote_match)//检测到遥控器已经绑定
		{
			mRemote_rxpack  = pack;
			remote_sys.mRemote_message = 1;
		}
	}
}


/************************************************************
 Description: Remote Controller information save
 Parameters: Remote ID and lamp
 Return: 0 - Not match the remote control
 1 - Match the remote control
 2 - Match the remote control, But no Lamp ID
 Other: None
 ************************************************************/
uint8_t light_remote_Check(uint32_t remote, uint8_t lamp,uint8_t type)
{
	uint8_t  j;
	uint32_t remoteid;
	uint8_t  rlampid;


	if(type == 0xFD)//APP直连
	{
		j = LIGHT_REMOTE_MATCHMAX;
		if(remote_array[j].use)
		{
			remoteid = remote_array[j].rid;
			rlampid  = remote_array[j].lid;

			if (remoteid == remote)
			{
				if (lamp == LIGHT_REMOTE_ALL || lamp == rlampid)
				{
					return j+1;
				}
			}
		}
	}
	else
	{
		for (j = 0; j < LIGHT_REMOTE_MATCHMAX; j++)
		{
			if(remote_array[j].use)
			{
				remoteid = remote_array[j].rid;
				rlampid  = remote_array[j].lid;

				if (remoteid == remote)
				{
					if (lamp == LIGHT_REMOTE_ALL || lamp == rlampid)
					{
						return j+1;
					}
				}
			}
		}
	}
	return 0;
}

/*
 * light_remote_externpoweron
 * */
void light_remote_externpoweron(uint8_t type)
{
	if(type)
	{
		if(lewyfan_infor.light_sw)remote_sys.mLight_mode = M_LIGHT_ON;
		else					  remote_sys.mLight_mode = M_LIGHT_OFF;
	}
	if(remote_sys.mLight_powercnt)
	{
		remote_sys.mLight_powercnt = 0;
		arch_psm_set_value(LIGHT_SAVE_NAME, LIGHT_KEY_POWER, (void*)&remote_sys.mLight_powercnt, sizeof(remote_sys.mLight_powercnt));
	}

	// 关闭开关分段
	remote_sys.Other = 0;
	arch_psm_set_value(LIGHT_SAVE_NAME, LIGHT_KEY_PONSETBIT, (void*)&remote_sys.Other, sizeof(remote_sys.Other));


	LOG_INFO_TAG("POWER ON EXIT", "lewyfan_powercnt = %d...",remote_sys.mLight_powercnt);
}



/************************************************************
Description: Lamp power on Clear
Other: None
************************************************************/
static uint32_t bRemoteID = 0;
static uint32_t bRemotelID = 0;
uint8_t light_remote_clear(uint8_t one)
{
	uint8_t  i,ret=0;

	////arch_printf("light_remote_clear  mRemote_rxpack.RemoteID = %08x  \r\n",mRemote_rxpack.RemoteID);
	////arch_printf("light_remote_clear  remote_sys.mRemote_matchcnt = %d\r\n",remote_sys.mRemote_matchcnt);
	if(lewyfan_infor.lock_state  & APP_LOCK_REMOTE)return ret;
	if (bRemoteID == mRemote_rxpack.RemoteID && bRemotelID == mRemote_rxpack.LampID)
	{
		remote_sys.mRemote_clearcnt++;
	}
	else
	{
		remote_sys.mRemote_clearcnt = 1;
		bRemoteID  = mRemote_rxpack.RemoteID;
		bRemotelID = mRemote_rxpack.LampID;
	}

	if (remote_sys.mRemote_clearcnt >= 3 || one != 0)
	{
		remote_sys.mLight_powertim     = 0;
		remote_sys.mRemote_clearcnt    = 0;
		remote_sys.mLight_mode         = M_LIGHT_ON;
		lewyfan_infor.light_sw         = 1;
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX;
		remote_sys.mLight_fadecnt = SHOW_CNT3;

		if(mRemote_rxpack.Type == 0xFD)//APP直连
		{
			i = LIGHT_REMOTE_MATCHMAX;
			remote_array[i].use = 0;
		}
		else
		{
			for (i = 0; i < LIGHT_REMOTE_MATCHMAX; i++)
			{
	#if(LIGHT_MATCH_CLEARALL)
				remote_array[i].use = 0;
				remote_sys.mRemote_MacUpdate |= (0x01<<i);
	#else
				if(remote_array[i].use)
				{
					if (remote_array[i].rid == mRemote_rxpack.RemoteID)
					{
						if (mRemote_rxpack.LampID == LIGHT_REMOTE_ALL ||
							mRemote_rxpack.LampID == remote_array[i].lid)
						{
							break;
						}
					}
				}
	#endif
			}
#if(LIGHT_MATCH_CLEARALL)
			remote_array[LIGHT_REMOTE_MATCHMAX].use = 0;
#endif
		}

#if(LIGHT_MATCH_CLEARALL)
		arch_psm_set_value(LIGHT_SAVE_NAME, LIGHT_KEY_MATCH, (void*)&remote_array, sizeof(remote_array));
#else
		if(i < LIGHT_REMOTE_MATCHMAX)
		{
			remote_sys.mRemote_MacUpdate |= (0x01<<i);
			remote_array[i].use = 0;
			for (i = 0; i < LIGHT_REMOTE_MATCHMAX; i++)remote_array[i].num = 0;
			arch_psm_set_value(LIGHT_SAVE_NAME, LIGHT_KEY_MATCH, (void*)&remote_array, sizeof(remote_array));
		}
#endif
		ret = 1;
#if(LIGHT_MATCH_CLEARALL)
#else
		for(i=0; i<LIGHT_REMOTE_MATCHMAX; i++)
		{
			if(remote_array[i].use)
			{
				//arch_printf("remote rid = %08x, lid = %d\r\n",remote_array[i].rid,remote_array[i].lid);
			}
		}
#endif
#if(LIGHT_REMOTE_NETSWITCH)
		if(lewyfan_infor.remote_switch)
		{
			remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
			remote_sys.mRemote_SaveUp  = 1;
		}
#endif
	}
	return ret;
}

/************************************************************
Description: Lamp power on match
Other: None
************************************************************/
uint8_t light_remote_match(uint8_t one)
{
	uint8_t ret= 0;
	uint8_t i  = 0;
	uint8_t num= 0;

	////arch_printf("light_remote_match  mRemote_rxpack.RemoteID = %08x  \r\n",mRemote_rxpack.RemoteID);
	////arch_printf("light_remote_match  remote_sys.mRemote_matchcnt = %d\r\n",remote_sys.mRemote_matchcnt);
	if(lewyfan_infor.lock_state  & APP_LOCK_REMOTE)return ret;
	if (bRemoteID == mRemote_rxpack.RemoteID && bRemotelID == mRemote_rxpack.LampID)
	{
		remote_sys.mRemote_matchcnt++;
	}
	else
	{
		remote_sys.mRemote_matchcnt = 1;
		bRemoteID  = mRemote_rxpack.RemoteID;
		bRemotelID = mRemote_rxpack.LampID;
	}
	if(mRemote_rxpack.LampID == 0)mRemote_rxpack.LampID = 1;

	if (remote_sys.mRemote_matchcnt >= 3 || one != 0)
	{
		remote_sys.mLight_powertim     = 0;
		remote_sys.mRemote_matchcnt    = 0;
		remote_sys.mLight_mode         = M_LIGHT_ON;
		lewyfan_infor.light_sw         = 1;
		lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX;
		remote_sys.mLight_fadecnt = SHOW_CNT3;

		if(mRemote_rxpack.Type == 0xFD)//APP直连
		{
			i = LIGHT_REMOTE_MATCHMAX;
			remote_array[i].use = 1;
			remote_array[i].rid = mRemote_rxpack.RemoteID;
			remote_array[i].lid = mRemote_rxpack.LampID;
		}
		else
		{
			//查询空
			for(i=0; i<LIGHT_REMOTE_MATCHMAX; i++)
			{
				if(remote_array[i].use == 0)break;
			}
			if(i < LIGHT_REMOTE_MATCHMAX)
			{
				remote_array[i].use = 1;
				remote_array[i].rid = mRemote_rxpack.RemoteID;
				remote_array[i].lid = mRemote_rxpack.LampID;
				if(mRemote_rxpack.Len < 4)
					remote_array[i].type= 0;
				else
					remote_array[i].type= mRemote_rxpack.Pter[3];
				remote_array[i].num = 0;	//存储满后第一个覆盖编号
	#if(LIGHT_REMOTE_NETSWITCH)
				remote_sys.mRemote_MacUpdate |= (0x01<<i);
				if(lewyfan_infor.remote_switch)
				{
					remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
					remote_sys.mRemote_SaveUp  = 1;
				}
	#endif
			}else{
				num = remote_array[0].num;				//存储满了-覆盖编号
				if(num >= LIGHT_REMOTE_MATCHMAX)num = 0; //存储错误从0开始
				remote_array[num].use = 1;
				remote_array[num].rid = mRemote_rxpack.RemoteID;
				remote_array[num].lid = mRemote_rxpack.LampID;
				if(mRemote_rxpack.Len < 4)
				remote_array[num].type= 0;
				else
				remote_array[num].type= mRemote_rxpack.Pter[3];
				if(++num >= LIGHT_REMOTE_MATCHMAX)num=0;
				for(i=0; i<LIGHT_REMOTE_MATCHMAX; i++)remote_array[i].num = num;
	#if(LIGHT_REMOTE_NETSWITCH)
				remote_sys.mRemote_MacUpdate |= (0x01<<num);
				if(lewyfan_infor.remote_switch)
				{
					remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
					remote_sys.mRemote_SaveUp  = 1;
				}
	#endif
			}
		}
		arch_psm_set_value(LIGHT_SAVE_NAME, LIGHT_KEY_MATCH, (void*)&remote_array, sizeof(remote_array));
		ret = 1;

		for(i=0; i<LIGHT_REMOTE_MATCHMAX; i++)
		{
			if(remote_array[i].use)
			{
				//arch_printf("remote rid = %08x, lid = %d\r\n",remote_array[i].rid,remote_array[i].lid);
			}
		}
	}

	return ret;
}



/************************************************************
APP清除遥控器
************************************************************/
void light_remote_appset(uint32_t remoteid)
{
	uint32_t i,j,k;

	LOG_INFO("remoteid = %08x\r\n",remoteid);
	if(remoteid == 0)//clear all
	{
		remote_sys.mLight_fadecnt += SHOW_CNT1;
		for (i = 0; i < (LIGHT_REMOTE_MATCHMAX+1); i++)
		{
			if(remote_array[i].use)
			{
				remote_array[i].use = 0;
#if(LIGHT_REMOTE_NETSWITCH)
				remote_sys.mRemote_MacUpdate |= (0x01<<i);
				if(lewyfan_infor.remote_switch)
				{
					remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
					remote_sys.mRemote_SaveUp  = 1;
				}
#endif
			}
		}
		arch_psm_set_value(LIGHT_SAVE_NAME, LIGHT_KEY_MATCH, (void*)&remote_array, sizeof(remote_array));
	}
	else
	if(remoteid == 0xFFFFFFFF)//配对开始
	{
		lewyfan_infor.lock_state   &= (~APP_LOCK_REMOTE);
		remote_sys.mLight_fadecnt  += SHOW_CNT1;
		remote_sys.mLight_mode      = M_LIGHT_PON;
		remote_sys.mLight_matchtimer= LIGHT_MATCH_TIMER;	//
	}
	else//删除一个遥控器
	{
		for (i = 0; i < LIGHT_REMOTE_MATCHMAX;)
		{
			if(remote_array[i].use)
			{
				LOG_INFO("remote_array[%d] = %08x\r\n",i,remote_array[i].rid);
				if (remote_array[i].rid == remoteid)
				{
					if(i == (LIGHT_REMOTE_MATCHMAX-1))//最后一个
					{
						remote_sys.mRemote_MacUpdate |= (0x01<<i);
						remote_array[i].use = 0;
						LOG_INFO("Del i = %d\r\n",i);
					}
					else//不是最后一个
					{
						k = 0;
						for (j=i; j < (LIGHT_REMOTE_MATCHMAX-1); j++)
						{
							if(remote_array[j+1].use)
							{
								k++;//后面存在遥控器需要往前移动
								LOG_INFO("Del i = %d\r\n",i);
								remote_array[j].use = remote_array[j+1].use;
								remote_array[j].rid = remote_array[j+1].rid;
								remote_array[j].lid = remote_array[j+1].lid;
								remote_array[j].type= remote_array[j+1].type;
								remote_sys.mRemote_MacUpdate |= (0x01<<j);
								remote_sys.mRemote_MacUpdate |= (0x01<<(j+1));
								remote_array[j+1].use = 0;
							}
						}
						if(k == 0)////最后一个
						{
							remote_sys.mRemote_MacUpdate |= (0x01<<i);
							remote_array[i].use = 0;
							LOG_INFO("Del i = %d\r\n",i);
						}
						if(k)
						{
							continue;
						}
					}
				}
			}
			i++;
		}

#if(LIGHT_REMOTE_NETSWITCH)
		if(lewyfan_infor.remote_switch)
		{
			remote_sys.mRemote_stimer  = LIGHT_REMOTE_STIMER/3;//有命令则延迟保存时间
			remote_sys.mRemote_SaveUp  = 1;
		}
#endif
		for (i = 0; i < LIGHT_REMOTE_MATCHMAX; i++)remote_array[i].num = 0;
		arch_psm_set_value(LIGHT_SAVE_NAME, LIGHT_KEY_MATCH, (void*)&remote_array, sizeof(remote_array));
	}
}


// 上电分段 处理
void RePowerOnInit(void)
{
	// 获得分段标记
	remote_sys.Other = 0;
	arch_psm_get_value(LIGHT_SAVE_NAME, LIGHT_KEY_PONSETBIT, (void*)&remote_sys.Other, sizeof(remote_sys.Other));
	if(remote_sys.Other)//分段开关打开
	{
#if(LIGHT_WY_PWMON)
		// 开关分段打开
		// 墙壁开关分段功能(白光、黄光、中性光 小夜灯）   灯状态不属于分段3种状态时，分段从白光最亮开始
		if(lewyfan_infor.light_sw == 0)//第二次开灯
		{
			remote_sys.Other = 0;
			lewyfan_infor.light_sw = 1;
		}
		else
#if(LIGHT_RE_POWER)	// WY-上电分段功能 是否打开
		if(lewyfan_infor.power_state == POWER_STATE_ON)
		{
			lewyfan_infor.light_sw = 1;
			lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX;
			lewyfan_infor.light_rgbmode = LIGHT_WY_NORMAL;
			switch(remote_sys.Other)
			{
			case 1://白光
				lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MAX;
				break;
			case 2://黄光
				lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MIN;
				break;
			case 3://中性光
				lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MID;
				break;
			default://小夜灯
				lewyfan_infor.light_templatetrue = LIGHT_TEMPLATETRUE_MID;
				lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MIN;
				break;
			}
		}
#endif
#endif
#if(LIGHT_RGB_PWMON != 0 && LIGHT_WY_PWMON == 0)
		// 开关分段打开
		// 墙壁开关分段功能(白光、黄光、中性光 小夜灯）   灯状态不属于分段3种状态时，分段从白光最亮开始
		if(lewyfan_infor.light_sw == 0)//第二次开灯
		{
			remote_sys.Other = 0;
			lewyfan_infor.light_sw = true;
		}else
#if(LIGHT_RE_POWER)	// WY-上电分段功能 是否打开
		if(lewyfan_infor.power_state == POWER_STATE_ON)
		{
			lewyfan_infor.light_brightness = LIGHT_BRIGHTNESS_MAX;
			lewyfan_infor.light_rgbmode = LIGHT_RGB_NORMAL;
			switch(remote_sys.Other)
			{
			case 1://red
				lewyfan_infor.light_rcolor = LIGHT_RGB_VMAX;
				lewyfan_infor.light_gcolor = 0;
				lewyfan_infor.light_bcolor = 0;
				break;
			case 2://green
				lewyfan_infor.light_rcolor = 0;
				lewyfan_infor.light_gcolor = LIGHT_RGB_VMAX;
				lewyfan_infor.light_bcolor = 0;
				break;
			case 3://blue
				lewyfan_infor.light_rcolor = 0;
				lewyfan_infor.light_gcolor = 0;
				lewyfan_infor.light_bcolor = LIGHT_RGB_VMAX;
				break;
			default:
				lewyfan_infor.light_rcolor = LIGHT_RGB_VMAX;
				lewyfan_infor.light_gcolor = LIGHT_RGB_VMAX;
				lewyfan_infor.light_bcolor = LIGHT_RGB_VMAX;
				lewyfan_infor.light_rgbmode = LIGHT_RGB_FADE;
				break;
			}
		}
#endif
#endif
		// 保存灯的状态
		mSave |= 0x01;
		//arch_psm_set_value(LIGHT_SAVE_NAME, LIGHT_KEY_STATE, (void*)&lewyfan_infor, sizeof(lewyfan_infor));
	}
	remote_sys.Other++;
	if(remote_sys.Other > 4)remote_sys.Other=1;
	mSave |= 0x02;
	//arch_psm_set_value(LIGHT_SAVE_NAME, LIGHT_KEY_PONSETBIT, (void*)&remote_sys.Other, sizeof(remote_sys.Other));

}

uint8_t 	Remote_update_light_sw           = 0xFF;
uint32_t 	Remote_update_light_cmd		     = 0x0;
uint32_t 	Remote_update_light_brightness   = 0x0;
uint32_t 	Remote_update_light_templatetrue = 0x0;
uint32_t 	Remote_update_light_rgbcolor	 = 0x0;
uint8_t 	Remote_update_light_rgbmode		 = 0x0;

uint8_t 	Remote_update_light_sleepmode	 = 0xFF;
uint8_t 	Remote_update_light_weakupmode	 = 0xFF;
uint8_t 	Remote_update_light_modetime	 = 0xFF;
uint8_t 	Remote_update_light_sleepsbr	 = 0xFF;
uint8_t 	Remote_update_light_weakebr	     = 0xFF;

uint8_t 	Remote_update_powerstate         = 0xFF;
uint8_t 	Remote_update_usermode           = 0xFF;
#if(LIGHT_FADE_ON == 1 && LIGHT_FADE_SWITCH == 1)
uint16_t 	Remote_update_fadeon			 = 0xFF;
uint16_t 	Remote_update_fadeoff			 = 0xFF;
uint16_t 	Remote_update_fadescen			 = 0xFF;
#endif
#if(LIGHT_SMARTSWITCH_ENABLE == 1)
uint8_t 	Remote_update_smart_switch = 0xFF;
#endif

#if(LIGHT_REMOTE_NETSWITCH)
uint8_t 	Remote_update_remoteswitch       = 0xFF;
#endif

#if(LIGHT_NIGHT_SWITCH == 1)
uint8_t 	Remote_update_night_switch = 0xFF;
#endif


#if(LIGHT_PROSET_SWITCH == 1)
uint32_t 	Remote_update_lowest_brightness = 0xFFFFFFFF;
uint32_t 	Remote_update_lock_state = 0xFFFFFFFF;
#endif


/************************************************************
Description: light_remote_update
Other: None
************************************************************/
void light_remote_update(void)
{
	uint32_t temp;
	uint8_t  upcnt;

	if(remote_sys.mLight_online == 0)return;
	if(remote_sys.mUpdateTimer != 0)return;

	upcnt = 0;
	//同步灯状态
	if(((uint8_t)(lewyfan_infor.light_sw)) != Remote_update_light_sw)
	{
		Remote_update_light_sw = (uint8_t)lewyfan_infor.light_sw;
		//P_2_1_Group_doChange_notify(lewyfan_infor.light_sw);
		upcnt++;
	}

	if(lewyfan_infor.light_brightness != Remote_update_light_brightness)
	{
		Remote_update_light_brightness = lewyfan_infor.light_brightness;
		temp = lewyfan_infor.light_brightness/10;
		if(temp < 1)temp = 1;
		else
		if(temp > 100)temp = 100;
		//P_2_2_Name_doChange_notify(temp);
		upcnt++;
	}
#if(LIGHT_WY_PWMON)
	if(lewyfan_infor.light_templatetrue != Remote_update_light_templatetrue)
	{
		Remote_update_light_templatetrue = lewyfan_infor.light_templatetrue;
		P_2_3_ColorTemperature_doChange_notify(lewyfan_infor.light_templatetrue);
		upcnt++;
	}
#endif

#if(LIGHT_RGB_PWMON)
	temp  = lewyfan_infor.light_rcolor;
	temp<<=8;
	temp |= lewyfan_infor.light_gcolor;
	temp<<=8;
	temp |= lewyfan_infor.light_bcolor;

	if(temp != Remote_update_light_rgbcolor)
	{
		Remote_update_light_rgbcolor = temp;
		P_2_4_RGBColor_doChange_notify(temp);
		upcnt++;
	}
#endif

#if(LIGHT_MODE_SWITCH == 1)
	if(lewyfan_infor.light_rgbmode != Remote_update_light_rgbmode)
	{
		Remote_update_light_rgbmode = lewyfan_infor.light_rgbmode;
		P_2_5_RGBMode_doChange_notify(Remote_update_light_rgbmode);
		upcnt++;
	}
	if(upcnt > 5)
	{
		remote_sys.mUpdateTimer = 2000/LIGNT_TIMER_TICK;
		return;
	}
#endif


#if(LIGHT_SMARTSWITCH_ENABLE == 1)
	if(lewyfan_infor.smart_switch != Remote_update_smart_switch)
	{
		Remote_update_smart_switch = lewyfan_infor.smart_switch;
		P_2_6_FlexSwitch_doChange_notify(lewyfan_infor.smart_switch);
		upcnt++;
	}
	if(upcnt > 5)
	{
		remote_sys.mUpdateTimer = 2000/LIGNT_TIMER_TICK;
		return;
	}
#endif


	//sleep
	if(lewyfan_infor.light_rgbmode == LIGHT_CD_SLEEP)temp = 1;
	else											 temp = 0;
	if(temp != Remote_update_light_sleepmode)
	{
		Remote_update_light_sleepmode = temp;
		P_2_7_SleepMode_doChange_notify(temp);
		upcnt++;
	}
	if(upcnt > 5)
	{
		remote_sys.mUpdateTimer = 2000/LIGNT_TIMER_TICK;
		return;
	}

	//weak up
	if(lewyfan_infor.light_rgbmode == LIGHT_CD_WEAKUP)temp = 1;
	else											  temp = 0;
	if(temp != Remote_update_light_weakupmode)
	{
		Remote_update_light_weakupmode = temp;
		P_2_8_WeakupMode_doChange_notify(temp);
		upcnt++;
	}
	if(upcnt > 5)
	{
		remote_sys.mUpdateTimer = 2000/LIGNT_TIMER_TICK;
		return;
	}


#if(LIGHT_POWER_DEFAULT)
	if(lewyfan_infor.power_state != Remote_update_powerstate)
	{
		Remote_update_powerstate = lewyfan_infor.power_state;
		P_2_12_LightPONDefault_doChange_notify(lewyfan_infor.power_state);
		upcnt++;
	}
	if(upcnt > 5)
	{
		remote_sys.mUpdateTimer = 2000/LIGNT_TIMER_TICK;
		return;
	}

	if(lewyfan_infor.user_mode != Remote_update_usermode)
	{
		Remote_update_usermode = lewyfan_infor.user_mode;
		P_4_2_UserMode_doChange_notify(lewyfan_infor.user_mode);
		upcnt++;
	}
	if(upcnt > 5)
	{
		remote_sys.mUpdateTimer = 2000/LIGNT_TIMER_TICK;
		return;
	}
#endif

#if(LIGHT_FADE_ON == 1 && LIGHT_FADE_SWITCH == 1)
	if(lewyfan_infor.fade_on != Remote_update_fadeon)
	{
		Remote_update_fadeon = lewyfan_infor.fade_on;
		P_4_3_FadeOn_doChange_notify(lewyfan_infor.fade_on);
		upcnt++;
	}
	if(upcnt > 5)
	{
		remote_sys.mUpdateTimer = 2000/LIGNT_TIMER_TICK;
		return;
	}

	if(lewyfan_infor.fade_off != Remote_update_fadeoff)
	{
		Remote_update_fadeoff = lewyfan_infor.fade_off;
		P_4_4_FadeOff_doChange_notify(lewyfan_infor.fade_off);
		upcnt++;
	}
	if(upcnt > 5)
	{
		remote_sys.mUpdateTimer = 2000/LIGNT_TIMER_TICK;
		return;
	}

	if(lewyfan_infor.fade_scen != Remote_update_fadescen)
	{
		Remote_update_fadescen = lewyfan_infor.fade_scen;
		P_4_5_FadeScen_doChange_notify(lewyfan_infor.fade_scen);
		upcnt++;
	}
	if(upcnt > 5)
	{
		remote_sys.mUpdateTimer = 2000/LIGNT_TIMER_TICK;
		return;
	}
#endif

#if(LIGHT_NIGHT_SWITCH == 1)
	if(remote_sys.mLight_night != Remote_update_night_switch)
	{
		Remote_update_night_switch = remote_sys.mLight_night;
		P_4_6_NightSwitch_doChange_notify(remote_sys.mLight_night);
		upcnt++;
	}
	if(upcnt > 5)
	{
		remote_sys.mUpdateTimer = 2000/LIGNT_TIMER_TICK;
		return;
	}
#endif

#if(LIGHT_PROSET_SWITCH == 1)
	if(lewyfan_infor.lowest_brightness != Remote_update_lowest_brightness)
	{
		Remote_update_lowest_brightness = lewyfan_infor.lowest_brightness;
		P_4_8_LowestBrightness_doChange_notify(lewyfan_infor.lowest_brightness);
		upcnt++;
	}

	if(upcnt > 5)
	{
		remote_sys.mUpdateTimer = 2000/LIGNT_TIMER_TICK;
		return;
	}

	if(lewyfan_infor.lock_state != Remote_update_lock_state)
	{
		Remote_update_lock_state = lewyfan_infor.lock_state;
		P_4_9_LockState_doChange_notify(lewyfan_infor.lock_state);
		upcnt++;
	}

	if(upcnt > 5)
	{
		remote_sys.mUpdateTimer = 2000/LIGNT_TIMER_TICK;
		return;
	}
#endif


#if(LIGHT_REMOTE_NETSWITCH)
	if(lewyfan_infor.remote_switch != Remote_update_remoteswitch)
	{
		Remote_update_remoteswitch = lewyfan_infor.remote_switch;
		P_8_1_RemoteSwitch_doChange_notify(Remote_update_remoteswitch);
		upcnt++;
	}

	if(upcnt > 5)
	{
		remote_sys.mUpdateTimer = 2000/LIGNT_TIMER_TICK;
		return;
	}
	if(lewyfan_infor.remote_switch)
	{
		//1-4
		if(remote_sys.mRemote_MacUpdate & 0x0F)
		{
			if(remote_array[3].use)temp = remote_array[3].type;
			else				   temp = 0;
			temp <<= 8;
			if(remote_array[2].use)temp |= remote_array[2].type;
			temp <<= 8;
			if(remote_array[1].use)temp |= remote_array[1].type;
			temp <<= 8;
			if(remote_array[0].use)temp |= remote_array[0].type;
			P_8_4_RemoteLType_doChange_notify(temp);
			upcnt++;
		}
		//1
		if(remote_sys.mRemote_MacUpdate & 0x01)
		{
			remote_sys.mRemote_MacUpdate ^= 0x01;
			if(remote_array[0].use)
			{
				P_8_6_Remote1Mac_doChange_notify(remote_array[0].rid);
			}
			else{
				P_8_6_Remote1Mac_doChange_notify(0);
			}
			upcnt++;
		}


		//2
		if(remote_sys.mRemote_MacUpdate & 0x02)
		{
			remote_sys.mRemote_MacUpdate ^= 0x02;
			if(remote_array[1].use)
			{
				P_8_7_Remote2Mac_doChange_notify(remote_array[1].rid);
			}
			else{
				P_8_7_Remote2Mac_doChange_notify(0);
			}
			upcnt++;
		}

		//3
		if(remote_sys.mRemote_MacUpdate & 0x04)
		{
			remote_sys.mRemote_MacUpdate ^= 0x04;
			if(remote_array[2].use)
			{
				P_8_8_Remote3Mac_doChange_notify(remote_array[2].rid);
			}
			else{
				P_8_8_Remote3Mac_doChange_notify(0);
			}
			upcnt++;
		}
		//4
		if(remote_sys.mRemote_MacUpdate & 0x08)
		{
			remote_sys.mRemote_MacUpdate ^= 0x08;
			if(remote_array[3].use)
			{
				P_8_9_Remote4Mac_doChange_notify(remote_array[3].rid);
			}
			else{
				P_8_9_Remote4Mac_doChange_notify(0);
			}
			upcnt++;
		}

		if(upcnt > 5)
		{
			remote_sys.mUpdateTimer = 2000/LIGNT_TIMER_TICK;
			return;
		}
		//5-8
		if(remote_sys.mRemote_MacUpdate & 0xF0)
		{
			if(remote_array[7].use)temp = remote_array[7].type;
			else				   temp = 0;
			temp <<= 8;
			if(remote_array[6].use)temp |= remote_array[6].type;
			temp <<= 8;
			if(remote_array[5].use)temp |= remote_array[5].type;
			temp <<= 8;
			if(remote_array[1].use)temp |= remote_array[4].type;
			P_8_5_RemoteHType_doChange_notify(temp);
			upcnt++;
		}


		if(remote_sys.mRemote_MacUpdate & 0x10)
		{
			remote_sys.mRemote_MacUpdate ^= 0x10;
			if(remote_array[4].use)
			{
				P_8_10_Remote5Mac_doChange_notify(remote_array[4].rid);
			}
			else{
				P_8_10_Remote5Mac_doChange_notify(0);
			}
			upcnt++;
		}

		if(remote_sys.mRemote_MacUpdate & 0x20)
		{
			remote_sys.mRemote_MacUpdate ^= 0x20;
			if(remote_array[5].use)
			{
				P_8_11_Remote6Mac_doChange_notify(remote_array[5].rid);
			}
			else{
				P_8_11_Remote6Mac_doChange_notify(0);
			}
			upcnt++;
		}

		if(remote_sys.mRemote_MacUpdate & 0x40)
		{
			remote_sys.mRemote_MacUpdate ^= 0x40;
			if(remote_array[6].use)
			{
				P_8_12_Remote7Mac_doChange_notify(remote_array[6].rid);
			}
			else{
				P_8_12_Remote7Mac_doChange_notify(0);
			}
			upcnt++;
		}

		if(remote_sys.mRemote_MacUpdate & 0x80)
		{
			remote_sys.mRemote_MacUpdate ^= 0x80;
			if(remote_array[7].use)
			{
				P_8_13_Remote8Mac_doChange_notify(remote_array[7].rid);
			}
			else{
				P_8_13_Remote8Mac_doChange_notify(0);
			}
			upcnt++;
		}
	}

#endif
}

/************************************************************
Description: light_remote_update
Other: None
************************************************************/
void light_remote_updateCmd(uint8_t cmd,uint8_t per1,uint8_t per2,uint8_t per3)
{
#if(LIGHT_REMOTE_NETSWITCH)
	uint32_t cdata;

	cdata  = per3;
	cdata<<= 8;
	cdata |= per2;
	cdata<<= 8;
	cdata |= per1;
	cdata<<= 8;
	cdata |= cmd;
	if(remote_sys.mLight_online == 0)return;
	if(lewyfan_infor.remote_switch == 0)return;
	if(remote_sys.mRemote_match > LIGHT_REMOTE_MATCHMAX)return;
	if(remote_sys.mRemote_match == 0)return;
	if(remote_array[remote_sys.mRemote_match-1].use == 0)return;
	if(remote_array[remote_sys.mRemote_match-1].type < 128)return;
	Remote_update_light_cmd = cdata;
	LOG_INFO_TAG("updateCmd","light_remote_updateCmd=%08x\r\n",cdata);
	P_8_14_RemoteCmd_doChange_notify(cdata);
#endif
}

