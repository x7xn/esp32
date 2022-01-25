/******************************************************************************
 * Main function used here.
 *
 * FileName: miio_main.c
 *
 * Description: entry file of user application
 *
 *
 * Time: 2016.10.
 *
*******************************************************************************/


/*********************************************************************
 * INCLUDES
 */
#include "miio_instance.h"
#include "miio_arch.h"
#include "jsmi.h"
#include "device/operation_executor.h"
#include "light.h"
#include "light_remote.h"
#include "light_drivers.h"

#define TAG "miio_main"

extern void light_remote_blereset(void);
extern void light_remote_offline(void);
extern void light_remote_online(void);

miio_handle_t g_miio_instance_handle = NULL;

static int app_online_hook_default(miio_handle_t handle, void *ctx)
{
	LOG_INFO_TAG("app_main", "%s called", __FUNCTION__);
	light_remote_online();
	return MIIO_OK;
}

static int app_offline_hook_default(miio_handle_t handle, void *ctx)
{
	LOG_INFO_TAG("app_main", " %s called", __FUNCTION__);
	light_remote_offline();
	return MIIO_OK;
}

static int app_restore_hook_default(miio_handle_t handle, void *ctx)
{
	LOG_INFO_TAG("app_main", " %s called", __FUNCTION__);
	light_remote_blereset();
	return MIIO_OK;
}

static int app_ota_task_hook_default(miio_handle_t handle, const char* ota_task_name, miio_ota_task_state_t ota_state, int ota_progress, void* ctx)
{
	switch(ota_state){
	case MIIO_OTA_TASK_DOWNLOADING:
		LOG_INFO_TAG("app_main", "ota_task=%X, name=\"%s\", state=\"%s\", progress=%d", (uint32_t)miio_get_ota_task_handle(handle, ota_task_name), ota_task_name, "downloading", ota_progress);
		break;
	case MIIO_OTA_TASK_DOWNLOADED:
		LOG_INFO_TAG("app_main", "ota_task=%X, name=\"%s\", state=\"%s\", progress=%d", (uint32_t)miio_get_ota_task_handle(handle, ota_task_name), ota_task_name, "downloaded", ota_progress);
		break;
	case MIIO_OTA_TASK_INSTALLING:
		LOG_INFO_TAG("app_main", "ota_task=%X, name=\"%s\", state=\"%s\", progress=%d", (uint32_t)miio_get_ota_task_handle(handle, ota_task_name), ota_task_name, "installing", ota_progress);
		break;
	case MIIO_OTA_TASK_INSTALLED:
		LOG_INFO_TAG("app_main", "ota_task=%X, name=\"%s\", state=\"%s\", progress=%d", (uint32_t)miio_get_ota_task_handle(handle, ota_task_name), ota_task_name, "installed", ota_progress);
		break;
	case MIIO_OTA_TASK_FAILED:
		LOG_INFO_TAG("app_main", "ota_task=%X, name=\"%s\", state=\"%s\", progress=%d", (uint32_t)miio_get_ota_task_handle(handle, ota_task_name), ota_task_name, "failed", ota_progress);
		break;
	default:
		LOG_ERROR_TAG("app_main", "ota_task=%X, name=\"%s\", state=%d, progress=%d", (uint32_t)miio_get_ota_task_handle(handle, ota_task_name), ota_task_name, ota_state, ota_progress);
		break;
	}

	return MIIO_OK;
}

#if MIIO_AUTO_OTA_ENABLE
/******************************************************************************
 * FunctionName : app_auto_ota_hook_default
 * Description  : determine if the device is busy for auto ota
 * Parameters   : handle: miio handler
                  device_state: device state flag
                  ctx: unused
 * Returns      : MIIO_OK
*******************************************************************************/
int app_auto_ota_hook_default(miio_handle_t handle, int *device_state, void* ctx)
{
	LOG_INFO_TAG("app_main", "%s called", __FUNCTION__);

	*device_state = MIIO_AUTO_OTA_READY;

	/**************************************/
	/* user logic code: *device_state = ? */
	/**************************************/

	return MIIO_OK;
}
#endif /* MIIO_AUTO_OTA_ENABLE */

static miio_hooks_t s_app_hooks = {
	.info = NULL,
	.ext_rpc = NULL,
	.online = app_online_hook_default,
	.offline = app_offline_hook_default,
	.restore = app_restore_hook_default,
	.reboot = NULL,
	.statistics = NULL,
	.ota_task = app_ota_task_hook_default,
#if MIIO_AUTO_OTA_ENABLE
	.auto_ota = app_auto_ota_hook_default,
#endif /* MIIO_AUTO_OTA_ENABLE */
	.ctx = NULL
};

/******************************************************************************
 * FunctionName : app_main
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void app_main(void)
{
	//miio_handle_t miio_handle = NULL;

	//	miio_handle = miio_instance_create();
	g_miio_instance_handle = miio_instance_create();


	//	if(NULL == miio_handle){
	if (NULL == g_miio_instance_handle){
		LOG_ERROR_TAG("app_main", "miio create error!");
		return;
	}
	//light_task_start(g_miio_instance_handle);

	miio_hooks_register(g_miio_instance_handle, &s_app_hooks);
}


static int do_get_properties(miio_rpc_delegate_arg_t *req_arg, miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
    LOG_ERROR_TAG(TAG, "--------------------- get_properties -------------------\r\n");

    do
	{
		int ret = 0;
        if (MIIO_DELEGATE_JSON != req_arg->type)
		{
			miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID, MIIO_OT_ERR_INFO_PARAM_INVALID, ack, ctx);
			break;
		}

		ret = execute_property_operation((const char *)req_arg->pload, req_arg->pload_len, PROPERTY_OPERATION_GET, req_arg->id, ack, ctx);
		if (ret != 0)
		{
			miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID, MIIO_OT_ERR_INFO_PARAM_INVALID, ack, ctx);
			break;
		}
	} while (0);

	return MIIO_OK;
}
MIIO_RPC_USER(get_properties, do_get_properties, "get_properties");

static int do_set_properties(miio_rpc_delegate_arg_t *req_arg, miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
    LOG_ERROR_TAG(TAG, " ------------------- set_properties -------------------\n");
	do
	{
		int ret = 0;
        if (MIIO_DELEGATE_JSON != req_arg->type)
		{
			miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID, MIIO_OT_ERR_INFO_PARAM_INVALID, ack, ctx);
			break;
		}

		ret = execute_property_operation((const char *)req_arg->pload, req_arg->pload_len, PROPERTY_OPERATION_SET, req_arg->id, ack, ctx);
		if (ret != 0)
		{
			miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID, MIIO_OT_ERR_INFO_PARAM_INVALID, ack, ctx);
			break;
		}
	} while (0);

	return MIIO_OK;
}
MIIO_RPC_USER(set_properties, do_set_properties, "set_properties");

static int do_invoke_action(miio_rpc_delegate_arg_t *req_arg, miio_fp_rpc_delegate_ack_t ack, void* ctx)
{
    LOG_ERROR_TAG(TAG, " ------------------- action -------------------\n");

    do
    {
        int ret = 0;
        if (MIIO_DELEGATE_JSON != req_arg->type)
        {
            miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID, MIIO_OT_ERR_INFO_PARAM_INVALID, ack, ctx);
            break;
        }

        ret = execute_action_invocation((const char *)req_arg->pload, req_arg->pload_len, req_arg->id, ack, ctx);
        if (ret != 0)
        {
            miio_rpc_delegate_ack_error(req_arg, MIIO_OT_ERR_CODE_PARAM_INVALID, MIIO_OT_ERR_INFO_PARAM_INVALID, ack, ctx);
            break;
        }
    } while (0);

    return MIIO_OK;
}
MIIO_RPC_USER(action, do_invoke_action, "action");



