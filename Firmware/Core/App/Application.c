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
#include "bitmap.h"
#include "main.h"
#include "Application.h"
#include "math.h"


BME280_Data_t BME280;
uint8_t DS3231_hour;
uint8_t DS3231_minute;
uint8_t DS3231_second;

double accel_g[3];
double roll;
double pitch;
double heading;

void App_Trekking_Init()
{
	SH1106_Init();
	DS3231_Init(&hi2c2);
	HAL_Delay(100);
	LSM303DLHC_AccInit(LSM303DLHC_ODR_100_HZ | 7);
	HAL_Delay(100);
	LSM303DLHC_MagInit();
	HAL_Delay(100);
}

void App_Compass()
{
	int16_t accel_data[3];
	LSM303DLHC_AccReadXYZ(accel_data);
	for(int i = 0; i < 3; i++)
	{
		accel_g[i] = accel_data[i] / 1000.0f; // mg → g
	}
	int16_t mag_data[3];
	LSM303DLHC_MagReadXYZ(mag_data);
    double ax = accel_g[0];
    double ay = accel_g[1];
    double az = accel_g[2];
    double norm = sqrtf(ax*ax + ay*ay + az*az);

    ax /= norm;
    ay /= norm;
    az /= norm;

    double mx = (double)mag_data[0];
    double my = (double)mag_data[1];
    double mz = (double)mag_data[2];

    roll  = atan2(ay, az);
    pitch = atan2(-ax, sqrt(ay*ay + az*az));

    double XH = mx*cosf(pitch) + mz*sinf(pitch);
    double YH = mx*sinf(roll)*sinf(pitch) + my*cosf(roll) - mz*sinf(roll)*cosf(pitch);
    heading = atan2f(YH,XH) * 180.0f / M_PI;

    if (heading < 0)
        heading += 360.0f;
}
void App_Weather()
{
	  BME280Calculation(&BME280);
	  char buffer[50];
}
void App_CurrentTime()
{
	char buffer[50];
	DS3231_hour = DS3231_GetHour();
	DS3231_minute = DS3231_GetMinute();
	DS3231_second = DS3231_GetSecond();

}
void App_UI()
{
	char buffer[50];
	sprintf(buffer, "T: %.1f C, H: %.1f %, P: %.1f", BME280.Temperature,BME280.Humidity,BME280.Pressure);
	SH1106_Clear();
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

