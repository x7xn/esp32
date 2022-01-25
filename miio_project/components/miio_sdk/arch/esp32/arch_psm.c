#include "arch_psm.h"
#include "esp_partition.h"
#include "arch_dbg.h"
#if MIIO_PSM_ENABLE
#include "miio_psm.h"
#endif

#undef MIIO_LOG_TAG
#define MIIO_LOG_TAG			"arch_psm"
#undef LOG_LEVEL
#define LOG_LEVEL				LOG_LEVEL_ERROR

int arch_psm_init(void)
{
#if MIIO_PSM_ENABLE

	const esp_partition_t *find_partition = NULL;
	find_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_MINVS, NULL);

	if(find_partition == NULL){
		LOG_ERROR_TAG(MIIO_LOG_TAG, "No MINVS partition found!");
	}
	else{
		LOG_DEBUG_TAG(MIIO_LOG_TAG, "MINVS partition @ %x %d bytes", find_partition->address , find_partition->size);

		if(MIIO_OK != miio_psm_init(find_partition->address, find_partition->size)){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "miio_psm init failed!");
		}
	}

#endif

	int ret = nvs_flash_init();
	if(ESP_OK != ret){
		LOG_ERROR_TAG(MIIO_LOG_TAG, "nvs init failed! ret = %d", ret);
		nvs_flash_erase();
		ret = nvs_flash_init();
		if(ESP_OK != ret){
			LOG_ERROR_TAG(MIIO_LOG_TAG, "nvs init failed again! ret = %d", ret);
		}
		else{
			LOG_WARN_TAG(MIIO_LOG_TAG, "nvs erased and init ok");
		}
	}

	return MIIO_OK;
}


int arch_psm_erase_key(const char* name_space, const char* key)
{
#if MIIO_PSM_ENABLE

	return miio_psm_erase_key(name_space, key);

#else

	int ret = MIIO_OK;

	nvs_handle handle;

	ret = nvs_open(name_space, NVS_READWRITE, &handle);
	if (ret != ESP_OK) {
		LOG_WARN_TAG(MIIO_LOG_TAG, "failed to open NVS name_space (0x%x)", ret);
		ret = MIIO_ERROR;
		return ret;
	}

	ret = nvs_erase_key(handle, key);

	nvs_close(handle);

	if(ret != ESP_OK){
		LOG_WARN_TAG(MIIO_LOG_TAG, "failed (0x%x)", ret);
		ret = MIIO_ERROR;
		return ret;
	}

	ret = MIIO_OK;

	return ret;

#endif
}

int arch_psm_erase_country_domain(const char* key)
{
#if MIIO_PSM_ENABLE
	return miio_psm_erase_country_domain(key);
#else
	return MIIO_ERROR;
#endif
}

int arch_psm_get_value(const char* name_space, const char* key, void *value, size_t length)
{

#if MIIO_PSM_ENABLE

	return miio_psm_get_value(name_space, key, value, length);

#else
	int ret = MIIO_OK;

	nvs_handle handle;

	ret = nvs_open(name_space, NVS_READONLY, &handle);
	if (ret != ESP_OK) {
		LOG_WARN_TAG(MIIO_LOG_TAG, "failed to open NVS name_space (0x%x)", ret);
		ret = MIIO_ERROR;
		return ret;
	}

	ret = nvs_get_blob(handle, key, value, &length);

	nvs_close(handle);

	if(ret != ESP_OK){
		LOG_WARN_TAG(MIIO_LOG_TAG, "failed (0x%x)", ret);
		ret = MIIO_ERROR;
		return ret;
	}

	ret = length;

	return ret;

#endif
}


int arch_psm_set_value(const char* name_space, const char* key, const void* value, size_t length)
{
#if MIIO_PSM_ENABLE

    return miio_psm_set_value(name_space, key, value, length);

#else

    int ret = MIIO_OK;

	nvs_handle handle;

	ret = nvs_open(name_space, NVS_READWRITE, &handle);
	if (ret != ESP_OK) {
		LOG_WARN_TAG(MIIO_LOG_TAG, "failed to open NVS name_space (0x%x)", ret);
		ret = MIIO_ERROR;
		return ret;
	}

	ret = nvs_set_blob(handle, key, value, length);

	nvs_close(handle);

	if(ret != ESP_OK){
		LOG_WARN_TAG(MIIO_LOG_TAG, "failed (0x%x)", ret);
		ret = MIIO_ERROR;
		return ret;
	}

	ret = length;

	return ret;

#endif

}



int arch_psm_get_str(const char* name_space, const char* key, char *str, size_t str_size)
{
#if MIIO_PSM_ENABLE

	return miio_psm_get_str(name_space, key, str, str_size);

#else

	int ret = MIIO_OK;

	nvs_handle handle;

	ret = nvs_open(name_space, NVS_READONLY, &handle);
	if (ret != ESP_OK) {
		LOG_WARN_TAG(MIIO_LOG_TAG, "failed to open NVS name_space (0x%x)", ret);
		ret = MIIO_ERROR;
		return ret;
	}

	ret = nvs_get_str(handle, key, str, &str_size);

	nvs_close(handle);

	if(ret != ESP_OK){
		LOG_WARN_TAG(MIIO_LOG_TAG, "failed (0x%x)", ret);
		ret = MIIO_ERROR;
		return ret;
	}

	ret = str_size-1;

	return ret;

#endif
}

int arch_psm_get_country_domain(const char* key, char *str, size_t str_size)
{
#if MIIO_PSM_ENABLE
	return miio_psm_get_country_domain(key, str, str_size);
#else
	return MIIO_ERROR;
#endif
}



int arch_psm_set_str(const char* name_space, const char* key, const char* str)
{
#if MIIO_PSM_ENABLE

	return miio_psm_set_str(name_space, key, str);

#else
	int ret = MIIO_OK;
	int str_len = strlen(str);

	nvs_handle handle;

	ret = nvs_open(name_space, NVS_READWRITE, &handle);
	if (ret != ESP_OK) {
		LOG_WARN_TAG(MIIO_LOG_TAG, "failed to open NVS name_space (0x%x)", ret);
		ret = MIIO_ERROR;
		return ret;
	}

	ret = nvs_set_str(handle, key, str);

	nvs_close(handle);

	if(ret != ESP_OK){
		LOG_WARN_TAG(MIIO_LOG_TAG, "failed (0x%x)", ret);
		ret = MIIO_ERROR;
		return ret;
	}

	ret = str_len;

	return ret;

#endif

}

int arch_psm_set_country_domain(const char* key, char *str)
{
#if MIIO_PSM_ENABLE
	return miio_psm_set_country_domain(key, str);
#else
	return MIIO_ERROR;
#endif
}


