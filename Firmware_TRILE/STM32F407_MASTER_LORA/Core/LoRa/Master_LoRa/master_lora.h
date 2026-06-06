/*
 * master.h
 *
 * Created on: May 22, 2026
 * Author: Lê Anh Trí Tri
 */

#ifndef MASTER_LORA_H
#define MASTER_LORA_H

#include <stdio.h>
#include "main.h"

#include "define_board.h"

#include "usb.h"
#include "i2c-lcd.h"
#include "button_led.h"
#include "LoRa_Frame.h"

void Master_Lora_Init(SPI_HandleTypeDef *hspi,
        GPIO_TypeDef *cs_port, uint16_t cs_pin,
        GPIO_TypeDef *rst_port, uint16_t rst_pin,
        GPIO_TypeDef *dio0_port, uint16_t dio0_pin);

void Master_Lora_Loop();

#endif /* MASTER_H */
