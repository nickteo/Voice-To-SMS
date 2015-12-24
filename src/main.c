#include <pebble.h>
#include "menu.h"

#define KEY_PHONE_NUMBER 0
#define KEY_MESSAGE 1
#define KEY_NUM_CONTACTS 2
#define KEY_NEED_CONTACTS 3
#define KEY_FIRST_CONTACT 100

char* initialString = "Speak your mind!";
  
static DictationSession *s_dictation_session;
static Window *s_main_window;
static TextLayer *s_output_layer;

// Declare a buffer for the DictationSession
static char s_last_text[512];

// Send a message
static void sendString(int key, char* value) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_cstring(iter, key, value);

  app_message_outbox_send();
}

// Dictation Callback
static void dictation_session_callback(DictationSession *session, DictationSessionStatus status, 
                                       char *transcription, void *context) {
  if(status == DictationSessionStatusSuccess) {
    // Display the dictated text
    snprintf(s_last_text, sizeof(s_last_text), "Transcription:\n\n%s", transcription);
    text_layer_set_text(s_output_layer, s_last_text);
    
    // Send the dictated text to the phone
    sendString(KEY_MESSAGE, transcription);
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
  int location;
  char *name;
  char *phoneNumber;
  int contactKey;
  // Get the first pair
  Tuple *data = dict_find(iterator, KEY_NUM_CONTACTS);
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

  s_output_layer = text_layer_create(GRect(5, 0, window_bounds.size.w - 5, window_bounds.size.h));
  text_layer_set_text(s_output_layer, initialString);
  text_layer_set_text_alignment(s_output_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_output_layer, GTextOverflowModeWordWrap);
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

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
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
}

static void deinit(void) {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}