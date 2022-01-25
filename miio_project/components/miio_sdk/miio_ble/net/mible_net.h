/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NET_CONFIG_H__
#define __NET_CONFIG_H__

/* Includes ------------------------------------------------------------------*/

#include "miio_define.h"
#include "miio_net.h"

/* Private typedef -----------------------------------------------------------*/

typedef struct {
    uint64_t uid;
    int gmt_offset;
    const char *country_domain;
    struct{
        const char* ssid;
        const char* password;
    } ap;
    struct{
        const char *cc;
    } wifi_channel;
    const char *bindkey;
    const char *config_type;

#if MIIO_KEY_EXCHANGE_ENABLE
    const char *bind_idx;
    uint64_t bind_ts;
#endif

} mible_net_config_params_t;

typedef int (*mible_net_config_callback_t)(miio_handle_t miio_handle, mible_net_config_params_t *params);
typedef int (*mible_net_restore_callback_t)(miio_handle_t miio_handle);

typedef struct {
    mible_net_config_callback_t config_callback;
    mible_net_restore_callback_t restore_callback;
} mible_net_callbacks_t;

typedef enum {
    NET_CONFIG_STATE_DISCONNECTED = 0,
    NET_CONFIG_STATE_CONNECTING,
    NET_CONFIG_STATE_CONNECTED,
    NET_CONFIG_STATE_COMPLETED,
    NET_CONFIG_STATE_ERR_UNKNOWN,
    NET_CONFIG_STATE_ERR_WRONG,
    NET_CONFIG_STATE_INVALID,
} mible_net_state_t;

/* Exported functions --------------------------------------------------------*/

int  mible_net_init(void);
int  mible_net_register(miio_handle_t handle, const mible_net_callbacks_t* p_callbacks);
void mible_net_construct(uint8_t *data, int length);
void mible_net_construct_from_package_data(uint8_t *data, int length);
mible_net_state_t mible_net_state_get(void);
bool mible_net_state_check(void);
bool mible_net_beacon_check(void);
bool mible_net_stop_check(void);
int mible_net_state_callback(miio_net_state_t state, miio_net_error_t error, void *ctx);
miio_handle_t mible_net_handle_get(void);

#endif
