/* Includes ------------------------------------------------------------------*/

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"

#include "mible_def.h"
#include "esp32_service.h"
#include "arch_def.h"

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t char_client_cfg_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t char_prop_notify_write = ESP_GATT_CHAR_PROP_BIT_NOTIFY | ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t char_prop_read = ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_write = ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t char_prop_write_no_rsp = ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
static const uint8_t char_prop_notify_write_no_rsp = ESP_GATT_CHAR_PROP_BIT_NOTIFY | ESP_GATT_CHAR_PROP_BIT_WRITE_NR;


static const uint16_t char_client_cfg_value = 0;

static const uint16_t mi_service_uuid = MISERVICE_UUID;
static const uint16_t mi_char_token_uuid = MISERVICE_CHAR_TOKEN_UUID;
static const uint16_t mi_char_pid_uuid = MISERVICE_CHAR_PRODUCTID_UUID;
static const uint16_t mi_char_version_uuid = MISERVICE_CHAR_VERSION_UUID;
static const uint16_t mi_char_wifi_uuid = MISERVICE_CHAR_WIFICFG_UUID;
static const uint16_t mi_char_ctrl_uuid = MISERVICE_CHAR_CTRL_POINT_UUID;
static const uint16_t mi_char_auth_uuid = MISERVICE_CHAR_STAND_AUTH_UUID;


/* Exported variables --------------------------------------------------------*/

const esp_gatts_attr_db_t mi_attr_db[MI_IDX_NB] = {
    [MI_IDX_SVC] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16,
         (uint8_t *)&primary_service_uuid,
         ESP_GATT_PERM_READ,
         sizeof(mi_service_uuid),
         sizeof(mi_service_uuid),
         (uint8_t *)&mi_service_uuid}
    },
    [MI_IDX_TOKEN_CHAR] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16,
         (uint8_t *)&character_declaration_uuid,
         ESP_GATT_PERM_READ,
         sizeof(char_prop_notify_write),
         sizeof(char_prop_notify_write),
         (uint8_t *)&char_prop_notify_write}
    },
    [MI_IDX_TOKEN_CHAR_VAL] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16,
         (uint8_t *)&mi_char_token_uuid,
         ESP_GATT_PERM_WRITE,
         MISERVICE_CHAR_TOKEN_LEN,
         0,
         NULL}
    },
    [MI_IDX_TOKEN_CLIENT_CFG] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16,
         (uint8_t *)&char_client_cfg_uuid,
         ESP_GATT_PERM_WRITE | ESP_GATT_PERM_READ,
         sizeof(char_client_cfg_value),
         sizeof(char_client_cfg_value),
         (uint8_t *)&char_client_cfg_value}
    },
    [MI_IDX_PRODUCTID_CHAR] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16,
         (uint8_t *)&character_declaration_uuid,
         ESP_GATT_PERM_READ,
         sizeof(char_prop_read),
         sizeof(char_prop_read),
         (uint8_t *)&char_prop_read}
    },
    [MI_IDX_PRODUCTID_CHAR_VAL] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16,
         (uint8_t *)&mi_char_pid_uuid,
         ESP_GATT_PERM_READ,
         MISERVICE_CHAR_PRODUCTID_LEN,
         0,
         NULL}
    },
    [MI_IDX_VER_CHAR] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16,
         (uint8_t *)&character_declaration_uuid,
         ESP_GATT_PERM_READ,
         sizeof(char_prop_read),
         sizeof(char_prop_read),
         (uint8_t *)&char_prop_read}
    },
    [MI_IDX_VER_CHAR_VAL] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16,
         (uint8_t *)&mi_char_version_uuid,
         ESP_GATT_PERM_READ,
         MISERVICE_CHAR_VERSION_LEN,
         0,
         NULL}
    },
    [MI_IDX_WIFI_CFG_CHAR] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16,
         (uint8_t *)&character_declaration_uuid,
         ESP_GATT_PERM_READ,
         sizeof(char_prop_notify_write),
         sizeof(char_prop_notify_write),
         (uint8_t *)&char_prop_notify_write}
    },
    [MI_IDX_WIFI_CFG_CHAR_VAL] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16,
         (uint8_t *)&mi_char_wifi_uuid,
         ESP_GATT_PERM_WRITE | ESP_GATT_PERM_READ,
         MISERVICE_CHAR_WIFICFG_LEN,
         0,
         NULL}
    },
    [MI_IDX_WIFI_CFG_CLIENT_CFG] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16,
         (uint8_t *)&char_client_cfg_uuid,
         ESP_GATT_PERM_WRITE | ESP_GATT_PERM_READ,
         sizeof(char_client_cfg_value),
         sizeof(char_client_cfg_value),
         (uint8_t *)&char_client_cfg_value}
    },
	[MI_IDX_CTRL_CHAR] = {
	    {ESP_GATT_AUTO_RSP},
	    {ESP_UUID_LEN_16,
	    (uint8_t *)&character_declaration_uuid,
	    ESP_GATT_PERM_READ,
	    sizeof(char_prop_notify_write_no_rsp),
	    sizeof(char_prop_notify_write_no_rsp),
	    (uint8_t *)&char_prop_notify_write_no_rsp}
	},
	[MI_IDX_CTRL_CHAR_VAL] = {
	    {ESP_GATT_AUTO_RSP},
	    {ESP_UUID_LEN_16,
	    (uint8_t *)&mi_char_ctrl_uuid,
	    ESP_GATT_PERM_WRITE,
	    MISERVICE_CHAR_CTRL_LEN,
	    0,
	    NULL}
	},
	[MI_IDX_CTRL_CHAR_CLIENT_CFG] = {
	    {ESP_GATT_AUTO_RSP},
	    {ESP_UUID_LEN_16,
	    (uint8_t *)&char_client_cfg_uuid,
	    ESP_GATT_PERM_WRITE | ESP_GATT_PERM_READ,
	    sizeof(char_client_cfg_value),
	    sizeof(char_client_cfg_value),
	    (uint8_t *)&char_client_cfg_value}
	},

    [MI_IDX_AUTHEN_CHAR] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16,
         (uint8_t *)&character_declaration_uuid,
         ESP_GATT_PERM_READ,
         sizeof(char_prop_notify_write_no_rsp),
         sizeof(char_prop_notify_write_no_rsp),
         (uint8_t *)&char_prop_notify_write_no_rsp}
    },
    [MI_IDX_AUTHEN_CHAR_VAL] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16,
        (uint8_t *)&mi_char_auth_uuid,
        ESP_GATT_PERM_WRITE,
        MISERVICE_CHAR_AUTH_LEN,
        0,
        NULL}
    },
    [MI_IDX_AUTHEN_CHAR_CLIENT_CFG] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16,
        (uint8_t *)&char_client_cfg_uuid,
        ESP_GATT_PERM_WRITE | ESP_GATT_PERM_READ,
        sizeof(char_client_cfg_value),
        sizeof(char_client_cfg_value),
        (uint8_t *)&char_client_cfg_value}
	}
};

/* Exported functions --------------------------------------------------------*/


