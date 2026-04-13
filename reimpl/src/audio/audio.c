/* Audio stub - BASS equivalent */
#include <stdio.h>
#include <stdbool.h>
#include "audio/audio.h"

bool audio_init(void) {
    printf("[AUDIO] Stub initialized (BASS equivalent)\n");
    return true;
}
void audio_shutdown(void) {}
bool audio_load_music(const char *path) { (void)path; return true; }
void audio_play_music(int pattern_index) { (void)pattern_index; }
void audio_stop_music(void) {}
void audio_set_music_volume(float vol) { (void)vol; }
bool audio_load_sfx(const char *name, const char *path) { (void)name; (void)path; return true; }
void audio_play_sfx(const char *name) { (void)name; }
void audio_set_sfx_volume(float vol) { (void)vol; }

/* Original main.c references these old signatures - provide compat */
void audio_load_music_compat(const char *path) { audio_load_music(path); }
void audio_play_music_compat(void) { audio_play_music(0); }
void audio_set_volume(float vol) { audio_set_music_volume(vol); }