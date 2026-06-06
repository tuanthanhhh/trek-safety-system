/*
 * slave.h
 *
 * Created on: May 23, 2026
 * Author: Lê Anh Trí Tri
 */

#ifndef SLAVE_LORA_H
#define SLAVE_LORA_H

#include <stdbool.h>
#include <string.h>

#include "gps_gp02.h"
#include "led.h"
#include "LoRa_Frame.h"

typedef struct
{
    float lat;
    float lng;
    uint32_t last_seen_tick;
    bool is_valid;
} Other_Node_GPS_t;

extern Other_Node_GPS_t other_nodes_gps[256];

/* Các hàm khởi tạo và xử lý Mạng Mesh cho Slave */
void Slave_Lora_Init(SPI_HandleTypeDef *hspi,
        GPIO_TypeDef *cs_port, uint16_t cs_pin,
        GPIO_TypeDef *rst_port, uint16_t rst_pin,
        GPIO_TypeDef *dio0_port, uint16_t dio0_pin);

void Slave_Lora_Loop(void);

#endif /* SLAVE_LORA_H */
