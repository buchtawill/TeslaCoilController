/*
 * menu.c
 *
 *  Created on: Aug 15, 2025
 *      Author: bucht
 */


#include "main.h"
#include "I2C_LCD.h"
#include "stdio.h"
#include "menu.hpp"

static LCD lcd;

/*----------------------------------------------------------------------------/
Misc display mode helper functions
/----------------------------------------------------------------------------*/



HAL_StatusTypeDef menu_start_cooldown_msg(){
    clearDisplay(&lcd);
    LCDPrintAtPos(&lcd, "In Cooldown...", 0, 0);
    LCDPrintAtPos(&lcd, "Time left: ", 0, 1);
    return HAL_OK; // Haven't implemented error checking
}

HAL_StatusTypeDef menu_update_cooldown_msg(uint16_t time_left){
    char buf[8];
    snprintf(buf, 8, "%6u", time_left);
    LCDPrintAtPos(&lcd, buf, 10, 1);
    return HAL_OK; // Haven't implemented error checking
}

HAL_StatusTypeDef menu_start_keyboard_msg(){
    clearDisplay(&lcd);
    LCDPrintAtPos(&lcd, "In Keyboard Mode.", 0, 0);
    LCDPrintAtPos(&lcd, "Time left: ", 0, 1);
    return HAL_OK; // Haven't implemented error checking
}

HAL_StatusTypeDef menu_update_keyboard_msg(uint16_t time_left){
    char buf[8];
    snprintf(buf, 8, "%5u", time_left);
    LCDPrintAtPos(&lcd, buf, 10, 1);
    return HAL_OK; // Haven't implemented error checking
}


/*----------------------------------------------------------------------------/
Menu functionality - project specific.

Mode choose
 - SD       --> return E_MENU_RSP_NA
    - Song1 --> return E_MENU_RSP_START_KEYBOARD
    - ...
 - Keyboard --> if enter, return E_MENU_RSP_START_KEYBOARD
 - Settings --> return E_MENU_RSP_NA
    - 
/----------------------------------------------------------------------------*/
typedef enum{
    S_MENU_CHOOSE_MODE = 0,
    S_MENU_CHOOSE_SONG,
    S_MENU_CHOOSE_SETTING
}MenuState;

MenuState mstate;

HAL_StatusTypeDef init_menu(){

    mstate = S_MENU_CHOOSE_MODE;


    initLCD(&lcd, &hi2c2, MAX_ROW_LCD, 20, 0x27, 1);
	LCDCursorOffBlinkOff(&lcd);
	LCDPrintAtPos(&lcd, "Hello! Use the piano", 0, 0);
	LCDPrintAtPos(&lcd, "keys to navigate.", 0, 1);
    return HAL_OK; // Haven't implemented error checking
}

MenuRsp_t update_menu(MenuCmd cmd){

    
}