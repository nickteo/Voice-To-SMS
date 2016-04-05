#include <pebble.h>
#include "dictation.h"
#include "messaging.h"
#include "globals.h"

DictationSession *s_dictation_session;

// Declare a buffer for the DictationSession
char s_last_text[512];

// Dictation Callback
void dictation_session_callback(DictationSession *session, DictationSessionStatus status, 
                                       char *transcription, void *context) {
  if(status == DictationSessionStatusSuccess) {
    // Display the dictated text
    snprintf(s_last_text, sizeof(s_last_text), "Transcription:\n\n%s", transcription);
    //text_layer_set_text(s_output_layer, s_last_text);
    
    // Send the dictated text to the phone
    sendText((char*) selectedNumber, transcription);
  } else {
    // Display the reason for any error
    static char s_failed_buff[128];
    snprintf(s_failed_buff, sizeof(s_failed_buff), "Transcription failed.\n\nReason:\n%d", 
             (int)status);
    //text_layer_set_text(s_output_layer, s_failed_buff);
  }
}