/**
 * Copyright (C) 2013-2015
 *
 * @author ouyangchengfeng@xiaomi.com
 * @date   2018-10-29
 *
 * @file   operation_executor.c
 *
 * @remark
 *
 */

#include "operation_executor.h"
#include "handler/on_property_get.h"
#include "handler/on_property_set.h"
#include "handler/on_action_invoke.h"
#include "codec/property_operation_decoder.h"
#include "codec/property_operation_encoder.h"
#include "codec/property_changed_encoder.h"
#include "codec/action_operation_decoder.h"
#include "codec/action_operation_encoder.h"
#include "codec/event_operation_encoder.h"
#include "arch_dbg.h"
#include "util.h"
#include "utils/arch_get_did_ex.h"
#include "miio_api.h"

#define TAG "executor"

static int miio_send_response(miio_fp_rpc_delegate_ack_t ack, uint32_t id, const char *result, void* ctx)
{
	char buffer[1024];

	memset(buffer, 0, 1024);
	snprintf(buffer, 1024 - 1, "{\"id\":%d,\"result\":%s}", id, result);

	miio_rpc_delegate_arg_t response =
	{
		.id = id,
		.type = MIIO_DELEGATE_JSON,
		.pload_len = strlen(buffer),
		.pload = buffer
	};

	return ack(&response, ctx);
}

static int miio_send_properties_changed(miio_handle_t handle, const char *params, miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
	uint32_t id = miio_get_rpc_id(handle);
	char buffer[1024];

	memset(buffer, 0, 1024);
	snprintf(buffer, 1024 - 1, "{\"id\":%d,\"method\":\"properties_changed\",\"params\":%s}", id, params);

	miio_rpc_delegate_context_t context;
	miio_rpc_context_init(&context);
	miio_rpc_context_config_delegate_arg(&context, id, MIIO_DELEGATE_JSON, buffer, strlen(buffer));
	miio_rpc_context_config_delegate_ack(&context, ack, NULL);

	return miio_set_up_rpc_delegate(handle, &context);
}

static int miio_send_event_occurred(miio_handle_t handle, const char *params, miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
	uint32_t id = miio_get_rpc_id(handle);
	char buffer[1024];

	memset(buffer, 0, 1024);
	snprintf(buffer, 1024 - 1, "{\"id\":%d,\"method\":\"event_occured\",\"params\":%s}", id, params);

	miio_rpc_delegate_context_t context;
	miio_rpc_context_init(&context);
	miio_rpc_context_config_delegate_arg(&context, id, MIIO_DELEGATE_JSON, buffer, strlen(buffer));
	miio_rpc_context_config_delegate_ack(&context, ack, NULL);

	return miio_set_up_rpc_delegate(handle, &context);
}

/**
 * request
 * ------------------------------------------------
 * {
 * 		"from":"",
 * 		"id":87604545,
 * 		"method":"get_properties",
 * 		"params":
 * 		[
 * 			{
 * 				"did":"57352830",
 * 				"piid":1,
 * 				"siid":2
 * 			}
 * 		]
 * }
 * 
 * response
 * ------------------------------------------------
 * {
 * 		"from":"",
 * 		"id":87604545,
 * 		"method":"get_properties",
 * 		"params":
 * 		[
 * 			{
 * 				"did":"57352830",
 * 				"piid":1,
 * 				"siid":2,
 * 				"code": 0,
 * 				"value": 192
 * 			}
 * 		]
 * }
 */


/**
 * request
 * ------------------------------------------------
 * {
 * 		"from":"",
 * 		"id":87604545,
 * 		"method":"set_properties",
 * 		"params":
 * 		[
 * 			{
 * 				"did":"57352830",
 * 				"piid":1,
 * 				"siid":2,
 * 				"value": true
 * 			}
 * 		]
 * }
 * 
 * response
 * ------------------------------------------------
 * {
 * 		"from":"",
 * 		"id":87604545,
 * 		"method":"set_properties",
 * 		"params":
 * 		[
 * 			{
 * 				"did":"57352830",
 * 				"piid":1,
 * 				"siid":2,
 * 				"code": 0
 * 			}
 * 		]
 * }
 */
int execute_property_operation(const char *str, int length, property_operation_type type, uint32_t id, miio_fp_rpc_delegate_ack_t ack, void *ctx)
{
	int ret = 0;

	do
	{
		bool hasValue = type == PROPERTY_OPERATION_SET;
		jsmi_parser_t parser = JSMI_PARSER_INIT(str, length);
		if(MIIO_OK != jsmi_parse_start(&parser))
		{
			ret = -1;
			break;
		}

		LOG_INFO_TAG(TAG, "property = %s\n",str);

		jsmntok_t *params = NULL;
		{
			jsmi_tok_path_t path[] =
			{
				{
					.key = "params",
					.type = JSMN_ARRAY
				}
			};

			params = jsmi_get_tok(&parser, NULL, path, NELEMENTS(path));
			if(NULL == params)
			{
				LOG_ERROR_TAG(TAG, "params not found\n");
				ret = -2;
				break;
			}
		}

		char result[1024];
		uint32_t position = 0;

		memset(result, 0, 1024);
		snprintf(result, 1024, "[");

		for (int i = 0; i < params->size; ++i)
		{
			//multi values result add ','
			if (i > 0)
			{
				position = strlen(result);
				snprintf(result + position, 1024-position, ",");
			}

			property_operation_t * o = property_operation_decode(&parser, params, i, hasValue);
			if (o == NULL)
			{
				break;
			}

			if (hasValue)
			{
				on_property_set(o);
			}
			else
			{
				on_property_get(o);
			}

			char object[128];
			size_t size = 128;

			memset(object, 0, size);

			if (property_operation_encode(o, ! hasValue, object, size) > 0)
			{
				position = strlen(result);
				snprintf(result + position, 1024-position, object);
			}

			//free value
			property_operation_delete(o);
		}

		position = strlen(result);
		snprintf(result + position, 1024-position, "]");

		jsmi_parse_finish(&parser);

		LOG_INFO_TAG(TAG, "property ack = %s\n",result);

		miio_send_response(ack, id, result, ctx);
	} while (false);

	return ret;
}

/**
 * 璇锋眰
 * ------------------------------------------------
 * {
 * 		"id":87604545,
 * 		"method":"action",
 * 		"params": {
 *      	"did" : "1234",
 *       	"siid" : 1,
 *       	"aiid" : 1,
 *       	"in" : [
 *           	{
 *              	"piid" : 1,
 *               	"value": 10
 *           	},
 * 				{
 *              	"piid" : 7,
 *               	"value": false
 *           	}
 *       	]
 *      }
 * }
 *
 * 鎴愬姛搴旂瓟
 * ------------------------------------------------
 * {
 *   "id" : 123,
 *    "result" : {
 *        "code" : 0,
 *        "out" : [
 *            {
 *                "piid" : 3,
 *                "value" : 10
 *            },
 * 		      {
 *                 "piid" : 9,
 *                 "value": 10.3
 *            }
 *        ]
 *    }
 * }
 * 
 * 澶辫触搴旂瓟
 * ------------------------------------------------
 * {
 *   "id": 123,
 *   "result" : {
 *       "code" : -4004
 *    }
 * }
 */
int execute_action_invocation(const char *str, int length, uint32_t id, miio_fp_rpc_delegate_ack_t ack, void *ctx)
{
	int ret = 0;

	LOG_INFO_TAG(TAG, "action = %s\n",str);
	do
	{
		jsmi_parser_t parser = JSMI_PARSER_INIT(str, length);
		if(MIIO_OK != jsmi_parse_start(&parser))
		{
			ret = -1;
			break;
		}

		jsmntok_t *params = NULL;
		{
			jsmi_tok_path_t path[] =
			{
				{
					.key = "params",
					.type = JSMN_OBJECT
				}
			};

			params = jsmi_get_tok(&parser, NULL, path, NELEMENTS(path));
			if(NULL == params)
			{
				LOG_ERROR_TAG(TAG, "params not found\n");
				ret = -2;
				break;
			}
		}

		char result[1024];
		uint32_t position = 0;

		memset(result, 0, 1024);
		//snprintf(result, 1024, "[");//LE+ 需要修改

		action_operation_t *o = action_operation_decode(&parser, params);
		if (o != NULL)
		{
			on_action_invoke(o);//方法处理

			char object[1024];
			size_t size = 1024;

			memset(object, 0, size);

			if (action_operation_encode_result(o, object, size) > 0)
			{
				position = strlen(result);
				snprintf(result + position, 1024 - position, object);
			}
			action_operation_delete(o);
		}

		//position = strlen(result);//LE+ 需要修改
		//snprintf(result + position, 1024 - position, "]");//LE+ 需要修改

		jsmi_parse_finish(&parser);
		LOG_INFO_TAG(TAG, "action ack = %s\n",result);

		miio_send_response(ack, id, result, ctx);
	} while (false);

	return ret;
}

/**
 * request
 * ------------------------------------------------
 * {
 *     "id": 123,
 *     "method": "properties_changed",
 *     "params": [
 *         {
 *            "did" : "1234",
 *            "siid" : 1,
 *            "piid": 1,
 *            "value": 17
 *         },
 *         {
 *            "did" : "1234",
 *            "siid" : 3,
 *            "piid": 2,
 *            "value": 11.3
 *         }
 *    ]
 * }
 */
static int _changed_ack(miio_rpc_delegate_arg_t *ack_arg, void* ctx)
{
    LOG_ERROR_TAG(TAG, "%s called", __FUNCTION__);
    return 0;
}

/*
*发送改变的属性
*/
int send_property_changed(miio_handle_t handle, uint32_t siid, uint32_t piid, property_value_t *newValue)
{
	int ret = 0;
	property_operation_t *o = NULL;

	do
	{
		if (handle == NULL)
		{
			LOG_ERROR_TAG(TAG, "handle is null\n");
			ret = -1;
			break;
		}

		if (newValue == NULL)
		{
			LOG_ERROR_TAG(TAG, "newValue is null\n");
			ret = -2;
			break;
		}

		o = property_operation_new_value(arch_get_did_ex(), siid, piid, newValue);
		if (o == NULL)
		{
			LOG_ERROR_TAG(TAG, "property_operation_new_value failed!\n");
			property_value_delete(newValue);
			ret = -3;
			break;
		}

		char buf[512];
		property_changed_encode(o, buf, 512);

		LOG_INFO_TAG(TAG, "action function = %s\n",buf);

		ret = miio_send_properties_changed(handle, buf, _changed_ack, NULL);
	} while (false);

	if (o != NULL)
	{
		property_operation_delete(o);
	}

	return ret;
}

/**
 * request
 * ------------------------------------------------
 * {
 *     "id": 123,
 *     "method": "event_occured",
 *     "params": {
 *         "did": "xxx",
 *         "siid": 1,
 *         "eiid": 4,
 *         "arguments": [
 *            {
 *              "piid": 1,
 *              "value": 17
 *            },
 *            {
 *              "piid": 2,
 *              "value": 11.3
 *           }
 *         ]
 *    }
 * }
 */
static int _event_ack(miio_rpc_delegate_arg_t *ack_arg, void* ctx)
{
    LOG_ERROR_TAG(TAG, "%s called", __FUNCTION__);
    return 0;
}

int send_event_occurred(miio_handle_t handle, event_operation_t *event)
{
	int ret = 0;
	do
	{
		if (handle == NULL)
		{
			LOG_ERROR_TAG(TAG, "handle is null\n");
			ret = -1;
			break;
		}

		if (event == NULL)
		{
			LOG_ERROR_TAG(TAG, "event is null\n");
			ret = -2;
			break;
		}

		char buf[512];
		event_operation_encode(event, buf, 512);

		ret = miio_send_event_occurred(handle, buf, _event_ack, NULL);
	} while (false);

	return ret;
}
