/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ESP32_SERVICE_H__
#define __ESP32_SERVICE_H__

/* Private define ------------------------------------------------------------*/

enum
{
    MI_IDX_SVC = 0,

    MI_IDX_TOKEN_CHAR,
    MI_IDX_TOKEN_CHAR_VAL,
    MI_IDX_TOKEN_CLIENT_CFG, 
    
    MI_IDX_PRODUCTID_CHAR,
    MI_IDX_PRODUCTID_CHAR_VAL,

    MI_IDX_VER_CHAR,
    MI_IDX_VER_CHAR_VAL,

    MI_IDX_WIFI_CFG_CHAR,
    MI_IDX_WIFI_CFG_CHAR_VAL,
    MI_IDX_WIFI_CFG_CLIENT_CFG,

    MI_IDX_CTRL_CHAR,
    MI_IDX_CTRL_CHAR_VAL,
    MI_IDX_CTRL_CHAR_CLIENT_CFG,

    MI_IDX_AUTHEN_CHAR,
    MI_IDX_AUTHEN_CHAR_VAL,
    MI_IDX_AUTHEN_CHAR_CLIENT_CFG,

    MI_IDX_NB,
};

/* Exported variables --------------------------------------------------------*/

extern const esp_gatts_attr_db_t mi_attr_db[MI_IDX_NB];

#endif
