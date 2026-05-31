/*
 * gps_gp02.c
 *
 * Created on: May 28, 2026
 * Author: Lê Anh Trí Tri
 */

#include "gps_gp02.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define NMEA_MAX_LEN 100
// Các biến quản lý bộ đệm UART
static char rx_buffer[NMEA_MAX_LEN];
static uint8_t rx_index = 0;
static bool nmea_ready = false;
static uint8_t rx_char; // Biến nhận từng byte từ UART1

// Biến lưu trữ dữ liệu GPS sau khi giải mã
GPS_Data_t current_gps =
{
    .latitude = 0.0f,
    .longitude = 0.0f,
    .is_valid = false
};

// =======================================================
// CÁC HÀM ĐIỀU KHIỂN PHẦN CỨNG
// =======================================================

/**
 * @brief Khởi tạo module GPS và tự động bật ngắt nhận UART1
 */
void GPS_Init(void)
{
    GPS_Power_Control(true);
    GPS_Hardware_Reset();
    HAL_UART_Receive_IT(&huart1, &rx_char, 1);
}

void GPS_Hardware_Reset(void)
{
    HAL_GPIO_WritePin(GNSS_RST_PORT, GNSS_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(GNSS_RST_PORT, GNSS_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(300); // Chờ module khởi động xong
}

void GPS_Power_Control(bool enable)
{
    if (enable)
    {
        HAL_GPIO_WritePin(GNSS_CTRL_PORT, GNSS_CTRL_PIN, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GNSS_CTRL_PORT, GNSS_CTRL_PIN, GPIO_PIN_RESET);
    }
}

// =======================================================
// CÁC HÀM XỬ LÝ CHUỖI NMEA (GIAO THỨC PHẦN MỀM)
// =======================================================

/**
 * @brief Chuyển đổi tọa độ NMEA (ddmm.mmmmm) sang Thập phân (dd.dddddd)
 */
static float NMEA_To_Decimal(float nmea_coord, char direction)
{
    int degrees = (int)(nmea_coord / 100.0f);
    float minutes = nmea_coord - (degrees * 100.0f);
    float decimal = degrees + (minutes / 60.0f);

    // Tọa độ âm nếu ở Bán cầu Nam (S) hoặc Tây (W)
    if (direction == 'S' || direction == 'W')
    {
        decimal = -decimal;
    }
    return decimal;
}

/**
 * @brief Phân tích bản tin RMC để chốt tọa độ
 */
static void Parse_NMEA_RMC(char* sentence)
{
    char* token;
    uint8_t field_idx = 0;

    char status = 'V';
    float raw_lat = 0.0f, raw_lon = 0.0f;
    char lat_dir = 'N', lon_dir = 'E';

    token = strtok(sentence, ",");

    while (token != NULL)
    {
        switch (field_idx)
        {
            case 2: status = token[0]; break;
            case 3: raw_lat = atof(token); break;
            case 4: lat_dir = token[0]; break;
            case 5: raw_lon = atof(token); break;
            case 6: lon_dir = token[0]; break;
        }
        token = strtok(NULL, ",");
        field_idx++;
    }

    if (status == 'A')
    {
        current_gps.is_valid = true;
        current_gps.latitude = NMEA_To_Decimal(raw_lat, lat_dir);
        current_gps.longitude = NMEA_To_Decimal(raw_lon, lon_dir);
    } else {
        current_gps.is_valid = false;
    }
}

// =======================================================
// HÀM NGẮT VÀ VÒNG LẶP (INTERRUPT & LOOP)
// =======================================================

/**
 * @brief Hàm gom byte. Đặt hàm này vào HAL_UART_RxCpltCallback trong main.c
 */
void GPS_UART_Callback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        if (!nmea_ready)
        {
            if (rx_char == '$') {
                rx_index = 0;
                rx_buffer[rx_index++] = rx_char;
            }
            else if (rx_char == '\n' || rx_char == '\r') {
                if (rx_index > 0) {
                    rx_buffer[rx_index] = '\0';
                    nmea_ready = true; // Báo hiệu đã nhận xong 1 dòng
                }
            }
            else {
                if (rx_index < NMEA_MAX_LEN - 1) {
                    rx_buffer[rx_index++] = rx_char;
                }
            }
        }
        // Kích hoạt nhận byte tiếp theo
        HAL_UART_Receive_IT(&huart1, &rx_char, 1);
    }
}

/**
 * @brief Hàm chạy nền. Đặt trong while(1)
 */
void GPS_Process_Loop(void)
{
    if (nmea_ready)
    {
        // Kiểm tra xem có phải gói RMC không (Bắt cả GNRMC, GPRMC, BDRMC)
        if (strstr(rx_buffer, "RMC") != NULL)
        {
            Parse_NMEA_RMC(rx_buffer);
        }
        // Xóa cờ để nhận dòng tiếp theo
        nmea_ready = false;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	GPS_UART_Callback(huart);
}
