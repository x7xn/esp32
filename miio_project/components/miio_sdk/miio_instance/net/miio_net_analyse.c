#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include "miio_arch.h"
#include "miio_net_analyse.h"
#include "jsmi.h"

static miio_offline_t miio_offline;

extern void ntp_monitor_ack(int result, uint32_t utc, void *ctx);
extern void ntp_monitor_reset(miio_net_t *miio_net);

#undef  MIIO_LOG_TAG
#define MIIO_LOG_TAG						"miio_net_analyse"

#define OTU_HOST_DOMAIN_DNS					"ot.io.mi.com"

static int dns_hijack_check(net_addr_t* paddr)
{
	uint32_t netmask_tmp = 0;
	uint32_t deviceip_tmp = 0;
	struct in_addr sin_netmask = { 0 };
	struct in_addr sin_deviceip = { 0 };

	if(0 == inet_aton(miio_offline.info.netmask, &sin_netmask)){
		LOG_WARN_TAG(MIIO_LOG_TAG,"gateway ip is invalid.");
		return MIIO_ERROR;
	}
	netmask_tmp = ntohl(sin_netmask.s_addr);

	if(0 == inet_aton(miio_offline.info.dev_ip, &sin_deviceip)){
		LOG_WARN_TAG(MIIO_LOG_TAG,"device ip is invalid.");
		return MIIO_ERROR;
	}
	deviceip_tmp = ntohl(sin_deviceip.s_addr);

	LOG_DEBUG_TAG(MIIO_LOG_TAG,"dns_hijack_check device = %u, dns_ddr = %u.",(deviceip_tmp & netmask_tmp),(paddr->s_addr & netmask_tmp));

	if((deviceip_tmp & netmask_tmp) == (paddr->s_addr & netmask_tmp)){
		return DNS_CHECK_FAKE;
	}
	return DNS_CHECK_OK;
}

static bool dns_fake_check(miio_net_t *miio_net)
{
	net_addr_t get_dns_ip;
	int ret = MIIO_OK;
	char ot_domain[MIIO_COUNTRY_DOMAIN_SIZE_MAX + sizeof(OTU_HOST_DOMAIN_DNS)] = {0};
	char country_domain[MIIO_COUNTRY_DOMAIN_SIZE_MAX] = {0};

	miio_get_country_domain(miio_net->miio_handle, country_domain, sizeof(country_domain));

	if(strnlen(country_domain, sizeof(country_domain))){
		snprintf(ot_domain, sizeof(ot_domain), "%s.%s", country_domain, OTU_HOST_DOMAIN_DNS);
	}
	else{
		strncpy(ot_domain, OTU_HOST_DOMAIN_DNS, sizeof(ot_domain)-1);
		ot_domain[sizeof(ot_domain)-1] = '\0';
	}
	arch_net_name2ip(ot_domain, &(get_dns_ip));
	LOG_DEBUG_TAG(MIIO_LOG_TAG,"ip = %u.%u.%u.%u", (get_dns_ip.s_addr >>24)&0xff, \
			(get_dns_ip.s_addr >>16)&0xff, (get_dns_ip.s_addr>>8)&0xff, (get_dns_ip.s_addr>>0)&0xff);

	ret = dns_hijack_check(&(get_dns_ip));

	if(ret == DNS_CHECK_FAKE){
		LOG_WARN_TAG(MIIO_LOG_TAG,"offline reason 15 dns nok \r\n");
		return true;
	}
	return false;
}

static int _miio_offline_net_check(miio_net_t *miio_net, int result)
{
	int ret = 0;
	ip_info_t ip_info = {0};

	if(result == MIIO_OK){ /* check if dns hijack happen */
		ret = dns_fake_check(miio_net);
		if(ret)
			return NET_STATUS_DNS_GET_FAIL;
	}
	if(result == MIIO_ERROR){
		if (MIIO_ERROR == arch_net_get_ip_info(&ip_info)) {
			LOG_WARN_TAG(MIIO_LOG_TAG, "device ip is 0!!!");
				return NET_STATUS_LOCAL_IP_GET_FAIL;
		}
		else {
			if(ip_info.ip.s_addr == 0){
			    LOG_WARN_TAG(MIIO_LOG_TAG, "device ip is 0!!!");
			    return NET_STATUS_LOCAL_IP_GET_FAIL;
			}
			if(ip_info.gw.s_addr == 0){
				LOG_WARN_TAG(MIIO_LOG_TAG, "gateway ip is 0!!!");
				return NET_STATUS_LOCAL_NETWORK_FAIL;
			}
		}
		return NET_STATUS_NETWORK_FAIL;
	}
	if(miio_net->monitor.ntp_successed	&& miio_net->monitor.ntp_ack_cb)
	{
		//reason 30
		LOG_WARN_TAG(MIIO_LOG_TAG,"offline reason 30 OT is nok \r\n");
		miio_net->monitor.ntp_ack_cb = false;
		return NET_STATUS_SERVER_IP_NOK;
	}
	if((miio_net->monitor.ntp_successed == false) && miio_net->monitor.ntp_ack_cb )
	{
		//reason 20
		LOG_WARN_TAG(MIIO_LOG_TAG,"offline reason 20 internet nok  \r\n");
		miio_net->monitor.ntp_ack_cb = false;
		return NET_STATUS_PUBLIC_NETWORK_FAIL;
	}

	return ret;
}


static void _miio_offline_info_clean()
{
	miio_offline.report_enable = false;
	miio_offline.analyse_start = false;

	memset(&(miio_offline.info), 0, sizeof(miio_offline.info));
}


static void _miio_offline_info_dump()
{
	LOG_INFO_TAG(MIIO_LOG_TAG, "dump miio offline info : start_time %d, reason %d, dev_ip %s, gateway %s, rssi %s ",
			miio_offline.info.start_time, miio_offline.info.reason,
			miio_offline.info.dev_ip, miio_offline.info.gateway, miio_offline.info.rssi);
}

static void _miio_offline_info_store()
{
    static bool first_offline = true;
    miio_offline_info_t info;

    LOG_INFO_TAG(MIIO_LOG_TAG, "_miio_offline_info_store");
    if (first_offline == true)
    {
        LOG_INFO_TAG(MIIO_LOG_TAG, "info store in psm ");
        memset(&info, 0, sizeof(info));
        memcpy(info.dev_ip, miio_offline.info.dev_ip, sizeof(info.dev_ip));
        memcpy(info.gateway, miio_offline.info.gateway, sizeof(info.gateway));
        memcpy(info.netmask, miio_offline.info.netmask, sizeof(info.netmask));
        memcpy(info.rssi, miio_offline.info.rssi, sizeof(info.rssi));
        info.reason = miio_offline.info.reason;
        info.sta_disconnect_reason = miio_offline.info.sta_disconnect_reason;
        arch_psm_set_value("ot_config", "offline_info", &info, sizeof(info));
        first_offline = false;
    }
}

static void _miio_offline_clear_psm()
{
    arch_psm_erase_key("ot_config", "offline_info");
}

static bool _miio_offline_psm_info_need_report()
{
    miio_offline_info_t info;
    if(arch_psm_get_value("ot_config", "offline_info", &info, sizeof(info)) == sizeof(info)){
		return true;
	}

    return false;
}

static void _miio_offline_record_specific(net_status_e reason, int sta_disconnect_reason)
{
	LOG_INFO_TAG(MIIO_LOG_TAG, "recording the miio offline with reason %d ", reason);

	miio_offline.analyse_start = false;
	miio_offline.report_enable = true;
	miio_offline.info.start_time = arch_os_ms_now()/1000;
	miio_offline.info.reason = reason;

    if (reason == NET_STATUS_STA_DISCONNECTED)
    {
        miio_offline.info.sta_disconnect_reason = sta_disconnect_reason;
    }

    if (reason == NET_STATUS_LOCAL_IP_GET_FAIL)
	{
		strncpy(miio_offline.info.dev_ip, "0.0.0.0", strlen("0.0.0.0"));
		strncpy(miio_offline.info.gateway, "0.0.0.0", strlen("0.0.0.0"));
	}

	_miio_offline_info_dump();

    _miio_offline_info_store();
}

static void _miio_offline_record_common(miio_net_t *miio_net)
{
	static int ntp_result = 0;
	int ret_reason = 0;

	if (miio_net == NULL)
	{
		LOG_ERROR_TAG(MIIO_LOG_TAG, "invalid input miio_net");
		return;
	}

	LOG_INFO_TAG(MIIO_LOG_TAG, "recording the miio offline begin ");

	if (miio_offline.report_enable != true)
	{
		miio_offline.report_enable = true;
		miio_offline.info.start_time = arch_os_ms_now() / 1000;

		int ntp_timeout_ms = MAX(MIIO_MONITOR_NTP_INTEVAL_S / 2, MIIO_MONITOR_INTERVAL_S) * 1000;
		ntp_result = miio_ntp(miio_net->miio_handle, ntp_monitor_ack, miio_net, ntp_timeout_ms);
		ntp_monitor_reset(miio_net);
	}

	ret_reason = _miio_offline_net_check(miio_net, ntp_result);
	if (ret_reason > 0)
	{
		miio_offline.info.reason = ret_reason;
		miio_offline.analyse_start = false;
		ntp_result = 0;

		LOG_INFO_TAG(MIIO_LOG_TAG, "recording the miio offline done ");
		_miio_offline_info_dump();
        _miio_offline_info_store();
	}
	else
	{
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "waiting for the miio ntp result ... ");
	}
}

static void _miio_offline_report_rpc(miio_handle_t handle)
{
	uint32_t method_id = miio_get_rpc_id(handle);
	size_t js_size = miio_get_rpc_max_size(handle);
	char *js = malloc(js_size);

	if (js)
	{
		jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, js_size);
		jsmi_compose_start(&jsmi_composer);
		jsmi_set_object_begin(&jsmi_composer);
		jsmi_set_key_value_uint(&jsmi_composer, "id", method_id);
		jsmi_set_key_value_str(&jsmi_composer, "method", "_async.stat", sizeof("_async.stat")-1);
		jsmi_set_key_object_begin(&jsmi_composer, "params");

		//offline
		jsmi_set_key_object_begin(&jsmi_composer, "miio.offline");
		jsmi_set_key_value_uint(&jsmi_composer, "reason", miio_offline.info.reason);
		jsmi_set_key_value_uint(&jsmi_composer, "time", (arch_os_ms_now()/1000 - miio_offline.info.start_time));
        jsmi_set_key_value_sint(&jsmi_composer, "wifi_disc_reason", miio_offline.info.sta_disconnect_reason);
		jsmi_set_key_object_end(&jsmi_composer);

		//dev.info
		jsmi_set_key_object_begin(&jsmi_composer, "dev.info");
		jsmi_set_key_value_str(&jsmi_composer, "localIp", miio_offline.info.dev_ip, strnlen(miio_offline.info.dev_ip, sizeof(miio_offline.info.dev_ip)));
		jsmi_set_key_value_str(&jsmi_composer, "gw", miio_offline.info.gateway, strnlen(miio_offline.info.gateway, sizeof(miio_offline.info.gateway)));
		jsmi_set_key_value_str(&jsmi_composer, "rssi", miio_offline.info.rssi, strnlen(miio_offline.info.rssi, sizeof(miio_offline.info.rssi)));
		jsmi_set_key_object_end(&jsmi_composer);

		//modle
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
				LOG_WARN_TAG(MIIO_LOG_TAG, "_async.stat offline report failed.");
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


static void _miio_offline_psm_info_report_rpc(miio_handle_t handle)
{
    uint32_t method_id = miio_get_rpc_id(handle);
    size_t js_size = miio_get_rpc_max_size(handle);
    arch_psm_get_value("ot_config", "offline_info", &miio_offline.info, sizeof(miio_offline.info));
    char *js = malloc(js_size);

	if (js)
	{
		jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, js_size);
		jsmi_compose_start(&jsmi_composer);
		jsmi_set_object_begin(&jsmi_composer);
		jsmi_set_key_value_uint(&jsmi_composer, "id", method_id);
		jsmi_set_key_value_str(&jsmi_composer, "method", "_async.stat", sizeof("_async.stat")-1);
		jsmi_set_key_object_begin(&jsmi_composer, "params");

		//offline
		jsmi_set_key_object_begin(&jsmi_composer, "miio.offline");
		jsmi_set_key_value_uint(&jsmi_composer, "reason", miio_offline.info.reason);
		jsmi_set_key_value_uint(&jsmi_composer, "time", 0);
        jsmi_set_key_value_sint(&jsmi_composer, "wifi_disc_reason", miio_offline.info.sta_disconnect_reason);
		jsmi_set_key_object_end(&jsmi_composer);

		//dev.info
		jsmi_set_key_object_begin(&jsmi_composer, "dev.info");
		jsmi_set_key_value_str(&jsmi_composer, "localIp", miio_offline.info.dev_ip, strnlen(miio_offline.info.dev_ip, sizeof(miio_offline.info.dev_ip)));
		jsmi_set_key_value_str(&jsmi_composer, "gw", miio_offline.info.gateway, strnlen(miio_offline.info.gateway, sizeof(miio_offline.info.gateway)));
		jsmi_set_key_value_str(&jsmi_composer, "rssi", miio_offline.info.rssi, strnlen(miio_offline.info.rssi, sizeof(miio_offline.info.rssi)));
		jsmi_set_key_object_end(&jsmi_composer);

		//modle
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
				LOG_WARN_TAG(MIIO_LOG_TAG, "_async.stat offline report failed.");
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


void miio_offline_open()
{
	LOG_INFO_TAG(MIIO_LOG_TAG, "enable the miio offline detect feature ");

	arch_os_mutex_get(miio_offline.offline_mutex, ARCH_OS_WAIT_FOREVER);
	miio_offline.analyse_start = true;
	arch_os_mutex_put(miio_offline.offline_mutex);
}

void miio_online_stats_gather(int8_t rssi)
{
	ip_info_t ip_info = {0};

	LOG_INFO_TAG(MIIO_LOG_TAG, "miio_online_stats_gather ");

	arch_os_mutex_get(miio_offline.offline_mutex, ARCH_OS_WAIT_FOREVER);
	if (MIIO_OK == arch_net_get_ip_info(&ip_info)) {
		arch_net_htoa_buf(&ip_info.ip, miio_offline.info.dev_ip, sizeof(miio_offline.info.dev_ip));
		arch_net_htoa_buf(&ip_info.gw, miio_offline.info.gateway, sizeof(miio_offline.info.gateway));
		arch_net_htoa_buf(&ip_info.netmask, miio_offline.info.netmask, sizeof(miio_offline.info.netmask));
	}

	snprintf(miio_offline.info.rssi, sizeof(miio_offline.info.rssi), "%d", rssi);

	LOG_INFO_TAG(MIIO_LOG_TAG, "prepare online event information ip=%s, gw=%s, netmask=%s, rssi=%s",
			miio_offline.info.dev_ip, miio_offline.info.gateway, miio_offline.info.netmask, miio_offline.info.rssi);
	arch_os_mutex_put(miio_offline.offline_mutex);
}

void miio_offline_close()
{
	LOG_INFO_TAG(MIIO_LOG_TAG, "close the miio offline detect feature ");

	arch_os_mutex_get(miio_offline.offline_mutex, ARCH_OS_WAIT_FOREVER);
	miio_offline.analyse_start = false;
	arch_os_mutex_put(miio_offline.offline_mutex);
}

void miio_offline_record(miio_net_t *miio_net, net_status_e reason, int sta_disconnect_reason)
{
	arch_os_mutex_get(miio_offline.offline_mutex, ARCH_OS_WAIT_FOREVER);

	if (miio_offline.analyse_start == true)
	{
		if (reason == NET_STATUS_UNKOWN)
			_miio_offline_record_common(miio_net);
		else
			_miio_offline_record_specific(reason, sta_disconnect_reason);
	}

	arch_os_mutex_put(miio_offline.offline_mutex);
}

void miio_offline_report(miio_handle_t handle)
{
	arch_os_mutex_get(miio_offline.offline_mutex, ARCH_OS_WAIT_FOREVER);

	if(miio_offline.report_enable)
	{
		LOG_INFO_TAG(MIIO_LOG_TAG, "reporting the miio offline information ");

		_miio_offline_report_rpc(handle);
		_miio_offline_info_clean();

        if (_miio_offline_psm_info_need_report())
        {
            _miio_offline_clear_psm();
        }
	}
    else
    {
        if (_miio_offline_psm_info_need_report())
        {
            LOG_INFO_TAG(MIIO_LOG_TAG, "reporting last stored miio offline information ");
            _miio_offline_psm_info_report_rpc(handle);
            _miio_offline_info_clean();
            _miio_offline_clear_psm();
        }
    }

	arch_os_mutex_put(miio_offline.offline_mutex);
}

int miio_offline_init()
{
	if (MIIO_OK != arch_os_mutex_create(&(miio_offline.offline_mutex)))
	{
		LOG_ERROR_TAG(MIIO_LOG_TAG, "init miio offline mutex error");
		return MIIO_ERROR_NOMEM;
	}

	_miio_offline_info_clean();

	return MIIO_OK;
}
