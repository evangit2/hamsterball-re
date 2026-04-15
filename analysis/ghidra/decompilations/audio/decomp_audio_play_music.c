/*
 * Audio_PlayMusic decompilation (0x46A310)
 * Plays a named track from the music channel's loaded track list
 * Uses BASS_MusicPlayEx for MO3 module playback
 */

void Audio_PlayMusic(void *this, char *track_name) {
    // Search through loaded music tracks (AthenaList at this+0x418)
    int index = 0;
    undefined4 *track = NULL;
    
    if (*(int*)(this + 0x10) < 1) {
        return;  // No tracks loaded
    }
    
    // Linear search for track name
    track = **(undefined4***)(this + 0x418);
    while (1) {
        if (track == NULL) return;
        
        // Track struct: +0x0 = name string, +0x4 = BASS music handle
        if (__stricmp((char*)track[0], track_name) == 0) {
            break;  // Found it
        }
        
        index++;
        if (index >= *(int*)(this + 0x10)) {
            return;  // Not found
        }
        
        track = *(undefined4**)(*(int*)(this + 0x418) + index * 4);
        if (track == NULL) return;
    }
    
    // Play found track
    uint music_handle = track[1];
    
    // Reset volume to 1.0 (0x3F800000)
    *(float*)(this + 0x528) = 1.0f;
    
    // BASS: set channel attributes (freq=-1=preserve, vol=100%, ramp)
    BASS_ChannelSetAttributes(*(uint*)(this + 8), -1, 100, 0xFFFFFF9B);
    
    // BASS: play with restart flag
    BASS_MusicPlayEx(*(uint*)(this + 8), music_handle, -1, TRUE);
}

/*
 * MusicChannel_Ctor (0x469FE0)
 * Initializes a music channel with track list
 * 
 * MusicChannel struct (0x534 bytes):
 *   +0x00: vtable (MusicChannel_DeletingDtor)
 *   +0x04: BASS device handle (param_1)
 *   +0x0C: AthenaList (track list)
 *   +0x528: volume (1.0f default)
 *   +0x530: muted flag (byte)
 *   +0x531: paused flag (byte)
 */

/*
 * Audio_PlayMusicAtSpeed (0x46A440)
 * Same as Audio_PlayMusic but with tempo control
 * Uses BASS_ChannelSetAttributes with specific frequency
 */

/*
 * MusicChannel_FadeUpdate (0x46A180)
 * Gradually adjusts volume toward target for fade in/out
 * Called each frame during transitions
 */

/*
 * MusicDevice struct and functions:
 *   MusicDevice_Ctor: Creates device with BASS_Init
 *   MusicDevice_dtor (0x4746A0): Cleanup
 *   MusicDevice_DeletingDtor (0x4747E0): Delete + dtor
 *   MusicDevice_SetVolume (0x474480): Master volume
 *   MusicDevice_ReadVolume (0x4744B0): Read current volume
 *   MusicDevice_MuteToggle (0x474510): Toggle mute
 *   MusicDevice_FadeAll (0x474780): Fade all channels
 */

/*
 * BASS API Calls Used:
 *   BASS_Init(0x487E4C) - Init with device -1 (default), 44100Hz
 *   BASS_MusicLoad(0x4794B6) - Load MO3 file
 *   BASS_MusicPlayEx(0x4794C8) - Play music module
 *   BASS_ChannelSetAttributes(0x4794C2) - Set freq/vol/ramp
 *   BASS_ChannelStop(0x4794BC) - Stop channel
 *   BASS_Start(0x487E46) - Resume playback
 *   BASS_Stop(0x487E58) - Pause all playback
 *   BASS_Free(0x487E52) - Shutdown
 *   BASS_ErrorGetCode(0x4794B0) - Get last error
 *   BASS_SetConfig(0x487E40) - Set config option
 *   
 * LoadMusicFile (0x46A020) - Opens MO3 file via BASS_MusicLoad
 * MusicPlayer_ctor (0x426030) - Creates jukebox player
 * MusicPlayer_Render (0x426150) - Per-frame music update
 * MusicPlayer_SetTempoScale (0x46A140) - Adjust playback speed
 */