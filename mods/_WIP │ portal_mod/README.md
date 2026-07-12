# Portal Mod

Custom event plane `E:PORTAL` that teleports the ball between linked portals, preserving velocity.

## How It Works

Place mesh objects in a custom MESHWORLD level file named with the `E:PORTAL(set.portal)` format:

- `E:PORTAL(1.1)` and `E:PORTAL(1.2)` — linked pair (set 1)
- `E:PORTAL(2.1)` and `E:PORTAL(2.2)` — linked pair (set 2)
- `E:PORTAL(3.1)` and `E:PORTAL(3.2)` — linked pair (set 3)
- etc.

When the ball touches an `E:PORTAL` collision plane, it instantly teleports to the matching portal in the same set (the other portal with the same first number but different second number). The ball's velocity is preserved through the teleport.

## Portal Naming Convention

```
E:PORTAL(setNumber.portalNumber)
```

- **setNumber** (first number): Links portals together. Two portals with the same set number are linked.
- **portalNumber** (second number): Identifies which portal in the set. Must be different for the two linked portals.

Examples:
- `E:PORTAL(1.1)` ↔ `E:PORTAL(1.2)` — set 1, portals 1 and 2
- `E:PORTAL(2.1)` ↔ `E:PORTAL(2.2)` — set 2, portals 1 and 2
- `E:PORTAL(10.1)` ↔ `E:PORTAL(10.2)` — set 10

## Technical Details

- **Platform**: Hamsterball Plus API (DLL mod loaded via `Mods/` folder)
- **Hook**: `onEventPlaneCollide` callback — intercepts all collision events, filters for `E:PORTAL` prefix
- **Portal Discovery**: Scans the level's SpatialTree named objects list on level start to find all `E:PORTAL` objects and caches their positions
- **Teleport**: Writes ball position (`pos_x/y/z`) and previous position (`prev_pos_x/y/z`) to the target portal's position. Velocity in `PhysicsObject` is untouched — the ball maintains whatever speed it had when entering
- **Cooldown**: 30-frame per-player cooldown after teleporting to prevent instant re-trigger at the destination portal
- **Safety**: All pointer dereferences guarded with `IsBadReadPtr`; only player balls (playerID 0-3) are affected

## Installation

1. Place `PortalMod.dll` in the game's `Mods/` folder
2. Launch the game
3. Play a custom level with `E:PORTAL(x.y)` named objects
