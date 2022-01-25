#ifndef __OBJECT_DELTA_H__
#define __OBJECT_DELTA_H__

#include "mibeacon_def.h"

#define OBJECT_ID_EVT_GENERAL                               0x0001
#define OBJECT_ID_EVT_PAIR                                  0x0002
#define OBJECT_ID_EVT_NEAR                                  0x0003
#define OBJECT_ID_EVT_FARAWAY                               0x0004
#define OBJECT_ID_EVT_LOCK                                  0x0005
#define OBJECT_ID_EVT_FINGER                                0x0006
#define OBJECT_ID_EVT_DOOR                                  0x0007
#define OBJECT_ID_EVT_DEPLOY                                0x0008
#define OBJECT_ID_EVT_GESTURE                               0x0009
#define OBJECT_ID_EVT_LOCK_NEW                              0x000B
#define OBJECT_ID_EVT_BUTTON                                0x1001

#define OBJECT_ID_ATT_SLEEP                                 0x1002
#define OBJECT_ID_ATT_RSSI                                  0x1003
#define OBJECT_ID_ATT_TEMP                                  0x1004
#define OBJECT_ID_ATT_BOIL                                  0x1005
#define OBJECT_ID_ATT_HUMIDITY                              0x1006
#define OBJECT_ID_ATT_LIGHT                                 0x1007
#define OBJECT_ID_ATT_SOILTMEP                              0x1008
#define OBJECT_ID_ATT_SOILEC                                0x1009
#define OBJECT_ID_ATT_BATTERY                               0x100A
#define OBJECT_ID_ATT_LOCK                                  0x100E
#define OBJECT_ID_ATT_DOOR                                  0x100F
#define OBJECT_ID_ATT_HCHO                                  0x1010
#define OBJECT_ID_ATT_BOND                                  0x1011

#define OBJECT_ID_ATT_MMC_TEMP                              0x2000
#define OBJECT_ID_ATT_HM_BAND                               0x2001
#define OBJECT_ID_ATT_RM_CLEANER                            0x2002
#define OBJECT_ID_ATT_RY_BAND                               0x2003

#define OBJECT_ID_EVT_HHCC_MONITOR                          0x3000
#define OBJECT_ID_EVT_QP_LOCATION                           0x3001

bool mibeacon_check_object_delta(mibeacon_object_t *p_old, mibeacon_object_t *p_new,
                                 uint32_t delta);

#endif
