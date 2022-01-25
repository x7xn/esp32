/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MIBLE_COMMAND_H__
#define __MIBLE_COMMAND_H__

/* Includes ------------------------------------------------------------------*/

#include "util.h"

/* Exported functions --------------------------------------------------------*/

int mible_cmd_init(void);
int mible_cmd_put(char *buf);
int mible_cmd_get(char *buf, uint32_t buf_size);

#endif
