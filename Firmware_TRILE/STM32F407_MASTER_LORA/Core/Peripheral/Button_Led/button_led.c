/*
 * button_led.c
 *
 * Created on: May 27, 2026
 * Author: Lê Anh Trí Tri
 */

#include "button_led.h"

// Khởi tạo mảng các nút nhấn: Gán Port, Pin, trạng thái ref mặc định là 1, cờ flag là false
Button_t Buttons[BTN_MAX_COUNT] =
{
    [BTN_LEFT]   = {BTN_LEFT_PORT, BTN_LEFT_PIN,     GPIO_PIN_SET, false},
    [BTN_RIGHT]  = {BTN_RIGHT_PORT, BTN_RIGHT_PIN,   GPIO_PIN_SET, false},
    [BTN_SELECT] = {BTN_SELECT_PORT, BTN_SELECT_PIN, GPIO_PIN_SET, false}
};

/**
 * @brief Hàm quét mảng nút nhấn. Đặt trong ngắt Timer định kỳ (VD: 20ms)
 */
void Button_Scan_Timer(void)
{
    for (int i = 0; i < BTN_MAX_COUNT; i++)
    {
        uint8_t current_state = HAL_GPIO_ReadPin(Buttons[i].port, Buttons[i].pin);

        // Phát hiện sườn xuống (Falling Edge)
        if (current_state != Buttons[i].ref)
        {
            if (current_state == GPIO_PIN_RESET)
            {
                Buttons[i].flag = true; // Kích hoạt cờ sự kiện
            }
        }

        // Cập nhật lại trạng thái tham chiếu
        Buttons[i].ref = current_state;
    }
}

#define PWM_PERIOD_US 250

volatile uint16_t t_high = 0;
volatile uint16_t t_low = 250;
volatile uint8_t buzzer_pin_state = 0;

/* * Hàm cài đặt âm lượng
 * vol: 0 đến 100. Lưu ý với còi chíp, max âm lượng thực tế là ở duty 50%
 */
void BUZZER_SetVolume(uint8_t vol) {
    if (vol == 0) {
        t_high = 0;
        t_low = PWM_PERIOD_US;
    } else {
        // Ánh xạ dải 0-100 vào mức duty từ 0% đến 50%
        uint8_t duty_percent = vol / 2;

        t_high = (PWM_PERIOD_US * duty_percent) / 100;
        t_low = PWM_PERIOD_US - t_high;
    }
}

/* * Trình phục vụ ngắt Timer (Callback)
 * Hàm này tự động được gọi mỗi khi Timer đếm tràn
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM6) { // Kiểm tra đúng Timer đang dùng

        // Trường hợp âm lượng = 0 (Tắt còi hoàn toàn)
        if (t_high == 0) {
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, GPIO_PIN_RESET);
            buzzer_pin_state = 0;
            return;
        }

        // Thuật toán đổi trạng thái và nạp chu kỳ đếm mới
        if (buzzer_pin_state == 0) {
            // Chuyển sang mức CAO
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, GPIO_PIN_SET);
            __HAL_TIM_SET_AUTORELOAD(htim, t_high); // Đổi thời gian ngắt tiếp theo thành T_high
            buzzer_pin_state = 1;
        } else {
            // Chuyển sang mức THẤP
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, GPIO_PIN_RESET);
            __HAL_TIM_SET_AUTORELOAD(htim, t_low);  // Đổi thời gian ngắt tiếp theo thành T_low
            buzzer_pin_state = 0;
        }
    }
}
