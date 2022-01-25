#ifndef MESH_GW_UART_PARSER_H
#define MESH_GW_UART_PARSER_H

uint16_t mesh_gw_uart_data_parsing(uint8_t* source_data,uint16_t data_len);
uint8_t calculate_checksum_result(uint8_t* source_data,uint8_t calculate_len);

#endif
