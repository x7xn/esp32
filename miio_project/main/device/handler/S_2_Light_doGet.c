/**
* Copyright (C) 2013-2015
*
* @author ouyangchengfeng@xiaomi.com
* @date   2018-11-14
*
* @file   S_2_Light_doGet.c
*
* @remark
*
*/

#include "S_2_Light_doGet.h"
#include "../typedef/operation_code.h"
#include "../iid/iid.h"
#include "../../light.h"
#include "../../light_remote.h"


#if 0
/**
 * 闂佸搫绉堕崢褏妲愰敓锟�: property_value_new_boolean(true 闂佺鎻幏锟� false)闂侀潧妫撮幏锟�
 * 闂佸憡鐟﹂悧鏇㈡晸閺傘倖瀚�: true 闂佺懓鐡ㄩ悧鎾绘晸閺傘倖瀚筬alse
 */
static void P_2_1_On_doGet(property_operation_t *o)
{
    o->value = property_value_new_boolean(light_getlight_sw()); // TODO: 闁哄鏅滈悷鈺呭闯閻戣姤顥嗛柨鐔烘櫕閹茬増鎷呯粵瀣闂佸憡甯楀姗�鎯堝锟介獮锟界憸蹇擄耿閳ユ剚娼伴柨婵嗘处閻ｉ亶鏌涙繝蹇斿�?}
}
/**
 * 闂佸搫绉堕崢褏妲愰敓锟�: property_value_new_integer(闂佽桨绀侀悺銊╁汲閿燂�?闂侀潧妫撮幏锟�
 * 闂佸憡鐟﹂悧鏇㈡晸閺傘倖瀚�: 闂佸搫鐗為幏鐑芥倶韫囨挾绠伴柨鐔告灮閹凤�? 1, 闂佸搫鐗為幏宄邦熆閸棗鍠涢幏鐑芥晸閿燂�? 100, 濠殿喗绺块崕鐢稿汲閿燂拷: 1
 */
static void P_2_2_Brightness_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_brightness()); // TODO: 闁哄鏅滈悷鈺呭闯閻戣姤顥嗛柨鐔烘櫕閹茬増鎷呯粵瀣闂佸憡甯楀姗�鎯堝锟介獮锟界憸蹇擄耿閳ユ剚娼伴柨婵嗘处閻ｉ亶鏌涙繝蹇斿�?}
}
//閼瑰弶淇�?
static void P_2_3_ColorTemperature_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_templatetrue()); // TODO: 闁哄鏅滈悷鈺呭闯閻戣姤顥嗛柨鐔烘櫕閹茬増鎷呯粵瀣闂佸憡甯楀姗�鎯堝锟介獮锟界憸蹇擄耿閳ユ剚娼伴柨婵嗘处閻ｉ亶鏌涙繝蹇斿�?}
}

static void P_2_4_RGBColor_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_rgbcolor()); // TODO: 闁哄鏅滈悷鈺呭闯閻戣姤顥嗛柨鐔烘櫕閹茬増鎷呯粵瀣闂佸憡甯楀姗�鎯堝锟介獮锟界憸蹇擄耿閳ユ剚娼伴柨婵嗘处閻ｉ亶鏌涙繝蹇斿�?}
}
static void P_2_5_RGBMode_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_rgbmode()); // TODO: 闁哄鏅滈悷鈺呭闯閻戣姤顥嗛柨鐔烘櫕閹茬増鎷呯粵瀣闂佸憡甯楀姗�鎯堝锟介獮锟界憸蹇擄耿閳ユ剚娼伴柨婵嗘处閻ｉ亶鏌涙繝蹇斿�?}

}

//鐏靛姩寮�鍏�?
static void P_2_6_FlexSwitch_doGet(property_operation_t *o)
{
	o->value = property_value_new_boolean(light_getlight_smartswitch());
}
//鍔╃湢妯″紡
static void P_2_7_SleepMode_doGet(property_operation_t *o)
{
	o->value = property_value_new_boolean(light_getlight_sleep());
}
//娓呮櫒鍞ら啋
static void P_2_8_WeakupMode_doGet(property_operation_t *o)
{
	o->value = property_value_new_boolean(light_getlight_weakup());
}

//娓愬彉鏃堕棿
static void P_2_9_WLduration_doGet(property_operation_t *o)
{
	o->value = property_value_new_integer(light_getlight_modetime());
}

//鍞ら啋妯″紡鏈�缁堜寒搴�?
static void P_2_10_WeakpEbr_doGet(property_operation_t *o)
{
	o->value = property_value_new_integer(light_getlight_weakebr());
}

//鐫＄湢鐨勫紑濮嬩寒搴�?
static void P_2_11_SleepSbr_doGet(property_operation_t *o)
{
	o->value = property_value_new_integer(light_getlight_sleepsbr());
}

//鐏厜涓婄數鐘舵��
static void P_2_12_LightPONDefault_doGet(property_operation_t *o)
{
	o->value = property_value_new_integer(light_getlight_powerstate());
}
#endif
static void P_2_1_Group_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(get_service_aircondition_group()); 
}
static void P_2_18_NotifyName_doGet(property_operation_t *o)
{
    o->value = property_value_new_boolean(get_notify_name_flag()); // TODO: 闁哄鏅滈悷鈺呭闯閻戣姤顥嗛柨鐔烘櫕閹茬増鎷呯粵瀣闂佸憡甯楀姗�鎯堝锟介獮锟界憸蹇擄耿閳ユ剚娼伴柨婵嗘处閻ｉ亶鏌涙繝蹇斿�?}
}

static void P_2_19_AirconditionNum_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(get_service_aircondition_num()); // TODO: 闁哄鏅滈悷鈺呭闯閻戣姤顥嗛柨鐔烘櫕閹茬増鎷呯粵瀣闂佸憡甯楀姗�鎯堝锟介獮锟界憸蹇擄耿閳ユ剚娼伴柨婵嗘处閻ｉ亶鏌涙繝蹇斿�?}
}


/**
 */
static void P_2_2_GroupName_doGet(property_operation_t *o,uint8_t group_id)
{
    o->value = property_value_new_string(get_service_aircondition_name(group_id));
}

void S_2_Light_doGet(property_operation_t *o)
{
	switch (o->piid)
	{
		case 1:
		P_2_1_Group_doGet(o);
		break;

		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
			P_2_2_GroupName_doGet(o,o->piid-2);
			break;
		case 18:
			P_2_18_NotifyName_doGet(o);
			break;
		case 19:
			P_2_19_AirconditionNum_doGet(o);
			break;
		default:
		o->code = OPERATION_ERROR_CANNOT_READ;
		break;
	}
}


//s3
static void P_3_1_airconditionSW_doGet(property_operation_t *o)
{
   // o->value = property_value_new_integer(light_getlight_powerstate());
   o->value = property_value_new_boolean(get_service_aircondition_sw());
}

static void P_3_2_airconditionMode_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(get_service_aircondition_mode());
}

static void P_3_3_airconditionFault_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(get_service_aircondition_fault());
}

static void P_3_4_airconditionTemp_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(get_service_aircondition_temp());
}

//s3
void S_3_Scens_doGet(property_operation_t *o)
{
    switch (o->piid)
    {
    	case 1:
			P_3_1_airconditionSW_doGet(o);
			break;

		case 2:
			P_3_2_airconditionMode_doGet(o);
			break;

		case 3:
			P_3_3_airconditionFault_doGet(o);
			break;

		case 4:
			P_3_4_airconditionTemp_doGet(o);
			break;
        default:
            o->code = OPERATION_ERROR_CANNOT_READ;
            break;
    }
}

//s4
static void P_4_2_Defualt_doGet(property_operation_t *o)
{
   // o->value = property_value_new_integer(light_getlight_powerstate());
   o->value = property_value_new_integer(get_service_aircondition_fanspeed());
}

static void P_4_2_UserMode_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_usermode());
}

static void P_4_3_FadeOn_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_fadeon());
}

static void P_4_4_FadeOff_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_fadeoff());
}

static void P_4_5_FadeScen_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_fadescen());
}

static void P_4_6_NightSwitch_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_nightswitch());
}

static void P_4_7_TimerOff_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_timeroff());
}

static void P_4_8_LowestBrightness_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_lowestbrightness());
}

static void P_4_9_LockState_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_lockstate());
}

void S_4_UserMode_doGet(property_operation_t *o)
{
    switch (o->piid)
    {
		case IID_4_1_Defualt:
			//P_4_2_Defualt_doGet(o);
			break;

		case IID_4_2_UserMode:
			//P_4_2_UserMode_doGet(o);
			P_4_2_Defualt_doGet(o);
			break;

		case IID_4_3_FadeOn:
			P_4_3_FadeOn_doGet(o);
			break;

		case IID_4_4_FadeOff:
			P_4_4_FadeOff_doGet(o);
			break;

		case IID_4_5_FadeScen:
			P_4_5_FadeScen_doGet(o);
			break;

		case IID_4_6_NightSwitch:
			P_4_6_NightSwitch_doGet(o);
			break;

		case IID_4_7_TimerOff:
			P_4_7_TimerOff_doGet(o);
			break;

		case IID_4_8_LowestBrightness:
			P_4_8_LowestBrightness_doGet(o);
			break;

		case IID_4_9_LockState:
			P_4_9_LockState_doGet(o);
			break;

        default:
            o->code = OPERATION_ERROR_CANNOT_READ;
            break;
    }
}




//s8
static void P_8_1_RemoteSwitch_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_remoteswitch());
}


static void P_8_4_RemoteLType_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_remotetype(1));
}

static void P_8_5_RemoteHType_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_remotetype(0));
}

static void P_8_6_Remote1Mac_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_remotemac(1));
}

static void P_8_7_Remote2Mac_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_remotemac(2));
}

static void P_8_8_Remote3Mac_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_remotemac(3));
}

static void P_8_9_Remote4Mac_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_remotemac(4));
}

static void P_8_10_Remote5Mac_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_remotemac(5));
}

static void P_8_11_Remote6Mac_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_remotemac(6));
}

static void P_8_12_Remote7Mac_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_remotemac(7));
}

static void P_8_13_Remote8Mac_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_remotemac(8));
}

static void P_8_14_RemoteCmd_doGet(property_operation_t *o)
{
    o->value = property_value_new_integer(light_getlight_remotecmd());
}


void S_8_Remote_doGet(property_operation_t *o)
{
    switch (o->piid)
    {
		case IID_8_1_RemoteSwitch:
			P_8_1_RemoteSwitch_doGet(o);
			break;

		case IID_8_4_LType:
			P_8_4_RemoteLType_doGet(o);
			break;

		case IID_8_5_HType:
			P_8_5_RemoteHType_doGet(o);
			break;

		case IID_8_6_Remote1Mac:
			P_8_6_Remote1Mac_doGet(o);
			break;

		case IID_8_7_Remote2Mac:
			P_8_7_Remote2Mac_doGet(o);
			break;

		case IID_8_8_Remote3Mac:
			P_8_8_Remote3Mac_doGet(o);
			break;

		case IID_8_9_Remote4Mac:
			P_8_9_Remote4Mac_doGet(o);
			break;

		case IID_8_10_Remote5Mac:
			P_8_10_Remote5Mac_doGet(o);
			break;

		case IID_8_11_Remote6Mac:
			P_8_11_Remote6Mac_doGet(o);
			break;

		case IID_8_12_Remote7Mac:
			P_8_12_Remote7Mac_doGet(o);
			break;

		case IID_8_13_Remote8Mac:
			P_8_13_Remote8Mac_doGet(o);
			break;

		case IID_8_14_RemoteCmd:
			P_8_14_RemoteCmd_doGet(o);
			break;

        default:
            o->code = OPERATION_ERROR_CANNOT_READ;
            break;
    }
}
