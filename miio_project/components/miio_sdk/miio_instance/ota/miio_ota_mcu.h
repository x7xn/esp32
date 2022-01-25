/*
 * ota_mcu.h
 *
 *  Created on: Dec 26, 2017
 *      Author: mashaoze
 */

#ifndef _OTA_MCU_H_
#define _OTA_MCU_H_

#include "miio_instance.h"
#include "miio_arch.h"


typedef struct{
	miio_ota_task_handle_t ota_task_handle;
	char mcu_ver[8];
	bool valid;
	ota_flash_info_t flash_info;
	struct {
	   uint32_t magic;
	   uint32_t length;
	}flash_header;
	int mcu_ota_cnt;
	arch_os_semaphore_handle_t lock;

#if MIIO_AUTO_OTA_ENABLE
	struct {
		bool auto_enable;
		bool force_enable;
	}auto_ota;
#endif /* MIIO_AUTO_OTA_ENABLE */
}miio_ota_mcu_ctx_t;

void miio_mcu_set_version(const char *ver);
char* miio_mcu_get_version(char *ver, int ver_len);
int miio_ota_mcu_init(miio_handle_t miio_handle, miio_ota_mcu_ctx_t *ota_mcu_ctx);
int miio_ota_mcu_refresh_fw(miio_ota_mcu_ctx_t *pctx);
int miio_ota_mcu_buffer_fw(miio_ota_mcu_ctx_t *pctx);
int miio_ota_mcu_erase_fw(miio_ota_mcu_ctx_t *pctx);
bool miio_ota_mcu_has_fw(miio_ota_mcu_ctx_t *pctx);

#endif /* _OTA_OTA_MCU_H_ */
