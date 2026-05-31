/*
 * UI.h
 *
 *  Created on: May 16, 2026
 *      Author: THANHLT
 */

#ifndef APP_UI_H_
#define APP_UI_H_
#include <stdint.h>

#define UPDATE_SCREEN_TIME		100

void Update_UI_Action(uint8_t action);
void Update_Up_Action();
void Update_Down_Action();
void Update_Left_Action();
void Update_Right_Action();
void Update_Select_Action();
void Update_Main_Screen();

void SOS_UI();

void Settings_None_UI();
void Settings_Hour_UI();
void Settings_Minute_UI();
void Settings_Second_UI();
void Settings_Day_UI();
void Settings_Date_UI();
void Settings_Month_UI();
void Settings_Year_UI();

void Menu_UI();
void Menu_Compass_UI();
void Menu_Settings_UI();
void Menu_GPS_UI();
void Compass_UI();
void UI_DrawCompassRotateDial(float heading);
const char* CompassDirection(float heading);

typedef enum{
	Main_Screen,
	Menu_Screen_Compass,
	Menu_Screen_Settings,
	Menu_Screen_GPS,
	Digital_Compass_Screen,
	Digital_Compass_Calib_Screen,
	GPS_Screen,
	Settings_Time_Screen,
	SOS_Screen
}UI_State_t;

extern UI_State_t UI_state;
extern UI_State_t UI_state_old;
typedef enum Button_Action{
	Button_Up,
	Button_Down,
	Button_Right,
	Button_Left,
	Button_Select
}Button_Action_t;

typedef enum{
	Settings_Hour_Index,
	Settings_Minute_Index,
	Settings_Second_Index,
	Settings_Day_Index,
	Settings_Date_Index,
	Settings_Month_Index,
	Settings_Year_Index,
	Settings_None_Index
}Settings_Index_t;

extern Settings_Index_t Settings_Index;
extern float battery_percent;
#endif /* APP_UI_H_ */
