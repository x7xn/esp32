/**
* @file    miio_instance.c
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
#include "miio_instance.h"
#include "miio_ota_app.h"
#include "miio_ota_mcu.h"
#include "miio_ciphers.h"
// #if MIIO_COMMANDS_ENABLE
#include "miio_command.h"
// #endif
#include "miio_net.h"
#include "text_on_uart.h"
#include "jsmi.h"
#include "arch_chip.h"
#if MIBLE_ENABLE
#include "mible_gateway.h"
#include "mible_net.h"
#endif

#undef MIIO_LOG_TAG
#define MIIO_LOG_TAG			"miio_instance"

static int miio_info_kvs_hook_default(miio_handle_t handle, miio_delegate_type_t type, void *composer, void *ctx)
{
	LOG_INFO_TAG(MIIO_LOG_TAG, "%s called", __FUNCTION__);

	jsmi_composer_t *jsmi_composer = composer;

	{//hw_ver
		jsmi_set_key_value_str(jsmi_composer, "hw_ver", arch_get_chip_type(), 0);
	}

	{//mmfree
		jsmi_set_key_value_uint(jsmi_composer, "mmfree", arch_os_get_free_heap_size());
	}


    return MIIO_OK;
}

static int miio_ext_rpc_hook_default(miio_handle_t handle, void *ctx)
{
	LOG_INFO_TAG(MIIO_LOG_TAG, "%s called", __FUNCTION__);
	return MIIO_OK;
}

#if MIIO_AUTO_OTA_ENABLE
static void miio_report_auto_ota_ability(miio_handle_t handle)
{
	uint32_t ability = 0;
	char mcu_ver[8] = {0};
	bool mcu_auto_ota_ability = false;

	if(arch_psm_get_str("ot_config", "mcu_ver", mcu_ver, sizeof(mcu_ver)) <= 0){
		ability = miio_get_app_auto_ota_ability(handle);
	}
	else{
		if(arch_psm_get_value("ot_config", "mcu_auto_ota_ability", &mcu_auto_ota_ability, sizeof(mcu_auto_ota_ability)) <= 0){
			ability = 0;
		}
		else{
			ability = (uint32_t)(miio_get_app_auto_ota_ability(handle) && mcu_auto_ota_ability);
		}
	}
		
	miio_set_property_uint(handle, "auto_ota_ability", ability);
}
#endif

static int miio_online_hook_default(miio_handle_t handle, void *ctx)
{
	LOG_INFO_TAG(MIIO_LOG_TAG, "%s called", __FUNCTION__);

#if MIIO_AUTO_OTA_ENABLE
	if((true == miio_get_app_auto_ota_ability(handle))
	||(true == miio_get_mcu_auto_ota_ability(handle)))
	{
		miio_report_auto_ota_ability(handle);
	}
#endif

	return MIIO_OK;
}

static int miio_offline_hook_default(miio_handle_t handle, void *ctx)
{
	LOG_INFO_TAG(MIIO_LOG_TAG, " %s called", __FUNCTION__);
	return MIIO_OK;
}

static int miio_restore_hook_default(miio_handle_t handle, void *ctx)
{
	LOG_INFO_TAG(MIIO_LOG_TAG, " %s called", __FUNCTION__);
	return MIIO_OK;
}

static int miio_reboot_hook_default(miio_handle_t handle, void *ctx)
{
	LOG_INFO_TAG(MIIO_LOG_TAG, " %s called", __FUNCTION__);
	return MIIO_OK;
}

static miio_hooks_t s_miio_instance_hooks = {
	.info = miio_info_kvs_hook_default,
	.ext_rpc = miio_ext_rpc_hook_default,
	.online = miio_online_hook_default,
	.offline = miio_offline_hook_default,
	.restore = miio_restore_hook_default,
	.reboot = miio_reboot_hook_default,
	.statistics = NULL,
	.ota_task = NULL,
	.ctx = NULL
};

static miio_ota_app_ctx_t s_miio_ota_app_ctx;

static void miio_instance_log(miio_handle_t miio_handle)
{
#ifndef MIIO_JKS_BUILD_NUMBER
#define MIIO_JKS_BUILD_NUMBER	"N/A"
#endif
#ifndef MIIO_BUILDER
#define MIIO_BUILDER			"N/A"
#endif
	/*
	LOG_PRINT("\r\n");
	LOG_PRINT("_|      _|  _|_|_|  _|_|_|    _|_|  ");
	LOG_PRINT("_|_|  _|_|    _|      _|    _|    _|");
	LOG_PRINT("_|  _|  _|    _|      _|    _|    _|");
	LOG_PRINT("_|      _|    _|      _|    _|    _|");
	LOG_PRINT("_|      _|  _|_|_|  _|_|_|    _|_|  ");
	*/

	LOG_PRINT("JENKINS BUILD NUMBER: %s", MIIO_JKS_BUILD_NUMBER);
	LOG_PRINT("BUILD TIME: %s,%s",__DATE__,__TIME__);
	LOG_PRINT("BUILT BY: %s", MIIO_BUILDER);
#if MIIO_APP_VERSION_NUMBER
	LOG_PRINT("MIIO APP VER: %s", miio_app_version(MIIO_APP_VERSION_NUMBER));
#else
	LOG_PRINT("MIIO APP VER: %s", miio_instance_version());
#endif
	{
		char mcu_ver[16] = {0};
		LOG_PRINT("MIIO MCU VER: %s", miio_mcu_get_version(mcu_ver, sizeof(mcu_ver)));
	}
	{
		char did_str[32] = {0};
		arch_u64toa(miio_get_did(miio_handle), did_str);
		LOG_PRINT("MIIO DID: %s", did_str);
#if MIIO_KEY_EXCHANGE_ENABLE
    {
        uint64_t new_did = 0;
        arch_psm_get_value("key_exchange", "bind_did", &new_did, sizeof(new_did));
        if ((new_did != 0) && (new_did != miio_get_did(miio_handle)))
        {
            LOG_PRINT("MIIO NEW DID: %llu", new_did);
        }
    }   
#endif
	}
	{
		uint8_t mac[6] = {0};
		arch_get_mac(mac);
		LOG_PRINT("MIIO WIFI MAC: %02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
	{
		char model[MIIO_MODEL_SIZE_MAX] = {0};
		LOG_PRINT("MIIO MODEL: %s", miio_get_model(miio_handle, model, sizeof(model)));
	}
	LOG_PRINT("ARCH TYPE: %s,0x%08x", arch_get_chip_type(), arch_get_chip_version());
	LOG_PRINT("ARCH VER: %s", arch_get_sdk_version());

	/* print flash info */
	arch_get_flash_info();
}

#if MIIO_AUTO_OTA_ENABLE
int miio_instance_auto_upgrate_time_restore(void)
{
    arch_psm_erase_key("ot_config", "ota_start_time");
    arch_psm_erase_key("ot_config", "ota_dura_time");

    return MIIO_OK;
}

void miio_instance_mcu_auto_upgrade_enable(miio_handle_t miio_handle, bool enable)
{
	if(enable != miio_get_mcu_auto_ota_ability(miio_handle)){
		arch_psm_set_value("ot_config", "mcu_auto_ota_ability", &enable, sizeof(enable));
		miio_set_mcu_auto_ota_ability(miio_handle, enable);
	}
}

bool mcu_is_auto_updating(void *mcmd_handle)
{
	mcmd_t *mcmd = (mcmd_t*)mcmd_handle;
	miio_ota_mcu_ctx_t* ota_mcu_ctx = &mcmd->ota_mcu_ctx;
	bool auto_enable = false;

	if(MIIO_OTA_TASK_IDLE != miio_ota_task_get_state(ota_mcu_ctx->ota_task_handle)){
		auto_enable = ota_mcu_ctx->auto_ota.auto_enable;
	}
	
	return auto_enable;
}

bool app_is_auto_updating(void)
{
	miio_ota_app_ctx_t *ota_app_ctx = &s_miio_ota_app_ctx;
	bool auto_enable = false;

	if(MIIO_OTA_TASK_IDLE != miio_ota_task_get_state(ota_app_ctx->ota_task_handle)){
		auto_enable = ota_app_ctx->auto_ota.auto_enable;
	}
	
	return auto_enable;
}
#endif

void miio_instance_set_uid(miio_handle_t miio_handle, uint64_t uid)
{
	if(uid != miio_get_uid(miio_handle)){
#if	MIIO_INSTANCE_LX2_ADAPT
		char uid_str[32] = {0};
		arch_u64toa(uid, uid_str);
		arch_psm_set_str("ot_config", "uid", uid_str);
#else

		arch_psm_set_value("ot_config", "uid", &uid, sizeof(uid));
#endif
		miio_set_uid(miio_handle, uid);
	}
}

void miio_instance_set_gmt_offset(miio_handle_t miio_handle, int gmt_offset)
{
	if(gmt_offset != miio_get_gmt_offset(miio_handle)){
#if	MIIO_INSTANCE_LX2_ADAPT
		char gmt_str[32] = {0};
		snprintf(gmt_str, sizeof(gmt_str), "%d", gmt_offset);
		arch_psm_set_str("ot_config", "gmt_offset", gmt_str);
#else
		arch_psm_set_value("ot_config", "gmt_offset", &gmt_offset, sizeof(gmt_offset));
#endif
		g_gmt_offset = gmt_offset;
		miio_set_gmt_offset(miio_handle, gmt_offset);
	}
}

void miio_instance_set_model(miio_handle_t miio_handle, const char *model)
{
	char miio_model[MIIO_MODEL_SIZE_MAX] = {0};

	if(NULL == model || 0 == strnlen(model, MIIO_MODEL_SIZE_MAX)){
		model = MIIO_INSTANCE_MODEL;
	}

	miio_get_model(miio_handle, miio_model, MIIO_MODEL_SIZE_MAX);

	if( 0 != strncmp(model, miio_model, MIIO_MODEL_SIZE_MAX) ){
		snprintf(miio_model, sizeof(miio_model), "%s", model);
		arch_psm_set_str("ot_config", "psm_model", miio_model);
		miio_set_model(miio_handle, miio_model);
	}
}

void miio_instance_set_country_domain(miio_handle_t miio_handle, const char *country_domain)
{
	if(NULL == country_domain || 0 == strnlen(country_domain, MIIO_COUNTRY_DOMAIN_SIZE_MAX)){
		arch_psm_erase_key("ot_config", "country_domain");
		arch_psm_erase_country_domain("country_domain");
		miio_set_country_domain(miio_handle, "");
		return;
	}

	char miio_country_domain[MIIO_COUNTRY_DOMAIN_SIZE_MAX] = {0};

	miio_get_country_domain(miio_handle, miio_country_domain, MIIO_COUNTRY_DOMAIN_SIZE_MAX);

	if( 0 != strncmp(country_domain, miio_country_domain, MIIO_COUNTRY_DOMAIN_SIZE_MAX) ){
		snprintf(miio_country_domain, sizeof(miio_country_domain), "%s", country_domain);
		arch_psm_set_str("ot_config", "country_domain", miio_country_domain);
		arch_psm_set_country_domain("country_domain", miio_country_domain);
		miio_set_country_domain(miio_handle, miio_country_domain);
	}
}

#if MIBLE_ENABLE
static int mible_net_config_callback(miio_handle_t miio_handle, mible_net_config_params_t *params)
{
	miio_net_stop();
	miio_instance_set_uid(miio_handle, params->uid);
	miio_instance_set_gmt_offset(miio_handle, params->gmt_offset);
	miio_instance_set_country_domain(miio_handle, params->country_domain);
	if(params->wifi_channel.cc)
		miio_net_set_wifi_channel(params->wifi_channel.cc, 0);
	miio_net_set_bindkey(params->bindkey);
	miio_net_set_config_type(params->config_type);
	miio_net_set_ap(params->ap.ssid, params->ap.password, false);

#if MIIO_KEY_EXCHANGE_ENABLE
	if(NULL != params->bind_idx) {
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "bind_index = %s", params->bind_idx);
		miio_net_set_bindkey_index(miio_handle, params->bind_idx);
	} else {
		miio_net_set_bindkey_index(miio_handle, NULL);
	}

	if(0 != params->bind_ts) {
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "bind_ts = %llu", params->bind_ts);
		miio_net_set_bind_key_ts(params->bind_ts);
	}
#endif /* MIIO_KEY_EXCHANGE_ENABLE */

	return miio_net_start_async(500);
}

static int mible_net_restore_callback(miio_handle_t miio_handle)
{
	miio_restore(miio_handle, "mible.restore");

	miio_reboot(miio_handle, "mible.restore", 10);

	return MIIO_OK;
}

static const mible_net_callbacks_t s_mible_net_callbacks = {
    .config_callback = mible_net_config_callback,
    .restore_callback = mible_net_restore_callback
};
#endif

static void miio_configs_init_default(miio_configs_t *configs)
{
	//get model, optional
	if(arch_psm_get_str("ot_config", "psm_model", configs->model, sizeof(configs->model)) <= 0){
		strncpy(configs->model, MIIO_INSTANCE_MODEL, sizeof(configs->model)-1);
		configs->model[sizeof(configs->model)-1] = '\0';
	}

	//get uid, optional
#if MIIO_INSTANCE_LX2_ADAPT
	{
		char uid_str[32] = {0};
		if(arch_psm_get_str("ot_config", "uid", uid_str, sizeof(uid_str)) <= 0){
			configs->uid = 0;
		}
		else{
			configs->uid = arch_atou64n(uid_str, strnlen(uid_str, sizeof(uid_str)));
		}
	}
#else
	if(arch_psm_get_value("ot_config", "uid", &configs->uid, sizeof(configs->uid)) <= 0){
		configs->uid = 0;
	}
#endif

	//get country_domain, optional
	if(arch_psm_get_str("ot_config", "country_domain", configs->country_domain, sizeof(configs->country_domain)) <= 0) {
		if(arch_psm_get_country_domain("country_domain", configs->country_domain, sizeof(configs->country_domain)) <= 0) {
			configs->country_domain[0] = '\0';
		}
	}

	//get gmt_offset
#if MIIO_INSTANCE_LX2_ADAPT
	{
		char gmt_str[32] = {0};
		if(arch_psm_get_str("ot_config", "gmt_offset", gmt_str, sizeof(gmt_str)) <= 0){
			configs->gmt_offset = 8*60*60;
		}
		else{
			configs->gmt_offset = atoi(gmt_str);
		}
	}
#else
	if(arch_psm_get_value("ot_config", "gmt_offset", &configs->gmt_offset, sizeof(configs->gmt_offset)) <= 0){
		configs->gmt_offset = 8*60*60;
	}
#endif

#if MIIO_AUTO_OTA_ENABLE
#if MIIO_AUTO_OTA_ABILITY
	configs->app_auto_ota_ability = true;
#else
	configs->app_auto_ota_ability = false;
#endif

	if(arch_psm_get_value("ot_config", "mcu_auto_ota_ability", &configs->mcu_auto_ota_ability, sizeof(configs->mcu_auto_ota_ability)) <= 0){
		configs->mcu_auto_ota_ability = false;
	}
#endif

	g_gmt_offset = configs->gmt_offset;

	//init cipher interface
	configs->ciphers_if = miio_ciphers_interface_default();
}

extern void light_startinit(void);
extern int  light_task_start(miio_handle_t handle);
miio_handle_t miio_instance_create(void)
{
	miio_handle_t miio_handle = NULL;

	arch_init();
	light_startinit();

	{
		miio_configs_t miio_configs;
		miio_configs_init_default(&miio_configs);
		miio_handle = miio_create(&miio_configs);
	}

#if MIBLE_ENABLE
	{
		mible_config_t mible_config = {
            .product_id = MIBLE_PRODUCT_ID,
        };
		if(MIIO_OK != mible_gateway_init(&mible_config)){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "Fail to start BLE gateway!");
		}
	}
#endif

	if(miio_handle){
		light_task_start(miio_handle);
		miio_hooks_register(miio_handle, &s_miio_instance_hooks);

		miio_ota_app_init(miio_handle, &s_miio_ota_app_ctx);
#if MIBLE_ENABLE
		{
			static const miio_net_state_callback_t miio_net_state_callbacks[] = {
				{
					.fp = mible_net_state_callback,
					.ctx = NULL
				}
			};
			miio_net_init(miio_handle, miio_net_state_callbacks, NELEMENTS(miio_net_state_callbacks));
		}
#else
		miio_net_init(miio_handle, NULL, 0);
#endif
		miio_net_start();
	}
	else{
		LOG_ERROR_TAG(MIIO_LOG_TAG, "miio create error!");
	}

#if MIBLE_ENABLE
	if(miio_handle){
		if(MIIO_OK != mible_net_register(miio_handle, &s_mible_net_callbacks)){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "Fail to init BLE network config!");
		}
	}
#endif

#if MIIO_COMMANDS_ENABLE
	if(miio_handle){

		mcmd_io_if_t io_if = {
			.create = tou_create,
			.echo = tou_set_echo,
			.block_in = tou_pend_til_recv,
			.out = tou_send,
			.block_quit = tou_quit_pending,
			.in_byte = tou_recv_byte,
			.out_byte = tou_send_byte,
			.destroy = tou_destroy
		};

		mcmd_t* mcmd = mcmd_create(miio_handle, &io_if, (void*)UART_COM);

		if(NULL == mcmd){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "mcmd create error!");
		}

	}

#endif

	miio_instance_log(miio_handle);

	return miio_handle;
}
