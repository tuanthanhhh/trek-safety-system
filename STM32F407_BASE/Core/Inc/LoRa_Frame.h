/*
 * LoRa_Frame.h
 *
 *  Created on: May 17, 2026
 *      Author: ACER
 */

#ifndef INC_LORA_FRAME_H_
#define INC_LORA_FRAME_H_

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include "LoRa.h"
#include "main.h"

#define MASTER
// #define SLAVE

#define RX_BUFFER_SIZE  128

extern uint8_t rx_buffer[RX_BUFFER_SIZE];
extern uint8_t tx_buffer[RX_BUFFER_SIZE];

extern uint8_t rssi;

typedef enum
{
    TYPE_DEFAULT = 0,
	TYPE_SOS,
    TYPE_HELP,
	TYPE_BACK_HOME,

} type_data_t;

typedef struct
{
    uint8_t id_device; // 1 Byte
    uint8_t type_data; // 1 Byte
    uint8_t batt;      // 1 Byte

    struct {
    	float lat; // 4 Bytes
    	float lng; // 4 Bytes

    } gps; // 4+4 = 8 Bytes

    struct {
    	uint8_t temp; // 1 Byte
    	uint8_t humi; // 1 Byte

    } sensor; // 1+1 = 2 Bytes

    uint16_t crc; // 2 Bytes

} lora_frame_t; // 1+1+1+1(Padding)+4+4+1+1+2 = 16 Bytes

extern lora_frame_t lora;

bool Lora_Init(SPI_HandleTypeDef *hspi,
               GPIO_TypeDef *cs_port, uint16_t cs_pin,
               GPIO_TypeDef *rst_port, uint16_t rst_pin,
               GPIO_TypeDef *dio0_port, uint16_t dio0_pin);

uint8_t LoRa_Read_Frame(lora_frame_t *p_frame);
uint8_t LoRa_Send_Frame(lora_frame_t *p_frame, int type_data);

void Lora_Test_Frame(uint32_t *send_count);

uint16_t calculate_crc16(const uint8_t *data, uint16_t length);

#endif /* INC_LORA_FRAME_H_ */
