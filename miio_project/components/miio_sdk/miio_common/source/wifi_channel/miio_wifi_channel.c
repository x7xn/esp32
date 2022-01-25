/**
* @file    miio_wifi_channel.c
* @author  mashaoze
* @date    2017
* @par     Copyright (c):
*
*    Copyright 2017 MIoT,MI
*
*    Licensed under the Apache License, Version 2.0 (the "License");
*    you may not use this file except in compliance with the License.
*    You may obtain a copy of the License at
*
*        http://www.apache.org/licenses/LICENSE-2.0
*
*    Unless required by applicable law or agreed to in writing, software
*    distributed under the License is distributed on an "AS IS" BASIS,
*    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*    See the License for the specific language governing permissions and
*    limitations under the License.
*/
#include "miio_wifi_channel.h"
#include "miio_arch.h"

#undef MIIO_LOG_TAG
#define MIIO_LOG_TAG			"miio_wifi"

static const miio_wifi_channel_info_t s_default_channel_info = {
	"CN",   1, 13, 0
};

static const miio_wifi_channel_info_t s_special_channel_info_table[] = {
	{"CA", 1, 11, 0},//CANADA
	{"CO", 1, 11, 0},//Colombia
	{"DO", 1, 11, 0},//DOMINICAN
	{"GT", 1, 11, 0},//Guatemala
	{"MX", 1, 11, 0},//Mexico
	{"PA", 1, 11, 0},//Panama
	{"PR", 1, 11, 0},//Puerto Rico
	{"US", 1, 11, 0},//USA
	{"TW", 1, 11, 0},//Tw,China
	{"JP", 1, 14, 0},//Japan
	{"UZ", 1, 11, 0},//Uzbekistan
	//{"IL", 3,  7, 0},//Israel
};


int miio_wifi_channel_info_get(const char* cc, miio_wifi_channel_info_t *channel_info)
{
	const miio_wifi_channel_info_t *channel_info_found = NULL;

	if(cc && (strnlen(cc, sizeof(channel_info->cc)) < sizeof(channel_info->cc))){

		for(int i = 0; i < sizeof(s_special_channel_info_table)/sizeof(miio_wifi_channel_info_t); i++){
			if(0 == strcmp(s_special_channel_info_table[i].cc, cc)){
				channel_info_found = &(s_special_channel_info_table[i]);
				break;
			}
		}
	}

	if(channel_info_found){
		memcpy(channel_info, channel_info_found, sizeof(miio_wifi_channel_info_t));
	}
	else{
		memcpy(channel_info, &s_default_channel_info, sizeof(miio_wifi_channel_info_t));
		strncpy(channel_info->cc, cc, sizeof(channel_info->cc)-1);
		channel_info->cc[sizeof(channel_info->cc)-1] = '\0';
	}

	LOG_INFO_TAG(MIIO_LOG_TAG, "cc=%s, schan=%d, nchan=%d, max_tx_dbm=%d", channel_info->cc, channel_info->schan, channel_info->nchan, channel_info->max_tx_dbm);

	return MIIO_OK;
}

