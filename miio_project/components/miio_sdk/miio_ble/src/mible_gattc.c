/* Includes ------------------------------------------------------------------*/

#include "mible_gattc.h"
#include "mible_master.h"

/* Exported functions --------------------------------------------------------*/

int mible_gattc_init(void)
{
    return mible_master_init();
}

int mible_gattc_found_handler(mible_gattc_found_t * p_found)
{
    return mible_master_found(p_found->conn_handle);
}

int mible_gattc_write_handler(mible_gattc_write_t * p_write)
{
    return mible_master_write(p_write->conn_handle, p_write->handle);
}

int mible_gattc_notify_handler(mible_gattc_notify_t * p_notify)
{
    return mible_master_notify(p_notify->conn_handle, p_notify->handle,
                               p_notify->data, p_notify->len);
}

int mible_gattc_read_handler(mible_gattc_read_t * p_read)
{
    return mible_master_read(p_read->conn_handle, p_read->handle,
                             p_read->data, p_read->len);
}


