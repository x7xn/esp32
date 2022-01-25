#ifndef __MIBEACON_DEF_H__
#define __MIBEACON_DEF_H__

#include <stdint.h>
#include <stdbool.h>

#define MIBEACON_TIME_MASK                                  0x0001
#define MIBEACON_ENCRYPT_MASK                               0x0008
#define MIBEACON_MAC_MASK                                   0x0010
#define MIBEACON_CAPABILITY_MASK                            0x0020
#define MIBEACON_OBJECT_MASK                                0x0040
#define MIBEACON_MESH_MASK                                  0x0080
#define MIBEACON_CONFIRM_MASK                               0x0200
#define MIBEACON_AUTH_MASK                                  0x0400
#define MIBEACON_LOGIN_MASK                                 0x0800
#define MIBEACON_VERSION_MASK                               0xF000

#define MIBEACON_VERSION_OFFSET                             12

#define CAPABILITY_BOND_MASK                                0x18
#define CAPABILITY_IO_MASK                                  0x20

#define CAPABILITY_BOND_OFFSET                              3

#define BOND_TYPE_NONE                                      0
#define BOND_TYPE_PRE                                       1
#define BOND_TYPE_POST                                      2
#define BOND_TYPE_COMBO                                     3

#define MAX_OBJECT_ID_IN_PID                                7
#define MAX_MULTI_OBJECT_LEN                                31
#define MAX_OBJECT_IN_BEACON                                4
#define MAX_DATA_LEN_IN_OBJECT                              10

#define MIBEACON_DID_LEN                                    21
#define MIBEACON_TOKEN_LEN                                  12
#define MIBEACON_BEACONKEY_LEN                              16
#define MIBEACON_NONCE_LEN                                  13

#define MIBEACON_UNKNOWN_TTL                                300
#define MIBEACON_IN_ADVANCE_TTL                             60

#define MAX_AUTHED_DEV_LIST_COUNT                           50
#define MAX_UNKNOWN_DEV_LIST_COUNT                          255
#define MAX_DENIED_DEV_LIST_COUNT                           50
#define MAX_MIBAND_DEV_LIST_COUNT                           10
#define MAX_KEEP_ALIVE_DEV_LIST_COUNT                       50
#define MAX_RPC_UPLOAD_LIST_COUNT                           50

#define MAX_OBJECT_RULE_LIST_COUNT                          100
#define MAX_PID_OBJ_ID_LIST_COUNT                           50
#define MAX_UNKNOWN_PID_LIST_COUNT                          255

#define KEEP_ALIVE_DEFAULT_INTERVAL                         1200
#define KEEP_ALIVE_DEFAULT_DELTA                            300

#define MIBEACON_ONE_SECOND_MS                              1000
#define MIBEACON_RPC_UPLOAD_MS                              500

#define MIBEACON_EVENT_ERROR_GATE                           100

#define MIBLE_BAND_MAX                                      10
#define MIBLE_BAND_TIMEOUT                                  90
#define MIBLE_BAND_INVALID_STATE                            0xFF
#define MIBLE_BAND_INVALID_STEP                             0xFFFF
#define MIBLE_BAND_INVALID_EXTENDED_DATA                    0xFFFFFFFF

#define MIBLE_BAND_PID                                      21

#define UNUSED(expr)            do { (void)(expr); } while (0)

struct _mibeacon {
    uint16_t frame_ctrl;
    uint16_t product_id;
    uint8_t counter;
    uint8_t payload[0];
} __attribute__((packed));
typedef struct _mibeacon mibeacon_t;

typedef struct {
    uint16_t frame_ctrl;
    uint8_t version;
    uint16_t product_id;
    int8_t rssi;
    uint8_t mac[6];
    uint8_t capability;
    uint8_t bond_type;
    uint8_t wifi_mac[2];
    uint16_t io_capability;
    uint16_t mesh_cfg;
    uint32_t counter;
    uint8_t encrypted;
    uint32_t mic;
    uint32_t timestamp;
    uint8_t object_len;
    uint8_t object[MAX_MULTI_OBJECT_LEN];
} mibeacon_info_t;

typedef struct {
    uint16_t id;
    uint8_t data_len;
    uint8_t data[MAX_DATA_LEN_IN_OBJECT];
    uint32_t timestamp;
} mibeacon_object_t;

typedef struct {
    uint8_t num;
    mibeacon_object_t object[MAX_OBJECT_IN_BEACON];
} mibeacon_object_array_t;

typedef struct {
    uint16_t frame_ctrl;
    uint16_t product_id;
    uint32_t counter;
    uint32_t mic;
    uint8_t mac[6];
    uint8_t key[MIBEACON_BEACONKEY_LEN];
} mibeacon_decrypt_config_t;

#endif
