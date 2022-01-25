/* Includes ------------------------------------------------------------------*/

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_bt_device.h"

#include "arch_api.h"
#include "arch_psm.h"
#include "esp32_handler.h"
#include "esp32_config.h"
#include "esp32_service.h"
//#include "hwcrypto/aes.h"
#include "esp32/aes.h"

#undef  MIBLE_LOG_TAG
#define MIBLE_LOG_TAG		                    "mible_arch"
#undef  LOG_LEVEL
#define LOG_LEVEL                               LOG_LEVEL_INFO

/* Private variables ---------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

int arch_stack_enable(void)
{
    esp_err_t ret;
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    if (ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "%s release BT RAM failed: %s", __func__, esp_err_to_name(ret));
        return MIIO_ERROR;
    }

    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(ret));
        return MIIO_ERROR;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return MIIO_ERROR;
    }

    ret = esp_bluedroid_init();
    if (ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return MIIO_ERROR;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return MIIO_ERROR;
    }

    ret = esp_ble_gatts_register_callback(esp32_gatts_event_handler);
    if (ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "gatts register error, error code = %x", ret);
        return MIIO_ERROR;
    }

    ret = esp_ble_gattc_register_callback(esp32_gattc_event_handler);
    if (ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "gattc register error, error code = %x", ret);
        return MIIO_ERROR;
    }

    ret = esp_ble_gap_register_callback(esp32_gap_event_handler);
    if (ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "gap register error, error code = %x", ret);
        return MIIO_ERROR;
    }

    ret = esp_ble_gatts_app_register(ESP32_GATTS_APP_ID);
    if (ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "gatts app register error, error code = %x", ret);
        return MIIO_ERROR;
    }

    ret = esp_ble_gattc_app_register(ESP32_GATTC_APP_ID);
    if (ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "gattc app register error, error code = %x", ret);
        return MIIO_ERROR;
    }

    if (ESP_BLUEDROID_STATUS_ENABLED != esp_bluedroid_get_status()) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "stack init error");
        return MIIO_ERROR_NOTREADY;
    }

    ret = esp32_event_init();
    if (ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "fail to init esp32 event");
        return MIIO_ERROR;
    }

    ret = mible_gateway_start();
    if (ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "fail to send enable event");
        return MIIO_ERROR;
    }

    return MIIO_OK;
}

int arch_address_get(mible_addr_t addr)
{
    const uint8_t *local_addr;

    local_addr = esp_bt_dev_get_address();
    if (NULL == local_addr) {
        return MIIO_ERROR_NOTFOUND;
    }

    addr[0] = local_addr[5];
    addr[1] = local_addr[4];
    addr[2] = local_addr[3];
    addr[3] = local_addr[2];
    addr[4] = local_addr[1];
    addr[5] = local_addr[0];

    return MIIO_OK;
}

int arch_generate_random(uint8_t * buffer, uint16_t length)
{
    uint32_t value;
    uint16_t copy_len;

    while (length > 0) {
        copy_len = (length > sizeof(value)) ? sizeof(value) : length;
        value = esp_random();
        memcpy(buffer, &value, copy_len);
        buffer += copy_len;
        length -= copy_len;
    }

    return MIIO_OK;
}

int arch_aes_encrypt(uint8_t out[16], const uint8_t in[16], const uint8_t key[16])
{
    esp_aes_context context;

    if (NULL == out || NULL == in || NULL == key) {
        return MIIO_ERROR_PARAM;
    }

    memcpy(&context.key[0], key, 16);
    context.key_bytes = 16;
    esp_aes_crypt_ecb(&context, ESP_AES_ENCRYPT, in, out);

    return MIIO_OK;
}

int arch_info_save(arch_info_t *p_info)
{
    if (NULL == p_info) {
        return MIIO_ERROR_PARAM;
    }

    struct {
        uint16_t product_id;
        uint8_t version[10];
        uint8_t name[10];
    } tmp;

    tmp.product_id = p_info->product_id;

    int ret = arch_psm_set_value("ble", "info", &tmp, sizeof(tmp));
    if (sizeof(tmp) != ret) {
        return MIIO_ERROR_SIZE;
    }

    return MIIO_OK;
}

int arch_info_restore(arch_info_t *p_info)
{
    if (NULL == p_info) {
        return MIIO_ERROR_PARAM;
    }

    struct {
        uint16_t product_id;
        uint8_t version[10];
        uint8_t name[10];
    } tmp;

    int ret = arch_psm_get_value("ble", "info", &tmp, sizeof(tmp));
    if (sizeof(tmp) != ret) {
        return MIIO_ERROR_SIZE;
    }

    p_info->product_id = tmp.product_id;

    return MIIO_OK;
}

int arch_gap_connect(mible_addr_t address)
{
    esp_bd_addr_t remote_bda;
    esp_err_t status;

    if (ESP_GATT_IF_NONE == esp32_gattc_if) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "The BLE stack is not ready");
        return MIIO_ERROR_NOTREADY;
    }

    remote_bda[0] = address[5];
    remote_bda[1] = address[4];
    remote_bda[2] = address[3];
    remote_bda[3] = address[2];
    remote_bda[4] = address[1];
    remote_bda[5] = address[0];

    status = esp_ble_gattc_open(esp32_gattc_if, remote_bda,
                                BLE_ADDR_TYPE_PUBLIC, true);
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "%s -- open status %d", __func__, status);

    return (ESP_OK == status) ? MIIO_OK : MIIO_ERROR;
}

int arch_gap_cancel_connection(void)
{
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "The stack has stopped trying");
    return MIIO_OK;
}

int arch_gap_disconnect(uint16_t conn_handle)
{
    esp_err_t status;
    esp_bd_addr_t address;

    esp32_find_connect(conn_handle, address);
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "disconnect with %02x:%02x:%02x:%02x:%02x:%02x",
                  address[0], address[1], address[2], address[3], address[4], address[5]);
    status = esp_ble_gap_disconnect(address);

    return (ESP_OK == status) ? MIIO_OK : MIIO_ERROR;
}

int arch_gap_adv_start(arch_adv_data_t type, uint8_t * data, uint16_t data_len)
{
    int ret = MIIO_OK;
    esp_ble_adv_data_t adv_data = {0};
    esp_ble_adv_params_t adv_params = {0};

    if (NULL == data || 0 == data_len) {
        return MIIO_ERROR_PARAM;
    }

    switch (type) {
        case ARCH_ADV_DATA_SERVICE:
            adv_data.service_data_len = data_len;
            adv_data.p_service_data = data;
            adv_params.adv_int_min = 0xC0;      // legacy int 120ms
            adv_params.adv_int_max = 0xC0;
            break;
        case ARCH_ADV_DATA_MANUFACTURER:
            adv_data.manufacturer_len = data_len;
            adv_data.p_manufacturer_data = data;
            adv_params.adv_int_min = 0x180;     // prox int 240ms
            adv_params.adv_int_max = 0x180;
            break;
        default:
            return MIIO_ERROR_PARAM;
    }

    adv_data.set_scan_rsp = false;
    adv_data.include_name = false;
    adv_data.include_txpower = false;
    adv_data.min_interval = 0;
    adv_data.max_interval = 0;
    adv_data.appearance = 0;
    adv_data.service_uuid_len = 0;
    adv_data.p_service_uuid = NULL;
    adv_data.flag = ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT;

    adv_params.adv_type = ADV_TYPE_IND;
    adv_params.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
    adv_params.channel_map = ADV_CHNL_ALL;
    adv_params.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY;

    esp32_event_lock();

    if (ESP_OK == esp_ble_gap_config_adv_data(&adv_data)) {
        esp32_event_wait();
    } else {
        ret = MIIO_ERROR_NOTREADY;
        goto end;
    }

    if (ESP_OK == esp_ble_gap_start_advertising(&adv_params)) {
        esp32_event_wait();
    } else {
        ret = MIIO_ERROR_NOTREADY;
        goto end;
    }

end:
    esp32_event_unlock();
    return ret;
}

int arch_gap_adv_stop(void)
{
    int ret = MIIO_OK;

    esp32_event_lock();

    if (ESP_OK == esp_ble_gap_stop_advertising()) {
        esp32_event_wait();
    } else {
        ret = MIIO_ERROR_NOTREADY;
        goto end;
    }

end:
    esp32_event_unlock();
    return ret;
}

int arch_gap_scan_start(void)
{
    int ret = MIIO_OK;
    esp_ble_scan_params_t scan_params = {0};

    scan_params.scan_type = BLE_SCAN_TYPE_ACTIVE;
    scan_params.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
    scan_params.scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL;
    scan_params.scan_interval = ARCH_SCAN_PARAM1_INTERVAL;
    scan_params.scan_window = ARCH_SCAN_PARAM1_WINDOW;
    scan_params.scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE;

    mible_net_state_t state = mible_net_state_get();
    switch(state){
    case NET_CONFIG_STATE_DISCONNECTED:
        scan_params.scan_interval = ARCH_SCAN_PARAM2_INTERVAL;
        scan_params.scan_window = ARCH_SCAN_PARAM2_WINDOW;
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "The wifi is in AP mode");
        break;
    case NET_CONFIG_STATE_CONNECTING:
        scan_params.scan_interval = ARCH_SCAN_PARAM3_INTERVAL;
        scan_params.scan_window = ARCH_SCAN_PARAM3_WINDOW;
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "The wifi is connecting");
        break;
    default:
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "The wifi is in another mode");
        break;
    }

    esp32_event_lock();

    if (ESP_OK == esp_ble_gap_set_scan_params(&scan_params)) {
        if(MIIO_OK != esp32_event_wait())
        {
            ret = MIIO_ERROR_TIMEOUT;
            goto end;
        }
    } else {
        ret = MIIO_ERROR_NOTREADY;
        goto end;
    }

    if (ESP_OK == esp_ble_gap_start_scanning(~0)) {
        if(MIIO_OK != esp32_event_wait())
        {
            ret = MIIO_ERROR_TIMEOUT;
            goto end;
        }
    } else {
        ret = MIIO_ERROR_NOTREADY;
        goto end;
    }

end:
    esp32_event_unlock();
    return ret;
}

int arch_gap_scan_stop(void)
{
    int ret = MIIO_OK;

    esp32_event_lock();

    if (ESP_OK == esp_ble_gap_stop_scanning()) {
        if(MIIO_OK != esp32_event_wait())
        {
            ret = MIIO_ERROR_TIMEOUT;
            goto end;
        }
    } else {
        ret = MIIO_ERROR_NOTREADY;
        goto end;
    }

end:
    esp32_event_unlock();
    return ret;
}

int arch_gatts_init(arch_gatt_t * p_gatt)
{
    int ret = MIIO_OK;

    if (NULL == p_gatt) {
        return MIIO_ERROR_PARAM;
    }

    esp32_event_lock();
    arch_gatt_db = p_gatt;
    if (ESP_OK == esp_ble_gatts_create_attr_tab(mi_attr_db, esp32_gatts_if,
                                                MI_IDX_NB, 0)) {

        esp32_event_wait();
    } else {
        ret = MIIO_ERROR_NOTREADY;
    }

    esp32_event_unlock();
    return ret;
}

int arch_gatts_value_set(uint16_t handle, void * buffer, uint16_t length)
{
    esp_err_t ret = esp_ble_gatts_set_attr_value(handle, length, buffer);

    return (ESP_OK == ret) ? MIIO_OK : MIIO_ERROR;
}

int arch_gatts_value_notify(uint16_t conn_handle, uint16_t handle, void * buffer, uint16_t length)
{
    esp_err_t ret = esp_ble_gatts_send_indicate(esp32_gatts_if, conn_handle, 
                                                handle, length, buffer, false);

    return (ESP_OK == ret) ? MIIO_OK : MIIO_ERROR;
}

int arch_gattc_search_service(uint16_t conn_handle)
{
    esp_bt_uuid_t mi_uuid;
    esp_err_t ret;

    mi_uuid.len = ESP_UUID_LEN_16;
    mi_uuid.uuid.uuid16 = MISERVICE_UUID;
    ret = esp_ble_gattc_search_service(esp32_gattc_if, conn_handle, &mi_uuid);

    return (ESP_OK == ret) ? MIIO_OK : MIIO_ERROR;
}

int arch_gattc_get_service(uint16_t conn_handle, arch_gatt_t * p_gatt)
{
    esp_bt_uuid_t mi_uuid;
    esp_err_t ret;
    esp_gattc_service_elem_t svc_elem;
    esp_gattc_char_elem_t chr_elem;
    esp_gattc_descr_elem_t descr_elem;
    uint16_t counter = 1;

    mi_uuid.len = ESP_UUID_LEN_16;
    mi_uuid.uuid.uuid16 = MISERVICE_UUID;
    ret = esp_ble_gattc_get_service(esp32_gattc_if, conn_handle, &mi_uuid,
                                    &svc_elem, &counter, 0);
    if (ESP_GATT_OK != ret) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Fail to get mi service");
        return MIIO_ERROR;
    }

    mi_uuid.uuid.uuid16 = MISERVICE_CHAR_TOKEN_UUID;
    counter = 1;
    ret = esp_ble_gattc_get_char_by_uuid(esp32_gattc_if, conn_handle, svc_elem.start_handle,
                                         svc_elem.end_handle, mi_uuid, &chr_elem, &counter);
    if (ESP_GATT_OK == ret) {
        p_gatt->token_handle = chr_elem.char_handle;
    } else {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Fail to get token char");
        return MIIO_ERROR;
    }

    mi_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
    ret = esp_ble_gattc_get_descr_by_char_handle(esp32_gattc_if, conn_handle, p_gatt->token_handle,
                                                 mi_uuid, &descr_elem, &counter);
    if (ESP_GATT_OK == ret) {
        p_gatt->token_ccc_handle = descr_elem.handle;
    } else {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Fail to get token ccc");
        return MIIO_ERROR;
    }

    mi_uuid.uuid.uuid16 = MISERVICE_CHAR_BEACONKEY_UUID;
    counter = 1;
    ret = esp_ble_gattc_get_char_by_uuid(esp32_gattc_if, conn_handle, svc_elem.start_handle,
                                         svc_elem.end_handle, mi_uuid, &chr_elem, &counter);
    if (ESP_GATT_OK == ret) {
        p_gatt->key_handle = chr_elem.char_handle;
    } else {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Fail to get beaconkey char");
        return MIIO_ERROR;
    }

    mi_uuid.uuid.uuid16 = MISERVICE_CHAR_STAND_AUTH_UUID;
    counter = 1;
    ret = esp_ble_gattc_get_char_by_uuid(esp32_gattc_if, conn_handle, svc_elem.start_handle,
                                         svc_elem.end_handle, mi_uuid, &chr_elem, &counter);
    if (ESP_GATT_OK == ret) {
        p_gatt->auth_handle = chr_elem.char_handle;

        //search for ctrl point handle
        mi_uuid.uuid.uuid16 = MISERVICE_CHAR_CTRL_POINT_UUID;
        counter = 1;
        ret = esp_ble_gattc_get_char_by_uuid(esp32_gattc_if, conn_handle, svc_elem.start_handle,
                                         svc_elem.end_handle, mi_uuid, &chr_elem, &counter);
        if (ESP_GATT_OK == ret) {
        p_gatt->ctrl_handle = chr_elem.char_handle;
        } else {
            LOG_WARN_TAG(MIBLE_LOG_TAG, "Fail to get ctrl char");
            return MIIO_ERROR;
        }
    } else {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "Fail to get stand auth char");
        // return MIIO_ERROR;
        // here to distinguish standard auth or rc4
        //search for lagacy rc4 auth handle
        mi_uuid.uuid.uuid16 = MISERVICE_CHAR_CTRL_POINT_UUID;
        counter = 1;
        ret = esp_ble_gattc_get_char_by_uuid(esp32_gattc_if, conn_handle, svc_elem.start_handle,
                                         svc_elem.end_handle, mi_uuid, &chr_elem, &counter);
        if (ESP_GATT_OK == ret) {
        p_gatt->auth_handle = chr_elem.char_handle;
        } else {
            LOG_WARN_TAG(MIBLE_LOG_TAG, "Fail to get lagacy rc4 auth char");
            return MIIO_ERROR;
        }
        
        return MIIO_OK;
    }

    return MISERVICE_CHAR_STAND_AUTH_UUID;
}

int arch_gattc_value_write(uint16_t conn_handle, uint16_t handle,
                                        void * buffer, uint16_t length)
{
    esp_err_t ret = esp_ble_gattc_write_char(esp32_gattc_if, conn_handle,
                                             handle, length, buffer,
                                             ESP_GATT_WRITE_TYPE_RSP,
                                             ESP_GATT_AUTH_REQ_NONE);
    return (ESP_OK == ret) ? MIIO_OK : MIIO_ERROR;
}

int arch_gattc_value_write_no_rsp(uint16_t conn_handle, uint16_t handle, void * buffer, uint16_t length)
{
    esp_err_t ret = esp_ble_gattc_write_char(esp32_gattc_if, conn_handle,
                                             handle, length, buffer,
                                             ESP_GATT_WRITE_TYPE_RSP,
                                             ESP_GATT_AUTH_REQ_NONE);
    return (ESP_OK == ret) ? MIIO_OK : MIIO_ERROR; 
}

int arch_gattc_value_read(uint16_t conn_handle, uint16_t handle)
{
    esp_err_t ret = esp_ble_gattc_read_char(esp32_gattc_if, conn_handle,
                                            handle, ESP_GATT_AUTH_REQ_NONE);
    return (ESP_OK == ret) ? MIIO_OK : MIIO_ERROR;
}

int arch_gattc_notify_enable(uint16_t conn_handle, uint16_t handle)
{
    uint16_t data = 0x0001, counter = 1;
    esp_bd_addr_t remote_bda;
    esp_gattc_descr_elem_t descr_elem;
    esp_bt_uuid_t notify_descr_uuid = {
            .len = ESP_UUID_LEN_16,
            .uuid = {.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG,},
    };

    esp32_find_connect(conn_handle, remote_bda);
    if (ESP_OK != esp_ble_gattc_register_for_notify(esp32_gattc_if,
                                                remote_bda, handle)) {

        LOG_WARN_TAG(MIBLE_LOG_TAG, "Register for notify fail");
        return MIIO_ERROR;
    }

    if (ESP_GATT_OK != esp_ble_gattc_get_descr_by_char_handle(esp32_gattc_if,
              conn_handle, handle, notify_descr_uuid, &descr_elem, &counter)) {

        LOG_WARN_TAG(MIBLE_LOG_TAG, "Find char descr fail");
        return MIIO_ERROR;
    }

    if (ESP_OK != esp_ble_gattc_write_char_descr(esp32_gattc_if, conn_handle,
                             descr_elem.handle, sizeof(data), (uint8_t *)&data,
                             ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE)) {

        LOG_WARN_TAG(MIBLE_LOG_TAG, "Write char descr fail");
        return MIIO_ERROR;
    }

    return MIIO_OK;
}

int arch_gattc_notify_enable_no_rsp(uint16_t conn_handle, uint16_t handle)
{
    return arch_gattc_notify_enable(conn_handle, handle);
}

