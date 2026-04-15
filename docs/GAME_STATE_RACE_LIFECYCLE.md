# Game State & Race Lifecycle

## Overview
Hamsterball's game state is managed through App struct mode flags and a set of
state transition functions. The race lifecycle goes: Menu → Loading → Race → Result → Menu.

## App State Machine

### App Struct Mode Fields
| Offset | Type | Description |
|--------|------|-------------|
| App+0x174 | GfxEngine* | Graphics engine (set cull mode on start) |
| App+0x178 | Scene* | Current scene |
| App+0x184 | Scene* | Loading screen scene |
| App+0x1DC | SoundChannel* | Sound channel for input events |
| App+0x1E4 | InputDev* | Player 1 input device |
| App+0x1E8 | InputDev* | Player 2 input device |
| App+0x21C | void* | State object (freed on start race) |
| App+0x220 | void* | Menu/dialog object (freed on start race) |
| App+0x224 | void* | Secondary dialog (freed on start race) |
| App+0x228 | void* | Tertiary dialog (freed on start race) |
| App+0x22C | LoadingScreenGadget* | Resource loader |
| App+0x534 | MusicPlayer* | Primary music channel |
| App+0x53C | MusicPlayer* | Secondary music channel |
| App+0x708 | int | Game state (3=racing) |
| App+0x7D2 | byte | Cull mode flag |

### Game States
The game uses numeric state values stored at App+0x708:
- **State 3**: Active racing
- Other states managed by vtable dispatch from App_GameLoop (0x46BD80)

### Race Start Sequence

#### App_StartRace (0x4287C0)
```
1. Scene_UpdateChildren(App+0x178)  // Update all objects
2. Gfx_SetCullMode(App+0x174)       // Set backface culling
3. Set D3D render state vtable[50]   // 0x16 = 2 or 3 depending on flag
4. App+0x708 = 3                    // Set game state to "racing"
5. Free old state objects at:
   - App+0x21C (vtable[0x40]() destructor)
   - App+0x228 (vtable[0x40]() destructor)  
   - App+0x224 (vtable[0x40]() destructor)
   - App+0x220 (direct destructor call with param 1)
6. If music[0] exists: MusicPlayer_SetTempoScale(music[0], 1.0)
7. If music[1] exists: MusicPlayer_SetTempoScale(music[1], 0.5)
```

#### App_StartTournamentRace (0x4288B0)
Same as StartRace but with tournament-specific initialization.

#### App_Start2PRace (0x429230)
Two-player split-screen race initialization.

#### App_StartPracticeRace (0x428C50)
Practice mode - same track load but no tournament tracking.

#### App_StartMPRace (0x428B20)
Multiplayer race init - sets up arena mode.

### Race End Sequence

#### App_CompleteRace (0x425F90)
When the ball crosses the finish line:
1. Calculate elapsed time from timer
2. Compare against best time (App+0x86C[player_index*4])
3. If better: update best time and save to registry
4. Determine medal: Bronze/Silver/Gold based on time thresholds
5. Create RaceGoalReached object:
   - Display "GOAL REACHED!" text (800ms timer)
   - Show weasel rank sprite ("textures\ranks\weasel.png")
   - Update best time blob at App+0x86C
6. Progress tournament if applicable

### Quit Race

#### QuitRace (0x42FAD0)
Exits current race back to menu:
1. Set game state to menu mode
2. Free scene objects
3. Restore menu display
4. Reset music tempo

#### QuitRaceMenu (0x42E6F0)
In-race quit confirmation menu.

### Level Data Loading

#### LoadRaceData (0x40A120)
Loads MESHWORLD data for a specific level:
1. Determine level path from race type
2. MeshWorld_ctor to parse level geometry
3. Level_InitScene to create objects from MESHWORLD data
4. Scene_SpawnBallsAndObjects to place ball and level objects
5. Level_SelectCameraProfile based on level type

### Level-SikeType Race Handlers
Each level type has its own HandleRaceEnd callback:

| Address | Level | Handler |
|---------|-------|---------|
| 0x420240 | Beginner | Board_Beginner_HandleRaceEnd |
| 0x420660 | Up | Board_Up_HandleRaceEnd |
| 0x222630 | Master | Board_Master_HandleRaceEnd |

These handle:
- Time comparison against stored best times
- Medal awards (Bronze/Silver/Gold thresholds)
- Tournament advancement
- Unlock conditions for mirror mode

### Tournament Progression

#### Board_ctor (0x419030)
Initializes board/level with:
- LoadRaceData for the specific level
- Set camera profile
- Create HUD elements (timer, score display)
- Initialize physics parameters

#### Tournament_AdvanceRace (0x427080)
Switch statement handling race progression:
```
case 0: Warm Up     → levels\arena-WarmUp
case 1: Beginner    → levels\arena-Beginner  
case 2: Intermediate→ levels\arena-Intermediate
case 3: Dizzy       → levels\arena-dizzy
case 4: Expert      → levels\arena-Expert
case 5: Neon        → levels\arena-neon
case 6: Sky         → levels\arena-Sky
case 7: Toob        → levels\arena-Toob
case 8: Odd         → levels\arena-Odd
case 9: Glass       → levels\arena-Glass
case 10: Impossible → levels\arena-impossible
case 11: Master     → levels\arena-Master
default: Mirror races (reverse direction)
```

### Start Position Selection

#### Scene_SpawnBallsAndObjects (0x41C5B0)
For each player:
1. Look up "START{player}-{player}" in hash table
2. In tournament/mirror modes: random between START2-1 and START2-2
3. If "START-DEBUG" exists: use that (cheat position)
4. Create Ball struct (0xC60 bytes)
5. Set defaults: radius=26, max_speed=5, gravity=0.5, speed_factor=1.05
6. Set position from start point
7. Append to ball list

### Timer System

#### TimerArray (0x475980-0x475AA0)
Manages multiple countdown timers:
- TimerArray_Ctor: Initialize timer array
- TimerArray_RenderTimed: Render and tick timers
- Timer_Decrement: Advance timer by 1 frame

#### UITimer (0x448AC0)
On-screen timer display:
- Shows minutes:seconds:centiseconds
- Position derived from Scene+0x87C (camera)

#### RumbleBoard Timer (0x458E60-0x458E90)
Arena tournament timer:
- RumbleBoard_InitTimer: Set countdown (typically 60-90 seconds)
- RumbleBoard_TickTimer: Per-frame countdown
- RumbleBoard_CleanupTimer: Reset timer