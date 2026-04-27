/**
 * sfx_dsound.c - OGG SFX playback via DirectSound8 for D3D8 win32 build
 *
 * Decodes OGG → PCM via stb_vorbis, uploads to IDirectSoundBuffer8, plays.
 * Self-contained — no SDL2, no stb_vorbis dependency needed in MinGW.
 */

/* MinGW fix: include mmeapi.h before dsound.h for LPWAVEFORMATEX */
#include <windows.h>
#include <mmeapi.h>
#include <dsound.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "audio/sfx_dsound.h"

/* stb_vorbis decode function — implementation in stb_vorbis_impl.c */
extern int stb_vorbis_decode_filename(const char *filename, int *channels,
                                       int *sample_rate, short **output);

/* MinGW fix: dsound.h needs LPWAVEFORMATEX before it's defined in mmeapi.h */
#ifndef LPWAVEFORMATEX
struct tWAVEFORMATEX; typedef struct tWAVEFORMATEX *LPWAVEFORMATEX;
typedef const struct tWAVEFORMATEX *LPCWAVEFORMATEX;
#endif

#define MAX_SFX 64
#define SFX_PATH_MAX 512

static struct {
    char name[64];
    IDirectSoundBuffer *buf;
} g_sfx[MAX_SFX];
static int g_sfx_count = 0;
static IDirectSound8 *g_dsound = NULL;
static float g_sfx_vol = 1.0f;

static int find_sfx(const char *name) {
    for (int i = 0; i < g_sfx_count; i++) {
        if (strcmp(g_sfx[i].name, name) == 0) return i;
    }
    return -1;
}

bool sfx_init(void *dsound) {
    g_dsound = (IDirectSound8 *)dsound;
    g_sfx_count = 0;
    if (!g_dsound) {
        fprintf(stderr, "[SFX] No DirectSound8 device\n");
        return false;
    }
    printf("[SFX] Initialized (max %d sounds)\n", MAX_SFX);
    return true;
}

void sfx_shutdown(void) {
    for (int i = 0; i < g_sfx_count; i++) {
        if (g_sfx[i].buf) {
            g_sfx[i].buf->lpVtbl->Stop(g_sfx[i].buf);
            g_sfx[i].buf->lpVtbl->Release(g_sfx[i].buf);
            g_sfx[i].buf = NULL;
        }
    }
    g_sfx_count = 0;
    g_dsound = NULL;
    printf("[SFX] Shutdown\n");
}

/* Decode OGG to PCM16 via stb_vorbis */
static short *decode_ogg_to_pcm(const char *path, int *out_channels,
                                 int *out_sample_rate, int *out_samples) {
    int channels = 0, sample_rate = 0;
    short *pcm = NULL;
    int samples = stb_vorbis_decode_filename(path, &channels, &sample_rate, &pcm);
    if (samples <= 0 || !pcm) {
        fprintf(stderr, "[SFX] Failed to decode OGG: %s\n", path);
        return NULL;
    }
    *out_channels = channels;
    *out_sample_rate = sample_rate;
    *out_samples = samples;  /* samples = frames per channel */
    return pcm;
}

bool sfx_load_ogg(const char *name, const char *path) {
    if (!g_dsound || g_sfx_count >= MAX_SFX) return false;
    if (find_sfx(name) >= 0) return true;  /* already loaded */

    int channels, sample_rate, samples;
    short *pcm = decode_ogg_to_pcm(path, &channels, &sample_rate, &samples);
    if (!pcm) return false;

    /* PCM data size: samples * channels * sizeof(short) */
    int frames = samples;  /* samples is per channel */
    DWORD data_bytes = (DWORD)(frames * channels * sizeof(short));

    WAVEFORMATEX wfx;
    memset(&wfx, 0, sizeof(wfx));
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = (WORD)channels;
    wfx.nSamplesPerSec = (DWORD)sample_rate;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = (WORD)(channels * sizeof(short));
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    DSBUFFERDESC dsbd;
    memset(&dsbd, 0, sizeof(dsbd));
    dsbd.dwSize = sizeof(dsbd);
    dsbd.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRLVOLUME;
    dsbd.dwBufferBytes = data_bytes;
    dsbd.lpwfxFormat = &wfx;

    IDirectSoundBuffer *buf = NULL;
    HRESULT hr = g_dsound->lpVtbl->CreateSoundBuffer(g_dsound, &dsbd, &buf, NULL);
    if (FAILED(hr) || !buf) {
        fprintf(stderr, "[SFX] CreateSoundBuffer failed: 0x%08lx\n", hr);
        free(pcm);
        return false;
    }

    /* Lock and fill */
    void *ptr1 = NULL, *ptr2 = NULL;
    DWORD len1 = 0, len2 = 0;
    hr = buf->lpVtbl->Lock(buf, 0, data_bytes, &ptr1, &len1, &ptr2, &len2, 0);
    if (SUCCEEDED(hr) && ptr1) {
        memcpy(ptr1, pcm, len1);
        if (ptr2 && len2 > 0) memcpy(ptr2, (BYTE *)pcm + len1, len2);
        buf->lpVtbl->Unlock(buf, ptr1, len1, ptr2, len2);
    } else {
        fprintf(stderr, "[SFX] Buffer lock failed: 0x%08lx\n", hr);
        buf->lpVtbl->Release(buf);
        free(pcm);
        return false;
    }

    free(pcm);

    strncpy(g_sfx[g_sfx_count].name, name, sizeof(g_sfx[0].name) - 1);
    g_sfx[g_sfx_count].buf = buf;
    g_sfx_count++;
    printf("[SFX] Loaded: %s (%s, %d ch, %d Hz, %d frames)\n",
           name, path, channels, sample_rate, frames);
    return true;
}

void sfx_play(const char *name) {
    if (!g_dsound) return;
    int idx = find_sfx(name);
    if (idx < 0) return;

    IDirectSoundBuffer *buf = g_sfx[idx].buf;
    if (!buf) return;

    /* Set volume: DSound volume is in hundredths of dB, 0 = max, -10000 = silent */
    LONG vol = (g_sfx_vol <= 0.0f) ? DSBVOLUME_MIN : (LONG)(2000.0f * log10f(g_sfx_vol));
    if (vol < DSBVOLUME_MIN) vol = DSBVOLUME_MIN;
    if (vol > DSBVOLUME_MAX) vol = DSBVOLUME_MAX;
    buf->lpVtbl->SetVolume(buf, vol);

    buf->lpVtbl->SetCurrentPosition(buf, 0);
    buf->lpVtbl->Play(buf, 0, 0, 0);
}

void sfx_set_volume(float vol) {
    g_sfx_vol = (vol < 0.0f) ? 0.0f : (vol > 1.0f) ? 1.0f : vol;
}
