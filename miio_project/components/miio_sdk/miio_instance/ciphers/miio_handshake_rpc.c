/**
 * @file miio_handshake_rpc.c
 * @author xusongsong (xusongsong@xiaomi.com)
 * @brief 
 * @version 0.1
 * @date 2020-06-01
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include "miio_net.h"
#include "miio_net_auto_provision.h"
#include "miio_ciphers.h"
#include "miio_handshake_rpc.h"
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
#define MIIO_LOG_TAG			"miio_handshake_rpc"



//////////////////////////////////////////////////////////////////////////////////////////
static const int miio_curve_suites_default[] = {
		MIIO_CURVE_SECP256R1,
		MIIO_CURVE_SECP384R1,
};
//////////////////////////////////////////////////////////////////////////////////////////
static uint8_t miio_ecdh_mode_features = (1 << MIIO_ECDH_MODE_1) ;

void miio_ecdh_mode_enable(miio_ecdh_mode_t mode){
	miio_ecdh_mode_features |= 1 << mode;
}
void miio_ecdh_mode_disable(miio_ecdh_mode_t mode){
	miio_ecdh_mode_features &= ~(1 << mode);
}
static bool miio_ecdh_mode_test(miio_ecdh_mode_t mode){
	
	return mode >= MIIO_ECDH_MODE_MAX ? false : miio_ecdh_mode_features & (1 << mode);
}
//////////////////////////////////////////////////////////////////////////////////////////
#define MIIO_HANDSHAKE_ENABLE 1
#if MIIO_HANDSHAKE_ENABLE
/**
{
	"method" : "miIO.handshake",
	"params" : {
		"type": 1
	},
	"id" : 111
}
*/
#define ECDH_OBJECT_RANDOM_SIZE (16)
static uint8_t _c_random_r[ECDH_OBJECT_RANDOM_SIZE];
static int handshake_hello(miio_rpc_delegate_arg_t *req_arg, miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
    int ret = 0;

	//construct ack packet
	miio_handle_t miio_handle = miio_from_delegtate_ctx(ctx);

	miio_rpc_delegate_arg_t ack_arg = {
		.id = req_arg->id ,
		.type = req_arg->type,
	};
#if MIIO_PROV_STATISTIC
	if(is_prov_stat_enable()){
		set_prov_stat_stage(PROV_STAGE_HANDSHAKE_HELLO);
	}
#endif

	memset(_c_random_r, 0, sizeof(_c_random_r));
    miio_ciphers_interface_default()->get_random(NULL, _c_random_r, ECDH_OBJECT_RANDOM_SIZE);
	size_t base64_c_random_r_length = MIIO_BASE64_SIZE(ECDH_OBJECT_RANDOM_SIZE);
	uint8_t base64_c_random_r[MIIO_BASE64_SIZE(ECDH_OBJECT_RANDOM_SIZE)] = {0};
    ret = mbedtls_base64_encode(base64_c_random_r, base64_c_random_r_length, &base64_c_random_r_length, _c_random_r, sizeof(_c_random_r));
	if (0 != ret) {
        LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d mbedtls_base64_encode failed，ret %d",  __FUNCTION__, __LINE__, ret);
        miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_BUSY, MIIO_OT_ERR_INFO_BUSY, ack, ctx);
#if MIIO_PROV_STATISTIC
		if(is_prov_stat_enable()){
			set_prov_stat_error_code(PROV_ERROR_HANDSHAKE_HELLO_FAIL);
		}
#endif
        return MIIO_ERROR;
    }
	ack_arg.pload_len = miio_get_rpc_max_size(miio_handle);
	ack_arg.pload = (char *) malloc(ack_arg.pload_len);
	if (ack_arg.pload) {
		jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(ack_arg.pload, ack_arg.pload_len);
		jsmi_compose_start(&jsmi_composer);
		jsmi_set_object_begin(&jsmi_composer);
			jsmi_set_key_value_uint(&jsmi_composer, "id", ack_arg.id);
			jsmi_set_key_object_begin(&jsmi_composer, "result");
				jsmi_set_key_value_sint(&jsmi_composer, "type", MIIO_HANDSHAKE_TYPE_HELLO);
				
				//set version="1.2"
				jsmi_set_key_value_str(&jsmi_composer, "version", "1.2", 3);

				//> info object
				char model[MIIO_MODEL_SIZE_MAX] = {0};
				if( arch_psm_get_str("ot_config", "psm_model", model, sizeof(model) ) <= 0 ){
					strncpy(model, MIIO_INSTANCE_MODEL, sizeof(model)-1);
					model[sizeof(model)-1] = '\0';
				}
				jsmi_set_key_object_begin(&jsmi_composer, "info");
				jsmi_set_key_value_str(&jsmi_composer, "model", model, strlen(model));
				jsmi_set_key_object_end(&jsmi_composer);

				//> ecdh
				jsmi_set_key_object_begin(&jsmi_composer, "ecdh");
				
					/*
						modes array ecdh挑战认证类型数组,类型定义如下�?						0: 无挑战认�?						1: 签名挑战认证
						其中�?						1) 无此参数时，等同于无挑战认证
					*/
					jsmi_set_key_array_begin(&jsmi_composer, "modes");
					for(miio_ecdh_mode_t m = MIIO_ECDH_MODE_0; m < MIIO_ECDH_MODE_MAX; ++m){
						if(miio_ecdh_mode_test(m)){
							jsmi_set_value_sint(&jsmi_composer, m);
						}
					}
					jsmi_set_key_array_end(&jsmi_composer);

					/*
						random string BASE64(c_random_r)
						其中�?						1) c_random_r为配网信息接收方生成�?6字节随机挑战�?						2) 无挑战认证时，无需提供此参�?					*/
					// set random = base64(c_random_r) 
					jsmi_set_key_value_str(&jsmi_composer, "random", (const char*)base64_c_random_r, strlen((const char*)base64_c_random_r));

					jsmi_set_key_array_begin(&jsmi_composer, "curve_suites");
						for(int i=0; i < NELEMENTS(miio_curve_suites_default); i++){
							jsmi_set_value_sint(&jsmi_composer, miio_curve_suites_default[i]);
						}
					jsmi_set_key_array_end(&jsmi_composer);
					jsmi_set_key_array_begin(&jsmi_composer, "sign_suites");
                    int sign_suites[3];
						for(int i=0; i < miio_ciphers_interface_default()->get_sign_suites(sign_suites, NELEMENTS(sign_suites)); i++){
							jsmi_set_value_sint(&jsmi_composer, sign_suites[i]);
						}
					jsmi_set_key_array_end(&jsmi_composer);
				jsmi_set_key_object_end(&jsmi_composer);
			jsmi_set_key_object_end(&jsmi_composer);
		jsmi_set_object_end(&jsmi_composer);
		if(MIIO_OK == jsmi_compose_finish(&jsmi_composer, NULL, &ack_arg.pload_len)){
			ack(&ack_arg, ctx);
			free(ack_arg.pload);
			return MIIO_OK;
		}
	}

	if(ack_arg.pload){
		free(ack_arg.pload);
	}

	miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_BUSY, MIIO_OT_ERR_INFO_BUSY, ack, ctx);
#if MIIO_PROV_STATISTIC
	if(is_prov_stat_enable()){
		set_prov_stat_error_code(PROV_ERROR_HANDSHAKE_HELLO_FAIL);
	}
#endif


	return MIIO_ERROR;
}

static uint8_t* public_key_buffer_calloc(size_t size){
	uint8_t * ptr = calloc(1, size + ECDH_OBJECT_RANDOM_SIZE);
	return ptr == NULL ? NULL : ptr + ECDH_OBJECT_RANDOM_SIZE;
}
#define public_key_buffer_free(ptr) \
( (ptr) ? ( free((ptr) - ECDH_OBJECT_RANDOM_SIZE), (ptr) = NULL, (void)0): ((ptr) = NULL, (void)0))

typedef struct{
	int mode;
	uint8_t random[ECDH_OBJECT_RANDOM_SIZE]; //> base64 decoded
	int curve_suite;
	int sign_suite;
	uint8_t * sign; //> base64 decoded
	size_t sign_size;
	/*public_key:
		1) base64 decoded 
		2)the buffer should be alloc by function "public_key_buffer_calloc" and free by "public_key_buffer_free"
		public_key_buffer_calloc: 
			alloc buffer by size = ECDH_OBJECT_RANDOM_SIZE + public_key_size 
			the first ECDH_OBJECT_RANDOM_SIZE bytes will be filled by "random"
			the remain bytes be used for "public_key"
	*/
	uint8_t *public_key; 
	size_t public_key_size;
	uint8_t *certs; //> base64 decoded
	size_t certs_size;
}ecdh_object_req_t;
typedef struct{
	uint8_t *public_key; 
	size_t public_key_size;
	uint8_t *certs; 
	size_t certs_size;
	uint8_t * sign;
	size_t sign_size;
	uint8_t *extent;
	size_t extent_size;
}ecdh_object_response_t;

#define ECDH_OBJECT_REQ_INIT(o) memset(	(o), 0, sizeof(ecdh_object_req_t) )
#define ECDH_OBJECT_REQ_UNINIT(o)\
do{\
	if((o)->sign != NULL){\
		free((o)->sign);\
		(o)->sign = NULL;\
		(o)->sign_size = 0;\
	}\
	if((o)->public_key != NULL){\
		public_key_buffer_free((o)->public_key);\
		(o)->public_key = NULL;\
		(o)->public_key_size = 0;\
	}\
	if((o)->certs != NULL){\
		free((o)->certs);\
		(o)->certs = NULL;\
		(o)->certs_size = 0;\
	}\
	memset((o), 0, sizeof(ecdh_object_req_t));\
}while(0)

#define ECDH_OBJECT_RESPONST_INIT(o) memset(	(o), 0, sizeof(ecdh_object_response_t) )
#define ECDH_OBJECT_RESPONST_UNINIT(o)\
do{\
	if((o)->sign != NULL){\
		free((o)->sign);\
		(o)->sign = NULL;\
		(o)->sign_size = 0;\
	}\
	if((o)->public_key != NULL){\
		public_key_buffer_free((o)->public_key);\
		(o)->public_key = NULL;\
		(o)->public_key_size = 0;\
	}\
	if((o)->certs != NULL){\
		free((o)->certs);\
		(o)->certs = NULL;\
		(o)->certs_size = 0;\
	}\
	if((o)->extent != NULL){\
		free((o)->extent);\
		(o)->extent = NULL;\
		(o)->extent_size = 0;\
	}\
	memset((o), 0, sizeof(ecdh_object_response_t));\
}while(0)

static int parse_ecdh_object_req(miio_rpc_delegate_arg_t *req_arg, ecdh_object_req_t *ecdh_req){
	int ret = -1;
	/*
		ecdh mode:
		0: 无挑战认�?		1: 签名挑战认证
	*/
	ecdh_req->mode = 0; //default 0
	if(MIIO_DELEGATE_JSON != req_arg->type){
		ret = -1;
		LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d" , __FUNCTION__, __LINE__);
		goto _END;
	}
	jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(req_arg->pload, req_arg->pload_len);
	if(MIIO_OK != jsmi_parse_start(&jsmi_parser) ){
		LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d" , __FUNCTION__, __LINE__);
		goto _END;
	}
	do{
		jsmntok_t *ecdh_tok = NULL;
		{
			jsmi_tok_path_t path[] = {
				{
					.key = "params",
					.type = JSMN_OBJECT
				},
				{
					.key = "ecdh",
					.type = JSMN_OBJECT
				}
			};
			ecdh_tok = jsmi_get_tok(&jsmi_parser, NULL, path, NELEMENTS(path));
			if(NULL == ecdh_tok){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d \"params/ecdh\" not found", __FUNCTION__, __LINE__);
				break;
			}
		}
		//> 1. parse "/ecdh/mode"
		{
			jsmi_tok_path_t path[] = {
				{
					.key = "mode",
					.type = JSMN_PRIMITIVE
				}
			};
			if(MIIO_OK != jsmi_get_value_sint(&jsmi_parser, ecdh_tok, path, NELEMENTS(path), &ecdh_req->mode)){
				LOG_DEBUG_TAG(MIIO_LOG_TAG, "%s:%d \"ecdh/mode\" not found",  __FUNCTION__, __LINE__);
			}
		}
		//> 2. parse "/ecdh/random" 	1) c_random_r为配网信息接收方生成�?6字节随机挑战�?2) 无挑战认证时，无需提供此参�?		
		if(ecdh_req->mode != 0)
		{
			int _r = 0;
			jsmi_tok_path_t path[] = {
				{
					.key = "random",
					.type = JSMN_STRING
				}
			};
			uint8_t base64_random[MIIO_BASE64_SIZE(ECDH_OBJECT_RANDOM_SIZE)] = {0};
			if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, ecdh_tok, path, NELEMENTS(path), (char*)base64_random, sizeof(base64_random))){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d \"ecdh/random\" not found",  __FUNCTION__, __LINE__);
				break;
			}
			size_t rdl = ECDH_OBJECT_RANDOM_SIZE;
			_r = mbedtls_base64_decode(ecdh_req->random, rdl, &rdl, base64_random, strnlen((const char*)base64_random, sizeof(base64_random)));
			if(0 != _r){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d mbedtls_base64_decode err code[%d]",  __FUNCTION__, __LINE__, _r );
				break;
			}

			if(rdl != ECDH_OBJECT_RANDOM_SIZE){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d \"ecdh/random\"length not == %d",  __FUNCTION__, __LINE__, ECDH_OBJECT_RANDOM_SIZE);
				break;
			}
		} 
		//> 3. parse "/ecdh/curve_suite"
		{
			jsmi_tok_path_t path[] = {
				{
					.key = "curve_suite",
					.type = JSMN_PRIMITIVE
				}
			};
			if(MIIO_OK != jsmi_get_value_sint(&jsmi_parser, ecdh_tok, path, NELEMENTS(path), &ecdh_req->curve_suite)){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d \"ecdh/curve_suite\" not found", __FUNCTION__, __LINE__);
				break;
			}
		}
		//> 4. parse "/ecdh/sign_suite"
		{
			jsmi_tok_path_t path[] = {
				{
					.key = "sign_suite",
					.type = JSMN_PRIMITIVE
				}
			};
			if(MIIO_OK != jsmi_get_value_sint(&jsmi_parser, ecdh_tok, path, NELEMENTS(path), &ecdh_req->sign_suite)){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d \"ecdh/sign_suite\" not found", __FUNCTION__, __LINE__);
				break;
			}
		}
		//>5.  parse "/ecdh/sign"
		if(ecdh_req->mode != 0){
			int _r = 0;
			/*
				1) mode = 0时：无此参数
				2) mode = 1，sign_suite = 1时：
				sign = BASE64(HMAC_SHA256(c_random_r+PUB_KEY_MAIN_S,PSK))
				3) mode=1，sign_suite=2时：
				sign = BASE64(ECDSA(c_random_r+PUB_KEY_MAIN_S,CERT_PRV_S))
			*/
			jsmi_tok_path_t path[] = {
				{
					.key = "sign",
					.type = JSMN_STRING
				}
			};
			jsmntok_t *sign_tok = jsmi_get_tok(&jsmi_parser, ecdh_tok, path, NELEMENTS(path));
			if(NULL == sign_tok){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d  \"ecdh/sign\" not found",  __FUNCTION__, __LINE__);
				break;
			}
			size_t base64_signature_length = sign_tok->end - sign_tok->start + 1;
			uint8_t * base64_signature = calloc(1, base64_signature_length);
            if (base64_signature == NULL) {
                LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d  base64_signature calloc failed",  __FUNCTION__, __LINE__);
                break;
            }
			if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, sign_tok, NULL, 0, (char*)base64_signature, base64_signature_length)){
				free(base64_signature);
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d \"ecdh/public_key\" not valid", __FUNCTION__, __LINE__);
				break;
			}
			ecdh_req->sign = calloc(1, base64_signature_length);
            if (ecdh_req->sign == NULL) {
                free(base64_signature);
                LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d  ecdh_req->sign calloc failed",  __FUNCTION__, __LINE__);
                break;
            }			
			_r = mbedtls_base64_decode(ecdh_req->sign, base64_signature_length, &ecdh_req->sign_size, base64_signature, strnlen((char*)base64_signature, base64_signature_length));

			free(base64_signature);

			if(0 != _r){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d mbedtls_base64_decode err code[%d]",  __FUNCTION__, __LINE__, _r );
				break;
			}
		}
		//>6. parse "/ecdh/public"
		{
			int _r = 0;
			jsmi_tok_path_t path[] = {
				{
					.key = "public_key",
					.type = JSMN_STRING
				}
			};
			jsmntok_t *public_tok = jsmi_get_tok(&jsmi_parser, ecdh_tok, path, NELEMENTS(path));
			if(NULL == public_tok){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d  \"ecdh/public_key\" not found",  __FUNCTION__, __LINE__);
				break;
			}
			
			size_t public_base64_size = public_tok->end - public_tok->start + 1;
			uint8_t *public_base64 = calloc(1, public_base64_size);
			if(NULL == public_base64){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d no memory",  __FUNCTION__, __LINE__);
				break;
			}
			if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, public_tok, NULL, 0, (char*)public_base64, public_base64_size)){
				free(public_base64);
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d  \"ecdh/public_key\" not valid",  __FUNCTION__, __LINE__);
				break;
			}
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "public_key:'%s'", public_base64);
			ecdh_req->public_key = public_key_buffer_calloc(public_base64_size);
			if(NULL == ecdh_req->public_key){
				free(public_base64);
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d no memory",  __FUNCTION__, __LINE__);
				break;
			}
			_r = mbedtls_base64_decode(ecdh_req->public_key, public_base64_size, &ecdh_req->public_key_size, public_base64, strnlen((char*)public_base64, public_base64_size));
			free(public_base64);
			if(0 != _r){
				public_key_buffer_free(ecdh_req->public_key);
				ecdh_req->public_key = NULL;
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d mbedtls_base64_decode err code[%d]",  __FUNCTION__, __LINE__, _r );
			}
			
		}
		//> 7. certs
		if(ecdh_req->sign_suite == 2){
			/*
				1) sign_suite = 1时：无此参数
				2) sign_suite = 2时：certs = BASE64(CERTS_DER_S)
				其中�?				1) CERTS_DER_S配网信息发送方的DER格式米家证书
			*/
			//int _r = 0;
			jsmi_tok_path_t path[] = {
				{
					.key = "certs",
					.type = JSMN_STRING
				}
			};
			jsmntok_t *certs_tok = jsmi_get_tok(&jsmi_parser, ecdh_tok, path, NELEMENTS(path));
			if(NULL != certs_tok){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d Not Support certs",  __FUNCTION__, __LINE__);
				break;
			}
		}
		ret = 0;
	}while(0);

	jsmi_parse_finish(&jsmi_parser);
	return ret;
_END:
	return ret;
}
static bool ecdh_object_req_verify(ecdh_object_req_t * req, uint8_t* random, size_t random_size, uint8_t *psk, size_t psk_size){
	/*
		1) mode = 0时：无此参数
		2) mode = 1，sign_suite = 1时：
			sign = BASE64(HMAC_SHA256(c_random_r+PUB_KEY_MAIN_S,PSK))
		3) mode=1，sign_suite=2时：
			sign = BASE64(ECDSA(c_random_r+PUB_KEY_MAIN_S,CERT_PRV_S))
		其中�?		1) PSK为配网信息发送方和配网信息接收方的预置共享密�?		2) CERT_PRV_S为配网信息发送方的米家证书私�?		3) �?’表示字节拼�?	*/
	if(req->mode == 0){
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "%s:%d mode[%d] verify pass through",  __FUNCTION__, __LINE__, req->mode);
		return true;
	}
	if(random_size != ECDH_OBJECT_RANDOM_SIZE){
		LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d random_size[%d] Not Support",  __FUNCTION__, __LINE__, random_size);
		return false;
	}
	if(req->mode != 1){
		LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d mode[%d],Not Support",  __FUNCTION__, __LINE__, req->mode);
		return false;
	}
	uint8_t sign[MIIO_SIGN_AN1_LEN_MAX] = {0};
	uint8_t sign_size = MIIO_SIGN_AN1_LEN_MAX;
	uint8_t *pdata = req->public_key - ECDH_OBJECT_RANDOM_SIZE;//see public_key_buffer_calloc
	memcpy(pdata, random, ECDH_OBJECT_RANDOM_SIZE);
	
	if(MIIO_SIGN_HMAC_SHA256 == req->sign_suite){
		int ret = MIIO_OK;
		ret = miio_ciphers_interface_default()->get_sign(NULL, MIIO_SIGN_HMAC_SHA256, psk, pdata, 
					req->public_key_size+ECDH_OBJECT_RANDOM_SIZE, sign, (size_t*)&sign_size, 0);

		if(ret != MIIO_OK){
			return false;
		}
		return ( (sign_size == req->sign_size) && (memcmp(sign, req->sign, sign_size) == 0) ) ? true : false;
	}
	if(MIIO_SIGN_PK == req->sign_suite){
		LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d sign_suite[MIIO_SIGN_PK] Not Support",  __FUNCTION__, __LINE__);
		return false;
	}
	return false;
}
static uint8_t *g_PRV_KEY_EXT_R = NULL;
static size_t g_PRV_KEY_EXT_R_size = 0;
uint8_t* get_ecc_pri_key_ext_r(size_t *psize){
	*psize = g_PRV_KEY_EXT_R_size;
	return g_PRV_KEY_EXT_R;
}
/**
 * @brief 
 *  PUB_KEY_MAIN_R g_ecdh_handle[0]
 *  PUB_KEY_EXT_R g_ecdh_handle[1]
 */
static miio_ecdh_handle_t g_ecdh_handle[2] = {0, 0};
#define FREE_ECDH_RESOURCE(handle) (miio_ciphers_ecdh_valid(handle) ? miio_ciphers_ecdh_finsh(handle) : (void)0, (handle) = 0)

miio_ecdh_handle_t miio_ecdh_ext_handle_get(){
    return g_ecdh_handle[1];
}
void miio_ecdh_ext_handle_free(){
    FREE_ECDH_RESOURCE(g_ecdh_handle[1]);
}
static int build_ecdh_object_response(const ecdh_object_req_t * req, ecdh_object_response_t * response){
	int ret = -1;
	int iret = -1;
	uint8_t* PUB_KEY_EXT_R = NULL;
    uint8_t main_share_key[16];
    size_t main_share_key_size = sizeof(main_share_key);

	do{
		//> 1. Generate main ECC keypair (PUB_KEY_MAIN_R, PRV_KEY_MAIN_R)
        response->public_key_size = req->public_key_size;
        response->public_key = public_key_buffer_calloc(response->public_key_size);
        if (response->public_key == NULL) {
            LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d public_key calloc failed",  __FUNCTION__, __LINE__);
            break;
        } 
        ret = miio_ciphers_ecdh_generate_key(&g_ecdh_handle[0], req->curve_suite, response->public_key, &response->public_key_size); 
        uint8_t* PUB_KEY_MAIN_R = response->public_key;
		size_t PUB_KEY_MAIN_R_length = response->public_key_size;

        if(ret != MIIO_OK){
            break;
        }

		// 2. Generate expand ECC keypair (PUB_KEY_EXT_R, PRV_KEY_EXT_R)
        size_t PUB_KEY_EXT_R_length = response->public_key_size;
        PUB_KEY_EXT_R = calloc(1, PUB_KEY_EXT_R_length);
        if (PUB_KEY_EXT_R == NULL) {
            LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d PUB_KEY_EXT_R calloc failed",  __FUNCTION__, __LINE__);
            break;
        }    
        ret = miio_ciphers_ecdh_generate_key(&g_ecdh_handle[1], req->curve_suite,PUB_KEY_EXT_R, &PUB_KEY_EXT_R_length);
		if(ret != MIIO_OK){
            break;
        }
		
        //> 3. //Generate share key from "PRV_KEY_MAIN_R" and "PUB_KEY_MAIN_S"
        ret = miio_ciphers_ecdh_establish_key(g_ecdh_handle[0], 
                                                req->public_key, 
                                                req->public_key_size, 
                                                main_share_key, 
                                                &main_share_key_size,
                                                true);
        if(ret != MIIO_OK){
            break;
        }                                               
		miio_ciphers_set_safekey(main_share_key);
#if _MIIO_HANDSHAKE_DEBUG
		arch_dump_hex(SHARE_KEY_MAIN_IKM, SHARE_KEY_MAIN_IKM_length, "ecdh_share_key");
		arch_dump_hex(main_share_key, sizeof(main_share_key), "hkdf(ecdh_share_key)");
#endif
		
		//> 1. gen sign:
		response->sign_size = MIIO_BASE64_SIZE(MIIO_SIGN_AN1_LEN_MAX);
		response->sign = calloc(1, response->sign_size);
        if (response->sign == NULL) {
            LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d sign calloc failed",  __FUNCTION__, __LINE__);
            break;
        }

		if(req->mode == 0){
			//> sign = HMAC_SHA256(PUB_KEY_MAIN_R,SHARE_KEY_MAIN)
			if(MIIO_OK != miio_ciphers_interface_default()->get_sign(NULL, req->sign_suite, main_share_key,
                                                                        PUB_KEY_MAIN_R, PUB_KEY_MAIN_R_length, 
                                                                        response->sign, &response->sign_size, 0)){

				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d miio_sign failed",  __FUNCTION__, __LINE__);
				break;
			}
		
		}
		if(req->mode == 1 && req->sign_suite == 1){
			//> sign = HMAC_SHA256(c_random_s+PUB_KEY_MAIN_R,PSK)
			uint8_t  PSK[16] = {0};
			miio_ciphers_interface_default()->get_psk(NULL, PSK);
			memcpy(response->public_key - ECDH_OBJECT_RANDOM_SIZE, req->random, ECDH_OBJECT_RANDOM_SIZE);
			if(MIIO_OK != miio_ciphers_interface_default()->get_sign(NULL, req->sign_suite, PSK,
                                                                    PUB_KEY_MAIN_R-ECDH_OBJECT_RANDOM_SIZE,
                                                                    ECDH_OBJECT_RANDOM_SIZE+PUB_KEY_MAIN_R_length, 
                                                                    response->sign, 
                                                                    &response->sign_size, 
                                                                    0)){

				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d miio_sign failed",  __FUNCTION__, __LINE__);
				break;
			}
		}
		if(req->mode == 1 && req->sign_suite == 2){
			//> sign = ECDSA(c_random_s+PUB_KEY_MAIN_R,CERT_PRV_R)
			LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d miio_sign not supported!!!",  __FUNCTION__, __LINE__);
			break;
		}

		//>2. gen extent:
			//>2.1 gen ecdh_ext: public_key string BASE64(PUB_KEY_EXT_R)
				//>2.1.1 gen BASE64(PUB_KEY_EXT_R)
				LOG_DEBUG_TAG(MIIO_LOG_TAG, "gen extent 2.1.1");
				size_t BASE64_PUB_KEY_EXT_R_size = MIIO_BASE64_SIZE(PUB_KEY_EXT_R_length);
				uint8_t *BASE64_PUB_KEY_EXT_R = calloc(1, BASE64_PUB_KEY_EXT_R_size);
                if (BASE64_PUB_KEY_EXT_R == NULL) {
                    LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d BASE64_PUB_KEY_EXT_R calloc failed",  __FUNCTION__, __LINE__);
                    break;
                }                
				if(0 != mbedtls_base64_encode(BASE64_PUB_KEY_EXT_R, BASE64_PUB_KEY_EXT_R_size,
											 &BASE64_PUB_KEY_EXT_R_size,PUB_KEY_EXT_R, PUB_KEY_EXT_R_length)){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d mbedtls_base64_encode failed",  __FUNCTION__, __LINE__);
					free(BASE64_PUB_KEY_EXT_R);
					break;
				}
				//>2.1.2 format to ecdh_ext
				LOG_DEBUG_TAG(MIIO_LOG_TAG, "gen extent 2.1.2 BASE64_PUB_KEY_EXT_R:%s", BASE64_PUB_KEY_EXT_R);
				size_t ecdh_ext_size = (64 + BASE64_PUB_KEY_EXT_R_size + 0x10) & 0x7FFFFFF0;//
				uint8_t *ecdh_ext = calloc(1, ecdh_ext_size);
                if (ecdh_ext == NULL) {
                    free(BASE64_PUB_KEY_EXT_R);
                    LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d ecdh_ext calloc failed",  __FUNCTION__, __LINE__);
                    break;
                }

				ecdh_ext_size = sprintf((char*)ecdh_ext, "{\"public_key\":\"%s\"}", BASE64_PUB_KEY_EXT_R);
				free(BASE64_PUB_KEY_EXT_R);
#if _MIIO_HANDSHAKE_DEBUG				
				arch_dump_hex(ecdh_ext, ecdh_ext_size, "ecdh_ext");
#endif				
			//>2.2 gen extent 
				//> 2.2.1 PKCS7Padding

				ecdh_ext_size += PKCS7_padding(ecdh_ext, ecdh_ext_size, 0x10);

#if _MIIO_HANDSHAKE_DEBUG
				arch_dump_hex(ecdh_ext, ecdh_ext_size, "PKCS7(ecdh_ext)");
#endif				
				//>2.2.2 extent = AES_CBC_ENCRYPT(PKCS7(ecdh_ext), SHARE_KEY_MAIN, 0)
				LOG_DEBUG_TAG(MIIO_LOG_TAG, "gen extent 2.2.2");
				int aes_ret = 0;
				mbedtls_aes_context aes;
				mbedtls_aes_init(&aes);
				aes_ret = mbedtls_aes_setkey_enc(&aes, main_share_key, sizeof(main_share_key) * 8);
				if(0 != aes_ret){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d mbedtls_aes_setkey_enc failed code[%d]",  __FUNCTION__, __LINE__, aes_ret);
					mbedtls_aes_free(&aes);
					free(ecdh_ext);
					break;
				}
				unsigned char iv[16] = {0};
				response->extent_size = ecdh_ext_size;
				response->extent = calloc(1, response->extent_size);
                if (response->extent == NULL) {
                    free(ecdh_ext);
                    mbedtls_aes_free(&aes);
                    LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d response->extent calloc failed",  __FUNCTION__, __LINE__);
                    break;
                }

				LOG_DEBUG_TAG(MIIO_LOG_TAG, "extent_size %d", response->extent_size);
				aes_ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, response->extent_size, iv, ecdh_ext, response->extent );
				free(ecdh_ext);
				mbedtls_aes_free(&aes);
				if(0 != aes_ret){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d mbedtls_aes_crypt_cbc failed code[%d]",  __FUNCTION__, __LINE__, aes_ret);
					break;
				}
#ifdef _MIIO_HANDSHAKE_DEBUG				
				arch_dump_hex(req->public_key, req->public_key_size, "req public");
				arch_dump_hex(response->public_key, response->public_key_size, "response public");				
				arch_dump_hex(response->extent, response->extent_size, "AES_CBC_ENCRYPT(PKCS7(ecdh_ext))");
				arch_dump_hex(main_share_key, sizeof(main_share_key), "SHARE_KEY_MAIN");
#endif				
		iret = 0;
	}while(0);
	
    NULL != PUB_KEY_EXT_R ? free(PUB_KEY_EXT_R) : (void)0;
    //free PUB_KEY_MAIN_R g_ecdh_handle[0]
    FREE_ECDH_RESOURCE(g_ecdh_handle[0]);
    
    iret != 0 ? FREE_ECDH_RESOURCE(g_ecdh_handle[1]) : (void)0;

	return iret;

}
static int format_object_to_ack(const ecdh_object_response_t * response, miio_rpc_delegate_arg_t *pack_arg){
	jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(pack_arg->pload, pack_arg->pload_len);
	jsmi_compose_start(&jsmi_composer);
	jsmi_set_object_begin(&jsmi_composer);
		jsmi_set_key_value_uint(&jsmi_composer, "id", pack_arg->id);
		jsmi_set_key_object_begin(&jsmi_composer, "result");
			jsmi_set_key_value_sint(&jsmi_composer, "type", MIIO_HANDSHAKE_TYPE_ECDH);
			jsmi_set_key_object_begin(&jsmi_composer, "ecdh");
				//> 1. format public_key
				{
					size_t public_base64_len = MIIO_BASE64_SIZE(response->public_key_size);
					uint8_t *public_base64 = calloc(1, public_base64_len);
					if(NULL == public_base64){
						LOG_ERROR_TAG(MIIO_LOG_TAG, "no memory");
						goto error_exit;
					}
					if(0 != mbedtls_base64_encode(public_base64, public_base64_len, &public_base64_len, response->public_key, response->public_key_size)){
						LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_base64_encode failed");
						free(public_base64);
						goto error_exit;
					}
					jsmi_set_key_value_str(&jsmi_composer, "public_key", (char*)public_base64, public_base64_len);
					free(public_base64);
				}
				
				//> 2. format cert
				if(response->certs_size > 0){
					jsmi_set_key_value_str(&jsmi_composer, "cert", (char*)response->certs, 0);
				}
				//> 3. format sign
				{
					uint8_t sign_base64[MIIO_BASE64_SIZE(MIIO_SIGN_AN1_LEN_MAX)] = {0};
					size_t sign_base64_len = sizeof(sign_base64);

					if(0 != mbedtls_base64_encode(sign_base64, sign_base64_len, &sign_base64_len, response->sign, response->sign_size)){
						LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_base64_encode failed");
						goto error_exit;
					}
					jsmi_set_key_value_str(&jsmi_composer, "sign", (char*)sign_base64, sign_base64_len);
				}
				//> 4. format extent
				{
					size_t extent_base64_size = MIIO_BASE64_SIZE(response->extent_size);
					uint8_t *extent_base64 = calloc(1, extent_base64_size);

					if(0 != mbedtls_base64_encode(extent_base64, extent_base64_size, &extent_base64_size, response->extent, response->extent_size)){
						LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_base64_encode failed");
						free(extent_base64);
						goto error_exit;
					}
					jsmi_set_key_value_str(&jsmi_composer, "extent", (char*)extent_base64, extent_base64_size);
					free(extent_base64);
				}
			jsmi_set_key_object_end(&jsmi_composer);
		jsmi_set_key_object_end(&jsmi_composer);
	jsmi_set_object_end(&jsmi_composer);
	
	return jsmi_compose_finish(&jsmi_composer, NULL, &pack_arg->pload_len) == MIIO_OK ? 0 : -1;

error_exit:
	return -1;
}
static inline bool is_suite_supported(const int *sign_suites, size_t sign_suites_len, int sign_suite)
{
	for(size_t i=0; i < sign_suites_len; i++){
		if(sign_suites[i] == sign_suite){
			return true;
		}
	}

	return false;
}

static int handshake_ecdh(miio_rpc_delegate_arg_t *req_arg, miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
	miio_handle_t miio_handle = miio_from_delegtate_ctx(ctx);
	int err_code = MIIO_OT_ERR_CODE_UNDEF_ERROR;
	const char *err_msg = MIIO_OT_ERR_INFO_UNDEF_ERROR;
	miio_rpc_delegate_arg_t ack_arg = {
		.id = req_arg->id ,
		.type = req_arg->type,
		.pload_len = 0,
		.pload = NULL
	};
#if MIIO_PROV_STATISTIC
	if(is_prov_stat_enable()){
		set_prov_stat_stage(PROV_STAGE_HANDSHAKE_ECDH);
	}
#endif

    FREE_ECDH_RESOURCE(g_ecdh_handle[0]/*ecdh main*/);
    FREE_ECDH_RESOURCE(g_ecdh_handle[1]/*ecdh ext*/);
	//> 1. parse ecdh req object
	ecdh_object_req_t ecdh_object_req;
	ecdh_object_response_t ecdh_object_response;
	ECDH_OBJECT_REQ_INIT(&ecdh_object_req);
	ECDH_OBJECT_RESPONST_INIT(&ecdh_object_response);

	if(0 != parse_ecdh_object_req(req_arg, &ecdh_object_req)){
		err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
		err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
		goto error_exit;
	}

	//> 2. check if it is supported
    int _sign_suites[10];
    int _sign_suites_size = miio_ciphers_interface_default()->get_sign_suites(_sign_suites, NELEMENTS(_sign_suites));
	if(   !is_suite_supported(_sign_suites, _sign_suites_size, ecdh_object_req.sign_suite) ||
		!is_suite_supported(miio_curve_suites_default, NELEMENTS(miio_curve_suites_default), ecdh_object_req.curve_suite) ||
		NULL == ecdh_object_req.public_key ){
		err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
		err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
		LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d this type req unsupported", __FUNCTION__, __LINE__);
		goto error_exit;
	}

	if( !miio_ecdh_mode_test(ecdh_object_req.mode) ){
		err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
		err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
		LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d the ecdh mode unsupported", __FUNCTION__, __LINE__);
		goto error_exit;
	}

	//>3. verify sign when it presence
	uint8_t  psk[16] = {0};
	arch_get_psk(psk);
	bool verify = ecdh_object_req_verify( &ecdh_object_req, _c_random_r, sizeof(_c_random_r), psk, sizeof(psk));
	if(!verify){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "verify sign error");
		err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
		err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
		goto error_exit;
	}

	
	//> 4. build ecdh_object_response_t
	LOG_DEBUG_TAG(MIIO_LOG_TAG, "build ecdh_object_response_t");

	if(0 != build_ecdh_object_response(&ecdh_object_req, &ecdh_object_response)){
		ECDH_OBJECT_RESPONST_UNINIT(&ecdh_object_response);
		err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
		err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
		goto error_exit;
	}

	//> 5. construct ack packet
	LOG_DEBUG_TAG(MIIO_LOG_TAG, "construct ack packet");
	ack_arg.pload_len = miio_get_rpc_max_size(miio_handle);
	ack_arg.pload = (char *) malloc(ack_arg.pload_len);
	if(!ack_arg.pload){
		err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
		err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
		goto error_exit;
	}

	if(0 != format_object_to_ack(&ecdh_object_response, &ack_arg)){
		err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
		err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
		goto error_exit;
	}
	miio_net_auto_provision_channel_lock();

	ack(&ack_arg, ctx);
	free(ack_arg.pload);

	ECDH_OBJECT_REQ_UNINIT(&ecdh_object_req);
	ECDH_OBJECT_RESPONST_UNINIT(&ecdh_object_response);

	return MIIO_OK;
error_exit:
	
#if MIIO_PROV_STATISTIC
		if(is_prov_stat_enable()){
			set_prov_stat_error_code(PROV_ERROR_HANDSHAKE_ECDH_FAIL);
		}
#endif

	if(ack_arg.pload){
		free(ack_arg.pload);
	}

	miio_rpc_delegate_ack_error(req_arg, err_code, err_msg, ack, ctx);
	ECDH_OBJECT_REQ_UNINIT(&ecdh_object_req);
	ECDH_OBJECT_RESPONST_UNINIT(&ecdh_object_response);
    FREE_ECDH_RESOURCE(g_ecdh_handle[0]);
    FREE_ECDH_RESOURCE(g_ecdh_handle[1]);
	return MIIO_ERROR;
}

#if MIIO_HANDSHAKE_TEST
static mbedtls_ecdh_context s_test_ecdh_context = {0};

static int handshake_test_public(miio_rpc_delegate_arg_t *req_arg, miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
	miio_handle_t miio_handle = miio_from_delegtate_ctx(ctx);
	int err_code = MIIO_OT_ERR_CODE_UNDEF_ERROR;
	const char *err_msg = MIIO_OT_ERR_INFO_UNDEF_ERROR;
	miio_rpc_delegate_arg_t ack_arg = {
		.id = req_arg->id ,
		.type = req_arg->type,
		.pload_len = 0,
		.pload = NULL
	};

	int curve_suite = MIIO_CURVE_NONE;
	int sign_suite = MIIO_SIGN_NONE;
	const miio_curve_info_t *curve_info = NULL;
	uint8_t *public = NULL;
	size_t public_len = 0;

	if(MIIO_DELEGATE_JSON == req_arg->type){
		jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(req_arg->pload, req_arg->pload_len);
		if(MIIO_OK == jsmi_parse_start(&jsmi_parser)){
			jsmntok_t *ecdh_tok = NULL;
			{
				jsmi_tok_path_t path[] = {
					{
						.key = "params",
						.type = JSMN_OBJECT
					},
					{
						.key = "ecdh",
						.type = JSMN_OBJECT
					}
				};
				ecdh_tok = jsmi_get_tok(&jsmi_parser, NULL, path, NELEMENTS(path));
				if(NULL == ecdh_tok){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "\"params/ecdh\" not found");
					goto jsmi_parse_exit;
				}
			}
			// /ecdh/curve_suite
			{
				jsmi_tok_path_t path[] = {
					{
						.key = "curve_suite",
						.type = JSMN_PRIMITIVE
					}
				};
				if(MIIO_OK != jsmi_get_value_sint(&jsmi_parser, ecdh_tok, path, NELEMENTS(path), &curve_suite)){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "\"ecdh/curve_suite\" not found");
					goto jsmi_parse_exit;
				}
			}
			// /ecdh/sign_suite
			{
				jsmi_tok_path_t path[] = {
					{
						.key = "sign_suite",
						.type = JSMN_PRIMITIVE
					}
				};
				if(MIIO_OK != jsmi_get_value_sint(&jsmi_parser, ecdh_tok, path, NELEMENTS(path), &sign_suite)){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "\"ecdh/sign_suite\" not found");
					goto jsmi_parse_exit;
				}
			}
jsmi_parse_exit:
			jsmi_parse_finish(&jsmi_parser);
		}
	}

	if(   !is_suite_supported(miio_sign_suites_default, NELEMENTS(miio_sign_suites_default), sign_suite)
		||!is_suite_supported(miio_curve_suites_default, NELEMENTS(miio_curve_suites_default), curve_suite)
		||!(curve_info = miio_curve_info(curve_suite)) ){
		err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
		err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
		goto error_exit;
	}

	//gen public
	{
		int ret = 0;

		mbedtls_ecdh_free(&s_test_ecdh_context);
		mbedtls_ecdh_init(&s_test_ecdh_context);
		ret = mbedtls_ecp_group_load(&s_test_ecdh_context.grp, curve_info->lib_id);
		if(0 != ret){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_ecp_group_load returned %d", ret);
			goto error_exit;
		}

		//generate keypair
		ret = mbedtls_ecdh_gen_public( &s_test_ecdh_context.grp, &s_test_ecdh_context.d, &s_test_ecdh_context.Q, get_random, NULL );
		if(0 != ret){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_ecdh_gen_public returned %d", ret);
			goto error_exit;
		}

		size_t plen = mbedtls_mpi_size( &s_test_ecdh_context.grp.P );
#if MIIO_HANDSHAKE_PUBKEY_RAW
		public_len = 2 * plen;
#else
		public_len = 2 * plen + 1;
#endif
		public = malloc(public_len);
		if(NULL == public){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "no memory");
			goto error_exit;
		}
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "public_len = %d", public_len);

#if MIIO_HANDSHAKE_PUBKEY_RAW
        ret = mbedtls_mpi_write_binary( &s_test_ecdh_context.Q.X, public, plen );
        if(0 != ret ){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_mpi_write_binary returned %d", ret);
			goto error_exit;
		}
        ret = mbedtls_mpi_write_binary( &s_test_ecdh_context.Q.Y, public + plen, plen );
        if(0 != ret ){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_mpi_write_binary returned %d", ret);
			goto error_exit;
		}
#else
		//write device public point to uint8*, not compressed
		ret = mbedtls_ecp_point_write_binary( &s_test_ecdh_context.grp, &s_test_ecdh_context.Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &public_len, public, public_len);
		if(0 != ret){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_ecp_point_write_binary returned %d", ret);
			goto error_exit;
		}
#endif
	}

	ack_arg.pload_len = miio_get_rpc_max_size(miio_handle);
	ack_arg.pload = (char *) malloc(ack_arg.pload_len);
	if (ack_arg.pload) {
		jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(ack_arg.pload, ack_arg.pload_len);
		jsmi_compose_start(&jsmi_composer);
		jsmi_set_object_begin(&jsmi_composer);
			jsmi_set_key_value_uint(&jsmi_composer, "id", ack_arg.id);
			jsmi_set_key_object_begin(&jsmi_composer, "result");
				jsmi_set_key_value_sint(&jsmi_composer, "type", (MIIO_HANDSHAKE_TYPE_ECDH+1));
				jsmi_set_key_object_begin(&jsmi_composer, "ecdh");
					jsmi_set_key_value_sint(&jsmi_composer, "curve_suite", curve_suite);
					jsmi_set_key_value_sint(&jsmi_composer, "sign_suite", sign_suite);
					{
						//public_key
						size_t public_base64_len = 0;
						uint8_t *public_base64 = calloc(1, MIIO_BASE64_SIZE(public_len));
						if(NULL == public_base64){
							goto error_exit;
						}
						mbedtls_base64_encode(public_base64, MIIO_BASE64_SIZE(public_len), &public_base64_len, public, public_len);
						jsmi_set_key_value_str(&jsmi_composer, "public_key", (char*)public_base64, public_base64_len);
						free(public_base64);
					}
				jsmi_set_key_object_end(&jsmi_composer);
			jsmi_set_key_object_end(&jsmi_composer);
		jsmi_set_object_end(&jsmi_composer);
		if(MIIO_OK == jsmi_compose_finish(&jsmi_composer, NULL, &ack_arg.pload_len)){
			free(public);
			ack(&ack_arg, ctx);
			free(ack_arg.pload);
			return MIIO_OK;
		}
	}

error_exit:

	mbedtls_ecdh_free(&s_test_ecdh_context);
	mbedtls_ecdh_init(&s_test_ecdh_context);

	if(public){
		free(public);
	}

	if(ack_arg.pload){
		free(ack_arg.pload);
	}

	miio_rpc_delegate_ack_error(req_arg, err_code, err_msg, ack, ctx);

	return MIIO_ERROR;
}

static int handshake_test_share_key(miio_rpc_delegate_arg_t *req_arg, miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
	int err_code = MIIO_OT_ERR_CODE_UNDEF_ERROR;
	const char *err_msg = MIIO_OT_ERR_INFO_UNDEF_ERROR;

	uint8_t *public = NULL;
	size_t public_len = 0;

	if(MIIO_DELEGATE_JSON == req_arg->type){
		jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(req_arg->pload, req_arg->pload_len);
		if(MIIO_OK == jsmi_parse_start(&jsmi_parser)){
			jsmntok_t *ecdh_tok = NULL;
			{
				jsmi_tok_path_t path[] = {
					{
						.key = "params",
						.type = JSMN_OBJECT
					},
					{
						.key = "ecdh",
						.type = JSMN_OBJECT
					}
				};
				ecdh_tok = jsmi_get_tok(&jsmi_parser, NULL, path, NELEMENTS(path));
				if(NULL == ecdh_tok){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "\"params/ecdh\" not found");
					goto jsmi_parse_exit;
				}
			}
			// /ecdh/public_key
			{
				jsmi_tok_path_t path[] = {
					{
						.key = "public_key",
						.type = JSMN_STRING
					}
				};
				jsmntok_t *public_tok = jsmi_get_tok(&jsmi_parser, ecdh_tok, path, NELEMENTS(path));
				if(NULL == public_tok){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "\"ecdh/public\" not found");
					goto jsmi_parse_exit;
				}

				size_t public_base64_size = public_tok->end - public_tok->start + 1;
				uint8_t *public_base64 = calloc(1, public_base64_size);
				if(NULL == public_base64){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "no memory");
					goto jsmi_parse_exit;
				}
				if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, public_tok, NULL, 0, (char*)public_base64, public_base64_size)){
					free(public_base64);
					LOG_ERROR_TAG(MIIO_LOG_TAG, "\"ecdh/public\" not valid");
					goto jsmi_parse_exit;
				}

				public = calloc(1, public_base64_size);
				if(public){
					if(0 != mbedtls_base64_decode(public, public_base64_size, &public_len, public_base64, strnlen((char*)public_base64, public_base64_size))){
						free(public);
						public = NULL;
						public_len = 0;
						LOG_ERROR_TAG(MIIO_LOG_TAG, "\"ecdh/public\" not valid");
					}
				}
				else{
					LOG_ERROR_TAG(MIIO_LOG_TAG, "no memory");
					public_len = 0;
				}

				free(public_base64);
			}
jsmi_parse_exit:
			jsmi_parse_finish(&jsmi_parser);
		}
	}

	if(NULL == public){
		goto error_exit;
	}

	{
		int ret = 0;
#if MIIO_HANDSHAKE_PUBKEY_RAW
		size_t plen = mbedtls_mpi_size( &s_test_ecdh_context.grp.P );
		if(public_len < 2*plen){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "ecp invalid");
			err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
			err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
			goto error_exit;
		}

		ret = mbedtls_mpi_read_binary( &s_test_ecdh_context.Qp.X, public, plen );
		if(0 != ret ){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_mpi_read_binary returned %d", ret);
			goto error_exit;
		}
		ret = mbedtls_mpi_read_binary( &s_test_ecdh_context.Qp.Y, public + plen, plen );
		if(0 != ret ){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_mpi_read_binary returned %d", ret);
			goto error_exit;
		}
		ret = mbedtls_mpi_lset( &s_test_ecdh_context.Qp.Z, 1 );
		if(0 != ret ){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_mpi_lset returned %d", ret);
			goto error_exit;
		}
#else
		//read peer point from uint8* buf[0]+MPI, must not compressed, (SEC1 2.3.4)
		ret = mbedtls_ecp_point_read_binary( &s_test_ecdh_context.grp, &s_test_ecdh_context.Qp, public, public_len);
		if(0 != ret ){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_ecp_point_read_binary returned %d", ret);
			err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
			err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
			goto error_exit;
		}
#endif
		//compute share key
#if 0
		ret = mbedtls_ecdh_compute_shared( &s_test_ecdh_context.grp, &s_test_ecdh_context.z, &s_test_ecdh_context.Qp, &s_test_ecdh_context.d, get_random, NULL);
		if(0 != ret){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_ecdh_compute_shared returned %d", ret);
			goto error_exit;
		}
#endif
	
		//write share key point to uint8*
		//int ecdh_key_len = mbedtls_mpi_size(&s_test_ecdh_context.z);
		int ecdh_key_len = /*mbedtls_mpi_size(&s_test_ecdh_context.z)*/128;
		uint8_t *ecdh_key = malloc(ecdh_key_len);
        if (ecdh_key == NULL) {
            LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d ecdh_key calloc failed",  __FUNCTION__, __LINE__);
            goto error_exit;;
        }

		ret = mbedtls_ecdh_calc_secret(&s_test_ecdh_context, &ecdh_key_len, ecdh_key, ecdh_key,  get_random, NULL);
		if(0 != ret){
			free(ecdh_key);
			LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_ecdh_calc_secret returned %d", ret);
			goto error_exit;
		}

		ret = mbedtls_mpi_write_binary( &s_test_ecdh_context.z, ecdh_key, ecdh_key_len);
		if(0 != ret){
			free(ecdh_key);
			LOG_ERROR_TAG(MIIO_LOG_TAG, "mbedtls_mpi_write_binary returned %d", ret);
			goto error_exit;
		}

		//compute aes key
		uint8_t share_key[16] = {0};
		uint8_t salt[sizeof(share_key)*2+1] = {0};
		get_token(NULL, share_key);
		snprintf_hex((char*)salt, sizeof(salt), share_key, sizeof(share_key), 0);
		ret = hkdf(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), salt, sizeof(salt)-1, ecdh_key, ecdh_key_len, NULL, 0, share_key, sizeof(share_key));
		if(MIIO_OK != ret){
			free(ecdh_key);
			LOG_ERROR_TAG(MIIO_LOG_TAG, "hkdf returned %d", ret);
			goto error_exit;
		}
		free(ecdh_key);
		arch_dump_hex(share_key, sizeof(share_key), "hkdf(ecdh_hare_key)");

		//sign
		uint8_t sign_base64[MIIO_BASE64_SIZE(32)] = {0};
		size_t sign_base64_len = sizeof(sign_base64);
		miio_sign(NULL, MIIO_SIGN_HMAC_SHA256, share_key, public, public_len, sign_base64, &sign_base64_len, 1);
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "sign = %s", sign_base64);
	}

	mbedtls_ecdh_free(&s_test_ecdh_context);
	mbedtls_ecdh_init(&s_test_ecdh_context);

	free(public);

	miio_rpc_delegate_ack_ok(req_arg, ack, ctx);

	return MIIO_OK;

error_exit:

	mbedtls_ecdh_free(&s_test_ecdh_context);
	mbedtls_ecdh_init(&s_test_ecdh_context);

	if(public){
		free(public);
	}

	miio_rpc_delegate_ack_error(req_arg, err_code, err_msg, ack, ctx);

	return MIIO_ERROR;
}
#endif

static int do_handshake(miio_rpc_delegate_arg_t *req_arg, miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
	int type = 0;

	if(MIIO_DELEGATE_JSON == req_arg->type){
		jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(req_arg->pload, req_arg->pload_len);
		if(MIIO_OK == jsmi_parse_start(&jsmi_parser)){
			// /params/type
			{
				jsmi_tok_path_t path[] = {
					{
						.key = "params",
						.type = JSMN_OBJECT
					},
					{
						.key = "type",
						.type = JSMN_PRIMITIVE
					}
				};
				if(MIIO_OK != jsmi_get_value_sint(&jsmi_parser, NULL, path, NELEMENTS(path), &type)){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "\"params/type\" not found");
					goto jsmi_parse_exit;
				}
			}
jsmi_parse_exit:
			jsmi_parse_finish(&jsmi_parser);
		}
	}

	switch(type){
	case MIIO_HANDSHAKE_TYPE_HELLO:
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "%s:%d MIIO_HANDSHAKE_TYPE_HELLO ...", __FUNCTION__,__LINE__);
		handshake_hello(req_arg, ack, ctx);
		break;
	case MIIO_HANDSHAKE_TYPE_ECDH:
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "%s:%d MIIO_HANDSHAKE_TYPE_ECDH ...", __FUNCTION__,__LINE__);
		handshake_ecdh(req_arg, ack, ctx);
		break;
#if MIIO_HANDSHAKE_TEST
	case (MIIO_HANDSHAKE_TYPE_ECDH+1):
		handshake_test_public(req_arg, ack, ctx);
		break;
	case (MIIO_HANDSHAKE_TYPE_ECDH+2):
		handshake_test_share_key(req_arg, ack, ctx);
		break;
#endif
	default:
		miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID, MIIO_OT_ERR_INFO_PARAM_INVALID, ack, ctx);
		break;
	}

	return MIIO_OK;
}
//MIIO_RPC(handshake, do_handshake, NULL);
MIIO_RPC_ACM(handshake, do_handshake, NULL, MIIO_RPC_ACM_NONE);
#endif
