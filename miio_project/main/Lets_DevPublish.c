#include "Lets_DevPublish.h"
#include "light.h"

#define LETS_PNUMBER	"193001R1"
/*
 * 通过蓝牙广播方式发送设备版本号
 * 经测试，名称最多29个字符,超出之后8269会无限重启
 * 目前占用情况为:
 * L[芯片]_[编号][版本]_[日期]_[MAC][天猫精灵]
 * 其中，[芯片]占2-3位，[日期]占6位，[MAC]占4位，[天猫精灵]占1位，前缀L占1位，下划线占3位，总共17或18位
 * 即[编号][版本]共可用11字节，除去193和R字符，还可用7字节，通常，193后面最多跟3位数字，则R后面的版本字符最多4字节，
 * 即[编号][版本]范围为193001R1至193001R9999
 *
 * */
#define IS_NUM_CHAR(n)   (((n)>='0')&&((n)<='9'))
#define IS_LOW_CHAR(n)   (((n)>='a')&&((n)<='z'))
#define IS_HIGH_CHAR(n)   (((n)>='A')&&((n)<='Z'))

const uint8_t L_code_2_code[]="62NFP4HTWjewpnyRYLE07IV8UJBt31Oz9lXq-MGohDvuZdcxQrfigSCA5Kkabsm";
//const uint8_t L_code_2_code[]="0123456789-ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
//const uint8_t L_code_2_code[]="0123456789-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
//顺序为:0~9,-,a-z,A~Z
const uint8_t TABLE_HEX[]="0123456789ABCDEF";

void my_strcpy_s(uint8_t *out,uint8_t*in,uint16_t len){
	uint16_t i=0;
	while(i<len){
		if((*in)==0)break;
		*out++ = *in++;
	}
}

void my_strcat_s(uint8_t *out,const  uint8_t*in,uint16_t len_max){
	uint16_t i=0;
	while(i<len_max){
		if((*out)==0)break;
		out++;
	}
	while(i<len_max){
		if((*in)==0)break;
		*out++ = *in++;
	}
	*out = 0;
}

void L_ByteRoll(uint8_t*str)
{
	uint8_t i=0;
	uint8_t len_out = strlen((const char*)str);
	uint8_t temp = 0;
	for(i=0;i<len_out;i++){
		temp = str[i];
		if(IS_NUM_CHAR(temp)){
			str[i] = L_code_2_code[(temp - '0')];
		}else if(IS_LOW_CHAR(temp)){
			str[i] = L_code_2_code[(temp - 'a'+11)];
		}else if(IS_HIGH_CHAR(temp)){
			str[i] = L_code_2_code[(temp - 'A'+37)];
		}else if('_'==(temp)){
			str[i] = L_code_2_code[10];
		}else {	//异常字符，不处理

		}
	}
}


void getVersionStr(uint8_t *out,const char* mode,uint16_t vsernum,uint8_t lenMax)
{
	//产品命名规则:
	//MCU_PNUM_PID_VER_MAC
	//即从前面193开始截取到_，从最后往前截取到R为止，两段混合
	//[MAC]取MAC后2位

	uint8_t ver_head[64]="L32_";
	uint8_t str_idx=0;

	str_idx = 4;

	//开始取PNUM_
	my_strcpy_s(&ver_head[str_idx],(uint8_t*)LETS_PNUMBER,sizeof(LETS_PNUMBER)-1);
	str_idx += (sizeof(LETS_PNUMBER)-1);
	ver_head[str_idx++] = '_';

	//开始取PID_
	my_strcpy_s(&ver_head[str_idx],(uint8_t*)mode,sizeof(mode));
	str_idx += (sizeof(mode)-1);
	ver_head[str_idx++] = '_';

	ver_head[str_idx++] = 'V';
	ver_head[str_idx++] = TABLE_HEX[vsernum%10000/1000];
	ver_head[str_idx++] = TABLE_HEX[vsernum%1000/100];
	ver_head[str_idx++] = TABLE_HEX[vsernum%100/10];
	ver_head[str_idx++] = TABLE_HEX[vsernum%10];

	uint8_t mac[6] = {0};
	arch_get_mac(mac);

	ver_head[str_idx++] = TABLE_HEX[mac[0]>>4];
	ver_head[str_idx++] = TABLE_HEX[mac[0]&0xF];
	ver_head[str_idx++] = TABLE_HEX[mac[1]>>4];
	ver_head[str_idx++] = TABLE_HEX[mac[1]&0xF];
	ver_head[str_idx++] = TABLE_HEX[mac[2]>>4];
	ver_head[str_idx++] = TABLE_HEX[mac[2]&0xF];
	ver_head[str_idx++] = TABLE_HEX[mac[3]>>4];
	ver_head[str_idx++] = TABLE_HEX[mac[3]&0xF];
	ver_head[str_idx++] = TABLE_HEX[mac[4]>>4];
	ver_head[str_idx++] = TABLE_HEX[mac[4]&0xF];
	ver_head[str_idx++] = TABLE_HEX[mac[5]>>4];
	ver_head[str_idx++] = TABLE_HEX[mac[5]&0xF];

	//拼接两个字符串
	out[0] = 0;
	my_strcat_s(out,ver_head,lenMax);
	//翻转
	L_ByteRoll(&out[1]);	//字节0固定为L
}

void Task_DevNamePublish_Init()
{
	uint8_t par[26]={0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
	//getVersionStr(par,MIIO_INSTANCE_MODEL,MIIO_APP_VERSION_NUMBER,26);
	light_blename_send(par,26);
}
