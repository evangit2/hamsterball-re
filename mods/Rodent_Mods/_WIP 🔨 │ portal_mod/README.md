# Portal Mod

Teleports the ball between linked portals using the game's native teleport system (same as Toob Race's E:BRANCH and Odd Race's N:JUMPFIRST).

## How It Works

Place three types of objects in a custom MESHWORLD level:

1. **`E:PORTAL(N)`** — An event plane meshbuffer. When the ball touches it, the ball teleports.
2. **`PORTALPOS(N)`** — An S1 ref point. The destination position.
3. **`PORTALVEC(N)`** — An S1 ref point (optional). Defines the exit direction. Direction = PORTALVEC - PORTALPOS.

When the ball touches `E:PORTAL(1)`, it teleports to `PORTALPOS(1)` and exits in the direction of `PORTALVEC(1)` - `PORTALPOS(1)`, preserving the ball's incoming speed.

If `PORTALVEC(N)` is not placed, the ball keeps its current velocity direction unchanged.

## Naming Convention

```
E:PORTAL(number)       — event plane (trigger)
PORTALPOS(number)       — S1 ref point (destination position)
PORTALVEC(number)       — S1 ref point (exit direction, optional)
```

Numbers can be multiple digits (1, 10, 100, etc.). The event plane and ref points are linked by matching numbers.

Examples:
- `E:PORTAL(1)` + `PORTALPOS(1)` + `PORTALVEC(1)` — portal 1 with custom exit direction
- `E:PORTAL(2)` + `PORTALPOS(2)` — portal 2, velocity direction preserved
- `E:PORTAL(10)` + `PORTALPOS(10)` + `PORTALVEC(10)` — portal 10

## Velocity

The ball's incoming speed is preserved through the teleport. The exit direction is determined by:

1. If `PORTALVEC(N)` exists: direction = normalize(PORTALVEC - PORTALPOS), scaled to incoming speed
2. If `PORTALVEC(N)` does not exist: velocity is untouched (ball keeps current direction and speed)

## Technical Details

- **Platform**: Hamsterball Plus API v2.1 (API version 3)
- **Hook**: `onEventPlaneCollide` callback
- **Portal Discovery**: Looks up S1 ref points on demand when a portal triggers (no pre-scanning or caching)
- **Teleport**: Uses the game's native teleport flag system (`ball+0xC3C=1` + `ball+0xC40/C44/C48` for destination XYZ), sets exit velocity via PhysicsObject (`+0xCA4/0xCA8/0xCAC`). Ball radius is added to the Y destination so the ball doesn't spawn inside the floor.
- **Cooldown**: 30-frame per-player cooldown to prevent re-trigger loops
- **Safety**: All pointer dereferences guarded with `IsBadReadPtr`

## Installation

1. Place `PortalMod.dll` in the game's `Mods/` folder
2. Launch the game
3. Play a custom level with `E:PORTAL(N)` event planes and `PORTALPOS(N)` ref points
