#include "mible_def.h"
#include "adv_beacon.h"
#include "object_rule.h"
#include "mible_rpc.h"
#include "list/list.h"

#undef  MIBLE_LOG_TAG
#define MIBLE_LOG_TAG                             "mible_beacon"
#undef  LOG_LEVEL
#define LOG_LEVEL                                 LOG_LEVEL_INFO

extern arch_os_mutex_handle_t mibeacon_list_mutex;

typedef struct object_rule_list {
	object_rule_t rule;
	struct list_head list;
} object_rule_list_t;

static object_rule_list_t rule_list;
static uint8_t rule_list_count;

int object_rule_init(void)
{
    int ret = MIIO_OK;

    INIT_LIST_HEAD(&rule_list.list);
    rule_list_count = 0;

    return ret;
}

int object_rule_add(object_rule_t *p_rule)
{
    int ret = MIIO_OK;
    object_rule_list_t *tmp;
    struct list_head *pos, *q;

    if (NULL == p_rule) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "object_rule_add invalid param");
        return MIIO_ERROR_PARAM;
    }

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);

	list_for_each_safe(pos, q, &rule_list.list) {
		tmp = list_entry(pos, object_rule_list_t, list);
        if (tmp->rule.obj_id == p_rule->obj_id) {
            memcpy((uint8_t *)&tmp->rule, (uint8_t *)p_rule, sizeof(object_rule_t));
            goto end;
        }
	}

    if (rule_list_count >= MAX_OBJECT_RULE_LIST_COUNT) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "rule table is full");
        ret = MIIO_ERROR_FULL;
        goto end;
    }

    tmp = (object_rule_list_t *)malloc(sizeof(object_rule_list_t));
    if (tmp == NULL) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for object_rule_list");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memcpy((uint8_t *)&tmp->rule, (uint8_t *)p_rule, sizeof(object_rule_t));
    list_add_tail(&(tmp->list), &(rule_list.list));
    rule_list_count ++;

end:
    arch_os_mutex_put(mibeacon_list_mutex);

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "object_rule_add count = %d", rule_list_count);

    return ret;
}

int object_rule_delete(uint16_t obj_id)
{
    int ret = MIIO_OK;
    object_rule_list_t *tmp;
    struct list_head *pos, *q;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &rule_list.list) {
		tmp = list_entry(pos, object_rule_list_t, list);
        if (tmp->rule.obj_id == obj_id) {
            list_del(pos);
            free(tmp);
            rule_list_count --;
            goto end;
        }
	}

end:
    arch_os_mutex_put(mibeacon_list_mutex);

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "object_rule_delete count = %d", rule_list_count);

    return ret;
}

object_rule_t *object_rule_search(uint16_t obj_id)
{
    object_rule_list_t *tmp, *result = NULL;
    struct list_head *pos, *q;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &rule_list.list) {
		tmp = list_entry(pos, object_rule_list_t, list);
        if (tmp->rule.obj_id == obj_id) {
            result = tmp;
            goto end;
        }
	}

end:
    arch_os_mutex_put(mibeacon_list_mutex);

    if (result != NULL)
        return &result->rule;
    return NULL;
}

int object_rule_print(void)
{
    int ret = MIIO_OK;
    object_rule_list_t *tmp;
    struct list_head *pos, *q;
    int index = 0;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "rule_list_count = %d", rule_list_count);
    list_for_each_safe(pos, q, &rule_list.list) {
        tmp = list_entry(pos, object_rule_list_t, list);
        UNUSED(tmp->rule.obj_id);
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "rule_list: index = %d, obj_id = %04x", index,
                      tmp->rule.obj_id);
        index++;
    }
    arch_os_mutex_put(mibeacon_list_mutex);

    return ret;
}


typedef struct pid_obj_id_list {
	pid_obj_id_t pid_obj_id;
	struct list_head list;
} pid_obj_id_list_t;

static pid_obj_id_list_t pid_obj_id_list;
static uint8_t pid_obj_id_list_count;

int pid_obj_id_init(void)
{
    int ret = MIIO_OK;

    INIT_LIST_HEAD(&pid_obj_id_list.list);
    pid_obj_id_list_count = 0;

    return ret;
}

int pid_obj_id_add(pid_obj_id_t *p_pid_obj_id)
{
    int ret = MIIO_OK;
    pid_obj_id_list_t *tmp;
    struct list_head *pos, *q;

    if (NULL == p_pid_obj_id) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "pid_obj_id_add invalid param");
        return MIIO_ERROR_PARAM;
    }

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);

	list_for_each_safe(pos, q, &pid_obj_id_list.list) {
		tmp = list_entry(pos, pid_obj_id_list_t, list);
        if (tmp->pid_obj_id.pid == p_pid_obj_id->pid) {
            memcpy((uint8_t *)&tmp->pid_obj_id, (uint8_t *)p_pid_obj_id, sizeof(pid_obj_id_t));
            goto end;
        }
	}

    if (pid_obj_id_list_count >= MAX_PID_OBJ_ID_LIST_COUNT) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "pid table is full");
        ret = MIIO_ERROR_FULL;
        goto end;
    }

    tmp = (pid_obj_id_list_t *)malloc(sizeof(pid_obj_id_list_t));
    if (tmp == NULL) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for pid_obj_id_list");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memcpy((uint8_t *)&tmp->pid_obj_id, (uint8_t *)p_pid_obj_id, sizeof(pid_obj_id_t));
    list_add_tail(&(tmp->list), &(pid_obj_id_list.list));
    pid_obj_id_list_count ++;

end:
    arch_os_mutex_put(mibeacon_list_mutex);

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "pid_obj_id_add count = %d", pid_obj_id_list_count);

    return ret;
}

int pid_obj_id_delete(uint16_t pid)
{
    int ret = MIIO_OK;
    pid_obj_id_list_t *tmp;
    struct list_head *pos, *q;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &pid_obj_id_list.list) {
		tmp = list_entry(pos, pid_obj_id_list_t, list);
        if (tmp->pid_obj_id.pid == pid) {
            list_del(pos);
            free(tmp);
            pid_obj_id_list_count --;
            goto end;
        }
	}

end:
    arch_os_mutex_put(mibeacon_list_mutex);

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "pid_obj_id_delete count = %d", pid_obj_id_list_count);

    return ret;
}

pid_obj_id_t *pid_obj_id_search(uint16_t pid)
{
    pid_obj_id_list_t * tmp, *result = NULL;
    struct list_head *pos, *q;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &pid_obj_id_list.list) {
		tmp = list_entry(pos, pid_obj_id_list_t, list);
        if (tmp->pid_obj_id.pid == pid) {
            result = tmp;
            goto end;
        }
	}

end:
    arch_os_mutex_put(mibeacon_list_mutex);

    if (result != NULL)
        return &result->pid_obj_id;
    return NULL;
}

int pid_obj_id_print(void)
{
    int ret = MIIO_OK;
    pid_obj_id_list_t *tmp;
    struct list_head *pos, *q;
    int index = 0;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "pid_obj_id_list_count = %d", pid_obj_id_list_count);
	list_for_each_safe(pos, q, &pid_obj_id_list.list) {
		tmp = list_entry(pos, pid_obj_id_list_t, list);
        UNUSED(tmp->pid_obj_id.pid);
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "pid_obj_id_list: index = %d, pid = %d",
                  index, tmp->pid_obj_id.pid);
        index ++;
	}
    arch_os_mutex_put(mibeacon_list_mutex);

    return ret;
}

int pid_obj_id_decrease_ttl(void)
{
    int ret = MIIO_OK;
    pid_obj_id_list_t *tmp;
    struct list_head *pos, *q;

    bool in_advance_flag = FALSE;
    uint16_t in_advance_pid;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &pid_obj_id_list.list) {
		tmp = list_entry(pos, pid_obj_id_list_t, list);
        if (tmp->pid_obj_id.ttl > 0) {
            tmp->pid_obj_id.ttl --;
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "pid_obj_id_decrease_ttl pid = %d, ttl = %d",
                          tmp->pid_obj_id.pid, tmp->pid_obj_id.ttl);

            // ask for server in advance
            if (tmp->pid_obj_id.ttl == MIBEACON_IN_ADVANCE_TTL) {
                in_advance_flag = TRUE;
                in_advance_pid = tmp->pid_obj_id.pid;
            }
        } else {
            list_del(pos);
            free(tmp);
            pid_obj_id_list_count --;
            LOG_DEBUG_TAG(MIBLE_LOG_TAG, "pid_obj_id_decrease_ttl delete count = %d", pid_obj_id_list_count);
        }
	}
    arch_os_mutex_put(mibeacon_list_mutex);

    if (in_advance_flag) {
        mible_rpc_query_prod(in_advance_pid);
    }

    return ret;
}


typedef struct unknown_pid_list {
	unknown_pid_t unknown_pid;
	struct list_head list;
} unknown_pid_list_t;

static unknown_pid_list_t unknown_pid_list;
static uint8_t unknown_pid_list_count;

int unknown_pid_init(void)
{
    int ret = MIIO_OK;

    INIT_LIST_HEAD(&unknown_pid_list.list);
    unknown_pid_list_count = 0;

    return ret;
}

int unknown_pid_add(unknown_pid_t *p_unknown_pid)
{
    int ret = MIIO_OK;
    unknown_pid_list_t *tmp;
    struct list_head *pos, *q;

    if (NULL == p_unknown_pid) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "unknown_pid_add invalid param");
        return MIIO_ERROR_PARAM;
    }

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);

 	list_for_each_safe(pos, q, &unknown_pid_list.list) {
		tmp = list_entry(pos, unknown_pid_list_t, list);
        if (tmp->unknown_pid.pid == p_unknown_pid->pid) {
            memcpy((uint8_t *)&tmp->unknown_pid, (uint8_t *)p_unknown_pid, sizeof(unknown_pid_t));
            goto end;
        }
	}

    if (unknown_pid_list_count >= MAX_UNKNOWN_PID_LIST_COUNT) {
        LOG_WARN_TAG(MIBLE_LOG_TAG, "unknown pid table is full");
        ret = MIIO_ERROR_FULL;
        goto end;
    }

    tmp = (unknown_pid_list_t *)malloc(sizeof(unknown_pid_list_t));
    if (tmp == NULL) {
        LOG_ERROR_TAG(MIBLE_LOG_TAG, "No memory for unknown_pid_list");
        ret = MIIO_ERROR_NOMEM;
        goto end;
    }
    memcpy((uint8_t *)&tmp->unknown_pid, (uint8_t *)p_unknown_pid, sizeof(unknown_pid_t));
    list_add_tail(&(tmp->list), &(unknown_pid_list.list));
    unknown_pid_list_count ++;

end:
    arch_os_mutex_put(mibeacon_list_mutex);

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "unknown_pid_add count = %d ", unknown_pid_list_count);

    return ret;
}

int unknown_pid_delete(uint16_t pid)
{
    int ret = MIIO_OK;
    unknown_pid_list_t *tmp;
    struct list_head *pos, *q;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &unknown_pid_list.list) {
		tmp = list_entry(pos, unknown_pid_list_t, list);
        if (tmp->unknown_pid.pid == pid) {
            list_del(pos);
            free(tmp);
            unknown_pid_list_count --;
            goto end;
        }
	}

end:
    arch_os_mutex_put(mibeacon_list_mutex);

    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "unknown_pid_delete count = %d ", unknown_pid_list_count);

    return ret;
}

unknown_pid_t *unknown_pid_search(uint16_t pid)
{
    unknown_pid_list_t * tmp, *result = NULL;
    struct list_head *pos, *q;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &unknown_pid_list.list) {
		tmp = list_entry(pos, unknown_pid_list_t, list);
        if (tmp->unknown_pid.pid == pid) {
            result = tmp;
            goto end;
        }
	}

end:
    arch_os_mutex_put(mibeacon_list_mutex);

    if (result != NULL)
        return &result->unknown_pid;
    return NULL;
}

int unknown_pid_print(void)
{
    int ret = MIIO_OK;
    unknown_pid_list_t *tmp;
    struct list_head *pos, *q;
    int index = 0;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
    LOG_DEBUG_TAG(MIBLE_LOG_TAG, "unknown_pid_list_count = %d", unknown_pid_list_count);
	list_for_each_safe(pos, q, &unknown_pid_list.list) {
		tmp = list_entry(pos, unknown_pid_list_t, list);
        UNUSED(tmp->unknown_pid.pid);
        LOG_DEBUG_TAG(MIBLE_LOG_TAG, "unknown_pid_list: index = %d, pid = %d",
                  index, tmp->unknown_pid.pid);
        index ++;
	}
    arch_os_mutex_put(mibeacon_list_mutex);

    return ret;
}

int unknown_pid_decrease_ttl(void)
{
    int ret = MIIO_OK;
    unknown_pid_list_t *tmp;
    struct list_head *pos, *q;

    arch_os_mutex_get(mibeacon_list_mutex, ARCH_OS_WAIT_FOREVER);
	list_for_each_safe(pos, q, &unknown_pid_list.list) {
		tmp = list_entry(pos, unknown_pid_list_t, list);
        if (tmp->unknown_pid.ttl > 0) {
            tmp->unknown_pid.ttl --;
        } else {
            list_del(pos);
            free(tmp);
            unknown_pid_list_count --;
        }
	}
    arch_os_mutex_put(mibeacon_list_mutex);

    return ret;
}
