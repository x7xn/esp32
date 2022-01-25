/*
 * light_scen.h
 *
 *  Created on: 2019Äê4ÔÂ18ÈÕ
 *      Author: Administrator
 */

#ifndef _LIGHT_SCEN_H_
#define _LIGHT_SCEN_H_

#include "stdint.h"
#include "miio_instance.h"
#include "miio_arch.h"
#include "jsmi.h"
#include "../../esp-idf/components/driver/include/driver/ledc.h"
#include "../../esp-idf/components/driver/include/driver/gpio.h"
#include "../../esp-idf/components/driver/include/driver/pcnt.h"


#pragma pack(1)

#define SCNE_MAXSIZE		16

#define SCEN_BIT_LIGHTSW	0x01
#define SCEN_BIT_HEATSW		0x02

typedef struct _scen_data_t
{
	uint8_t  use;
	uint8_t  next;

	uint8_t	 scenum;
	uint8_t	 lid;
	uint32_t rid;

	uint8_t  bdata;
	uint16_t brightness;
	uint16_t templater;
	uint8_t  rcolor;
	uint8_t  gcolor;
	uint8_t  bcolor;
	uint8_t  rgbmode;
} scen_data_t;
#pragma pack()

void light_scne_init(void);
void light_scne_save(uint8_t lid, uint32_t rid,uint8_t scen);
void light_scne_read(uint8_t lid, uint32_t rid,uint8_t scen,uint8_t onoff);

#endif /* _LIGHT_SCEN_H_ */
