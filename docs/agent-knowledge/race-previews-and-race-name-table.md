# Race Previews & Race-Name Table

Byte-level analysis of how Hamsterball displays race previews (practice menu
icons + tournament lineup strip) and where the race names come from —
including the "YOU FOUND A BUG RACE" overflow easter egg.

Source: `originals/installed/extracted/Hamsterball.exe` (image base 0x400000).

## 1. Two separate preview systems

There are TWO independent sets of preview images, used by two different menus.

### 1.1 Practice menu — `practice-*.png` sprites

| Item | Value |
|------|-------|
| Strings | `practice-level1.png` .. `practice-impossible.png` @ 0x4D4360-0x4D4518 (15 strings, 0x18-0x20 bytes apart) |
| Ctor | PracticeMenu_ctor @ 0x42EA30 (vtable 0x4D4560) |
| Per-entry code | 0x42EAC2 .. 0x42EE5E (one 0x42-byte block per race, 15 total) |
| Sprite alloc | `operator_new(0xD4)` @ 0x4BA57B |
| Sprite ctor | 0x45D0C0 (surface-ish ctor, vtable 0x4D8F84; width/height @ +0x60/+0x64, handles @ +0x68/+0x84/+0xA0/+0xBC, scale 1.0 @ +0x60) |
| Args | texture-manager ptr from `board+0x174` + png path string |
| Storage | menu object @ +0xCDC, +0xCE0, +0xCE4 .. +0xCFC (one slot per race) |
| Render | vtable[2] = 0x449D40 (UIList_Render) — standard menu item list |

The practice menu loads all 15 previews eagerly at ctor time; each becomes a
0xD4-byte sprite object in the menu's item slots. Rendering is the standard
UIList path (no special-case render code).

### 1.2 Tournament lineup — `tourney-*.png` icon strip

| Item | Value |
|------|-------|
| Strings | `tourney-beginner.png` .. `tourney-Impossible.png` @ 0x4D3050-0x4D3168 (15 strings) |
| Load site | 0x42A350-0x42A4B0 (inside TimerDisplay region, fn start 0x4298C7) |
| Load call | `App+0x22C` (texture manager) vtable[0x58] — `call *0x58(%eax)` with (output slot, string) |
| Storage | **App+0x3B4 .. App+0x3EC** — 15 consecutive texture slots, 4 bytes each (0x3B4 + i*4) |
| Render fn | 0x450AF0 (TourneyMenu_Render, vtable 0x4D83F0 slot [2]) |
| Render loop | 0x450C46-0x450CD8 |
| Draw helper | 0x455D60 — draws 128x128 (0x80/0x80) textured quad at (idx, y) |
| Spacing | 0x9B (155 px) vertical; Y = (0x16 - scroll) * 0x9B |
| Loop bounds | `mov $0x3b4,%ebx` / `add $0x4,%ebx` / `cmp $0x3f0,%ebx` / `jl` = exactly 15 icons |

Key disassembly (0x450C45-0x450CD8):

```
450c45:  bb b4 03 00 00        mov    $0x3b4,%ebx
450c50:  sub    $0x14,%esp
450c59:  push   $0x3f800000     ; scale
450c5e:  push   $0x0
450c60:  push   $0x0
450c62:  push   $0x0
450c64:  call   0x453150        ; Vec3/scale setup
450c69:  push   $0x80           ; h
450c6e:  push   $0x80           ; w
450c73:  lea    0x5(%ebp),%eax  ; y = (0x16-scroll)*0x9B + 5
450c76:  push   %eax
450c77:  push   $0x7f           ; z / layer
450c79:  call   0x455d60        ; draw textured quad
450c86:  mov    0x878(%esi),%ecx
450c86:  mov    (%ebx,%ecx,1),%edx  ; tex = App+0x3B4[i]   (ECX=App, EBX=slot)
450cc5:  add    $0x9b,%ebp      ; next y
450ccb:  add    $0x4,%ebx       ; next slot
450cce:  cmp    $0x3f0,%ebx
450cd8:  jl     0x450c50        ; loop while slot < 0x3F0 (i < 15)
```

So the tournament between-races screen draws a vertically scrollable strip of
all 15 tourney icons as 128x128 quads spaced 155px apart. The textures are
preloaded once into App+0x3B4..0x3EC at startup.

## 2. Race-name table (0x4F7080) and the "NEXT UP:" banner

### 2.1 The table

Race-name pointer table at **0x4F7080**: **16 entries** (indices 0-15).

| Index | String VA | Name |
|-------|-----------|------|
| 0 | 0x4D2878 | WARM-UP |
| 1 | 0x4D2818 | CASCADE |
| 2 | 0x4D2858 | INTERMEDIATE |
| 3 | 0x4D2878 | DIZZY |
| 4 | 0x4D28B8 | TOWER |
| 5 | 0x4D28F8 | NEON |
| 6 | 0x4D2938 | EXPERT |
| 7 | 0x4D2978 | ODD |
| 8 | 0x4D29B8 | TOOB |
| 9 | 0x4D29F8 | WOBBLY |
| 10 | 0x4D2A38 | GLASS |
| 11 | 0x4D2A78 | SKY |
| 12 | 0x4D2AB8 | MASTER |
| 13 | 0x4D2AF8 | IMPOSSIBLE |
| 14 | 0x4D219C | IMPOSSIBLE RACE |
| **15** | **0x4D2610** | **YOU FOUND A BUG RACE** |

(Indices 0-13 are the 14 actual races; [14] is a second IMPOSSIBLE string;
[15] is the overflow/easter-egg slot. Exact per-entry strings verified by
dumping the table.)

### 2.2 Accessor

```
0x4264A0:  mov 0x4f7080(,%eax,4),%eax   ; NO bounds check
```

`GetRaceName(profile+8)` — reads table[race_index] with no validation.
Called with ECX = profile (App+0x220).

### 2.3 Sole caller — the "NEXT UP:" banner

Only caller: **0x451430**, inside fn 0x450AF7 (TourneyMenu render region).
Context (0x4513B0-0x451442):

```
4513b0:  mov 0x878(%esi),%ecx      ; App
4513b6:  push $0x2; push $0x2; push $0xa; push $0x140
4513bc:  push 0x4d860c             ; "NEXT UP:"
4513cd:  call 0x4012c0             ; draw text with font App+0x318
...
451415:  mov 0x878(%esi),%edx
45141b:  mov 0x220(%edx),%ecx      ; profile
451430:  call 0x4264a0             ; GetRaceName(profile+8)
451435:  push %eax
45143c:  mov 0x318(%eax),%ecx      ; big title font
451442:  call 0x4012c0             ; draw race name
```

So the tournament between-races screen draws "NEXT UP:" followed by the race
name from table[profile+8], using the big title font (App+0x318).

## 3. "YOU FOUND A BUG RACE" reachability

- The string (0x4D2610) has **no direct references** in .text — it is reached
  ONLY via table[15] at 0x4F7080 through the accessor 0x4264A0.
- Tournament_AdvanceRace bounds check at **0x4270F9**: `cmp $0xe,%eax; ja
  0x4273F5` — only indices 0-14 dispatch (jump table 0x42761C). Index 15
  jumps straight to a silent dead-end epilogue, never reaching a board ctor.
- TourneyMenu_CreateBoard jump table at 0x426AB0 has the same bounds.
- Therefore the string is only displayable if profile+8 == 15 at the 0x451430
  call site, which normal tournament play can never produce (race index is
  incremented 0-14 and bounds-checked). It is an unreachable easter egg in
  normal play — effectively dead code; would need memory corruption or a mod
  to display.

## 4. Menu vtables reference

| Vtable | Class | Notes |
|--------|-------|-------|
| 0x4D4560 | PracticeMenu | ctor 0x42EA30; renders via 0x449D40 (UIList_Render) |
| 0x4D83F0 | TourneyMenu | "Tourney Menu" name @ +0x868 (0x4D83DC); render = 0x450AF0 (slot [2]); icon strip loop 0x450C46 |
| 0x4D8628 | TourneyMenu (2nd variant) | render = 0x452160; "NEXT UP:" banner region 0x451380-0x451442; race-name display 0x451430 |
| 0x4D47C0 | ArenaLevelSelect | render = 0x432D20 (no tourney icons) |

Both TourneyMenu ctors write vtable 0x4D83F0 (at 0x44FDD6 and 0x45030D) and
the "Tourney Menu" name string; the class with vtable 0x4D8628 renders the
text banner. The 0x4D83F0 vtable's render (0x450AF0) draws the icon strip;
the 0x4D8628 variant's render (0x452160) draws "NEXT UP:" + race name.
