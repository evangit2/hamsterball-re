# Hamsterball Sound Effects System

## Overview

Hamsterball uses **two completely separate audio systems**:

| System | Backend | Purpose | App Offset |
|--------|---------|---------|------------|
| **Music** | BASS.dll (un4seen) | Background music (.mo3/.mod) | App+0x17C (music_device) |
| **Sound Effects (SFX)** | DirectSound (DSOUND.dll) | All in-game sound effects (.ogg/.wav) | App+0x178 (sound_device) |

This document covers the **sound effects (SFX) system** only.

---

## Architecture

### SoundDevice (0x91C+ bytes)

The central SFX manager. Created by `App_CreateAudioDevice` (0x0046C0B0), which allocates 0x424 bytes for the MusicDevice (BASS), but the SoundDevice is created separately.

**SoundDevice struct layout (key offsets):**

| Offset | Type | Field | Notes |
|--------|------|-------|-------|
| +0x000 | void* | vtable | → 0x4D911C (SoundDevice vtable) |
| +0x004 | AthenaList | sound_channels | List of all loaded SoundChannel objects |
| +0x41C | AthenaList | active_queue | Pending/repeating sounds queue |
| +0x828 | AthenaList* | active_buffers | Pointer to active buffer list |
| +0x834 | void* | app_ptr | Back-pointer to App struct |
| +0x838 | float | volume | Master SFX volume (default 1.0, 0x3F800000) |
| +0x83C | byte | enabled | Sound enabled flag (1=active) |
| +0x840 | int | buffer_count | Default buffer count (0) |
| +0x844 | float | rolloff_factor | Distance rolloff (0xC5FA0000 = -8000.0?) |
| +0x848 | int | max_listeners | Listener count (4) |
| +0x84C | IDirectSound8* | dsound | DirectSound interface pointer |
| +0x850 | int | listener_count | Active 3D listener count |
| +0x854 | float[16][3] | listener_positions | Up to 16 listener positions (x,y,z) |
| +0x914 | float | min_distance | Min distance for attenuation (0.0) |
| +0x918 | float | max_distance | Max distance for attenuation (0x45BB8000 = 6000.0) |

### SoundChannel (0x420 bytes)

Each loaded sound effect. Created by `Sound_LoadAndAppend` (0x004664F0).

**SoundChannel struct layout (key offsets):**

| Offset | Type | Field | Notes |
|--------|------|-------|-------|
| +0x000 | void* | vtable | → 0x4D90E8 (SoundChannel vtable) |
| +0x004 | SoundDevice* | device | Parent SoundDevice pointer |
| +0x008 | AthenaList | buffers | List of DSound buffer clones |
| +0x00C | int | buffer_count | Number of buffer clones |
| +0x010 | int | next_buffer_idx | Round-robin index for next playback |
| +0x414 | int** | buffer_array_ptr | Pointer to AthenaList's internal data array |
| +0x418 | AthenaList* | buffer_list_ptr | Direct pointer to buffer list |
| +0x41C | int | loaded_flag | Whether sound data was loaded successfully |

### SoundBuffer (0x0C bytes)

A single DSound buffer wrapper. Created during `Sound_LoadOgg`.

| Offset | Type | Field |
|--------|------|-------|
| +0x000 | void* | vtable (→ 0x4D8E78) |
| +0x004 | IDirectSoundBuffer8* | buffer |
| +0x008 | SoundDevice* | device |

---

## Sound Loading

### Loading Order

All 61 sound effects are loaded during scene initialization in `TimerDisplay` (0x004298C0). This function loads textures, meshes, levels, and sounds via the resource manager's vtable methods:

- **vtable[0x48]** = Load texture (param: path, format)
- **vtable[0x4C]** = Load mesh (param: path)
- **vtable[0x50]** = Load level geometry (param: path)
- **vtable[0x54]** = Link level data
- **vtable[0x58]** = Load PNG texture (param: path)
- **vtable[0x5C]** = Load font (param: path)
- **vtable[0x60]** = **Load sound** (param: path, buffer_count)

The sound loading call signature is:
```c
resourceManager->vtable[0x60](board + offset, "sounds\\name", buffer_count);
```

### Sound File Format

`Sound_LoadOggOrWav` (0x00459660) tries `.ogg` first, then falls back to `.wav`:

```c
void Sound_LoadOggOrWav(void *channel) {
    char path[256];
    sprintf(path, "%s.ogg", channel->name);
    if (check_file_access(path, 0) == 0) {
        Sound_LoadOgg(channel, path);  // OGG decoder
        return;
    }
    sprintf(path, "%s.wav", channel->name);
    if (check_file_access(path, 0) == 0) {
        Sound_LoadWav(channel, path);  // WAV loader
    }
}
```

### OGG Decoding

`Sound_LoadOgg` (0x00459310) uses an OGG Vorbis decoder (functions at 0x00478xxx range, likely vorbisfile or custom decoder). The decoded PCM data is uploaded to DirectSound buffers via:

1. Create DSound buffer with WAVEFORMATEX (channels, samples/sec, bits per sample)
2. Lock buffer, write PCM data, unlock
3. Create buffer clones for overlapping playback
4. Append each clone to the SoundChannel's AthenaList

---

## Complete Sound Catalog

All 61 sound effects loaded in `TimerDisplay` (0x004298C0). Board offsets are relative to the Board struct.

### Board Sound Channel Offsets

| Board Offset | Sound Name | Buffers | Used By |
|-------------|------------|---------|---------|
| +0x43C | sounds\collide | 10 | Ball collision (general) |
| +0x440 | sounds\roll | 10 | Ball rolling |
| +0x444 | sounds\whistle | 1 | Tournament/arena whistle |
| +0x448 | sounds\bumper | 10 | Bumper collision |
| +0x44C | sounds\ballbreak | 5 | Ball shatter (full) |
| +0x450 | sounds\ballbreaksmall | 5 | Ball shatter (small) |
| +0x454 | sounds\thwomp | 2 | Heavy collision |
| +0x458 | sounds\snap | 2 | Rotator collision |
| +0x45C | sounds\popup | 2 | Popup sign |
| +0x460 | sounds\dropin | 2 | Drop-in (E:DROPIN) |
| +0x464 | sounds\dropinshort | 2 | Short drop-in |
| +0x468 | sounds\popout | 2 | Pop-out (E:POPOUT) |
| +0x46C | sounds\pipebump1 | 10 | Pipe collision variant 1 |
| +0x470 | sounds\pipebump2 | 10 | Pipe collision variant 2 |
| +0x474 | sounds\pipebump3 | 10 | Pipe collision variant 3 |
| +0x478 | sounds\gearclank | 20 | Gear collision |
| +0x47C | sounds\bridgeslam | 2 | Breakbridge slam |
| +0x480 | sounds\platformtick | 5 | Platform tick |
| +0x484 | sounds\gluestuck | 5 | Glue/tar stuck (N:TARPIT) |
| +0x488 | sounds\bubble1 | 5 | Tar bubble variant 1 |
| +0x48C | sounds\bubble2 | 5 | Tar bubble variant 2 |
| +0x490 | sounds\wheelcreak | 2 | Wheel creak |
| +0x494 | sounds\catapult | 2 | Catapult launch |
| +0x498 | sounds\trapdoor | 2 | Trapdoor |
| +0x49C | sounds\fwing | 2 | Jump (E:JUMP) |
| +0x4A0 | sounds\clink | 3 | Light collision |
| +0x4A4 | sounds\whoosh | 3 | Whoosh |
| +0x4A8 | sounds\chomp | 1 | Chomp (Blockdawg) |
| +0x4AC | sounds\fan-start | 10 | Fan startup |
| +0x4B0 | sounds\fan-blow | 10 | Fan blowing |
| +0x4B4 | sounds\crack | 2 | Glass crack |
| +0x4B8 | sounds\crumble | 2 | Crumble |
| +0x4BC | sounds\sawstartup | 2 | Saw startup |
| +0x4C0 | sounds\sawcut | 2 | Saw cutting |
| +0x4C4 | sounds\minipop | 5 | Mini pop |
| +0x4C8 | sounds\bell | 3 | Bell (bonus) |
| +0x4CC | sounds\zip | 2 | Zip (E:ZIP) |
| +0x4D0 | sounds\ting | 20 | Ting |
| +0x4D4 | sounds\shrink | 3 | Ball shrink |
| +0x4D8 | sounds\grow | 3 | Ball grow (E:GROWSOUND) |
| +0x4DC | sounds\tweet | 3 | Tweet (bird) |
| +0x4E0 | sounds\creakyplatform | 20 | Creaky platform |
| +0x4E4 | sounds\wubba | 5 | Wubba |
| +0x4E8 | sounds\saw | 2 | Saw running |
| +0x4EC | sounds\sawspeedy | 2 | Saw speedup |
| +0x4F0 | sounds\dawgstep1 | 10 | Blockdawg step variant 1 |
| +0x4F4 | sounds\dawgstep2 | 10 | Blockdawg step variant 2 |
| +0x4F8 | sounds\dawgsmash | 10 | Blockdawg smash |
| +0x4FC | sounds\sizzle | 2 | Sizzle |
| +0x500 | sounds\explode | 3 | Explode |
| +0x504 | sounds\vac-o-sux | 3 | Vacuum (vac-o-sux) |
| +0x508 | sounds\speedcylinder | 2 | Speed cylinder |
| +0x50C | sounds\bonuspop | 5 | Bonus pop |
| +0x510 | sounds\buzzbonus | 1 | Buzz bonus |
| +0x514 | sounds\breakbridge | 1 | Break bridge |
| +0x518 | sounds\unlock | 1 | Unlock (secret) |
| +0x51C | sounds\NeonRide | 1 | Neon ride |
| +0x520 | sounds\NeonFlicker | 50 | Neon flicker |
| +0x524 | sounds\ZoopDown | 2 | Zoop down |
| +0x528 | sounds\LightsOff | 2 | Lights off |
| +0x52C | sounds\GlassBonus | 2 | Glass bonus |

### Buffer Count Meaning

The second parameter to vtable[0x60] is the **buffer count** — how many DSound buffer clones are created for that sound. This allows **overlapping playback** (same sound played multiple times simultaneously before the first finishes). Sounds that play frequently during gameplay (collide, roll, pipebump) get 10 buffers; one-shot sounds (whistle, chomp) get 1.

---

## Sound Playback Pipeline

### 1. Sound_Play3D (0x00459860) — Positional 3D Sound

```c
void Sound_Play3D(SoundChannel *this, float x, float y, float z) {
    Sound_CalculateDistanceAttenuation(this->device, x, y, z);
    Sound_PlayChannel(this);
}
```

Calculates distance-based volume attenuation from listener positions, then plays the next available buffer.

### 2. Sound_CalculateDistanceAttenuation (0x00466750)

Iterates through up to 16 listener positions (stored at device+0x854 as float[16][3]). For each listener:
- Computes 3D Euclidean distance to the sound source
- Finds the **minimum** distance across all listeners

Then applies a linear attenuation model:
- If distance ≤ `min_distance` (device+0x914, default 0.0): **full volume** (1.0)
- If distance ≥ `max_distance` (device+0x918, default 6000.0): **silence** (0.0)
- Between min and max: linear interpolation: `1.0 - (distance - min) / (max - min)`

The calculated volume is stored and used by `Sound_PlayChannel` to set the buffer volume.

### 3. Sound_PlayChannel (0x004597B0) — Round-Robin Playback

```c
void Sound_PlayChannel(SoundChannel *channel) {
    SoundDevice *dev = channel->device;
    if (dev == NULL || dev->dsound == NULL || dev->volume == 0.0f)
        return;  // Sound disabled or muted

    // Try next buffer in round-robin order
    int idx = channel->next_buffer_idx;
    if (idx < channel->buffer_count) {
        buffer = channel->buffer_array[idx];
        channel->next_buffer_idx = idx + 1;
        if (buffer != NULL) goto play;
    }

    // Wrap around to first buffer
    buffer = AthenaList_SetIndex(&channel->buffers, 0);
    if (buffer != NULL) goto play;
    return;

play:
    Sound_StartSample(buffer);
}
```

### 4. Sound_StartSample (0x004595B0) — DSound Buffer Playback

```c
void Sound_StartSample(SoundBuffer *buf) {
    SoundDevice *dev = buf->device;
    if (dev->enabled) {
        // vtable[0x48] = IDirectSoundBuffer8::SetCurrentPosition(0)
        buf->vtable[0x48](buf);  // Rewind to start
        // Set pan value via Audio_ClampPanValue
        int pan = Audio_ClampPanValue();
        buf->vtable[0x3C](buf, pan);  // SetPan
        // vtable[0x30] = IDirectSoundBuffer8::Play(0, 0, 0)
        buf->vtable[0x30](buf, 0, 0, 0);  // Play (not looping)
    }
}
```

### 5. Audio_ClampPanValue (0x004665E0)

Calculates and claps stereo panning. Values below -2000 are clamped to -10000 (full left). The pan value is derived from the sound's 3D position relative to the listener.

### 6. Sound_Play3DAtPosition (0x00458EE0)

Updates the pan value for a specific buffer based on its 3D position, then calls the DSound `SetPan` method via vtable[0x3C].

---

## Sound Triggering in Gameplay

### Collision Events (DispatchCollisionEvents, 0x0040C5D0)

The main collision dispatch function checks event names stored at `collision_obj+0x864` and plays the corresponding sound:

| Event Name | Sound Played | Board Offset | Cooldown |
|-----------|---------------|-------------|----------|
| E:JUMP | sounds\fwing | +0x49C | param_1[0x1F7] = 10 |
| E:DROPIN | sounds\dropin | +0x460 | param_1[0x1F2] = 0x32 |
| E:PIPEBONK | sounds\pipebump{1-3} | +0x46C+rand*4 | param_1[500] = 10 |
| E:POPOUT | sounds\popout | +0x468 | param_1[499] = 0x32 |
| E:ZIP | sounds\zip | +0x4CC | param_1[0x1F5] = 0x32 |
| N:TARPIT | sounds\gluestuck | +0x484 | param_1[0xB3] (sticky flag) |
| E:GROWSOUND | sounds\grow | (via Ball_Shrink) | — |

**PIPEBONK** randomly selects between pipebump1, pipebump2, pipebump3 via `CPUID_CheckProcessorFeature` (which is actually a renamed `rand()` wrapper) — index 0, 1, or 2.

### Object-Specific Sound Functions

- **Rotator_PlayCollisionSound** (0x00434030): Plays `sounds\snap` (board+0x458) via `Sound_PlayChannel`
- **Rotator_StartSound** (0x004367E0): Plays `sounds\trapdoor` (board+0x498) via `Sound_Play3D` with rotator position
- **Pendulum_PlayCollisionSound** (0x00436B70): Plays `sounds\speedcylinder` (board+0x508) via `Sound_Play3D`
- **Ball_Update** (0x00405E00): Plays roll/collide sounds based on ball state
- **Ball_Shatter** (0x00408D70): Plays `sounds\ballbreak` (board+0x44C)
- **Bell_Activate** (0x00434E20): Plays `sounds\bell` (board+0x4C8)

### SoundDevice_Play3DAll (0x00466B80)

Iterates through ALL SoundChannels in the device and plays each one at its configured 3D position. Used for batch playback (e.g., arena collisions).

### SoundDevice_UpdateChannels (0x00466A10)

Per-frame update of active sound queue. Handles repeating sounds:
- Each queued sound has a countdown timer (`_Memory[2]`)
- When countdown reaches 0, replays the sound and resets timer to `_Memory[3]`
- If the repeat count drops below threshold, removes the sound from the queue

---

## Volume Control

### Level_ReadSoundVolume (0x00466570)

Reads the SFX volume from the Windows registry:

```c
void Level_ReadSoundVolume(SoundDevice *dev) {
    RegKey_Open(dev->app_ptr->registry_key);
    if (RegKey_ReadString(registry_key, "Sound Volume")) {
        dev->volume = Registry_ReadFloat(registry_key, "Sound Volume");
    } else {
        dev->volume = 1.0f;  // Default full volume
    }
    RegKey_Close(registry_key);
}
```

### DSound_SetVolume (0x004ABFAF)

Sets the DSound buffer volume via IDirectSoundBuffer::SetVolume, using DSBVOLUME format (0 = full, -10000 = silence, in hundredths of a decibel).

### Scene_SetSoundMode (0x00401090)

Sets the sound mode via the input device vtable[200] (offset 0xC8):
- Mode 0: Normal
- Mode 1: Reduced (maps to 1)
- Mode 2: Silent (maps to 3 if `scene+0x7D2` flag is set)
- Mode 3: Muted (maps to 2)

Stored at `scene+0x708`.

---

## Key Function Reference

| Function | Address | Description |
|----------|---------|-------------|
| SoundDevice_ctor | 0x00466620 | Initialize SoundDevice, create DSound interface |
| SoundDevice_dtor | 0x004668A0 | Destroy SoundDevice, release DSound |
| SoundDevice_Play3DAll | 0x00466B80 | Play all queued 3D sounds |
| SoundDevice_UpdateChannels | 0x00466A10 | Per-frame active sound update |
| Sound_LoadAndAppend | 0x004664F0 | Create SoundChannel, load file, append to list |
| Sound_LoadOggOrWav | 0x00459660 | Try .ogg then .wav file format |
| Sound_LoadOgg | 0x00459310 | Decode OGG Vorbis, create DSound buffers |
| Sound_Play3D | 0x00459860 | Play sound at 3D position with attenuation |
| Sound_Play3DAtPosition | 0x00458EE0 | Update pan for 3D positioned buffer |
| Sound_PlayChannel | 0x004597B0 | Round-robin buffer selection + play |
| Sound_StartSample | 0x004595B0 | Rewind + set pan + play DSound buffer |
| Sound_GetNextChannel | 0x00459810 | Get next available buffer (round-robin) |
| Sound_CalculateDistanceAttenuation | 0x00466750 | 3D distance-based volume calculation |
| Sound_InitChannels | 0x00434580 | Initialize saw/rotator sound channels |
| Audio_ClampPanValue | 0x004665E0 | Calculate and clamp stereo pan |
| DSound_SetVolume | 0x004ABFAF | Set DSound buffer volume |
| Level_ReadSoundVolume | 0x00466570 | Read SFX volume from registry |
| Scene_SetSoundMode | 0x00401090 | Set sound mode (normal/reduced/muted) |
| TimerDisplay | 0x004298C0 | Scene init — loads all textures, meshes, sounds |

---

## DirectSound Interface

The game links against `DSOUND.dll` (import table at 0x4F612A). The `DllEntryPoint` call in `SoundDevice_ctor` creates the DirectSound interface via `DirectSoundCreate8` (or similar — Ghidra decompiles it as `DllEntryPoint` due to the import thunk).

The DSound interface is stored at `SoundDevice+0x84C`. Sound buffers use the standard IDirectSoundBuffer8 vtable:

| Vtable Offset | Method | Used For |
|--------------|--------|----------|
| +0x00 | QueryInterface | — |
| +0x04 | AddRef | — |
| +0x08 | Release | — |
| +0x0C | GetCapabilities | — |
| +0x10 | GetStatus | — |
| +0x14 | GetFormat | — |
| +0x18 | GetVolume | — |
| +0x1C | GetPan | — |
| +0x20 | GetFrequency | — |
| +0x24 | GetStatus | — |
| +0x28 | SetFormat | — |
| +0x2C | SetVolume | DSound_SetVolume uses this |
| +0x30 | Play | Sound_StartSample calls this |
| +0x34 | SetFrequency | — |
| +0x38 | Stop | Audio_StopChannel calls this |
| +0x3C | SetPan | Sound_StartSample / Sound_Play3DAtPosition |
| +0x40 | SetCurrentPosition | — |
| +0x44 | GetCurrentPosition | — |
| +0x48 | SetCurrentPosition(0) | Sound_StartSample rewinds to start |
| +0x4C | Unlock | Sound_LoadOgg uses this |

---

## Modding Notes

### Adding Custom Sounds

1. Place `.ogg` or `.wav` files in the `sounds\` directory
2. The game will find them via the `sounds\<name>` path references
3. Custom level MESHWORLD files can reference custom sound names in collision events

### Changing Sound Volume at Runtime

Patch `SoundDevice+0x838` (float) to change master SFX volume (0.0 = muted, 1.0 = full).

### Disabling Specific Sounds

Set the SoundChannel's buffer count to 0, or null out the SoundChannel pointer at the corresponding Board offset (e.g., Board+0x49C for jump sounds).

### 3D Audio Attenuation

Modify `SoundDevice+0x914` (min_distance, float) and `SoundDevice+0x918` (max_distance, float) to change how quickly sounds fade with distance. Default max is 6000.0 game units.
