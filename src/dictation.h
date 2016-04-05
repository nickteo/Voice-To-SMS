#pragma once

char s_last_text[512];
DictationSession *s_dictation_session;

void dictation_session_callback(DictationSession *session, DictationSessionStatus status, char *transcription, void *context);