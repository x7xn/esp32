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

#include "on_property_set.h"
#include "print_value.h"
#include "arch_dbg.h"
#include "../typedef/operation_code.h"
#include "../iid/iid.h"
#include "S_2_Light_doSet.h"

void on_property_set(property_operation_t *o)
{
    LOG_INFO("on_property_set: did = [%s], siid = [%d], piid = [%d]\n", o->did, o->siid, o->piid);
    
    if (o->value == NULL)
    {
    	LOG_ERROR("value is NULL\n");
    	return;
    }
    
    print_value(o->value);

    switch (o->siid)
    {
        case IID_2_Light:
            S_2_Light_doSet(o);
            break;

        case IID_3_Scens:
            S_3_Scens_doSet(o);
            break;

        case IID_4_UserSet:
        	S_4_UserMode_doSet(o);
        	break;

/*
        case IID_8_Remote:
			S_8_Remote_doSet(o);
			break;
*/

        default:
            o->code = OPERATION_OK;
            break;
    }
}
