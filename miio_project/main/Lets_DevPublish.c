#include "Lets_DevPublish.h"
#include "light.h"

#define LETS_PNUMBER	"193001R1"
/*
 * ͨ�������㲥��ʽ�����豸�汾��
 * �����ԣ��������29���ַ�,����֮��8269����������
 * Ŀǰռ�����Ϊ:
 * L[оƬ]_[���][�汾]_[����]_[MAC][��è����]
 * ���У�[оƬ]ռ2-3λ��[����]ռ6λ��[MAC]ռ4λ��[��è����]ռ1λ��ǰ׺Lռ1λ���»���ռ3λ���ܹ�17��18λ
 * ��[���][�汾]������11�ֽڣ���ȥ193��R�ַ���������7�ֽڣ�ͨ����193��������3λ���֣���R����İ汾�ַ����4�ֽڣ�
 * ��[���][�汾]��ΧΪ193001R1��193001R9999
 *
 * */
#define IS_NUM_CHAR(n)   (((n)>='0')&&((n)<='9'))
#define IS_LOW_CHAR(n)   (((n)>='a')&&((n)<='z'))
#define IS_HIGH_CHAR(n)   (((n)>='A')&&((n)<='Z'))

const uint8_t L_code_2_code[]="62NFP4HTWjewpnyRYLE07IV8UJBt31Oz9lXq-MGohDvuZdcxQrfigSCA5Kkabsm";
//const uint8_t L_code_2_code[]="0123456789-ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
//const uint8_t L_code_2_code[]="0123456789-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
//˳��Ϊ:0~9,-,a-z,A~Z
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
		}else {	//�쳣�ַ���������

		}
	}
}


void getVersionStr(uint8_t *out,const char* mode,uint16_t vsernum,uint8_t lenMax)
{
	//��Ʒ��������:
	//MCU_PNUM_PID_VER_MAC
	//����ǰ��193��ʼ��ȡ��_���������ǰ��ȡ��RΪֹ�����λ��
	//[MAC]ȡMAC��2λ

	uint8_t ver_head[64]="L32_";
	uint8_t str_idx=0;

	str_idx = 4;

	//��ʼȡPNUM_
	my_strcpy_s(&ver_head[str_idx],(uint8_t*)LETS_PNUMBER,sizeof(LETS_PNUMBER)-1);
	str_idx += (sizeof(LETS_PNUMBER)-1);
	ver_head[str_idx++] = '_';

	//��ʼȡPID_
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

	//ƴ�������ַ���
	out[0] = 0;
	my_strcat_s(out,ver_head,lenMax);
	//��ת
	L_ByteRoll(&out[1]);	//�ֽ�0�̶�ΪL
}

void Task_DevNamePublish_Init()
{
	uint8_t par[26]={0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
	//getVersionStr(par,MIIO_INSTANCE_MODEL,MIIO_APP_VERSION_NUMBER,26);
	light_blename_send(par,26);
}
