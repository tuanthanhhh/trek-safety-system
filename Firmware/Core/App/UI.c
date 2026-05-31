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
#include <math.h>
#include <stdio.h>
#include "slave_lora.h"
#include "BME280.h"
#include "stm32f4xx_it.h"
#define DEG_TO_RAD 0.0174532925f


UI_State_t UI_state = Main_Screen;
UI_State_t UI_state_old = Main_Screen;
const char *day_str[7] = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
const char *month_str[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
Settings_Index_t Settings_Index = Settings_None_Index;
uint8_t hour_temp, minute_temp, second_temp;
uint8_t date_temp, day_temp, month_temp;
uint16_t year_temp;
float battery_percent;
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
		UI_state = Menu_Screen_Compass;
		break;
	case Menu_Screen_GPS:
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
			break;
		case Settings_Day_Index:
			if(day_temp == 6)
			{
				day_temp = 0;
			}
			else
			{
				day_temp++;
			}
			sprintf(buffer, "Day: %s", day_str[day_temp]);
			SH1106_GotoXY(1, 12);
			SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
			break;
		case Settings_Date_Index:
			if(date_temp >= 31)
			{
				date_temp = 1;
			}
			else
			{
				date_temp++;
			}
			sprintf(buffer, "%02d", date_temp);
			SH1106_GotoXY(42, 25);
			SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
			break;
		case Settings_Month_Index:
			if(month_temp >= 12)
			{
				month_temp = 1;
			}
			else
			{
				month_temp++;
			}
			sprintf(buffer, "%02d", month_temp);
			SH1106_GotoXY(63, 25);
			SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
			break;
		case Settings_Year_Index:
			if(year_temp < 9999)
			{
				year_temp++;
			}
			sprintf(buffer, "%04d", year_temp);
			SH1106_GotoXY(84, 25);
			SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
			break;
		default: break;
		}
		SH1106_UpdateScreen();
		break;
	default: break;
	}
}
void Update_Down_Action()
{
	char buffer[20];
	switch(UI_state)
	{
	case Menu_Screen_Compass:
		UI_state = Menu_Screen_Settings;
		break;
	case Menu_Screen_Settings:
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
			break;
		case Settings_Day_Index:
			if(day_temp == 0)
			{
				day_temp = 6;
			}
			else
			{
				day_temp--;
			}
			sprintf(buffer, "Day: %s", day_str[day_temp]);
			SH1106_GotoXY(1, 12);
			SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
			break;
		case Settings_Date_Index:
			if(date_temp <= 1)
			{
				date_temp = 31;
			}
			else
			{
				date_temp--;
			}
			sprintf(buffer, "%02d", date_temp);
			SH1106_GotoXY(42, 25);
			SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
			break;
		case Settings_Month_Index:
			if(month_temp <= 1)
			{
				month_temp = 12;
			}
			else
			{
				month_temp--;
			}
			sprintf(buffer, "%02d", month_temp);
			SH1106_GotoXY(63, 25);
			SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
			break;
		case Settings_Year_Index:
			if(year_temp > 0)
			{
				year_temp--;
			}
			sprintf(buffer, "%04d", year_temp);
			SH1106_GotoXY(84, 25);
			SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
			break;
		default: break;
		}
		SH1106_UpdateScreen();
		break;
	default: break;
	}
}
void Update_Left_Action()
{
	switch(UI_state)
	{
	case Digital_Compass_Screen:
		UI_state = Menu_Screen_Compass;
		break;
	case Menu_Screen_Compass:
	case Menu_Screen_GPS:
	case Menu_Screen_Settings:
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
			UI_state = Menu_Screen_Settings;
			break;
		case Settings_Second_Index:
			Settings_Minute_UI();
			break;
		case Settings_Day_Index:
			Settings_Second_UI();
			break;
		case Settings_Date_Index:
			Settings_Day_UI();
			break;
		case Settings_Month_Index:
			Settings_Date_UI();
			break;
		case Settings_Year_Index:
			Settings_Month_UI();
			break;
		}
		break;
	case GPS_Screen:
		UI_state = Menu_Screen_GPS;
		break;
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
		case Settings_Second_Index:
			Settings_Day_UI();
			break;
		case Settings_Day_Index:
			Settings_Date_UI();
			break;
		case Settings_Date_Index:
			Settings_Month_UI();
			break;
		case Settings_Month_Index:
			Settings_Year_UI();
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
		UI_state = Menu_Screen_Compass;
		break;
	case Settings_Time_Screen:
		SH1106_Clear();
		Settings_Index = Settings_None_Index;
		Settings_None_UI();
		break;
	case Menu_Screen_Settings:
		SH1106_Clear();
		UI_state = Settings_Time_Screen;

		Settings_None_UI();
		break;
	case Menu_Screen_Compass:
		UI_state = Digital_Compass_Screen;
		break;
	case Menu_Screen_GPS:
		UI_state = GPS_Screen;
		break;
	case SOS_Screen:
		UI_state = Main_Screen;
		tx_lora_frame.cmd_lora = CMD_LORA_TRACKING;
		break;
	default: break;
	}
}

void Update_Main_Screen()
{
	char buffer[20];
	if(UI_state != UI_state_old)
	{
		SH1106_Clear();
		UI_state_old = UI_state;
		HAL_TIM_Base_Stop_IT(&htim1);
	}
	SH1106_DrawBitmap(112, 1, icon_battery, 13, 8, SH1106_COLOR_WHITE);

	if(Voltage > 4.0f)
	{
		battery_percent = 100;
	}
	else if (Voltage < 3.4f)
	{
		battery_percent = 0;
	}
	else
	{
		battery_percent = ( Voltage - 3.4)/0.006;
	}
	if(battery_percent < 10)
	{
		SH1106_DrawBitmap(114, 3, pin_cell, 2, 4, SH1106_COLOR_BLACK);
		SH1106_DrawBitmap(117, 3, pin_cell, 2, 4, SH1106_COLOR_BLACK);
		SH1106_DrawBitmap(120, 3, pin_cell, 2, 4, SH1106_COLOR_BLACK);
	}
	else if(battery_percent < 30)
	{
		SH1106_DrawBitmap(114, 3, pin_cell, 2, 4, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(117, 3, pin_cell, 2, 4, SH1106_COLOR_BLACK);
		SH1106_DrawBitmap(120, 3, pin_cell, 2, 4, SH1106_COLOR_BLACK);
	}
	else if (battery_percent < 60)
	{
		SH1106_DrawBitmap(114, 3, pin_cell, 2, 4, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(117, 3, pin_cell, 2, 4, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(120, 3, pin_cell, 2, 4, SH1106_COLOR_BLACK);
	}
	else
	{
		SH1106_DrawBitmap(114, 3, pin_cell, 2, 4, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(117, 3, pin_cell, 2, 4, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(120, 3, pin_cell, 2, 4, SH1106_COLOR_WHITE);
	}
	sprintf(buffer, "%s,%02d %s",day_str[DS3231_day],DS3231_date,month_str[DS3231_month]);
//	SH1106_GotoXY((128-strlen(buffer)*7)/2, 30);
	SH1106_GotoXY(1, 1);
	SH1106_Puts(buffer, &Font_7x10, 1);
	sprintf(buffer, "%02d:%02d", DS3231_hour,DS3231_minute);
//	SH1106_GotoXY((128-strlen(buffer)*11)/2, 44);
	SH1106_GotoXY(1, 15);
	SH1106_Puts(buffer, &Font_11x18, 1);

	SH1106_GotoXY(1, 40);
	SH1106_Puts("ID:", &Font_7x10, 1);
	sprintf(buffer, "%02d", ID_DEVICE);
	SH1106_GotoXY(1, 52);
	SH1106_Puts(buffer, &Font_7x10, 1);

	sprintf(buffer, "T:%3d~C", (int)BME280.Temperature);
	SH1106_GotoXY(60, 40);
	SH1106_Puts(buffer, &Font_7x10, 1);

	sprintf(buffer, "H:%3d%%", (int)BME280.Humidity);
	SH1106_GotoXY(60, 52);
	SH1106_Puts(buffer, &Font_7x10, 1);

	SH1106_DrawLine(50, 37, 50, 64, SH1106_COLOR_WHITE);
	SH1106_DrawLine(0, 37, 128, 37, SH1106_COLOR_WHITE);

	SH1106_UpdateScreen();
	HAL_Delay(UPDATE_SCREEN_TIME);
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

		DS3231_SetDayOfWeek(day_temp);
		DS3231_SetDate(date_temp);
		DS3231_SetMonth(month_temp);
		DS3231_SetYear(year_temp);

		sprintf(buffer, "Time: %02d:%02d:%02d", hour_temp, minute_temp, second_temp);
		SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
		SH1106_DrawLine(42, 11, 56, 11, SH1106_COLOR_BLACK);

		SH1106_GotoXY(1, 12);
		sprintf(buffer, "Day: %s", day_str[day_temp]);
		SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);

		SH1106_GotoXY(1, 25);
		sprintf(buffer, "Date: %02d/%02d/%04d", date_temp, month_temp, year_temp);
		SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);

	}
	else
	{
		hour_temp = DS3231_hour;
		minute_temp = DS3231_minute;
		second_temp = DS3231_second;

		date_temp = DS3231_date;
		day_temp = DS3231_day;
		month_temp = DS3231_month;
		year_temp = DS3231_year;
		SH1106_Clear();
		SH1106_GotoXY(1, 0);
		sprintf(buffer, "Time: %02d:%02d:%02d", hour_temp, minute_temp, second_temp);
		SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);

		SH1106_GotoXY(1, 12);
		sprintf(buffer, "Day: %s", day_str[day_temp]);
		SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);

		SH1106_GotoXY(1, 25);
		sprintf(buffer, "Date: %02d/%02d/%04d", date_temp, month_temp, year_temp);
		SH1106_Puts(buffer, &Font_7x10, SH1106_COLOR_WHITE);
		UI_state = Settings_Time_Screen;
	}
	Settings_Index = Settings_None_Index;
	SH1106_UpdateScreen();

}

void Settings_Hour_UI()
{
	SH1106_DrawLine(42, 10, 56, 10, SH1106_COLOR_WHITE);
	SH1106_DrawLine(63, 10, 77, 10, SH1106_COLOR_BLACK);
	SH1106_DrawLine(36, 22, 57, 22, SH1106_COLOR_BLACK);
	SH1106_UpdateScreen();
	Settings_Index = Settings_Hour_Index;
}
void Settings_Minute_UI()
{
	SH1106_DrawLine(42, 10, 56, 10, SH1106_COLOR_BLACK);
	SH1106_DrawLine(63, 10, 77, 10, SH1106_COLOR_WHITE);
	SH1106_DrawLine(84, 10, 98, 10, SH1106_COLOR_BLACK);
	SH1106_UpdateScreen();
	Settings_Index = Settings_Minute_Index;
}
void Settings_Second_UI()
{
	SH1106_DrawLine(63, 10, 77, 10, SH1106_COLOR_BLACK);
	SH1106_DrawLine(84, 10, 98, 10, SH1106_COLOR_WHITE);
	SH1106_DrawLine(36, 22, 57, 22, SH1106_COLOR_BLACK);
	SH1106_UpdateScreen();
	Settings_Index = Settings_Second_Index;
}

void Settings_Day_UI()
{
	SH1106_DrawLine(84, 10, 98, 10, SH1106_COLOR_BLACK);
	SH1106_DrawLine(36, 22, 57, 22, SH1106_COLOR_WHITE);
	SH1106_UpdateScreen();
	Settings_Index = Settings_Day_Index;
}

void Settings_Date_UI()
{
	SH1106_DrawLine(36, 22, 57, 22, SH1106_COLOR_BLACK);
	SH1106_DrawLine(42, 35, 56, 35, SH1106_COLOR_WHITE);
	SH1106_DrawLine(63, 35, 77, 35, SH1106_COLOR_BLACK);
	SH1106_UpdateScreen();
	Settings_Index = Settings_Date_Index;
}

void Settings_Month_UI()
{
	SH1106_DrawLine(42, 35, 56, 35, SH1106_COLOR_BLACK);
	SH1106_DrawLine(63, 35, 77, 35, SH1106_COLOR_WHITE);
	SH1106_DrawLine(84, 35, 112, 35, SH1106_COLOR_BLACK);
	SH1106_UpdateScreen();
	Settings_Index = Settings_Month_Index;
}
void Settings_Year_UI()
{
	SH1106_DrawLine(63, 35, 77, 35, SH1106_COLOR_BLACK);
	SH1106_DrawLine(84, 35, 112, 35, SH1106_COLOR_WHITE);
	SH1106_UpdateScreen();
	Settings_Index = Settings_Year_Index;
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
	if(UI_state_old != UI_state)
	{
		SH1106_Clear();
		int cx = 64;
		int cy = 32;
		int r  = 28;

		SH1106_DrawCircle(cx, cy, r, SH1106_COLOR_WHITE);

		SH1106_DrawLine(cx, cy + 8, cx, cy - 18, SH1106_COLOR_WHITE);
		SH1106_DrawLine(cx, cy - 18, cx - 4, cy - 10, SH1106_COLOR_WHITE);
		SH1106_DrawLine(cx, cy - 18, cx + 4, cy - 10, SH1106_COLOR_WHITE);

		SH1106_DrawFilledCircle(cx, cy, 2, SH1106_COLOR_WHITE);
		UI_state_old = UI_state;
	}
	App_Compass();
}

void UI_DrawCompassRotateDial(float heading)
{
    int cx = 64;
    int cy = 32;
    int r  = 28;
    int text_r = r - 8;

    static int old_nx = -100, old_ny = -100;
    static int old_ex = -100, old_ey = -100;
    static int old_sx = -100, old_sy = -100;
    static int old_wx = -100, old_wy = -100;

    float angleN = (0.0f   - heading - 90.0f) * DEG_TO_RAD;
    float angleE = (90.0f  - heading - 90.0f) * DEG_TO_RAD;
    float angleS = (180.0f - heading - 90.0f) * DEG_TO_RAD;
    float angleW = (270.0f - heading - 90.0f) * DEG_TO_RAD;

    int nx = cx + (int)(cosf(angleN) * text_r) - 3;
    int ny = cy + (int)(sinf(angleN) * text_r) - 5;

    int ex = cx + (int)(cosf(angleE) * text_r) - 3;
    int ey = cy + (int)(sinf(angleE) * text_r) - 5;

    int sx = cx + (int)(cosf(angleS) * text_r) - 3;
    int sy = cy + (int)(sinf(angleS) * text_r) - 5;

    int wx = cx + (int)(cosf(angleW) * text_r) - 3;
    int wy = cy + (int)(sinf(angleW) * text_r) - 5;

    SH1106_DrawFilledRectangle(old_nx - 1, old_ny - 1, 9, 12, SH1106_COLOR_BLACK);
    SH1106_DrawFilledRectangle(old_ex - 1, old_ey - 1, 9, 12, SH1106_COLOR_BLACK);
    SH1106_DrawFilledRectangle(old_sx - 1, old_sy - 1, 9, 12, SH1106_COLOR_BLACK);
    SH1106_DrawFilledRectangle(old_wx - 1, old_wy - 1, 9, 12, SH1106_COLOR_BLACK);

    SH1106_DrawCircle(cx, cy, r, SH1106_COLOR_WHITE);

    SH1106_DrawLine(cx, cy + 8, cx, cy - 18, SH1106_COLOR_WHITE);
    SH1106_DrawLine(cx, cy - 18, cx - 4, cy - 10, SH1106_COLOR_WHITE);
    SH1106_DrawLine(cx, cy - 18, cx + 4, cy - 10, SH1106_COLOR_WHITE);
    SH1106_DrawFilledCircle(cx, cy, 2, SH1106_COLOR_WHITE);

    SH1106_GotoXY(nx, ny);
    SH1106_Puts("N", &Font_7x10, SH1106_COLOR_WHITE);

    SH1106_GotoXY(ex, ey);
    SH1106_Puts("E", &Font_7x10, SH1106_COLOR_WHITE);

    SH1106_GotoXY(sx, sy);
    SH1106_Puts("S", &Font_7x10, SH1106_COLOR_WHITE);

    SH1106_GotoXY(wx, wy);
    SH1106_Puts("W", &Font_7x10, SH1106_COLOR_WHITE);

    old_nx = nx;
    old_ny = ny;
    old_ex = ex;
    old_ey = ey;
    old_sx = sx;
    old_sy = sy;
    old_wx = wx;
    old_wy = wy;

    char heading_str[8];

	sprintf(heading_str, "%.1f", heading);

	SH1106_GotoXY(92, 40);
	SH1106_Puts(heading_str, &Font_7x10, SH1106_COLOR_WHITE);

    SH1106_UpdateScreen();
    HAL_Delay(UPDATE_SCREEN_TIME);
}

void SOS_UI()
{
	SH1106_Clear();
	SH1106_DrawBitmap(112, 1, icon_battery, 13, 8, SH1106_COLOR_WHITE);

	if(Voltage > 4.0f)
	{
		battery_percent = 100;
	}
	else if (Voltage < 3.4f)
	{
		battery_percent = 0;
	}
	else
	{
		battery_percent = ( Voltage - 3.4)/0.006;
	}
	if(battery_percent < 10)
	{
		SH1106_DrawBitmap(114, 3, pin_cell, 2, 4, SH1106_COLOR_BLACK);
		SH1106_DrawBitmap(117, 3, pin_cell, 2, 4, SH1106_COLOR_BLACK);
		SH1106_DrawBitmap(120, 3, pin_cell, 2, 4, SH1106_COLOR_BLACK);
	}
	else if(battery_percent < 30)
	{
		SH1106_DrawBitmap(114, 3, pin_cell, 2, 4, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(117, 3, pin_cell, 2, 4, SH1106_COLOR_BLACK);
		SH1106_DrawBitmap(120, 3, pin_cell, 2, 4, SH1106_COLOR_BLACK);
	}
	else if (battery_percent < 60)
	{
		SH1106_DrawBitmap(114, 3, pin_cell, 2, 4, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(117, 3, pin_cell, 2, 4, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(120, 3, pin_cell, 2, 4, SH1106_COLOR_BLACK);
	}
	else
	{
		SH1106_DrawBitmap(114, 3, pin_cell, 2, 4, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(117, 3, pin_cell, 2, 4, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(120, 3, pin_cell, 2, 4, SH1106_COLOR_WHITE);
	}


	SH1106_GotoXY(40, 15);
	SH1106_Puts("SOS", &Font_16x26, 1);

	SH1106_GotoXY(8, 45);
	SH1106_Puts("Press OK to exit", &Font_7x10, 1);

	SH1106_UpdateScreen();
	UI_state = SOS_Screen;
}

const char* CompassDirection(float heading)
{
    if(heading < 22.5f)   return "N";
    if(heading < 67.5f)   return "NE";
    if(heading < 112.5f)  return "E";
    if(heading < 157.5f)  return "SE";
    if(heading < 202.5f)  return "S";
    if(heading < 247.5f)  return "SW";
    if(heading < 292.5f)  return "W";
    if(heading < 337.5f)  return "NW";
    return "N";
}

void Menu_Compass_UI()
{
	switch(UI_state_old)
	{
	case Menu_Screen_Settings:
		SH1106_DrawBitmap(2, 22, selected_frame, SELECT_FRAME_WIDTH, SELECT_FRAME_HEIGHT, SH1106_COLOR_BLACK);
		SH1106_DrawBitmap(2, 0, selected_frame, SELECT_FRAME_WIDTH, SELECT_FRAME_HEIGHT, SH1106_COLOR_WHITE);
		break;

	default:
		SH1106_Clear();
		SH1106_DrawBitmap(7, 2, icon_compass, 16, 16, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(7, 24, icon_settings, 16, 16, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(7, 45, icon_GPS, 16, 16, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(30, 7, Compass, 55 , 11, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(30, 28, Settings, 58 , 11, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(30, 50, GPS, 22 , 9, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(2, 0, selected_frame, SELECT_FRAME_WIDTH, SELECT_FRAME_HEIGHT, SH1106_COLOR_WHITE);
	}
	SH1106_UpdateScreen();
	HAL_Delay(UPDATE_SCREEN_TIME);
	UI_state = Menu_Screen_Compass;
}
void Menu_Settings_UI()
{
	switch(UI_state_old)
	{
	case Menu_Screen_Compass:
		SH1106_DrawBitmap(2, 0, selected_frame, SELECT_FRAME_WIDTH, SELECT_FRAME_HEIGHT, SH1106_COLOR_BLACK);
		SH1106_DrawBitmap(2, 22, selected_frame, SELECT_FRAME_WIDTH, SELECT_FRAME_HEIGHT, SH1106_COLOR_WHITE);
		break;
	case Menu_Screen_GPS:
		SH1106_DrawBitmap(2, 22, selected_frame, SELECT_FRAME_WIDTH, SELECT_FRAME_HEIGHT, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(2, 43, selected_frame, SELECT_FRAME_WIDTH, SELECT_FRAME_HEIGHT, SH1106_COLOR_BLACK);
		break;
	default:
		SH1106_Clear();
		SH1106_DrawBitmap(7, 2, icon_compass, 16, 16, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(7, 24, icon_settings, 16, 16, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(7, 45, icon_GPS, 16, 16, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(30, 7, Compass, 55 , 11, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(30, 28, Settings, 58 , 11, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(30, 50, GPS, 22 , 9, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(2, 22, selected_frame, SELECT_FRAME_WIDTH, SELECT_FRAME_HEIGHT, SH1106_COLOR_WHITE);
		break;
	}

	SH1106_UpdateScreen();
	HAL_Delay(UPDATE_SCREEN_TIME);
}
void Menu_GPS_UI()
{
	switch(UI_state_old)
	{
	case Menu_Screen_Settings:
		SH1106_DrawBitmap(2, 22, selected_frame, SELECT_FRAME_WIDTH, SELECT_FRAME_HEIGHT, SH1106_COLOR_BLACK);
		SH1106_DrawBitmap(2, 43, selected_frame, SELECT_FRAME_WIDTH, SELECT_FRAME_HEIGHT, SH1106_COLOR_WHITE);
		break;
	default:
		SH1106_Clear();
		SH1106_DrawBitmap(7, 2, icon_compass, 16, 16, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(7, 24, icon_settings, 16, 16, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(7, 45, icon_GPS, 16, 16, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(30, 7, Compass, 55 , 11, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(30, 28, Settings, 58 , 11, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(30, 50, GPS, 22 , 9, SH1106_COLOR_WHITE);
		SH1106_DrawBitmap(2, 43, selected_frame, SELECT_FRAME_WIDTH, SELECT_FRAME_HEIGHT, SH1106_COLOR_WHITE);
		break;
	}
	SH1106_UpdateScreen();
	HAL_Delay(UPDATE_SCREEN_TIME);
	UI_state = Menu_Screen_GPS;
}
