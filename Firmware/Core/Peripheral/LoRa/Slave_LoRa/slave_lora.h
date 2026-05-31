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


// ----- CẤU HÌNH THÔNG SỐ MESH CHO SLAVE -----
#define CACHE_SIZE 10        // Số lượng gói tin gần nhất được lưu để chống lặp
#define MAX_HOPS   3         // Số bước nhảy tối đa cho một gói tin (Time-To-Live)

/* Các hàm khởi tạo và xử lý Mạng Mesh cho Slave */
void Slave_Lora_Init(SPI_HandleTypeDef *hspi,
        GPIO_TypeDef *cs_port, uint16_t cs_pin,
        GPIO_TypeDef *rst_port, uint16_t rst_pin,
        GPIO_TypeDef *dio0_port, uint16_t dio0_pin);

void Slave_Lora_Loop(void);

#endif /* SLAVE_LORA_H */
