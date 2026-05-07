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

void App_Trekking_Init();

extern BME280_Data_t BME280;

extern uint8_t DS3231_hour;
extern uint8_t DS3231_minute;
extern uint8_t DS3231_second;

extern double accel_g[3];
extern double roll;
extern double pitch;
extern double heading;


typedef enum UI_State {
	Main_Screen,
	Menu_Screen_1,
	Menu_Screen_2,
	Digital_Compass_Screen,
	Weather_Screen,
	GPS_Screen
};

#endif /* APP_APPLICATION_H_ */
