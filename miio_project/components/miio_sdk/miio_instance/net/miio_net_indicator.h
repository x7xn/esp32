/**
* @file    miio_net_indicator.h
* @author  liuyujia
* @date    2019
*/
#ifndef _MIIO_NET_INDICATOR_H_
#define _MIIO_NET_INDICATOR_H_

#include "miio_net.h"

int miio_net_indicator_init(void);
void miio_net_indicator_set(miio_net_state_t state);

#endif
