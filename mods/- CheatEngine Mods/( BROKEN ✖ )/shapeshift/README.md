# Shapeshift Mod

Transform Player 1 into different ball entities and objects via Cheat Engine hotkeys.

## Current Forms

| Form | Mesh | Description |
|------|------|-------------|
| Normal Hamster | `Meshes\Sphere` (App+0x244) | Default player ball with hamster inside |
| 8-Ball / BadBall | `Meshes\8Ball` (App+0x268) | AI enemy ball appearance, player-controlled |

## How It Works

The player ball has a **vtable** at `ball+0x00` that controls all virtual dispatch:
- **vtable[2]** (offset +0x08) = **Render function** — determines what mesh is drawn
- **vtable[4]** (offset +0x10) = **Update function** — determines physics + input handling

| Vtable | Render (vtable[2]) | Update (vtable[4]) | Mesh Used |
|--------|--------------------|--------------------|-----------|
| `0x4CF314` (Player) | `0x403DC0` — Sphere + hamster animation | `0x405E00` — Ball_Update (physics + keyboard input) | App+0x244 (Sphere) |
| `0x4CF3A0` (AI) | `0x402DE0` — 8Ball mesh | `0x408390` — Ball_AI_ChaseNearest (AI targeting) | App+0x268 (8Ball) |

### The Trick: Custom Hybrid Vtable

We can't just swap the entire vtable — that would replace vtable[4] with `Ball_AI_ChaseNearest`, which checks the `is_8ball` flag and only calls `Ball_Update` if certain conditions are met. If those conditions aren't met, **the ball gets no physics and no input**.

Instead, we create a **custom vtable** in CEA-allocated memory that:
- Copies ALL 21 entries from the player vtable (`0x4CF314`)
- Patches **only vtable[2]** to the AI render function (`0x00402DE0`)
- Keeps vtable[4] as `Ball_Update` (`0x00405E00`) — **full player control preserved**

Result: Player 1 looks like an 8-Ball but controls identically to the normal hamster ball.

### Hook Point

The hook intercepts `Ball_Update` at address `0x00405E22` (after `MOV ESI,ECX` sets ESI = ball pointer):
- Saves ESI as `g_PlayerBall` if `ball+0x18 == 0` (player 1)
- If `g_ShapeState == 1`: writes `g_CustomVtable` address to `ball+0x00`
- If `g_ShapeState == 0`: writes `0x4CF314` (original player vtable) to `ball+0x00`
- Restores original instruction and jumps back

## Usage

1. Open `Shapeshift.CEA` in Cheat Engine
2. Activate the script (Enable checkbox)
3. Set hotkeys in CE table:
   - **Numpad 1** → Set `g_ShapeState` to `1` (transform to 8-Ball)
   - **Numpad 0** → Set `g_ShapeState` to `0` (revert to hamster)
4. Start a race in Hamsterball
5. Press Numpad 1 to transform, Numpad 0 to revert

## CE Address Reference

| Symbol | Description |
|--------|-------------|
| `g_PlayerBall` | Pointer to P1's ball struct (updated each frame) |
| `g_ShapeState` | 0 = normal hamster, 1 = 8-Ball form |
| `g_CustomVtable` | Custom vtable with AI render + player update |

## Ball Struct Key Offsets

| Offset | Type | Field | Player Value | AI Value |
|--------|------|-------|-------------|----------|
| +0x00 | ptr | vtable | 0x4CF314 | 0x4CF3A0 |
| +0x10 | ptr | App* | scene+0x878 | scene+0x878 |
| +0x14 | ptr | Scene/Board | board | board |
| +0x18 | int32 | player_index | 0 (P1) | -1 (AI) |
| +0x281 | byte | is_player_ctrl | 1 | 0 |
| +0x284 | float | radius | 26.0 | 35.0 (default) |
| +0x314 | float | anim_timer | frame selector | unused |
| +0x754 | int32 | mesh_state | 0=Sphere | unused (AI ignores) |
| +0x768 | byte | is_active | 1 | 1 |

## Future Forms (Planned)

- FunBall (App+0x26C, index 10) — unused by game code
- Bell (index 11) — arena bell mesh
- Dizzy (index 12) — dizzy ball mesh
- Sphere+Tar (App+0x264, index 8) — tar-covered ball
- RBGlare (index 7) — reflective glare ball
- Custom objects (BONK, Catapult, etc.) — requires different approach (not vtable swap)
