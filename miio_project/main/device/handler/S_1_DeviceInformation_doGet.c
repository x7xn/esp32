/**
* Copyright (C) 2013-2015
*
* @author ouyangchengfeng@xiaomi.com
* @date   2018-11-14
*
* @file   S_1_DeviceInformation_doGet.c
*
* @remark
*
*/

#include "S_1_DeviceInformation_doGet.h"
#include "../typedef/operation_code.h"
#include "../iid/iid.h"

/**
 * 格式: property_value_new_string(字符�?　
 * 取�? 字符串，没有限制取值范�? */
static void P_1_1_Manufacturer_doGet(property_operation_t *o)
{
    o->value = property_value_new_string("hello"); // TODO: 这里需要读到属性真正的�?
}

/**
 * 格式: property_value_new_string(字符�?　
 * 取�? 字符串，没有限制取值范�? */
static void P_1_2_Model_doGet(property_operation_t *o)
{
    o->value = property_value_new_string("hello"); // TODO: 这里需要读到属性真正的�?
}

/**
 * 格式: property_value_new_string(字符�?　
 * 取�? 字符串，没有限制取值范�? */
static void P_1_3_SerialNumber_doGet(property_operation_t *o)
{
    o->value = property_value_new_string("hello"); // TODO: 这里需要读到属性真正的�?
}

/**
 * 格式: property_value_new_string(字符�?　
 * 取�? 字符串，没有限制取值范�? */
static void P_1_4_FirmwareRevision_doGet(property_operation_t *o)
{
    o->value = property_value_new_string("hello"); // TODO: 这里需要读到属性真正的�?
}

static void P_1_5_SerialNo_doGet(property_operation_t *o)
{
    o->value = property_value_new_string("12345"); // TODO: 这里需要读到属性真正的�?
}

void S_1_DeviceInformation_doGet(property_operation_t *o)
{
    switch (o->piid)
    {
        case IID_1_1_Manufacturer:
            P_1_1_Manufacturer_doGet(o);
            break;

        case IID_1_2_Model:
            P_1_2_Model_doGet(o);
            break;

        case IID_1_3_SerialNumber:
            P_1_3_SerialNumber_doGet(o);
            break;

        case IID_1_4_FirmwareRevision:
            P_1_4_FirmwareRevision_doGet(o);
            break;

		case IID_1_5_SerialNo:
			P_1_5_SerialNo_doGet(o);
			break;

        default:
            o->code = OPERATION_ERROR_CANNOT_READ;
            break;
    }
}