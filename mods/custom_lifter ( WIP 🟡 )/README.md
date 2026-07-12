# custom_lifter

Modified Up Race lifter state machine via bass.dll proxy.

## What it does

Replaces the Rotator vtable[0x0B] render function (0x0043D420) with a custom state machine that modifies the lifter's timing, speed, and behavior:

| State | Description | Timer | Movement | Ball Carry | Sound |
|-------|-------------|-------|----------|------------|-------|
| 0 | Bottom pause | 300 frames | None | No | No |
| 1 | Rising | 1500 frames | +0.1 Y × 3 sub-steps/frame = 0.3/frame | No | No |
| 2 | Top pause | 300 frames | None | No | No |
| 3 | Falling | 1500 frames | -0.2 Y × 3 sub-steps/frame = -0.6/frame | No | No |
| → 0 | Return to bottom | 400 frames | None | No | No (arrival sound disabled) |

### Key differences from original:
- **Slower rise**: 0.3/frame vs 3.0/frame (10× slower)
- **Slower fall**: 0.6/frame vs 6.0/frame (10× slower)
- **No ball carrying**: Ball stays in place, lifter passes through
- **No sounds**: No clunk sound during rise, no arrival sound at bottom
- **Longer phases**: 300/1500/300/1500/400 vs 60/150/50/150/60
- **Equal sub-steps**: 3 per frame for both rise and fall (original: 3 rise, 6 fall)

## How it works

The DLL hooks the Rotator vtable at 0x004D5770 (entry 0x0B at offset 0x2C). On each render call:

1. Saves the custom state/timer/pos_y
2. Temporarily sets state=0, timer=99999, calls the original function (handles dirty-flag mesh repositioning without running the state machine)
3. Restores custom values
4. Runs the custom state machine

## Installation

1. Rename original `bass.dll` to `bass_real.dll` in the Hamsterball game folder
2. Copy this mod's `bass.dll` to the game folder
3. Set Wine DLL override to "native" for bass.dll (if on Android/Wine)

## Technical details

- **Hook target**: Rotator vtable[0x0B] at 0x004D579C (vtable 0x004D5770 + 0x2C)
- **Original function**: 0x0043D420
- **Hook method**: Vtable patch (no code cave, no IAT hook)
- **Struct offsets**: state=+0x10E4, timer=+0x10E8, pos_y=+0x10D8, dirty=+0x10F0
