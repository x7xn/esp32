/* Includes ------------------------------------------------------------------*/

#include "mible_gatts.h"
#include "mible_slave.h"

/* Exported functions --------------------------------------------------------*/

int mible_gatts_init(void)
{
    return mible_slave_init();
}

int mible_gatts_write_handler(mible_gatts_write_t *p_write)
{
    return mible_slave_write(p_write->handle, &p_write->value[0], p_write->length);
}


