# Global FunBall Spawner

Spawns a real FunBall ball entity at Player 1's position in any level.

## Usage

Set `SpawnObject` to 1 to spawn a FunBall at the player's position.

## How the FunBall Works

### Mesh Loading

The `"Meshes\FunBall"` mesh (string at 0x4D3474) is loaded by `App_ResourceLoader` (TimerDisplay, 0x4298C0) via vtable[0x4C] into `App+0x26C` as a MeshNode (0x18 bytes).

### Ball Mesh Array

The game stores all ball meshes in an array at `board+0x244`, indexed by `ball+0x754` (mesh type index):

| Index | Offset | Mesh Path |
|-------|--------|-----------|
| 0 | App+0x244 | `Meshes\Sphere` |
| 1 | App+0x248 | `Meshes\SphereBreak1` |
| 2 | App+0x24C | `Meshes\SphereBreak2` |
| 3 | App+0x250 | `Meshes\Hamster-Waiting` |
| 4 | App+0x254 | `Meshes\Hamster-trot1` |
| 5 | App+0x258 | `Meshes\Hamster-trot2` |
| 6 | App+0x25C | `Meshes\Hamster-trot3` |
| 7 | App+0x260 | `Meshes\RBGlare` |
| 8 | App+0x264 | `Meshes\Sphere+Tar` |
| 9 | App+0x268 | `Meshes\8Ball` |
| **10** | **App+0x26C** | **`Meshes\FunBall`** |
| 11 | App+0x270 | `Meshes\Bell` |
| 12 | App+0x274 | `Meshes\Dizzy` |

### Ball Construction

- **Ball_ctor** (0x40AFE0): alloc 0xC98, `__thiscall(this, board)`
  - Calls `Ball_ctor2` which initializes all fields including `ball+0x754 = 0` (Sphere)
  - Sets vtable to 0x4CF3A0
- After construction, set `ball+0x754 = 10` to use the FunBall mesh

### Rendering

Ball_Render (0x402DE0) and Ball_RenderShadow (0x401B00) read `board+0x244[ball+0x754]` to select the mesh for rendering. The render function calls `mesh->vtable[0x07](matrix, 0)` to render.

### Registration

Balls are registered in two lists:
- `board+0x29D4` — ball list (for updates and rendering)
- `board+0x2DEC` — collision list (for physics)

### Ball Key Fields

| Offset | Type | Description |
|--------|------|-------------|
| +0x000 | int* | vtable (0x4CF3A0) |
| +0x010 | int | board pointer |
| +0x014 | int | scene pointer |
| +0x164 | float | position X |
| +0x168 | float | position Y |
| +0x16C | float | position Z |
| +0x281 | byte | unused_init_flag (DEAD: set by ctor, never read) |
| +0x284 | float | radius (26.0) |
| +0x2F9 | byte | is_stunned (0=not falling) |
| +0x2FC | float | alpha (1.0) |
| +0x754 | int32 | mesh type index (10=FunBall) |
| +0xC74 | int32 | AI chase target (0=none) |
| +0xC80 | byte | flag (0) |
