#include <pebble.h>
#include "menu.h"
#include "messaging.h"
#include "globals.h"

char* initialString = "Press up to see your favorites then choose a contact\n Press select and speak your mind!";
  
static Window *s_main_window;
static TextLayer *s_output_layer;
static int needContactsFlag;
static int needCreateMenuFlag;

static DictationSession *s_dictation_session;

// Declare a buffer for the DictationSession
static char s_last_text[512];

// Dictation Callback
static void dictation_session_callback(DictationSession *session, DictationSessionStatus status, 
                                       char *transcription, void *context) {
  if(status == DictationSessionStatusSuccess) {
    // Display the dictated text
    snprintf(s_last_text, sizeof(s_last_text), "Transcription:\n\n%s", transcription);
    text_layer_set_text(s_output_layer, s_last_text);
    
    // Send the dictated text to the phone
    sendText((char*) selectedNumber, transcription);
  } else {
    // Display the reason for any error
    static char s_failed_buff[128];
    snprintf(s_failed_buff, sizeof(s_failed_buff), "Transcription failed.\n\nReason:\n%d", 
             (int)status);
    text_layer_set_text(s_output_layer, s_failed_buff);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////
// App Message Handlers //////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void outbox_sent_callback(DictionaryIterator *iter, void *context) {
  // Ready for next command
  text_layer_set_text(s_output_layer, initialString);
}

static void outbox_failed_callback(DictionaryIterator *iter, AppMessageResult reason, void *context) {
  text_layer_set_text(s_output_layer, "Send failed!");
  APP_LOG(APP_LOG_LEVEL_ERROR, "Fail reason: %d", (int)reason);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  int numContacts;
  char *name;
  char *phoneNumber;
  int contactKey;
  // Get the first pair
  Tuple *data = dict_find(iterator, KEY_NUM_CONTACTS);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Message");
  if (data) {
    numContacts =  data -> value -> int32;
    // Start the new list
    create_new_menu_list(numContacts);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "numContacts: %d", numContacts);
    for (int i = 0; i < numContacts; i++) {
      contactKey = i * 2 + KEY_FIRST_CONTACT;
      data = dict_find(iterator, contactKey);
      name = data -> value -> cstring;
      data = dict_find(iterator, contactKey + 1);
      phoneNumber = data -> value -> cstring;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Name; %s Number: %s", name, phoneNumber);
      add_to_menu_items(name, phoneNumber, i);
    }
    if (needCreateMenuFlag) {
      createMenu();
    }
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

//////////////////////////////////////////////////////////////////////////////////////////
// Window Loading Stuffs /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  s_output_layer = text_layer_create(GRect(0, 0, window_bounds.size.w, window_bounds.size.h));
  text_layer_set_text(s_output_layer, initialString);
  text_layer_set_text_alignment(s_output_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_output_layer, GTextOverflowModeWordWrap);
  //text_layer_set_background_color(s_output_layer, GColorBlue);
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_output_layer);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Click Handlers ////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Start dictation UI
  dictation_session_start(s_dictation_session);
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Ask for list of contacts
  sendString(KEY_NEED_CONTACTS, "");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (needContactsFlag) {
    sendString(KEY_NEED_CONTACTS, "");
    needContactsFlag = 0;
    needCreateMenuFlag = 1;
  } else {
    createMenu();
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, select_long_click_handler, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Initialization and Deinit /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void init() {
  // Create the window
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  // Create new dictation session
  s_dictation_session = dictation_session_create(sizeof(s_last_text), 
                                               dictation_session_callback, NULL);
  // Register handler
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_stack_push(s_main_window, true);
  
  // Open AppMessage
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  // Ask for list of contacts
  sendString(KEY_NEED_CONTACTS, "");
  needContactsFlag = 1;
}

static void deinit(void) {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}