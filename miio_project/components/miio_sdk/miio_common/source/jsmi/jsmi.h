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

#ifndef __JSMI_H__
#define __JSMI_H__

#include "jsmn.h"
#include "miio_define.h"

#define JSMI_PARSER_TOK_NUM_MIN 			100
#define JSMI_PARSER_TOK_NUM_STEP			50
#define JSMI_PARSER_TOK_NUM_MAX 			400

typedef struct{
	void *key;
	jsmntype_t type;
}jsmi_tok_path_t;

typedef struct{
	const char *js;
	size_t js_len;
	jsmn_parser jsmn_psr;
	jsmntok_t *tok_array;
	int tok_array_len;
}jsmi_parser_t;

typedef struct {
	char *js;
	size_t js_size;
	size_t js_len;
}jsmi_composer_t;

#define JSMI_PARSER_INIT(str, str_size)		{.js = (str), .js_len = (str_size), .tok_array = NULL, .tok_array_len = 0}
#define JSMI_COMPOSER_INIT(str, str_size)	{.js = (str), .js_size = (str_size), .js_len = 0}

int jsmi_parse_start(jsmi_parser_t *parser);
int jsmi_parse_finish(jsmi_parser_t *parser);
jsmntok_t* jsmi_get_tok(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth);
int jsmi_get_value_uint(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, unsigned int *value);
int jsmi_get_value_sint(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, int *value);
int jsmi_get_value_str(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, char *str, size_t str_size);
int jsmi_get_value_bool(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, bool *value);
int jsmi_get_value_double(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, double *value);
int jsmi_get_value_uint64(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, uint64_t *value);
int jsmi_get_value_sint64(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, int64_t *value);
#define jsmi_get_value_u64	jsmi_get_value_uint64
#define jsmi_get_value_s64	jsmi_get_value_sint64
int jsmi_get_value_uint32(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, uint32_t *value);
int jsmi_get_value_sint32(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, int32_t *value);
#define jsmi_get_value_u32	jsmi_get_value_uint32
#define jsmi_get_value_s32	jsmi_get_value_sint32
int jsmi_get_value_uint16(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, uint16_t *value);
int jsmi_get_value_sint16(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, int16_t *value);
#define jsmi_get_value_u16	jsmi_get_value_uint16
#define jsmi_get_value_s16	jsmi_get_value_sint16
int jsmi_get_value_uint8(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, uint8_t *value);
int jsmi_get_value_sint8(jsmi_parser_t *parser, jsmntok_t *parent_tok, jsmi_tok_path_t *tok_path, size_t tok_path_depth, int8_t *value);
#define jsmi_get_value_u8	jsmi_get_value_uint8
#define jsmi_get_value_s8	jsmi_get_value_sint8

size_t jsmi_escape(const char* in, size_t in_len, char *out, size_t out_len);
int jsmi_verify_key(const char * key);
int jsmi_verify_value(const char * value);
int jsmi_compose_start(jsmi_composer_t *composer);
int jsmi_compose_finish(jsmi_composer_t *composer, char **js, size_t *js_len);
int jsmi_set_object_begin(jsmi_composer_t *composer);
int jsmi_set_object_end(jsmi_composer_t *composer);
int jsmi_set_array_begin(jsmi_composer_t *composer);
int jsmi_set_array_end(jsmi_composer_t *composer);
int jsmi_set_key(jsmi_composer_t *composer, const char *key);
int jsmi_set_value_sint(jsmi_composer_t *composer, int value);
int jsmi_set_value_uint(jsmi_composer_t *composer, unsigned int value);
int jsmi_set_value_sint32(jsmi_composer_t *composer, int32_t value);
int jsmi_set_value_uint32(jsmi_composer_t *composer, uint32_t value);
#define jsmi_set_value_s32	jsmi_set_value_sint32
#define jsmi_set_value_u32	jsmi_set_value_uint32
int jsmi_set_value_bytes(jsmi_composer_t *composer, const char* value, size_t value_len);
int jsmi_set_value_str(jsmi_composer_t *composer, const char* value, size_t value_len);
int jsmi_set_value_bool(jsmi_composer_t *composer, bool value);
int jsmi_set_value_sint64(jsmi_composer_t *composer, int64_t value);
int jsmi_set_value_uint64(jsmi_composer_t *composer, uint64_t value);
#define jsmi_set_value_s64	jsmi_set_value_sint64
#define jsmi_set_value_u64	jsmi_set_value_uint64
int jsmi_set_value_double(jsmi_composer_t *composer, double value, uint8_t precision);
int jsmi_set_value_object_begin(jsmi_composer_t *composer);
int jsmi_set_value_object_end(jsmi_composer_t *composer);
int jsmi_set_key_value_sint(jsmi_composer_t *composer, const char *key, int value);
int jsmi_set_key_value_uint(jsmi_composer_t *composer, const char *key, unsigned int value);
int jsmi_set_key_value_sint32(jsmi_composer_t *composer, const char *key, int32_t value);
int jsmi_set_key_value_uint32(jsmi_composer_t *composer, const char *key, uint32_t value);
#define jsmi_set_key_value_s32	jsmi_set_key_value_sint32
#define jsmi_set_key_value_u32	jsmi_set_key_value_uint32
int jsmi_set_key_value_bytes(jsmi_composer_t *composer, const char *key, const char* value, size_t value_len);
int jsmi_set_key_value_str(jsmi_composer_t *composer, const char *key, const char* value, size_t value_len);
int jsmi_set_key_value_bool(jsmi_composer_t *composer, const char *key, bool value);
int jsmi_set_key_value_sint64(jsmi_composer_t *composer, const char *key, int64_t value);
int jsmi_set_key_value_uint64(jsmi_composer_t *composer, const char *key, uint64_t value);
#define jsmi_set_key_value_s64	jsmi_set_key_value_sint64
#define jsmi_set_key_value_u64	jsmi_set_key_value_uint64
int jsmi_set_key_value_double(jsmi_composer_t *composer, const char *key, double value, uint8_t precision);
int jsmi_set_key_object_begin(jsmi_composer_t *composer, const char *key);
int jsmi_set_key_object_end(jsmi_composer_t *composer);
int jsmi_set_key_array_begin(jsmi_composer_t *composer, const char *key);
int jsmi_set_key_array_end(jsmi_composer_t *composer);
#endif /* __JSMN_H__ */
