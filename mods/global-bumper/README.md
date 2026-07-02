# Global Bumper Spawner (CEA)

Spawns PopCylinder objects that act as bumpers on ANY level. When a ball touches a spawned bumper, exact N:BUMPER physics are applied — velocity scaled, clamped, sound played, hit flag set.

## Usage

1. Enable script in Cheat Engine
2. Press hotkey to set `SpawnBumper` to 1 (or use `Set SpawnBumper to 1` in CE)
3. A PopCylinder spawns 50 units to the side of the ball, 5 units below
4. Any ball (player or AI) that touches it gets bumped with exact bumper physics
5. Max 16 bumpers can be active simultaneously

## How It Works

### N:BUMPER Physics (traced from 3 native handlers)

All 3 handlers (Warm-Up 0x4111E0, Beginner/Master 0x410020, Arena 0x412850) apply identical physics:

1. **Sound**: `Sound_Play3D(App+0x448, ball.x, ball.y, ball.z)` — bumper hit sound
2. **Velocity Scale**: Read `phys+0xCA4` (X), `phys+0xCAC` (Z) from `ball+0x1A4`. Multiply XZ by **4.0** (race) or **5.0** (arena)
3. **Zero Y**: Set `phys+0xCA8 = 0.0`
4. **Min Clamp**: If horizontal speed < **5.0**, call `Vec3_NormalizeAndScale(&vec, 5.0)` at 0x401AA0
5. **Max Clamp**: If total speed > **10.0** (race) / **12.0** (arena), call `Vec3_NormalizeAndScale(&vec, max)`
6. **Write Back**: Store to `phys+0xCA4/CA8/CAC`
7. **Hit Flag**: `board + N*4 + offset = 1.0f` (0x3F800000)
   - Race boards: offset = **0x6448** (Warm-Up uses 0x6428)
   - Arena boards: offset = **0x53FC**, N is 0-indexed (N-1)

### Key Functions

| Function | Address | Purpose |
|---|---|---|
| `Sound_Play3D` | 0x459860 | Play 3D positioned sound (ECX=sound obj, stack: x,y,z) |
| `Vec3_NormalizeAndScale` | 0x401AA0 | Normalize vec3 and scale to length (ECX=&vec, stack: float) |
| `PopCylinder_ctor` | 0x436EE0 | Construct PopCylinder (ECX=this, stack: mesh,x,y,z,board) |
| `SceneObject_ctor` | 0x461510 | Init mesh data (ECX=this, stack: tree, path) |
| `AthenaList_Append` | 0x453810 | Add to list (ECX=&list, stack: item) |
| `DispatchCollisionEvents` | 0x40C5D0 | Base collision handler |
| `Ball_SetVec3AtOffset` | 0x402A20 | Write velocity to phys (ECX=phys, stack: &vec3) |

### Constants (.rdata)

| Address | Hex | Float | Purpose |
|---|---|---|---|
| 0x4CF41C | 40800000 | 4.0 | Velocity scale (race) |
| 0x4CF55C | 40A00000 | 5.0 | Min speed / Arena scale |
| 0x4CF9F8 | 41200000 | 10.0 | Max speed (race) |
| 0x4CF3DC | 41400000 | 12.0 | Max speed (arena) |
| 0x4CF368 | 00000000 | 0.0 | Zero constant |

### Board Type Detection

Check `board->vtable[0x1D]` (offset +0x74):
- `0x412850` = Arena board → use arena constants (scale 5.0, max 12.0, flag at 0x53FC)
- Anything else → use race constants (scale 4.0, max 10.0, flag at 0x6448)

## Fixes from v9

1. **Velocity scaling instead of direction replacement** — v9 used `Ball_SetVec3AtOffset` with a direction vector (ball→bumper). Real bumpers SCALE the existing velocity by 4.0/5.0, preserving the ball's travel direction.
2. **Min/max speed clamping** — v9 had no clamping. Real bumpers clamp to [5.0, 10.0] race / [5.0, 12.0] arena.
3. **Bumper sound** — v9 played no sound. Now calls `Sound_Play3D(App+0x448, x, y, z)`.
4. **Hit flag** — v9 didn't set the board hit flag. Now sets `board+N*4+offset = 1.0f` for bumper animation.
5. **Correct position offsets** — v9 read `ecx+10DC`/`ecx+10E4` for proximity, but PopCylinder stores position at `+0x10D4` (X) and `+0x10DC` (Z).
6. **Board type detection** — v9 didn't detect arena vs race boards, using wrong constants.
