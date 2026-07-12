# Global Chomper Mod

Spawns Tower Race chompers (purple thing in the pit) on any level with a hotkey.

## Usage

1. Load `GlobalChomper.CEA` in Cheat Engine
2. Enable the script
3. Set `SpawnChomper` to 1 in CE (or use a hotkey)
4. Chomper spawns at player 1's position
5. Ball touching chomper takes 25.0 damage (E:BITE event) + chomp sound

## How It Works

- Hooks at `Ball_Update` (0x405E22)
- On first spawn: loads `Meshes\Chomper` mesh via `MeshWorld_ctor` → stores at `board+0x4390`
- Creates CollisionLevel from mesh via `CollisionLevel_ctorWithLevel` (0x465080)
- Loads mesh data via `Level_LoadMeshes` (0x465200)
- Registers collision with scene manager at 0x4F7360
- E:BITE events are baked into the chomper mesh's collision triangles
- When ball touches chomper → E:BITE → board+0x43A0 = 25.0 (bite damage)
- Game loop reads board+0x43A0 → applies damage to ball → "sounds\chomp" plays
- Registered to board+0x2578 (general), board+0xCD4, scene spatial tree

## Object Details

- **Type:** MeshWorld + CollisionLevel (NOT a game object with vtable)
- **Mesh:** "Meshes\Chomper" (VA 0x4D094C)
- **Sound:** "sounds\chomp" (VA 0x4D2D98)
- **Collision Event:** E:BITE → 25.0 damage (0x41C80000)
- **No per-frame update needed** — collision events fire automatically

## How E:BITE Works

The chomper mesh's collision triangles have `E:BITE` event names embedded in them.
When the ball intersects these triangles, the game's collision system fires `E:BITE`:

```c
// In Level_HandleCollision (0x40DCD0):
if (stricmp(eventName, "E:BITE") == 0) {
    board+0x43A0 = 25.0;   // bite damage
    board+0x43A8 = 0;      // reset counter
}
```

The damage value is read by the game loop at 0x4023C1 and applied to the ball.

## Full Analysis

See `docs/MACE_WINDMILL_CHOMPER_SYSTEM.md` for complete reverse-engineering documentation.
