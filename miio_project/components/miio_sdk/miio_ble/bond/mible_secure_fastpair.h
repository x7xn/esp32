#ifndef __MIBLE_STD_FASTPAIR_H__
#define __MIBLE_STD_FASTPAIR_H__
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SCHD_IDLE                      0

#define REG_TYPE                       0x10UL
#define REG_START                      (REG_TYPE)
#define REG_SUCCESS                    (REG_TYPE+1)
#define REG_FAILED                     (REG_TYPE+2)
#define REG_VERIFY_SUCC                (REG_TYPE+3)
#define REG_VERIFY_FAIL                (REG_TYPE+4)
#define REG_START_WO_PKI               (REG_TYPE+5)

#define ERR_TYPE                       0xE0UL
#define ERR_NOT_REGISTERED             (ERR_TYPE)
#define ERR_REGISTERED                 (ERR_TYPE+1)
#define ERR_REPEAT_LOGIN               (ERR_TYPE+2)
#define ERR_INVALID_OOB                (ERR_TYPE+3)

typedef enum {
    UNAUTHORIZATION = 0,
    ADMIN_AUTHORIZATION,
    SHARE_AUTHORIZATION
} mi_author_stat_t;

typedef enum {
    SCHD_EVT_REG_SUCCESS                    = 0x01,
    SCHD_EVT_REG_FAILED                     = 0x02,
    SCHD_EVT_ADMIN_LOGIN_SUCCESS            = 0x03,
    SCHD_EVT_ADMIN_LOGIN_FAILED             = 0x04,
    SCHD_EVT_SHARE_LOGIN_SUCCESS            = 0x05,
    SCHD_EVT_SHARE_LOGIN_FAILED             = 0x06,
    SCHD_EVT_TIMEOUT                        = 0x07,
    SCHD_EVT_KEY_NOT_FOUND                  = 0x08,
    SCHD_EVT_KEY_FOUND                      = 0x09,
    SCHD_EVT_KEY_DEL_FAIL                   = 0x0A,
    SCHD_EVT_KEY_DEL_SUCC                   = 0x0B,
    SCHD_EVT_MESH_REG_SUCCESS               = 0x0C,
    SCHD_EVT_MESH_REG_FAILED                = 0x0D,
    SCHD_EVT_OOB_REQUEST                    = 0x0E,
    SCHD_EVT_MSC_SELF_TEST_PASS             = 0x0F,
    SCHD_EVT_MSC_SELF_TEST_FAIL             = 0x10,
    SCHD_EVT_INTERNAL                       = 0x11
} schd_evt_id_t;


void fastpair_auth_setup(uint16_t conn_handle, uint16_t srv_handle, uint16_t auth_handle);

void fastpair_auth_opcode_process(uint8_t *pdata, uint8_t len, uint16_t conn_handle);

void fastpair_auth_data_process(uint8_t *pdata, uint16_t len);

uint32_t fastpair_scheduler_init(uint32_t interval);

uint32_t fastpair_scheduler_uninit(void);

uint32_t fastpair_scheduler_start(uint32_t procedure);

uint32_t fastpair_scheduler_stop(void);

void fastpair_schd_process(void);

#ifdef __cplusplus
}
#endif

/** @} */

#endif  /* __MIBLE_STD_FASTPAIR_H__ */
