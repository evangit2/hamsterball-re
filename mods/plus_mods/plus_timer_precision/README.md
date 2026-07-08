# Timer Precision

Increases the arena timer display from 1 to 2 decimal places.

## What It Does

The game timer counts in ticks (100 ticks = 1 second). The original display shows seconds with 1 decimal digit (e.g., `12.3`). This mod changes it to 2 decimal digits (e.g., `12.34`), giving centisecond precision.

## How It Works

Two patches to `ArenaBoard_Render` (0x421910):

1. **Format string** (at 0x421BB3): Changes `PUSH 0x4D03F0` (`".%.1d"`) to push a custom `".%.2d"` string from DLL memory
2. **Computation** (at 0x421B8C, 27 bytes): Replaces `(timer/10)%10` (magic multiply by 0x66666667 + IDIV by 10) with `timer%100` (direct IDIV by 100)

### Original computation (27 bytes):
```asm
MOV EAX,0x66666667   ; magic multiplier for /10
IMUL ECX             ; EDX:EAX = timer * 0x66666667
MOV EAX,EDX          ; take high dword
SAR EAX,2
MOV ECX,EAX
SHR ECX,31           ; sign fixup
ADD EAX,ECX          ; EAX = timer/10
CDQ
MOV ECX,10
IDIV ECX             ; EDX = (timer/10) % 10
```

### Patched computation (27 bytes):
```asm
MOV EAX,ECX          ; EAX = timer
CDQ                  ; sign extend
MOV ECX,100
IDIV ECX             ; EDX = timer % 100
NOP * 17             ; fill remaining space
```

## Scope

Currently patches the **arena timer** (Rodent Rumble HUD). The results screen and race HUD timers use different code paths with pre-computed values — those are not patched in this version.

## Controls

Toggle in Options menu: **Timer Precision (2 decimals)** (default: ON)

## Limitations

- Patches are applied at initialization and cannot be cleanly un-patched at runtime
- Only the arena timer is affected; other timer displays keep 1 decimal
- Maximum meaningful precision is 2 decimals (100 ticks/second = centisecond resolution)

## Author

BookwormKevin
