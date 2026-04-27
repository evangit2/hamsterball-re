/**
 * sfx_dsound.h - OGG SFX playback via DirectSound8
 *
 * For D3D8 win32 build. Decodes OGG → PCM via stb_vorbis,
 * uploads to IDirectSoundBuffer, plays.
 */

#ifndef SFX_DSOUND_H
#define SFX_DSOUND_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration — actual IDirectSound8* is opaque here */
bool sfx_init(void *dsound);
void sfx_shutdown(void);
bool sfx_load_ogg(const char *name, const char *path);
void sfx_play(const char *name);
void sfx_set_volume(float vol);   /* 0.0 ~ 1.0 */

#ifdef __cplusplus
}
#endif

#endif /* SFX_DSOUND_H */
