#include "mible_rpc_test.h"
#include "adv_beacon.h"
#include "mible_dev.h"
#include "mible_keep_alive.h"
#include "mible_rpc.h"
#include "mible_def.h"

int mible_rpc_test(void)
{
    int ret = MIIO_OK;
/*
    authed_dev_t dev;
    memset((uint8_t *)&dev, 0, sizeof(authed_dev_t));

    dev.pid = 1;
    for(int i = 0; i < 6; i++) {
        dev.mac[i] = i;
    }

    mible_rpc_query_dev(dev.mac, dev.pid);

    pid_obj_id_t pid_obj_id;
    memset((uint8_t *)&pid_obj_id, 0, sizeof(pid_obj_id));

    pid_obj_id.pid = 1;
    mible_rpc_query_prod(pid_obj_id.pid);
*/
/*
    authed_dev_t dev;
    mibeacon_object_array_t object_array;

    memset((uint8_t *)&dev, 0, sizeof(authed_dev_t));
    memset((uint8_t *)&object_array, 0, sizeof(mibeacon_object_array_t));

    dev.pid = 1;
    char *s = "12345678901234567890";
    memcpy((uint8_t *)&dev.did, (uint8_t *)s, strlen(s) + 1);
    for(int i = 0; i < 6; i++) {
        dev.mac[i] = i;
    }
    dev.counter = 123;

    object_array.num = MAX_OBJECT_IN_BEACON;

    for (int i = 0; i < object_array.num; i++) {
        object_array.object[i].id = i;
        object_array.object[i].data_len = 3;
        object_array.object[i].data[0] = 1;
        object_array.object[i].data[1] = 2;
        object_array.object[i].data[2] = 3;
    }

    mible_rpc_report_evt(&dev, &object_array);
*/
/*
    for (int i = 0; i < 10; i ++) {
        char *s1 = "12345678901234567890";
        keep_alive_dev_t dev1;
        memset((uint8_t *)&dev1, 0, sizeof(keep_alive_dev_t));
        dev1.pid = i;
        memcpy((uint8_t *)&dev1.did, (uint8_t *)s1, strlen(s1) + 1);
        dev1.rssi = -30;
        dev1.timestamp = arch_os_time_now();
        keep_alive_dev_add(&dev1);
    }
    for (int i = 0; i < 6; i ++) {
        char *s2 = "87654321";
        keep_alive_dev_t dev2;
        memset((uint8_t *)&dev2, 0, sizeof(keep_alive_dev_t));
        dev2.pid = i + 20;
        memcpy((uint8_t *)&dev2.did, (uint8_t *)s2, strlen(s2) + 1);
        dev2.rssi = -40;
        dev2.timestamp = arch_os_time_now();
        keep_alive_dev_add(&dev2);
    }
    keep_alive_dev_print();
    mible_rpc_keep_alive();
*/

    // char *s = "{\"error\":{\"code\":-x,\"message\":{\"operation\":\"query_dev\",\"mac\":\"84:68:3E:00:7A:E3\",\"pdid\":157}}}";
    // char *s = "{\"result\":{\"operation\":\"query_dev\",\"mac\":\"84:68:3E:00:7A:E3\",\"ttl\":600,\"pdid\":157,\"did\":\"12345678901234567890\",\"token\":\"112233445566778899001122\",\"beaconkey\":\"00112233445566778899001122334455\"}}";
    // char *s = "{\"result\":{\"operation\":\"query_prod\",\"pdid\":156,\"ttl\":600,\"thr\":-40,\"upRule\":[{\"eid\":1234,\"intvl\":10,\"delta\":100},{\"eid\":4100,\"intvl\":5,\"delta\":100}]}}";
    // char *s = "{\"error\":{\"code\":-x,\"message\":{\"operation\":\"query_prod\",\"pdid\":156}}}";
    // char *s = "{\"result\":{\"operation\":\"keep_alive\",\"intvl\":1200,\"delta\":60,\"filter\":[{\"mac\":\"84:68:3E:00:7A:E3\",\"pdid\":156,\"ttl\":1800},{\"mac\":\"84:a8:34:04:71:46\",\"pdid\":156,\"ttl\":1800}]}}";
    // char *s = "{\"error\":{\"code\":-x,\"message\":\"keep_alive\"}}";

    // miio_rpc_delegate_arg_t ack;
    // ack.pload = s;
    // ack.pload_len = strlen(s);

    // mible_query_dev_cb(&ack, NULL);
    // mible_query_prod_cb(&ack, NULL);
    // mible_keep_alive_cb(&ack, NULL);

    return ret;
}

#include "mible_rpc.h"

void mible_rpc_add_test(void)
{
    // gateway_rpc_upload_init();

    rpc_upload_add("111", 0, MIBLE_RPC_QUERY_PROB);

    rpc_upload_print();

    rpc_upload_add("222", 0, MIBLE_RPC_QUERY_PROB);

    rpc_upload_print();

    rpc_upload_add("333", 0, MIBLE_RPC_REPORT_EVT);

    rpc_upload_print();

    rpc_upload_add("444", 0, MIBLE_RPC_QUERY_DEV);

    rpc_upload_print();

    rpc_upload_add("555", 0, MIBLE_RPC_REPORT_EVT);

    rpc_upload_print();
}
