
#ifndef __MIIO_CIPHERS_H_
#define __MIIO_CIPHERS_H_

#include "miio_instance.h"
#include "miio_arch.h"

typedef enum{
	MIIO_HANDSHAKE_TYPE_HELLO = 1,
	MIIO_HANDSHAKE_TYPE_ECDH,
}miio_handshake_type_t;

/**
 * @brief Supported message digests.
 *
 */
typedef enum {
    MIIO_MD_NONE=0,    /**< None. */
    MIIO_MD_MD2,       /**< The MD2 message digest. */
    MIIO_MD_MD4,       /**< The MD4 message digest. */
    MIIO_MD_MD5,       /**< The MD5 message digest. */
    MIIO_MD_SHA1,      /**< The SHA-1 message digest. */
    MIIO_MD_SHA224,    /**< The SHA-224 message digest. */
    MIIO_MD_SHA256,    /**< The SHA-256 message digest. */
    MIIO_MD_SHA384,    /**< The SHA-384 message digest. */
    MIIO_MD_SHA512,    /**< The SHA-512 message digest. */
    MIIO_MD_RIPEMD160, /**< The RIPEMD-160 message digest. */
} miio_md_type_t;

typedef enum
{
    MIIO_CURVE_NONE = 0,       /*!< Curve not defined. */
	MIIO_CURVE_SECP192R1,      /*!< Domain parameters for the 192-bit curve defined by FIPS 186-4 and SEC1. */
	MIIO_CURVE_SECP224R1,      /*!< Domain parameters for the 224-bit curve defined by FIPS 186-4 and SEC1. */
	MIIO_CURVE_SECP256R1,      /*!< Domain parameters for the 256-bit curve defined by FIPS 186-4 and SEC1. */
	MIIO_CURVE_SECP384R1,      /*!< Domain parameters for the 384-bit curve defined by FIPS 186-4 and SEC1. */
	MIIO_CURVE_SECP521R1,      /*!< Domain parameters for the 521-bit curve defined by FIPS 186-4 and SEC1. */
	MIIO_CURVE_BP256R1,        /*!< Domain parameters for 256-bit Brainpool curve. */
	MIIO_CURVE_BP384R1,        /*!< Domain parameters for 384-bit Brainpool curve. */
	MIIO_CURVE_BP512R1,        /*!< Domain parameters for 512-bit Brainpool curve. */
	MIIO_CURVE_CURVE25519,     /*!< Domain parameters for Curve25519. */
	MIIO_CURVE_SECP192K1,      /*!< Domain parameters for 192-bit "Koblitz" curve. */
	MIIO_CURVE_SECP224K1,      /*!< Domain parameters for 224-bit "Koblitz" curve. */
	MIIO_CURVE_SECP256K1,      /*!< Domain parameters for 256-bit "Koblitz" curve. */
	MIIO_CURVE_CURVE448,       /*!< Domain parameters for Curve448. */
}miio_curve_suite_id_t;

typedef enum{
	MIIO_SIGN_NONE = 0,
	MIIO_SIGN_HMAC_SHA256,
	MIIO_SIGN_PK
}miio_sign_suite_id_t;

typedef struct{
	uint8_t safekey[16];
}miio_ciphers_t;

void* miio_sha256_init(void);
void miio_sha256_update(void *handle, unsigned char *data, size_t data_len);
void miio_sha256_finish(void *handle, unsigned char hash[32]);
int miio_signature_verify(const unsigned char* pems, size_t pems_size, const char* org, unsigned char *hash, size_t hash_len, unsigned char *signature, size_t sign_len);
int miio_sign(void *identity, int sign_suite, void *sign_params, void *data, size_t data_len, uint8_t *sign_out, size_t *sign_out_len, int base64);
void miio_ciphers_get_safekey(uint8_t safekey[16]);
void miio_ciphers_set_safekey(uint8_t safekey[16]);
const miio_ciphers_interface_t *miio_ciphers_interface_default(void);


typedef int32_t miio_ecdh_handle_t;

/**
 * @brief miio_ciphers_ecdh_valid
 * 
 * @param handle 
 * @return true valid
 * @return false invalid
 */

bool miio_ciphers_ecdh_valid(miio_ecdh_handle_t handle);

/**
 * @brief miio_ciphers_ecdh_generate_key
 * 
 * @param phandle 
 * @param id 
 * @param Q 
 * @param size 
 * @return MIIO_OK success 
 */
int miio_ciphers_ecdh_generate_key(miio_ecdh_handle_t *phandle,
									miio_curve_suite_id_t id,
									uint8_t *Q, 
									size_t *size);

/**
 * @brief miio_ciphers_ecdh_establish_key
 * 
 * @param handle 
 * @param Qp 	The value of the public key of the peer.
 * @param Qplen 
 * @param z 	The shared secret.
 * @param zlen 
 * @param hkdf 
 * @return MIIO_OK success 
 */
int miio_ciphers_ecdh_establish_key(miio_ecdh_handle_t handle, 
									const uint8_t *Qp, 
									size_t Qplen, 
									uint8_t *z, 
									size_t *zlen,
									bool hkdf);

/**
 * @brief miio_ciphers_ecdh_reset
 * 
 * @param handle 
 * @return MIIO_OK success 
 */
int miio_ciphers_ecdh_finsh(miio_ecdh_handle_t handle);

/**
 * @brief reset all of ecdh sessions
 * 
 */
void miio_ciphers_ecdh_reset();

/**
 * @brief miio_ciphers_hkdf
 * 
 * @param ikm 
 * @param ikmlen 
 * @param info 
 * @param ilen 
 * @param okm 
 * @param olen 
 * @return MIIO_OK success 
 */
 int miio_ciphers_hkdf(miio_md_type_t md,
						const unsigned char *salt,
						size_t salt_len,
						const uint8_t *ikm,
						size_t ikmlen,
						const uint8_t *info,
						size_t ilen,
						uint8_t* okm,
						size_t olen);

#if MIIO_KEY_EXCHANGE_ENABLE
int miio_hkdf(const unsigned char *salt,
          size_t salt_len, const unsigned char *ikm, size_t ikm_len,
          const unsigned char *info, size_t info_len,
          unsigned char *okm, size_t okm_len );
#endif		  
													
#endif
