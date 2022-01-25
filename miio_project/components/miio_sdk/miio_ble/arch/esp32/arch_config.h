#ifndef __ARCH_CONFIG_H__
#define __ARCH_CONFIG_H__

/* WiFi module is in other mode */
#define ARCH_SCAN_PARAM1_INTERVAL                         0x0020
#define ARCH_SCAN_PARAM1_WINDOW                           0x0020

/* WiFi module is in AP mode */
#define ARCH_SCAN_PARAM2_INTERVAL                         0x0064
#define ARCH_SCAN_PARAM2_WINDOW                           0x0042

/* WiFi module is connecting */
#define ARCH_SCAN_PARAM3_INTERVAL                         0x0064
#define ARCH_SCAN_PARAM3_WINDOW                           0x0028

#define ARCH_MAINLOOP_THREAD_PRIORITY                     1

#endif