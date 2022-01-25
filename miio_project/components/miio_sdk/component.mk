#
# Main Makefile. This is basically the same as a component makefile.
#
# This Makefile should, at the very least, just include $(SDK_PATH)/make/component_common.mk. By default, 
# this will take the sources in the src/ directory, compile them and link them into 
# lib(subdirectory_name).a in the build directory. This behaviour is entirely configurable,
# please read the ESP-IDF documents if you need to do this.
#


COMPONENT_ADD_INCLUDEDIRS := .
COMPONENT_SRCDIRS         := .


COMPONENT_ADD_INCLUDEDIRS += 	arch						\
								arch/misc					\
								arch/esp32					\
								arch/psm					\
								miio_common/include			\
								miio_common/source			\
								miio_common/source/list		\
								miio_common/source/jsmn		\
								miio_common/source/jsmi		\
								miio_common/source/tou		\
								miio_common/source/xmodem	\
								miio_common/source/wifi_channel	\
								miio_instance/ciphers		\
								miio_instance/cmd			\
								miio_instance/net			\
								miio_instance/ota			\
								miio_instance


ifdef CONFIG_BT_ENABLED

COMPONENT_ADD_INCLUDEDIRS +=	miio_ble/arch			\
								miio_ble/arch/esp32		\
								miio_ble/arch/esp32/micro-ecc/micro-ecc	\
								miio_ble/band			\
								miio_ble/beacon			\
								miio_ble/bond			\
								miio_ble/cmd			\
								miio_ble/dev			\
								miio_ble/net			\
								miio_ble/src			\
								miio_ble/utils

endif


COMPONENT_SRCDIRS 		  += 	arch						\
								arch/misc					\
								arch/esp32					\
								arch/psm					\
								miio_common/source			\
								miio_common/source/list		\
								miio_common/source/jsmn		\
								miio_common/source/jsmi		\
								miio_common/source/tou		\
								miio_common/source/xmodem	\
								miio_common/source/wifi_channel	\
								miio_instance/ciphers		\
								miio_instance/cmd			\
								miio_instance/net			\
								miio_instance/ota			\
								miio_instance


ifdef CONFIG_BT_ENABLED

COMPONENT_SRCDIRS         +=	miio_ble/arch/esp32		\
								miio_ble/arch/esp32/micro-ecc/micro-ecc		\
								miio_ble/band			\
								miio_ble/beacon			\
								miio_ble/bond			\
								miio_ble/cmd			\
								miio_ble/dev			\
								miio_ble/net			\
								miio_ble/src			\
								miio_ble/utils

endif


COMPONENT_ADD_LDFLAGS += \
	-L $(COMPONENT_PATH)/miio_common/libs -l miio \
	-L $(COMPONENT_PATH)/arch/psm -l miio_psm


ifdef CONFIG_BT_ENABLED

COMPONENT_ADD_LDFLAGS += \
	-L $(COMPONENT_PATH)/miio_ble/libs -l miio_ble

endif

ifdef CONFIG_BT_ENABLED

CFLAGS += -D MIBLE_ENABLE=1
CFLAGS += -D MIBLE_GATEWAY_DISABLE=0

else

CFLAGS += -D MIBLE_ENABLE=0
CFLAGS += -D MIBLE_GATEWAY_DISABLE=1

endif
