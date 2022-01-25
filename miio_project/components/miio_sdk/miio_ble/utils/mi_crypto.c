#ifndef __MI_CRYPTO_C__
#define __MI_CRYPTO_C__
#include <stdint.h>

#include "arch_api.h"
#include "arch_os.h"

#include "mi_crypto.h"
#include "mi_crypto_backend_uECC.h"
#include "mi_crypto_backend_mbedtls.h"
#include "pt.h"

#undef  MIBLE_LOG_TAG
#define MIBLE_LOG_TAG "mi_crypto"


//static __ALIGN(4) ecc256_sk_t dev_crt_sk;
//static __ALIGN(4) ecc256_sk_t dev_sk;
static ecc256_sk_t dev_crt_sk;
static ecc256_sk_t dev_sk;

/**@brief Initialize the mi crypto module.
 * */
int mi_crypto_init(void)
{
    int tmp_errno = 0;

    micro_ecc_init(NULL);

    return tmp_errno;
}

/**@brief Create a public/private key pair.
 *
 * @param[out]  pk   Public key. Pointer must be aligned to a 4-byte boundary.
 *
 * @retval      3:  Key pair successfuly created.
 * @retval     -1:  NULL pointer provided.
 * @retval     -2:  Unaligned pointer provided.
 */
int mi_crypto_ecc_keypair_gen(ecc_curve_t curve, ecc256_pk_t pk)
{
    if (pk == 0)
    return -1;
	
    int ret = micro_ecc_keypair_gen(&curve, dev_sk, pk);
    //MI_ERR_CHECK(ret);
    return (ret == MI_SUCCESS ? PT_ENDED : ret);
}

/**@brief Create a public key from a provided private key.
 *
 * @param[in]   p_sk   Private key. Pointer must be aligned to a 4-byte boundary.
 * @param[out]  p_pk   Public key. Pointer must be aligned to a 4-byte boundary.
 *
 * @retval      3:  Public key successfuly created.
 * @retval     -1:  NULL pointer provided.
 * @retval     -2:  Unaligned pointer provided.
 */
int mi_crypto_ecc_pubkey_compute(ecc_curve_t curve, const ecc256_sk_t sk, ecc256_pk_t pk)
{
    if (sk == 0 || pk == 0)
        return -1;

    int ret = micro_ecc_public_key_compute(&curve, sk, pk);
    //MI_ERR_CHECK(ret);
    return (ret == MI_SUCCESS ? PT_ENDED : ret);
}

/**@brief Create a shared secret from a provided public/private key pair.
 *
 * @param[in]   p_pk   Public key. Pointer must be aligned to a 4-byte boundary.
 * @param[out]  p_ss   Shared secret. Pointer must be aligned to a 4-byte boundary.
 *
 * @retval      3:  Shared secret successfuly created.
 * @retval     -1:  NULL pointer provided.
 * @retval     -2:  Unaligned pointer provided.
 */
int mi_crypto_ecc_shared_secret_compute(ecc_curve_t curve, const ecc256_pk_t pk, ecc256_ss_t ss)
{
    if (pk == 0 || ss == 0)
        return -1;

    int ret = micro_ecc_shared_secret_compute(&curve, dev_sk, pk, ss);
    //MI_ERR_CHECK(ret);
    return (ret == MI_SUCCESS ? PT_ENDED : ret);
}

/**@brief Sign a hash or digest using a private key.
 *
 * @param[in]   p_hash Hash. Pointer must be aligned to a 4-byte boundary.
 * @param[out]  p_sig  Signature. Pointer must be aligned to a 4-byte boundary.
 *
 * @retval      3:  Signature successfuly created.
 * @retval     -1:  NULL pointer provided.
 * @retval     -2:   Unaligned pointer provided.
 */
int mi_crypto_ecc_sign(ecc_curve_t curve, const uint8_t *p_hash, uint8_t *p_sig)
{
    if (p_hash == 0 || p_sig == 0)
        return -1;

    int ret = micro_ecc_sign(&curve, dev_crt_sk, p_hash, p_sig);
    //MI_ERR_CHECK(ret);
    return (ret == MI_SUCCESS ? PT_ENDED : PT_EXITED);
}

/**@brief Verify a signature using a public key.
 *
 * @param[in]   p_pk   Public key. Pointer must be aligned to a 4-byte boundary.
 * @param[in]   p_hash Hash. Pointer must be aligned to a 4-byte boundary.
 * @param[in]   p_sig  Signature. Pointer must be aligned to a 4-byte boundary.
 *
 * @retval      3:  Signature verified.
 * @retval      2:  Signature failed verification.
 * @retval     -1:  NULL pointer provided.
 * @retval     -2:  Unaligned pointer provided.
 */
int mi_crypto_ecc_verify(ecc_curve_t curve, const ecc256_pk_t pk, const uint8_t *p_hash, const uint8_t *p_sig)
{
    if (pk == 0 || p_hash == 0 || p_sig == 0)
        return -1;

    int ret = micro_ecc_verify(&curve, pk, p_hash, p_sig);
    //MI_ERR_CHECK(ret);
    return (ret == MI_SUCCESS ? PT_ENDED : PT_EXITED);
}


int mi_crypto_get_crt_len(uint16_t *p_dev_len, uint16_t *p_manu_len, uint16_t *p_root_len)
{
    return PT_ENDED;
}

int mi_crypto_get_crt_der(mi_cert_t type, uint8_t *p_der, uint16_t der_len)
{
    if (p_der == NULL)
        return -1;

    return PT_ENDED;
}

//int mi_crypto_crt_parse_der(const unsigned char *crt, size_t crt_sz,
//        const msc_crt_time *now, msc_crt_t *msc_crt)
//{
//    return mbedtls_crt_parse_der(crt, crt_sz, now, msc_crt);
//}


int mi_crypto_hash_init(void * p_ctx)
{
    return 0;
}

int mi_crypto_hash_update(void *p_ctx, uint8_t *p_data, uint16_t data_len)
{
    return 0;
}

int mi_crypto_hash_finish(void *p_ctx, uint8_t *p_digest, uint8_t *p_digest_len)
{
    return 0;
}

int mi_crypto_sha256(uint8_t *p_in, uint32_t in_len, uint8_t *p_out)
{
    //mbedtls_sha256(p_in, in_len, p_out, 0);
    return 0;
}

int mi_crypto_hkdf_sha256(const uint8_t *key, uint8_t key_len, const uint8_t *salt, uint8_t salt_len,
                        const uint8_t *info, uint16_t info_len, uint8_t *out, uint8_t out_len)
{
    return mbedtls_sha256_hkdf(key, key_len, salt, salt_len, info, info_len, out, out_len);
}

int mi_crypto_hmac_init(void *p_ctx, uint8_t *p_key, uint8_t key_len)
{
    return 0;
}

int mi_crypto_hmac_update(void *p_ctx, uint8_t *p_data, uint16_t data_len){
    return 0;
}

int mi_crypto_hmac_finish(void *p_ctx, uint8_t *p_digest, uint8_t *p_digest_len)
{
    return 0;
}

int mi_crypto_hmac_sha256(const uint8_t *k, uint32_t key_len, const uint8_t *p_in, uint32_t in_len, uint8_t *p_out)
{
    return mbedtls_hmac_sha256(k, key_len, p_in, in_len, p_out);
}

int mi_crypto_ccm_encrypt(const uint8_t *key,
                         const uint8_t *iv, size_t iv_len,
                         const uint8_t *add, size_t add_len,
                         const uint8_t *in, size_t in_len,
                         uint8_t *out,
                         uint8_t *tag, size_t tag_len )
{
    return 0;
}

int mi_crypto_ccm_decrypt( const uint8_t *key,
                        const uint8_t *iv, size_t iv_len,
                        const uint8_t *add, size_t add_len,
                        const uint8_t *in, size_t in_len,
                        uint8_t *out,
                        const uint8_t *tag, size_t tag_len )
{
    return mbedtls_crypto_ccm_decrypt(key, iv, iv_len, add, add_len, in, in_len, out, tag, tag_len);
}

int mi_crypto_record_write(uint8_t record_id, const uint8_t *p_data, uint8_t len)
{
    return 0;
}

int mi_crypto_record_read(uint8_t record_id, uint8_t *p_data, uint8_t len)
{
    return 0;
}

int mi_crypto_record_delete(uint8_t record_id)
{
    return 0;
}

#endif  /* __MI_CRYPTO_C__ */ 
