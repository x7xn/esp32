set(COMPONENT_ADD_INCLUDEDIRS device_driver mesh_gw)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES esp_hap_core)
set(COMPONENT_PRIV_REQUIRES )
set(COMPONENT_REQUIRES esp_hap_apple_profiles)


set(COMPONENT_SRCS device_driver/device_uart.c mesh_gw/mesh_gw_handler.c mesh_gw/mesh_gw_uart_parser.c)

register_component()
component_compile_options(-Wno-unused-function)

