# Save, Config & Registry System

## Overview
Hamsterball uses a dual persistence system:
1. **Registry** (ADVAPI32) for game settings and unlock flags
2. **Files** (DATA\/*.sav) for tournament progress

Both are loaded at startup (App_Initialize step 12-13) and saved at shutdown.

## App_SaveAllConfig (0x4284C0)

Called when game exits or settings change. Writes to Windows registry:

```
Registry Path: HKEY_CURRENT_USER\/software\reapti\banana (likely Hamsterball)
  → Based on RegKey_SetSoftwarePath (0x472F50) called from startup

Fields written:
  MouseSensitivity     DWORD  (App+0x84C, float as DWORD)
  MirrorTournament     BOOL   (App+0x850)
  
  // Race unlock flags (App+0x851..0x859)
  DizzyRace            BOOL
  TowerRace            BOOL
  UpRace               BOOL
  ExpertRace           BOOL
  OddRace              BOOL
  ToobRace             BOOL
  WobblyRace           BOOL
  SkyRace              BOOL
  MasterRace           BOOL
  
  // Arena unlock flags (App+0x85A..0x868)
  DizzyArena           BOOL
  TowerArena           BOOL
  UpArena              BOOL
  ExpertArena          BOOL
  NeonRace             BOOL   // Note: Neon is race, not arena
  GlassRace            BOOL
  ImpossibleRace       BOOL
  NeonArena            BOOL
  GlassArena           BOOL
  ImpossibleArena      BOOL
  OddArena             BOOL
  ToobArena            BOOL
  WobblyArena          BOOL
  SkyArena             BOOL
  MasterArena          BOOL
  
  // Binary blobs (0x50 = 80 bytes each)
  BestTime             BINARY (App+0x86C) - array of best times per level
  Medals               BINARY (App+0x8BC) - array of medal status per level
  
  // 2P controller mappings
  2PController1        DWORD  (App+0xB28)
  2PController2        DWORD  (App+0xB2C)
  2PController3        DWORD  (App+0xB30)
  2PController4        DWORD  (App+0xB34)
```

## App Struct Field Layout (Config)

| Offset | Type | Description |
|--------|------|-------------|
| App+0x54 | RegKey* | Registry handle |
| App+0x84C | float | MouseSensitivity |
| App+0x850 | bool | MirrorTournament |
| App+0x851 | bool | DizzyRace |
| App+0x852 | bool | TowerRace |
| App+0x853 | bool | UpRace |
| App+0x854 | bool | ExpertRace |
| App+0x855 | bool | OddRace |
| App+0x856 | bool | ToobRace |
| App+0x857 | bool | WobblyRace |
| App+0x858 | bool | SkyRace |
| App+0x859 | bool | MasterRace |
| App+0x85A | bool | DizzyArena |
| App+0x85B | bool | TowerArena |
| App+0x85C | bool | UpArena |
| App+0x85D | bool | ExpertArena |
| App+0x85E | bool | OddArena |
| App+0x85F | bool | ToobArena |
| App+0x860 | bool | WobblyArena |
| App+0x861 | bool | SkyArena |
| App+0x862 | bool | MasterArena |
| App+0x863 | bool | NeonRace |
| App+0x864 | bool | GlassRace |
| App+0x865 | bool | ImpossibleRace |
| App+0x866 | bool | NeonArena |
| App+0x867 | bool | GlassArena |
| App+0x868 | bool | ImpossibleArena |
| App+0x86C | uint8[0x50] | BestTime (per-level best times, in milliseconds) |
| App+0x8BC | uint8[0x50] | Medals (per-level medal status: 0=none, 1=bronze, 2=silver, 3=gold) |
| App+0xB28 | DWORD | 2PController1 (DirectInput device index) |
| App+0xB2C | DWORD | 2PController2 |
| App+0xB30 | DWORD | 2PController3 |
| App+0xB34 | DWORD | 2PController4 |

## LoadOrSaveConfig (0x4279F0)

Called at shutdown. Frees all config-related resources:
- Closes registry (RegKey_Close)
- Frees music channels (vtable[0x243], vtable[0x244])
- Saves to DATA\/*.sav files via vtable[0x8C]+8
- Clears all level data at App+0x88..0x9D
- Calls ShellExecuteA to open raptisoft.com if no reg key flag set (0x80)
- Calls App_Shutdown to close window

## Tourney_SaveTournament (0x446730)

Saves tournament progress to DATA\tournament.sav:
```
1. vtable[0x40]() - flush pending writes
2. CRT_remove() - delete existing file
3. vtable[0x54](&DAT_004d48a0) - write tournament save data
```

DAT_004D48A0 is a global string constant containing the save format signature.

## Registry Functions

| Address | Name | Description |
|---------|------|-------------|
| 0x472EA0 | RegKey_Ctor | Constructor |
| 0x472EC0 | RegKey_Open | Open registry key |
| 0x472F30 | RegKey_Close | Close registry key |
| 0x472F50 | RegKey_SetSoftwarePath | Set software name for key path |
| 0x472FD0 | RegKey_WriteDword | Write DWORD value |
| 0x473000 | RegKey_WriteDWORD | Write DWORD (alternate) |
| 0x473050 | RegKey_WriteBool | Write boolean |
| 0x473080 | RegKey_ReadDword | Read DWORD |
| 0x4730A0 | RegKey_ReadDword | Read DWORD (alternate) |
| 0x473130 | RegKey_ReadBool | Read boolean |
| 0x473170 | RegKey_ReadString | Read string |
| 0x473100 | RegKey_QueryValue | Query registry value |
| 0x473220 | RegKey_DeletingDtor | Deleting destructor |
| 0x46A0E0 | RegKeyList_AppendStr | Append string to list |
| 0x46A3C0 | RegKeyList_CopyFromSibling | Copy from sibling key |

## Tournament Save Format

The TOURNAMENT.SAV file format (DATA\tournament.sav):
- Written via Tourney vtable[0x54] 
- Signature: DAT_004D48A0 (likely a magic header string)
- Contains: race completion status, best times, medal counts
- Deleted and rewritten each save cycle (atomic write pattern)

## Config File: DATA\/

| File | Description |
|------|-------------|
| HS.CFG | Main config (loaded via vtable[0x8C]+8) |
| tournament.sav | Tournament progress save |

## Startup Sequence (App_Initialize_Full 0x429530)

```
Step 12: RegKey_Open(App+0x54)
Step 13: RegKey_ReadString/ReadDword(App+0x54, PlayCount)
Step 15-22: 4x InputDevice_SetType (types 1=kb, 2=mouse, 4=joy1, 5=joy2)
Step 23: RegKey_Close(App+0x54)
```

On exit: App_SaveAllConfig → LoadOrSaveConfig → Tourney_SaveTournament