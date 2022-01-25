/**
 * Copyright (C) 2013-2015
 *
 * @author ouyangchengfeng@xiaomi.com
 * @date   2018-10-29
 *
 * @file   on_action_invoke.c
 *
 * @remark
 *
 */

#include "arch_dbg.h"
#include "on_action_invoke.h"
#include "../typedef/operation_code.h"
#include "../iid/iid.h"
#include "../../light.h"

//weak up mode
void P_2_3_ActionSet_light_piid(action_operation_t *o,uint32_t num)
{
	LOG_INFO("piid: %d\n", o->in->arguments[num].piid);
	switch (o->in->arguments[num].piid)
    {
		case IID_2_9_WLduration:
			if(o->in->arguments[num].value->format == PROPERTY_FORMAT_NUMBER)
			{
				o->code = OPERATION_OK;
				light_setlight_modetime(o->in->arguments[num].value->data.number.value.integerValue);
			}else{
				o->code = OPERATION_ERROR_VALUE;
			}
			break;

		case IID_2_10_WeakpEbr:
			if(o->in->arguments[num].value->format == PROPERTY_FORMAT_NUMBER)
			{
				o->code = OPERATION_OK;
				light_setlight_weakebr(o->in->arguments[num].value->data.number.value.integerValue);
			}else{
				o->code = OPERATION_ERROR_VALUE;
			}
			break;

		case IID_2_11_SleepSbr:
			if(o->in->arguments[num].value->format == PROPERTY_FORMAT_NUMBER)
			{
				o->code = OPERATION_OK;
			}else{
				o->code = OPERATION_ERROR_VALUE;
			}
			break;
        default:
            o->code = OPERATION_ERROR_CANNOT_WRITE;
            break;
    }
}


void P_2_4_ActionSet_light_piid(action_operation_t *o,uint32_t num)
{
	switch (o->in->arguments[num].piid)
    {
        case IID_2_9_WLduration:
        	if(o->in->arguments[num].value->format == PROPERTY_FORMAT_NUMBER)
        	{
        		o->code = OPERATION_OK;
        		light_setlight_modetime(o->in->arguments[num].value->data.number.value.integerValue);
        	}else{
        		o->code = OPERATION_ERROR_VALUE;
        	}
            break;

        case IID_2_10_WeakpEbr:
        	if(o->in->arguments[num].value->format == PROPERTY_FORMAT_NUMBER)
        	{
        		o->code = OPERATION_OK;
        	}else{
        		o->code = OPERATION_ERROR_VALUE;
        	}
            break;

        case IID_2_11_SleepSbr:
        	if(o->in->arguments[num].value->format == PROPERTY_FORMAT_NUMBER)
        	{
        		o->code = OPERATION_OK;
        		light_setlight_sleepsbr(o->in->arguments[num].value->data.number.value.integerValue);
        	}else{
        		o->code = OPERATION_ERROR_VALUE;
        	}
            break;
        default:
            o->code = OPERATION_ERROR_CANNOT_WRITE;
            break;
    }
}

void on_action_light(action_operation_t *o)
{
	uint32_t  i;
	switch(o->aiid)
	{
	case AID_2_1_GetWeakup:
		o->out->size = 3;
		o->out->arguments[0].piid  = 9;
		o->out->arguments[0].value = property_value_new_integer(light_getlight_modetime());

		o->out->arguments[1].piid  = 10;
		o->out->arguments[1].value = property_value_new_integer(light_getlight_weakebr());

		o->out->arguments[2].piid  = 11;
		o->out->arguments[2].value = property_value_new_integer(1);
		o->code = OPERATION_OK;
		break;
	case AID_2_2_GetSleep:
		o->out->size = 3;
		o->out->arguments[0].piid  = 9;
		o->out->arguments[0].value = property_value_new_integer(light_getlight_modetime());

		o->out->arguments[1].piid  = 10;
		o->out->arguments[1].value = property_value_new_integer(0);

		o->out->arguments[2].piid  = 11;
		o->out->arguments[2].value = property_value_new_integer(light_getlight_sleepsbr());
		o->code = OPERATION_OK;
		break;
	case AID_2_3_SetWeakup://模式时间，最终亮度，开始亮度
		for(i=0;i<(o->in->size);i++)
		{
			P_2_3_ActionSet_light_piid(o,i);
		}
		o->code = OPERATION_OK;
		break;
	case AID_2_4_SetSleep://模式时间，最终亮度，开始亮度
		for(i=0;i<(o->in->size);i++)
		{
			P_2_4_ActionSet_light_piid(o,i);
		}
		o->code = OPERATION_OK;
		break;
    default:
        o->code = OPERATION_INVALID;
        break;
	}
}

#if 0
void on_action_aircondition(action_operation_t *o)
{
	uint32_t  i;
	switch(o->aiid)
	{
	case 1:
		o->out->size = 3;
		o->out->arguments[0].piid  = 9;
		o->out->arguments[0].value = property_value_new_integer(light_getlight_modetime());

		o->out->arguments[1].piid  = 10;
		o->out->arguments[1].value = property_value_new_integer(light_getlight_weakebr());

		o->out->arguments[2].piid  = 11;
		o->out->arguments[2].value = property_value_new_integer(1);
		o->code = OPERATION_OK;
		break;
	
    default:
        o->code = OPERATION_INVALID;
        break;
	}
}
#endif

void on_action_invoke(action_operation_t *o)
{
    LOG_INFO("on_action_invoke\n");
    LOG_INFO("did: %s\n", o->did);
    LOG_INFO("siid: %d\n", o->siid);
    LOG_INFO("aiid: %d\n", o->aiid);

    switch (o->siid)
    {
		case IID_2_Light:
			on_action_light(o);
		break;
		case 3:
			//on_action_light(o);
			aircondition_set_sw_toggle();
			o->out->size = 0;
/*
			o->out->arguments[0].piid  = 1;
			o->out->arguments[0].value = property_value_new_boolean(aircondition_get_sw());

			o->out->arguments[1].piid  = 10;
			o->out->arguments[1].value = property_value_new_integer(light_getlight_weakebr());

			o->out->arguments[2].piid  = 11;
			o->out->arguments[2].value = property_value_new_integer(1);
*/
			o->code = OPERATION_OK;
		break;

        default:
            o->code = OPERATION_INVALID;
            break;
    }
}
