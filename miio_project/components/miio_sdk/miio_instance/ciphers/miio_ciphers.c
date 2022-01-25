/*
 * ot_certs.c
 *
 *  Created on: Oct 12, 2017
 *      Author: mashaoze
 */
#include "miio_net.h"
#include "miio_ciphers.h"
#include "jsmi.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/oid.h"
#include "mbedtls/md.h"
#include "mbedtls/pk.h"
#include "mbedtls/base64.h"
#include "mbedtls/sha256.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/aes.h"
#undef MIIO_LOG_TAG
#define MIIO_LOG_TAG			"miio_ciphers"

//////////////////////////////////////////////////////////////////////////////////////////
static miio_ciphers_t s_miio_ciphers = {
	.safekey = {0}
};

const unsigned char miio_mijia_root_cert[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIBazCCAQ+gAwIBAgIEA/UKYDAMBggqhkjOPQQDAgUAMCIxEzARBgNVBAoTCk1p\r\n"
"amlhIFJvb3QxCzAJBgNVBAYTAkNOMCAXDTE2MTEyMzAxMzk0NVoYDzIwNjYxMTEx\r\n"
"MDEzOTQ1WjAiMRMwEQYDVQQKEwpNaWppYSBSb290MQswCQYDVQQGEwJDTjBZMBMG\r\n"
"ByqGSM49AgEGCCqGSM49AwEHA0IABL71iwLa4//4VBqgRI+6xE23xpovqPCxtv96\r\n"
"2VHbZij61/Ag6jmi7oZ/3Xg/3C+whglcwoUEE6KALGJ9vccV9PmjLzAtMAwGA1Ud\r\n"
"EwQFMAMBAf8wHQYDVR0OBBYEFJa3onw5sblmM6n40QmyAGDI5sURMAwGCCqGSM49\r\n"
"BAMCBQADSAAwRQIgchciK9h6tZmfrP8Ka6KziQ4Lv3hKfrHtAZXMHPda4IYCIQCG\r\n"
"az93ggFcbrG9u2wixjx1HKW4DUA5NXZG0wWQTpJTbQ==\r\n"
"-----END CERTIFICATE-----\r\n";

#if MIIO_DEVICE_CERT_ENABLE
const unsigned char miio_device_cert_chain[] = "";
const unsigned char miio_device_pk[] = "";
#endif

static const int miio_sign_suites_default[] = {
#if MIIO_DEVICE_CERT_ENABLE
		MIIO_SIGN_PK,
#else
		MIIO_SIGN_HMAC_SHA256
#endif
};



typedef struct {
	miio_curve_suite_id_t miio_id;
	mbedtls_ecp_group_id lib_id;
	const char *name;
}miio_curve_info_t;

static const miio_curve_info_t miio_curves_default[] = {
	{
		.miio_id = MIIO_CURVE_SECP256R1,
		.lib_id = MBEDTLS_ECP_DP_SECP256R1,
		.name = "secp256r1"
	},
	{
		.miio_id = MIIO_CURVE_SECP384R1,
		.lib_id = MBEDTLS_ECP_DP_SECP384R1,
		.name = "secp384r1"
	}
};

static const miio_curve_info_t* miio_curve_info(miio_curve_suite_id_t id)
{
	for(int i=0; i < NELEMENTS(miio_curves_default); i++){
		if(miio_curves_default[i].miio_id == id){
			return &miio_curves_default[i];
		}
	}

	return NULL;
}


static int hkdf_expand( const mbedtls_md_info_t *md, const unsigned char *prk,
                         size_t prk_len, const unsigned char *info,
                         size_t info_len, unsigned char *okm, size_t okm_len )
{
    size_t hash_len;
    size_t where = 0;
    size_t n;
    size_t t_len = 0;
    size_t i;
    int ret = 0;
    mbedtls_md_context_t ctx;
    unsigned char t[MBEDTLS_MD_MAX_SIZE];

    if( okm == NULL )
    {
        return( MIIO_ERROR_PARAM );
    }

    hash_len = mbedtls_md_get_size( md );

    if( prk_len < hash_len || hash_len == 0 )
    {
        return( MIIO_ERROR_PARAM );
    }

    if( info == NULL )
    {
        info = (const unsigned char *) "";
        info_len = 0;
    }

    n = okm_len / hash_len;

    if( (okm_len % hash_len) != 0 )
    {
        n++;
    }

    /*
     * Per RFC 5869 Section 2.3, okm_len must not exceed
     * 255 times the hash length
     */
    if( n > 255 )
    {
        return( MIIO_ERROR_PARAM );
    }

    mbedtls_md_init( &ctx );

    if( (ret = mbedtls_md_setup( &ctx, md, 1) ) != 0 )
    {
        goto exit;
    }

    /*
     * Compute T = T(1) | T(2) | T(3) | ... | T(N)
     * Where T(N) is defined in RFC 5869 Section 2.3
     */
    for( i = 1; i <= n; i++ )
    {
        size_t num_to_copy;
        unsigned char c = i & 0xff;

        ret = mbedtls_md_hmac_starts( &ctx, prk, prk_len );
        if( ret != 0 )
        {
            goto exit;
        }

        ret = mbedtls_md_hmac_update( &ctx, t, t_len );
        if( ret != 0 )
        {
            goto exit;
        }

        ret = mbedtls_md_hmac_update( &ctx, info, info_len );
        if( ret != 0 )
        {
            goto exit;
        }

        /* The constant concatenated to the end of each T(n) is a single octet.
         * */
        ret = mbedtls_md_hmac_update( &ctx, &c, 1 );
        if( ret != 0 )
        {
            goto exit;
        }

        ret = mbedtls_md_hmac_finish( &ctx, t );
        if( ret != 0 )
        {
            goto exit;
        }

        num_to_copy = i != n ? hash_len : okm_len - where;
        memcpy( okm + where, t, num_to_copy );
        where += hash_len;
        t_len = hash_len;
    }

exit:
    mbedtls_md_free( &ctx );
    memset( t, 0,sizeof( t ) );

    return( ret );
}

static int hkdf_extract( const mbedtls_md_info_t *md,
                          const unsigned char *salt, size_t salt_len,
                          const unsigned char *ikm, size_t ikm_len,
                          unsigned char *prk )
{
    unsigned char null_salt[MBEDTLS_MD_MAX_SIZE] = { '\0' };

    if( salt == NULL )
    {
        size_t hash_len;

        if( salt_len != 0 )
        {
            return MIIO_ERROR_PARAM;
        }

        hash_len = mbedtls_md_get_size( md );

        if( hash_len == 0 )
        {
            return MIIO_ERROR_PARAM;
        }

        salt = null_salt;
        salt_len = hash_len;
    }

    return( mbedtls_md_hmac( md, salt, salt_len, ikm, ikm_len, prk ) );
}

static int hkdf( const mbedtls_md_info_t *md, const unsigned char *salt,
                  size_t salt_len, const unsigned char *ikm, size_t ikm_len,
                  const unsigned char *info, size_t info_len,
                  unsigned char *okm, size_t okm_len )
{
    int ret;
    unsigned char prk[MBEDTLS_MD_MAX_SIZE] = {0};

    ret = hkdf_extract( md, salt, salt_len, ikm, ikm_len, prk );

    if( ret == 0 )
    {
        ret = hkdf_expand( md, prk, mbedtls_md_get_size( md ),
                                   info, info_len, okm, okm_len );
    }

    return( ret );
}

static int get_did(void* identity, uint64_t *did)
{
	arch_get_did(did);

	return sizeof(uint64_t);
}

static int get_random(void* identity, uint8_t *output, size_t output_len)
{
	arch_get_random(output, output_len);

	return 0;
}


static int get_psk(void* identity, uint8_t psk[16])
{
#if MIIO_DEVICE_CERT_ENABLE
	return 0;
#else
	arch_get_psk(psk);
	return 16;
#endif
}

#if MIIO_KEY_EXCHANGE_ENABLE
int miio_hkdf(const unsigned char *salt,
                  size_t salt_len, const unsigned char *ikm, size_t ikm_len,
                  const unsigned char *info, size_t info_len,
                  unsigned char *okm, size_t okm_len )
{
    return hkdf(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), salt, salt_len, ikm, ikm_len, info, info_len, okm, okm_len);
}

static int get_new_did(void* identity, uint64_t *did)
{
    uint64_t bind_did = 0;
    int ret = 0;
    
    if(sizeof(bind_did) == arch_psm_get_value("key_exchange", "bind_did", &bind_did, sizeof(bind_did))){
        *did = bind_did;
        return sizeof(uint64_t);     
    }
    else{
        ret = get_did(identity, did);
        return ret;
    }
}

static int get_new_psk(void* identity, uint8_t psk[16])
{
    int ret = 0;
#if MIIO_DEVICE_CERT_ENABLE
	return 0;
#else
    uint8_t ot_key[16];
    memset(ot_key, 0, sizeof(ot_key));
    if(sizeof(ot_key) == arch_psm_get_value("key_exchange", "ot_key", ot_key, sizeof(ot_key))){
        memcpy(psk, ot_key, sizeof(ot_key));
        return sizeof(ot_key);
    }
    else{
        ret=  get_psk(identity, psk);
        return ret;
    }
#endif
}
#endif

static int get_token(void* identity, uint8_t token[16])
{
#if MIIO_INSTANCE_LX2_ADAPT
	char token_hex[32];
	if(32 == arch_psm_get_value("ot_config", "psm_token", token_hex, 32)){
		return arch_axtobuf(token_hex, 32, token, 16, NULL);
	}
	return 0;
#else
	return arch_psm_get_value("ot_config", "psm_token", token, 16);
#endif
}

static int update_token(void* identity, uint8_t token[16])
{
	uint8_t token_generate_buffer[16*3];
	uint8_t token_temp[16];

	//fill token_generate_buffer
	get_random(identity, token_generate_buffer, 16);
	snprintf((char *)token_generate_buffer+16, 16, "%u", arch_os_ms_now());
#if MIIO_DEVICE_CERT_ENABLE
	get_random(identity, token_generate_buffer+16*2, 16);
#else
	get_psk(identity, token_generate_buffer+16*2);
#endif
	//generate token
	arch_md5(token_generate_buffer, sizeof(token_generate_buffer), token_temp);

#if MIIO_INSTANCE_LX2_ADAPT
	snprintf_hex((char*)token_generate_buffer, 32+1, token_temp, 16, 0);
	if(32 != arch_psm_set_value("ot_config", "psm_token", token_generate_buffer, 32)){
		LOG_ERROR_TAG(MIIO_LOG_TAG, "token save failed");
	}
	else{
		LOG_INFO_TAG(MIIO_LOG_TAG, "token updated");
	}
#else
	if(16 != arch_psm_set_value("ot_config", "psm_token", token_temp, 16)){
		LOG_ERROR_TAG(MIIO_LOG_TAG, "token save failed");
	}
	else{
		LOG_INFO_TAG(MIIO_LOG_TAG, "token updated");
	}
#endif

	if(token){
		memcpy(token, token_temp, 16);
	}

	return 16;
}


const unsigned char* miio_get_server_root_cert_pem(void)
{
	return miio_mijia_root_cert;
}

const unsigned char* miio_get_verify_root_cert_pem(void)
{
	return miio_mijia_root_cert;
}

void* miio_sha256_init(void)
{
	mbedtls_sha256_context *sha256_ctx = malloc(sizeof(mbedtls_sha256_context));

	if(sha256_ctx){

		mbedtls_sha256_init(sha256_ctx);
		mbedtls_sha256_starts(sha256_ctx, 0);
	}

	return (void*)sha256_ctx;
}

void miio_sha256_update(void *handle, unsigned char *data, size_t data_len)
{
	if(handle){
		mbedtls_sha256_context *sha256_ctx = handle;
		mbedtls_sha256_update(sha256_ctx, data, data_len);
	}
}

void miio_sha256_finish(void *handle, unsigned char hash[32])
{
	if(handle){
		mbedtls_sha256_context *sha256_ctx = handle;
		mbedtls_sha256_finish(sha256_ctx, hash);
		mbedtls_sha256_free(sha256_ctx);
		free(handle);
	}
}


int miio_signature_verify(const unsigned char* pems, size_t pems_size, const char* org, unsigned char *hash, size_t hash_len, unsigned char *signature, size_t sign_len)
{

	int ret = 0;

	//parse pems
	mbedtls_x509_crt ca_root;
	mbedtls_x509_crt cert_chain;
	mbedtls_x509_crt_init( &ca_root );
	mbedtls_x509_crt_init( &cert_chain );

	ret = mbedtls_x509_crt_parse( &ca_root, miio_mijia_root_cert , sizeof(miio_mijia_root_cert) );
	if(ret != 0){
		LOG_ERROR_TAG(MIIO_LOG_TAG, "verify root cert is not valid!");
		goto verify_exit;
	}

	ret = mbedtls_x509_crt_parse( &cert_chain, pems, pems_size );
	if(ret != 0){
		LOG_ERROR_TAG(MIIO_LOG_TAG, "pems are not valid!");
		goto verify_exit;
	}

	//verify cert chain
	mbedtls_x509_crt *parent = &ca_root;
	mbedtls_x509_crt *child = &cert_chain;

	do{
#if 0
		char *buf = malloc(1024);
		if(buf){
			mbedtls_x509_crt_info( buf, 1024, "parent:", parent );
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "%s", buf);
			mbedtls_x509_crt_info( buf, 1024, "child:", child );
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "%s", buf);
			free(buf);
		}
#endif

		uint32_t flags = 0;
		ret = mbedtls_x509_crt_verify(child, parent, NULL, NULL, &flags, NULL, NULL);

		if(0 != ret){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "pems verify failed[%d]...", flags);
			goto verify_exit;
		}

		parent = child;
		child = child->next;
	}while(child);

	//check org
	if(org && strlen(org)){

		int found = 0;

		mbedtls_x509_name *name = &parent->subject;
		while( name != NULL ){

			if( MBEDTLS_OID_CMP( MBEDTLS_OID_AT_ORGANIZATION, &name->oid ) == 0 ){

				if( name->val.len == (sizeof("xiaomi")-1) &&
					memcasecmp( name->val.p, "xiaomi", (sizeof("xiaomi")-1) ) == 0 ){
					found++;
					break;
				}

				size_t org_len = strlen(org);

				if( name->val.len == org_len &&
					memcasecmp( name->val.p, org, org_len ) == 0 ){
					found++;
					break;
				}

			}

			name = name->next;
		}

		if(0 == found){
			ret = -1;
			LOG_ERROR_TAG(MIIO_LOG_TAG, "invalid org...");
			goto verify_exit;
		}
	}

	LOG_DEBUG_TAG(MIIO_LOG_TAG, "sign verify...");

	//verify app sign
	ret = mbedtls_pk_verify(&parent->pk, MBEDTLS_MD_SHA256, hash, hash_len, signature, sign_len);

verify_exit:

	mbedtls_x509_crt_free( &ca_root  );
	mbedtls_x509_crt_free( &cert_chain );

	return ( (ret != 0) ? MIIO_ERROR : MIIO_OK);

}

const unsigned char* miio_get_device_cert_pem(void *identity)
{
#if MIIO_DEVICE_CERT_ENABLE
	return miio_device_cert_chain;
#endif
	return NULL;
}

int miio_get_sign_suites(int sign_suites[], size_t sign_suites_nums)
{
	size_t i;
	for(i=0; i<sign_suites_nums; i++){
		if(i >= NELEMENTS(miio_sign_suites_default))break;
		sign_suites[i] = miio_sign_suites_default[i];
	}

	return i;
}


int miio_sign(void *identity, int sign_suite, void *sign_params, void *data, size_t data_len, uint8_t *sign_out, size_t *sign_out_len, int base64)
{
	int ret = 0;

	switch(sign_suite){

#if MIIO_DEVICE_CERT_ENABLE
		case MIIO_SIGN_PK:
			{
				uint8_t sign[MIIO_SIGN_AN1_LEN_MAX] = {0};
				size_t sign_len = sizeof(sign);
				uint8_t hash[32] = {0};

				mbedtls_pk_context pk;
				mbedtls_pk_init(&pk);

				ret = mbedtls_pk_parse_key(&pk, miio_device_pk, sizeof(miio_device_pk), NULL, 0);
				if(0 != ret){
					mbedtls_pk_free(&pk);
					LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_pk_parse_key() returned -0x%04x\n", -ret);
					break;
				}

				ret = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), data, data_len, hash);
				if(0 != ret){
					mbedtls_pk_free(&pk);
					LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_md() returned -0x%04x\n", -ret);
					break;
				}

				ret = mbedtls_pk_sign(&pk, MBEDTLS_MD_SHA256, hash, sizeof(hash), sign, &sign_len, get_random, NULL);

				mbedtls_pk_free(&pk);

				if(0 != ret){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_pk_sign() returned -0x%04x\n", -ret);
					break;
				}

				if(base64){
					ret = mbedtls_base64_encode(sign_out, *sign_out_len, sign_out_len, sign, sign_len);
					if(0 != ret){
						LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_base64_encode() returned -0x%04x\n", -ret);
					}
					break;
				}

				if(*sign_out_len < sizeof(sign)){
					ret = -1;
					*sign_out_len = sizeof(sign);
					break;
				}

				memcpy(sign_out, sign, sign_len);
				*sign_out_len = sign_len;
			}
			break;
#endif

		case MIIO_SIGN_HMAC_SHA256:
			{
				uint8_t sign[32] = {0};
				size_t sign_len = sizeof(sign);

				mbedtls_md_context_t sha_ctx;
				mbedtls_md_init(&sha_ctx);

				ret = mbedtls_md_setup(&sha_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
				if(0 != ret){
					mbedtls_md_free( &sha_ctx );
					LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_md_setup() returned -0x%04x\n", -ret);
					break;
				}

				uint8_t *hmac_key = sign_params;
				mbedtls_md_hmac_starts(&sha_ctx, hmac_key, 16);
				mbedtls_md_hmac_update(&sha_ctx, data, data_len);
				mbedtls_md_hmac_finish(&sha_ctx, sign );

				mbedtls_md_free( &sha_ctx );

				if(base64){
					ret = mbedtls_base64_encode(sign_out, *sign_out_len, sign_out_len, sign, sign_len);
					if(0 != ret){
						LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_base64_encode() returned -0x%04x\n", -ret);
					}
					break;
				}

				if(*sign_out_len < sign_len){
					ret = -1;
					*sign_out_len = sign_len;
					break;
				}

				memcpy(sign_out, sign, sign_len);
				*sign_out_len = sign_len;
			}
			break;
		default:

			ret = -1;

			break;
	}

	return (ret == 0 ? MIIO_OK : MIIO_ERROR);
}


static const miio_ciphers_interface_t s_miio_ciphers_if_default = {
	.identity = NULL,
	.get_did = get_did,
#if MIIO_KEY_EXCHANGE_ENABLE
    .get_new_did = get_new_did,
    .get_new_psk = get_new_psk,
#endif
	.get_psk = get_psk,
	.get_token = get_token,
	.update_token = update_token,
	.get_random = get_random,
	.get_root_cert_pem = miio_get_verify_root_cert_pem,
	.get_device_cert_pem = miio_get_device_cert_pem,
	.get_sign_suites = miio_get_sign_suites,
	.get_sign = miio_sign
};


void miio_ciphers_get_safekey(uint8_t safekey[16])
{
	memcpy(safekey, s_miio_ciphers.safekey, 16);
}
void miio_ciphers_set_safekey(uint8_t safekey[16]){
	memcpy(s_miio_ciphers.safekey,safekey,  16);
}

const miio_ciphers_interface_t *miio_ciphers_interface_default(void)
{
	return &s_miio_ciphers_if_default;
}
	
int miio_ciphers_hkdf(miio_md_type_t md,
						const unsigned char *salt,
						size_t salt_len,
						const uint8_t *ikm,
						size_t ikmlen,
						const uint8_t *info,
						size_t ilen,
						uint8_t* okm,
						size_t olen){

#define CONVER_MD_TO_MBEDTLS(md) \
(\
	(md) == MIIO_MD_MD2 		? MBEDTLS_MD_MD2 	 	:\
	(md) == MIIO_MD_MD4 		? MBEDTLS_MD_MD4 	 	:\
	(md) == MIIO_MD_MD5 		? MBEDTLS_MD_MD5 		:\
	(md) == MIIO_MD_SHA1 		? MBEDTLS_MD_SHA1 	 	:\
	(md) == MIIO_MD_SHA224 		? MBEDTLS_MD_SHA224 	:\
	(md) == MIIO_MD_SHA256 		? MBEDTLS_MD_SHA256 	:\
	(md) == MIIO_MD_SHA384 		? MBEDTLS_MD_SHA384 	:\
	(md) == MIIO_MD_SHA512 		? MBEDTLS_MD_SHA512 	:\
	(md) == MIIO_MD_RIPEMD160 	? MBEDTLS_MD_RIPEMD160 	:\
	MBEDTLS_MD_NONE\
)
	return hkdf(mbedtls_md_info_from_type(CONVER_MD_TO_MBEDTLS(md)), salt, salt_len, ikm, ikmlen, info, ilen, okm, olen);
}

///////////////////////////////////////////////////////////////

#define MIIO_CIPHERS_ERROR_CHECK(expression,label, fmt, ...) \
do{\
	if( !(expression) ){\
		LOG_ERROR_TAG( MIIO_LOG_TAG, fmt, ##__VA_ARGS__ );\
		goto label; \
	} \
}while(0)

#define ECDH_DEVICE_FOURCC	(('E'<< 24) | ('C'<< 16) | ('D'<< 8) | ('H'))

#define ECDH_DEVICE_FOURCC_CHECK(fourcc) ( (fourcc) == ECDH_DEVICE_FOURCC )

#define ECDH_DEVICE_COUNT ( sizeof( g_ecdh_device ) / sizeof( g_ecdh_device[0] ) )

#define ECDH_DEVICE_INDEX_TO_HANDLE( i ) ( (i) + 1 )

#define ECDH_DEVICE_HANDLE_TO_INDEX(handle) ( (handle) -1 )

#define ECDH_HANDLE_VALID( handle ) (( 0 <= ECDH_DEVICE_HANDLE_TO_INDEX((handle)) ) && ( ECDH_DEVICE_HANDLE_TO_INDEX((handle)) < ECDH_DEVICE_COUNT )) 


typedef struct miio_ecdh_device{

	const int32_t fourcc;
	
	mbedtls_ecp_group_id group_id;
	
	mbedtls_ecdh_context context;

	bool working;

}miio_ecdh_device_t;

static miio_ecdh_device_t g_ecdh_device[2]={
	[0]={
		.fourcc = ECDH_DEVICE_FOURCC,
		.group_id = MBEDTLS_ECP_DP_NONE,
		.working = false,
	},
	[1]={
		.fourcc = ECDH_DEVICE_FOURCC,
		.group_id = MBEDTLS_ECP_DP_NONE,
		.working = false,
	},
};
bool miio_ciphers_ecdh_valid(miio_ecdh_handle_t handle){

	return ECDH_HANDLE_VALID(handle);
}
int miio_ciphers_ecdh_generate_key(miio_ecdh_handle_t *phandle,
									miio_curve_suite_id_t id,									 
									uint8_t *Q, 
									size_t *size){
	int r = 0;
	int index = 0;
	miio_ecdh_device_t * ctx = NULL;
	
	const miio_curve_info_t* pcurve = miio_curve_info(id);
	
	MIIO_CIPHERS_ERROR_CHECK( NULL != phandle, _ERROR1, "param error");
	
	*phandle = 0;

	for( index = 0; index < ECDH_DEVICE_COUNT; index++ ){

		if(!g_ecdh_device[index].working){

			ctx = &g_ecdh_device[index];

			break;
		}
	}
	MIIO_CIPHERS_ERROR_CHECK( ctx != NULL , _ERROR1, "all device is working" );
	
	MIIO_CIPHERS_ERROR_CHECK ( NULL != pcurve, _ERROR1, "miio_curve_suite_id %d error", id);
	
	
	ctx->group_id = pcurve->lib_id;
	
	mbedtls_ecdh_init( &ctx->context );

	r = mbedtls_ecp_group_load( &ctx->context.grp, ctx->group_id );
	MIIO_CIPHERS_ERROR_CHECK( 0 == r, _ERROR2, "mbedtls_ecp_group_load error %d", r );

	//Generate main ECC keypair
	r = mbedtls_ecdh_gen_public( &ctx->context.grp, &ctx->context.d, &ctx->context.Q, get_random, NULL );
	MIIO_CIPHERS_ERROR_CHECK( 0 == r, _ERROR2, "mbedtls_ecdh_gen_public error %d", r );

	//Qszie = mbedtls_mpi_size( &ctx->context.grp.P ) * 2 + 1;
	r = mbedtls_ecp_point_write_binary( &ctx->context.grp, &ctx->context.Q, MBEDTLS_ECP_PF_UNCOMPRESSED, size, Q, *size);
	MIIO_CIPHERS_ERROR_CHECK( 0 == r, _ERROR2, "mbedtls_ecp_point_write_binary error %d", r);

	g_ecdh_device[index].working = true;

	*phandle = ECDH_DEVICE_INDEX_TO_HANDLE(index);
	
	return MIIO_OK;
	
_ERROR2:
	mbedtls_ecdh_free( &ctx->context );
	
_ERROR1:
	return MIIO_ERROR;
}
int miio_ciphers_ecdh_establish_key(miio_ecdh_handle_t handle, 
									const uint8_t *Qp, 
									size_t Qplen, 
									uint8_t *z, 
									size_t *zlen, bool hkdf){
	miio_ecdh_device_t * ctx = NULL;
	
	int r = 0;

	MIIO_CIPHERS_ERROR_CHECK( ECDH_HANDLE_VALID(handle), _ERROR, "paramter error" );
	
	ctx = &g_ecdh_device[ECDH_DEVICE_HANDLE_TO_INDEX(handle)];

	MIIO_CIPHERS_ERROR_CHECK( ECDH_DEVICE_FOURCC_CHECK( ctx->fourcc ), _ERROR, "big error" );

	MIIO_CIPHERS_ERROR_CHECK( ctx->working, _ERROR, "device is not working" );
	
	r = mbedtls_ecp_point_read_binary( &ctx->context.grp, &ctx->context.Qp, Qp, Qplen );
	MIIO_CIPHERS_ERROR_CHECK( 0 == r, _ERROR, "mbedtls_ecp_point_read_binary error %d", r );
	
	if( !hkdf ){
		
		r = mbedtls_ecdh_calc_secret( &ctx->context, zlen, z, *zlen, get_random, NULL );
		MIIO_CIPHERS_ERROR_CHECK( 0 == r, _ERROR, "mbedtls_ecdh_calc_secret error %d", r );
	}else{
		uint8_t secret_buffer[128];
		size_t secret_buffer_size = sizeof( secret_buffer );
		
		r = mbedtls_ecdh_calc_secret( &ctx->context, &secret_buffer_size, secret_buffer, secret_buffer_size, get_random, NULL );
		MIIO_CIPHERS_ERROR_CHECK( 0 == r, _ERROR, "mbedtls_ecdh_calc_secret error %d", r );
		
		uint8_t token[16] = {0};
		uint8_t salt[sizeof(token)*2+1] = {0};
		get_token(NULL, token);
		snprintf_hex((char*)salt, sizeof(salt), token, sizeof(token), 0);

		r = miio_ciphers_hkdf( MIIO_MD_SHA256, salt, sizeof(salt)-1, secret_buffer, secret_buffer_size, NULL, 0, z, *zlen );
		MIIO_CIPHERS_ERROR_CHECK( 0 == r, _ERROR,  "miio_ciphers_hkdf error %d", r );
	}
	
	return MIIO_OK;
	
_ERROR:
	return MIIO_ERROR;
}
int miio_ciphers_ecdh_finsh(miio_ecdh_handle_t handle){

	miio_ecdh_device_t * ctx = NULL;

	MIIO_CIPHERS_ERROR_CHECK( ECDH_HANDLE_VALID(handle), _ERROR, "paramter error" );
	
	ctx = &g_ecdh_device[ECDH_DEVICE_HANDLE_TO_INDEX( handle )];
	
	MIIO_CIPHERS_ERROR_CHECK( ECDH_DEVICE_FOURCC_CHECK( ctx->fourcc ), _ERROR, "big error" );

	if( !ctx->working ){
		goto _END;
	}
	//> uninit miio_ecdh_device_t
	mbedtls_ecdh_free(&ctx->context);
	
	ctx->group_id = MBEDTLS_ECP_DP_NONE;

	ctx->working = false;

_END:
	return MIIO_OK;
	
_ERROR:
	return MIIO_ERROR;
	
}
void miio_ciphers_ecdh_reset(){

	for( int index = 0; index < ECDH_DEVICE_COUNT; index++ ){

		if(g_ecdh_device[index].working){

			miio_ciphers_ecdh_finsh(ECDH_DEVICE_INDEX_TO_HANDLE(index));
		}
	}
}
