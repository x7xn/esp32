
#include "miio_ota_mcu.h"
#include "xmodem.h"
#include "jsmi.h"

#undef MIIO_LOG_TAG
#define MIIO_LOG_TAG			"ota_mcu"
#define MIIO_MCU_OTA_THRESH     3

#define USER_FW_MAGIC			(('U' << 0)|('F' << 8)|('L' << 16)|('X' << 24))

char* miio_mcu_get_version(char *ver, int ver_len)
{
	memset(ver, 0, ver_len);

	arch_psm_get_str("ot_config", "mcu_ver", ver, ver_len);

	return ver;
}

void miio_mcu_set_version(const char *ver)
{
	int ver_len = 0;

	if(!ver || !(ver_len = strlen(ver))){
		arch_psm_erase_key("ot_config", "mcu_ver");
		return;
	}

	char mcu_ver[8] = {0};
	arch_psm_get_str("ot_config", "mcu_ver", mcu_ver, sizeof(mcu_ver));

	if( (strlen(mcu_ver) !=  ver_len) || (0 != strcmp(mcu_ver, ver)) ){
		arch_psm_set_str("ot_config", "mcu_ver", ver);
	}
}

int miio_ota_mcu_lock_init(miio_ota_mcu_ctx_t *pctx)
{
	if(MIIO_OK != arch_os_semaphore_create(&pctx->lock)){
		LOG_ERROR_TAG(MIIO_LOG_TAG, "lock create err");
		return MIIO_ERROR;
	}

	return MIIO_OK;
}

int miio_ota_mcu_lock(miio_ota_mcu_ctx_t *pctx)
{
	return arch_os_semaphore_get(pctx->lock, ARCH_OS_WAIT_FOREVER);
}

int miio_ota_mcu_lock_try(miio_ota_mcu_ctx_t *pctx)
{
	return arch_os_semaphore_get(pctx->lock, ARCH_OS_NO_WAIT);
}

int miio_ota_mcu_unlock(miio_ota_mcu_ctx_t *pctx)
{
	return arch_os_semaphore_put(pctx->lock);
}

int miio_ota_mcu_is_busy(miio_ota_mcu_ctx_t *pctx)
{
	bool busy = true;
	if(MIIO_OK == miio_ota_mcu_lock_try(pctx)){
		if(false == pctx->valid){
			busy = false;
		}
		miio_ota_mcu_unlock(pctx);
	}
	return busy;
}

bool miio_ota_mcu_has_fw(miio_ota_mcu_ctx_t *pctx)
{
	bool valid = false;
	if(MIIO_OK == miio_ota_mcu_lock_try(pctx)){
		valid = pctx->valid;
		miio_ota_mcu_unlock(pctx);
	}
	return valid;
}

int miio_ota_mcu_erase_fw(miio_ota_mcu_ctx_t *pctx)
{
	if(MIIO_OK == miio_ota_mcu_lock(pctx)){
		pctx->valid = false;

		if (pctx->mcu_ota_cnt > 0) {
			arch_psm_erase_key("ot_config", "mcu_ota_count");
			pctx->mcu_ota_cnt = 0;
		}

		arch_ota_erase_flash(&pctx->flash_info, 0, sizeof(pctx->flash_header));
		miio_ota_mcu_unlock(pctx);
		return MIIO_OK;
	}
	return MIIO_ERROR;
}

int miio_ota_mcu_buffer_fw(miio_ota_mcu_ctx_t *pctx)
{
	if(MIIO_OK == miio_ota_mcu_lock(pctx)){
		if(pctx->valid){
			pctx->valid = false;
		}
		miio_ota_mcu_unlock(pctx);
		return MIIO_OK;
	}
	return MIIO_ERROR;
}

static int ota_mcu_refresh(miio_ota_mcu_ctx_t *pctx)
{
	int ret;
	//check mcu fw exist or not
	do{
		ret = arch_ota_select_mcu_flash(&pctx->flash_info, 0);
		if(ret != MIIO_OK)
			break;

		ret = arch_ota_read_flash(&pctx->flash_info, 0, (uint8_t *)&pctx->flash_header, sizeof(pctx->flash_header));
		if(ret != MIIO_OK)
			break;

		if(pctx->flash_header.magic != USER_FW_MAGIC){
			ret = MIIO_ERROR_NOTFOUND;
			break;
		}

		ret = arch_ota_check_crc(&pctx->flash_info, sizeof(pctx->flash_header), pctx->flash_header.length);
		if(ret != MIIO_OK){
			arch_ota_erase_flash(&pctx->flash_info, 0, sizeof(pctx->flash_header));//擦除magic所在的块
			LOG_ERROR_TAG(MIIO_LOG_TAG, "check crc error in mcu fw");
			break;
		}

	}while(0);

	if(MIIO_OK == ret){
		LOG_INFO_TAG(MIIO_LOG_TAG, "valid mcu firmware was found, length is %d", pctx->flash_header.length);
		pctx->valid = true;
	}
	else{
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "no valid mcu firmware found");
		pctx->valid = false;
	}

	return ret;
}

int miio_ota_mcu_refresh_fw(miio_ota_mcu_ctx_t *pctx)
{
	int ret = MIIO_ERROR_NOTFOUND;
	if(MIIO_OK == miio_ota_mcu_lock_try(pctx)){
		ret = ota_mcu_refresh(pctx);
		miio_ota_mcu_unlock(pctx);
	}
	return ret;
}

static int mcu_fw_init(miio_ota_task_handle_t task_handle, void* pctx, miio_delegate_type_t type, const void* params, size_t params_len)
{
	if(miio_ota_mcu_is_busy((miio_ota_mcu_ctx_t*)pctx)){
		LOG_WARN_TAG(MIIO_LOG_TAG, "busy");
		return MIIO_ERROR_BUSY;
	}

	int ret = MIIO_OK;
	miio_download_params_t *download_params = miio_ota_task_get_download_params(task_handle);
	download_params->url[0] = '\0';
	download_params->hostip[0] = '\0';

	if(MIIO_DELEGATE_JSON == type){
		jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT((const char*)params, params_len);
		if(MIIO_OK == jsmi_parse_start(&jsmi_parser)){
			// /mcu_url
			{
				jsmi_tok_path_t path[] = {
					{
						.key = "mcu_url",
						.type = JSMN_STRING
					}
				};
				if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, NULL, path, NELEMENTS(path), download_params->url, sizeof(download_params->url))){
					LOG_DEBUG_TAG(MIIO_LOG_TAG, "\"mcu_url\" not found");
					ret = MIIO_ERROR_PARAM;
					goto jsmi_parse_exit;
				}

				if(0 == strnlen(download_params->url, sizeof(download_params->url))){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "\"mcu_url\" is empty");
					ret = MIIO_ERROR_PARAM;
					goto jsmi_parse_exit;
				}
			}

			// /mucu_hostip
			{
				jsmi_tok_path_t path[] = {
					{
						.key = "mcu_hostip",
						.type = JSMN_STRING
					}
				};
				if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, NULL, path, NELEMENTS(path), download_params->hostip, sizeof(download_params->hostip))){
					download_params->hostip[0] = '\0';
				}
			}

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
			miio_ota_mcu_ctx_t *ota_mcu_ctx = (miio_ota_mcu_ctx_t *)pctx;
			{ /* parse auto ota parameters */
				bool ota_force_enable = false;
				jsmi_tok_path_t path[] = {
					{
						.key = "mcu_force",
						.type = JSMN_PRIMITIVE
					}
				};

				/* auto ota process */
				if(MIIO_OK == jsmi_get_value_bool(&jsmi_parser, NULL, path, NELEMENTS(path), &ota_force_enable)) {
					ota_mcu_ctx->auto_ota.auto_enable = true;
					ota_mcu_ctx->auto_ota.force_enable = ota_force_enable;
				}
				/* normal ota process */
				else {
					ota_mcu_ctx->auto_ota.auto_enable = false;
					ota_mcu_ctx->auto_ota.force_enable = false;
				}
			}

			LOG_INFO_TAG(MIIO_LOG_TAG, "auto enable: %d, force enable: %d", ota_mcu_ctx->auto_ota.auto_enable, ota_mcu_ctx->auto_ota.force_enable);
#endif /* MIIO_AUTO_OTA_ENABLE */

			LOG_INFO_TAG(MIIO_LOG_TAG, "mcu ota coming...");

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



static int mcu_fw_version(miio_ota_task_handle_t task_handle, void* ctx, miio_delegate_type_t type, void *composer)
{//mcu_fw_ver
	jsmi_composer_t *jsmi_composer = composer;

	char mcu_ver[8] = {0};
	miio_mcu_get_version(mcu_ver, sizeof(mcu_ver));

	if(strlen(mcu_ver)){
		jsmi_set_key_value_str(jsmi_composer, "mcu_fw_ver", mcu_ver, strnlen(mcu_ver, sizeof(mcu_ver)));
	}

	return MIIO_ERROR_NOTFOUND;
}

static int mcu_fw_open(miio_ota_task_handle_t task_handle, void* pctx, size_t content_len)
{
	miio_ota_mcu_ctx_t* ota_mcu_ctx = (miio_ota_mcu_ctx_t*)pctx;

	if(MIIO_OK != miio_ota_mcu_lock_try(ota_mcu_ctx))
		return MIIO_ERROR;

	ota_mcu_ctx->flash_header.magic  = USER_FW_MAGIC;
	ota_mcu_ctx->flash_header.length = content_len;

	//init flash
	return arch_ota_select_mcu_flash(&ota_mcu_ctx->flash_info, content_len);
}

//这里执行将下载的固件写入到flash中职能，在升级mcu固件时，还需要进行写入头部和crc校验。
static int mcu_fw_write(miio_ota_task_handle_t task_handle, void* pctx, uint32_t offset, uint8_t *pkt, size_t pkt_len)
{
	miio_ota_mcu_ctx_t* ota_mcu_ctx = (miio_ota_mcu_ctx_t*)pctx;

	int ret;
	ret = arch_ota_write_flash_with_erase(&ota_mcu_ctx->flash_info, sizeof(ota_mcu_ctx->flash_header) + offset, pkt,  pkt_len);

	if(MIIO_OK != ret) {
		LOG_ERROR_TAG(MIIO_LOG_TAG, "write error, return %d",ret);
		return -1;
	}

	return pkt_len;
}

static int mcu_fw_close(miio_ota_task_handle_t task_handle, void* pctx)
{
	miio_ota_mcu_ctx_t* ota_mcu_ctx = (miio_ota_mcu_ctx_t*)pctx;

	int ret;

	ret = arch_ota_write_flash(&ota_mcu_ctx->flash_info, 0, (uint8_t*)&ota_mcu_ctx->flash_header, sizeof(ota_mcu_ctx->flash_header));

	if(MIIO_OK != ret){
		LOG_ERROR_TAG(MIIO_LOG_TAG, "write user_fw magic error, return %d",ret);
	}
	else{
		ret = ota_mcu_refresh(ota_mcu_ctx);
		if (MIIO_OK == ret) {
			if(ota_mcu_ctx->mcu_ota_cnt > 0) {
				arch_psm_erase_key("ot_config", "mcu_ota_count");
				ota_mcu_ctx->mcu_ota_cnt = 0;
			}
		}
	}

	miio_ota_mcu_unlock(ota_mcu_ctx);

	return ret;
}

//表示连接异常中断，或者写flash出错
static int mcu_fw_error(miio_ota_task_handle_t task_handle, void* pctx)
{
	miio_ota_mcu_ctx_t* ota_mcu_ctx = (miio_ota_mcu_ctx_t*)pctx;
	miio_ota_mcu_unlock(ota_mcu_ctx);
	return MIIO_OK;
}

int miio_ota_mcu_init(miio_handle_t miio_handle, miio_ota_mcu_ctx_t *ota_mcu_ctx)
{
	int ret = 0;
	memset(ota_mcu_ctx, 0, sizeof(miio_ota_mcu_ctx_t));

	if(MIIO_OK != miio_ota_mcu_lock_init(ota_mcu_ctx)){
		return MIIO_ERROR;
	}

	//check mcu fw exist or not
	miio_ota_mcu_refresh_fw(ota_mcu_ctx);

	if (ota_mcu_ctx->valid) {
		ret = arch_psm_get_value("ot_config", "mcu_ota_count", &ota_mcu_ctx->mcu_ota_cnt, sizeof(int));
		if (ret > 0) {
			if (ota_mcu_ctx->mcu_ota_cnt < MIIO_MCU_OTA_THRESH) {
				ota_mcu_ctx->mcu_ota_cnt++;
				arch_psm_set_value("ot_config", "mcu_ota_count", &ota_mcu_ctx->mcu_ota_cnt, sizeof(int));
			} else {
				ota_mcu_ctx->mcu_ota_cnt = 0;
				arch_psm_erase_key("ot_config", "mcu_ota_count");
				arch_ota_erase_flash(&ota_mcu_ctx->flash_info, 0, sizeof(ota_mcu_ctx->flash_header)); /* 擦除magic所在的块 */
				LOG_INFO_TAG(MIIO_LOG_TAG, "valid mcu firmware was found, but retry count larger than threshold");
				ota_mcu_ctx->valid = false;
			}
		} else {
			ota_mcu_ctx->mcu_ota_cnt++;
			arch_psm_set_value("ot_config", "mcu_ota_count", &ota_mcu_ctx->mcu_ota_cnt, sizeof(int));
		}
	}

	const miio_ota_task_hook_if_t ota_mcu_hook_if = {
		.fp_init = mcu_fw_init,
		.fp_version = mcu_fw_version,
		.fp_open = mcu_fw_open,
		.fp_write = mcu_fw_write,
		.fp_close = mcu_fw_close,
		.fp_error = mcu_fw_error,
		.fp_deinit = NULL,
	};

	ota_mcu_ctx->ota_task_handle = miio_ota_task_attach(miio_handle, "ota_mcu", ota_mcu_ctx->valid ? MIIO_OTA_TASK_DOWNLOADED : MIIO_OTA_TASK_IDLE, &ota_mcu_hook_if, ota_mcu_ctx);

	if(NULL == ota_mcu_ctx->ota_task_handle){
		LOG_ERROR_TAG(MIIO_LOG_TAG, "ota task attach error!");
		return MIIO_ERROR;
	}

	LOG_DEBUG_TAG(MIIO_LOG_TAG, "init ok");

	return MIIO_OK;
}


