/**
* @file    jsmi.h
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
#include "jsmi.h"
#include "miio_arch.h"

#undef MIIO_LOG_TAG
#define MIIO_LOG_TAG			"jsmi"

size_t jsmi_escape(const char* in, size_t in_len, char *out, size_t out_len)
{
	size_t out_len_need = 0;

	for(size_t i=0; i<in_len; i++){
		switch(in[i]){
		case '\"': case '\\': case '/' :
			if(out && (out_len_need+1) < out_len){
				out[out_len_need] = '\\';
				out[out_len_need+1] = in[i];
			}
			out_len_need += 2;
			break;
		case '\b':
			if(out && (out_len_need+1) < out_len){
				out[out_len_need] = '\\';
				out[out_len_need+1] = 'b';
			}
			out_len_need += 2;
			break;
		case '\f':
			if(out && (out_len_need+1) < out_len){
				out[out_len_need] = '\\';
				out[out_len_need+1] = 'f';
			}
			out_len_need += 2;
			break;
		case '\r':
			if(out && (out_len_need+1) < out_len){
				out[out_len_need] = '\\';
				out[out_len_need+1] = 'r';
			}
			out_len_need += 2;
			break;
		case '\n':
			if(out && (out_len_need+1) < out_len){
				out[out_len_need] = '\\';
				out[out_len_need+1] = 'n';
			}
			out_len_need += 2;
			break;
		case '\t':
			if(out && (out_len_need+1) < out_len){
				out[out_len_need] = '\\';
				out[out_len_need+1] = 't';
			}
			out_len_need += 2;
			break;
		default:
			if(out && (out_len_need) < out_len){
				out[out_len_need] = in[i];
			}
			out_len_need += 1;
			break;
		}
	}

	return out_len_need;
}

static size_t jsmi_escape_unprintable(const char* in, size_t in_len, char *out, size_t out_len)
{
	size_t out_len_need = 0;

	for(size_t i=0; i<in_len; i++){
		switch(in[i]){
		case '\b':
			if(out && (out_len_need+1) < out_len){
				out[out_len_need] = '\\';
				out[out_len_need+1] = 'b';
			}
			out_len_need += 2;
			break;
		case '\f':
			if(out && (out_len_need+1) < out_len){
				out[out_len_need] = '\\';
				out[out_len_need+1] = 'f';
			}
			out_len_need += 2;
			break;
		case '\r':
			if(out && (out_len_need+1) < out_len){
				out[out_len_need] = '\\';
				out[out_len_need+1] = 'r';
			}
			out_len_need += 2;
			break;
		case '\n':
			if(out && (out_len_need+1) < out_len){
				out[out_len_need] = '\\';
				out[out_len_need+1] = 'n';
			}
			out_len_need += 2;
			break;
		case '\t':
			if(out && (out_len_need+1) < out_len){
				out[out_len_need] = '\\';
				out[out_len_need+1] = 't';
			}
			out_len_need += 2;
			break;
		default:
			if(out && (out_len_need) < out_len){
				out[out_len_need] = in[i];
			}
			out_len_need += 1;
			break;
		}
	}

	return out_len_need;
}

static size_t jsmi_unescape(const char *in, size_t in_len, char* out, size_t out_len)
{
	size_t out_len_need = 0;

	for(size_t i=0; i<in_len; i++){
		if(in[i] == '\\' && (i+1) < in_len){
			switch(in[++i]){
			case '\"': case '\\' : case '/' :
				if(out && out_len_need < out_len){
					out[out_len_need] = in[i];
				}
				break;
			case 'b' :
				if(out && out_len_need < out_len){
					out[out_len_need] = '\b';
				}
				break;
			case 'f' :
				if(out && out_len_need < out_len){
					out[out_len_need] = '\f';
				}
				break;
			case 'r' :
				if(out && out_len_need < out_len){
					out[out_len_need] = '\r';
				}
				break;
			case 'n'  :
				if(out && out_len_need < out_len){
					out[out_len_need] = '\n';
				}
				break;
			case 't' :
				if(out && out_len_need < out_len){
					out[out_len_need] = '\t';
				}
				break;
			default:
				if(out && out_len_need < out_len){
					out[out_len_need] = in[i-1];
				}
				out_len_need++;
				if(out && out_len_need < out_len){
					out[out_len_need] = in[i];
				}
				break;
			}
		}
		else{
			if(out && out_len_need < out_len){
				out[out_len_need] = in[i];
			}
		}
		out_len_need++;
	}

	return out_len_need;
}

#define TOKEN_VALID(pt) (pt != NULL \
                		&& ((pt)->type == JSMN_PRIMITIVE \
                        || (pt)->type == JSMN_OBJECT \
                        || (pt)->type == JSMN_ARRAY \
                        || (pt)->type == JSMN_STRING))

/**
 *	@brief 		Jump to next same level token.
 *	@details
 *	@return		Next token
 */
//用于跳到下一个同层次的节点(从key跳到下一个key 或者 array单元跳到下一个)
static jsmntok_t* jsmn_next(jsmntok_t* t)
{
	int range = 1;
	while(range > 0 && TOKEN_VALID(t)){
		range += t->size;
		t++; range--;
	}
	if(TOKEN_VALID(t))
		return t;
	else
		return NULL;
}


/**
 *	@brief 		Get token val from father token via 'key' string.
 *	@details	Token A point to "{"abc":123}", call jsmn_api.key_value(js, A, "abc") will return token points to '123'.
 *	@return		Value token
 */
//输入原始串 、解析数组 和 需要定位的key
//仅仅扫描当前层面的名值对
static jsmntok_t* jsmn_key_value(const char* js, jsmntok_t* tokens, const char* key)
{
	jsmntok_t* t = tokens;
	int pairs = t->size;

	//若不在object中 或者空obj则报错
	if(t->type != JSMN_OBJECT || pairs == 0)
		return NULL;

	t += 1;
	while(TOKEN_VALID(t) && pairs > 0){
		if((strlen(key) == (t->end - t->start)) && (0 == strncmp(key, js + t->start, t->end - t->start)))
			return t+1;
		t = jsmn_next(t+1);

		pairs --;
	}
	return NULL;
}

/**
 *	@brief 		Get token val from father array token via index.
 *	@details	Token A point to "[1,2,3]", call jsmn_api.array_value(js, A, 2) will return token points to '3'.
 *	@return		Value token
 */
//输入array开头解析数组 和 idx
static jsmntok_t* jsmn_array_value(const char* js, jsmntok_t* tokens, uint32_t idx)
{
	jsmntok_t* t = tokens;
	uint32_t num = t->size;

	//若不是array 或者空 则返回null
	if(t->type != JSMN_ARRAY || num == 0 || idx >= num)
		return NULL;

	t += 1;	//跳过外框

    while(idx > 0 && t != NULL) {
        t = jsmn_next(t);
        idx--;
    }

	return t;
}

/**
 *	@brief 		Get unsigned long integer val(64bits) from jsmn token object
 *	@details
 *	@return		MIIO_OK : Successful.
 *				other : if an error occurred.
 */
static int jsmn_tkn2val_u64(const char *js ,jsmntok_t * tk, uint64_t *val)
{
	if(NULL == tk || tk->type != JSMN_PRIMITIVE || !isdigit((int)(js[tk->start]))) {
        return MIIO_ERROR_NOTFOUND;
    }

	if(val)
		*val = arch_atou64n( (const char *)&(js[tk->start]), tk->end - tk->start);

    return MIIO_OK;
}

/**
 *	@brief 		Get signed long integer val(64bits) from jsmn token object
 *	@details
 *	@return		MIIO_OK : Successful.
 *				other : if an error occurred.
 */
static int jsmn_tkn2val_s64(const char *js ,jsmntok_t * tk, int64_t *val)
{
    if(NULL == tk || tk->type != JSMN_PRIMITIVE || (!isdigit((int)(js[tk->start])) && js[tk->start] != '-'))
        return MIIO_ERROR_NOTFOUND;

    if(val)
        *val = arch_atos64n( (const char *)&(js[tk->start]), tk->end - tk->start);

    return MIIO_OK;
}

/**
 *	@brief 		Get unsigned integer val(32bits) from jsmn token object
 *	@details
 *	@return		MIIO_OK : Successful.
 *				other : if an error occurred.
 */
static int jsmn_tkn2val_u32(const char* js, jsmntok_t* tk, uint32_t* val)
{
	if(NULL == tk || tk->type != JSMN_PRIMITIVE || !isdigit((int)(js[tk->start]))){
		return MIIO_ERROR_NOTFOUND;
	}
	*val = arch_atoun( (const char *)&(js[tk->start]), tk->end - tk->start);
	return MIIO_OK;
}

/**
 *	@brief 		Get signed integer val(32bits) from jsmn token object
 *	@details
 *	@return		MIIO_OK : Successful.
 *				other : if an error occurred.
 */
static int jsmn_tkn2val_s32(const char* js, jsmntok_t* tk, int32_t* val)
{
    if(NULL == tk || tk->type != JSMN_PRIMITIVE || (!isdigit((int)(js[tk->start])) && js[tk->start] != '-'))
        return MIIO_ERROR_NOTFOUND;
    *val = arch_atoin( (const char *)&(js[tk->start]), tk->end - tk->start);
    return MIIO_OK;
}

/**
 *	@brief 		Get signed short integer val(16bits) from jsmn token object
 *	@details
 *	@return		MIIO_OK : Successful.
 *				other : if an error occurred.
 */
static int jsmn_tkn2val_u16(const char* js, jsmntok_t* tk, uint16_t* val)
{
	int ret;
	uint32_t tmp;
	ret = jsmn_tkn2val_u32(js, tk, &tmp);
	if(MIIO_OK == ret)*val = tmp;
	return ret;
}

/**
 *	@brief 		Get signed short integer val(16bits) from jsmn token object
 *	@details
 *	@return		MIIO_OK : Successful.
 *				other : if an error occurred.
 */

static int jsmn_tkn2val_s16(const char* js, jsmntok_t* tk, int16_t* val)
{
    int ret;
    int32_t tmp;
    ret = jsmn_tkn2val_s32(js, tk, &tmp);
    if(MIIO_OK == ret)*val = tmp;
    return ret;
}

/**
 *	@brief 		Get unsigned char val(8bits) from jsmn token object
 *	@details
 *	@return		MIIO_OK : Successful.
 *				other : if an error occurred.
 */
static int jsmn_tkn2val_u8(const char* js, jsmntok_t* tk, uint8_t* val)
{
	int ret;
	uint32_t tmp;
	ret = jsmn_tkn2val_u32(js, tk, &tmp);
	if(MIIO_OK == ret)*val = tmp;
	return ret;
}

/**
 *	@brief 		Get signed char val(8bits) from jsmn token object
 *	@details
 *	@return		MIIO_OK : Successful.
 *				other : if an error occurred.
 */
static int jsmn_tkn2val_s8(const char* js, jsmntok_t* tk, int8_t* val)
{
    int ret;
    int32_t tmp;
    ret = jsmn_tkn2val_s32(js, tk, &tmp);
    if(MIIO_OK == ret)*val = tmp;
    return ret;
}

/**
 *	@brief 		Get string from jsmn token object, most copy (str_size-1) bytes
 *	@details
 *	@return		MIIO_OK : Successful.
 *				other : if an error occurred.
 */
static int jsmn_tkn2val_str(const char* js, jsmntok_t* tk, char* str, size_t str_size)
{
	if(NULL == tk || tk->type != JSMN_STRING)return MIIO_ERROR_NOTFOUND;
	if(str_size < 1)return MIIO_ERROR_NOMEM;
	size_t str_len = jsmi_unescape(&(js[tk->start]), tk->end - tk->start, str, str_size-1);
	if(str_len >= str_size)return MIIO_ERROR_NOMEM;
	str[str_len] = '\0';
	return MIIO_OK;
}


/**
 *	@brief 		Get bool from jsmn token object
 *	@details
 *	@return		MIIO_OK : Successful.
 *				other : if an error occurred.
 */
static int jsmn_tkn2val_bool(const char* js, jsmntok_t* tk, bool* val)
{
	if(NULL == tk || tk->type != JSMN_PRIMITIVE)return MIIO_ERROR_NOTFOUND;
	if(js[tk->start] == 't')*val = true;
	else if(js[tk->start] == 'f')*val = false;
	else return MIIO_ERROR_NOTFOUND;
	return MIIO_OK;
}

/**
 *	@brief 		Get double from jsmn token object
 *	@details
 *	@return		MIIO_OK : Successful.
 *				other : if an error occurred.
 */
static int jsmn_tkn2val_double(const char* js, jsmntok_t* tk, double* val)
{
	if(NULL == tk || tk->type != JSMN_PRIMITIVE || (!isdigit((int)(js[tk->start])) && js[tk->start] != '-'))
		return MIIO_ERROR_NOTFOUND;
	*val = arch_atofn( (const char *)&(js[tk->start]), tk->end - tk->start);
	return MIIO_OK;
}

int jsmi_parse_start(jsmi_parser_t *parser)
{
	int tok_array_len = JSMI_PARSER_TOK_NUM_MIN;
	parser->tok_array = malloc(sizeof(jsmntok_t)*(tok_array_len+1));
	if(NULL == parser->tok_array){
		LOG_ERROR_TAG(MIIO_LOG_TAG, "tok_array alloc err.");
		return MIIO_ERROR_NOMEM;
	}

	jsmn_init(&parser->jsmn_psr);

	while((parser->tok_array_len = jsmn_parse(&parser->jsmn_psr, parser->js, parser->js_len, parser->tok_array, tok_array_len)) == JSMN_ERROR_NOMEM){
		LOG_WARN_TAG(MIIO_LOG_TAG, "length of tok_array is %d, but too small...", tok_array_len );
		tok_array_len += JSMI_PARSER_TOK_NUM_STEP;
		if(tok_array_len > JSMI_PARSER_TOK_NUM_MAX){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "js is too large, no space to parse...");
			free(parser->tok_array);
			parser->tok_array = NULL;
			return MIIO_ERROR_NOMEM;
		}
		parser->tok_array = realloc(parser->tok_array, sizeof(jsmntok_t)*(tok_array_len+1));
		if(NULL == parser->tok_array){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "tok_array alloc err.");
			return MIIO_ERROR_NOMEM;
		}
	}

	if(parser->tok_array_len < 0){
		LOG_ERROR_TAG(MIIO_LOG_TAG, "jsmn parse err.");
		free(parser->tok_array);
		parser->tok_array = NULL;
		return MIIO_ERROR_PARAM;
	}

	parser->tok_array[parser->tok_array_len].type = JSMN_UNDEFINED;	//标识尾部

	return MIIO_OK;
}

int jsmi_parse_finish(jsmi_parser_t *parser)
{
	if(parser){
		if(parser->tok_array){
			free(parser->tok_array);
			parser->tok_array = NULL;
			return MIIO_OK;
		}
		else{
			return MIIO_ERROR_NOMEM;
		}
	}

	return MIIO_ERROR;
}


jsmntok_t* jsmi_get_tok(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth)
{
	jsmntok_t *child_tok;
	if( NULL == parent_tok){
		parent_tok = parser->tok_array;
	}
	child_tok = parent_tok;

	for(size_t i=0; i<tok_path_depth;i++){
		if(JSMN_ARRAY == parent_tok->type){
			child_tok = jsmn_array_value(parser->js, parent_tok, (size_t)(tok_path[i].key));
		}
		else{
			child_tok = jsmn_key_value(parser->js, parent_tok, (const char*)(tok_path[i].key));
		}

		if(NULL == child_tok){
			break;
		}

		if(JSMN_UNDEFINED != tok_path[i].type){
		   if(child_tok->type != tok_path[i].type){
			   child_tok = NULL;
			   break;
		   }
		}

		parent_tok = child_tok;
	}
	return child_tok;
}


int jsmi_get_value_uint32(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, uint32_t *value)
{
	jsmntok_t *tok = jsmi_get_tok(parser, parent_tok, tok_path, tok_path_depth);

	if(NULL == tok){
		return MIIO_ERROR_NOTFOUND;
	}

	return jsmn_tkn2val_u32(parser->js, tok, value);
}

int jsmi_get_value_sint32(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, int32_t *value)
{
	jsmntok_t *tok = jsmi_get_tok(parser, parent_tok, tok_path, tok_path_depth);

	if(NULL == tok){
		return MIIO_ERROR_NOTFOUND;
	}

	return jsmn_tkn2val_s32(parser->js, tok, value);
}


int jsmi_get_value_uint(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, unsigned int *value)
{
	uint32_t value32 = 0;

	int ret = jsmi_get_value_uint32(parser, parent_tok, tok_path, tok_path_depth, &value32);

	if(MIIO_OK == ret)
		*value = value32;

	return ret;
}

int jsmi_get_value_sint(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, int *value)
{
	int32_t value32 = 0;

	int ret = jsmi_get_value_sint32(parser, parent_tok, tok_path, tok_path_depth, &value32);

	if(MIIO_OK == ret)
		*value = value32;

	return ret;
}

int jsmi_get_value_uint64(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, uint64_t *value)
{
	jsmntok_t *tok = jsmi_get_tok(parser, parent_tok, tok_path, tok_path_depth);

	if(NULL == tok){
		return MIIO_ERROR_NOTFOUND;
	}

	return jsmn_tkn2val_u64(parser->js, tok, value);
}


int jsmi_get_value_sint64(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, int64_t *value)
{
	jsmntok_t *tok = jsmi_get_tok(parser, parent_tok, tok_path, tok_path_depth);

	if(NULL == tok){
		return MIIO_ERROR_NOTFOUND;
	}

	return jsmn_tkn2val_s64(parser->js, tok, value);
}

int jsmi_get_value_uint16(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, uint16_t *value)
{
	jsmntok_t *tok = jsmi_get_tok(parser, parent_tok, tok_path, tok_path_depth);

	if(NULL == tok){
		return MIIO_ERROR_NOTFOUND;
	}

	return jsmn_tkn2val_u16(parser->js, tok, value);
}

int jsmi_get_value_sint16(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, int16_t *value)
{
	jsmntok_t *tok = jsmi_get_tok(parser, parent_tok, tok_path, tok_path_depth);

	if(NULL == tok){
		return MIIO_ERROR_NOTFOUND;
	}

	return jsmn_tkn2val_s16(parser->js, tok, value);
}

int jsmi_get_value_uint8(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, uint8_t *value)
{
	jsmntok_t *tok = jsmi_get_tok(parser, parent_tok, tok_path, tok_path_depth);

	if(NULL == tok){
		return MIIO_ERROR_NOTFOUND;
	}

	return jsmn_tkn2val_u8(parser->js, tok, value);
}

int jsmi_get_value_sint8(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, int8_t *value)
{
	jsmntok_t *tok = jsmi_get_tok(parser, parent_tok, tok_path, tok_path_depth);

	if(NULL == tok){
		return MIIO_ERROR_NOTFOUND;
	}

	return jsmn_tkn2val_s8(parser->js, tok, value);
}

int jsmi_get_value_str(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, char *str, size_t str_size)
{
	jsmntok_t *tok = jsmi_get_tok(parser, parent_tok, tok_path, tok_path_depth);

	if(NULL == tok){
		return MIIO_ERROR_NOTFOUND;
	}

	return jsmn_tkn2val_str(parser->js, tok, str, str_size);
}


int jsmi_get_value_bool(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, bool *value)
{
	jsmntok_t *tok = jsmi_get_tok(parser, parent_tok, tok_path, tok_path_depth);

	if(NULL == tok){
		return MIIO_ERROR_NOTFOUND;
	}

	return jsmn_tkn2val_bool(parser->js, tok, value);
}

int jsmi_get_value_double(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, double *value)
{
	jsmntok_t *tok = jsmi_get_tok(parser, parent_tok, tok_path, tok_path_depth);

	if(NULL == tok){
		return MIIO_ERROR_NOTFOUND;
	}

	return jsmn_tkn2val_double(parser->js, tok, value);
}

static int verify_as_number(const char * str, size_t len)
{
    const char * str_head = str;
    const char * str_tail = str + len - 1;
    const char * p = str_head + 1;

    if(str_tail < str_head)
        return MIIO_ERROR_PARAM;

    if(!((*str_head >= '0' && *str_head <= '9') || *str_head == '-'))
        return MIIO_ERROR_PARAM;

    while(p <= str_tail) {
        if((*p < '0' || *p > '9') && *p != '.')
            return MIIO_ERROR_PARAM;
        p++;
    }

    return MIIO_OK;
}

static int verify_as_string(const char * str, size_t len)
{
    const char * str_head = str;
    const char * str_tail = str + len - 1;
    const char * p = str_head + 1;

    if(str_tail <= str_head)
        return MIIO_ERROR_PARAM;

    if(*str_head != '"' || *str_tail != '"' || (*str_tail == '"' && *(str_tail-1) == '\\'))
        return MIIO_ERROR_PARAM;

    while(p < str_tail) {
        if(*p < 32 || *p > 126 || (*p == '"' && *(p-1) != '\\'))
            return MIIO_ERROR_PARAM;
        p++;
    }

    return MIIO_OK;
}

static int verify_as_bool(const char * str, size_t len)
{
    const char * str_head = str;
    const char * str_tail = str + len - 1;

    if(str_tail < str_head)
        return MIIO_ERROR_PARAM;

    if(str_tail - str_head == 3 && strncmp(str_head, "true", 4) == 0)
        return MIIO_OK;

    if(str_tail - str_head == 4 && strncmp(str_head, "false", 5) == 0)
        return MIIO_OK;

    return MIIO_ERROR_PARAM;
}

static inline int verify_as_null(const char * str, size_t len)
{
    if(len == 0 || (len == 4 && strncmp(str, "null", 4) == 0))
        return MIIO_OK;
    else
        return MIIO_ERROR_PARAM;
}

int jsmi_verify_key(const char * key)
{
    if(!key || *key == '\0')
        return MIIO_ERROR_PARAM;
    while(*key != '\0') {
        if(!((*key >= '0' && *key <= '9')||(*key >= 'A' && *key <= 'Z')||(*key >= 'a' && *key <= 'z')||*key == '_'))
            return MIIO_ERROR_PARAM;
        key++;
    }
    return MIIO_OK;
}

int jsmi_verify_value(const char * value)
{
    if(!value)
        return MIIO_ERROR_PARAM;
    if(   verify_as_string(value, strlen(value)) != MIIO_OK
       && verify_as_number(value, strlen(value)) != MIIO_OK
	   && verify_as_bool(value, strlen(value)) != MIIO_OK
	   && verify_as_null(value, strlen(value)) != MIIO_OK)
        return MIIO_ERROR_PARAM;

    return MIIO_OK;
}

int jsmi_compose_start(jsmi_composer_t *composer)
{
	composer->js_len = 0;

	if( NULL == composer->js ||
		0 == composer->js_size )
		return MIIO_ERROR_PARAM;

	return MIIO_OK;
}

int jsmi_compose_finish(jsmi_composer_t *composer, char **js, size_t *js_len)
{
	if(js){
		*js = composer->js;
	}
	if(js_len){
		*js_len = composer->js_len;
	}

	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}

	composer->js[composer->js_len] = '\0';

	return MIIO_OK;
}

int jsmi_set_object_begin(jsmi_composer_t *composer)
{
	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, "{");
	}
	else{
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "{");
	}

	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}

int jsmi_set_object_end(jsmi_composer_t *composer)
{
	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, "}");
	}
	else{
		composer->js_len -= str_cut_tail_c(composer->js, composer->js_len, ',');
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "}");
	}

	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}

int jsmi_set_array_begin(jsmi_composer_t *composer)
{
	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, "[");
	}
	else{
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "[");
	}

	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}

	return MIIO_OK;
}

int jsmi_set_array_end(jsmi_composer_t *composer)
{
	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, "]");
	}
	else{
		composer->js_len -= str_cut_tail_c(composer->js, composer->js_len, ',');
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "]");
	}

	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}

int jsmi_set_key(jsmi_composer_t *composer, const char *key)
{
	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, "\"%s\":", key);
	}
	else{
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "\"%s\":", key);
	}
	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}

int jsmi_set_value_object_begin(jsmi_composer_t *composer)
{
	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, "{");
	}
	else{
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "{");
	}
	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}

int jsmi_set_value_object_end(jsmi_composer_t *composer)
{
	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, "},");
	}
	else{
		composer->js_len -= str_cut_tail_c(composer->js, composer->js_len, ',');
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "},");
	}
	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}

int jsmi_set_value_sint(jsmi_composer_t *composer, int value)
{
	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, "%d,", value);
	}
	else{
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "%d,", value);
	}
	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}

int jsmi_set_value_uint(jsmi_composer_t *composer, unsigned int value)
{
	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, "%u,", value);
	}
	else{
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "%u,", value);
	}
	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}

int jsmi_set_value_sint32(jsmi_composer_t *composer, int32_t value)
{
	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, "%d,", value);
	}
	else{
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "%d,", value);
	}
	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}

int jsmi_set_value_uint32(jsmi_composer_t *composer, uint32_t value)
{
	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, "%u,", value);
	}
	else{
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "%u,", value);
	}
	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}

int jsmi_set_value_str(jsmi_composer_t *composer, const char* value, size_t value_len)
{
	if(0 == value_len){
		value_len = strlen(value);
	}
	size_t ext_len = sizeof("\"\",") - 1 + jsmi_escape(value, value_len, NULL, 0);
	if((composer->js_len+ext_len) >= composer->js_size){
		composer->js_len += ext_len;
	}
	else{
		composer->js[composer->js_len++] = '\"';
		composer->js_len += jsmi_escape(value, value_len, composer->js+composer->js_len, composer->js_size-composer->js_len);
		composer->js[composer->js_len++] = '\"';
		composer->js[composer->js_len++] = ',';
		composer->js[composer->js_len] = '\0';
	}
	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}


int jsmi_set_value_bytes(jsmi_composer_t *composer, const char* value, size_t value_len)
{
	if(0 == value_len){
		value_len = strlen(value);
	}

	size_t ext_len = sizeof(",") - 1 + jsmi_escape_unprintable(value, value_len, NULL, 0);
	if((composer->js_len+ext_len) >= composer->js_size){
		composer->js_len += ext_len;
	}
	else{
		composer->js_len += jsmi_escape_unprintable(value, value_len, composer->js+composer->js_len, composer->js_size-composer->js_len);
		composer->js[composer->js_len++] = ',';
		composer->js[composer->js_len] = '\0';
	}
	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}

int jsmi_set_value_bool(jsmi_composer_t *composer, bool value)
{
	if(composer->js_len >= composer->js_size){
		if(value == true){
			composer->js_len += snprintf(NULL, 0, "true,");
		}
		else{
			composer->js_len += snprintf(NULL, 0, "false,");
		}
	}
	else{
		if(value == true){
			composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "true,");
		}
		else{
			composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "false,");
		}
	}
	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}

int jsmi_set_value_sint64(jsmi_composer_t *composer, int64_t value)
{
	char str[32] = {0};
	arch_s64toa(value, str);
	return jsmi_set_value_bytes(composer, str, strnlen(str, sizeof(str)));
}

int jsmi_set_value_uint64(jsmi_composer_t *composer, uint64_t value)
{
	char str[32] = {0};
	arch_u64toa(value, str);
	return jsmi_set_value_bytes(composer, str, strnlen(str, sizeof(str)));
}

int jsmi_set_value_double(jsmi_composer_t *composer, double value, uint8_t precision)
{
	char format_str[16] = {0};
	if(0 == precision)precision = 4;
	snprintf(format_str, sizeof(format_str), "%%.%df,", precision);

	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, format_str, value);
	}
	else{
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, format_str, value);
	}
	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}

int jsmi_set_key_value_sint32(jsmi_composer_t *composer, const char *key, int32_t value)
{
	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, "\"%s\":%d,", key, value);
	}
	else{
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "\"%s\":%d,", key, value);
	}
	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}

int jsmi_set_key_value_uint32(jsmi_composer_t *composer, const char *key, uint32_t value)
{
	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, "\"%s\":%u,", key, value);
	}
	else{
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "\"%s\":%u,", key, value);
	}
	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}

int jsmi_set_key_value_sint(jsmi_composer_t *composer, const char *key, int value)
{
	return jsmi_set_key_value_sint32(composer, key, value);
}

int jsmi_set_key_value_uint(jsmi_composer_t *composer, const char *key, unsigned int value)
{
	return jsmi_set_key_value_uint32(composer, key, value);
}

int jsmi_set_key_value_bytes(jsmi_composer_t *composer, const char *key, const char* value, size_t value_len)
{
	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, "\"%s\":", key);
	}
	else{
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "\"%s\":", key);
	}

	return jsmi_set_value_bytes(composer, value, value_len);
}

int jsmi_set_key_value_str(jsmi_composer_t *composer, const char *key, const char* value, size_t value_len)
{
	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, "\"%s\":", key);
	}
	else{
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "\"%s\":", key);
	}

	return jsmi_set_value_str(composer, value, value_len);
}

int jsmi_set_key_value_bool(jsmi_composer_t *composer, const char *key, bool value)
{
	if(composer->js_len >= composer->js_size){
		if(value == true){
			composer->js_len += snprintf(NULL, 0, "\"%s\":true,", key);
		}
		else{
			composer->js_len += snprintf(NULL, 0, "\"%s\":false,", key);
		}
	}
	else{
		if(value == true){
			composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "\"%s\":true,", key);
		}
		else{
			composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "\"%s\":false,", key);
		}
	}

	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}

int jsmi_set_key_value_sint64(jsmi_composer_t *composer, const char *key, int64_t value)
{
	char str[32] = {0};
	arch_s64toa(value, str);
	return jsmi_set_key_value_bytes(composer, key, str, strnlen(str, sizeof(str)));
}

int jsmi_set_key_value_uint64(jsmi_composer_t *composer, const char *key, uint64_t value)
{
	char str[32] = {0};
	arch_u64toa(value, str);
	return jsmi_set_key_value_bytes(composer, key, str, strnlen(str, sizeof(str)));
}

int jsmi_set_key_value_double(jsmi_composer_t *composer, const char *key, double value, uint8_t precision)
{
	char format_str[16] = {0};
	if(0 == precision)precision = 4;
	snprintf(format_str, sizeof(format_str), "\"%%s\":%%.%df,", precision);

	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, format_str, key, value);
	}
	else{
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, format_str, key, value);
	}
	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}

int jsmi_set_key_object_begin(jsmi_composer_t *composer, const char *key)
{
	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, "\"%s\":{", key);
	}
	else{
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "\"%s\":{", key);
	}
	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}

int jsmi_set_key_object_end(jsmi_composer_t *composer)
{
	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, "},");
	}
	else{
		composer->js_len -= str_cut_tail_c(composer->js, composer->js_len, ',');
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "},");
	}
	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}

int jsmi_set_key_array_begin(jsmi_composer_t *composer, const char *key)
{
	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, "\"%s\":[", key);
	}
	else{
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "\"%s\":[", key);
	}
	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}

int jsmi_set_key_array_end(jsmi_composer_t *composer)
{
	if(composer->js_len >= composer->js_size){
		composer->js_len += snprintf(NULL, 0, "],");
	}
	else{
		composer->js_len -= str_cut_tail_c(composer->js, composer->js_len, ',');
		composer->js_len += snprintf(composer->js+composer->js_len, composer->js_size-composer->js_len, "],");
	}
	if(composer->js_len >= composer->js_size){
		return MIIO_ERROR_NOMEM;
	}
	return MIIO_OK;
}
