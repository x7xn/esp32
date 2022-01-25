/*
 * miio_net.c
 *
 *  Created on: Nov 7, 2017
 *      Author: mashaoze
 */
#include "miio_net.h"
#include "miio_ciphers.h"
#include "miio_net_auto_provision.h"
#include "jsmi.h"
#include "miio_net_indicator.h"
#include "miio_net_analyse.h"
#if MIBLE_ENABLE
#include "mible_gateway.h"
#endif

#undef MIIO_LOG_TAG
#define MIIO_LOG_TAG			"miio_net"

# define MIIO_NET_PROVISIONER_SSID "25c829b1922d3123_miwifi"
# define MIIO_NET_PROVISIONER_PASSWD ""

static int cloud2idle_reason = NET_STATUS_UNKOWN;
static int sta_disconnect_reason = 0;
#if MIIO_KEY_EXCHANGE_ENABLE
static int miio_net_restore_bind_key(void);
#endif
static miio_net_t s_miio_net = {.factory_status = false};

#if MIIO_PROV_STATISTIC
static miio_prov_statistic_t s_prov_statistic = {
	.enable = false,
	.prov_stage = PROV_STAGE_INIT,
};

bool is_prov_stat_enable(void)
{
	miio_prov_statistic_t *prov_statistic = &s_prov_statistic;

	return prov_statistic->enable;
}
uint8_t get_prov_stat_type(void)
{
	miio_prov_statistic_t *prov_statistic = &s_prov_statistic;

	return prov_statistic->prov_stat_cur.prov_type;
}

void miio_net_clear_provision_stat(void)
{
	memset(&s_prov_statistic, 0, sizeof(miio_prov_statistic_t));
}

void increase_prov_times(void)
{
	miio_prov_statistic_t *prov_statistic = &s_prov_statistic;

	prov_statistic->prov_stat_cur.times++;
	LOG_INFO_TAG(MIIO_LOG_TAG, "provision times: %d", prov_statistic->prov_stat_cur.times);
}

void set_prov_stat_stage(miio_prov_rpc_stage_t prov_stage)
{
	miio_prov_statistic_t *prov_statistic = &s_prov_statistic;

	prov_statistic->prov_stage = prov_stage;
}

void set_prov_stat_error_code(miio_prov_error_code_t error_code)
{
	miio_prov_statistic_t *prov_statistic = &s_prov_statistic;

	prov_statistic->prov_stat_cur.prov_error_code = error_code;
}

void set_prov_start_time(void)
{
	miio_prov_statistic_t *prov_statistic = &s_prov_statistic;

	prov_statistic->provision_start_time = arch_os_ms_now();
}

int prov_stat_error_code_process(void)
{
	int ret = 0;
	miio_prov_statistic_t *prov_statistic = &s_prov_statistic;
	prov_stat_one_t *prov_stat_cur = &(prov_statistic->prov_stat_cur);
	prov_stat_one_t *prov_stat_pre = &(prov_statistic->prov_stat_pre);
	prov_stat_one_t *prov_stat_tmp = &(prov_statistic->prov_stat_tmp);

	if(prov_stat_cur->prov_error_code > PROV_ERROR_NONE){
		/*no need to update flash if no error_code change*/
		if((prov_stat_cur->prov_error_code != prov_stat_tmp->prov_error_code) ||
				(prov_stat_cur->times != prov_stat_tmp->times) ||
				(prov_stat_cur->prov_type != prov_stat_tmp->prov_type)) {
			LOG_INFO_TAG(MIIO_LOG_TAG, "update provision reason code: %d", prov_stat_cur->prov_error_code);
			memcpy(prov_stat_tmp, prov_stat_cur, sizeof(prov_stat_one_t));
			ret = arch_psm_set_value("network", PROVISION_PSM_STR, (void*)prov_stat_tmp, sizeof(prov_stat_one_t));
			if(prov_statistic->prov_stage < PROV_STAGE_DEVICE_WIFI_CONNECT){
				/* we just report prov_stat_pre data as last one;
				   before stage PROV_STAGE_DEVICE_WIFI_CONNECT, device may not reboot and can provision again, so copy prov_stat_cur to prov_stat_pre can keep prov_stat_pre new;
				   after having got config router params, device has to be restored to provision again, prov_stat_pre will be updated from flash;
				*/
				memcpy(prov_stat_pre, prov_stat_cur, sizeof(prov_stat_one_t));
			}
		}
	}
	return ret;
}

static int prov_stat_pre_read(miio_net_t *miio_net)
{
	int ret = 0;
	prov_stat_one_t *prov_stat_pre = &(miio_net->prov_statistic->prov_stat_pre);
	prov_stat_one_t *prov_stat_tmp = &(miio_net->prov_statistic->prov_stat_tmp);
	
	/*prov_stat_pre record last provision stat*/	
	ret = arch_psm_get_value("network", PROVISION_PSM_STR, (void*)prov_stat_pre, sizeof(prov_stat_one_t));
	memcpy(prov_stat_tmp, prov_stat_pre, sizeof(prov_stat_one_t));
	LOG_DEBUG_TAG(MIIO_LOG_TAG, "prov_stat_pre: [times %d],[prov_type %d],[prov_error_code %d]",
			prov_stat_pre->times, prov_stat_pre->prov_type, prov_stat_pre->prov_error_code);
	return ret;
}

/*provision type maybe unknown before receiving some rpc like handshake_hello or config_router*/
static void prov_type_update_for_apmode(miio_net_t *miio_net)
{
	prov_stat_one_t *prov_stat_cur = &(miio_net->prov_statistic->prov_stat_cur);

	switch(miio_net->prov_statistic->prov_stage){
		case PROV_STAGE_CONFIG_ROUTER:
			prov_stat_cur->prov_type = PROV_TYPE_AP_NORMAL;
			break;
		case PROV_STAGE_HANDSHAKE_HELLO:
		case PROV_STAGE_HANDSHAKE_ECDH:
		case PROV_STAGE_CONFIG_ROUTER_SAFE:
			prov_stat_cur->prov_type = PROV_TYPE_AP_SAFE;
			break;
		default:
			break;
	}
}

static void miio_prov_statistic_init(miio_net_t *miio_net)
{	
	miio_net->prov_statistic = &s_prov_statistic;
	memset(miio_net->prov_statistic, 0, sizeof(miio_prov_statistic_t));

	/*read last provision data in flash*/
	prov_stat_pre_read(miio_net);
	miio_net->prov_statistic->prov_stat_cur.times = miio_net->prov_statistic->prov_stat_pre.times;

	/*init provision statistic if need provision*/
	if(miio_net->provision_status == false){
		LOG_INFO_TAG(MIIO_LOG_TAG, "start provision stat.");
		miio_net->prov_statistic->enable = true;
		miio_net->prov_statistic->prov_stage = PROV_STAGE_INIT;
		miio_net->prov_statistic->prov_stat_cur.prov_error_code = PROV_ERROR_NONE;
	}
}

static void _miio_provision_report_rpc(miio_net_t *miio_net)
{
	miio_handle_t handle = miio_net->miio_handle;
	uint32_t method_id = miio_get_rpc_id(handle);
	size_t js_size = miio_get_rpc_max_size(handle);
	prov_stat_one_t *prov_stat_pre = &(miio_net->prov_statistic->prov_stat_pre);
	prov_stat_one_t *prov_stat_cur = &(miio_net->prov_statistic->prov_stat_cur);
	char *js = malloc(js_size);
	uint32_t prov_time = 0;

	/*prov_time keep 0 if provision was interrupted by reboot*/
	if(miio_net->prov_statistic->provision_start_time > 0){
		prov_time = arch_os_ms_elapsed(miio_net->prov_statistic->provision_start_time)/1000;
	}
	
	if (js)
	{
		jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, js_size);
		jsmi_compose_start(&jsmi_composer);
		jsmi_set_object_begin(&jsmi_composer);
		jsmi_set_key_value_uint(&jsmi_composer, "id", method_id);
		jsmi_set_key_value_str(&jsmi_composer, "method", "_async.stat", sizeof("_async.stat")-1);
		jsmi_set_key_object_begin(&jsmi_composer, "params");

		//provision info
		jsmi_set_key_object_begin(&jsmi_composer, "miio.provision");
		jsmi_set_key_value_uint(&jsmi_composer, "prov_type", prov_stat_cur->prov_type);
		jsmi_set_key_value_uint(&jsmi_composer, "last_prov_type", prov_stat_pre->prov_type);
		jsmi_set_key_value_uint(&jsmi_composer, "last_reason", prov_stat_pre->prov_error_code);
		jsmi_set_key_value_uint(&jsmi_composer, "prov_time", prov_time);
		jsmi_set_key_value_uint(&jsmi_composer, "fail_count", prov_stat_cur->times-1);
		jsmi_set_key_object_end(&jsmi_composer);

		//model
		char model[32] = {0};
		miio_get_model(handle, model, sizeof(model));
		jsmi_set_key_value_str(&jsmi_composer, "model", model, strnlen(model, sizeof(model)));
		jsmi_set_key_value_str(&jsmi_composer, "miio_ver", MIIO_VERSION_STR, 0);

		jsmi_set_key_object_end(&jsmi_composer);
		jsmi_set_object_end(&jsmi_composer);

		if (MIIO_OK == jsmi_compose_finish(&jsmi_composer, NULL, &js_size))
		{
			miio_rpc_delegate_context_t rpc_context;
			miio_rpc_context_init(&rpc_context);
			miio_rpc_context_config_delegate_arg(&rpc_context, method_id, MIIO_DELEGATE_JSON, js, js_size);
			miio_rpc_context_config_delegate_ack(&rpc_context, NULL, NULL);

			if (MIIO_OK != miio_set_up_rpc_delegate(handle, &rpc_context))
				LOG_WARN_TAG(MIIO_LOG_TAG, "_async.stat provision report failed.");
		}
		else
		{
			LOG_WARN_TAG(MIIO_LOG_TAG, "memory not enough for ncinfo.");
		}

		free(js);
	}
	else
	{
		LOG_WARN_TAG(MIIO_LOG_TAG, "no memory for ncinfo report.");
	}
}

static void miio_provision_stat_report(miio_net_t *miio_net)
{
	prov_stat_one_t prov_stat_pre = {
		.times = 0,
		.prov_type = 0,
		.prov_error_code = PROV_ERROR_NONE,
	};
	if(miio_net->prov_statistic->prov_stat_cur.times > 0)
	{
		LOG_INFO_TAG(MIIO_LOG_TAG, "reporting miio provision information ");

		_miio_provision_report_rpc(miio_net);
		/*avoid offline-online,report again*/
		miio_net_clear_provision_stat();
		/*clear provision stat*/
		arch_psm_set_value("network", PROVISION_PSM_STR, (void*)&prov_stat_pre, sizeof(prov_stat_one_t));
	}
}

static int miio_net_provision_stat_hook(miio_handle_t handle, miio_prov_rpc_stage_t prov_stage, miio_prov_error_code_t error_code, void *ctx)
{
	LOG_INFO_TAG(MIIO_LOG_TAG, "%s called", __FUNCTION__);

	miio_net_t *miio_net = ctx;

	if(miio_net->prov_statistic->enable == false){
		return MIIO_OK;
	}
	
	LOG_DEBUG_TAG(MIIO_LOG_TAG, "prov_stat hook: prov_stage %d, error_code %d", prov_stage, error_code);
	if(prov_stage > PROV_STAGE_INIT){
		miio_net->prov_statistic->prov_stage = prov_stage;
	}

	if(error_code > PROV_ERROR_NONE){
		miio_net->prov_statistic->prov_stat_cur.prov_error_code = error_code;
	}
	
	/*update only after config_router in miio_common stage, otherwise update after disconnect wifi*/
	if(prov_stage > PROV_STAGE_DEVICE_WIFI_CONNECT){
		prov_stat_error_code_process();
	}

	return MIIO_OK;
}

#endif

static int ssid_escape(const char in[32], char out[])
{
    const char *pin;
    char *pout;

    pin = in;
    pout = out;

    while (*pin && pin - in < 32) {
        switch (*pin) {
        case '"':
        case '\\':
        case '/':
            *pout++ = '\\';
            break;
        default:
            break;
        }
        *pout++ = *pin++;
    }

    *pout = '\0';
    return pout - out;
}

static void miio_net_state_change(miio_net_t *miio_net, miio_net_state_t state, miio_net_error_t error)
{
	static miio_net_state_t old_net_state = MIIO_NET_UNPROV;
	static miio_net_error_t old_net_error = MIIO_NET_ERROR_NONE;
	bool changed = false;
	bool open_offline_check = false;
	int ret = MIIO_OK;

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	if(old_net_state != state || old_net_error != error){
		if(old_net_state == MIIO_NET_CLOUD){
			open_offline_check = true;
		}
		old_net_state = state;
		old_net_error = error;
		changed = true;
	}

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	if(changed){
		if(open_offline_check) {
			miio_offline_open();
		}
#if	MIIO_NET_INDICATOR_ENABLE
		miio_net_indicator_set(state);
#endif

		for(int i=0; i < miio_net->callbacks_count;i++){
			if(miio_net->callbacks){
				ret = miio_net->callbacks[i].fp(state, error, miio_net->callbacks[i].ctx);
				if (MIIO_OK != ret) {
					LOG_ERROR_TAG(MIIO_LOG_TAG, "net state callback failed!");
#if MIIO_MONITOR_REBOOT_ENABLE
					miio_reboot(miio_net->miio_handle, "net state callback failed", MIIO_RPC_TIMEOUT_MS);
#endif
				}
			}
		}
	}
}

static int miio_net_info_kvs_hook(miio_handle_t handle, miio_delegate_type_t type, void *composer, void *ctx)
{
	LOG_DEBUG_TAG(MIIO_LOG_TAG, "%s called", __FUNCTION__);
	miio_net_t *miio_net = ctx;

	jsmi_composer_t *jsmi_composer = composer;

    {//mac
    	uint8_t mac[6];
    	char mac_str[18];
    	arch_get_mac(mac);
    	snprintf_hex(mac_str, sizeof(mac_str), mac, sizeof(mac), 0x80 | ':');
        jsmi_set_key_value_str(jsmi_composer, "mac", mac_str, strnlen(mac_str, sizeof(mac_str)));
    }

    {//wifi_fw_ver
    	jsmi_set_key_value_str(jsmi_composer, "wifi_fw_ver", arch_net_get_wifi_version(), 0);
    }

    do{//ap info
        jsmi_set_key_object_begin(jsmi_composer, "ap");
			{
				char ssid[sizeof(miio_net->ssid)*2];
				ssid_escape(miio_net->ssid, ssid);
				jsmi_set_key_value_str(jsmi_composer, "ssid", ssid, strnlen(ssid, sizeof(ssid)));
			}
			{
				char bssid[6*3];
				snprintf_hex(bssid, sizeof(bssid), miio_net->wifi_connect_ctx.current_ap_info.bssid, 6, 0x80 | ':');
				jsmi_set_key_value_str(jsmi_composer, "bssid", bssid, strnlen(bssid, sizeof(bssid)));
			}

			{ /* get realtime rssi */
				net_wifi_ap_info_t ap_info;
				int8_t rssi;
				if(MIIO_OK == arch_net_get_ap_info(&ap_info))
				{
					rssi = ap_info.rssi;
				} else {
					rssi = miio_net->wifi_connect_ctx.current_ap_info.rssi;
				}
				jsmi_set_key_value_sint(jsmi_composer, "rssi", rssi);
			}

			jsmi_set_key_value_sint(jsmi_composer, "primary", miio_net->wifi_connect_ctx.current_ap_info.primary);
		jsmi_set_key_object_end(jsmi_composer);
	}while(0);

    do{//net if
        ip_info_t ip_info = IP_INFO_INIT_VALUE;
        if(MIIO_OK != arch_net_get_ip_info(&ip_info))
        	break;

        char ip_str[64] = {0};
        jsmi_set_key_object_begin(jsmi_composer, "netif");
			arch_net_htoa_buf(&ip_info.ip, ip_str, sizeof(ip_str));
			jsmi_set_key_value_str(jsmi_composer, "localIp", ip_str, strnlen(ip_str, sizeof(ip_str)));
			arch_net_htoa_buf(&ip_info.netmask, ip_str, sizeof(ip_str));
			jsmi_set_key_value_str(jsmi_composer, "mask", ip_str, strnlen(ip_str, sizeof(ip_str)));
			arch_net_htoa_buf(&ip_info.gw, ip_str, sizeof(ip_str));
			jsmi_set_key_value_str(jsmi_composer, "gw", ip_str, strnlen(ip_str, sizeof(ip_str)));
		jsmi_set_key_object_end(jsmi_composer);

    }while(0);

    {//bind_key
		int bind_key_len = strnlen(miio_net->tob.bind_key, sizeof(miio_net->tob.bind_key)-1);
		if(bind_key_len){
			jsmi_set_key_value_str(jsmi_composer, "bindkey", miio_net->tob.bind_key, bind_key_len);
		}
	}

    {//config_type
		int config_type_len = strnlen(miio_net->config_type, sizeof(miio_net->config_type)-1);
		if(config_type_len){
			jsmi_set_key_value_str(jsmi_composer, "config_type", miio_net->config_type, config_type_len);
		}
	}

    return MIIO_OK;
}

static int miio_net_statistics_kvs_hook(miio_handle_t handle, miio_delegate_type_t type, void *composer, void *ctx)
{
	LOG_DEBUG_TAG(MIIO_LOG_TAG, "%s called", __FUNCTION__);

#if MIIO_STATASTIC_ENABLE
	miio_net_t *miio_net = ctx;

	jsmi_composer_t *jsmi_composer = composer;

    {//miio status statistics
		jsmi_set_key_array_begin(jsmi_composer, "miio_times");
			jsmi_set_value_uint32(jsmi_composer, miio_net->monitor.work_time);
			jsmi_set_value_uint32(jsmi_composer, miio_net->monitor.miio_times.idle);
			jsmi_set_value_uint32(jsmi_composer, miio_net->monitor.miio_times.local);
			jsmi_set_value_uint32(jsmi_composer, miio_net->monitor.miio_times.cloud);
		jsmi_set_key_array_end(jsmi_composer);
    }

    do{//ap info
        jsmi_set_key_object_begin(jsmi_composer, "ap");
			{
				char ssid[sizeof(miio_net->ssid)*2];
				ssid_escape(miio_net->ssid, ssid);
				jsmi_set_key_value_str(jsmi_composer, "ssid", ssid, strnlen(ssid, sizeof(ssid)));
			}
			jsmi_set_key_value_sint(jsmi_composer, "connect_count", miio_net->wifi_connect_ctx.connect_count);

			if(miio_net->net_mutex)
				arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

        	if(miio_net->wifi_connect_ctx.repeater_count){
        		jsmi_set_key_array_begin(jsmi_composer,"repeaters");
				{
					miio_ap_info_entry_t *entry = NULL;
					list_for_each_entry(entry, &miio_net->wifi_connect_ctx.ap_info_list, list, miio_ap_info_entry_t)
					{
						jsmi_set_value_object_begin(jsmi_composer);
						{
							char bssid[6*3];
							snprintf_hex(bssid, sizeof(bssid), entry->ap_info.bssid, 6, 0x80 | ':');
							jsmi_set_key_value_str(jsmi_composer, "bssid", bssid, strnlen(bssid, sizeof(bssid)));

							{ /* get realtime rssi */
								if(0 == memcmp(entry->ap_info.bssid, miio_net->wifi_connect_ctx.current_ap_info.bssid, 6)) {
									net_wifi_ap_info_t ap_info;
									int8_t rssi;
									if(MIIO_OK == arch_net_get_ap_info(&ap_info)) {
										rssi = ap_info.rssi;
									} else {
										rssi = miio_net->wifi_connect_ctx.current_ap_info.rssi;
									}
									jsmi_set_key_value_sint(jsmi_composer, "rssi", rssi);
								} else {
									jsmi_set_key_value_sint(jsmi_composer, "rssi", entry->ap_info.rssi);
								}
							}

							jsmi_set_key_value_sint(jsmi_composer, "primary", entry->ap_info.primary);
							jsmi_set_key_value_sint(jsmi_composer, "active", entry->active?1:0);
						}
						jsmi_set_value_object_end(jsmi_composer);
					}
				}
				jsmi_set_key_array_end(jsmi_composer);
        	}
        	if(miio_net->net_mutex)
        		arch_os_mutex_put(miio_net->net_mutex);

        jsmi_set_key_object_end(jsmi_composer);
    }while(0);

#endif

    return MIIO_OK;
}

static int miio_net_online_hook(miio_handle_t handle, void *ctx)
{
	LOG_DEBUG_TAG(MIIO_LOG_TAG, "%s called", __FUNCTION__);

	miio_net_t *miio_net = ctx;

	miio_net_state_change(miio_net, MIIO_NET_CLOUD, MIIO_NET_ERROR_NONE);

	/* if device disconnected once, we need to report it */
	miio_offline_report(handle);
#if MIIO_PROV_STATISTIC
	miio_provision_stat_report(miio_net);
#endif

	/* MUST report last offline first, then update offline info below */
	/* OTU, OTT, OTS will call this hook. So it is appropriate to retrieve rssi, ip, gw for offline report here */
	miio_online_stats_gather(miio_net->wifi_connect_ctx.current_ap_info.rssi);

	return MIIO_OK;
}

static int miio_net_offline_hook(miio_handle_t handle, void *ctx)
{
	LOG_DEBUG_TAG(MIIO_LOG_TAG, "%s called", __FUNCTION__);

	miio_net_t *miio_net = ctx;

	miio_net_state_change(miio_net, MIIO_NET_LOCAL, MIIO_NET_ERROR_NONE);

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	miio_net->monitor.offline_count++;

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	return MIIO_OK;
}

static int miio_net_restore_hook(miio_handle_t handle, void *ctx)
{
	LOG_DEBUG_TAG(MIIO_LOG_TAG, "%s called", __FUNCTION__);

	miio_net_t *miio_net = ctx;

	miio_net_set_ap(NULL, NULL, false);

	miio_net_set_disable_local_restore(false);

	miio_instance_set_uid(miio_net->miio_handle, 0);

	miio_instance_set_gmt_offset(miio_net->miio_handle, 8*60*60);

	miio_instance_set_country_domain(miio_net->miio_handle, NULL);

#if MIIO_AUTO_OTA_ENABLE
	miio_instance_auto_upgrate_time_restore();
#endif

#if MIIO_KEY_EXCHANGE_ENABLE
    miio_net_restore_bind_key();
#endif

#if MIBLE_ENABLE
	mible_gateway_restore();
#endif
	return MIIO_OK;
}

static int miio_net_reboot_hook(miio_handle_t handle, void *ctx)
{
	LOG_DEBUG_TAG(MIIO_LOG_TAG, "%s called", __FUNCTION__);

	miio_net_stop();

	return MIIO_OK;
}

static miio_hooks_t s_miio_net_hooks = {
	.online = miio_net_online_hook,
	.offline = miio_net_offline_hook,
	.info = miio_net_info_kvs_hook,
	.ext_rpc = NULL,
	.restore = miio_net_restore_hook,
	.reboot = miio_net_reboot_hook,
	.statistics = miio_net_statistics_kvs_hook,
#if MIIO_PROV_STATISTIC
	.prov_stat = miio_net_provision_stat_hook,
#endif
	.ctx = &s_miio_net
};


static miio_ap_info_entry_t *get_ap_info_by_bssid(list_head_t *ap_info_list, uint8_t bssid[6])
{
	miio_ap_info_entry_t *entry = NULL;
	list_for_each_entry(entry, ap_info_list, list, miio_ap_info_entry_t)
	{
		if (memcmp(bssid, entry->ap_info.bssid, 6) == 0) {
			return entry;
		}
	}
	return NULL;
}

static miio_ap_info_entry_t *get_ap_info_best(list_head_t *ap_info_list)
{
	miio_ap_info_entry_t *best = NULL;
	miio_ap_info_entry_t *entry = NULL;
	list_for_each_entry(entry, ap_info_list, list, miio_ap_info_entry_t)
	{
		if(entry->active){
			if(best == NULL){
				best = entry;
			}
			else if(best->ap_info.rssi < entry->ap_info.rssi){
				best = entry;
			}
		}
	}
	return best;
}

static miio_ap_info_entry_t *get_ap_info_worst(list_head_t *ap_info_list)
{
	miio_ap_info_entry_t *worst = NULL;
	miio_ap_info_entry_t *entry = NULL;
	list_for_each_entry(entry, ap_info_list, list, miio_ap_info_entry_t)
	{
		if(worst == NULL){
			worst = entry;
		}
		else if(worst->active && !entry->active){
			worst = entry;
		}
		else if(worst->ap_info.rssi > entry->ap_info.rssi){
			worst = entry;
		}
	}
	return worst;
}

static size_t get_ap_info_count(list_head_t *ap_info_list, bool active)
{
	size_t ap_info_count = 0;
	miio_ap_info_entry_t *entry = NULL;
	list_for_each_entry(entry, ap_info_list, list, miio_ap_info_entry_t)
	{
		if(entry->active == active)
			ap_info_count++;
	}
	return ap_info_count;
}

static miio_ap_info_entry_t *insert_ap_info(list_head_t *ap_info_list, const wifi_ap_record_t* ap_info)
{
	miio_ap_info_entry_t *entry = NULL;

	list_for_each_entry(entry, ap_info_list, list, miio_ap_info_entry_t)
	{
		if (memcmp(ap_info->bssid, entry->ap_info.bssid, 6) == 0) {
			return NULL; /* alreay exist */
		}
	}

	/* new */
	entry = calloc(1, sizeof(miio_ap_info_entry_t));
	if(entry){
		INIT_LIST_HEAD(&entry->list);
		list_add(&entry->list, ap_info_list);
	} else {
		LOG_WARN_TAG(MIIO_LOG_TAG, "no mem for ap info entry");
	}

	return entry;
}

static void delete_ap_info_by_bssid(list_head_t *ap_info_list, uint8_t bssid[6])
{
	miio_ap_info_entry_t *entry = NULL;
	miio_ap_info_entry_t *tmp = NULL;
	list_for_each_entry_safe(entry, tmp, ap_info_list, list, miio_ap_info_entry_t)
	{
		if (memcmp(bssid, entry->ap_info.bssid, 6) == 0) {
			list_del(&entry->list);
			free(entry);
		}
	}
}

static int miio_wifi_if_has_next(miio_net_t *miio_net)
{
	int searched = 0;

	if(miio_net->net_mutex) {
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);
	}

	if(miio_net->net_started) {
		miio_ap_info_entry_t *entry = get_ap_info_by_bssid(&miio_net->wifi_connect_ctx.ap_info_list, miio_net->wifi_connect_ctx.current_ap_info.bssid);
		if(entry) {
			entry->active = false;
		}

		list_for_each_entry(entry, &miio_net->wifi_connect_ctx.ap_info_list, list, miio_ap_info_entry_t) {
			if(entry->active) {
				searched = 1;
			}
		}

		miio_net->wifi_connect_ctx.sta_status = MIIO_WIFI_STA_IDLE;
	}

	if(miio_net->net_mutex) {
		arch_os_mutex_put(miio_net->net_mutex);
	}

	return searched;
}


void miio_wifi_disconnect(miio_net_t *miio_net)
{
	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	if(miio_net->net_started){

		esp_wifi_disconnect();

		miio_ap_info_entry_t *entry = get_ap_info_by_bssid(&miio_net->wifi_connect_ctx.ap_info_list, miio_net->wifi_connect_ctx.current_ap_info.bssid);
		if(entry){
			entry->active = false;
		}

		miio_net->wifi_connect_ctx.sta_status = MIIO_WIFI_STA_IDLE;
	}

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);
}

int miio_wifi_connect(miio_net_t *miio_net)
{
	int ret = MIIO_ERROR;
	
	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	if(!miio_net->net_started){
		LOG_WARN_TAG(MIIO_LOG_TAG, "net stoped");
		ret = MIIO_OK;
		goto safe_exit;
	}

	miio_ap_info_entry_t *entry = get_ap_info_best(&miio_net->wifi_connect_ctx.ap_info_list);
	if(NULL == entry){
		ret = MIIO_ERROR_NOTFOUND;
		goto safe_exit;
	}
	else{
		char bssid[sizeof(entry->ap_info.bssid)*3];
		snprintf_hex(bssid, sizeof(bssid), entry->ap_info.bssid, sizeof(entry->ap_info.bssid), 0x80 | ':');
		LOG_INFO_TAG(MIIO_LOG_TAG, "bingo ssid=%s, bssid=%s, pri=%d, rssi=%d", entry->ap_info.ssid, bssid, entry->ap_info.primary, entry->ap_info.rssi);
	}
	memcpy(&miio_net->wifi_connect_ctx.current_ap_info, &entry->ap_info, sizeof(miio_ap_info_t));

	/* if cc != CN, esp_wifi_set_country() will restart wifi in esp-idf v4.0, and this will cause connect error. */
#if 0
	{
		wifi_country_t country;
		memset(&country, 0, sizeof(wifi_country_t));
		strncpy(country.cc, miio_net->wifi_channel_info.cc, sizeof(country.cc)-1);
		country.cc[sizeof(country.cc)-1] = '\0';
		country.schan = miio_net->wifi_channel_info.schan;
		country.nchan = miio_net->wifi_channel_info.nchan;
		country.max_tx_power = miio_net->wifi_channel_info.max_tx_dbm;
		country.policy = WIFI_COUNTRY_POLICY_MANUAL;

		if(ESP_OK != esp_wifi_set_country(&country)){
			LOG_WARN_TAG(MIIO_LOG_TAG, "sta set cc failed");
		}
	}
#endif

	{
		wifi_config_t config;
		memset(&config, 0, sizeof(wifi_config_t));
		strncpy((char *)config.sta.ssid, miio_net->ssid, sizeof(config.sta.ssid));
		strncpy((char *)config.sta.password, miio_net->password, sizeof(config.sta.password));
		config.sta.bssid_set = true; // need to check MAC address of AP
		memcpy(config.sta.bssid, miio_net->wifi_connect_ctx.current_ap_info.bssid, 6);
		config.sta.scan_method = WIFI_FAST_SCAN;// do all channel scan or fast scan
		config.sta.channel = miio_net->wifi_connect_ctx.current_ap_info.primary;       // channel of target AP. Set to 1~13 to scan starting from the specified channel before connecting to AP. If the channel of AP is unknown, set it to 0.
		config.sta.listen_interval = 1;   // Listen interval for ESP32 station to receive beacon when WIFI_PS_MAX_MODEM is set. Units: AP beacon intervals. Defaults to 3 if set to 0.
		config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;    // sort the connect AP in the list by rssi or security mode */
		config.sta.threshold.rssi = -127;     // When scan_method is set to WIFI_FAST_SCAN, only APs which have an auth mode that is more secure than the selected auth mode and a signal stronger than the minimum RSSI will be used.
		if(WIFI_AUTH_OPEN == miio_net->wifi_connect_ctx.current_ap_info.authmode)
			config.sta.threshold.authmode = WIFI_AUTH_OPEN;
		else
			config.sta.threshold.authmode = WIFI_AUTH_WEP;

		/* support WPA3, compatible with WPA/WPA2 */
		config.sta.pmf_cfg.capable = true;
		config.sta.pmf_cfg.required = false;

		if(ESP_OK != esp_wifi_set_config(WIFI_IF_STA, &config)){
			LOG_WARN_TAG(MIIO_LOG_TAG, "sta config failed");
		}
	}

	if(ESP_OK != esp_wifi_connect()){
		miio_net->wifi_connect_ctx.sta_status = MIIO_WIFI_STA_IDLE;
		LOG_ERROR_TAG(MIIO_LOG_TAG, "sta connect failed");
		ret = MIIO_ERROR_NOTREADY;
		goto safe_exit;
	}

	miio_net->wifi_connect_ctx.sta_status = MIIO_WIFI_STA_CONNECTING;
	miio_net->wifi_connect_ctx.connect_count++;

	ret = MIIO_OK;

safe_exit:

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	return ret;
}

int miio_wifi_try_reconnect(miio_net_t *miio_net)
{
	int ret = MIIO_ERROR;

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	if(!miio_net->net_started){
		LOG_WARN_TAG(MIIO_LOG_TAG, "net stoped");
		ret = MIIO_OK;
		goto safe_exit;
	}

	esp_wifi_disconnect();

	if(get_ap_info_count(&miio_net->wifi_connect_ctx.ap_info_list, true) < 2){
		ret = MIIO_ERROR_TRYOUT;
		goto safe_exit;
	}

	LOG_INFO_TAG(MIIO_LOG_TAG, "Wifi station reconnecting.");

	ret = MIIO_OK;

safe_exit:

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	return ret;
}

static int wifi_scan_next_channel_start(miio_net_t *miio_net)
{
	static uint8_t scan_round = 0, scan_channel = 0;
	uint8_t prefer_channels[4] = {0, 1, 6, 11};
	wifi_scan_config_t scan_config = {0};

	prefer_channels[0] = miio_net->wifi_connect_ctx.current_ap_info.primary;

	while (scan_channel == 0
			&& miio_net->wifi_connect_ctx.ichan < miio_net->wifi_channel_info.nchan + NELEMENTS(prefer_channels)) {
		uint8_t channel = 0;

		if (miio_net->wifi_connect_ctx.ichan < NELEMENTS(prefer_channels)) {
			channel = prefer_channels[miio_net->wifi_connect_ctx.ichan];
			if (channel < miio_net->wifi_channel_info.schan
					|| channel > miio_net->wifi_channel_info.schan + miio_net->wifi_channel_info.nchan - 1
					|| (miio_net->wifi_connect_ctx.ichan != 0 && channel == prefer_channels[0])) {
				miio_net->wifi_connect_ctx.ichan++;
				continue;
			}

			/* scan prefer with const time */
			scan_round = MIIO_NET_WIFI_SCAN_TIME_MS_MAX/MIIO_NET_WIFI_SCAN_TIME_MS_MIN;
		} else {
			bool is_in_prefer = false;
			int i;

			channel = miio_net->wifi_channel_info.schan + miio_net->wifi_connect_ctx.ichan - NELEMENTS(prefer_channels);

			for (i = 0; i < NELEMENTS(prefer_channels); i++) {
				if (channel == prefer_channels[i]) {
					is_in_prefer = true;
					break;
				}
			}

			if (is_in_prefer) {
				miio_net->wifi_connect_ctx.ichan++;
				continue;
			}

			/* scan others with dynamic time */
			scan_round = miio_net->wifi_connect_ctx.scan_time/MIIO_NET_WIFI_SCAN_TIME_MS_MIN;
		}

		miio_net->wifi_connect_ctx.ichan++;
		scan_channel = channel;
	}

	if (scan_channel) {
		scan_config.ssid = (uint8_t*)miio_net->ssid;
		scan_config.bssid = NULL;
		scan_config.channel = scan_channel;
		scan_config.show_hidden = true;
		scan_config.scan_type = WIFI_SCAN_TYPE_ACTIVE;

#if 0 /* scan time cannot be set in v4.0 idf, default scan time is 120 ms */
		scan_config.scan_time.active.min = MIIO_NET_WIFI_SCAN_TIME_MS_MIN;
		scan_config.scan_time.active.max = MIIO_NET_WIFI_SCAN_TIME_MS_MIN;
#endif

		if(ESP_OK == esp_wifi_scan_start(&scan_config, false)){
			/* default scan time is 120 ms in v4.0 idf */
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "wifi scanning channel %d for 120ms...", scan_config.channel);

			scan_round--;
			if (scan_round == 0) {
				scan_channel = 0;
			}
			return MIIO_OK;
		}
	}

	return MIIO_ERROR;
}

static int miio_wifi_scan_start(miio_net_t *miio_net)
{
	int ret = MIIO_ERROR;

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	if(!miio_net->net_started){
		LOG_WARN_TAG(MIIO_LOG_TAG, "net stoped");
		ret = MIIO_OK;
		goto safe_exit;
	}

	//delete all entry
	{
		miio_ap_info_entry_t *entry = NULL;
		miio_ap_info_entry_t *tmp = NULL;
		list_for_each_entry_safe(entry, tmp, &miio_net->wifi_connect_ctx.ap_info_list, list, miio_ap_info_entry_t)
		{
			list_del(&entry->list);
			free(entry);
		}
		miio_net->wifi_connect_ctx.repeater_count = 0;
	}

	//start scan
	miio_net->wifi_connect_ctx.ichan = 0;
	if (MIIO_OK == wifi_scan_next_channel_start(miio_net)) {
		miio_net->wifi_connect_ctx.sta_status = MIIO_WIFI_STA_SCANNING;
		ret = MIIO_OK;
	} else {
		ret = MIIO_ERROR_NOTREADY;
	}

safe_exit:

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	return ret;
}

static int miio_wifi_scan_finish(miio_net_t *miio_net)
{
	int ret = MIIO_ERROR;

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	if(!miio_net->net_started){
		LOG_WARN_TAG(MIIO_LOG_TAG, "net stoped");
		ret = MIIO_OK;
		goto safe_exit;
	}

	{//handle records
		uint16_t ap_nums = 0;
		esp_wifi_scan_get_ap_num(&ap_nums);
		if(ap_nums){
			wifi_ap_record_t *ap_records = malloc(ap_nums * sizeof(wifi_ap_record_t));
			if(NULL == ap_records){
				LOG_WARN_TAG(MIIO_LOG_TAG, "no mem for scan records");
			}
			else{
				if (ESP_OK != esp_wifi_scan_get_ap_records(&ap_nums,ap_records)) {
					LOG_WARN_TAG(MIIO_LOG_TAG, "get scan records failed");
				}
				else{
					for(int i=0; i<ap_nums; i++) {
						if(ap_records[i].authmode == WIFI_AUTH_OPEN){
							if('\0' != miio_net->password[0])
								continue;
						}
                        if (!miio_net->provision_status && ap_records[i].rssi < -70) {
                            LOG_DEBUG_TAG(MIIO_LOG_TAG, "ignore weak bssid=%02x:%02x:%02x:%02x:%02x:%02x, pri=%d, rssi=%d",
                                ap_records[i].bssid[0], ap_records[i].bssid[1], ap_records[i].bssid[2],
                                ap_records[i].bssid[3], ap_records[i].bssid[4], ap_records[i].bssid[5],
                                ap_records[i].primary, ap_records[i].rssi);
                            continue;
                        }
						miio_ap_info_entry_t *entry = insert_ap_info(&miio_net->wifi_connect_ctx.ap_info_list, &ap_records[i]);
						if(NULL == entry){
							continue;
						}
						else {
							//init
							entry->ap_info.ssid = miio_net->ssid;
							memcpy(entry->ap_info.bssid, ap_records[i].bssid, 6);
							entry->ap_info.primary = ap_records[i].primary;
							entry->ap_info.second = ap_records[i].second;
							entry->ap_info.rssi = ap_records[i].rssi;
							entry->ap_info.authmode = ap_records[i].authmode;
							entry->ap_info.pairwise_cipher = ap_records[i].pairwise_cipher;
							entry->ap_info.phy_mode = ap_records[i].phy_11b | (ap_records[i].phy_11g << 1) | (ap_records[i].phy_11n << 2) |
									(ap_records[i].phy_lr << 3) | (ap_records[i].wps << 4);
							entry->active = true;
							{
								char bssid[sizeof(entry->ap_info.bssid)*3];
								snprintf_hex(bssid, sizeof(bssid), entry->ap_info.bssid, sizeof(entry->ap_info.bssid), 0x80 | ':');
								LOG_DEBUG_TAG(MIIO_LOG_TAG, "insert ssid=%s, bssid=%s, pri=%d, rssi=%d, authmode=%d", entry->ap_info.ssid, bssid, entry->ap_info.primary, entry->ap_info.rssi, entry->ap_info.authmode);
							}
						}

						miio_net->wifi_connect_ctx.repeater_count++;
						if(miio_net->wifi_connect_ctx.repeater_count > MIIO_NET_WIFI_AP_REPEATER_NUMS){
							entry = get_ap_info_worst(&miio_net->wifi_connect_ctx.ap_info_list);
							{
								char bssid[sizeof(entry->ap_info.bssid)*3];
								snprintf_hex(bssid, sizeof(bssid), entry->ap_info.bssid, sizeof(entry->ap_info.bssid), 0x80 | ':');
								LOG_DEBUG_TAG(MIIO_LOG_TAG, "delete ssid=%s, bssid=%s, pri=%d, rssi=%d, authmode=%d", entry->ap_info.ssid, bssid, entry->ap_info.primary, entry->ap_info.rssi, entry->ap_info.authmode);
							}
							delete_ap_info_by_bssid(&miio_net->wifi_connect_ctx.ap_info_list, entry->ap_info.bssid);
							miio_net->wifi_connect_ctx.repeater_count--;
						}
					}
				}
				free(ap_records);
			}
		}
	}

	//scan next channel
	if (MIIO_OK == wifi_scan_next_channel_start(miio_net)) {
		ret = MIIO_ERROR_BUSY;
		goto safe_exit;
	}

	//scan finished
	{
		esp_wifi_scan_stop();
		miio_net->wifi_connect_ctx.repeater_count = get_ap_info_count(&miio_net->wifi_connect_ctx.ap_info_list, true);
		miio_net->wifi_connect_ctx.sta_status = MIIO_WIFI_STA_SCANNED;
		miio_net->wifi_connect_ctx.ichan = 0;
		if(0 == miio_net->wifi_connect_ctx.repeater_count){
			miio_net->wifi_connect_ctx.scan_time = MIIO_NET_WIFI_SCAN_TIME_MS(miio_net->wifi_connect_ctx.scan_time*2);
			miio_net->monitor.offline_timeout = MIIO_MONITOR_OFFLINE_TIMEOUT_S;
			miio_net->monitor.offline_count_out = MIIO_NET_MONITOR_OFFLINE_COUNT_MAX(miio_net->monitor.offline_timeout*1000);
		}
		else{
			miio_net->wifi_connect_ctx.scan_time = MIIO_NET_WIFI_SCAN_TIME_MS(miio_net->wifi_connect_ctx.scan_time/2);
			miio_net->monitor.offline_timeout = MIIO_MONITOR_OFFLINE_TIMEOUT_S/miio_net->wifi_connect_ctx.repeater_count;
			miio_net->monitor.offline_count_out = MIIO_NET_MONITOR_OFFLINE_COUNT_MAX(miio_net->monitor.offline_timeout*1000);
		}
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "wifi scan finished, %u %s found.", miio_net->wifi_connect_ctx.repeater_count, miio_net->ssid);
		ret = MIIO_OK;
	}

safe_exit:

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	return ret;
}

int miio_net_event_post(miio_wifi_event_type_t type)
{
	miio_net_t *miio_net = &s_miio_net;
	miio_wifi_event_t event = {
		.type = type
	};
	return arch_os_queue_send(miio_net->monitor.mq, &event, ARCH_OS_WAIT_FOREVER);
}
extern void light_remote_blesucess(void);
extern void light_remote_getipsucess(void);
static esp_err_t wifi_cb_event_handler(void *ctx, system_event_t *event)
{
	miio_net_t *miio_net = &s_miio_net;
# define MIIO_NET_CONNECT_RETRY 3 /* connect retry times */
	static uint8_t connect_retry = MIIO_NET_CONNECT_RETRY;
#if MIIO_PROV_STATISTIC
	miio_prov_statistic_t *prov_statistic = miio_net->prov_statistic;
#endif

	LOG_INFO_TAG(MIIO_LOG_TAG, "Wifi event[%u]",event->event_id);

	if(miio_net->auto_provision && miio_net_auto_provision_wifi_event_process(miio_net->auto_provision, event)){
		return ESP_OK;
	}
	switch(event->event_id) {
	case SYSTEM_EVENT_SCAN_DONE:
		miio_net_event_post(MIIO_WIFI_EVENT_SCAN_DONE);

		break;
	case SYSTEM_EVENT_STA_START:

		LOG_INFO_TAG(MIIO_LOG_TAG, "Wifi STA start");
		if(ESP_OK != tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, miio_net->host_name)){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "dhcp set hostname error!");
		}
		if(!miio_net->provision_status){
			LOG_INFO_TAG(MIIO_LOG_TAG, "Wifi STA start and try to connect provisoner ssid");
			esp_wifi_connect();
			break;
		}
		miio_net_event_post(MIIO_WIFI_EVENT_STA_START);

		break;
	case SYSTEM_EVENT_STA_STOP:

		if(!miio_net->provision_status){
			break;
		}

		miio_set_mode(miio_net->miio_handle, MIIO_MODE_IDLE);

		miio_net_event_post(MIIO_WIFI_EVENT_STA_STOP);

		break;
	case SYSTEM_EVENT_STA_CONNECTED:

		if(!miio_net->provision_status){
			LOG_INFO_TAG(MIIO_LOG_TAG, "connected to provisoner ssid:%s bssid:%s channel:%d, authmode:%d", 
			event->event_info.connected.bssid,
			event->event_info.connected.bssid,
			event->event_info.connected.channel,
			event->event_info.connected.authmode);
			break;
		}
#if MIIO_PROV_STATISTIC
		if(prov_statistic->enable){
			prov_statistic->prov_stage = PROV_STAGE_DEVICE_WIFI_CONNECT;
		}	
#endif

		miio_net_event_post(MIIO_WIFI_EVENT_STA_CONNECTED);

		break;
	case SYSTEM_EVENT_STA_GOT_IP:

		if(!miio_net->provision_status){
			break;
		}
		connect_retry = MIIO_NET_CONNECT_RETRY;

		LOG_DEBUG_TAG(MIIO_LOG_TAG, "Wifi ip="IPSTR",mask="IPSTR",gw="IPSTR"",
			IP2STR(&event->event_info.got_ip.ip_info.ip),
			IP2STR(&event->event_info.got_ip.ip_info.netmask),
			IP2STR(&event->event_info.got_ip.ip_info.gw));

		miio_net_event_post(MIIO_WIFI_EVENT_STA_GOT_IP);

		light_remote_getipsucess();
		break;
	case SYSTEM_EVENT_GOT_IP6:

		LOG_DEBUG_TAG(MIIO_LOG_TAG, "Wifi ipv6="IPV6STR,
					IPV62STR(event->event_info.got_ip6.ip6_info.ip));

		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:

		if(!miio_net->provision_status){
			break;
		}

		LOG_WARN_TAG(MIIO_LOG_TAG, "Wifi disconnect from ssid %s, reason %d",
				   event->event_info.disconnected.ssid,
				   event->event_info.disconnected.reason);
		cloud2idle_reason = NET_STATUS_STA_DISCONNECTED;
        sta_disconnect_reason = event->event_info.disconnected.reason;

        miio_set_mode(miio_net->miio_handle, MIIO_MODE_IDLE);
		switch(event->event_info.disconnected.reason)
		{
			case WIFI_REASON_AUTH_FAIL:
				miio_net_state_change(miio_net, MIIO_NET_DISCONNECTED, MIIO_NET_ERROR_AUTH_FAIL);
				break;
			default:
				miio_net_state_change(miio_net, MIIO_NET_DISCONNECTED, MIIO_NET_ERROR_UNKNOW);
				break;
		}
#if MIIO_PROV_STATISTIC
		{
			static uint8_t prov_stat_passwderror_cnt = 0;
			/*passwd error*/
			if(prov_statistic->enable){
				if((event->event_info.disconnected.reason == WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT) ||
						(event->event_info.disconnected.reason == WIFI_REASON_HANDSHAKE_TIMEOUT)){
					/*passwd error 3 times,record error code*/
					if(++prov_stat_passwderror_cnt >= MAX_PROV_PASSWD_ERROR_TIMES){
						prov_stat_passwderror_cnt = 0;
						prov_statistic->prov_stat_cur.prov_error_code = PROV_ERROR_PASSWD_ERROR;
						prov_stat_error_code_process();
					}
				}
			}	
		}	
#endif

		if (event->event_info.disconnected.reason == WIFI_REASON_ASSOC_EXPIRE
				|| event->event_info.disconnected.reason == WIFI_REASON_AUTH_EXPIRE
				|| event->event_info.disconnected.reason == WIFI_REASON_NO_AP_FOUND) {
			if (connect_retry-- > 0) {
				if(MIIO_OK == miio_wifi_connect(miio_net)){
					break;
				}
			}
		}
		connect_retry = MIIO_NET_CONNECT_RETRY;

		miio_net_event_post(MIIO_WIFI_EVENT_STA_DISCONNECTED);

		break;
	case SYSTEM_EVENT_STA_LOST_IP:

		LOG_WARN_TAG(MIIO_LOG_TAG, "dhcp failed");

		cloud2idle_reason = NET_STATUS_LOCAL_IP_GET_FAIL;

		miio_set_mode(miio_net->miio_handle, MIIO_MODE_IDLE);
#if MIIO_PROV_STATISTIC
		/*dhcp fail after provision*/
		if(prov_statistic->enable && miio_net->provision_status){
			prov_statistic->prov_stat_cur.prov_error_code = PROV_ERROR_STA_DHCP_FAIL;
			prov_stat_error_code_process();
		}	
#endif
		miio_net_event_post(MIIO_WIFI_EVENT_STA_LOST_IP);

		break;
	case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:

		LOG_INFO_TAG(MIIO_LOG_TAG, "Wifi auth_mode %d -> %d",
				   event->event_info.auth_change.old_mode,
				   event->event_info.auth_change.new_mode);

		break;
	case SYSTEM_EVENT_AP_START:

		LOG_INFO_TAG(MIIO_LOG_TAG, "Wifi ap start");

		if(ESP_OK != tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_AP, miio_net->host_name))
			LOG_ERROR_TAG(MIIO_LOG_TAG, "dhcp set hostname error!");

		miio_set_mode(miio_net->miio_handle, MIIO_MODE_LOCAL);

		miio_net_state_change(miio_net, MIIO_NET_UAP, MIIO_NET_ERROR_NONE);

		break;
	case SYSTEM_EVENT_AP_STOP:

		LOG_INFO_TAG(MIIO_LOG_TAG, "Wifi ap stop");

		miio_set_mode(miio_net->miio_handle, MIIO_MODE_IDLE);

		miio_net_state_change(miio_net, MIIO_NET_UNPROV, MIIO_NET_ERROR_NONE);

		if(s_miio_net.provision_status)light_remote_blesucess();
		break;
	case SYSTEM_EVENT_AP_STACONNECTED:

		LOG_INFO_TAG(MIIO_LOG_TAG, "Wifi station "MACSTR" join, AID = %d",
				  MAC2STR(event->event_info.sta_connected.mac),
				  event->event_info.sta_connected.aid);
#if MIIO_PROV_STATISTIC
		if(prov_statistic->enable){
			prov_statistic->prov_stage = PROV_STAGE_AP_WIFI_CONNECT;
			prov_statistic->prov_stat_cur.prov_type = PROV_TYPE_AP;
			/*ap sta connected means a provision start*/
			prov_statistic->prov_stat_cur.times++;
			LOG_INFO_TAG(MIIO_LOG_TAG, "provision times: %d", prov_statistic->prov_stat_cur.times);
			set_prov_start_time();
		}	
#endif
		break;
	case SYSTEM_EVENT_AP_STADISCONNECTED:

		LOG_INFO_TAG(MIIO_LOG_TAG, "Wifi station "MACSTR"leave, AID = %d",
				  MAC2STR(event->event_info.sta_disconnected.mac),
				  event->event_info.sta_disconnected.aid);
#if MIIO_PROV_STATISTIC
		if(prov_statistic->enable  && !miio_net->provision_status){
			if(prov_statistic->prov_stat_cur.prov_error_code == PROV_ERROR_NONE){
				/*some rpc may lost*/
				switch(prov_statistic->prov_stage){
					case PROV_STAGE_AP_WIFI_CONNECT:
						prov_statistic->prov_stat_cur.prov_error_code = PROV_ERROR_PROBE_LOST;
						break;
					case PROV_STAGE_PROBE:
					case PROV_STAGE_INFO:
						prov_statistic->prov_stat_cur.prov_error_code = PROV_ERROR_ONLY_PROBE_RECEIVED;
						break;
					case PROV_STAGE_HANDSHAKE_HELLO:
						prov_statistic->prov_stat_cur.prov_error_code = PROV_ERROR_HANDSHAKE_ECDH_RPC_LOST;
						break;
					case PROV_STAGE_HANDSHAKE_ECDH:
						prov_statistic->prov_stat_cur.prov_error_code = PROV_ERROR_CONFIG_ROUTER_SAFE_RPC_LOST;
						break;
					default:
						break;
				}
			}
			prov_type_update_for_apmode(miio_net);
			/*if some error happened after disconnect, save provision stat info */
			prov_stat_error_code_process();	
		}
		
#endif

		break;
	default:
		break;
	}
	return ESP_OK;
}

static bool info_monitor_timeout(miio_net_t *miio_net, uint32_t monitor_interval_s)
{
	bool timeout = false;

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	miio_net->monitor.info_time += monitor_interval_s;

	if(miio_net->monitor.info_time >= MIIO_MONITOR_INFO_TIMEOUT_S){
		miio_net->monitor.info_time = 0;
		timeout = true;
	}
	else{
		timeout = false;
	}

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	return timeout;
}

static bool smart_config_monitor_timeout(miio_net_t *miio_net, uint32_t monitor_interval_s)
{
	bool timeout = false;

	if(miio_net->net_mutex) {
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);
	}

	miio_net->monitor.smart_config_time += monitor_interval_s;

	if(miio_net->monitor.smart_config_time >= MIIO_NET_SMART_CONFIG_TIMEOUT_S) {
		miio_net->monitor.smart_config_time = 0;
		timeout = true;
		LOG_WARN_TAG(MIIO_LOG_TAG, "smart config timeout");
	}
	else {
		timeout = false;
	}

	if(miio_net->net_mutex) {
		arch_os_mutex_put(miio_net->net_mutex);
	}

	return timeout;
}


static bool ap_monitor_timeout(miio_net_t *miio_net, uint32_t monitor_interval_s)
{
	bool timeout = false;

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	miio_net->monitor.ap_time += monitor_interval_s;

	if(miio_net->monitor.ap_time >= MIIO_MONITOR_AP_TIMEOUT_S){
		miio_net->monitor.ap_time = 0;
		timeout = true;
		LOG_WARN_TAG(MIIO_LOG_TAG, "ap timeout");

	}
	else{
		timeout = false;
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "ap will close in %us", MIIO_MONITOR_AP_TIMEOUT_S - miio_net->monitor.ap_time);
	}

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	return timeout;
}

static bool sta_monitor_offline_timeout(miio_net_t *miio_net, uint32_t monitor_interval_s)
{
	bool triggered = false;

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	miio_net->monitor.offline_time += monitor_interval_s;

	if(miio_net->monitor.offline_time >= miio_net->monitor.offline_timeout){
		miio_net->monitor.offline_time = 0;
		triggered = true;
		miio_set_event_str(miio_net->miio_handle, "miio_monitor", "offline timeout", sizeof("offline timeout")-1);
		LOG_WARN_TAG(MIIO_LOG_TAG, "sta offline timeout");
	}
	else{
		triggered = false;
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "sta will close in %us",
				miio_net->monitor.offline_timeout - miio_net->monitor.offline_time);
	}

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	return triggered;
}

static void sta_monitor_offline_timeout_reset(miio_net_t *miio_net)
{
	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	miio_net->monitor.offline_time = 0;

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);
}

static uint32_t offline_count(uint32_t now, uint32_t last)
{
	if(last < now)
		return now - last;
	else//鍙兘婧㈠嚭
		return 0xFFFFFFFF - last + now + 1;
}

static bool sta_monitor_offline_countout(miio_net_t *miio_net, uint32_t monitor_interval_s)
{
	bool triggered = false;

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	miio_net->monitor.offline_count_time += monitor_interval_s;
	if(miio_net->monitor.offline_count_time >= miio_net->monitor.offline_timeout/NELEMENTS(miio_net->monitor.offline_count_stamp)){
		miio_net->monitor.offline_count_time = 0;
		for(int i=0; i < NELEMENTS(miio_net->monitor.offline_count_stamp)-1; i++){
			miio_net->monitor.offline_count_stamp[i] = miio_net->monitor.offline_count_stamp[i+1];
		}
		miio_net->monitor.offline_count_stamp[NELEMENTS(miio_net->monitor.offline_count_stamp)-1] = miio_net->monitor.offline_count;

		if(offline_count(miio_net->monitor.offline_count_stamp[NELEMENTS(miio_net->monitor.offline_count_stamp)-1], miio_net->monitor.offline_count_stamp[0]) >= miio_net->monitor.offline_count_out){
			triggered = true;
			miio_set_event_str(miio_net->miio_handle, "miio_monitor", "offline countout", sizeof("offline countout")-1);
			LOG_WARN_TAG(MIIO_LOG_TAG, "sta offline %u times in last %us",
					miio_net->monitor.offline_count_out,
					miio_net->monitor.offline_timeout);
		}
		else{
			triggered = false;
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "sta offline %u times in last %us",
					offline_count(miio_net->monitor.offline_count_stamp[NELEMENTS(miio_net->monitor.offline_count_stamp)-1], miio_net->monitor.offline_count_stamp[0]),
					miio_net->monitor.offline_timeout);
		}
	}

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	return triggered;
}

static void sta_monitor_offline_countout_reset(miio_net_t *miio_net)
{
	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	miio_net->monitor.offline_count_time = 0;
	for(int i=0; i < NELEMENTS(miio_net->monitor.offline_count_stamp); i++){
		miio_net->monitor.offline_count_stamp[i] = miio_net->monitor.offline_count;
	}

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);
}

static bool reboot_monitor_timeout(miio_net_t *miio_net, uint32_t monitor_interval_s)
{
	bool triggered = false;

#if	MIIO_MONITOR_REBOOT_ENABLE

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	miio_net->monitor.reboot_time += monitor_interval_s;

	if(miio_net->monitor.reboot_time >= MIIO_MONITOR_REBOOT_TIMEOUT_S){
		miio_net->monitor.reboot_time = 0;
		triggered = true;
		LOG_WARN_TAG(MIIO_LOG_TAG, "reboot trigger pulled...");
	}
	else{
		triggered = false;
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "reboot trigger will been pulled in %us",
				MIIO_MONITOR_REBOOT_TIMEOUT_S - miio_net->monitor.reboot_time);
	}

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

#endif

	return triggered;
}

static void reboot_monitor_reset(miio_net_t *miio_net)
{
#if	MIIO_MONITOR_REBOOT_ENABLE

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	miio_net->monitor.reboot_time = 0;

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

#endif
}

static bool ntp_monitor_timeout(miio_net_t *miio_net, uint32_t monitor_interval_s)
{
	bool timeout = false;

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	miio_net->monitor.ntp_time += monitor_interval_s;
	if(miio_net->monitor.ntp_time >= MIIO_MONITOR_NTP_INTEVAL_S){
		miio_net->monitor.ntp_time = 0;
		timeout = true;
	}

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	return timeout;
}

void ntp_monitor_ack(int result, uint32_t utc, void *ctx)
{
	miio_net_t *miio_net = ctx;
	miio_net->monitor.ntp_ack_cb = true;
	if(MIIO_OK == result){
		miio_net->monitor.ntp_successed = true;
		if(utc){
			arch_os_utc_set(utc);
		}
	}
	else{
		miio_net->monitor.ntp_successed = false;
		LOG_WARN_TAG(MIIO_LOG_TAG, "wan may be disconnected!");
	}
}

void ntp_monitor_reset(miio_net_t *miio_net)
{
	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	miio_net->monitor.ntp_time = 0;

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	ntp_monitor_ack(MIIO_OK, 0, miio_net);
	miio_net->monitor.ntp_ack_cb = false;
}

static void crash_monitor_report_once(miio_net_t *miio_net)
{
	static bool crash_checked = false;

	if(false == crash_checked){
		crash_checked = true;
		if(arch_crash_exist()){
			arch_crash_delete();
			{//crash report
				uint32_t method_id = miio_get_rpc_id(miio_net->miio_handle);
				size_t js_size = miio_get_rpc_max_size(miio_net->miio_handle);
				char *js = malloc(js_size);
				if(js){
					jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, js_size);
					jsmi_compose_start(&jsmi_composer);
					jsmi_set_object_begin(&jsmi_composer);
						jsmi_set_key_value_uint(&jsmi_composer, "id", method_id);
						jsmi_set_key_value_str(&jsmi_composer, "method", "_otc.crash", sizeof("_otc.crash")-1);
						jsmi_set_key_object_begin(&jsmi_composer, "params");
						{//model
							char model[32] = {0};
							miio_get_model(miio_net->miio_handle, model, sizeof(model));
							jsmi_set_key_value_str(&jsmi_composer, "model", model, strnlen(model, sizeof(model)));
						}
						{//fw version
#if MIIO_APP_VERSION_NUMBER
							jsmi_set_key_value_str(&jsmi_composer, "fw_ver", miio_app_version(MIIO_APP_VERSION_NUMBER), 0);
#else
							jsmi_set_key_value_str(&jsmi_composer, "fw_ver", miio_instance_version(), 0);
#endif
						}
						jsmi_set_key_object_end(&jsmi_composer);
					jsmi_set_object_end(&jsmi_composer);
					if(MIIO_OK == jsmi_compose_finish(&jsmi_composer, NULL, &js_size)){
						miio_rpc_delegate_context_t rpc_context;
						miio_rpc_context_init(&rpc_context);
						miio_rpc_context_config_delegate_arg(&rpc_context, method_id, MIIO_DELEGATE_JSON, js, js_size);
						miio_rpc_context_config_delegate_ack(&rpc_context, NULL, NULL);
						if(MIIO_OK != miio_set_up_rpc_delegate(miio_net->miio_handle, &rpc_context)){
							LOG_ERROR_TAG(MIIO_LOG_TAG, "_otc.crash failed.");
						}
					}
					else{
						LOG_ERROR_TAG(MIIO_LOG_TAG, "memory not enough for _otc.crash.");
					}

					free(js);
				}
				else{
					LOG_ERROR_TAG(MIIO_LOG_TAG, "no memory for _otc.crash.");
				}
			}
		}
	}
}

static bool net_try_switch_to_provisionner(miio_net_t *miio_net)
{
	int ret = false;

	/* bug fix, if module work in factory mode, disable smart config */
	if (!miio_net->smart_config_enable  || miio_net->factory_status) {
		return false;
	}

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	if (0 == strcmp(miio_net->ssid, MIIO_NET_PROVISIONER_SSID)
			&& 0 == strcmp(miio_net->password, MIIO_NET_PROVISIONER_PASSWD)) {
		arch_psm_get_str("network", "ssid", miio_net->ssid, sizeof(miio_net->ssid));
		arch_psm_get_str("network", "password", miio_net->password, sizeof(miio_net->password));

	} else {
		strncpy(miio_net->ssid, MIIO_NET_PROVISIONER_SSID, sizeof(miio_net->ssid));
		strncpy(miio_net->password, MIIO_NET_PROVISIONER_PASSWD, sizeof(miio_net->password));
		ret = true;
	}

	miio_net->wifi_connect_ctx.scan_time = MIIO_NET_WIFI_SCAN_TIME_MS_MIN;

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	if (ret) {
		LOG_INFO_TAG(MIIO_LOG_TAG, "try to scan provisioner ssid.");
	} else {
		LOG_INFO_TAG(MIIO_LOG_TAG, "try to scan master ssid.");
	}

	return ret;
}

static arch_os_function_return_t task_monitor(void *arg)
{
	miio_net_t *miio_net = (miio_net_t *)arg;
	int ret = MIIO_OK;

	int smart_config_retry = MIIO_NET_SMART_CONFIG_RETRY;
	bool is_in_smart_config = false;
#if MIIO_PROV_STATISTIC
	static uint8_t prov_stat_scan_cnt = 0;
	miio_prov_statistic_t *prov_statistic = miio_net->prov_statistic;
#endif

	miio_offline_init();

	while(1){
		miio_wifi_event_t event;
		if(MIIO_OK == arch_os_queue_recv(miio_net->monitor.mq, &event, MIIO_MONITOR_INTERVAL_S*1000)){
			switch(event.type){
			case MIIO_WIFI_EVENT_CONFIG_ROUTER:

				break;
			case MIIO_WIFI_EVENT_SCAN_DONE:

				LOG_INFO_TAG(MIIO_LOG_TAG, "Wifi scan done");

				if(MIIO_OK == miio_wifi_scan_finish(miio_net)){

					do {
						ret = miio_wifi_connect(miio_net);
#if MIIO_PROV_STATISTIC
						if(prov_statistic->enable && (prov_statistic->prov_stage >= PROV_STAGE_CONFIG_ROUTER) && (MIIO_ERROR_NOTFOUND == ret)){						
							/*not found ap 3 times, record error code*/
							if(strcmp(miio_net->ssid, MIIO_NET_PROVISIONER_SSID) && (++prov_stat_scan_cnt >= MAX_PROV_SCAN_TIMES)){
								prov_stat_scan_cnt = 0;
								prov_statistic->prov_stat_cur.prov_error_code = PROV_ERROR_AP_NOT_FOUND;
								prov_stat_error_code_process();
							}
						}	 
#endif

						if(MIIO_OK == ret) {
							break;
						} else if(!miio_wifi_if_has_next(miio_net)) {
							/* ssid may changed */
							smart_config_retry--;
							if (smart_config_retry == 0) {
								smart_config_retry = MIIO_NET_SMART_CONFIG_RETRY;
								net_try_switch_to_provisionner(miio_net);
							}

							if(MIIO_OK != miio_wifi_scan_start(miio_net)) {
								LOG_ERROR_TAG(MIIO_LOG_TAG, "wifi scan start failed!");
#if MIIO_MONITOR_REBOOT_ENABLE
								miio_reboot(miio_net->miio_handle, "wifi scan failed", MIIO_RPC_TIMEOUT_MS);
#endif
							}
							break;
						} else { /* try next ssid in ap list */
							continue;
						}
					} while (true);
				}

				break;
			case MIIO_WIFI_EVENT_STA_START:

				LOG_INFO_TAG(MIIO_LOG_TAG, "Wifi station started");

				if(MIIO_OK != miio_wifi_scan_start(miio_net)){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "wifi scan start failed!");
#if	MIIO_MONITOR_REBOOT_ENABLE
					miio_reboot(miio_net->miio_handle, "wifi scan failed", MIIO_RPC_TIMEOUT_MS);
#endif
				}

				break;
			case MIIO_WIFI_EVENT_STA_STOP:

				LOG_INFO_TAG(MIIO_LOG_TAG, "Wifi station stoped");

				is_in_smart_config = false;

				break;
			case MIIO_WIFI_EVENT_STA_CONNECTED:

				LOG_INFO_TAG(MIIO_LOG_TAG, "Wifi station connected");

				miio_net->wifi_connect_ctx.sta_status = MIIO_WIFI_STA_CONNECTED;

				break;
			case MIIO_WIFI_EVENT_STA_DISCONNECTED:

				LOG_INFO_TAG(MIIO_LOG_TAG, "Wifi station disconnected");

				miio_wifi_disconnect(miio_net);
				do {
					int ret = MIIO_ERROR;
					ret = miio_wifi_connect(miio_net);

					if(MIIO_OK == ret) {
						break;
					} else if (!miio_wifi_if_has_next(miio_net)) {
						LOG_INFO_TAG(MIIO_LOG_TAG,
								"sta disconnect, then new connect faild: %d, and no other ap of same ssid found", ret);

						/* passwd may changed */
						smart_config_retry--;
						if (smart_config_retry == 0) {
							smart_config_retry = MIIO_NET_SMART_CONFIG_RETRY;
							net_try_switch_to_provisionner(miio_net);
						}

						if(MIIO_OK != miio_wifi_scan_start(miio_net)) {
							LOG_ERROR_TAG(MIIO_LOG_TAG, "wifi scan start failed!");
#if	MIIO_MONITOR_REBOOT_ENABLE
							miio_reboot(miio_net->miio_handle, "wifi scan failed", MIIO_RPC_TIMEOUT_MS);
#endif
						}
						break;
					}

					/* else: wifi has another bssid, do-while will go back and connect other bssid */
					LOG_INFO_TAG(MIIO_LOG_TAG, "new connect faild %d, still another ap of same ssid exists", ret);
				} while(true);

				break;
			case MIIO_WIFI_EVENT_STA_GOT_IP:

				LOG_INFO_TAG(MIIO_LOG_TAG, "Wifi station got ip");
				miio_net->wifi_connect_ctx.sta_status = MIIO_WIFI_STA_GOT_IP;

				if (0 == strcmp(miio_net->ssid, MIIO_NET_PROVISIONER_SSID)
						&& 0 == strcmp(miio_net->password, MIIO_NET_PROVISIONER_PASSWD)) {
					is_in_smart_config = true;
				} else {
					is_in_smart_config = false;
					smart_config_retry = MIIO_NET_SMART_CONFIG_RETRY;
				}

				if(miio_net->factory_status || is_in_smart_config){
					miio_set_mode(miio_net->miio_handle, MIIO_MODE_LOCAL);
				}
				else{
					miio_set_mode(miio_net->miio_handle, MIIO_MODE_CLOUD);
				}

				miio_net_state_change(miio_net, MIIO_NET_LOCAL, MIIO_NET_ERROR_NONE);

				break;
			case MIIO_WIFI_EVENT_STA_LOST_IP:

				LOG_INFO_TAG(MIIO_LOG_TAG, "Wifi station lost ip");

				miio_set_event_str(miio_net->miio_handle, "miio_monitor", "dhcp failed", sizeof("dhcp failed")-1);

				miio_net_stop();
				if(MIIO_OK != miio_net_start_async(MIIO_MONITOR_STA_CONNECT_INTEVAL_S*1000)){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "wifi start failed!");
#if	MIIO_MONITOR_REBOOT_ENABLE
					miio_reboot(miio_net->miio_handle, "wifi start failed", MIIO_RPC_TIMEOUT_MS);
#endif
				}

				break;
			default:
				break;
			}
		}

		{
			uint32_t time_elapsed = arch_os_ms_elapsed(miio_net->monitor.refresh_ms)/1000;

			if(time_elapsed >= MIIO_MONITOR_INTERVAL_S)
			{
				miio_net->monitor.refresh_ms = arch_os_ms_now();

				miio_net->monitor.work_time += time_elapsed;

				{//time and heap monitor
					uint32_t heap_size = arch_os_get_free_heap_size();
					if(info_monitor_timeout(miio_net, time_elapsed)){
						miio_net->monitor.heap_size = heap_size;
						LOG_INFO_TAG("miio_monitor", "work time = %us, memory left = %u", miio_net->monitor.work_time, miio_net->monitor.heap_size);
					}
					else if(heap_size != miio_net->monitor.heap_size){
						miio_net->monitor.heap_size = heap_size;
						LOG_INFO_TAG("miio_monitor", "work time = %us, memory left = %u", miio_net->monitor.work_time, miio_net->monitor.heap_size);
					}
				}

				/* smart config timeout */
				if (is_in_smart_config) {
					if(smart_config_monitor_timeout(miio_net, time_elapsed)) {
						is_in_smart_config = false;
						miio_wifi_disconnect(miio_net);
					}
				}

				if(miio_net->monitor.net_status_check){//miio status monitor
					wifi_mode_t wifi_mode = WIFI_MODE_STA;
					esp_wifi_get_mode(&wifi_mode);

					if(wifi_mode == WIFI_MODE_AP || wifi_mode == WIFI_MODE_APSTA){
						if(ap_monitor_timeout(miio_net, time_elapsed)){
							miio_net_stop();
							miio_net_state_change(miio_net, MIIO_NET_UNPROV, MIIO_NET_ERROR_NONE);
						}
					}
					else if(wifi_mode == WIFI_MODE_STA){
						miio_mode_t miio_mode = miio_get_mode(miio_net->miio_handle);
						if(MIIO_MODE_CLOUD == miio_mode){
							do{
								if(miio_is_online(miio_net->miio_handle)){
#if MIIO_STATASTIC_ENABLE
									miio_net->monitor.miio_times.cloud += time_elapsed;
#endif
									reboot_monitor_reset(miio_net);
									sta_monitor_offline_timeout_reset(miio_net);
									crash_monitor_report_once(miio_net);
									ntp_monitor_reset(miio_net);
								}
								else{
									miio_offline_record(miio_net, NET_STATUS_UNKOWN, 0);

									if(ntp_monitor_timeout(miio_net, time_elapsed)){
										miio_ntp(miio_net->miio_handle, ntp_monitor_ack, miio_net, MAX(MIIO_MONITOR_NTP_INTEVAL_S/2, MIIO_MONITOR_INTERVAL_S)*1000);
									}
#if MIIO_STATASTIC_ENABLE
									if(miio_net->monitor.ntp_successed){
										miio_net->monitor.miio_times.local += time_elapsed;
									}
#endif
									if(reboot_monitor_timeout(miio_net, time_elapsed)){
										miio_reboot(miio_net->miio_handle, "miio_monitor", MIIO_RPC_TIMEOUT_MS);
										break;
									}
									else if(sta_monitor_offline_timeout(miio_net, time_elapsed)){
										if(MIIO_OK != miio_wifi_try_reconnect(miio_net)){
											miio_net_stop();
											if(MIIO_OK != miio_net_start_async(MIIO_MONITOR_STA_CONNECT_INTEVAL_S*1000)){
												LOG_ERROR_TAG(MIIO_LOG_TAG, "wifi start failed!");
#if	MIIO_MONITOR_REBOOT_ENABLE
												miio_reboot(miio_net->miio_handle, "wifi start failed", MIIO_RPC_TIMEOUT_MS);
#endif
											}
										}
										break;
									}
								}

								if(sta_monitor_offline_countout(miio_net, time_elapsed)){
									if(MIIO_OK != miio_wifi_try_reconnect(miio_net)){
										miio_net_stop();
										if(MIIO_OK != miio_net_start_async(MIIO_MONITOR_STA_CONNECT_INTEVAL_S*1000)){
											LOG_ERROR_TAG(MIIO_LOG_TAG, "wifi start failed!");
#if	MIIO_MONITOR_REBOOT_ENABLE
											miio_reboot(miio_net->miio_handle, "wifi start failed", MIIO_RPC_TIMEOUT_MS);
#endif
										}
									}
									break;
								}
							}while(0);

						}
						else if(MIIO_MODE_LOCAL == miio_mode){
							if(ntp_monitor_timeout(miio_net, time_elapsed)){
								miio_ntp(miio_net->miio_handle, ntp_monitor_ack, miio_net, MAX(MIIO_MONITOR_NTP_INTEVAL_S/2, MIIO_MONITOR_INTERVAL_S)*1000);
							}
							reboot_monitor_reset(miio_net);
							sta_monitor_offline_timeout_reset(miio_net);
							sta_monitor_offline_countout_reset(miio_net);
						}
						else{
							/* sta disconnect or lost ip will set mode to idle */
							miio_offline_record(miio_net, cloud2idle_reason, sta_disconnect_reason);
#if MIIO_STATASTIC_ENABLE
							miio_net->monitor.miio_times.idle += time_elapsed;
#endif
							sta_monitor_offline_countout_reset(miio_net);

							if(reboot_monitor_timeout(miio_net, time_elapsed)){
								miio_reboot(miio_net->miio_handle, "miio_monitor", MIIO_RPC_TIMEOUT_MS);
							}
							else if(sta_monitor_offline_timeout(miio_net, time_elapsed)){
								if(MIIO_OK != miio_wifi_try_reconnect(miio_net)){
									miio_net_stop();
									if(MIIO_OK != miio_net_start_async(MIIO_MONITOR_STA_CONNECT_INTEVAL_S*1000)){
										LOG_ERROR_TAG(MIIO_LOG_TAG, "wifi start failed!");
#if	MIIO_MONITOR_REBOOT_ENABLE
										miio_reboot(miio_net->miio_handle, "wifi start failed", MIIO_RPC_TIMEOUT_MS);
#endif
									}
								}
							}
						}

						if(miio_is_updating(miio_net->miio_handle)) {
							miio_net_state_change(miio_net, MIIO_NET_UPDATING, MIIO_NET_ERROR_NONE);
						}
						else if(miio_is_online(miio_net->miio_handle)){
							miio_net_state_change(miio_net, MIIO_NET_CLOUD, MIIO_NET_ERROR_NONE);
						}
						else if(!arch_net_is_offline()){
							miio_net_state_change(miio_net, MIIO_NET_LOCAL, MIIO_NET_ERROR_NONE);
						}
						else{
							miio_net_state_change(miio_net, MIIO_NET_DISCONNECTED, MIIO_NET_ERROR_NONE);
						}
					}
				}
				else{
#if MIIO_STATASTIC_ENABLE
					miio_net->monitor.miio_times.idle += time_elapsed;
#endif
				}
			}
		}
	}

	return ARCH_OS_FUNCTION_RETURN(0);
}

extern void light_remote_wifiap(uint8_t state);
int miio_net_init(miio_handle_t miio_handle,
		const miio_net_state_callback_t callbacks[], size_t callbacks_count)
{
	miio_net_t *miio_net = &s_miio_net;

	memset(miio_net, 0, sizeof(miio_net_t));

	miio_net->miio_handle = miio_handle;

	//get mac
	arch_get_mac(miio_net->mac);
	
	miio_net->smart_config_enable = false;
	miio_net->auto_provision_enable = false;
	//get ap_ssid
	if(arch_psm_get_str("network", "ssid", miio_net->ssid, sizeof(miio_net->ssid)) > 0){
		//already provisioned
		miio_net->provision_status = true;
		//get ap_password, optional
		if(arch_psm_get_str("network", "password", miio_net->password, sizeof(miio_net->password)) <= 0){
			miio_net->password[0] = '\0';
		}
		miio_net->smart_config_enable = true;
		light_remote_wifiap(1);
	}
	else{
		//not provisioned
		miio_net->provision_status = false;

		miio_net->auto_provision_enable = true;
		light_remote_wifiap(0);
	}

	//LOG_DEBUG_TAG(MIIO_LOG_TAG, "ssid=%s, password=%s", miio_net->ssid, miio_net->password);

	//get wifi channel info
	{
		char cc[sizeof(miio_net->wifi_channel_info.cc)] = {0};
		if(miio_net_get_wifi_channel(cc, sizeof(cc), -1) <= 0){
			strncpy(cc, "CN", sizeof(cc)-1);
			cc[sizeof(cc)-1] = '\0';
		}
		miio_wifi_channel_info_get(cc, &miio_net->wifi_channel_info);


		if( 0 != memcmp(cc, "CN", sizeof(cc)-1) ){

			miio_net->smart_config_enable = false;

			miio_net->auto_provision_enable = false;
		}

	}
#if MIIO_PROV_STATISTIC
	miio_prov_statistic_init(miio_net);
#endif
	//init wifi scan time
	miio_net->wifi_connect_ctx.scan_time = MIIO_NET_WIFI_SCAN_TIME_MS_MIN;
	//init wifi ap info list
	INIT_LIST_HEAD(&(miio_net->wifi_connect_ctx.ap_info_list));

	//init tcp adapter
	tcpip_adapter_init();

	//init wifi
	esp_event_loop_init(wifi_cb_event_handler, miio_net);//register wifi callback.
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&cfg);
	esp_wifi_set_storage(WIFI_STORAGE_RAM);

	//init miio net event
	miio_net->callbacks = callbacks;
	miio_net->callbacks_count = callbacks_count;

	//regsiter miio hook
	miio_hooks_register(miio_net->miio_handle, &s_miio_net_hooks);

	//tob init
	{
		memset(miio_net->tob.bind_key, 0, sizeof(miio_net->tob.bind_key));
		if(arch_psm_get_value("ot_config", "disable_local_restore", &miio_net->tob.disable_local_restore, sizeof(miio_net->tob.disable_local_restore)) <= 0){
			miio_net->tob.disable_local_restore = 0;
		}
	}

	if(MIIO_OK != arch_os_mutex_create(&(miio_net->net_mutex))){
		LOG_ERROR_TAG(MIIO_LOG_TAG, "net mutex err");
	}

	miio_net->monitor.net_status_check = false;
	do{
		if(MIIO_OK != arch_os_queue_create(&miio_net->monitor.mq, 4, sizeof(miio_wifi_event_t))){
			LOG_WARN_TAG(MIIO_LOG_TAG, "monitor queue create failed");
			break;
		}

                /* expand task size from 2048 to 3072 to avoid crash */
		if(MIIO_OK != arch_os_thread_create(&miio_net->monitor.thread, "netMonitorTask", task_monitor, 3072, miio_net, ARCH_OS_PRIORITY_DEFAULT)) {
			arch_os_queue_delete(miio_net->monitor.mq);
			miio_net->monitor.mq = NULL;
			LOG_WARN_TAG(MIIO_LOG_TAG, "monitor thread create failed");
			break;
		}
	}while(0);

#if MIIO_NET_INDICATOR_ENABLE
	if(MIIO_OK != miio_net_indicator_init()){
		LOG_WARN_TAG(MIIO_LOG_TAG, "net indicator init failed");
	}
#endif
	miio_net->wifi_scan_start = miio_wifi_scan_start;
	miio_net->wifi_scan_finish = miio_wifi_scan_finish;
	miio_net->wifi_connect = miio_wifi_connect;
	miio_net->wifi_disconnect = miio_wifi_disconnect;
	miio_net->wifi_if_has_next = miio_wifi_if_has_next;

	miio_net->auto_provision = NULL;


	if( miio_net->auto_provision_enable ){
		memset(miio_net->ssid, 0, sizeof( miio_net->ssid ) );
		memset(miio_net->password, 0, sizeof(miio_net->password));
		strncpy(miio_net->ssid, MIIO_NET_PROVISIONER_SSID, sizeof(miio_net->ssid));
		strncpy(miio_net->password, MIIO_NET_PROVISIONER_PASSWD, sizeof(miio_net->password));
		miio_net_auto_provision_init(&miio_net->auto_provision, miio_net);
	}
	
	miio_net->net_started = false;
	LOG_DEBUG_TAG(MIIO_LOG_TAG, "provision_status[%d], smart_config[%d], auto_provision[%d]", 
					miio_net->provision_status,
					miio_net->smart_config_enable,
					 miio_net->auto_provision_enable);

	return MIIO_OK;
}

int miio_net_switch_smart_config_onoff_in_migration(void)
{
	miio_net_t *miio_net = &s_miio_net;

	char cc[sizeof(miio_net->wifi_channel_info.cc)] = {0};
	if(miio_net_get_wifi_channel(cc, sizeof(cc), -1) <= 0){
		strncpy(cc, "CN", sizeof(cc) - 1);
		cc[sizeof(cc) - 1] = '\0';
	}

	miio_wifi_channel_info_get(cc, &miio_net->wifi_channel_info);


	if( 0 != memcmp(cc, "CN", sizeof(cc) - 1) ){
		miio_net->smart_config_enable = false;
		LOG_INFO_TAG(MIIO_LOG_TAG, "smart config disabled");
		miio_net->auto_provision_enable = false;
		LOG_INFO_TAG(MIIO_LOG_TAG, "auto provision disabled");
	}

	return MIIO_OK;
}

int miio_net_stop(void)
{
	LOG_DEBUG_TAG(MIIO_LOG_TAG, "wifi stopping...");

	miio_net_t *miio_net = &s_miio_net;

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	miio_net->monitor.net_status_check = false;
	miio_net->monitor.ap_time = 0;
	miio_net->monitor.offline_time = 0;
	miio_net->monitor.offline_timeout = MIIO_MONITOR_OFFLINE_TIMEOUT_S;
	miio_net->monitor.offline_count_time = 0;
	miio_net->monitor.offline_count = 0;
	miio_net->monitor.smart_config_time = 0;
	for(int i=0; i < NELEMENTS(miio_net->monitor.offline_count_stamp); i++){
		miio_net->monitor.offline_count_stamp[i] = miio_net->monitor.offline_count;
	}
	miio_net->monitor.offline_count_out = MIIO_NET_MONITOR_OFFLINE_COUNT_MAX(miio_net->monitor.offline_timeout*1000);
	miio_net->monitor.ntp_successed = true;
	miio_net->monitor.ntp_time = 0;

	miio_net->net_started = false;
	miio_net->wifi_connect_ctx.sta_status = MIIO_WIFI_STA_IDLE;

	int ret = esp_wifi_stop();

	esp_wifi_set_mode(WIFI_MODE_NULL);

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	return (ret == ESP_OK) ? MIIO_OK : MIIO_ERROR;
}

int miio_net_start(void)
{
#if MIBLE_ENABLE
#define MIIO_NET_NAME_FORMAT_STR		"%s_mibt%02X%02X"
#else
#define MIIO_NET_NAME_FORMAT_STR		"%s_miap%02X%02X"
#endif

	LOG_DEBUG_TAG(MIIO_LOG_TAG, "start");

	miio_net_t *miio_net = &s_miio_net;

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	if(miio_net->net_started){
		LOG_WARN_TAG(MIIO_LOG_TAG, "net alreay started");
		goto safe_exit;
	}

	//init host_name
	{
		char model[MIIO_MODEL_SIZE_MAX] = {0};
		miio_get_model(miio_net->miio_handle, model, sizeof(model));
		snprintf(miio_net->host_name, sizeof(miio_net->host_name), MIIO_NET_NAME_FORMAT_STR, model, miio_net->mac[4], miio_net->mac[5]);
		for(char *c = miio_net->host_name; *c; c++){
			//replace '.' to '-'
			if ('.' == *c) {
				*c = '-';
			}
		}
	}

	/* Check if the status is provisioned */
	if (miio_net->provision_status){
		if((memcmp(miio_net->ssid,"miio_default",12) == 0) && (memcmp(miio_net->ssid,"miio_default",12) == 0))
		{
			miio_net->factory_status = true;
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "factory_status");
		}
		/* Already provisioned, start STA mode */
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "station mode");

		if(miio_net->factory_status)
			miio_enable_provision(miio_net->miio_handle);
		else
			miio_disable_provision(miio_net->miio_handle);

		if( miio_net->auto_provision_enable && miio_net->auto_provision){
			miio_net_auto_provision_deinit(miio_net->auto_provision);
			miio_net->auto_provision = NULL;
		}

		/* if CN, smartconfig should enable */
		{
			char cc[sizeof(miio_net->wifi_channel_info.cc)] = {0};
			int cc_len = arch_psm_get_str("network", "wifi_country_1", cc, sizeof(cc));
			if(cc_len > 0){
				LOG_DEBUG_TAG(MIIO_LOG_TAG, "cc[1] = %s", cc);
			}
			else{
				cc_len = arch_psm_get_str("network", "wifi_country", cc, sizeof(cc));
				if(cc_len > 0){
					LOG_DEBUG_TAG(MIIO_LOG_TAG, "cc[0] = %s", cc);
				}
				else{
					strncpy(cc, "CN", sizeof(cc)-1);
					cc[sizeof(cc)-1] = '\0';
				}
			}

			miio_wifi_channel_info_get(cc, &miio_net->wifi_channel_info);
			if( 0 != memcmp(cc, "CN", sizeof(cc)-1) ){
				miio_net->smart_config_enable = false;
			}
			else{
				miio_net->smart_config_enable = true;
			}
		}

		if(ESP_OK != esp_wifi_set_mode(WIFI_MODE_STA))
			goto error_exit;

		{
			wifi_country_t country;
			memset(&country, 0, sizeof(wifi_country_t));
			strncpy(country.cc, miio_net->wifi_channel_info.cc, sizeof(country.cc)-1);
			country.cc[sizeof(country.cc)-1] = '\0';
			country.schan = miio_net->wifi_channel_info.schan;
			country.nchan = miio_net->wifi_channel_info.nchan;
			country.max_tx_power = miio_net->wifi_channel_info.max_tx_dbm;
			country.policy = WIFI_COUNTRY_POLICY_MANUAL;
			if(ESP_OK != esp_wifi_set_country(&country))
				goto error_exit;
		}

		if(ESP_OK != esp_wifi_set_ps(WIFI_PS_MAX_MODEM)){
			LOG_WARN_TAG(MIIO_LOG_TAG, "sta set ps failed");
		}

		if(ESP_OK != esp_wifi_set_mac(WIFI_IF_STA, miio_net->mac))
			goto error_exit;
	}
	else{
		/* Not provisioned, should start AP mode */
		miio_net->factory_status = false;

		//enable provision
		miio_enable_provision(miio_net->miio_handle);

		if(miio_net->auto_provision_enable){
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "Provision APSTA Mode");
#if MIIO_PROV_STATISTIC
			miio_net->prov_statistic->prov_stat_cur.prov_type = PROV_TYPE_AUTO;
#endif
			if(ESP_OK != esp_wifi_set_mode(WIFI_MODE_APSTA)){
				goto error_exit;
			}

			if(ESP_OK != esp_wifi_set_ps(WIFI_PS_MAX_MODEM)){
				LOG_WARN_TAG(MIIO_LOG_TAG, "sta set ps failed");
			}

			if(ESP_OK != esp_wifi_set_mac(WIFI_IF_STA, miio_net->mac)){
				goto error_exit;
			}
			
		}else{
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "Provision AP Mode");
#if MIIO_PROV_STATISTIC
			/*set prov_type to PROV_TYPE_AP, because normal or safe mode is unknown now*/
			miio_net->prov_statistic->prov_stat_cur.prov_type = PROV_TYPE_AP;
#endif

			if(ESP_OK != esp_wifi_set_mode(WIFI_MODE_AP))
				goto error_exit;
		}

		{
			wifi_country_t country;
			memset(&country, 0, sizeof(wifi_country_t));
			strncpy(country.cc, miio_net->wifi_channel_info.cc, sizeof(country.cc)-1);
			country.cc[sizeof(country.cc)-1] = '\0';
			country.schan = miio_net->wifi_channel_info.schan;
			country.nchan = miio_net->wifi_channel_info.nchan;
			country.max_tx_power = miio_net->wifi_channel_info.max_tx_dbm;
			country.policy = WIFI_COUNTRY_POLICY_MANUAL;
			if(ESP_OK != esp_wifi_set_country(&country))
				goto error_exit;
		}
		{
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "AP dhcp 10.10.1.xxx");
			tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
			tcpip_adapter_ip_info_t ipInfo;
			IP4_ADDR(&ipInfo.ip, 10,10,1,1);
			IP4_ADDR(&ipInfo.gw, 10,10,1,1);
			IP4_ADDR(&ipInfo.netmask, 255,255,255,0);
			tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &ipInfo);
			tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP);
		}

		{
			wifi_config_t config;
			memset(&config, 0, sizeof(wifi_config_t));
			strncpy((char *)config.ap.ssid, miio_net->host_name, sizeof(config.ap.ssid));
			memset(config.ap.password, '\0', sizeof(config.ap.password));
			config.ap.authmode = WIFI_AUTH_OPEN;//AUTH_WPA_WPA2_PSK;
			config.ap.ssid_len = strnlen((char *)config.ap.ssid, sizeof(config.ap.ssid));// or its actual length
			config.ap.channel = 6;
			config.ap.max_connection = 2; // how many stations can connect to ESP32 softAP at most.
			config.ap.beacon_interval = 100;
			if(ESP_OK != esp_wifi_set_config(WIFI_IF_AP, &config))
				goto error_exit;
			
		}
			
		{
			uint8_t mac[6];
			memcpy(mac, miio_net->mac, 6);
			//涔愰懌esp32骞冲彴璁剧疆AP妯″紡鍜孲TA妯″紡鐨凪AC涓嶈兘鐩稿悓锛屼笖绗竴涓瓧鑺傜殑鏈�浣庝綅锛坆it0锛変笉鑳戒负1銆�
			if(mac[0]== 0xFE)
				mac[0] = 0x00;
			else if((mac[0] & 0x01))
				mac[0] = mac[0] & 0xFE;
			else
				mac[0] = mac[0] + 0x02;

			if(ESP_OK != esp_wifi_set_mac(WIFI_IF_AP, mac))//first bit of first byte can't be 1.
				goto error_exit;
		}
	}

	if(ESP_OK != esp_wifi_start())
		goto error_exit;

	/*start monitor*/
	miio_net->monitor.net_status_check = true;
	miio_net->monitor.ap_time = 0;
	miio_net->monitor.offline_time = 0;
	miio_net->monitor.offline_timeout = MIIO_MONITOR_OFFLINE_TIMEOUT_S;
	miio_net->monitor.offline_count_time = 0;
	miio_net->monitor.offline_count = 0;
	miio_net->monitor.smart_config_time = 0;
	for(int i=0; i < NELEMENTS(miio_net->monitor.offline_count_stamp); i++){
		miio_net->monitor.offline_count_stamp[i] = miio_net->monitor.offline_count;
	}
	miio_net->monitor.offline_count_out = MIIO_NET_MONITOR_OFFLINE_COUNT_MAX(miio_net->monitor.offline_timeout*1000);
	miio_net->monitor.ntp_successed = true;
	miio_net->monitor.ntp_time = 0;
	miio_net->net_started = true;

safe_exit:

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	return MIIO_OK;

error_exit:

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	LOG_ERROR_TAG(MIIO_LOG_TAG, "start failed");

#if MIIO_MONITOR_REBOOT_ENABLE
	miio_reboot(miio_net->miio_handle, "net start failed", MIIO_RPC_TIMEOUT_MS);
#endif

	return MIIO_ERROR;
}

static void net_start_async(void *arg)
{
	miio_net_start();
}

int miio_net_start_async(int delay_ms)
{
	LOG_DEBUG_TAG(MIIO_LOG_TAG, "start in %dms....", delay_ms);
	if(MIIO_OK != arch_os_async_call(net_start_async, NULL, delay_ms)){
		return miio_net_start();
	}
	return MIIO_OK;
}

int miio_net_restart(void)
{
	if(MIIO_OK != miio_net_stop()){
		LOG_WARN_TAG(MIIO_LOG_TAG, "stop failed");
	}
	arch_os_ms_sleep(1000);
	if(MIIO_OK != miio_net_start()){
		LOG_WARN_TAG(MIIO_LOG_TAG, "start failed");
	}

	return MIIO_OK;
}

static void net_restart_async(void *ctx)
{
	miio_net_restart();
}

int miio_net_restart_async(int delay_ms)
{
	LOG_DEBUG_TAG(MIIO_LOG_TAG, "restart in %dms....", delay_ms);
	if(MIIO_OK != arch_os_async_call(net_restart_async, NULL, delay_ms)){
		net_restart_async(NULL);
	}
	return MIIO_OK;
}

void miio_net_set_ap(const char* ssid, const char* password, bool in_ram)
{
	miio_net_t *miio_net = &s_miio_net;

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	if(in_ram){
		snprintf(miio_net->ssid, sizeof(miio_net->ssid), "%s", ssid);
		snprintf(miio_net->password, sizeof(miio_net->password), "%s", password);
	}
	else{
		do{
			int password_len;

			if(!password || !(password_len = strlen(password))){
				memset(miio_net->password, 0, sizeof(miio_net->password));
				arch_psm_erase_key("network", "password");
				break;
			}

			if( (strlen(miio_net->password) != password_len) || (0 != strncmp(password, miio_net->password, password_len)) ){
				snprintf(miio_net->password, sizeof(miio_net->password), "%s", password);
				arch_psm_set_str("network", "password", miio_net->password);
			}

		}while(0);

		do{
			int ssid_len;

			if(!ssid || !(ssid_len = strlen(ssid))){
				memset(miio_net->ssid, 0, sizeof(miio_net->ssid));
				arch_psm_erase_key("network", "ssid");
				break;
			}

			if( (strlen(miio_net->ssid) != ssid_len) || (0 != strncmp(ssid, miio_net->ssid, ssid_len)) ){
				snprintf(miio_net->ssid, sizeof(miio_net->ssid), "%s", ssid);
				arch_psm_set_str("network", "ssid", miio_net->ssid);
			}

		}while(0);
	}

	miio_net->provision_status = strnlen(miio_net->ssid, sizeof(miio_net->ssid)) ? true : false;

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	return;
}

void miio_net_set_wifi_channel(const char* cc, int priority)
{
	miio_net_t *miio_net = &s_miio_net;
	char cc_temp[sizeof(miio_net->wifi_channel_info.cc)] = {0};

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	if(NULL == cc || 0 == strnlen(cc, sizeof(miio_net->wifi_channel_info.cc))){
		switch(priority){
		case 0:
			arch_psm_erase_key("network", "wifi_country");
			break;
		case 1:
			arch_psm_erase_key("network", "wifi_country_1");
			break;
		default:
			break;
		}
	}
	else if(strnlen(cc, sizeof(miio_net->wifi_channel_info.cc)) < sizeof(miio_net->wifi_channel_info.cc)){
		switch(priority){
		case 0:
			if(arch_psm_get_str("network", "wifi_country", cc_temp, sizeof(cc_temp)) > 0){
				if(0 == strcmp(cc_temp, cc)){
					LOG_DEBUG_TAG(MIIO_LOG_TAG, "cc[0] = %s already.", cc);
					break;
				}
			}
			arch_psm_set_str("network", "wifi_country", cc);
			break;
		case 1:
			if(arch_psm_get_str("network", "wifi_country_1", cc_temp, sizeof(cc_temp)) > 0){
				if(0 == strcmp(cc_temp, cc)){
					LOG_DEBUG_TAG(MIIO_LOG_TAG, "cc[1] = %s already.", cc);
					break;
				}
			}
			arch_psm_set_str("network", "wifi_country_1", cc);
			break;
		default:
			break;
		}
	}

	if(arch_psm_get_str("network", "wifi_country_1", cc_temp, sizeof(cc_temp)) > 0){
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "cc[1] = %s", cc_temp);
		miio_wifi_channel_info_get(cc_temp, &miio_net->wifi_channel_info);
	}
	else if(arch_psm_get_str("network", "wifi_country", cc_temp, sizeof(cc_temp)) > 0){
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "cc[0] = %s", cc_temp);
		miio_wifi_channel_info_get(cc_temp, &miio_net->wifi_channel_info);
	}
	else{
		miio_wifi_channel_info_get("CN", &miio_net->wifi_channel_info);
	}

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);
}

int miio_net_get_wifi_channel(char *cc, size_t cc_size, int priority)
{
	miio_net_t *miio_net = &s_miio_net;

	if(NULL == cc || cc_size <= 0)
		return 0;

	int cc_len = 0;

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	switch(priority){
	case 0:
		cc_len = arch_psm_get_str("network", "wifi_country", cc, cc_size);
		if(cc_len > 0){
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "cc[0] = %s", cc);
		}
		break;
	case 1:
		cc_len = arch_psm_get_str("network", "wifi_country_1", cc, cc_size);
		if(cc_len > 0){
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "cc[1] = %s", cc);
		}
		break;
	default:
		cc_len = arch_psm_get_str("network", "wifi_country_1", cc, cc_size);
		if(cc_len > 0){
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "cc[1] = %s", cc);
			break;
		}
		cc_len = arch_psm_get_str("network", "wifi_country", cc, cc_size);
		if(cc_len > 0){
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "cc[0] = %s", cc);
			break;
		}
		break;
	}

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	if(cc_len < 0){
		cc_len = 0;
	}

	return cc_len;
}

int miio_net_get_disable_local_restore(void)
{
	miio_net_t *miio_net = &s_miio_net;

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	int disable_local_restore = s_miio_net.tob.disable_local_restore;

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	return disable_local_restore;
}

int miio_net_set_disable_local_restore(int disable_local_restore)
{
	miio_net_t *miio_net = &s_miio_net;

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	if(s_miio_net.tob.disable_local_restore != disable_local_restore){
		s_miio_net.tob.disable_local_restore = disable_local_restore;
		arch_psm_set_value("ot_config", "disable_local_restore", &s_miio_net.tob.disable_local_restore, sizeof(s_miio_net.tob.disable_local_restore));
	}

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	return MIIO_OK;
}

void miio_net_factory(void)
{
	miio_net_t *miio_net = &s_miio_net;

    miio_net->factory_status = true;

    miio_restore(miio_net->miio_handle, "factory");

    miio_net_set_ap("miio_default", "0x82562647", true);

    miio_net_restart_async(1000);
}

miio_net_state_t miio_net_get_state(void)
{
	miio_net_t *miio_net = &s_miio_net;

	if (!miio_net->provision_status){
		wifi_mode_t wifi_mode = WIFI_MODE_NULL;
		if(ESP_OK != esp_wifi_get_mode(&wifi_mode)){
			return MIIO_NET_UNPROV;
		}
		if(wifi_mode == WIFI_MODE_AP || wifi_mode == WIFI_MODE_APSTA){
			return MIIO_NET_UAP;
		}
		else{
			return MIIO_NET_UNPROV;
		}
	}
	else if(miio_is_updating(miio_net->miio_handle)) {
		return MIIO_NET_UPDATING;
	}
	else if(miio_is_online(miio_net->miio_handle)){
		return MIIO_NET_CLOUD;
	}
	else if(!arch_net_is_offline()){
		return MIIO_NET_LOCAL;
	}
	else{
		return MIIO_NET_DISCONNECTED;
	}

	return MIIO_NET_DISCONNECTED;
}

int miio_net_set_bindkey(const char *bindkey)
{
	miio_net_t *miio_net = &s_miio_net;

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	if(bindkey){
		strncpy(miio_net->tob.bind_key, bindkey, sizeof(miio_net->tob.bind_key)-1);
		miio_net->tob.bind_key[sizeof(miio_net->tob.bind_key)-1] = '\0';
	}
	else{
		miio_net->tob.bind_key[0] = '\0';
	}

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	return MIIO_OK;
}

#if MIIO_KEY_EXCHANGE_ENABLE
static int miio_net_restore_bind_key(void)
{
    do{
        arch_psm_erase_key("key_exchange", "bind_did");
        arch_psm_erase_key("key_exchange", "ot_key");
    }while(0);
            
    return MIIO_OK;
}

int miio_net_set_bindkey_index(miio_handle_t miio_handle, const char *bindkey_index)
{
    miio_net_t *miio_net = &s_miio_net;

    struct{
        char bind_key[65];
        char bind_index[65];
    }bind_info;

    memset(&bind_info, 0, sizeof(bind_info));

    if(miio_net->net_mutex)
        arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

    if(bindkey_index){
        strncpy(bind_info.bind_index, bindkey_index, sizeof(bind_info.bind_index)-1);
        bind_info.bind_index[sizeof(bind_info.bind_index)-1] = '\0';

        strncpy(bind_info.bind_key, miio_net->tob.bind_key, sizeof(bind_info.bind_key)-1);
        bind_info.bind_key[sizeof(bind_info.bind_key)-1] = '\0';

        miio_set_bind_index(miio_handle, &bind_info);
    }
    else{
        miio_set_bind_index(miio_handle, NULL);
    }

    if(miio_net->net_mutex)
        arch_os_mutex_put(miio_net->net_mutex);

	return MIIO_OK;
}

void miio_net_set_bind_key_ts(uint64_t utc)
{
    if(utc){
		arch_os_utc_set(utc);
	}
}
#endif

int miio_net_set_config_type(const char *config_type)
{
	miio_net_t *miio_net = &s_miio_net;

	if(miio_net->net_mutex)
		arch_os_mutex_get(miio_net->net_mutex, ARCH_OS_WAIT_FOREVER);

	if(config_type){
		strncpy(miio_net->config_type, config_type, sizeof(miio_net->config_type)-1);
		miio_net->config_type[sizeof(miio_net->config_type)-1] = '\0';
	}
	else{
		miio_net->config_type[0] = '\0';
	}

	if(miio_net->net_mutex)
		arch_os_mutex_put(miio_net->net_mutex);

	return MIIO_OK;
}

int miio_net_config_router(miio_handle_t miio_handle, const char* params, size_t params_len, const char *extents_param, size_t extents_param_size)
{
	char ssid[IEEEtypes_SSID_SIZE+1] = {0};
	char passwd[WLAN_PSK_MAX_LENGTH+1] = {0};
	bool extents_valid = (extents_param != NULL) && (extents_param_size > 0);
	

	jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(params, params_len);
	jsmi_parser_t jsmi_parser_extents = JSMI_PARSER_INIT(extents_param, extents_param_size);
	if( MIIO_OK != jsmi_parse_start(&jsmi_parser) ){
		return MIIO_ERROR_NOMEM;
	}
	if(extents_valid){
		if( MIIO_OK != jsmi_parse_start(&jsmi_parser_extents) ){
			// release jsmi_parser
			jsmi_parse_finish(&jsmi_parser);
			return MIIO_ERROR_NOMEM;
		}
	}
	// /ssid
	do{
		jsmi_tok_path_t path[] = {
			{
				.key = "ssid",
				.type = JSMN_STRING
			}
		};

		if(MIIO_OK == jsmi_get_value_str(&jsmi_parser, NULL, path, NELEMENTS(path), ssid, sizeof(ssid)) ){
			break;
		}
		
		if(extents_valid ? MIIO_OK == jsmi_get_value_str(&jsmi_parser_extents, NULL, path, NELEMENTS(path), ssid, sizeof(ssid)) : false ){
			break;
		}

		LOG_ERROR_TAG(MIIO_LOG_TAG, "\"ssid\" not found");
		goto jsmi_parse_exit;
	}while(0);

	// /password
	do{
		jsmi_tok_path_t path[] = {
			{
				.key = "passwd",
				.type = JSMN_STRING
			}
		};
		if(MIIO_OK == jsmi_get_value_str(&jsmi_parser, NULL, path, NELEMENTS(path), passwd, sizeof(passwd))){
			break;
		}
		if(extents_valid ? MIIO_OK == jsmi_get_value_str(&jsmi_parser_extents, NULL, path, NELEMENTS(path), passwd, sizeof(passwd)) : false){
			break;			
		}
		LOG_WARN_TAG(MIIO_LOG_TAG, "'passwd' not found");
	}while(0);

	// /uid
	do{
		uint64_t uid = 0;
		jsmi_tok_path_t path[] = {
			{
				.key = "uid",
				.type = JSMN_PRIMITIVE
			}
		};
		if( MIIO_OK == jsmi_get_value_u64(&jsmi_parser, NULL, path, NELEMENTS(path), &uid) ){
			miio_instance_set_uid(miio_handle, uid);
			break;
		}
		if(extents_valid ? MIIO_OK == jsmi_get_value_u64(&jsmi_parser_extents, NULL, path, NELEMENTS(path), &uid) : false){
			miio_instance_set_uid(miio_handle, uid);
			break;
		}
	}while(0);

	// /gmt_offset
	do{
		int gmt_offset = 0;
		jsmi_tok_path_t path[] = {
			{
				.key = "gmt_offset",
				.type = JSMN_PRIMITIVE
			}
		};
		if( MIIO_OK == jsmi_get_value_sint(&jsmi_parser, NULL, path, NELEMENTS(path), &gmt_offset) ){
			miio_instance_set_gmt_offset(miio_handle, gmt_offset);
			break;
		}
		if(extents_valid ? MIIO_OK == jsmi_get_value_sint(&jsmi_parser_extents, NULL, path, NELEMENTS(path), &gmt_offset) : false){
			miio_instance_set_gmt_offset(miio_handle, gmt_offset);
			break;
		}
	}while(0);

	// /country_domian
	do{
		char country_domain[MIIO_COUNTRY_DOMAIN_SIZE_MAX] = {0};
		jsmi_tok_path_t path[] = {
			{
				.key = "country_domain",
				.type = JSMN_STRING
			}
		};
		if( MIIO_OK == jsmi_get_value_str(&jsmi_parser, NULL, path, NELEMENTS(path), country_domain, sizeof(country_domain)) ){
			miio_instance_set_country_domain(miio_handle, country_domain);
			break;
		}
		if(extents_valid ? MIIO_OK == jsmi_get_value_str(&jsmi_parser_extents, NULL, path, NELEMENTS(path), country_domain, sizeof(country_domain)) : false){
			miio_instance_set_country_domain(miio_handle, country_domain);
			break;
		}
	}while(0);

	// /wifi_config/cc
	do{
		char cc[3] = {0};

		jsmi_tok_path_t path[] = {
			{
				.key = "wifi_config",
				.type = JSMN_OBJECT
			},
			{
				.key = "cc",
				.type = JSMN_STRING
			}
		};

		if(MIIO_OK == jsmi_get_value_str(&jsmi_parser, NULL, path, NELEMENTS(path), cc, sizeof(cc))){
			miio_net_set_wifi_channel(cc, 0);
			break;
		}
		if(extents_valid ? MIIO_OK == jsmi_get_value_str(&jsmi_parser_extents, NULL, path, NELEMENTS(path), cc, sizeof(cc)) : false){
			miio_net_set_wifi_channel(cc, 0);
			break;
		}
	}while(0);

	// /bindkey
	do{
		char bind_key[65] = {0};

		jsmi_tok_path_t path[] = {
			{
				.key = "bind_key",
				.type = JSMN_STRING
			}
		};
		if(MIIO_OK == jsmi_get_value_str(&jsmi_parser, NULL, path, NELEMENTS(path), bind_key, sizeof(bind_key))){
			miio_net_set_bindkey(bind_key);
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "bind_key=%s", bind_key);
			break;
		}
		if(extents_valid ? MIIO_OK == jsmi_get_value_str(&jsmi_parser_extents, NULL, path, NELEMENTS(path), bind_key, sizeof(bind_key)) : false){
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "bind_key=%s", bind_key);
			miio_net_set_bindkey(bind_key);
			break;
		}
	}while(0);
        
#if MIIO_KEY_EXCHANGE_ENABLE
    //bind_key_index
    do
    {
		char bind_key_index[65] = {0};

		jsmi_tok_path_t path[] = {
			{
				.key = "bind_index",
				.type = JSMN_STRING
			}
		};

		if(MIIO_OK == jsmi_get_value_str(&jsmi_parser, NULL, path, NELEMENTS(path), bind_key_index, sizeof(bind_key_index))){
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "bind_index=%s", bind_key_index);
			miio_net_set_bindkey_index(miio_handle, bind_key_index);
		}
        else{
            miio_net_set_bindkey_index(miio_handle, NULL);
        }
    }while(0); 

    //bind_key_ts
    do{
		uint64_t bind_key_ts = 0;
		jsmi_tok_path_t path[] = {
			{
				.key = "bind_ts",
				.type = JSMN_PRIMITIVE
			}
		};
		if(MIIO_OK == jsmi_get_value_u64(&jsmi_parser, NULL, path, NELEMENTS(path), &bind_key_ts)){
			miio_net_set_bind_key_ts(bind_key_ts);
		}
	}while(0);
#endif
	// /config_type
	do{
		char config_type[16] = {0};

		jsmi_tok_path_t path[] = {
			{
				.key = "config_type",
				.type = JSMN_STRING
			}
		};
		if(MIIO_OK == jsmi_get_value_str(&jsmi_parser, NULL, path, NELEMENTS(path), config_type, sizeof(config_type))){
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "config_type=%s", config_type);
			miio_net_set_config_type(config_type);
			break;
		}
		if(extents_valid ? MIIO_OK == jsmi_get_value_str(&jsmi_parser_extents, NULL, path, NELEMENTS(path), config_type, sizeof(config_type)) : false ){
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "config_type=%s", config_type);
			miio_net_set_config_type(config_type);
		}
	}while(0);
jsmi_parse_exit:
		jsmi_parse_finish(&jsmi_parser);
		if(extents_valid){
			jsmi_parse_finish(&jsmi_parser_extents);
		}

	if(0 == strnlen(ssid, sizeof(ssid)-1)){
		return MIIO_ERROR_PARAM;
	}
	miio_net_set_ap(ssid, passwd, false);

	miio_net_restart_async(1000);

    return MIIO_OK;
}

