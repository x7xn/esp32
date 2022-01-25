/* Includes ------------------------------------------------------------------*/

#include "mible_gap.h"
#include "arch_api.h"
#include "adv_beacon.h"
#include "mible_master.h"
#include "mible_slave.h"
#include "mible_fastpair.h"
#include "mible_net.h"

/* Private define ------------------------------------------------------------*/
#undef  MIBLE_LOG_TAG
#define MIBLE_LOG_TAG                                 "mible_gap"
#undef  LOG_LEVEL
#define LOG_LEVEL                                     LOG_LEVEL_INFO

#define MIBEACON_ITEM_SEC_ENABLE                      0x0008
#define MIBEACON_ITEM_MAC_INCLUDE                     0x0010
#define MIBEACON_ITEM_CAP_INCLUDE                     0x0020
#define MIBEACON_ITEM_EVT_INCLUDE                     0x0040

#define MIBEACON_CAP_CONNECTABLE                      0x01
#define MIBEACON_CAP_ENCRYPTABLE                      0x04
#define MIBEACON_CAP_TYPE_COMBO                       (0x03 << 3)

#define MIBEACON_VERSION                              (0x0005 << 12)
#define MIBEACON_AUTH_MODE                            (0x0002 << 10)

#define MIBEACON_CHECK_PERIOD                         1000
#define MIBLE_GAP_SCAN_DELAY                          6000

#define HI_UINT16(a)                                  (((a) >> 8) & 0xFF)
#define LO_UINT16(a)                                  ((a) & 0xFF)

/* Private variables ---------------------------------------------------------*/

static mible_connect_t central_info[MIBLE_CONNECTION_CENTRAL_NUM];
static mible_connect_t peripheral_info;
static bool adv_enabled;
static arch_os_timer_handle_t gap_timer_handle;
static arch_os_timer_handle_t scan_timer_handle;

/* Private function prototypes -----------------------------------------------*/

static void mible_scan_timeout(arch_os_timer_handle_t handle);
static void mible_beacon_timeout(arch_os_timer_handle_t handle);
static void mible_beacon_send(void * arg);
static void mible_beacon_proximity(void);
static void mible_beacon_legacy(void);

/* Exported functions --------------------------------------------------------*/

int mible_gap_init(void)
{
    int ret;

    ret = arch_os_timer_create(&scan_timer_handle,
                               "scan_delay",
                               MIBLE_GAP_SCAN_DELAY,
                               mible_scan_timeout,
                               NULL,
                               ARCH_OS_TIMER_ONE_SHOT,
                               ARCH_OS_TIMER_AUTO_ACTIVATE);
    if (MIIO_OK != ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "create proximity timer fail (err %d)", ret);
        return ret;
    } else {
        LOG_INFO_TAG(MIBLE_LOG_TAG, "start mible gap scan in %ums", MIBLE_GAP_SCAN_DELAY);
    }

    ret = arch_os_timer_create(&gap_timer_handle, "proximity",
                               MIBEACON_CHECK_PERIOD,
                               mible_beacon_timeout, NULL,
                               ARCH_OS_TIMER_PERIODIC,
                               ARCH_OS_TIMER_AUTO_ACTIVATE);
    if (MIIO_OK != ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "create proximity timer fail (err %d)", ret);
        return ret;
    }

    adv_enabled = false;
    peripheral_info.conn_handle = MIBLE_HANDLE_INVALID;
    for (uint8_t index = 0; index < MIBLE_CONNECTION_CENTRAL_NUM; ++index) {
        central_info[index].conn_handle = MIBLE_HANDLE_INVALID;
    }

    return MIIO_OK;
}

int mible_gap_deinit(void)
{
    int ret;
    ret = mible_gap_scan(false);
    if (MIIO_OK != ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "Fail to stop scan.");
        return ret;
    }

    if(NULL != gap_timer_handle)
    {
        ret = arch_os_timer_delete(gap_timer_handle);
        if (MIIO_OK != ret) {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "delete proximity timer fail (err %d)", ret);
            return ret;
        }
    }

    adv_enabled = false;
    peripheral_info.conn_handle = MIBLE_HANDLE_INVALID;
    for (uint8_t index = 0; index < MIBLE_CONNECTION_CENTRAL_NUM; ++index) {
        central_info[index].conn_handle = MIBLE_HANDLE_INVALID;
    }

    return MIIO_OK;
}

int mible_connect_handler(mible_connect_t *p_connect)
{
    uint8_t index;

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "%s -- connect with %02X:%02X:%02X:%02X:%02X:%02X (handle %04x)",
        __func__, p_connect->address[5], p_connect->address[4],
        p_connect->address[3], p_connect->address[2], p_connect->address[1],
        p_connect->address[0], p_connect->conn_handle);

    if (MIBLE_LOCAL_ROLE_SLAVE == p_connect->local_role) {
        adv_enabled = false;
        if (MIBLE_HANDLE_INVALID == peripheral_info.conn_handle) {
            peripheral_info = *p_connect;
            return mible_slave_connect(peripheral_info.conn_handle);
        } else {
            LOG_WARN_TAG(MIBLE_LOG_TAG, "The slave connection %04x cannot be added", p_connect->conn_handle);
            return MIIO_ERROR_BUSY;
        }
    }

    if (MIBLE_LOCAL_ROLE_MASTER != p_connect->local_role) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Don't know the local role type");
        return MIIO_ERROR_PARAM;
    }

    for (index = 0; index < MIBLE_CONNECTION_CENTRAL_NUM; ++index) {
        if (central_info[index].conn_handle == p_connect->conn_handle) {
            LOG_WARN_TAG(MIBLE_LOG_TAG, "The master connection %04x has existed", p_connect->conn_handle);
            return MIIO_ERROR;
        }
    }

    for (index = 0; index < MIBLE_CONNECTION_CENTRAL_NUM; ++index) {
        if (MIBLE_HANDLE_INVALID == central_info[index].conn_handle) {
            central_info[index] = *p_connect;
            return mible_master_connect(p_connect->conn_handle, mible_fastpair_get_pid());
        }
    }

    LOG_WARN_TAG(MIBLE_LOG_TAG, "The connection table is full");
    return MIIO_ERROR_FULL;
}

int mible_disconnect_handler(mible_disconnect_t *p_disconnect)
{
    uint8_t index;

    if (p_disconnect->conn_handle == peripheral_info.conn_handle) {
        peripheral_info.conn_handle = MIBLE_HANDLE_INVALID;
        mible_beacon_start();
        return mible_slave_disconnect();
    }

    for (index = 0; index < MIBLE_CONNECTION_CENTRAL_NUM; ++index) {
        if (p_disconnect->conn_handle == central_info[index].conn_handle) {
            central_info[index].conn_handle = MIBLE_HANDLE_INVALID;
            return mible_master_disconnect(p_disconnect->conn_handle);
        }
    }

    LOG_WARN_TAG(MIBLE_LOG_TAG, "Cannot find the connection %04x", p_disconnect->conn_handle);
    return MIIO_ERROR_EMPTY;
}

mible_connect_t * mible_find_connection_by_handle(uint16_t conn_handle)
{
    if (MIBLE_HANDLE_INVALID == conn_handle) {
        return NULL;
    }

    if (peripheral_info.conn_handle == conn_handle) {
        return &peripheral_info;
    }

    for (uint8_t index = 0; index < MIBLE_CONNECTION_CENTRAL_NUM; ++index) {
        if (central_info[index].conn_handle == conn_handle) {
            return &central_info[index];
        }
    }

    return NULL;
}

mible_connect_t * mible_find_connection_by_address(mible_addr_t address)
{
    if (NULL == address) {
        return NULL;
    }

    if (0 == memcmp(address, peripheral_info.address, sizeof(mible_addr_t)) &&
        MIBLE_HANDLE_INVALID != peripheral_info.conn_handle) {

        return &peripheral_info;
    }

    for (uint8_t index = 0; index < MIBLE_CONNECTION_CENTRAL_NUM; ++index) {
        if (0 == memcmp(address, central_info[index].address, sizeof(mible_addr_t)) &&
            MIBLE_HANDLE_INVALID != central_info[index].conn_handle) {

            return &central_info[index];
        }
    }

    return NULL;
}

int mible_gap_scan(bool enable)
{
    static bool instance = false;

    if (enable) {
        if (true == instance) {
            LOG_WARN_TAG(MIBLE_LOG_TAG, "The system has started scaning");
        } else {
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Start BLE scan");
            if (MIIO_OK != arch_gap_scan_start())
            {
                return MIIO_ERROR;
            }
            instance = true;
        }
    } else {
        if (false == instance) {
            LOG_WARN_TAG(MIBLE_LOG_TAG, "The system has stopped scaning");
        } else {
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Stop BLE scan");
            if (MIIO_OK != arch_gap_scan_stop())
            {
                return MIIO_ERROR;
            }
            instance = false;
        }
    }

    return MIIO_OK;
}

int mible_beacon_start(void)
{
    if (MIIO_OK != arch_os_timer_is_active(gap_timer_handle)) {
        arch_os_timer_activate(gap_timer_handle);
    }

    return MIIO_OK;
}

int mible_beacon_stop(void)
{
    int ret = MIIO_OK;

    if (adv_enabled) {
        ret = arch_gap_adv_stop();
        if (MIIO_OK == ret) {
            adv_enabled = false;
        }
    }

    return ret;
}

static void mible_beacon_timeout(arch_os_timer_handle_t handle)
{
    mible_event_t event;

    memset(&event, 0, sizeof(mible_event_t));
    event.task.func = mible_beacon_send;
    event.task.arg = NULL;
    mible_event_send(MIBLE_EVT_TASK_POST, &event);
}

static void mible_beacon_send(void * arg)
{
    if (MIBLE_HANDLE_INVALID != peripheral_info.conn_handle) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Still connected with mijia APP, retry later");
        return;
    }

    if (mible_net_stop_check()) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "WiFi is shut down, retry later");
        return;
    }

    arch_os_timer_deactivate(gap_timer_handle);

    if (mible_net_beacon_check()) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "The device is ready for config");
        mible_beacon_legacy();
    } else {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "The device is sending proximity");
        mible_beacon_proximity();
    }
}

static void mible_beacon_proximity(void)
{
    int ret;
    uint8_t manu_data[31], wifi_addr[6];
    uint16_t manu_len = 0;

    mible_beacon_stop();

    if (INT8_MIN == gateway_state.proximity_rssi) {
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Stop proximity beacon");
        return;
    }

    arch_get_mac(wifi_addr);

    manu_data[manu_len++] = LO_UINT16(MIBLE_MANUFT_ID);
    manu_data[manu_len++] = HI_UINT16(MIBLE_MANUFT_ID);
    manu_data[manu_len++] = MIBEACON_PROXIMITY_LENGTH;
    manu_data[manu_len++] = MIBEACON_PROXIMITY_TYPE;
    manu_data[manu_len++] = LO_UINT16(gateway_config.product_id);
    manu_data[manu_len++] = HI_UINT16(gateway_config.product_id);
    manu_data[manu_len++] = 0;               /* This device's PID is 2-byte */
    manu_data[manu_len++] = wifi_addr[5];
    manu_data[manu_len++] = wifi_addr[4];
    manu_data[manu_len++] = wifi_addr[3];
    manu_data[manu_len++] = wifi_addr[2];
    manu_data[manu_len++] = wifi_addr[1];
    manu_data[manu_len++] = wifi_addr[0];
    manu_data[manu_len++] = gateway_state.proximity_rssi;

    ret = arch_gap_adv_start(ARCH_ADV_DATA_MANUFACTURER, manu_data, manu_len);
    if (MIIO_OK != ret) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Start proximity beacon fail (err %d)", ret);
        return;
    }

    adv_enabled = true;
}

static void mible_beacon_legacy(void)
{
    int ret;
    uint8_t svc_data[31], capability, wifi_addr[6];
    uint16_t svc_len = 0, frame_ctrl = 0;
    mible_addr_t addr;
    static uint8_t counter = 0;

    ret = arch_address_get(addr);
    if (MIIO_OK != ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "Fail to get BLE address.");
        return;
    }
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Device address -- %02X:%02X:%02X:%02X:%02X:%02X",
              addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);

    arch_get_mac(wifi_addr);
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "WiFi address -- %02X:%02X:%02X:%02X:%02X:%02X",
               wifi_addr[0], wifi_addr[1], wifi_addr[2],
               wifi_addr[3], wifi_addr[4], wifi_addr[5]);

    frame_ctrl = MIBEACON_VERSION |
                 MIBEACON_AUTH_MODE |
                 MIBEACON_ITEM_MAC_INCLUDE |
                 MIBEACON_ITEM_CAP_INCLUDE;
    capability = MIBEACON_CAP_CONNECTABLE |
                 MIBEACON_CAP_ENCRYPTABLE |
                 MIBEACON_CAP_TYPE_COMBO;

    svc_data[svc_len++] = LO_UINT16(MISERVICE_UUID);
    svc_data[svc_len++] = HI_UINT16(MISERVICE_UUID);
    svc_data[svc_len++] = LO_UINT16(frame_ctrl);
    svc_data[svc_len++] = HI_UINT16(frame_ctrl);
    svc_data[svc_len++] = LO_UINT16(gateway_config.product_id);
    svc_data[svc_len++] = HI_UINT16(gateway_config.product_id);
    svc_data[svc_len++] = counter++;
    memcpy(&svc_data[svc_len], addr, sizeof(mible_addr_t));
    svc_len += sizeof(mible_addr_t);
    svc_data[svc_len++] = capability;
    svc_data[svc_len++] = wifi_addr[5];
    svc_data[svc_len++] = wifi_addr[4];

    mible_beacon_stop();

    ret = arch_gap_adv_start(ARCH_ADV_DATA_SERVICE, svc_data, svc_len);
    if (MIIO_OK != ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "Start mi beacom fail (err %d)", ret);
        return;
    }

    adv_enabled = true;
}

static void mible_scan_delay(void * arg)
{
    int ret = mible_gap_scan(true);
    if (MIIO_OK != ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "Fail to start scan.");
    } else {
        LOG_INFO_TAG(MIBLE_LOG_TAG, "Start mible gap scan");
    }
    if(NULL != scan_timer_handle)
    {
        ret = arch_os_timer_delete(scan_timer_handle);
        if (MIIO_OK != ret) {
            LOG_ERROR_TAG(MIBLE_LOG_TAG, "delete scan timer fail (err %d)", ret);
        }
    }
}

static void mible_scan_timeout(arch_os_timer_handle_t handle)
{
    mible_event_t event;

    memset(&event, 0, sizeof(mible_event_t));
    event.task.func = mible_scan_delay;
    event.task.arg = NULL;
    mible_event_send(MIBLE_EVT_TASK_POST, &event);
}

