/**
* Copyright (C) 2013-2015
*
* @author ouyangchengfeng@xiaomi.com
* @date   2018-11-14
*
* @file   S_2_Light_doSet.c
*
* @remark
*
*/

#include "S_2_Light_doSet.h"
#include "S_2_Light_doChange.h"
#include "../typedef/operation_code.h"
#include "../iid/iid.h"
#include "../../light.h"
#include "../../light_remote.h"


#define GROUP_NAME_SIZE	((uint8_t)37)
#if 0
static void P_2_1_Group_doSet(property_operation_t *o)
{
    // 闂佸憡甯囬崐鏍蓟閸ヮ剙鏋侀柣妤�鐗嗙粊锕傛煛瀹ュ洤甯剁紒鎲嬬節瀵即顢涘顓炲墾濠殿喗绻愮徊鍧楀灳濮楋拷閺佸秶浠︾粵瀣悙闂佸搫顑嗙划鎾诲极婵犲嫭瀚氭い鏍ュ�楃粈澶愬级閳哄倹鐓ユ繛鍙夌墬缁傛帡鏁愰崶鈺佺仯: OPERATION_ERROR_VALUE
    if (o->value->format != PROPERTY_FORMAT_BOOLEAN)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

    // TODO: 闂佸湱鐟抽崱鈺傛杸闂佸憡鍔栭悷锕傚箹闁垮濯撮柨鐕傛�? o->value->data.boolean.value;
    light_setlight_sw(o->value->data.boolean.value);

    // 婵犵锟藉啿锟藉綊鎮樻径鎰闁归偊鍓欓～鐘绘煥濞戞鐒风紒缁樺灴瀹曞爼鎮欓弶鎸庢殨闂佹椿鍣幏锟�? OPERATION_OK
    o->code = OPERATION_OK;
    
    //婵炴垶鎸搁敃銉︽叏閵堝鍋愰柤鍝ヮ暜閹烽攱绗熸繝鍕槷闂備緡鍋呭銊╂偂椤☆湺p闂佺粯顭堥崺鏍晸閹存帞顦︾憸鏉款樀瀹曠娀寮介悽鐢殿槷缂傚倷鑳堕崰宥囩博绾板尙P婵烇絽娴傞崰妤呭极婵傜违濞达絽鎽滈弳浼存煛閸愵厽纭炬繛鍛笧閹叉挳鏁冮敓鍊熴亹閸岀偛瑙﹂幖杈剧稻閻ｅ崬鈽夐幘绛规敾濠殿喓鍊濆鐢稿传閸曨偆鍘�
    P_2_1_Group_doChange_notify(o->value->data.boolean.value);

    return;
}

#else
static void P_2_1_Group_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_NUMBER)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

	if((o->value->data.number.value.integerValue) > 16)
	{
		//arch_printf("Group:%d\n",(int)o->value->data.number.value.integerValue);
		
    	LOG_ERROR("p2-1-value error\n");

		o->code = OPERATION_ERROR_VALUE;
		return;
	}

	//ִ����Ӧ����
    update_service_aircondition_group(o->value->data.number.value.integerValue);

	//����OK
    o->code = OPERATION_OK;
    
    //
    //P_2_1_Group_doChange_notify(o->value->data.number.value.integerValue);

    return;
}
#endif

static void P_2_18_NotifyGroupName_doSet(property_operation_t *o)
{
	if (o->value->format != PROPERTY_FORMAT_NUMBER)
	{
		o->code = OPERATION_ERROR_VALUE;
		return;
	}

	if((o->value->data.number.value.integerValue) > 2)
	{
		//arch_printf("Notify:%d\n",(int)o->value->data.number.value.integerValue);
		
		LOG_ERROR("p2-18-value error\n");

		o->code = OPERATION_ERROR_VALUE;
		return;
	}
	
	aircondition_set_notify_name(o->value->data.number.value.integerValue);

	// 婵犵锟藉啿锟藉綊鎮樻径鎰闁归偊鍓欓～鐘绘煥濞戞鐒风紒缁樺灴瀹曞爼鎮欓弶鎸庢殨闂佹椿鍣幏锟�? OPERATION_OK
	o->code = OPERATION_OK;
	
	//婵炴垶鎸搁敃銉︽叏閵堝鍋愰柤鍝ヮ暜閹烽攱绗熸繝鍕槷闂備緡鍋呭銊╂偂椤☆湺p闂佺粯顭堥崺鏍晸閹存帞顦︾憸鏉款樀瀹曠娀寮介悽鐢殿槷缂傚倷鑳堕崰宥囩博绾板尙P婵烇絽娴傞崰妤呭极婵傜违濞达絽鎽滈弳浼存煛閸愵厽纭炬繛鍛笧閹叉挳鏁冮敓鍊熴亹閸岀偛瑙﹂幖杈剧稻閻ｅ崬鈽夐幘绛规敾濠殿喓鍊濆鐢稿传閸曨偆鍘�
	P_2_18_NotifyGroupName_doChange_notify(o->value->data.number.value.integerValue);

	return;
}


#if 0
static void P_2_2_Brightness_doSet(property_operation_t *o)
{
    // 闂佸憡甯囬崐鏍蓟閸ヮ剙鏋侀柣妤�鐗嗙粊锕傛煛瀹ュ洤甯剁紒鎲嬬節瀵即顢涘顓炲墾濠殿喗绻愮徊鍧楀灳濮楋拷閺佸秶浠︾粵瀣悙闂佸搫顑嗙划鎾诲极婵犲嫭瀚氭い鏍ュ�楃粈澶愬级閳哄倹鐓ユ繛鍙夌墬缁傛帡鏁愰崶鈺佺仯: OPERATION_ERROR_VALUE
    if (o->value->format != PROPERTY_FORMAT_NUMBER)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

    // 闂佸憡甯囬崐鏍蓟閸ヮ剙鏋侀柣妤�鐗嗙粊锕傛煕濞嗘劗澧柨鐔凤拷鐔奉棈閻庡灚鐗犲畷鍓侊拷娑櫳戠瑧闂佸憡鐔粻鎴﹀箖鎼粹檧鏋栭柡鍥朵簽缁�澶娾攽閳ュ啿锟藉綊鎮樻径瀣枖鐎广儱鎳忛崐銈嗙箾婢跺绌跨紒杈ㄧ箖濞煎寮幐搴ｎ槬婵炲濯寸徊鍧楁偉閿燂拷: OPERATION_ERROR_VALUE
    if (o->value->data.number.value.integerValue < 1 || o->value->data.number.value.integerValue > 100)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

    // TODO: 闂佸湱鐟抽崱鈺傛杸闂佸憡鍔栭悷锕傚箹闁垮濯撮柨鐕傛�? o->value->data.number.value.integerValue;
    light_setlight_brightness(o->value->data.number.value.integerValue);

    // 婵犵锟藉啿锟藉綊鎮樻径鎰闁归偊鍓欓～鐘绘煥濞戞鐒风紒缁樺灴瀹曞爼鎮欓弶鎸庢殨闂佹椿鍣幏锟�? OPERATION_OK
    o->code = OPERATION_OK;
    
    //婵炴垶鎸搁敃銉︽叏閵堝鍋愰柤鍝ヮ暜閹烽攱绗熸繝鍕槷闂備緡鍋呭銊╂偂椤☆湺p闂佺粯顭堥崺鏍晸閹存帞顦︾憸鏉款樀瀹曠娀寮介悽鐢殿槷缂傚倷鑳堕崰宥囩博绾板尙P婵烇絽娴傞崰妤呭极婵傜违濞达絽鎽滈弳浼存煛閸愵厽纭炬繛鍛笧閹叉挳鏁冮敓鍊熴亹閸岀偛瑙﹂幖杈剧稻閻ｅ崬鈽夐幘绛规敾濠殿喓鍊濆鐢稿传閸曨偆鍘�
    P_2_2_Name_doChange_notify(o->value->data.number.value.integerValue);

    return;
}
#else
static void P_2_2_Group0Name_doSet(property_operation_t *o)
{
	//����ֵ��ʽ���Ϸ�
    if (o->value->format != PROPERTY_FORMAT_STRING)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

	
	//����ֵ���ݲ��Ϸ�
	if(o->value->data.string.length > GROUP_NAME_SIZE)
	{
		//arch_printf("Name:%s\n",(char*)o->value->data.string.value);
		
    	LOG_ERROR("p2-2-value error\n");

		o->code = OPERATION_ERROR_VALUE;
		return;
	}
	
	//arch_printf("Name0:%d,%s\r\n",o->value->data.string.length,o->value->data.string.value);

	//����д��״̬
    o->code = OPERATION_OK;
	//���¶�Ӧ�յ���name
	update_service_aircondition_name(0,o->value->data.string.value,o->value->data.string.length);

	//����notify֪ͨƽ̨
    //P_2_2_Name_doChange_notify(o->value->data.string.value);

    return;
}

static void P_2_3_Group1Name_doSet(property_operation_t *o)
{
    // 闂佸憡甯囬崐鏍蓟閸ヮ剙鏋侀柣妤�鐗嗙粊锕傛煛瀹ュ洤甯剁紒鎲嬬節瀵即顢涘顓炲墾濠殿喗绻愮徊鍧楀灳濮楋拷閺佸秶浠︾粵瀣悙闂佸搫顑嗙划鎾诲极婵犲嫭瀚氭い鏍ュ�楃粈澶愬级閳哄倹鐓ユ繛鍙夌墬缁傛帡鏁愰崶鈺佺仯: OPERATION_ERROR_VALUE
    if (o->value->format != PROPERTY_FORMAT_STRING)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

	if(o->value->data.string.length > GROUP_NAME_SIZE)
	{
		//arch_printf("Name:%s\n",(char*)o->value->data.string.value);
		
    	LOG_ERROR("p2-3-value error\n");

		o->code = OPERATION_ERROR_VALUE;
		return;
	}

    // 婵犵锟藉啿锟藉綊鎮樻径鎰闁归偊鍓欓～鐘绘煥濞戞鐒风紒缁樺灴瀹曞爼鎮欓弶鎸庢殨闂佹椿鍣幏锟�? OPERATION_OK
    o->code = OPERATION_OK;
	update_service_aircondition_name(1,o->value->data.string.value,o->value->data.string.length);
    
    //婵炴垶鎸搁敃銉︽叏閵堝鍋愰柤鍝ヮ暜閹烽攱绗熸繝鍕槷闂備緡鍋呭銊╂偂椤☆湺p闂佺粯顭堥崺鏍晸閹存帞顦︾憸鏉款樀瀹曠娀寮介悽鐢殿槷缂傚倷鑳堕崰宥囩博绾板尙P婵烇絽娴傞崰妤呭极婵傜违濞达絽鎽滈弳浼存煛閸愵厽纭炬繛鍛笧閹叉挳鏁冮敓鍊熴亹閸岀偛瑙﹂幖杈剧稻閻ｅ崬鈽夐幘绛规敾濠殿喓鍊濆鐢稿传閸曨偆鍘�
    //P_2_3_Name_doChange_notify(o->value->data.string.value);

    return;
}
static void P_2_4_Group2Name_doSet(property_operation_t *o)
{
    // 闂佸憡甯囬崐鏍蓟閸ヮ剙鏋侀柣妤�鐗嗙粊锕傛煛瀹ュ洤甯剁紒鎲嬬節瀵即顢涘顓炲墾濠殿喗绻愮徊鍧楀灳濮楋拷閺佸秶浠︾粵瀣悙闂佸搫顑嗙划鎾诲极婵犲嫭瀚氭い鏍ュ�楃粈澶愬级閳哄倹鐓ユ繛鍙夌墬缁傛帡鏁愰崶鈺佺仯: OPERATION_ERROR_VALUE
    if (o->value->format != PROPERTY_FORMAT_STRING)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

	if(o->value->data.string.length > GROUP_NAME_SIZE)
	{
		//arch_printf("Name:%s\n",(char*)o->value->data.string.value);
		
    	LOG_ERROR("p2-4-value error\n");

		o->code = OPERATION_ERROR_VALUE;
		return;
	}

    // 婵犵锟藉啿锟藉綊鎮樻径鎰闁归偊鍓欓～鐘绘煥濞戞鐒风紒缁樺灴瀹曞爼鎮欓弶鎸庢殨闂佹椿鍣幏锟�? OPERATION_OK
    o->code = OPERATION_OK;
	update_service_aircondition_name(2,o->value->data.string.value,o->value->data.string.length);
    
    //婵炴垶鎸搁敃銉︽叏閵堝鍋愰柤鍝ヮ暜閹烽攱绗熸繝鍕槷闂備緡鍋呭銊╂偂椤☆湺p闂佺粯顭堥崺鏍晸閹存帞顦︾憸鏉款樀瀹曠娀寮介悽鐢殿槷缂傚倷鑳堕崰宥囩博绾板尙P婵烇絽娴傞崰妤呭极婵傜违濞达絽鎽滈弳浼存煛閸愵厽纭炬繛鍛笧閹叉挳鏁冮敓鍊熴亹閸岀偛瑙﹂幖杈剧稻閻ｅ崬鈽夐幘绛规敾濠殿喓鍊濆鐢稿传閸曨偆鍘�
    //P_2_4_Name_doChange_notify(o->value->data.string.value);

    return;
}

static void P_2_5_Group3Name_doSet(property_operation_t *o)
{
    // 闂佸憡甯囬崐鏍蓟閸ヮ剙鏋侀柣妤�鐗嗙粊锕傛煛瀹ュ洤甯剁紒鎲嬬節瀵即顢涘顓炲墾濠殿喗绻愮徊鍧楀灳濮楋拷閺佸秶浠︾粵瀣悙闂佸搫顑嗙划鎾诲极婵犲嫭瀚氭い鏍ュ�楃粈澶愬级閳哄倹鐓ユ繛鍙夌墬缁傛帡鏁愰崶鈺佺仯: OPERATION_ERROR_VALUE
    if (o->value->format != PROPERTY_FORMAT_STRING)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

	if(o->value->data.string.length > GROUP_NAME_SIZE)
	{
		//arch_printf("Name:%s\n",(char*)o->value->data.string.value);
		
    	LOG_ERROR("p2-5-value error\n");

		o->code = OPERATION_ERROR_VALUE;
		return;
	}

    // 婵犵锟藉啿锟藉綊鎮樻径鎰闁归偊鍓欓～鐘绘煥濞戞鐒风紒缁樺灴瀹曞爼鎮欓弶鎸庢殨闂佹椿鍣幏锟�? OPERATION_OK
    o->code = OPERATION_OK;
	update_service_aircondition_name(3,o->value->data.string.value,o->value->data.string.length);
    
    //婵炴垶鎸搁敃銉︽叏閵堝鍋愰柤鍝ヮ暜閹烽攱绗熸繝鍕槷闂備緡鍋呭銊╂偂椤☆湺p闂佺粯顭堥崺鏍晸閹存帞顦︾憸鏉款樀瀹曠娀寮介悽鐢殿槷缂傚倷鑳堕崰宥囩博绾板尙P婵烇絽娴傞崰妤呭极婵傜违濞达絽鎽滈弳浼存煛閸愵厽纭炬繛鍛笧閹叉挳鏁冮敓鍊熴亹閸岀偛瑙﹂幖杈剧稻閻ｅ崬鈽夐幘绛规敾濠殿喓鍊濆鐢稿传閸曨偆鍘�
    //P_2_5_Name_doChange_notify(o->value->data.string.value);
    return;
}

static void P_2_6_Group4Name_doSet(property_operation_t *o)
{
    // 闂佸憡甯囬崐鏍蓟閸ヮ剙鏋侀柣妤�鐗嗙粊锕傛煛瀹ュ洤甯剁紒鎲嬬節瀵即顢涘顓炲墾濠殿喗绻愮徊鍧楀灳濮楋拷閺佸秶浠︾粵瀣悙闂佸搫顑嗙划鎾诲极婵犲嫭瀚氭い鏍ュ�楃粈澶愬级閳哄倹鐓ユ繛鍙夌墬缁傛帡鏁愰崶鈺佺仯: OPERATION_ERROR_VALUE
    if (o->value->format != PROPERTY_FORMAT_STRING)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

	if(o->value->data.string.length > GROUP_NAME_SIZE)
	{
		//arch_printf("Name:%s\n",(char*)o->value->data.string.value);
		
    	LOG_ERROR("p2-6-value error\n");

		o->code = OPERATION_ERROR_VALUE;
		return;
	}

    // 婵犵锟藉啿锟藉綊鎮樻径鎰闁归偊鍓欓～鐘绘煥濞戞鐒风紒缁樺灴瀹曞爼鎮欓弶鎸庢殨闂佹椿鍣幏锟�? OPERATION_OK
    o->code = OPERATION_OK;
	update_service_aircondition_name(4,o->value->data.string.value,o->value->data.string.length);
    
    //婵炴垶鎸搁敃銉︽叏閵堝鍋愰柤鍝ヮ暜閹烽攱绗熸繝鍕槷闂備緡鍋呭銊╂偂椤☆湺p闂佺粯顭堥崺鏍晸閹存帞顦︾憸鏉款樀瀹曠娀寮介悽鐢殿槷缂傚倷鑳堕崰宥囩博绾板尙P婵烇絽娴傞崰妤呭极婵傜违濞达絽鎽滈弳浼存煛閸愵厽纭炬繛鍛笧閹叉挳鏁冮敓鍊熴亹閸岀偛瑙﹂幖杈剧稻閻ｅ崬鈽夐幘绛规敾濠殿喓鍊濆鐢稿传閸曨偆鍘�
    //P_2_6_Name_doChange_notify(o->value->data.string.value);

    return;
}
static void P_2_7_Group5Name_doSet(property_operation_t *o)
{
    // 闂佸憡甯囬崐鏍蓟閸ヮ剙鏋侀柣妤�鐗嗙粊锕傛煛瀹ュ洤甯剁紒鎲嬬節瀵即顢涘顓炲墾濠殿喗绻愮徊鍧楀灳濮楋拷閺佸秶浠︾粵瀣悙闂佸搫顑嗙划鎾诲极婵犲嫭瀚氭い鏍ュ�楃粈澶愬级閳哄倹鐓ユ繛鍙夌墬缁傛帡鏁愰崶鈺佺仯: OPERATION_ERROR_VALUE
    if (o->value->format != PROPERTY_FORMAT_STRING)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

	if(o->value->data.string.length > GROUP_NAME_SIZE)
	{
		//arch_printf("Name:%s\n",(char*)o->value->data.string.value);
		
    	LOG_ERROR("p2-7-value error\n");

		o->code = OPERATION_ERROR_VALUE;
		return;
	}

    // 婵犵锟藉啿锟藉綊鎮樻径鎰闁归偊鍓欓～鐘绘煥濞戞鐒风紒缁樺灴瀹曞爼鎮欓弶鎸庢殨闂佹椿鍣幏锟�? OPERATION_OK
    o->code = OPERATION_OK;
	update_service_aircondition_name(5,o->value->data.string.value,o->value->data.string.length);
    
    //婵炴垶鎸搁敃銉︽叏閵堝鍋愰柤鍝ヮ暜閹烽攱绗熸繝鍕槷闂備緡鍋呭銊╂偂椤☆湺p闂佺粯顭堥崺鏍晸閹存帞顦︾憸鏉款樀瀹曠娀寮介悽鐢殿槷缂傚倷鑳堕崰宥囩博绾板尙P婵烇絽娴傞崰妤呭极婵傜违濞达絽鎽滈弳浼存煛閸愵厽纭炬繛鍛笧閹叉挳鏁冮敓鍊熴亹閸岀偛瑙﹂幖杈剧稻閻ｅ崬鈽夐幘绛规敾濠殿喓鍊濆鐢稿传閸曨偆鍘�
    //P_2_7_Name_doChange_notify(o->value->data.string.value);

    return;
}

static void P_2_8_Group6Name_doSet(property_operation_t *o)
{
    // 闂佸憡甯囬崐鏍蓟閸ヮ剙鏋侀柣妤�鐗嗙粊锕傛煛瀹ュ洤甯剁紒鎲嬬節瀵即顢涘顓炲墾濠殿喗绻愮徊鍧楀灳濮楋拷閺佸秶浠︾粵瀣悙闂佸搫顑嗙划鎾诲极婵犲嫭瀚氭い鏍ュ�楃粈澶愬级閳哄倹鐓ユ繛鍙夌墬缁傛帡鏁愰崶鈺佺仯: OPERATION_ERROR_VALUE
    if (o->value->format != PROPERTY_FORMAT_STRING)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

	if(o->value->data.string.length > GROUP_NAME_SIZE)
	{
		//arch_printf("Name:%s\n",(char*)o->value->data.string.value);
		
    	LOG_ERROR("p2-8-value error\n");

		o->code = OPERATION_ERROR_VALUE;
		return;
	}

    // 婵犵锟藉啿锟藉綊鎮樻径鎰闁归偊鍓欓～鐘绘煥濞戞鐒风紒缁樺灴瀹曞爼鎮欓弶鎸庢殨闂佹椿鍣幏锟�? OPERATION_OK
    o->code = OPERATION_OK;
	update_service_aircondition_name(6,o->value->data.string.value,o->value->data.string.length);
    
    //婵炴垶鎸搁敃銉︽叏閵堝鍋愰柤鍝ヮ暜閹烽攱绗熸繝鍕槷闂備緡鍋呭銊╂偂椤☆湺p闂佺粯顭堥崺鏍晸閹存帞顦︾憸鏉款樀瀹曠娀寮介悽鐢殿槷缂傚倷鑳堕崰宥囩博绾板尙P婵烇絽娴傞崰妤呭极婵傜违濞达絽鎽滈弳浼存煛閸愵厽纭炬繛鍛笧閹叉挳鏁冮敓鍊熴亹閸岀偛瑙﹂幖杈剧稻閻ｅ崬鈽夐幘绛规敾濠殿喓鍊濆鐢稿传閸曨偆鍘�
    //P_2_8_Name_doChange_notify(o->value->data.string.value);

    return;
}
static void P_2_9_Group7Name_doSet(property_operation_t *o)
{
    // 闂佸憡甯囬崐鏍蓟閸ヮ剙鏋侀柣妤�鐗嗙粊锕傛煛瀹ュ洤甯剁紒鎲嬬節瀵即顢涘顓炲墾濠殿喗绻愮徊鍧楀灳濮楋拷閺佸秶浠︾粵瀣悙闂佸搫顑嗙划鎾诲极婵犲嫭瀚氭い鏍ュ�楃粈澶愬级閳哄倹鐓ユ繛鍙夌墬缁傛帡鏁愰崶鈺佺仯: OPERATION_ERROR_VALUE
    if (o->value->format != PROPERTY_FORMAT_STRING)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

	if(o->value->data.string.length > GROUP_NAME_SIZE)
	{
		//arch_printf("Name:%s\n",(char*)o->value->data.string.value);
		
    	LOG_ERROR("p2-9-value error\n");

		o->code = OPERATION_ERROR_VALUE;
		return;
	}

    // 婵犵锟藉啿锟藉綊鎮樻径鎰闁归偊鍓欓～鐘绘煥濞戞鐒风紒缁樺灴瀹曞爼鎮欓弶鎸庢殨闂佹椿鍣幏锟�? OPERATION_OK
    o->code = OPERATION_OK;
	update_service_aircondition_name(7,o->value->data.string.value,o->value->data.string.length);
    
    //婵炴垶鎸搁敃銉︽叏閵堝鍋愰柤鍝ヮ暜閹烽攱绗熸繝鍕槷闂備緡鍋呭銊╂偂椤☆湺p闂佺粯顭堥崺鏍晸閹存帞顦︾憸鏉款樀瀹曠娀寮介悽鐢殿槷缂傚倷鑳堕崰宥囩博绾板尙P婵烇絽娴傞崰妤呭极婵傜违濞达絽鎽滈弳浼存煛閸愵厽纭炬繛鍛笧閹叉挳鏁冮敓鍊熴亹閸岀偛瑙﹂幖杈剧稻閻ｅ崬鈽夐幘绛规敾濠殿喓鍊濆鐢稿传閸曨偆鍘�
    //P_2_9_Name_doChange_notify(o->value->data.string.value);

    return;
}

static void P_2_10_Group8Name_doSet(property_operation_t *o)
{
    // 闂佸憡甯囬崐鏍蓟閸ヮ剙鏋侀柣妤�鐗嗙粊锕傛煛瀹ュ洤甯剁紒鎲嬬節瀵即顢涘顓炲墾濠殿喗绻愮徊鍧楀灳濮楋拷閺佸秶浠︾粵瀣悙闂佸搫顑嗙划鎾诲极婵犲嫭瀚氭い鏍ュ�楃粈澶愬级閳哄倹鐓ユ繛鍙夌墬缁傛帡鏁愰崶鈺佺仯: OPERATION_ERROR_VALUE
    if (o->value->format != PROPERTY_FORMAT_STRING)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

	if(o->value->data.string.length > GROUP_NAME_SIZE)
	{
		//arch_printf("Name:%s\n",(char*)o->value->data.string.value);
		
    	LOG_ERROR("p2-10-value error\n");

		o->code = OPERATION_ERROR_VALUE;
		return;
	}

    // 婵犵锟藉啿锟藉綊鎮樻径鎰闁归偊鍓欓～鐘绘煥濞戞鐒风紒缁樺灴瀹曞爼鎮欓弶鎸庢殨闂佹椿鍣幏锟�? OPERATION_OK
    o->code = OPERATION_OK;
	update_service_aircondition_name(8,o->value->data.string.value,o->value->data.string.length);
    
    //婵炴垶鎸搁敃銉︽叏閵堝鍋愰柤鍝ヮ暜閹烽攱绗熸繝鍕槷闂備緡鍋呭銊╂偂椤☆湺p闂佺粯顭堥崺鏍晸閹存帞顦︾憸鏉款樀瀹曠娀寮介悽鐢殿槷缂傚倷鑳堕崰宥囩博绾板尙P婵烇絽娴傞崰妤呭极婵傜违濞达絽鎽滈弳浼存煛閸愵厽纭炬繛鍛笧閹叉挳鏁冮敓鍊熴亹閸岀偛瑙﹂幖杈剧稻閻ｅ崬鈽夐幘绛规敾濠殿喓鍊濆鐢稿传閸曨偆鍘�
    //P_2_10_Name_doChange_notify(o->value->data.string.value);

    return;
}
static void P_2_11_Group9Name_doSet(property_operation_t *o)
{
    // 闂佸憡甯囬崐鏍蓟閸ヮ剙鏋侀柣妤�鐗嗙粊锕傛煛瀹ュ洤甯剁紒鎲嬬節瀵即顢涘顓炲墾濠殿喗绻愮徊鍧楀灳濮楋拷閺佸秶浠︾粵瀣悙闂佸搫顑嗙划鎾诲极婵犲嫭瀚氭い鏍ュ�楃粈澶愬级閳哄倹鐓ユ繛鍙夌墬缁傛帡鏁愰崶鈺佺仯: OPERATION_ERROR_VALUE
    if (o->value->format != PROPERTY_FORMAT_STRING)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

	if(o->value->data.string.length > GROUP_NAME_SIZE)
	{
		//arch_printf("Name:%s\n",(char*)o->value->data.string.value);
		
    	LOG_ERROR("p2-11-value error\n");

		o->code = OPERATION_ERROR_VALUE;
		return;
	}

    // 婵犵锟藉啿锟藉綊鎮樻径鎰闁归偊鍓欓～鐘绘煥濞戞鐒风紒缁樺灴瀹曞爼鎮欓弶鎸庢殨闂佹椿鍣幏锟�? OPERATION_OK
    o->code = OPERATION_OK;
	update_service_aircondition_name(9,o->value->data.string.value,o->value->data.string.length);
    
    //婵炴垶鎸搁敃銉︽叏閵堝鍋愰柤鍝ヮ暜閹烽攱绗熸繝鍕槷闂備緡鍋呭銊╂偂椤☆湺p闂佺粯顭堥崺鏍晸閹存帞顦︾憸鏉款樀瀹曠娀寮介悽鐢殿槷缂傚倷鑳堕崰宥囩博绾板尙P婵烇絽娴傞崰妤呭极婵傜违濞达絽鎽滈弳浼存煛閸愵厽纭炬繛鍛笧閹叉挳鏁冮敓鍊熴亹閸岀偛瑙﹂幖杈剧稻閻ｅ崬鈽夐幘绛规敾濠殿喓鍊濆鐢稿传閸曨偆鍘�
    //P_2_11_Name_doChange_notify(o->value->data.string.value);

    return;
}
static void P_2_12_Group10Name_doSet(property_operation_t *o)
{
    // 闂佸憡甯囬崐鏍蓟閸ヮ剙鏋侀柣妤�鐗嗙粊锕傛煛瀹ュ洤甯剁紒鎲嬬節瀵即顢涘顓炲墾濠殿喗绻愮徊鍧楀灳濮楋拷閺佸秶浠︾粵瀣悙闂佸搫顑嗙划鎾诲极婵犲嫭瀚氭い鏍ュ�楃粈澶愬级閳哄倹鐓ユ繛鍙夌墬缁傛帡鏁愰崶鈺佺仯: OPERATION_ERROR_VALUE
    if (o->value->format != PROPERTY_FORMAT_STRING)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

	if(o->value->data.string.length > GROUP_NAME_SIZE)
	{
		//arch_printf("Name:%s\n",(char*)o->value->data.string.value);
		
    	LOG_ERROR("p2-12-value error\n");

		o->code = OPERATION_ERROR_VALUE;
		return;
	}

    // 婵犵锟藉啿锟藉綊鎮樻径鎰闁归偊鍓欓～鐘绘煥濞戞鐒风紒缁樺灴瀹曞爼鎮欓弶鎸庢殨闂佹椿鍣幏锟�? OPERATION_OK
    o->code = OPERATION_OK;
	update_service_aircondition_name(10,o->value->data.string.value,o->value->data.string.length);
    
    //婵炴垶鎸搁敃銉︽叏閵堝鍋愰柤鍝ヮ暜閹烽攱绗熸繝鍕槷闂備緡鍋呭銊╂偂椤☆湺p闂佺粯顭堥崺鏍晸閹存帞顦︾憸鏉款樀瀹曠娀寮介悽鐢殿槷缂傚倷鑳堕崰宥囩博绾板尙P婵烇絽娴傞崰妤呭极婵傜违濞达絽鎽滈弳浼存煛閸愵厽纭炬繛鍛笧閹叉挳鏁冮敓鍊熴亹閸岀偛瑙﹂幖杈剧稻閻ｅ崬鈽夐幘绛规敾濠殿喓鍊濆鐢稿传閸曨偆鍘�
    //P_2_12_Name_doChange_notify(o->value->data.string.value);

    return;
}
static void P_2_13_Group11Name_doSet(property_operation_t *o)
{
    // 闂佸憡甯囬崐鏍蓟閸ヮ剙鏋侀柣妤�鐗嗙粊锕傛煛瀹ュ洤甯剁紒鎲嬬節瀵即顢涘顓炲墾濠殿喗绻愮徊鍧楀灳濮楋拷閺佸秶浠︾粵瀣悙闂佸搫顑嗙划鎾诲极婵犲嫭瀚氭い鏍ュ�楃粈澶愬级閳哄倹鐓ユ繛鍙夌墬缁傛帡鏁愰崶鈺佺仯: OPERATION_ERROR_VALUE
    if (o->value->format != PROPERTY_FORMAT_STRING)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

	if(o->value->data.string.length > GROUP_NAME_SIZE)
	{
		//arch_printf("Name:%s\n",(char*)o->value->data.string.value);
		
    	LOG_ERROR("p2-13-value error\n");

		o->code = OPERATION_ERROR_VALUE;
		return;
	}

    // 婵犵锟藉啿锟藉綊鎮樻径鎰闁归偊鍓欓～鐘绘煥濞戞鐒风紒缁樺灴瀹曞爼鎮欓弶鎸庢殨闂佹椿鍣幏锟�? OPERATION_OK
    o->code = OPERATION_OK;
	update_service_aircondition_name(11,o->value->data.string.value,o->value->data.string.length);
    
    //婵炴垶鎸搁敃銉︽叏閵堝鍋愰柤鍝ヮ暜閹烽攱绗熸繝鍕槷闂備緡鍋呭銊╂偂椤☆湺p闂佺粯顭堥崺鏍晸閹存帞顦︾憸鏉款樀瀹曠娀寮介悽鐢殿槷缂傚倷鑳堕崰宥囩博绾板尙P婵烇絽娴傞崰妤呭极婵傜违濞达絽鎽滈弳浼存煛閸愵厽纭炬繛鍛笧閹叉挳鏁冮敓鍊熴亹閸岀偛瑙﹂幖杈剧稻閻ｅ崬鈽夐幘绛规敾濠殿喓鍊濆鐢稿传閸曨偆鍘�
    //P_2_13_Name_doChange_notify(o->value->data.string.value);

    return;
}
static void P_2_14_Group12Name_doSet(property_operation_t *o)
{
    // 闂佸憡甯囬崐鏍蓟閸ヮ剙鏋侀柣妤�鐗嗙粊锕傛煛瀹ュ洤甯剁紒鎲嬬節瀵即顢涘顓炲墾濠殿喗绻愮徊鍧楀灳濮楋拷閺佸秶浠︾粵瀣悙闂佸搫顑嗙划鎾诲极婵犲嫭瀚氭い鏍ュ�楃粈澶愬级閳哄倹鐓ユ繛鍙夌墬缁傛帡鏁愰崶鈺佺仯: OPERATION_ERROR_VALUE
    if (o->value->format != PROPERTY_FORMAT_STRING)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

	if(o->value->data.string.length > GROUP_NAME_SIZE)
	{
		//arch_printf("Name:%s\n",(char*)o->value->data.string.value);
		
    	LOG_ERROR("p2-14-value error\n");

		o->code = OPERATION_ERROR_VALUE;
		return;
	}

    // 婵犵锟藉啿锟藉綊鎮樻径鎰闁归偊鍓欓～鐘绘煥濞戞鐒风紒缁樺灴瀹曞爼鎮欓弶鎸庢殨闂佹椿鍣幏锟�? OPERATION_OK
    o->code = OPERATION_OK;
	update_service_aircondition_name(12,o->value->data.string.value,o->value->data.string.length);
    
    //婵炴垶鎸搁敃銉︽叏閵堝鍋愰柤鍝ヮ暜閹烽攱绗熸繝鍕槷闂備緡鍋呭銊╂偂椤☆湺p闂佺粯顭堥崺鏍晸閹存帞顦︾憸鏉款樀瀹曠娀寮介悽鐢殿槷缂傚倷鑳堕崰宥囩博绾板尙P婵烇絽娴傞崰妤呭极婵傜违濞达絽鎽滈弳浼存煛閸愵厽纭炬繛鍛笧閹叉挳鏁冮敓鍊熴亹閸岀偛瑙﹂幖杈剧稻閻ｅ崬鈽夐幘绛规敾濠殿喓鍊濆鐢稿传閸曨偆鍘�
    //P_2_14_Name_doChange_notify(o->value->data.string.value);

    return;
}
static void P_2_15_Group13Name_doSet(property_operation_t *o)
{
    // 闂佸憡甯囬崐鏍蓟閸ヮ剙鏋侀柣妤�鐗嗙粊锕傛煛瀹ュ洤甯剁紒鎲嬬節瀵即顢涘顓炲墾濠殿喗绻愮徊鍧楀灳濮楋拷閺佸秶浠︾粵瀣悙闂佸搫顑嗙划鎾诲极婵犲嫭瀚氭い鏍ュ�楃粈澶愬级閳哄倹鐓ユ繛鍙夌墬缁傛帡鏁愰崶鈺佺仯: OPERATION_ERROR_VALUE
    if (o->value->format != PROPERTY_FORMAT_STRING)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

	if(o->value->data.string.length > GROUP_NAME_SIZE)
	{
		//arch_printf("Name:%s\n",(char*)o->value->data.string.value);
		
    	LOG_ERROR("p2-15-value error\n");

		o->code = OPERATION_ERROR_VALUE;
		return;
	}

    // 婵犵锟藉啿锟藉綊鎮樻径鎰闁归偊鍓欓～鐘绘煥濞戞鐒风紒缁樺灴瀹曞爼鎮欓弶鎸庢殨闂佹椿鍣幏锟�? OPERATION_OK
    o->code = OPERATION_OK;
	update_service_aircondition_name(13,o->value->data.string.value,o->value->data.string.length);
    
    //婵炴垶鎸搁敃銉︽叏閵堝鍋愰柤鍝ヮ暜閹烽攱绗熸繝鍕槷闂備緡鍋呭銊╂偂椤☆湺p闂佺粯顭堥崺鏍晸閹存帞顦︾憸鏉款樀瀹曠娀寮介悽鐢殿槷缂傚倷鑳堕崰宥囩博绾板尙P婵烇絽娴傞崰妤呭极婵傜违濞达絽鎽滈弳浼存煛閸愵厽纭炬繛鍛笧閹叉挳鏁冮敓鍊熴亹閸岀偛瑙﹂幖杈剧稻閻ｅ崬鈽夐幘绛规敾濠殿喓鍊濆鐢稿传閸曨偆鍘�
    //P_2_15_Name_doChange_notify(o->value->data.string.value);

    return;
}
static void P_2_16_Group14Name_doSet(property_operation_t *o)
{
    // 闂佸憡甯囬崐鏍蓟閸ヮ剙鏋侀柣妤�鐗嗙粊锕傛煛瀹ュ洤甯剁紒鎲嬬節瀵即顢涘顓炲墾濠殿喗绻愮徊鍧楀灳濮楋拷閺佸秶浠︾粵瀣悙闂佸搫顑嗙划鎾诲极婵犲嫭瀚氭い鏍ュ�楃粈澶愬级閳哄倹鐓ユ繛鍙夌墬缁傛帡鏁愰崶鈺佺仯: OPERATION_ERROR_VALUE
    if (o->value->format != PROPERTY_FORMAT_STRING)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

	if(o->value->data.string.length > GROUP_NAME_SIZE)
	{
		//arch_printf("Name:%s\n",(char*)o->value->data.string.value);
		
    	LOG_ERROR("p2-16-value error\n");

		o->code = OPERATION_ERROR_VALUE;
		return;
	}

    // 婵犵锟藉啿锟藉綊鎮樻径鎰闁归偊鍓欓～鐘绘煥濞戞鐒风紒缁樺灴瀹曞爼鎮欓弶鎸庢殨闂佹椿鍣幏锟�? OPERATION_OK
    o->code = OPERATION_OK;
	update_service_aircondition_name(14,o->value->data.string.value,o->value->data.string.length);
    
    //婵炴垶鎸搁敃銉︽叏閵堝鍋愰柤鍝ヮ暜閹烽攱绗熸繝鍕槷闂備緡鍋呭銊╂偂椤☆湺p闂佺粯顭堥崺鏍晸閹存帞顦︾憸鏉款樀瀹曠娀寮介悽鐢殿槷缂傚倷鑳堕崰宥囩博绾板尙P婵烇絽娴傞崰妤呭极婵傜违濞达絽鎽滈弳浼存煛閸愵厽纭炬繛鍛笧閹叉挳鏁冮敓鍊熴亹閸岀偛瑙﹂幖杈剧稻閻ｅ崬鈽夐幘绛规敾濠殿喓鍊濆鐢稿传閸曨偆鍘�
    //P_2_16_Name_doChange_notify(o->value->data.string.value);

    return;
}
static void P_2_17_Group15Name_doSet(property_operation_t *o)
{
    // 闂佸憡甯囬崐鏍蓟閸ヮ剙鏋侀柣妤�鐗嗙粊锕傛煛瀹ュ洤甯剁紒鎲嬬節瀵即顢涘顓炲墾濠殿喗绻愮徊鍧楀灳濮楋拷閺佸秶浠︾粵瀣悙闂佸搫顑嗙划鎾诲极婵犲嫭瀚氭い鏍ュ�楃粈澶愬级閳哄倹鐓ユ繛鍙夌墬缁傛帡鏁愰崶鈺佺仯: OPERATION_ERROR_VALUE
    if (o->value->format != PROPERTY_FORMAT_STRING)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

	if(o->value->data.string.length > GROUP_NAME_SIZE)
	{
		//arch_printf("Name:%s\n",(char*)o->value->data.string.value);
		
    	LOG_ERROR("p2-17-value error\n");

		o->code = OPERATION_ERROR_VALUE;
		return;
	}

    // 婵犵锟藉啿锟藉綊鎮樻径鎰闁归偊鍓欓～鐘绘煥濞戞鐒风紒缁樺灴瀹曞爼鎮欓弶鎸庢殨闂佹椿鍣幏锟�? OPERATION_OK
    o->code = OPERATION_OK;
	update_service_aircondition_name(15,o->value->data.string.value,o->value->data.string.length);
    
    //婵炴垶鎸搁敃銉︽叏閵堝鍋愰柤鍝ヮ暜閹烽攱绗熸繝鍕槷闂備緡鍋呭銊╂偂椤☆湺p闂佺粯顭堥崺鏍晸閹存帞顦︾憸鏉款樀瀹曠娀寮介悽鐢殿槷缂傚倷鑳堕崰宥囩博绾板尙P婵烇絽娴傞崰妤呭极婵傜违濞达絽鎽滈弳浼存煛閸愵厽纭炬繛鍛笧閹叉挳鏁冮敓鍊熴亹閸岀偛瑙﹂幖杈剧稻閻ｅ崬鈽夐幘绛规敾濠殿喓鍊濆鐢稿传閸曨偆鍘�
    //P_2_17_Name_doChange_notify(o->value->data.string.value);

    return;
}

#endif

#if 0
static void P_2_3_ColorTemperature_doSet(property_operation_t *o)
{
    // 闂佸憡甯囬崐鏍蓟閸ヮ剙鏋侀柣妤�鐗嗙粊锕傛煛瀹ュ洤甯剁紒鎲嬬節瀵即顢涘顓炲墾濠殿喗绻愮徊鍧楀灳濮楋拷閺佸秶浠︾粵瀣悙闂佸搫顑嗙划鎾诲极婵犲嫭瀚氭い鏍ュ�楃粈澶愬级閳哄倹鐓ユ繛鍙夌墬缁傛帡鏁愰崶鈺佺仯: OPERATION_ERROR_VALUE
    if (o->value->format != PROPERTY_FORMAT_NUMBER)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

    // 闂佸憡甯囬崐鏍蓟閸ヮ剙鏋侀柣妤�鐗嗙粊锕傛煕濞嗘劗澧柨鐔凤拷鐔奉棈閻庡灚鐗犲畷鍓侊拷娑櫳戠瑧闂佸憡鐔粻鎴﹀箖鎼粹檧鏋栭柡鍥朵簽缁�澶娾攽閳ュ啿锟藉綊鎮樻径瀣枖鐎广儱鎳忛崐銈嗙箾婢跺绌跨紒杈ㄧ箖濞煎寮幐搴ｎ槬婵炲濯寸徊鍧楁偉閿燂拷: OPERATION_ERROR_VALUE
    if (o->value->data.number.value.integerValue < 3000 || o->value->data.number.value.integerValue > 6400)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

    // TODO: 闂佸湱鐟抽崱鈺傛杸闂佸憡鍔栭悷锕傚箹闁垮濯撮柨鐕傛�? o->value->data.number.value.integerValue;
    light_setlight_templatetrue(o->value->data.number.value.integerValue);


    // 婵犵锟藉啿锟藉綊鎮樻径鎰闁归偊鍓欓～鐘绘煥濞戞鐒风紒缁樺灴瀹曞爼鎮欓弶鎸庢殨闂佹椿鍣幏锟�? OPERATION_OK
    o->code = OPERATION_OK;
    
    //婵炴垶鎸搁敃銉︽叏閵堝鍋愰柤鍝ヮ暜閹烽攱绗熸繝鍕槷闂備緡鍋呭銊╂偂椤☆湺p闂佺粯顭堥崺鏍晸閹存帞顦︾憸鏉款樀瀹曠娀寮介悽鐢殿槷缂傚倷鑳堕崰宥囩博绾板尙P婵烇絽娴傞崰妤呭极婵傜违濞达絽鎽滈弳浼存煛閸愵厽纭炬繛鍛笧閹叉挳鏁冮敓鍊熴亹閸岀偛瑙﹂幖杈剧稻閻ｅ崬鈽夐幘绛规敾濠殿喓鍊濆鐢稿传閸曨偆鍘�
    P_2_3_ColorTemperature_doChange_notify(o->value->data.number.value.integerValue);

    return;
}


//
static void P_2_4_RGBColor_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_NUMBER)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }
    /*
    if (o->value->data.number.value.integerValue < 3000 || o->value->data.number.value.integerValue > 6400)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }
    */
    light_setlight_rgbcolor(o->value->data.number.value.integerValue);
    o->code = OPERATION_OK;
    P_2_4_RGBColor_doChange_notify(o->value->data.number.value.integerValue);

    return;
}

//
static void P_2_5_RGBMode_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_NUMBER)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }
    /*
    if (o->value->data.number.value.integerValue < 3000 || o->value->data.number.value.integerValue > 6400)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }
    */
    light_setlight_rgbmode(o->value->data.number.value.integerValue);
    o->code = OPERATION_OK;
    P_2_5_RGBMode_doChange_notify(o->value->data.number.value.integerValue);

    return;
}

//鐏靛姩寮�鍏�?
static void P_2_6_FlexSwitch_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_BOOLEAN)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

    light_setlight_smartswitch(o->value->data.boolean.value);
    o->code = OPERATION_OK;
    P_2_6_FlexSwitch_doChange_notify(o->value->data.boolean.value);
    return;
}
//鍔╃湢妯″紡
static void P_2_7_SleepMode_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_BOOLEAN)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

    light_setlight_sleep(o->value->data.boolean.value);
    o->code = OPERATION_OK;
    P_2_7_SleepMode_doChange_notify(o->value->data.boolean.value);
    return;
}
//娓呮櫒鍞ら啋
static void P_2_8_WeakupMode_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_BOOLEAN)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

    light_setlight_weakup(o->value->data.boolean.value);
    o->code = OPERATION_OK;
    P_2_8_WeakupMode_doChange_notify(o->value->data.boolean.value);
    return;
}

//娓愬彉鏃堕棿
static void P_2_9_WLduration_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_NUMBER)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

    if (o->value->data.number.value.integerValue < 1 || o->value->data.number.value.integerValue > 60)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

    light_setlight_modetime(o->value->data.number.value.integerValue);

    o->code = OPERATION_OK;

    P_2_9_WLduration_doChange_notify(o->value->data.number.value.integerValue);

    return;
}

//鍞ら啋妯″紡鏈�缁堜寒搴�?
static void P_2_10_WeakpEbr_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_NUMBER)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

    if (o->value->data.number.value.integerValue < 1 || o->value->data.number.value.integerValue > 100)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

    light_setlight_weakebr(o->value->data.number.value.integerValue);

    o->code = OPERATION_OK;

    P_2_10_WeakpEbr_doChange_notify(o->value->data.number.value.integerValue);

    return;
}

//鐫＄湢鐨勫紑濮嬩寒搴�?
static void P_2_11_SleepSbr_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_NUMBER)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

    if (o->value->data.number.value.integerValue < 1 || o->value->data.number.value.integerValue > 100)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

    light_setlight_sleepsbr(o->value->data.number.value.integerValue);

    o->code = OPERATION_OK;

    P_2_11_SleepSbr_doChange_notify(o->value->data.number.value.integerValue);

    return;
}


//鐏厜涓婄數鐘舵��
static void P_2_12_LightPONDefault_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_NUMBER)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }


    light_setlight_powerstate(o->value->data.number.value.integerValue);

    o->code = OPERATION_OK;

    P_2_12_LightPONDefault_doChange_notify(o->value->data.number.value.integerValue);

    return;
}
#endif


void S_2_Light_doSet(property_operation_t *o)
{
	switch (o->piid)
	{
		case IID_2_1_On:
		P_2_1_Group_doSet(o);
		break;

		case IID_2_2_Brightness:
		P_2_2_Group0Name_doSet(o);
		break;

		case 3:
		P_2_3_Group1Name_doSet(o);
		break;
		case 4:
		P_2_4_Group2Name_doSet(o);
		break;

		case 5:
		P_2_5_Group3Name_doSet(o);
		break;
		case 6:
		P_2_6_Group4Name_doSet(o);
		break;

		case 7:
		P_2_7_Group5Name_doSet(o);
		break;
		case 8:
		P_2_8_Group6Name_doSet(o);
		break;

		case 9:
		P_2_9_Group7Name_doSet(o);
		break;
		case 10:
		P_2_10_Group8Name_doSet(o);
		break;

		case 11:
		P_2_11_Group9Name_doSet(o);
		break;
		case 12:
		P_2_12_Group10Name_doSet(o);
		break;

		case 13:
		P_2_13_Group11Name_doSet(o);
		break;
		case 14:
		P_2_14_Group12Name_doSet(o);
		break;

		case 15:
		P_2_15_Group13Name_doSet(o);
		break;
		case 16:
		P_2_16_Group14Name_doSet(o);
		break;

		case 17:
		P_2_17_Group15Name_doSet(o);
		break;
		
		case 18:
		P_2_18_NotifyGroupName_doSet(o);
		break;

		default:
		o->code = OPERATION_ERROR_CANNOT_WRITE;
		break;
	}
}


//IID_3_1_OnOff
static void P_3_1_OnOff_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_BOOLEAN)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }
   // light_setlight_onoff(o->value->data.boolean.value);
  	 update_service_aircondition_sw(o->value->data.boolean.value);
    o->code = OPERATION_OK;
    //P_3_1_OnOff_doChange_notify(o->value->data.boolean.value);
    return;
}
#if 0
//IID_3_2_BrAdd
static void P_3_2_BrAdd_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_BOOLEAN)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }
    light_setlight_bradd(o->value->data.boolean.value);
    o->code = OPERATION_OK;
    P_3_2_Mode_doChange_notify(o->value->data.boolean.value);
    return;
}
#endif
//IID_3_2_BrAdd
static void P_3_2_SetMode_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_NUMBER)
    {
		//LOG_ERROR("p3-2-format error\n");
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

	if (o->value->data.number.value.integerValue > 4)
    {
		////arch_printf("Mode:%d\n",(int)o->value->data.number.value.integerValue);
		
    	//LOG_ERROR("p3-2-value error\n");
        o->code = OPERATION_ERROR_VALUE;
        return;
    }
	  
	update_service_aircondition_mode(o->value->data.number.value.integerValue);
    o->code = OPERATION_OK;
    //P_3_2_Mode_doChange_notify(o->value->data.number.value.integerValue);
    return;
}
//IID_3_3_BrDec
#if 0
static void P_3_3_BrDec_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_BOOLEAN)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }
    light_setlight_brdec(o->value->data.boolean.value);
    o->code = OPERATION_OK;
    P_3_3_Fault_doChange_notify(o->value->data.boolean.value);
    return;
}
#endif

#if 0
//IID_3_4_BrSw
static void P_3_4_BrSw_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_BOOLEAN)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }
    light_setlight_brsw(o->value->data.boolean.value);
    o->code = OPERATION_OK;
    P_3_4_SetTemp_doChange_notify(o->value->data.boolean.value);
    return;
}
#endif
//IID_3_4_SetTemp
static void P_3_4_SetTemp_doSet(property_operation_t *o)
{
	uint8_t set_temp = 0;
	
    if (o->value->format != PROPERTY_FORMAT_NUMBER)
    {		
    	LOG_ERROR("p3-4-format error\n");
        o->code = OPERATION_ERROR_VALUE;
        return;
    }

	//�ж������¶��Ƿ�Ϸ�
	/*if (o->value->data.number.value.integerValue < 16 || o->value->data.number.value.integerValue > 32)
	{
		//arch_printf("Temp:%d\n",(int)o->value->data.number.value.integerValue);
    	LOG_ERROR("p3-4-value error\n");
		o->code = OPERATION_ERROR_VALUE;
		return;
	}
	*/
	
	set_temp =	o->value->data.number.value.integerValue;
	
	update_service_aircondition_SetTemp(set_temp);
    o->code = OPERATION_OK;
    //P_3_4_SetTemp_doChange_notify(o->value->data.number.value.integerValue);
    return;
}


#if 0
//IID_3_5_OnBrSw
static void P_3_5_OnBrSw_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_BOOLEAN)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }
    light_setlight_onbrsw(o->value->data.boolean.value);
    o->code = OPERATION_OK;
    P_3_5_OnBrSw_doChange_notify(o->value->data.boolean.value);
    return;
}
//
//P_3_6_TmAdd_doSet
static void P_3_6_TmAdd_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_BOOLEAN)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }
    light_setlight_tmadd(o->value->data.boolean.value);
    o->code = OPERATION_OK;
    P_3_6_TmAdd_doChange_notify(o->value->data.boolean.value);
    return;
}
//P_3_7_TmDec_doSet
static void P_3_7_TmDec_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_BOOLEAN)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }
    light_setlight_tmdec(o->value->data.boolean.value);
    o->code = OPERATION_OK;
    P_3_7_TmDec_doChange_notify(o->value->data.boolean.value);
    return;
}
//P_3_8_TmSw_doSet
static void P_3_8_TmSw_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_BOOLEAN)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }
    light_setlight_tmsw(o->value->data.boolean.value);
    o->code = OPERATION_OK;
    P_3_8_TmSw_doChange_notify(o->value->data.boolean.value);
    return;
}

//P_3_9_OnTmSw_doSet
static void P_3_9_OnTmSw_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_BOOLEAN)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }
    light_setlight_ontmsw(o->value->data.boolean.value);
    o->code = OPERATION_OK;
    P_3_9_OnTmSw_doChange_notify(o->value->data.boolean.value);
    return;
}

//P_3_10_RGBSw_doSet
static void P_3_10_RGBSw_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_BOOLEAN)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }
    light_setlight_rgbsw(o->value->data.boolean.value);
    o->code = OPERATION_OK;
    P_3_10_RGBSw_doChange_notify(o->value->data.boolean.value);
    return;
}

//P_3_11_OnRGBSw_doSet
static void P_3_11_OnRGBSw_doSet(property_operation_t *o)
{
    if (o->value->format != PROPERTY_FORMAT_BOOLEAN)
    {
        o->code = OPERATION_ERROR_VALUE;
        return;
    }
    light_setlight_onrgbsw(o->value->data.boolean.value);
    o->code = OPERATION_OK;
    P_3_11_OnRGBSw_doChange_notify(o->value->data.boolean.value);
    return;
}
#endif

void S_3_Scens_doSet(property_operation_t *o)
{
    switch (o->piid)
    {
        case IID_3_1_OnOff:
        	P_3_1_OnOff_doSet(o);
            break;

        case IID_3_2_BrAdd:
        	P_3_2_SetMode_doSet(o);
            break;

       // case IID_3_3_BrDec:
        	//P_3_3_BrDec_doSet(o);
           // break;

        case IID_3_4_BrSw:
        	P_3_4_SetTemp_doSet(o);
            break;

        default:
            o->code = OPERATION_ERROR_CANNOT_WRITE;
            break;
    }
}

#if 0
//P_4_1_Defualt_doSet
static void P_4_1_Defualt_doSet(property_operation_t *o)
{
	if (o->value->format != PROPERTY_FORMAT_NUMBER)
	{
		o->code = OPERATION_ERROR_VALUE;
		return;
	}

	if (o->value->data.number.value.integerValue < 0||
		o->value->data.number.value.integerValue > 1)
	{
		o->code = OPERATION_ERROR_VALUE;
		return;
	}
	light_setlight_powerstate(o->value->data.number.value.integerValue);
	o->code = OPERATION_OK;
	P_4_2_FanSpeed_doChange_notify(o->value->data.number.value.integerValue);
	return;
}
#endif

//P_4_2_SetFanSpeed_doSet
static void P_4_2_SetFanSpeed_doSet(property_operation_t *o)
{
	if (o->value->format != PROPERTY_FORMAT_NUMBER)
	{
		o->code = OPERATION_ERROR_VALUE;
		return;
	}

	if (o->value->data.number.value.integerValue > 3)
	{
		o->code = OPERATION_ERROR_VALUE;
		return;
	}

	update_service_aircondition_fanSpeed(o->value->data.number.value.integerValue);
	o->code = OPERATION_OK;
	//P_4_2_FanSpeed_doChange_notify(o->value->data.number.value.integerValue);
	return;
}

#if 0
//P_4_2_UserMode_doSet
static void P_4_2_UserMode_doSet(property_operation_t *o)
{
	if (o->value->format != PROPERTY_FORMAT_NUMBER)
	{
		o->code = OPERATION_ERROR_VALUE;
		return;
	}
	/*
	if (o->value->data.number.value.integerValue < 0||
		o->value->data.number.value.integerValue > 1)
	{
		o->code = OPERATION_ERROR_VALUE;
		return;
	}
	*/
	light_setlight_usermode(o->value->data.number.value.integerValue);
	o->code = OPERATION_OK;
	P_4_2_UserMode_doChange_notify(o->value->data.number.value.integerValue);
	return;
}
//P_4_3_FadeOn_doSet
static void P_4_3_FadeOn_doSet(property_operation_t *o)
{
	if (o->value->format != PROPERTY_FORMAT_NUMBER)
	{
		o->code = OPERATION_ERROR_VALUE;
		return;
	}

	light_setlight_fadeon(o->value->data.number.value.integerValue);
	o->code = OPERATION_OK;
	P_4_3_FadeOn_doChange_notify(o->value->data.number.value.integerValue);
	return;
}

//P_4_4_FadeOff_doSet
static void P_4_4_FadeOff_doSet(property_operation_t *o)
{
	if (o->value->format != PROPERTY_FORMAT_NUMBER)
	{
		o->code = OPERATION_ERROR_VALUE;
		return;
	}

	light_setlight_fadeoff(o->value->data.number.value.integerValue);
	o->code = OPERATION_OK;
	P_4_4_FadeOff_doChange_notify(o->value->data.number.value.integerValue);
	return;
}

//P_4_5_FadeScen_doSet
static void P_4_5_FadeScen_doSet(property_operation_t *o)
{
	if (o->value->format != PROPERTY_FORMAT_NUMBER)
	{
		o->code = OPERATION_ERROR_VALUE;
		return;
	}

	light_setlight_fadescen(o->value->data.number.value.integerValue);
	o->code = OPERATION_OK;
	P_4_5_FadeScen_doChange_notify(o->value->data.number.value.integerValue);
	return;
}


//P_4_6_NightSwitch_doSet
static void P_4_6_NightSwitch_doSet(property_operation_t *o)
{
	if (o->value->format != PROPERTY_FORMAT_NUMBER)
	{
		o->code = OPERATION_ERROR_VALUE;
		return;
	}

	/*
	if (o->value->data.number.value.integerValue < 0||
		o->value->data.number.value.integerValue > 1)
	{
		o->code = OPERATION_ERROR_VALUE;
		return;
	}
	*/
	light_setlight_nightswitch(o->value->data.number.value.integerValue);
	o->code = OPERATION_OK;
	P_4_6_NightSwitch_doChange_notify(o->value->data.number.value.integerValue);
	return;
}

//P_4_7_TimerOff_doSet
static void P_4_7_TimerOff_doSet(property_operation_t *o)
{
	if (o->value->format != PROPERTY_FORMAT_NUMBER)
	{
		o->code = OPERATION_ERROR_VALUE;
		return;
	}

	light_setlight_timeroff(o->value->data.number.value.integerValue);
	o->code = OPERATION_OK;
	P_4_7_TimerOff_doChange_notify(o->value->data.number.value.integerValue);
	return;
}


//P_4_8_LowestBrightness_doSet
static void P_4_8_LowestBrightness_doSet(property_operation_t *o)
{
	if (o->value->format != PROPERTY_FORMAT_NUMBER)
	{
		o->code = OPERATION_ERROR_VALUE;
		return;
	}

	light_setlight_lowestbrightness(o->value->data.number.value.integerValue);
	o->code = OPERATION_OK;
	P_4_8_LowestBrightness_doChange_notify(o->value->data.number.value.integerValue);
	return;
}

//P_4_9_LockState_doSet
static void P_4_9_LockState_doSet(property_operation_t *o)
{
	if (o->value->format != PROPERTY_FORMAT_NUMBER)
	{
		o->code = OPERATION_ERROR_VALUE;
		return;
	}

	light_setlight_lockstate(o->value->data.number.value.integerValue);
	o->code = OPERATION_OK;
	P_4_9_LockState_doChange_notify(o->value->data.number.value.integerValue);
	return;
}

#endif

//
void S_4_UserMode_doSet(property_operation_t *o)
{
    switch (o->piid)
    {
		case IID_4_2_UserMode:
			P_4_2_SetFanSpeed_doSet(o);
			break;

        default:
            o->code = OPERATION_ERROR_CANNOT_READ;
            break;
    }
}



#if 0

//P_8_1_RemoteSwitch_doSet
static void P_8_1_RemoteSwitch_doSet(property_operation_t *o)
{
	if (o->value->format != PROPERTY_FORMAT_NUMBER)
	{
		o->code = OPERATION_ERROR_VALUE;
		return;
	}

	light_setlight_remoteswitch(o->value->data.number.value.integerValue);
	o->code = OPERATION_OK;
	P_8_1_RemoteSwitch_doChange_notify(o->value->data.number.value.integerValue);
	return;
}

//P_8_2_RemoteAdd_doSet
static void P_8_2_RemoteAdd_doSet(property_operation_t *o)
{
	if (o->value->format != PROPERTY_FORMAT_NUMBER)
	{
		o->code = OPERATION_ERROR_VALUE;
		return;
	}

	light_setlight_remoteset(0xFFFFFFFF);
	o->code = OPERATION_OK;
	P_8_2_RemoteAdd_doChange_notify(o->value->data.number.value.integerValue);
	return;
}

//P_8_3_RemoteDel_doSet
static void P_8_3_RemoteDel_doSet(property_operation_t *o)
{
	if (o->value->format != PROPERTY_FORMAT_NUMBER)
	{
		o->code = OPERATION_ERROR_VALUE;
		return;
	}

	light_setlight_remoteset(o->value->data.number.value.integerValue);
	o->code = OPERATION_OK;
	P_8_3_RemoteDel_doChange_notify(o->value->data.number.value.integerValue);
	return;
}



void S_8_Remote_doSet(property_operation_t *o)
{
    switch (o->piid)
    {
		case IID_8_1_RemoteSwitch:
			P_8_1_RemoteSwitch_doSet(o);
			break;

		case IID_8_2_RemoteAdd:
			P_8_2_RemoteAdd_doSet(o);
			break;

		case IID_8_3_RemoteDel:
			P_8_3_RemoteDel_doSet(o);
			break;

        default:
            o->code = OPERATION_ERROR_CANNOT_READ;
            break;
    }
}
#endif

