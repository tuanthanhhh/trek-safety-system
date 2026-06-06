/*
 * Function_Help.c
 *
 *  Created on: Jun 6, 2026
 *      Author: Antigravity
 */

#include "Function_Help.h"
#include "SH1106.h"
#include "lsm303dlhc.h"
#include "gps_gp02.h"
#include "Application.h"
#include "main.h"
#include "UI.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define EARTH_RADIUS     6371000.0f
#define DEG_TO_RAD_VAL   0.0174532925f

Help_State_t help_state = HELP_STATE_NONE;

static uint8_t distress_node_id = 0;
static float distress_lat = 0.0f;
static float distress_lng = 0.0f;
static uint8_t selected_option = 0; // 0: Ho tro, 1: Bo qua

// Bảng lưu danh sách các Node bị bỏ qua (ignore) SOS
static bool node_ignored[256] = {false};

void Function_Help_Trigger(lora_frame_t *p_frame)
{
    if (p_frame == NULL) return;

    // 1. Không ghi đè hoặc ngắt quãng khi đang hiển thị POPUP hoặc đang dẫn đường
    if (help_state != HELP_STATE_NONE) return;

    // Xác định ID của Node thực sự cần giúp đỡ:
    // - Với gói HELP (do Master gửi): ID nằm ở id_dest
    // - Với gói SOS (do chính Slave đó gửi): ID nằm ở id_source
    uint8_t target_node_id = (p_frame->cmd_lora == CMD_LORA_HELP) ? p_frame->id_dest : p_frame->id_source;

    // Không xử lý nếu đối tượng cần cứu trợ là chính bản thân mình (ID_DEVICE)
    if (target_node_id == ID_DEVICE) return;

    // 2. Không nhận tin SOS từ Node này nếu đang bị bỏ qua (ignored)
    if (target_node_id < 256 && node_ignored[target_node_id]) return;

    distress_node_id = target_node_id;
    distress_lat = p_frame->gps.lat;
    distress_lng = p_frame->gps.lng;
    selected_option = 0; // Mặc định chọn Hỗ trợ
    help_state = HELP_STATE_POPUP;
}

bool Function_Help_Is_Active(void)
{
    return (help_state != HELP_STATE_NONE);
}

static float Get_Current_Heading(void)
{
    int16_t accel_data[3];
    int16_t mag_data[3];

    LSM303DLHC_AccReadXYZ(accel_data);
    LSM303DLHC_MagReadXYZ(mag_data);

    float ax = (float)accel_data[0];
    float ay = (float)accel_data[1];
    float az = (float)accel_data[2];

    float mx_raw = (float)mag_data[0];
    float my_raw = (float)mag_data[1];
    float mz_raw = (float)mag_data[2];

    /* Bias từ MagMaster */
    float bx = -54.251f;
    float by = 0.16f;
    float bz = -42.926f;

    /* Ma trận xoay hiệu chuẩn từ MagMaster */
    float M11 = 0.998f;
    float M12 = -0.007f;
    float M13 = -0.002f;

    float M21 = 0.001f;
    float M22 = 1.006f;
    float M23 = -0.058f;

    float M31 = -0.013f;
    float M32 = 0.039f;
    float M33 = 0.996f;

    /* Khử nhiễu bias */
    float mx_b = mx_raw - bx;
    float my_b = my_raw - by;
    float mz_b = mz_raw - bz;

    /* Nhân ma trận hiệu chỉnh */
    float mx = M11 * mx_b + M12 * my_b + M13 * mz_b;
    float my = M21 * mx_b + M22 * my_b + M23 * mz_b;
    float mz = M31 * mx_b + M32 * my_b + M33 * mz_b;

    /* Tính góc Roll và Pitch từ cảm biến gia tốc */
    pitch = atan2f(-ax, sqrtf(ay * ay + az * az));
    roll  = atan2f(ay, az);

    /* Bù nghiêng la bàn (Tilt Compensation) */
    float x_heading = mx * cosf(pitch)
                    + my * sinf(roll) * sinf(pitch)
                    - mz * cosf(roll) * sinf(pitch);

    float y_heading = my * cosf(roll)
                    + mz * sinf(roll);

    /* Tính toán góc hướng Heading */
    #define HEADING_OFFSET_HELP    100
    float local_heading = atan2f(y_heading, x_heading) * 57.2957795f;
    local_heading = local_heading + HEADING_OFFSET_HELP;

    if (local_heading < 0.0f)
    {
        local_heading += 360.0f;
    }
    else if (local_heading >= 360.0f)
    {
        local_heading -= 360.0f;
    }

    heading = (double)local_heading;
    return local_heading;
}

void Function_Help_Draw(void)
{
    SH1106_Fill(SH1106_COLOR_BLACK);

    if (help_state == HELP_STATE_POPUP)
    {
        // Vẽ khung viền Popup ngoài
        SH1106_DrawRectangle(2, 2, 124, 60, SH1106_COLOR_WHITE);
        SH1106_DrawRectangle(4, 4, 120, 56, SH1106_COLOR_WHITE);

        char buf[32];
        SH1106_GotoXY(12, 8);
        SH1106_Puts("SOS / HELP", &Font_11x18, SH1106_COLOR_WHITE);

        snprintf(buf, sizeof(buf), "NODE ID: %02d", distress_node_id);
        SH1106_GotoXY(28, 28);
        SH1106_Puts(buf, &Font_7x10, SH1106_COLOR_WHITE);

        // Nút lựa chọn 1: Hỗ trợ
        if (selected_option == 0)
        {
            SH1106_DrawFilledRectangle(12, 42, 48, 14, SH1106_COLOR_WHITE);
            SH1106_GotoXY(16, 44);
            SH1106_Puts("Ho tro", &Font_7x10, SH1106_COLOR_BLACK);
        }
        else
        {
            SH1106_DrawRectangle(12, 42, 48, 14, SH1106_COLOR_WHITE);
            SH1106_GotoXY(16, 44);
            SH1106_Puts("Ho tro", &Font_7x10, SH1106_COLOR_WHITE);
        }

        // Nút lựa chọn 2: Bỏ qua
        if (selected_option == 1)
        {
            SH1106_DrawFilledRectangle(68, 42, 48, 14, SH1106_COLOR_WHITE);
            SH1106_GotoXY(72, 44);
            SH1106_Puts("Bo qua", &Font_7x10, SH1106_COLOR_BLACK);
        }
        else
        {
            SH1106_DrawRectangle(68, 42, 48, 14, SH1106_COLOR_WHITE);
            SH1106_GotoXY(72, 44);
            SH1106_Puts("Bo qua", &Font_7x10, SH1106_COLOR_WHITE);
        }
    }
    else if (help_state == HELP_STATE_NAVIGATING)
    {
        // 1. Cập nhật góc hướng la bàn hiện thời của Slave
        float cur_heading = Get_Current_Heading();

        // 2. Tính toán khoảng cách và góc bearing tới mục tiêu
        double my_lat = current_gps.latitude;
        double my_lng = current_gps.longitude;

        double d_lat = (distress_lat - my_lat) * DEG_TO_RAD_VAL;
        double d_lng = (distress_lng - my_lng) * DEG_TO_RAD_VAL;
        double mean_lat = (my_lat + distress_lat) * 0.5f * DEG_TO_RAD_VAL;

        double x = d_lng * cos(mean_lat);
        double y = d_lat;
        double distance = sqrt(x * x + y * y) * EARTH_RADIUS;

        double lat1 = my_lat * DEG_TO_RAD_VAL;
        double lat2 = distress_lat * DEG_TO_RAD_VAL;
        double y_brg = sin(d_lng) * cos(lat2);
        double x_brg = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(d_lng);
        double bearing = atan2(y_brg, x_brg) * 57.2957795f;
        if (bearing < 0.0f)
        {
            bearing += 360.0f;
        }

        // Góc hướng tương đối để xoay mũi tên chỉ đường
        float rel_angle = bearing - cur_heading;
        if (rel_angle < 0.0f) rel_angle += 360.0f;
        if (rel_angle >= 360.0f) rel_angle -= 360.0f;

        // Vẽ thông số dẫn đường bên trái màn hình
        SH1106_GotoXY(2, 2);
        SH1106_Puts("NAVIGATING", &Font_7x10, SH1106_COLOR_WHITE);
        SH1106_DrawLine(2, 13, 70, 13, SH1106_COLOR_WHITE);

        char buf[32];
        snprintf(buf, sizeof(buf), "Node:%02d", distress_node_id);
        SH1106_GotoXY(2, 18);
        SH1106_Puts(buf, &Font_7x10, SH1106_COLOR_WHITE);

        if (current_gps.is_valid && distress_lat != 0.0f && distress_lng != 0.0f)
        {
            snprintf(buf, sizeof(buf), "Dist:%dm", (int)distance);
            SH1106_GotoXY(2, 30);
            SH1106_Puts(buf, &Font_7x10, SH1106_COLOR_WHITE);

            snprintf(buf, sizeof(buf), "Brg: %d*", (int)bearing);
            SH1106_GotoXY(2, 42);
            SH1106_Puts(buf, &Font_7x10, SH1106_COLOR_WHITE);
        }
        else
        {
            SH1106_GotoXY(2, 30);
            SH1106_Puts("Wait GPS", &Font_7x10, SH1106_COLOR_WHITE);
        }

        snprintf(buf, sizeof(buf), "Hdg: %d*", (int)cur_heading);
        SH1106_GotoXY(2, 52);
        SH1106_Puts(buf, &Font_7x10, SH1106_COLOR_WHITE);

        // Vẽ đồng hồ chỉ hướng la bàn hoặc chữ "Arrived" ở bên phải màn hình
        int cx = 96, cy = 32, r = 18;
        if (current_gps.is_valid && distress_lat != 0.0f && distress_lng != 0.0f && distance < 6.0)
        {
            SH1106_GotoXY(cx - 24, cy - 5);
            SH1106_Puts("Arrived", &Font_7x10, SH1106_COLOR_WHITE);
        }
        else
        {
            SH1106_DrawCircle(cx, cy, 21, SH1106_COLOR_WHITE);

            // Vẽ các vạch chia nhỏ hướng N/S/E/W
            SH1106_DrawLine(cx, cy - 21, cx, cy - 19, SH1106_COLOR_WHITE); // North
            SH1106_DrawLine(cx, cy + 19, cx, cy + 21, SH1106_COLOR_WHITE); // South
            SH1106_DrawLine(cx - 21, cy, cx - 19, cy, SH1106_COLOR_WHITE); // West
            SH1106_DrawLine(cx + 19, cy, cx + 21, cy, SH1106_COLOR_WHITE); // East

            // Vẽ mũi tên hướng xoay tương đối
            float rad = (rel_angle - 90.0f) * DEG_TO_RAD_VAL;
            int x_end = cx + (int)(cosf(rad) * r);
            int y_end = cy + (int)(sinf(rad) * r);

            // Thân mũi tên
            SH1106_DrawLine(cx, cy, x_end, y_end, SH1106_COLOR_WHITE);

            // Đầu mũi tên (2 cánh lệch góc 150 độ chỉ ngược lại từ điểm cuối)
            float wing1 = rad + 2.61799f;
            float wing2 = rad - 2.61799f;
            int arrow_w = 5;
            SH1106_DrawLine(x_end, y_end, x_end + (int)(cosf(wing1) * arrow_w), y_end + (int)(sinf(wing1) * arrow_w), SH1106_COLOR_WHITE);
            SH1106_DrawLine(x_end, y_end, x_end + (int)(cosf(wing2) * arrow_w), y_end + (int)(sinf(wing2) * arrow_w), SH1106_COLOR_WHITE);
        }
    }

    SH1106_UpdateScreen();
}

bool Function_Help_Handle_Button(uint16_t GPIO_Pin)
{
    if (help_state == HELP_STATE_NONE)
    {
        return false;
    }

    if (help_state == HELP_STATE_POPUP)
    {
        if (GPIO_Pin == SW_LEFT_Pin || GPIO_Pin == SW_RIGHT_Pin)
        {
            selected_option = (selected_option == 0) ? 1 : 0;
            return true;
        }
        else if (GPIO_Pin == SW_SELECT_Pin)
        {
            if (selected_option == 0)
            {
                help_state = HELP_STATE_NAVIGATING;
                SH1106_Clear(); // Xóa sạch màn hình trước khi vẽ giao diện Navigating
            }
            else
            {
                help_state = HELP_STATE_NONE;
                SH1106_Clear(); // Xóa sạch màn hình cứu hộ
                UI_state_old = (UI_State_t)100; // Ép hệ thống vẽ lại giao diện cũ

                // Đánh dấu Node này vào danh sách bị bỏ qua (ignore) nếu là cứu hộ
                if (distress_node_id < 256)
                {
                    node_ignored[distress_node_id] = true;
                }
            }
            return true;
        }
    }
    else if (help_state == HELP_STATE_NAVIGATING)
    {
        // Khi đang dẫn đường, nhấn nút SELECT hoặc nút trái (LEFT) sẽ thoát màn hình
        if (GPIO_Pin == SW_SELECT_Pin || GPIO_Pin == SW_LEFT_Pin)
        {
            help_state = HELP_STATE_NONE;
            SH1106_Clear(); // Xóa sạch màn hình cứu hộ
            UI_state_old = (UI_State_t)100; // Ép hệ thống vẽ lại giao diện cũ

            // Đánh dấu Node này vào danh sách bị bỏ qua (ignore) để không bị popup lại nếu là cứu hộ
            if (distress_node_id < 256)
            {
                node_ignored[distress_node_id] = true;
            }
            return true;
        }
    }

    return true; // Chặn các sự kiện nút nhấn khác
}

void Function_Help_Clear_Ignore(uint8_t node_id)
{
    if (node_id < 256)
    {
        node_ignored[node_id] = false;
    }
}
