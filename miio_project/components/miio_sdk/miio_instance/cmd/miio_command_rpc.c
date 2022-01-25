#include "miio_command.h"
#include "list/list.h"
#include "jsmi.h"
#include "xmodem.h"

#undef MIIO_LOG_TAG
#define MIIO_LOG_TAG			"mcmd"

int mcmd_rpc_error_default(mcmd_t *mcmd, int code, char *message)
{
	char* js = mcmd->down.buf;
	int js_size = MIN(mcmd->down.buf_size, miio_get_rpc_max_size(mcmd->miio_handle));

	jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, js_size);
	jsmi_compose_start(&jsmi_composer);
	jsmi_set_object_begin(&jsmi_composer);
		jsmi_set_key_value_uint(&jsmi_composer, "id", mcmd->down.method_ctx.arg.id);
		jsmi_set_key_object_begin(&jsmi_composer, "error");
			jsmi_set_key_value_sint(&jsmi_composer, "code", MIIO_USER_ERR_CODE_FILTER(code));
			if(message && strlen(message)){
				jsmi_set_key_value_str(&jsmi_composer, "message", message, strlen(message));
			}
		jsmi_set_key_object_end(&jsmi_composer);
	jsmi_set_object_end(&jsmi_composer);

	if(MIIO_OK == jsmi_compose_finish(&jsmi_composer, NULL, NULL)){
		miio_rpc_delegate_arg_t ack_arg = {
			.id = mcmd->down.method_ctx.arg.id,
			.type = MIIO_DELEGATE_JSON,
			.pload_len = strnlen(js, js_size),
			.pload = js
		};

		return mcmd->down.method_ctx.ack(&ack_arg, mcmd->down.method_ctx.ack_ctx);
	}

	return MIIO_ERROR;
}

int mcmd_rpc_result_default(mcmd_t *mcmd, char *params)
{
	char *argv[MCMD_COMMAND_ARG_MAX];
	int argc = mcmd_parse_params(params, argv, NELEMENTS(argv), " ,");
	if (argc) {
		char* js = mcmd->down.buf;
		int js_size = MIN(mcmd->down.buf_size, miio_get_rpc_max_size(mcmd->miio_handle));

		jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, js_size);
		jsmi_compose_start(&jsmi_composer);
		jsmi_set_object_begin(&jsmi_composer);
			jsmi_set_key_value_uint(&jsmi_composer, "id", mcmd->down.method_ctx.arg.id);
			jsmi_set_key_array_begin(&jsmi_composer, "result");
			for (int i = 0; i < argc; i++) {
				jsmi_set_value_bytes(&jsmi_composer, argv[i], strlen(argv[i]));
			}
			jsmi_set_key_array_end(&jsmi_composer);
		jsmi_set_object_end(&jsmi_composer);

		if(MIIO_OK == jsmi_compose_finish(&jsmi_composer, NULL, NULL)){
			miio_rpc_delegate_arg_t ack_arg = {
				.id = mcmd->down.method_ctx.arg.id,
				.type = MIIO_DELEGATE_JSON,
				.pload_len = strnlen(js, js_size),
				.pload = js
			};
			return mcmd->down.method_ctx.ack(&ack_arg, mcmd->down.method_ctx.ack_ctx);
		}
	}

	mcmd_rpc_error_default(mcmd, MIIO_USER_ERR_CODE_ACK_INVALID, MIIO_USER_ERR_INFO_ACK_INVALID);

	return MIIO_ERROR;
}


static int rpc_handle_default(mcmd_t *mcmd, const char* method, size_t method_len, const char *params, size_t params_len)
{
	mcmd_composer_t mcmd_composer = MCMD_COMPOSER_INIT(mcmd->command_buf, mcmd->command_buf_size);
	mcmd_compose_start(&mcmd_composer, "down");
	mcmd_compose_bytes(&mcmd_composer, method, method_len);

	jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(params, params_len);
	if(MIIO_OK == jsmi_parse_start(&jsmi_parser)){
		jsmntok_t *params_tok = jsmi_get_tok(&jsmi_parser, NULL, NULL, 0);
		if(params_tok && JSMN_ARRAY == params_tok->type){
			if((params_tok->end - params_tok->start) > 2){
#if MCMD_OLD_PROTO_ADAPT
				mcmd_compose_bytes(&mcmd_composer, jsmi_parser.js + params_tok->start + 1, params_tok->end - params_tok->start - 2);
#else
				for(int i=0; i < params_tok->size; i++){
					jsmi_tok_path_t path[] = {
						{
							.key = (void*)i,
							.type = JSMN_UNDEFINED
						}
					};
					jsmntok_t *item_tok = jsmi_get_tok(&jsmi_parser, params_tok, path, NELEMENTS(path));
					if(item_tok){
						if(JSMN_STRING == item_tok->type){
							mcmd_compose_bytes(&mcmd_composer, jsmi_parser.js + item_tok->start - 1, item_tok->end - item_tok->start + 2);
						}
						else if((item_tok->end - item_tok->start) > 0){
							mcmd_compose_bytes(&mcmd_composer, jsmi_parser.js + item_tok->start, item_tok->end - item_tok->start);
						}
					}
				}
#endif
			}
		}
		else{
			mcmd_compose_bytes(&mcmd_composer, params, params_len);
		}
		jsmi_parse_finish(&jsmi_parser);
	}

	if(MIIO_OK == mcmd_compose_finish(&mcmd_composer, NULL, NULL)){
		mcmd->down.result = mcmd_rpc_result_default;
		mcmd->down.error = mcmd_rpc_error_default;
		mcmd->io.out(mcmd->io.handle, mcmd->command_buf);
		return MIIO_OK;
	}

	return MIIO_ERROR;
}


static void* find_cmdrpc(const char *name, int name_len)
{
	if (!name || !name_len)
		return NULL;

	miio_addon_symbol_t* table = miio_addon_entry_start(MIIO_CMD_RPC_ADDON_NAME);
	int table_len = miio_addon_entry_count(MIIO_CMD_RPC_ADDON_NAME);

	miio_addon_symbol_t *cmdtp;
	for (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		if ( (name_len == strlen(cmdtp->name)) &&
				(strncmp(name, cmdtp->name, name_len) == 0) ){
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "%s found", cmdtp->name);
			return cmdtp->value;
		}
	}

	LOG_DEBUG_TAG(MIIO_LOG_TAG, "rpc not found, call rpc_handle_default");

	return rpc_handle_default;
}


/**********************************************
* Transfer firmware in flash to mcu
***********************************************/
static int ota_mcu_error(mcmd_t *mcmd, int code, char *message)
{
	if((MIIO_USER_ERR_CODE_ACK_TIMEOUT == code) || MIIO_OTA_ERR_CODE_CHECK(code)){
		miio_ota_mcu_ctx_t* ota_mcu_ctx = &mcmd->ota_mcu_ctx;
		miio_ota_mcu_buffer_fw(ota_mcu_ctx);
		miio_ota_task_install_error(ota_mcu_ctx->ota_task_handle, code, message);
		return MIIO_OK;
	}
	return MIIO_ERROR;
}


static int ota_mcu_result(mcmd_t *mcmd, char *params)
{
	char *argv[MCMD_COMMAND_ARG_MAX];
	int argc = mcmd_parse_params(params, argv, NELEMENTS(argv), NULL);
	if (argc < 1) {
		return MIIO_ERROR_PARAM;
	}

	miio_ota_mcu_ctx_t* ota_mcu_ctx = &mcmd->ota_mcu_ctx;

#if MIIO_AUTO_OTA_ENABLE
	if(0 == strcmp(argv[0], "\"busy\"")) { /* mcu send "busy" */

		mcmd->io.out(mcmd->io.handle, "ok");

		if(ota_mcu_ctx->auto_ota.auto_enable) {
			/* erase mcu fw when auto ota failed */
			miio_ota_mcu_erase_fw(ota_mcu_ctx);
		} else {
			/* wait for next mcu ota when module reboot */
			miio_ota_mcu_buffer_fw(ota_mcu_ctx);
		}

		LOG_WARN_TAG(MIIO_LOG_TAG, "mcu is busy for ota install");
		miio_ota_task_install_error(ota_mcu_ctx->ota_task_handle, MIIO_OTA_ERR_CODE_INSTALL_BUSY, MIIO_OTA_ERR_INFO_INSTALL_BUSY);

		return MIIO_OK;
	} else
#endif /* MIIO_AUTO_OTA_ENABLE */
	if(0 == strcmp(argv[0], "\"ready\"")) { /* mcu send "ready" */

		int ret = -1;
		xmodem x = xmodem_create(mcmd->io.handle, mcmd->io.out_byte, mcmd->io.in_byte);
		if(NULL == x){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "xmodem create error ");
			mcmd->io.out(mcmd->io.handle, "error");
			goto ready_exit;
		}

		//send "ok" to tell mcu turn to xmodem recv mode
		mcmd->io.out(mcmd->io.handle, "ok");

		miio_ota_task_install_start(ota_mcu_ctx->ota_task_handle);

		//xmodem send data
		do{
			int crc;
			ret = xmodem_get_crc_config(x, &crc);
			if(ret < 0) {
				LOG_ERROR_TAG(MIIO_LOG_TAG, "xmodem get crc error");
				break;
			}

			uint32_t firmware_size = ota_mcu_ctx->flash_header.length - 4; //ignore crc32
			uint32_t bytes_copied  = 0;
			uint32_t size_to_read = 0;

			//read add transfer
			while(bytes_copied < firmware_size) {
			   size_to_read = MIN(mcmd->down.buf_size, (firmware_size - bytes_copied));
			   if(MIIO_OK != arch_ota_read_flash(&ota_mcu_ctx->flash_info, bytes_copied + sizeof(ota_mcu_ctx->flash_header), (uint8_t*)mcmd->down.buf, size_to_read)){
				   ret = -1;
				   LOG_ERROR_TAG(MIIO_LOG_TAG, "read from flash error");
				   cancel_xmodem_transfer(x);
				   break;
			   }

			   bytes_copied += size_to_read;
			   if(bytes_copied < firmware_size) {
				   ret = xmodem_transfer_data(x, crc, (uint8_t*)mcmd->down.buf, size_to_read, XMODEM_MTU);
				   if(ret != 0) {
					   LOG_ERROR_TAG(MIIO_LOG_TAG, "xmodem transfer end error");
					   break;
				   }
				   miio_ota_task_install_update(ota_mcu_ctx->ota_task_handle, bytes_copied*100/firmware_size);
			   }
			   else{
				   ret = xmodem_transfer_end(x, crc, (uint8_t*)mcmd->down.buf, size_to_read, XMODEM_MTU);
				   if(ret != 0) {
					   LOG_ERROR_TAG(MIIO_LOG_TAG, "xmodem transfer end error");
					   break;
				   }
				   miio_ota_task_install_update(ota_mcu_ctx->ota_task_handle, 100);
			   }
			}
		}while(0);

ready_exit:

		if(x){
			xmodem_destroy(&x);
		}

		if(ret != 0 ){
#if MIIO_AUTO_OTA_ENABLE
			if(ota_mcu_ctx->auto_ota.auto_enable) {
				miio_ota_mcu_erase_fw(ota_mcu_ctx);
			} else
#endif /* MIIO_AUTO_OTA_ENABLE */
			{
				miio_ota_mcu_buffer_fw(ota_mcu_ctx);
			}
			miio_ota_task_install_error(ota_mcu_ctx->ota_task_handle, MIIO_OTA_ERR_CODE_INSTALL , MIIO_OTA_ERR_INFO_INSTALL);
		}
		else{
			miio_ota_mcu_erase_fw(ota_mcu_ctx);
			miio_ota_task_install_finish(ota_mcu_ctx->ota_task_handle);
		}
		return MIIO_OK;
	}

	return MIIO_ERROR;
}

static int ota_mcu_handle(mcmd_t *mcmd)
{
	if(mcmd->ota_mcu_ctx.valid){
		mcmd->down.ts = arch_os_ms_now();
		mcmd->down.method_ctx.timeout = 6000;
		mcmd->down.result = ota_mcu_result;
		mcmd->down.error = ota_mcu_error;
		return MIIO_OK;
	}

	return MIIO_ERROR_NOTFOUND;
}


static int json_result_default(mcmd_t *mcmd, char *params)
{
	if (params) {
		uint32_t method_id = 0;
		char *js = params;
		int js_len = strlen(js);
		jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(js, js_len);
		if(MIIO_OK == jsmi_parse_start(&jsmi_parser)){
			// /params
			{
				jsmi_tok_path_t path[] = {
					{
						.key = "id",
						.type = JSMN_PRIMITIVE
					}
				};
				if(MIIO_OK != jsmi_get_value_uint32(&jsmi_parser, NULL, path, NELEMENTS(path), &method_id)){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "\"id\" not found");
				}
			}
			jsmi_parse_finish(&jsmi_parser);
		}

		if(method_id == mcmd->down.method_ctx.arg.id){
			miio_rpc_delegate_arg_t ack_arg = {
				.id = method_id,
				.type = MIIO_DELEGATE_JSON,
				.pload_len = js_len,
				.pload = js
			};

			return mcmd->down.method_ctx.ack(&ack_arg, mcmd->down.method_ctx.ack_ctx);
		}
	}

	mcmd_rpc_error_default(mcmd, MIIO_USER_ERR_CODE_ACK_INVALID, MIIO_USER_ERR_INFO_ACK_INVALID);

	return MIIO_ERROR;
}


static int json_error_default(mcmd_t *mcmd, int code, char *message)
{
	return mcmd_rpc_error_default(mcmd, code, message);
}

static int json_rpc_handle_default(mcmd_t *mcmd, const char* method, size_t method_len, const char *params, size_t params_len)
{
	mcmd_composer_t mcmd_composer = MCMD_COMPOSER_INIT(mcmd->command_buf, mcmd->command_buf_size);
	mcmd_compose_start(&mcmd_composer, "json_down");
	mcmd_compose_bytes(&mcmd_composer, params, params_len);
	if(MIIO_OK == mcmd_compose_finish(&mcmd_composer, NULL, NULL)){
		mcmd->down.result = json_result_default;
		mcmd->down.error = json_error_default;
		mcmd->io.out(mcmd->io.handle, mcmd->command_buf);
		return MIIO_OK;
	}
	return MIIO_ERROR;
}

int mcmd_rpc_get(mcmd_t *mcmd, int raw_rpc)
{
#if MIIO_AUTO_OTA_ENABLE
	miio_ota_mcu_ctx_t* ota_mcu_ctx = &mcmd->ota_mcu_ctx;
#endif /* MIIO_AUTO_OTA_ENABLE */

	//handle call ack first
	if(mcmd->up.state > MCMD_UP_IDLE){
		if(MCMD_UP_HAD_RESULT == mcmd->up.state){
			mcmd_composer_t mcmd_composer = MCMD_COMPOSER_INIT(mcmd->command_buf, mcmd->command_buf_size);
			if(raw_rpc)
				mcmd_compose_start(&mcmd_composer, "json_down");
			else
				mcmd_compose_start(&mcmd_composer, "down");
			mcmd_compose_bytes(&mcmd_composer, mcmd->up.buf, strnlen(mcmd->up.buf, mcmd->up.buf_size));
			mcmd_compose_finish(&mcmd_composer, NULL, NULL);
			mcmd->io.out(mcmd->io.handle, mcmd->command_buf);
			mcmd->up.state = MCMD_UP_IDLE;
		}
		else if(MCMD_UP_HAD_ERROR == mcmd->up.state){
			mcmd_composer_t mcmd_composer = MCMD_COMPOSER_INIT(mcmd->command_buf, mcmd->command_buf_size);
			if(raw_rpc)
				mcmd_compose_start(&mcmd_composer, "json_down");
			else
				mcmd_compose_start(&mcmd_composer, "down");
			mcmd_compose_bytes(&mcmd_composer, "error", sizeof("error")-1);
			mcmd_compose_finish(&mcmd_composer, NULL, NULL);
			mcmd->io.out(mcmd->io.handle, mcmd->command_buf);
			mcmd->up.state = MCMD_UP_IDLE;
		}
		else{
			if(raw_rpc)
				mcmd->io.out(mcmd->io.handle, "json_down none");
			else
				mcmd->io.out(mcmd->io.handle, "down none");
		}
		return 1;
	}

	//check last down method is timout or not
	if(mcmd->down.state > MCMD_DOWN_IDLE){
		if(arch_os_ms_elapsed(mcmd->down.ts) > mcmd->down.method_ctx.timeout){
			if(mcmd->down.error){
				mcmd->down.error(mcmd, MIIO_USER_ERR_CODE_ACK_TIMEOUT, MIIO_USER_ERR_INFO_ACK_TIMEOUT);
			}
			mcmd->down.state = MCMD_DOWN_IDLE;
		}
		else if(MCMD_DOWN_OTA == mcmd->down.state){
			if(raw_rpc) {
				mcmd->io.out(mcmd->io.handle, "error");
			} else {
#if MIIO_AUTO_OTA_ENABLE
				if(ota_mcu_ctx->auto_ota.force_enable) {
					 mcmd->io.out(mcmd->io.handle, "down update_fw force");
				} else if(ota_mcu_ctx->auto_ota.auto_enable) {
					mcmd->io.out(mcmd->io.handle, "down update_fw auto");
				} else
#endif /* MIIO_AUTO_OTA_ENABLE */
				{
					mcmd->io.out(mcmd->io.handle, "down update_fw");
				}
			}

			return 1;
		}

		mcmd->io.out(mcmd->io.handle, "error");
		return 1;
	}
	else{
		//get rpc
		memset(&mcmd->down.method_ctx, 0, sizeof(miio_rpc_delegate_context_t));

		fp_cmd_rpc_t cmd_rpc = NULL;
		const char *method = NULL;
		size_t method_len = 0;
		const char *params = NULL;
		size_t params_len = 0;

		mcmd->down.ts = arch_os_ms_now();
		mcmd->down.method_ctx.arg.pload = mcmd->down.buf;
		mcmd->down.method_ctx.arg.pload_len = mcmd->down.buf_size;

		if(MIIO_OK == miio_get_down_rpc_delegate_ext(mcmd->miio_handle, &(mcmd->down.method_ctx))){

			LOG_DEBUG_TAG(MIIO_LOG_TAG, "ext rpc[id=%d]", mcmd->down.method_ctx.arg.id);

			if(mcmd->down.method_ctx.arg.type == MIIO_DELEGATE_JSON){
				if(raw_rpc){
					cmd_rpc = json_rpc_handle_default;
					params = mcmd->down.method_ctx.arg.pload;
					params_len = mcmd->down.method_ctx.arg.pload_len;
				}
				else{
					jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(mcmd->down.method_ctx.arg.pload, mcmd->down.method_ctx.arg.pload_len);
					if(MIIO_OK == jsmi_parse_start(&jsmi_parser)){
						// /method
						{
							jsmntok_t *method_tok = NULL;
							jsmi_tok_path_t path[] = {
								{
									.key = "method",
									.type = JSMN_STRING
								}
							};
							method_tok = jsmi_get_tok(&jsmi_parser, NULL, path, NELEMENTS(path));
							if (method_tok) {
								method = jsmi_parser.js + method_tok->start;
								method_len = method_tok->end - method_tok->start;
							}
							else{
								LOG_ERROR_TAG(MIIO_LOG_TAG, "json parse err, no method found");
							}
						}

						// /params
						{
							jsmntok_t *params_tok = NULL;
							jsmi_tok_path_t path[] = {
								{
									.key = "params",
									.type = JSMN_UNDEFINED
								}
							};

							params_tok = jsmi_get_tok(&jsmi_parser, NULL, path, NELEMENTS(path));
							if (params_tok) {
								if(JSMN_STRING == params_tok->type){
									params = jsmi_parser.js + params_tok->start - 1;
									params_len = params_tok->end - params_tok->start + 2;
								}
								else{
									params = jsmi_parser.js + params_tok->start;
									params_len = params_tok->end - params_tok->start;
								}
							}
							else{
								LOG_ERROR_TAG(MIIO_LOG_TAG, "json parse err, no params found");
							}
						}
						jsmi_parse_finish(&jsmi_parser);
					}
					else{
						LOG_ERROR_TAG(MIIO_LOG_TAG, "jsmi_parser_init err.");
					}

					if(method && params){
						cmd_rpc = find_cmdrpc(method, method_len);
					}
				}
			}
		}

		if(cmd_rpc){
			if(MIIO_OK == cmd_rpc(mcmd, method, method_len, params, params_len)){
				mcmd->down.state = MCMD_DOWN_RPC;
				return 1;
			}
		}
	}

	if(MIIO_OK == ota_mcu_handle(mcmd)){
		mcmd->down.state = MCMD_DOWN_OTA;
		if(raw_rpc) {
			mcmd->io.out(mcmd->io.handle, "error");
		} else {
#if MIIO_AUTO_OTA_ENABLE
			if(ota_mcu_ctx->auto_ota.force_enable) {
				mcmd->io.out(mcmd->io.handle, "down update_fw force");
			} else if(ota_mcu_ctx->auto_ota.auto_enable) {
				mcmd->io.out(mcmd->io.handle, "down update_fw auto");
			} else
#endif /* MIIO_AUTO_OTA_ENABLE */
			{
				mcmd->io.out(mcmd->io.handle, "down update_fw");
			}
		}
		return 1;
	}

	return 0;
}
int mcmd_rpc_result(mcmd_t *mcmd, char* params)
{
	LOG_DEBUG_TAG(MIIO_LOG_TAG, "state=%d, timeout=%d, ts=%d, now=%d", mcmd->down.state, mcmd->down.method_ctx.timeout, mcmd->down.ts, arch_os_ms_now());

	//check last down method is timout or not
	if(mcmd->down.state > MCMD_DOWN_IDLE){
		int ret = MIIO_ERROR;
		if(arch_os_ms_elapsed(mcmd->down.ts) > mcmd->down.method_ctx.timeout){
			if(mcmd->down.error){
				mcmd->down.error(mcmd, MIIO_USER_ERR_CODE_ACK_TIMEOUT, MIIO_USER_ERR_INFO_ACK_TIMEOUT);
			}
		}
		else{
			if(mcmd->down.result){
				ret = mcmd->down.result(mcmd, params);
			}
		}

		if(MIIO_OK == ret){
			if(MCMD_DOWN_OTA != mcmd->down.state){
				mcmd->io.out(mcmd->io.handle, "ok");
			}
		}
		else{
			mcmd->io.out(mcmd->io.handle, "error");
		}
		mcmd->down.state = MCMD_DOWN_IDLE;
		return 1;
	}

	return 0;
}

static int verify_as_string(const char * str, size_t len)
{
    const char * str_head = str;
    const char * str_tail = str + len - 1;
    const char * p = str_head + 1;

    if(str_tail <= str_head)
        return MIIO_ERROR_PARAM;

    if(*str_head != '"' || *str_tail != '"' || (*str_tail == '"' && *(str_tail-1) == '\\'))
        return MIIO_ERROR_PARAM;

    while(p < str_tail) {
        if(*p < 32 || *p > 126 || (*p == '"' && *(p-1) != '\\'))
            return MIIO_ERROR_PARAM;
        p++;
    }

    return MIIO_OK;
}

int mcmd_rpc_error(mcmd_t *mcmd, char* params)
{
	if(mcmd->down.state > MCMD_DOWN_IDLE){
		int ret = MIIO_ERROR;
		if(arch_os_ms_elapsed(mcmd->down.ts) > mcmd->down.method_ctx.timeout){
			if(mcmd->down.error){
				mcmd->down.error(mcmd, MIIO_USER_ERR_CODE_ACK_TIMEOUT, MIIO_USER_ERR_INFO_ACK_TIMEOUT);
			}
		}
		else{
			char *argv[2] = {NULL};
			int argc = mcmd_parse_params(params, argv, NELEMENTS(argv), " ,");
			if(argc > 0){
				int argv_len = strlen(argv[0]);
				if(argv_len > 2 && MIIO_OK == verify_as_string(argv[0], argv_len)){
					*(argv[0]) = '\0';
					*(argv[0]+argv_len-1) = '\0';
					argv[0] += 1;
				}
				else{
					argv[0] = NULL;
				}
			}
			{
				char *message = argv[0] ? argv[0] : MIIO_USER_ERR_INFO_UNDEF_ERROR;
				int code = argv[1] ? atoi(argv[1]) : MIIO_USER_ERR_CODE_UNDEF_ERROR;
				if(mcmd->down.error){
					ret = mcmd->down.error(mcmd, code, message);
				}
			}
		}

		if(MIIO_OK == ret){
			mcmd->io.out(mcmd->io.handle, "ok");
		}
		else{
			mcmd->io.out(mcmd->io.handle, "error");
		}

		mcmd->down.state = MCMD_DOWN_IDLE;
		return 1;
	}

	return 0;
}

static int json_send_ack(miio_rpc_delegate_arg_t *ack_arg, void* ctx)
{
	LOG_DEBUG_TAG(MIIO_LOG_TAG, "%s called", __FUNCTION__);

	mcmd_t *mcmd = (mcmd_t *)ctx;

	mcmd_composer_t mcmd_composer = MCMD_COMPOSER_INIT(mcmd->up.buf, mcmd->up.buf_size);
	mcmd_compose_start(&mcmd_composer, "result");
	mcmd_compose_bytes(&mcmd_composer, ack_arg->pload, ack_arg->pload_len);
	if(MIIO_OK == mcmd_compose_finish(&mcmd_composer, NULL, NULL)){
		mcmd->up.state = MCMD_UP_HAD_RESULT;
	}
	else{
		mcmd->up.state = MCMD_UP_HAD_ERROR;
	}

    return MIIO_OK;
}

static void do_json_send(mcmd_t *mcmd, char *params)
{
	if(mcmd->up.state > MCMD_UP_IDLE){
		mcmd->io.out(mcmd->io.handle, "error");
		return;
	}

    if (NULL == params) {
    	mcmd->io.out(mcmd->io.handle, "error");
        return;
    }

    char *js = params;
    size_t js_len = strlen(js);
	uint32_t method_id = 0;
	jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(js, js_len);
	if(MIIO_OK == jsmi_parse_start(&jsmi_parser)){
		// /params
		{
			jsmi_tok_path_t path[] = {
				{
					.key = "id",
					.type = JSMN_PRIMITIVE
				}
			};
			if(MIIO_OK != jsmi_get_value_uint32(&jsmi_parser, NULL, path, NELEMENTS(path), &method_id)){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "\"id\" not found");
			}
		}

		jsmi_parse_finish(&jsmi_parser);
	}

    if(method_id){
		miio_rpc_delegate_context_t context;
		miio_rpc_context_init(&context);
		miio_rpc_context_config_delegate_arg(&context, method_id, MIIO_DELEGATE_JSON, js, js_len);
		miio_rpc_context_config_delegate_ack(&context, json_send_ack, mcmd);
		if (MIIO_OK == miio_set_up_rpc_delegate(mcmd->miio_handle, &context)) {
			mcmd->up.state = MCMD_UP_RPC;
			mcmd->io.out(mcmd->io.handle, "ok");
			return;
		}
    }

    mcmd->io.out(mcmd->io.handle, "error");

    return;
}
MIIO_CMD(json_send, do_json_send, NULL);

static int log_ack(miio_rpc_delegate_arg_t *ack_arg, void* ctx)
{
	LOG_DEBUG_TAG(MIIO_LOG_TAG, "%s called", __FUNCTION__);

	//mcmd_t *mcmd = (mcmd_t *)ctx;

	return MIIO_OK;
}

static void do_log(mcmd_t *mcmd, char *params)
{
    char *argv[MCMD_COMMAND_ARG_MAX];
    int argc = mcmd_parse_params(params, argv, NELEMENTS(argv), " ,");
    if (argc < 2) {
    	mcmd->io.out(mcmd->io.handle, "error");
        return;
    }

    if(MIIO_OK != jsmi_verify_key(argv[0])){
    	mcmd->io.out(mcmd->io.handle, "error");
    	return;
    }

    for(int i=1; i < argc; i++){
		if(MIIO_OK != jsmi_verify_value(argv[i])){
			mcmd->io.out(mcmd->io.handle, "error");
			return;
		}
	}

    uint32_t method_id = miio_get_rpc_id(mcmd->miio_handle);
    char* js = mcmd->down.buf;
    size_t js_size = MIN(mcmd->down.buf_size, miio_get_rpc_max_size(mcmd->miio_handle));
    size_t js_len = 0;
	jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, js_size);
	jsmi_compose_start(&jsmi_composer);
	jsmi_set_object_begin(&jsmi_composer);
		jsmi_set_key_value_uint(&jsmi_composer, "id", method_id);
		jsmi_set_key_value_str(&jsmi_composer, "method", "_otc.log", sizeof("_otc.log")-1);
		jsmi_set_key_object_begin(&jsmi_composer, "params");
			jsmi_set_key_array_begin(&jsmi_composer, argv[0]);
			for(int i=1; i < argc; i++){
				jsmi_set_value_bytes(&jsmi_composer, argv[i], strlen(argv[i]));
			}
			jsmi_set_key_array_end(&jsmi_composer);
		jsmi_set_key_object_end(&jsmi_composer);
	jsmi_set_object_end(&jsmi_composer);
	if(MIIO_OK == jsmi_compose_finish(&jsmi_composer, NULL, &js_len)){

		miio_rpc_delegate_context_t context;
		miio_rpc_context_init(&context);
		miio_rpc_context_config_delegate_arg(&context, method_id, MIIO_DELEGATE_JSON, js, js_len);
		miio_rpc_context_config_delegate_ack(&context, log_ack, mcmd);

		if (MIIO_OK == miio_set_up_rpc_delegate(mcmd->miio_handle, &context)){
			mcmd->io.out(mcmd->io.handle, "ok");
			return;
		}
	}

    mcmd->io.out(mcmd->io.handle, "error");

    return;
}
MIIO_CMD(log, do_log, NULL);

static int store_ack(miio_rpc_delegate_arg_t *ack_arg, void* ctx)
{
	LOG_DEBUG_TAG(MIIO_LOG_TAG, "%s called", __FUNCTION__);

	//mcmd_t *mcmd = (mcmd_t *)ctx;

	return MIIO_OK;
}

static void do_store(mcmd_t *mcmd, char *params)
{
    char *argv[MCMD_COMMAND_ARG_MAX];
    int argc = mcmd_parse_params(params, argv, NELEMENTS(argv), " ,");
    if (argc < 2) {
    	mcmd->io.out(mcmd->io.handle, "error");
        return;
    }

    if(MIIO_OK != jsmi_verify_key(argv[0])){
		mcmd->io.out(mcmd->io.handle, "error");
		return;
	}

	for(int i=1; i < argc; i++){
		if(MIIO_OK != jsmi_verify_value(argv[i])){
			mcmd->io.out(mcmd->io.handle, "error");
			return;
		}
	}

    uint32_t method_id = miio_get_rpc_id(mcmd->miio_handle);
	char* js = mcmd->down.buf;
	size_t js_size = MIN(mcmd->down.buf_size, miio_get_rpc_max_size(mcmd->miio_handle));
	size_t js_len = 0;
	jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, js_size);
	jsmi_compose_start(&jsmi_composer);
	jsmi_set_object_begin(&jsmi_composer);
		jsmi_set_key_value_uint(&jsmi_composer, "id", method_id);
		jsmi_set_key_value_str(&jsmi_composer, "method", "_async.store", sizeof("_async.store")-1);
		jsmi_set_key_object_begin(&jsmi_composer, "params");
			jsmi_set_key_array_begin(&jsmi_composer, argv[0]);
			for(int i=1; i < argc; i++){
				jsmi_set_value_bytes(&jsmi_composer, argv[i], strlen(argv[i]));
			}
			jsmi_set_key_array_end(&jsmi_composer);
		jsmi_set_key_object_end(&jsmi_composer);
	jsmi_set_object_end(&jsmi_composer);
	if(MIIO_OK == jsmi_compose_finish(&jsmi_composer, NULL, &js_len)){

		miio_rpc_delegate_context_t context;
		miio_rpc_context_init(&context);
		miio_rpc_context_config_delegate_arg(&context, method_id, MIIO_DELEGATE_JSON, js, js_len);
		miio_rpc_context_config_delegate_ack(&context, store_ack, mcmd);

		if (MIIO_OK == miio_set_up_rpc_delegate(mcmd->miio_handle, &context)){
			mcmd->io.out(mcmd->io.handle, "ok");
			return;
		}
	}

	mcmd->io.out(mcmd->io.handle, "error");

    return;
}
MIIO_CMD(store, do_store, NULL);

static int call_ack(miio_rpc_delegate_arg_t *ack_arg, void* ctx)
{
	mcmd_t *mcmd = (mcmd_t *)ctx;
	const char *result = NULL;
	size_t result_len = 0;

	jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(ack_arg->pload, ack_arg->pload_len);
	if(MIIO_OK == jsmi_parse_start(&jsmi_parser)){
		// /result
		{
			jsmi_tok_path_t path[] = {
				{
					.key = "result",
					.type = JSMN_UNDEFINED
				}
			};
			jsmntok_t *result_tok = jsmi_get_tok(&jsmi_parser, NULL, path, NELEMENTS(path));
			if(result_tok){
				if(JSMN_STRING == result_tok->type){
					result = jsmi_parser.js + result_tok->start - 1;
					result_len = result_tok->end - result_tok->start + 2;
				}
				else{
					result = jsmi_parser.js + result_tok->start;
					result_len = result_tok->end - result_tok->start;
				}
			}
		}

		jsmi_parse_finish(&jsmi_parser);
	}

	if(result && result_len){
		mcmd_composer_t mcmd_composer = MCMD_COMPOSER_INIT(mcmd->up.buf, mcmd->up.buf_size);
		mcmd_compose_start(&mcmd_composer, "result");
		mcmd_compose_bytes(&mcmd_composer, result, result_len);
		if(MIIO_OK == mcmd_compose_finish(&mcmd_composer, NULL, NULL)){
			mcmd->up.state = MCMD_UP_HAD_RESULT;
			return MIIO_OK;
		}
	}

	mcmd->up.state = MCMD_UP_HAD_ERROR;

    return MIIO_OK;
}

static void do_call(mcmd_t *mcmd, char *params)
{
	if(mcmd->up.state > MCMD_UP_IDLE){
		mcmd->io.out(mcmd->io.handle, "error");
		return;
	}

    char *argv[MCMD_COMMAND_ARG_MAX];
    int argc = mcmd_parse_params(params, argv, NELEMENTS(argv), " ,");
    if (argc < 1) {
    	mcmd->io.out(mcmd->io.handle, "error");
        return;
    }

    uint32_t method_id = miio_get_rpc_id(mcmd->miio_handle);
    char method[64] = {0};
    snprintf(method, sizeof(method), "_sync.%s", argv[0]);
    char* js = mcmd->up.buf;
    size_t js_size = MIN(mcmd->up.buf_size, miio_get_rpc_max_size(mcmd->miio_handle));
	size_t js_len = 0;

	jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, js_size);
	jsmi_compose_start(&jsmi_composer);
	jsmi_set_object_begin(&jsmi_composer);
		jsmi_set_key_value_uint(&jsmi_composer, "id", method_id);
		jsmi_set_key_value_str(&jsmi_composer, "method", method, strlen(method));
		jsmi_set_key_object_begin(&jsmi_composer, "params");
			for(int i=1; i < argc; i++){
				jsmi_set_value_bytes(&jsmi_composer, argv[i], strlen(argv[i]));
			}
		jsmi_set_key_object_end(&jsmi_composer);
	jsmi_set_object_end(&jsmi_composer);
	if(MIIO_OK == jsmi_compose_finish(&jsmi_composer, NULL, &js_len)){

		LOG_DEBUG_TAG(MIIO_LOG_TAG, "call=%s", js);

		miio_rpc_delegate_context_t context;
		miio_rpc_context_init(&context);
		miio_rpc_context_config_delegate_arg(&context, method_id, MIIO_DELEGATE_JSON, js, js_len);
		miio_rpc_context_config_delegate_ack(&context, call_ack, mcmd);

		if (MIIO_OK == miio_set_up_rpc_delegate(mcmd->miio_handle, &context)){
			mcmd->io.out(mcmd->io.handle, "ok");
			return;
		}
	}

	mcmd->io.out(mcmd->io.handle, "error");

    return;
}
MIIO_CMD(call, do_call, NULL);
