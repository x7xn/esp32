/**
 * @file miio_handshake_rpc.h
 * @author xusongsong (xusongsong@xiaomi.com)
 * @brief 
 * @version 0.1
 * @date 2020-06-02
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef __MIIO_HANDSHAKE_RPC_H__
#define __MIIO_HANDSHAKE_RPC_H__

#include "miio_ciphers.h"

typedef enum{
	MIIO_ECDH_MODE_0 = 0, //无挑战认证
	MIIO_ECDH_MODE_1, //签名挑战认证
	MIIO_ECDH_MODE_MAX
}miio_ecdh_mode_t;

/**
 * @brief miio_ecdh_mode_disable
 * 
 * @param mode 
 */
void miio_ecdh_mode_enable(miio_ecdh_mode_t mode);

/**
 * @brief miio_ecdh_mode_disable
 * 
 * @param mode 
 */
void miio_ecdh_mode_disable(miio_ecdh_mode_t mode);

/**
 * @brief miio_ecdh_ext_handle_get
 * 
 * @return miio_ecdh_handle_t 
 */
miio_ecdh_handle_t miio_ecdh_ext_handle_get();

void miio_ecdh_ext_handle_free();

#endif
