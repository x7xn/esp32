/**
* @file    miio_arch.h
* @author  mashaoze
* @date    2017
* @par     Copyright (c):
*
*    Copyright 2017 MIoT,MI
*
*    Licensed under the Apache License, Version 2.0 (the "License");
*    you may not use this file except in compliance with the License.
*    You may obtain a copy of the License at
*
*        http://www.apache.org/licenses/LICENSE-2.0
*
*    Unless required by applicable law or agreed to in writing, software
*    distributed under the License is distributed on an "AS IS" BASIS,
*    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*    See the License for the specific language governing permissions and
*    limitations under the License.
*/

#ifndef __MIIO_ARCH_H__
#define __MIIO_ARCH_H__

#define NO_OPTIMIZE 		__attribute__((optimize("O0")))

#ifndef WEAK
#define WEAK				__attribute__((weak))
#endif  /* WEAK */

#ifndef PACKED
#define PACKED				__attribute__((__packed__))
#endif

#ifndef PACK_STRUCT_FIELD

#if defined(__CC_ARM)   /* ARMCC compiler */

#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT __attribute__ ((__packed__))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

#elif defined(__ICCARM__)   /* IAR Compiler */

#define PACK_STRUCT_BEGIN __packed
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_USE_INCLUDES

#elif defined(__GNUC__)     /* GNU GCC Compiler */

#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

#else

#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

#endif

#endif


#include "misc/util.h"
#include "misc/soft_crc.h"
#include "esp32/arch_dbg.h"
#include "esp32/arch_chip.h"
#include "esp32/arch_crypto.h"
#include "esp32/arch_flash.h"
#include "esp32/arch_serial.h"
#include "esp32/arch_time.h"
#include "esp32/arch_psm.h"
#include "esp32/arch_net.h"
#include "esp32/arch_os.h"
#include "esp32/arch_ota.h"
#include "esp32/arch_crash.h"

int arch_init(void);

#endif /* __MIIO_ARCH_H__ */

