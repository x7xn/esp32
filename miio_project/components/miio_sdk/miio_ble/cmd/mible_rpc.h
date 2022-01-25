/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MIBLE_RPC_H__
#define __MIBLE_RPC_H__

#include "adv_beacon.h"
#include "mible_dev.h"
#include "object_rule.h"
#include "mible_keep_alive.h"
#include "mible_linkage_search.h"

typedef enum {
    MIBLE_RPC_QUERY_DEV,
    MIBLE_RPC_QUERY_PROB,
    MIBLE_RPC_REPORT_EVT,
    MIBLE_RPC_KEEP_ALIVE,
    MIBLE_RPC_GATEWAY_ENABLE,
    MIBLE_RPC_SCAN_RESULT_EVT,
    MIBLE_RPC_MESH_SWITCH,
} mible_rpc_type_t;


int mible_rpc_query_dev(uint8_t *mac, uint16_t pid);
int mible_rpc_query_prod(uint16_t pid);
int mible_rpc_report_evt(authed_dev_t *p_dev, mibeacon_object_array_t *p_object_array);
int mible_rpc_keep_alive(void);
int mible_rpc_single_alive(keep_alive_dev_t *p_dev);
int mible_rpc_gateway_enable(void);
int mible_rpc_search_result_evt(search_target_array_t *result);
int mible_rpc_mesh_switch(uint8_t state);

int rpc_upload_init(void);
int rpc_upload_add(char *rpc, uint32_t method_id, mible_rpc_type_t type);
int rpc_upload_print(void);
int get_first_rpc_and_upload(void);

#endif
