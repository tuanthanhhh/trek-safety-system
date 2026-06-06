/*
 * Function_Gather.h
 *
 *  Created on: Jun 6, 2026
 *      Author: Antigravity
 */

#ifndef CORE_FUNCTION_FUNCTION_GATHER_H_
#define CORE_FUNCTION_FUNCTION_GATHER_H_

#include "slave_lora.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    GATHER_STATE_NONE = 0,
    GATHER_STATE_POPUP,
    GATHER_STATE_NAVIGATING
} Gather_State_t;

extern Gather_State_t gather_state;

/**
 * @brief Kích hoạt trạng thái tập hợp khi nhận được gói tin MARK_POINT
 * @param p_frame Con trỏ tới khung dữ liệu LoRa nhận được
 */
void Function_Gather_Trigger(lora_frame_t *p_frame);

/**
 * @brief Kiểm tra xem chức năng tập hợp có đang hoạt động hay không
 * @retval true: Đang hoạt động, false: Không hoạt động
 */
bool Function_Gather_Is_Active(void);

/**
 * @brief Vẽ giao diện popup hoặc giao diện dẫn đường tập hợp lên màn hình SH1106
 */
void Function_Gather_Draw(void);

/**
 * @brief Xử lý ngắt nút nhấn khi đang ở màn hình tập hợp
 * @param GPIO_Pin Chân ngắt nút nhấn nhận được
 * @retval true nếu nút nhấn được tiêu thụ bởi chức năng này, false nếu chuyển cho hệ thống xử lý mặc định
 */
bool Function_Gather_Handle_Button(uint16_t GPIO_Pin);

#endif /* CORE_FUNCTION_FUNCTION_GATHER_H_ */
