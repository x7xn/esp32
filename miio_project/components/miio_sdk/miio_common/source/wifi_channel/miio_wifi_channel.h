/**
* @file    miio_wifi_channel.h
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

#ifndef __MIIO_WIFI_CHANNEL_H__
#define __MIIO_WIFI_CHANNEL_H__

#include "miio_define.h"

typedef struct{
    char  cc[3];
    uint8_t schan;   		/**< start channel */
    uint8_t nchan;   		/**< total channel number */
    int8_t  max_tx_dbm;		/**< maximum tx power */
}miio_wifi_channel_info_t;


int miio_wifi_channel_info_get(const char* cc, miio_wifi_channel_info_t *channel_info);

#endif
