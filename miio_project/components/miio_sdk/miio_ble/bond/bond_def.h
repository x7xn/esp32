#ifndef __BOND_TYPEDEF_H__
#define __BOND_TYPEDEF_H__

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>

/* Private define ------------------------------------------------------------*/

#define AUTH_START_VAL                            0xDE85CA90
#define AUTH_ACK_VAL                              0xFA54AB92
#define AUTH_LOGIN                                0xCD43BC00
#define AUTH_LOGIN_CFM                            0x93BFAC09
#define AUTH_LOGIN_CFM_ACK                        0x369A58C9
#define AUTH_BIND_SUCC                            0xAC61B130
#define AUTH_BIND_FAIL                            0xE941BF30

/* Private typedef -----------------------------------------------------------*/

typedef void (*state_handler_t)(uint16_t conn_handle, uint8_t *buffer, uint16_t size);

typedef enum {
	STATE_IDLE,
	STATE_WAIT_T1,
	STATE_WAIT_BINDING_ACK,
	STATE_WAIT_LOGIN_ACK,
	STATE_BOND,
	STATE_REGISTER,
	STATE_LOGIN,
	STATE_WAIT_TOKEN_ENABLE,
	STATE_WAIT_T2,
	STATE_WAIT_TICK,
	STATE_WAIT_WRITE_BINDCFM,
	STATE_WAIT_LOGIN_CFM,
} bond_state_t;

typedef enum {
	EVENT_AUTH_CHANGED,
	EVENT_TOKEN_CHANGED,
	EVENT_WIFI_CONFIG,
	EVENT_SERVICE_FOUND,
	EVENT_WRITE_AUTH,
	EVENT_WRITE_TOKEN,
	EVENT_ENABLE_TOKEN,
	EVENT_KEY_CHANGED,
} bond_event_t;

typedef struct
{
    bond_state_t  current_state;
    bond_event_t  event;
    state_handler_t handler;
} bond_fsm_t;

#define ARRAY_SIZE(a)                             (sizeof(a) / sizeof((a)[0]))

#endif
