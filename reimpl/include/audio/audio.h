/*
 * Audio system stub - BASS library equivalent using SDL2_mixer
 * Corresponds to BASS_Init at 0x4D9238, BASS_MusicLoad at 0x4D926A
 * Status: INFERRED
 */

#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

bool audio_init(void);
void audio_shutdown(void);

/* Music - MO3 format via BASS (using SDL2_mixer as backend) */
bool audio_load_music(const char *path);
void audio_play_music(int pattern_index);
void audio_stop_music(void);
void audio_set_music_volume(float volume);

/* Sound effects - OGG format */
bool audio_load_sfx(const char *name, const char *path);
void audio_play_sfx(const char *name);
void audio_set_sfx_volume(float volume);

#endif /* AUDIO_H */