#ifndef __LETS_DEV_PUBLISH_H
#define __LETS_DEV_PUBLISH_H

#include "stdint.h"
#include "miio_instance.h"
#include "miio_arch.h"
#include "jsmi.h"
/*
 * 通过蓝牙广播方式发送设备版本号
 *
 * */

void Task_DevNamePublish_Init();
void DevName_SendImm();

#endif
