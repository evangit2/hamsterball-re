# Magnet Mod

Attracts or repels the ball using S1 ref points in MESHWORLD level files.

## Usage

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy `bass.dll` and `magnet_config.txt` to the game folder
3. Add S1 ref points to your level's MESHWORLD file:
   - `MAGNET(P)` — attracts the ball (pulls toward point)
   - `MAGNET(N)` — repels the ball (pushes away from point)

## Config (`magnet_config.txt`)

| Setting    | Default | Description                        |
|------------|---------|------------------------------------|
| range      | 300.0   | How far magnets reach (world units)|
| strength   | 0.5     | Force magnitude per magnet         |

## How It Works

- Scans S1 ref points on level load for `MAGNET(P)` and `MAGNET(N)` names
- Each frame, the Present hook computes total magnet force (safe C context)
- Force is applied via FPU writes to ball+0x170/0x174/0x178 (force accumulators)
  at the Phase 15 convergence point (0x407BB4) — same technique as the jump mod
- The game's physics engine then consumes these accumulators with proper
  collision response, friction, and momentum — no jitter

## MESHWORLD Example

Add these to Section 1 (ref points) in your level file:

```
MAGNET(P)    500.0    100.0    500.0
MAGNET(N)    800.0    100.0    800.0
```

Position is (X, Y, Z) in world coordinates.
