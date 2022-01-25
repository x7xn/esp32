/* Includes ------------------------------------------------------------------*/

#include "mible_def.h"
#include "mible_cmd.h"
#include "mible_prph.h"
#include "mible_rpc.h"
#include "mible_gateway.h"
#include "mible_fastpair.h"
#include "mible_hooks.h"
#include "miio_command.h"
#include "list/list.h"
#include "mible_linkage.h"

/* Private define ------------------------------------------------------------*/

#undef  MIBLE_LOG_TAG
#define MIBLE_LOG_TAG                            "mible_cmd"
#undef  LOG_LEVEL
#define LOG_LEVEL                                LOG_LEVEL_INFO

#define MIBLE_COMMAND_MAX                        10

/* Private typedef -----------------------------------------------------------*/

typedef struct {
    list_head_t node;
    char *buffer;
} mible_cmd_t;

/* Private functions ---------------------------------------------------------*/

static void do_ble_event(mcmd_t *mcmd, char *params);
static void do_ble_config(mcmd_t *mcmd, char *params);
static void do_ble_fastpair(mcmd_t *mcmd, char *params);
static void do_linkage_rule_report(mcmd_t *mcmd, char *params);
static void cmd_get_object(mible_addr_t addr, mibeacon_object_t *p_object);
static void cmd_fastpair_report(mible_addr_t addr, fastpair_state_t state);
static void cmd_fastpair_check(mible_addr_t addr, uint16_t product_id,
                                         uint16_t object_id, int8_t rssi);
static void cmd_linkage_rule_report(void);
static void cmd_linkage_event_report(mible_addr_t addr, uint16_t pid, mibeacon_object_t *p_object);



/* Private variables ---------------------------------------------------------*/

static list_head_t command_hdr;
static arch_os_mutex_handle_t command_mutex;
static uint16_t command_num;
static mible_hooks_t command_hooks;

/* Exported functions --------------------------------------------------------*/

int mible_cmd_init(void)
{
    int ret;

    /* Avoid compiler optimizing */
    do_ble_event(NULL, NULL);
    do_ble_config(NULL, NULL);
    do_ble_fastpair(NULL, NULL);
    do_linkage_rule_report(NULL, NULL);

    INIT_LIST_HEAD(&command_hdr);
    command_num = 0;

    memset(&command_hooks, 0, sizeof(mible_hooks_t));
    command_hooks.get_object = cmd_get_object;
    command_hooks.fastpair_check = cmd_fastpair_check;
    command_hooks.fastpair_report = cmd_fastpair_report;
    command_hooks.linkage_rule_report = cmd_linkage_rule_report;
    command_hooks.linkage_event_report = cmd_linkage_event_report;
    ret = mible_hooks_register(&command_hooks);
    if (MIIO_OK != ret) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "fail to register hooks (err %d)", ret);
        return ret;
    }

    ret = arch_os_mutex_create(&command_mutex);
    if (MIIO_OK != ret) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "fail to create mutex (err %d)", ret);
        return ret;
    }

    return MIIO_OK;
}

int mible_cmd_put(char *buffer)
{
    mible_cmd_t *command;

    command = (mible_cmd_t *)malloc(sizeof(mible_cmd_t));
    if (NULL == command) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for mbile command");
        return MIIO_ERROR_NOMEM;
    }
    command->buffer = buffer;

    arch_os_mutex_get(command_mutex, ARCH_OS_WAIT_FOREVER);

    list_add_tail(&command->node, &command_hdr);
    ++command_num;

    while (command_num > MIBLE_COMMAND_MAX) {
        command = list_first_entry(&command_hdr, mible_cmd_t, node);
        list_del(&command->node);
        free(command->buffer);
        free(command);
        --command_num;
    }

    arch_os_mutex_put(command_mutex);

    return MIIO_OK;
}

int mible_cmd_get(char *buf, uint32_t buf_size)
{
    int ret;

    arch_os_mutex_get(command_mutex, ARCH_OS_WAIT_FOREVER);

    if (list_empty(&command_hdr)) {
        ret = MIIO_ERROR_NOTFOUND;
    } else {
        mible_cmd_t *command = list_first_entry(&command_hdr, mible_cmd_t, node);
        list_del(&command->node);
        snprintf_safe(buf, buf_size, "down %s", command->buffer);
        free(command->buffer);
        free(command);
        --command_num;
        ret = MIIO_OK;
    }

    arch_os_mutex_put(command_mutex);

    return ret;
}

static void copy_str_to_byte(uint8_t *des, const char *str, uint32_t len)
{
    int i, value = 0;
    int using_long_format = 0;
    int using_hex_sign = 0;

    if (str[2] == ':' || str[2] == '-') {
        using_long_format = 1;
    }

    if (str[1] == 'x') {
        using_hex_sign = 2;
    }

    for (i = 0; i < len; i++) {
        sscanf(str + using_hex_sign + i * (2 + using_long_format), "%02x", &value);
        des[len - 1 - i] = (uint8_t) value;
        value = 0;
    }
}

static void do_ble_event(mcmd_t *mcmd, char *params)
{
    int  argc;
	char *argv[10];

    if (NULL == mcmd || NULL == params) {
        return;
    }

    argc = mcmd_parse_params(params, argv, 10, NULL);

    if (1 == argc && 0 == strcmp(argv[0], "reset")) {
        mible_prph_reset();
    } else if (1 == argc && 0 == strcmp(argv[0], "list")) {
        char *dump_buf = (char *) malloc(1000);
        if (dump_buf) {
            if (mible_prph_dump(dump_buf, 1000) > 0) {
                mcmd->io.out(mcmd->io.handle, dump_buf);
            }
            free(dump_buf);
            return;
        } else {
            goto err_exit;
        }
#if (defined(MIBLE_LEGACY_ENABLE) && MIBLE_LEGACY_ENABLE)
    } else if (1 == argc && 0 == strcmp(argv[0], "dump")) {
        uint32_t mible_legacy_event_dump(char *buf, uint32_t buf_size);
        char *dump_buf = (char *) malloc(1000);
        if (dump_buf) {
            if (mible_legacy_event_dump(dump_buf, 1000) > 0) {
                mcmd->io.out(mcmd->io.handle, dump_buf);
            }
            free(dump_buf);
            return;
        } else {
            goto err_exit;
        }
#endif
    } else if (3 == argc && 0 == strcmp(argv[0], "delete")) {
        mible_addr_t addr;
        int object_id;
        if (17 != strlen(argv[1])) {
            goto err_exit;
        }
        copy_str_to_byte(addr, argv[1], 6);
        sscanf(argv[2], "%d", &object_id);
        mible_prph_delete(addr, (uint16_t)object_id);
    } else if (5 == argc && 0 == strcmp(argv[0], "add")) {
        mible_addr_t addr;
        uint8_t key[MISERVICE_CHAR_BEACONKEY_LEN];
        int object_id, product_id;
        if (17 != strlen(argv[1]) ||
            (MISERVICE_CHAR_BEACONKEY_LEN * 2) != strlen(argv[4])) {

            goto err_exit;
        }
        copy_str_to_byte(addr, argv[1], 6);
        sscanf(argv[2], "%d", &object_id);
        sscanf(argv[3], "%d", &product_id);
        arch_str2hex(key, argv[4], MISERVICE_CHAR_BEACONKEY_LEN);
        if (MIIO_ERROR_FULL == mible_prph_add(addr, product_id, object_id, key)) {
            goto err_exit;
        }
    } else {
        goto err_exit;
    }

    mcmd->io.out(mcmd->io.handle, "ok");
    return;

err_exit:

    mcmd->io.out(mcmd->io.handle, "error");
}
MIIO_CMD(ble_event, do_ble_event, NULL);

static void do_ble_config(mcmd_t *mcmd, char *params)
{
    int  argc;
	char *argv[10];

    if (NULL == mcmd || NULL == params) {
        return;
    }

    argc = mcmd_parse_params(params, argv, 10, NULL);

    if (1 == argc && 0 == strcmp(argv[0], "dump")) {
        char *dump_buf = (char *) malloc(100);
        if (dump_buf) {
            if (mible_gateway_dump(dump_buf, 100) > 0) {
                mcmd->io.out(mcmd->io.handle, dump_buf);
            }
            free(dump_buf);
            return;
        } else {
            goto err_exit;
        }
    } else if (argc >= 2 && argc <= 3 && 0 == strcmp(argv[0], "set")) {
        int product_id, version;
        mible_config_t config = gateway_config;
        /* Parse the product ID */
        sscanf(argv[1], "%d", &product_id);
        if (product_id < 0 || product_id > UINT16_MAX) {
            goto err_exit;
        }
        config.product_id = product_id;
        /* Parse the firmware version */
        if (argc >= 3) {
            if (4 != strlen(argv[2]) || !isdigit((int)argv[2][0]) ||
                            !isdigit((int)argv[2][1]) || !isdigit((int)argv[2][2]) ||
                            !isdigit((int)argv[2][3])) {

                goto err_exit;
            }
            sscanf(argv[2], "%04d", &version);
            if (version >= 10000 || version < 0) {    /* the version is 0000 ~ 9999 */
                goto err_exit;
            }
            config.version = version;
        }
        /* update mijia gateway config */
        if (MIIO_OK != mible_gateway_update(&config)) {
            goto err_exit;
        }
    } else {
        goto err_exit;
    }

    mcmd->io.out(mcmd->io.handle, "ok");
    return;

err_exit:

    mcmd->io.out(mcmd->io.handle, "error");
}
MIIO_CMD(ble_config, do_ble_config, NULL);

static void do_ble_fastpair(mcmd_t *mcmd, char *params)
{
    int  argc;
	char *argv[10];
    mible_addr_t addr;
    int object_id, product_id, permit;

    if (NULL == mcmd || NULL == params) {
        return;
    }

    argc = mcmd_parse_params(params, argv, 10, NULL);

    if (5 != argc || 0 != strcmp(argv[0], "set") || 17 != strlen(argv[1])) {
        goto err_exit;
    }

    copy_str_to_byte(addr, argv[1], 6);
    sscanf(argv[2], "%d", &object_id);
    sscanf(argv[3], "%d", &product_id);
    sscanf(argv[4], "%d", &permit);
    mible_fastpair_update(addr, product_id, object_id, permit > 0);

    mcmd->io.out(mcmd->io.handle, "ok");
    return;

err_exit:

    mcmd->io.out(mcmd->io.handle, "error");
}
MIIO_CMD(ble_fastpair, do_ble_fastpair, NULL);

static void do_linkage_rule_report(mcmd_t *mcmd, char *params)
{
    int  argc;
    char *argv[10];

    if (NULL == mcmd || NULL == params) {
        return;
    }

    argc = mcmd_parse_params(params, argv, 10, NULL);

    if (1 == argc && 0 == strcmp(argv[0], "get")) {
        char *dump_buf = (char *) malloc(1000);
        if (dump_buf) {
            if (mible_linkage_rule_dump(dump_buf, 1000) > 0) {
                mcmd->io.out(mcmd->io.handle, dump_buf);
            }
            free(dump_buf);
            return;
        } else {
            goto err_exit;
        }
    } else {
        goto err_exit;
    }
    return;

err_exit:

    mcmd->io.out(mcmd->io.handle, "error");
}
MIIO_CMD(ble_linkage, do_linkage_rule_report, NULL);

static void cmd_get_object(mible_addr_t addr, mibeacon_object_t *p_object)
{
    uint16_t buf_len = 1000, n = 0;
    char *buffer;

    buffer = (char *)malloc(buf_len);
    if (NULL == buffer) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for ble object");
        return;
    }

    n += snprintf_safe(buffer + n, buf_len - n,
                       "ble_event %02x:%02x:%02x:%02x:%02x:%02x",
                       addr[5], addr[4], addr[3],
                       addr[2], addr[1], addr[0]);
    n += snprintf_safe(buffer + n, buf_len - n, " %d,", p_object->id);
    n += snprintf_hex(buffer + n, buf_len - n, p_object->data, p_object->data_len, 0);

    if (MIIO_OK != mible_cmd_put(buffer)) {
        free(buffer);
    }
}

static void cmd_fastpair_report(mible_addr_t addr, fastpair_state_t state)
{
    uint16_t buf_len = 100, n = 0;
    char *buffer;

    buffer = (char *)malloc(buf_len);
    if (NULL == buffer) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for fastpair state report");
        return;
    }

    n += snprintf_safe(buffer + n, buf_len - n,
                       "ble_fastpair_rsp %02x:%02x:%02x:%02x:%02x:%02x ",
                       addr[5], addr[4], addr[3],
                       addr[2], addr[1], addr[0]);

    switch (state) {
        case FASTPAIR_STATE_SUCC:
            n += snprintf_safe(buffer + n, buf_len - n, "succ");
            break;
        case FASTPAIR_STATE_TIMEOUT:
            n += snprintf_safe(buffer + n, buf_len - n, "timeout");
            break;
        case FASTPAIR_STATE_EXISTED:
            n += snprintf_safe(buffer + n, buf_len - n, "done");
            break;
        case FASTPAIR_STATE_FAIL:
        default:
            n += snprintf_safe(buffer + n, buf_len - n, "fail");
            break;
    }

    if (MIIO_OK != mible_cmd_put(buffer)) {
        free(buffer);
    }
}

static void cmd_fastpair_check(mible_addr_t addr, uint16_t product_id,
                                         uint16_t object_id, int8_t rssi)
{
    uint16_t buf_len = 100, n = 0;
    char *buffer;

    buffer = (char *)malloc(buf_len);
    if (NULL == buffer) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for fastpair check buffer");
        return;
    }

    n += snprintf_safe(buffer + n, buf_len - n, "ble_fastpair %02x:%02x:%02x:%02x:%02x:%02x",
                        addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
    n += snprintf_safe(buffer + n, buf_len - n, " %d", object_id);
    n += snprintf_safe(buffer + n, buf_len - n, " %d", product_id);
    n += snprintf_safe(buffer + n, buf_len - n, " %d", rssi);

    if (MIIO_OK != mible_cmd_put(buffer)) {
        free(buffer);
    }
}

static void cmd_linkage_rule_report(void)
{
    uint16_t buf_len = 1000;
    char *buffer;

    buffer = (char *)malloc(buf_len);
    if (NULL == buffer) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for linkage report buffer");
        return;
    }

    if (mible_linkage_rule_dump(buffer, 1000) > 0) {
        if (MIIO_OK != mible_cmd_put(buffer)) {
            free(buffer);
        }
    }
    else
    {
        free(buffer);
    }
}

static void cmd_linkage_event_report(mible_addr_t addr, uint16_t pid, mibeacon_object_t *p_object)
{
    uint16_t buf_len = 1000, n = 0;
    char *buffer;

    buffer = (char *)malloc(buf_len);
    if (NULL == buffer) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for ble object");
        return;
    }

    // MAC
    n += snprintf_safe(buffer + n, buf_len - n,
                       "ble_event %02x:%02x:%02x:%02x:%02x:%02x",
                       addr[5], addr[4], addr[3],
                       addr[2], addr[1], addr[0]);
    // PID
    n += snprintf_safe(buffer + n, buf_len - n, " %d,", pid);
    // is_miot-spec
    // now is default 0, not spec
    n += snprintf_safe(buffer + n, buf_len - n, " %d,", 0);
    // event num
    // now is default 1, only 1 object
    n += snprintf_safe(buffer + n, buf_len - n, " %d,", 1);
    // event id
    n += snprintf_safe(buffer + n, buf_len - n, " %d,", p_object->id);
    // event data
    n += snprintf_hex(buffer + n, buf_len - n, p_object->data, p_object->data_len, 0);

    if (MIIO_OK != mible_cmd_put(buffer)) {
        free(buffer);
    }
}

