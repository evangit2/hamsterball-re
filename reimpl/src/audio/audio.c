/*
 * audio.c - SDL2_mixer audio backend
 * Replaces BASS_Init / BASS_MusicLoad / BASS_SampleLoad
 *
 * Original calls matched:
 *   BASS_Init(-1, 44100, 0, hwnd, NULL)        → Mix_OpenAudio
 *   BASS_MusicLoad(FALSE, mo3_path, 0, 0, ...) → Mix_LoadMUS (WAV)
 *   BASS_SampleLoad(FALSE, ogg_path, ...)       → Mix_LoadWAV (OGG)
 *   BASS_ChannelPlay(channel, TRUE)             → Mix_PlayMusic / Mix_PlayChannel
 */

#include "audio/audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL_mixer.h>

#define MAX_SFX 128

static bool   g_audio_initialized = false;
static bool   g_music_loaded      = false;
static bool   g_music_playing     = false;
static Mix_Music *g_music         = NULL;
static float  g_music_volume      = 0.85f;
static float  g_sfx_volume        = 0.90f;

/* SFX registry */
static struct {
    char name[64];
    Mix_Chunk *chunk;
} g_sfx[MAX_SFX];
static int g_sfx_count = 0;

/* ---- Helpers ---- */
static int find_sfx(const char *name) {
    for (int i = 0; i < g_sfx_count; i++) {
        if (strcmp(g_sfx[i].name, name) == 0) return i;
    }
    return -1;
}

/* ---- Public API ---- */

bool audio_init(void) {
    if (g_audio_initialized) return true;

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "[AUDIO] Mix_OpenAudio failed: %s\n", Mix_GetError());
        return false;
    }
    Mix_AllocateChannels(16);
    g_audio_initialized = true;
    printf("[AUDIO] SDL2_mixer initialized: 44100Hz, 16 channels\n");
    return true;
}

void audio_shutdown(void) {
    if (!g_audio_initialized) return;

    audio_stop_music();

    for (int i = 0; i < g_sfx_count; i++) {
        if (g_sfx[i].chunk) {
            Mix_FreeChunk(g_sfx[i].chunk);
            g_sfx[i].chunk = NULL;
        }
    }
    g_sfx_count = 0;

    if (g_music) {
        Mix_FreeMusic(g_music);
        g_music = NULL;
    }
    g_music_loaded = false;

    Mix_CloseAudio();
    g_audio_initialized = false;
    printf("[AUDIO] Shutdown complete\n");
}

bool audio_load_music(const char *path) {
    if (!g_audio_initialized) return false;

    /* Unload previous */
    if (g_music) {
        Mix_FreeMusic(g_music);
        g_music = NULL;
    }
    g_music_loaded = false;

    /* SDL2_mixer supports WAV, MP3, OGG, FLAC, MOD, etc.
     * For MO3 we pre-render to WAV (see audio-test/songs/). */
    g_music = Mix_LoadMUS(path);
    if (!g_music) {
        fprintf(stderr, "[AUDIO] Mix_LoadMUS('%s') failed: %s\n", path, Mix_GetError());
        return false;
    }

    g_music_loaded = true;
    printf("[AUDIO] Music loaded: %s\n", path);
    return true;
}

void audio_play_music(int subsong_index) {
    (void)subsong_index;
    /* subsong_index is handled at a higher level (jukebox) by loading the
     * appropriate pre-rendered file before calling this. */
    if (!g_music_loaded || !g_music) return;

    Mix_VolumeMusic((int)(g_music_volume * MIX_MAX_VOLUME));
    if (Mix_PlayMusic(g_music, -1) == 0) {
        g_music_playing = true;
    } else {
        fprintf(stderr, "[AUDIO] Mix_PlayMusic failed: %s\n", Mix_GetError());
    }
}

void audio_stop_music(void) {
    if (g_music_playing) {
        Mix_HaltMusic();
        g_music_playing = false;
    }
}

void audio_set_music_volume(float vol) {
    g_music_volume = (vol < 0.0f) ? 0.0f : (vol > 1.0f) ? 1.0f : vol;
    Mix_VolumeMusic((int)(g_music_volume * MIX_MAX_VOLUME));
}

/* ---- SFX ---- */

bool audio_load_sfx(const char *name, const char *path) {
    if (!g_audio_initialized) return false;
    if (g_sfx_count >= MAX_SFX) {
        fprintf(stderr, "[AUDIO] SFX table full (%d)\n", MAX_SFX);
        return false;
    }

    /* Check for duplicate */
    if (find_sfx(name) >= 0) return true;

    Mix_Chunk *chunk = Mix_LoadWAV(path);
    if (!chunk) {
        fprintf(stderr, "[AUDIO] Mix_LoadWAV('%s' = '%s') failed: %s\n",
                name, path, Mix_GetError());
        return false;
    }

    strncpy(g_sfx[g_sfx_count].name, name, sizeof(g_sfx[0].name) - 1);
    g_sfx[g_sfx_count].chunk = chunk;
    g_sfx_count++;
    printf("[AUDIO] SFX loaded: %s\n", name);
    return true;
}

void audio_play_sfx(const char *name) {
    if (!g_audio_initialized) return;
    int idx = find_sfx(name);
    if (idx < 0) return;
    Mix_VolumeChunk(g_sfx[idx].chunk, (int)(g_sfx_volume * MIX_MAX_VOLUME));
    Mix_PlayChannel(-1, g_sfx[idx].chunk, 0);
}

void audio_set_sfx_volume(float vol) {
    g_sfx_volume = (vol < 0.0f) ? 0.0f : (vol > 1.0f) ? 1.0f : vol;
}

/* ---- Compat shims ---- */

void audio_load_music_compat(const char *path) { audio_load_music(path); }
void audio_play_music_compat(void)             { audio_play_music(0); }
void audio_set_volume(float vol)                { audio_set_music_volume(vol); }
