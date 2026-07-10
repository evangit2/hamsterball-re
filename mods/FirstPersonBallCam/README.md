# First-Person Ball Cam

## Description
Pure CEA script that overrides the camera in Hamsterball to create a first-person ball cam experience. The camera is positioned directly above the ball and looks in the direction the ball is moving.

Works in both arenas and races.

## How It Works
- **Hook point:** `Scene_SetCamera` epilogue at `0x0041A254`
- **Camera position:** Ball position + `(0, radius + height, 0)` — directly above the ball
- **Camera direction:** Normalized ball velocity vector — wherever the ball is going
- **Stationary fallback:** When ball velocity is near zero, direction defaults to `(0, 0, 1)` (forward)
- The original camera orbit angle/distance system is bypassed entirely

## Adjustable Parameters (CE Address Table)
| Symbol | Default | Description |
|---|---|---|
| `fpCamHeight` | 10.0 | Height above ball surface (higher = more top-down, lower = more first-person) |
| `fpCamDist` | 1.0 | Camera distance from target (lower = more first-person) |

## Installation
1. Open Cheat Engine, attach to `Hamsterball.exe`
2. Load `FirstPersonBallCam.CEA`
3. Enable the script

## Tuning Tips
- **More first-person feel:** Lower `fpCamHeight` to ~3.0 and `fpCamDist` to ~0.5
- **More top-down feel:** Raise `fpCamHeight` to ~30.0
- **Arena combat:** `fpCamHeight=15.0`, `fpCamDist=2.0` gives good situational awareness
- **Race speedruns:** `fpCamHeight=5.0`, `fpCamDist=0.5` for immersive low cam

## Technical Details
- **Hook:** 5-byte JMP at `0x0041A254` → code cave
- **Camera object:** `Scene+0x87C`, vtable[2]=SetPosition, vtable[3]=SetDirection
- **Ball data:** position at `+0x14`, velocity at `+0x20`, radius at `+0x3C`
- **Jump-back:** `0x0041A2BF` (ApplyCamera: `graphics->vtable[1](camera)`)
- **Original bytes:** `8B 44 24 14 8B` (restored on disable)
