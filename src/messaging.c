#include <pebble.h>
#include "messaging.h"
#include "globals.h"

// Send a message
void sendString(int key, char* value) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_cstring(iter, key, value);

  app_message_outbox_send();
}

// Send text
void sendText(char* number, char* message) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  
  dict_write_cstring(iter, KEY_PHONE_NUMBER, number);
  dict_write_cstring(iter, KEY_MESSAGE, message);
  
  app_message_outbox_send();
}