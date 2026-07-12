# Power Ball Mod

Makes the player ball stronger than 8-Ball and Fun Ball.

## What it does

When toggled ON in the Options menu:
- **Radius** increased to 55.0 (player default 26.0, 8-ball 35.0)
- **Max speed** increased to 10.0 (default 6.0)

### Why 55.0 radius?

The knockout formula from `Ball_Update` (0x405E00):
```
if (otherBall.radius < myRadius * 0.7) → knockout otherBall
```
- Player at 55.0: `55.0 × 0.7 = 38.5` → knocks out 8-balls (35.0 < 38.5) ✓
- 8-ball at 35.0: `35.0 × 0.7 = 24.5` → cannot knock out player (55.0 > 24.5) ✓
- Fun ball at 26.0: `26.0 × 0.7 = 18.2` → cannot knock out player ✓

## Install

1. Build as DLL in Visual Studio (using HBmodTemplate)
2. Place `.dll` in game's `Mods\` folder
3. Toggle "POWER BALL" in Options menu

## Author
Hamsterbot
