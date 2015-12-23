#include <pebble.h>

static DictationSession *s_dictation_session;
static Window *s_main_window;
static TextLayer *s_output_layer;

// Declare a buffer for the DictationSession
static char s_last_text[512];

// Dictation Callback
static void dictation_session_callback(DictationSession *session, DictationSessionStatus status, 
                                       char *transcription, void *context) {
  if(status == DictationSessionStatusSuccess) {
    // Display the dictated text
    snprintf(s_last_text, sizeof(s_last_text), "Transcription:\n\n%s", transcription);
    text_layer_set_text(s_output_layer, s_last_text);
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

static void outbox_sent_handler(DictionaryIterator *iter, void *context) {
  // Ready for next command
  text_layer_set_text(s_output_layer, "Press up or down.");
}

static void outbox_failed_handler(DictionaryIterator *iter, AppMessageResult reason, void *context) {
  text_layer_set_text(s_output_layer, "Send failed!");
  APP_LOG(APP_LOG_LEVEL_ERROR, "Fail reason: %d", (int)reason);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Window Loading Stuffs /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  const int text_height = 20;
  const GEdgeInsets text_insets = GEdgeInsets((bounds.size.h - text_height) / 2, 0);

  s_output_layer = text_layer_create(bounds);
  text_layer_set_text(s_output_layer, "Speak your mind!");
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
  app_message_register_outbox_sent(outbox_sent_handler);
  app_message_register_outbox_failed(outbox_failed_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit(void) {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}