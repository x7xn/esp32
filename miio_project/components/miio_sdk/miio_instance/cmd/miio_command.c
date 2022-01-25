
#include "miio_command.h"

#undef MIIO_LOG_TAG
#define MIIO_LOG_TAG			"mcmd"

static arch_os_function_return_t mcmd_main(void *arg)
{
	mcmd_t* mcmd = (mcmd_t*)arg;

	char *cmd = NULL;
	char *params = NULL;

    while(mcmd->thread_state[0] > 0) {

    	mcmd_spec_flush(mcmd->miio_handle, &mcmd->spec);

    	if(0 == mcmd->io.block_in(mcmd->io.handle, mcmd->command_buf, mcmd->command_buf_size, 1000))
    		continue;

        if (MIIO_OK == mcmd_parse_name(mcmd->command_buf, &cmd, &params)) {
        	mcmd_handle(mcmd, cmd, params);
        }
    }


    mcmd->mcmd_thread = NULL;

    arch_os_thread_delete(mcmd->mcmd_thread);

    mcmd->thread_state[1] = -1;

    return ARCH_OS_FUNCTION_RETURN(0);
}


mcmd_t* mcmd_create(miio_handle_t miio_handle, mcmd_io_if_t *pio, void *io_arg)
{
	mcmd_t* mcmd = malloc(sizeof(mcmd_t));
    if(NULL == mcmd)
    	goto err_exit;

    mcmd->miio_handle = miio_handle;
	mcmd->io.create = pio->create;
	mcmd->io.block_in = pio->block_in;
	mcmd->io.block_quit = pio->block_quit;
	mcmd->io.out = pio->out;
	mcmd->io.in_byte = pio->in_byte;
	mcmd->io.out_byte = pio->out_byte;
	mcmd->io.destroy = pio->destroy;
	mcmd->io.echo = pio->echo;
	mcmd->io.handle = mcmd->io.create(io_arg);
	if(NULL == mcmd->io.handle) {
		LOG_ERROR_TAG(MIIO_LOG_TAG, "io create failed");
		goto err_exit;
	}

    mcmd->command_buf_size = MCMD_COMMAND_LEN_MAX;
    mcmd->command_buf = malloc(mcmd->command_buf_size);
    if(NULL == mcmd->command_buf) {
        LOG_ERROR_TAG(MIIO_LOG_TAG, "command recv buf malloc failed");
        goto err_exit;
    }

    mcmd->down.state = MCMD_DOWN_IDLE;
    mcmd->down.buf_size = MCMD_JSON_BUF_SIZE;
    mcmd->down.buf = malloc(mcmd->down.buf_size+1);
    if(NULL == mcmd->down.buf){
    	LOG_ERROR_TAG(MIIO_LOG_TAG, "down buf malloc failed");
		goto err_exit;
    }
    mcmd->down.buf[mcmd->down.buf_size] = '\0';

    mcmd->up.state = MCMD_UP_IDLE;
    mcmd->up.buf_size = MCMD_JSON_BUF_SIZE;
    mcmd->up.buf = malloc(mcmd->up.buf_size+1);
    if(NULL == mcmd->up.buf){
		LOG_ERROR_TAG(MIIO_LOG_TAG, "up buf malloc failed");
		goto err_exit;
	}
    mcmd->up.buf[mcmd->up.buf_size] = '\0';

    //spec init
    mcmd_spec_init(&mcmd->spec, miio_get_did(mcmd->miio_handle));

    //mcu ota
    miio_ota_mcu_init(miio_handle, &(mcmd->ota_mcu_ctx));

    mcmd->thread_state[0] = 1;
    mcmd->thread_state[1] = 1;
    if(MIIO_OK != arch_os_thread_create(&mcmd->mcmd_thread, "mi_mcmd", mcmd_main, 4096, mcmd, ARCH_OS_PRIORITY_DEFAULT)) {
        LOG_ERROR_TAG(MIIO_LOG_TAG, "mcmd_main create failed");
        goto err_exit;
    }

    return mcmd;

err_exit:

	if(mcmd){

		mcmd->thread_state[0] = -1;
		mcmd->thread_state[1] = -1;

		if(mcmd->command_buf)
			free(mcmd->command_buf);

		if(mcmd->io.handle)
			mcmd->io.destroy(&mcmd->io.handle);

		if(mcmd->down.buf){
			free(mcmd->down.buf);
		}

		if(mcmd->up.buf){
			free(mcmd->up.buf);
		}

		free(mcmd);
	}

    return NULL;
}

void mcmd_destroy(mcmd_t** pmcmd)
{
	mcmd_t* mcmd = *pmcmd;

	mcmd->io.block_quit(mcmd->io.handle);

	while(mcmd->thread_state[1] > 0){
		mcmd->thread_state[0] = -1;
		arch_os_ms_sleep(100);
		LOG_WARN_TAG(MIIO_LOG_TAG, "destroy waiting!");
	}

	mcmd->io.destroy(&mcmd->io.handle);

	if(mcmd->command_buf)
		free(mcmd->command_buf);
	if(mcmd->down.buf)
		free(mcmd->down.buf);
	if(mcmd->up.buf)
		free(mcmd->up.buf);

	mcmd_spec_deinit(&mcmd->spec);

	free(mcmd);

	*pmcmd = NULL;
}


/*
 * ' ' = "\u00xx"
 */
int mcmd_escape(const char *in, int in_len, char *out, int out_len)
{
    int encode_len = 0;

    for(int i=0; i < in_len; i++){
		switch(in[i]){
		case ' ':
			if(out && encode_len+5 < out_len){
				out[encode_len] = '\\';
				out[encode_len+1] = 'u';
				out[encode_len+2] = '0';
				out[encode_len+3] = '0';
				out[encode_len+4] = '2';
				out[encode_len+5] = '0';
			}
			encode_len += 6;
			break;
		default:
			if(out && encode_len < out_len){
				out[encode_len] = in[i];
			}
			encode_len += 1;
			break;
		}
	}

    return encode_len;

}
/*
 * "\u00xx" = ' '
 */
int mcmd_unescape(const char *in, size_t in_len, char* out, size_t out_len)
{
	size_t decode_len = 0;

	for(int i=0; i<in_len; i++){
		if(in[i] == '\\' && (i+1) < in_len){
			switch(in[++i]){
			case 'u' :
				if(  ((i+4) < in_len)
				   &&(in[i+1] == '0')
				   &&(in[i+2] == '0')
				   &&(in[i+3] == '2')
				   &&(in[i+4] == '0')){
					if(out && decode_len < out_len){
						out[decode_len] = ' ';
					}
					i += 4;
				}
				else{
					if(out && decode_len < out_len){
						out[decode_len] = in[i-1];
					}
					decode_len++;
					if(out && decode_len < out_len){
						out[decode_len] = in[i];
					}
				}
				break;
			default:
				if(out && decode_len < out_len){
					out[decode_len] = in[i-1];
				}
				decode_len++;
				if(out && decode_len < out_len){
					out[decode_len] = in[i];
				}
				break;
			}
		}
		else{
			if(out && decode_len < out_len){
				out[decode_len] = in[i];
			}
		}
		decode_len++;
	}

	return decode_len;
}

/**
 * @brief  Parse name from command buffer
 *
 * @param  inbuf    Command buffer
 * @param  name     name of received command
 * @param  params  	params of received command
 *
 * @return
 *     - MIIO_OK on success
 *     - MIIO_ERROR on fail
 */
int mcmd_parse_name(char *str, char **name, char **params)
{
	if(NULL == str)
		return MIIO_ERROR;

	int argc = 0;
	char *argv[2] = {NULL, NULL};

	bool in_arg = false;
	for(int i=0; argc < 2; i++){
		if('\0' == str[i]){
			break;
		}
		else if(' ' == str[i]){
			if (in_arg) {
				in_arg = false;
				str[i] = '\0';
			}
		}
		else{
			if(!in_arg){
				in_arg = true;
				argv[argc++] = &str[i];
			}
		}
	}

	if (argc < 1)
		return MIIO_ERROR;

	*name = argv[0];
	*params = argv[1];

	return MIIO_OK;
}

static bool is_divider(char c, const char *dividers, int dividers_len)
{
	for(int i = 0; i < dividers_len; i++){
		if(c == dividers[i]){
			return true;
		}
	}
	return false;
}
/**
 * @brief  Parse params to argv
 *
 * @param  cmd
 * @param  argv     array for argv
 * @param  argvs  	array length
 * @param  dividers  string of divider, length must less than 127
 *
 * @return
 *     - count of argv (>=0)
 */
int mcmd_parse_params(char *params, char *argv[], int argvs, const char *dividers)
{
	if(NULL == params || NULL == argv || 0 == argvs){
		return 0;
	}

	int dividers_len = 0;
	if(NULL == dividers){
		dividers = " ";
		dividers_len = sizeof(" ") - 1;
	}
	else{
		dividers_len = strnlen(dividers, 127);
	}

	int argc = 0;

	bool in_arg = false;
	bool in_str = false;
	bool no_escape = true;
	for(int i=0; argc < argvs; i++){
		if('\0' == params[i]){
			if(in_str){
				break;
			}
			if (in_arg) {
				in_arg = false;
				argc++;
			}
			break;
		}
		else if(is_divider(params[i], dividers, dividers_len)){
			if(in_str){
				continue;
			}
			if (in_arg) {
				in_arg = false;
				argc++;
				params[i] = '\0';
			}
		}
		else{
			if(!in_arg){
				in_arg = true;
				argv[argc] = &params[i];
				if('"' == params[i]){
					//if arg's first c is ", arg is a str
					in_str = true;
					no_escape = true;
				}
			}
			else if(in_str){
				if('"' == params[i]){
					if(no_escape){
						in_str = false;
					}
				}
				else if('\\' == params[i]){
					no_escape = !no_escape;
				}
				else{
					no_escape = true;
				}
			}
		}
	}

	for(int i=0; i < argc; i++){
		//decode
		int arg_len = strlen(argv[i]);
		arg_len = mcmd_unescape(argv[i], arg_len, argv[i], arg_len);
		argv[i][arg_len] = '\0';
	}

	return argc;
}


int mcmd_compose_start(mcmd_composer_t *composer, const char *cmd)
{
	if(NULL == composer->cmd ||
	   0 == composer->cmd_size ||
	   NULL == cmd){
		return MIIO_ERROR_PARAM;
	}

	composer->cmd_len = snprintf(composer->cmd, composer->cmd_size, "%s ", cmd);

	return MIIO_OK;
}

void mcmd_compose_str_escape(mcmd_composer_t *composer, const char *value, size_t value_len)
{
	if(0 == value_len){
		value_len = strlen(value);
	}

	size_t ext_len = sizeof("\"\" ") - 1 + mcmd_escape(value, value_len, NULL, 0);
	if(composer->cmd_len+ext_len >= composer->cmd_size){
		composer->cmd_len += ext_len;
	}
	else{
		composer->cmd[composer->cmd_len++] = '\"';
		composer->cmd_len += mcmd_escape(value, value_len, composer->cmd+composer->cmd_len, composer->cmd_size-composer->cmd_len);
		composer->cmd[composer->cmd_len++] = '\"';
		composer->cmd[composer->cmd_len++] = ' ';
		composer->cmd[composer->cmd_len] = '\0';
	}
}

void mcmd_compose_str(mcmd_composer_t *composer, const char *value, size_t value_len)
{
	if(0 == value_len){
		value_len = strlen(value);
	}

	size_t ext_len = sizeof("\"\" ") - 1 + value_len;
	if(composer->cmd_len+ext_len >= composer->cmd_size){
		composer->cmd_len += ext_len;
	}
	else{
		composer->cmd[composer->cmd_len++] = '\"';
		strncpy(composer->cmd+composer->cmd_len, value, value_len);
		composer->cmd_len += value_len;
		composer->cmd[composer->cmd_len++] = '\"';
		composer->cmd[composer->cmd_len++] = ' ';
		composer->cmd[composer->cmd_len] = '\0';
	}
}

void mcmd_compose_bytes_escape(mcmd_composer_t *composer, const char* value, size_t value_len)
{
	if(0 == value_len){
		return;
	}

	size_t ext_len = sizeof(" ") - 1 + mcmd_escape(value, value_len, NULL, 0);
	if((composer->cmd_len+ext_len) >= composer->cmd_size){
		composer->cmd_len += ext_len;
	}
	else{
		composer->cmd_len += mcmd_escape(value, value_len, composer->cmd+composer->cmd_len, composer->cmd_size-composer->cmd_len);
		composer->cmd[composer->cmd_len++] = ' ';
		composer->cmd[composer->cmd_len] = '\0';
	}
}

void mcmd_compose_bytes(mcmd_composer_t *composer, const char* value, size_t value_len)
{
	if(0 == value_len){
		return;
	}

	size_t ext_len = sizeof(" ") - 1 + value_len;
	if((composer->cmd_len+ext_len) >= composer->cmd_size){
		composer->cmd_len += ext_len;
	}
	else{
		memcpy(composer->cmd+composer->cmd_len, value, value_len);
		composer->cmd_len += value_len;
		composer->cmd[composer->cmd_len++] = ' ';
		composer->cmd[composer->cmd_len] = '\0';
	}
}

void mcmd_compose_sint32(mcmd_composer_t *composer, int32_t value)
{
	if(composer->cmd_len >= composer->cmd_size){
		composer->cmd_len += snprintf(NULL, 0, "%d ", value);
	}
	else{
		composer->cmd_len += snprintf(composer->cmd+composer->cmd_len, composer->cmd_size-composer->cmd_len, "%d ", value);
	}
}

void mcmd_compose_uint32(mcmd_composer_t *composer, uint32_t value)
{
	if(composer->cmd_len >= composer->cmd_size){
		composer->cmd_len += snprintf(NULL, 0, "%u ", value);
	}
	else{
		composer->cmd_len += snprintf(composer->cmd+composer->cmd_len, composer->cmd_size-composer->cmd_len, "%u ", value);
	}
}

void mcmd_compose_sint64(mcmd_composer_t *composer, int64_t value)
{
	char str[32] = {0};
	arch_s64toa(value, str);
	mcmd_compose_bytes(composer, str, strnlen(str, sizeof(str)));
}

void mcmd_compose_uint64(mcmd_composer_t *composer, uint64_t value)
{
	char str[32] = {0};
	arch_u64toa(value, str);
	mcmd_compose_bytes(composer, str, strnlen(str, sizeof(str)));
}


int mcmd_compose_finish(mcmd_composer_t *composer, char **cmd, size_t *cmd_len)
{
	if(composer->cmd_len <= composer->cmd_size)
		composer->cmd_len -= str_cut_tail_c(composer->cmd, composer->cmd_len, ' ');

	if(cmd){
		*cmd = composer->cmd;
	}

	if(cmd_len){
		*cmd_len = composer->cmd_len;
	}

	if(composer->cmd_len >= composer->cmd_size){
		return MIIO_ERROR_NOMEM;
	}

	return MIIO_OK;
}
