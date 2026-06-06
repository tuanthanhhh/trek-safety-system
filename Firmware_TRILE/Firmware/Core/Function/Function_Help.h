/*
 * Function_Help.h
 *
 *  Created on: Jun 6, 2026
 *      Author: Antigravity
 */

#ifndef CORE_FUNCTION_FUNCTION_HELP_H_
#define CORE_FUNCTION_FUNCTION_HELP_H_

#include "slave_lora.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    HELP_STATE_NONE = 0,
    HELP_STATE_POPUP,
    HELP_STATE_NAVIGATING
} Help_State_t;

extern Help_State_t help_state;

/**
 * @brief Kích hoạt trạng thái cứu trợ khi nhận được gói tin SOS hoặc HELP
 * @param p_frame Con trỏ tới khung dữ liệu LoRa nhận được
 */
void Function_Help_Trigger(lora_frame_t *p_frame);

/**
 * @brief Kiểm tra xem chức năng cứu trợ có đang hoạt động hay không
 * @retval true: Đang hoạt động, false: Không hoạt động
 */
bool Function_Help_Is_Active(void);

/**
 * @brief Vẽ giao diện popup hoặc giao diện dẫn đường cứu trợ lên màn hình SH1106
 */
void Function_Help_Draw(void);

/**
 * @brief Xử lý ngắt nút nhấn khi đang ở màn hình cứu trợ
 * @param GPIO_Pin Chân ngắt nút nhấn nhận được
 * @retval true nếu nút nhấn được tiêu thụ bởi chức năng này, false nếu chuyển cho hệ thống xử lý mặc định
 */
bool Function_Help_Handle_Button(uint16_t GPIO_Pin);

/**
 * @brief Xóa trạng thái bỏ qua (ignore) của một Node khi nhận được tin tracking bình thường
 * @param node_id ID của Node cần xóa bỏ qua
 */
void Function_Help_Clear_Ignore(uint8_t node_id);

#endif /* CORE_FUNCTION_FUNCTION_HELP_H_ */
