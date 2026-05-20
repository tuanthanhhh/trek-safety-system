/*
 * UI.c
 *
 *  Created on: May 16, 2026
 *      Author: THANHLT
 */

#include "UI.h"
#include "SH1106.h"
#include "bitmap.h"
#include "Application.h"
#include "fonts.h"
#include "DS3231.h"

UI_State_t UI_state = Main_Screen;
const char *date_str[7] = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
const char *month_str[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
Settings_Index_t Settings_Index = Settings_None_Index;
uint8_t hour_temp, minute_temp, second_temp;
void Update_UI_Action(uint8_t action)
{
	switch(action)
	{
	case Button_Up:
		Update_Up_Action();
		break;
	case Button_Down:
		Update_Down_Action();
		break;
	case Button_Left:
		Update_Left_Action();
		break;
	case Button_Right:
		Update_Right_Action();
		break;
	case Button_Select:
		break;

	default:
	}
}

void Update_Up_Action()
{
	char buffer[20];
	switch(UI_state)
	{
	case Menu_Screen_Settings:
		SH1106_DrawBitmap(2, 0, selected_frame, SELECT_FRAME_WIDTH, SELECT_FRAME_HEIGHT, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(2, 22, selected_frame, SELECT_FRAME_WIDTH, SELECT_FRAME_HEIGHT, SH1106_COLOR_BLACK);
		UI_state = Menu_Screen_Compass;
		break;
	case Menu_Screen_GPS:
		SH1106_DrawBitmap(2, 22, selected_frame, SELECT_FRAME_WIDTH, SELECT_FRAME_HEIGHT, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(2, 43, selected_frame, SELECT_FRAME_WIDTH, SELECT_FRAME_HEIGHT, SH1106_COLOR_BLACK);
		UI_state = Menu_Screen_Settings;
		break;
	case Settings_Time_Screen:
		switch(Settings_Index)
		{
		case Settings_Hour_Index:
			if(hour_temp == 24)
			{
				hour_temp = 0;
			}
			else
			{
				hour_temp++;
			}
			sprintf(buffer, "%02d", hour_temp);
			SH1106_GotoXY(42, 0);
			SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
			SH1106_UpdateScreen();
			break;
		case Settings_Minute_Index:
			if(minute_temp == 60)
			{
				minute_temp = 0;
			}
			else
			{
				minute_temp++;
			}
			sprintf(buffer, "%02d", minute_temp);
			SH1106_GotoXY(63, 0);
			SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
			SH1106_UpdateScreen();
			break;
		case Settings_Second_Index:
			if(second_temp == 60)
			{
				second_temp = 0;
			}
			else
			{
				second_temp++;
			}
			sprintf(buffer, "%02d", second_temp);
			SH1106_GotoXY(84, 0);
			SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
			SH1106_UpdateScreen();
			break;
		default: break;
		}
	default: break;
	}
	SH1106_UpdateScreen();
}
void Update_Down_Action()
{
	char buffer[20];
	switch(UI_state)
	{
	case Menu_Screen_Compass:
		SH1106_DrawBitmap(2, 0, selected_frame, SELECT_FRAME_WIDTH, SELECT_FRAME_HEIGHT, SH1106_COLOR_BLACK);
		SH1106_DrawBitmap(2, 22, selected_frame, SELECT_FRAME_WIDTH, SELECT_FRAME_HEIGHT, SH1106_COLOR_WHITE);
		UI_state = Menu_Screen_Settings;
		break;
	case Menu_Screen_Settings:
		SH1106_DrawBitmap(2, 22, selected_frame, SELECT_FRAME_WIDTH, SELECT_FRAME_HEIGHT, SH1106_COLOR_BLACK);
		SH1106_DrawBitmap(2, 43, selected_frame, SELECT_FRAME_WIDTH, SELECT_FRAME_HEIGHT, SH1106_COLOR_WHITE);
		UI_state = Menu_Screen_GPS;
		break;
	case Settings_Time_Screen:
		switch(Settings_Index)
		{
		case Settings_Hour_Index:
			if(hour_temp == 0)
			{
				hour_temp = 24;
			}
			else
			{
				hour_temp--;
			}
			sprintf(buffer, "%02d", hour_temp);
			SH1106_GotoXY(42, 0);
			SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
			SH1106_UpdateScreen();
			break;
		case Settings_Minute_Index:
			if(minute_temp == 0)
			{
				minute_temp = 60;
			}
			else
			{
				minute_temp--;
			}
			sprintf(buffer, "%02d", minute_temp);
			SH1106_GotoXY(63, 0);
			SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
			SH1106_UpdateScreen();
			break;
		case Settings_Second_Index:
			if(second_temp == 0)
			{
				second_temp = 60;
			}
			else
			{
				second_temp--;
			}
			sprintf(buffer, "%02d", second_temp);
			SH1106_GotoXY(84, 0);
			SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
			SH1106_UpdateScreen();
			break;
		default: break;
		}
		break;
	default: break;
	}
	SH1106_UpdateScreen();
}
void Update_Left_Action()
{
	switch(UI_state)
	{
	case Main_Screen:
		UI_state = Digital_Compass_Calib_Screen;
		break;
	case Digital_Compass_Screen:
	case Menu_Screen_Compass:
	case Menu_Screen_GPS:
	case Menu_Screen_Settings:
		SH1106_Clear();
		Update_Main_Screen();
		UI_state = Main_Screen;
		break;
	case Settings_Time_Screen:
		switch(Settings_Index)
		{
		case Settings_Minute_Index:
			Settings_Hour_UI();
			break;
		case Settings_Hour_Index:
			Settings_None_UI();
			break;
		case Settings_None_Index:
			Menu_UI();
		case Settings_Second_Index:
			Settings_Minute_UI();
			break;
		}
		default: break;
	}
}
void Update_Right_Action()
{
	switch(UI_state)
	{
	case Digital_Compass_Calib_Screen:
		Finish_Calib_Compass();
		UI_state = Main_Screen;
		break;
	case Settings_Time_Screen:
		switch(Settings_Index)
		{
		case Settings_None_Index:
			Settings_Hour_UI();
			break;
		case Settings_Hour_Index:
			Settings_Minute_UI();
			break;
		case Settings_Minute_Index:
			Settings_Second_UI();
			break;
		default: break;
		}
		break;
	}
}
void Update_Select_Action()
{
	switch(UI_state)
	{
	case Main_Screen:
		Menu_UI();
		break;
	case Menu_Screen_Settings:
		Settings_None_UI();
		break;
	case Menu_Screen_Compass:
		SH1106_Clear();
		Compass_UI();
		break;
	default: break;
	}
}

void Update_Main_Screen()
{
	char buffer[20];
	SH1106_DrawBitmap(112, 1, icon_battery, 13, 8, SH1106_COLOR_WHITE);
	sprintf(buffer, "%s,%02d %s",date_str[DS3231_day],DS3231_date,month_str[DS3231_month]);
	SH1106_GotoXY((128-strlen(buffer)*7)/2, 30);
	SH1106_Puts(buffer, &Font_7x10, 1);
	sprintf(buffer, "%02d:%02d", DS3231_hour,DS3231_minute);
	SH1106_GotoXY((128-strlen(buffer)*11)/2, 44);
	SH1106_Puts(buffer, &Font_11x18, 1);
	SH1106_UpdateScreen();
}

void Settings_None_UI()
{
	char buffer[20];
	if(Settings_Index != Settings_None_Index)
	{
		SH1106_GotoXY(1, 0);
		DS3231_SetHour(hour_temp);
		DS3231_SetMinute(minute_temp);
		DS3231_SetSecond(second_temp);
		sprintf(buffer, "Time: %02d:%02d:%02d", hour_temp, minute_temp, second_temp);
		SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
		SH1106_DrawLine(42, 11, 56, 11, SH1106_COLOR_BLACK);
	}
	else
	{
		hour_temp = DS3231_hour;
		minute_temp = DS3231_minute;
		second_temp = DS3231_second;
		SH1106_Clear();
		SH1106_GotoXY(1, 0);
		sprintf(buffer, "Time: %02d:%02d:%02d", hour_temp, minute_temp, second_temp);
		SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
		UI_state = Settings_Time_Screen;
	}
	Settings_Index = Settings_None_Index;
	SH1106_UpdateScreen();

}

void Settings_Hour_UI()
{
	SH1106_DrawLine(42, 11, 56, 11, SH1106_COLOR_WHITE);
	SH1106_DrawLine(63, 11, 77, 11, SH1106_COLOR_BLACK);
	SH1106_UpdateScreen();
	Settings_Index = Settings_Hour_Index;
}
void Settings_Minute_UI()
{
	SH1106_DrawLine(42, 11, 56, 11, SH1106_COLOR_BLACK);
	SH1106_DrawLine(63, 11, 77, 11, SH1106_COLOR_WHITE);
	SH1106_DrawLine(84, 11, 98, 11, SH1106_COLOR_BLACK);
	SH1106_UpdateScreen();
	Settings_Index = Settings_Minute_Index;
}
void Settings_Second_UI()
{
	SH1106_DrawLine(63, 11, 77, 11, SH1106_COLOR_BLACK);
	SH1106_DrawLine(84, 11, 98, 11, SH1106_COLOR_WHITE);
	SH1106_UpdateScreen();
	Settings_Index = Settings_Second_Index;
}

void Menu_UI()
{
	SH1106_Clear();
	SH1106_DrawBitmap(7, 2, icon_compass, 16, 16, SH1106_COLOR_WHITE);
	SH1106_DrawBitmap(7, 24, icon_settings, 16, 16, SH1106_COLOR_WHITE);
	SH1106_DrawBitmap(7, 45, icon_GPS, 16, 16, SH1106_COLOR_WHITE);
	SH1106_DrawBitmap(30, 7, Compass, 55 , 11, SH1106_COLOR_WHITE);
	SH1106_DrawBitmap(30, 28, Settings, 58 , 11, SH1106_COLOR_WHITE);
	SH1106_DrawBitmap(30, 50, GPS, 22 , 9, SH1106_COLOR_WHITE);
	SH1106_DrawBitmap(2, 0, selected_frame, SELECT_FRAME_WIDTH, SELECT_FRAME_HEIGHT, SH1106_COLOR_WHITE);
	SH1106_UpdateScreen();
	UI_state = Menu_Screen_Compass;
}

void Compass_UI()
{
	App_Compass();
//	char buffer[20];
//	SH1106_GotoXY(1, 0);
//	sprintf(buffer, "Heading: %5.1f", heading);
//	SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
//	UI_state = Digital_Compass_Screen;
//	SH1106_UpdateScreen();
}
