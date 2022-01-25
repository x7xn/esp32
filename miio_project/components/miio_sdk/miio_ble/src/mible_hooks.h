/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MIBLE_HOOKS_H__
#define __MIBLE_HOOKS_H__

/* Includes ------------------------------------------------------------------*/

#include "mible_def.h"
#include "mibeacon_def.h"
#include "mible_fastpair.h"

/* Exported types ------------------------------------------------------------*/

typedef void (* hooks_get_object)(mible_addr_t addr, mibeacon_object_t *p_object);
typedef void (* hooks_fastpair_check)(mible_addr_t addr, uint16_t product_id,
                                         uint16_t object_id, int8_t rssi);
typedef void (* hooks_fastpair_report)(mible_addr_t addr, fastpair_state_t state);
typedef void (* hooks_linkage_rule_report)(void);
typedef void (* hooks_linkage_event_report)(mible_addr_t addr, uint16_t product_id, mibeacon_object_t *p_object);



typedef struct {
    hooks_get_object get_object;
    hooks_fastpair_check fastpair_check;
    hooks_fastpair_report fastpair_report;
    hooks_linkage_rule_report linkage_rule_report;
    hooks_linkage_event_report linkage_event_report;
} mible_hooks_t;

/* Exported functions --------------------------------------------------------*/

int mible_hooks_init(void);
int mible_hooks_register(mible_hooks_t *p_hooks);
int mible_hooks_unregister(mible_hooks_t *p_hooks);

void mible_hooks_get_object(mible_addr_t addr, mibeacon_object_t *p_object);
void mible_hooks_fastpair_report(mible_addr_t addr, fastpair_state_t state);
void mible_hooks_fastpair_check(mible_addr_t addr, uint16_t product_id,
                                            uint16_t object_id, int8_t rssi);
void mible_hooks_linkage_rule_report(void);
void mible_hooks_linkage_event_report(mible_addr_t addr, uint16_t pid, mibeacon_object_t *p_object);


#endif
