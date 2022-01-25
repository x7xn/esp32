/**
* @file    miio_api.h
* @author  mashaoze
* @date    2017
* @par     Copyright (c):
*
*    Copyright 2017 MIoT,MI
*
*    Licensed under the Apache License, Version 2.0 (the "License");
*    you may not use this file except in compliance with the License.
*    You may obtain a copy of the License at
*
*        http://www.apache.org/licenses/LICENSE-2.0
*
*    Unless required by applicable law or agreed to in writing, software
*    distributed under the License is distributed on an "AS IS" BASIS,
*    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*    See the License for the specific language governing permissions and
*    limitations under the License.
*/
#ifndef _MIIO_API_H_
#define _MIIO_API_H_

#include "miio_define.h"

/**
 * get did of miio
 * @param[in]   miio_handle		handle of miio
 * @retval      did				device id, type is uint64_t
 */
uint64_t miio_get_did(miio_handle_t miio_handle);

/**
 * get country_domain of miio
 * @param[in]   miio_handle			handle of miio
 * @param[in]   country_domain		buffer for country_domain
 * @param[in]   country_domain_size size of buffer
 * @retval      country_domain		buffer for country_domain, end with '\0'
 * @warning     if buffer for country_domain is smaller than the size of miio needed, country_domain will been cut
 */
char* miio_get_country_domain(miio_handle_t miio_handle, char *country_domain, size_t country_domain_size);

/**
 * set country_domain of miio
 * @param[in]   miio_handle 		handle of miio
 * @param[in]   country_domain
 * @retval      void
 * @warning     if length of country_domain is longer than the size of miio needed, country_domain will been cut
 */
void miio_set_country_domain(miio_handle_t miio_handle, const char *country_domain);

/**
 * get model of miio
 * @param[in]   miio_handle			handle of miio
 * @param[in]   model				buffer for model
 * @param[in]   model_size			size of buffer
 * @retval      model				buffer for model, end with '\0'
 * @warning     if buffer for model is smaller than the size of miio needed, model will been cut
 */
char* miio_get_model(miio_handle_t miio_handle, char *model, size_t model_size);

/**
 * set model of miio
 * @param[in]   miio_handle			handle of miio
 * @param[in]   model
 * @retval      void
 * @warning     if length of model is longer than the size of miio needed, model will been cut
 */
void miio_set_model(miio_handle_t miio_handle, const char *model);

/**
 * get uid of miio
 * @param[in]   miio_handle 		handle of miio
 * @retval      uid  				user id, type is uint64_t
 */
uint64_t miio_get_uid(miio_handle_t miio_handle);

/**
 * set uid of miio
 * @param[in]   miio_handle			handle of miio
 * @param[in]   uid					user id, type is uint64_t
 * @retval      void
 */
void miio_set_uid(miio_handle_t miio_handle, uint64_t uid);

/**
 * get gmt_offset of miio
 * @param[in]   miio_handle			handle of miio
 * @retval      gmt_offset
 */
int miio_get_gmt_offset(miio_handle_t miio_handle);

/**
 * set gmt_offset of miio
 * @param[in]   miio_handle			handle of miio
 * @param[in]   gmt_offset
 * @retval      void
 */
void miio_set_gmt_offset(miio_handle_t miio_handle, int gmt_offset);

/**
 * set property, miio will report this property at the proper time
 * @param[in]   miio_handle			handle of miio
 * @param[in]   key					the name of property, type is string
 * @param[in]   value				the value of property, type is json value
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_set_property(miio_handle_t miio_handle, const char *key, const char *value);

/**
 * set property, miio will report this property at the proper time
 * @param[in]   miio_handle			handle of miio
 * @param[in]   key					the name of property, type is string
 * @param[in]   value				the value of property, type is int
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_set_property_int(miio_handle_t miio_handle, const char *key, int value);

/**
 * set property, miio will report this property at the proper time
 * @param[in]   miio_handle			handle of miio
 * @param[in]   key					the name of property, type is string
 * @param[in]   value				the value of property, type is unsigned int
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_set_property_uint(miio_handle_t miio_handle, const char *key, unsigned int value);

/**
 * set property, miio will report this property at the proper time
 * @param[in]   miio_handle			handle of miio
 * @param[in]   key					the name of property, type is string
 * @param[in]   value				the value of property, type is int64
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_set_property_int64(miio_handle_t miio_handle, const char *key, int64_t value);

/**
 * set property, miio will report this property at the proper time
 * @param[in]   miio_handle			handle of miio
 * @param[in]   key					the name of property, type is string
 * @param[in]   value				the value of property, type is uint64
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_set_property_uint64(miio_handle_t miio_handle, const char *key, uint64_t value);

/**
 * set property, miio will report this property at the proper time
 * @param[in]   miio_handle			handle of miio
 * @param[in]   key					the name of property, type is string
 * @param[in]   value				the value of property, type is double
 * @param[in]   precision			the precision of property, type is unsigned int
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_set_property_double(miio_handle_t miio_handle, const char *key, double value, unsigned int precision);

/**
 * set property, miio will report this property at the proper time
 * @param[in]   miio_handle			handle of miio
 * @param[in]   key					the name of property, type is string
 * @param[in]   value				the value of property, type is string
 * @param[in]   value_len			string length
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_set_property_str(miio_handle_t miio_handle, const char *key, const char* value, size_t value_len);

/**
 * set property, miio will report this property at the proper time
 * @param[in]   miio_handle			handle of miio
 * @param[in]   key					the name of property, type is string
 * @param[in]   value				the value of property, type is bool
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_set_property_bool(miio_handle_t miio_handle, const char *key, bool value);


/**
 * set property, miio will report this property at the proper time
 * @param[in]   miio_handle			handle of miio
 * @param[in]   key					the name of property, type is string
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_set_property_null(miio_handle_t miio_handle, const char *key);


/**
 * flush property, miio will report all of the properties to cloud immediately
 * @param[in]   miio_handle			handle of miio
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_flush_property(miio_handle_t miio_handle);

/**
 * set event, miio will report this event at the proper time to avoid cloud cooldown
 * @param[in]   miio_handle			handle of miio
 * @param[in]   key					the name of event, type is string
 * @param[in]   value				the value of event, type is an array of js value
 * @param[in]   value_count			the count of value
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_set_event(miio_handle_t miio_handle, const char *key, const char *value[], size_t value_count);

/**
 * set event, miio will report this event at the proper time to avoid cloud cooldown
 * @param[in]   miio_handle			handle of miio
 * @param[in]   key					the name of event, type is string
 * @param[in]   value				the value of event, type is int
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_set_event_int(miio_handle_t miio_handle, const char *key, int value);

/**
 * set event, miio will report this event at the proper time to avoid cloud cooldown
 * @param[in]   miio_handle			handle of miio
 * @param[in]   key					the name of event, type is string
 * @param[in]   value				the value of event, type is unsigned int
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_set_event_uint(miio_handle_t miio_handle, const char *key, unsigned int value);

/**
 * set event, miio will report this event at the proper time to avoid cloud cooldown
 * @param[in]   miio_handle			handle of miio
 * @param[in]   key					the name of event, type is string
 * @param[in]   value				the value of event, type is int64
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_set_event_int64(miio_handle_t miio_handle, const char *key, int64_t value);

/**
 * set event, miio will report this event at the proper time to avoid cloud cooldown
 * @param[in]   miio_handle			handle of miio
 * @param[in]   key					the name of event, type is string
 * @param[in]   value				the value of event, type is uint64
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_set_event_uint64(miio_handle_t miio_handle, const char *key, uint64_t value);

/**
 * set event, miio will report this event at the proper time to avoid cloud cooldown
 * @param[in]   miio_handle			handle of miio
 * @param[in]   key					the name of event, type is string
 * @param[in]   value				the value of property, type is double
 * @param[in]   precision			the precision of property, type is unsigned int
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_set_event_double(miio_handle_t miio_handle, const char *key, double value, unsigned int precision);

/**
 * set event, miio will report this event at the proper time to avoid cloud cooldown
 * @param[in]   miio_handle			handle of miio
 * @param[in]   key					the name of event, type is string
 * @param[in]   value				the value of property, type is string
 * @param[in]   value_len			string length
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_set_event_str(miio_handle_t miio_handle, const char *key, const char* value, size_t value_len);

/**
 * set event, miio will report this event at the proper time to avoid cloud cooldown
 * @param[in]   miio_handle			handle of miio
 * @param[in]   key					the name of event, type is string
 * @param[in]   value				the value of property, type is bool
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_set_event_bool(miio_handle_t miio_handle, const char *key, bool value);


/**
 * set event, miio will report this event at the proper time to avoid cloud cooldown
 * @param[in]   miio_handle			handle of miio
 * @param[in]   key					the name of event, type is string
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_set_event_null(miio_handle_t miio_handle, const char *key);

/**
 * flush event, miio will report all of the events to cloud immediately
 * @param[in]   miio_handle			handle of miio
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_flush_event(miio_handle_t miio_handle);

/**
 * init a miio_rpc_delegate_context_t
 * @param[in]   context
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_rpc_context_init(miio_rpc_delegate_context_t *context);

/**
 * config arguments of RPC in miio_rpc_delegate_context_t
 * @param[in]   context				point of miio_rpc_delegate_context_t
 * @param[in]   method_id			id of RPC
 * @param[in]   type				miio_delegate_type_t
 * @param[in]   pload				point of RPC
 * @param[in]   pload_len			length of pload
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_rpc_context_config_delegate_arg(miio_rpc_delegate_context_t *context, uint32_t method_id, miio_delegate_type_t type, void* pload, size_t pload_len);

/**
 * config timeout in in miio_rpc_delegate_context_t
 * @param[in]   context				point of miio_rpc_delegate_context_t
 * @param[in]   timeout_ms
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_rpc_context_config_delegate_ctrl(miio_rpc_delegate_context_t* context, uint32_t timeout_ms);

/**
 * config callback in in miio_rpc_delegate_context_t
 * @param[in]   ack					function point
 * @param[in]   ack_ctx				point of context for ack
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_rpc_context_config_delegate_ack(miio_rpc_delegate_context_t* context, miio_fp_rpc_delegate_ack_t ack, void* ack_ctx);

/**
 * set an delegate to call rpc of cloud
 * @param[in]   miio_handle			handle of miio
 * @param[in]   rpc_context			point of miio_rpc_delegate_context_t
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_set_up_rpc_delegate(miio_handle_t miio_handle, miio_rpc_delegate_context_t *rpc_context);

/**
 * get an unhandled delegate which is from cloud
 * @param[in]   miio_handle			handle of miio
 * @param[in]   rpc_context			point of miio_rpc_delegate_context_t
 * @retval      MIIO_OK				OK
 * @retval      !MIIO_OK			error
 */
int miio_get_down_rpc_delegate_ext(miio_handle_t miio_handle, miio_rpc_delegate_context_t *rpc_context);

/**
 * convert an ctx of delegate to miio_handle_t
 * @param[in]   ctx					context of rpc
 * @retval      miio_handle
 */
miio_handle_t miio_from_delegtate_ctx(void *ctx);

/**
 * get the max size of rpc
 * @param[in]   miio_handle			handle of miio
 * @retval      rpc_max_size
 */
size_t miio_get_rpc_max_size(miio_handle_t miio_handle);

/**
 * get an unique id for rpc
 * @param[in]   miio_handle			handle of miio
 * @retval      rpc_id
 */
uint32_t miio_get_rpc_id(miio_handle_t miio_handle);

/**
 * attach an ota task to miio
 * @param[in]   miio_handle			handle of miio
 * @param[in]   task_name
 * @param[in]   state				initial state of ota task
 * @param[in]   task_if				miio_ota_task_hook_if_t
 * @param[in]   task_ctx			context of ota task
 * @retval      !NULL				OK
 * @retval		NULL				failed
 */
miio_ota_task_handle_t miio_ota_task_attach(miio_handle_t miio_handle, const char *task_name, miio_ota_task_state_t state, const miio_ota_task_hook_if_t* task_if, void* task_ctx);

/**
 * detach an ota task from miio
 * @param[in]   miio_handle			handle of miio
 * @param[in]   task_name
 * @retval      MIIO_OK				OK
 * @retval		!MIIO_OK			failed
 */
int miio_ota_task_detach(miio_handle_t miio_handle, const char *task_name);

/**
 *
 * @param[in]   ota_task_handle		handle of ota task
 * @retval      void
 */
void miio_ota_task_install_start(miio_ota_task_handle_t ota_task_handle);

/**
 *
 * @param[in]   ota_task_handle		handle of ota task
 * @param[in]   progress			handle of ota task
 * @retval      void
 */
void miio_ota_task_install_update(miio_ota_task_handle_t ota_task_handle, uint32_t progress);

/**
 *
 * @param[in]   ota_task_handle		handle of ota task
 * @retval      void
 */
void miio_ota_task_install_finish(miio_ota_task_handle_t ota_task_handle);

/**
 *
 * @param[in]   ota_task_handle		handle of ota task
 * @param[in]   code				error code
 * @param[in]   message				error message
 * @retval      void
 */
void miio_ota_task_install_error(miio_ota_task_handle_t ota_task_handle, int code, const char *message);

/**
 * set download_params of ota task
 * @param[in]   ota_task_handle		handle of ota task
 * @param[in]   download_params		miio_download_params_t
 * @retval      MIIO_OK				OK
 * @retval		!MIIO_OK			failed
 */
int miio_ota_task_set_download_params(miio_ota_task_handle_t ota_task_handle, miio_download_params_t *download_params);

/**
 * get download_params of ota task
 * @param[in]   ota_task_handle		handle of ota task
 * @retval      miio_download_params_t*
 */
miio_download_params_t* miio_ota_task_get_download_params(miio_ota_task_handle_t ota_task_handle);

/**
 * get state of ota task
 * @param[in]   ota_task_handle		handle of ota task
 * @retval      miio_ota_task_state_t
 */
miio_ota_task_state_t miio_ota_task_get_state(miio_ota_task_handle_t ota_task_handle);

/**
 * get progress of ota task
 * @param[in]   ota_task_handle		handle of ota task
 * @retval      progress
 */
int miio_ota_task_get_progress(miio_ota_task_handle_t ota_task_handle);

/**
 * get name of ota task
 * @param[in]   ota_task_handle		handle of ota task
 * @retval      name
 */
const char* miio_ota_task_get_name(miio_ota_task_handle_t ota_task_handle);

/**
* set ota params and start ota task
* @param[in]   miio_handle        handle of miio
* @param[in]   req_arg            miio_rpc_delegate_arg_t
* @retval      MIIO_OK            OK
* @retval      !MIIO_OK           failed
*/
int miio_ota_task_start(miio_handle_t miio_handle, miio_rpc_delegate_arg_t *req_arg);

/**
* get app auto ota ability
* @param[in]   miio_handle        handle of miio
* @retval      ability            app auto ota ability, type is bool
*/
bool miio_get_app_auto_ota_ability(miio_handle_t miio_handle);

/**
* set app auto ota ability
* @param[in]   miio_handle        handle of miio
* @param[in]   ability            app auto ota ability, type is bool
*/
void miio_set_app_auto_ota_ability(miio_handle_t miio_handle, bool ability);

/**
* get mcu auto ota ability
* @param[in]   miio_handle        handle of miio
* @retval      ability            mcu auto ota ability, type is bool
*/
bool miio_get_mcu_auto_ota_ability(miio_handle_t miio_handle);

/**
* set mcu auto ota ability
* @param[in]   miio_handle        handle of miio
* @param[in]   ability            app auto ota ability, type is bool
*/
void miio_set_mcu_auto_ota_ability(miio_handle_t miio_handle, bool ability);


/**
 * create a miio
 * @param[in]   configs				point of miio_configs_t
 * @retval      miio_handle_t
 */
miio_handle_t miio_create(miio_configs_t *configs);

/**
 * destroy a miio
 * @param[in]   miio_handle
 * @retval      void
 */
void miio_destroy(miio_handle_t miio_handle);

/**
 * set mode of miio
 * @param[in]   miio_handle
 * @param[in]   mode
 * @retval      void
 */
void miio_set_mode(miio_handle_t miio_handle, miio_mode_t mode);

/**
 * get mode of miio
 * @param[in]   miio_handle
 * @retval      miio_mode_t
 */
miio_mode_t miio_get_mode(miio_handle_t miio_handle);

/**
 * enable provision miio
 * @param[in]   miio_handle
 * @retval      MIIO_OK				OK
 * @retval		!MIIO_OK			failed
 */
int miio_enable_provision(miio_handle_t miio_handle);

/**
 * disable provision miio
 * @param[in]   miio_handle
 * @retval      MIIO_OK				OK
 * @retval		!MIIO_OK			failed
 */
int miio_disable_provision(miio_handle_t miio_handle);

/**
 * add an rpc to blacklist
 * @param miio_handle 
 * @retval      MIIO_OK				OK
 * @retval		!MIIO_OK			failed
 */
int miio_rpc_blacklist_add(miio_handle_t miio_handle, const char * rpc_name);

/**
 * delete an rpc from blacklist
 * @param miio_handle 
 * @retval      MIIO_OK				OK
 * @retval		!MIIO_OK			failed
 */
int miio_rpc_blacklist_del(miio_handle_t miio_handle, const char * rpc_name);
/**
 * check miio is online or not
 * @param[in]   miio_handle			handle of miio
 * @retval      true				miio is online
 * @retval		false				miio is offline
 */
bool miio_is_online(miio_handle_t miio_handle);

/**
 * check miio is updating or not
 * @param[in]   miio_handle			handle of miio
 * @retval      true				miio is updating
 * @retval		false				miio is not updating
 */
bool miio_is_updating(miio_handle_t miio_handle);

/**
 * get miio ota progress
 * @param[in]   miio_handle			handle of miio
 * @retval      ota progress		[0,101]
 */
int miio_get_ota_progress(miio_handle_t miio_handle);

/**
 * get miio ota state
 * @param[in]   miio_handle			handle of miio
 * @retval      ota state			miio_ota_task_state_t
 */
miio_ota_task_state_t miio_get_ota_state(miio_handle_t miio_handle);

/**
 * get miio ota task handle
 * @param[in]   miio_handle			handle of miio
 * @param[in]   name				name of task
 * @retval      ota task_handle		miio_ota_task_handle_t
 */
miio_ota_task_handle_t miio_get_ota_task_handle(miio_handle_t miio_handle, const char* name);

/**
 * gerneric ack handle function for rpc delegate
 * @param[in]   req_arg				miio_rpc_delegate_arg_t*
 * @param[in]   ack					miio_fp_rpc_delegate_ack_t
 * @param[in]   ack_ctx				void*
 * @retval      MIIO_OK				OK
 * @retval		!MIIO_OK			failed
 */
int miio_rpc_delegate_ack_ok(miio_rpc_delegate_arg_t* req_arg, miio_fp_rpc_delegate_ack_t ack, void* ack_ctx);

/**
 * gerneric ack handle function for rpc delegate
 * @param[in]   req_arg				miio_rpc_delegate_arg_t*
 * @param[in]   err_code			code of error
 * @param[in]   err_msg				messge of error
 * @param[in]   ack					miio_fp_rpc_delegate_ack_t
 * @param[in]   ack_ctx				context for ack
 * @retval      MIIO_OK				OK
 * @retval		!MIIO_OK			failed
 */
int miio_rpc_delegate_ack_error(miio_rpc_delegate_arg_t* req_arg, int err_code, const char* err_msg, miio_fp_rpc_delegate_ack_t ack, void* ack_ctx);

/**
 * register hooks in miio
 * @param[in]   miio_handle			handle of miio
 * @param[in]   hooks				miio_hooks_t *
 * @retval      MIIO_OK				OK
 * @retval		!MIIO_OK			failed
 */
int miio_hooks_register(miio_handle_t miio_handle, miio_hooks_t* hooks);

/**
 * unregister hooks in miio
 * @param[in]   miio_handle			handle of miio
 * @param[in]   hooks				miio_hooks_t*
 * @retval      MIIO_OK				OK
 * @retval		!MIIO_OK			failed
 */
int miio_hooks_unregister(miio_handle_t miio_handle, miio_hooks_t* hooks);

/**
 * run http request in miio
 * @param[in]   miio_handle			handle of miio
 * @param[in]   params				miio_httpc_req_params_t*
 * @param[in]   io_if				miio_stream_io_if_t*
 * @param[in]   io_ctx				context for io_if
 * @retval      MIIO_OK				OK
 * @retval		!MIIO_OK			failed
 */
int miio_httpc_req(miio_handle_t miio_handle, miio_httpc_req_params_t* params, const miio_stream_io_if_t* io_if, void* io_ctx);

/**
 * get miio_instance_version
 * @retval      const char*
 */
const char* miio_instance_version(void);

/**
 * format miio_app_version
 * @param[in]   app_version_number	<=9999
 * @retval      const char*
 */
const char* miio_app_version(unsigned int app_version_number);

/**
 * reboot miio
 * @param[in]   miio_handle			handle of miio
 * @param[in]   reason				reason string
 */
void miio_reboot(miio_handle_t miio_handle, const char *reason, uint32_t timeout_ms);

/**
 * restore miio
 * @param[in]   miio_handle			handle of miio
 * @param[in]   reason				reason string
 */
void miio_restore(miio_handle_t miio_handle, const char *reason);

/**
 * miio ntp
 * @param[in]   miio_handle			handle of miio
 * @param[in]   ack					async callback for ntp, result is MIIO_OK or !MIIO_OK
 * @param[in]   ack_ctx				context for callback
 * @param[in]   timeout_ms			timeout for ntp's result
 * @retval      MIIO_OK				OK
 * @retval		!MIIO_OK			failed
 */
int miio_ntp(miio_handle_t miio_handle,
		void (*ack)(int result, uint32_t utc, void* ack_ctx), void *ack_ctx, uint32_t timeout_ms);

/**
 * miio_set_bind_index
 * @param[in]   miio_handle         handle of miio
 * @param[in]   bind_info           bind index
 * @retval      MIIO_OK             OK
 * @retval      !MIIO_OK            failed
 */
int miio_set_bind_index(miio_handle_t miio_handle, void *arg);

#endif /* _MIIO_API_H_ */
