/**
 * Copyright (C) 2013-2015
 *
 * @author ouyangchengfeng@xiaomi.com
 * @date   2018-10-29
 *
 * @file   on_property_get.c
 *
 * @remark
 *
 */

#include "on_property_get.h"
#include "print_value.h"
#include "arch_dbg.h"
#include "../typedef/operation_code.h"
#include "../iid/iid.h"
#include "S_1_DeviceInformation_doGet.h"
#include "S_2_Light_doGet.h"

void on_property_get(property_operation_t *o)
{
    LOG_INFO("on_property_get\n");
    LOG_INFO("did: %s\n", o->did);
	LOG_INFO("siid: %d\n", o->siid);
	LOG_INFO("piid: %d\n", o->piid);

    switch (o->siid)
    {
            case IID_1_DeviceInformation:
            	S_1_DeviceInformation_doGet(o);
            break;

            case IID_2_Light:
            	S_2_Light_doGet(o);
            break;

            case IID_3_Scens:
            	S_3_Scens_doGet(o);
            break;

            case IID_4_UserSet:
            	S_4_UserMode_doGet(o);
            break;

            case IID_8_Remote:
            	S_8_Remote_doGet(o);
			break;

        default:
            o->code = OPERATION_INVALID;
            break;
    }
}
