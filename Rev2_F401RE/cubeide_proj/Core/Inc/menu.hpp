/*
 * menu.hpp
 *
 * This header and associated menu.c file will be the middle man between the application FW and the LCD, providing
 * a layer of abstraction to menu commands (i.e. up/down/enter/back). The programmer then connects user input (buttons, keyboard)
 * from main or some other module to this one through the MenuCmd API
 * i.e., if in the menu state for the tesla coil case, main() will have to interpret MidiMessages from the piano
 * and convert them to MenuCmds.
 *
 *  Created on: Aug 15, 2025
 *      Author: bucht
 */

#ifndef INC_MENU_H_
#define INC_MENU_H_
#include <string>
#include <functional>

/**
 * Define command types from main to menu
 */
typedef enum{
	E_MENU_UP = 0,
	E_MENU_DOWN,
	E_MENU_RIGHT,
	E_MENU_LEFT,
	E_MENU_ENTER,
	E_MENU_BACK
}MenuCmd;

/**
 * Define response types from menu to main
 */
typedef enum{
	E_MENU_RSP_NA = 0, // Stay in the menu choosing state
	E_MENU_RSP_START_KEYBOARD, // Go to the keyboard
	E_MENU_RSP_START_SD
}MenuRspType;

typedef struct{
	HAL_StatusTypeDef stat;
	MenuRspType type;
	const char *p_data; // i.e., filename to play
}MenuRsp_t;

/**
 * Initialize globals
 */
HAL_StatusTypeDef init_menu();

HAL_StatusTypeDef menu_start_cooldown_msg();

HAL_StatusTypeDef menu_update_cooldown_msg(uint16_t time_left);

HAL_StatusTypeDef menu_start_keyboard_msg();

HAL_StatusTypeDef menu_update_keyboard_msg(uint16_t time_left);

/**
 * Top-level state machine that holds the menu's state
 */
MenuRsp_t menu_callback(MenuCmd *cmd);

class Menu; 

class MenuItem {
public:
	using Action = void(*)();

 	std::string name;
	Menu* submenu;
 	Action action;

 	MenuItem(const std::string& n, Action a = nullptr, Menu* subm = nullptr)
		: name(name), action(a), submenu(subm){}

	bool hasAction() const { return action != nullptr; }
    bool hasSubmenu() const { return submenu != nullptr; }
    void trigger() { if (action) action(); }

};

class Menu {
public:
	std::string title;
//	std::vector<MenuItem> items;
	Menu* parent; // if submenu

	Menu(const std::string& t, Menu* p = nullptr)
        : title(t), parent(p) {}

//    void addItem(const MenuItem& item) { items.push_back(item); }
};

#endif /* INC_MENU_H_ */
