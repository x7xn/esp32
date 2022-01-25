#include "miio_api.h"
#include "miio_arch.h"
#include "jsmi.h"
#include "miio_command.h"
#include "miio_net.h"
#if MIBLE_ENABLE
#include "mible_cmd.h"
#endif

#undef MIIO_LOG_TAG
#define MIIO_LOG_TAG			"mcmd"

static void* find_cmd(char *name, int name_len)
{
	if (!name || !name_len)
		return NULL;

	miio_addon_symbol_t* table = miio_addon_entry_start(MIIO_CMD_ADDON_NAME);
	int table_len = miio_addon_entry_count(MIIO_CMD_ADDON_NAME);

	miio_addon_symbol_t *cmdtp;
	for (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		if ( (name_len == strlen(cmdtp->name)) &&
				(strncmp(name, cmdtp->name, name_len) == 0) ){
			return cmdtp->value;
		}
	}
	return NULL;
}


int mcmd_handle(mcmd_t *mcmd, char* cmd, char *params)
{
	int cmd_found;

	void* fp = find_cmd(cmd, strlen(cmd));
	if(fp){
		((fp_cmd_t)fp)(mcmd, params);
		cmd_found = 1;
	}
	else{
		LOG_WARN_TAG(MIIO_LOG_TAG, "cmd %s not found", cmd);
		mcmd->io.out(mcmd->io.handle, "error");
		cmd_found = 0;
	}
	return cmd_found;
}


static const char* get_net_state_change_info(mcmd_t *mcmd)
{
	static miio_net_state_t old_net_state = MIIO_NET_UNPROV;

	const char* info = NULL;

	miio_net_state_t new_net_state = miio_net_get_state();

	if(new_net_state != old_net_state){
		switch(new_net_state){
		case MIIO_NET_UNPROV:
			info = "unprov";
			break;
		case MIIO_NET_UAP:
			info = "uap";
			break;
		case MIIO_NET_DISCONNECTED:
			info = "offline";
			break;
		case MIIO_NET_LOCAL:
			info = "local";
			break;
		case MIIO_NET_CLOUD:
			info = "cloud";
			break;
		case MIIO_NET_UPDATING:
#if MIIO_AUTO_OTA_ENABLE
			if(mcu_is_auto_updating(mcmd)){
				if(true == mcmd->ota_mcu_ctx.auto_ota.force_enable){
					info = "updating force";
				}
				else{
					info = "updating auto";
				}
			}
			else if(app_is_auto_updating()){
				info = "updating auto";
			}
			else
#endif  /* MIIO_AUTO_OTA_ENABLE */
			{
				info = "updating";
			}
			break;
		default:
			info = NULL;
			break;
		}
		old_net_state = new_net_state;
	}

	return info;
}

static void do_help(mcmd_t *mcmd, char *params)
{
	miio_addon_symbol_t* table = miio_addon_entry_start(MIIO_CMD_ADDON_NAME);
	int table_len = miio_addon_entry_count(MIIO_CMD_ADDON_NAME);

	miio_addon_symbol_t *cmdtp;
	for (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		uint32_t n = 0;
		n += snprintf_safe(mcmd->command_buf+n, mcmd->command_buf_size-n, "%-32.32s ", cmdtp->name);
		if(cmdtp->tip){
			n += snprintf_safe(mcmd->command_buf+n, mcmd->command_buf_size-n, "%s", cmdtp->tip);
		}

		mcmd->io.out(mcmd->io.handle, mcmd->command_buf);
	}
}
MIIO_CMD(help, do_help, NULL);


static void do_echo(mcmd_t *mcmd, char *params)
{
	char *argv[1];
	int argc = mcmd_parse_params(params, argv, NELEMENTS(argv), NULL);

	if (argc > 0) {
		if(0 == mcmd->io.echo(mcmd->io.handle, argv[0]))
			mcmd->io.out(mcmd->io.handle, "ok");
		else
			mcmd->io.out(mcmd->io.handle, "error");
	}
	else{
		mcmd->io.out(mcmd->io.handle, "error");
	}
}
MIIO_CMD(echo, do_echo, NULL);


static void do_mac(mcmd_t *mcmd, char *params)
{
	uint8_t mac[6];
	arch_get_mac(mac);

	snprintf(mcmd->command_buf, mcmd->command_buf_size, "%02x%02x%02x%02x%02x%02x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

	mcmd->io.out(mcmd->io.handle, mcmd->command_buf);
}
MIIO_CMD(mac, do_mac, NULL);


static void do_model(mcmd_t *mcmd, char *params)
{
	char *argv[1];
	int argc = mcmd_parse_params(params, argv, NELEMENTS(argv), NULL);

    if (argc > 0) { // set
        bool illegal = false;
        char *model = argv[0];
        {
            // verify model string
            int len = 0;
            int dot_idx[2] = { 0, 0 };
            int dot_cnt = 0;
            while ('\0' != model[len]) {
                if ('.' == model[len]) {
                    if (dot_cnt >= 2) {
                        illegal = true;
                        break;
                    }
                    dot_idx[dot_cnt++] = len;
                }

                if (!isdigit((int)(model[len])) && !isalpha((int)(model[len])) && '_' != model[len] && '-' != model[len] && '.' != model[len]) {
                    illegal = true;
                    break;
                }

                len++;
            }

            if (   dot_cnt != 2
            	|| dot_idx[0] <= 0
                || dot_idx[1] - dot_idx[0] <= 1
                || len - dot_idx[1] <= 1
                || len >= MIIO_MODEL_SIZE_MAX) {
                illegal = true;
            }
        }

        if (illegal) {
        	mcmd->io.out(mcmd->io.handle, "error");

        }
        else {
            miio_instance_set_model(mcmd->miio_handle, model);

            mcmd->io.out(mcmd->io.handle, "ok");
        }
    }
    else { // get
    	char model[32];
    	mcmd->io.out(mcmd->io.handle, miio_get_model(mcmd->miio_handle, model, sizeof(model)));
    }
}
MIIO_CMD(model, do_model, NULL);


static void do_mcu_version(mcmd_t *mcmd, char *params)
{
	char *argv[1];
	int argc = mcmd_parse_params(params, argv, NELEMENTS(argv), NULL);

	if (argc < 1) {
        mcmd->io.out(mcmd->io.handle, "error");
        return;
    }

	char *version = argv[0];
    if (strlen(version) != 4) {
        mcmd->io.out(mcmd->io.handle, "error");
        return;
    }

    if (isdigit((int)(version[0])) && isdigit((int)(version[1])) && isdigit((int)(version[2])) && isdigit((int)(version[3]))) {
    	miio_mcu_set_version(version);
        mcmd->io.out(mcmd->io.handle, "ok");
    }
    else {
        mcmd->io.out(mcmd->io.handle, "error");
    }
}
MIIO_CMD(mcu_version, do_mcu_version, NULL);


static void do_reboot(mcmd_t *mcmd, char *params)
{
    mcmd->io.out(mcmd->io.handle, "ok");

    miio_reboot(mcmd->miio_handle, "cmd.reboot", MIIO_RPC_TIMEOUT_MS);
}
MIIO_CMD(reboot, do_reboot, NULL);

static void do_restore(mcmd_t *mcmd, char *params)
{
	if(miio_net_get_disable_local_restore()){
		if(miio_is_online(mcmd->miio_handle) || arch_os_time_now() < 3600){
			mcmd->io.out(mcmd->io.handle, "ok");
			return;
		}
	}

    mcmd->io.out(mcmd->io.handle, "ok");

    miio_restore(mcmd->miio_handle, "cmd.restore");

    miio_reboot(mcmd->miio_handle, "cmd.restore", 10);
}
MIIO_CMD(restore, do_restore, NULL);


static void do_update_me(mcmd_t *mcmd, char *params)
{
	if(MIIO_OK == miio_ota_mcu_refresh_fw(&mcmd->ota_mcu_ctx))
		mcmd->io.out(mcmd->io.handle, "ok");
	else
		mcmd->io.out(mcmd->io.handle, "error");
}
MIIO_CMD(update_me, do_update_me, NULL);


static void do_factory(mcmd_t *mcmd, char *params)
{
    mcmd->io.out(mcmd->io.handle, "ok");
    miio_net_factory();
}
MIIO_CMD(factory, do_factory, NULL);

static void do_setwifi(mcmd_t *mcmd, char *params)
{
	char *argv[2];
	int argc = mcmd_parse_params(params, argv, NELEMENTS(argv), NULL);

	//ssid pwd
	if (argc == 2) {
		mcmd->io.out(mcmd->io.handle, "ok");

		char ssid[33] = {0};
		char passwd[65] = {0};
		mcmd_unescape(argv[0], strlen(argv[0]), ssid, sizeof(ssid)-1);
		mcmd_unescape(argv[1], strlen(argv[1]), passwd, sizeof(passwd)-1);
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "ssid = \"%s\", passwd = \"%s\"", ssid, passwd);
#if MIIO_PROV_STATISTIC
		miio_net_clear_provision_stat();
#endif
		miio_net_set_ap(ssid, passwd, true);

		miio_net_restart_async(1000);
	}
	else {
		mcmd->io.out(mcmd->io.handle,"error");
	}

}
MIIO_CMD(setwifi, do_setwifi, NULL);

static void do_getwifi(mcmd_t *mcmd, char *params)
{
	net_wifi_ap_info_t ap_info;

	if(MIIO_OK != arch_net_get_ap_info(&ap_info)){
		mcmd->io.out(mcmd->io.handle,"error");
		return;
	}

	int ssid_len = strnlen((char*)ap_info.ssid, sizeof(ap_info.ssid));
	if(0 == ssid_len){
		mcmd->io.out(mcmd->io.handle,"error");
		return;
	}

	//ssid rssi primary bssid
	uint32_t command_buf_size = mcmd_escape((char*)ap_info.ssid, ssid_len, NULL ,0) + 1 + sizeof("-000")*2 + sizeof(ap_info.bssid)*3;
	char *command_buf = malloc(command_buf_size);
	if(command_buf){
		uint32_t command_buf_len = 0;
		command_buf_len = mcmd_escape((char*)ap_info.ssid, ssid_len, command_buf, command_buf_size);
		command_buf[command_buf_len++] = ' ';
		command_buf_len += snprintf(command_buf+command_buf_len, command_buf_size-command_buf_len, "%d ", ap_info.rssi);
		command_buf_len += snprintf(command_buf+command_buf_len, command_buf_size-command_buf_len, "%d ", ap_info.primary);
		command_buf_len += snprintf_hex(command_buf+command_buf_len, command_buf_size-command_buf_len, ap_info.bssid, sizeof(ap_info.bssid), 0x80 | ':');
		command_buf[command_buf_len] = '\0';
		mcmd->io.out(mcmd->io.handle, command_buf);
		free(command_buf);
		return;
	}

	mcmd->io.out(mcmd->io.handle,"error");
}
MIIO_CMD(getwifi, do_getwifi, NULL);

static void do_country_code(mcmd_t *mcmd, char *params)
{
	char *argv[1];
	int argc = mcmd_parse_params(params, argv, NELEMENTS(argv), NULL);

	if(argc){
		miio_net_set_wifi_channel(argv[0], 1);
		mcmd->io.out(mcmd->io.handle,"ok");
	}
	else{
		char cc[3] = "CN";
		miio_net_get_wifi_channel(cc, sizeof(cc), -1);
		mcmd->io.out(mcmd->io.handle, cc);
	}
}
MIIO_CMD(country_code, do_country_code, NULL);


static void do_arch(mcmd_t *mcmd, char *params)
{
	mcmd->io.out(mcmd->io.handle, arch_get_chip_type());
}
MIIO_CMD(arch, do_arch, NULL);


static void do_version(mcmd_t *mcmd, char *params)
{
#if MIIO_APP_VERSION_NUMBER
	mcmd->io.out(mcmd->io.handle, miio_app_version(MIIO_APP_VERSION_NUMBER));
#else
	mcmd->io.out(mcmd->io.handle, miio_instance_version());
#endif
}
MIIO_CMD(version, do_version, NULL);


static void do_net(mcmd_t *mcmd, char *params)
{
    switch(miio_net_get_state()){
    case MIIO_NET_UAP:
    	mcmd->io.out(mcmd->io.handle, "uap");
    	break;
    case MIIO_NET_UNPROV:
    	mcmd->io.out(mcmd->io.handle, "unprov");
    	break;
    case MIIO_NET_DISCONNECTED:
    	mcmd->io.out(mcmd->io.handle, "offline");
    	break;
    case MIIO_NET_LOCAL:
		mcmd->io.out(mcmd->io.handle, "local");
		break;
    case MIIO_NET_CLOUD:
    	mcmd->io.out(mcmd->io.handle, "cloud");
        break;
    case MIIO_NET_UPDATING:
    	mcmd->io.out(mcmd->io.handle, "updating");
    	break;
    default:
        mcmd->io.out(mcmd->io.handle, "error");
        break;
    }
}
MIIO_CMD(net, do_net, NULL);

static void do_time(mcmd_t *mcmd, char *params)
{
	char *argv[1];
	int argc = mcmd_parse_params(params, argv, NELEMENTS(argv), NULL);

    if(argc > 0 && strncmp(argv[0], "posix", 5) == 0) {
    	snprintf(mcmd->command_buf, mcmd->command_buf_size, "%u", (uint32_t)arch_os_utc_now());
    	mcmd->io.out(mcmd->io.handle, mcmd->command_buf);
    }
    else {

    	int gmt_offset;

		if(argc > 0){
			gmt_offset = atoi(argv[0])*60*60;
		}
		else{
			gmt_offset = miio_get_gmt_offset(mcmd->miio_handle);
		}

		if(gmt_offset<(-12)*60*60 || gmt_offset>12*60*60)
		{
			gmt_offset = 8*60*60;//default beijing time.
		}

		struct tm time;
	    time_t t = arch_os_utc_now() + gmt_offset;
	    gmtime_r(&t,&time);

    	snprintf(mcmd->command_buf, mcmd->command_buf_size, "%d-%02d-%02d %d:%02d:%02d", time.tm_year + 1900, time.tm_mon+1, time.tm_mday,time.tm_hour, time.tm_min, time.tm_sec);
    	mcmd->io.out(mcmd->io.handle, mcmd->command_buf);

    }
}
MIIO_CMD(time, do_time, NULL);


static void do_props(mcmd_t *mcmd, char *params)
{
	char *argv[MCMD_COMMAND_ARG_MAX];
	int argc = mcmd_parse_params(params, argv, NELEMENTS(argv), " ,");
    if (argc < 2 || (argc % 2) != 0) {
        LOG_ERROR_TAG(MIIO_LOG_TAG, "wrong number of prop arg");
        goto err_exit;
    }

    for(int i = 0; i  < argc; i += 2) {
        if(MIIO_OK != miio_set_property(mcmd->miio_handle, argv[i], argv[i+1])) {
            LOG_ERROR_TAG(MIIO_LOG_TAG, "update prop failed");
            goto err_exit;
        }
    }

    mcmd->io.out(mcmd->io.handle, "ok");

    return;

err_exit:

	mcmd->io.out(mcmd->io.handle, "error");

    return;
}
MIIO_CMD(props, do_props, NULL);

static void do_event(mcmd_t *mcmd, char *params)
{
    char *argv[MCMD_COMMAND_ARG_MAX];
    int argc = mcmd_parse_params(params, argv, NELEMENTS(argv), " ,");
	if (argc < 1) {
		goto err_exit;
	}

	if(MIIO_OK != miio_set_event(mcmd->miio_handle, argv[0], (const char**)&argv[1], argc-1)){
		goto err_exit;
	}

	miio_flush_event(mcmd->miio_handle);

	mcmd->io.out(mcmd->io.handle, "ok");

	return;

err_exit:

	mcmd->io.out(mcmd->io.handle, "error");

	return;
}
MIIO_CMD(event, do_event, NULL);

static void do_get_down(mcmd_t *mcmd, char *params)
{
	//handle rpc
	if(0 != mcmd_rpc_get(mcmd, 0)){
		return;
	}

	//check net status
	const char *net_change_info = get_net_state_change_info(mcmd);
	if(net_change_info){
		mcmd_composer_t mcmd_composer = MCMD_COMPOSER_INIT(mcmd->command_buf, mcmd->command_buf_size);
		mcmd_compose_start(&mcmd_composer, "down");
		mcmd_compose_bytes(&mcmd_composer, "MIIO_net_change", sizeof("MIIO_net_change")-1);
		mcmd_compose_bytes(&mcmd_composer, net_change_info, strlen(net_change_info));
		mcmd_compose_finish(&mcmd_composer, NULL, NULL);
		mcmd->io.out(mcmd->io.handle, mcmd->command_buf);
		return;
	}

#if MIBLE_ENABLE
	//check BLE event
    if (MIIO_OK == mible_cmd_get(mcmd->command_buf, mcmd->command_buf_size)) {
        mcmd->io.out(mcmd->io.handle, mcmd->command_buf);
        return;
    }
#endif

	mcmd->io.out(mcmd->io.handle, "down none");

    return;
}
MIIO_CMD(get_down, do_get_down, NULL);


static void do_result(mcmd_t *mcmd, char *params)
{
    if(0 != mcmd_rpc_result(mcmd, params)){
		return;
    }

	mcmd->io.out(mcmd->io.handle, "error");

    return;
}
MIIO_CMD(result, do_result, NULL);


static void do_error(mcmd_t *mcmd, char *params)
{
	if(0 != mcmd_rpc_error(mcmd, params)){
		return;
    }

    mcmd->io.out(mcmd->io.handle, "error");

	return;
}
MIIO_CMD(error, do_error, NULL);


static void do_json_get_down(mcmd_t *mcmd, char *params)
{
    //handle rpc
	if(0 != mcmd_rpc_get(mcmd, 1)){
		return;
	}

	mcmd->io.out(mcmd->io.handle, "json_down none");

    return;
}
MIIO_CMD(json_get_down, do_json_get_down, NULL);


static void do_json_ack(mcmd_t *mcmd, char *params)
{
	if(0 != mcmd_rpc_result(mcmd, params)){
		return;
	}

	mcmd->io.out(mcmd->io.handle, "error");
	return;
}
MIIO_CMD(json_ack, do_json_ack, NULL);

static void do_config_router(mcmd_t *mcmd, char *params)
{
	if(params == NULL){
		mcmd->io.out(mcmd->io.handle, "error");
		return;
	}

	if(MIIO_OK == miio_net_config_router(mcmd->miio_handle, params, strlen(params), NULL, 0)){
		mcmd->io.out(mcmd->io.handle, "ok");
	}else{
		mcmd->io.out(mcmd->io.handle, "error");
	}
}
MIIO_CMD(config_router, do_config_router, NULL);

#if MIIO_AUTO_OTA_ENABLE
static void do_set_mcu_auto_ota(mcmd_t *mcmd, char *params)
{
	char *argv[1];
	int argc = mcmd_parse_params(params, argv, NELEMENTS(argv), NULL);

	if (argc > 0) {
		if(strcmp(argv[0], "on") == 0) {
			miio_instance_mcu_auto_upgrade_enable(mcmd->miio_handle, true);
		} else if(strcmp(argv[0],"off") == 0) {
			miio_instance_mcu_auto_upgrade_enable(mcmd->miio_handle, false);
		} else{
			goto err_exit;
		}
	}
	else{
		goto err_exit;
	}

    mcmd->io.out(mcmd->io.handle, "ok");

    return;

err_exit:

	mcmd->io.out(mcmd->io.handle, "error");

    return;
}
MIIO_CMD(set_mcu_auto_ota, do_set_mcu_auto_ota, NULL);
#endif
