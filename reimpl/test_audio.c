/*
 * test_audio.c — Standalone audio test using SDL2_mixer
 * Compile (Linux):   gcc -o test_audio test_audio.c $(pkg-config --cflags --libs SDL2_mixer)
 * Compile (MinGW):   i686-w64-mingw32-gcc -o test_audio.exe test_audio.c \
 *                      -L mingw-libs/SDL2_mixer-2.8.0/i686-w64-mingw32/lib \
 *                      -lSDL2_mixer -lwinmm -mwindows
 */

/* Define BEFORE SDL.h to prevent WinMain redirect on Windows builds */
#if defined(WIN32) || defined(_WIN32)
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#ifdef main
#undef main
#endif
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_SFX 128

static bool   g_audio_initialized = false;
static bool   g_music_loaded      = false;
static bool   g_music_playing     = false;
static Mix_Music *g_music         = NULL;
static float  g_music_volume      = 0.85f;
static float  g_sfx_volume        = 0.90f;

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
    if (g_music_playing) Mix_HaltMusic();
    for (int i = 0; i < g_sfx_count; i++) {
        if (g_sfx[i].chunk) Mix_FreeChunk(g_sfx[i].chunk);
    }
    g_sfx_count = 0;
    if (g_music) { Mix_FreeMusic(g_music); g_music = NULL; }
    g_music_loaded = false;
    Mix_CloseAudio();
    g_audio_initialized = false;
}

bool audio_load_music(const char *path) {
    if (!g_audio_initialized) return false;
    if (g_music) { Mix_FreeMusic(g_music); g_music = NULL; }
    g_music_loaded = false;
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
    if (!g_music_loaded || !g_music) return;
    Mix_VolumeMusic((int)(g_music_volume * MIX_MAX_VOLUME));
    if (Mix_PlayMusic(g_music, -1) == 0) {
        g_music_playing = true;
        printf("[AUDIO] Music playing\n");
    }
}

void audio_stop_music(void) {
    if (g_music_playing) { Mix_HaltMusic(); g_music_playing = false; }
}

void audio_set_music_volume(float vol) {
    g_music_volume = (vol < 0.0f) ? 0.0f : (vol > 1.0f) ? 1.0f : vol;
    Mix_VolumeMusic((int)(g_music_volume * MIX_MAX_VOLUME));
}

bool audio_load_sfx(const char *name, const char *path) {
    if (!g_audio_initialized) return false;
    if (g_sfx_count >= MAX_SFX) return false;
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

/* ---- Test ---- */
static void test_music(const char *songs_dir) {
    printf("\n=== MUSIC TEST ===\n");
    char path[512];

    /* Test song 0: Main Theme */
    snprintf(path, sizeof(path), "%s/00_main_theme.wav", songs_dir);
    if (audio_load_music(path)) {
        printf("Playing Main Theme for 5 seconds...\n");
        audio_play_music(0);
        SDL_Delay(5000);
        audio_stop_music();
    }

    /* Test song 2: Hamster Nation */
    snprintf(path, sizeof(path), "%s/02_hamster_nation.wav", songs_dir);
    if (audio_load_music(path)) {
        printf("Playing Hamster Nation for 5 seconds...\n");
        audio_play_music(0);
        SDL_Delay(5000);
        audio_stop_music();
    }

    /* Volume fade test */
    printf("Volume fade test...\n");
    if (audio_load_music(path)) {
        audio_play_music(0);
        for (int v = 100; v >= 0; v -= 10) {
            audio_set_music_volume(v / 100.0f);
            SDL_Delay(200);
        }
        audio_stop_music();
    }
}

static void test_sfx(const char *sounds_dir) {
    printf("\n=== SFX TEST ===\n");
    char path[512];
    const char *test_sounds[] = {
        "BallBreak.ogg",
        "Bell.ogg",
        "Bing.ogg",
        "Bumper.ogg",
        "Whistle.ogg",
        "Whoosh.ogg",
        "Popout.ogg",
        "Zip.ogg",
    };

    for (int i = 0; i < 8; i++) {
        snprintf(path, sizeof(path), "%s/%s", sounds_dir, test_sounds[i]);
        FILE *f = fopen(path, "rb");
        if (!f) {
            printf("  %s: NOT FOUND\n", test_sounds[i]);
            continue;
        }
        fclose(f);

        /* Build a name without extension */
        char name[64];
        strncpy(name, test_sounds[i], 64);
        char *dot = strrchr(name, '.');
        if (dot) *dot = '\0';

        if (audio_load_sfx(name, path)) {
            printf("  Playing %s...\n", name);
            audio_play_sfx(name);
            SDL_Delay(500);
        } else {
            printf("  %s: LOAD FAILED\n", name);
        }
    }
}

int main(int argc, char *argv[]) {
    printf("Hamsterball Audio Test — SDL2_mixer\n");
    printf("====================================\n");

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    if (!audio_init()) {
        SDL_Quit();
        return 1;
    }

    /* Resolve paths from argv or default */
    const char *songs_dir  = argc > 1 ? argv[1] : "./audio-test/songs";
    const char *sounds_dir = argc > 2 ? argv[2] : "./originals/installed/extracted/Sounds";

    test_music(songs_dir);
    test_sfx(sounds_dir);

    printf("\n=== DONE ===\n");
    audio_shutdown();
    SDL_Quit();
    return 0;
}
