#ifndef __OTA_APP_H
#define __OTA_APP_H

#include "miio_instance.h"
#include "miio_arch.h"

typedef struct{
	miio_ota_task_handle_t ota_task_handle;
	ota_flash_info_t flash_info;
	bool signed_file;
	uint32_t original_file_len;
	struct{
		uint32_t file_len;
		char org[MIIO_MODEL_SIZE_MAX];
		void* sha256_ctx;
		uint8_t hash[32];
	}verify;

#if MIIO_AUTO_OTA_ENABLE
	struct {
		bool auto_enable;
		bool force_enable;
	}auto_ota;
#endif /* MIIO_AUTO_OTA_ENABLE */
}miio_ota_app_ctx_t;

int miio_ota_app_init(miio_handle_t miio_handle, miio_ota_app_ctx_t *ota_app_ctx);

#endif
