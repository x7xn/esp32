/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TOOL_BOX_H__
#define __TOOL_BOX_H__

/* Includes ------------------------------------------------------------------*/

#include "mible_def.h"

/* Exported functions --------------------------------------------------------*/

void tool_mix_1(uint8_t * in, uint8_t * mmac, uint8_t * smac,
                    uint8_t * pid, uint8_t * out);
void tool_mix_2(uint8_t * in, uint8_t * mmac, uint8_t * smac,
                    uint8_t * pid, uint8_t * out);

void tool_encrypt(uint8_t * in, int inLen, uint8_t * key,
                    int keyLen, uint8_t * out);
void tool_decrypt(uint8_t * in, int inLen, uint8_t * key,
                    int keyLen, uint8_t * out);

#endif
