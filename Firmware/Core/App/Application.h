/*
 * Application.h
 *
 *  Created on: May 1, 2026
 *      Author: THANHLT
 */

#ifndef APP_APPLICATION_H_
#define APP_APPLICATION_H_

#include <stdio.h>
#include <stdint.h>

void App_Compass();
void App_Weather();
void App_CurrentTime();
void App_UI();
void App_GPS();

void Calib_Compass();
void Finish_Calib_Compass();

void App_Trekking_Init();
void BME280_Init();

extern uint8_t DS3231_hour;
extern uint8_t DS3231_minute;
extern uint8_t DS3231_second;
extern uint8_t DS3231_day;
extern uint8_t DS3231_date;
extern uint8_t DS3231_month;
extern uint8_t DS3231_year;

extern double accel_g[3];
extern double roll;
extern double pitch;
extern double heading;

typedef struct {
    int16_t x_min;
    int16_t x_max;

    int16_t y_min;
    int16_t y_max;

    float x_offset;
    float y_offset;
    float z_offset;

    float x_scale;
    float y_scale;
    float z_scale;

    uint8_t calibrated;
}MagCalib_t;




#endif /* APP_APPLICATION_H_ */
