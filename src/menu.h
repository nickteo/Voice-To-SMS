#pragma once

#include <pebble.h>

#define LIST_MESSAGE_WINDOW_NUM_ROWS    5
#define LIST_MESSAGE_WINDOW_CELL_HEIGHT 30
#define LIST_MESSAGE_WINDOW_MENU_HEIGHT \
    LIST_MESSAGE_WINDOW_NUM_ROWS * LIST_MESSAGE_WINDOW_CELL_HEIGHT
#define LIST_MESSAGE_WINDOW_HINT_TEXT   "Your list items"

void add_all_to_menu_items(char** name, char** number, int numTotal);
void up_click_handler();