#include "miio_ota_app.h"
#include "miio_ciphers.h"
#include "jsmi.h"

#undef MIIO_LOG_TAG
#define MIIO_LOG_TAG			"ota_app"

static int app_fw_hash_init(miio_ota_app_ctx_t *ota_app_ctx, size_t file_len)
{
	if(ota_app_ctx->signed_file){
		if(file_len < ota_app_ctx->original_file_len){
			return MIIO_ERROR_PARAM;
		}
		else{
			memset(&ota_app_ctx->verify, 0, sizeof(ota_app_ctx->verify));
			ota_app_ctx->verify.file_len = file_len;
			ota_app_ctx->verify.sha256_ctx = miio_sha256_init();
			if(NULL == ota_app_ctx->verify.sha256_ctx){
				return MIIO_ERROR_NOMEM;
			}
		}
	}
	else{
		ota_app_ctx->original_file_len = file_len;
	}

	return MIIO_OK;
}

static void app_fw_hash_update(miio_ota_app_ctx_t *ota_app_ctx, uint32_t offset, uint8_t* data, size_t data_len)
{
	if(ota_app_ctx->signed_file){
		if(offset < ota_app_ctx->original_file_len){
			size_t sha_len = ota_app_ctx->original_file_len - offset;
			if(sha_len > data_len)
				sha_len = data_len;

			miio_sha256_update(ota_app_ctx->verify.sha256_ctx, data, sha_len);
		}
	}
}


static void app_fw_hash_finish(miio_ota_app_ctx_t *ota_app_ctx)
{
	if(ota_app_ctx->signed_file){
		miio_sha256_finish(ota_app_ctx->verify.sha256_ctx, ota_app_ctx->verify.hash);
		ota_app_ctx->verify.sha256_ctx = NULL;
	}
}

static int app_fw_signature_verify(miio_ota_app_ctx_t *ota_app_ctx)
{
	int ret = MIIO_OK;

	if(ota_app_ctx->signed_file){

		uint8_t *ext = NULL;
		uint32_t ext_len = 0;
		uint8_t *pems = NULL;
		int pems_size = 0;
		uint8_t *sign = NULL;
		int sign_len = 0;

		LOG_DEBUG_TAG(MIIO_LOG_TAG, "verify begin...");

		//parase ext
		ext_len = ota_app_ctx->verify.file_len - ota_app_ctx->original_file_len;

		LOG_DEBUG_TAG(MIIO_LOG_TAG, "ext_len[%d]", ext_len);

		ext = malloc(ext_len + 1);
		if(NULL == ext){
			ret = MIIO_ERROR_NOMEM;
			goto safe_exit;
		}
		ext[ext_len] = '\0';

		ret = arch_ota_read_flash(&ota_app_ctx->flash_info, ota_app_ctx->original_file_len, ext, ext_len);
		if(MIIO_OK != ret){
			goto safe_exit;
		}

		{
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "pems finding...");

			//find perms and sign
			char *footer = "-----END CERTIFICATE-----";
			uint8_t *end;
			end = (uint8_t*)strrstr((char*)ext, footer);
			if(NULL == end){
				ret = MIIO_ERROR_PARAM;
				goto safe_exit;
			}

			end += strlen(footer);
			if( *end == ' '  ) end++;
			if( *end == '\r' ) end++;
			if( *end == '\n' ) end++;

			//insert '\0' to *end and shift sign
			uint8_t *start = ext + ext_len;
			while(start > end){
				*start = *(start-1);
				start--;
			}
			*end = '\0';

			pems = ext;
			pems_size = end + 1 - pems;
			sign = end + 1;
			sign_len = ext + ext_len + 1 - sign;

			LOG_DEBUG_TAG(MIIO_LOG_TAG, "pems = %s", pems);
		}

		LOG_DEBUG_TAG(MIIO_LOG_TAG, "pems verify...");

		ret = miio_signature_verify(pems, pems_size, ota_app_ctx->verify.org, ota_app_ctx->verify.hash, sizeof(ota_app_ctx->verify.hash), sign, sign_len);

		if(MIIO_OK == ret){
			ret = arch_ota_check_crc(&ota_app_ctx->flash_info, 0, ota_app_ctx->original_file_len);
		}
safe_exit:

		if(ext){
			free(ext);
		}
	}
	else{
		ret = arch_ota_check_crc(&ota_app_ctx->flash_info, 0, ota_app_ctx->original_file_len);
	}

	return ret;
}

static int app_fw_init(miio_ota_task_handle_t task_handle, void* pctx, miio_delegate_type_t type, const void* params, size_t params_len)
{
	int ret = MIIO_OK;
	miio_ota_app_ctx_t *ota_app_ctx = (miio_ota_app_ctx_t*)pctx;
	ota_app_ctx->signed_file = false;
	ota_app_ctx->original_file_len = 0;
	miio_download_params_t *download_params = miio_ota_task_get_download_params(task_handle);
	download_params->url[0] = '\0';
	download_params->hostip[0] = '\0';

	if(MIIO_DELEGATE_JSON == type){
		jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT((const char*)params, params_len);
		if(MIIO_OK == jsmi_parse_start(&jsmi_parser)){
			// /app_url
			{
				jsmi_tok_path_t path[] = {
					{
						.key = "app_url",
						.type = JSMN_STRING
					}
				};
				if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, NULL, path, NELEMENTS(path), download_params->url, sizeof(download_params->url))){
					LOG_DEBUG_TAG(MIIO_LOG_TAG, "\"app_url\" not found");
					ret = MIIO_ERROR_PARAM;
					goto jsmi_parse_exit;
				}

				if(0 == strnlen(download_params->url, sizeof(download_params->url))){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "\"app_url\" is empty");
					ret = MIIO_ERROR_PARAM;
					goto jsmi_parse_exit;
				}
			}

			// /app_hostip
			{
				jsmi_tok_path_t path[] = {
					{
						.key = "app_hostip",
						.type = JSMN_STRING
					}
				};
				if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, NULL, path, NELEMENTS(path), download_params->hostip, sizeof(download_params->hostip))){
					download_params->hostip[0] = '\0';
				}
			}

			// /signed_file
			{
				jsmi_tok_path_t path[] = {
					{
						.key = "signed_file",
						.type = JSMN_PRIMITIVE
					}
				};

				jsmi_get_value_bool(&jsmi_parser, NULL, path, NELEMENTS(path), &ota_app_ctx->signed_file);
			}

			if(ota_app_ctx->signed_file){
				jsmi_tok_path_t path[] = {
					{
						.key = "original_length",
						.type = JSMN_PRIMITIVE
					}
				};

				if(MIIO_OK != jsmi_get_value_uint32(&jsmi_parser, NULL, path, NELEMENTS(path), &ota_app_ctx->original_file_len)){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "\"original_length\" not found when \"signed_file\" is true");
					ret = MIIO_ERROR_PARAM;
					goto jsmi_parse_exit;
				}

				LOG_DEBUG_TAG(MIIO_LOG_TAG, "signed_file: %d, original_length: %d", ota_app_ctx->signed_file, ota_app_ctx->original_file_len);
			}

			//if has special root cert, else use default
			//download_params->root_cert_pem = pem;

			// /partial_enable
			{
				jsmi_tok_path_t path[] = {
					{
						.key = "partial_enable",
						.type = JSMN_PRIMITIVE
					}
				};

				jsmi_get_value_bool(&jsmi_parser, NULL, path, NELEMENTS(path), &download_params->partial_enable);
			}

#if MIIO_AUTO_OTA_ENABLE
			{ /* parse auto ota parameters */
				bool ota_force_enable = false;
				jsmi_tok_path_t path[] = {
					{
						.key = "app_force",
						.type = JSMN_PRIMITIVE
					}
				};

				/* auto ota process */
				if(MIIO_OK == jsmi_get_value_bool(&jsmi_parser, NULL, path, NELEMENTS(path), &ota_force_enable)) {
					ota_app_ctx->auto_ota.auto_enable = true;
					ota_app_ctx->auto_ota.force_enable = ota_force_enable;
				}
				/* normal ota process */
				else {
					ota_app_ctx->auto_ota.auto_enable = false;
					ota_app_ctx->auto_ota.force_enable = false;
				}

				LOG_INFO_TAG(MIIO_LOG_TAG, "auto enable: %d, force enable: %d", ota_app_ctx->auto_ota.auto_enable, ota_app_ctx->auto_ota.force_enable);
			}
#endif /* MIIO_AUTO_OTA_ENABLE */

			LOG_INFO_TAG(MIIO_LOG_TAG, "app ota coming...");

jsmi_parse_exit:

			jsmi_parse_finish(&jsmi_parser);
		}
		else{
			ret = MIIO_ERROR_NOMEM;
		}
	}
	else{
		ret = MIIO_ERROR_PARAM;
	}

	return ret;
}


static int app_fw_version(miio_ota_task_handle_t task_handle, void* ctx, miio_delegate_type_t type, void *composer)
{//fw_ver
	jsmi_composer_t *jsmi_composer = composer;
#if MIIO_APP_VERSION_NUMBER
	jsmi_set_key_value_str(jsmi_composer, "fw_ver", miio_app_version(MIIO_APP_VERSION_NUMBER), 0);
#else
	jsmi_set_key_value_str(jsmi_composer, "fw_ver", miio_instance_version(), 0);
#endif
	return MIIO_OK;
}


static int app_fw_open(miio_ota_task_handle_t task_handle, void* pctx, size_t content_len)
{
	int ret;
	miio_ota_app_ctx_t *ota_app_ctx = (miio_ota_app_ctx_t*)pctx;
	ota_flash_info_t *pflash = &ota_app_ctx->flash_info;

	ret = app_fw_hash_init(ota_app_ctx, content_len);
	if(MIIO_OK != ret)
		return ret;

	//init flash
	ret = arch_ota_select_flash(pflash, content_len);

	return ret;
}


//这里执行将下载的固件写入到flash中职能
static int app_fw_write(miio_ota_task_handle_t task_handle, void* pctx, uint32_t offset, uint8_t *pkt, size_t pkt_len)
{
	int ret;
	miio_ota_app_ctx_t *ota_app_ctx = (miio_ota_app_ctx_t*)pctx;
	ota_flash_info_t *pflash = &ota_app_ctx->flash_info;

	app_fw_hash_update(ota_app_ctx, offset, pkt, pkt_len);

	ret = arch_ota_write_flash_with_erase(pflash, offset, pkt, pkt_len);

	if(MIIO_OK != ret) {
		LOG_ERROR_TAG(MIIO_LOG_TAG, "write error, return %d",ret);
		return -1;
	}

	return pkt_len;
}

static int app_fw_close(miio_ota_task_handle_t task_handle, void* pctx)
{
	int ret = MIIO_OK;
	miio_ota_app_ctx_t *ota_app_ctx = (miio_ota_app_ctx_t*)pctx;
	ota_flash_info_t *pflash = &ota_app_ctx->flash_info;

	app_fw_hash_finish(ota_app_ctx);

	ret = app_fw_signature_verify(ota_app_ctx);

	if(MIIO_OK == ret){
		miio_ota_task_install_start(task_handle);

#if MIIO_AUTO_OTA_ENABLE
		int device_state = MIIO_AUTO_OTA_READY;
		struct {
			int *device_state;
		} auto_ota_hooks_ctx = { 0 };
		auto_ota_hooks_ctx.device_state = &device_state;

		if(ota_app_ctx->auto_ota.auto_enable && !(ota_app_ctx->auto_ota.force_enable)) {
			app_auto_ota_hook_default(NULL, auto_ota_hooks_ctx.device_state, NULL);
			if(MIIO_AUTO_OTA_BUSY == *(auto_ota_hooks_ctx.device_state)) {
				LOG_WARN_TAG(MIIO_LOG_TAG, "devie is busy for ota install");
				miio_ota_task_install_error(task_handle, MIIO_OTA_ERR_CODE_INSTALL_BUSY, MIIO_OTA_ERR_INFO_INSTALL_BUSY);
				goto exit;
			}
		}
#endif /* MIIO_AUTO_OTA_ENABLE */

		if(MIIO_OK != arch_ota_set_flash_boot(pflash)){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "close failed at arch_ota_set_flash_boot");
			miio_ota_task_install_error(task_handle, MIIO_OTA_ERR_CODE_INSTALL, MIIO_OTA_ERR_INFO_INSTALL);
		}
		else{
			miio_ota_task_install_finish(task_handle);
		}
	}

#if MIIO_AUTO_OTA_ENABLE
exit :
#endif /* MIIO_AUTO_OTA_ENABLE */
	return ret;
}


//表示连接异常中断，或者写flash出错
static int app_fw_error(miio_ota_task_handle_t task_handle, void* pctx)
{
	miio_ota_app_ctx_t *ota_app_ctx = (miio_ota_app_ctx_t*)pctx;

	app_fw_hash_finish(ota_app_ctx);

	return MIIO_OK;
}


int miio_ota_app_init(miio_handle_t miio_handle, miio_ota_app_ctx_t *ota_app_ctx)
{
	memset(ota_app_ctx, 0, sizeof(miio_ota_app_ctx_t));

	miio_get_model(miio_handle, ota_app_ctx->verify.org, MIIO_MODEL_SIZE_MAX);
	char *s = strchr(ota_app_ctx->verify.org, '.');
	if(s){*s = '\0';}

	const miio_ota_task_hook_if_t ota_app_hook_if = {
		.fp_init = app_fw_init,
		.fp_version = app_fw_version,
		.fp_open = app_fw_open,
		.fp_write = app_fw_write,
		.fp_close = app_fw_close,
		.fp_error = app_fw_error,
		.fp_deinit = NULL,
	};

	ota_app_ctx->ota_task_handle = miio_ota_task_attach(miio_handle, "ota_app", MIIO_OTA_TASK_IDLE, &ota_app_hook_if, ota_app_ctx);

	if(NULL == ota_app_ctx->ota_task_handle){
		LOG_ERROR_TAG(MIIO_LOG_TAG, "ota task attach error!");
		return MIIO_ERROR;
	}

	return MIIO_OK;
}
