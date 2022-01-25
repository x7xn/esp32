/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MIBLE_BAND_H__
#define __MIBLE_BAND_H__

#include "mible_gap.h"

int mible_band_init(void);
int mible_band_parse(mible_addr_t address, uint8_t *p_adv_data, uint16_t adv_size, int8_t rssi);
int mible_band_handler(mible_band_t *p_band);
int mible_band_refresh(void);
int mible_band_set(uint32_t delta);
int mible_band_rpc_delegate_ack(miio_rpc_delegate_arg_t *req_arg, miio_fp_rpc_delegate_ack_t ack,
                            void *ctx);

#endif
