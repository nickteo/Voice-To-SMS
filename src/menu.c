#include <pebble.h>
#include "menu.h"

static Window *s_main_window;
static SimpleMenuLayer *s_simple_menu_layer;
static TextLayer *s_list_message_layer;

// Menu stuff
static SimpleMenuSection s_menu_sections[1];
static SimpleMenuItem s_first_menu_items[3];

// Choose the phone number to send back to the phone
static void menu_select_callback(int index, void *ctx) {

}

// Add menu item in location in s_first_menu_items
static void add_to_menu_items(char* name, char* number, int location) {
  s_first_menu_items[location] = (SimpleMenuItem) {
    .title = name,
    .subtitle = number,
    .callback = menu_select_callback,
  };
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_main_window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Although we already defined NUM_FIRST_MENU_ITEMS, you can define
  // an int as such to easily change the order of menu items later
  int num_a_items = 3;
  const char *a[num_a_items];
  a[0] = "blah";
  a[1] = "blah blah";
  a[2] = "blah blah blah";
  
  const char *b[num_a_items];
  b[0] = "777-777-7777";
  b[1] = "111-111-1111";
  b[2] = "121-111-1111";

  for (int i = 0; i < num_a_items; i++) {
    add_to_menu_items((char*) a[i], (char*) b[i], i);
  }

  s_menu_sections[0] = (SimpleMenuSection) {
    .num_items = num_a_items,
    .items = s_first_menu_items,
  };

  s_simple_menu_layer = simple_menu_layer_create(window_bounds, s_main_window, s_menu_sections, 1, NULL);
  
  layer_add_child(window_layer, simple_menu_layer_get_layer(s_simple_menu_layer));
}

static void window_unload(Window *window) {
  simple_menu_layer_destroy(s_simple_menu_layer);
  text_layer_destroy(s_list_message_layer);

  window_destroy(window);
  s_main_window = NULL;
}

void up_click_handler() {
  if(!s_main_window) {
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
  }
  window_stack_push(s_main_window, true);
}
