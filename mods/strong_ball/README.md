# Strong Ball Mod

Makes Player 1 knock other balls with enormous force during ball-ball collisions, while other balls still bump P1 with normal force.

## How It Works

In `Ball_Update` (0x405E00), when two balls collide, the game calls `vtable[0x18]` on **each** ball with:
- A direction vector (x, y, z) — normalized direction from one ball to the other
- A force multiplier — `1.0f` (0x3F800000) for both balls

The two calls happen at:
- **0x406D87** — `PUSH 0x3F800000` → force multiplier for the **OTHER** ball (pushed away from player)
- **0x406DAF** — `PUSH 0x3F800000` → force multiplier for the **PLAYER** ball (pushed toward other)

This script intercepts **0x406D87** (the OTHER ball's force). When the current ball is P1 (`ball+0x18 == 0`), it replaces the constant `1.0f` with a configurable `STRONG_FORCE` value (default 20.0f). When not P1, it pushes the original `1.0f`.

The player ball's own force (at 0x406DAF) is never touched — P1 still gets bumped with normal 1.0x force.

## Key Addresses

| Address | Instruction | Purpose |
|---------|------------|---------|
| 0x406D87 | `PUSH 0x3F800000` | Other ball's force multiplier (HOOK POINT) |
| 0x406DAF | `PUSH 0x3F800000` | Player ball's force multiplier (untouched) |
| ball+0x18 | DWORD | Player ID: 0=P1, 1=P2, 2=P3, 3=P4, -1=AI |

## Configuration

- `ForceMult` — Force multiplier when P1 bumps another ball (default: 20.0)
  - 1.0 = normal, 10.0 = strong, 20.0 = extreme, 50.0 = insane
- `Enabled` — Set to 0 to disable (1 = enabled)

## Register Safety

At the hook point (0x406D87):
- `ESI` = current ball (preserved by cave)
- `EDI` = not yet loaded (loaded at 0x406D95)
- `EDX` = other ball vtable (preserved)
- FPU `ST0` = dir_z (preserved — cave uses only EAX, which is overwritten at 0x406DA9)
