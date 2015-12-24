#include <pebble.h>
#include "menu.h"

static Window *s_main_window;
static SimpleMenuLayer *s_simple_menu_layer;
static TextLayer *s_list_message_layer;

// Menu stuff
static SimpleMenuSection s_menu_sections[1];
static SimpleMenuItem* s_first_menu_items;
static int numItems;

// Choose the phone number to send back to the phone
static void menu_select_callback(int index, void *ctx) {

}

// Add menu item in location in s_first_menu_items
void add_to_menu_items(char* name, char* number, int location) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "location: %d", location);
  s_first_menu_items[location] = (SimpleMenuItem) {
    .title = name,
    .subtitle = number,
    .callback = menu_select_callback,
  };
}

// Add all menu items
void add_all_to_menu_items(char** name, char** number, int numTotal) {
  if (s_first_menu_items != NULL) {
    free(s_first_menu_items);
  }
  s_first_menu_items = (SimpleMenuItem*) malloc(numTotal * sizeof(SimpleMenuItem));
  for (int i = 0; i < numTotal; i++) {
    add_to_menu_items((char*) name[i], (char*) number[i], i);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Name; %s Number: %s", name[i], number[i]);
  }
  numItems = numTotal;
  if (!!s_simple_menu_layer) {
    layer_mark_dirty((Layer*) s_simple_menu_layer);
  }
}

// Initialize menu items
void create_new_menu_list(int numTotal) {
  if (s_first_menu_items != NULL) {
    free(s_first_menu_items);
  }
  s_first_menu_items = (SimpleMenuItem*) malloc(numTotal * sizeof(SimpleMenuItem));
  numItems = numTotal;
}

// Mark layer as dirty
void mark_dirty() {
  if (!!s_simple_menu_layer) {
    layer_mark_dirty((Layer*) s_simple_menu_layer);
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_main_window);
  GRect window_bounds = layer_get_bounds(window_layer);

  s_menu_sections[0] = (SimpleMenuSection) {
    .num_items = numItems,
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
