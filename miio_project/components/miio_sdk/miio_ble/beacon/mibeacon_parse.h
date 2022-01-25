#ifndef __MIBEACON_PARSE_H__
#define __MIBEACON_PARSE_H__

#include "mibeacon_def.h"

int mibeacon_parse(mibeacon_t *p_beacon, uint16_t beacon_len, mibeacon_info_t *p_info);
int mibeacon_upload(mibeacon_info_t *p_info);
int mibeacon_send_by_serial(mibeacon_info_t *p_info);

#endif
