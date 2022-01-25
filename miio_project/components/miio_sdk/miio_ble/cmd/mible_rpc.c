#include "mible_rpc.h"
#include "jsmi.h"
#include "list/list.h"
#include "mible_net.h"
#include "miio_api.h"
#include "object_rule.h"
#include "mible_def.h"
#include "mible_gateway.h"
#include "util.h"
#include "mible_linkage.h"
#include "mible_linkage_search.h"
#include "mible_hooks.h"

#undef  MIBLE_LOG_TAG
#define MIBLE_LOG_TAG                           "mible_rpc"
#undef  LOG_LEVEL
#define LOG_LEVEL                               LOG_LEVEL_INFO

/* Private define ------------------------------------------------------------*/

#define MIBLE_RPC_LEGACY_ERR_PERMISSION         -1
#define MIBLE_RPC_LEGACY_ERR_OFFLINE            -2
#define MIBLE_RPC_LEGACY_ERR_TIMEOUT            -3
#define MIBLE_RPC_LEGACY_ERR_SERVICE            -4
#define MIBLE_RPC_LEGACY_ERR_DEVICE             -5
#define MIBLE_RPC_LEGACY_ERR_REQUEST            -6
#define MIBLE_RPC_LEGACY_ERR_NULL               -7
#define MIBLE_RPC_LEGACY_ERR_PARAMETER          -8
#define MIBLE_RPC_LEGACY_ERR_UNKNOWN            -9
#define MIBLE_RPC_LEGACY_ERR_HOLDER             -10
#define MIBLE_RPC_LEGACY_ERR_REPEAT             -11
#define MIBLE_RPC_LEGACY_ERR_FREQUENT           -12

#define MIBLE_RPC_ERR_CODE_REQUEST              -4001001
#define MIBLE_RPC_ERR_CODE_DATA                 -4001002
#define MIBLE_RPC_ERR_CODE_SUPPORT              -4001003
#define MIBLE_RPC_ERR_CODE_NUMBER               -4001004
#define MIBLE_RPC_ERR_CODE_CONFIG               -4001005
#define MIBLE_RPC_ERR_CODE_FREQUENT             -4011001
#define MIBLE_RPC_ERR_CODE_SERVICE              -5001001

#define MIBLE_RPC_ERR_INFO_PARAM         "Invalid param."
#define MIBLE_RPC_ERR_INFO_FULL          "resource full."
#define MIBLE_RPC_ERR_INFO_NOTFOUND      "Not found."
#define MIBLE_RPC_ERR_INFO_SIZE          "Invalid size"
#define MIBLE_RPC_ERR_INFO_NOMEM         "No memory."

/* Exported variables --------------------------------------------------------*/

extern arch_os_mutex_handle_t mibeacon_list_mutex;

/* Private functions ---------------------------------------------------------*/

static int mible_query_dev_cb(miio_rpc_delegate_arg_t *ack_arg, void *ctx);
static int mible_query_prod_cb(miio_rpc_delegate_arg_t *ack_arg, void *ctx);
static int mible_report_evt_cb(miio_rpc_delegate_arg_t *ack_arg, void *ctx);
static int mible_keep_alive_cb(miio_rpc_delegate_arg_t *ack_arg, void *ctx);
static int mible_gateway_enable_cb(miio_rpc_delegate_arg_t *ack_arg, void *ctx);
static int mible_search_result_evt_cb(miio_rpc_delegate_arg_t *ack_arg, void *ctx);

static int send_rpc_to_wifi(char* rpc, uint32_t method_id, mible_rpc_type_t type);

static int do_ble_linkage_search(miio_rpc_delegate_arg_t *req_arg,
                            miio_fp_rpc_delegate_ack_t ack, void* ctx);
static int do_ble_add_linkage(miio_rpc_delegate_arg_t *req_arg,
                            miio_fp_rpc_delegate_ack_t ack, void* ctx);
static int do_ble_del_linkage(miio_rpc_delegate_arg_t *req_arg,
                            miio_fp_rpc_delegate_ack_t ack, void* ctx);
static int do_ble_get_linkage(miio_rpc_delegate_arg_t *req_arg,
                            miio_fp_rpc_delegate_ack_t ack, void* ctx);
static int do_ble_enable_linkage(miio_rpc_delegate_arg_t *req_arg,
                            miio_fp_rpc_delegate_ack_t ack, void* ctx);
static int do_ble_dev_bind_notify(miio_rpc_delegate_arg_t *req_arg,
                                        miio_fp_rpc_delegate_ack_t ack, void* ctx);
static int do_ble_dev_unbind_notify(miio_rpc_delegate_arg_t *req_arg,
                                        miio_fp_rpc_delegate_ack_t ack, void* ctx);


static void delete_special_char(char *buffer, int len)
{
    if (len != 17) return;
    char *s = (char *)malloc(sizeof(char) * len);
    if (NULL == s) return;
    int j = 0;
    for (int i = 0; i < len; i++) {
        if (buffer[i] != ':') {
            s[j++] = buffer[i];
        }
    }
    memcpy(buffer, s, j);
    free(s);
}

static void reverse_mac(uint8_t *mac) {
    if (NULL == mac) return;
    uint8_t tmp;
    for (int i = 0; i < 3; i ++) {
        tmp = mac[i];
        mac[i] = mac[5 - i];
        mac[5 - i] = tmp;
    }
}

static int mac_convert(char *buffer, uint8_t *dst) {
    if (strlen(buffer) != 17) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "mac_convert: mac length error");
        return MIIO_ERROR_NOTFOUND;
    }
    delete_special_char(buffer, 17);
    arch_str2hex(dst, buffer, 6);
    reverse_mac(dst);
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "mac_convert: mac = %02X %02X %02X %02X %02X %02X ", dst[0],
                  dst[1], dst[2], dst[3], dst[4], dst[5]);
    return MIIO_OK;
}

// _sync.ble_query_dev
// {"method":"_sync.ble_query_dev","params":{"mac":"84:68:3E:00:7A:E3","pdid":157}}
// {"result":{"operation":"query_dev","mac":"84:68:3E:00:7A:E3","ttl":600,"pdid":157,"did":"12345678","token":"112233445566778899001122","beaconkey":"00112233445566778899001122334455"}}
// {"error":{"code":-x,"message":{"operation":"query_dev","mac":"84:68:3E:00:7A:E3","pdid":157}}}

int mible_rpc_query_dev(uint8_t *mac, uint16_t pid)
{
    int ret = MIIO_OK;
    char *buffer = NULL;
    size_t buf_size = 512; // OTS_PACKET_PLOAD_SIZE_MAX

    if (NULL == mac) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "rpc query dev invalid param");
        return MIIO_ERROR_PARAM;
    }

    miio_handle_t miio_handle = mible_net_handle_get();
    if (NULL == miio_handle) {
        return MIIO_ERROR_NOTREADY;
    }

    if (!mible_net_state_check()) {
        return MIIO_ERROR_NOTREADY;
    }

    uint32_t method_id = miio_get_rpc_id(miio_handle);

    buffer = (char *)malloc(sizeof(char) * buf_size);
    if (NULL == buffer) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for query dev buffer");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memset((uint8_t *)buffer, 0, buf_size);

    char mac_string[18] = {0};
    snprintf(mac_string, sizeof(mac_string), "%02X:%02X:%02X:%02X:%02X:%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);

    jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(buffer, buf_size);
    jsmi_compose_start(&jsmi_composer);
    jsmi_set_object_begin(&jsmi_composer);
        jsmi_set_key_value_uint(&jsmi_composer, "id", method_id);
        jsmi_set_key_value_str(&jsmi_composer, "method", "_sync.ble_query_dev", strlen("_sync.ble_query_dev"));
        jsmi_set_key_object_begin(&jsmi_composer, "params");
            jsmi_set_key_value_str(&jsmi_composer, "mac", mac_string, strlen(mac_string));
            jsmi_set_key_value_uint(&jsmi_composer, "pdid", pid);
        jsmi_set_key_object_end(&jsmi_composer);
    jsmi_set_object_end(&jsmi_composer);

    if(MIIO_OK != jsmi_compose_finish(&jsmi_composer, NULL, NULL)){
        ret = MIIO_ERROR_NOTFOUND;
        goto end;
    }

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "mible_rpc_query_dev = %s", buffer);
    ret = rpc_upload_add(buffer, method_id, MIBLE_RPC_QUERY_DEV);

end:

    if (NULL != buffer) {
        free(buffer);
        buffer = NULL;
    }
    return ret;
}

static int mible_query_dev_cb(miio_rpc_delegate_arg_t *ack_arg, void *ctx)
{
    int ret = MIIO_OK;

    char *buffer = NULL;
    size_t buf_size = 128;

    authed_dev_t *p_authed_dev = NULL;
    unknown_dev_t *p_unknown_dev = NULL;

    if (NULL == ack_arg) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "query dev cb invalid param");
        return MIIO_ERROR_PARAM;
    }

    jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(ack_arg->pload, ack_arg->pload_len);
    ret = jsmi_parse_start(&jsmi_parser);
    if (MIIO_OK != ret) {
        return ret;
    }

    buffer = (char *)malloc(sizeof(char) * buf_size);
    if (NULL == buffer) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for query cb buffer");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memset((uint8_t *)buffer, 0, buf_size);

    jsmi_tok_path_t error_path[] = {
        {
            .key = "error",
            .type = JSMN_OBJECT
        }
    };
    jsmntok_t *error_tok = jsmi_get_tok(&jsmi_parser, NULL, error_path, NELEMENTS(error_path));
    if(NULL != error_tok) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "query_dev_cb: found error");

        p_unknown_dev = (unknown_dev_t *)malloc(sizeof(unknown_dev_t));
        if (NULL == p_unknown_dev) {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for unknown_dev");
            ret = MIIO_ERROR_NOMEM;
            goto end;
        }
        memset((uint8_t *)p_unknown_dev, 0, sizeof(unknown_dev_t));

        jsmi_tok_path_t mac_path[] = {
            {
                .key = "message",
                .type = JSMN_OBJECT
            },
            {
                .key = "mac",
                .type = JSMN_STRING
            }
        };
        memset((uint8_t *)buffer, 0, buf_size);
        if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, error_tok, mac_path, NELEMENTS(mac_path), buffer, buf_size)) {
            ret = MIIO_ERROR_NOTFOUND;
            goto end;
        }
        if (MIIO_OK != mac_convert(buffer, (uint8_t *)p_unknown_dev->mac)) {
            ret = MIIO_ERROR_NOTFOUND;
            goto end;
        }

        jsmi_tok_path_t pid_path[] = {
            {
                .key = "message",
                .type = JSMN_OBJECT
            },
            {
                .key = "pdid",
                .type = JSMN_PRIMITIVE
            }
        };
        if(MIIO_OK != jsmi_get_value_u16(&jsmi_parser, error_tok, pid_path, NELEMENTS(pid_path), &p_unknown_dev->pid)) {
            ret = MIIO_ERROR_NOTFOUND;
            goto end;
        }
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "query_dev_cb: pid = %d", p_unknown_dev->pid);

        // there is no definication of unknown ttl
        p_unknown_dev->ttl = MIBEACON_UNKNOWN_TTL;
        unknown_dev_add(p_unknown_dev);

        // unknown_dev_print();
        goto end;
    }

    jsmi_tok_path_t result_path[] = {
        {
            .key = "result",
            .type = JSMN_OBJECT
        }
    };
    jsmntok_t *result_tok = jsmi_get_tok(&jsmi_parser, NULL, result_path, NELEMENTS(result_path));
    if (NULL != result_tok) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "query_dev_cb: found result");

        p_authed_dev = (authed_dev_t *)malloc(sizeof(authed_dev_t));
        if (NULL == p_authed_dev) {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for authed_dev");
            ret = MIIO_ERROR_NOMEM;
            goto end;
        }
        memset((uint8_t *)p_authed_dev, 0, sizeof(authed_dev_t));

        jsmi_tok_path_t mac_path[] = {
            {
                .key = "mac",
                .type = JSMN_STRING
            }
        };
        memset((uint8_t *)buffer, 0, buf_size);
        if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, result_tok, mac_path, NELEMENTS(mac_path), buffer, buf_size)) {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "query_dev_cb: cannot parse mac");
            ret = MIIO_ERROR_NOTFOUND;
            goto end;
        }
        if (MIIO_OK != mac_convert(buffer, (uint8_t *)p_authed_dev->mac)) {
            ret = MIIO_ERROR_NOTFOUND;
            goto end;
        }

        jsmi_tok_path_t pid_path[] = {
            {
                .key = "pdid",
                .type = JSMN_PRIMITIVE
            }
        };
        if(MIIO_OK != jsmi_get_value_u16(&jsmi_parser, result_tok, pid_path, NELEMENTS(pid_path), &p_authed_dev->pid)) {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "query_dev_cb: cannot parse pid");
            ret = MIIO_ERROR_NOTFOUND;
            goto end;
        }
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "query_dev_cb: pid = %d", p_authed_dev->pid);

        jsmi_tok_path_t did_path[] = {
            {
                .key = "did",
                .type = JSMN_STRING
            }
        };
        memset((uint8_t *)buffer, 0, buf_size);
        if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, result_tok, did_path, NELEMENTS(did_path), buffer, buf_size)) {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "query_dev_cb: cannot parse did");
            ret = MIIO_ERROR_NOTFOUND;
            goto end;
        }
        if (strlen(buffer) > (MIBEACON_DID_LEN - 1) || strlen(buffer) < 1) {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "query_dev_cb: did length error");
            ret = MIIO_ERROR_NOTFOUND;
            goto end;
        }
        // did is a string!
        memcpy((uint8_t *)p_authed_dev->did, (uint8_t *)buffer, strlen(buffer) + 1);
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "query_dev_cb: did = %s", p_authed_dev->did);

        if (p_authed_dev->pid != 21) {
            jsmi_tok_path_t token_path[] = {
                {
                    .key = "token",
                    .type = JSMN_STRING
                }
            };
            memset((uint8_t *)buffer, 0, buf_size);
            if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, result_tok, token_path, NELEMENTS(token_path), buffer, buf_size)) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "query_dev_cb: cannot parse token");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }
            if (strlen(buffer) != MIBEACON_TOKEN_LEN * 2) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "query_dev_cb: token length error");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }
            arch_str2hex(p_authed_dev->token, buffer, MIBEACON_TOKEN_LEN);
            // LOG_DEBUG_TAG(MIBLE_LOG_TAG, "query_dev_cb: token = ");
            // arch_dump_hex(p_authed_dev->token, MIBEACON_TOKEN_LEN, NULL);
        }

        jsmi_tok_path_t ttl_path[] = {
            {
                .key = "ttl",
                .type = JSMN_PRIMITIVE
            }
        };
        if(MIIO_OK != jsmi_get_value_uint(&jsmi_parser, result_tok, ttl_path, NELEMENTS(ttl_path), &p_authed_dev->ttl)) {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "query_dev_cb: cannot parse ttl");
            ret = MIIO_ERROR_NOTFOUND;
            goto end;
        }
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "query_dev_cb: ttl = %d", p_authed_dev->ttl);

        if (p_authed_dev->pid != 21) {
            jsmi_tok_path_t key_path[] = {
                {
                    .key = "beaconkey",
                    .type = JSMN_STRING
                }
            };
            memset((uint8_t *)buffer, 0, buf_size);
            if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, result_tok, key_path, NELEMENTS(key_path), buffer, buf_size)) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "query_dev_cb: cannot parse token");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }
            // special for hey+ band beaconkey length = 20
            if (strlen(buffer) < MIBEACON_BEACONKEY_LEN * 2) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "query_dev_cb: beaconkey length error");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }
            arch_str2hex(p_authed_dev->beaconkey, buffer, MIBEACON_BEACONKEY_LEN);
            // LOG_DEBUG_TAG(MIBLE_LOG_TAG, "query_dev_cb: beaconkey = ");
            // arch_dump_hex(p_authed_dev->beaconkey, MIBEACON_BEACONKEY_LEN, NULL);
        }

        // NOTE: do not forget to delete device in unknown list
        if (MIIO_OK == authed_dev_add(p_authed_dev)) {
            unknown_dev_delete(p_authed_dev->mac, p_authed_dev->pid);
        }
        // unknown_dev_print();
        // authed_dev_print();
    }

end:
    jsmi_parse_finish(&jsmi_parser);

    if (NULL != buffer) {
        free(buffer);
        buffer = NULL;
    }
    if (NULL != p_authed_dev) {
        free(p_authed_dev);
        p_authed_dev = NULL;
    }
    if (NULL != p_unknown_dev) {
        free(p_unknown_dev);
        p_unknown_dev = NULL;
    }

    return ret;
}


// _sync.ble_query_prod
// {"method":"_sync.ble_query_prod","params":{"pdid":156}}
// {"result":{"operation":"query_prod","pdid":156,"ttl":600,"thr":-40,"upRule":[{"eid":1234,"intvl":10,"delta":100},{"eid":4100,"intvl":5,"delta":100}]}}
// {"error":{"code":-x,"message":{"operation":"query_prod","pdid":156}}}

int mible_rpc_query_prod(uint16_t pid)
{
    int ret = MIIO_OK;
    char *buffer = NULL;
    size_t buf_size = 512; // OTS_PACKET_PLOAD_SIZE_MAX

    miio_handle_t miio_handle = mible_net_handle_get();
    if (NULL == miio_handle) {
        return MIIO_ERROR_NOTREADY;
    }

    if (!mible_net_state_check()) {
        return MIIO_ERROR_NOTREADY;
    }

    uint32_t method_id = miio_get_rpc_id(miio_handle);

    buffer = (char *)malloc(sizeof(char) * buf_size);
    if (NULL == buffer) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for query prod buffer");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memset((uint8_t *)buffer, 0, buf_size);

    jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(buffer, buf_size);
    jsmi_compose_start(&jsmi_composer);
    jsmi_set_object_begin(&jsmi_composer);
        jsmi_set_key_value_uint(&jsmi_composer, "id", method_id);
        jsmi_set_key_value_str(&jsmi_composer, "method", "_sync.ble_query_prod", strlen("_sync.ble_query_prod"));
        jsmi_set_key_object_begin(&jsmi_composer, "params");
            jsmi_set_key_value_uint(&jsmi_composer, "pdid", pid);
        jsmi_set_key_object_end(&jsmi_composer);
    jsmi_set_object_end(&jsmi_composer);

    if(MIIO_OK != jsmi_compose_finish(&jsmi_composer, NULL, NULL)){
        ret = MIIO_ERROR_NOTFOUND;
        goto end;
    }

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "mible_rpc_query_prod = %s", buffer);
    ret = rpc_upload_add(buffer, method_id, MIBLE_RPC_QUERY_PROB);

end:

    if (NULL != buffer) {
        free(buffer);
        buffer = NULL;
    }

    return ret;
}

static int mible_query_prod_cb(miio_rpc_delegate_arg_t *ack_arg, void *ctx)
{
    int ret = MIIO_OK;

    pid_obj_id_t *p_pid_obj_id = NULL;
    object_rule_t *p_rule = NULL;

    if (NULL == ack_arg) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "query prod cb invalid param");
        return MIIO_ERROR_PARAM;
    }

    jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(ack_arg->pload, ack_arg->pload_len);
    ret = jsmi_parse_start(&jsmi_parser);
    if (MIIO_OK != ret) {
        return ret;
    }

    jsmi_tok_path_t error_path[] = {
        {
            .key = "error",
            .type = JSMN_OBJECT
        }
    };
    jsmntok_t *error_tok = jsmi_get_tok(&jsmi_parser, NULL, error_path, NELEMENTS(error_path));
    if(NULL != error_tok) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "mible_query_prod_cb: found error");
        ret = MIIO_ERROR_NOTFOUND;
        goto end;
    }

    p_pid_obj_id = (pid_obj_id_t *)malloc(sizeof(pid_obj_id_t));
    if (NULL == p_pid_obj_id) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for pid_obj_id");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memset((uint8_t *)p_pid_obj_id, 0, sizeof(pid_obj_id_t));

    jsmi_tok_path_t pid_path[] = {
        {
            .key = "result",
            .type = JSMN_OBJECT
        },
        {
            .key = "pdid",
            .type = JSMN_PRIMITIVE
        }
    };
    if(MIIO_OK != jsmi_get_value_u16(&jsmi_parser, NULL, pid_path, NELEMENTS(pid_path), &p_pid_obj_id->pid)) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "mible_query_prod_cb: cannot parse pid");
        ret = MIIO_ERROR_NOTFOUND;
        goto end;
    }
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "mible_query_prod_cb: pid = %d", p_pid_obj_id->pid);

    jsmi_tok_path_t ttl_path[] = {
        {
            .key = "result",
            .type = JSMN_OBJECT
        },
        {
            .key = "ttl",
            .type = JSMN_PRIMITIVE
        }
    };
    if(MIIO_OK != jsmi_get_value_uint(&jsmi_parser, NULL, ttl_path, NELEMENTS(ttl_path), &p_pid_obj_id->ttl)) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "mible_query_prod_cb: cannot parse ttl");
        ret = MIIO_ERROR_NOTFOUND;
        goto end;
    }
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "mible_query_prod_cb: ttl = %d", p_pid_obj_id->ttl);

    jsmi_tok_path_t rule_path[] = {
        {
            .key = "result",
            .type = JSMN_OBJECT
        },
        {
            .key = "upRule",
            .type = JSMN_ARRAY
        }
    };
    jsmntok_t *rule_list_tok = jsmi_get_tok(&jsmi_parser, NULL, rule_path, NELEMENTS(rule_path));
    if (NULL == rule_list_tok) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "query_prod_cb: cannot find upRule list");
        ret = MIIO_ERROR_NOTFOUND;
        goto end;
    }

    p_rule = (object_rule_t *)malloc(sizeof(object_rule_t));
    if (NULL == p_rule) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for object_rule");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memset((uint8_t *)p_rule, 0, sizeof(object_rule_t));

    for (int i = 0; i < rule_list_tok->size; i++) {

        memset((uint8_t *)p_rule, 0, sizeof(object_rule_t));

        jsmi_tok_path_t eid_path[] = {
            {
                .key = (void*)i,
                .type = JSMN_OBJECT
            },
            {
                .key = "eid",
                .type = JSMN_PRIMITIVE
            }
        };
        if(MIIO_OK != jsmi_get_value_u16(&jsmi_parser, rule_list_tok, eid_path, NELEMENTS(eid_path), &p_rule->obj_id)) {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "query_prod_cb: cannot parse eid");
            ret = MIIO_ERROR_NOTFOUND;
            goto end;
        }
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "query_prod_cb: eid = %04x", p_rule->obj_id);

        jsmi_tok_path_t intvl_path[] = {
            {
                .key = (void*)i,
                .type = JSMN_OBJECT
            },
            {
                .key = "intvl",
                .type = JSMN_PRIMITIVE
            }
        };
        if(MIIO_OK != jsmi_get_value_uint(&jsmi_parser, rule_list_tok, intvl_path, NELEMENTS(intvl_path), &p_rule->interval)) {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "query_prod_cb: cannot parse intvl");
            ret = MIIO_ERROR_NOTFOUND;
            goto end;
        }
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "query_prod_cb: interval = %d", p_rule->interval);

        jsmi_tok_path_t delta_path[] = {
            {
                .key = (void*)i,
                .type = JSMN_OBJECT
            },
            {
                .key = "delta",
                .type = JSMN_PRIMITIVE
            }
        };
        if(MIIO_OK != jsmi_get_value_uint(&jsmi_parser, rule_list_tok, delta_path, NELEMENTS(delta_path), &p_rule->delta)) {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "query_prod_cb: cannot parse delta");
            ret = MIIO_ERROR_NOTFOUND;
            goto end;
        }
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "query_prod_cb: delta = %d", p_rule->delta);

        // do not add object to pid
        if (p_pid_obj_id->obj_num >= MAX_OBJECT_ID_IN_PID) {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "query_prod_cb: obj_num >= MAX_OBJECT_ID_IN_PID");
            ret = MIIO_ERROR_FULL;
            goto end;
        }
        p_pid_obj_id->obj_id[p_pid_obj_id->obj_num] = p_rule->obj_id;
        p_pid_obj_id->obj_num ++;

        object_rule_add(p_rule);
    }

    // NOTE: do not forget to delete denide pid list
    unknown_pid_delete(p_pid_obj_id->pid);
    pid_obj_id_add(p_pid_obj_id);

    // pid_obj_id_print();
    // object_rule_print();

end:
    jsmi_parse_finish(&jsmi_parser);

    if (NULL != p_pid_obj_id) {
        free(p_pid_obj_id);
        p_pid_obj_id = NULL;
    }

    if (NULL != p_rule) {
        free(p_rule);
        p_rule = NULL;
    }

    return ret;
}

// _async.ble_event
// {"method":"_async.ble_event", "params":{"dev":{"did":"blt.xxxx", "mac":"11:22:33:44:55:66", "pdid":206},"evt":[{"eid":1001, "edata":"xxxxxx"},{"eid":2001, "edata":"xxxxxx"}],"frmCnt":199, "gwts":123456}}
// {"result":[“ok”]}
// {"error":{"code":-x,"message":""}}
int mible_rpc_report_evt(authed_dev_t *p_dev, mibeacon_object_array_t *p_object_array)
{
    int ret = MIIO_OK;
    char *buffer = NULL;
    size_t buf_size = 512; // OTS_PACKET_PLOAD_SIZE_MAX

    miio_handle_t miio_handle = mible_net_handle_get();
    if (NULL == miio_handle) {
        return MIIO_ERROR_NOTREADY;
    }

    if (!mible_net_state_check()) {
        return MIIO_ERROR_NOTREADY;
    }

    uint32_t method_id = miio_get_rpc_id(miio_handle);

    buffer = (char *)malloc(sizeof(char) * buf_size);
    if (NULL == buffer) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for report evt buffer");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memset((uint8_t *)buffer, 0, buf_size);

    char mac_string[18] = {0};
    snprintf(mac_string, sizeof(mac_string), "%02X:%02X:%02X:%02X:%02X:%02X",
             p_dev->mac[5], p_dev->mac[4], p_dev->mac[3], p_dev->mac[2], p_dev->mac[1], p_dev->mac[0]);

    jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(buffer, buf_size);
    jsmi_compose_start(&jsmi_composer);
    jsmi_set_object_begin(&jsmi_composer);
        jsmi_set_key_value_uint(&jsmi_composer, "id", method_id);
        jsmi_set_key_value_str(&jsmi_composer, "method", "_async.ble_event", strlen("_async.ble_event"));
        jsmi_set_key_object_begin(&jsmi_composer, "params");
            jsmi_set_key_object_begin(&jsmi_composer, "dev");
                jsmi_set_key_value_str(&jsmi_composer, "did", (char *)p_dev->did, strlen((char *)p_dev->did));
                jsmi_set_key_value_str(&jsmi_composer, "mac", mac_string, strlen(mac_string));
                jsmi_set_key_value_uint(&jsmi_composer, "pdid", p_dev->pid);
            jsmi_set_key_object_end(&jsmi_composer);

            jsmi_set_key_array_begin(&jsmi_composer, "evt");
                for (int i = 0; i < p_object_array->num; i++) {
                    jsmi_set_value_object_begin(&jsmi_composer);
                        jsmi_set_key_value_uint(&jsmi_composer, "eid", p_object_array->object[i].id);
                        char tmp[32] = {0};
                        snprintf_hex(tmp, sizeof(tmp), p_object_array->object[i].data, p_object_array->object[i].data_len, 0);
                        jsmi_set_key_value_str(&jsmi_composer, "edata", tmp, strlen(tmp));
                    jsmi_set_value_object_end(&jsmi_composer);
                }
            jsmi_set_key_array_end(&jsmi_composer);

            jsmi_set_key_value_uint(&jsmi_composer, "frmCnt", p_dev->counter & 0xFF);
            jsmi_set_key_value_uint(&jsmi_composer, "gwts", arch_os_time_now());

        jsmi_set_key_object_end(&jsmi_composer);
    jsmi_set_object_end(&jsmi_composer);

    if(MIIO_OK != jsmi_compose_finish(&jsmi_composer, NULL, NULL)){
        ret = MIIO_ERROR_NOTFOUND;
        goto end;
    }

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "mible_rpc_report_evt = %s", buffer);
    ret = rpc_upload_add(buffer, method_id, MIBLE_RPC_REPORT_EVT);

end:
    if (NULL != buffer) {
        free(buffer);
        buffer = NULL;
    }
    return ret;
}

static int mible_report_evt_cb(miio_rpc_delegate_arg_t *ack_arg, void *ctx)
{
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "mible_report_evt_cb");
    return MIIO_OK;
}

// _sync.ble_keep_alive
// {"method":"_sync.ble_keep_alive","params":[{"did":"350385098","rssi":-30},{"did":"684958","rssi":-53}]}
// {"result":{"operation":"keep_alive","intvl":1200,"delta":60,"filter":[{"mac":"84:68:3E:00:7A:E3","pdid":156,"ttl":1800},{"mac":"84:a8:34:04:71:46","pdid":156,"ttl":1800}]}}
// {"error":{"code":-x,"message":"keep_alive"}}

int mible_rpc_keep_alive(void)
{
    int ret = MIIO_OK;
    char *buffer = NULL;
    size_t buf_size = 512; // OTS_PACKET_PLOAD_SIZE_MAX

    miio_handle_t miio_handle = mible_net_handle_get();
    if (NULL == miio_handle) {
        return MIIO_ERROR_NOTREADY;
    }

    if (!mible_net_state_check()) {
        return MIIO_ERROR_NOTREADY;
    }

    buffer = (char *)malloc(sizeof(char) * buf_size);
    if (NULL == buffer) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for keep alive buffer");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memset((uint8_t *)buffer, 0, buf_size);

    uint8_t dev_count = keep_alive_dev_get_count();
    uint8_t dev_count_10 = dev_count / 10;
    if (dev_count % 10 != 0) {
        dev_count_10 += 1;
    }
    for (int i = 0; i < dev_count_10; i++) {
        memset((uint8_t *)buffer, 0, buf_size);

        uint32_t method_id = miio_get_rpc_id(miio_handle);

        jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(buffer, buf_size);
        jsmi_compose_start(&jsmi_composer);
        jsmi_set_object_begin(&jsmi_composer);
            jsmi_set_key_value_uint(&jsmi_composer, "id", method_id);
            jsmi_set_key_value_str(&jsmi_composer, "method", "_sync.ble_keep_alive", strlen("_sync.ble_keep_alive"));

            jsmi_set_key_array_begin(&jsmi_composer, "params");
            // get all keep alive dev
            if ((i + 1) * 10 <= dev_count) {
                keep_alive_dev_get_string(&jsmi_composer, i * 10, (i + 1) * 10);
            } else {
                keep_alive_dev_get_string(&jsmi_composer, i * 10, dev_count);
            }
            jsmi_set_key_array_end(&jsmi_composer);
        jsmi_set_object_end(&jsmi_composer);

        if(MIIO_OK != jsmi_compose_finish(&jsmi_composer, NULL, NULL)){
            ret = MIIO_ERROR_NOTFOUND;
            goto end;
        }

        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "mible_rpc_keep_alive = %s", buffer);
        ret = rpc_upload_add(buffer, method_id, MIBLE_RPC_KEEP_ALIVE);
    }

end:

    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }

    return ret;
}

int mible_rpc_single_alive(keep_alive_dev_t *p_dev)
{
    int ret = MIIO_OK;
    char *buffer = NULL;
    size_t buf_size = 512; // OTS_PACKET_PLOAD_SIZE_MAX
    uint32_t method_id;

    miio_handle_t miio_handle = mible_net_handle_get();
    if (NULL == miio_handle) {
        return MIIO_ERROR_NOTREADY;
    }

    if (!mible_net_state_check()) {
        return MIIO_ERROR_NOTREADY;
    }

    buffer = (char *)malloc(sizeof(char) * buf_size);
    if (NULL == buffer) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for single alive buffer");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memset((uint8_t *)buffer, 0, buf_size);

    method_id = miio_get_rpc_id(miio_handle);

    jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(buffer, buf_size);
    jsmi_compose_start(&jsmi_composer);
    jsmi_set_object_begin(&jsmi_composer);
        jsmi_set_key_value_uint(&jsmi_composer, "id", method_id);
        jsmi_set_key_value_str(&jsmi_composer, "method", "_sync.ble_keep_alive", strlen("_sync.ble_keep_alive"));

        jsmi_set_key_array_begin(&jsmi_composer, "params");
        // get all keep alive dev
            jsmi_set_value_object_begin(&jsmi_composer);
                jsmi_set_key_value_str(&jsmi_composer, "did", (char *)p_dev->did, strlen((char *)p_dev->did));
                jsmi_set_key_value_sint(&jsmi_composer, "rssi", p_dev->rssi);
            jsmi_set_value_object_end(&jsmi_composer);
        jsmi_set_key_array_end(&jsmi_composer);
    jsmi_set_object_end(&jsmi_composer);

    if(MIIO_OK != jsmi_compose_finish(&jsmi_composer, NULL, NULL)){
        ret = MIIO_ERROR_NOTFOUND;
        goto end;
    }

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "mible_rpc_single_alive = %s", buffer);
    ret = rpc_upload_add(buffer, method_id, MIBLE_RPC_KEEP_ALIVE);

end:

    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }

    return ret;
}

static int mible_keep_alive_cb(miio_rpc_delegate_arg_t *ack_arg, void *ctx)
{
    int ret = MIIO_OK;

    uint32_t interval, delta;

    denied_dev_t *p_dev = NULL;

    char *buffer = NULL;
    size_t buf_size = 128;

    if (NULL == ack_arg) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "keep alive cb invalid param");
        return MIIO_ERROR_PARAM;
    }

    jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(ack_arg->pload, ack_arg->pload_len);
    ret = jsmi_parse_start(&jsmi_parser);
    if (MIIO_OK != ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "mible_keep_alive_cb: JSMI init error, return");
        return ret;
    }

    buffer = (char *)malloc(sizeof(char) * buf_size);
    if (NULL == buffer) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for keep alive cb buffer");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memset((uint8_t *)buffer, 0, buf_size);

    jsmi_tok_path_t error_path[] = {
        {
            .key = "error",
            .type = JSMN_OBJECT
        }
    };
    jsmntok_t *error_tok = jsmi_get_tok(&jsmi_parser, NULL, error_path, NELEMENTS(error_path));
    if(NULL != error_tok) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "mible_query_prod_cb: found error");
        ret = MIIO_ERROR_NOTFOUND;
        goto end;
    }

    jsmi_tok_path_t intvl_path[] = {
        {
            .key = "result",
            .type = JSMN_OBJECT
        },
        {
            .key = "intvl",
            .type = JSMN_PRIMITIVE
        }
    };
    if (MIIO_OK != jsmi_get_value_uint(&jsmi_parser, NULL, intvl_path, NELEMENTS(intvl_path), &interval)) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "keep_alive_cb: cannot parse intvl");
        ret = MIIO_ERROR_NOTFOUND;
        goto end;
    }
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "keep_alive_cb: interval = %d", interval);

    jsmi_tok_path_t delta_path[] = {
        {
            .key = "result",
            .type = JSMN_OBJECT
        },
        {
            .key = "delta",
            .type = JSMN_PRIMITIVE
        }
    };
    if (MIIO_OK != jsmi_get_value_uint(&jsmi_parser, NULL, delta_path, NELEMENTS(delta_path), &delta)) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "keep_alive_cb: cannot parse delta");
        ret = MIIO_ERROR_NOTFOUND;
        goto end;
    }
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "keep_alive_cb: delta = %d", delta);
    keep_alive_set_config(interval, delta);

    jsmi_tok_path_t filter_path[] = {
        {
            .key = "result",
            .type = JSMN_OBJECT
        },
        {
            .key = "filter",
            .type = JSMN_ARRAY
        }
    };
    jsmntok_t *filter_list_tok = jsmi_get_tok(&jsmi_parser, NULL, filter_path, NELEMENTS(filter_path));
    if (NULL == filter_list_tok) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "keep_alive_cb: cannot find filter list");
        ret = MIIO_ERROR_NOTFOUND;
        goto end;
    }

    p_dev = (denied_dev_t *)malloc(sizeof(denied_dev_t));
    if (NULL == p_dev) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for denied_dev");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memset((uint8_t *)p_dev, 0, sizeof(denied_dev_t));

    for (int i = 0; i < filter_list_tok->size; i++) {

        memset((uint8_t *)p_dev, 0, sizeof(denied_dev_t));

        jsmi_tok_path_t mac_path[] = {
            {
                .key = (void*)i,
                .type = JSMN_OBJECT
            },
            {
                .key = "mac",
                .type = JSMN_STRING
            }
        };
        memset((uint8_t *)buffer, 0, buf_size);
        if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, filter_list_tok, mac_path, NELEMENTS(mac_path), buffer, buf_size)) {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "keep_alive_cb: cannot parse mac");
            ret = MIIO_ERROR_NOTFOUND;
            goto end;
        }
        if (MIIO_OK != mac_convert(buffer, (uint8_t *)p_dev->mac)) {
            ret = MIIO_ERROR_NOTFOUND;
            goto end;
        }

        jsmi_tok_path_t pid_path[] = {
            {
                .key = (void*)i,
                .type = JSMN_OBJECT
            },
            {
                .key = "pdid",
                .type = JSMN_PRIMITIVE
            }
        };
        if(MIIO_OK != jsmi_get_value_u16(&jsmi_parser, filter_list_tok, pid_path, NELEMENTS(pid_path), &p_dev->pid)) {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "keep_alive_cb: cannot parse pid");
            ret = MIIO_ERROR_NOTFOUND;
            goto end;
        }
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "keep_alive_cb: pid = %d", p_dev->pid);

        jsmi_tok_path_t ttl_path[] = {
            {
                .key = (void*)i,
                .type = JSMN_OBJECT
            },
            {
                .key = "ttl",
                .type = JSMN_PRIMITIVE
            }
        };
        if(MIIO_OK != jsmi_get_value_uint(&jsmi_parser, filter_list_tok, ttl_path, NELEMENTS(ttl_path), &p_dev->ttl)) {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "keep_alive_cb: cannot parse ttl");
            ret = MIIO_ERROR_NOTFOUND;
            goto end;
        }
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "keep_alive_cb: ttl = %d", p_dev->ttl);

        denied_dev_add(p_dev);
    }
    // denied_dev_print();

end:
    jsmi_parse_finish(&jsmi_parser);

    if (NULL != buffer) {
        free(buffer);
        buffer = NULL;
    }

    if (NULL != p_dev) {
        free(p_dev);
        p_dev = NULL;
    }

    return ret;
}

int mible_rpc_gateway_enable(void)
{
    int ret = MIIO_OK;
    char *buffer = NULL;
    size_t buf_size = 512; // OTS_PACKET_PLOAD_SIZE_MAX
    uint32_t method_id;

    miio_handle_t miio_handle = mible_net_handle_get();
    if (NULL == miio_handle) {
        return MIIO_ERROR_NOTREADY;
    }

    if (!mible_net_state_check()) {
        return MIIO_ERROR_NOTREADY;
    }

    buffer = (char *)malloc(sizeof(char) * buf_size);
    if (NULL == buffer) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for checking gateway state");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memset((uint8_t *)buffer, 0, buf_size);

    method_id = miio_get_rpc_id(miio_handle);
    jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(buffer, buf_size);

    jsmi_compose_start(&jsmi_composer);
    jsmi_set_object_begin(&jsmi_composer);
        jsmi_set_key_value_uint(&jsmi_composer, "id", method_id);
        jsmi_set_key_value_str(&jsmi_composer, "method", "_sync.ble_gateway_enable", strlen("_sync.ble_gateway_enable"));
        jsmi_set_key_object_begin(&jsmi_composer, "params");
        jsmi_set_key_object_end(&jsmi_composer);
    jsmi_set_object_end(&jsmi_composer);

    if(MIIO_OK != jsmi_compose_finish(&jsmi_composer, NULL, NULL)){
        ret = MIIO_ERROR_NOTFOUND;
        goto end;
    }

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "mible_rpc_gateway_enable = %s", buffer);
    ret = rpc_upload_add(buffer, method_id, MIBLE_RPC_GATEWAY_ENABLE);
    if (MIIO_OK != ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "Add rpc into list fail (err %d)", ret);
        goto end;
    }

    gateway_state.last_timestamp = arch_os_time_now();
    gateway_state.cur_state = MIBLE_GATEWAY_STATE_PENDING;

end:
    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }

    return ret;
}

static int mible_gateway_enable_cb(miio_rpc_delegate_arg_t *ack_arg, void *ctx)
{
    int ret;

    if (NULL == ack_arg) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "invalid param");
        return MIIO_ERROR_PARAM;
    }

    jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(ack_arg->pload, ack_arg->pload_len);
    ret = jsmi_parse_start(&jsmi_parser);
    if (MIIO_OK != ret) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "start jsmi parser fail (err %d)", ret);
        return ret;
    }

    jsmi_tok_path_t error_path[] = {
        {
            .key = "error",
            .type = JSMN_OBJECT
        }
    };
    jsmntok_t *error_tok = jsmi_get_tok(&jsmi_parser, NULL, error_path, NELEMENTS(error_path));
    if (NULL == error_tok) {
        ret = gateway_enable_handler(&jsmi_parser, "result");
        goto end;
    }

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "found error");

    int32_t code;
    jsmi_tok_path_t code_path[] = {
        {
            .key = "code",
            .type = JSMN_PRIMITIVE
        }
    };
    ret = jsmi_get_value_sint32(&jsmi_parser, error_tok, code_path, NELEMENTS(code_path), &code);
    if (MIIO_OK != ret) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "parse error code fail (err %d)", ret);
        goto end;
    }

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "error code %d", code);
    switch (code) {
        case MIBLE_RPC_LEGACY_ERR_SERVICE:
        case MIBLE_RPC_LEGACY_ERR_FREQUENT:
        case MIBLE_RPC_ERR_CODE_FREQUENT:
        case MIBLE_RPC_ERR_CODE_SERVICE:
            gateway_state.back_off = gateway_state.back_off * 2 + 10;
            if (gateway_state.back_off > MIBLE_GATEWAY_BACKOFF_MAX_VALUE) {
                gateway_state.back_off = MIBLE_GATEWAY_BACKOFF_MAX_VALUE;
            }
            break;
        case MIBLE_RPC_LEGACY_ERR_REQUEST:
        case MIBLE_RPC_ERR_CODE_SUPPORT:
            if (++gateway_state.retry_times > MIBLE_GATEWAY_BACKOFF_MAX_RETRY_TIMES) {
                gateway_state.back_off = MIBLE_GATEWAY_BACKOFF_FIXED_VALUE;
            } else {
                gateway_state.back_off = gateway_state.back_off * 4 + 10;
            }
            break;
        default:
            LOG_WARN_TAG(MIBLE_LOG_TAG, "unknown error code %d", code);
            ret = MIIO_ERROR_PARAM;
            goto end;
    }
    gateway_state.cur_state = MIBLE_GATEWAY_STATE_UNKNOWN;

end:
    jsmi_parse_finish(&jsmi_parser);
    return ret;
}

static int mible_search_result_evt_cb(miio_rpc_delegate_arg_t *ack_arg, void *ctx)
{
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "mible_report_evt_cb");
    return MIIO_OK;
}

// _async.ble_notify_linkage_search_result
// {"id":xxxxxxxx,"method":"miIO.ble_notify_linkage_search_result","params":{"list":[{"mac":"50:EC:50:CB:01:91","pdid":xx,"rssi":xx},{"mac":"50:EC:50:CB:01:92","pdid":xx,"rssi":xx}]}}
// {"result":["ok"]}
// {"error":{"code":-x,"message":""}}
int mible_rpc_search_result_evt(search_target_array_t *result)
{
    int ret = MIIO_OK;
    char *buffer = NULL;
    size_t buf_size = 512; // OTS_PACKET_PLOAD_SIZE_MAX
    char mac_string[18] = {0};

    miio_handle_t miio_handle = mible_net_handle_get();
    if (NULL == miio_handle) {
        return MIIO_ERROR_NOTREADY;
    }

    if (!mible_net_state_check()) {
        return MIIO_ERROR_NOTREADY;
    }

    uint32_t method_id = miio_get_rpc_id(miio_handle);

    buffer = (char *)malloc(sizeof(char) * buf_size);
    if (NULL == buffer) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for report evt buffer");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memset((uint8_t *)buffer, 0, buf_size);

    jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(buffer, buf_size);
    jsmi_compose_start(&jsmi_composer);
    jsmi_set_object_begin(&jsmi_composer);
        jsmi_set_key_value_uint(&jsmi_composer, "id", method_id);
        jsmi_set_key_value_str(&jsmi_composer, "method", "event.ble_scan_result", strlen("event.ble_scan_result"));
        jsmi_set_key_object_begin(&jsmi_composer, "params");
            jsmi_set_key_array_begin(&jsmi_composer, "list");
                for (int i = 0; i < result->cur_num; i++) {
                    jsmi_set_value_object_begin(&jsmi_composer);
                        snprintf(mac_string, sizeof(mac_string), "%02X:%02X:%02X:%02X:%02X:%02X",
                            result->target[i].address[5], result->target[i].address[4], result->target[i].address[3],
                            result->target[i].address[2], result->target[i].address[1], result->target[i].address[0]);
                        jsmi_set_key_value_str(&jsmi_composer, "mac", (char *)mac_string, strlen((char *)mac_string));
                        jsmi_set_key_value_uint(&jsmi_composer, "pdid", result->target[i].product_id);
                        jsmi_set_key_value_sint(&jsmi_composer, "rssi", result->target[i].rssi);
                    jsmi_set_value_object_end(&jsmi_composer);
                }
            jsmi_set_key_array_end(&jsmi_composer);
        jsmi_set_key_object_end(&jsmi_composer);
    jsmi_set_object_end(&jsmi_composer);

    if (MIIO_OK != jsmi_compose_finish(&jsmi_composer, NULL, NULL)) {
        ret = MIIO_ERROR;
        goto end;
    }

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "mible_search_result_evt = %s", buffer);
    rpc_upload_add(buffer, method_id, MIBLE_RPC_SCAN_RESULT_EVT);

end:
    if (NULL != buffer) {
        free(buffer);
        buffer = NULL;
    }

    return ret;
}

int mible_rpc_mesh_switch(uint8_t state)
{
    int ret = MIIO_OK;
    char *buffer = NULL;
    size_t buf_size = 512; // OTS_PACKET_PLOAD_SIZE_MAX
    uint32_t method_id;

    miio_handle_t miio_handle = mible_net_handle_get();
    if (NULL == miio_handle) {
        return MIIO_ERROR_NOTREADY;
    }

    if (!mible_net_state_check()) {
        return MIIO_ERROR_NOTREADY;
    }

    buffer = (char *)malloc(sizeof(char) * buf_size);
    if (NULL == buffer) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for report gateway state");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memset((uint8_t *)buffer, 0, buf_size);

    method_id = miio_get_rpc_id(miio_handle);
    jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(buffer, buf_size);

    jsmi_compose_start(&jsmi_composer);
    jsmi_set_object_begin(&jsmi_composer);
        jsmi_set_key_value_uint(&jsmi_composer, "id", method_id);
        jsmi_set_key_value_str(&jsmi_composer, "method", "props", strlen("props"));
        jsmi_set_key_object_begin(&jsmi_composer, "params");
            jsmi_set_key_value_str(&jsmi_composer, "ble_mesh_switch", state?"enable":"disable", state?strlen("enable"):strlen("disable"));
        jsmi_set_key_object_end(&jsmi_composer);
    jsmi_set_object_end(&jsmi_composer);

    if(MIIO_OK != jsmi_compose_finish(&jsmi_composer, NULL, NULL)){
        ret = MIIO_ERROR_NOTFOUND;
        goto end;
    }

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "mible_rpc_mesh_switch = %s", buffer);
    ret = rpc_upload_add(buffer, method_id, MIBLE_RPC_MESH_SWITCH);
    if (MIIO_OK != ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "Add rpc into list fail (err %d)", ret);
        goto end;
    }

end:
    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }

    return ret;
}


static int mible_mesh_switch_cb(miio_rpc_delegate_arg_t *ack_arg, void *ctx)
{
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "mible_mesh_switch_cb");
    return MIIO_OK;
}


typedef struct rpc_upload_list {
    char *s; //string, be careful
    uint32_t method_id;
    mible_rpc_type_t type;
    struct list_head list;
} rpc_upload_list_t;

static rpc_upload_list_t rpc_upload_list;
static uint8_t rpc_upload_list_count;

int rpc_upload_init(void)
{
    int ret = MIIO_OK;

    INIT_LIST_HEAD(&rpc_upload_list.list);
    rpc_upload_list_count = 0;

    return ret;
}

int rpc_upload_add(char *rpc, uint32_t method_id, mible_rpc_type_t type)
{
    int ret = MIIO_OK;
    static uint16_t error_num = 0;
    rpc_upload_list_t *tmp;

    if (NULL == rpc) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "rpc_upload_add invalid param");
        return MIIO_ERROR_PARAM;
    }

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);

    // do not need to check existed element
    if (rpc_upload_list_count >= MAX_RPC_UPLOAD_LIST_COUNT) {
        error_num++;
        if (MIBEACON_EVENT_ERROR_GATE == error_num) {
            LOG_WARN_TAG(MIBLE_LOG_TAG, "RPC table is full");
            error_num = 0;
        }
        ret = MIIO_ERROR_FULL;
        goto end;
    }

    tmp = (rpc_upload_list_t *)malloc(sizeof(rpc_upload_list_t));
    if (tmp == NULL) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for rpc_upload_list");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    tmp->s = (char *)malloc(sizeof(char) * (strlen(rpc) + 1));
    if (tmp->s == NULL) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for string in rpc_upload_list");
        // do not forge to free tmp
        if (tmp != NULL) {
            free(tmp);
            tmp = NULL;
        }
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memcpy(tmp->s, rpc, strlen(rpc) + 1);
    tmp->method_id = method_id;
    tmp->type = type;
    // LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Add rpc upload to list");

    struct list_head *pos, *q;
    rpc_upload_list_t *p_upload;
    bool is_need_add = true;

    if (tmp->type == MIBLE_RPC_REPORT_EVT) {
        list_for_each_safe(pos, q, &rpc_upload_list.list) {
            p_upload = list_entry(pos, rpc_upload_list_t, list);
            if (p_upload->type != MIBLE_RPC_REPORT_EVT) {
                list_add_tail(&(tmp->list), &(p_upload->list));
                is_need_add = false;
                break;
            }
        }
    }
    if (is_need_add) {
        list_add_tail(&(tmp->list), &(rpc_upload_list.list));
    }

    // if (type == GATEWAY_RPC_REPORT_EVT)
    //     list_add(&(tmp->list), &(rpc_upload_list.list));
    // else
    //     list_add_tail(&(tmp->list), &(rpc_upload_list.list));
    rpc_upload_list_count ++;

end:
    arch_os_mutex_put(mibeacon_list_mutex);

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "rpc_upload_add count = %d", rpc_upload_list_count);

    return ret;
}

int rpc_upload_print(void)
{
    int ret = MIIO_OK;
    rpc_upload_list_t *tmp;
    struct list_head *pos, *q;
    int index = 0;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "rpc_upload_list_count = %d", rpc_upload_list_count);
    list_for_each_safe(pos, q, &rpc_upload_list.list) {
        tmp = list_entry(pos, rpc_upload_list_t, list);
        UNUSED(tmp->s);
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "rpc_upload_list: index = %d, %s", index, tmp->s);
        index ++;
    }
    arch_os_mutex_put(mibeacon_list_mutex);

    return ret;
}

int get_first_rpc_and_upload(void)
{
    int ret = MIIO_OK;
    bool get_first_rpc = FALSE;
    rpc_upload_list_t *tmp;
    struct list_head *pos, *q;

    rpc_upload_list_t first;
    first.s = NULL;

    if (TRUE == list_empty(&rpc_upload_list.list)) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "rpc_upload_list is empty");
        return ret;
    }

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
    list_for_each_safe(pos, q, &rpc_upload_list.list) {
        if (!get_first_rpc) {
            get_first_rpc = TRUE;
            tmp = list_entry(pos, rpc_upload_list_t, list);

            // get the first node from list
            first.s = tmp->s;
            first.method_id = tmp->method_id;
            first.type = tmp->type;

            list_del(pos);
            free(tmp);
            rpc_upload_list_count --;
        }
    }
    arch_os_mutex_put(mibeacon_list_mutex);

    if (get_first_rpc) {
        // send to wifi and upload to server
        ret = send_rpc_to_wifi(first.s, first.method_id, first.type);
        if (MIIO_OK != ret) {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "send_rpc_to_wifi error!!");
        }

        // NOTE: do not forget to free memory
        // because first node, delete node and free memory run only once!
        if (first.s != NULL) {
            free(first.s);
            first.s = NULL;
        }
    }

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "get_first_rpc_and_upload count = %d", rpc_upload_list_count);

    return ret;
}

static int send_rpc_to_wifi(char* rpc, uint32_t method_id, mible_rpc_type_t type)
{
    int ret = MIIO_OK;

    if (NULL == rpc) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "send rpc to wifi invalid param");
        return MIIO_ERROR_PARAM;
    }

    miio_handle_t miio_handle = mible_net_handle_get();
    if (NULL == miio_handle) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Cannot get miio handle, discard rpc");
        return MIIO_ERROR_NOTREADY;
    }

    if (!mible_net_state_check()) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Wifi is not ready, discard rpc");
        return MIIO_ERROR_NOTREADY;
    }

    miio_rpc_delegate_context_t context;
    miio_rpc_context_init(&context);
    miio_rpc_context_config_delegate_arg(&context, method_id, MIIO_DELEGATE_JSON, rpc,
                                         strlen(rpc) + 1);

    switch (type)
    {
        case MIBLE_RPC_QUERY_DEV:
            miio_rpc_context_config_delegate_ack(&context, mible_query_dev_cb, NULL);
            break;
        case MIBLE_RPC_QUERY_PROB:
            miio_rpc_context_config_delegate_ack(&context, mible_query_prod_cb, NULL);
            break;
        case MIBLE_RPC_REPORT_EVT:
            miio_rpc_context_config_delegate_ack(&context, mible_report_evt_cb, NULL);
            break;
        case MIBLE_RPC_KEEP_ALIVE:
            miio_rpc_context_config_delegate_ack(&context, mible_keep_alive_cb, NULL);
            break;
        case MIBLE_RPC_GATEWAY_ENABLE:
            miio_rpc_context_config_delegate_ack(&context, mible_gateway_enable_cb, NULL);
            break;
        case MIBLE_RPC_SCAN_RESULT_EVT:
            miio_rpc_context_config_delegate_ack(&context, mible_search_result_evt_cb, NULL);
            break;            
        case MIBLE_RPC_MESH_SWITCH:
            miio_rpc_context_config_delegate_ack(&context, mible_mesh_switch_cb, NULL);
            break;
        default:
            break;
    }

    ret = miio_set_up_rpc_delegate(miio_handle, &context);

    return ret;
}

//{"id":xxxxxxxx,"method":"miIO.ble_start_linkage_search","params":{"list":[{"mac":"50:EC:50:CB:01:91","pdid":xx},{"mac":"50:EC:50:CB:01:92","pdid":xx}],"timeout":10}}
//{"id":xxxxxxxx, "result":["ok"]}
//{"error":{"code":-x,"message":""}}
static int do_ble_linkage_search(miio_rpc_delegate_arg_t *req_arg,
                            miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
    int ret = MIIO_ERROR_PARAM;
    char *buffer = NULL;
    size_t buf_size = 128;
    search_device_t *p_dev = NULL;
    search_target_array_t target_array;

    if (NULL == req_arg || NULL == ack) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "do_ble_start_linkage_search invalid param");
        return ret;
    }

    buffer = (char *)malloc(sizeof(char) * buf_size);
    if (NULL == buffer) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for linkage serarch buffer");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memset((uint8_t *)buffer, 0, buf_size);

    memset(&target_array, 0xFF, sizeof(search_target_array_t));
    target_array.cur_num = 0;

    if (MIIO_DELEGATE_JSON == req_arg->type) {
        jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(req_arg->pload, req_arg->pload_len);
        if(MIIO_OK == jsmi_parse_start(&jsmi_parser)) {
            jsmi_tok_path_t search_path[] = {
                {
                    .key = "params",
                    .type = JSMN_OBJECT
                },
                {
                    .key = "list",
                    .type = JSMN_ARRAY
                }
            };
            jsmntok_t *search_list_tok = jsmi_get_tok(&jsmi_parser, NULL, search_path, NELEMENTS(search_path));
            if (NULL == search_list_tok) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_start_linkage_search: cannot find list");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }

            if (search_list_tok->size > MIBLE_LINKAGE_SEARCH_MAX_NUM) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_start_linkage_search: exceed max search size");
                ret = MIIO_ERROR_SIZE;
                goto end;
            }

            p_dev = (search_device_t *)malloc(sizeof(search_device_t));
            if (NULL == p_dev) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for search device");
                ret = MIIO_ERROR_NOMEM;
                goto end;
            }

            for (int i = 0; i < search_list_tok->size; i++) {
                memset((uint8_t *)p_dev, 0, sizeof(search_device_t));

                jsmi_tok_path_t mac_path[] = {
                    {
                        .key = (void*)i,
                        .type = JSMN_OBJECT
                    },
                    {
                        .key = "mac",
                        .type = JSMN_STRING
                    }
                };
                memset((uint8_t *)buffer, 0, buf_size);
                if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, search_list_tok, mac_path, NELEMENTS(mac_path), buffer, buf_size)) {
                    LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_start_linkage_search: cannot parse mac");
                    ret = MIIO_ERROR_NOTFOUND;
                    goto end;
                }
                if (MIIO_OK != mac_convert(buffer, (uint8_t *)p_dev->address)) {
                    ret = MIIO_ERROR_NOTFOUND;
                    goto end;
                }

                jsmi_tok_path_t pid_path[] = {
                    {
                        .key = (void*)i,
                        .type = JSMN_OBJECT
                    },
                    {
                        .key = "pdid",
                        .type = JSMN_PRIMITIVE
                    }
                };
                if(MIIO_OK != jsmi_get_value_u16(&jsmi_parser, search_list_tok, pid_path, NELEMENTS(pid_path), &p_dev->product_id)) {
                    LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_start_linkage_search: cannot parse pid");
                    ret = MIIO_ERROR_NOTFOUND;
                    goto end;
                }
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "do_ble_start_linkage_search: pid = %d", p_dev->product_id);

                p_dev->rssi = MIBLE_LINKAGE_DEFAULT_RSSI;
                memcpy(&target_array.target[target_array.cur_num], p_dev, sizeof(search_device_t));
                target_array.cur_num++;
            }

            jsmi_tok_path_t timeout_path[] = {
                {
                    .key = "params",
                    .type = JSMN_OBJECT
                },
                {
                    .key = "timeout",
                    .type = JSMN_PRIMITIVE
                }
            };
            uint32_t timeout = 0;
            if (MIIO_OK != jsmi_get_value_u32(&jsmi_parser, NULL, timeout_path, NELEMENTS(timeout_path), &timeout)) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_start_linkage_search: cannot find timeout");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }
            jsmi_parse_finish(&jsmi_parser);
            ret = mible_linkage_search_start(&target_array, timeout);
        }
    }

end:
    if (ret == MIIO_OK) {
        miio_rpc_delegate_ack_ok(req_arg, ack, ctx);
    } else if (ret == MIIO_ERROR_FULL) {
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_FULL, ack, ctx);
    } else if (ret == MIIO_ERROR_NOTFOUND){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_NOTFOUND, ack, ctx);
    }else if (ret == MIIO_ERROR_SIZE){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_SIZE, ack, ctx);
    } else if (ret == MIIO_ERROR_NOMEM){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_NOMEM, ack, ctx);
    } else {
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_PARAM, ack, ctx);
    }

    if (NULL != buffer) {
        free(buffer);
        buffer = NULL;
    }

    if (NULL != p_dev) {
        free(p_dev);
        p_dev = NULL;
    }

    return ret;
}
MIIO_RPC(ble_scan, do_ble_linkage_search, "Hello, MIoT");

//{"id":123457,"method":"miIO.ble_config_linkage","params":{"mac":"50:EC:50:CB:01:91","pdid":156,"beacon_key":"00112233445566778899001122334455","intvl":10,"fun_type":1,"rule":[{"object_id":4101,"value_type":1},{"object_id":4102,"value_type":2}]}
//{"id": 123457, "result":["ok"]}
//{"error":{"code":-x,"message":""}}
static int do_ble_add_linkage(miio_rpc_delegate_arg_t *req_arg,
                            miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
    int ret = MIIO_ERROR_PARAM;
    char *buffer = NULL;
    size_t buf_size = 128;
    linkage_device_t *p_dev = NULL;

    if (NULL == req_arg || NULL == ack) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "do_ble_add_linkage invalid param");
        return MIIO_ERROR_PARAM;
    }

    buffer = (char *)malloc(sizeof(char) * buf_size);
    if (NULL == buffer) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for add linkage buffer");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memset((uint8_t *)buffer, 0, buf_size);

    if (MIIO_DELEGATE_JSON == req_arg->type) {
        jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(req_arg->pload, req_arg->pload_len);
        if(MIIO_OK == jsmi_parse_start(&jsmi_parser)) {
            p_dev = (linkage_device_t *)malloc(sizeof(linkage_device_t));
            if (NULL == p_dev) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for linkage device");
                ret = MIIO_ERROR_NOMEM;
                goto end;
            }
            memset((uint8_t *)p_dev, 0xFF, sizeof(linkage_device_t));

            jsmi_tok_path_t mac_path[] = {
                {
                    .key = "params",
                    .type = JSMN_OBJECT
                },
                {
                    .key = "mac",
                    .type = JSMN_STRING
                }
            };
            memset((uint8_t *)buffer, 0, buf_size);
            if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, NULL, mac_path, NELEMENTS(mac_path), buffer, buf_size)) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_add_linkage: cannot parse mac");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }
            if (MIIO_OK != mac_convert(buffer, (uint8_t *)p_dev->address)) {
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }

            jsmi_tok_path_t pid_path[] = {
                {
                    .key = "params",
                    .type = JSMN_OBJECT
                },
                {
                    .key = "pdid",
                    .type = JSMN_PRIMITIVE
                }
            };
            if(MIIO_OK != jsmi_get_value_u16(&jsmi_parser, NULL, pid_path, NELEMENTS(pid_path), &p_dev->product_id)) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_add_linkage: cannot parse pid");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }

            jsmi_tok_path_t beaconkey_path[] = {
                {
                    .key = "params",
                    .type = JSMN_OBJECT
                },
                {
                    .key = "beacon_key",
                    .type = JSMN_STRING
                }
            };
            memset((uint8_t *)buffer, 0, buf_size);
            if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, NULL, beaconkey_path, NELEMENTS(beaconkey_path), buffer, buf_size)) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_add_linkage: cannot parse beaconkey_path");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }
            // special for hey+ band beaconkey length = 20
            if (strlen(buffer) < MIBEACON_BEACONKEY_LEN * 2) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_add_linkage: beaconkey length error");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }
            arch_str2hex(p_dev->beaconkey, buffer, MIBEACON_BEACONKEY_LEN);

            jsmi_tok_path_t intvl_path[] = {
                {
                    .key = "params",
                    .type = JSMN_OBJECT
                },
                {
                    .key = "intvl",
                    .type = JSMN_PRIMITIVE
                }
            };
            if(MIIO_OK != jsmi_get_value_u32(&jsmi_parser, NULL, intvl_path, NELEMENTS(intvl_path), &p_dev->interval)) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_add_linkage: cannot parse intvl");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }

            jsmi_tok_path_t fun_type_path[] = {
                {
                    .key = "params",
                    .type = JSMN_OBJECT
                },
                {
                    .key = "fun_type",
                    .type = JSMN_PRIMITIVE
                }
            };
            if(MIIO_OK != jsmi_get_value_u16(&jsmi_parser, NULL, fun_type_path, NELEMENTS(fun_type_path), &p_dev->function_type)) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_add_linkage: cannot parse fun_type");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }

            jsmi_tok_path_t rule_path[] = {
                {
                    .key = "params",
                    .type = JSMN_OBJECT
                },
                {
                    .key = "rule",
                    .type = JSMN_ARRAY
                }
            };
            jsmntok_t *rule_list_tok = jsmi_get_tok(&jsmi_parser, NULL, rule_path, NELEMENTS(rule_path));
            if (NULL == rule_list_tok) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_add_linkage: cannot find rule");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }

            if (rule_list_tok->size > MIBLE_LINKAGE_OBJECT_MAX_NUM) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_add_linkage:object exceed max num");
                ret = MIIO_ERROR_SIZE;
                goto end;
            }

            for (int i = 0; i < rule_list_tok->size; i++) {
                jsmi_tok_path_t objectid_path[] = {
                    {
                        .key = (void*)i,
                        .type = JSMN_OBJECT
                    },
                    {
                        .key = "object_id",
                        .type = JSMN_PRIMITIVE
                    }
                };
                if(MIIO_OK != jsmi_get_value_u16(&jsmi_parser, rule_list_tok, objectid_path, NELEMENTS(objectid_path), &p_dev->object[i].object_id)) {
                    LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_add_linkage: cannot parse object_id");
                    ret = MIIO_ERROR_NOTFOUND;
                    goto end;
                }

                jsmi_tok_path_t value_type_path[] = {
                    {
                        .key = (void*)i,
                        .type = JSMN_OBJECT
                    },
                    {
                        .key = "value_type",
                        .type = JSMN_PRIMITIVE
                    }
                };
                if(MIIO_OK != jsmi_get_value_u8(&jsmi_parser, rule_list_tok, value_type_path, NELEMENTS(value_type_path), &p_dev->object[i].value_type)) {
                    LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_add_linkage: cannot parse value_type");
                    ret = MIIO_ERROR_NOTFOUND;
                    goto end;
                }

                p_dev->object[i].timestamp = 0;
            }

            jsmi_parse_finish(&jsmi_parser);

            p_dev->enabled = LINKAGE_FUNCTION_DISABLE;
            p_dev->frame_counter = 0;
            ret = mible_linkage_add(p_dev);
        }
    }

end:
    if (ret == MIIO_OK) {
        miio_rpc_delegate_ack_ok(req_arg, ack, ctx);
    } else if (ret == MIIO_ERROR_FULL) {
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_FULL, ack, ctx);
    } else if (ret == MIIO_ERROR_NOTFOUND){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_NOTFOUND, ack, ctx);
    }else if (ret == MIIO_ERROR_SIZE){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_SIZE, ack, ctx);
    } else if (ret == MIIO_ERROR_NOMEM){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_NOMEM, ack, ctx);
    } else {
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_PARAM, ack, ctx);
    }

    if (NULL != buffer) {
        free(buffer);
        buffer = NULL;
    }

    if (NULL != p_dev) {
        free(p_dev);
        p_dev = NULL;
    }

    return ret;
}
MIIO_RPC(ble_add_linkage, do_ble_add_linkage, "Hello, MIoT");

//{"id":123458,"method":"miIO.ble_del_linkage","params":{"op":0}}
//{"id":123458,"method":"miIO.ble_del_linkage","params":{"op":1,"mac":"50:EC:50:CB:01:91"}}
//{"id": 123457, "result":["ok"]}
//{"error":{"code":-x,"message":""}}
static int do_ble_del_linkage(miio_rpc_delegate_arg_t *req_arg,
                            miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
    int ret = MIIO_ERROR_PARAM;
    linkage_device_t *p_dev = NULL;

    if (NULL == req_arg || NULL == ack) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "do_ble_del_linkage invalid param");
        return MIIO_ERROR_PARAM;
    }

    if (MIIO_DELEGATE_JSON == req_arg->type) {
        jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(req_arg->pload, req_arg->pload_len);
        if(MIIO_OK == jsmi_parse_start(&jsmi_parser)) {
            jsmi_tok_path_t op_path[] = {
                {
                    .key = "params",
                    .type = JSMN_OBJECT
                },
                {
                    .key = "op",
                    .type = JSMN_PRIMITIVE
                }
            };
            int op = 0;
            if(MIIO_OK != jsmi_get_value_sint(&jsmi_parser, NULL, op_path, NELEMENTS(op_path), &op)) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_del_linkage: cannot parse op");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }

            if (op == 0) {
                ret = mible_linkage_reset();
                goto end;
            }

            p_dev = (linkage_device_t *)malloc(sizeof(linkage_device_t));
            if (NULL == p_dev) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for linkage device");
                ret = MIIO_ERROR_NOMEM;
                goto end;
            }
            memset((uint8_t *)p_dev, 0xFF, sizeof(linkage_device_t));
            char mac_string[18] = {0};

            jsmi_tok_path_t mac_path[] = {
                {
                    .key = "params",
                    .type = JSMN_OBJECT
                },
                {
                    .key = "mac",
                    .type = JSMN_STRING
                }
            };
            if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, NULL, mac_path, NELEMENTS(mac_path), mac_string, sizeof(mac_string))) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_del_linkage: cannot parse mac");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }
            if (MIIO_OK != mac_convert(mac_string, (uint8_t *)p_dev->address)) {
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }

            jsmi_tok_path_t pid_path[] = {
                {
                    .key = "params",
                    .type = JSMN_OBJECT
                },
                {
                    .key = "pdid",
                    .type = JSMN_PRIMITIVE
                }
            };
            if(MIIO_OK != jsmi_get_value_u16(&jsmi_parser, NULL, pid_path, NELEMENTS(pid_path), &p_dev->product_id)) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_add_linkage: cannot parse pid");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }

            jsmi_parse_finish(&jsmi_parser);

            ret = mible_linkage_delete(p_dev);
        }
    }

end:
    if (ret == MIIO_OK) {
        miio_rpc_delegate_ack_ok(req_arg, ack, ctx);
    } else if (ret == MIIO_ERROR_FULL) {
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_FULL, ack, ctx);
    } else if (ret == MIIO_ERROR_NOTFOUND){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_NOTFOUND, ack, ctx);
    }else if (ret == MIIO_ERROR_SIZE){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_SIZE, ack, ctx);
    } else if (ret == MIIO_ERROR_NOMEM){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_NOMEM, ack, ctx);
    } else {
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_PARAM, ack, ctx);
    }

    if (NULL != p_dev) {
        free(p_dev);
        p_dev = NULL;
    }

    return ret;
}
MIIO_RPC(ble_del_linkage, do_ble_del_linkage, "Hello, MIoT");

//{"id":1234,"method":"miIO.ble_get_linkage","params":{"op":0}}
//{"id":1234,"method":"miIO.ble_get_linkage","params":{"op":1,"mac":"50:EC:50:CB:01:91"}} 
//{"id":1234,"result":{"device":[{"fun_type":1,"eanbled":1,"mac":"50:EC:50:CB:01:91","pdid":156,"rule":[{"object_id":4101},{"object_id":4102},{"object_id":4103}]},{"fun_type":2,"eanbled":0,"mac":"50:EC:50:CB:01:92","pdid":157,"rule":[{"object_id":4101},{"object_id":4105}]}]}}
static int do_ble_get_linkage(miio_rpc_delegate_arg_t *req_arg,
                            miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
    int ret = MIIO_ERROR_PARAM;
    int op = 0;
    char mac_string[18] = {0};
    linkage_device_array_t device_list;

    if (NULL == req_arg || NULL == ack) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "do_ble_get_linkage invalid param");
        return MIIO_ERROR_PARAM;
    }

    if (MIIO_DELEGATE_JSON == req_arg->type) {
        jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(req_arg->pload, req_arg->pload_len);
        if(MIIO_OK == jsmi_parse_start(&jsmi_parser)) {
            jsmi_tok_path_t op_path[] = {
                {
                    .key = "params",
                    .type = JSMN_OBJECT
                },
                {
                    .key = "op",
                    .type = JSMN_PRIMITIVE
                }
            };
            if(MIIO_OK != jsmi_get_value_sint(&jsmi_parser, NULL, op_path, NELEMENTS(op_path), &op)) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_get_linkage: cannot parse op");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }

            memset(&device_list, 0xFF, sizeof(linkage_device_array_t));
            device_list.cur_num = 0;
            if (op == 0) {
                ret = mible_linkage_get_all(&device_list);
                if(MIIO_OK != ret) {
                    goto end;
                }
            } else {
                jsmi_tok_path_t mac_path[] = {
                    {
                        .key = "params",
                        .type = JSMN_OBJECT
                    },
                    {
                        .key = "mac",
                        .type = JSMN_STRING
                    }
                };
                if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, NULL, mac_path, NELEMENTS(mac_path), mac_string, sizeof(mac_string))) {
                    LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_get_linkage: cannot parse mac");
                    ret = MIIO_ERROR_NOTFOUND;
                    goto end;
                }
                if (MIIO_OK != mac_convert(mac_string, (uint8_t *)device_list.device[0].address)) {
                    ret = MIIO_ERROR_NOTFOUND;
                    goto end;
                }

                jsmi_tok_path_t pid_path[] = {
                    {
                        .key = "params",
                        .type = JSMN_OBJECT
                    },
                    {
                        .key = "pdid",
                        .type = JSMN_PRIMITIVE
                    }
                };
                if(MIIO_OK != jsmi_get_value_u16(&jsmi_parser, NULL, pid_path, NELEMENTS(pid_path), &(device_list.device[0].product_id))) {
                    LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_add_linkage: cannot parse pid");
                    ret = MIIO_ERROR_NOTFOUND;
                    goto end;
                }
            
                ret = mible_linkage_get_specified_device(&device_list.device[0]);
                if (MIIO_OK != ret) {
                    goto end;
                }
                device_list.cur_num++;
            }
            jsmi_parse_finish(&jsmi_parser);
        }else {
            ret = MIIO_ERROR_PARAM;
            goto end;
        }

        char js[1024] = {0};
        miio_rpc_delegate_arg_t ack_arg = {
            .id = req_arg->id,
            .type = req_arg->type,
            .pload_len = 0,
            .pload = js
        };
        jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, sizeof(js));
        jsmi_compose_start(&jsmi_composer);
        jsmi_set_object_begin(&jsmi_composer);
            jsmi_set_key_value_uint(&jsmi_composer, "id", ack_arg.id);
            jsmi_set_key_object_begin(&jsmi_composer, "result");
                jsmi_set_key_array_begin(&jsmi_composer, "device");
                for(int index = 0; index < device_list.cur_num; index++) {
                    snprintf_safe(mac_string, sizeof(mac_string), "%02X:%02X:%02X:%02X:%02X:%02X",
                                device_list.device[index].address[5], device_list.device[index].address[4],
                                device_list.device[index].address[3], device_list.device[index].address[2],
                                device_list.device[index].address[1], device_list.device[index].address[0]);
                    jsmi_set_value_object_begin(&jsmi_composer);
                        jsmi_set_key_value_sint(&jsmi_composer, "fun_type", device_list.device[index].function_type);
                        jsmi_set_key_value_sint(&jsmi_composer, "enabled", device_list.device[index].enabled);
                        jsmi_set_key_value_str(&jsmi_composer, "mac", (char *)mac_string, strlen(mac_string));
                        jsmi_set_key_value_sint(&jsmi_composer, "pdid", device_list.device[index].product_id);
                        jsmi_set_key_array_begin(&jsmi_composer, "rule");
                        for (int i = 0; i < MIBLE_LINKAGE_OBJECT_MAX_NUM; i++) {
                            if (device_list.device[index].object[i].object_id == 0xFFFF) {
                                continue;
                            }
                            jsmi_set_value_object_begin(&jsmi_composer);
                                jsmi_set_key_value_sint(&jsmi_composer, "object_id", device_list.device[index].object[i].object_id);
                            jsmi_set_value_object_end(&jsmi_composer);
                        }
                        jsmi_set_key_array_end(&jsmi_composer);
                    jsmi_set_value_object_end(&jsmi_composer);
                }
                jsmi_set_key_array_end(&jsmi_composer);
            jsmi_set_key_object_end(&jsmi_composer);
        jsmi_set_object_end(&jsmi_composer);
        ret = jsmi_compose_finish(&jsmi_composer, NULL, &ack_arg.pload_len);
        if(MIIO_OK == ret){
            ret = ack(&ack_arg, ctx);
        }
    }

end:
    if (ret == MIIO_ERROR_FULL) {
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_FULL, ack, ctx);
    } else if (ret == MIIO_ERROR_NOTFOUND){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_NOTFOUND, ack, ctx);
    }else if (ret == MIIO_ERROR_SIZE){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_SIZE, ack, ctx);
    } else if (ret == MIIO_ERROR_NOMEM){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_NOMEM, ack, ctx);
    } else if (MIIO_OK != ret){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_PARAM, ack, ctx);
    }

    return ret;
}
MIIO_RPC(ble_get_linkage, do_ble_get_linkage, "Hello, MIoT");

//{"id":123459,"method":"miIO.ble_enable_linkage","params":{"op":0,"enabled":0}
//{"id":123459,"method":"miIO.ble_enable_linkage","params":{"op":1,"mac":"50:EC:50:CB:01:91","enabled":0}}
//{"id":123459,"result":{"op":0,"code":0}
//{"id":123459,"result":{"op":1,"mac":"50:EC:50:CB:01:91","code":0}
//{"error":{"code":-x,"message":""}}
static int do_ble_enable_linkage(miio_rpc_delegate_arg_t *req_arg,
                            miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
    int ret = MIIO_ERROR_PARAM;
    int op = 0;
    char mac_string[18] = {0};
    linkage_device_t *p_dev = NULL;

    if (NULL == req_arg || NULL == ack) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "do_ble_enable_linkage invalid param");
        return MIIO_ERROR_PARAM;
    }

    if (MIIO_DELEGATE_JSON == req_arg->type) {
        jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(req_arg->pload, req_arg->pload_len);
        if(MIIO_OK == jsmi_parse_start(&jsmi_parser)) {
            jsmi_tok_path_t op_path[] = {
                {
                    .key = "params",
                    .type = JSMN_OBJECT
                },
                {
                    .key = "op",
                    .type = JSMN_PRIMITIVE
                }
            };
            if(MIIO_OK != jsmi_get_value_sint(&jsmi_parser, NULL, op_path, NELEMENTS(op_path), &op)) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_enable_linkage: cannot parse op");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }

            jsmi_tok_path_t enabled_path[] = {
                {
                    .key = "params",
                    .type = JSMN_OBJECT
                },
                {
                    .key = "enabled",
                    .type = JSMN_PRIMITIVE
                }
            };
            uint16_t state = 0;
            if(MIIO_OK != jsmi_get_value_u16(&jsmi_parser, NULL, enabled_path, NELEMENTS(enabled_path), &state)) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_enable_linkage: cannot parse enabled");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }

            if (op == 1) {
                p_dev = (linkage_device_t *)malloc(sizeof(linkage_device_t));
                if (NULL == p_dev) {
                    LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for linkage device");
                    ret = MIIO_ERROR_NOMEM;
                    goto end;
                }
                memset((uint8_t *)p_dev, 0xFF, sizeof(linkage_device_t));

                jsmi_tok_path_t mac_path[] = {
                    {
                        .key = "params",
                        .type = JSMN_OBJECT
                    },
                    {
                        .key = "mac",
                        .type = JSMN_STRING
                    }
                };
                if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, NULL, mac_path, NELEMENTS(mac_path), mac_string, sizeof(mac_string))) {
                    LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_enable_linkage: cannot parse mac");
                    ret = MIIO_ERROR_NOTFOUND;
                    goto end;
                }
                if (MIIO_OK != mac_convert(mac_string, (uint8_t *)p_dev->address)) {
                    ret = MIIO_ERROR_NOTFOUND;
                    goto end;
                }

                jsmi_tok_path_t pid_path[] = {
                    {
                        .key = "params",
                        .type = JSMN_OBJECT
                    },
                    {
                        .key = "pdid",
                        .type = JSMN_PRIMITIVE
                    }
                };
                if(MIIO_OK != jsmi_get_value_u16(&jsmi_parser, NULL, pid_path, NELEMENTS(pid_path), &p_dev->product_id)) {
                    LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_add_linkage: cannot parse pid");
                    ret = MIIO_ERROR_NOTFOUND;
                    goto end;
                }
            
                p_dev->enabled = (linkage_function_state_t)state;
                ret = mible_linkage_enable(p_dev);
                if (ret != MIIO_OK) {
                    goto end;
                }
            }else {
                ret = mible_linkage_enable_all((linkage_function_state_t)state);
                if (ret != MIIO_OK) {
                    goto end;
                }
            }

            jsmi_parse_finish(&jsmi_parser);
        }else {
            ret = MIIO_ERROR_PARAM;
            goto end;
        }

        char js[1024] = {0};
        miio_rpc_delegate_arg_t ack_arg = {
            .id = req_arg->id,
            .type = req_arg->type,
            .pload_len = 0,
            .pload = js
        };
        jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, sizeof(js));
        jsmi_compose_start(&jsmi_composer);
        jsmi_set_object_begin(&jsmi_composer);
            jsmi_set_key_value_uint(&jsmi_composer, "id", ack_arg.id);
            jsmi_set_key_object_begin(&jsmi_composer, "result");
                jsmi_set_key_value_sint(&jsmi_composer, "op", op);
                if (op == 1) {
                    memset(mac_string, 0, sizeof(mac_string));
                    snprintf_safe(mac_string, sizeof(mac_string), "%02X:%02X:%02X:%02X:%02X:%02X",
                                p_dev->address[5], p_dev->address[4],p_dev->address[3],
                                p_dev->address[2],p_dev->address[1], p_dev->address[0]);
                    jsmi_set_key_value_str(&jsmi_composer, "mac", (char *)mac_string, strlen(mac_string));
                }
                jsmi_set_key_value_sint(&jsmi_composer, "code", ret);
            jsmi_set_key_object_end(&jsmi_composer);
        jsmi_set_object_end(&jsmi_composer);
        ret = jsmi_compose_finish(&jsmi_composer, NULL, &ack_arg.pload_len);
        if(MIIO_OK == ret){
            ret = ack(&ack_arg, ctx);
        }
    }

end:
    if (ret == MIIO_ERROR_FULL) {
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_FULL, ack, ctx);
    } else if (ret == MIIO_ERROR_NOTFOUND){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_NOTFOUND, ack, ctx);
    }else if (ret == MIIO_ERROR_SIZE){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_SIZE, ack, ctx);
    } else if (ret == MIIO_ERROR_NOMEM){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_NOMEM, ack, ctx);
    } else if (MIIO_OK != ret){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_PARAM, ack, ctx);
    }

    if (NULL != p_dev) {
        free(p_dev);
        p_dev = NULL;
    }

    return ret;
}
MIIO_RPC(ble_enable_linkage, do_ble_enable_linkage, "Hello, MIoT");


static int do_ble_dev_bind_notify(miio_rpc_delegate_arg_t *req_arg,
                            miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
    int ret = MIIO_OK;
    char js[256] = {0};
    int rsp = -1;

    if (NULL == req_arg || NULL == ack) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "do_ble_dev_bind_notify invalid param");
        return MIIO_ERROR_PARAM;
    }

    if (MIIO_DELEGATE_JSON == req_arg->type) {
        jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(req_arg->pload, req_arg->pload_len);
        if(MIIO_OK == jsmi_parse_start(&jsmi_parser)) {

            char mac_string[18] = {0};
            uint8_t addr[6] = {0xFF};
            uint16_t pdid = 0;

            jsmi_tok_path_t mac_path[] = {
                {
                    .key = "params",
                    .type = JSMN_OBJECT
                },
                {
                    .key = "mac",
                    .type = JSMN_STRING
                }
            };
            if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, NULL, mac_path, NELEMENTS(mac_path), mac_string, sizeof(mac_string))) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_dev_bind_notify: cannot parse mac");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }
            if (MIIO_OK != mac_convert(mac_string, addr)) {
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }

            jsmi_tok_path_t pid_path[] = {
                {
                    .key = "params",
                    .type = JSMN_OBJECT
                },
                {
                    .key = "pdid",
                    .type = JSMN_PRIMITIVE
                }
            };
            if(MIIO_OK != jsmi_get_value_u16(&jsmi_parser, NULL, pid_path, NELEMENTS(pid_path), &pdid)) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_dev_bind_notify: cannot parse pid");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }

            jsmi_parse_finish(&jsmi_parser);

            //to do bind action
            //LOG_INFO_TAG(MIBLE_LOG_TAG, "pid : %d\n", pdid);
            //arch_dump_hex(addr, 6);
            rsp = mible_dev_bind_action(addr, pdid);
        }
    }


    miio_rpc_delegate_arg_t ack_arg = {
        .id = req_arg->id,
        .type = req_arg->type,
        .pload_len = 0,
        .pload = js
    };

    jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, sizeof(js));
    jsmi_compose_start(&jsmi_composer);
    jsmi_set_object_begin(&jsmi_composer);
        jsmi_set_key_value_uint(&jsmi_composer, "id", ack_arg.id);
        jsmi_set_key_object_begin(&jsmi_composer, "result");
            jsmi_set_key_value_sint(&jsmi_composer, "notified", rsp);
        jsmi_set_key_object_end(&jsmi_composer);
    jsmi_set_object_end(&jsmi_composer);

    if(MIIO_OK == jsmi_compose_finish(&jsmi_composer, NULL, &ack_arg.pload_len)){
            return ack(&ack_arg, ctx);
    }

end:
    if (ret == MIIO_ERROR_FULL) {
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_FULL, ack, ctx);
    } else if (ret == MIIO_ERROR_NOTFOUND){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_NOTFOUND, ack, ctx);
    }else if (ret == MIIO_ERROR_SIZE){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_SIZE, ack, ctx);
    } else if (ret == MIIO_ERROR_NOMEM){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_NOMEM, ack, ctx);
    } else if (MIIO_OK != ret){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_PARAM, ack, ctx);
    }

    return ret;
}
MIIO_RPC(ble_bind_notified, do_ble_dev_bind_notify, "Hello, MIoT");

static int do_ble_dev_unbind_notify(miio_rpc_delegate_arg_t *req_arg,
                            miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
    int ret = MIIO_OK;
    char js[256] = {0};
    int rsp = -1;

    if (NULL == req_arg || NULL == ack) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "do_ble_dev_unbind_notify invalid param");
        return MIIO_ERROR_PARAM;
    }

    if (MIIO_DELEGATE_JSON == req_arg->type) {
        jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(req_arg->pload, req_arg->pload_len);
        if(MIIO_OK == jsmi_parse_start(&jsmi_parser)) {

            char mac_string[18] = {0};
            uint8_t addr[6] = {0xFF};
            uint16_t pdid = 0;

            jsmi_tok_path_t mac_path[] = {
                {
                    .key = "params",
                    .type = JSMN_OBJECT
                },
                {
                    .key = "mac",
                    .type = JSMN_STRING
                }
            };
            if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, NULL, mac_path, NELEMENTS(mac_path), mac_string, sizeof(mac_string))) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_dev_unbind_notify: cannot parse mac");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }
            if (MIIO_OK != mac_convert(mac_string, addr)) {
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }

            jsmi_tok_path_t pid_path[] = {
                {
                    .key = "params",
                    .type = JSMN_OBJECT
                },
                {
                    .key = "pdid",
                    .type = JSMN_PRIMITIVE
                }
            };
            if(MIIO_OK != jsmi_get_value_u16(&jsmi_parser, NULL, pid_path, NELEMENTS(pid_path), &pdid)) {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "do_ble_dev_unbind_notify: cannot parse pid");
                ret = MIIO_ERROR_NOTFOUND;
                goto end;
            }

            jsmi_parse_finish(&jsmi_parser);

            //to do unbind action
            //LOG_INFO_TAG(MIBLE_LOG_TAG, "pid : %d\n", pdid);
            //arch_dump_hex(addr, 6);
            rsp = mible_dev_unbind_action(addr, pdid);
        }
    }


    miio_rpc_delegate_arg_t ack_arg = {
        .id = req_arg->id,
        .type = req_arg->type,
        .pload_len = 0,
        .pload = js
    };

    jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, sizeof(js));
    jsmi_compose_start(&jsmi_composer);
    jsmi_set_object_begin(&jsmi_composer);
        jsmi_set_key_value_uint(&jsmi_composer, "id", ack_arg.id);
        jsmi_set_key_object_begin(&jsmi_composer, "result");
            jsmi_set_key_value_sint(&jsmi_composer, "notified", rsp);
        jsmi_set_key_object_end(&jsmi_composer);
    jsmi_set_object_end(&jsmi_composer);

    if(MIIO_OK == jsmi_compose_finish(&jsmi_composer, NULL, &ack_arg.pload_len)){
            return ack(&ack_arg, ctx);
    }

end:
    if (ret == MIIO_ERROR_FULL) {
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_FULL, ack, ctx);
    } else if (ret == MIIO_ERROR_NOTFOUND){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_NOTFOUND, ack, ctx);
    }else if (ret == MIIO_ERROR_SIZE){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_SIZE, ack, ctx);
    } else if (ret == MIIO_ERROR_NOMEM){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_NOMEM, ack, ctx);
    } else if (MIIO_OK != ret){
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID,
            MIBLE_RPC_ERR_INFO_PARAM, ack, ctx);
    }

    return ret;

}
MIIO_RPC(ble_unbind_notified, do_ble_dev_unbind_notify, "Hello, MIoT");


static int do_bt_gateway_enable(miio_rpc_delegate_arg_t *req_arg,
                            miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
    int ret = MIIO_ERROR_PARAM;
    char js[256] = {0};
    mible_param_t parameter;

    if (NULL == req_arg || NULL == ack) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "do_bt_gateway_enable invalid param");
        return ret;
    }

    miio_rpc_delegate_arg_t ack_arg = {
        .id = req_arg->id,
        .type = req_arg->type,
        .pload_len = 0,
        .pload = js
    };

    memset(&parameter, 0, sizeof(parameter));
    parameter.user_state = MIBLE_GATEWAY_STATE_ENABLE;
    gateway_state.user_state = parameter.user_state;
    arch_psm_set_value("ble", "param", &parameter, sizeof(parameter));

    jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, sizeof(js));
    jsmi_compose_start(&jsmi_composer);
    jsmi_set_object_begin(&jsmi_composer);
        jsmi_set_key_value_uint(&jsmi_composer, "code", 0);
        jsmi_set_key_value_uint(&jsmi_composer, "id", ack_arg.id);
        jsmi_set_key_value_str(&jsmi_composer, "message", "ok", strlen("ok"));
        jsmi_set_key_object_begin(&jsmi_composer, "result");
        jsmi_set_key_object_end(&jsmi_composer);
    jsmi_set_object_end(&jsmi_composer);

    if(MIIO_OK == jsmi_compose_finish(&jsmi_composer, NULL, &ack_arg.pload_len)){
        ret = ack(&ack_arg, ctx);
        LOG_INFO_TAG(MIBLE_LOG_TAG, "do_bt_gateway_enable ack %s",(ret == MIIO_OK)?"succ":"fail");
        if(MIIO_OK != ret)
            return ret;
    }
    ret = mible_rpc_mesh_switch(parameter.user_state);
    return ret;
}
MIIO_RPC_USER(bt_gateway_enable, do_bt_gateway_enable, "Hello, MIoT");

static int do_bt_gateway_disable(miio_rpc_delegate_arg_t *req_arg,
                            miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
    int ret = MIIO_ERROR_PARAM;
    char js[256] = {0};
    mible_param_t parameter;

    if (NULL == req_arg || NULL == ack) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "do_bt_gateway_disable invalid param");
        return ret;
    }

    miio_rpc_delegate_arg_t ack_arg = {
        .id = req_arg->id,
        .type = req_arg->type,
        .pload_len = 0,
        .pload = js
    };

    memset(&parameter, 0, sizeof(parameter));
    parameter.user_state = MIBLE_GATEWAY_STATE_DISABLE;
    gateway_state.user_state = parameter.user_state;
    arch_psm_set_value("ble", "param", &parameter, sizeof(parameter));

    jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, sizeof(js));
    jsmi_compose_start(&jsmi_composer);
    jsmi_set_object_begin(&jsmi_composer);
        jsmi_set_key_value_uint(&jsmi_composer, "code", 0);
        jsmi_set_key_value_uint(&jsmi_composer, "id", ack_arg.id);
        jsmi_set_key_value_str(&jsmi_composer, "message", "ok", strlen("ok"));
        jsmi_set_key_object_begin(&jsmi_composer, "result");
        jsmi_set_key_object_end(&jsmi_composer);
    jsmi_set_object_end(&jsmi_composer);

    if(MIIO_OK == jsmi_compose_finish(&jsmi_composer, NULL, &ack_arg.pload_len)){
        ret = ack(&ack_arg, ctx);
        LOG_INFO_TAG(MIBLE_LOG_TAG, "do_bt_gateway_disable ack %s",(ret == MIIO_OK)?"succ":"fail");
        if(MIIO_OK != ret)
            return ret;
    }
    ret = mible_rpc_mesh_switch(parameter.user_state);
    return ret;
}
MIIO_RPC_USER(bt_gateway_disable, do_bt_gateway_disable, "Hello, MIoT");

static int do_bt_gateway_status(miio_rpc_delegate_arg_t *req_arg,
                            miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
    int ret = MIIO_ERROR_PARAM;
    char js[256] = {0};

    if (NULL == req_arg || NULL == ack) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "do_ble_gateway_state invalid param");
        return ret;
    }

    miio_rpc_delegate_arg_t ack_arg = {
        .id = req_arg->id,
        .type = req_arg->type,
        .pload_len = 0,
        .pload = js
    };
    uint8_t state = (MIBLE_GATEWAY_STATE_UNKNOWN == gateway_state.user_state)?
            gateway_state.cur_state : gateway_state.user_state;
    
    jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, sizeof(js));
    jsmi_compose_start(&jsmi_composer);
    jsmi_set_object_begin(&jsmi_composer);
        jsmi_set_key_value_uint(&jsmi_composer, "code", 0);
        jsmi_set_key_value_uint(&jsmi_composer, "id", ack_arg.id);
        jsmi_set_key_value_str(&jsmi_composer, "message", "ok", strlen("ok"));
        jsmi_set_key_object_begin(&jsmi_composer, "result");
            jsmi_set_key_value_str(&jsmi_composer, "gateway_status", state? "enable":"disable", state? strlen("enable"):strlen("disable"));
        jsmi_set_key_object_end(&jsmi_composer);
    jsmi_set_object_end(&jsmi_composer);

    if(MIIO_OK == jsmi_compose_finish(&jsmi_composer, NULL, &ack_arg.pload_len)){
            return ack(&ack_arg, ctx);
    }

    return ret;
}
MIIO_RPC_USER(bt_gateway_status, do_bt_gateway_status, "Hello, MIoT");

static int do_get_gateway_version(miio_rpc_delegate_arg_t *req_arg,
                            miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
    int ret = MIIO_OK;
    char js[256] = {0};

    if (NULL == req_arg || NULL == ack) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "do_get_gateway_version invalid param");
        return MIIO_ERROR_PARAM;
    }

    miio_rpc_delegate_arg_t ack_arg = {
        .id = req_arg->id,
        .type = req_arg->type,
        .pload_len = 0,
        .pload = js
    };
    
    jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, sizeof(js));
    jsmi_compose_start(&jsmi_composer);
    jsmi_set_object_begin(&jsmi_composer);
        jsmi_set_key_value_uint(&jsmi_composer, "id", ack_arg.id);
        jsmi_set_key_array_begin(&jsmi_composer, "result");
            jsmi_set_value_str(&jsmi_composer, MIBLE_LIB_VERSION,sizeof(MIBLE_LIB_VERSION)-1);
        jsmi_set_key_array_end(&jsmi_composer);
    jsmi_set_object_end(&jsmi_composer);

    if(MIIO_OK == jsmi_compose_finish(&jsmi_composer, NULL, &ack_arg.pload_len)){
            return ack(&ack_arg, ctx);
    }
    
    return ret;
}
MIIO_RPC_USER(get_gateway_version, do_get_gateway_version, "Hello, MIoT");



