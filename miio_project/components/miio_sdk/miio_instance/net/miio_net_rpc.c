#include "miio_net.h"
#include "miio_ciphers.h"
#include "miio_handshake_rpc.h"
#include "mbedtls/base64.h"
#include "jsmi.h"
#include "mbedtls/ecdh.h"
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
#define MIIO_LOG_TAG			"miio_net"

//{"method":"miIO.config_router","params":{"ssid":"xxxxxx","passwd":"xxxxx","country_domain":"cn","gmt_offset":0,"tz":""}}
static int do_config_router(miio_rpc_delegate_arg_t* req_arg, miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
	int err_code = MIIO_OK;
	const char *err_msg = NULL;
	const char *params = NULL;
	int params_len = 0;
#if MIIO_PROV_STATISTIC
	if(is_prov_stat_enable()){
		set_prov_stat_stage(PROV_STAGE_CONFIG_ROUTER);
	}
#endif

	if(MIIO_DELEGATE_JSON == req_arg->type){
		jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(req_arg->pload, req_arg->pload_len);
		if(MIIO_OK == jsmi_parse_start(&jsmi_parser)){
			{
				// /params
				jsmi_tok_path_t path[] = {
					{
						.key = "params",
						.type = JSMN_OBJECT
					}
				};
				jsmntok_t *params_tok = jsmi_get_tok(&jsmi_parser, NULL, path, NELEMENTS(path));
				if(NULL == params_tok){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "\"params\" not found");
					goto jsmi_parse_exit;
				}
				params = jsmi_parser.js + params_tok->start;
				params_len = params_tok->end - params_tok->start;
			}
jsmi_parse_exit:
			jsmi_parse_finish(&jsmi_parser);
		}
		else{
			err_code = MIIO_OT_ERR_CODE_BUSY;
			err_msg = MIIO_OT_ERR_INFO_BUSY;
			goto err_exit;
		}
	}

	if(NULL == params || params_len <= 0){
		LOG_ERROR_TAG(MIIO_LOG_TAG, "\"params\" is empty!");
		err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
		err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
		goto err_exit;
	}

	if(MIIO_OK == miio_net_config_router(miio_from_delegtate_ctx(ctx), params, params_len, NULL, 0)){
		miio_rpc_delegate_ack_ok(req_arg, ack, ctx);
#if MIIO_PROV_STATISTIC
		if(is_prov_stat_enable()){
			set_prov_stat_error_code(PROV_ERROR_NONE);
		}
#endif
		return MIIO_OK;
	}
	else{
		err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
		err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
	}

err_exit:

	miio_rpc_delegate_ack_error(req_arg, err_code, err_msg, ack, ctx);
#if MIIO_PROV_STATISTIC
	if(is_prov_stat_enable()){
		set_prov_stat_error_code(PROV_ERROR_CONFIG_ROUTER_PARAM_INVAILD);
	}
#endif


    return MIIO_ERROR;
}
//MIIO_RPC(config_router, do_config_router, "config_router");
MIIO_RPC_ACM(config_router, do_config_router, "config_router", MIIO_RPC_ACM_NONE);
#if MIIO_CONFIG_ROUTER_SAFE_ENABLE

//{"method":"miIO.config_router_safe","params":{"data":"xxxxxx", "sign":"xxxxx"}}

static int do_config_router_safe(miio_rpc_delegate_arg_t* req_arg, miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
	int err_code = MIIO_OK;
	const char *err_msg = NULL;

	uint8_t* data = NULL;
	size_t data_len = 0;
	uint8_t sign[32] = {0};
	size_t sign_len = 0;

	//> extents paramters
	bool extents_valid = false;
	uint8_t *extents_data = NULL;
	size_t extents_data_size = 0;
	uint8_t extents_sign[32] = {0};
	size_t extents_sign_size = 0;
	uint8_t SHARE_KEY_EXT[16] = {0};

#if MIIO_PROV_STATISTIC
	if(is_prov_stat_enable()){
		set_prov_stat_stage(PROV_STAGE_CONFIG_ROUTER_SAFE);
		if(get_prov_stat_type() == PROV_TYPE_AUTO){
			/*avoid increase many provision times in one provision*/
			static uint8_t auto_provision_times = 0;
			if(auto_provision_times < 1){
				set_prov_start_time();
				increase_prov_times();
				auto_provision_times++;
			}
		}
	}
#endif

	if(MIIO_DELEGATE_JSON != req_arg->type){
		goto __STEP2;
	}

	jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(req_arg->pload, req_arg->pload_len);
	if(MIIO_OK != jsmi_parse_start(&jsmi_parser)){
		err_code = MIIO_OT_ERR_CODE_BUSY;
		err_msg = MIIO_OT_ERR_INFO_BUSY;
		goto err_exit;
	}
	// /params
	jsmntok_t *params_tok = NULL;
	{
		jsmi_tok_path_t path[] = {
			{
				.key = "params",
				.type = JSMN_OBJECT
			}
		};
		params_tok = jsmi_get_tok(&jsmi_parser, NULL, path, NELEMENTS(path));
		if(NULL == params_tok){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "\"params\" not found");
			err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
			err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
			goto jsmi_parse_exit;
		}
	}
	// /params/data
	{
		jsmi_tok_path_t path[] = {
			{
				.key = "data",
				.type = JSMN_STRING
			}
		};
		jsmntok_t *data_tok = jsmi_get_tok(&jsmi_parser, params_tok, path, NELEMENTS(path));
		if(NULL == data_tok){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "\"data\" not found");
			err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
			err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
			goto jsmi_parse_exit;
		}

		size_t data_base64_size = data_tok->end - data_tok->start + 1;
		uint8_t *data_base64  = calloc(1, data_base64_size);
		if(NULL == data_base64){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "no memory");
			err_code = MIIO_OT_ERR_CODE_BUSY;
			err_msg = MIIO_OT_ERR_INFO_BUSY;
			goto jsmi_parse_exit;
		}
		if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, data_tok, NULL, 0, (char*)data_base64, data_base64_size)){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "\"data\" not valid");
			free(data_base64);
			err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
			err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
			goto jsmi_parse_exit;
		}

		data = calloc(1, data_base64_size);
		if( NULL == data){
			free(data_base64);
			LOG_ERROR_TAG(MIIO_LOG_TAG, "no memory");
			err_code = MIIO_OT_ERR_CODE_BUSY;
			err_msg = MIIO_OT_ERR_INFO_BUSY;
			goto jsmi_parse_exit;
		}
		
		if(0 != mbedtls_base64_decode(data, data_base64_size, &data_len, data_base64, strnlen((char*)data_base64, data_base64_size))){
			free(data_base64);
			free(data);
			data = NULL;
			LOG_ERROR_TAG(MIIO_LOG_TAG, "\"data\" not valid");
			err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
			err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
			goto jsmi_parse_exit;
		}
		free(data_base64);
	}

	// /params/sign
	{
		uint8_t sign_base64[MIIO_BASE64_SIZE(sizeof(sign))] = {0};
		jsmi_tok_path_t path[] = {
			{
				.key = "sign",
				.type = JSMN_STRING
			}
		};
		if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, params_tok, path, NELEMENTS(path), (char*)sign_base64, sizeof(sign_base64))){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "\"sign\" not valid");
			err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
			err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
			goto jsmi_parse_exit;
		}

		if(0 != mbedtls_base64_decode(sign, sizeof(sign), &sign_len, sign_base64, strnlen((char*)sign_base64, sizeof(sign_base64)))){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "\"sign\" not valid");
			err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
			err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
			goto jsmi_parse_exit;
		}
	}
	jsmntok_t *extents_tok = NULL;
	do{
		jsmi_tok_path_t path[] = {
			{
				.key = "params",
				.type = JSMN_OBJECT
			},
			{
				.key = "extents",
				.type = JSMN_OBJECT
			}
		};
		extents_tok = jsmi_get_tok(&jsmi_parser, NULL, path, NELEMENTS(path));
		if(NULL == extents_tok){
			LOG_WARN_TAG(MIIO_LOG_TAG, "%s:%d \"params/extents\" not found", __FUNCTION__, __LINE__);
			break;
		}
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "find 'params/extents'");
		//> 1. get public_key string BASE64(PUB_KEY_EXT_S)
		if( !miio_ciphers_ecdh_valid( miio_ecdh_ext_handle_get() ) ){
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "find 'params/extents' but no valid ext ecdh handle");
			break;
		}
		{
			int _r = 0;
			jsmi_tok_path_t path[] = {
				{
					.key = "public_key",
					.type = JSMN_STRING
				}
			};
			jsmntok_t *public_tok = jsmi_get_tok(&jsmi_parser, extents_tok, path, NELEMENTS(path));
			if(NULL == public_tok){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d  \"extents/public_key\" not found",  __FUNCTION__, __LINE__);
				err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
				err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
				break;
			}
			size_t public_base64_size = public_tok->end - public_tok->start + 1;
			uint8_t *public_base64 = calloc(1, public_base64_size);
			if(NULL == public_base64){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d no memory",  __FUNCTION__, __LINE__);
				err_code = MIIO_OT_ERR_CODE_UNDEF_ERROR;
				err_msg = MIIO_OT_ERR_INFO_UNDEF_ERROR;
				break;
			}
			if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, public_tok, NULL, 0, (char*)public_base64, public_base64_size)){
				free(public_base64);
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d  \"extents/public_key\" not valid",  __FUNCTION__, __LINE__);
				err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
				err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
				break;
			}
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "'extents/public_key':%d '%.*s'", public_base64_size, public_base64_size, public_base64);
			uint8_t *PUB_KEY_EXT_S = NULL;
			size_t PUB_KEY_EXT_S_length= public_base64_size;
			PUB_KEY_EXT_S = calloc(1, PUB_KEY_EXT_S_length);
			if(NULL == PUB_KEY_EXT_S){
				free(public_base64);
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d no memory",  __FUNCTION__, __LINE__);
				err_code = MIIO_OT_ERR_CODE_UNDEF_ERROR;
				err_msg = MIIO_OT_ERR_INFO_UNDEF_ERROR;
				break;;
			}
			_r = mbedtls_base64_decode(PUB_KEY_EXT_S, PUB_KEY_EXT_S_length, &PUB_KEY_EXT_S_length, public_base64, strnlen((char*)public_base64, public_base64_size));
			free(public_base64);
			if(0 != _r){
				free(PUB_KEY_EXT_S);
				PUB_KEY_EXT_S = NULL;
				PUB_KEY_EXT_S_length = 0;
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d mbedtls_base64_decode err code[%d]",  __FUNCTION__, __LINE__, _r );
				err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
				err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
				break;
			}

			size_t SHARE_KEY_EXT_size = sizeof(SHARE_KEY_EXT);
			_r = miio_ciphers_ecdh_establish_key(miio_ecdh_ext_handle_get(), PUB_KEY_EXT_S, PUB_KEY_EXT_S_length, SHARE_KEY_EXT, &SHARE_KEY_EXT_size, true);
			
			free(PUB_KEY_EXT_S);
			PUB_KEY_EXT_S = NULL;
			PUB_KEY_EXT_S_length = 0;

			if(0 != _r){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "miio_ciphers_ecdh_establish_key returned %d", _r);
				err_code = MIIO_OT_ERR_CODE_UNDEF_ERROR;
				err_msg = MIIO_OT_ERR_INFO_UNDEF_ERROR;
				break;
			}
#if _MIIO_HANDSHAKE_DEBUG			
			arch_dump_hex(SHARE_KEY_EXT, sizeof(SHARE_KEY_EXT), "hkdf(ecdh_share_key)");
#endif			
			
		}
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "find 'params/extents/public_key'");

		//>2. data
		{
			jsmi_tok_path_t path[] = {
				{
					.key = "data",
					.type = JSMN_STRING
				}
			};
			jsmntok_t *data_tok = jsmi_get_tok(&jsmi_parser, extents_tok, path, NELEMENTS(path));
			if(NULL == data_tok){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "%s:%d  \"extents/data\" not found",  __FUNCTION__, __LINE__);
				err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
				err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
				break;
			}
			size_t data_base64_size = data_tok->end - data_tok->start + 1;
			uint8_t *data_base64 = calloc(1, data_base64_size);
			if(NULL == data_base64){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "no memory");
				err_code = MIIO_OT_ERR_CODE_BUSY;
				err_msg = MIIO_OT_ERR_INFO_BUSY;
				break;
			}
			if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, data_tok, NULL, 0, (char*)data_base64, data_base64_size)){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "\"data\" not valid");
				free(data_base64);
				err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
				err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
				break;
			}
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "extents/data:'%.*s'", data_base64_size, data_base64);
			extents_data = calloc(1, data_base64_size);
			extents_data_size = data_base64_size;
			if(NULL == extents_data){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "no memory");
				free(data_base64);
				err_code = MIIO_OT_ERR_CODE_BUSY;
				err_msg = MIIO_OT_ERR_INFO_BUSY;
				break;
			}

			if(0 != mbedtls_base64_decode(extents_data, data_base64_size, &extents_data_size, data_base64, strnlen((char*)data_base64, data_base64_size))){
				
				LOG_ERROR_TAG(MIIO_LOG_TAG, "\"data\" not valid");
				free(data_base64);
				free(extents_data);
				extents_data = NULL;
				extents_data_size = 0;
				err_code = MIIO_OT_ERR_CODE_UNDEF_ERROR;
				err_msg = MIIO_OT_ERR_INFO_UNDEF_ERROR;
				break;
			}
			free(data_base64);
		}
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "find 'params/extents/data'");
		//>3. sign
		{
			uint8_t sign_base64[MIIO_BASE64_SIZE(sizeof(extents_sign))] = {0};
			jsmi_tok_path_t path[] = {
				{
					.key = "sign",
					.type = JSMN_STRING
				}
			};
			if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, extents_tok, path, NELEMENTS(path), (char*)sign_base64, sizeof(sign_base64))){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "\"sign\" not valid");
				err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
				err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
				break;
			}

			if(0 != mbedtls_base64_decode(extents_sign, sizeof(extents_sign), &extents_sign_size, sign_base64, strnlen((char*)sign_base64, sizeof(sign_base64)))){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "\"extents/sign\" not valid");
				err_code = MIIO_OT_ERR_CODE_UNDEF_ERROR;
				err_msg = MIIO_OT_ERR_INFO_UNDEF_ERROR;
				break;
			}
		}
		extents_valid = true;
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "find 'params/extents/sign'");
	}while(0);
jsmi_parse_exit:
	jsmi_parse_finish(&jsmi_parser);


	__STEP2:
	if(MIIO_OK != err_code){
		goto err_exit;
	}

	if(NULL == data || 0 == data_len || sign_len != sizeof(sign)){
		err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
		err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
		goto err_exit;
	}

#define SIGN_CHECK(pKey, keySize, pData, dataSize, originalSign, signSize)\
do{\
	uint8_t __sign_local[(signSize)] = {0};\
	size_t __sign_len = (signSize);\
	if(MIIO_OK != miio_sign(NULL, MIIO_SIGN_HMAC_SHA256, (pKey), (pData), (dataSize), __sign_local, &__sign_len, 0)){\
		LOG_ERROR_TAG(MIIO_LOG_TAG, "sign failed");\
		err_code = MIIO_OT_ERR_CODE_UNDEF_ERROR;\
		err_msg = MIIO_OT_ERR_INFO_UNDEF_ERROR;\
		goto err_exit;\
	}\
	if(__sign_len != (signSize) || 0 != memcmp(__sign_local, (originalSign), __sign_len)){\
		LOG_ERROR_TAG(MIIO_LOG_TAG, "sign invalid");\
		err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;\
		err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;\
		goto err_exit;\
	}\
}while(0)

#define _DECRYPT(pData, pDataSize, pKey, keySize)\
do{\
	uint8_t* __dec_out = malloc(*(pDataSize));\
	if(NULL == __dec_out){\
		LOG_ERROR_TAG(MIIO_LOG_TAG, "no memory");\
		err_code = MIIO_OT_ERR_CODE_BUSY;\
		err_msg = MIIO_OT_ERR_INFO_BUSY;\
		goto err_exit;\
	}\
	uint8_t __iv[16] = {0};\
	arch_aes_decrypt_cbc((pData), *(pDataSize), __dec_out, (pDataSize), (pKey), (keySize), __iv);\
	memcpy((pData), __dec_out, *(pDataSize));\
	free(__dec_out);\
	*(pDataSize) = *(pDataSize) - PKCS7_depadding((pData), *(pDataSize), 16);\
}while(0)

	uint8_t safekey[16] = {0};
	miio_ciphers_get_safekey(safekey);
	SIGN_CHECK(safekey, sizeof(safekey), data, data_len, sign, sizeof(sign));
	_DECRYPT(data, &data_len, safekey, sizeof(safekey));

	if(extents_valid){
		SIGN_CHECK(SHARE_KEY_EXT, sizeof(SHARE_KEY_EXT), extents_data, extents_data_size, extents_sign, sizeof(extents_sign));
		_DECRYPT(extents_data, &extents_data_size, SHARE_KEY_EXT, sizeof(SHARE_KEY_EXT));
	}

	LOG_DEBUG_TAG(MIIO_LOG_TAG, "data = %.*s, len = %d", data_len, data, data_len);
	LOG_DEBUG_TAG(MIIO_LOG_TAG, "extents_data = %.*s, len = %d", extents_data_size, extents_data, extents_data_size);
#undef _DECRYPT
#undef SIGN_CHECK

	miio_ecdh_ext_handle_free();
#if 1
	//config_router
	if(MIIO_OK == miio_net_config_router(miio_from_delegtate_ctx(ctx), (char*)data, data_len, 
											extents_valid ? (const char*)extents_data: NULL, 
											extents_valid ? extents_data_size: 0)){
		data == NULL? (void)0 : free(data);
		extents_data== NULL? (void)0 : free(extents_data);
		miio_rpc_delegate_ack_ok(req_arg, ack, ctx);
#if MIIO_PROV_STATISTIC
		if(is_prov_stat_enable()){
			set_prov_stat_error_code(PROV_ERROR_NONE);
		}
#endif

		return MIIO_OK;
	}
	else{
		err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
		err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
	}
#else
	data == NULL? (void)0 : free(data);
	extents_data == NULL? (void)0 : free(extents_data);
	miio_rpc_delegate_ack_ok(req_arg, ack, ctx);
	return MIIO_OK;
#endif

err_exit:
	miio_ecdh_ext_handle_free();
	data == NULL? (void)0 : free(data);
	extents_data== NULL? (void)0 : free(extents_data);
	miio_rpc_delegate_ack_error(req_arg, err_code, err_msg, ack, ctx);
#if MIIO_PROV_STATISTIC
	if(is_prov_stat_enable()){
		set_prov_stat_error_code(PROV_ERROR_SAFE_DECRYPT_FAIL);
		if(get_prov_stat_type() == PROV_TYPE_AUTO){
			prov_stat_error_code_process();	
		}
	}
#endif

    return MIIO_ERROR;
}
//MIIO_RPC(config_router_safe, do_config_router_safe, "config_router_safe");
MIIO_RPC_ACM(config_router_safe, do_config_router_safe, "config_router_safe", MIIO_RPC_ACM_NONE);
#if MIIO_CONFIG_ROUTER_SAFE_TEST
//{"method":"miIO.config_router_safe_test","params":{"data":"xxxxxx"}}
static int do_config_router_safe_test(miio_rpc_delegate_arg_t* req_arg, miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
	miio_handle_t miio_handle = miio_from_delegtate_ctx(ctx);
	int err_code = MIIO_OK;
	const char *err_msg = NULL;

	uint8_t* data = NULL;

	if(MIIO_DELEGATE_JSON == req_arg->type){
		jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(req_arg->pload, req_arg->pload_len);
		if(MIIO_OK == jsmi_parse_start(&jsmi_parser)){
			// /params
			jsmntok_t *params_tok = NULL;
			{
				jsmi_tok_path_t path[] = {
					{
						.key = "params",
						.type = JSMN_OBJECT
					}
				};
				params_tok = jsmi_get_tok(&jsmi_parser, NULL, path, NELEMENTS(path));
				if(NULL == params_tok){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "\"params\" not found");
					err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
					err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
					goto jsmi_parse_exit;
				}
			}
			// /params/data
			{
				jsmi_tok_path_t path[] = {
					{
						.key = "data",
						.type = JSMN_STRING
					}
				};
				jsmntok_t *data_tok = jsmi_get_tok(&jsmi_parser, params_tok, path, NELEMENTS(path));
				if(NULL == data_tok){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "\"data\" not found");
					err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
					err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
					goto jsmi_parse_exit;
				}

				size_t data_size = data_tok->end - data_tok->start + 1;
				data = calloc(1, data_size);
				if(NULL == data){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "no memory");
					err_code = MIIO_OT_ERR_CODE_BUSY;
					err_msg = MIIO_OT_ERR_INFO_BUSY;
					goto jsmi_parse_exit;
				}
				if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, data_tok, NULL, 0, (char*)data, data_size)){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "\"data\" not valid");
					err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
					err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
					goto jsmi_parse_exit;
				}
			}
jsmi_parse_exit:
			jsmi_parse_finish(&jsmi_parser);
		}
		else{
			err_code = MIIO_OT_ERR_CODE_BUSY;
			err_msg = MIIO_OT_ERR_INFO_BUSY;
			goto err_exit;
		}
	}

	if(MIIO_OK != err_code){
		goto err_exit;
	}

	if(NULL == data){
		err_code = MIIO_OT_ERR_CODE_PARAM_INVALID;
		err_msg = MIIO_OT_ERR_INFO_PARAM_INVALID;
		goto err_exit;
	}

	LOG_DEBUG_TAG(MIIO_LOG_TAG, "data = %s", data);

	miio_rpc_delegate_arg_t ack_arg = {
		.id = req_arg->id ,
		.type = req_arg->type,
		.pload_len = 0,
		.pload = NULL
	};

	ack_arg.pload_len = miio_get_rpc_max_size(miio_handle);
	ack_arg.pload = (char *) malloc(ack_arg.pload_len);
	if (ack_arg.pload) {
		jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(ack_arg.pload, ack_arg.pload_len);
		jsmi_compose_start(&jsmi_composer);
		jsmi_set_object_begin(&jsmi_composer);
			jsmi_set_key_value_uint(&jsmi_composer, "id", ack_arg.id);
			jsmi_set_key_object_begin(&jsmi_composer, "result");
			{
				uint8_t *data_base64 = NULL;
				size_t data_base64_len = 0;
				uint8_t sign_base64[MIIO_BASE64_SIZE(32)] = {0};
				size_t sign_base64_len = sizeof(sign_base64);
				//aes_encrypt and sign
				{
					size_t data_enc_len = strlen((char*)data);
					data_enc_len += PKCS7_padding(data, data_enc_len, 16);
					uint8_t* data_enc = malloc(data_enc_len);
					if(data_enc){
						uint8_t iv[16] = {0};
						uint8_t safekey[16] = {0};
						miio_ciphers_get_safekey(safekey);
						arch_aes_encrypt_cbc(data, data_enc_len, data_enc, &data_enc_len, safekey, sizeof(safekey), iv);
						miio_sign(NULL, MIIO_SIGN_HMAC_SHA256, safekey, data_enc, data_enc_len, sign_base64, &sign_base64_len, 1);

						data_base64 = calloc(1, MIIO_BASE64_SIZE(data_enc_len));
						if(data_base64){
							mbedtls_base64_encode(data_base64, MIIO_BASE64_SIZE(data_enc_len), &data_base64_len, data_enc, data_enc_len);
						}
						free(data_enc);
					}
				}

				if(data_base64){
					jsmi_set_key_value_str(&jsmi_composer, "data", (char*)data_base64, data_base64_len);
					jsmi_set_key_value_str(&jsmi_composer, "sign", (char*)sign_base64, sign_base64_len);
					free(data_base64);
				}
				else{
					err_code = MIIO_OT_ERR_CODE_BUSY;
					err_msg = MIIO_OT_ERR_INFO_BUSY;
					goto err_exit;
				}
			}
			jsmi_set_key_object_end(&jsmi_composer);
		jsmi_set_object_end(&jsmi_composer);
		if(MIIO_OK == jsmi_compose_finish(&jsmi_composer, NULL, &ack_arg.pload_len)){
			free(data);
			ack(&ack_arg, ctx);
			free(ack_arg.pload);
			return MIIO_OK;
		}
	}
	else{
		err_code = MIIO_OT_ERR_CODE_BUSY;
		err_msg = MIIO_OT_ERR_INFO_BUSY;
	}

err_exit:

	if(data){
		free(data);
	}

	miio_rpc_delegate_ack_error(req_arg, err_code, err_msg, ack, ctx);

    return MIIO_ERROR;
}
MIIO_RPC(config_router_safe_test, do_config_router_safe_test, "config_router_safe_test");
#endif
#endif

static int do_restore(miio_rpc_delegate_arg_t *req_arg, miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
	if(MIIO_DELEGATE_AUTH_BIT_CLOUD != (req_arg->auth & MIIO_DELEGATE_AUTH_BIT_CLOUD)){
		LOG_ERROR_TAG(MIIO_LOG_TAG, "restore is from local, droped...");
		miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_SERVICE_NOT_AVAILABLE, MIIO_OT_ERR_INFO_SERVICE_NOT_AVAILABLE, ack, ctx);
		return MIIO_ERROR;
	}

	miio_rpc_delegate_ack_ok(req_arg, ack, ctx);

	miio_restore(miio_from_delegtate_ctx(ctx), "miIO.restore");

	miio_reboot(miio_from_delegtate_ctx(ctx), "miIO.restore", MIIO_RPC_TIMEOUT_MS);

	return MIIO_OK;
}
MIIO_RPC_ACM(restore, do_restore,  NULL, ( MIIO_RPC_ACM_PROVISIONED & MIIO_RPC_ACM_PSK ) );


/*
close disable_local_restore
{
    "method" : "miIO.disable_local_restore",
    "params" : [0],
    "id" : 111
}
*/
static int do_disable_local_restore(miio_rpc_delegate_arg_t *req_arg, miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
	int ret = MIIO_ERROR;
	int disable_local_restore = 0;

	if(MIIO_DELEGATE_JSON == req_arg->type){
		jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(req_arg->pload, req_arg->pload_len);
		if(MIIO_OK == jsmi_parse_start(&jsmi_parser)){
			// /params/[0]
			{
				jsmi_tok_path_t path[] = {
					{
						.key = "params",
						.type = JSMN_ARRAY
					},
					{
						.key = (void*)0,
						.type = JSMN_PRIMITIVE
					}
				};
				if(MIIO_OK == jsmi_get_value_sint(&jsmi_parser, NULL, path, NELEMENTS(path), &disable_local_restore)){
					ret = MIIO_OK;
				}
			}
			jsmi_parse_finish(&jsmi_parser);
		}
	}

	if(MIIO_OK == ret){
		miio_net_set_disable_local_restore(disable_local_restore);
		miio_rpc_delegate_ack_ok(req_arg, ack, ctx);
	}
	else{
		miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID, MIIO_OT_ERR_INFO_PARAM_INVALID, ack, ctx);
	}

	return ret;
}
MIIO_RPC(disable_local_restore, do_disable_local_restore, "disable_local_restore");

/*
{
    "method" : "miIO.get_disable_local_restore",
    "params" : [],
    "id" : 111
}

return
{
    "result" : 0,
    "id" : 111
}

{
    "result" : 1,
    "id" : 111
}
 */
static int do_get_disable_local_restore(miio_rpc_delegate_arg_t *req_arg, miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
	if(MIIO_DELEGATE_JSON == req_arg->type)
	{
		char js[64] = {0};
		miio_rpc_delegate_arg_t ack_arg = {
			.id = req_arg->id,
			.type = req_arg->type,
			.pload_len = 0,
			.pload = js
		};

		jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, sizeof(js));
		jsmi_compose_start(&jsmi_composer);
		jsmi_set_object_begin(&jsmi_composer);
			jsmi_set_key_value_uint(&jsmi_composer, "id", ack_arg.id);
			jsmi_set_key_value_sint(&jsmi_composer, "result", miio_net_get_disable_local_restore());
		jsmi_set_object_end(&jsmi_composer);
		if(MIIO_OK == jsmi_compose_finish(&jsmi_composer, NULL, &ack_arg.pload_len)){
			ack(&ack_arg, ctx);
			return MIIO_OK;
		}
	}

	miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_BUSY, MIIO_OT_ERR_INFO_BUSY, ack, ctx);

	return MIIO_ERROR;
}
MIIO_RPC(get_disable_local_restore, do_get_disable_local_restore, "get_disable_local_restore");

/*
{
    "method" : "bind_key",
    "params" : ["failed"],
    "id" : 111
}
*/
static int do_bind_key(miio_rpc_delegate_arg_t *req_arg, miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
	miio_rpc_delegate_ack_ok(req_arg, ack, ctx);

	miio_net_set_bindkey(NULL);

	return MIIO_OK;
}
MIIO_RPC_USER(bind_key, do_bind_key, "bind_key");

/*
{
    "method" : "miIO.bind_stat",
    "params" : [1],
    "id" : 111
}
*/
static int do_bind_stat(miio_rpc_delegate_arg_t *req_arg, miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
	miio_rpc_delegate_ack_ok(req_arg, ack, ctx);

	miio_net_set_bindkey(NULL);

	return MIIO_OK;
}
MIIO_RPC(bind_stat, do_bind_stat, "bind_stat");

/*
{
    "method" : "miIO.migration",
    "params" : {
		"ot":{
			"country_domain":"sg",
			"gmt_offset":8,
			"tz": ""
		},
		"wifi_config" : {
			"cc": "SG"
		}

	},
    "id" : 111
}
*/
static int do_migration(miio_rpc_delegate_arg_t *req_arg, miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
	int ret = MIIO_ERROR;

	miio_handle_t miio_handle = miio_from_delegtate_ctx(ctx);

	if(MIIO_DELEGATE_JSON == req_arg->type){
		jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(req_arg->pload, req_arg->pload_len);
		ret = jsmi_parse_start(&jsmi_parser);
		if(MIIO_OK == ret){
			// /ot/country_domian
			{
				char country_domain[MIIO_COUNTRY_DOMAIN_SIZE_MAX] = {0};
				jsmi_tok_path_t path[] = {
					{
						.key = "params",
						.type = JSMN_OBJECT
					},
					{
						.key = "ot",
						.type = JSMN_OBJECT
					},
					{
						.key = "country_domain",
						.type = JSMN_STRING
					}
				};

				if(MIIO_OK == jsmi_get_value_str(&jsmi_parser, NULL, path, NELEMENTS(path), country_domain, sizeof(country_domain))){
					miio_instance_set_country_domain(miio_handle, country_domain);
				}
			}

			// /ot/gmt_offset
			{
				int gmt_offset = 0;
				jsmi_tok_path_t path[] = {
					{
						.key = "params",
						.type = JSMN_OBJECT
					},
					{
						.key = "ot",
						.type = JSMN_OBJECT
					},
					{
						.key = "gmt_offset",
						.type = JSMN_PRIMITIVE
					}
				};
				if(MIIO_OK == jsmi_get_value_sint(&jsmi_parser, NULL, path, NELEMENTS(path), &gmt_offset)){
					miio_instance_set_gmt_offset(miio_handle, gmt_offset);
				}
			}

			// /wifi_config/cc
			{
				char cc[3] = {0};
				jsmi_tok_path_t path[] = {
					{
						.key = "params",
						.type = JSMN_OBJECT
					},
					{
						.key = "wifi_config",
						.type = JSMN_OBJECT
					},
					{
						.key = "cc",
						.type = JSMN_STRING
					}
				};

				if(MIIO_OK == jsmi_get_value_str(&jsmi_parser, NULL, path, NELEMENTS(path), cc, sizeof(cc))){
					miio_net_set_wifi_channel(cc, 1);
				}
			}

			jsmi_parse_finish(&jsmi_parser);
		}

		if(MIIO_OK == ret){
			/* if migrate to country code non-CN, cannot scan forbidden channels in smart config */
			miio_net_switch_smart_config_onoff_in_migration();
			miio_rpc_delegate_ack_ok(req_arg, ack, ctx);
			miio_net_restart_async(1000);
		}
		else{
			miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID, MIIO_OT_ERR_INFO_PARAM_INVALID, ack, ctx);
		}
	}

    return ret;
}
MIIO_RPC(migration, do_migration, "migration");
