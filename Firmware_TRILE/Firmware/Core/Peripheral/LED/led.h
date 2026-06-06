/*
 * led.h
 *
 * Created on: May 27, 2026
 * Author: Lê Anh Trí Tri
 */

#ifndef LED_H
#define LED_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>

#define LED01_ON()       HAL_GPIO_WritePin(LED_01_GPIO_Port, LED_01_Pin, GPIO_PIN_RESET)
#define LED01_OFF()      HAL_GPIO_WritePin(LED_01_GPIO_Port, LED_01_Pin, GPIO_PIN_SET)
#define LED01_TOGGLE()   HAL_GPIO_TogglePin(LED_01_GPIO_Port, LED_01_Pin)

#define LED02_ON()       HAL_GPIO_WritePin(LED_02_GPIO_Port, LED_02_Pin, GPIO_PIN_RESET)
#define LED02_OFF()      HAL_GPIO_WritePin(LED_02_GPIO_Port, LED_02_Pin, GPIO_PIN_SET)
#define LED02_TOGGLE()   HAL_GPIO_TogglePin(LED_02_GPIO_Port, LED_02_Pin)

#define LED03_ON()       HAL_GPIO_WritePin(LED_03_GPIO_Port, LED_03_Pin, GPIO_PIN_RESET)
#define LED03_OFF()      HAL_GPIO_WritePin(LED_03_GPIO_Port, LED_03_Pin, GPIO_PIN_SET)
#define LED03_TOGGLE()   HAL_GPIO_TogglePin(LED_03_GPIO_Port, LED_03_Pin)

#endif
