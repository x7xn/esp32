/**
* @file    led_indicator.c
* @author  liuyujia
* @date    2019
*/
#include "miio_net_indicator.h"

#undef MIIO_LOG_TAG
#define MIIO_LOG_TAG			"miio_net_indicator"

#define MIIO_NET_INDICATOR_LED_BLINK_INTERVAL_MS			50

/* Indicator blink value
	normal blink: 1,1,0,0,0,0
	slow blink:	  1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	fast blink:	  1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
*/
#define MIIO_NET_INDICATOR_LED_BLINK_NORMAL           		(0x3)
#define MIIO_NET_INDICATOR_LED_BLINK_NORMAL_BITS        	(6)
#define MIIO_NET_INDICATOR_LED_BLINK_SLOW             		(0xf)
#define MIIO_NET_INDICATOR_LED_BLINK_SLOW_BITS				(20)
#define MIIO_NET_INDICATOR_LED_BLINK_FAST					(0x9)
#define MIIO_NET_INDICATOR_LED_BLINK_FAST_BITS				(20)
#define MIIO_NET_INDICATOR_LED_DARK							(0x0)
#define MIIO_NET_INDICATOR_LED_DARK_BITS					(1)
#define MIIO_NET_INDICATOR_LED_BRIGHT						(0x1)
#define MIIO_NET_INDICATOR_LED_BRIGHT_BITS					(1)

typedef struct {
	arch_os_mutex_handle_t mutex;

	arch_os_timer_handle_t led_blink_timer;
	uint32_t led_blink_bit_index;
	uint32_t led_blink_pin_current;
	uint32_t led_blink_value_current;
	uint32_t led_blink_bits_current;
	uint32_t led_blink_pin_next;
	uint32_t led_blink_value_next;
	uint32_t led_blink_bits_next;
#if	MIIO_NET_INDICATOR_LED_SINGLE
	uint32_t led_pin_blue_level;
#else
	uint32_t led_pin_blue_level;
	uint32_t led_pin_yellow_level;
#endif
}miio_net_indicator_t;

static miio_net_indicator_t s_miio_net_indicator = {0};

static void led_blink(uint32_t led_pin, uint32_t level)
{
#if	!MIIO_NET_INDICATOR_LED_COMMON_CATHODE
	level = level ? 0 : 1;
#endif

#if	MIIO_NET_INDICATOR_LED_SINGLE
	if(level != s_miio_net_indicator.led_pin_blue_level){
		gpio_set_level(MIIO_NET_INDICATOR_LED_PIN_BLUE, level);
		s_miio_net_indicator.led_pin_blue_level = level;
	}
#else
#if	MIIO_NET_INDICATOR_LED_COMMON_CATHODE
	uint32_t led_off = 0;
#else
	uint32_t led_off = 1;
#endif
	switch(led_pin){
		case MIIO_NET_INDICATOR_LED_PIN_BLUE:
			if(level != s_miio_net_indicator.led_pin_blue_level){
				gpio_set_level(MIIO_NET_INDICATOR_LED_PIN_BLUE, level);
				s_miio_net_indicator.led_pin_blue_level = level;
			}
			if(led_off != s_miio_net_indicator.led_pin_yellow_level){
				gpio_set_level(MIIO_NET_INDICATOR_LED_PIN_YELLOW, led_off);
				s_miio_net_indicator.led_pin_yellow_level = led_off;
			}
			break;
		case MIIO_NET_INDICATOR_LED_PIN_YELLOW:
			if(led_off != s_miio_net_indicator.led_pin_blue_level){
				gpio_set_level(MIIO_NET_INDICATOR_LED_PIN_BLUE, led_off);
				s_miio_net_indicator.led_pin_blue_level = led_off;
			}
			if(level != s_miio_net_indicator.led_pin_yellow_level){
				gpio_set_level(MIIO_NET_INDICATOR_LED_PIN_YELLOW, level);
				s_miio_net_indicator.led_pin_yellow_level = level;
			}
			break;
		}
#endif
}

int led_pin_init(void)
{
	uint32_t pin_bit_mask = 0;
#if MIIO_NET_INDICATOR_LED_SINGLE
	pin_bit_mask = (1ULL<<MIIO_NET_INDICATOR_LED_PIN_BLUE);
#else
	pin_bit_mask = ((1ULL<<MIIO_NET_INDICATOR_LED_PIN_BLUE) | (1ULL<<MIIO_NET_INDICATOR_LED_PIN_YELLOW));
#endif

    gpio_config_t io_conf = {0};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask = pin_bit_mask;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;

    //configure GPIO with the given settings
    if(ESP_OK != gpio_config(&io_conf)) {
    	LOG_ERROR_TAG(MIIO_LOG_TAG, "led_pin init error.");
    	return MIIO_ERROR;
    }

#if MIIO_NET_INDICATOR_LED_SINGLE
    gpio_set_level(MIIO_NET_INDICATOR_LED_PIN_BLUE, s_miio_net_indicator.led_pin_blue_level);
#else
    gpio_set_level(MIIO_NET_INDICATOR_LED_PIN_BLUE, s_miio_net_indicator.led_pin_blue_level);
    gpio_set_level(MIIO_NET_INDICATOR_LED_PIN_YELLOW, s_miio_net_indicator.led_pin_yellow_level);
#endif

    return MIIO_OK;
}


void miio_net_indicator_set(miio_net_state_t state)
{
	if(s_miio_net_indicator.mutex)
		arch_os_mutex_get(s_miio_net_indicator.mutex, ARCH_OS_WAIT_FOREVER);

	switch(state){
	case MIIO_NET_UNPROV:
		s_miio_net_indicator.led_blink_value_next = MIIO_NET_INDICATOR_LED_DARK;
		s_miio_net_indicator.led_blink_bits_next = MIIO_NET_INDICATOR_LED_DARK_BITS;
		s_miio_net_indicator.led_blink_pin_next = MIIO_NET_INDICATOR_LED_PIN_BLUE;
		break;
	case MIIO_NET_UAP:
#if MIIO_NET_INDICATOR_LED_SINGLE
		s_miio_net_indicator.led_blink_value_next = MIIO_NET_INDICATOR_LED_BLINK_SLOW;
		s_miio_net_indicator.led_blink_bits_next = MIIO_NET_INDICATOR_LED_BLINK_SLOW_BITS;
		s_miio_net_indicator.led_blink_pin_next = MIIO_NET_INDICATOR_LED_PIN_BLUE;
#else
		s_miio_net_indicator.led_blink_value_next = MIIO_NET_INDICATOR_LED_BLINK_NORMAL;
		s_miio_net_indicator.led_blink_bits_next = MIIO_NET_INDICATOR_LED_BLINK_NORMAL_BITS;
		s_miio_net_indicator.led_blink_pin_next = MIIO_NET_INDICATOR_LED_PIN_YELLOW;
#endif
		break;
	case MIIO_NET_DISCONNECTED:
		s_miio_net_indicator.led_blink_value_next = MIIO_NET_INDICATOR_LED_BLINK_NORMAL;
		s_miio_net_indicator.led_blink_bits_next = MIIO_NET_INDICATOR_LED_BLINK_NORMAL_BITS;
		s_miio_net_indicator.led_blink_pin_next = MIIO_NET_INDICATOR_LED_PIN_BLUE;
		break;
	case MIIO_NET_LOCAL:
	case MIIO_NET_CLOUD:
		s_miio_net_indicator.led_blink_value_next = MIIO_NET_INDICATOR_LED_BRIGHT;
		s_miio_net_indicator.led_blink_bits_next = MIIO_NET_INDICATOR_LED_BRIGHT_BITS;
		s_miio_net_indicator.led_blink_pin_next = MIIO_NET_INDICATOR_LED_PIN_BLUE;
		break;
	case MIIO_NET_UPDATING:
#if MIIO_NET_INDICATOR_LED_SINGLE
		s_miio_net_indicator.led_blink_value_next = MIIO_NET_INDICATOR_LED_BLINK_FAST;
		s_miio_net_indicator.led_blink_bits_next = MIIO_NET_INDICATOR_LED_BLINK_FAST_BITS;
		s_miio_net_indicator.led_blink_pin_next = MIIO_NET_INDICATOR_LED_PIN_BLUE;
#else
		s_miio_net_indicator.led_blink_value_next = MIIO_NET_INDICATOR_LED_BLINK_SLOW;
		s_miio_net_indicator.led_blink_bits_next = MIIO_NET_INDICATOR_LED_BLINK_SLOW_BITS;
		s_miio_net_indicator.led_blink_pin_next = MIIO_NET_INDICATOR_LED_PIN_YELLOW;
#endif
		break;
	}

	if(s_miio_net_indicator.mutex)
		arch_os_mutex_put(s_miio_net_indicator.mutex);
}


static void timer_led_blink(arch_os_timer_handle_t timer)
{
	miio_net_indicator_t *net_indicator = (miio_net_indicator_t *)arch_os_timer_get_context(timer);

	uint32_t level = (net_indicator->led_blink_value_current >> net_indicator->led_blink_bit_index) & 0x1;

	led_blink(net_indicator->led_blink_pin_current, level);

	net_indicator->led_blink_bit_index++;
	if(net_indicator->led_blink_bit_index >= net_indicator->led_blink_bits_current){
		net_indicator->led_blink_bit_index = 0;

		if(s_miio_net_indicator.mutex){
			if(MIIO_OK != arch_os_mutex_get(s_miio_net_indicator.mutex, 0))
				return;
		}

		net_indicator->led_blink_bits_current = net_indicator->led_blink_bits_next;
		net_indicator->led_blink_value_current = net_indicator->led_blink_value_next;
		net_indicator->led_blink_pin_current = net_indicator->led_blink_pin_next;

		if(s_miio_net_indicator.mutex)
			arch_os_mutex_put(s_miio_net_indicator.mutex);
	}
}

int miio_net_indicator_init(void)
{
	memset(&s_miio_net_indicator, 0, sizeof(s_miio_net_indicator));

	if(MIIO_OK != led_pin_init()){
		return MIIO_ERROR;
	}

	if(MIIO_OK != arch_os_timer_create(&s_miio_net_indicator.led_blink_timer, "netIndicatorTimer", MIIO_NET_INDICATOR_LED_BLINK_INTERVAL_MS, timer_led_blink, &s_miio_net_indicator, ARCH_OS_TIMER_PERIODIC, ARCH_OS_TIMER_AUTO_ACTIVATE)){
		return MIIO_ERROR;
	}

	if(MIIO_OK != arch_os_mutex_create(&(s_miio_net_indicator.mutex))){
		return MIIO_ERROR;
	}

	return MIIO_OK;
}

