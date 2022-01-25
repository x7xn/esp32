#ifndef __MIBLE_KEEP_ALIVE_H__
#define __MIBLE_KEEP_ALIVE_H__

#include "mibeacon_def.h"
#include "jsmi.h"

typedef struct {
    uint8_t mac[6];
    uint16_t pid;
    uint8_t did[MIBEACON_DID_LEN];
    int8_t rssi;
    uint32_t timestamp;    
} keep_alive_dev_t;

typedef struct {
    uint32_t interval;
    uint32_t delta;
    uint32_t ttl;
} keep_alive_config_t;

int keep_alive_dev_init(void);
int keep_alive_set_config(uint32_t interval, uint32_t delta);
int keep_alive_dev_add(keep_alive_dev_t *p_dev);
int keep_alive_dev_delete(uint8_t *mac, uint16_t pid);
keep_alive_dev_t *keep_alive_dev_search(uint8_t *mac, uint16_t pid);
int keep_alive_dev_print(void);

int beacon_keep_alive_parse(mibeacon_info_t *p_info);
int keep_alive_decrease_ttl(void);

uint8_t keep_alive_dev_get_count(void);
int keep_alive_dev_get_string(jsmi_composer_t *composer, uint8_t start, uint8_t end);

#endif