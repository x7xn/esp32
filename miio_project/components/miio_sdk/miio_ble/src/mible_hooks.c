/* Includes ------------------------------------------------------------------*/

#include "mible_hooks.h"

/* Private define ------------------------------------------------------------*/

#define MIBLE_HOOKS_NUM                           4

/* Private variables ---------------------------------------------------------*/

static mible_hooks_t* hooks_func[MIBLE_HOOKS_NUM];
static arch_os_mutex_handle_t hooks_mutex;

/* Exported functions --------------------------------------------------------*/

int mible_hooks_init(void)
{
    for (int index = 0; index < MIBLE_HOOKS_NUM; index++) {
        hooks_func[index] = NULL;
    }

    return arch_os_mutex_create(&hooks_mutex);
}

int mible_hooks_register(mible_hooks_t *p_hooks)
{
    int index;

    if (NULL == p_hooks) {
        return MIIO_ERROR_PARAM;
    }

    arch_os_mutex_get(hooks_mutex, ARCH_OS_WAIT_FOREVER);
    for (index = 0; index < MIBLE_HOOKS_NUM; index++) {
        if (NULL == hooks_func[index]) {
            hooks_func[index] = p_hooks;
            break;
        }
    }
    arch_os_mutex_put(hooks_mutex);

    return (index < MIBLE_HOOKS_NUM) ? MIIO_OK : MIIO_ERROR_FULL;
}

int mible_hooks_unregister(mible_hooks_t *p_hooks)
{
    int index;

    if (NULL == p_hooks) {
        return MIIO_ERROR_PARAM;
    }

    arch_os_mutex_get(hooks_mutex, ARCH_OS_WAIT_FOREVER);
    for (index = 0; index < MIBLE_HOOKS_NUM; index++) {
        if (p_hooks == hooks_func[index]) {
            hooks_func[index] = NULL;
            break;
        }
    }
    arch_os_mutex_put(hooks_mutex);

    return (index < MIBLE_HOOKS_NUM) ? MIIO_OK : MIIO_ERROR_NOTFOUND;
}

void mible_hooks_get_object(mible_addr_t addr, mibeacon_object_t *p_object)
{
    arch_os_mutex_get(hooks_mutex, ARCH_OS_WAIT_FOREVER);
    for (int index = 0; index < MIBLE_HOOKS_NUM; index++) {
        if (NULL != hooks_func[index] && NULL != hooks_func[index]->get_object) {
            hooks_func[index]->get_object(addr, p_object);
        }
    }
    arch_os_mutex_put(hooks_mutex);
}

void mible_hooks_fastpair_check(mible_addr_t addr, uint16_t product_id,
                                            uint16_t object_id, int8_t rssi)
{
    arch_os_mutex_get(hooks_mutex, ARCH_OS_WAIT_FOREVER);
    for (int index = 0; index < MIBLE_HOOKS_NUM; index++) {
        if (NULL != hooks_func[index] && NULL != hooks_func[index]->fastpair_check) {
            hooks_func[index]->fastpair_check(addr, product_id, object_id, rssi);
        }
    }
    arch_os_mutex_put(hooks_mutex);
}

void mible_hooks_fastpair_report(mible_addr_t addr, fastpair_state_t state)
{
    arch_os_mutex_get(hooks_mutex, ARCH_OS_WAIT_FOREVER);
    for (int index = 0; index < MIBLE_HOOKS_NUM; index++) {
        if (NULL != hooks_func[index] && NULL != hooks_func[index]->fastpair_report) {
            hooks_func[index]->fastpair_report(addr, state);
        }
    }
    arch_os_mutex_put(hooks_mutex);
}

void mible_hooks_linkage_rule_report(void)
{
    arch_os_mutex_get(hooks_mutex, ARCH_OS_WAIT_FOREVER);
    for (int index = 0; index < MIBLE_HOOKS_NUM; index++) {
        if (NULL != hooks_func[index] && NULL != hooks_func[index]->linkage_rule_report) {
            hooks_func[index]->linkage_rule_report();
        }
    }
    arch_os_mutex_put(hooks_mutex);
}

void mible_hooks_linkage_event_report(mible_addr_t addr, uint16_t pid, mibeacon_object_t *p_object)
{
    arch_os_mutex_get(hooks_mutex, ARCH_OS_WAIT_FOREVER);
    for (int index = 0; index < MIBLE_HOOKS_NUM; index++) {
        if (NULL != hooks_func[index] && NULL != hooks_func[index]->linkage_event_report) {
            hooks_func[index]->linkage_event_report(addr, pid, p_object);
        }
    }
    arch_os_mutex_put(hooks_mutex);
}

