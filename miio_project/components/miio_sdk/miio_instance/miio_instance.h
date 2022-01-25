/**
* @file    miio_instance.h
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
#ifndef __MIIO_INSTANCE_T__
#define	__MIIO_INSTANCE_T__

#include "miio_instance_config.h"
#include "miio_api.h"

void miio_instance_set_uid(miio_handle_t miio_handle, uint64_t uid);
void miio_instance_set_gmt_offset(miio_handle_t miio_handle, int gmt_offset);
void miio_instance_set_model(miio_handle_t miio_handle, const char *model);
void miio_instance_set_country_domain(miio_handle_t miio_handle, const char *country_domain);

#if MIIO_AUTO_OTA_ENABLE
void miio_instance_mcu_auto_upgrade_enable(miio_handle_t miio_handle, bool enable);
int miio_instance_auto_upgrate_time_restore(void);
int app_auto_ota_hook_default(miio_handle_t handle, int *device_state, void* ctx);
bool app_is_auto_updating(void);
bool mcu_is_auto_updating(void *mcmd_handle);
#endif

miio_handle_t miio_instance_create(void);

#endif
