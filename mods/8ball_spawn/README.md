# 8-Ball Spawn Mod

Press **B** during gameplay to spawn an 8-ball in front of your hamster ball. The 8-ball spawns with physics and rolls/collides just like the balls in Rodent Rumble arenas.

## Installation

1. In your Hamsterball game folder, rename `bass.dll` → `bass_real.dll`
2. Copy the mod's `bass.dll` into the game folder
3. (Optional) Copy `8ball_spawn.ini` next to `bass.dll` for configuration
4. Launch the game, enter a level or arena, press **B** to spawn 8-balls

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
8. Radius stays at `Ball_InitPhysicsDefaults` default (35.0) — not player radius
9. AI tuning: HOME=100000, CHASE=100000, SPINDISTANCE=1 (always chase, no leash)
10. Ball is added to `scene+0x29D4` (bad_balls_list) and `scene+0x2DEC` (all_balls_list)

Key detection: `GetAsyncKeyState('B')` polled in a background thread at 16ms intervals, edge-triggered (only spawns on key-down transition).

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

- BASS_SetConfig/BASS_GetConfig are intentionally NOT exported. The game calls these via SEH-protected code — Wine's "unimplemented function" exception is caught by the game's `KiUserCallbackDispatcher`. Exporting them with wrong calling convention crashes the game (stack corruption at 0x424).
- Scene/Board discovery uses brute-force App struct scan for the Scene vtable (0x4D0260), same approach as the player_clones mod.
- The 8-ball has no AI — it spawns as a physics debris ball with player_index = -1. It rolls, falls, and collides with other balls and geometry.
