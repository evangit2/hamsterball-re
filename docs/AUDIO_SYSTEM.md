# Audio System — Deep Documentation

## Architecture Overview

Hamsterball uses a dual-API audio system:
1. **BASS library** (un4seen.com) — music playback (.mo3/.xm/.it tracker modules)
2. **DirectSound8** — sound effects with 3D positional audio

Both systems are managed through the `SoundDevice` object stored at `App+0x178` (offset `0x84C` from SoundDevice's App pointer).

## SoundDevice Structure

The SoundDevice (vtable at `0x4D911C`) is the central audio manager:

| Offset | Size | Type | Description |
|--------|------|------|-------------|
| +0x000 | 4 | vtable* | SoundDevice vtable |
| +0x004 | 4 | App* | Back-pointer to App |
| +0x008 | 4 | int | Sound entry list count (AthenaList) |
| +0x00C | 4 | int | Sound entry list capacity |
| +0x010 | 4 | int | Next channel index (circular allocator) |
| +0x414 | 4 | int** | Sound entry array pointer |
| +0x838 | 4 | float | Global sound volume (default 1.0, registry "Sound Volume") |
| +0x83C | 1 | bool | Sound enabled flag |
| +0x84C | 4 | IDirectSound8* | DirectSound8 COM interface |
| +0x850 | 4 | int | Listener count |
| +0x854 | varies | Vec3[] | Listener position array (12 bytes each: X, Y, Z) |
| +0x914 | 4 | float | Min rolloff distance |
| +0x918 | 4 | float | Max rolloff distance |
| +0x107*4 | — | AthenaList | Secondary sound buffer list |

## Sound Loading Pipeline

### Entry Point: Sound_LoadOggOrWav (0x459660)
```
Sound_LoadOggOrWav(filename):
  1. Try "%s.ogg" — check file access
  2. If .ogg exists → Sound_LoadOgg()
  3. Else try "%s.wav" — check file access
  4. If .wav exists → SoundList_LoadWAV()
```

The game prefers OGG Vorbis over WAV, always trying .ogg first.

### SoundList_LoadWAV (0x458F40)
Manual WAV file parser — does NOT use DirectSound helpers:
1. Open file, read entire contents into memory
2. Validate "RIFF" + "WAVE" headers (4-char string compare)
3. Parse WAV format chunks: sample rate, channels, bits per sample
4. Scan for "data" chunk by iterating through file
5. Create IDirectSound8 buffer (WAVEFORMATEX + DSBUFFERDESC):
   - `DSBUFFERDESC.dwSize = 0x24`
   - `DSBUFFERDESC.dwFlags = 0x82` (DSBCAPS_STATIC | DSBCAPS_CTRLVOLUME)
   - `DSBUFFERDESC.dwBufferBytes = data_chunk_size`
6. Lock buffer (DSLock), copy PCM data, unlock
7. Create SoundEntry (12 bytes: vtable + App* + buffer_handle) per DirectSound buffer
8. Append to SoundList's AthenaList
9. For multi-buffer sounds (streaming): create additional buffers with duplicate entries

### Sound_LoadOgg (0x459310)
1. Open .ogg file with CRT_fsopen("rb")
2. Parse OGG Vorbis headers (channels, sample rate, bits)
3. Compute buffer size: channels × bits/8 × sample_count
4. Create IDirectSound8 buffer with computed format
5. Decode OGG stream into DirectSound buffer (read loop)
6. Create SoundEntry entries, append to AthenaList
7. For multi-buffer sounds: duplicate entries for streaming segments

### SoundEntry Structure (12 bytes)
| Offset | Size | Type | Description |
|--------|------|------|-------------|
| +0x000 | 4 | vtable* | SoundEntry vtable (scalar dtor at 0x4D8E78) |
| +0x004 | 4 | App* | Back-pointer to App |
| +0x008 | 4 | handle | DirectSound buffer handle |

## Sound Playback Pipeline

### Channel Management

**Sound_GetNextChannel** (0x459810) — Circular allocator:
```
Sound_GetNextChannel(SoundList):
  index = list.next_index
  if index < list.count:
    entry = list.entries[index]
    list.next_index = index + 1
    if entry != NULL: return entry
  // Wrap around
  list.next_index = 0
  if list.count > 0:
    return list.entries[0]
  return 0
```

### 3D Positional Audio

**Sound_CalculateDistanceAttenuation** (0x466750):
```
Sound_CalculateDistanceAttenuation(this, x, y, z) → float (0.0-1.0):
  // Find nearest listener
  best_dist = FLT_MAX
  for each listener in this.listeners:
    dist = sqrt((x-lx)² + (y-ly)² + (z-lz)²)
    if dist < best_dist: best_dist = dist
  
  // Linear rolloff between min and max distances
  if best_dist <= this.min_rolloff:
    return 1.0  // Full volume
  elif best_dist >= this.max_rolloff:
    return 0.0  // Silent
  else:
    range = max_rolloff - min_rolloff
    if range == 0.0: range = 1.0  // Avoid divide-by-zero
    return 1.0 - (best_dist - min_rolloff) / range
```

Key constants:
- `_DAT_004d9120` = FLT_MAX (initial distance for comparison)
- `_DAT_004cf368` = 0.0 (zero/silence)
- `_DAT_004cf310` = 1.0 (full volume)

**Sound_Play3D** (0x459860):
```
Sound_Play3D(this, x, y, z):
  Sound_CalculateDistanceAttenuation(this->device, x, y, z)
  Sound_PlayChannel(this)
```

**Sound_PlayChannel** (0x4597B0):
```
Sound_PlayChannel(SoundList):
  if !app || !device || volume == 0.0: return
  // Get next channel from circular list
  channel = next_entry_from_list
  if channel: Sound_StartSample(channel)
  else: wrap to beginning, try first entry
```

**Sound_StartSample** (0x4595B0):
```
Sound_StartSample(entry):
  if entry->app->sound_enabled:
    Reset sample via vtable(+0x48)       // IDirectSoundBuffer::SetCurrentPosition(0)
    Set volume via vtable(+0x3C)         // IDirectSoundBuffer::SetVolume(clamped)
    Play via vtable(+0x30, 0, 0, 0)     // IDirectSoundBuffer::Play(0, 0, 0)
```

### Sound_InitChannels (0x434580)
Used during game object initialization (e.g. sawblades):
```
Sound_InitChannels(this, randomize_pitch):
  this.sound_flag = 0
  this.channel_1 = Sound_GetNextChannel(this->app->sound_ids_0x4AC)
  this.channel_2 = Sound_GetNextChannel(this->app->sound_ids_0x4B0)
  if channel_1: Sound_StartSample(channel_1)
  this.pitch = 0x140 (320 decimal = base pitch/frequency)
  if channel_2: Sound_Play3DAtPosition(channel_2)
  if channel_1: Sound_Play3DAtPosition(channel_1)
  if randomize_pitch:
    this.float_pitch = RNG_Rand(0, 25)  // Random 0-25
  this.sound_playing = 0
```

## Music System (BASS)

### MusicPlayer Structure
| Offset | Size | Type | Description |
|--------|------|------|-------------|
| +0x000 | 4 | vtable* | MusicPlayer vtable |
| +0x004 | varies | — | Internal state |
| +0x008 | 4 | HMUSIC | BASS music handle |
| +0x424 | varies | char[] | Path buffer (filepath copy) |

### LoadMusicFile (0x46A020)
```
LoadMusicFile(this, filepath):
  strcpy(this+0x424, filepath)
  handle = BASS_MusicLoad(0, filepath, 0, 0, BASS_MUSIC_PRESCAN, 0)
  this->handle = handle
  if handle != 0:
    return (1 << 24) | handle  // Success
  else:
    BASS_ErrorGetCode()
    Window_Notify("Music Initialization Error: %s")
    return 0
```

BASS_MusicLoad flags: `4` = BASS_MUSIC_PRESCAN (pre-scan for accurate length/sync)

### LoadJukebox (0x46A4D0)
Parses `jukebox.xml` for song definitions:
```
LoadJukebox(filename):
  FileHandle_Open(filename)
  while tag = MWParser_ReadTag():
    if tag.name == "SONG":
      name = ""
      hex_id = 0
      while inner_tag = MWParser_ReadTag():
        if inner_tag.name == "NAME": name = inner_tag.value
        if inner_tag.name == "HEX": hex_id = sscanf(value, "%x")
      RegKeyList_AppendStr(name, hex_id)
```

Example jukebox.xml:
```xml
<SONG>
  <NAME>music_mo3</NAME>
  <HEX>1A</HEX>
</SONG>
```

### Audio_StopChannel (0x46A0D0)
```
Audio_StopChannel(music_player):
  BASS_ChannelStop(music_player->handle)
```

### Music Channels (App Offsets)
| Offset | Description |
|--------|-------------|
| App+0x534 | MusicPlayer* (BASS music handle + path at +0x424) |
| App+0x538 | music_channel_2 (BASS channel handle) |
| App+0x53C | music_channel_1 (BASS channel handle) |

## Sound Volume Management

### Level_ReadSoundVolume (0x466570)
```
Level_ReadSoundVolume(SoundDevice):
  RegKey_Open(app->registry)
  if RegKey_ReadString("Sound Volume") exists:
    volume = Registry_ReadFloat("Sound Volume")
    device->volume = volume
  else:
    device->volume = 1.0  // 0x3F800000
  RegKey_Close()
```

### SoundDevice_dtor (0x4668A0)
On destruction:
1. Save volume to registry: `RegKey_WriteDWORD("Sound Volume", device->volume)`
2. Iterate and delete all SoundEntry objects (AthenaList at +0x4)
3. Iterate and free secondary buffer list (AthenaList at +0x41C)
4. Release IDirectSound8 interface (COM Release at vtable+8)
5. Free listener position arrays (Vec3List_Free)

## 3D Listener System

The 3D listener array enables per-ball positional audio in multiplayer:
- Listener count at `SoundDevice+0x850`
- Each listener is a `Vec3` (12 bytes: X, Y, Z) starting at `SoundDevice+0x854`
- Updated per-ball in `Ball_Update` Phase 17 (multiplayer sync)
- Ball positions are the listener positions for distance calculations

## Sound Events During Gameplay

### Ball_Update Phases
| Phase | Sound Action |
|-------|-------------|
| 0 | Play ambient sound if `ambient_sound_id != 0` |
| 12 | Ball-ball collision: `Sound_Play3D` with collision sound |
| 15 | Calculate distance attenuation; replay ambient if active |

### Collision Event Sounds
| Event | Location | Description |
|-------|----------|-------------|
| E:JUMP | GameObject_HandleCollision (0x40C5D0) | Jump 3D sound at collision point |
| N:TARPIT | GameObject_HandleCollision | Tarpit 3D sound + physics modify |
| N:MOUSETRAP | GameObject_HandleCollision | Mousetrap 3D sound |
| PIPEBONK | GameObject_HandleCollision | Random from 3 impact sounds |
| E:CATAPULTBOTTOM | Level_HandleCollision (0x40DCD0) | Catapult launch sound |
| E:OPENSESAME | Level_HandleCollision | Door opening sound |
| N:TRAPDOOR | Level_HandleCollision | 3D trapdoor + timer activation |
| E:MACETRIGGER | Level_HandleCollision | Mace activation 3D sound |

## Key Address Map

| Address | Function | Description |
|---------|----------|-------------|
| 0x458F40 | SoundList_LoadWAV | Parse WAV → IDirectSound8 buffer |
| 0x459310 | Sound_LoadOgg | Decode OGG Vorbis → IDirectSound8 buffer |
| 0x4595B0 | Sound_StartSample | Start IDirectSound8 sample playback |
| 0x459660 | Sound_LoadOggOrWav | Try .ogg first, fallback .wav |
| 0x4597B0 | Sound_PlayChannel | Dispatch sound to channel pool |
| 0x459810 | Sound_GetNextChannel | Circular channel allocator |
| 0x459860 | Sound_Play3D | 3D positioned sound (attenuation + play) |
| 0x434580 | Sound_InitChannels | Init game object sound channels |
| 0x466570 | Level_ReadSoundVolume | Read volume from registry |
| 0x466750 | Sound_CalculateDistanceAttenuation | Linear 3D rolloff |
| 0x4668A0 | SoundDevice_dtor | Release buffers, save volume |
| 0x46A020 | LoadMusicFile | BASS_MusicLoad wrapper |
| 0x46A0D0 | Audio_StopChannel | BASS_ChannelStop |
| 0x46A4D0 | LoadJukebox | Parse jukebox.xml song list |

## Reimplementation Notes (SDL2/SDL_mixer or OpenAL)

### Replacement Strategy
- **BASS → SDL_mixer**: Load .mo3/.xm/.it via SDL_mixer with mikmod/timidity
- **DirectSound8 → SDL_mixer**: Use SDL_mixer channels (Mix_Chunk/Mix_Music)
- **Alternative**: OpenAL for true 3D positional audio with proper distance model

### Key Differences
1. **Channel allocation**: SDL_mixer has built-in channel management (Mix_GroupChannel), similar to circular allocator pattern
2. **3D audio**: SDL_mixer lacks native 3D — approximate with pan/volume or use OpenAL
3. **OGG/WAV loading**: Both natively supported by SDL_mixer (Mix_LoadWAV_RW)
4. **Volume control**: SDL_mixer uses 0-128 scale; DirectSound uses dB attenuation
5. **Music format**: .mo3 tracker modules need mikmod/timidity — SDL_mixer supports these natively

### Recommended Architecture
```cpp
// Audio system (SDL_mixer + OpenAL for 3D)
class SoundDevice {
    std::vector<SoundEntry> entries;
    float volume;           // 0.0-1.0
    float min_rolloff;      // 3D min distance
    float max_rolloff;      // 3D max distance
    std::vector<Vec3> listeners;  // Per-ball positions
    
    SoundEntry* LoadOggOrWav(const char* filename);
    void Play3D(SoundEntry* entry, Vec3 position);
    float CalculateDistanceAttenuation(Vec3 position);
};

class MusicPlayer {
    Mix_Music* handle;
    std::string filepath;
    
    uint32_t LoadMusicFile(const char* filepath);
    void Stop();
};
```