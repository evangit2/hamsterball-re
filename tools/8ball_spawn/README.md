# 8-Ball Spawn Mod

Press **B** during gameplay to spawn an 8-ball in front of your hamster ball. The 8-ball spawns with physics and rolls/collides just like the balls in Rodent Rumble arenas.

## What's New

- **Audio forwarding**: All 10 BASS functions the game imports are forwarded to `bass_real.dll`. The game now has full audio while the mod is active.
- **Single-ball overwrite**: Only one 8-ball can exist at a time. Pressing B again repositions the existing 8-ball to your current location and velocity — no new ball is created.
- **Dead AI fields documented**: CHASE/HOME/SPINDISTANCE values are set but have no effect because `is_8ball` (ball+0xC74) is never set to 1. The ball is a physics-only debris ball.

## Installation

1. In your Hamsterball game folder, rename `bass.dll` → `bass_real.dll`
2. Copy the mod's `bass.dll` into the game folder
3. (Optional) Copy `8ball_spawn.ini` next to `bass.dll` for configuration
4. Launch the game, enter a level or arena, press **B** to spawn an 8-ball

## How It Works

The 8Ball mesh is preloaded by the game's resource loader into the board mesh array at `board+0x268` (index 9 of the array at `board+0x244`). The ball mesh index field `ball+0x754` controls which mesh the ball uses — setting it to 9 makes the ball render as the 8-ball.

Ball creation follows the same pattern as `CreateBadBall` (0x40BCA0):
1. `operator_new(0xC98)` allocates ball memory
2. `Ball_ctor(mem, scene)` constructs the ball
3. `vtable[1]()` initializes physics defaults
4. Position is set in front of the player's ball
5. `ball+0x754 = 9` selects the 8Ball mesh
6. `ball+0x18 = -1` sets player_index to none (debris ball)
7. Player's exact velocity vector is copied (same direction and speed)
8. Radius stays at `Ball_InitPhysicsDefaults` default (35.0)
9. Ball is added to `scene+0x29D4` (bad_balls_list) and `scene+0x2DEC` (all_balls_list)

### Single-Ball Overwrite

If a previously spawned 8-ball is still valid (vtable check against 0x4CF3A0), pressing B again repositions it to the new spawn location with the player's current velocity. No new allocation is made — the same ball object is reused. If the old ball has been freed or corrupted, a fresh ball is allocated.

### Audio Forwarding

The mod loads `bass_real.dll` on the first BASS function call (not in DllMain, to avoid loader lock deadlocks on real Windows). All 10 functions the game imports are forwarded with correct `__stdcall` calling conventions:

| Function | Forwarded |
|----------|-----------|
| BASS_Init | ✅ |
| BASS_Free | ✅ |
| BASS_Start | ✅ |
| BASS_Stop | ✅ |
| BASS_SetConfig | ✅ |
| BASS_ErrorGetCode | ✅ |
| BASS_MusicLoad | ✅ |
| BASS_MusicPlayEx | ✅ |
| BASS_ChannelSetAttributes | ✅ |
| BASS_ChannelStop | ✅ |

If `bass_real.dll` is not found, all functions return success values (1/0) — the game runs without audio but does not crash.

### AI Fields (Dead Code)

The mod sets CHASE=100000, HOME=100000, SPINDISTANCE=1.0 on the spawned ball. However, these fields have **no effect** because:

- `ball+0xC74` (`is_8ball` flag) is never set to 1
- Without that flag, `Ball_Update` (0x408390) skips the AI chase block entirely via `if (ball[0x31d] != 0 || scene[0x237] != 0)`
- The ball behaves as a physics-only debris ball — it rolls, falls, and collides but never actively chases other balls

To enable AI behavior, `ball+0xC74` would need to be set to 1. This would make the 8-ball chase the nearest player using the CHASE/HOME/SPINDISTANCE parameters.

## Configuration

Edit `8ball_spawn.ini` (placed next to `bass.dll`):

```ini
[8ball]
spawn_key=0x42       ; B key (default). See VK_* constants on MSDN
spawn_distance=40    ; Distance in front of player
```

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll 8ball_spawn.c \
    -lwinmm -Wl,--enable-stdcall-fixup \
    -O2 -static -static-libgcc -Wl,--add-stdcall-alias
```

## Files

| File | Description |
|------|-------------|
| `8ball_spawn.c` | Mod source code |
| `8ball_spawn.ini` | Configuration file |
| `bass.dll` | Compiled mod (proxy bass.dll) |

## Technical Notes

- BASS_SetConfig/BASS_GetConfig are now forwarded to bass_real.dll (previously stubbed). The game calls BASS_SetConfig via SEH-protected code — forwarding with correct `__stdcall` convention works safely.
- Scene/Board discovery uses brute-force App struct scan for the Scene vtable (0x4D0260), same approach as the player_clones mod.
- The 8-ball has no AI — it spawns as a physics debris ball with player_index = -1. It rolls, falls, and collides with other balls and geometry.
- `load_real_bass()` is called from `spawn_thread` (not DllMain) to avoid the Windows loader lock deadlock that affects `LoadLibraryA` calls during DLL initialization.
