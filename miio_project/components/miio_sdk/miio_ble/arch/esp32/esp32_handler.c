/* Includes ------------------------------------------------------------------*/

#include "esp32_handler.h"
#include "esp32_service.h"
#include "esp32_config.h"
#include "mible_gateway.h"
#include "adv_beacon.h"

#undef  MIBLE_LOG_TAG
#define MIBLE_LOG_TAG		                    "mible_arch"
#undef  LOG_LEVEL
#define LOG_LEVEL                               LOG_LEVEL_INFO

#define ESP32_HANDLE_CONNECT_NUM                5

extern uint8_t L2CA_GetBleConnRole (esp_bd_addr_t bd_addr);

static void esp32_event_connect(esp_bd_addr_t remote_bda, uint16_t conn_id);
static void esp32_event_disconnect(esp_bd_addr_t remote_bda, uint16_t conn_id);

/* Private variables ---------------------------------------------------------*/

static xSemaphoreHandle esp32_lock_sem, esp32_wait_sem;
static struct gatts_connect_evt_param esp32_connect[ESP32_HANDLE_CONNECT_NUM];

/* Exported variables --------------------------------------------------------*/

esp_gatt_if_t esp32_gatts_if = ESP_GATT_IF_NONE;
esp_gatt_if_t esp32_gattc_if = ESP_GATT_IF_NONE;
arch_gatt_t * arch_gatt_db = NULL;

/* Exported functions --------------------------------------------------------*/

int esp32_event_init(void)
{
    esp32_lock_sem = xSemaphoreCreateCounting(1, 1);
    esp32_wait_sem = xSemaphoreCreateCounting(1, 0);

    if (NULL == esp32_lock_sem || NULL == esp32_wait_sem) {
        return MIIO_ERROR_NOMEM;
    }

    memset(&esp32_connect[0], 0,
        sizeof(struct gatts_connect_evt_param) * ESP32_HANDLE_CONNECT_NUM);

    return MIIO_OK;
}

void esp32_event_lock(void)
{
    xSemaphoreTake(esp32_lock_sem, portMAX_DELAY);
}

void esp32_event_unlock(void)
{
    xSemaphoreGive(esp32_lock_sem);
}

int esp32_event_wait(void)
{
    if (pdPASS != xSemaphoreTake(esp32_wait_sem, pdMS_TO_TICKS(10 * 1000)))
    {
        return MIIO_ERROR_TIMEOUT;
    }
    return MIIO_OK;
}

void esp32_gatts_event_handler(esp_gatts_cb_event_t event,
                        esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    mible_event_t mible_event;

    if (ESP_GATTS_REG_EVT == event) {
        if (ESP_GATT_OK == param->reg.status &&
            ESP32_GATTS_APP_ID == param->reg.app_id) {

            esp32_gatts_if = gatts_if;
        }
        return;
    }

    if (ESP_GATT_IF_NONE != gatts_if && esp32_gatts_if != gatts_if) {
        return;
    }

    switch (event) {
        case ESP_GATTS_CREAT_ATTR_TAB_EVT:
            if (ESP_GATT_OK == param->add_attr_tab.status &&
                ESP_UUID_LEN_16 == param->add_attr_tab.svc_uuid.len &&
                MISERVICE_UUID == param->add_attr_tab.svc_uuid.uuid.uuid16 &&
                MI_IDX_NB == param->add_attr_tab.num_handle &&
                NULL != arch_gatt_db) {

                arch_gatt_db->token_handle = param->add_attr_tab.handles[MI_IDX_TOKEN_CHAR_VAL];
                arch_gatt_db->token_ccc_handle = param->add_attr_tab.handles[MI_IDX_TOKEN_CLIENT_CFG];
                arch_gatt_db->pid_handle = param->add_attr_tab.handles[MI_IDX_PRODUCTID_CHAR_VAL];
                arch_gatt_db->version_handle = param->add_attr_tab.handles[MI_IDX_VER_CHAR_VAL];
                arch_gatt_db->wifi_handle = param->add_attr_tab.handles[MI_IDX_WIFI_CFG_CHAR_VAL];
                arch_gatt_db->wifi_ccc_handle = param->add_attr_tab.handles[MI_IDX_WIFI_CFG_CLIENT_CFG];
                arch_gatt_db->ctrl_handle = param->add_attr_tab.handles[MI_IDX_CTRL_CHAR_VAL];
                arch_gatt_db->ctrl_ccc_handle = param->add_attr_tab.handles[MI_IDX_CTRL_CHAR_CLIENT_CFG];
                arch_gatt_db->auth_handle = param->add_attr_tab.handles[MI_IDX_AUTHEN_CHAR_VAL];
                arch_gatt_db->auth_ccc_handle = param->add_attr_tab.handles[MI_IDX_AUTHEN_CHAR_CLIENT_CFG];
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Create mi service success.");
                esp_ble_gatts_start_service(param->add_attr_tab.handles[MI_IDX_SVC]);
            }
            xSemaphoreGive(esp32_wait_sem);
            break;
        case ESP_GATTS_CONNECT_EVT:
            if (1 == L2CA_GetBleConnRole(param->connect.remote_bda)) {
                mible_event.connect.local_role = MIBLE_LOCAL_ROLE_SLAVE;
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Connect %04x as slave", param->connect.conn_id);
            } else if (0 == L2CA_GetBleConnRole(param->connect.remote_bda)) {
                mible_event.connect.local_role = MIBLE_LOCAL_ROLE_MASTER;
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Connect %04x as master", param->connect.conn_id);
            } else {
                LOG_WARN_TAG(MIBLE_LOG_TAG, "Connect %04x in unknown role", param->connect.conn_id);
                break;
            }
            esp32_event_connect(param->connect.remote_bda, param->connect.conn_id);
            mible_event.connect.conn_handle = param->connect.conn_id;
            mible_event.connect.address[0] = param->connect.remote_bda[5];
            mible_event.connect.address[1] = param->connect.remote_bda[4];
            mible_event.connect.address[2] = param->connect.remote_bda[3];
            mible_event.connect.address[3] = param->connect.remote_bda[2];
            mible_event.connect.address[4] = param->connect.remote_bda[1];
            mible_event.connect.address[5] = param->connect.remote_bda[0];
            mible_event_send(MIBLE_EVT_GAP_CONNECT, &mible_event);
            break;
        case ESP_GATTS_DISCONNECT_EVT:
            esp32_event_disconnect(param->disconnect.remote_bda, param->disconnect.conn_id);
            mible_event.disconnect.conn_handle = param->disconnect.conn_id;
            mible_event_send(MIBLE_EVT_GAP_DISCONNECT, &mible_event);
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Disconnect reason %04x", param->disconnect.reason);
            break;
        case ESP_GATTS_WRITE_EVT:
            mible_event.gatts_write.handle = param->write.handle;
            mible_event.gatts_write.length = param->write.len;
            memcpy(mible_event.gatts_write.value, param->write.value, param->write.len);
            mible_event_send(MIBLE_EVT_GATTS_WRITE, &mible_event);
            break;
        default:
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "GATTS -- unhandled event %d", event);
            break;
    }
}

void esp32_gattc_event_handler(esp_gattc_cb_event_t event,
                        esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    mible_event_t mible_event;

    if (ESP_GATTC_REG_EVT == event) {
        if (ESP_GATT_OK == param->reg.status &&
            ESP32_GATTC_APP_ID == param->reg.app_id) {

            esp32_gattc_if = gattc_if;
        }
        return;
    }

    if (ESP_GATT_IF_NONE != gattc_if && esp32_gattc_if != gattc_if) {
        return;
    }

    switch (event) {
        case ESP_GATTC_SEARCH_CMPL_EVT:
            if (ESP_GATT_OK == param->search_cmpl.status) {
                mible_event.gattc_found.conn_handle = param->search_cmpl.conn_id;
                mible_event_send(MIBLE_EVT_GATTC_FOUND, &mible_event);
            } else {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "Fail to discover mi service (err %d)", param->search_cmpl.status);
            }
            break;
        case ESP_GATTC_WRITE_CHAR_EVT:
        case ESP_GATTC_WRITE_DESCR_EVT:
            if (ESP_GATT_OK == param->write.status) {
                mible_event.gattc_write.conn_handle = param->write.conn_id;
                mible_event.gattc_write.handle = param->write.handle;
                mible_event_send(MIBLE_EVT_GATTC_WRITE, &mible_event);
            } else {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "Fail to write mi service (err %d)", param->write.status);
            }
            break;
        case ESP_GATTC_NOTIFY_EVT:
            if (param->notify.is_notify) {
                mible_event.gattc_notify.conn_handle = param->notify.conn_id;
                mible_event.gattc_notify.handle = param->notify.handle;
                mible_event.gattc_notify.len = (param->notify.value_len > 20) ? 
                                                20 : param->notify.value_len;
                memcpy(&mible_event.gattc_notify.data[0],
                       param->notify.value, mible_event.gattc_notify.len);
                mible_event_send(MIBLE_EVT_GATTC_NOTIFY, &mible_event);
            }
            break;
        case ESP_GATTC_READ_CHAR_EVT:
            if (ESP_GATT_OK == param->read.status) {
                mible_event.gattc_read.conn_handle = param->read.conn_id;
                mible_event.gattc_read.handle = param->read.handle;
                mible_event.gattc_read.len = (param->read.value_len > 20) ?
                                              20 : param->read.value_len;
                memcpy(&mible_event.gattc_read.data[0],
                       param->read.value, mible_event.gattc_read.len);
                mible_event_send(MIBLE_EVT_GATTC_READ, &mible_event);
            } else {
                LOG_ERROR_TAG(MIBLE_LOG_TAG, "Fail to read mi service (err %d)", param->read.status);
            }
            break;
        default:
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "GATTC -- unhandled event %d", event);
            break;
    }
}

extern void light_ble_rx(unsigned char *mac,unsigned char *bledata,uint8_t datalen);

void esp32_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Event %d status %d", event, param->adv_data_cmpl.status);
            xSemaphoreGive(esp32_wait_sem);
            break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Event %d status %d", event, param->adv_start_cmpl.status);
            xSemaphoreGive(esp32_wait_sem);
            break;
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Event %d status %d", event, param->adv_stop_cmpl.status);
            xSemaphoreGive(esp32_wait_sem);
            break;
        case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Event %d status %d", event, param->scan_param_cmpl.status);
            xSemaphoreGive(esp32_wait_sem);
            break;
        case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Event %d status %d", event, param->scan_start_cmpl.status);
            xSemaphoreGive(esp32_wait_sem);
            break;
        case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Event %d status %d", event, param->scan_stop_cmpl.status);
            xSemaphoreGive(esp32_wait_sem);
            break;
        case ESP_GAP_BLE_SCAN_RESULT_EVT:
            if (ESP_GAP_SEARCH_INQ_RES_EVT == param->scan_rst.search_evt) {
                mible_addr_t address;
                address[0] = param->scan_rst.bda[5];
                address[1] = param->scan_rst.bda[4];
                address[2] = param->scan_rst.bda[3];
                address[3] = param->scan_rst.bda[2];
                address[4] = param->scan_rst.bda[1];
                address[5] = param->scan_rst.bda[0];

                if (param->scan_rst.adv_data_len > 0)
                {
                	light_ble_rx(&param->scan_rst.bda[0],&param->scan_rst.ble_adv[0],param->scan_rst.adv_data_len);

                    ble_adv_parse(address, &param->scan_rst.ble_adv[0],
                            param->scan_rst.adv_data_len, param->scan_rst.rssi);
                }
                if (param->scan_rst.scan_rsp_len > 0)
                {
                	light_ble_rx(&param->scan_rst.bda[0],&param->scan_rst.ble_adv[param->scan_rst.adv_data_len],param->scan_rst.scan_rsp_len);

                    ble_adv_parse(address, &param->scan_rst.ble_adv[param->scan_rst.adv_data_len],
                            param->scan_rst.scan_rsp_len, param->scan_rst.rssi);
                }
            } else if (ESP_GAP_SEARCH_INQ_DISCARD_NUM_EVT == param->scan_rst.search_evt) {
                LOG_WARN_TAG(MIBLE_LOG_TAG, "Scan result discard %d packets", param->scan_rst.num_dis);
            } else {
                LOG_DEBUG_TAG(MIBLE_LOG_TAG, "Scan result type %d", param->scan_rst.search_evt);
            }
            break;
        default:
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "GAP -- unhandled event %d", event);
            break;
    }
}

static void esp32_event_connect(esp_bd_addr_t remote_bda, uint16_t conn_id)
{
    esp_bd_addr_t tmp = {0};

    if (0 == memcmp(tmp, remote_bda, sizeof(esp_bd_addr_t))) {
        return;
    }

    for (int index = 0; index < ESP32_HANDLE_CONNECT_NUM; ++index) {
        if (0 == memcmp(tmp, esp32_connect[index].remote_bda, sizeof(esp_bd_addr_t))) {
            memcpy(esp32_connect[index].remote_bda, remote_bda, sizeof(esp_bd_addr_t));
            esp32_connect[index].conn_id = conn_id;
            break;
        }
    }
}

static void esp32_event_disconnect(esp_bd_addr_t remote_bda, uint16_t conn_id)
{
    esp_bd_addr_t tmp = {0};

    if (0 == memcmp(tmp, remote_bda, sizeof(esp_bd_addr_t))) {
        return;
    }

    for (int index = 0; index < ESP32_HANDLE_CONNECT_NUM; ++index) {
        if (0 == memcmp(remote_bda, esp32_connect[index].remote_bda, sizeof(esp_bd_addr_t)) &&
            conn_id == esp32_connect[index].conn_id) {

            memset(&esp32_connect[index], 0, sizeof(struct gatts_connect_evt_param));
            break;
        }
    }
}

void esp32_find_connect(uint16_t conn_id, esp_bd_addr_t remote_bda)
{
    esp_bd_addr_t tmp = {0};

    for (int index = 0; index < ESP32_HANDLE_CONNECT_NUM; ++index) {
        if (0 != memcmp(tmp, esp32_connect[index].remote_bda, sizeof(esp_bd_addr_t)) &&
            conn_id == esp32_connect[index].conn_id) {

            memcpy(remote_bda, esp32_connect[index].remote_bda, sizeof(esp_bd_addr_t));
            return;
        }
    }

    memcpy(remote_bda, tmp, sizeof(esp_bd_addr_t));
}

