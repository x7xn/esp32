#ifndef _MIIO_PSM_H_
#define _MIIO_PSM_H_

#include "miio_define.h"

int miio_psm_init(unsigned int flash_address, size_t flash_size);
int miio_psm_erase_key(const char* name_space, const char* key);
int miio_psm_erase_country_domain(const char* key);
int miio_psm_get_value(const char* name_space, const char* key, void *value, size_t length);
int miio_psm_set_value(const char* name_space, const char* key, const void* value, size_t length);
int miio_psm_get_str(const char* name_space, const char* key, char *str, size_t str_size);
int miio_psm_get_country_domain(const char* key, char *str, size_t str_size);
int miio_psm_set_str(const char* name_space, const char* key, const char* str);
int miio_psm_set_country_domain(const char* key, const char* str);

#endif
