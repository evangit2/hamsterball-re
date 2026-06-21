# Audio System

## Overview
Hamsterball uses a dual audio system:
1. **BASS.dll** (shipped alongside exe) for music playback (MO3 tracker format)
2. **DirectSound** (dsound.dll) for 3D positional sound effects (OGG)

## BASS Music System

### API Calls
| Address | Function | Parameters |
|---------|----------|------------|
| 0x487E40 | BASS_SetConfig | Config options |
| 0x487E4C | BASS_Init | device=-1, freq=44100, flags=0, win=NULL |
| 0x487E46 | BASS_Start | Resume playback |
| 0x487E58 | BASS_Stop | Pause all playback |
| 0x487E52 | BASS_Free | Shutdown |
| 0x4794B6 | BASS_MusicLoad | Load .MO3 file |
| 0x4794C8 | BASS_MusicPlayEx | Play with flags |
| 0x4794C2 | BASS_ChannelSetAttributes | freq, volume, ramp |
| 0x4794BC | BASS_ChannelStop | Stop channel |
| 0x4794B0 | BASS_ErrorGetCode | Last error |

### MusicChannel Struct (0x534 bytes)
| Offset | Type | Field |
|--------|------|-------|
| +0x00 | vtable* | MusicChannel_DeletingDtor |
| +0x04 | DWORD | BASS device handle |
| +0x0C | AthenaList | Track list |
| +0x10 | int | Track count |
| +0x418 | AthenaList | Track data array |
| +0x528 | float | Current volume (1.0 default) |
| +0x530 | byte | Muted flag |
| +0x531 | byte | Paused flag |

### Music Playback
```
App_Initialize_Full step 7:
  music_channel = MusicChannel_LoadAndAppend(App+0x17C, "music\\music.mo3")
  
Audio_PlayMusic(channel, track_name):
  Linear search track list (this+0x418) for matching name
  Set volume = 1.0
  BASS_ChannelSetAttributes(handle, -1, 100, fade_ramp)
  BASS_MusicPlayEx(handle, track_handle, restart, flags)

Audio_PlayMusicAtSpeed(channel, track_name, speed):
  Same but with tempo control
  speed=4.0 for intro, speed=2.0 for normal

MusicPlayer_ctor (0x426030):
  Creates jukebox player for XML-driven music sequences

MusicPlayer_SetTempoScale (0x46A140):
  Adjusts playback speed (2.0=normal, 4.0=fast forward)

MusicDevice_FadeAll (0x474780):
  Fade all music channels simultaneously
```

### Jukebox XML Format
```
jukebox.xml contains song sequences:
  <jukebox>
    <song name="TrackName" file="music\track.mo3"/>
    ...
  </jukebox>

LoadJukebox("jukebox.xml") loads this at startup
```

## DirectSound SFX System

### SoundDevice Struct
Created at 0x466620 (SoundDevice_ctor).
Manages DirectSound buffer pool for 3D positional audio.

### Sound List
| Address | Function | Description |
|---------|----------|-------------|
| 0x458F20 | SoundList_Ctor | Sound list constructor |
| 0x59700 | SoundList_DtorInner | Cleanup inner data |
| 0x59570 | SoundEntry_ScalarDtor | Individual sound destructor |
| 0x466320 | SoundChannel_Ctor | DirectSound channel (0x80 bytes) |
| 0x466C90 | SoundBuffer_ScalarDtor | DSound buffer destructor |
| 0x466A10 | SoundDevice_UpdateChannels | Per-frame mix/update |
| 0x466B80 | SoundDevice_Play3DAll | 3D positional sound playback |
| 0x4ABAFAF | DSound_SetVolume | Volume control |

### 3D Sound Effects
Each sound effect is played at a 3D position:
- SceneObject_SpawnWithSound (0x439A30) - Play sound at object position
- Audio position comes from Scene+0x29C0 (camera distance)
- Sound attenuation: 3D falloff based on distance

### Level-Specific Sound Functions
| Address | Function | Description |
|---------|----------|-------------|
| 0x435B00 | CollisionLevel_PlayBreakSound | Collision break sound |
| 0x437300 | Lifter_PlaySound | Lifter mechanism sound |
| 0x436B70 | Pendulum_PlayCollisionSound | Pendulum impact sound |
| 0x434030 | Rotator_PlayCollisionSound | Rotator impact sound |
| 0x4367E0 | Rotator_StartSound | Rotator running sound |
| 0x436CF0 | Rotator_TriggerSound | Rotator trigger effect |
| 0x434AB0 | Sawblade_SetBreakSound | Sawblade break sound |
| 0x401090 | Scene_SetSoundMode | Toggle sound mode |

### Menu Sound
| Address | Function | Description |
|---------|----------|-------------|
| 0x475430 | Menu_AddSound | Play menu click/transition sound |

### Sound Volume
| Address | Function | Description |
|---------|----------|-------------|
| 0x466570 | Level_ReadSoundVolume | Read volume from registry |
| 0x474480 | MusicDevice_SetVolume | Set master music volume |
| 0x4744B0 | MusicDevice_ReadVolume | Read current volume |
| 0x474510 | MusicDevice_MuteToggle | Toggle mute |

## App Audio Initialization

App_CreateAudioDevice (0x46C0B0):
  Creates DirectSound device for SFX
  Allocates 16 channels for sound mixing

App_Initialize_Full step 7-11:
  - Load "music\\music.mo3" via BASS
  - Load "jukebox.xml" for track sequencing
  - RegKeyList_CopyFromSibling for music channel linking
  - Two music channels: primary (App+0x534) and secondary (App+0x53C/0x538)

Level_InitScene (0x40B090):
  - Create SoundChannel (0x80 bytes) for level SFX
  - Set volume to -50.0 dB initially
  - Play level music at speed 2.0 (normal) or 4.0 (intro skip)