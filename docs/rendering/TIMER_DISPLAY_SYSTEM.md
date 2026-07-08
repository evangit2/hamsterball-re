# Timer Display System

How Hamsterball formats and renders timer values on screen.

## Timer Storage

The game stores time as an **integer tick count** where **100 ticks = 1 second**.

| Context | Storage Location | Notes |
|---------|-----------------|-------|
| Arena timer (Rodent Rumble) | `board + 0x47AC` | Counts down from 6000 (60s default) |
| Race timer (Time Trial/Tournament) | `board + 0x4340` | Counts up during race |
| Best time / medal times | Pre-computed in results struct | Stored before results screen renders |

## Display Format

The timer is displayed in two parts, rendered as separate text draw calls:

1. **Integer seconds**: formatted with `%d` (string at `0x4D03F8`) via `AthenaString_Format(0x4F7448, &DAT_004D03F8)`
2. **Decimal fraction**: formatted with `.%.1d` (string at `0x4D03F0`) — drawn at a slight Y offset below the integer part using `UI_DrawTextShadow_Wrapper`

The `%d` format string lives at `0x4D03F8` (4 bytes: `25 64 00 00` = `"%d\0"`).
The `.%.1d` format string lives at `0x4D03F0` (7 bytes: `2E 25 2E 31 64 00 00` = `".%.1d\0"`).

### Decimal Computation

The decimal digit is computed from the raw tick count:

```
decimal = (timer / 10) % 10
```

This gives a single digit 0–9 representing tenths of a second.

The game implements this using **magic-number multiplication** for the division by 10:

```asm
; ECX (or EDI, or EBP) = timer tick count at entry
MOV EAX, 0x66666667    ; magic multiplier for signed /10
IMUL <reg>             ; EDX:EAX = timer * 0x66666667
MOV EAX, EDX           ; take high dword
SAR EAX, 2             ; arithmetic shift right by 2
MOV ECX, EAX           ; (or MOV EDX, EAX at some sites)
SHR ECX, 31            ; sign fixup
ADD EAX, ECX           ; EAX = timer / 10
CDQ                    ; sign extend for division
MOV ECX, 10            ; divisor
DIV/IDIV ECX           ; EDX = (timer / 10) % 10 = decimal digit
```

This 27-byte block is identical at all 14 timer display sites, with three register variants:

| Variant | IMUL register | Timer value in | Sites | Patch MOV |
|---------|--------------|----------------|-------|-----------|
| ECX | `F7 E9` (IMUL ECX) | ECX | 11 | `8B C1` (MOV EAX,ECX) |
| EDI | `F7 EF` (IMUL EDI) | EDI | 2 | `8B C7` (MOV EAX,EDI) |
| EBP | `F7 ED` (IMUL EBP) | EBP | 1 | `8B C5` (MOV EAX,EBP) |

Some sites use `DIV ECX` (unsigned, `F7 F1`) and others use `IDIV ECX` (signed, `F7 F9`) for the final division. Since timer values are always positive, both produce the same result.

## All 14 Display Sites

Found by searching for the magic multiplier byte pattern `B8 67 66 66 66` and cross-referencing with `.%.1d` data references:

| # | Computation Addr | Format PUSH Addr | Function | Context | Register |
|---|------------------|-----------------|----------|---------|----------|
| 1 | `0x421B8C` | `0x421BB3` | `ArenaBoard_Render` (0x421910) | Arena timer | ECX |
| 2 | `0x41BE1C` | `0x41BE47` | `FUN_0041B710` (0x41B710) | Race HUD timer | ECX |
| 3 | `0x41C229` | `0x41C250` | `FUN_0041BFD0` (0x41BFD0) | Split-screen timer #1 | EDI |
| 4 | `0x41C4C5` | `0x41C4EC` | `FUN_0041BFD0` (0x41BFD0) | Split-screen timer #2 | EDI |
| 5 | `0x44CF86` | `0x44CFB7` | `FUN_0044CD10` (0x44CD10) | TT results: time remaining | ECX |
| 6 | `0x44D18A` | `0x44D1B8` | `FUN_0044CD10` (0x44CD10) | TT results: par time | ECX |
| 7 | `0x44E25F` | `0x44E290` | `FUN_0044DF70` (0x44DF70) | Results: race time | ECX |
| 8 | `0x44E448` | `0x44E479` | `FUN_0044DF70` (0x44DF70) | Results: best time | ECX |
| 9 | `0x44E63C` | `0x44E66D` | `FUN_0044DF70` (0x44DF70) | Results: weasel time | ECX |
| 10 | `0x44EB19` | `0x44EB4A` | `FUN_0044DF70` (0x44DF70) | Results: bronze time | ECX |
| 11 | `0x44ECFF` | `0x44ED30` | `FUN_0044DF70` (0x44DF70) | Results: silver time | ECX |
| 12 | `0x44EEE5` | `0x44EF16` | `FUN_0044DF70` (0x44DF70) | Results: gold time | ECX |
| 13 | `0x451157` | `0x451185` | `TourneyMenu_Render` | Tournament menu #1 | ECX |
| 14 | `0x451935` | `0x451963` | `TourneyMenu_Render` | Tournament menu #2 | EBP |

### Functions not patched (non-timer)

Four additional `B8 67 66 66 66` sites exist but are NOT timer displays:

| Address | Function | Purpose |
|---------|----------|---------|
| `0x43CE54` | `Blockdawg_ctor` area | Not /10 for timer display |
| `0x43CFB3` | `Blockdawg_ctor` area | Not /10 for timer display |
| `0x44BBF7` | Unknown | Different post-multiply code |
| `0x44C20D` | Unknown | Different post-multiply code |

These were identified by reading 27 bytes at each address and comparing — the code after the magic multiply diverges from the timer pattern (no `MOV ECX,10` / `IDIV ECX` sequence).

## Rendering Pipeline

The timer is drawn through the game's UI text system:

1. `AthenaString_Format(0x4F7448, formatString)` — formats the value into a temp string buffer
   - `0x4F7448` is a global `AthenaString` object (temporary format buffer)
   - Returns a `char*` to the formatted result
2. `UI_DrawTextCentered(font, formattedStr, x, y, ...)` — draws the integer part
3. `UI_DrawTextShadow_Wrapper(smallerFont, formattedDecimal, x, y+offset, ...)` — draws the decimal part at a Y offset

### Fonts used

| Font | Loaded at | Used for |
|------|-----------|---------|
| `showcardgothic72` | `board+0x328` | Large timer integer (arena HUD) |
| `showcardgothic28` | `board+0x318` | Medium timer (results, menus) |
| `showcardgothic14` | `board+0x31C` | Small timer decimal part |
| `showcardgothic16` | `board+0x320` | Score display |

## Precision Limit

Since 100 ticks = 1 second, the maximum meaningful display precision is **2 decimal places** (centiseconds). A 3rd decimal would always be 0. The `plus_timer_precision` mod exploits this by changing the computation from `(timer/10)%10` to `timer%100`, giving the full centisecond resolution that the timer value already contains.

## Related

- **`AthenaString_Format`** (0x466C70) — 98 xrefs, general-purpose string formatter, wraps `sprintf` into an `AthenaString` object's internal buffer
- **`UI_DrawTextCentered`** — draws centered text at screen coordinates
- **`UI_DrawTextShadow_Wrapper`** — draws text with a drop shadow (used for the decimal part)
- **`Scene_RenderTimerHUD`** (0x41BFD0) — renders the in-race timer HUD including oval background and text
- **`App_ResourceLoader`** (0x4298C0) — loads `timerblot.png` and all timer-related textures/fonts
- **`plus_timer_precision` mod** — patches all 14 sites to display 2 decimal places
