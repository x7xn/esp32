/**
 * @file miio_net_auto_provision.h
 * @author xusongsong (xusongsong@xiaomi.con)
 * @brief The module is a submodule of 'miio net'. This module provides the function of
 *        'auto provision' from which the wireless's ssid and password is configured
 *         without human interaction.This module is very depend on the functionality of 
 *         'miio net' that providing functions of below:
 *         int (*wifi_scan_start)( struct miio_net *miio_net );
 *         int (*wifi_scan_finish)( struct miio_net *miio_net );
 *         int (*wifi_if_has_next)( struct miio_net *miio_net );
 *         int (*wifi_connect)( struct miio_net *miio_net );
 *         void (*wifi_disconnect)( struct miio_net *miio_net);
 * @version 0.1
 * @date 2020-06-05
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef __MIIO_NET_AUTO_PROVISION_H__
#define __MIIO_NET_AUTO_PROVISION_H__
#include "miio_arch.h"
struct miio_net;
struct miio_auto_provision;



int miio_net_auto_provision_init(struct miio_auto_provision **auto_provision, struct miio_net *miio_net);

int miio_net_auto_provision_deinit(struct miio_auto_provision* auto_provision);

bool miio_net_auto_provision_wifi_event_process(struct miio_auto_provision* auto_provision, system_event_t *event);

void miio_net_auto_provision_channel_lock();
#endif