void on_result(const char *result, char is_last);
void on_speech_begin();
void on_speech_end(int reason);

int InitVoice();
void RecognizeVoice(char ** pResult);
void UninitVoice();


