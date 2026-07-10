# First-Person Ball Cam

## Description
Pure CEA script that overrides the camera in Hamsterball to create a first-person ball cam. Camera sits directly above the ball and looks forward in the direction of travel.

Works in both arenas and races.

## v2 Fix — Root Cause
v1 was broken because it misunderstood the camera system:

- **SetPosition(X,Y,Z)** = sets the LOOK-AT TARGET (where camera looks toward), NOT the eye position
- **SetDirection(&vec3, dist)** = sets the direction FROM eye TO target + orbit distance
- Actual eye position = `target - direction * distance`

v1 treated SetPosition as eye position and SetDirection as look direction → camera was in wrong place, looking wrong way, wiggling because velocity changed every frame.

**v2 fix:**
- Target = ball_pos + velocity_dir × lookAhead → looks ahead in travel direction
- Direction = `(vel_dir.x × la, -height, vel_dir.z × la)` → from eye (above ball) toward target ahead
- Distance = 1.0 → eye = target - direction = ball_pos + (0, height, 0) → directly above ball
- Uses XZ velocity only (ignores Y) → no camera flipping when falling/bouncing

## How It Works
- **Hook:** `Scene_SetCamera` epilogue at `0x41A254` (5-byte JMP to code cave)
- **Jump-back:** `0x41A2BF` (ApplyCamera — applies the camera transform to the render pipeline)
- Bypasses the original orbit angle/distance system entirely
- Original bytes restored on disable: `8B 44 24 14 8B`

## Camera Math
```
target = ball_pos + (velX_norm × lookAhead, 0, velZ_norm × lookAhead)
direction = (velX_norm × lookAhead, -height, velZ_norm × lookAhead)
distance = 1.0
eye = target - direction × distance = ball_pos + (0, height, 0)
```
Camera is directly above the ball at height `fpCamHeight`, looking forward toward a point `fpCamLookAhead` units ahead in the travel direction.

## Adjustable Parameters (CE Address Table)
| Symbol | Default | Description |
|---|---|---|
| `fpCamHeight` | 40.0 | Height above ball center (ball radius ~26, so 40 puts camera just above surface) |
| `fpCamLookAhead` | 150.0 | How far ahead to look in the travel direction |

## Tuning Tips
- **More first-person:** Lower `fpCamHeight` to ~28 (inside ball) and `fpCamLookAhead` to ~50
- **More top-down:** Raise `fpCamHeight` to ~100 and lower `fpCamLookAhead` to ~30
- **Wider view ahead:** Increase `fpCamLookAhead` to ~300

## Technical Details
- **Camera object:** `Scene+0x87C`, vtable[2]=SetPosition (look-at target), vtable[3]=SetDirection (direction + orbit distance)
- **Ball data:** position at `+0x14` (vec3), velocity at `+0x20` (vec3), radius at `+0x3C`
- **Velocity Y ignored:** Only XZ components used for direction, prevents camera flipping on vertical movement
- **Stationary fallback:** When horizontal speed < 0.001, direction defaults to (0, 0, 1) — forward +Z
- **vtable[2] calling convention:** `__thiscall`, ECX=camera, push Z/Y/X on stack, `RET 0xC`
- **vtable[3] calling convention:** `__thiscall`, ECX=camera, push distance + &vec3, `RET 0x8`
