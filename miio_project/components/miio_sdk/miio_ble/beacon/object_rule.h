#ifndef __OBJECT_RULE_H__
#define __OBJECT_RULE_H__

#include "mibeacon_def.h"
#include "mible_dev.h"

typedef struct {
    uint16_t obj_id;
    uint32_t interval;
    uint32_t delta;
} object_rule_t;

typedef struct {
    uint16_t pid;
    uint8_t obj_num;
    uint16_t obj_id[MAX_OBJECT_ID_IN_PID];
    uint32_t ttl;
} pid_obj_id_t;

typedef struct {
    uint16_t pid;
    uint32_t ttl;
} unknown_pid_t;

int object_rule_init(void);
int object_rule_add(object_rule_t *p_rule);
int object_rule_delete(uint16_t obj_id);
object_rule_t *object_rule_search(uint16_t obj_id);
int object_rule_print(void);

int pid_obj_id_init(void);
int pid_obj_id_add(pid_obj_id_t *p_pid_obj_id);
int pid_obj_id_delete(uint16_t pid);
pid_obj_id_t *pid_obj_id_search(uint16_t pid);
int pid_obj_id_print(void);
int pid_obj_id_decrease_ttl(void);

int unknown_pid_init(void);
int unknown_pid_add(unknown_pid_t *p_unknown_pid);
int unknown_pid_delete(uint16_t pid);
unknown_pid_t *unknown_pid_search(uint16_t pid);
int unknown_pid_print(void);
int unknown_pid_decrease_ttl(void);

#endif
