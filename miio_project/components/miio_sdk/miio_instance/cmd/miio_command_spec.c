#include "miio_command.h"
#include "list/list.h"
#include "jsmi.h"

#undef MIIO_LOG_TAG
#define MIIO_LOG_TAG			"mcmd"

typedef struct{
	list_head_t list;
	mcmd_spec_property_t property;
	int code;
}mcmd_spec_property_rsp_entry_t;

#define mcmd_spec_property_rsp_list_init(head)	INIT_LIST_HEAD(head)

static int mcmd_spec_property_rsp_list_insert(list_head_t *head, mcmd_spec_property_t *property)
{
	mcmd_spec_property_rsp_entry_t *entry = calloc(1, sizeof(mcmd_spec_property_rsp_entry_t));

	if(NULL == entry){
		return MIIO_ERROR_NOMEM;
	}

	INIT_LIST_HEAD(&entry->list);

	entry->code = MIIO_USER_ERR_PROP_CODE;

	memcpy(&entry->property, property, sizeof(mcmd_spec_property_t));

	list_add_tail(&entry->list, head);

	return MIIO_OK;
}

static int mcmd_spec_property_rsp_list_update(list_head_t *head, mcmd_spec_property_t *property, int code)
{
	mcmd_spec_property_rsp_entry_t *entry,*tmp_entry;
	list_for_each_entry_safe(entry, tmp_entry, head, list, mcmd_spec_property_rsp_entry_t)
	{
		if(	entry->property.siid == property->siid &&
		    entry->property.piid == property->piid){
			entry->code = code;
			entry->property.value = property->value;
			entry->property.value_len = property->value_len;
			return MIIO_OK;
		}
	}

	return MIIO_ERROR_NOTFOUND;
}

static int mcmd_spec_property_rsp_list_empty(list_head_t *head)
{
	mcmd_spec_property_rsp_entry_t *entry,*tmp_entry;
	list_for_each_entry_safe(entry, tmp_entry, head, list, mcmd_spec_property_rsp_entry_t)
	{
		list_del(&entry->list);
		free(entry);
	}

	return MIIO_OK;
}

static void mcmd_spec_property_rsp_list_to_json(list_head_t *head, jsmi_composer_t *jsmi_composer)
{
	jsmi_set_array_begin(jsmi_composer);
	mcmd_spec_property_rsp_entry_t *entry,*tmp_entry;
	list_for_each_entry_safe(entry, tmp_entry, head, list, mcmd_spec_property_rsp_entry_t)
	{
		jsmi_set_value_object_begin(jsmi_composer);
			jsmi_set_key_value_str(jsmi_composer, "did", entry->property.did, strnlen(entry->property.did, sizeof(entry->property.did)));
			jsmi_set_key_value_uint(jsmi_composer, "siid", entry->property.siid);
			jsmi_set_key_value_uint(jsmi_composer, "piid", entry->property.piid);
			jsmi_set_key_value_sint(jsmi_composer, "code", entry->code);
			if(entry->property.value && entry->property.value_len){
				jsmi_set_key_value_bytes(jsmi_composer, "value", entry->property.value, entry->property.value_len);
				entry->property.value = NULL;
				entry->property.value_len = 0;
			}
		jsmi_set_value_object_end(jsmi_composer);
	}
	jsmi_set_array_end(jsmi_composer);
}

static void mcmd_spec_property_rsp_list_to_cmd(list_head_t *head, mcmd_composer_t *mcmd_composer)
{
	mcmd_spec_property_rsp_entry_t *entry,*tmp_entry;
	list_for_each_entry_safe(entry, tmp_entry, head, list, mcmd_spec_property_rsp_entry_t)
	{
		mcmd_compose_uint32(mcmd_composer, entry->property.siid);
		mcmd_compose_uint32(mcmd_composer, entry->property.piid);
		if(entry->property.value && entry->property.value_len){
			mcmd_compose_bytes(mcmd_composer, entry->property.value, entry->property.value_len);
			entry->property.value = NULL;
			entry->property.value_len = 0;
		}
	}
}

static int get_properties_error(mcmd_t *mcmd, int code, char* message)
{
	mcmd_spec_property_rsp_list_empty(&mcmd->spec.property_rsp_list);

	return MIIO_ERROR;
}
//result <siid> <piid> <code> [value] ... <siid> <piid> <code> [value]
static int get_properties_result(mcmd_t *mcmd, char* params)
{
	char *argv[MCMD_COMMAND_ARG_MAX];
	int argc = mcmd_parse_params(params, argv, NELEMENTS(argv), NULL);
	if (argc >= 3) {
		for(int i=0; i < argc; i++){
			mcmd_spec_property_t spec_property = {0};
			spec_property.siid = arch_atoun(argv[i], strlen(argv[i]));
			if(++i >= argc)break;
			spec_property.piid = arch_atoun(argv[i], strlen(argv[i]));
			if(++i >= argc)break;
			int code = arch_atoin(argv[i], strlen(argv[i]));
			if(0 == code){
				if(++i > argc)break;
				spec_property.value = argv[i];
				spec_property.value_len = strlen(argv[i]);
			}
			else{
				spec_property.value = NULL;
				spec_property.value_len = 0;
			}

			mcmd_spec_property_rsp_list_update(&mcmd->spec.property_rsp_list, &spec_property, code);
		}
	}

	{
		char* js = mcmd->down.buf;
		int js_size = MIN(mcmd->down.buf_size, miio_get_rpc_max_size(mcmd->miio_handle));

		jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, js_size);
		jsmi_compose_start(&jsmi_composer);
		jsmi_set_object_begin(&jsmi_composer);
			jsmi_set_key_value_uint(&jsmi_composer, "id", mcmd->down.method_ctx.arg.id);
			jsmi_set_key(&jsmi_composer, "result");
			mcmd_spec_property_rsp_list_to_json(&mcmd->spec.property_rsp_list, &jsmi_composer);
		jsmi_set_object_end(&jsmi_composer);

		mcmd_spec_property_rsp_list_empty(&mcmd->spec.property_rsp_list);

		if(MIIO_OK == jsmi_compose_finish(&jsmi_composer, NULL, NULL)){
			miio_rpc_delegate_arg_t ack_arg = {
				.id = mcmd->down.method_ctx.arg.id,
				.type = MIIO_DELEGATE_JSON,
				.pload_len = strnlen(js, js_size),
				.pload = js
			};

			return mcmd->down.method_ctx.ack(&ack_arg, mcmd->down.method_ctx.ack_ctx);
		}
	}


	return MIIO_ERROR;
}

/*
 *  down get_properties <siid> <piid>...
 *  json:
	{
 		"id" :  123,
        "method" : "get_properties",
        "params" : [
            { "did" : "1234" , "siid" : 1, "piid" : 2},
            { "did" : "1234" , "siid" : 1, "piid" : 3}
        ]
    }
 */


static int do_get_properties(mcmd_t *mcmd, const char *method, size_t method_len, const char *params, size_t params_len)
{
	int ret = MIIO_OK;

	mcmd_spec_property_rsp_list_init(&mcmd->spec.property_rsp_list);

	jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(params, params_len);
	if(MIIO_OK == jsmi_parse_start(&jsmi_parser)){
		jsmntok_t *params_tok = jsmi_get_tok(&jsmi_parser, NULL, NULL, 0);
		if(params_tok && (JSMN_ARRAY == params_tok->type) && params_tok->size){
			for(int i = 0; i < params_tok->size; i++){
				mcmd_spec_property_t spec_property = {0};
				{
					jsmi_tok_path_t path[] = {
						{
							.key = (void*)i,
							.type = JSMN_OBJECT
						},
						{
							.key = "did",
							.type = JSMN_STRING
						}
					};

					if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, params_tok, path, NELEMENTS(path), spec_property.did, sizeof(spec_property.did))){
						LOG_WARN_TAG(MIIO_LOG_TAG, "\"did\" not found");
						memcpy(spec_property.did, mcmd->spec.did, MIN(sizeof(spec_property.did), sizeof(mcmd->spec.did)));
					}
				}
				{
					jsmi_tok_path_t path[] = {
						{
							.key = (void*)i,
							.type = JSMN_OBJECT
						},
						{
							.key = "siid",
							.type = JSMN_PRIMITIVE
						}
					};

					if(MIIO_OK != jsmi_get_value_uint32(&jsmi_parser, params_tok, path, NELEMENTS(path), &spec_property.siid)){
						LOG_ERROR_TAG(MIIO_LOG_TAG, "\"siid\" not found");
						ret = MIIO_ERROR_PARAM;
						goto jsmi_parse_exit;
					}
				}
				{
					jsmi_tok_path_t path[] = {
						{
							.key = (void*)i,
							.type = JSMN_OBJECT
						},
						{
							.key = "piid",
							.type = JSMN_PRIMITIVE
						}
					};

					if(MIIO_OK != jsmi_get_value_uint32(&jsmi_parser, params_tok, path, NELEMENTS(path), &spec_property.piid)){
						LOG_ERROR_TAG(MIIO_LOG_TAG, "\"piid\" not found");
						ret = MIIO_ERROR_PARAM;
						goto jsmi_parse_exit;
					}
				}

				spec_property.value = NULL;
				spec_property.value_len = 0;

				if(MIIO_OK != mcmd_spec_property_rsp_list_insert(&mcmd->spec.property_rsp_list, &spec_property)){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "property node insert failed");
					ret = MIIO_ERROR_NOMEM;
					goto jsmi_parse_exit;
				}
			}

		}
		else{
			LOG_ERROR_TAG(MIIO_LOG_TAG, "params parse error");
			ret = MIIO_ERROR_PARAM;
		}
jsmi_parse_exit:
		jsmi_parse_finish(&jsmi_parser);
	}
	else{
		LOG_ERROR_TAG(MIIO_LOG_TAG, "params parse error");
		ret = MIIO_ERROR_NOMEM;
	}


	if(MIIO_OK == ret){
		mcmd_composer_t mcmd_composer = MCMD_COMPOSER_INIT(mcmd->command_buf, mcmd->command_buf_size);
		mcmd_compose_start(&mcmd_composer, "down");
		mcmd_compose_bytes(&mcmd_composer, method, method_len);
		mcmd_spec_property_rsp_list_to_cmd(&mcmd->spec.property_rsp_list, &mcmd_composer);

		if(MIIO_OK == mcmd_compose_finish(&mcmd_composer, NULL, NULL)){
			mcmd->down.result = get_properties_result;
			mcmd->down.error = get_properties_error;
			mcmd->io.out(mcmd->io.handle, mcmd->command_buf);
			return MIIO_OK;
		}
	}

	mcmd_spec_property_rsp_list_empty(&mcmd->spec.property_rsp_list);

	return MIIO_ERROR;
}
MIIO_CMD_RPC(get_properties, do_get_properties, NULL);

static int set_properties_error(mcmd_t *mcmd, int error, char* message)
{
	mcmd_spec_property_rsp_list_empty(&mcmd->spec.property_rsp_list);

	return MIIO_ERROR;
}
//result <siid> <piid> <code> ... <siid> <piid> <code>
static int set_properties_result(mcmd_t *mcmd, char* params)
{
	char *argv[MCMD_COMMAND_ARG_MAX];
	int argc = mcmd_parse_params(params, argv, NELEMENTS(argv), NULL);
	if (argc && ((argc%3) == 0)) {
		for(int i=0; i < argc; i++){
			mcmd_spec_property_t spec_property = {0};
			spec_property.siid = arch_atoun(argv[i], strlen(argv[i]));
			if(++i >= argc)break;
			spec_property.piid = arch_atoun(argv[i], strlen(argv[i]));
			if(++i >= argc)break;
			int code = arch_atoin(argv[i], strlen(argv[i]));
			spec_property.value = NULL;
			spec_property.value_len = 0;
			mcmd_spec_property_rsp_list_update(&mcmd->spec.property_rsp_list, &spec_property, code);
		}
	}

	{
		char* js = mcmd->down.buf;
		int js_size = MIN(mcmd->down.buf_size, miio_get_rpc_max_size(mcmd->miio_handle));

		jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, js_size);
		jsmi_compose_start(&jsmi_composer);
		jsmi_set_object_begin(&jsmi_composer);
			jsmi_set_key_value_uint(&jsmi_composer, "id", mcmd->down.method_ctx.arg.id);
			jsmi_set_key(&jsmi_composer, "result");
			mcmd_spec_property_rsp_list_to_json(&mcmd->spec.property_rsp_list, &jsmi_composer);
		jsmi_set_object_end(&jsmi_composer);

		mcmd_spec_property_rsp_list_empty(&mcmd->spec.property_rsp_list);

		if(MIIO_OK == jsmi_compose_finish(&jsmi_composer, NULL, NULL)){
			miio_rpc_delegate_arg_t ack_arg = {
				.id = mcmd->down.method_ctx.arg.id,
				.type = MIIO_DELEGATE_JSON,
				.pload_len = strnlen(js, js_size),
				.pload = js
			};

			return mcmd->down.method_ctx.ack(&ack_arg, mcmd->down.method_ctx.ack_ctx);
		}
	}

	return MIIO_ERROR;
}

/*
 * com: set_properties <siid> <piid> <value> ... <siid> <piid> <value>
 * json:
	{
		"id": 123,
        "method" : "set_properties",
        "params" : [
            {
            	"did"  : "1234",
                "siid" : 1,
                "piid" : 1,
                "value": 10

            },
            {
                "did"  : "1234",
                "siid" : 1,
                "piid" : 88,
                "value": "hello"
            }
        ]
    }
 */
static int do_set_properties(mcmd_t *mcmd, const char *method, size_t method_len, const char *params, size_t params_len)
{
	int ret = MIIO_OK;

	mcmd_spec_property_rsp_list_init(&mcmd->spec.property_rsp_list);

	jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(params, params_len);
	if(MIIO_OK == jsmi_parse_start(&jsmi_parser)){
		jsmntok_t *params_tok = jsmi_get_tok(&jsmi_parser, NULL, NULL, 0);
		if(params_tok && (JSMN_ARRAY == params_tok->type) && params_tok->size){
			for(int i = 0; i < params_tok->size; i++){
				mcmd_spec_property_t spec_property = {0};
				{
					jsmi_tok_path_t path[] = {
						{
							.key = (void*)i,
							.type = JSMN_OBJECT
						},
						{
							.key = "did",
							.type = JSMN_STRING
						}
					};

					if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, params_tok, path, NELEMENTS(path), spec_property.did, sizeof(spec_property.did))){
						LOG_WARN_TAG(MIIO_LOG_TAG, "\"did\" not found");
						memcpy(spec_property.did, mcmd->spec.did, MIN(sizeof(spec_property.did), sizeof(mcmd->spec.did)));
					}
				}
				{
					jsmi_tok_path_t path[] = {
						{
							.key = (void*)i,
							.type = JSMN_OBJECT
						},
						{
							.key = "siid",
							.type = JSMN_PRIMITIVE
						}
					};

					if(MIIO_OK != jsmi_get_value_uint32(&jsmi_parser, params_tok, path, NELEMENTS(path), &spec_property.siid)){
						LOG_ERROR_TAG(MIIO_LOG_TAG, "\"siid\" not found");
						ret = MIIO_ERROR_PARAM;
						goto jsmi_parse_exit;
					}
				}
				{
					jsmi_tok_path_t path[] = {
						{
							.key = (void*)i,
							.type = JSMN_OBJECT
						},
						{
							.key = "piid",
							.type = JSMN_PRIMITIVE
						}
					};

					if(MIIO_OK != jsmi_get_value_uint32(&jsmi_parser, params_tok, path, NELEMENTS(path), &spec_property.piid)){
						LOG_ERROR_TAG(MIIO_LOG_TAG, "\"piid\" not found");
						ret = MIIO_ERROR_PARAM;
						goto jsmi_parse_exit;
					}
				}
				{
					jsmi_tok_path_t path[] = {
						{
							.key = (void*)i,
							.type = JSMN_OBJECT
						},
						{
							.key = "value",
							.type = JSMN_UNDEFINED
						}
					};

					jsmntok_t *value_tok = jsmi_get_tok(&jsmi_parser, params_tok, path, NELEMENTS(path));
					if(NULL == value_tok){
						LOG_ERROR_TAG(MIIO_LOG_TAG, "\"value\" not found");
						ret = MIIO_ERROR_PARAM;
						goto jsmi_parse_exit;
					}
					if(JSMN_STRING == value_tok->type){
						spec_property.value = (char*)(jsmi_parser.js + value_tok->start - 1);
						spec_property.value_len = value_tok->end - value_tok->start + 2;
					}
					else{
						spec_property.value = (char*)(jsmi_parser.js + value_tok->start);
						spec_property.value_len = value_tok->end - value_tok->start;
					}
				}

				if(MIIO_OK != mcmd_spec_property_rsp_list_insert(&mcmd->spec.property_rsp_list, &spec_property)){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "property node insert failed");
					ret = MIIO_ERROR_NOMEM;
					goto jsmi_parse_exit;
				}
			}

		}
		else{
			LOG_ERROR_TAG(MIIO_LOG_TAG, "params parse error");
			ret = MIIO_ERROR_PARAM;
		}
jsmi_parse_exit:
		jsmi_parse_finish(&jsmi_parser);
	}
	else{
		LOG_ERROR_TAG(MIIO_LOG_TAG, "params parse error");
		ret = MIIO_ERROR_NOMEM;
	}

	if(MIIO_OK == ret){
		mcmd_composer_t mcmd_composer = MCMD_COMPOSER_INIT(mcmd->command_buf, mcmd->command_buf_size);
		mcmd_compose_start(&mcmd_composer, "down");
		mcmd_compose_bytes(&mcmd_composer, method, method_len);
		mcmd_spec_property_rsp_list_to_cmd(&mcmd->spec.property_rsp_list, &mcmd_composer);

		if(MIIO_OK == mcmd_compose_finish(&mcmd_composer, NULL, NULL)){
			mcmd->down.result = set_properties_result;
			mcmd->down.error = set_properties_error;
			mcmd->io.out(mcmd->io.handle, mcmd->command_buf);
			return MIIO_OK;
		}
	}

	mcmd_spec_property_rsp_list_empty(&mcmd->spec.property_rsp_list);

	return MIIO_ERROR;
}
MIIO_CMD_RPC(set_properties, do_set_properties, NULL);

static int action_error(mcmd_t *mcmd, int code, char* message)
{
	return mcmd_rpc_error_default(mcmd, code, message);
}

//result <siid> <aiid> <code> <piid> <value> ... <piid> <value>
static int action_result(mcmd_t *mcmd, char* params)
{
	char *argv[MCMD_COMMAND_ARG_MAX];
	int argc = mcmd_parse_params(params, argv, NELEMENTS(argv), NULL);
	if (argc >=3  && (((argc-3)%2) == 0)) {
		char* js = mcmd->down.buf;
		int js_size = MIN(mcmd->down.buf_size, miio_get_rpc_max_size(mcmd->miio_handle));

		jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, js_size);
		jsmi_compose_start(&jsmi_composer);
		jsmi_set_object_begin(&jsmi_composer);
			jsmi_set_key_value_uint(&jsmi_composer, "id", mcmd->down.method_ctx.arg.id);
			jsmi_set_key_object_begin(&jsmi_composer, "result");
				{
					uint32_t siid = arch_atoun(argv[0], strlen(argv[0]));
					uint32_t aiid = arch_atoun(argv[1], strlen(argv[1]));
					if((siid != mcmd->spec.action.siid) || (aiid != mcmd->spec.action.aiid)){
						LOG_ERROR_TAG(MIIO_LOG_TAG, "siid or aiid not valid");
						goto error_exit;
					}
					jsmi_set_key_value_str(&jsmi_composer, "did", mcmd->spec.action.did, strnlen(mcmd->spec.action.did, sizeof(mcmd->spec.action.did)));
					jsmi_set_key_value_bytes(&jsmi_composer, "siid", argv[0], strlen(argv[0]));
					jsmi_set_key_value_bytes(&jsmi_composer, "aiid", argv[1], strlen(argv[1]));
				}
				int code = atoi(argv[2]);
				jsmi_set_key_value_sint(&jsmi_composer, "code", code);
				if(0 == code && argc > 3){
					jsmi_set_key_array_begin(&jsmi_composer, "out");
					for(int i=3; i < argc; i++){
						jsmi_set_value_object_begin(&jsmi_composer);
							jsmi_set_key_value_bytes(&jsmi_composer, "piid", argv[i], strlen(argv[i]));
							++i;
							jsmi_set_key_value_bytes(&jsmi_composer, "value", argv[i], strlen(argv[i]));
						jsmi_set_value_object_end(&jsmi_composer);
					}
					jsmi_set_key_array_end(&jsmi_composer);
				}
			jsmi_set_key_object_end(&jsmi_composer);
		jsmi_set_object_end(&jsmi_composer);


		if(MIIO_OK == jsmi_compose_finish(&jsmi_composer, NULL, NULL)){
			miio_rpc_delegate_arg_t ack_arg = {
				.id = mcmd->down.method_ctx.arg.id,
				.type = MIIO_DELEGATE_JSON,
				.pload_len = strnlen(js, js_size),
				.pload = js
			};

			return mcmd->down.method_ctx.ack(&ack_arg, mcmd->down.method_ctx.ack_ctx);
		}
	}

error_exit:

	mcmd_rpc_error_default(mcmd, MIIO_USER_ERR_CODE_ACK_INVALID, MIIO_USER_ERR_INFO_ACK_INVALID);

	return MIIO_ERROR;
}

/*
 * action <siid> <aiid> <piid> <value> ... <piid> <value>
 * json:
 	 {
        "id": 123,
        "method" : "action",
        "params" : {
            "did" : "1234",
            "siid" : "1",
            "aiid" : "1",
            "in" : [
                {
                    "piid" : 1,
                    "value": 10
                }
            ]
        }
    }
 */
static int do_action(mcmd_t *mcmd, const char *method, size_t method_len, const char *params, size_t params_len)
{
	int ret = MIIO_OK;

	mcmd_composer_t mcmd_composer = MCMD_COMPOSER_INIT(mcmd->command_buf, mcmd->command_buf_size);
	mcmd_compose_start(&mcmd_composer, "down");
	mcmd_compose_bytes(&mcmd_composer, method, method_len);

	jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(params, params_len);
	if(MIIO_OK == jsmi_parse_start(&jsmi_parser)){
		{
			jsmi_tok_path_t path[] = {
				{
					.key = "did",
					.type = JSMN_STRING
				}
			};

			if(MIIO_OK != jsmi_get_value_str(&jsmi_parser, NULL, path, NELEMENTS(path), mcmd->spec.action.did, sizeof(mcmd->spec.action.did))){
				LOG_WARN_TAG(MIIO_LOG_TAG, "\"did\" not found");
				memcpy(mcmd->spec.action.did, mcmd->spec.did, MIN(sizeof(mcmd->spec.action.did), sizeof(mcmd->spec.did)));
			}
		}
		{
			jsmi_tok_path_t path[] = {
				{
					.key = "siid",
					.type = JSMN_PRIMITIVE
				}
			};

			if(MIIO_OK != jsmi_get_value_uint32(&jsmi_parser, NULL, path, NELEMENTS(path), &mcmd->spec.action.siid)){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "\"siid\" not found");
				ret = MIIO_ERROR_PARAM;
				goto jsmi_parse_exit;
			}

			mcmd_compose_uint32(&mcmd_composer, mcmd->spec.action.siid);
		}
		{
			jsmi_tok_path_t path[] = {
				{
					.key = "aiid",
					.type = JSMN_PRIMITIVE
				}
			};

			if(MIIO_OK != jsmi_get_value_uint32(&jsmi_parser, NULL, path, NELEMENTS(path), &mcmd->spec.action.aiid)){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "\"aiid\" not found");
				ret = MIIO_ERROR_PARAM;
				goto jsmi_parse_exit;
			}

			mcmd_compose_uint32(&mcmd_composer, mcmd->spec.action.aiid);
		}
		{
			jsmi_tok_path_t path[] = {
				{
					.key = "in",
					.type = JSMN_ARRAY
				}
			};
			jsmntok_t *in_tok = jsmi_get_tok(&jsmi_parser, NULL, path, NELEMENTS(path));
			if(in_tok && in_tok->size){
				for(int i = 0; i < in_tok->size; i++){

					mcmd_spec_argument_t spec_argument = {0};

					{
						jsmi_tok_path_t path[] = {
							{
								.key = (void*)i,
								.type = JSMN_OBJECT
							},
							{
								.key = "piid",
								.type = JSMN_PRIMITIVE
							}
						};

						if(MIIO_OK != jsmi_get_value_uint32(&jsmi_parser, in_tok, path, NELEMENTS(path), &spec_argument.piid)){
							LOG_ERROR_TAG(MIIO_LOG_TAG, "\"piid\" not found");
							ret = MIIO_ERROR_PARAM;
							goto jsmi_parse_exit;
						}
					}
					{
						jsmi_tok_path_t path[] = {
							{
								.key = (void*)i,
								.type = JSMN_OBJECT
							},
							{
								.key = "value",
								.type = JSMN_UNDEFINED
							}
						};

						jsmntok_t *value_tok = jsmi_get_tok(&jsmi_parser, in_tok, path, NELEMENTS(path));
						if(NULL == value_tok){
							LOG_ERROR_TAG(MIIO_LOG_TAG, "\"value\" not found");
							ret = MIIO_ERROR_PARAM;
							goto jsmi_parse_exit;
						}
						if(JSMN_STRING == value_tok->type){
							spec_argument.value = (char*)(jsmi_parser.js + value_tok->start - 1);
							spec_argument.value_len = value_tok->end - value_tok->start + 2;
						}
						else{
							spec_argument.value = (char*)(jsmi_parser.js + value_tok->start);
							spec_argument.value_len = value_tok->end - value_tok->start;
						}
					}

					mcmd_compose_uint32(&mcmd_composer, spec_argument.piid);
					mcmd_compose_bytes(&mcmd_composer, spec_argument.value, spec_argument.value_len);
				}

			}
		}
jsmi_parse_exit:
		jsmi_parse_finish(&jsmi_parser);
	}
	else{
		LOG_ERROR_TAG(MIIO_LOG_TAG, "params parse error");
		ret = MIIO_ERROR_NOMEM;
	}

	if(MIIO_OK == ret){
		if(MIIO_OK == mcmd_compose_finish(&mcmd_composer, NULL, NULL)){
			mcmd->down.result = action_result;
			mcmd->down.error = action_error;
			mcmd->io.out(mcmd->io.handle, mcmd->command_buf);
			return MIIO_OK;
		}
	}

	return MIIO_ERROR;
}
MIIO_CMD_RPC(action, do_action, NULL);

static int mcmd_spec_property_list_insert(mcmd_spec_t *mcmd_spec, mcmd_spec_property_t *property, arch_os_mutex_handle_t prop_mutex)
{
	int ret = MIIO_OK;

	mcmd_spec_property_entry_t *spec_property_entry = NULL;

	if(prop_mutex)
		arch_os_mutex_get(prop_mutex, ARCH_OS_WAIT_FOREVER);

	{//if exist
		mcmd_spec_property_entry_t *entry,*tmp_entry;
		list_for_each_entry_safe(entry, tmp_entry, &mcmd_spec->property_list, list, mcmd_spec_property_entry_t)
		{
			if(    0 == strncmp(entry->property.did, property->did, sizeof(property->did))
				&& entry->property.siid == property->siid
				&& entry->property.piid == property->piid){
				spec_property_entry = entry;
				break;
			}
		}
	}

	if(NULL == spec_property_entry){
		//insert a property
		spec_property_entry = calloc(1, sizeof(mcmd_spec_property_entry_t));
		if(NULL == spec_property_entry){
			ret = MIIO_ERROR_NOMEM;
			goto safe_exit;
		}
		INIT_LIST_HEAD(&spec_property_entry->list);
		memcpy(spec_property_entry->property.did, property->did, sizeof(property->did));
		spec_property_entry->property.siid = property->siid;
		spec_property_entry->property.piid = property->piid;
		spec_property_entry->property.value = NULL;
		spec_property_entry->property.value_len = 0;
		spec_property_entry->group_id = 0;
		spec_property_entry->last_sent_ms = 0;
		spec_property_entry->state = MCMD_SPEC_STATE_SENT;
		spec_property_entry->status = MCMD_SPEC_STATUS_CLEAR;
		list_add_tail(&spec_property_entry->list, &mcmd_spec->property_list);
	}

	if(property->value_len != spec_property_entry->property.value_len){
		if(spec_property_entry->property.value){
			free(spec_property_entry->property.value);
			spec_property_entry->property.value = NULL;
			spec_property_entry->property.value_len = 0;
		}
		spec_property_entry->property.value = calloc(1, property->value_len);
		if(NULL == spec_property_entry->property.value){
			ret = MIIO_ERROR_NOMEM;
			goto safe_exit;
		}
		spec_property_entry->property.value_len = property->value_len;
	}

	if(property->value && property->value_len){
		memcpy(spec_property_entry->property.value, property->value, property->value_len);
		spec_property_entry->status = MCMD_SPEC_STATUS_DIRTY;
	}

safe_exit:

	if(prop_mutex)
		arch_os_mutex_put(prop_mutex);

	return ret;
}

static void mcmd_spec_property_list_empty(mcmd_spec_t *mcmd_spec, arch_os_mutex_handle_t prop_mutex)
{
	if(prop_mutex)
		arch_os_mutex_get(prop_mutex, ARCH_OS_WAIT_FOREVER);

	mcmd_spec_property_entry_t *entry,*tmp_entry;
	list_for_each_entry_safe(entry, tmp_entry, &mcmd_spec->property_list, list, mcmd_spec_property_entry_t)
	{
		list_del(&entry->list);
		if(entry->property.value){
			free(entry->property.value);
		}
		free(entry);
	}

	if(prop_mutex)
		arch_os_mutex_put(prop_mutex);
}

static void mcmd_spec_property_list_refresh(mcmd_spec_t *mcmd_spec, uint32_t group_id, mcmd_spec_status_t status, mcmd_spec_state_t state, uint32_t ms, arch_os_mutex_handle_t prop_mutex)
{
	if(prop_mutex)
		arch_os_mutex_get(prop_mutex, ARCH_OS_WAIT_FOREVER);

	mcmd_spec_property_entry_t *entry;
	list_for_each_entry(entry, &mcmd_spec->property_list, list, mcmd_spec_property_entry_t)
	{
		if (entry->group_id == group_id) {
			entry->group_id = 0;
			if(ms)
				entry->last_sent_ms = ms;
			if(MCMD_SPEC_STATUS_DIRTY == status)
				entry->status = status;
			if(MCMD_SPEC_STATE_SENT == state)
				entry->state = state;
		}
	}

	if(prop_mutex)
		arch_os_mutex_put(prop_mutex);
}
/*
 * cmd:
 * 		properties_changed <siid> <piid> <value> ... <siid> <piid> <value>
 *
 * json: {
        "id": 123,
        "method": "properties_changed",
        "params": [
            {
                "did" : "1234",
                "siid" : 1,
                "piid": 1,
                "value": 17

            },

            {
                "did" : "1234",
                "siid" : 1,
                "piid": 1,
                "value": "hi"
            }
        ]
    }
 */
static int properties_changed_ack(miio_rpc_delegate_arg_t *ack_arg, void* ctx)
{
	mcmd_spec_t *mcmd_spec = (mcmd_spec_t*)ctx;
	int ret = MIIO_ERROR;
	{
		jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(ack_arg->pload, ack_arg->pload_len);
		if(MIIO_OK == jsmi_parse_start(&jsmi_parser)){
			// /params
			{
				jsmi_tok_path_t path[] = {
					{
						.key = "result",
						.type = JSMN_UNDEFINED
					}
				};
				jsmntok_t *result_tok = jsmi_get_tok(&jsmi_parser, NULL, path, NELEMENTS(path));
				if(result_tok){
					ret = MIIO_OK;
				}
				else{
					ret = MIIO_ERROR;
				}
			}

			jsmi_parse_finish(&jsmi_parser);
		}
	}

	if(MIIO_OK == ret){
		mcmd_spec_property_list_refresh(mcmd_spec, ack_arg->id, MCMD_SPEC_STATUS_CLEAR, MCMD_SPEC_STATE_SENT, arch_os_ms_now(), mcmd_spec->prop_mutex);
	}
	else{
		LOG_WARN_TAG(MIIO_LOG_TAG, "properties_changed failed");
		mcmd_spec_property_list_refresh(mcmd_spec, ack_arg->id, MCMD_SPEC_STATUS_DIRTY, MCMD_SPEC_STATE_SENT, arch_os_ms_now(), mcmd_spec->prop_mutex);
	}

	return 0;
}

static int mcmd_spec_properties_changed(miio_handle_t miio_handle, mcmd_spec_t *mcmd_spec)
{
	int prop_sent_count = 0;

	if(mcmd_spec->prop_mutex)
		arch_os_mutex_get(mcmd_spec->prop_mutex, ARCH_OS_WAIT_FOREVER);

	{
		bool prop_dirty = false;
		mcmd_spec_property_entry_t *entry;
		list_for_each_entry(entry, &mcmd_spec->property_list, list, mcmd_spec_property_entry_t)
		{
			if (   entry->status == MCMD_SPEC_STATUS_DIRTY
				&& entry->state == MCMD_SPEC_STATE_SENT
				&& arch_os_ms_elapsed(entry->last_sent_ms) > MIIO_PROPERTY_COOLDOWN_TIME_MS) {
				prop_dirty = true;
				break;
			}
		}

		if(false == prop_dirty){
			goto safe_exit;
		}
	}

	if(false == miio_is_online(miio_handle)){
		goto safe_exit;
	}

	size_t js_size = miio_get_rpc_max_size(miio_handle);
	char *js = malloc(js_size);
	if(js){
		uint32_t group_id = miio_get_rpc_id(miio_handle);

		jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, js_size);
		jsmi_compose_start(&jsmi_composer);
		jsmi_set_object_begin(&jsmi_composer);
			jsmi_set_key_value_uint(&jsmi_composer, "id", group_id);
			jsmi_set_key_value_str(&jsmi_composer, "method", "properties_changed", sizeof("properties_changed")-1);
			jsmi_set_key_array_begin(&jsmi_composer, "params");
			{
				mcmd_spec_property_entry_t *entry;
				list_for_each_entry(entry, &mcmd_spec->property_list, list, mcmd_spec_property_entry_t)
				{
					if (   entry->status == MCMD_SPEC_STATUS_DIRTY
						&& entry->state == MCMD_SPEC_STATE_SENT
						&& arch_os_ms_elapsed(entry->last_sent_ms) > MIIO_PROPERTY_COOLDOWN_TIME_MS) {
						{
							//try to composer
							jsmi_composer_t temp_composer;
							memcpy(&temp_composer, &jsmi_composer, sizeof(jsmi_composer_t));
							jsmi_set_value_object_begin(&temp_composer);
								jsmi_set_key_value_str(&temp_composer, "did", entry->property.did, strnlen(entry->property.did, sizeof(entry->property.did)));
								jsmi_set_key_value_uint32(&temp_composer, "siid", entry->property.siid);
								jsmi_set_key_value_uint32(&temp_composer, "piid", entry->property.piid);
								jsmi_set_key_value_bytes(&temp_composer, "value", entry->property.value, entry->property.value_len);
							jsmi_set_value_object_end(&temp_composer);

							jsmi_set_key_array_end(&temp_composer);
							jsmi_set_object_end(&temp_composer);
							if(MIIO_OK != jsmi_compose_finish(&temp_composer, NULL, NULL)){
								LOG_WARN_TAG(MIIO_LOG_TAG, "try to packet more prop failed");
								break;
							}
						}

						jsmi_set_value_object_begin(&jsmi_composer);
							jsmi_set_key_value_str(&jsmi_composer, "did", entry->property.did, strnlen(entry->property.did, sizeof(entry->property.did)));
							jsmi_set_key_value_uint32(&jsmi_composer, "siid", entry->property.siid);
							jsmi_set_key_value_uint32(&jsmi_composer, "piid", entry->property.piid);
							jsmi_set_key_value_bytes(&jsmi_composer, "value", entry->property.value, entry->property.value_len);
						jsmi_set_value_object_end(&jsmi_composer);

						prop_sent_count++;
						entry->status = MCMD_SPEC_STATUS_CLEAR;
						entry->state  = MCMD_SPEC_STATE_SENDING;
						entry->group_id = group_id;
					}
				}
			}
			jsmi_set_key_array_end(&jsmi_composer);
		jsmi_set_object_end(&jsmi_composer);
		if(MIIO_OK != jsmi_compose_finish(&jsmi_composer, NULL, &js_size)){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "make prop group failed");
			mcmd_spec_property_list_refresh(mcmd_spec, group_id, MCMD_SPEC_STATUS_DIRTY, MCMD_SPEC_STATE_SENT, 0, NULL);
			prop_sent_count = 0;
		}

		if(prop_sent_count > 0){
			miio_rpc_delegate_context_t context;
			miio_rpc_context_init(&context);
			miio_rpc_context_config_delegate_arg(&context, group_id, MIIO_DELEGATE_JSON, js, js_size);
			miio_rpc_context_config_delegate_ack(&context, properties_changed_ack, mcmd_spec);
			if(MIIO_OK != miio_set_up_rpc_delegate(miio_handle, &context)){
				LOG_ERROR_TAG(MIIO_LOG_TAG, "set up rpc delegate failed");
				mcmd_spec_property_list_refresh(mcmd_spec, group_id, MCMD_SPEC_STATUS_DIRTY, MCMD_SPEC_STATE_SENT, 0, NULL);
				prop_sent_count = 0;
			}
		}

		free(js);
	}

safe_exit:

	if(mcmd_spec->prop_mutex)
		arch_os_mutex_put(mcmd_spec->prop_mutex);

    return prop_sent_count;
}

static void do_properties_changed(mcmd_t *mcmd, char *params)
{
	char *argv[MCMD_COMMAND_ARG_MAX];
	int argc = mcmd_parse_params(params, argv, NELEMENTS(argv), NULL);
    if (argc && ((argc%3) == 0)) {

    	for(int i=0; i < argc; i++){
    		mcmd_spec_property_t spec_property = {0};
    		memcpy(spec_property.did, mcmd->spec.did, MIN(sizeof(spec_property.did), sizeof(mcmd->spec.did)));
    		spec_property.siid = arch_atoun(argv[i], strlen(argv[i]));
    		++i;
    		spec_property.piid = arch_atoun(argv[i], strlen(argv[i]));
    		++i;
    		spec_property.value = argv[i];
    		spec_property.value_len = strlen(argv[i]);

    		if(MIIO_OK != mcmd_spec_property_list_insert(&mcmd->spec, &spec_property, mcmd->spec.prop_mutex)){
    			goto error_exit;
    		}
		}

    	mcmd->io.out(mcmd->io.handle, "ok");

    	return;
    }

error_exit:

    mcmd->io.out(mcmd->io.handle, "error");

    return;
}
MIIO_CMD(properties_changed, do_properties_changed, NULL);


#define spec_argument_list_init(head)	INIT_LIST_HEAD(head)

static int spec_argument_list_insert(list_head_t *head, mcmd_spec_argument_t *argument)
{
	if(NULL == argument->value || 0 == argument->value_len){
		return MIIO_ERROR_PARAM;
	}

	mcmd_spec_argument_entry_t *entry = calloc(1, sizeof(mcmd_spec_argument_entry_t));

	if(NULL == entry){
		return MIIO_ERROR_NOMEM;
	}

	INIT_LIST_HEAD(&entry->list);
	entry->argument.piid = argument->piid;
	entry->argument.value_len = argument->value_len;
	entry->argument.value = malloc(entry->argument.value_len);
	if(NULL == entry->argument.value){
		free(entry);
		return MIIO_ERROR_NOMEM;
	};
	memcpy(entry->argument.value, argument->value, entry->argument.value_len);

	list_add_tail(&entry->list, head);

	return MIIO_OK;
}

static void spec_argument_list_empty(list_head_t *head)
{
	mcmd_spec_argument_entry_t *entry,*tmp_entry;
	list_for_each_entry_safe(entry, tmp_entry, head, list, mcmd_spec_argument_entry_t)
	{
		list_del(&entry->list);
		if(entry->argument.value){
			free(entry->argument.value);
		}
		free(entry);
	}
}

static int mcmd_spec_event_list_insert(mcmd_spec_t *mcmd_spec, mcmd_spec_event_t *event, arch_os_mutex_handle_t event_mutex)
{
	int ret = MIIO_OK;
	mcmd_spec_event_entry_t *spec_event_entry = NULL;

	if(event_mutex)
		arch_os_mutex_get(event_mutex, ARCH_OS_WAIT_FOREVER);

	{
	    int count = 0;
	    list_head_t *iter;
	    list_for_each(iter, &mcmd_spec->event_list){
	        count++;
	    }

	    if(count >= MIIO_EVENT_LIST_LEN_MAX){
	    	ret = MIIO_ERROR_FULL;
	    	goto error_exit;
	    }
	}

	spec_event_entry = calloc(1, sizeof(mcmd_spec_event_entry_t));
	if(NULL == spec_event_entry){
		ret = MIIO_ERROR_NOMEM;
		goto error_exit;
	}

	spec_event_entry->occur_ms = arch_os_ms_now();
	spec_event_entry->timestamp = arch_os_utc_now();
	spec_event_entry->timeout_ms = MIIO_EVENT_TIMEOUT_MS;
	spec_event_entry->state = MCMD_SPEC_STATE_WAITING;

	memcpy(spec_event_entry->event.did, event->did, sizeof(event->did));
	spec_event_entry->event.siid = event->siid;
	spec_event_entry->event.eiid = event->eiid;
	spec_argument_list_init(&spec_event_entry->event.argument_list);
	{
		mcmd_spec_argument_entry_t *entry;
		list_for_each_entry(entry, &event->argument_list, list, mcmd_spec_argument_entry_t)
		{
			ret = spec_argument_list_insert(&spec_event_entry->event.argument_list, &entry->argument);
			if(MIIO_OK != ret){
				goto error_exit;
			}
		}
	}

	list_add_tail(&spec_event_entry->list, &mcmd_spec->event_list);

	if(event_mutex)
		arch_os_mutex_put(event_mutex);

	return MIIO_OK;

error_exit:

	if(event_mutex)
		arch_os_mutex_put(event_mutex);

	if(spec_event_entry){
		spec_argument_list_empty(&spec_event_entry->event.argument_list);
		free(spec_event_entry);
	}

	return ret;
}

static void mcmd_spec_event_list_empty(mcmd_spec_t *mcmd_spec, arch_os_mutex_handle_t event_mutex)
{
	if(event_mutex)
		arch_os_mutex_get(event_mutex, ARCH_OS_WAIT_FOREVER);

	mcmd_spec_event_entry_t *entry,*tmp_entry;
	list_for_each_entry_safe(entry, tmp_entry, &mcmd_spec->event_list, list, mcmd_spec_event_entry_t)
	{
		list_del(&entry->list);
		spec_argument_list_empty(&entry->event.argument_list);
		free(entry);
	}

	if(event_mutex)
		arch_os_mutex_put(event_mutex);
}

static void mcmd_spec_event_list_refresh(mcmd_spec_t *mcmd_spec, uint32_t group_id, mcmd_spec_state_t state, arch_os_mutex_handle_t event_mutex)
{
	if(event_mutex)
		arch_os_mutex_get(event_mutex, ARCH_OS_WAIT_FOREVER);

	mcmd_spec_event_entry_t *entry,*tmp_entry;
	list_for_each_entry_safe(entry, tmp_entry, &mcmd_spec->event_list, list, mcmd_spec_event_entry_t)
	{
		if (entry->group_id == group_id) {

			if(MCMD_SPEC_STATE_SENT == state){
				list_del(&entry->list);
				spec_argument_list_empty(&entry->event.argument_list);
				free(entry);
			}
			else if(MCMD_SPEC_STATE_WAITING == state){
				entry->retry++;
				entry->state = state;
			}
		}
	}

	if(event_mutex)
		arch_os_mutex_put(event_mutex);
}

static int event_occured_ack(miio_rpc_delegate_arg_t *ack_arg, void* ctx)
{
	mcmd_spec_t *mcmd_spec = (mcmd_spec_t*)ctx;
	int ret = MIIO_ERROR;
	{
		jsmi_parser_t jsmi_parser = JSMI_PARSER_INIT(ack_arg->pload, ack_arg->pload_len);
		if(MIIO_OK == jsmi_parse_start(&jsmi_parser)){
			// /params
			{
				jsmi_tok_path_t path[] = {
					{
						.key = "result",
						.type = JSMN_UNDEFINED
					}
				};
				jsmntok_t *result_tok = jsmi_get_tok(&jsmi_parser, NULL, path, NELEMENTS(path));
				if(result_tok){
					ret = MIIO_OK;
				}
				else{
					ret = MIIO_ERROR;
				}
			}

			jsmi_parse_finish(&jsmi_parser);
		}
	}

	if(MIIO_OK == ret){
		mcmd_spec_event_list_refresh(mcmd_spec, ack_arg->id, MCMD_SPEC_STATE_SENT, mcmd_spec->event_mutex);
	}
	else{
		LOG_WARN_TAG(MIIO_LOG_TAG, "event_occured failed");
		mcmd_spec_event_list_refresh(mcmd_spec, ack_arg->id, MCMD_SPEC_STATE_WAITING, mcmd_spec->event_mutex);
	}

	return 0;
}

static int mcmd_spec_event_occured(miio_handle_t miio_handle, mcmd_spec_t *mcmd_spec)
{
	int event_sent_count = 0;

	if(mcmd_spec->event_mutex)
		arch_os_mutex_get(mcmd_spec->event_mutex, ARCH_OS_WAIT_FOREVER);

	{
		bool event_waiting = false;
		mcmd_spec_event_entry_t *event_entry,*tmp_event_entry;
		list_for_each_entry_safe(event_entry, tmp_event_entry, &mcmd_spec->event_list, list, mcmd_spec_event_entry_t)
		{
			if( arch_os_ms_elapsed(event_entry->occur_ms) >= event_entry->timeout_ms){
				LOG_WARN_TAG(MIIO_LOG_TAG, "event[ssid=%u,eiid=%u] timeout", event_entry->event.siid, event_entry->event.eiid);
				list_del(&event_entry->list);
				spec_argument_list_empty(&event_entry->event.argument_list);
				free(event_entry);
				continue;
			}

			if(MCMD_SPEC_STATE_WAITING == event_entry->state){
				event_waiting = true;
				break;
			}
		}

		if(false == event_waiting){
			goto safe_exit;
		}
	}

	if(false == miio_is_online(miio_handle)){
		goto safe_exit;
	}

	size_t js_size = miio_get_rpc_max_size(miio_handle);
	char *js = malloc(js_size);
	if(js){
		mcmd_spec_event_entry_t *event_entry,*tmp_event_entry;
		list_for_each_entry_safe(event_entry, tmp_event_entry, &mcmd_spec->event_list, list, mcmd_spec_event_entry_t)
		{
			if ( event_entry->state == MCMD_SPEC_STATE_WAITING ) {
				uint32_t group_id = 0;
				if(event_entry->retry > 0 && event_entry->group_id != 0){
					group_id = event_entry->group_id;
				}
				else{
					group_id = miio_get_rpc_id(miio_handle);
				}

				jsmi_composer_t jsmi_composer = JSMI_COMPOSER_INIT(js, js_size);
				jsmi_compose_start(&jsmi_composer);
				jsmi_set_object_begin(&jsmi_composer);
					jsmi_set_key_value_uint(&jsmi_composer, "id", group_id);
					jsmi_set_key_value_str(&jsmi_composer, "method", "event_occured", sizeof("event_occured")-1);
					jsmi_set_key_object_begin(&jsmi_composer, "params");

						jsmi_set_key_value_str(&jsmi_composer, "did", event_entry->event.did, strnlen(event_entry->event.did, sizeof(event_entry->event.did)));
						jsmi_set_key_value_uint32(&jsmi_composer, "siid", event_entry->event.siid);
						jsmi_set_key_value_uint32(&jsmi_composer, "eiid", event_entry->event.eiid);
						jsmi_set_key_array_begin(&jsmi_composer, "arguments");
							{
								mcmd_spec_argument_entry_t *argument_entry;
								list_for_each_entry(argument_entry, &event_entry->event.argument_list, list, mcmd_spec_argument_entry_t)
								{
									{
										//try to composer arguments
										jsmi_composer_t temp_composer;
										memcpy(&temp_composer, &jsmi_composer, sizeof(jsmi_composer_t));
										jsmi_set_value_object_begin(&temp_composer);
											jsmi_set_key_value_uint32(&temp_composer, "piid", argument_entry->argument.piid);
											jsmi_set_key_value_bytes(&temp_composer, "value", argument_entry->argument.value, argument_entry->argument.value_len);
										jsmi_set_value_object_end(&temp_composer);

										jsmi_set_key_array_end(&temp_composer);
										jsmi_set_object_end(&temp_composer);
										jsmi_set_key_value_uint(&temp_composer, "retry", event_entry->retry);
										jsmi_set_key_value_uint(&temp_composer, "timestamp", event_entry->timestamp);
										jsmi_set_key_value_uint(&temp_composer, "tick", event_entry->occur_ms);
										jsmi_set_object_end(&temp_composer);
										if(MIIO_OK != jsmi_compose_finish(&temp_composer, NULL, NULL)){
											LOG_WARN_TAG(MIIO_LOG_TAG, "event[ssid=%u,eiid=%u] try to packet all argument failed", event_entry->event.siid, event_entry->event.eiid);
											break;
										}
									}
									jsmi_set_value_object_begin(&jsmi_composer);
										jsmi_set_key_value_uint32(&jsmi_composer, "piid", argument_entry->argument.piid);
										jsmi_set_key_value_bytes(&jsmi_composer, "value", argument_entry->argument.value, argument_entry->argument.value_len);
									jsmi_set_value_object_end(&jsmi_composer);
								}

							}
						jsmi_set_key_array_end(&jsmi_composer);
					jsmi_set_key_object_end(&jsmi_composer);
					jsmi_set_key_value_uint(&jsmi_composer, "retry", event_entry->retry);
					jsmi_set_key_value_uint(&jsmi_composer, "timestamp", event_entry->timestamp);
					jsmi_set_key_value_uint(&jsmi_composer, "tick", event_entry->occur_ms);
				jsmi_set_object_end(&jsmi_composer);

				if(MIIO_OK != jsmi_compose_finish(&jsmi_composer, NULL, &js_size)){
					LOG_ERROR_TAG(MIIO_LOG_TAG, "make event group failed");
				}
				else {
					miio_rpc_delegate_context_t context;
					miio_rpc_context_init(&context);
					miio_rpc_context_config_delegate_arg(&context, group_id, MIIO_DELEGATE_JSON, js, js_size);
					miio_rpc_context_config_delegate_ack(&context, event_occured_ack, mcmd_spec);
					if(MIIO_OK != miio_set_up_rpc_delegate(miio_handle, &context)){
						LOG_ERROR_TAG(MIIO_LOG_TAG, "set up rpc delegate failed");
					}
					else {
						event_sent_count++;
						event_entry->state = MCMD_SPEC_STATE_SENDING;
						event_entry->group_id = group_id;
						break;
					}
				}
			}
		}

		free(js);
	}

safe_exit:

	if(mcmd_spec->event_mutex)
		arch_os_mutex_put(mcmd_spec->event_mutex);

    return event_sent_count;
}

//event_occured <siid> <eiid> <piid> <value> ... <piid> <value>
static void do_event_occured(mcmd_t *mcmd, char *params)
{
	char *argv[MCMD_COMMAND_ARG_MAX];
	int argc = mcmd_parse_params(params, argv, NELEMENTS(argv), NULL);
    if (argc >= 2 && (((argc-2)%2) == 0)) {

    	mcmd_spec_event_t spec_event = {0};
    	memcpy(spec_event.did, mcmd->spec.did, MIN(sizeof(spec_event.did), sizeof(mcmd->spec.did)));
    	spec_event.siid = arch_atoun(argv[0], strlen(argv[0]));
    	spec_event.eiid = arch_atoun(argv[1], strlen(argv[1]));
    	spec_argument_list_init(&spec_event.argument_list);
    	for(int i=2; i < argc; i++){
    		mcmd_spec_argument_t spec_argument = {0};
    		spec_argument.piid = arch_atoun(argv[i], strlen(argv[i]));
    		i++;
    		spec_argument.value = argv[i];
    		spec_argument.value_len = strlen(argv[i]);
    		if(MIIO_OK != spec_argument_list_insert(&spec_event.argument_list, &spec_argument)){
    			spec_argument_list_empty(&spec_event.argument_list);
    			goto error_exit;
    		}
		}

    	if(MIIO_OK != mcmd_spec_event_list_insert(&mcmd->spec, &spec_event, mcmd->spec.event_mutex)){
    		spec_argument_list_empty(&spec_event.argument_list);
    		goto error_exit;
    	}

    	spec_argument_list_empty(&spec_event.argument_list);

    	mcmd_spec_event_occured(mcmd->miio_handle, &mcmd->spec);

    	mcmd->io.out(mcmd->io.handle, "ok");

    	return;
	}

error_exit:

	mcmd->io.out(mcmd->io.handle, "error");

    return;
}
MIIO_CMD(event_occured, do_event_occured, NULL);

void mcmd_spec_init(mcmd_spec_t* mcmd_spec, uint64_t did)
{
	memset(mcmd_spec, 0, sizeof(mcmd_spec_t));
	arch_u64toa(did, mcmd_spec->did);
	mcmd_spec_property_rsp_list_init(&mcmd_spec->property_rsp_list);

	if(MIIO_OK != arch_os_mutex_create(&(mcmd_spec->prop_mutex))){
		LOG_WARN_TAG(MIIO_LOG_TAG, "prop mutex err");
	}
	INIT_LIST_HEAD(&(mcmd_spec->property_list));

	if(MIIO_OK != arch_os_mutex_create(&(mcmd_spec->event_mutex))){
		LOG_WARN_TAG(MIIO_LOG_TAG, "evt mutex err");
	}
	INIT_LIST_HEAD(&(mcmd_spec->event_list));
}

void mcmd_spec_deinit(mcmd_spec_t* mcmd_spec)
{
	mcmd_spec_property_rsp_list_empty(&mcmd_spec->property_rsp_list);
	mcmd_spec_property_list_empty(mcmd_spec, mcmd_spec->prop_mutex);
	mcmd_spec_event_list_empty(mcmd_spec, mcmd_spec->event_mutex);
	if(mcmd_spec->prop_mutex)
		arch_os_mutex_delete(mcmd_spec->prop_mutex);
	if(mcmd_spec->event_mutex)
		arch_os_mutex_delete(mcmd_spec->event_mutex);
	memset(mcmd_spec, 0, sizeof(mcmd_spec_t));
}

void mcmd_spec_flush(miio_handle_t miio_handle, mcmd_spec_t* mcmd_spec)
{
	static uint32_t s_last_flush_ms = 0;

	if( arch_os_ms_elapsed(s_last_flush_ms) >= MIIO_PROPERTY_COOLDOWN_TIME_MS){
		int count = 0;

		count = mcmd_spec_properties_changed(miio_handle, mcmd_spec);
		if(count > 0){
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "%d properties flushed", count);
		}
		count = mcmd_spec_event_occured(miio_handle, mcmd_spec);
		if(count > 0){
			LOG_DEBUG_TAG(MIIO_LOG_TAG, "%d event flushed", count);
		}

		s_last_flush_ms = arch_os_ms_now();
	}
}
