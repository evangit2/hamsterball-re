# Save & Registry System

## Tournament Save File

**Path**: `DATA\TOURNAMENT.SAV`
**Format**: Binary, raw struct dump (no header/magic)

### Write Function: TourneyMenu_WriteSave (0x4264B0)
### Read Function: TourneyMenu_LoadSaveAndShow (0x4265A0)

The save file is a simple sequential binary write of PlayerProfile fields + App state:

| Offset (file) | Size | Source (PlayerProfile) | Description |
|---------------|------|----------------------|-------------|
| 0x00 | 4 | Profile+0x08 | current_race (1-14) |
| 0x04 | 4 | Profile+0x14 | (unknown int) |
| 0x08 | 0x3C | Profile+0x18 | race_time_array[15] (float × 15) |
| 0x44 | 0x3C | Profile+0x54 | race_time_array_2[15] (float × 15) |
| 0x80 | 4 | Profile+0x90 | accumulated_time_int |
| 0x84 | 1 | Profile+0x94 | difficulty_flag |
| 0x85 | 1 | Profile+0x95 | has_rollback_available |
| 0x86 | 1 | App+0x236 | is_mirror_mode |
| 0x87 | 4 | App+0x23C | race_active_flag |
| 0x8B | 4 | App+0x5E8 | total_time_float |
| 0x8F | 4 | App+0x5E4 | ranking_time_float |
| 0x93 | 4 | App+0x5F4 | (unknown App float) |
| | | | |
| **Total** | **~151 bytes** | | |

### Write Process
```
TourneyMenu_WriteSave(profile, "DATA\\TOURNAMENT.SAV"):
  1. CRT_OpenFileExclusive(path, 0x180)  // Create/truncate exclusive
  2. Close handle (file created empty)
  3. FID_open(path, 0x8002)  // Open for write (O_WRONLY|O_CREAT)
  4. Sequential __write() calls — raw struct fields
  5. __close()
```

### Read Process
```
TourneyMenu_LoadSaveAndShow(profile, "DATA\\TOURNAMENT.SAV"):
  1. FID_open(path, 0x8000)  // Open for read
  2. Sequential __read() calls — same fields in same order
  3. __close()
  4. Create TourneyMenu (0x111C bytes) with param_2=1 (play music)
  5. Scene_AddObject() — add tourney menu to scene
```

### Reimplementation Notes
The save format is fragile (no version, no magic, no checksum). For reimplementation:
- Add a magic header + version number
- Use JSON for debuggability
- Race time arrays are 15 floats each (one per race slot, some may be 0.0 for unplayed)
- Float sizes assumed to be IEEE 754 32-bit (4 bytes each)

## Registry System (Windows)

The game uses Windows Registry for persistent settings:
- Key path: Under `HKEY_CURRENT_USER\Software\Hamsterball` (or similar)
- `App+0x54` = Registry key handle

### Stored Values

| Registry Key | Type | Description | Default |
|-------------|------|-------------|---------|
| Sound Volume | DWORD/float | Master sound volume | 1.0 (0x3F800000) |
| Music Volume | DWORD/float | Music volume | 1.0 |
| (difficulty) | DWORD | Selected difficulty | 0 |

### Registry Functions
- `RegKey_Open(handle)` — Opens registry key
- `RegKey_Close(handle)` — Closes registry key
- `RegKey_ReadString(handle, name)` → bool — Check if value exists
- `Registry_ReadFloat(handle, name)` → float — Read float value
- `RegKey_WriteDWORD(handle, name, value)` — Write DWORD value
- `RegKeyList_AppendStr(name, hex_id)` — Append to registry string list

### Read on Startup
`Level_ReadSoundVolume` (0x466570):
```
Level_ReadSoundVolume(SoundDevice):
  RegKey_Open(app->registry)
  if RegKey_ReadString("Sound Volume"):
    device->volume = Registry_ReadFloat("Sound Volume")
  else:
    device->volume = 1.0
  RegKey_Close()
```

### Write on Shutdown
`SoundDevice_dtor` (0x4668A0):
```
SoundDevice_dtor(device):
  RegKey_Open(app->registry)
  RegKey_WriteDWORD("Sound Volume", device->volume)
  RegKey_Close()
  ... cleanup ...
```

## High Score System

High scores are gated behind registration:
- Demo version: "BUY HAMSTERBALL AND YOU CAN SAVE YOUR HIGH SCORES!"
- Full version: Scores saved to registry or local file
- No separate high score file found — likely stored in registry

## Demo vs Full Version Gates

The binary contains extensive demo limitation code:
- "You have reached the end of the demo version of Hamsterball!"
- Tournament save/continue only in full version
- High scores only in full version
- Registration check via name+serial (at http://www.raptisoft.com/buyhamsterball)
- `App+0x200` = is_registered flag
- `App+0x918` = has_mini_games flag (for "MG" menu button)

## Key Address Map

| Address | Function | Description |
|---------|----------|-------------|
| 0x4264B0 | TourneyMenu_WriteSave | Write TOURNAMENT.SAV |
| 0x4265A0 | TourneyMenu_LoadSaveAndShow | Read TOURNAMENT.SAV + show menu |
| 0x466570 | Level_ReadSoundVolume | Read volume from registry |
| 0x4668A0 | SoundDevice_dtor | Save volume to registry on exit |

## Reimplementation Notes (SDL2)

### Save System
```cpp
// Use JSON for save files
struct TournamentSave {
    int current_race;
    int unknown_field;
    float race_times[15];
    float race_times_alt[15];
    int accumulated_time;
    bool difficulty;
    bool has_rollback;
    bool is_mirror;
    bool race_active;
    float total_time;
    float ranking_time;
    float unknown_app_float;
};

// Save as JSON
void WriteSave(const TournamentSave& save, const char* path) {
    std::ofstream f(path);
    // Use nlohmann/json or simple serialization
    json j = { ... };
    f << j.dump(2);
}
```

### Registry Replacement
Replace Windows Registry with a simple INI/JSON config file:
```ini
[Settings]
SoundVolume=1.0
MusicVolume=1.0
Difficulty=0
Registered=true
MiniGamesUnlocked=true
MirrorTournamentUnlocked=false
```