#ifndef _ARCH_CHIP_H_
#define _ARCH_CHIP_H_

#include "freertos/FreeRTOS.h"//must ahead of timers.h
#include "freertos/timers.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/portmacro.h"
#include "freertos/portable.h"
#include "freertos/FreeRTOSConfig.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "sys/types.h"
#include "mdns.h"

#include "esp32/rom/gpio.h"
#include "esp32/rom/uart.h"
#include "sys/queue.h"
#include "esp32/rom/efuse.h"
#include "esp32/rom/md5_hash.h"
#include "esp32/rom/spi_flash.h"
#include "esp32/aes.h"
#include "soc/soc.h"
#include "soc/uart_reg.h"
#include "soc/efuse_reg.h"
#include "driver/gpio.h"
#include "driver/uart.h"

//lwip
#include "lwip/api.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/sockets.h"

#include "miio_define.h"
#include "miio_instance_config.h"

int arch_chip_init(void);
char* arch_get_chip_type(void);
uint32_t arch_get_chip_version(void);
#define arch_get_sdk_version()		esp_get_idf_version()
void arch_get_did(uint64_t *did);
void arch_get_psk(uint8_t psk[16]);
int arch_get_random(uint8_t *output, size_t output_len);
void arch_get_mac(uint8_t mac[6]);
void arch_get_flash_info(void);

#endif /* _ARCH_CHIP_H_ */
