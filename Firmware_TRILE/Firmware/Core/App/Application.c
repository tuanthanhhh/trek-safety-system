/*
 * Application.c
 *
 *  Created on: May 1, 2026
 *      Author: THANHLT
 */

#include "lsm303dlhc.h"
#include "DS3231.h"
#include "SH1106.h"
#include "BME280.h"
#include "fonts.h"
#include "main.h"
#include "Application.h"
#include "math.h"
#include "UI.h"
#include "gps_gp02.h"
#include "slave_lora.h"
#include "../Function/Function_Help.h"
#include "../Function/Function_Gather.h"

#define HEADING_OFFSET	100

BME280_Data_t BME280;
uint8_t DS3231_hour;
uint8_t DS3231_minute;
uint8_t DS3231_second;
uint8_t DS3231_day;
uint8_t DS3231_date;
uint8_t DS3231_month;
uint16_t DS3231_year;


double accel_g[3];
double roll;
double pitch;
double heading;

static MagCalib_t mag_calib = {
		.x_offset = 0,
		.y_offset = 0,
		.z_offset = 0,

		.x_scale = 1,
		.y_scale = 1,
		.z_scale = 1
};

void App_Trekking_Init()
{
	GPS_Init();
	SH1106_Init();
	BME280_Init();
	DS3231_Init(&hi2c2);
	LSM303DLHC_AccInit(LSM303DLHC_ODR_100_HZ | 7);
	LSM303DLHC_MagInit();
}
void App_Compass()
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

	/* Transformation Matrix từ MagMaster */
	float M11 = 0.998f;
	float M12 = -0.007f;
	float M13 = -0.002f;

	float M21 = 0.001f;
	float M22 = 1.006f;
	float M23 = -0.058f;

	float M31 = -0.013f;
	float M32 = 0.039f;
	float M33 = 0.996f;

	/* Trừ bias trước */
	float mx_b = mx_raw - bx;
	float my_b = my_raw - by;
	float mz_b = mz_raw - bz;

	/* Nhân ma trận calib */
	float mx = M11 * mx_b + M12 * my_b + M13 * mz_b;
	float my = M21 * mx_b + M22 * my_b + M23 * mz_b;
	float mz = M31 * mx_b + M32 * my_b + M33 * mz_b;

	/* Tính pitch roll từ accel */
	pitch = atan2f(-ax, sqrtf(ay * ay + az * az));
	roll  = atan2f(ay, az);

	/* Tilt compensation */
	float x_heading = mx * cosf(pitch)
	                + my * sinf(roll) * sinf(pitch)
	                - mz * cosf(roll) * sinf(pitch);

	float y_heading = my * cosf(roll)
	                + mz * sinf(roll);

	/* Heading */
	heading = atan2f(y_heading, x_heading) * 57.2957795f;
	heading = heading + HEADING_OFFSET;

	if (heading < 0.0f)
	{
	    heading += 360.0f;
	}
	else if (heading >= 360.0f)
	{
	    heading -= 360.0f;
	}

	UI_DrawCompassRotateDial(heading);

}
void App_Weather()
{
    BME280Calculation(&BME280);
}
void App_GPS()
{
    // Sử dụng Fill Black thay thế Clear để tránh nhấp nháy màn hình
    SH1106_Fill(SH1106_COLOR_BLACK);
    UI_state_old = UI_state;

    if (current_gps.is_valid)
    {
        char buffer[32];

        // 1. Vẽ tọa độ GPS của bản thân (Căn chỉnh khoảng cách đều đặn)
        snprintf(buffer, sizeof(buffer), "Lat: %.6f", current_gps.latitude);
        SH1106_GotoXY(1, 2);
        SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);

        snprintf(buffer, sizeof(buffer), "Lng: %.6f", current_gps.longitude);
        SH1106_GotoXY(1, 12);
        SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);

        // Vẽ đường chia ngang nằm chính xác ở giữa (Y=25)
        SH1106_DrawLine(0, 25, 128, 25, SH1106_COLOR_WHITE);

        // 2. Tính toán và vẽ khoảng cách đến các Node khác (Y=29)
        SH1106_GotoXY(1, 29);
        SH1106_Puts("MESH DISTANCES:", &Font_7x10, SH1106_COLOR_WHITE);

        int count = 0;
        int y_pos = 41;
        int x_pos = 1;

        #define EARTH_RADIUS     6371000.0f
        #define DEG_TO_RAD_VAL   0.0174532925f

        for (int i = 1; i < 256; i++)
        {
            if (i == ID_DEVICE) continue;

            // Nếu Node đó có GPS hợp lệ và được cập nhật trong vòng 5 phút (300000 ms) gần đây
            if (other_nodes_gps[i].is_valid && (HAL_GetTick() - other_nodes_gps[i].last_seen_tick < 300000))
            {
                double my_lat = current_gps.latitude;
                double my_lng = current_gps.longitude;
                double other_lat = other_nodes_gps[i].lat;
                double other_lng = other_nodes_gps[i].lng;

                double d_lat = (other_lat - my_lat) * DEG_TO_RAD_VAL;
                double d_lng = (other_lng - my_lng) * DEG_TO_RAD_VAL;
                double mean_lat = (my_lat + other_lat) * 0.5f * DEG_TO_RAD_VAL;

                double x = d_lng * cos(mean_lat);
                double y = d_lat;
                double distance = sqrt(x * x + y * y) * EARTH_RADIUS;

                char dist_buf[16];
                if (distance < 1000.0f)
                {
                    snprintf(dist_buf, sizeof(dist_buf), "N%02d:%dm", i, (int)distance);
                }
                else
                {
                    snprintf(dist_buf, sizeof(dist_buf), "N%02d:%.1fk", i, distance / 1000.0f);
                }

                SH1106_GotoXY(x_pos, y_pos);
                SH1106_Puts(dist_buf, &Font_7x10, SH1106_COLOR_WHITE);

                count++;
                if (count % 2 == 1)
                {
                    x_pos = 66; // Cột thứ 2
                }
                else
                {
                    x_pos = 1;  // Cột thứ 1
                    y_pos += 12; // Xuống dòng (giãn cách rộng rãi hơn)
                }

                if (count >= 4) // Hiển thị tối đa 4 node gần nhất để tránh tràn màn hình
                {
                    break;
                }
            }
        }

        if (count == 0)
        {
            SH1106_GotoXY(1, 41);
            SH1106_Puts("No other nodes", &Font_7x10, SH1106_COLOR_WHITE);
        }
    }
    else
    {
        SH1106_GotoXY(1, 20);
        SH1106_Puts("Waiting GPS ...", &Font_7x10, SH1106_COLOR_WHITE);
    }

    SH1106_UpdateScreen();
    HAL_Delay(UPDATE_SCREEN_TIME);
}
void App_CurrentTime()
{
    DS3231_hour = DS3231_GetHour();
    DS3231_minute = DS3231_GetMinute();
    DS3231_second = DS3231_GetSecond();
    DS3231_day = DS3231_GetDayOfWeek();
    DS3231_date = DS3231_GetDate();
    DS3231_month = DS3231_GetMonth();
    DS3231_year = DS3231_GetYear();
}

void App_UI()
{
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "T: %.1f C, H: %.1f %%, P: %.1f", BME280.Temperature, BME280.Humidity, BME280.Pressure);
    SH1106_GotoXY(1, 0);
    SH1106_Puts(buffer, &Font_7x10, 1);

    snprintf(buffer, sizeof(buffer), "%d:%d:%d", DS3231_hour, DS3231_minute, DS3231_second);
    SH1106_GotoXY(1, 12);
    SH1106_Puts(buffer, &Font_7x10, 1);

    snprintf(buffer, sizeof(buffer), "%.2f:%.2f:%.2f", accel_g[0], accel_g[1], accel_g[2]);
    SH1106_GotoXY(1, 24);
    SH1106_Puts(buffer, &Font_7x10, 1);

    snprintf(buffer, sizeof(buffer), "%f", heading);
    SH1106_GotoXY(1, 36);
    SH1106_Puts(buffer, &Font_7x10, 1);

    SH1106_UpdateScreen();
}

void BME280_Init(void)
{

  //Init structure definition section
	BME280_Init_t BME280_InitStruct = {0};

	//Reset section
	Reset_BME280();

	/*============================ *BME280 Initialization* ============================*/

	BME280_InitStruct.Filter = FILTER_8;     				//FILTER_X
	BME280_InitStruct.Mode = BME280_NORMAL_MODE;		 	//SLEEP, NORMAL or FORCE can be written
	BME280_InitStruct.OverSampling_H = OVERSAMPLING_16;		//OVERSAMPLING_X
	BME280_InitStruct.OverSampling_P = OVERSAMPLING_16;		//OVERSAMPLING_X
	BME280_InitStruct.OverSampling_T = OVERSAMPLING_16;		//OVERSAMPLING_X
	BME280_InitStruct.SPI_EnOrDıs = SPI3_W_DISABLE;			//SPI3_W_DISABLE or SPI3_W_ENABLE can be written
	BME280_InitStruct.T_StandBy = T_SB_250;					//T_SB_X

	BME280Init(BME280_InitStruct);
}

void Calib_Compass()
{
	int16_t accel_data[3];
	int16_t mag_data[3];
	LSM303DLHC_AccReadXYZ(accel_data);
	LSM303DLHC_MagReadXYZ(mag_data);

	float mx = (float)mag_data[0];
	float my =(float)mag_data[1];
    if (mx < mag_calib.x_min) mag_calib.x_min = mx;
    if (mx > mag_calib.x_max) mag_calib.x_max = mx;

    if (my < mag_calib.y_min) mag_calib.y_min = my;
    if (my > mag_calib.y_max) mag_calib.y_max = my;
}

void Finish_Calib_Compass()
{
    mag_calib.x_offset = (mag_calib.x_max + mag_calib.x_min) / 2.0f;
    mag_calib.y_offset = (mag_calib.y_max + mag_calib.y_min) / 2.0f;

    float x_range;
    float y_range;
    float avg_range;

    x_range = ((float)mag_calib.x_max - mag_calib.x_min) / 2.0f;
    y_range = ((float)mag_calib.y_max - mag_calib.y_min) / 2.0f;

    avg_range = (x_range + y_range) / 2.0f;
    if (x_range > 0.0f && y_range > 0.0f)
    {
        mag_calib.x_scale = avg_range / x_range;
        mag_calib.y_scale = avg_range / y_range;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    static uint32_t last_exti_ticks[16] = {0};
    uint32_t current_tick = HAL_GetTick();
    uint8_t pin_idx = 0;
    uint16_t temp_pin = GPIO_Pin;
    while (temp_pin > 1)
    {
        temp_pin >>= 1;
        pin_idx++;
    }

    if (pin_idx < 16)
    {
        if (current_tick - last_exti_ticks[pin_idx] < 250) // Chống dội nút nhấn 250ms
        {
            return;
        }
        last_exti_ticks[pin_idx] = current_tick;
    }

    if (Function_Help_Handle_Button(GPIO_Pin))
    {
        return;
    }
    if (Function_Gather_Handle_Button(GPIO_Pin))
    {
        return;
    }
	switch(GPIO_Pin)
	{
	case SW_SELECT_Pin:
		Update_Select_Action();
		break;
	case SW_DOWN_Pin:
		Update_Down_Action();
		break;
	case SW_UP_Pin:
		Update_Up_Action();
		break;
	case SW_LEFT_Pin:
		Update_Left_Action();
		break;
	case SW_RIGHT_Pin:
		Update_Right_Action();
		break;
	case SW_SOS_Pin:
		SOS_Action();
		UI_state = SOS_Screen;
		break;
	default: break;
	}
}

void SOS_Action()
{
	tx_lora_frame.cmd_lora = CMD_LORA_SOS;

}
