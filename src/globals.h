#include <pebble.h>

#define KEY_PHONE_NUMBER 0
#define KEY_MESSAGE 1
#define KEY_NUM_CONTACTS 2
#define KEY_NEED_CONTACTS 3
#define KEY_MESSAGE_STATUS 4
#define KEY_FIRST_CONTACT 100

#define MESSAGE_FAILED -1
#define MESSAGE_SENT 0

// Selected number. Used in main to send the message
char selectedNumber[15];