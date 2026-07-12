# Timer Precision

Increases **all** timer displays from 1 to 2 decimal places.

## What It Does

The game timer counts in ticks (100 ticks = 1 second). The original display shows seconds with 1 decimal digit (e.g., `12.3`). This mod changes it to 2 decimal digits (e.g., `12.34`), giving centisecond precision.

## Coverage (14 sites across 6 functions)

| Location | Function | Description |
|----------|----------|-------------|
| 0x421B8C | ArenaBoard_Render | Arena timer |
| 0x41BE1C | FUN_0041B710 | Race HUD timer |
| 0x41C229 | FUN_0041BFD0 | Split-screen timer #1 |
| 0x41C4C5 | FUN_0041BFD0 | Split-screen timer #2 |
| 0x44CF86 | FUN_0044CD10 | Tournament results: time remaining |
| 0x44D18A | FUN_0044CD10 | Tournament results: par time |
| 0x44E25F | FUN_0044DF70 | TT results: race time |
| 0x44E448 | FUN_0044DF70 | TT results: best time |
| 0x44E63C | FUN_0044DF70 | TT results: weasel time |
| 0x44EB19 | FUN_0044DF70 | TT results: bronze time |
| 0x44ECFF | FUN_0044DF70 | TT results: silver time |
| 0x44EEE5 | FUN_0044DF70 | TT results: gold time |
| 0x451157 | TourneyMenu_Render | Tournament menu #1 |
| 0x451935 | TourneyMenu_Render | Tournament menu #2 |

## How It Works

Each site gets two patches:

1. **Format string** (5 bytes): Changes `PUSH 0x4D03F0` (`".%.1d"`) to push a custom `".%.2d"` string from DLL memory
2. **Computation** (27 bytes): Replaces `(timer/10)%10` (magic multiply by 0x66666667 + DIV/IDIV by 10) with `timer%100` (direct DIV by 100)

### Register variants

The timer value lives in different registers depending on the call site:

| Register | MOV instruction | Sites |
|----------|-----------------|-------|
| ECX | `MOV EAX,ECX` (8B C1) | 11 sites |
| EDI | `MOV EAX,EDI` (8B C7) | 2 sites |
| EBP | `MOV EAX,EBP` (8B C5) | 1 site |

### Original computation (27 bytes):
```asm
MOV EAX,0x66666667   ; magic multiplier for /10
IMUL <reg>            ; EDX:EAX = timer * 0x66666667
MOV EAX,EDX           ; take high dword
SAR EAX,2
MOV ECX,EAX
SHR ECX,31            ; sign fixup
ADD EAX,ECX           ; EAX = timer/10
CDQ
MOV ECX,10
DIV/IDIV ECX          ; EDX = (timer/10) % 10
```

### Patched computation (27 bytes):
```asm
MOV EAX,<timer_reg>   ; EAX = timer
CDQ                   ; sign extend
MOV ECX,100
DIV ECX               ; EDX = timer % 100
NOP * 17              ; fill remaining space
```

## Controls

Toggle in Options menu: **Timer Precision (2 decimals)** (default: ON)

## Limitations

- Patches are applied at initialization and cannot be cleanly un-patched at runtime
- 2 decimals is the max meaningful precision (100 ticks/second = centisecond resolution)
- A 3rd decimal would always be 0

## Author

BookwormKevin
