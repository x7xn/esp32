/*
 *  FIPS-180-2 compliant SHA-256 implementation
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */
/*
 *  The SHA-256 Secure Hash Standard was published by NIST in 2002.
 *
 *  http://csrc.nist.gov/publications/fips/fips180-2/fips180-2.pdf
 */

#include "mi_crypto_backend_mbedtls.h"
#include "arch_api.h"
#include "arch_os.h"

#include "mible_port.h"
#define MI_LOG_MODULE_NAME "MBEDTLS"

#undef  MIBLE_LOG_TAG
#define MIBLE_LOG_TAG "mi_crypto"

#include "mbedtls/md.h"
#include "mbedtls/ccm.h"


unsigned int mbedtls_sha256_hkdf(const unsigned char *key, unsigned int key_len,
                                const unsigned char *salt, unsigned int salt_len,
                                const unsigned char *info, unsigned int info_len,
                                unsigned char *out, unsigned int out_len)
{
    const unsigned char null_salt[32] = {0};
    unsigned char PRK[32];
    unsigned char T_n[32];
    unsigned int loop;
    unsigned int temp_len;

    // Step 1: HKDF-Extract(salt, IKM) -> PRK
    if (salt == NULL)
        mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), null_salt, 32, key, key_len, PRK);
    else
        mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), salt, salt_len, key, key_len, PRK);

    // Step 2: HKDF-Expand(PRK, info, L) -> OKM
    //T(0) = empty string (zero length)
    //T(1) = HMAC-Hash(PRK, T(0) | info | 0x01)
    //T(2) = HMAC-Hash(PRK, T(1) | info | 0x02)
    //T(3) = HMAC-Hash(PRK, T(2) | info | 0x03)

    unsigned char temp[32 + info_len + 1];
    memset(temp, 0, 32 + info_len +1);
    loop = CEIL_DIV(out_len, 32);

    int i;
    for (i = 0; i < loop ; i++) {
        if (i == 0) {
            temp_len = 0;
        } else {
            memcpy(temp, T_n, 32);
            temp_len = 32;
        }

        memcpy(temp + temp_len, info, info_len);
        temp_len += info_len;

        temp[temp_len] = i + 1;
        temp_len += 1;

        mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), PRK, 32, temp, temp_len, T_n);

        memcpy(out + 32 * i, T_n, MIN(32, out_len));
        out_len -= 32;
    }

    return 0;
}

unsigned int mbedtls_hmac_sha256(const uint8_t *k, uint32_t key_len, const uint8_t *p_in, uint32_t in_len, uint8_t *p_out)
{
    return mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), k, key_len, p_in, in_len, p_out);
}

unsigned int mbedtls_crypto_ccm_decrypt(const uint8_t *key,
                        const uint8_t *iv, size_t iv_len,
                        const uint8_t *add, size_t add_len,
                        const uint8_t *in, size_t in_len,
                        uint8_t *out,
                        const uint8_t *tag, size_t tag_len)
{
    mbedtls_ccm_context ctx;
    int ret = -1;
    
    mbedtls_ccm_init( &ctx );

    //LOG_DEBUG_TAG(MIBLE_LOG_TAG, "sizeof key = %d !!", sizeof key);

    if( mbedtls_ccm_setkey( &ctx, MBEDTLS_CIPHER_ID_AES, key, 8 * 16 ) != 0 )
    {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "mbedtls_ccm_setkey error!!");

        mbedtls_ccm_free( &ctx );        
        return( 1 );
    }

    ret = mbedtls_ccm_auth_decrypt( &ctx, in_len,
                                iv, iv_len, add, add_len,
                                in, out,
                                tag, tag_len );
                                
    mbedtls_ccm_free( &ctx );

    return ret;
}
