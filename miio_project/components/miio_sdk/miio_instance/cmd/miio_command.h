#ifndef MIIO_COMMAND_H_
#define MIIO_COMMAND_H_

#include "miio_ota_mcu.h"
#include "miio_ota_app.h"
#include "miio_instance.h"
#include "list/list.h"
#include "miio_arch.h"

#define MCMD_OLD_PROTO_ADAPT				1
#define MCMD_COMMAND_LEN_MAX				(900)
#define MCMD_COMMAND_ARG_MAX				(64)
#define MCMD_JSON_BUF_SIZE					(1024)

typedef enum{
	MCMD_DOWN_IDLE = 0,
	MCMD_DOWN_RPC,
	MCMD_DOWN_OTA
}MCMD_DOWN_STATE;

typedef enum{
	MCMD_UP_IDLE = 0,
	MCMD_UP_RPC,
	MCMD_UP_HAD_RESULT,
	MCMD_UP_HAD_ERROR,
}MCMD_UP_STATE;

typedef void* (*fp_mcmd_io_create_t)(void *arg);
typedef int (*fp_mcmd_io_block_in_t)(void *handle, char *buf, int buf_size, int idle_timeout_ms);
typedef void (*fp_mcmd_io_block_quit_t)(void *handle);
typedef int (*fp_mcmd_io_out_t)(void *handle, const char *str);
typedef void (*fp_mcmd_io_destroy_t)(void **handle);
typedef int (*fp_mcmd_io_echo_t)(void *handle, const char *arg);
typedef int (*fp_mcmd_io_int_byte_t)(void *handle, unsigned char *c, int timeout_ms);
typedef void (*fp_mcmd_io_out_byte_t)(void *handle, unsigned char c);

typedef struct{
	void *handle;
	fp_mcmd_io_create_t 	create;
	fp_mcmd_io_block_in_t 	block_in;
	fp_mcmd_io_block_quit_t block_quit;
	fp_mcmd_io_out_t		out;
	fp_mcmd_io_destroy_t	destroy;
	fp_mcmd_io_echo_t		echo;
	fp_mcmd_io_int_byte_t	in_byte;
	fp_mcmd_io_out_byte_t	out_byte;
}mcmd_io_if_t;

typedef struct{
	char did[32];
	uint32_t siid;
	uint32_t piid;
	size_t value_len;
	char *value;
}mcmd_spec_property_t;

typedef enum{
	MCMD_SPEC_STATUS_CLEAR = 0,
	MCMD_SPEC_STATUS_DIRTY
}mcmd_spec_status_t;

typedef enum{
	MCMD_SPEC_STATE_SENT = 0,
	MCMD_SPEC_STATE_SENDING,
	MCMD_SPEC_STATE_WAITING
}mcmd_spec_state_t;

typedef struct{
	list_head_t list;
	mcmd_spec_property_t property;
	mcmd_spec_status_t status;
	mcmd_spec_state_t state;
	uint32_t group_id;
	uint32_t last_sent_ms;
}mcmd_spec_property_entry_t;

typedef struct{
	uint32_t piid;
	size_t value_len;
	char *value;
}mcmd_spec_argument_t;

typedef struct{
	list_head_t list;
	mcmd_spec_argument_t argument;
}mcmd_spec_argument_entry_t;

typedef struct{
	char did[32];
	uint32_t siid;
	uint32_t eiid;
	list_head_t argument_list;
}mcmd_spec_event_t;

typedef struct{
	list_head_t list;
	mcmd_spec_event_t event;
	uint32_t group_id;
	uint32_t timestamp;
	uint32_t occur_ms;
	uint32_t timeout_ms;
	int retry;
	mcmd_spec_state_t state;
}mcmd_spec_event_entry_t;

typedef struct{
	char did[32];
	uint32_t siid;
	uint32_t aiid;
}mcmd_spec_action_t;

typedef struct{
	char did[32];
	list_head_t property_rsp_list;
	mcmd_spec_action_t action;
	list_head_t property_list;
	arch_os_mutex_handle_t prop_mutex;
	list_head_t event_list;
	arch_os_mutex_handle_t event_mutex;
}mcmd_spec_t;

typedef struct _mcmd{
	miio_handle_t miio_handle;
	mcmd_io_if_t io;
	char *command_buf;
	int command_buf_size;
	struct{
		MCMD_DOWN_STATE state;
		uint32_t ts;
		miio_rpc_delegate_context_t method_ctx;
		int (*result)(struct _mcmd *mcmd, char *params);
		int (*error)(struct _mcmd *mcmd, int code, char *message);
		//void *ack_ctx;
		char *buf;
		int buf_size;
	}down;

	miio_ota_mcu_ctx_t ota_mcu_ctx;

	struct{
		MCMD_UP_STATE state;
//		uint32_t ts;
//		int timeout;
		char *buf;
		int buf_size;
	}up;

	mcmd_spec_t spec;

	arch_os_thread_handle_t mcmd_thread;

	volatile int thread_state[2];
}mcmd_t;

typedef void (*fp_cmd_t)(mcmd_t *mcmd, char *params);
typedef int (*fp_cmd_rpc_t)(mcmd_t *mcmd, const char *method, size_t method_len, const char *params, size_t params_len);

#define MIIO_CMD_ADDON_NAME								cmd
#define MIIO_CMD(_name, _cmd, _tip)						\
	miio_addon_entry_complete(MIIO_CMD_ADDON_NAME, _name, _cmd, _tip, 0)

#define MIIO_CMD_RPC_ADDON_NAME							cmdrpc
#define MIIO_CMD_RPC(_name, _cmd, _tip)					\
	miio_addon_entry_complete(MIIO_CMD_RPC_ADDON_NAME, _name, _cmd, _tip, 0)


typedef struct{
	char *cmd;
	size_t cmd_size;
	size_t cmd_len;
}mcmd_composer_t;

int mcmd_escape(const char *in, int in_len, char *out, int out_len);
int mcmd_unescape(const char *in, size_t in_len, char* out, size_t out_len);
#define MCMD_COMPOSER_INIT(str, str_size)		{.cmd = (str), .cmd_size = (str_size), .cmd_len = 0}
int mcmd_compose_start(mcmd_composer_t *composer, const char *cmd);
void mcmd_compose_str(mcmd_composer_t *composer, const char *value, size_t value_len);
void mcmd_compose_str_escape(mcmd_composer_t *composer, const char *value, size_t value_len);
void mcmd_compose_bytes(mcmd_composer_t *composer, const char* value, size_t value_len);
void mcmd_compose_bytes_escape(mcmd_composer_t *composer, const char* value, size_t value_len);
void mcmd_compose_sint32(mcmd_composer_t *composer, int32_t value);
void mcmd_compose_uint32(mcmd_composer_t *composer, uint32_t value);
void mcmd_compose_sint64(mcmd_composer_t *composer, int64_t value);
void mcmd_compose_uint64(mcmd_composer_t *composer, uint64_t value);
int mcmd_compose_finish(mcmd_composer_t *composer, char **cmd, size_t *cmd_len);

void mcmd_spec_init(mcmd_spec_t* mcmd_spec, uint64_t did);
void mcmd_spec_deinit(mcmd_spec_t* mcmd_spec);
void mcmd_spec_flush(miio_handle_t miio_handle, mcmd_spec_t* mcmd_spec);

mcmd_t* mcmd_create(miio_handle_t miio_handle, mcmd_io_if_t *pio, void *io_arg);
void mcmd_destroy(mcmd_t** pmcmd);
int mcmd_parse_name(char *str, char **name, char **params);
int mcmd_parse_params(char *params, char *argv[], int argvs, const char *dividers);
int mcmd_handle(mcmd_t *mcmd, char* cmd, char *params);
int mcmd_rpc_get(mcmd_t *mcmd, int raw_rpc);
int mcmd_rpc_result(mcmd_t *mcmd, char* params);
int mcmd_rpc_error(mcmd_t *mcmd, char* params);
int mcmd_rpc_error_default(mcmd_t *mcmd, int code, char *params);
#endif /* MIIO_COMMAND_H_ */
