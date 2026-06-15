# Hamsterball Audio Modding Guide

This guide covers how the original Hamsterball Windows executable loads, plays, and mixes sound effects and music, and how a modder can trigger existing sounds or add new custom audio from inside a DLL hook.

It is built from live Ghidra decompilation of `Hamsterball.exe` and the existing `AUDIO_SYSTEM.md` / `AUDIO_SYSTEM_SFX.md` docs.

---

## 1. Audio architecture

Hamsterball uses two completely separate audio APIs:

| System | Library | Purpose | File formats |
|---|---|---|---|
| **Music** | BASS.dll (un4seen) | Background music, menu stings | `.mo3` tracker modules |
| **Sound effects** | DirectSound8 (`dsound.dll`) | 3D positional gameplay sounds | `.ogg` (preferred), `.wav` (fallback) |

Both systems live under the single `App` object. The audio device object is at `App + 0x178` (`SoundDevice`). Music channels are stored at `App + 0x534`, `0x538`, `0x53C`.

---

## 2. Key structs

### 2.1 `SoundDevice` — the DirectSound8 manager

Created by `SoundDevice_ctor` (`0x00466620`). It wraps the IDirectSound8 interface and owns all loaded sound buffers.

| Offset | Type | Description |
|---|---|---|
| `+0x000` | vtable* | `SoundDevice` vtable (`0x4D911C`) |
| `+0x004` | `App*` | Back-pointer to owning `App` |
| `+0x008` | `int` | Count of `SoundList` objects in the master list |
| `+0x00C` | `int` | Capacity of that list |
| `+0x010` | `int*` | Array pointer for the master `SoundList` list |
| `+0x414` | `int*` | Same array pointer (redundant) |
| `+0x834` | `App*` | Cached `App` pointer |
| `+0x838` | `float` | Global SFX volume (read from registry `"Sound Volume"`) |
| `+0x83C` | `bool` | Sound enabled flag |
| `+0x84C` | `IDirectSound8*` | DirectSound8 COM interface |
| `+0x850` | `int` | Number of 3D listeners |
| `+0x854` | `Vec3[16]` | Listener positions for 3D attenuation |
| `+0x914` | `float` | Min rolloff distance (default `0.0`) |
| `+0x918` | `float` | Max rolloff distance (default `6000.0f`) |

The global volume is read by `Level_ReadSoundVolume` (`0x00466570`) and written back by `SoundDevice_dtor` (`0x004668A0`).

### 2.2 `SoundList` — one logical sound with N hardware buffers

A `SoundList` is an `AthenaList` that owns one or more `SoundEntry` objects. Each `SoundEntry` wraps one DirectSound buffer handle. Multiple entries allow the same sound to be played polyphonically.

| Offset | Type | Description |
|---|---|---|
| `+0x000` | vtable* | `SoundList` vtable (`0x4D8E7C`) |
| `+0x004` | `App*` | Back-pointer |
| `+0x008` | `AthenaList` | List of `SoundEntry*` buffers |
| `+0x00C` | `int` | Entry count |
| `+0x010` | `int` | Circular "next" index for playback |
| `+0x414` | `int*` | Entry array pointer |

Object size is `0x420` bytes.

### 2.3 `SoundEntry` — one DirectSound buffer

| Offset | Type | Description |
|---|---|---|
| `+0x000` | vtable* | `SoundEntry` scalar dtor (`0x4D8E78`) |
| `+0x004` | `App*` | Back-pointer |
| `+0x008` | `IDirectSoundBuffer*` | DirectSound buffer handle |

### 2.4 `MusicChannel` / `MusicPlayer` — BASS music

| Offset | Type | Description |
|---|---|---|
| `+0x000` | vtable* | `MusicChannel` dtor |
| `+0x004` | `DWORD` | BASS device handle |
| `+0x008` | `HMUSIC` | BASS music handle |
| `+0x00C` | `AthenaList` | Track list (name → handle) |
| `+0x418` | `int*` | Track array |
| `+0x528` | `float` | Current music volume |
| `+0x530` | `byte` | Muted flag |
| `+0x531` | `byte` | Paused flag |

`MusicPlayer` (`App + 0x534`) also stores the loaded file path at `+0x424`.

---

## 3. How the game loads SFX at startup

`App_ResourceLoader` (misnamed `TimerDisplay`, `0x0042A8C0`) is the master loader. It creates a `LoadingScreenGadget` and calls its vtable methods to queue every asset. Slot `+0x60` queues a sound load.

```cpp
// LoadingScreenGadget vtable
+0x48  LoadTexture(dest, filename, alpha_flag)
+0x4C  LoadMesh  (dest, filename)
+0x50  LoadLevel (dest, filename)
+0x54  LoadCollision(dest, level_src)
+0x58  LoadSprite(dest, filename)
+0x5C  LoadFont  (dest, font_path)
+0x60  LoadSound (dest, sound_path, max_channels)
```

Example queue call from the loader:

```cpp
LoadingScreenGadget* loader = *(LoadingScreenGadget**)(app + 0x22C);
(**(code**)(*(int*)loader + 0x60))(loader, app + 0x460, "sounds\\dropin", 2);
```

The `dest` argument is an **App offset**, not a pointer. The loader writes the resulting `SoundList*` into `App + dest`.

### 3.1 Full startup sound → App offset map

| App offset | Sound name | Max channels | Notes |
|---|---|---|---|
| `+0x43C` | `sounds\\collide` | 10 | Ball/ball and ball/wall impacts |
| `+0x440` | `sounds\\roll` | 10 | Rolling loop |
| `+0x444` | `sounds\\whistle` | 1 | Spin/whistle |
| `+0x448` | `sounds\\bumper` | 10 | Bumper impacts |
| `+0x44C` | `sounds\\ballbreak` | 5 | Large break |
| `+0x450` | `sounds\\ballbreaksmall` | 5 | Small break |
| `+0x454` | `sounds\\thwomp` | 2 | Thwomp hazard |
| `+0x458` | `sounds\\snap` | 2 | Snap hazard |
| `+0x45C` | `sounds\\popup` | 2 | Popup sign |
| `+0x460` | `sounds\\dropin` | 2 | DROPIN event |
| `+0x464` | `sounds\\dropinshort` | 2 | Short drop |
| `+0x468` | `sounds\\popout` | 2 | POPOUT event |
| `+0x46C` | `sounds\\pipebump1` | 10 | PIPEBONK random set |
| `+0x470` | `sounds\\pipebump2` | 10 | PIPEBONK random set |
| `+0x474` | `sounds\\pipebump3` | 10 | PIPEBONK random set |
| `+0x478` | `sounds\\gearclank` | 20 | Gear mechanisms |
| `+0x47C` | `sounds\\bridgeslam` | 2 | Bridge slam |
| `+0x480` | `sounds\\platformtick` | 5 | Ticking platform |
| `+0x484` | `sounds\\gluestuck` | 5 | Tarpit / glue |
| `+0x488` | `sounds\\bubble1` | 5 | Bubble sound 1 |
| `+0x48C` | `sounds\\bubble2` | 5 | Bubble sound 2 |
| `+0x490` | `sounds\\wheelcreak` | 2 | Creaking wheel |
| `+0x494` | `sounds\\catapult` | 2 | Catapult launch |
| `+0x498` | `sounds\\trapdoor` | 2 | Trapdoor |
| `+0x49C` | `sounds\\fwing` | 2 | Fwing hazard |
| `+0x4A0` | `sounds\\clink` | 3 | Metal clink |
| `+0x4A4` | `sounds\\whoosh` | 3 | Whoosh |
| `+0x4A8` | `sounds\\chomp` | 1 | Chomper |
| `+0x4AC` | `sounds\\fan-start` | 10 | Fan startup |
| `+0x4B0` | `sounds\\fan-blow` | 10 | Fan loop |
| `+0x4B4` | `sounds\\crack` | 2 | Crack |
| `+0x4B8` | `sounds\\crumble` | 2 | Crumble |
| `+0x4BC` | `sounds\\sawstartup` | 2 | Saw startup |
| `+0x4C0` | `sounds\\sawcut` | 2 | Saw cutting |
| `+0x4C4` | `sounds\\minipop` | 5 | Mini pop |
| `+0x4C8` | `sounds\\bell` | 3 | Bell |
| `+0x4CC` | `sounds\\zip` | 2 | Zip (mouse-trap / fast move) |
| `+0x4D0` | `sounds\\ting` | 20 | Ting |
| `+0x4D4` | `sounds\\shrink` | 3 | Shrink powerup |
| `+0x4D8` | `sounds\\grow` | 3 | Grow powerup |
| `+0x4DC` | `sounds\\tweet` | 3 | Tweet |
| `+0x4E0` | `sounds\\creakyplatform` | 20 | Creaky platform |
| `+0x4E4` | `sounds\\wubba` | 5 | Wubba |
| `+0x4E8` | `sounds\\saw` | 2 | Saw loop |
| `+0x4EC` | `sounds\\sawspeedy` | 2 | Speedy saw |
| `+0x4F0` | `sounds\\dawgstep1` | 10 | Dawg step 1 |
| `+0x4F4` | `sounds\\dawgstep2` | 10 | Dawg step 2 |
| `+0x4F8` | `sounds\\dawgsmash` | 10 | Dawg smash |
| `+0x4FC` | `sounds\\sizzle` | 2 | Sizzle |
| `+0x500` | `sounds\\explode` | 3 | Explosion |
| `+0x504` | `sounds\\vac-o-sux` | 3 | Vacuum |
| `+0x508` | `sounds\\speedcylinder` | 2 | Speed cylinder |
| `+0x50C` | `sounds\\bonuspop` | 5 | Bonus pop |
| `+0x510` | `sounds\\buzzbonus` | 1 | Buzz bonus |
| `+0x514` | `sounds\\breakbridge` | 1 | Break bridge |
| `+0x518` | `sounds\\unlock` | 1 | Unlock |
| `+0x51C` | `sounds\\NeonRide` | 1 | Neon ride |
| `+0x520` | `sounds\\NeonFlicker` | 50 | Neon flicker |
| `+0x524` | `sounds\\ZoopDown` | 2 | Zoop down |
| `+0x528` | `sounds\\LightsOff` | 2 | Lights off |
| `+0x52C` | `sounds\\GlassBonus` | 2 | Glass bonus |

All of these are stored as `SoundList*` pointers inside `App`.

---

## 4. Core SFX engine functions

| Address | Name | Notes |
|---|---|---|
| `0x00458F20` | `SoundList_Ctor` | `__thiscall (SoundList* this, App* app)` |
| `0x00466620` | `SoundDevice_ctor` | Creates DirectSound8 device |
| `0x00458F40` | `SoundList_LoadWAV` | Manual WAV parser → DirectSound buffer |
| `0x00459310` | `Sound_LoadOgg` | OGG Vorbis decoder → DirectSound buffer |
| `0x00459660` | `Sound_LoadOggOrWav` | Tries `name.ogg`, falls back to `name.wav` |
| `0x00466500` | `Sound_LoadAndAppend` | Allocates `SoundList`, constructs it, loads file, appends to master list |
| `0x00459810` | `Sound_GetNextChannel` | Circular allocator from a `SoundList` |
| `0x004597B0` | `Sound_PlayChannel` | Plays the next available buffer in a list |
| `0x00459860` | `Sound_Play3D` | Computes distance attenuation, then plays |
| `0x00458EE0` | `Sound_Play3DAtPosition` | Applies the attenuated volume to a channel |
| `0x00466570` | `Level_ReadSoundVolume` | Reads registry volume into `SoundDevice+0x838` |
| `0x00466750` | `Sound_CalculateDistanceAttenuation` | Linear 3D rolloff from nearest listener |

### 4.1 `Sound_LoadOggOrWav` behavior

```cpp
void __thiscall Sound_LoadOggOrWav(SoundList* this, const char* baseName);
```

It builds the filenames `"baseName.ogg"` and `"baseName.wav"` and checks file access. OGG is tried first. If neither file exists, nothing is loaded and the list stays empty.

### 4.2 `Sound_PlayChannel` behavior

```cpp
void __fastcall Sound_PlayChannel(SoundList* soundList);
```

- If sound is disabled or global volume is `0`, returns.
- Grabs the next `SoundEntry*` from the circular list.
- Calls DirectSound `Stop` → `SetVolume` → `Play`.
- The volume passed to DirectSound comes from `Audio_ClampPanValue()` and the global volume.

### 4.3 `Sound_Play3D` behavior

```cpp
void __thiscall Sound_Play3D(SoundList* soundList, float x, float y, float z);
```

1. Calls `Sound_CalculateDistanceAttenuation` using all listeners.
2. Calls `Sound_PlayChannel`.

For menu/UI sounds where you do **not** want 3D attenuation, call `Sound_PlayChannel` directly on the list.

---

## 5. How to trigger an existing sound effect

Because every SFX is already loaded into an `App` offset, the easiest trigger is:

```cpp
// Get App base. In a hook you usually already have this.
App* app = GetAppSomehow();

// Example: play the "dropin" sound at full volume (no 3D)
SoundList* dropin = *(SoundList**)((char*)app + 0x460);
if (dropin) {
    typedef void (__fastcall *Sound_PlayChannel_t)(SoundList*);
    static auto fn = (Sound_PlayChannel_t)0x004597B0;
    fn(dropin);
}

// Example: play "fwing" at a 3D position
SoundList* fwing = *(SoundList**)((char*)app + 0x49C);
if (fwing) {
    typedef void (__thiscall *Sound_Play3D_t)(SoundList*, float, float, float);
    static auto fn = (Sound_Play3D_t)0x00459860;
    fn(fwing, x, y, z);
}
```

### 5.1 Triggering sounds from collision events

The engine already wires many level tags to sounds in `GameObject_HandleCollision` (a.k.a. `CreateNoDizzy`, `0x0040C5D0`) and `Level_HandleCollision` (`0x0040DCD0`):

| Tag | Sound source | Function called |
|---|---|---|
| `E:JUMP` | `App + 0x49C` (`sounds\\fwing`) | `Sound_Play3D` |
| `N:TARPIT` | `App + 0x484` (`sounds\\gluestuck`) | `Sound_Play3D` |
| `DROPIN` | `App + 0x460` (`sounds\\dropin`) | `Sound_PlayChannel` |
| `PIPEBONK` | random `App + 0x46C/0x470/0x474` | `Sound_Play3D` |
| `POPOUT` | `App + 0x468` (`sounds\\popout`) | `Sound_PlayChannel` |
| `N:GOAL` | `Audio_PlayMusic(App+0x53C, "Goal!")` | BASS |
| `E:CATAPULTBOTTOM` | `App + 0x464` (`sounds\\dropinshort`) | `Sound_PlayChannel` |

So adding an object with one of these tags in a MESHWORLD level already produces the corresponding sound. For custom tags, hook `GameObject_HandleCollision` and call the sound yourself.

---

## 6. How to add a completely custom sound

There are two practical approaches.

### 6.1 Option A: load during app initialization via the loader queue

If your DLL is loaded early enough (before `App_ResourceLoader` finishes), you can queue a sound load exactly like the game does:

```cpp
void QueueCustomSound(App* app, const char* path, int maxChannels) {
    // App+0x22C = LoadingScreenGadget* loader
    void* loader = *(void**)((char*)app + 0x22C);
    if (!loader) return;

    // Pick an unused App offset. The block +0x530..+0x52C is the last used SFX region,
    // so +0x5B0, +0x5B4, etc. are safe unused slots.
    int destOffset = 0x5B0;

    void** vtable = *(void***)loader;
    ((void(__thiscall*)(void*, int, const char*, int))(vtable[0x60 / 4]))
        (loader, destOffset, path, maxChannels);
}
```

After the loader merges its lists, `App + 0x5B0` will hold a `SoundList*` you can play with `Sound_PlayChannel`.

### 6.2 Option B: load a sound manually at runtime (recommended)

This works any time after `SoundDevice_ctor` has run.

```cpp
// Operator new from the original CRT
void* operator_new(size_t);

typedef void* (__thiscall *SoundList_Ctor_t)(void* this_ptr, App* app);
typedef void  (__thiscall *Sound_LoadOggOrWav_t)(void* this_ptr, const char* baseName);
typedef void  (__fastcall *Sound_PlayChannel_t)(void* soundList);

void* LoadCustomSound(App* app, const char* baseName) {
    static auto op_new      = (void*(*)(size_t))0x004BA57B;
    static auto ctor        = (SoundList_Ctor_t)0x00458F20;
    static auto load        = (Sound_LoadOggOrWav_t)0x00459660;

    void* soundList = op_new(0x420);
    if (!soundList) return nullptr;

    ctor(soundList, app);
    load(soundList, baseName);   // tries baseName.ogg then baseName.wav
    return soundList;
}

void PlayCustomSound(void* soundList) {
    static auto play = (Sound_PlayChannel_t)0x004597B0;
    if (soundList) play(soundList);
}

void PlayCustomSound3D(void* soundList, float x, float y, float z) {
    typedef void (__thiscall *fn_t)(void*, float, float, float);
    static auto play3d = (fn_t)0x00459860;
    if (soundList) play3d(soundList, x, y, z);
}
```

Place `MySound.ogg` (or `.wav`) in the game root or under `Sounds/` and call:

```cpp
void* mySound = LoadCustomSound(app, "sounds\\MySound");  // or just "MySound"
PlayCustomSound(mySound);
```

The file path is passed to `_check_file_access`, so relative paths work from the game's working directory.

### 6.3 Notes on custom sounds

- **Format**: OGG Vorbis is preferred. WAV is parsed manually and must be standard PCM RIFF/WAVE.
- **Channels / polyphony**: `maxChannels` in the loader queue controls how many duplicate DirectSound buffers are created. For manual loads, the game only creates one buffer per file; rapid re-triggers will cut off the previous instance. If you need polyphony, load the same file into multiple `SoundList` objects and rotate between them, or use the loader queue.
- **3D attenuation**: `Sound_Play3D` uses the listener array at `SoundDevice+0x854`. In single-player, listener 0 is the camera/ball position. In multiplayer, each ball is a listener.
- **Global volume**: `Sound_PlayChannel` already multiplies by `SoundDevice+0x838`, so custom sounds respect the user's volume setting.

---

## 7. Music system (BASS)

### 7.1 BASS imports

| Address | Function | Typical call |
|---|---|---|
| `0x00487E40` | `BASS_SetConfig` | Configuration |
| `0x00487E4C` | `BASS_Init` | `BASS_Init(-1, 44100, 0, 0, NULL)` |
| `0x00487E46` | `BASS_Start` | Resume |
| `0x00487E58` | `BASS_Stop` | Pause all |
| `0x00487E52` | `BASS_Free` | Shutdown |
| `0x004794B0` | `BASS_ErrorGetCode` | Last error |
| `0x004794B6` | `BASS_MusicLoad` | Load `.mo3`/`.xm`/`.it` |
| `0x004794BC` | `BASS_ChannelStop` | Stop channel |
| `0x004794C2` | `BASS_ChannelSetAttributes` | Freq/volume/ramp |
| `0x004794C8` | `BASS_MusicPlayEx` | Play module |

### 7.2 Music channels

| App offset | Type | Use |
|---|---|---|
| `+0x534` | `MusicPlayer*` | Main music player, file path at `+0x424` |
| `+0x538` | `HMUSIC` | Music channel 2 |
| `+0x53C` | `HMUSIC` | Music channel 1 (used by `Audio_PlayMusic`) |

### 7.3 Playing a music track by name

```cpp
typedef void (__thiscall *Audio_PlayMusic_t)(void* musicChannel, const char* trackName);
static auto playMusic = (Audio_PlayMusic_t)0x0046A310;

void* channel = *(void**)((char*)app + 0x53C);
playMusic(channel, "Goal!");   // plays the track named "Goal!" from the loaded MO3
```

`Audio_PlayMusic` (`0x0046A310`) searches the track list linearly by name, sets volume to `1.0`, and calls `BASS_MusicPlayEx`.

### 7.4 Tempo control

```cpp
typedef void (__thiscall *Audio_PlayMusicAtSpeed_t)(void* musicChannel, const char* trackName, float speed);
static auto playSpeed = (Audio_PlayMusicAtSpeed_t)0x0046A440;

playSpeed(channel, "Intro", 4.0f);  // 4x speed
playSpeed(channel, "Race",  2.0f);  // normal-ish
```

### 7.5 Stopping music

```cpp
typedef void (__fastcall *Audio_StopChannel_t)(void* musicChannel);
static auto stopMusic = (Audio_StopChannel_t)0x0046A0D0;

stopMusic(channel);
```

### 7.6 Jukebox / track list

`jukebox.xml` defines named tracks:

```xml
<jukebox>
  <SONG>
    <NAME>Goal!</NAME>
    <HEX>1A</HEX>
  </SONG>
</jukebox>
```

`LoadJukebox` (`0x0046A4D0`) parses this and stores name→handle mappings. If you add a custom music track, you must either:
- Edit `jukebox.xml` and the `.mo3` file, or
- Use `BASS_MusicLoad` directly and call `BASS_MusicPlayEx` yourself.

### 7.7 Custom music with BASS directly

```cpp
typedef DWORD (__stdcall *BASS_MusicLoad_t)(BOOL mem, void* file, DWORD offset, DWORD length, DWORD flags, DWORD freq);
typedef BOOL  (__stdcall *BASS_MusicPlayEx_t)(DWORD handle, DWORD pos, BOOL restart, DWORD flags);
typedef BOOL  (__stdcall *BASS_ChannelStop_t)(DWORD handle);

static auto BASS_MusicLoad   = (BASS_MusicLoad_t)0x004794B6;
static auto BASS_MusicPlayEx = (BASS_MusicPlayEx_t)0x004794C8;
static auto BASS_ChannelStop = (BASS_ChannelStop_t)0x004794BC;

DWORD h = BASS_MusicLoad(0, "music\\custom.mo3", 0, 0, 4, 0);
if (h) BASS_MusicPlayEx(h, 0, TRUE, 0);
```

Flag `4` is `BASS_MUSIC_PRESCAN`.

---

## 8. Volume and 3D math

### 8.1 Reading/changing SFX volume

```cpp
float* volume = (float*)((char*)soundDevice + 0x838);
*volume = 0.75f;  // 75% SFX volume
```

The registry key `"Sound Volume"` is read at startup and saved on shutdown.

### 8.2 3D attenuation formula

`Sound_CalculateDistanceAttenuation` (`0x00466750`) does:

1. Find nearest listener in `SoundDevice+0x854`.
2. `dist = distance(soundPos, nearestListener)`.
3. If `dist <= minRolloff` → return `1.0`.
4. If `dist >= maxRolloff` → return `0.0`.
5. Else linear: `1.0 - (dist - minRolloff) / (maxRolloff - minRolloff)`.

Default `maxRolloff` is `6000.0f` (`0x45BB8000`).

---

## 9. Code example: a complete custom sound hook

```cpp
#include <windows.h>

// Type defs
using SoundList_Ctor_t      = void* (__thiscall*)(void*, void*);
using Sound_LoadOggOrWav_t  = void  (__thiscall*)(void*, const char*);
using Sound_PlayChannel_t   = void  (__fastcall*)(void*);
using Sound_Play3D_t        = void  (__thiscall*)(void*, float, float, float);
using operator_new_t        = void* (*)(size_t);

static SoundList_Ctor_t     SoundList_Ctor     = (SoundList_Ctor_t)    0x00458F20;
static Sound_LoadOggOrWav_t Sound_LoadOggOrWav = (Sound_LoadOggOrWav_t)0x00459660;
static Sound_PlayChannel_t  Sound_PlayChannel   = (Sound_PlayChannel_t) 0x004597B0;
static Sound_Play3D_t       Sound_Play3D        = (Sound_Play3D_t)      0x00459860;
static operator_new_t       operator_new        = (operator_new_t)      0x004BA57B;

void* g_CustomSound = nullptr;

void InitCustomSound(void* app) {
    if (g_CustomSound) return;
    void* list = operator_new(0x420);
    if (!list) return;
    SoundList_Ctor(list, app);
    Sound_LoadOggOrWav(list, "sounds\\MyCustom");
    g_CustomSound = list;
}

void PlayCustomSFX(void* app, float x, float y, float z) {
    InitCustomSound(app);
    if (!g_CustomSound) return;
    Sound_Play3D(g_CustomSound, x, y, z);
}

void PlayCustomUI(void* app) {
    InitCustomSound(app);
    if (!g_CustomSound) return;
    Sound_PlayChannel(g_CustomSound);
}
```

Drop `MyCustom.ogg` into `Sounds/MyCustom.ogg` (or the game root) and call `PlayCustomUI(app)` from any hook.

---

## 10. Reimplementation / replacement notes

If you are rebuilding the audio layer for a port:

- **BASS music**: replace with SDL_mixer, OpenAL, or miniaudio + libopenmpt / libxmp for `.mo3`/`.xm`/`.it`.
- **DirectSound8 SFX**: replace with SDL_mixer `Mix_Chunk` channels or OpenAL sources. For true 3D, OpenAL gives you the listener/distance model for free.
- **Channel pool**: the original uses a circular allocator across duplicated buffers. A modern replacement can just allocate dynamic channels.
- **OGG/WAV**: both are trivial with stb_vorbis / dr_wav or SDL_mixer.
- **Volume scale**: DirectSound uses dB attenuation (`-10000` to `0`). SDL_mixer uses `0..128`. Convert linear `0..1` → mixer volume.

---

## 11. Quick reference: call these addresses

| What you want | Call |
|---|---|
| Play existing SFX at full volume | `Sound_PlayChannel(App+offset)` (`0x004597B0`) |
| Play existing SFX at a 3D position | `Sound_Play3D(App+offset, x, y, z)` (`0x00459860`) |
| Load custom SFX at runtime | `SoundList_Ctor` → `Sound_LoadOggOrWav` (`0x00458F20`, `0x00459660`) |
| Play a BASS music track | `Audio_PlayMusic(App+0x53C, name)` (`0x0046A310`) |
| Stop BASS music | `Audio_StopChannel(channel)` (`0x0046A0D0`) |
| Load BASS file directly | `BASS_MusicLoad` (`0x004794B6`) |
| Read global SFX volume | `*(float*)(SoundDevice + 0x838)` |

---

*Generated from GhidraMCP decompilation of `Hamsterball.exe` at commit time. Addresses assume the default image base `0x00400000`. If you are hooking a relocated process, convert to RVA first: `RVA = addr - 0x400000`, then `real = GetModuleHandle(NULL) + RVA`.*
