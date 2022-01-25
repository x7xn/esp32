#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <hap_apple_servs.h>
#include <hap_apple_chars.h>
#include "mesh_gw_uart_parser.h"
#include "device_uart.h"
//#include "mesh_gw_uart_parser.h"
#include "mesh_gw_handler.h"

<<<<<<< HEAD
#define MESH_GW_SUPPORT_MAX_DEVICE_NUM	((uint8_t)100)

=======
#define MESH_GW_SUPPORT_MAX_DEVICE_NUM	(100)
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
static const char *TAG = "MESH HANDLE";

static uint8_t m_mesh_sub_device_update_task_f = 0;//子设备状态更新(添加/更新状态)任务
static uint32_t m_mesh_sub_device_update_time = 0;//子设备状态更新(添加/更新状态)时间

int hap_factory_keystore_set(const char *name_space, const char *key, const uint8_t *val, size_t val_size);
<<<<<<< HEAD
int hap_keystore_delete_namespace(const char *name_space);
=======
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2

//调光灯相关
typedef struct rgb {
    uint8_t r;  // 0-100 %
    uint8_t g;  // 0-100 %
    uint8_t b;  // 0-100 %
} rgb_t;

typedef struct hsp {
    uint16_t h;  // 0-360
    uint16_t s;  // 0-100
    uint16_t b;  // 0-100
} hsp_t;

static hsp_t s_hsb_val;
static uint16_t s_brightness;
static bool s_on = false;
static uint16_t s_color_temperature;

<<<<<<< HEAD
mesh_device_t m_mesh_gw_sub_device_list[MESH_GW_SUPPORT_MAX_DEVICE_NUM] = {0};
=======
mesh_device_t m_mesh_gw_sub_device_list[100] = {0};
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
uint8_t m_mesh_sub_device_count = 0;
mesh_gw_task_t m_mesh_gw_task_list[16] = {0};

#if 0
/* Mandatory identify routine for the accessory (bridge)
 * In a real accessory, something like LED blink should be implemented
 * got visual identification
 */
static int bridge_identify(hap_acc_t *ha)
{
    ESP_LOGI(TAG, "Bridge identified");
    return HAP_SUCCESS;
}
#endif
/* A dummy callback for handling a write on the "On" characteristic of Fan.
 * In an actual accessory, this should control the hardware
 */
static int characteristic_on(bool value)
{
    ESP_LOGI(TAG, "Received Write. CHAR-ON %s", value ? "On" : "Off");
    /* TODO: Control Actual Hardware */
    return 0;
}

/* Mandatory identify routine for the bridged accessory
 * In a real bridge, the actual accessory must be sent some request to
 * identify itself visually
 */
static int accessory_identify(hap_acc_t *ha)
{
    hap_serv_t *hs = hap_acc_get_serv_by_uuid(ha, HAP_SERV_UUID_ACCESSORY_INFORMATION);
    hap_char_t *hc = hap_serv_get_char_by_uuid(hs, HAP_CHAR_UUID_NAME);
    const hap_val_t *val = hap_char_get_val(hc);
    char *name = val->s;

    ESP_LOGI(TAG, "Bridged Accessory %s identified", name);
    return HAP_SUCCESS;
}

/* A dummy callback for handling a write on the "On" characteristic of Fan.
 * In an actual accessory, this should control the hardware
 */
static int accessory_write(hap_write_data_t write_data[], int count,
        void *serv_priv, void *write_priv)
{
    ESP_LOGI(TAG, "Write called for Accessory %s", (char *)serv_priv);
    int i, ret = HAP_SUCCESS;
    hap_write_data_t *write;
    for (i = 0; i < count; i++) 
	{
        write = &write_data[i];
        if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_ON)) 
		{
            characteristic_on(write->val.b);
            hap_char_update_val(write->hc, &(write->val));
            *(write->status) = HAP_STATUS_SUCCESS;
        } 
		else 
		{
            *(write->status) = HAP_STATUS_RES_ABSENT;
        }
    }
    return ret;
}

/**
 * @brief transform lightbulb's "RGB" and other parameter
 */
static void lightbulb_set_aim(uint32_t r, uint32_t g, uint32_t b, uint32_t cw, uint32_t ww, uint32_t period)
{
/*
	ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, r);
	ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, g);
	ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2, b);
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2);
*/
}

/**
 * @brief transform lightbulb's "HSV" to "RGB"
 */
static bool lightbulb_set_hsb2rgb(uint16_t h, uint16_t s, uint16_t v, rgb_t *rgb)
{
	bool res = true;
	uint16_t hi, F, P, Q, T;

	if (!rgb)
		return false;

	if (h > 360) return false;
	if (s > 100) return false;
	if (v > 100) return false;

	hi = (h / 60) % 6;
	F = 100 * h / 60 - 100 * hi;
	P = v * (100 - s) / 100;
	Q = v * (10000 - F * s) / 10000;
	T = v * (10000 - s * (100 - F)) / 10000;

	switch (hi) {
	case 0:
		rgb->r = v;
		rgb->g = T;
		rgb->b = P;
		break;
	case 1:
		rgb->r = Q;
		rgb->g = v;
		rgb->b = P;
		break;
	case 2:
		rgb->r = P;
		rgb->g = v;
		rgb->b = T;
		break;
	case 3:
		rgb->r = P;
		rgb->g = Q;
		rgb->b = v;
		break;
	case 4:
		rgb->r = T;
		rgb->g = P;
		rgb->b = v;
		break;
	case 5:
		rgb->r = v;
		rgb->g = P;
		rgb->b = Q;
		break;
	default:
		return false;
	}
	return res;
}

/**
 * @brief set the lightbulb's "HSV"
 */
<<<<<<< HEAD
static bool lightbulb_set_aim_hsv(uint8_t sub_device_index)
{
	rgb_t rgb_tmp;
	bool ret = lightbulb_set_hsb2rgb(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_hue, m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_saturation, m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness, &rgb_tmp);
=======
static bool lightbulb_set_aim_hsv(uint16_t h, uint16_t s, uint16_t v)
{
	rgb_t rgb_tmp;
	bool ret = lightbulb_set_hsb2rgb(h, s, v, &rgb_tmp);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2

	if (ret == false)
		return false;

<<<<<<< HEAD
	ESP_LOGI(TAG,"h:%d,s:%d,v:%d,R:%d,G:%d,B:%d\r\n",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_hue, m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_saturation, m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness,rgb_tmp.r,rgb_tmp.g,rgb_tmp.b);
=======
	printf("h:%d,s:%d,v:%d,R:%d,G:%d,B:%d\r\n",h,s,v,rgb_tmp.r,rgb_tmp.g,rgb_tmp.b);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2

/*
	lightbulb_set_aim(rgb_tmp.r * PWM_TARGET_DUTY / 100, rgb_tmp.g * PWM_TARGET_DUTY / 100,
			rgb_tmp.b * PWM_TARGET_DUTY / 100, (100 - s) * 5000 / 100, v * 2000 / 100, 1000);
*/

	return true;
}

<<<<<<< HEAD
#if 0
/**
 * @brief update the lightbulb's state
 */
static void lightbulb_update(uint8_t sub_device_index)
{
	lightbulb_set_aim_hsv(sub_device_index);
}
#endif
=======
/**
 * @brief update the lightbulb's state
 */
static void lightbulb_update()
{
	lightbulb_set_aim_hsv(s_hsb_val.h, s_hsb_val.s, s_hsb_val.b);
}
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2

/**
 * @brief turn on/off the lowlevel lightbulb
 */
int lightbulb_set_on(bool value,uint8_t sub_device_index)
{
	ESP_LOGI(TAG, "lightbulb_set_on : %s", value == true ? "true" : "false");

	if (value == true) 
	{
		s_hsb_val.b = s_brightness;
		s_on = true;

		m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off = 1;
	}
	else 
	{
		s_brightness = s_hsb_val.b;
		s_hsb_val.b = 0;
		s_on = false;
		
		m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off = 0;
	}

	
	m_mesh_gw_sub_device_list[sub_device_index].is_control_f |= LIGHTBULB_NOTIFY_SW_MASK;
	
<<<<<<< HEAD
	lightbulb_set_aim_hsv(sub_device_index);
=======
	lightbulb_update();
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2

	return 0;
}

/**
 * @brief set the brightness of the lowlevel lightbulb
 */
int lightbulb_set_brightness(int value,uint8_t sub_device_index)
{
    ESP_LOGI(TAG, "lightbulb_set_brightness : %d->%d", value,value*10);

	m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness = value;
	m_mesh_gw_sub_device_list[sub_device_index].is_control_f |= LIGHTBULB_NOTIFY_BRIGHTNESS_MASK;
<<<<<<< HEAD
	m_mesh_gw_sub_device_list[sub_device_index].is_control_f |= LIGHTBULB_NOTIFY_RGB_MASK;
=======
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
	
    s_hsb_val.b = value;
    s_brightness = s_hsb_val.b; 
    if (true == s_on)
<<<<<<< HEAD
        lightbulb_set_aim_hsv(sub_device_index);
=======
        lightbulb_update();
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2

    return 0;
}

/**
 * @brief set the color temperature of the lowlevel lightbulb
 */
int lightbulb_set_color_temp(int value,uint8_t sub_device_index)
{
	s_color_temperature = (value*20-1000)/7;
	
    ESP_LOGI(TAG, "lightbulb_set_color_temp : %d->%d", value,s_color_temperature);
	
	m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_color_temp = value;
	m_mesh_gw_sub_device_list[sub_device_index].is_control_f |= LIGHTBULB_NOTIFY_COLOR_TEMP_MASK;
	
    if (true == s_on)
<<<<<<< HEAD
        lightbulb_set_aim_hsv(sub_device_index);
=======
        lightbulb_update();
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2

    return 0;
}

/**
 * @brief set the hue of the lowlevel lightbulb
 */
<<<<<<< HEAD
int lightbulb_set_hue(float value,uint8_t sub_device_index)
{
    ESP_LOGI(TAG, "lightbulb_set_hue : %f", value);

	
	m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_hue = value;
	m_mesh_gw_sub_device_list[sub_device_index].is_control_f |= LIGHTBULB_NOTIFY_HUE_MASK;
    s_hsb_val.h = value;
    if (true == s_on)
        lightbulb_set_aim_hsv(sub_device_index);
=======
int lightbulb_set_hue(float value)
{
    ESP_LOGI(TAG, "lightbulb_set_hue : %f", value);

    s_hsb_val.h = value;
    if (true == s_on)
        lightbulb_update();
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2

    return 0;
}

/**
 * @brief set the saturation of the lowlevel lightbulb
 */
<<<<<<< HEAD
int lightbulb_set_saturation(float value,uint8_t sub_device_index)
{
    ESP_LOGI(TAG, "lightbulb_set_saturation : %f", value);
	
	m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_saturation = value;
	m_mesh_gw_sub_device_list[sub_device_index].is_control_f |= LIGHTBULB_NOTIFY_SATURATION_MASK;
    s_hsb_val.s = value;
    if (true == s_on)
        lightbulb_set_aim_hsv(sub_device_index);
=======
int lightbulb_set_saturation(float value)
{
    ESP_LOGI(TAG, "lightbulb_set_saturation : %f", value);

    s_hsb_val.s = value;
    if (true == s_on)
        lightbulb_update();
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2

    return 0;
}

/** 
 * @brief:   [比较sub id是否相同]
 * @Author:   xunuo
 * @DateTime: 2021年11月15日T10:32:28+0800
 * @param-sub_id[IN]:                   [要查找的sub id]
<<<<<<< HEAD
 * @param-find_index[OUT]:               [该设备在设备列表中的下标]
=======
 * @param-find_index[IN]:               [该设备在设备列表中的下标]
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
 * @return:                            [0-未找到 其它-找到]
 */
uint8_t find_device_by_sub_id(uint8_t* sub_id,uint8_t* find_index)
{
	uint8_t i = 0;
	uint8_t j = 0;
	
	for(i = 0; i < m_mesh_sub_device_count; i++)
	{
		for(j = 0; j < 10; j++)
		{
			if(m_mesh_gw_sub_device_list[i].sub_id[j] != sub_id[j])
			{
				break;
			}
		}

		//id相同
		if(j == 10)
		{
			*find_index = i;
			return 1;
		}
	}

	return 0;
}

<<<<<<< HEAD
/** 
 * @brief:   [Callback for handling writes on the Light Bulb Service]
 * @Author:   xunuo
 * @DateTime: 2021年11月17日T14:33:38+0800
 * @param-write_data[][IN-OUT]:             [write 信息]
 * @param-count[IN]:                    [write信息个数]
 * @param-serv_priv[IN]:            	 [write 信息对应的service 标识，用于识别是对那个service进行操作]
 * @param-write_priv[IN]:                 []
 * @return:                            [0-成功 -1-失败]
 */
static int lightbulb_write(hap_write_data_t write_data[], int count,void *serv_priv, void *write_priv)
=======

/* Callback for handling writes on the Light Bulb Service
 */
static int lightbulb_write(hap_write_data_t write_data[], int count,
		void *serv_priv, void *write_priv)
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
{
	uint8_t sub_device_index = 0;
	uint8_t find_f = 0;
	int i, ret = HAP_SUCCESS;
	uint8_t* temp_serv_priv = (uint8_t*)serv_priv;
	hap_write_data_t *write;
	
<<<<<<< HEAD
    ESP_LOGI(TAG, "Write called for Accessory %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x-%02x", temp_serv_priv[0],
		temp_serv_priv[1],temp_serv_priv[2],temp_serv_priv[3],temp_serv_priv[4],temp_serv_priv[5],
		temp_serv_priv[6],temp_serv_priv[7],temp_serv_priv[8],temp_serv_priv[9],temp_serv_priv[10]);
=======
    ESP_LOGI(TAG, "Write called for Accessory %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", temp_serv_priv[0],
		temp_serv_priv[1],temp_serv_priv[2],temp_serv_priv[3],temp_serv_priv[4],temp_serv_priv[5],
		temp_serv_priv[6],temp_serv_priv[7],temp_serv_priv[8],temp_serv_priv[9]);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
	
	//查找要控制的设备
	find_f = find_device_by_sub_id((uint8_t *)serv_priv,&sub_device_index);

<<<<<<< HEAD
	//找到该设备
=======
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
	if(find_f)
	{
		for (i = 0; i < count; i++) 
		{
<<<<<<< HEAD
			//依次获取所有write信息
			write = &write_data[i];
			
			/* Setting a default error value */
			*(write->status) = HAP_STATUS_VAL_INVALID;

			/*characteristic 为 on*/
			if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_ON)) 
			{
//				ESP_LOGI(TAG, "Received Write for Light %s", write->val.b ? "On" : "Off");
			
				*(write->status) = HAP_STATUS_SUCCESS;
				lightbulb_set_on(write->val.b,sub_device_index);
			} 
			/*characteristic 为 Brightness*/
			else if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_BRIGHTNESS)) 
			{
				ESP_LOGI(TAG, "Received Write for Light Brightness %d", write->val.i);
				
				*(write->status) = HAP_STATUS_SUCCESS;
				lightbulb_set_brightness(write->val.i,sub_device_index);
			} 
			/*characteristic 为 Hue*/
			else if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_HUE)) 
			{
				ESP_LOGI(TAG, "Received Write for Light Hue %f", write->val.f);
			
				*(write->status) = HAP_STATUS_SUCCESS;
				lightbulb_set_hue(write->val.f,sub_device_index);
			} 
			/*characteristic 为 Saturation*/
			else if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_SATURATION)) 
			{
				ESP_LOGI(TAG, "Received Write for Light Saturation %f", write->val.f);
			
				*(write->status) = HAP_STATUS_SUCCESS;
				lightbulb_set_saturation(write->val.f, sub_device_index);
			} 
			/*characteristic 为 Color Temperature*/
			else if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_COLOR_TEMPERATURE)) 
			{
				ESP_LOGI(TAG, "Received Write for Light Color Temperature %d", write->val.i);
			
				*(write->status) = HAP_STATUS_SUCCESS;
=======
			write = &write_data[i];
			/* Setting a default error value */
			*(write->status) = HAP_STATUS_VAL_INVALID;
			
			if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_ON)) 
			{
				ESP_LOGI(TAG, "Received Write for Light %s", write->val.b ? "On" : "Off");
				//if (lightbulb_set_on(write->val.b,sub_device_index) == 0) 
				//{
					*(write->status) = HAP_STATUS_SUCCESS;
				//}
				lightbulb_set_on(write->val.b,sub_device_index);
			} 
			else if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_BRIGHTNESS)) 
			{
				ESP_LOGI(TAG, "Received Write for Light Brightness %d", write->val.i);
				//if (lightbulb_set_brightness(write->val.i,sub_device_index) == 0) 
				//{
					*(write->status) = HAP_STATUS_SUCCESS;
				//}
				lightbulb_set_brightness(write->val.i,sub_device_index);
			} 
			else if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_HUE)) 
			{
				ESP_LOGI(TAG, "Received Write for Light Hue %f", write->val.f);
				if (lightbulb_set_hue(write->val.f) == 0) 
				{
					*(write->status) = HAP_STATUS_SUCCESS;
				}
			} 
			else if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_SATURATION)) 
			{
				ESP_LOGI(TAG, "Received Write for Light Saturation %f", write->val.f);
				if (lightbulb_set_saturation(write->val.f) == 0) 
				{
					*(write->status) = HAP_STATUS_SUCCESS;
				}
			} 
			else if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_COLOR_TEMPERATURE)) 
			{
				ESP_LOGI(TAG, "Received Write for Light Color Temperature %d", write->val.i);
				//if (lightbulb_set_color_temp(write->val.i) == 0) 
				//{
					*(write->status) = HAP_STATUS_SUCCESS;
				//}
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
				lightbulb_set_color_temp(write->val.i,sub_device_index);
			}
			else 
			{
				*(write->status) = HAP_STATUS_RES_ABSENT;
			}

			/* If the characteristic write was successful, update it in hap core */
			if (*(write->status) == HAP_STATUS_SUCCESS) 
			{
				hap_char_update_val(write->hc, &(write->val));
			} 
			else 
			{
				/* Else, set the return value appropriately to report error */
				ret = HAP_FAIL;
			}
			
		}
	}
<<<<<<< HEAD
	//没有对应的子设备
=======
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
	else
	{
		for (i = 0; i < count; i++) 
		{
			write = &write_data[i];
			
			*(write->status) = HAP_STATUS_RES_ABSENT;

			/* If the characteristic write was successful, update it in hap core */
			if (*(write->status) == HAP_STATUS_SUCCESS) 
			{
				hap_char_update_val(write->hc, &(write->val));
			} 
			else 
			{
				/* Else, set the return value appropriately to report error */
				ret = HAP_FAIL;
			}
		}
	}
	
	
	
	return ret;
}
		
/* Callback for handling writes on the Light Bulb Service
 */
<<<<<<< HEAD
static int auxiliary1_write(hap_write_data_t write_data[], int count,
		void *serv_priv, void *write_priv)
{
	
	uint8_t sub_device_index = 0;
	uint8_t find_f = 0;
	int i, ret = HAP_SUCCESS;
	hap_write_data_t *write;
	
	//ESP_LOGI(TAG, "Write called for auxiliary Accessory %s", (char *)serv_priv);
	
	//查找要控制的设备
	find_f = find_device_by_sub_id((uint8_t *)serv_priv,&sub_device_index);

	if(find_f)
	{
		for (i = 0; i < count; i++) 
		{
			write = &write_data[i];
			/* Setting a default error value */
			*(write->status) = HAP_STATUS_VAL_INVALID;
			
			if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_ON)) 
			{
				ESP_LOGI(TAG, "Received Write for auxiliary1 Light %s", write->val.b ? "On" : "Off");
				*(write->status) = HAP_STATUS_SUCCESS;
				//实际控制灯光
				m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.auxiliary_lamp1_sw = write->val.b;
				m_mesh_gw_sub_device_list[sub_device_index].is_control_f |= LIGHTBULB_NOTIFY_A1_MASK;
			} 
			else 
			{
				*(write->status) = HAP_STATUS_RES_ABSENT;
			}
			
			/* If the characteristic write was successful, update it in hap core
			 */
			if (*(write->status) == HAP_STATUS_SUCCESS) 
			{
				hap_char_update_val(write->hc, &(write->val));
			} 
			else 
			{
				/* Else, set the return value appropriately to report error */
				ret = HAP_FAIL;
			}
		}
	}
	else
	{
		for (i = 0; i < count; i++) 
		{
			write = &write_data[i];
			
			*(write->status) = HAP_STATUS_RES_ABSENT;
			
			/* If the characteristic write was successful, update it in hap core
			 */
			if (*(write->status) == HAP_STATUS_SUCCESS) 
			{
				hap_char_update_val(write->hc, &(write->val));
			} 
			else 
			{
				/* Else, set the return value appropriately to report error */
				ret = HAP_FAIL;
			}
		}
	}
	
	return ret;
}

/* Callback for handling writes on the Light Bulb Service
 */
static int auxiliary2_write(hap_write_data_t write_data[], int count,
		void *serv_priv, void *write_priv)
{
	
	uint8_t sub_device_index = 0;
	uint8_t find_f = 0;
	int i, ret = HAP_SUCCESS;
	hap_write_data_t *write;
	
	//ESP_LOGI(TAG, "Write called for auxiliary Accessory %s", (char *)serv_priv);
	
	//查找要控制的设备
	find_f = find_device_by_sub_id((uint8_t *)serv_priv,&sub_device_index);

	if(find_f)
	{
		for (i = 0; i < count; i++) 
		{
			write = &write_data[i];
			/* Setting a default error value */
			*(write->status) = HAP_STATUS_VAL_INVALID;
			
			if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_ON)) 
			{
				ESP_LOGI(TAG, "Received Write for auxiliary2 Light %s", write->val.b ? "On" : "Off");
				*(write->status) = HAP_STATUS_SUCCESS;
				//实际控制灯光
				m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.auxiliary_lamp2_sw = write->val.b;
				m_mesh_gw_sub_device_list[sub_device_index].is_control_f |= LIGHTBULB_NOTIFY_A2_MASK;
			} 
			else 
			{
				*(write->status) = HAP_STATUS_RES_ABSENT;
			}
			
			/* If the characteristic write was successful, update it in hap core
			 */
			if (*(write->status) == HAP_STATUS_SUCCESS) 
			{
				hap_char_update_val(write->hc, &(write->val));
			} 
			else 
			{
				/* Else, set the return value appropriately to report error */
				ret = HAP_FAIL;
			}
		}
	}
	else
	{
		for (i = 0; i < count; i++) 
		{
			write = &write_data[i];
			
			*(write->status) = HAP_STATUS_RES_ABSENT;
			
			/* If the characteristic write was successful, update it in hap core
			 */
			if (*(write->status) == HAP_STATUS_SUCCESS) 
			{
				hap_char_update_val(write->hc, &(write->val));
			} 
			else 
			{
				/* Else, set the return value appropriately to report error */
				ret = HAP_FAIL;
			}
		}
	}
	
	return ret;
}

/* Callback for handling writes on the Switch Service
 */
static int curtain_write(hap_write_data_t write_data[], int count,
		void *serv_priv, void *write_priv)
{
	uint8_t sub_device_index = 0;
	uint8_t find_f = 0;
	int i, ret = HAP_SUCCESS;
	hap_write_data_t *write;
	
/*
	ESP_LOGI(TAG, "Write called for Accessory %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x-%02x", temp_serv_priv[0],
		  temp_serv_priv[1],temp_serv_priv[2],temp_serv_priv[3],temp_serv_priv[4],temp_serv_priv[5],
		  temp_serv_priv[6],temp_serv_priv[7],temp_serv_priv[8],temp_serv_priv[9],temp_serv_priv[10]);
*/

/*
		   if (hap_req_get_ctrl_id(write_priv)) {
		ESP_LOGI(TAG, "Received write from %s", hap_req_get_ctrl_id(write_priv));
	}
*/
	  
	//查找要控制的设备
	find_f = find_device_by_sub_id((uint8_t *)serv_priv,&sub_device_index);

	if(find_f)
	{
		for (i = 0; i < count; i++) 
		{
			write = &write_data[i];
			*(write->status) = HAP_STATUS_VAL_INVALID;
			
			if(!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_TARGET_POSITION)) 
			{
				ESP_LOGI(TAG, "Received Write for Curtain target pos %d", write->val.i);

				*(write->status) = HAP_STATUS_SUCCESS;
				//控制实际的硬件
				m_mesh_gw_sub_device_list[sub_device_index].is_control_f |= CURTAIN_NOTIFY_TARGET_POS_MASK;
				m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.target_position = write->val.i;
			} 
			else 
			{
				*(write->status) = HAP_STATUS_RES_ABSENT;
			}

			/* If the characteristic write was successful, update it in hap core
			*/
			if (*(write->status) == HAP_STATUS_SUCCESS) 
			{
				hap_char_update_val(write->hc, &(write->val));
			} 
			else 
			{
				/* Else, set the return value appropriately to report error */
				ret = HAP_FAIL;
			}
		}

	}
	else
	{
		for (i = 0; i < count; i++) 
		{
			write = &write_data[i];
			
			/* Setting a default error value */
			*(write->status) = HAP_STATUS_RES_ABSENT;

			/* If the characteristic write was successful, update it in hap core
			*/
			if (*(write->status) == HAP_STATUS_SUCCESS) 
			{
				hap_char_update_val(write->hc, &(write->val));
			} 
			else 
			{
				/* Else, set the return value appropriately to report error */
				ret = HAP_FAIL;
			}
		}
	}


	return ret;
}

static int curtain_read(hap_char_t *hc, hap_status_t *status_code, void *serv_priv, void *read_priv)
{
	
	uint8_t sub_device_index = 0;
	//uint8_t find_f = 0;

	
	//查找要控制的设备
	//find_f = 
	find_device_by_sub_id((uint8_t *)serv_priv,&sub_device_index);
/*
	if (hap_req_get_ctrl_id(read_priv)) {
		ESP_LOGI(TAG, "Received read from %s", hap_req_get_ctrl_id(read_priv));
	}
*/
	if (!strcmp(hap_char_get_type_uuid(hc), HAP_CHAR_UUID_CURRENT_POSITION)) {
	   /* Read the current value, toggle it and set the new value.
		* A separate variable should be used for the new value, as the hap_char_get_val()
		* API returns a const pointer
		*/
		const hap_val_t *cur_val = hap_char_get_val(hc);

		hap_val_t new_val;
		new_val.i = m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.current_position;
		hap_char_update_val(hc, &new_val);
		*status_code = HAP_STATUS_SUCCESS;
		
		ESP_LOGI(TAG, "Received read curtain current pos:%d,%d", cur_val->i,m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.current_position);
	}
	//���� speed
	else if (!strcmp(hap_char_get_type_uuid(hc), HAP_CHAR_UUID_TARGET_POSITION)) {
	   /* Read the current value, toggle it and set the new value.
		* A separate variable should be used for the new value, as the hap_char_get_val()
		* API returns a const pointer
		*/
		const hap_val_t *cur_val = hap_char_get_val(hc);

		hap_val_t new_val;
		new_val.i = m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.target_position;
		hap_char_update_val(hc, &new_val);
		*status_code = HAP_STATUS_SUCCESS;
		
		ESP_LOGI(TAG, "Received read curtain target pos:%d,%d", cur_val->i,m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.target_position);
	}
	else if (!strcmp(hap_char_get_type_uuid(hc), HAP_CHAR_UUID_POSITION_STATE)) {
	   /* Read the current value, toggle it and set the new value.
		* A separate variable should be used for the new value, as the hap_char_get_val()
		* API returns a const pointer
		*/
		const hap_val_t *cur_val = hap_char_get_val(hc);

		hap_char_update_val(hc, (hap_val_t *)cur_val);
		*status_code = HAP_STATUS_SUCCESS;
		
		ESP_LOGI(TAG, "Received read curtain pos state:%d,%d", cur_val->i,m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.position_state);
	}
	return HAP_SUCCESS;
}


=======
static int auxiliary_write(hap_write_data_t write_data[], int count,
		void *serv_priv, void *write_priv)
{
	int i, ret = HAP_SUCCESS;
	hap_write_data_t *write;
	
	ESP_LOGI(TAG, "Write called for auxiliary Accessory %s", (char *)serv_priv);
	//查找要控制的设备
	
	
	for (i = 0; i < count; i++) 
	{
		write = &write_data[i];
		/* Setting a default error value */
		*(write->status) = HAP_STATUS_VAL_INVALID;
		
		if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_ON)) 
		{
			ESP_LOGI(TAG, "Received Write for auxiliary Light %s", write->val.b ? "On" : "Off");
			//if (lightbulb_set_on(write->val.b) == 0) 
			//{
				*(write->status) = HAP_STATUS_SUCCESS;
			//}
			//实际控制灯光
		} 
		else 
		{
			*(write->status) = HAP_STATUS_RES_ABSENT;
		}
		
		/* If the characteristic write was successful, update it in hap core
		 */
		if (*(write->status) == HAP_STATUS_SUCCESS) 
		{
			hap_char_update_val(write->hc, &(write->val));
		} 
		else 
		{
			/* Else, set the return value appropriately to report error */
			ret = HAP_FAIL;
		}
	}
	return ret;
}

>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
/* Callback for handling writes on the Switch Service
 */
static int switch_write(hap_write_data_t write_data[], int count,
		void *serv_priv, void *write_priv)
{
<<<<<<< HEAD
	uint8_t sub_device_index = 0;
	uint8_t find_f = 0;
	int i, ret = HAP_SUCCESS;
	uint8_t* temp_serv_priv = (uint8_t*)serv_priv;
	hap_write_data_t *write;
	
	ESP_LOGI(TAG, "Write called for Accessory %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x-%02x", temp_serv_priv[0],
		  temp_serv_priv[1],temp_serv_priv[2],temp_serv_priv[3],temp_serv_priv[4],temp_serv_priv[5],
		  temp_serv_priv[6],temp_serv_priv[7],temp_serv_priv[8],temp_serv_priv[9],temp_serv_priv[10]);

/*
		   if (hap_req_get_ctrl_id(write_priv)) {
        ESP_LOGI(TAG, "Received write from %s", hap_req_get_ctrl_id(write_priv));
    }
*/
	  
	//查找要控制的设备
	find_f = find_device_by_sub_id((uint8_t *)serv_priv,&sub_device_index);

	if(find_f)
	{
		for (i = 0; i < count; i++) 
		{
			write = &write_data[i];
			*(write->status) = HAP_STATUS_VAL_INVALID;
			
			if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_ON)) 
			{
				ESP_LOGI(TAG, "Received Write for Switch %s", write->val.b ? "On" : "Off");

				*(write->status) = HAP_STATUS_SUCCESS;
				//控制实际的硬件
				m_mesh_gw_sub_device_list[sub_device_index].is_control_f |= SWITCH_NOTIFY_SW1_MASK;
				m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw1_on_ff = write->val.b;

#if 0
				//if(temp_serv_priv[10] == 0)
				//{
					
				//}
				else if(temp_serv_priv[10] == 1)
				{
					//m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw2_on_ff = write->val.b;
				}
				else if(temp_serv_priv[10] == 2)
				{
					//m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw3_on_ff = write->val.b;
				}
				else if(temp_serv_priv[10] == 3)
				{
					//m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw4_on_ff = write->val.b;
				}
#endif

			} 
			else 
			{
				*(write->status) = HAP_STATUS_RES_ABSENT;
			}

			/* If the characteristic write was successful, update it in hap core
			*/
			if (*(write->status) == HAP_STATUS_SUCCESS) 
			{
				hap_char_update_val(write->hc, &(write->val));
			} 
			else 
			{
				/* Else, set the return value appropriately to report error */
				ret = HAP_FAIL;
			}
		}

	}
	else
	{
		for (i = 0; i < count; i++) 
		{
			write = &write_data[i];
			
			/* Setting a default error value */
			*(write->status) = HAP_STATUS_RES_ABSENT;

			/* If the characteristic write was successful, update it in hap core
			*/
			if (*(write->status) == HAP_STATUS_SUCCESS) 
			{
				hap_char_update_val(write->hc, &(write->val));
			} 
			else 
			{
				/* Else, set the return value appropriately to report error */
				ret = HAP_FAIL;
			}
		}
	}


	return ret;
}

/* Callback for handling writes on the Switch Service
 */
static int switch_key2_write(hap_write_data_t write_data[], int count,
		void *serv_priv, void *write_priv)
{
	uint8_t sub_device_index = 0;
	uint8_t find_f = 0;
	int i, ret = HAP_SUCCESS;
	uint8_t* temp_serv_priv = (uint8_t*)serv_priv;
	hap_write_data_t *write;
	
	ESP_LOGI(TAG, "Write called for Switch-key2 %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x-%02x", temp_serv_priv[0],
		  temp_serv_priv[1],temp_serv_priv[2],temp_serv_priv[3],temp_serv_priv[4],temp_serv_priv[5],
		  temp_serv_priv[6],temp_serv_priv[7],temp_serv_priv[8],temp_serv_priv[9],temp_serv_priv[10]);

/*
		   if (hap_req_get_ctrl_id(write_priv)) {
        ESP_LOGI(TAG, "Received write from %s", hap_req_get_ctrl_id(write_priv));
    }
*/
	  
	//查找要控制的设备
	find_f = find_device_by_sub_id((uint8_t *)serv_priv,&sub_device_index);

	if(find_f)
	{
		for (i = 0; i < count; i++) 
		{
			write = &write_data[i];
			*(write->status) = HAP_STATUS_VAL_INVALID;
			
			if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_ON)) 
			{
				ESP_LOGI(TAG, "Received Write for Switch %s", write->val.b ? "On" : "Off");

				*(write->status) = HAP_STATUS_SUCCESS;
				//控制实际的硬件
				m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw2_on_ff = write->val.b;
				m_mesh_gw_sub_device_list[sub_device_index].is_control_f |= SWITCH_NOTIFY_SW2_MASK;
			} 
			else 
			{
				*(write->status) = HAP_STATUS_RES_ABSENT;
			}

			/* If the characteristic write was successful, update it in hap core
			*/
			if (*(write->status) == HAP_STATUS_SUCCESS) 
			{
				hap_char_update_val(write->hc, &(write->val));
			} 
			else 
			{
				/* Else, set the return value appropriately to report error */
				ret = HAP_FAIL;
			}
		}

	}
	else
	{
		for (i = 0; i < count; i++) 
		{
			write = &write_data[i];
			
			/* Setting a default error value */
			*(write->status) = HAP_STATUS_RES_ABSENT;

			/* If the characteristic write was successful, update it in hap core
			*/
			if (*(write->status) == HAP_STATUS_SUCCESS) 
			{
				hap_char_update_val(write->hc, &(write->val));
			} 
			else 
			{
				/* Else, set the return value appropriately to report error */
				ret = HAP_FAIL;
			}
		}
	}


	return ret;
}
/* Callback for handling writes on the Switch Service
 */
static int switch_key3_write(hap_write_data_t write_data[], int count,
		void *serv_priv, void *write_priv)
{
	uint8_t sub_device_index = 0;
	uint8_t find_f = 0;
	int i, ret = HAP_SUCCESS;
	//uint8_t* temp_serv_priv = (uint8_t*)serv_priv;
	hap_write_data_t *write;
	
/*
		   if (hap_req_get_ctrl_id(write_priv)) {
		ESP_LOGI(TAG, "Received write from %s", hap_req_get_ctrl_id(write_priv));
	}
*/
	  
	//查找要控制的设备
	find_f = find_device_by_sub_id((uint8_t *)serv_priv,&sub_device_index);

	if(find_f)
	{
		for (i = 0; i < count; i++) 
		{
			write = &write_data[i];
			*(write->status) = HAP_STATUS_VAL_INVALID;
			
			if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_ON)) 
			{
				ESP_LOGI(TAG, "Received Write for Switch3 %s", write->val.b ? "On" : "Off");

				*(write->status) = HAP_STATUS_SUCCESS;
				//控制实际的硬件
				m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw3_on_ff = write->val.b;
				m_mesh_gw_sub_device_list[sub_device_index].is_control_f |= SWITCH_NOTIFY_SW3_MASK;
			} 
			else 
			{
				*(write->status) = HAP_STATUS_RES_ABSENT;
			}

			/* If the characteristic write was successful, update it in hap core
			*/
			if (*(write->status) == HAP_STATUS_SUCCESS) 
			{
				hap_char_update_val(write->hc, &(write->val));
			} 
			else 
			{
				/* Else, set the return value appropriately to report error */
				ret = HAP_FAIL;
			}
		}

	}
	else
	{
		for (i = 0; i < count; i++) 
		{
			write = &write_data[i];
			
			/* Setting a default error value */
			*(write->status) = HAP_STATUS_RES_ABSENT;

			/* If the characteristic write was successful, update it in hap core
			*/
			if (*(write->status) == HAP_STATUS_SUCCESS) 
			{
				hap_char_update_val(write->hc, &(write->val));
			} 
			else 
			{
				/* Else, set the return value appropriately to report error */
				ret = HAP_FAIL;
			}
		}
	}


	return ret;
}

/* Callback for handling writes on the Switch Service
 */
static int switch_key4_write(hap_write_data_t write_data[], int count,
		void *serv_priv, void *write_priv)
{
	uint8_t sub_device_index = 0;
	uint8_t find_f = 0;
	int i, ret = HAP_SUCCESS;
	//uint8_t* temp_serv_priv = (uint8_t*)serv_priv;
	hap_write_data_t *write;
	
/*
		   if (hap_req_get_ctrl_id(write_priv)) {
		ESP_LOGI(TAG, "Received write from %s", hap_req_get_ctrl_id(write_priv));
	}
*/
	  
	//查找要控制的设备
	find_f = find_device_by_sub_id((uint8_t *)serv_priv,&sub_device_index);

	if(find_f)
	{
		for (i = 0; i < count; i++) 
		{
			write = &write_data[i];
			*(write->status) = HAP_STATUS_VAL_INVALID;
			
			if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_ON)) 
			{
				ESP_LOGI(TAG, "Received Write for Switch4 %s", write->val.b ? "On" : "Off");

				*(write->status) = HAP_STATUS_SUCCESS;
				//控制实际的硬件
				m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw4_on_ff = write->val.b;
				m_mesh_gw_sub_device_list[sub_device_index].is_control_f |= SWITCH_NOTIFY_SW4_MASK;
			} 
			else 
			{
				*(write->status) = HAP_STATUS_RES_ABSENT;
			}

			/* If the characteristic write was successful, update it in hap core
			*/
			if (*(write->status) == HAP_STATUS_SUCCESS) 
			{
				hap_char_update_val(write->hc, &(write->val));
			} 
			else 
			{
				/* Else, set the return value appropriately to report error */
				ret = HAP_FAIL;
			}
		}

	}
	else
	{
		for (i = 0; i < count; i++) 
		{
			write = &write_data[i];
			
			/* Setting a default error value */
			*(write->status) = HAP_STATUS_RES_ABSENT;

			/* If the characteristic write was successful, update it in hap core
			*/
			if (*(write->status) == HAP_STATUS_SUCCESS) 
			{
				hap_char_update_val(write->hc, &(write->val));
			} 
			else 
			{
				/* Else, set the return value appropriately to report error */
				ret = HAP_FAIL;
			}
		}
	}


=======
	int i, ret = HAP_SUCCESS;
	hap_write_data_t *write;
	
	ESP_LOGI(TAG, "Write called for Switch Accessory %s", (char *)serv_priv);
	//查找要控制的设备
	
	
	for (i = 0; i < count; i++) 
	{
		write = &write_data[i];
		/* Setting a default error value */
		*(write->status) = HAP_STATUS_VAL_INVALID;
		if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_ON)) 
		{
			ESP_LOGI(TAG, "Received Write for Switch %s", write->val.b ? "On" : "Off");
		
			*(write->status) = HAP_STATUS_SUCCESS;
			//控制实际的硬件
			//lightbulb_set_on(write->val.b
		} 
		else 
		{
			*(write->status) = HAP_STATUS_RES_ABSENT;
		}
		
		/* If the characteristic write was successful, update it in hap core
		 */
		if (*(write->status) == HAP_STATUS_SUCCESS) 
		{
			hap_char_update_val(write->hc, &(write->val));
		} 
		else 
		{
			/* Else, set the return value appropriately to report error */
			ret = HAP_FAIL;
		}
	}
	
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
	return ret;
}

uint8_t add_accessory_to_bridge(uint8_t sub_device_index)
#if 0

{
	hap_serv_t *service;
	hap_acc_t *accessory;

	char accessory_name[20] = {0};
	hap_acc_cfg_t bridge_cfg = {
		.name = accessory_name,
		.manufacturer = "LeMesh",
		.model = "Le-Light01",
		.serial_num = "abcdefg",
		.fw_rev = "0.9.0",
		.hw_rev = NULL,
		.pv = "1.1.0",
		.identify_routine = accessory_identify,
		.cid = HAP_CID_BRIDGE,
	};
	//uint8_t i = 1;
	//设置name
	sprintf(accessory_name, "S-Lightbulb-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
	//


	/* Create accessory object */
	accessory = hap_acc_create(&bridge_cfg);

	/* Create the Fan Service. Include the "name" since this is a user visible service	*/
	service = hap_serv_lightbulb_create(false);
	hap_serv_add_char(service, hap_char_name_create(accessory_name));

	/* Set the Accessory name as the Private data for the service,
	 * so that the correct accessory can be identified in the
	 * write callback
	 */
	hap_serv_set_priv(service, strdup(accessory_name));

	/* Set the write callback for the service */
	hap_serv_set_write_cb(service, accessory_write);

	/* Add the Fan Service to the Accessory Object */
	hap_acc_add_serv(accessory, service);

	/* Add the Accessory to the HomeKit Database */
	hap_add_bridged_accessory(accessory, hap_get_unique_aid(accessory_name));
}
#else
{
    hap_acc_t *accessory = NULL;
    hap_serv_t *service;
	int ret = 0;
	char accessory_name[20] = {0};
	uint8_t result_code = 0;
	
	hap_acc_cfg_t bridge_cfg = {
		.name = accessory_name,
		.manufacturer = "LeMesh",
		.model = "Le-Light01",
		.serial_num = "abcdefg",
		.fw_rev = "0.9.0",
		.hw_rev = NULL,
		.pv = "1.1.0",
		.identify_routine = accessory_identify,
		.cid = HAP_CID_BRIDGE,
	};

	ESP_LOGI(TAG,"add_accessory_to_bridge\r\n");
	ESP_LOGI(TAG,"sub_type:%d,sub_reg_id:%d\r\n",m_mesh_gw_sub_device_list[sub_device_index].sub_type,m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
		
	switch(m_mesh_gw_sub_device_list[sub_device_index].sub_type)
	{
			//单色灯
<<<<<<< HEAD
		case HAP_TYPE_SINGLE_LIGHTBULB_E:
			ESP_LOGI(TAG,"single color lamp\r\n");
=======
		case MESH_SUB_TYPE_SINGLE_LIGHTBULB_E:
			ESP_LOGI(TAG,"I am single lamp\r\n");
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//设置name
			sprintf(accessory_name, "S-Lightbulb-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//

			/* Create accessory object */
			accessory = hap_acc_create(&bridge_cfg);
		
			/* Create the Fan Service. Include the "name" since this is a user visible service	*/
			service = hap_serv_lightbulb_create(false);
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
			ret |= hap_serv_add_char(service, hap_char_brightness_create(50));
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness = 50;
<<<<<<< HEAD
			
=======
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			  /* Get pointer to the outlet in use characteristic which we need to monitor for state changes */
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.hap_char_on_handle = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_ON);
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.hap_char_brightness_handle = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_BRIGHTNESS);
			/* Set the Accessory name as the Private data for the service,
			 * so that the correct accessory can be identified in the
			 * write callback
			 */
<<<<<<< HEAD
			//hap_serv_set_priv(service, strdup(accessory_name));
			m_mesh_gw_sub_device_list[sub_device_index].sub_id[10] = 0;
			hap_serv_set_priv(service, m_mesh_gw_sub_device_list[sub_device_index].sub_id);
=======
			hap_serv_set_priv(service, strdup(accessory_name));
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, lightbulb_write);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
			
			/* Add the Accessory to the HomeKit Database */
			hap_add_bridged_accessory(accessory, hap_get_unique_aid(accessory_name));
		
			break;
			//双色灯
<<<<<<< HEAD
		case HAP_TYPE_DOUBLE_LIGHTBULB_E:
			
			ESP_LOGI(TAG,"double color lamp\r\n");
=======
		case MESH_SUB_TYPE_DOUBLE_LIGHTBULB_E:
			
			ESP_LOGI(TAG,"I am double lamp\r\n");
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//设置name
			sprintf(accessory_name, "D-Lightbulb-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-Light02";
		
			/* Create accessory object */
			accessory = hap_acc_create(&bridge_cfg);
		
			/* Create the Fan Service. Include the "name" since this is a user visible service	*/
			service = hap_serv_lightbulb_create(false);
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
			hap_serv_add_char(service, hap_char_brightness_create(50));
			
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness = 50;

			/*双色灯*/
			//hap_serv_add_char(service, hap_char_double_hue_create(0.0));
			hap_serv_add_char(service, hap_char_color_temperature_create(50));
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_color_temp = 50;
			
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.hap_char_on_handle = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_ON);
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.hap_char_brightness_handle = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_BRIGHTNESS);
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.hap_char_color_temp_handle = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_COLOR_TEMPERATURE);
			/* Set the Accessory name as the Private data for the service,
			 * so that the correct accessory can be identified in the
			 * write callback
			 */
			//hap_serv_set_priv(service, strdup(accessory_name));
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_id[10] = 0;
=======
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			hap_serv_set_priv(service, m_mesh_gw_sub_device_list[sub_device_index].sub_id);
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, lightbulb_write);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
			
			/* Add the Accessory to the HomeKit Database */
			hap_add_bridged_accessory(accessory, hap_get_unique_aid(accessory_name));
			break;
			
			//RGB灯
<<<<<<< HEAD
		case HAP_TYPE_RGB_LIGHTBULB_E:
=======
		case MESH_SUB_TYPE_RGB_LIGHTBULB_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//设置name
			sprintf(accessory_name, "R-Lightbulb-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-Light03";
		
			/* Create accessory object */
			accessory = hap_acc_create(&bridge_cfg);
		
			/* Create the Fan Service. Include the "name" since this is a user visible service	*/
			service = hap_serv_lightbulb_create(false);
			hap_serv_add_char(service, hap_char_name_create(accessory_name));

			/*RGB*/			
			ret |= hap_serv_add_char(service, hap_char_brightness_create(50));
			ret |= hap_serv_add_char(service, hap_char_hue_create(180));
			ret |= hap_serv_add_char(service, hap_char_saturation_create(100));
			//ret |= hap_serv_add_char(service, hap_char_color_temperature_create(50));
			
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off = 0;
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness = 50;
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_hue = 180;
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_saturation = 100;
			
=======
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			 if (ret != HAP_SUCCESS) {
		        ESP_LOGE(TAG, "Failed to add optional characteristics to LightBulb");
		       // goto light_err;
		    }
			/* Set the Accessory name as the Private data for the service,
			 * so that the correct accessory can be identified in the
			 * write callback
			 */
<<<<<<< HEAD
			//hap_serv_set_priv(service, strdup(accessory_name));
			 m_mesh_gw_sub_device_list[sub_device_index].sub_id[10] = 0;
			 hap_serv_set_priv(service, m_mesh_gw_sub_device_list[sub_device_index].sub_id);
=======
			hap_serv_set_priv(service, strdup(accessory_name));
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, lightbulb_write);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
			
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.hap_char_on_handle = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_ON);
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.hap_char_brightness_handle = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_BRIGHTNESS);
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.hap_char_hue_handle = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_HUE);
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.hap_char_saturation_handle = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_SATURATION);
			
=======
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			/* Add the Accessory to the HomeKit Database */
			hap_add_bridged_accessory(accessory, hap_get_unique_aid(accessory_name));
			break;
			//WYRGB
<<<<<<< HEAD
		case HAP_TYPE_WYRGB_LIGHTBULB_E:
=======
		case MESH_SUB_TYPE_WYRGB_LIGHTBULB_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//设置name
			sprintf(accessory_name, "WY-Lightbulb-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-Light04";
		
			/* Create accessory object */
			accessory = hap_acc_create(&bridge_cfg);
		
			/* Create the Fan Service. Include the "name" since this is a user visible service	*/
			service = hap_serv_lightbulb_create(false);
			hap_serv_add_char(service, hap_char_name_create(accessory_name));

			/*RGB*/			
			ret |= hap_serv_add_char(service, hap_char_brightness_create(50));
			ret |= hap_serv_add_char(service, hap_char_hue_create(180));
			ret |= hap_serv_add_char(service, hap_char_saturation_create(100));
			ret |= hap_serv_add_char(service, hap_char_color_temperature_create(50));

<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off = 0;
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness = 50;
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_hue = 180;
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_saturation = 100;
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_color_temp = 50;

=======
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			 if (ret != HAP_SUCCESS) {
		        ESP_LOGE(TAG, "Failed to add optional characteristics to LightBulb");
		        //goto light_err;
		    }
			/* Set the Accessory name as the Private data for the service,
			 * so that the correct accessory can be identified in the
			 * write callback
			 */
<<<<<<< HEAD
			//hap_serv_set_priv(service, strdup(accessory_name));
			 m_mesh_gw_sub_device_list[sub_device_index].sub_id[10] = 0;
			 hap_serv_set_priv(service, m_mesh_gw_sub_device_list[sub_device_index].sub_id);
=======
			hap_serv_set_priv(service, strdup(accessory_name));
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, lightbulb_write);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
			
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.hap_char_on_handle = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_ON);
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.hap_char_brightness_handle = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_BRIGHTNESS);
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.hap_char_color_temp_handle = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_COLOR_TEMPERATURE);
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.hap_char_hue_handle = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_HUE);
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.hap_char_saturation_handle = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_SATURATION);
			

			/*Add 辅灯1*/
			service = hap_serv_lightbulb_create(false);
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.auxiliary_lamp1_sw = 0;
			//设置name
			sprintf(accessory_name, "WY-Lightbulb-%d-A1", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
			//hap_serv_set_priv(service, strdup(accessory_name));
			 m_mesh_gw_sub_device_list[sub_device_index].sub_id[10] = 1;
			 hap_serv_set_priv(service, m_mesh_gw_sub_device_list[sub_device_index].sub_id);
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service,auxiliary1_write);
			
			/* Add the Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
			
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.hap_char_auxiliary1_on_handle = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_ON);

			/*Add 辅灯2*/
			service = hap_serv_lightbulb_create(false);
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.auxiliary_lamp2_sw = 0;
			//设置name
			sprintf(accessory_name, "WY-Lightbulb-%d-A2", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
			//hap_serv_set_priv(service, strdup(accessory_name));
			 m_mesh_gw_sub_device_list[sub_device_index].sub_id[10] = 2;
			 hap_serv_set_priv(service, m_mesh_gw_sub_device_list[sub_device_index].sub_id);
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service,auxiliary2_write);
			
			/* Add the Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
			
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.hap_char_auxiliary2_on_handle = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_ON);
			
			/* Add the Accessory to the HomeKit Database */
			//设置name
			sprintf(accessory_name, "WY-Lightbulb-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			hap_add_bridged_accessory(accessory, hap_get_unique_aid(accessory_name));
			break;
			//双色灯+辅灯
		case HAP_TYPE_DOUBLE_PLUS_LIGHTBULB_E:
=======
			/* Add the Accessory to the HomeKit Database */
			hap_add_bridged_accessory(accessory, hap_get_unique_aid(accessory_name));
			break;
			//双色灯+辅灯
		case MESH_SUB_TYPE_DOUBLE_PLUS_LIGHTBULB_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//设置name
			sprintf(accessory_name, "D+-Lightbulb-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-Light05";
		
			/* Create accessory object */
			accessory = hap_acc_create(&bridge_cfg);
		
			/* Create the Fan Service. Include the "name" since this is a user visible service	*/
			service = hap_serv_lightbulb_create(false);
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
			hap_serv_add_char(service, hap_char_brightness_create(50));

			/*双色灯*/
			hap_serv_add_char(service, hap_char_color_temperature_create(50));
			/* Set the Accessory name as the Private data for the service,
			 * so that the correct accessory can be identified in the
			 * write callback
			 */
			hap_serv_set_priv(service, strdup(accessory_name));
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, lightbulb_write);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
			
			/*辅灯1*/
			service = hap_serv_lightbulb_create(false);
			//设置name
			sprintf(accessory_name, "D+%d-Auxiliary-1", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
			hap_serv_set_priv(service, strdup(accessory_name));
			/* Set the write callback for the service */
<<<<<<< HEAD
			hap_serv_set_write_cb(service, auxiliary1_write);
=======
			hap_serv_set_write_cb(service, auxiliary_write);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);

			/*辅灯2*/
			service = hap_serv_lightbulb_create(false);
			//设置name
			sprintf(accessory_name, "D+%d-Auxiliary-2", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
			hap_serv_set_priv(service, strdup(accessory_name));
			/* Set the write callback for the service */
<<<<<<< HEAD
			hap_serv_set_write_cb(service, auxiliary2_write);
=======
			hap_serv_set_write_cb(service, auxiliary_write);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
			
			/* Add the Accessory to the HomeKit Database */
			hap_add_bridged_accessory(accessory, hap_get_unique_aid(accessory_name));
			break;
			//单色灯+辅灯
<<<<<<< HEAD
		case HAP_TYPE_SINGLE_PLUS_LIGHTBULB_E:
=======
		case MESH_SUB_TYPE_SINGLE_PLUS_LIGHTBULB_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//设置name
			sprintf(accessory_name, "S+-Lightbulb-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-Light06";
			/* Create accessory object */
			accessory = hap_acc_create(&bridge_cfg);
		
			/* Create the Fan Service. Include the "name" since this is a user visible service	*/
			service = hap_serv_lightbulb_create(false);
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
			ret |= hap_serv_add_char(service, hap_char_brightness_create(50));
			
			/* Set the Accessory name as the Private data for the service,
			 * so that the correct accessory can be identified in the
			 * write callback
			 */
			hap_serv_set_priv(service, strdup(accessory_name));
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, lightbulb_write);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);

			/*辅灯1*/
			service = hap_serv_lightbulb_create(false);
			//设置name
			sprintf(accessory_name, "S+%d-Auxiliary-1", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
			hap_serv_set_priv(service, strdup(accessory_name));
			/* Set the write callback for the service */
<<<<<<< HEAD
			hap_serv_set_write_cb(service, auxiliary1_write);
=======
			hap_serv_set_write_cb(service, auxiliary_write);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);

			/*辅灯2*/
			service = hap_serv_lightbulb_create(false);
			//设置name
			sprintf(accessory_name, "S+%d-Auxiliary-2", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
			hap_serv_set_priv(service, strdup(accessory_name));
			/* Set the write callback for the service */
<<<<<<< HEAD
			hap_serv_set_write_cb(service, auxiliary2_write);
=======
			hap_serv_set_write_cb(service, auxiliary_write);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
			
			/* Add the Accessory to the HomeKit Database */
			hap_add_bridged_accessory(accessory, hap_get_unique_aid(accessory_name));
			break;
			//WRGB
<<<<<<< HEAD
		case HAP_TYPE_WRGB_LIGHTBULB_E:
=======
		case MESH_SUB_TYPE_WRGB_LIGHTBULB_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//设置name
			sprintf(accessory_name, "W-Lightbulb-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-Light07";
			/* Create accessory object */
			accessory = hap_acc_create(&bridge_cfg);
		
			/* Create the Fan Service. Include the "name" since this is a user visible service	*/
			service = hap_serv_lightbulb_create(false);
			hap_serv_add_char(service, hap_char_name_create(accessory_name));

			/*RGB*/			
			ret |= hap_serv_add_char(service, hap_char_brightness_create(50));
			ret |= hap_serv_add_char(service, hap_char_hue_create(180));
			ret |= hap_serv_add_char(service, hap_char_saturation_create(100));
			ret |= hap_serv_add_char(service, hap_char_color_temperature_create(50));

			 if (ret != HAP_SUCCESS) {
		        ESP_LOGE(TAG, "Failed to add optional characteristics to LightBulb");
		        //goto light_err;
		    }
			/* Set the Accessory name as the Private data for the service,
			 * so that the correct accessory can be identified in the
			 * write callback
			 */
			hap_serv_set_priv(service, strdup(accessory_name));
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, lightbulb_write);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
			
			/* Add the Accessory to the HomeKit Database */
			hap_add_bridged_accessory(accessory, hap_get_unique_aid(accessory_name));
			break;
			//开关控制器
<<<<<<< HEAD
		case HAP_TYPE_SW_CONTROLLER_E:
=======
		case MESH_SUB_TYPE_SW_CONTROLLER_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//设置name
			sprintf(accessory_name, "SW-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-SW01";
		
			/* Create accessory object */
			accessory = hap_acc_create(&bridge_cfg);
		
			/* Create the Fan Service. Include the "name" since this is a user visible service	*/
			service = hap_serv_switch_create(false);
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw1_on_ff = 0;
=======
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
			/* Set the Accessory name as the Private data for the service,
			 * so that the correct accessory can be identified in the
			 * write callback
			 */
<<<<<<< HEAD
			//hap_serv_set_priv(service, strdup(accessory_name));
			 m_mesh_gw_sub_device_list[sub_device_index].sub_id[10] = 0;
			 hap_serv_set_priv(service, m_mesh_gw_sub_device_list[sub_device_index].sub_id);
=======
			hap_serv_set_priv(service, strdup(accessory_name));
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, switch_write);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
<<<<<<< HEAD

			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.hap_char_on_handle1 = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_ON);
=======
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
			/* Add the Accessory to the HomeKit Database */
			hap_add_bridged_accessory(accessory, hap_get_unique_aid(accessory_name));
			break;
<<<<<<< HEAD
		case HAP_TYPE_CLASSROOM_SINGLE_LIGHTBULB_E:
=======
		case MESH_SUB_TYPE_CLASSROOM_SINGLE_LIGHTBULB_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//设置name
			sprintf(accessory_name, "C-Lightbulb-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-Light08";
			break;
			//照度传感器
<<<<<<< HEAD
		case HAP_TYPE_ILLUMINANCE_SENSOR_E:
=======
		case MESH_SUB_TYPE_ILLUMINANCE_SENSOR_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//设置name
			sprintf(accessory_name, "Light-Sensor-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-Sensor01";
			
			break;
<<<<<<< HEAD
		case HAP_TYPE_ELECTRIC_CURTAINS_E:
=======
		case MESH_SUB_TYPE_ELECTRIC_CURTAINS_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//设置name
			sprintf(accessory_name, "Curtain-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-Curtain01";
<<<<<<< HEAD
		
			/* Create accessory object */
			accessory = hap_acc_create(&bridge_cfg);
		
			/* Create the Fan Service. Include the "name" since this is a user visible service	*/
			service = hap_serv_window_covering_create(0, 0, 2);
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.current_position = 0;
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.target_position = 0;
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.position_state = 2;
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
			/* Set the Accessory name as the Private data for the service,
			 * so that the correct accessory can be identified in the
			 * write callback
			 */
			//hap_serv_set_priv(service, strdup(accessory_name));
			 m_mesh_gw_sub_device_list[sub_device_index].sub_id[10] = 0;
			 hap_serv_set_priv(service, m_mesh_gw_sub_device_list[sub_device_index].sub_id);
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, curtain_write);
			hap_serv_set_read_cb(service, curtain_read);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);

			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.hap_char_current_position_handle = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_CURRENT_POSITION);
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.hap_char_target_position_handle = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_TARGET_POSITION);
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.hap_char_position_state_handle = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_POSITION_STATE);
			
			/* Add the Accessory to the HomeKit Database */
			sprintf(accessory_name, "Curtain-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			hap_add_bridged_accessory(accessory, hap_get_unique_aid(accessory_name));
			break;
			//一路开关
		case HAP_TYPE_SWITCH_1KEY_E:
=======
			break;
			//一路开关
		case MESH_SUB_TYPE_SWITCH_1KEY_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//设置name
			sprintf(accessory_name, "Switch-1Key-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-Switch01";
		
			/* Create accessory object */
			accessory = hap_acc_create(&bridge_cfg);
		
			/* Create the Fan Service. Include the "name" since this is a user visible service	*/
			service = hap_serv_switch_create(false);
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw1_on_ff = 0;
=======
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
			/* Set the Accessory name as the Private data for the service,
			 * so that the correct accessory can be identified in the
			 * write callback
			 */
<<<<<<< HEAD
			//hap_serv_set_priv(service, strdup(accessory_name));
			 m_mesh_gw_sub_device_list[sub_device_index].sub_id[10] = 0;
			 hap_serv_set_priv(service, m_mesh_gw_sub_device_list[sub_device_index].sub_id);
=======
			hap_serv_set_priv(service, strdup(accessory_name));
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, switch_write);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
<<<<<<< HEAD

			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.hap_char_on_handle1 = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_ON);
=======
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
			/* Add the Accessory to the HomeKit Database */
			hap_add_bridged_accessory(accessory, hap_get_unique_aid(accessory_name));
			break;
<<<<<<< HEAD
		case HAP_TYPE_SWITCH_2KEY_E:
=======
		case MESH_SUB_TYPE_SWITCH_2KEY_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//设置name
			sprintf(accessory_name, "Switch-2Key-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-Switch01";
		
			/* Create accessory object */
			accessory = hap_acc_create(&bridge_cfg);
		
			/* Create the Fan Service. Include the "name" since this is a user visible service	*/
			service = hap_serv_switch_create(false);
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw1_on_ff = 0;

						
			sprintf(accessory_name, "Switch-2Key%d-1", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
=======
			
			//设置name
			sprintf(accessory_name, "Switch-2Key%d-2-1", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
			/* Set the Accessory name as the Private data for the service,
			 * so that the correct accessory can be identified in the
			 * write callback
			 */
<<<<<<< HEAD
			//hap_serv_set_priv(service, strdup(accessory_name));
			 m_mesh_gw_sub_device_list[sub_device_index].sub_id[10] = 1;
			 hap_serv_set_priv(service, m_mesh_gw_sub_device_list[sub_device_index].sub_id);
=======
			hap_serv_set_priv(service, strdup(accessory_name));
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, switch_write);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
<<<<<<< HEAD

			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.hap_char_on_handle1 = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_ON);
			
=======
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
			//第2路
			/* Create the Fan Service. Include the "name" since this is a user visible service	*/
			service = hap_serv_switch_create(false);
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw2_on_ff = 0;
			//设置name
			sprintf(accessory_name, "Switch-2Key%d-2", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
=======
			
			//设置name
			sprintf(accessory_name, "Switch-2Key%d-2-2", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
			/* Set the Accessory name as the Private data for the service,
			 * so that the correct accessory can be identified in the
			 * write callback
			 */
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_id[10] = 2;
			hap_serv_set_priv(service, m_mesh_gw_sub_device_list[sub_device_index].sub_id);
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, switch_key2_write);
=======
			hap_serv_set_priv(service, strdup(accessory_name));
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, switch_write);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
			
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.hap_char_on_handle2 = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_ON);
			
			/* Add the Accessory to the HomeKit Database */
			//设置name
			sprintf(accessory_name, "Switch-2Key-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			hap_add_bridged_accessory(accessory, hap_get_unique_aid(bridge_cfg.name));
			break;
			//3路开关
		case HAP_TYPE_SWITCH_3KEY_E:
=======
			/* Add the Accessory to the HomeKit Database */
			hap_add_bridged_accessory(accessory, hap_get_unique_aid(accessory_name));
			break;
			//3路开关
		case MESH_SUB_TYPE_SWITCH_3KEY_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//设置name
			sprintf(accessory_name, "Switch-3Key-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-Switch01";
			/* Create accessory object */
			accessory = hap_acc_create(&bridge_cfg);
		
			/* Create the Fan Service. Include the "name" since this is a user visible service	*/
			service = hap_serv_switch_create(false);
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw1_on_ff = 0;
			
			//设置name
			sprintf(accessory_name, "Switch-3Key%d-1", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
=======
			
			//设置name
			sprintf(accessory_name, "Switch-3Key%d-3-1", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
			/* Set the Accessory name as the Private data for the service,
			 * so that the correct accessory can be identified in the
			 * write callback
			 */
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_id[10] = 0;
			hap_serv_set_priv(service, m_mesh_gw_sub_device_list[sub_device_index].sub_id);
=======
			hap_serv_set_priv(service, strdup(accessory_name));
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, switch_write);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.hap_char_on_handle1 = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_ON);
			
=======
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
			//第2路
			/* Create the Fan Service. Include the "name" since this is a user visible service	*/
			service = hap_serv_switch_create(false);
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw2_on_ff = 0;
			
			//设置name
			sprintf(accessory_name, "Switch-3Key%d-2", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
=======
			
			//设置name
			sprintf(accessory_name, "Switch-3Key%d-3-2", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
			/* Set the Accessory name as the Private data for the service,
			 * so that the correct accessory can be identified in the
			 * write callback
			 */
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_id[10] = 1;
			hap_serv_set_priv(service, m_mesh_gw_sub_device_list[sub_device_index].sub_id);
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, switch_key2_write);
=======
			hap_serv_set_priv(service, strdup(accessory_name));
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, switch_write);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
			
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.hap_char_on_handle2 = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_ON);

=======
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//第3路
			/* Create the Fan Service. Include the "name" since this is a user visible service	*/
			service = hap_serv_switch_create(false);
			
			//设置name
<<<<<<< HEAD
			sprintf(accessory_name, "Switch-3Key%d-3", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw3_on_ff = 0;
=======
			sprintf(accessory_name, "Switch-3Key%d-3-3", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			/* Set the Accessory name as the Private data for the service,
			 * so that the correct accessory can be identified in the
			 * write callback
			 */
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_id[10] = 2;
			hap_serv_set_priv(service, m_mesh_gw_sub_device_list[sub_device_index].sub_id);
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, switch_key3_write);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.hap_char_on_handle3 = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_ON);
			
			/* Add the Accessory to the HomeKit Database */
			
			//设置name
			sprintf(accessory_name, "Switch-3Key-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			hap_add_bridged_accessory(accessory, hap_get_unique_aid(accessory_name));
			break;
		case HAP_TYPE_SWITCH_4KEY_E:
=======
			hap_serv_set_priv(service, strdup(accessory_name));
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, switch_write);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
			
			/* Add the Accessory to the HomeKit Database */
			hap_add_bridged_accessory(accessory, hap_get_unique_aid(accessory_name));
			break;
		case MESH_SUB_TYPE_SWITCH_4KEY_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//设置name
			sprintf(accessory_name, "Switch-4Key-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-Switch01";
		
			/* Create accessory object */
			accessory = hap_acc_create(&bridge_cfg);
		
			/* Create the Fan Service. Include the "name" since this is a user visible service	*/
			service = hap_serv_switch_create(false);
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw1_on_ff = 0;
			//设置name
			sprintf(accessory_name, "Switch-4Key%d-1", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
=======
			
			//设置name
			sprintf(accessory_name, "Switch-4Key%d-4-1", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
			/* Set the Accessory name as the Private data for the service,
			 * so that the correct accessory can be identified in the
			 * write callback
			 */
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_id[10] = 0;
			hap_serv_set_priv(service, m_mesh_gw_sub_device_list[sub_device_index].sub_id);
=======
			hap_serv_set_priv(service, strdup(accessory_name));
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, switch_write);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
			
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.hap_char_on_handle1 = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_ON);
			
			//第2路
			/* Create the Fan Service. Include the "name" since this is a user visible service	*/
			service = hap_serv_switch_create(false);
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw2_on_ff = 0;
			//设置name
			sprintf(accessory_name, "Switch-4Key%d-2", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
=======
			//第2路
			/* Create the Fan Service. Include the "name" since this is a user visible service	*/
			service = hap_serv_switch_create(false);
			
			//设置name
			sprintf(accessory_name, "Switch-4Key%d-4-2", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
			/* Set the Accessory name as the Private data for the service,
			 * so that the correct accessory can be identified in the
			 * write callback
			 */
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_id[10] = 1;
			hap_serv_set_priv(service, m_mesh_gw_sub_device_list[sub_device_index].sub_id);
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, switch_key2_write);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.hap_char_on_handle2 = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_ON);
=======
			hap_serv_set_priv(service, strdup(accessory_name));
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, switch_write);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
			//第3路
			/* Create the Fan Service. Include the "name" since this is a user visible service	*/
			service = hap_serv_switch_create(false);
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw3_on_ff = 0;
			//设置name
			sprintf(accessory_name, "Switch-4Key%d-3", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
=======
			
			//设置name
			sprintf(accessory_name, "Switch-4Key%d-4-3", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
			/* Set the Accessory name as the Private data for the service,
			 * so that the correct accessory can be identified in the
			 * write callback
			 */
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_id[10] = 2;
			hap_serv_set_priv(service, m_mesh_gw_sub_device_list[sub_device_index].sub_id);
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, switch_key3_write);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.hap_char_on_handle3 = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_ON);
=======
			hap_serv_set_priv(service, strdup(accessory_name));
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, switch_write);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2

			//第4路
			/* Create the Fan Service. Include the "name" since this is a user visible service	*/
			service = hap_serv_switch_create(false);
			
			//设置name
<<<<<<< HEAD
			sprintf(accessory_name, "Switch-4Key%d-4", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw4_on_ff = 0;
=======
			sprintf(accessory_name, "Switch-4Key%d-4-4", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			hap_serv_add_char(service, hap_char_name_create(accessory_name));
			
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			/* Set the Accessory name as the Private data for the service,
			 * so that the correct accessory can be identified in the
			 * write callback
			 */
<<<<<<< HEAD
			m_mesh_gw_sub_device_list[sub_device_index].sub_id[10] = 3;
			hap_serv_set_priv(service, m_mesh_gw_sub_device_list[sub_device_index].sub_id);
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, switch_key4_write);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
			m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.hap_char_on_handle4 = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_ON);
			/* Add the Accessory to the HomeKit Database */
			//设置name
			sprintf(accessory_name, "Switch-4Key-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			hap_add_bridged_accessory(accessory, hap_get_unique_aid(accessory_name));
			break;
		case HAP_TYPE_AUDIBLE_VISUAL_ALARM_E:
=======
			hap_serv_set_priv(service, strdup(accessory_name));
			
			/* Set the write callback for the service */
			hap_serv_set_write_cb(service, switch_write);
			
			/* Add the Fan Service to the Accessory Object */
			hap_acc_add_serv(accessory, service);
			
			/* Add the Accessory to the HomeKit Database */
			hap_add_bridged_accessory(accessory, hap_get_unique_aid(accessory_name));
			break;
		case MESH_SUB_TYPE_AUDIBLE_VISUAL_ALARM_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//设置name
			sprintf(accessory_name, "Alarm-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-Alarm01";
			break;
<<<<<<< HEAD
		case HAP_TYPE_ELECTRIC_VALVE_E:
=======
		case MESH_SUB_TYPE_ELECTRIC_VALVE_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//设置name
			sprintf(accessory_name, "Valve-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-Valve01";
			break;
<<<<<<< HEAD
		case HAP_TYPE_MAGNETIC_DOOR_SENSOR_E:
=======
		case MESH_SUB_TYPE_MAGNETIC_DOOR_SENSOR_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//设置name
			sprintf(accessory_name, "D-Sensor-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-Sensor02";
			break;
<<<<<<< HEAD
		case HAP_TYPE_IR_SENSOR_E:
=======
		case MESH_SUB_TYPE_IR_SENSOR_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//设置name
			sprintf(accessory_name, "IR-Sensor-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-Sensor03";
			break;
		case MESH_SUB_TYPE_UNDER_WATER_SENSOR_E:
			//设置name
			sprintf(accessory_name, "W-Sensor-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-Sensor04";
			break;
		case MESH_SUB_TYPE_SMOKE_SENSOR_E:
			//设置name
			sprintf(accessory_name, "Smoke-Sensor-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-Sensor05";
			break;
<<<<<<< HEAD
		case HAP_TYPE_GAS_SENSOR_E:
=======
		case MESH_SUB_TYPE_GAS_SENSOR_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			//设置name
			sprintf(accessory_name, "Gas-Sensor-%d", m_mesh_gw_sub_device_list[sub_device_index].sub_register_id);
			//设置model
			bridge_cfg.model = "Le-Sensor06";
			break;
<<<<<<< HEAD
		case HAP_TYPE_SCENE_1KEY_E:
			result_code = 1;
			break;
		case HAP_TYPE_SCENE_2KEY_E:
			result_code = 1;
			break;
		case HAP_TYPE_SCENE_3KEY_E:
			result_code = 1;
			break;
		case HAP_TYPE_SCENE_4KEY_E:
			result_code = 1;
			break;
		case HAP_TYPE_SCENE_5KEY_E:
			result_code = 1;
			break;
		case HAP_TYPE_SCENE_6KEY_E:
=======
		case MESH_SUB_TYPE_SCENE_1KEY_E:
			result_code = 1;
			break;
		case MESH_SUB_TYPE_SCENE_2KEY_E:
			result_code = 1;
			break;
		case MESH_SUB_TYPE_SCENE_3KEY_E:
			result_code = 1;
			break;
		case MESH_SUB_TYPE_SCENE_4KEY_E:
			result_code = 1;
			break;
		case MESH_SUB_TYPE_SCENE_5KEY_E:
			result_code = 1;
			break;
		case MESH_SUB_TYPE_SCENE_6KEY_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			result_code = 1;
			break;
		default:
			ESP_LOGI(TAG,"unknow sub device\r\n");
			result_code = 1;
			break;
	}

	return result_code;

}
#endif

static int m_profile_num = 0;

void get_device_list_init(void)
{
	//从内存中读取现有设备列表/向网关查询-未实现
	uint8_t i = 0;
	char key_name[9] = {'s','u','b','_','i','d','0','0','\0'};
	char key_sub_id[9] = {'s','u','b','_','i','f','0','0','\0'};
	size_t size = sizeof(m_profile_num);
	uint16_t sub_devicd_info = 0;
	
	ESP_LOGI(TAG,"get_device_list_init-");
	m_mesh_sub_device_count = 0;

	//从内存中读取已添加的设备个数,当前内存还未存在，初始化
	if (hap_factory_keystore_get("emulator", "profile", (uint8_t *)&m_profile_num, &size) != HAP_SUCCESS) 
	{
		hap_factory_keystore_set("emulator", "profile", (uint8_t *)&m_profile_num, size);
		ESP_LOGI(TAG,"INIT-");
	}
	
	ESP_LOGI(TAG,"profile NUM:%d\r\n",m_profile_num);

	//已有添加的设备,读取设备信息
	//size = sizeof(sub_devicd_info);
		
	for(i = 0; i < m_profile_num; i++)
	{
		key_name[6] = (i > 9)?(i/10+'0'):('0');
		key_name[7] = (i % 10)+'0';
		
		key_sub_id[6] = (i > 9)?(i/10+'0'):('0');
		key_sub_id[7] = (i % 10)+'0';
		ESP_LOGI(TAG,"key:%s:",key_name);

		
		size = sizeof(sub_devicd_info);
		if (hap_factory_keystore_get("emulator",key_name, (uint8_t *)&sub_devicd_info, &size) != HAP_SUCCESS) 
		{
			ESP_LOGI(TAG,"get sub info error\r\n");
		}
		else
		{
			m_mesh_gw_sub_device_list[i].sub_register_id = (sub_devicd_info >> 8);
			m_mesh_gw_sub_device_list[i].sub_type = (sub_devicd_info & 0x00ff);
			//m_mesh_sub_device_count++;
<<<<<<< HEAD
			ESP_LOGI(TAG,"%d,sub_type:%d",m_mesh_gw_sub_device_list[i].sub_register_id,m_mesh_gw_sub_device_list[i].sub_type);

			
			size = 10;
			if (hap_factory_keystore_get("emulator",key_sub_id,m_mesh_gw_sub_device_list[i].sub_id, &size) != HAP_SUCCESS) 
			{
				ESP_LOGI(TAG,"get sub id error\r\n");
=======
			ESP_LOGI(TAG,"%d,sub_type:%d\r\n",m_mesh_gw_sub_device_list[i].sub_register_id,m_mesh_gw_sub_device_list[i].sub_type);

			
			size = sizeof(m_mesh_gw_sub_device_list[0].sub_id);
			if (hap_factory_keystore_get("emulator",key_sub_id,m_mesh_gw_sub_device_list[i].sub_id, &size) != HAP_SUCCESS) 
			{
				ESP_LOGI(TAG,"get sub info error\r\n");
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			}
			else
			{
				m_mesh_gw_sub_device_list[i].sub_register_id = (sub_devicd_info >> 8);
				m_mesh_gw_sub_device_list[i].sub_type = (sub_devicd_info & 0x00ff);
				m_mesh_sub_device_count++;
<<<<<<< HEAD
				ESP_LOGI(TAG,"%d,sub_id:%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\r\n",m_mesh_gw_sub_device_list[i].sub_register_id,m_mesh_gw_sub_device_list[i].sub_id[0],
					m_mesh_gw_sub_device_list[i].sub_id[1],m_mesh_gw_sub_device_list[i].sub_id[2],m_mesh_gw_sub_device_list[i].sub_id[3],m_mesh_gw_sub_device_list[i].sub_id[4],
					m_mesh_gw_sub_device_list[i].sub_id[5],m_mesh_gw_sub_device_list[i].sub_id[6],m_mesh_gw_sub_device_list[i].sub_id[7],m_mesh_gw_sub_device_list[i].sub_id[8],
					m_mesh_gw_sub_device_list[i].sub_id[9]);
=======
				ESP_LOGI(TAG,"%d,sub_type:%d\r\n",m_mesh_gw_sub_device_list[i].sub_register_id,m_mesh_gw_sub_device_list[i].sub_type);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			}

		}

	}
	
	if(m_mesh_sub_device_count != m_profile_num)
	{
		m_profile_num = 0;  
		m_mesh_sub_device_count = 0;
		
<<<<<<< HEAD
		//擦除内存中设备相关的信息
		hap_keystore_delete_namespace("emulator");

		//重新写入
		//size = sizeof(sub_devicd_info);
		//hap_factory_keystore_set("emulator", "profile", (uint8_t *)&m_profile_num, size);
=======
		size = sizeof(sub_devicd_info);
		hap_factory_keystore_set("emulator", "profile", (uint8_t *)&m_profile_num, size);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
		ESP_LOGI(TAG,"Correct the profile_num\r\n");
	}
	
}

void query_sub_device_status(void)
{
	uint8_t query_data[7] = {0x55,0xaa,0x00,0x0b,0x00,0x00,0x0a};
	device_uart_send_data(7, query_data);
}

void update_accessories_list(void)
{
    //hap_acc_t *accessory;
    //hap_serv_t *service;
	
	 /* Initialize the HAP core */
   // hap_init(HAP_TRANSPORT_WIFI);

#if 0
    /* Initialise the mandatory parameters for Accessory which will be added as
     * the mandatory services internally
     */
    hap_acc_cfg_t cfg = {
        .name = "LeMesh-Bridge",
        .manufacturer = "LeMesh",
        .model = "LeMeshBridge01",
        .serial_num = "001122334455",
        .fw_rev = "0.9.0",
        .hw_rev = NULL,
        .pv = "1.1.0",
        .identify_routine = bridge_identify,
        .cid = HAP_CID_BRIDGE,
    };
    /* Create accessory object */
    accessory = hap_acc_create(&cfg);

    /* Add a dummy Product Data */
    uint8_t product_data[] = {'E','S','P','3','2','H','A','P'};
    hap_acc_add_product_data(accessory, product_data, sizeof(product_data));

    /* Add the Accessory to the HomeKit Database */
    hap_add_accessory(accessory);
#endif

#if 1
	
	//根据当前设备列表，创建配件
    /* Create and add the Accessory to the Bridge object*/
    for (uint8_t i = 0; i < m_mesh_sub_device_count; i++) 
    {
		add_accessory_to_bridge(i);
		
/*
		//添加成功,查询一遍当前状态
		if(!add_accessory_to_bridge(i))
		{
			//查询一遍当前的状态
			m_mesh_gw_sub_device_list[i].task_pending_f |= TASK_SUB_DEVICE_QUERY_MASK;
		}
*/
    }

#endif


}

<<<<<<< HEAD
#if 0
void convert_number_to_char(char* characters,uint16_t number)
{
	uint16_t result_code = 0;
	uint8_t i = 0;

	while(i < 4)
	{
		if((characters[i] < 0x30) || (characters[i] > 0x66) || ((characters[0] > 0x39) && (characters[0] < 0x61)))
		{
			return 0;
		}
		i++;
	}

	result_code = (characters[0] > 0x60)?(characters[0]-87):(characters[0]-0x30);
	result_code = (result_code << 12);
	i = (characters[1] > 0x60)?(characters[1]-87):(characters[1]-0x30);
	result_code |= (i << 8);
	i = (characters[2] > 0x60)?(characters[2]-87):(characters[2]-0x30);
	result_code |= (i << 4);
	i = (characters[3] > 0x60)?(characters[3]-87):(characters[3]-0x30);
	result_code |= i ;

}
#endif
=======
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2


void mesh_gw_task_perform(void)
{
	uint8_t i = 0;
	char key_name[9] = {'s','u','b','_','i','d','0','0','\0'};
	char key_sub_id[9] = {'s','u','b','_','i','f','0','0','\0'};
	uint16_t sub_devicd_info = 0;
<<<<<<< HEAD
	uint16_t data_len = 11;
	uint8_t data_offset = 17;
	uint8_t is_need_reboot_f = 0;
	uint8_t control_device_data[60] = {0x55,0xaa,0x00,0x0c,0,0,0x0a,0,};
=======
	uint8_t is_need_reboot_f = 0;
	uint8_t control_device_data[50] = {0x55,0xaa,0x00,0x0c,0,0,0x0a,0,};
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2

	//获取当前时间
	uint32_t xCurrentTickCount = xTaskGetTickCount(); 

	//判断是否有溢出
	if(xCurrentTickCount < m_mesh_sub_device_update_time)
	{
		m_mesh_sub_device_update_time = 0;
	}

	//有等待更新结束的任务
	if(m_mesh_sub_device_update_task_f)
	{
		//等待时间已到达
		if(xCurrentTickCount > (m_mesh_sub_device_update_time + 3000))
		{
			m_mesh_sub_device_update_task_f = 0;
			ESP_LOGI(TAG,"\r\nwait gw timeout%d\r\n",xCurrentTickCount);
		}

		//串口已经超时无数据更新了
		if(xCurrentTickCount > (get_device_uart_communication_time() + 1000))
		{
			ESP_LOGI(TAG,"\r\nuart recv timeout%d\r\n",xCurrentTickCount);
			m_mesh_sub_device_update_task_f = 0;
		}
		
	}

	//首次上电设备信息同步未完成,不执行其它任务
	if(m_mesh_sub_device_update_task_f)
	{
		return;
	}
	
	//有添加任务
	for(i = 0; i < m_mesh_sub_device_count;i++)
	{
		if(m_mesh_gw_sub_device_list[i].task_pending_f)
		{
			//添加任务
			if(m_mesh_gw_sub_device_list[i].task_pending_f & TASK_SUB_DEVICE_ADD_MASK)
			{
				//添加配件
				key_name[6] = (i > 9)?(i/10+'0'):('0');
				key_name[7] = (i % 10)+'0';
				key_sub_id[6] = (i > 9)?(i/10+'0'):('0');
				key_sub_id[7] = (i % 10)+'0';
				
				sub_devicd_info = (m_mesh_gw_sub_device_list[i].sub_register_id << 8)|(m_mesh_gw_sub_device_list[i].sub_type);
				
				hap_factory_keystore_set("emulator", key_name, (uint8_t *)&sub_devicd_info, sizeof(sub_devicd_info));
				hap_factory_keystore_set("emulator", key_sub_id, m_mesh_gw_sub_device_list[i].sub_id, 10);
		
				//清除任务
				m_mesh_gw_sub_device_list[i].task_pending_f &= (~(TASK_SUB_DEVICE_ADD_MASK));
				
				//重启模块
				ESP_LOGI(TAG,"need_reboot_accessory:profile_num:%d,id:%d,type:%d\r\n",m_profile_num,m_mesh_gw_sub_device_list[i].sub_register_id,m_mesh_gw_sub_device_list[i].sub_type);

				//需要重启模块
				is_need_reboot_f = 1;
				//hap_reboot_accessory();
				
			}
			//删除
			else if(m_mesh_gw_sub_device_list[i].task_pending_f & TASK_SUB_DEVICE_DELETE_MASK)
			{
				ESP_LOGI(TAG,"delete accessory from bridge\r\n");
				
				m_mesh_gw_sub_device_list[i].task_pending_f &= (~(TASK_SUB_DEVICE_DELETE_MASK));
			}
			//更新状态
			else if(m_mesh_gw_sub_device_list[i].task_pending_f & TASK_SUB_DEVICE_UPDATE_MASK)
			{
				ESP_LOGI(TAG,"update accessory status\r\n");
				m_mesh_gw_sub_device_list[i].task_pending_f &= (~(TASK_SUB_DEVICE_UPDATE_MASK));
			}
	
		}

		//状态有更新
		if(m_mesh_gw_sub_device_list[i].is_notify_f)
		{
			switch(m_mesh_gw_sub_device_list[i].sub_type)
			{
<<<<<<< HEAD
				case HAP_TYPE_DOUBLE_LIGHTBULB_E:
				case HAP_TYPE_RGB_LIGHTBULB_E:
=======
				case MESH_SUB_TYPE_DOUBLE_LIGHTBULB_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
					//开关状态更新
					if(m_mesh_gw_sub_device_list[i].is_notify_f & LIGHTBULB_NOTIFY_SW_MASK)
					{
						hap_char_update_val(m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.hap_char_on_handle, (hap_val_t*)&(m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_on_off));
						m_mesh_gw_sub_device_list[i].is_notify_f &= (~LIGHTBULB_NOTIFY_SW_MASK);
					}
					//亮度状态更新
					if(m_mesh_gw_sub_device_list[i].is_notify_f & LIGHTBULB_NOTIFY_BRIGHTNESS_MASK)
					{
						hap_char_update_val(m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.hap_char_brightness_handle, (hap_val_t*)&(m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_brightness));
						m_mesh_gw_sub_device_list[i].is_notify_f &= (~LIGHTBULB_NOTIFY_BRIGHTNESS_MASK);
					}
					//色温状态更新
					if(m_mesh_gw_sub_device_list[i].is_notify_f & LIGHTBULB_NOTIFY_COLOR_TEMP_MASK)
					{
						hap_char_update_val(m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.hap_char_color_temp_handle ,(hap_val_t*)&(m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_color_temp));
						m_mesh_gw_sub_device_list[i].is_notify_f &= (~LIGHTBULB_NOTIFY_COLOR_TEMP_MASK);
					}
<<<<<<< HEAD
					//hue状态更新
					if(m_mesh_gw_sub_device_list[i].is_notify_f & LIGHTBULB_NOTIFY_HUE_MASK)
					{
						hap_char_update_val(m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.hap_char_hue_handle, (hap_val_t*)&(m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_hue));
						m_mesh_gw_sub_device_list[i].is_notify_f &= (~LIGHTBULB_NOTIFY_HUE_MASK);
					}
					//saturation状态更新
					if(m_mesh_gw_sub_device_list[i].is_notify_f & LIGHTBULB_NOTIFY_SATURATION_MASK)
					{
						hap_char_update_val(m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.hap_char_saturation_handle ,(hap_val_t*)&(m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_saturation));
						m_mesh_gw_sub_device_list[i].is_notify_f &= (~LIGHTBULB_NOTIFY_SATURATION_MASK);
					}
					break;
				case HAP_TYPE_SW_CONTROLLER_E:
				case HAP_TYPE_SWITCH_1KEY_E:
				case HAP_TYPE_SWITCH_2KEY_E:
				case HAP_TYPE_SWITCH_3KEY_E:
				case HAP_TYPE_SWITCH_4KEY_E:
					//开关状态更新
					if(m_mesh_gw_sub_device_list[i].is_notify_f & SWITCH_NOTIFY_SW1_MASK)
					{
						hap_char_update_val(m_mesh_gw_sub_device_list[i].sub_device.sub_switch.hap_char_on_handle1, (hap_val_t*)&(m_mesh_gw_sub_device_list[i].sub_device.sub_switch.sw1_on_ff));
						m_mesh_gw_sub_device_list[i].is_notify_f &= (~SWITCH_NOTIFY_SW1_MASK);
					}

					if(m_mesh_gw_sub_device_list[i].is_notify_f & SWITCH_NOTIFY_SW2_MASK)
					{
						hap_char_update_val(m_mesh_gw_sub_device_list[i].sub_device.sub_switch.hap_char_on_handle2, (hap_val_t*)&(m_mesh_gw_sub_device_list[i].sub_device.sub_switch.sw2_on_ff));
						m_mesh_gw_sub_device_list[i].is_notify_f &= (~SWITCH_NOTIFY_SW2_MASK);
					}
					if(m_mesh_gw_sub_device_list[i].is_notify_f & SWITCH_NOTIFY_SW3_MASK)
					{
						hap_char_update_val(m_mesh_gw_sub_device_list[i].sub_device.sub_switch.hap_char_on_handle3, (hap_val_t*)&(m_mesh_gw_sub_device_list[i].sub_device.sub_switch.sw3_on_ff));
						m_mesh_gw_sub_device_list[i].is_notify_f &= (~SWITCH_NOTIFY_SW3_MASK);
					}
					if(m_mesh_gw_sub_device_list[i].is_notify_f & SWITCH_NOTIFY_SW4_MASK)
					{
						hap_char_update_val(m_mesh_gw_sub_device_list[i].sub_device.sub_switch.hap_char_on_handle4, (hap_val_t*)&(m_mesh_gw_sub_device_list[i].sub_device.sub_switch.sw4_on_ff));
						m_mesh_gw_sub_device_list[i].is_notify_f &= (~SWITCH_NOTIFY_SW4_MASK);
					}
				
					break;
				case HAP_TYPE_ELECTRIC_CURTAINS_E:
					//目的状态更新
					if(m_mesh_gw_sub_device_list[i].is_notify_f & CURTAIN_NOTIFY_TARGET_POS_MASK)
					{
						hap_char_update_val(m_mesh_gw_sub_device_list[i].sub_device.sub_curtain.hap_char_target_position_handle, (hap_val_t*)&(m_mesh_gw_sub_device_list[i].sub_device.sub_curtain.target_position));
						m_mesh_gw_sub_device_list[i].is_notify_f &= (~CURTAIN_NOTIFY_TARGET_POS_MASK);
					}

					//当前状态更新
					if(m_mesh_gw_sub_device_list[i].is_notify_f & CURTAIN_NOTIFY_CURRENT_POS_MASK)
					{
						m_mesh_gw_sub_device_list[i].sub_device.sub_curtain.current_position = m_mesh_gw_sub_device_list[i].sub_device.sub_curtain.target_position;
						ESP_LOGI(TAG,"UPDATE CHAR CURRENT POS");
						hap_char_update_val(m_mesh_gw_sub_device_list[i].sub_device.sub_curtain.hap_char_current_position_handle, (hap_val_t*)&(m_mesh_gw_sub_device_list[i].sub_device.sub_curtain.current_position));
						m_mesh_gw_sub_device_list[i].is_notify_f &= (~CURTAIN_NOTIFY_CURRENT_POS_MASK);
					}
					
					//位置状态
					if(m_mesh_gw_sub_device_list[i].is_notify_f & CURTAIN_NOTIFY_POS_STATE_MASK)
					{
						ESP_LOGI(TAG,"UPDATE CHAR POS");
						m_mesh_gw_sub_device_list[i].sub_device.sub_curtain.position_state = 2;
						hap_char_update_val(m_mesh_gw_sub_device_list[i].sub_device.sub_curtain.hap_char_position_state_handle, (hap_val_t*)&(m_mesh_gw_sub_device_list[i].sub_device.sub_curtain.position_state));
						m_mesh_gw_sub_device_list[i].is_notify_f &= (~CURTAIN_NOTIFY_POS_STATE_MASK);
					}
=======
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
					break;
				default:
					m_mesh_gw_sub_device_list[i].is_notify_f = 0;
					break;
			}
		}

		/*控制任务*/
		if(m_mesh_gw_sub_device_list[i].is_control_f)
		{
<<<<<<< HEAD
			data_offset = 17;
			data_len = 11;
=======
			
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			memcpy(&control_device_data[7],m_mesh_gw_sub_device_list[i].sub_id,10);
			
			switch(m_mesh_gw_sub_device_list[i].sub_type)
			{
<<<<<<< HEAD
#if 0
				case HAP_TYPE_DOUBLE_LIGHTBULB_E:
=======
				case MESH_SUB_TYPE_DOUBLE_LIGHTBULB_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
					//设置DATA_LEN
					sub_devicd_info = 11+21;
					control_device_data[4] = sub_devicd_info >> 8;
					control_device_data[5] = sub_devicd_info;

					//设置数据点-------------------------------
					//开关状态
					control_device_data[17] = DPID_LIGHT_SW;
					control_device_data[18] = DPTYPE_BOOL;
					control_device_data[19] = 0;
					control_device_data[20] = 1;
					control_device_data[21] = (m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_on_off)?(1):(0);

					//亮度
					control_device_data[22] = DPID_LIGHT_BRIGHTNESS;
					control_device_data[23] = DPTYPE_VALUE;
					control_device_data[24] = 0;
					control_device_data[25] = 4;
					sub_devicd_info = m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_brightness*10;
					control_device_data[26] = 0;
					control_device_data[27] = 0;
					control_device_data[28] = sub_devicd_info >> 8;
					if(!sub_devicd_info)
					{
						control_device_data[29] = 5;
					}
					else
					{
						control_device_data[29] = sub_devicd_info;
					}

					//色温(8000-value*20)/7
					control_device_data[30] = DPID_LIGHT_COLOR_TEMP;
					control_device_data[31] = DPTYPE_VALUE;
					control_device_data[32] = 0;
					control_device_data[33] = 4;

					if(m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_color_temp < 50)
					{
						m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_color_temp = 50;
					}
					
					sub_devicd_info = (8000-m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_color_temp*20)/7;
					control_device_data[34] = 0;
					control_device_data[35] = 0;
					control_device_data[36] = sub_devicd_info >> 8;
					control_device_data[37] = sub_devicd_info;
					control_device_data[38] = calculate_checksum_result(control_device_data, 38);

					device_uart_send_data(39, control_device_data);

<<<<<<< HEAD
					m_mesh_gw_sub_device_list[i].is_control_f = 0;
					break;
				case HAP_TYPE_RGB_LIGHTBULB_E:
					//设置DATA_LEN
					sub_devicd_info = 11+29;
					control_device_data[4] = sub_devicd_info >> 8;
					control_device_data[5] = sub_devicd_info;

					//设置数据点-------------------------------
					//开关状态
					control_device_data[17] = DPID_LIGHT_SW;
					control_device_data[18] = DPTYPE_BOOL;
					control_device_data[19] = 0;
					control_device_data[20] = 1;
					control_device_data[21] = (m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_on_off)?(1):(0);

					//亮度
					control_device_data[22] = DPID_LIGHT_BRIGHTNESS;
					control_device_data[23] = DPTYPE_VALUE;
					control_device_data[24] = 0;
					control_device_data[25] = 4;
					sub_devicd_info = m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_brightness*10;
					control_device_data[26] = 0;
					control_device_data[27] = 0;
					control_device_data[28] = sub_devicd_info >> 8;
					if(!sub_devicd_info)
					{
						control_device_data[29] = 5;
					}
					else
					{
						control_device_data[29] = sub_devicd_info;
					}

					//RGB-HSV
					control_device_data[30] = DPID_LIGHT_HSV;
					control_device_data[31] = DPTYPE_STRING;
					control_device_data[32] = 0;
					control_device_data[33] = 12;

					sprintf((char*)&control_device_data[34],"%04x%04x%04x",m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_hue,m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_saturation*10,m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_brightness*10);
					
					control_device_data[46] = calculate_checksum_result(control_device_data, 46);

					device_uart_send_data(47, control_device_data);

					m_mesh_gw_sub_device_list[i].is_control_f = 0;
					break;
#endif
				case HAP_TYPE_SINGLE_LIGHTBULB_E:
				case HAP_TYPE_DOUBLE_LIGHTBULB_E:
				case HAP_TYPE_RGB_LIGHTBULB_E:
				case HAP_TYPE_WYRGB_LIGHTBULB_E:
					//开关
					if(m_mesh_gw_sub_device_list[i].is_control_f & LIGHTBULB_NOTIFY_SW_MASK)
					{
						data_len += 5;

						control_device_data[data_offset++] = DPID_LIGHT_SW;
						control_device_data[data_offset++] = DPTYPE_BOOL;
						control_device_data[data_offset++] = 0;
						control_device_data[data_offset++] = 1;
						control_device_data[data_offset++] = (m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_on_off)?(1):(0);
					}

					//亮度
					if(m_mesh_gw_sub_device_list[i].is_control_f & LIGHTBULB_NOTIFY_BRIGHTNESS_MASK)
					{
						data_len += 8;
						
						control_device_data[data_offset++] = DPID_LIGHT_BRIGHTNESS;
						control_device_data[data_offset++] = DPTYPE_VALUE;
						control_device_data[data_offset++] = 0;
						control_device_data[data_offset++] = 4;
						sub_devicd_info = m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_brightness*10;
						control_device_data[data_offset++] = 0;
						control_device_data[data_offset++] = 0;
						control_device_data[data_offset++] = sub_devicd_info >> 8;
						if(!sub_devicd_info)
						{
							control_device_data[data_offset++] = 5;
						}
						else
						{
							control_device_data[data_offset++] = sub_devicd_info;
						}
					}

					//色温
					if(m_mesh_gw_sub_device_list[i].is_control_f & LIGHTBULB_NOTIFY_COLOR_TEMP_MASK)
					{
						data_len += 8;
						control_device_data[data_offset++] = DPID_LIGHT_COLOR_TEMP;
						control_device_data[data_offset++] = DPTYPE_VALUE;
						control_device_data[data_offset++] = 0;
						control_device_data[data_offset++] = 4;

						if(m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_color_temp < 75)
						{
							m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_color_temp = 50;
						}
						
						sub_devicd_info = (8000-m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_color_temp*20)/7;
						control_device_data[data_offset++] = 0;
						control_device_data[data_offset++] = 0;
						control_device_data[data_offset++] = sub_devicd_info >> 8;
						control_device_data[data_offset++] = sub_devicd_info;
					}

					if((m_mesh_gw_sub_device_list[i].is_control_f & LIGHTBULB_NOTIFY_HUE_MASK) || (m_mesh_gw_sub_device_list[i].is_control_f & LIGHTBULB_NOTIFY_SATURATION_MASK))
					{
						data_len += 16;
						control_device_data[data_offset++] = DPID_LIGHT_HSV;
						control_device_data[data_offset++] = DPTYPE_STRING;
						control_device_data[data_offset++] = 0;
						control_device_data[data_offset++] = 12;

						sprintf((char*)&control_device_data[data_offset],"%04x%04x%04x",m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_hue,m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_saturation*10,m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.lamp_brightness*10);
						data_offset += 12;						
					}

					//辅灯1开关
					if(m_mesh_gw_sub_device_list[i].is_control_f & LIGHTBULB_NOTIFY_A1_MASK)
					{
						data_len += 5;

						control_device_data[data_offset++] = DPID_LIGHT_AUXILIARY1_SW;
						control_device_data[data_offset++] = DPTYPE_BOOL;
						control_device_data[data_offset++] = 0;
						control_device_data[data_offset++] = 1;
						control_device_data[data_offset++] = (m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.auxiliary_lamp1_sw)?(1):(0);
					}

					//辅灯2开关
					if(m_mesh_gw_sub_device_list[i].is_control_f & LIGHTBULB_NOTIFY_A2_MASK)
					{
						data_len += 5;

						control_device_data[data_offset++] = DPID_LIGHT_AUXILIARY2_SW;
						control_device_data[data_offset++] = DPTYPE_BOOL;
						control_device_data[data_offset++] = 0;
						control_device_data[data_offset++] = 1;
						control_device_data[data_offset++] = (m_mesh_gw_sub_device_list[i].sub_device.sub_lightbulb.auxiliary_lamp2_sw)?(1):(0);
					}

					
					control_device_data[4] = data_len >> 8;
					control_device_data[5] = data_len;
					control_device_data[data_offset] = calculate_checksum_result(control_device_data, data_offset);
					
					//device_uart_send_data(data_offset+1, control_device_data);
					
					sub_devicd_info = 1;
					break;

				case HAP_TYPE_SW_CONTROLLER_E:
				case HAP_TYPE_SWITCH_1KEY_E:
				case HAP_TYPE_SWITCH_2KEY_E:
				case HAP_TYPE_SWITCH_3KEY_E:
				case HAP_TYPE_SWITCH_4KEY_E:

					//1路开关
					if(m_mesh_gw_sub_device_list[i].is_control_f & SWITCH_NOTIFY_SW1_MASK)
					{
						data_len += 5;

						control_device_data[data_offset++] = DPID_SWITCH_KEY1_SW;
						control_device_data[data_offset++] = DPTYPE_BOOL;
						control_device_data[data_offset++] = 0;
						control_device_data[data_offset++] = 1;
						control_device_data[data_offset++] = (m_mesh_gw_sub_device_list[i].sub_device.sub_switch.sw1_on_ff)?(1):(0);
					}

					//2路开关
					if(m_mesh_gw_sub_device_list[i].is_control_f & SWITCH_NOTIFY_SW2_MASK)
					{
						data_len += 5;

						control_device_data[data_offset++] = DPID_SWITCH_KEY2_SW;
						control_device_data[data_offset++] = DPTYPE_BOOL;
						control_device_data[data_offset++] = 0;
						control_device_data[data_offset++] = 1;
						control_device_data[data_offset++] = (m_mesh_gw_sub_device_list[i].sub_device.sub_switch.sw2_on_ff)?(1):(0);
					}

					//3路开关
					if(m_mesh_gw_sub_device_list[i].is_control_f & SWITCH_NOTIFY_SW3_MASK)
					{
						data_len += 5;

						control_device_data[data_offset++] = DPID_SWITCH_KEY3_SW;
						control_device_data[data_offset++] = DPTYPE_BOOL;
						control_device_data[data_offset++] = 0;
						control_device_data[data_offset++] = 1;
						control_device_data[data_offset++] = (m_mesh_gw_sub_device_list[i].sub_device.sub_switch.sw3_on_ff)?(1):(0);
					}

					//4路开关
					if(m_mesh_gw_sub_device_list[i].is_control_f & SWITCH_NOTIFY_SW4_MASK)
					{
						data_len += 5;

						control_device_data[data_offset++] = DPID_SWITCH_KEY4_SW;
						control_device_data[data_offset++] = DPTYPE_BOOL;
						control_device_data[data_offset++] = 0;
						control_device_data[data_offset++] = 1;
						control_device_data[data_offset++] = (m_mesh_gw_sub_device_list[i].sub_device.sub_switch.sw4_on_ff)?(1):(0);
					}
					
					control_device_data[4] = data_len >> 8;
					control_device_data[5] = data_len;

					
					control_device_data[data_offset] = calculate_checksum_result(control_device_data, data_offset);
					
					//device_uart_send_data(data_offset+1, control_device_data);

					sub_devicd_info = 1;
					break;
					
				case HAP_TYPE_ELECTRIC_CURTAINS_E:
					data_len += 8;
					control_device_data[data_offset++] = DPID_CURTAIN_TARGET_STATE;
					control_device_data[data_offset++] = DPTYPE_VALUE;
					control_device_data[data_offset++] = 0;
					control_device_data[data_offset++] = 4;
					
					control_device_data[data_offset++] = 0;
					control_device_data[data_offset++] = 0;
					control_device_data[data_offset++] = 0;
					control_device_data[data_offset++] = 100 - m_mesh_gw_sub_device_list[i].sub_device.sub_curtain.target_position;

					
					control_device_data[4] = data_len >> 8;
					control_device_data[5] = data_len;
					
					control_device_data[data_offset] = calculate_checksum_result(control_device_data, data_offset);
					
					//device_uart_send_data(data_offset+1, control_device_data);
					
					//m_mesh_gw_sub_device_list[i].is_control_f = 0;
					sub_devicd_info = 1;
					m_mesh_gw_sub_device_list[i].is_notify_f |= CURTAIN_NOTIFY_POS_STATE_MASK;
					m_mesh_gw_sub_device_list[i].is_notify_f |= CURTAIN_NOTIFY_CURRENT_POS_MASK;
					break;
				
#if 0
				case HAP_TYPE_SWITCH_2KEY_E:
				case HAP_TYPE_SWITCH_3KEY_E:
				case HAP_TYPE_SWITCH_4KEY_E:
					//设置DATA_LEN
					sub_devicd_info = 11+5+5*(m_mesh_gw_sub_device_list[i].sub_type-HAP_TYPE_SWITCH_1KEY_E);
					control_device_data[4] = sub_devicd_info >> 8;
					control_device_data[5] = sub_devicd_info;

					//设置数据点-------------------------------
					//开关状态
					control_device_data[17] = DPID_SWITCH_KEY1_SW;
					control_device_data[18] = DPTYPE_BOOL;
					control_device_data[19] = 0;
					control_device_data[20] = 1;
					control_device_data[21] = (m_mesh_gw_sub_device_list[i].sub_device.sub_switch.sw1_on_ff)?(1):(0);

					//开关状态
					control_device_data[22] = DPID_SWITCH_KEY2_SW;
					control_device_data[23] = DPTYPE_BOOL;
					control_device_data[24] = 0;
					control_device_data[25] = 1;
					control_device_data[26] = (m_mesh_gw_sub_device_list[i].sub_device.sub_switch.sw2_on_ff)?(1):(0);

					if(m_mesh_gw_sub_device_list[i].sub_type > HAP_TYPE_SWITCH_2KEY_E)
					{
						//开关状态
						control_device_data[27] = DPID_SWITCH_KEY3_SW;
						control_device_data[28] = DPTYPE_BOOL;
						control_device_data[29] = 0;
						control_device_data[30] = 1;
						control_device_data[31] = (m_mesh_gw_sub_device_list[i].sub_device.sub_switch.sw3_on_ff)?(1):(0);
					}

					if(m_mesh_gw_sub_device_list[i].sub_type > HAP_TYPE_SWITCH_3KEY_E)
					{
						//开关状态
						control_device_data[32] = DPID_SWITCH_KEY4_SW;
						control_device_data[33] = DPTYPE_BOOL;
						control_device_data[34] = 0;
						control_device_data[35] = 1;
						control_device_data[36] = (m_mesh_gw_sub_device_list[i].sub_device.sub_switch.sw4_on_ff)?(1):(0);
					}

					control_device_data[sub_devicd_info+6] = calculate_checksum_result(control_device_data, sub_devicd_info+6);
					
					device_uart_send_data(sub_devicd_info+7, control_device_data);

					m_mesh_gw_sub_device_list[i].is_control_f = 0;
					break;
#endif
				default:
					//m_mesh_gw_sub_device_list[i].is_control_f = 0;
					sub_devicd_info = 0;
					
					break;
			}

			
			ESP_LOGI(TAG, "send control data:%d",data_offset+1);

			if(sub_devicd_info)
			{
				for(uint8_t j = 0;j < (data_offset+1); j++)
				{
					printf("%02x ",control_device_data[j]);
				}
				
				printf("\r\n");
			}

			device_uart_send_data(data_offset+1, control_device_data);
	
		
			m_mesh_gw_sub_device_list[i].is_control_f = 0;
=======
					//开关状态更新
					//if(m_mesh_gw_sub_device_list[i].is_control_f & LIGHTBULB_NOTIFY_SW_MASK)
					//{
						m_mesh_gw_sub_device_list[i].is_control_f &= (~LIGHTBULB_NOTIFY_SW_MASK);
					//}
					//亮度状态更新
					//if(m_mesh_gw_sub_device_list[i].is_control_f & LIGHTBULB_NOTIFY_BRIGHTNESS_MASK)
					//{
						m_mesh_gw_sub_device_list[i].is_control_f &= (~LIGHTBULB_NOTIFY_BRIGHTNESS_MASK);
					//}
					//色温状态更新
					//if(m_mesh_gw_sub_device_list[i].is_control_f & LIGHTBULB_NOTIFY_COLOR_TEMP_MASK)
					//{
						m_mesh_gw_sub_device_list[i].is_control_f &= (~LIGHTBULB_NOTIFY_COLOR_TEMP_MASK);
					//}
					break;
				default:
					m_mesh_gw_sub_device_list[i].is_control_f = 0;
					break;
			}
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
		}
	}

	
	//重启模块
	if(is_need_reboot_f)
	{
		//写入到内存
		if(m_profile_num != m_mesh_sub_device_count)
		{
			m_profile_num = m_mesh_sub_device_count;
			hap_factory_keystore_set("emulator", "profile", (uint8_t *)&m_profile_num, sizeof(m_profile_num));
		}
		
		ESP_LOGI(TAG,"hap_reboot_accessory");
	
		hap_reboot_accessory();
	}
}
/** 
 * @brief:   [mesh gw模块的相关任务处理]
 * @Author:   xunuo
 * @DateTime: 2021年11月6日T17:00:56+0800
 * @param-void[]:                     [description]
 */
void mesh_gw_task_handle(void* context)
{
	ESP_LOGI(TAG,"mesh_gw_task memory: %u\r\n",esp_get_free_heap_size());
	
	while(1)
	{
		
		uart1_rx_task_perform();
		uart1_tx_task_handle();
		mesh_gw_task_perform();
    	vTaskDelay(100);
		
	}

    vTaskDelete(NULL);
}
/** 
 * @brief:   [初始化mesh gw模块相关资源]
 * @Author:   xunuo
 * @DateTime: 2021年11月6日T17:00:03+0800
 * @param-void[]:                     [description]
 */
void mesh_gw_init(void)
{
	
	//硬件资源初始化
	device_uart_init();

	//从内存中获取已添加的所有设备信息
	get_device_list_init();

	//为设备创建对应配件，添加到 HAP database
	update_accessories_list();

	//查询设备状态，用于设备信息同步-------------
	//挂起查询任务
	
	m_mesh_sub_device_update_task_f = 1;
	//记下查询时间
	m_mesh_sub_device_update_time = xTaskGetTickCount();
	
	ESP_LOGI(TAG,"\r\nstart wait mesh gw data%d\r\n",m_mesh_sub_device_update_time);
	query_sub_device_status();
	
<<<<<<< HEAD
}

=======
	

}
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2

void mesh_gw_start(void)
{
	//创建网关任务
	xTaskCreate(mesh_gw_task_handle, "mesh_gw_task", 1024*4, NULL, configMAX_PRIORITIES, NULL);
}

/** 
 * @brief:   [添加子设备]
 * @Author:   xunuo
 * @DateTime: 2021年11月9日T9:44:56+0800
 * @param-sub_type[IN]:                 [子设备类型]
 * @param-sub_id[IN]:                   [子设备ID]
 */
void add_sub_device(uint8_t sub_type,uint8_t sub_id)
{
	uint8_t i = 0;
	
	//当前可以添加
	if(m_mesh_sub_device_count < MESH_GW_SUPPORT_MAX_DEVICE_NUM)
	{
		//当前没有重复
		for(i = 0; i < m_mesh_sub_device_count; i++)
		{
			//
			if((m_mesh_gw_sub_device_list[i].sub_register_id == sub_id) && (m_mesh_gw_sub_device_list[i].sub_type == sub_type))
			{
				break;
			}
		}

		//未找到,加入列表
		if(i == m_mesh_sub_device_count)
		{
			m_mesh_gw_sub_device_list[i].sub_register_id = sub_id;
			m_mesh_gw_sub_device_list[i].sub_type = sub_type;
			
			//挂起任务
			m_mesh_gw_sub_device_list[i].task_pending_f |= TASK_SUB_DEVICE_ADD_MASK;
			m_mesh_sub_device_count++;
		}
	}
}

void delete_sub_device(uint8_t sub_type, uint8_t sub_id)
{
	uint8_t i = 0;
	//mesh_device_t temp_sub_device = {0};

	//当前可以删除
	if(m_mesh_sub_device_count)
	{
		//当前没有重复
		for(i = 0; i < m_mesh_sub_device_count; i++)
		{
			//找到
			if((m_mesh_gw_sub_device_list[i].sub_register_id == sub_id) && (m_mesh_gw_sub_device_list[i].sub_register_id == sub_type))
			{
				//当前不为最后一个设备
				if(i != (m_mesh_sub_device_count - 1))
				{
					
				}
				//当前为最后一个设备
				else
				{
					//直接删除 
					m_mesh_gw_sub_device_list[i].task_pending_f |= TASK_SUB_DEVICE_DELETE_MASK;
				}
				break;
			}
		}

		//未找到,加入列表
		if(i == m_mesh_sub_device_count)
		{
			i++;
			m_mesh_sub_device_count++;
			m_mesh_gw_sub_device_list[i].sub_register_id = sub_id;
			m_mesh_gw_sub_device_list[i].sub_type = sub_type;
			//挂起任务
			m_mesh_gw_sub_device_list[i].task_pending_f |= TASK_SUB_DEVICE_ADD_MASK;
		}
	}
}


/** 
<<<<<<< HEAD
 * @brief:   [检查某个数据点是否存在]
=======
 * @brief:   [获取数据点的个数]
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
 * @Author:   xunuo
 * @DateTime: 2021年11月15日T15:44:56+0800
 * @param-dataponit_data[IN]:           [数据点内容]
 * @param-data_len[IN]:                 [数据长度]
<<<<<<< HEAD
 * @param-dpid[IN]:              		[要查找的数据点]
 * @return:                            [数据点个数]
 */
uint8_t is_datapoint_exist(uint8_t* dataponit_data,uint8_t data_len,uint8_t dpid)
=======
 * @return:                            [数据点个数]
 */
uint8_t get_datapoint_num(uint8_t* dataponit_data,uint8_t data_len)
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
{
	uint8_t i = 0;
	uint8_t result_code = 0;
	datapoint_info_t* temp_datapoint = NULL;
	uint16_t temp_d_len = 0;

	for(i = 0; i < data_len;)
	{
		temp_datapoint = (datapoint_info_t*)(&dataponit_data[i]);

		temp_d_len = (temp_datapoint->d_len[0] << 8)|temp_datapoint->d_len[1];
		
		if((temp_d_len+4+i) <= data_len)
		{
<<<<<<< HEAD
			if(temp_datapoint->d_pid == dpid)
			{
				result_code = 1;
				break;
			}
=======
			result_code++;
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
		}

		i += (temp_d_len+4);
	}

	return result_code;
}

/** 
<<<<<<< HEAD
 * @brief:   [根据sub id中的@MESH_SUB_DEVICE_TYPE 字段获取新设备的 HomeKit设备类型 @HAP_DEVICE_TYPE_e]
 * @Author:   xunuo
 * @DateTime: 2021年11月15日T14:39:24+0800
 * @param-sub_device_data[IN]:                   [子设备数据：sub id+ datapoint]
 * @param-dpoint_len[IN]:                  		 [datapoint字段总长度]
 * @return:                            			[子设备类型 0-unknow 其它-refer to @HAP_DEVICE_TYPE_e]
=======
 * @brief:   [根据sub id获取新设备的设备类型]
 * @Author:   xunuo
 * @DateTime: 2021年11月15日T14:39:24+0800
 * @param-sub_device_data[IN]:                   [子设备数据：sub id+ datapoint]
 * @param-dpoint_len[IN]:                  		 [datapointc长度]
 * @return:                            				[设备类型 0-unknow]
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
 */
uint8_t get_sub_device_type_by_id(uint8_t* sub_device_data,uint8_t dpoint_len)
{
	uint16_t temp_type = (sub_device_data[0] << 8)| sub_device_data[1];
	uint8_t result_code = 0;

	switch(temp_type)
	{
		case SUB_TYPE_SINGLE_LIGHTBULB:
<<<<<<< HEAD
			result_code = HAP_TYPE_SINGLE_LIGHTBULB_E;
			break;
		case SUB_TYPE_DOUBLE_LIGHTBULB:
			result_code = HAP_TYPE_DOUBLE_LIGHTBULB_E;
			break;
		case SUB_TYPE_RGB_LIGHTBULB:
			result_code = HAP_TYPE_RGB_LIGHTBULB_E;
			break;
		case SUB_TYPE_WYRGB_LIGHTBULB:
			result_code = HAP_TYPE_WYRGB_LIGHTBULB_E;
			break;
		case SUB_TYPE_SW_CONTROLLER:
		case SUB_TYPE_SW_CONTROLLER_COMPATIBLE:
			result_code = HAP_TYPE_SW_CONTROLLER_E;
			break;
		case SUB_TYPE_SWITCH_KEY:
			//根据数据点个数获取是几开面板
			result_code = is_datapoint_exist(&sub_device_data[10],dpoint_len,DPID_SWITCH_KEY1_SW);
			result_code += is_datapoint_exist(&sub_device_data[10],dpoint_len,DPID_SWITCH_KEY2_SW);
			result_code += is_datapoint_exist(&sub_device_data[10],dpoint_len,DPID_SWITCH_KEY3_SW);
			result_code += is_datapoint_exist(&sub_device_data[10],dpoint_len,DPID_SWITCH_KEY4_SW);
=======
			result_code = MESH_SUB_TYPE_SINGLE_LIGHTBULB_E;
			break;
		case SUB_TYPE_DOUBLE_LIGHTBULB:
			result_code = MESH_SUB_TYPE_DOUBLE_LIGHTBULB_E;
			break;
		case SUB_TYPE_RGB_LIGHTBULB:
			result_code = MESH_SUB_TYPE_RGB_LIGHTBULB_E;
			break;
		case SUB_TYPE_WYRGB_LIGHTBULB:
			result_code = MESH_SUB_TYPE_WYRGB_LIGHTBULB_E;
			break;
		case SUB_TYPE_SW_CONTROLLER:
			result_code = MESH_SUB_TYPE_SW_CONTROLLER_E;
			break;
		case SUB_TYPE_SWITCH_KEY:
			//根据数据点个数获取是几开面板
			result_code = get_datapoint_num(&sub_device_data[10],dpoint_len);
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
			if(result_code && result_code <= 4)
			{
				result_code -= 1;
<<<<<<< HEAD
				result_code += HAP_TYPE_SWITCH_1KEY_E;
=======
				result_code += MESH_SUB_TYPE_SWITCH_1KEY_E;
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			}
			else
			{
				result_code = 0;
			}
			
			break;
		case SUB_TYPE_SMART_OUTLET:
<<<<<<< HEAD
			result_code = HAP_TYPE_SMART_OUTLET_E;
			break;
		case SUB_TYPE_CURTAIN:
		//case SUB_TYPE_GAUZE:
			result_code = HAP_TYPE_ELECTRIC_CURTAINS_E;
=======
			result_code = MESH_SUB_TYPE_SMART_OUTLET_E;
			break;
		case SUB_TYPE_CURTAIN:
		//case SUB_TYPE_GAUZE:
			result_code = MESH_SUB_TYPE_ELECTRIC_CURTAINS_E;
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			break;
			
	}

	return result_code;
}

/** 
 * @brief:   [获取指定dpid对应的value]
 * @Author:   xunuo
 * @DateTime: 2021年11月15日T16:42:25+0800
 * @param-d_pid[IN]:                    [指定的dpid]
 * @param-dpoint_data[IN]:              [datapoint数据]
 * @param-data_len[IN]:                 [datapoint数据长度]
<<<<<<< HEAD
 * @param-get_value[OUT]:               [获取到的value]
 * @return:                            [0-未找到 其它-找到]
 */
uint8_t get_bool_value_by_dpid(uint8_t d_pid,uint8_t* dpoint_data,uint8_t data_len,uint8_t* get_value)
=======
 * @return:                            [value]
 */
uint8_t get_bool_value_by_dpid(uint8_t d_pid,uint8_t* dpoint_data,uint8_t data_len)
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
{
	uint8_t i = 0;
	uint8_t result_code = 0;
	datapoint_info_t* temp_datapoint = NULL;
	uint16_t temp_p_dlen = 0;

<<<<<<< HEAD
//	ESP_LOGI(TAG,"start get bool\r\n");
=======
	ESP_LOGI(TAG,"start get bool\r\n");
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
	for(i = 0; i < data_len;)
	{
		temp_datapoint = (datapoint_info_t*)(&dpoint_data[i]);

		temp_p_dlen = (temp_datapoint->d_len[0] <<8) | temp_datapoint->d_len[1];

		
		if((temp_p_dlen +4+i) <= data_len)
		{
			if(temp_datapoint->d_pid == d_pid)
			{
/*
				printf("dpid:%d,d_type:%d,d_len:%d,d_value:",temp_datapoint->d_pid,temp_datapoint->d_type,temp_p_dlen);
				while(result_code < temp_p_dlen)
				{
					printf("%d ",dpoint_data[i+4+result_code]);
					result_code++;
				}
*/
				
<<<<<<< HEAD
				result_code = 1;
				*get_value = dpoint_data[i+4];
=======
				result_code = dpoint_data[i+4];
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
				break;
			}
		}

		i += (temp_p_dlen+4);
	}
	
<<<<<<< HEAD
//	ESP_LOGI(TAG,"stop get bool\r\n");
=======
	ESP_LOGI(TAG,"stop get bool\r\n");
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2

	return result_code;
}

/** 
 * @brief:   [获取指定dpid对应的value]
 * @Author:   xunuo
 * @DateTime: 2021年11月15日T16:42:25+0800
 * @param-d_pid[IN]:                    [指定的dpid]
 * @param-dpoint_data[IN]:              [datapoint数据]
 * @param-data_len[IN]:                 [datapoint数据长度]
<<<<<<< HEAD
 * @param-get_value[OUT]:               [获取到的value]
 * @return:                            [高字节：0-未找到该指定的dpid，其它-找到]
 */
uint8_t get_int_value_by_dpid(uint8_t d_pid,uint8_t* dpoint_data,uint8_t data_len,uint32_t* get_value)
{
	uint8_t i = 0;
	uint8_t result_code = 0;
	datapoint_info_t* temp_datapoint = NULL;
	uint16_t temp_p_dlen = 0;
	
//	ESP_LOGI(TAG,"start get int\r\n");
=======
 * @return:                            [高字节：0-未找到该指定的dpid，其它-找到；低字节：value]
 */
uint32_t get_int_value_by_dpid(uint8_t d_pid,uint8_t* dpoint_data,uint8_t data_len)
{
	uint8_t i = 0;
	uint32_t result_code = 0;
	datapoint_info_t* temp_datapoint = NULL;
	uint16_t temp_p_dlen = 0;
	
	ESP_LOGI(TAG,"start get int\r\n");
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2

	for(i = 0; i < data_len;)
	{
		temp_datapoint = (datapoint_info_t*)(&dpoint_data[i]);
		
		temp_p_dlen = (temp_datapoint->d_len[0] <<8) | temp_datapoint->d_len[1];
		if((temp_p_dlen +4+i) <= data_len)
		{
			if(temp_datapoint->d_pid == d_pid)
			{
<<<<<<< HEAD
				get_value[0] = dpoint_data[i+4] << 24;
				get_value[0] |= dpoint_data[i+5] << 16;
				get_value[0] |= dpoint_data[i+6] << 8;
				get_value[0] |= dpoint_data[i+7];
				result_code = 1;
=======
				result_code = dpoint_data[i+4] << 24;
				result_code |= dpoint_data[i+5] << 16;
				result_code |= dpoint_data[i+6] << 8;
				result_code |= dpoint_data[i+7];
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
				break;
			}
		}

		i += (temp_p_dlen +4);
	}
<<<<<<< HEAD
//	ESP_LOGI(TAG,"stop get int\r\n");
=======
	ESP_LOGI(TAG,"stop get int\r\n");
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2

	return result_code;
}

<<<<<<< HEAD
uint8_t get_string_value_by_dpid(uint8_t d_pid,uint8_t* dpoint_data,uint8_t data_len,uint8_t*string_value_offset,uint8_t* string_len)
{
	uint8_t i = 0;
	uint8_t result_code = 0;
=======
char* get_string_value_by_dpid(uint8_t d_pid,uint8_t* dpoint_data,uint8_t data_len,uint8_t* string_len)
{
	uint8_t i = 0;
	char* result_code = NULL;
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
	datapoint_info_t* temp_datapoint = NULL;
	uint16_t temp_p_dlen = 0;

	
	ESP_LOGI(TAG,"start get string\r\n");
	for(i = 0; i < data_len;)
	{
		temp_datapoint = (datapoint_info_t*)(&dpoint_data[i]);
		
		temp_p_dlen = (temp_datapoint->d_len[0] <<8) | temp_datapoint->d_len[1];
		if((temp_p_dlen +4+i) <= data_len)
		{
			if(temp_datapoint->d_pid == d_pid)
			{
				*string_len = temp_p_dlen;
<<<<<<< HEAD
				*string_value_offset = i+4;
				result_code = 1;
=======
				result_code = (char*)&dpoint_data[i+4];
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
				break;
			}
		}

		i += (temp_p_dlen + 4);
	}
	ESP_LOGI(TAG,"stop get string\r\n");

	return result_code;
}

<<<<<<< HEAD
uint16_t convert_char_to_number(char* characters)
{
	uint16_t result_code = 0;
	uint8_t i = 0;

	while(i < 4)
	{
		if((characters[i] < 0x30) || (characters[i] > 0x66) || ((characters[0] > 0x39) && (characters[0] < 0x61)))
		{
			return 0;
		}
		i++;
	}

	result_code = (characters[0] > 0x60)?(characters[0]-87):(characters[0]-0x30);
	result_code = (result_code << 12);
	i = (characters[1] > 0x60)?(characters[1]-87):(characters[1]-0x30);
	result_code |= (i << 8);
	i = (characters[2] > 0x60)?(characters[2]-87):(characters[2]-0x30);
	result_code |= (i << 4);
	i = (characters[3] > 0x60)?(characters[3]-87):(characters[3]-0x30);
	result_code |= i ;

	return result_code;
}
=======
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
/** 
 * @brief:   [更新子设备信息]
 * @Author:   xunuo
 * @DateTime: 2021年11月15日T10:01:14+0800
<<<<<<< HEAD
 * @param-sub_device_info[IN]:          [子设备信息,从sub id字段开始]
=======
 * @param-sub_device_info[IN]:          [子设备信息]
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
 * @param-info_len[IN]:          		[子设备信息长度]
 */
void update_sub_device_status(uint8_t* sub_device_info,uint8_t info_len)
{
<<<<<<< HEAD
	uint8_t sub_device_index  = 0;//当前子设备在设备列表中的位置
	uint8_t result_code = 0;
	uint8_t temp_bool_value = 0;
	//uint16_t i = 0;
	uint32_t temp_calculate_value = 0;
	uint8_t temp_string_p = 0;
=======
	uint8_t sub_device_index  = 0;
	uint8_t result_code = 0;
	//uint16_t i = 0;
	uint32_t temp_calculate_value = 0;
	char* temp_string_p = NULL;
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
	uint8_t temp_string_len = 0;
	
	//查找当前设备列表，判断当前设备是否存在
	result_code = find_device_by_sub_id(sub_device_info,&sub_device_index);

	//当前设备不存在
	if(!result_code)
	{
<<<<<<< HEAD
		ESP_LOGI(TAG, "Device does not exist");
		
		/*设备列表有空间,允许添加设备*/
		if(m_mesh_sub_device_count < MESH_GW_SUPPORT_MAX_DEVICE_NUM)
=======
		
		ESP_LOGI(TAG, "Device does not exist");
		
		//允许添加,添加该设备
		if(m_mesh_sub_device_count < 100)
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
		{
			sub_device_index = m_mesh_sub_device_count;
			m_mesh_gw_sub_device_list[sub_device_index].sub_type = get_sub_device_type_by_id(sub_device_info,info_len-10);
			
			ESP_LOGI(TAG, "Add device:");

<<<<<<< HEAD
			/*设备类型合法*/
			if(m_mesh_gw_sub_device_list[sub_device_index].sub_type)
			{
				/*记下sub id*/
=======
			//合法
			if(m_mesh_gw_sub_device_list[sub_device_index].sub_type)
			{
				//记下sub id
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
				memcpy(m_mesh_gw_sub_device_list[sub_device_index].sub_id,sub_device_info,10);

				switch(m_mesh_gw_sub_device_list[sub_device_index].sub_type)
				{
<<<<<<< HEAD
					case HAP_TYPE_SINGLE_LIGHTBULB_E:
=======
					case MESH_SUB_TYPE_SINGLE_LIGHTBULB_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
						ESP_LOGI(TAG, "SINGLE_LIGHTBULB");
						
#if 0
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off = get_bool_value_by_dpid(DPID_LIGHT_SW,&sub_device_info[10],info_len-10);
						ESP_LOGI(TAG, "SW:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off);

						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness = get_int_value_by_dpid(DPID_LIGHT_BRIGHTNESS,&sub_device_info[10],info_len-10)/10;
						ESP_LOGI(TAG, "BRIGHTNESS:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness);
#endif
<<<<<<< HEAD
=======

>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
						m_mesh_gw_sub_device_list[sub_device_index].task_pending_f |= TASK_SUB_DEVICE_ADD_MASK;
						m_mesh_gw_sub_device_list[sub_device_index].sub_register_id = sub_device_index;
						m_mesh_sub_device_count++;

						
						//m_mesh_sub_device_count++;
						break;
<<<<<<< HEAD
					case HAP_TYPE_DOUBLE_LIGHTBULB_E:
=======
					case MESH_SUB_TYPE_DOUBLE_LIGHTBULB_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
						ESP_LOGI(TAG, "DOUBLE_LIGHTBULB");
#if 0
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off = get_bool_value_by_dpid(DPID_LIGHT_SW,&sub_device_info[10],info_len-10);
						//printf("SW:%d,",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off);
						ESP_LOGI(TAG, "SW:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off);

						temp_calculate_value = get_int_value_by_dpid(DPID_LIGHT_BRIGHTNESS,&sub_device_info[10],info_len-10)/10;
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness = temp_calculate_value;
						//ESP_LOGI(TAG, "BRIGHTNESS:%d",temp_calculate_value);
						ESP_LOGI(TAG, "BRIGHTNESS:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness);

						temp_calculate_value = (get_int_value_by_dpid(DPID_LIGHT_COLOR_TEMP,&sub_device_info[10],info_len-10)*7+1000)/20;
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_color_temp = temp_calculate_value;
						//ESP_LOGI(TAG, "COLOR_TEMP:%d",temp_calculate_value);
						ESP_LOGI(TAG, "COLOR_TEMP:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_color_temp);
						
#endif
						m_mesh_gw_sub_device_list[sub_device_index].task_pending_f |= TASK_SUB_DEVICE_ADD_MASK;
						m_mesh_gw_sub_device_list[sub_device_index].sub_register_id = sub_device_index;
						m_mesh_sub_device_count++;
						break;
<<<<<<< HEAD
					case HAP_TYPE_RGB_LIGHTBULB_E:
=======
					case MESH_SUB_TYPE_RGB_LIGHTBULB_E:
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
						ESP_LOGI(TAG, "RGB_LIGHTBULB");
						
#if 0
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off = get_bool_value_by_dpid(DPID_LIGHT_SW,&sub_device_info[10],info_len-10);
						ESP_LOGI(TAG, "SW:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off);
					
						temp_calculate_value = get_int_value_by_dpid(DPID_LIGHT_BRIGHTNESS,&sub_device_info[10],info_len-10)/10;
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness = temp_calculate_value;
						ESP_LOGI(TAG, "BRIGHTNESS:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness);
						
						temp_calculate_value = (get_int_value_by_dpid(DPID_LIGHT_COLOR_TEMP,&sub_device_info[10],info_len-10)*7+1000)/20;
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_color_temp = temp_calculate_value;
						ESP_LOGI(TAG, "COLOR_TEMP:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_color_temp);
						
						temp_string_p = get_string_value_by_dpid(DPID_LIGHT_HSV,&sub_device_info[10],info_len-10,&temp_string_len);

						temp_string_p[temp_string_len] = 0;
						ESP_LOGI(TAG, "HSV:%s",temp_string_p);
#endif
						
						m_mesh_gw_sub_device_list[sub_device_index].task_pending_f |= TASK_SUB_DEVICE_ADD_MASK;
						m_mesh_gw_sub_device_list[sub_device_index].sub_register_id = sub_device_index;
						m_mesh_sub_device_count++;
						break;
<<<<<<< HEAD
					case HAP_TYPE_WYRGB_LIGHTBULB_E:
						ESP_LOGI(TAG, "WYRGB_LIGHTBULB");
						m_mesh_gw_sub_device_list[sub_device_index].task_pending_f |= TASK_SUB_DEVICE_ADD_MASK;
						m_mesh_gw_sub_device_list[sub_device_index].sub_register_id = sub_device_index;
						m_mesh_sub_device_count++;
						break;

					case HAP_TYPE_SW_CONTROLLER_E:
						ESP_LOGI(TAG, "SW_CONTROLLER");
						m_mesh_gw_sub_device_list[sub_device_index].task_pending_f |= TASK_SUB_DEVICE_ADD_MASK;
						m_mesh_gw_sub_device_list[sub_device_index].sub_register_id = sub_device_index;
						m_mesh_sub_device_count++;
						break;

					case HAP_TYPE_SWITCH_1KEY_E:
						ESP_LOGI(TAG, "SWITCH_1KEY");
						m_mesh_gw_sub_device_list[sub_device_index].task_pending_f |= TASK_SUB_DEVICE_ADD_MASK;
						m_mesh_gw_sub_device_list[sub_device_index].sub_register_id = sub_device_index;
						m_mesh_sub_device_count++;
						break;
					case HAP_TYPE_SWITCH_2KEY_E:
						ESP_LOGI(TAG, "SWITCH_2KEY");
						m_mesh_gw_sub_device_list[sub_device_index].task_pending_f |= TASK_SUB_DEVICE_ADD_MASK;
						m_mesh_gw_sub_device_list[sub_device_index].sub_register_id = sub_device_index;
						m_mesh_sub_device_count++;
						break;
						
					case HAP_TYPE_SWITCH_3KEY_E:
						ESP_LOGI(TAG, "SWITCH_3KEY");
						m_mesh_gw_sub_device_list[sub_device_index].task_pending_f |= TASK_SUB_DEVICE_ADD_MASK;
						m_mesh_gw_sub_device_list[sub_device_index].sub_register_id = sub_device_index;
						m_mesh_sub_device_count++;
						break;
						
					case HAP_TYPE_SWITCH_4KEY_E:
						ESP_LOGI(TAG, "SWITCH_2KEY");
						m_mesh_gw_sub_device_list[sub_device_index].task_pending_f |= TASK_SUB_DEVICE_ADD_MASK;
						m_mesh_gw_sub_device_list[sub_device_index].sub_register_id = sub_device_index;
						m_mesh_sub_device_count++;
						break;

					case HAP_TYPE_ELECTRIC_CURTAINS_E:
						ESP_LOGI(TAG, "CURTAINS");
						m_mesh_gw_sub_device_list[sub_device_index].task_pending_f |= TASK_SUB_DEVICE_ADD_MASK;
						m_mesh_gw_sub_device_list[sub_device_index].sub_register_id = sub_device_index;
						m_mesh_sub_device_count++;
						break;
					
					default:
						ESP_LOGI(TAG, "UNKNOW_DEVICE");
=======
					case MESH_SUB_TYPE_WYRGB_LIGHTBULB_E:
						ESP_LOGI(TAG, "WYRGB_LIGHTBULB");
						break;
					
					default:
						ESP_LOGI(TAG, "UNKNOW_LIGHTBULB");
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
						break;
				}
				
			}
<<<<<<< HEAD
			else
			{
				ESP_LOGI(TAG, "UNKNOW_DEVICE");
			}
=======
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
			
		}
		else
		{
			ESP_LOGI(TAG, "There's no space to add");
<<<<<<< HEAD
=======
			return;
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
		}
	}
	//设备已存在
	else
	{
		
		ESP_LOGI(TAG, "Device status update");
		
		//更新状态-------------------------------------------------------------------------------------------------
		switch(m_mesh_gw_sub_device_list[sub_device_index].sub_type)
		{
<<<<<<< HEAD
			case HAP_TYPE_SINGLE_LIGHTBULB_E:
				ESP_LOGI(TAG, "SINGLE_LIGHTBULB");
				
				//找到该数据点
				if(get_bool_value_by_dpid(DPID_LIGHT_SW,&sub_device_info[10],info_len-10,&temp_bool_value))
				{
					if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off != temp_bool_value)
					{
						m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= LIGHTBULB_NOTIFY_SW_MASK;
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off = temp_bool_value;
						
						ESP_LOGI(TAG, "SW:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off);
					}
				}
				
				if(get_int_value_by_dpid(DPID_LIGHT_BRIGHTNESS,&sub_device_info[10],info_len-10,&temp_calculate_value))
				{
					temp_calculate_value = temp_calculate_value/10;
					if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness != temp_calculate_value)
					{
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness = temp_calculate_value;
						m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= LIGHTBULB_NOTIFY_BRIGHTNESS_MASK;
						ESP_LOGI(TAG, "BRIGHTNESS:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness);
					}
				}
				
				break;
			case HAP_TYPE_DOUBLE_LIGHTBULB_E:
				ESP_LOGI(TAG, "DOUBLE_LIGHTBULB");
				
				if(get_bool_value_by_dpid(DPID_LIGHT_SW,&sub_device_info[10],info_len-10,&temp_bool_value))
				{
					if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off != temp_bool_value)
					{
						m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= LIGHTBULB_NOTIFY_SW_MASK;
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off = temp_bool_value;
						
						ESP_LOGI(TAG, "SW:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off);
					}
				}
				
				if(get_int_value_by_dpid(DPID_LIGHT_BRIGHTNESS,&sub_device_info[10],info_len-10,&temp_calculate_value))
				{
					temp_calculate_value = temp_calculate_value/10;
					if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness != temp_calculate_value)
					{
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness = temp_calculate_value;
						m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= LIGHTBULB_NOTIFY_BRIGHTNESS_MASK;
						ESP_LOGI(TAG, "BRIGHTNESS:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness);
					}
				}

				if(get_int_value_by_dpid(DPID_LIGHT_COLOR_TEMP,&sub_device_info[10],info_len-10,&temp_calculate_value))
				{
					temp_calculate_value = (8000 - temp_calculate_value*7)/20;

					if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_color_temp != temp_calculate_value)
					{
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_color_temp = temp_calculate_value;
						m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= DPID_LIGHT_COLOR_TEMP;
						
						ESP_LOGI(TAG, "COLOR_TEMP:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_color_temp);
					}	
				}
							
				break;
				
			case HAP_TYPE_RGB_LIGHTBULB_E:
				ESP_LOGI(TAG, "RGB_LIGHTBULB");
				
				if(get_bool_value_by_dpid(DPID_LIGHT_SW,&sub_device_info[10],info_len-10,&temp_bool_value))
				{
					if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off != temp_bool_value)
					{
						m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= LIGHTBULB_NOTIFY_SW_MASK;
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off = temp_bool_value;
						
						ESP_LOGI(TAG, "SW:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off);
					}
				}
				
				
				if(get_int_value_by_dpid(DPID_LIGHT_BRIGHTNESS,&sub_device_info[10],info_len-10,&temp_calculate_value))
				{
					temp_calculate_value = temp_calculate_value/10;
					if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness != temp_calculate_value)
					{
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness = temp_calculate_value;
						m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= LIGHTBULB_NOTIFY_BRIGHTNESS_MASK;
						ESP_LOGI(TAG, "BRIGHTNESS:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness);
					}
				}
				
/*
=======
			case MESH_SUB_TYPE_SINGLE_LIGHTBULB_E:
				ESP_LOGI(TAG, "SINGLE_LIGHTBULB");
				result_code = get_bool_value_by_dpid(DPID_LIGHT_SW,&sub_device_info[10],info_len-10);
				if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off != result_code)
				{
					m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= LIGHTBULB_NOTIFY_SW_MASK;
					m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off = result_code;
				}
				
				ESP_LOGI(TAG, "SW:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off);
				
				temp_calculate_value = get_int_value_by_dpid(DPID_LIGHT_BRIGHTNESS,&sub_device_info[10],info_len-10)/10;
				if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness != temp_calculate_value)
				{
					m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness = temp_calculate_value;
					m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= LIGHTBULB_NOTIFY_BRIGHTNESS_MASK;
				}
				ESP_LOGI(TAG, "BRIGHTNESS:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness);
				
				break;
			case MESH_SUB_TYPE_DOUBLE_LIGHTBULB_E:
				ESP_LOGI(TAG, "DOUBLE_LIGHTBULB");
				
				result_code = get_bool_value_by_dpid(DPID_LIGHT_SW,&sub_device_info[10],info_len-10);
				if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off != result_code)
				{
					m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= LIGHTBULB_NOTIFY_SW_MASK;
					m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off = result_code;
				}
				
				ESP_LOGI(TAG, "SW:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off);
				
				temp_calculate_value = get_int_value_by_dpid(DPID_LIGHT_BRIGHTNESS,&sub_device_info[10],info_len-10)/10;
				if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness != temp_calculate_value)
				{
					m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness = temp_calculate_value;
					m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= LIGHTBULB_NOTIFY_BRIGHTNESS_MASK;
				}
				ESP_LOGI(TAG, "BRIGHTNESS:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness);
				
				temp_calculate_value = (8000-get_int_value_by_dpid(DPID_LIGHT_COLOR_TEMP,&sub_device_info[10],info_len-10)*7)/20;
				if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_color_temp != temp_calculate_value)
				{
					m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_color_temp = temp_calculate_value;
					m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= LIGHTBULB_NOTIFY_BRIGHTNESS_MASK;
				}				
				ESP_LOGI(TAG, "COLOR_TEMP:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_color_temp);
				break;
				
			case MESH_SUB_TYPE_RGB_LIGHTBULB_E:
				ESP_LOGI(TAG, "RGB_LIGHTBULB");
				
				result_code = get_bool_value_by_dpid(DPID_LIGHT_SW,&sub_device_info[10],info_len-10);
				if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off != result_code)
				{
					m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= LIGHTBULB_NOTIFY_SW_MASK;
					m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off = result_code;
				}
				
				ESP_LOGI(TAG, "SW:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off);
				
				temp_calculate_value = get_int_value_by_dpid(DPID_LIGHT_BRIGHTNESS,&sub_device_info[10],info_len-10)/10;
				if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness != temp_calculate_value)
				{
					m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness = temp_calculate_value;
					m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= LIGHTBULB_NOTIFY_BRIGHTNESS_MASK;
				}
				ESP_LOGI(TAG, "BRIGHTNESS:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness);
				
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
				temp_calculate_value = (8000-get_int_value_by_dpid(DPID_LIGHT_COLOR_TEMP,&sub_device_info[10],info_len-10)*7)/20;
				if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_color_temp != temp_calculate_value)
				{
					m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_color_temp = temp_calculate_value;
					m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= LIGHTBULB_NOTIFY_BRIGHTNESS_MASK;
				}				
				ESP_LOGI(TAG, "COLOR_TEMP:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_color_temp);
				
<<<<<<< HEAD
*/
				if(get_string_value_by_dpid(DPID_LIGHT_HSV,&sub_device_info[10],info_len-10,&temp_string_p,&temp_string_len))
				{
					//temp_string_p[temp_string_len] = 0;
					
					//获取hue
					temp_calculate_value = convert_char_to_number((char*)&sub_device_info[10+temp_string_p]); 
					if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_hue != temp_calculate_value)
					{
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_hue = temp_calculate_value;
						m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= LIGHTBULB_NOTIFY_HUE_MASK;
	
						ESP_LOGI(TAG, "H:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_hue);
					}
	
					//获取saturation
					temp_calculate_value = convert_char_to_number((char*)&sub_device_info[10+temp_string_p+4]); 
					temp_calculate_value = temp_calculate_value/10;
					if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_saturation != temp_calculate_value)
					{
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_saturation = temp_calculate_value;
						m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= LIGHTBULB_NOTIFY_SATURATION_MASK;
						
						ESP_LOGI(TAG, "S:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_saturation);
					}
				}
				
				
				break;
			case HAP_TYPE_WYRGB_LIGHTBULB_E:
				ESP_LOGI(TAG, "WYRGB_LIGHTBULB");

				//开关
				if(get_bool_value_by_dpid(DPID_LIGHT_SW,&sub_device_info[10],info_len-10,&temp_bool_value))
				{
					if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off != temp_bool_value)
					{
						m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= LIGHTBULB_NOTIFY_SW_MASK;
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off = temp_bool_value;
						
						ESP_LOGI(TAG, "SW:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_on_off);
					}
				}
				
				//亮度
				if(get_int_value_by_dpid(DPID_LIGHT_BRIGHTNESS,&sub_device_info[10],info_len-10,&temp_calculate_value))
				{
					temp_calculate_value = temp_calculate_value/10;
					if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness != temp_calculate_value)
					{
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness = temp_calculate_value;
						m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= LIGHTBULB_NOTIFY_BRIGHTNESS_MASK;
						ESP_LOGI(TAG, "BRIGHTNESS:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_brightness);
					}
				}

				//色温
				if(get_int_value_by_dpid(DPID_LIGHT_COLOR_TEMP,&sub_device_info[10],info_len-10,&temp_calculate_value))
				{
					temp_calculate_value = (8000 - temp_calculate_value*7)/20;

					if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_color_temp != temp_calculate_value)
					{
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_color_temp = temp_calculate_value;
						m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= DPID_LIGHT_COLOR_TEMP;
						
						ESP_LOGI(TAG, "COLOR_TEMP:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_color_temp);
					}	
				}
				
				//HSV
				if(get_string_value_by_dpid(DPID_LIGHT_HSV,&sub_device_info[10],info_len-10,&temp_string_p,&temp_string_len))
				{
					//temp_string_p[temp_string_len] = 0;
					
					//获取hue
					temp_calculate_value = convert_char_to_number((char*)&sub_device_info[10+temp_string_p]); 
					if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_hue != temp_calculate_value)
					{
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_hue = temp_calculate_value;
						m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= LIGHTBULB_NOTIFY_HUE_MASK;
	
						ESP_LOGI(TAG, "H:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_hue);
					}
	
					//获取saturation
					temp_calculate_value = convert_char_to_number((char*)&sub_device_info[10+temp_string_p+4]); 
					temp_calculate_value = temp_calculate_value/10;
					if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_saturation != temp_calculate_value)
					{
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_saturation = temp_calculate_value;
						m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= LIGHTBULB_NOTIFY_SATURATION_MASK;
						
						ESP_LOGI(TAG, "S:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_lightbulb.lamp_saturation);
					}
				}
				break;
				
			case HAP_TYPE_SW_CONTROLLER_E:
			case HAP_TYPE_SWITCH_1KEY_E:
			case HAP_TYPE_SWITCH_2KEY_E:
			case HAP_TYPE_SWITCH_3KEY_E:
			case HAP_TYPE_SWITCH_4KEY_E:
				ESP_LOGI(TAG, "SW_CONTROLLER");
				if(get_bool_value_by_dpid(DPID_SWITCH_KEY1_SW,&sub_device_info[10],info_len-10,&temp_bool_value))
				{
					if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw1_on_ff != temp_bool_value)
					{
						m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= SWITCH_NOTIFY_SW1_MASK;
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw1_on_ff = temp_bool_value;
						
						ESP_LOGI(TAG, "SW1:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw1_on_ff);
					}
				}
				

				if(m_mesh_gw_sub_device_list[sub_device_index].sub_type > HAP_TYPE_SWITCH_1KEY_E)
				{
					if(get_bool_value_by_dpid(DPID_SWITCH_KEY2_SW,&sub_device_info[10],info_len-10,&temp_bool_value))
					{
						if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw2_on_ff != temp_bool_value)
						{
							m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= SWITCH_NOTIFY_SW2_MASK;
							m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw2_on_ff = temp_bool_value;
							
							ESP_LOGI(TAG, "SW2:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw2_on_ff);
						}
					}
				}

				if(m_mesh_gw_sub_device_list[sub_device_index].sub_type > HAP_TYPE_SWITCH_2KEY_E)
				{
					if(get_bool_value_by_dpid(DPID_SWITCH_KEY3_SW,&sub_device_info[10],info_len-10,&temp_bool_value))
					{
						if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw3_on_ff != temp_bool_value)
						{
							m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= SWITCH_NOTIFY_SW3_MASK;
							m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw3_on_ff = temp_bool_value;
							
							ESP_LOGI(TAG, "SW3:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw3_on_ff);
						}
					}
					
				}

				if(m_mesh_gw_sub_device_list[sub_device_index].sub_type > HAP_TYPE_SWITCH_3KEY_E)
				{
					if(get_bool_value_by_dpid(DPID_SWITCH_KEY4_SW,&sub_device_info[10],info_len-10,&temp_bool_value))
					{
						if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw4_on_ff != temp_bool_value)
						{
							m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= SWITCH_NOTIFY_SW4_MASK;
							m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw4_on_ff = temp_bool_value;
							
							ESP_LOGI(TAG, "SW4:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_switch.sw4_on_ff);
						}
					}
				}
				
				break;
				
			case HAP_TYPE_ELECTRIC_CURTAINS_E:
				ESP_LOGI(TAG, "CURTAINS");
				
				if(get_int_value_by_dpid(DPID_CURTAIN_CURRENT_STATE,&sub_device_info[10],info_len-10,&temp_calculate_value))
				{
					temp_calculate_value = 100-temp_calculate_value;
					
					if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.current_position != temp_calculate_value)
					{
						m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= CURTAIN_NOTIFY_CURRENT_POS_MASK;
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.current_position = temp_calculate_value;
						
						ESP_LOGI(TAG, "CURRENT POS:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.current_position);
					}
				}
				
				if(get_int_value_by_dpid(DPID_CURTAIN_TARGET_STATE,&sub_device_info[10],info_len-10,&temp_calculate_value))
				{
					temp_calculate_value = 100-temp_calculate_value;
					
					if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.target_position != temp_calculate_value)
					{
						m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= CURTAIN_NOTIFY_TARGET_POS_MASK;
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.target_position = temp_calculate_value;
						
						ESP_LOGI(TAG, "TARGET POS:%d",m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.target_position);
					}
				}

#if 0
				if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.current_position < m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.target_position)
				{
					if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.position_state != 1)
					{
						m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= CURTAIN_NOTIFY_POS_STATE_MASK;
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.position_state = 1;
						ESP_LOGI(TAG, "POS STATE:1");
					}
				}
				else if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.current_position > m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.target_position)
				{
					if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.position_state != 0)
					{
						m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= CURTAIN_NOTIFY_POS_STATE_MASK;
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.position_state = 0;
						ESP_LOGI(TAG, "POS STATE:0");
					}
				}
				else
				{
					if(m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.position_state != 2)
					{
						m_mesh_gw_sub_device_list[sub_device_index].is_notify_f |= CURTAIN_NOTIFY_POS_STATE_MASK;
						m_mesh_gw_sub_device_list[sub_device_index].sub_device.sub_curtain.position_state = 2;
						ESP_LOGI(TAG, "POS STATE:2");
					}
				}
#endif
				break;
			default:
				ESP_LOGI(TAG, "UNKNOW_device");
				break;
		}
		
=======
				temp_string_p = get_string_value_by_dpid(DPID_LIGHT_HSV,&sub_device_info[10],info_len-10,&temp_string_len);

				temp_string_p[temp_string_len] = 0;
				ESP_LOGI(TAG, "HSV:%s",temp_string_p);
				//m_mesh_gw_sub_device_list[m_mesh_sub_device_count].sub_device.sub_lightbulb.lamp_hue
				break;
			case MESH_SUB_TYPE_WYRGB_LIGHTBULB_E:
				ESP_LOGI(TAG, "WYRGB_LIGHTBULB");
				break;
			default:
				ESP_LOGI(TAG, "UNKNOW_LIGHTBULB");
				break;
		}
>>>>>>> 0f8aba836d2dbd47d1beee00e977a2c24b71dcd2
	}

	//测试，清除所有挂起的任务
	//m_mesh_gw_sub_device_list[sub_device_index].is_notify_f = 0;
	//m_mesh_gw_sub_device_list[sub_device_index].task_pending_f = 0;
}

