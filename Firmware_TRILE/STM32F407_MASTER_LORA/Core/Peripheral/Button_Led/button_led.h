/*
 * button_led.h
 *
 * Created on: May 27, 2026
 * Author: Lê Anh Trí Tri
 */

#ifndef BUTTON_LED_H
#define BUTTON_LED_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>

// =======================================================
// 1. ĐỊNH NGHĨA CHÂN VÀ PORT
// =======================================================

/* Nhóm Nút nhấn (Input) */

#if (DEFINE_PCB == 1)

    /* KIT TEST: Sử dụng K0 và K1 làm phím Trái/Phải */
    #define BTN_LEFT_PORT    GPIOE
    #define BTN_LEFT_PIN     GPIO_PIN_4   // Tương đương K0

    #define BTN_RIGHT_PORT   GPIOE
    #define BTN_RIGHT_PIN    GPIO_PIN_3   // Tương đương K1

    #define BTN_SELECT_PORT  GPIOE
    #define BTN_SELECT_PIN   GPIO_PIN_10

#elif (DEFINE_PCB == 0)

    /* DEV BOARD: Cấu hình mặc định */
    #define BTN_LEFT_PORT    GPIOE
    #define BTN_LEFT_PIN     GPIO_PIN_7

    #define BTN_RIGHT_PORT   GPIOE
    #define BTN_RIGHT_PIN    GPIO_PIN_9

    #define BTN_SELECT_PORT  GPIOE
    #define BTN_SELECT_PIN   GPIO_PIN_10

#endif

// =======================================================
// 2. MACRO ĐIỀU KHIỂN LED VÀ BUZZER
// =======================================================

#if (DEFINE_PCB == 1)

	#define LED01_ON()       HAL_GPIO_WritePin(LED_M1_GPIO_Port, LED_M1_Pin, GPIO_PIN_RESET)
	#define LED01_OFF()      HAL_GPIO_WritePin(LED_M1_GPIO_Port, LED_M1_Pin, GPIO_PIN_SET)
	#define LED01_TOGGLE()   HAL_GPIO_TogglePin(LED_M1_GPIO_Port, LED_M1_Pin)

	#define LED02_ON()       HAL_GPIO_WritePin(LED_M2_GPIO_Port, LED_M2_Pin, GPIO_PIN_RESET)
	#define LED02_OFF()      HAL_GPIO_WritePin(LED_M2_GPIO_Port, LED_M2_Pin, GPIO_PIN_SET)
	#define LED02_TOGGLE()   HAL_GPIO_TogglePin(LED_M2_GPIO_Port, LED_M2_Pin)

#else

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

// =======================================================
// 3. KHAI BÁO CÁC CỜ SỰ KIỆN NÚT NHẤN (Dùng cho main.c)
// =======================================================
// Định nghĩa ID cho từng nút để làm chỉ số (index) truy cập mảng

typedef enum
{
    BTN_LEFT = 0,
    BTN_RIGHT,
    BTN_SELECT,

    BTN_MAX_COUNT // Tự động đếm tổng số lượng nút nhấn (hiện tại là 3)

} Button_ID_t;

// Cấu trúc gói gọn toàn bộ thông tin và trạng thái của 1 nút
typedef struct
{
    GPIO_TypeDef* port;  // Port của vi điều khiển
    uint16_t      pin;   // Chân của vi điều khiển
    uint8_t       ref;   // Trạng thái cũ (Reference state)
    volatile bool flag;  // Cờ báo hiệu có sự kiện nhấn

} Button_t;

// Khai báo extern mảng nút nhấn để file main.c có thể gọi được
extern Button_t Buttons[BTN_MAX_COUNT];

// =======================================================
// 4. KHAI BÁO HÀM QUÉT NÚT NHẤN
// =======================================================
void Button_Scan_Timer(void);

// =======================================================
// 5. KHAI BÁO HÀM XỬ LÝ BUZZER
// =======================================================

void BUZZER_SetVolume(uint8_t vol);

#endif /* BUTTON_LED_H */
