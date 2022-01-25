/**
 * @file miio_net_auto_provision.c
 * @author xusongsong (xusongsong@xiaomi.com)
 * @brief 
 * @version 0.1
 * @date 2020-06-05
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include "miio_net.h"
#include "miio_net_auto_provision.h"
#include "miio_handshake_rpc.h"

#undef MIIO_LOG_TAG
#define MIIO_LOG_TAG "auto_provision"
#define AUTO_PROVISION_RESCAN_TIME (30000)
#define STA_CONNECT_RETRY_TIMES (2)
#define MIIO_NET_PROVISIONER_SSID "25c829b1922d3123_miwifi"
typedef enum {
	AUTO_PROVISION_TIMER_CMD_NONE,
	AUTO_PROVISION_TIMER_CMD_RESCAN,
	AUTO_PROVISION_TIMER_CMD_DISCONNECT_CURRENT_AP,
}auto_provision_timer_cmd_e;
typedef struct miio_auto_provision{
    miio_net_t *miio_net;
    bool (*is_finished)(const struct miio_auto_provision * provision);
    arch_os_timer_handle_t timer;
    bool enable;
    /**
     * The time point When the ECDH is done successfully 
     * between device and provisioner called channel locked.
     * see 'miio_net_auto_provision_channel_lock'
     */
    bool channel_locked;
    int32_t scan_count;
    int32_t connect_retry_count;
    auto_provision_timer_cmd_e tcmd;
}miio_auto_provision_t;

miio_auto_provision_t g_miio_auto_provison={ .miio_net = NULL, .enable= false };


static inline void __set_timer_cmd(miio_auto_provision_t *provision, auto_provision_timer_cmd_e tcmd){
    provision->tcmd = tcmd;
    LOG_DEBUG_TAG(MIIO_LOG_TAG, "set tcmd %s",
    tcmd == AUTO_PROVISION_TIMER_CMD_NONE? "AUTO_PROVISION_TIMER_CMD_NONE" :
    tcmd == AUTO_PROVISION_TIMER_CMD_RESCAN ? "AUTO_PROVISION_TIMER_CMD_RESCAN":
    tcmd == AUTO_PROVISION_TIMER_CMD_DISCONNECT_CURRENT_AP ? "AUTO_PROVISION_TIMER_CMD_DISCONNECT_CURRENT_AP":
    "unknown");
}

static void _auto_provision_timer(arch_os_timer_handle_t timer){

    miio_auto_provision_t *provision = (miio_auto_provision_t *)arch_os_timer_get_context(timer);

    if( provision->is_finished(provision) ){

        LOG_WARN_TAG(MIIO_LOG_TAG, "provision  is finished !!!");
        return ;
    }
	if(AUTO_PROVISION_TIMER_CMD_NONE == provision->tcmd){
		return;
	}
	if(AUTO_PROVISION_TIMER_CMD_RESCAN == provision->tcmd /*rescan*/){
		
		if(MIIO_OK != provision->miio_net->wifi_scan_start(provision->miio_net)){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "wifi scan start failed!");
#if	MIIO_MONITOR_REBOOT_ENABLE
			miio_reboot(provision->miio_net->miio_handle, "wifi scan failed", MIIO_RPC_TIMEOUT_MS);
#endif
		}
    
	}
	if(AUTO_PROVISION_TIMER_CMD_DISCONNECT_CURRENT_AP == provision->tcmd/*switch ap*/){

		provision->miio_net->wifi_disconnect(provision->miio_net);	
	}
	
}
#if 0
static void __scan(miio_auto_provision_t *provision, int delayMs){

	if(delayMs){
		//sleep and scan agin
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "sleep %d and try to Scan provisoner ssid", delayMs);
		arch_os_timer_deactivate(provision->timer);

		__set_timer_cmd(provision, AUTO_PROVISION_TIMER_CMD_RESCAN);

		arch_os_timer_change(provision->timer, delayMs);

		arch_os_timer_activate(provision->timer);

	}else{

		int ierr = provision->miio_net->wifi_scan_start(provision->miio_net);

		if( MIIO_OK != ierr ){

			LOG_ERROR_TAG("auto provision", "wifi scan start failed!");
#if	MIIO_MONITOR_REBOOT_ENABLE
			miio_reboot(provision->miio_net->miio_handle, "wifi scan failed", MIIO_RPC_TIMEOUT_MS);
#endif
		}
	}
}
#endif

static int _on_scan_done(miio_auto_provision_t *provision){
    
    int ierr = MIIO_OK;

    if( provision->channel_locked ){
        goto _end;
    }

    ierr = provision->miio_net->wifi_scan_finish(provision->miio_net);
    if( MIIO_OK != ierr ){
        goto _end;
    }
    provision->scan_count ++;
    do{
        //connect to next ap
        ierr = provision->miio_net->wifi_connect(provision->miio_net);

        if(MIIO_OK == ierr){
            goto _end;
        }
        if( !provision->miio_net->wifi_if_has_next(provision->miio_net) ){
#if 0
            //sleep and scan agin
             __scan(provision, 0 == (provision->scan_count % 2) ? AUTO_PROVISION_RESCAN_TIME : 0);
#endif
           goto _end;
        }
    }while(1);
_end:
    return MIIO_OK;
}
static int _on_sta_start(miio_auto_provision_t *provision){
    int ierr = MIIO_OK;
    if( provision->channel_locked ){
        goto _end;
    }
    miio_rpc_blacklist_add(provision->miio_net->miio_handle, "miIO.config_router");

    LOG_DEBUG_TAG(MIIO_LOG_TAG, "Wifi STA start and try to Scan provisoner ssid");
    ierr = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, provision->miio_net->host_name);

    if(ESP_OK != ierr){
        LOG_ERROR_TAG(MIIO_LOG_TAG, "dhcp set hostname error!");
    }
    LOG_DEBUG_TAG(MIIO_LOG_TAG, "Wifi STA Scan");

    ierr = provision->miio_net->wifi_scan_start(provision->miio_net);

    if( MIIO_OK != ierr ){

    LOG_ERROR_TAG(MIIO_LOG_TAG, "wifi scan start failed!");

#if	MIIO_MONITOR_REBOOT_ENABLE
        miio_reboot(provision->miio_net->miio_handle, "wifi scan failed", MIIO_RPC_TIMEOUT_MS);
#endif

    }
_end:
    return MIIO_OK;
}
static int _on_sta_connected(miio_auto_provision_t *provision, system_event_t *event){

    LOG_DEBUG_TAG(MIIO_LOG_TAG, "connected to provisoner ssid:%s bssid:%02x:%02x:%02x:%02x:%02x:%02x channel:%d, authmode:%d", 
                    event->event_info.connected.ssid,
                    event->event_info.connected.bssid[0],
                    event->event_info.connected.bssid[1],
                    event->event_info.connected.bssid[2],
                    event->event_info.connected.bssid[3],
                    event->event_info.connected.bssid[4],
                    event->event_info.connected.bssid[5],
                    event->event_info.connected.channel,
                    event->event_info.connected.authmode);
#if MIIO_PROV_STATISTIC
	{
		miio_net_t *miio_net = provision->miio_net;
		miio_net->prov_statistic->prov_stage = PROV_STAGE_APSTA_WIFI_CONNECT;
	}	
#endif

    /**
     * At Now, The Device has connected to provisoner.
     * The Timer will be reseted with paramters 'AUTO_PROVISION_TIMER_CMD_DISCONNECT_CURRENT_AP' and 'AUTO_PROVISION_RESCAN_TIME',
     * that means After AUTO_PROVISION_RESCAN_TIME/1000 seconds:
     * 1. if the ECDH between device and provisoner have not been done:
     *      the connection will be disconnetd by the timer command of 'AUTO_PROVISION_TIMER_CMD_DISCONNECT_CURRENT_AP '
     * 2. if the ECDH between device and provisoner is already done
     *      2.1 miio_net_auto_provision_channel_lock should be called by function of ecdh in file of miio_handshake_rpc.c
     *      2.1 The Timer will be reseted with inactively paramter in this time point which is called channel lock @see miio_net_auto_provision_channel_lock
     */
    arch_os_timer_deactivate(provision->timer);

    __set_timer_cmd(provision, AUTO_PROVISION_TIMER_CMD_DISCONNECT_CURRENT_AP);

    arch_os_timer_change(provision->timer, AUTO_PROVISION_RESCAN_TIME);

    arch_os_timer_activate(provision->timer);

    return MIIO_OK;
}
static int _on_sta_disconnected(miio_auto_provision_t *provision, system_event_t *event){
     int lock = provision->channel_locked;
    LOG_DEBUG_TAG(MIIO_LOG_TAG, "Wifi disconnect from ssid %s, bssid:%02x:%02x:%02x:%02x:%02x:%02x reason %d lock %d",
				   event->event_info.disconnected.ssid,
				   event->event_info.disconnected.bssid[0],
				   event->event_info.disconnected.bssid[1],
				   event->event_info.disconnected.bssid[2],
				   event->event_info.disconnected.bssid[3],
				   event->event_info.disconnected.bssid[4],
				   event->event_info.disconnected.bssid[5],
				   event->event_info.disconnected.reason,
                   lock);
   

    /**
     * There are some reasons for receiving the event of disconnected 
     */
      
    //> 1. channel locked
        if(provision->channel_locked){
             
            if(0 != strcmp(provision->miio_net->ssid, MIIO_NET_PROVISIONER_SSID) ){
                //>1.1 we are fired and fuction of 'config_router_safe' was called. This is very what we want!!!
                provision->enable = false;
                arch_os_timer_deactivate(provision->timer);
                LOG_DEBUG_TAG(MIIO_LOG_TAG,"'config_router_safe' was called");
                goto _end;
            }else{
                //>1.2 we are fired without fuction of 'config_router_safe' been called. 
                //> try to connect to another  provisoner
                 LOG_DEBUG_TAG(MIIO_LOG_TAG,"'config_router_safe' was not called");
                goto _retry;
            }
        }

    //> 2. channel unlocked 
        //> 2.1 the couse of disconnection is WIFI_REASON_ASSOC_EXPIRE || WIFI_REASON_NO_AP_FOUND 
        //> reconnect to the same provisoner by 'STA_CONNECT_RETRY_TIMES' times
        if (event->event_info.disconnected.reason == WIFI_REASON_ASSOC_EXPIRE
                || event->event_info.disconnected.reason == WIFI_REASON_AUTH_EXPIRE
                || event->event_info.disconnected.reason == WIFI_REASON_NO_AP_FOUND) {

            if(provision->connect_retry_count-- > 0){

                if (provision->miio_net->wifi_connect(provision->miio_net) == MIIO_OK) {
                    goto _end;
                }
            }

        }
_retry:
        //>2.2 finding next provisoner and connect to...
        //>2.2.1 unlock the channel if it is locked
        provision->channel_locked = false; 
        provision->connect_retry_count = STA_CONNECT_RETRY_TIMES;
        do{
            if( !provision->miio_net->wifi_if_has_next(provision->miio_net) ){
#if 0
                //> rescan
                __scan(provision, lock ?  0 : AUTO_PROVISION_RESCAN_TIME);
#endif
                
                break;
            }
            provision->miio_net->wifi_disconnect(provision->miio_net);
            //connect to next ap
            if(MIIO_OK == provision->miio_net->wifi_connect( provision->miio_net )){
                break;
            }
        }while(1);

_end:
    return MIIO_OK;
}
static int _on_apsta_connected(miio_auto_provision_t *provision){
    /**
     * This time point when sta(e.g: cell phone) connected to ap indicates that 
     * the user wants to manually configure the network, so the fuction of auto 
     * provision will be disabled
     */
    provision->enable = false;
    miio_ecdh_mode_enable(MIIO_ECDH_MODE_0);
    miio_ecdh_mode_enable(MIIO_ECDH_MODE_1);
    arch_os_timer_deactivate(provision->timer);
    esp_wifi_scan_stop();
    esp_wifi_disconnect();
    miio_rpc_blacklist_del(provision->miio_net->miio_handle, "miIO.config_router");
    return MIIO_OK;
}
static bool _is_finished(const struct miio_auto_provision * provision){

    return ( ( !provision->enable ) || provision->miio_net->provision_status );
}
bool miio_net_auto_provision_wifi_event_process(miio_auto_provision_t* auto_provision, system_event_t *event){

    bool throw_away = ( NULL == auto_provision || 
                        NULL == auto_provision->miio_net || 
                        auto_provision->miio_net->provision_status || 
                        ( !auto_provision->enable ));
    if( throw_away ){
        
		return false; //> throw it to miio_net Component
	}
    
    switch(event->event_id){
        case SYSTEM_EVENT_SCAN_DONE:
        {
            _on_scan_done(auto_provision);
            return true;
        }
        case SYSTEM_EVENT_STA_START:
        {
            _on_sta_start(auto_provision);
            return true;
        }
        case SYSTEM_EVENT_STA_STOP:
		    return true;
        case SYSTEM_EVENT_STA_CONNECTED:
        {
            _on_sta_connected(auto_provision, event);
             return true;
        }
        case SYSTEM_EVENT_STA_GOT_IP:
        {
            LOG_DEBUG_TAG(MIIO_LOG_TAG, "Wifi ip="IPSTR",mask="IPSTR",gw="IPSTR"",
                IP2STR(&event->event_info.got_ip.ip_info.ip),
                IP2STR(&event->event_info.got_ip.ip_info.netmask),
                IP2STR(&event->event_info.got_ip.ip_info.gw));
            return true;
        }
        case SYSTEM_EVENT_GOT_IP6:
        {
            LOG_DEBUG_TAG(MIIO_LOG_TAG, "Wifi ipv6="IPV6STR,
                IPV62STR(event->event_info.got_ip6.ip6_info.ip));
            return true;
        }
        case SYSTEM_EVENT_STA_DISCONNECTED:
        {
            _on_sta_disconnected(auto_provision, event);
            return true;
        }
        case SYSTEM_EVENT_STA_LOST_IP:
        {
            LOG_WARN_TAG(MIIO_LOG_TAG, "dhcp failed");
            return true;
        }
        case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
        {
            LOG_DEBUG_TAG(MIIO_LOG_TAG, "Wifi auth_mode %d -> %d",
                    event->event_info.auth_change.old_mode,
                    event->event_info.auth_change.new_mode);
            return true;
        }
        case SYSTEM_EVENT_AP_STACONNECTED:
        {
            _on_apsta_connected(auto_provision);
            return false;
        }
        	
	    default:
		    return false;
    }
    return false;
}

#define AUTO_PROVISION_ERROR_CHECK(expression,label, fmt, ...) \
do{\
	if( !(expression) ){\
		LOG_ERROR_TAG( MIIO_LOG_TAG, fmt, ##__VA_ARGS__ );\
		goto label; \
	} \
}while(0)

int miio_net_auto_provision_init( miio_auto_provision_t **auto_provision,  miio_net_t *miio_net){

    *auto_provision = NULL;
    memset(&g_miio_auto_provison, 0, sizeof(g_miio_auto_provison));
    g_miio_auto_provison.miio_net = miio_net;

    AUTO_PROVISION_ERROR_CHECK( miio_net != NULL , ERROR, "miio_net == NULL");
    AUTO_PROVISION_ERROR_CHECK( miio_net->wifi_scan_start != NULL , ERROR, "wifi_scan_start == NULL");
    AUTO_PROVISION_ERROR_CHECK( miio_net->wifi_scan_finish != NULL , ERROR, "wifi_scan_finish == NULL");
    AUTO_PROVISION_ERROR_CHECK( miio_net->wifi_if_has_next != NULL , ERROR, "wifi_if_has_next == NULL");
    AUTO_PROVISION_ERROR_CHECK( miio_net->wifi_connect != NULL , ERROR, "wifi_connect == NULL");
    AUTO_PROVISION_ERROR_CHECK( miio_net->wifi_disconnect != NULL , ERROR, "wifi_disconnect == NULL");
    g_miio_auto_provison.is_finished = _is_finished;

    if( miio_net->auto_provision_enable ){

        g_miio_auto_provison.enable = true;
        g_miio_auto_provison.connect_retry_count = STA_CONNECT_RETRY_TIMES;
        g_miio_auto_provison.tcmd = AUTO_PROVISION_TIMER_CMD_NONE;
        miio_ecdh_mode_disable(MIIO_ECDH_MODE_0);
        miio_ecdh_mode_enable(MIIO_ECDH_MODE_1);
        arch_os_timer_create(&g_miio_auto_provison.timer, "auto_provision", AUTO_PROVISION_RESCAN_TIME, _auto_provision_timer, &g_miio_auto_provison, ARCH_OS_TIMER_ONE_SHOT, ARCH_OS_TIMER_NO_ACTIVATE);
    }
    *auto_provision = &g_miio_auto_provison;
    return MIIO_OK;
ERROR:
    return MIIO_ERROR;
}
int miio_net_auto_provision_deinit(miio_auto_provision_t* auto_provision){

    LOG_DEBUG_TAG(MIIO_LOG_TAG,"'auto provision deinit");

    miio_rpc_blacklist_del(auto_provision->miio_net->miio_handle, "miIO.config_router");
    do{
        if(auto_provision != &g_miio_auto_provison ){
          break;
        }
        if(g_miio_auto_provison.timer != NULL){

            arch_os_timer_delete( g_miio_auto_provison.timer );
        }
        memset( &g_miio_auto_provison, 0, sizeof(g_miio_auto_provison) );

    }while(0);

    return MIIO_OK;
}
void miio_net_auto_provision_channel_lock(){
    
    //> reset the timeer to keep current state when channel locked
	g_miio_auto_provison.channel_locked = true;

    if(NULL != g_miio_auto_provison.timer){
        
	    arch_os_timer_deactivate(g_miio_auto_provison.timer);
        
        LOG_DEBUG_TAG(MIIO_LOG_TAG,"channel have been locked, then wait for config forever");

    }
}
