/*
 * gps_gp02.h
 *
 * Created on: May 28, 2026
 * Author: Lê Anh Trí Tri
 */

#ifndef GPS_GP02_H
#define GPS_GP02_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

//#include "button_led.h"

// Báo cho thư viện biết sẽ sử dụng bộ UART1 từ main.c
extern UART_HandleTypeDef huart1;

// =======================================================
// 1. ĐỊNH NGHĨA CHÂN ĐIỀU KHIỂN
// =======================================================
#define GNSS_PPS_PORT     GPIOC
#define GNSS_PPS_PIN      GPIO_PIN_6

#define GNSS_CTRL_PORT    GPIOC
#define GNSS_CTRL_PIN     GPIO_PIN_7

#define GNSS_RST_PORT     GPIOC
#define GNSS_RST_PIN      GPIO_PIN_8

// =======================================================
// 2. CẤU TRÚC DỮ LIỆU GPS
// =======================================================
typedef struct
{
    float    latitude;       // Vĩ độ (Decimal Degrees)
    float    longitude;      // Kinh độ (Decimal Degrees)
    bool     is_valid;       // Trạng thái (true = Đã chốt được vị trí)
} GPS_Data_t;

// Biến toàn cục chứa tọa độ hiện tại, file main.c có thể gọi biến này
extern GPS_Data_t current_gps;

// =======================================================
// 3. KHAI BÁO HÀM
// =======================================================
void GPS_Init(void);
void GPS_Hardware_Reset(void);
void GPS_Power_Control(bool enable);

// Hàm dùng trong ngắt UART1 (UART Rx Interrupt)
void GPS_UART_Callback(UART_HandleTypeDef *huart);

// Hàm dùng trong vòng lặp while(1) để xử lý dữ liệu
void GPS_Process_Loop(void);

#endif /* GPS_GP02_H */
