#ifndef MAIN_H
#define	MAIN_H


#include "clcd.h"
#include "adc.h"
#include"i2c.h"
#include "digital_keypad.h"
#include "ds1307.h"
#include<xc.h>
#include "car_black_box.h"
#include<string.h>
#include "eeprom.h"
#include "timers.h"
#include "uart.h"

// macros for car black box reset_flag and control_flag
#define LOGIN_SUCCESS           0x01
#define RETURN_BACK             0x02
#define TASK_SUCCESS            0x11
#define TASK_FAIL               0x22
#define DASHBOARD_SCREEN        0x01
#define LOGIN_SCREEN              0x02
#define MAIN_MENU_SCREEN         0x04
#define VIEW_LOG_FLAG          0x08
#define CLEAR_LOG_FLAG          0x10
#define DOWNLOAD_LOG_FLAG 		0x20
#define SET_TIME_FLAG	 		0x40
#define CHANGE_PASSWORD_FLAG 	0x80
#define RESET_NOTHING	        0x00
#define RESET_PASSWORD          0x01
#define RESET_MENU        0x02
#define RESET_MEMORY            0x04
#define RESET_VIEW_LOG_POS      0x08
#define RESET_TIME				0x10
#define RESET_SPOS              0x41
#define PRINT_UART              0x42

#endif	/* MAIN_H */


