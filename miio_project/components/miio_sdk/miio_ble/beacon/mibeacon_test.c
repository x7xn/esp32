#include "mibeacon_test.h"
#include "mible_keep_alive.h"
#include "mibeacon_parse.h"
#include "mible_dev.h"
#include "mible_rpc.h"
#include "object_rule.h"
#include "mible_def.h"

#undef  MIBLE_LOG_TAG
#define MIBLE_LOG_TAG                           "mible_beacon"
#undef  LOG_LEVEL
#define LOG_LEVEL                               LOG_LEVEL_INFO

// test case
// 1. authed_dev_add authed_dev_delete MAX_AUTHED_DEV_LIST_COUNT duplicate_add
// 2. mible_dev.c and object_rule.c
// 3. denied_dev_search, unknown_dev_search and authed_dev_search
// 4. get objects from beacon
// 5. check report interval
// 6. deal with same objects

void mibeacon_device_set_test(void)
{
    // 1. get authed device (ttl) from server
    authed_dev_t dev1;
    memset((uint8_t *)&dev1, 0, sizeof(authed_dev_t));
    for (int i = 0; i < 6; i++) {
        dev1.mac[i] = 0x01;
    }
    dev1.pid = 1;
    dev1.report_num = 1;
    dev1.report[0].id = 1;
    dev1.report[0].timestamp = 1;
    // dev1.report[1].id = 2;
    // dev1.report[1].timestamp = 1;
    dev1.ttl = 0x0F;

    authed_dev_t dev2;
    memset((uint8_t *)&dev2, 0, sizeof(authed_dev_t));
    for (int i = 0; i < 6; i++) {
        dev2.mac[i] = 0x02;
    }
    dev2.pid = 2;
    dev2.report_num = 0;

    authed_dev_t dev2_dup;
    memset((uint8_t *)&dev2_dup, 0, sizeof(authed_dev_t));
    for (int i = 0; i < 6; i++) {
        dev2_dup.mac[i] = 0x02;
    }
    dev2_dup.pid = 2;
    dev2_dup.report_num = 0;

    authed_dev_add(&dev1);
    authed_dev_add(&dev2);
    authed_dev_add(&dev2_dup);
    authed_dev_print();
    // authed_dev_delete(dev1.mac, dev1.pid);
    // authed_dev_print();
    // authed_dev_delete(dev2.mac, dev2.pid);
    // authed_dev_print();
    // authed_dev_add(&dev2_dup);
    // authed_dev_print();

    // unknown_dev_t unknown_dev;
    // memset((uint8_t *)&unknown_dev, 0, sizeof(unknown_dev_t));
    // for (int i = 0; i < 6; i++) {
    //     unknown_dev.mac[i] = 0x01;
    // }
    // unknown_dev.pid = 1;
    // unknown_dev.ttl = 0x0F;
    // unknown_dev_add(&unknown_dev);

    // denied_dev_t denied_dev;
    // memset((uint8_t *)&denied_dev, 0, sizeof(denied_dev_t));
    // for (int i = 0; i < 6; i++) {
    //     denied_dev.mac[i] = 0x02;
    // }
    // denied_dev.pid = 2;
    // denied_dev.ttl = 0x0A;
    // denied_dev_add(&denied_dev);

    // 2. get pid obj_id relation from server
    pid_obj_id_t pid1;
    memset(&pid1, 0, sizeof(pid_obj_id_t));
    pid1.pid = 1;
    pid1.obj_num = 2;
    pid1.obj_id[0] = 1;
    pid1.obj_id[1] = 2;
    pid1.ttl = 0x0A;

    pid_obj_id_t pid2;
    memset(&pid2, 0, sizeof(pid_obj_id_t));
    pid2.pid = 2;
    pid2.obj_num = 2;
    pid2.obj_id[0] = 3;
    pid2.obj_id[1] = 4;
    pid2.ttl = 0x0A;

    pid_obj_id_add(&pid1);
    pid_obj_id_add(&pid2);
    pid_obj_id_print();

    // 3. get rules (ttl) from server
    object_rule_t rule1;
    memset(&rule1, 0, sizeof(object_rule_t));
    rule1.obj_id = 1;
    rule1.interval = 0x02;
    rule1.delta = 0x0F;

    object_rule_t rule2;
    memset(&rule2, 0, sizeof(object_rule_t));
    rule2.obj_id = 2;
    rule2.interval = 0x02;
    rule2.delta = 0x0F;

    object_rule_add(&rule1);
    object_rule_add(&rule2);
    object_rule_print();
}

void mibeacon_upload_test(void)
{
    mibeacon_info_t info;
    memset(&info, 0, sizeof(mibeacon_info_t));
    info.version = 5;
    info.product_id = 1;
    info.timestamp = arch_os_time_now();
    info.counter = 2;
    for (int i = 0; i < 6; i++) {
        info.mac[i] = 0x01;
    }
    info.encrypted = 0;
    info.object_len = 10;
    info.object[0] = 0x01;
    info.object[1] = 0x00;
    info.object[2] = 0x02;
    info.object[3] = 0x01;
    info.object[4] = 0x02;
    info.object[5] = 0x02;
    info.object[6] = 0x00;
    info.object[7] = 0x02;
    info.object[8] = 0x03;
    info.object[9] = 0x04;
    mibeacon_upload(&info);
}

void mibeacon_parse_test(void)
{
    mible_beacon_t beacon;
    memset((uint8_t *)&beacon, 0, sizeof(mible_beacon_t));
    for (int i = 0; i < 6; i++) {
        beacon.address[i] = i;
    }
    beacon.rssi = -35;
    beacon.len = 28;

    // 15   14    13    12  |  11      10      9      8  |   7     6      5      4   |  3    2  1  0
    // version              |  sec     sec     bind      |   mesh  obj    cap    mac |  enc

    beacon.data[0] = 0x70;  // 0111 0000
    beacon.data[1] = 0x20; // 0101 0000
    beacon.data[2] = 0xCF;
    beacon.data[3] = 0x01; // product id
    beacon.data[4] = 0x0A; // frame count
    beacon.data[5] = 0xFF;
    beacon.data[6] = 0xEE;
    beacon.data[7] = 0xDD;
    beacon.data[8] = 0xCC;
    beacon.data[9] = 0xBB;
    beacon.data[10] = 0xAA;
    beacon.data[11] = 0x1D; // cap
    beacon.data[12] = 0x88; // wifi mac
    beacon.data[13] = 0x99; // wifi mac
    beacon.data[14] = 0x01; // obj id
    beacon.data[15] = 0x02; // obj id
    beacon.data[16] = 0x02; // obj len
    beacon.data[17] = 0x0A;
    beacon.data[18] = 0x0B; // obj data
    beacon.data[19] = 0x03; // obj id
    beacon.data[20] = 0x04; // obj id
    beacon.data[21] = 0x02; // obj len
    beacon.data[22] = 0x0C;
    beacon.data[23] = 0x0D; // obj data
    beacon.data[24] = 0x01; // count
    beacon.data[25] = 0x02; // count
    beacon.data[26] = 0x03; // count
    beacon.data[27] = 0x0A; // mic

    // beacon.data[0] = 0x70;  // 0111 0000
    // beacon.data[1] = 0x50;  // 0101 0000
    // beacon.data[2] = 0xCF;
    // beacon.data[3] = 0x01;  // product id
    // beacon.data[4] = 0x0A;  // frame count
    // beacon.data[5] = 0xFF;
    // beacon.data[6] = 0xEE;
    // beacon.data[7] = 0xDD;
    // beacon.data[8] = 0xCC;
    // beacon.data[9] = 0xBB;
    // beacon.data[10] = 0xAA;
    // beacon.data[11] = 0x3D;  // cap
    // beacon.data[12] = 0x88;  // wifi mac
    // beacon.data[13] = 0x99;  // wifi mac
    // beacon.data[14] = 0x02;  // io
    // beacon.data[15] = 0x03;  // io
    // beacon.data[16] = 0x01;  // obj id
    // beacon.data[17] = 0x02;  // obj id
    // beacon.data[18] = 0x02;  // obj len
    // beacon.data[19] = 0x0A;
    // beacon.data[20] = 0x0B;  // obj data
    // beacon.data[21] = 0x01;  // count
    // beacon.data[22] = 0x02;  // count
    // beacon.data[23] = 0x03;  // count
    // beacon.data[24] = 0x0A;  // mic
    // beacon.data[25] = 0x0B;  // mic
    // beacon.data[26] = 0x0C;  // mic
    // beacon.data[27] = 0x0D;  // mic

    mibeacon_info_t info;
    memset((uint8_t *)&info, 0, sizeof(mibeacon_info_t));

    mibeacon_parse((mibeacon_t *)beacon.data, beacon.len, &info);
}

void miband_set(void)
{
    // 1. get authed device (ttl) from server
    authed_dev_t dev1;
    memset((uint8_t *)&dev1, 0, sizeof(authed_dev_t));
    for (int i = 0; i < 6; i++) {
        dev1.mac[i] = i;
    }
    dev1.pid = 21;
    dev1.report_num = 0;
    dev1.ttl = 0x0F;
    authed_dev_add(&dev1);

    // 2. get pid obj_id relation from server
    pid_obj_id_t pid1;
    memset(&pid1, 0, sizeof(pid_obj_id_t));
    pid1.pid = 21;
    pid1.obj_num = 1;
    pid1.obj_id[0] = 0x2001;
    pid1.ttl = 0x0F;
    pid_obj_id_add(&pid1);

    // 3. get rules (ttl) from server
    object_rule_t rule1;
    memset(&rule1, 0, sizeof(object_rule_t));
    rule1.obj_id = 0x2001;
    rule1.interval = 0x02;
    rule1.delta = 0x02;
    object_rule_add(&rule1);
}

void miband_test(void)
{
    // band manu 57 01 00 87 21 b3 fe c9 a4 8b 9d 17 63 dd 5b f9 8e f3 a4 01 ec 2e 28 35 3c 38
    uint8_t mac[6];
    for (int i = 0; i < 6; i++) {
        mac[i] = i;
    }

    uint8_t adv_data[9];
    adv_data[0] = 0x02;
    adv_data[1] = 0x01;
    adv_data[2] = 0x06;

    adv_data[3] = 0x05;  // len
    adv_data[4] = 0x16;
    adv_data[5] = 0xe0;
    adv_data[6] = 0xfe;
    adv_data[7] = 0x01;
    adv_data[8] = 0x02;

    // uint8_t adv_data[10];
    // adv_data[0] = 0x02;
    // adv_data[1] = 0x01;
    // adv_data[2] = 0x06;

    // adv_data[3] = 0x06; //len
    // adv_data[4] = 0xff;
    // adv_data[5] = 0x57;
    // adv_data[6] = 0x01;
    // adv_data[7] = 0x01;
    // adv_data[8] = 0x00;
    // adv_data[9] = 0x01;

    // adv_data[10] = 0x03; // len
    // adv_data[11] = 0x16;
    // adv_data[12] = 0x1D;
    // adv_data[13] = 0x18;

    ble_adv_parse(mac, adv_data, sizeof(adv_data), -35);
}

#include <tinycrypt/ccm_mode.h>
#include <tinycrypt/constants.h>

static uint8_t aes_key[] = { 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
                             0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f };

static uint8_t nonce[] = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c };

static uint8_t adddata[] = { 0x11 };

static uint8_t plain[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                           0x99, 0x00, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

static uint8_t cipher[] = { 0x58, 0x92, 0x4c, 0xca, 0x6f, 0xc7, 0x97, 0x98, 0xdb,
                            0x41, 0x42, 0x06, 0x9e, 0xbd, 0x6b, 0xb1, 0x64 };

void mibeacon_aes_ccm_test(void)
{
    int ret;
    struct tc_ccm_mode_struct mode;
    uint8_t encrypted[sizeof(cipher)];
    uint8_t decrypted[sizeof(plain)];

    ret = tc_ccm_config(&mode, aes_key, nonce, sizeof(nonce), 1);
    if (TC_CRYPTO_SUCCESS != ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "%s -- fail to config", __func__);
    }

    ret = tc_ccm_generation_encryption(encrypted, sizeof(encrypted), adddata, sizeof(adddata), plain, sizeof(plain),
                                       &mode);
    if (TC_CRYPTO_SUCCESS != ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "%s -- fail to encrypt", __func__);
    }
    LOG_INFO_TAG(MIBLE_LOG_TAG, "%s -- encrypt %s", __func__,
                 (0 == memcmp(encrypted, cipher, sizeof(cipher))) ? "succ" : "fail");

    ret = tc_ccm_decryption_verification(decrypted, sizeof(decrypted), adddata, sizeof(adddata), encrypted,
                                         sizeof(encrypted), &mode);
    if (TC_CRYPTO_SUCCESS != ret) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "%s -- fail to decrypt", __func__);
    }
    LOG_INFO_TAG(MIBLE_LOG_TAG, "%s -- compare %s", __func__,
                 (0 == memcmp(decrypted, plain, sizeof(plain))) ? "succ" : "fail");
}
