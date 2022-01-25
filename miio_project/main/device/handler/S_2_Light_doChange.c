/**
 * Copyright (C) 2013-2015
 *
 * @author ouyangchengfeng@xiaomi.com
 * @date   2018-11-14
 *
 * @file   S_2_Light_doChange.c
 *
 * @remark
 *
 */

#include "arch_dbg.h"
#include "arch_os.h"
#include "S_2_Light_doChange.h"
#include "../operation_executor.h"
#include "../../light.h"

#define TAG "S_2_Light_doChange"

extern miio_handle_t g_miio_instance_handle;

#if 0
void P_2_1_Group_doChange(miio_handle_t handle, bool newValue)
{
    if (send_property_changed(handle, 2, 1, property_value_new_boolean(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_1_Group_doChange_cb(void * newValue)
{
	bool value = (bool) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_1_Group_doChange(g_miio_instance_handle,value);
	return;
}

void P_2_1_Group_doChange_notify(bool newValue)
{
	arch_os_async_call(P_2_1_Group_doChange_cb,(void *)newValue,500);
}

void P_2_2_Brightness_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 2, 2, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}
#else


void P_2_1_Group_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 2, 1, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_1_Group_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_1_Group_doChange(g_miio_instance_handle,value);
	return;
}

void P_2_1_Group_doChange_notify(int newValue)
{
	arch_os_async_call(P_2_1_Group_doChange_cb,(void *)newValue,500);
}

void P_2_18_GroupName_doChange(miio_handle_t handle, bool newValue)
{
    if (send_property_changed(handle, 2, 18, property_value_new_boolean(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_18_GroupName_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_18_GroupName_doChange(g_miio_instance_handle,value);
	return;
}

void P_2_18_NotifyGroupName_doChange_notify(int newValue)
{
	arch_os_async_call(P_2_18_GroupName_doChange_cb,(void *)newValue,500);
}

void P_2_19_Num_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 2, 19, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_19_Num_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_19_Num_doChange(g_miio_instance_handle,value);
	return;
}

void P_2_19_NotifyNum_doChange_notify(int newValue)
{
	arch_os_async_call(P_2_19_Num_doChange_cb,(void *)newValue,500);
}


#endif


void P_2_2_Name_doChange(miio_handle_t handle, char* newValue)
{
    if (send_property_changed(handle, 2, 2, property_value_new_string(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_2_Name_doChange_cb(void * newValue)
{
	char* value = (char*) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_2_Name_doChange(g_miio_instance_handle,value);
	return;
}


void P_2_2_Name_doChange_notify(char* newValue)
{
	arch_os_async_call(P_2_2_Name_doChange_cb,(void *)newValue,500);
}


void P_2_3_Name_doChange(miio_handle_t handle, char* newValue)
{
    if (send_property_changed(handle, 2, 3, property_value_new_string(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_3_Name_doChange_cb(void * newValue)
{
	char* value = (char*) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_3_Name_doChange(g_miio_instance_handle,value);
	return;
}


void P_2_3_Name_doChange_notify(char* newValue)
{
	arch_os_async_call(P_2_3_Name_doChange_cb,(void *)newValue,500);
}


void P_2_4_Name_doChange(miio_handle_t handle, char* newValue)
{
    if (send_property_changed(handle, 2, 4, property_value_new_string(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_4_Name_doChange_cb(void * newValue)
{
	char* value = (char*) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_4_Name_doChange(g_miio_instance_handle,value);
	return;
}


void P_2_4_Name_doChange_notify(char* newValue)
{
	arch_os_async_call(P_2_4_Name_doChange_cb,(void *)newValue,500);
}


void P_2_5_Name_doChange(miio_handle_t handle, char* newValue)
{
    if (send_property_changed(handle, 2, 5, property_value_new_string(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_5_Name_doChange_cb(void * newValue)
{
	char* value = (char*) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_5_Name_doChange(g_miio_instance_handle,value);
	return;
}


void P_2_5_Name_doChange_notify(char* newValue)
{
	arch_os_async_call(P_2_5_Name_doChange_cb,(void *)newValue,500);
}


void P_2_6_Name_doChange(miio_handle_t handle, char* newValue)
{
    if (send_property_changed(handle, 2, 6, property_value_new_string(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_6_Name_doChange_cb(void * newValue)
{
	char* value = (char*) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_6_Name_doChange(g_miio_instance_handle,value);
	return;
}


void P_2_6_Name_doChange_notify(char* newValue)
{
	arch_os_async_call(P_2_6_Name_doChange_cb,(void *)newValue,500);
}

void P_2_7_Name_doChange(miio_handle_t handle, char* newValue)
{
    if (send_property_changed(handle, 2, 7, property_value_new_string(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_7_Name_doChange_cb(void * newValue)
{
	char* value = (char*) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_7_Name_doChange(g_miio_instance_handle,value);
	return;
}


void P_2_7_Name_doChange_notify(char* newValue)
{
	arch_os_async_call(P_2_7_Name_doChange_cb,(void *)newValue,500);
}


void P_2_8_Name_doChange(miio_handle_t handle, char* newValue)
{
    if (send_property_changed(handle, 2, 8, property_value_new_string(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_8_Name_doChange_cb(void * newValue)
{
	char* value = (char*) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_8_Name_doChange(g_miio_instance_handle,value);
	return;
}


void P_2_8_Name_doChange_notify(char* newValue)
{
	arch_os_async_call(P_2_8_Name_doChange_cb,(void *)newValue,500);
}


void P_2_9_Name_doChange(miio_handle_t handle, char* newValue)
{
    if (send_property_changed(handle, 2, 9, property_value_new_string(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_9_Name_doChange_cb(void * newValue)
{
	char* value = (char*) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_9_Name_doChange(g_miio_instance_handle,value);
	return;
}


void P_2_9_Name_doChange_notify(char* newValue)
{
	arch_os_async_call(P_2_9_Name_doChange_cb,(void *)newValue,500);
}


void P_2_10_Name_doChange(miio_handle_t handle, char* newValue)
{
    if (send_property_changed(handle, 2, 10, property_value_new_string(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_10_Brightness_doChange_cb(void * newValue)
{
	char* value = (char*) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_10_Name_doChange(g_miio_instance_handle,value);
	return;
}


void P_2_10_Name_doChange_notify(char* newValue)
{
	arch_os_async_call(P_2_10_Brightness_doChange_cb,(void *)newValue,500);
}


void P_2_11_Brightness_doChange(miio_handle_t handle, char* newValue)
{
    if (send_property_changed(handle, 2, 11, property_value_new_string(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_11_Brightness_doChange_cb(void * newValue)
{
	char* value = (char*) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_11_Brightness_doChange(g_miio_instance_handle,value);
	return;
}


void P_2_11_Name_doChange_notify(char* newValue)
{
	arch_os_async_call(P_2_11_Brightness_doChange_cb,(void *)newValue,500);
}


void P_2_12_Brightness_doChange(miio_handle_t handle, char* newValue)
{
    if (send_property_changed(handle, 2, 12, property_value_new_string(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_12_Brightness_doChange_cb(void * newValue)
{
	char* value = (char*) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_12_Brightness_doChange(g_miio_instance_handle,value);
	return;
}


void P_2_12_Name_doChange_notify(char* newValue)
{
	arch_os_async_call(P_2_12_Brightness_doChange_cb,(void *)newValue,500);
}


void P_2_13_Brightness_doChange(miio_handle_t handle, char* newValue)
{
    if (send_property_changed(handle, 2, 13, property_value_new_string(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_13_Brightness_doChange_cb(void * newValue)
{
	char* value = (char*) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_13_Brightness_doChange(g_miio_instance_handle,value);
	return;
}


void P_2_13_Name_doChange_notify(char* newValue)
{
	arch_os_async_call(P_2_13_Brightness_doChange_cb,(void *)newValue,500);
}


void P_2_14_Brightness_doChange(miio_handle_t handle, char* newValue)
{
    if (send_property_changed(handle, 2, 14, property_value_new_string(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_14_Brightness_doChange_cb(void * newValue)
{
	char* value = (char*) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_14_Brightness_doChange(g_miio_instance_handle,value);
	return;
}


void P_2_14_Name_doChange_notify(char* newValue)
{
	arch_os_async_call(P_2_14_Brightness_doChange_cb,(void *)newValue,500);
}


void P_2_15_Brightness_doChange(miio_handle_t handle, char* newValue)
{
    if (send_property_changed(handle, 2, 15, property_value_new_string(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_15_Brightness_doChange_cb(void * newValue)
{
	char* value = (char*) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_15_Brightness_doChange(g_miio_instance_handle,value);
	return;
}


void P_2_15_Name_doChange_notify(char* newValue)
{
	arch_os_async_call(P_2_15_Brightness_doChange_cb,(void *)newValue,500);
}


void P_2_16_Brightness_doChange(miio_handle_t handle, char* newValue)
{
    if (send_property_changed(handle, 2, 16, property_value_new_string(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_16_Brightness_doChange_cb(void * newValue)
{
	char* value = (char*) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_16_Brightness_doChange(g_miio_instance_handle,value);
	return;
}


void P_2_16_Name_doChange_notify(char* newValue)
{
	arch_os_async_call(P_2_16_Brightness_doChange_cb,(void *)newValue,500);
}


void P_2_17_Brightness_doChange(miio_handle_t handle, char* newValue)
{
    if (send_property_changed(handle, 2, 17, property_value_new_string(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_17_Brightness_doChange_cb(void * newValue)
{
	char* value = (char*) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_17_Brightness_doChange(g_miio_instance_handle,value);
	return;
}


void P_2_17_Name_doChange_notify(char* newValue)
{
	arch_os_async_call(P_2_17_Brightness_doChange_cb,(void *)newValue,500);
}





//É«ÎÂ
void P_2_3_ColorTemperature_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 2, 3, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_3_ColorTemperature_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_3_ColorTemperature_doChange(g_miio_instance_handle,value);
	return;
}

void P_2_3_ColorTemperature_doChange_notify(int newValue)
{
	arch_os_async_call(P_2_3_ColorTemperature_doChange_cb,(void *)newValue,500);
}

//rgbcolor
void P_2_4_RGBColor_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 2, 4, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_4_RGBColor_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_4_RGBColor_doChange(g_miio_instance_handle,value);
	return;
}

void P_2_4_RGBColor_doChange_notify(int newValue)
{
	arch_os_async_call(P_2_4_RGBColor_doChange_cb,(void *)newValue,500);
}

//rgbmode
void P_2_5_RGBMode_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 2, 5, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_5_RGBMode_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_5_RGBMode_doChange(g_miio_instance_handle,value);
	return;
}

void P_2_5_RGBMode_doChange_notify(int newValue)
{
	arch_os_async_call(P_2_5_RGBMode_doChange_cb,(void *)newValue,500);
}


//

void P_2_6_FlexSwitch_doChange(miio_handle_t handle, bool newValue)
{
    if (send_property_changed(handle, 2, 6, property_value_new_boolean(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_6_FlexSwitch_doChange_cb(void * newValue)
{
	bool value = (bool) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_6_FlexSwitch_doChange(g_miio_instance_handle,value);
	return;
}

void P_2_6_FlexSwitch_doChange_notify(bool newValue)
{
	arch_os_async_call(P_2_6_FlexSwitch_doChange_cb,(void *)newValue,500);
}


//

void P_2_7_SleepMode_doChange(miio_handle_t handle, bool newValue)
{
    if (send_property_changed(handle, 2, 7, property_value_new_boolean(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_7_SleepMode_doChange_cb(void * newValue)
{
	bool value = (bool) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_7_SleepMode_doChange(g_miio_instance_handle,value);
	return;
}

void P_2_7_SleepMode_doChange_notify(bool newValue)
{
	arch_os_async_call(P_2_7_SleepMode_doChange_cb,(void *)newValue,500);
}

//

void P_2_8_WeakupMode_doChange(miio_handle_t handle, bool newValue)
{
    if (send_property_changed(handle, 2, 8, property_value_new_boolean(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_8_WeakupMode_doChange_cb(void * newValue)
{
	bool value = (bool) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_8_WeakupMode_doChange(g_miio_instance_handle,value);
	return;
}

void P_2_8_WeakupMode_doChange_notify(bool newValue)
{
	arch_os_async_call(P_2_8_WeakupMode_doChange_cb,(void *)newValue,500);
}

//
void P_2_9_WLduration_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 2, 9, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_9_WLduration_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_9_WLduration_doChange(g_miio_instance_handle,value);
	return;
}

void P_2_9_WLduration_doChange_notify(int newValue)
{
	arch_os_async_call(P_2_9_WLduration_doChange_cb,(void *)newValue,500);
}

//
void P_2_10_WeakpEbr_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 2, 10, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_10_WeakpEbr_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_10_WeakpEbr_doChange(g_miio_instance_handle,value);
	return;
}

void P_2_10_WeakpEbr_doChange_notify(int newValue)
{
	arch_os_async_call(P_2_10_WeakpEbr_doChange_cb,(void *)newValue,500);
}


//
void P_2_11_SleepSbr_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 2, 11, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_11_SleepSbr_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_11_SleepSbr_doChange(g_miio_instance_handle,value);
	return;
}

void P_2_11_SleepSbr_doChange_notify(int newValue)
{
	arch_os_async_call(P_2_11_SleepSbr_doChange_cb,(void *)newValue,500);
}



//
void P_2_12_LightPONDefault_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 2, 12, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_2_12_LightPONDefault_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_2_12_LightPONDefault_doChange(g_miio_instance_handle,value);
	return;
}

void P_2_12_LightPONDefault_doChange_notify(int newValue)
{
	arch_os_async_call(P_2_12_LightPONDefault_doChange_cb,(void *)newValue,500);
}



//S3---------------------------------

//============================================

void P_3_1_OnOff_doChange(miio_handle_t handle, bool newValue)
{
    if (send_property_changed(handle, 3, 1, property_value_new_boolean(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_3_1_OnOff_doChange_cb(void * newValue)
{
	bool value = (bool) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_3_1_OnOff_doChange(g_miio_instance_handle,value);
	return;
}

void P_3_1_OnOff_doChange_notify(bool newValue)
{
	arch_os_async_call(P_3_1_OnOff_doChange_cb,(void *)newValue,500);
}

#if 0
//============================================

void P_3_2_BrAdd_doChange(miio_handle_t handle, bool newValue)
{
    if (send_property_changed(handle, 3, 2, property_value_new_boolean(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_3_2_BrAdd_doChange_cb(void * newValue)
{
	bool value = (bool) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_3_2_BrAdd_doChange(g_miio_instance_handle,value);
	return;
}

void P_3_2_Mode_doChange_notify(bool newValue)
{
	arch_os_async_call(P_3_2_BrAdd_doChange_cb,(void *)newValue,500);
}
#else
void P_3_2_BrAdd_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 3, 2, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_3_2_BrAdd_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_3_2_BrAdd_doChange(g_miio_instance_handle,value);
	return;
}

void P_3_2_Mode_doChange_notify(int newValue)
{
	arch_os_async_call(P_3_2_BrAdd_doChange_cb,(void *)newValue,500);
}


#endif

#if 0
//============================================

void P_3_3_BrDec_doChange(miio_handle_t handle, bool newValue)
{
    if (send_property_changed(handle, 3, 3, property_value_new_boolean(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_3_3_BrDec_doChange_cb(void * newValue)
{
	bool value = (bool) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_3_3_BrDec_doChange(g_miio_instance_handle,value);
	return;
}

void P_3_3_Fault_doChange_notify(bool newValue)
{
	arch_os_async_call(P_3_3_BrDec_doChange_cb,(void *)newValue,500);
}

#else
void P_3_3_BrDec_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 3, 3, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_3_3_BrDec_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_3_3_BrDec_doChange(g_miio_instance_handle,value);
	return;
}

void P_3_3_Fault_doChange_notify(int newValue)
{
	arch_os_async_call(P_3_3_BrDec_doChange_cb,(void *)newValue,500);
}



#endif

#if 0
//============================================

void P_3_4_BrSw_doChange(miio_handle_t handle, bool newValue)
{
    if (send_property_changed(handle, 3, 4, property_value_new_boolean(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_3_4_BrSw_doChange_cb(void * newValue)
{
	bool value = (bool) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_3_4_BrSw_doChange(g_miio_instance_handle,value);
	return;
}

void P_3_4_SetTemp_doChange_notify(bool newValue)
{
	arch_os_async_call(P_3_4_BrSw_doChange_cb,(void *)newValue,500);
}

#else

//============================================

void P_3_4_BrSw_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 3, 4, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_3_4_BrSw_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_3_4_BrSw_doChange(g_miio_instance_handle,value);
	return;
}

void P_3_4_SetTemp_doChange_notify(int newValue)
{
	arch_os_async_call(P_3_4_BrSw_doChange_cb,(void *)newValue,500);
}


#endif
//============================================

void P_3_5_OnBrSw_doChange(miio_handle_t handle, bool newValue)
{
    if (send_property_changed(handle, 3, 5, property_value_new_boolean(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_3_5_OnBrSw_doChange_cb(void * newValue)
{
	bool value = (bool) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_3_5_OnBrSw_doChange(g_miio_instance_handle,value);
	return;
}

void P_3_5_OnBrSw_doChange_notify(bool newValue)
{
	arch_os_async_call(P_3_5_OnBrSw_doChange_cb,(void *)newValue,500);
}

//============================================
//É«ÎÂ
void P_3_6_TmAdd_doChange(miio_handle_t handle, bool newValue)
{
    if (send_property_changed(handle, 3, 6, property_value_new_boolean(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_3_6_TmAdd_doChange_cb(void * newValue)
{
	bool value = (bool) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_3_6_TmAdd_doChange(g_miio_instance_handle,value);
	return;
}

void P_3_6_TmAdd_doChange_notify(bool newValue)
{
	arch_os_async_call(P_3_6_TmAdd_doChange_cb,(void *)newValue,500);
}

//============================================

void P_3_7_TmDec_doChange(miio_handle_t handle, bool newValue)
{
    if (send_property_changed(handle, 3, 7, property_value_new_boolean(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_3_7_TmDec_doChange_cb(void * newValue)
{
	bool value = (bool) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_3_7_TmDec_doChange(g_miio_instance_handle,value);
	return;
}

void P_3_7_TmDec_doChange_notify(bool newValue)
{
	arch_os_async_call(P_3_7_TmDec_doChange_cb,(void *)newValue,500);
}

//============================================

void P_3_8_TmSw_doChange(miio_handle_t handle, bool newValue)
{
    if (send_property_changed(handle, 3, 8, property_value_new_boolean(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_3_8_TmSw_doChange_cb(void * newValue)
{
	bool value = (bool) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_3_8_TmSw_doChange(g_miio_instance_handle,value);
	return;
}

void P_3_8_TmSw_doChange_notify(bool newValue)
{
	arch_os_async_call(P_3_8_TmSw_doChange_cb,(void *)newValue,500);
}

//============================================

void P_3_9_OnTmSw_doChange(miio_handle_t handle, bool newValue)
{
    if (send_property_changed(handle, 3, 9, property_value_new_boolean(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_3_9_OnTmSw_doChange_cb(void * newValue)
{
	bool value = (bool) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_3_9_OnTmSw_doChange(g_miio_instance_handle,value);
	return;
}

void P_3_9_OnTmSw_doChange_notify(bool newValue)
{
	arch_os_async_call(P_3_9_OnTmSw_doChange_cb,(void *)newValue,500);
}

//============================================
void P_3_10_RGBSw_doChange(miio_handle_t handle, bool newValue)
{
    if (send_property_changed(handle, 3, 10, property_value_new_boolean(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_3_10_RGBSw_doChange_cb(void * newValue)
{
	bool value = (bool) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_3_10_RGBSw_doChange(g_miio_instance_handle,value);
	return;
}

void P_3_10_RGBSw_doChange_notify(bool newValue)
{
	arch_os_async_call(P_3_10_RGBSw_doChange_cb,(void *)newValue,500);
}

//============================================

void P_3_11_OnRGBSw_doChange(miio_handle_t handle, bool newValue)
{
    if (send_property_changed(handle, 3, 11, property_value_new_boolean(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_3_11_OnRGBSw_doChange_cb(void * newValue)
{
	bool value = (bool) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_3_11_OnRGBSw_doChange(g_miio_instance_handle,value);
	return;
}

void P_3_11_OnRGBSw_doChange_notify(bool newValue)
{
	arch_os_async_call(P_3_11_OnRGBSw_doChange_cb,(void *)newValue,500);
}

//===============================

//===============================
void P_4_2_Default_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 4, 2, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_4_2_Default_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_4_2_Default_doChange(g_miio_instance_handle,value);
	return;
}

void P_4_2_FanSpeed_doChange_notify(int newValue)
{
	arch_os_async_call(P_4_2_Default_doChange_cb,(void *)newValue,500);
}

#if 1
//===============================
void P_4_2_UserMode_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 4, 1, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_4_2_UserMode_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_4_2_UserMode_doChange(g_miio_instance_handle,value);
	return;
}

void P_4_2_UserMode_doChange_notify(int newValue)
{
	arch_os_async_call(P_4_2_UserMode_doChange_cb,(void *)newValue,500);
}

#endif

//===============================
void P_4_3_FadeOn_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 4, 3, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_4_3_FadeOn_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_4_3_FadeOn_doChange(g_miio_instance_handle,value);
	return;
}

void P_4_3_FadeOn_doChange_notify(int newValue)
{
	arch_os_async_call(P_4_3_FadeOn_doChange_cb,(void *)newValue,500);
}


//===============================
void P_4_4_FadeOff_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 4, 4, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_4_4_FadeOff_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_4_4_FadeOff_doChange(g_miio_instance_handle,value);
	return;
}

void P_4_4_FadeOff_doChange_notify(int newValue)
{
	arch_os_async_call(P_4_4_FadeOff_doChange_cb,(void *)newValue,500);
}


//===============================
void P_4_5_FadeScen_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 4, 5, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_4_5_FadeScen_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_4_5_FadeScen_doChange(g_miio_instance_handle,value);
	return;
}

void P_4_5_FadeScen_doChange_notify(int newValue)
{
	arch_os_async_call(P_4_5_FadeScen_doChange_cb,(void *)newValue,500);
}

//===============================
void P_4_6_NightSwitch_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 4, 6, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_4_6_NightSwitch_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_4_6_NightSwitch_doChange(g_miio_instance_handle,value);
	return;
}

void P_4_6_NightSwitch_doChange_notify(int newValue)
{
	arch_os_async_call(P_4_6_NightSwitch_doChange_cb,(void *)newValue,500);
}


//===============================
void P_4_7_TimerOff_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 4, 6, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_4_7_TimerOff_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_4_7_TimerOff_doChange(g_miio_instance_handle,value);
	return;
}

void P_4_7_TimerOff_doChange_notify(int newValue)
{
	arch_os_async_call(P_4_7_TimerOff_doChange_cb,(void *)newValue,500);
}



//===============================
void P_4_8_LowestBrightness_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 4, 8, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_4_8_LowestBrightness_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_4_8_LowestBrightness_doChange(g_miio_instance_handle,value);
	return;
}

void P_4_8_LowestBrightness_doChange_notify(int newValue)
{
	arch_os_async_call(P_4_8_LowestBrightness_doChange_cb,(void *)newValue,500);
}


//===============================
void P_4_9_LockState_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 4, 9, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_4_9_LockState_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_4_9_LockState_doChange(g_miio_instance_handle,value);
	return;
}

void P_4_9_LockState_doChange_notify(int newValue)
{
	arch_os_async_call(P_4_9_LockState_doChange_cb,(void *)newValue,500);
}

//S8
//===============================
void P_8_1_RemoteSwitch_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 8, 1, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_8_1_RemoteSwitch_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_8_1_RemoteSwitch_doChange(g_miio_instance_handle,value);
	return;
}

void P_8_1_RemoteSwitch_doChange_notify(int newValue)
{
	arch_os_async_call(P_8_1_RemoteSwitch_doChange_cb,(void *)newValue,500);
}


//===============================
void P_8_4_RemoteLType_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 8, 4, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_8_4_RemoteLType_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_8_4_RemoteLType_doChange(g_miio_instance_handle,value);
	return;
}

void P_8_4_RemoteLType_doChange_notify(int newValue)
{
	arch_os_async_call(P_8_4_RemoteLType_doChange_cb,(void *)newValue,500);
}


//===============================
void P_8_5_RemoteHType_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 8, 5, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_8_5_RemoteHType_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_8_5_RemoteHType_doChange(g_miio_instance_handle,value);
	return;
}

void P_8_5_RemoteHType_doChange_notify(int newValue)
{
	arch_os_async_call(P_8_5_RemoteHType_doChange_cb,(void *)newValue,500);
}


//===============================
void P_8_6_Remote1Mac_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 8, 6, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_8_6_Remote1Mac_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_8_6_Remote1Mac_doChange(g_miio_instance_handle,value);
	return;
}

void P_8_6_Remote1Mac_doChange_notify(int newValue)
{
	arch_os_async_call(P_8_6_Remote1Mac_doChange_cb,(void *)newValue,500);
}

//===============================
void P_8_7_Remote2Mac_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 8, 7, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_8_7_Remote2Mac_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_8_7_Remote2Mac_doChange(g_miio_instance_handle,value);
	return;
}

void P_8_7_Remote2Mac_doChange_notify(int newValue)
{
	arch_os_async_call(P_8_7_Remote2Mac_doChange_cb,(void *)newValue,500);
}

//===============================
void P_8_8_Remote3Mac_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 8, 8, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_8_8_Remote3Mac_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_8_8_Remote3Mac_doChange(g_miio_instance_handle,value);
	return;
}

void P_8_8_Remote3Mac_doChange_notify(int newValue)
{
	arch_os_async_call(P_8_8_Remote3Mac_doChange_cb,(void *)newValue,500);
}

//===============================
void P_8_9_Remote4Mac_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 8, 9, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_8_9_Remote4Mac_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_8_9_Remote4Mac_doChange(g_miio_instance_handle,value);
	return;
}

void P_8_9_Remote4Mac_doChange_notify(int newValue)
{
	arch_os_async_call(P_8_9_Remote4Mac_doChange_cb,(void *)newValue,500);
}

//===============================
void P_8_10_Remote5Mac_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 8, 10, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_8_10_Remote5Mac_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_8_10_Remote5Mac_doChange(g_miio_instance_handle,value);
	return;
}

void P_8_10_Remote5Mac_doChange_notify(int newValue)
{
	arch_os_async_call(P_8_10_Remote5Mac_doChange_cb,(void *)newValue,500);
}

//===============================
void P_8_11_Remote6Mac_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 8, 11, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_8_11_Remote6Mac_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_8_11_Remote6Mac_doChange(g_miio_instance_handle,value);
	return;
}

void P_8_11_Remote6Mac_doChange_notify(int newValue)
{
	arch_os_async_call(P_8_11_Remote6Mac_doChange_cb,(void *)newValue,500);
}

//===============================
void P_8_12_Remote7Mac_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 8, 12, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_8_12_Remote7Mac_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_8_12_Remote7Mac_doChange(g_miio_instance_handle,value);
	return;
}

void P_8_12_Remote7Mac_doChange_notify(int newValue)
{
	arch_os_async_call(P_8_12_Remote7Mac_doChange_cb,(void *)newValue,500);
}

//===============================
void P_8_13_Remote8Mac_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 8, 13, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_8_13_Remote8Mac_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_8_13_Remote8Mac_doChange(g_miio_instance_handle,value);
	return;
}

void P_8_13_Remote8Mac_doChange_notify(int newValue)
{
	arch_os_async_call(P_8_13_Remote8Mac_doChange_cb,(void *)newValue,500);
}


//===============================
void P_8_14_RemoteCmd_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 8, 14, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_8_14_RemoteCmd_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_8_14_RemoteCmd_doChange(g_miio_instance_handle,value);
	return;
}

void P_8_14_RemoteCmd_doChange_notify(int newValue)
{
	arch_os_async_call(P_8_14_RemoteCmd_doChange_cb,(void *)newValue,500);
}



//===============================
void P_8_2_RemoteAdd_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 8, 2, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_8_2_RemoteAdd_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_8_2_RemoteAdd_doChange(g_miio_instance_handle,value);
	return;
}

void P_8_2_RemoteAdd_doChange_notify(int newValue)
{
	arch_os_async_call(P_8_2_RemoteAdd_doChange_cb,(void *)newValue,500);
}

//===============================
void P_8_3_RemoteDel_doChange(miio_handle_t handle, int newValue)
{
    if (send_property_changed(handle, 8, 3, property_value_new_integer(newValue)) != MIIO_OK)
    {
        LOG_ERROR_TAG(TAG, "send_property_changed failed!\n");
    }
}

static void P_8_3_RemoteDel_doChange_cb(void * newValue)
{
	int value = (int) newValue;
	if (NULL == g_miio_instance_handle)
	{
		LOG_ERROR("please create miio instance first!\r\n");
		return;
	}

	P_8_3_RemoteDel_doChange(g_miio_instance_handle,value);
	return;
}

void P_8_3_RemoteDel_doChange_notify(int newValue)
{
	arch_os_async_call(P_8_3_RemoteDel_doChange_cb,(void *)newValue,500);
}


