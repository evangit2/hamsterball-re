// =============================================================================
// Audio/Music System — BASS + DirectSound8
// =============================================================================
//
// The Hamsterball audio system uses two APIs:
// 1. BASS library (un4seen.com) for music playback (.mo3/.xm/.it tracker modules)
// 2. DirectSound8 for sound effects (3D positioned sounds)
//
// =============================================================================
// BASS Music System
// =============================================================================
//
// LoadMusicFile (0x46A020):
//   __thiscall (ECX=this=MusicPlayer)
//   - Copies param_1 (filepath) into this+0x424 (internal path buffer)
//   - Calls BASS_MusicLoad(0, filepath, 0, 0, 4, 0)
//     BASS_MusicLoad params: mem=false, file=path, offset=0, length=0, flags=4, freq=0
//     Flag 4 = BASS_MUSIC_PRESCAN (pre-scan for accurate length)
//   - Stores result handle at this+8 (BASS music handle)
//   - If handle != 0: return (1 << 24) | handle (success)
//   - If handle == 0: BASS_ErrorGetCode(), Window_Notify with "Music Initialization Error: %s"
//
// LoadJukebox (0x46A4D0):
//   Parses jukebox.xml — an XML file with <SONG> entries
//   - Opens param_1 (filename) via FileHandle_Open
//   - Iterates XML tags via MWParser_ReadTag
//   - For each <SONG> tag:
//     - Reads inner tags: <NAME> (song name), <HEX> (hex ID via sscanf %x)
//     - Appends to jukebox list via RegKeyList_AppendStr(name, hex_id)
//   - Example jukebox.xml structure:
//     <SONG>
//       <NAME>music_mo3</NAME>
//       <HEX>1A</HEX>
//     </SONG>
//
// Music channels (App offsets):
//   App[0x534] = MusicPlayer* (BASS music handle + path at +0x424)
//   App[0x538] = music_channel_2 (BASS channel handle)
//   App[0x53C] = music_channel_1 (BASS channel handle)
//
// =============================================================================
// DirectSound8 System (3D Positional Audio)
// =============================================================================
//
// SoundDevice (at App+0x178):
//   Main sound device object. Contains:
//   - DirectSound buffer pool (circular, up to N channels)
//   - 3D listener positions (at SoundDevice+0x854)
//   - Sound volume (read from registry, default 1.0f)
//   - Min/max rolloff distances (at SoundDevice+0x914/0x918)
//
// Sound Channel Allocator:
//   Sound_GetNextChannel (0x459810): Circular buffer allocator
//   Returns next available channel from pool. Wraps around when full.
//
// Sound_Lifecycle:
//   1. Sound_LoadOggOrWav (0x459660): Try .ogg first, then .wav fallback
//   2. Sound_LoadOgg (0x459310): Load OGG Vorbis into DS buffer
//   3. SoundDevice_LoadWAV (0x458F40): Load WAV file → DirectSound buffer
//   4. Sound_StartSample (0x4595B0): Start BASS sample (vtable: reset, volume, 3D pos)
//   5. Sound_Play3D (0x459860): Play 3D positioned sound (set BASS position + play)
//   6. Sound_PlayChannel (0x4597B0): Play sound channel (pool dispatch)
//   7. Sound_CalculateDistanceAttenuation (0x466750):
//      Linear rolloff: full volume below min_distance, silent above max_distance
//      Find nearest listener from listener array at SoundDevice+0x854
//
// 3D Listener Array (at SoundDevice+0x854):
//   - listener_count at SoundDevice+0x850
//   - Each listener = Vec3 (12 bytes: X, Y, Z)
//   - Updated per-ball in Ball_Update Phase 17 (multiplayer sync)
//   - Used for distance attenuation in 3D sound system
//
// =============================================================================
// Sound Events During Gameplay
// =============================================================================
//
// In Ball_Update:
// - Phase 0: If ambient_sound_id != 0: Sound_Play3DAtPosition
// - Phase 12 (ball-ball collision): Sound_Play3D with collision sound
// - Phase 15: Sound_CalculateDistanceAttenuation at ball position
//            If ambient_sound_id != 0: play at current position
//
// Collision Events (GameObject_HandleCollision 0x40C5D0):
// - E:JUMP: plays 3D sound at collision point
// - N:TARPIT: plays 3D sound, then modifies ball physics
// - N:MOUSETRAP: plays 3D sound at collision point
// - PIPEBONK: plays random sound from 3 possible impact sounds
//
// Level Events (Level_HandleCollision 0x40DCD0):
// - E:CATAPULTBOTTOM: plays catapult launch sound
// - E:OPENSESAME: plays door opening sound
// - N:TRAPDOOR: plays 3D trapdoor sound + timer activation
// - E:MACETRIGGER: plays mace activation 3D sound
//
// =============================================================================
// Key Address Map
// =============================================================================
// 0x459660  Sound_LoadOggOrWav    Try .ogg, fallback .wav
// 0x459310  Sound_LoadOgg        Load OGG → DirectSound buffer
// 0x458F40  SoundDevice_LoadWAV  Load WAV → DirectSound buffer
// 0x4595B0  Sound_StartSample    Start BASS sample playback
// 0x459810  Sound_GetNextChannel Circular channel allocator
// 0x459860  Sound_Play3D         Play 3D positioned sound
// 0x4597B0  Sound_PlayChannel    Pool dispatch play
// 0x466750  Sound_CalculateDistanceAttenuation  Linear 3D rolloff
// 0x4668A0  SoundDevice_dtor     Release buffers, save volume to registry
// 0x466570  SoundDevice_ReadVolume  Read volume from registry (default 1.0f)
// 0x46A020  LoadMusicFile        BASS_MusicLoad wrapper
// 0x46A4D0  LoadJukebox          Parse jukebox.xml (<SONG> entries)
// 0x434580  Sound_InitChannels   Allocate channels + set timers
// 0x46A0D0  Audio_StopChannel   BASS_ChannelStop
//
// =============================================================================
// Reimplementation Notes (SDL2/SDL_mixer)
// =============================================================================
// - Replace BASS with SDL_mixer for music playback
// - Replace DirectSound8 with SDL_mixer Channels + SDL audio
// - 3D positional audio can be approximated with panning/volume control
//   or OpenAL for proper 3D positional audio
// - The jukebox.xml parser reads same format: <SONG><NAME>..</NAME><HEX>..</HEX></SONG>
// - OGG files can be loaded directly by SDL_mixer (no conversion needed)
// - WAV files load directly too (SoundDevice_LoadWAV just reads RIFF/WAVE format)
// - Channel allocation: SDL_mixer has 16+ channels, similar to circular buffer approach
// - Volume/attenuation: Implement linear rolloff similar to current system
//   full volume below min_rolloff, silent above max_rolloff