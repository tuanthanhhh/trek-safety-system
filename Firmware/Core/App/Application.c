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


BME280_Data_t BME280;
uint8_t DS3231_hour;
uint8_t DS3231_minute;
uint8_t DS3231_second;
uint8_t DS3231_day;
uint8_t DS3231_date;
uint8_t DS3231_month;
uint8_t DS3231_year;

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

//	float mx = ((float)mag_data[0] + 235.5f) * 1.249f;
//	float my = ((float)mag_data[1] + 108.0f) * 0.834f;
//	float mz = ((float)mag_data[2] - mag_calib.z_offset)*mag_calib.z_scale;

	float mx = ((float)mag_data[0] - mag_calib.x_offset)*mag_calib.x_scale;
	float my = ((float)mag_data[1] - mag_calib.y_offset)*mag_calib.y_scale;
	float mz = ((float)mag_data[2] - mag_calib.z_offset)*mag_calib.z_scale;
	pitch = atan2f(-ax, sqrtf(ay * ay + az * az));
	roll = atan2f(ay,az);

	float x_heading = mx*cosf(pitch) + my*sinf(roll)*sinf(pitch) - mz*cosf(roll)*sinf(pitch);
	float y_heading = my * cosf(roll) + mz*sinf(roll);

	heading = atan2f(my,mx)* 57.2957795f;

	if(heading < 0.0f)
	{
		heading += 360.0f;
	}

	char buffer[20];
	SH1106_GotoXY(1, 0);
	sprintf(buffer, "Heading: %5.1f", heading);
	SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
	SH1106_GotoXY(1, 10);
	sprintf(buffer, "%.1f,%.1f", mx,my);
	SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
	UI_state = Digital_Compass_Screen;
	SH1106_GotoXY(1, 21);
	sprintf(buffer, "%.1f", accel_data[0]);
	SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
	UI_state = Digital_Compass_Screen;
	SH1106_GotoXY(1, 32);
	sprintf(buffer, "%.1f", accel_data[1]);
	SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
	UI_state = Digital_Compass_Screen;
	SH1106_GotoXY(1, 43);
	sprintf(buffer, "%.1f", accel_data[2]);
	SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
	UI_state = Digital_Compass_Screen;
	SH1106_UpdateScreen();
}
void App_Weather()
{
	  BME280Calculation(&BME280);
	  char buffer[50];
}
void App_GPS()
{
	GPS_Process_Loop();
	if (current_gps.is_valid)
	{
		char buffer[20];
//		tx_lora_frame.gps.lat = current_gps.latitude;
//		tx_lora_frame.gps.lng = current_gps.longitude;

		sprintf(buffer,"%f",current_gps.latitude);
		SH1106_GotoXY(1, 0);
		SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
		sprintf(buffer,"%f",current_gps.longitude);
		SH1106_GotoXY(1, 12);
		SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
		SH1106_UpdateScreen();
	}
	else
	{
		SH1106_GotoXY(1, 0);
		SH1106_Puts("Waiting ...", &Font_7x10, SH1106_COLOR_WHITE);
		SH1106_UpdateScreen();
	}
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
	char buffer[50];
	sprintf(buffer, "T: %.1f C, H: %.1f %, P: %.1f", BME280.Temperature,BME280.Humidity,BME280.Pressure);
	SH1106_GotoXY(1, 0);
	SH1106_Puts(buffer, &Font_7x10, 1);

	sprintf(buffer, "%d:%d:%d", DS3231_hour,DS3231_minute,DS3231_second);
	SH1106_GotoXY(1, 12);
	SH1106_Puts(buffer, &Font_7x10, 1);

	snprintf(buffer, sizeof(buffer), "%.2f:%.2f:%.2f",accel_g[0], accel_g[1], accel_g[2]);
	SH1106_GotoXY(1, 24);
	SH1106_Puts(buffer, &Font_7x10, 1);

	snprintf(buffer, sizeof(buffer), "%f",heading);
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
	mag_calib.x_offset = (mag_calib.x_min + mag_calib.x_min) /2.0f;
	mag_calib.y_offset = (mag_calib.y_min + mag_calib.y_min) /2.0f;

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
	default: break;
	}
}
