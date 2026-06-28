# Hamsterball UI Text Elements: Drawing On-Screen Text

**Scope:** Original `Hamsterball.exe` (PE32, i386, Athena engine, VS2003).
**Method:** Direct Ghidra decompilation and disassembly verification.
**Last Updated:** 2026-06-20 (revision 2 — vtable params, Color struct passing, troubleshooting)

---

## 1. Overview

This document explains how to draw text on screen in Hamsterball using the
engine's built-in bitmap-font rendering pipeline. It covers the correct `this`
(Font*) pointer chain, function signatures, color/transform struct layout, and
practical DLL-mod examples.

The old version of this doc incorrectly described the `this` pointer as
"GraphicsDevice/App" and gave wrong parameter descriptions. All findings below
are verified against raw Ghidra decompilation and disassembly, cross-referenced
with actual call sites in the game code.

**Revision 2 adds:** critical finding that params 6 and 11 are overwritten
internally by `UI_DrawTextCentered`/`UI_DrawTextShadow_Wrapper`, troubleshooting
for Color struct passing, and a concrete failing-call analysis.

---

## 2. The Font Object

All text drawing functions are `__thiscall` with `this` = a **Font object
pointer**, NOT the App, GraphicsDevice, or Scene.

### 2.1 Font* pointer chain from App

The global App pointer is at `0x005341E0`:

```
void* g_App = *(void**)0x005341E0;
```

The App object stores pointers to loaded Font objects at these offsets
(verified via `decomp_resource_manifest.c` — the master asset loader at
`0x0042A8C0`):

| App Offset | Font Path                  | Usage                     |
|------------|----------------------------|---------------------------|
| `+0x318`   | `fonts\showcardgothic28`   | Main title / UI text      |
| `+0x31C`   | `fonts\showcardgothic14`   | Small label text          |
| `+0x324`   | `fonts\arialnarrow12bold`  | UI detail text            |
| `+0x328`   | `fonts\showcardgothic72`   | Race timer digits (large) |
| `+0x320`   | `fonts\showcardgothic16`   | Info text                 |

### 2.2 How to get the Font* in a DLL mod

```cpp
// Global App pointer (Athena engine)
void* g_App = *(void**)0x005341E0;

// Get font pointers
void* font_title    = *(void**)((char*)g_App + 0x318);  // showcardgothic28
void* font_small    = *(void**)((char*)g_App + 0x31C);  // showcardgothic14
void* font_detail   = *(void**)((char*)g_App + 0x324);  // arialnarrow12bold
void* font_timer    = *(void**)((char*)g_App + 0x328);  // showcardgothic72
void* font_info     = *(void**)((char*)g_App + 0x320);  // showcardgothic16
```

### 2.3 Font struct layout (from `LoadFont` at `0x00457130`)

| Offset | Type     | Field                          |
|--------|----------|--------------------------------|
| `+0x00`| `void*`  | vtable                         |
| `+0x04`| `void*`  | Graphics device ptr            |
| `+0x08`| AthenaList| Glyph texture list             |
| `+0x420`| `int`   | Space width                    |
| `+0x424`| `int`   | Max line height               |
| `+0x428`| `float` | Scale (default 1.0 = `0x3f800000`) |
| `+0x42C`| `char[0x500]` | Per-glyph table (0x14 bytes × 128 entries) |

Each glyph entry at `font + char × 0x14 + 0x42C`:

| Offset | Type  | Field         |
|--------|-------|---------------|
| `+0x00`| `char`| Valid flag    |
| `+0x04`| `int` | advance_width |
| `+0x08`| `int` | offset_x      |
| `+0x0C`| `int` | offset_y      |
| `+0x10`| `int` | width         |
| `+0x14`| `void*`| sprite ptr   |

> **Note:** The glyph entry offsets are approximate based on `LoadFont` and
> `Font_DrawGlyph` decompilation. The key fields used by the renderer are:
> valid flag (`+0x00`), advance width (`+0x04`→`+0x430` via index math),
> and sprite pointer (`+0x43C`).

---

## 3. Color / Transform Struct

The text-drawing functions use a **5-DWORD struct** for color/transform data.
This is NOT a simple RGBA int — it's a struct with a vtable pointer followed
by 4 floats (R, G, B, A).

### 3.1 Static identity transform

A pre-built static struct exists at `0x004CF300`:

| Offset | Value         | Meaning       |
|--------|---------------|---------------|
| `+0x00`| `0x00401070`  | vtable (Vec3_dtor) |
| `+0x04`| `255.0f`      | R (white)     |
| `+0x08`| (garbage)     | G             |
| `+0x0C`| `1.45f`       | B / scale     |

In practice, most callers use `Matrix_Scale4x4` to build a fresh 4×4 matrix on
the stack, then pass its address. The first DWORD of the matrix is treated as a
vtable pointer by the rendering code.

### 3.2 How the game builds color structs

From `TourneyContinueDialog_Render` (`0x00445F50`):

```cpp
// The game creates two 20-byte structs on the stack
// (one for text color, one for shadow color)
// via Matrix_Scale4x4, then passes them to UI_DrawTextShadow.

float text_color[5];    // [vtable, R, G, B, A]
float shadow_color[5];   // [vtable, R, G, B, A]

// Matrix_Scale4x4 fills the struct with a scale matrix
Matrix_Scale4x4(text_color, 0, 0, 0, 1.0f);    // identity
Matrix_Scale4x4(shadow_color, 1.0f, 1.0f, 1.0f, 1.0f); // white shadow
```

**Practical shortcut:** Use `0x004CF300` as the vtable pointer value and fill
the rest with floats. Or better yet, use the wrapper function (§4.3) which
auto-fills the vtable.

---

## 4. Text Drawing Functions

### 4.1 `UI_DrawTextShadow` — Full control (15 stack params + this)

**Address:** `0x004012C0`
**Calling convention:** `__thiscall` (ECX = Font*)
**RET:** `0x3C` (15 × 4 = 60 bytes cleaned)

```cpp
void __thiscall UI_DrawTextShadow(
    Font* this,           // ECX
    char* text,           // [ESP+0x00] param_1
    int x,                // [ESP+0x04] param_2
    int y,                // [ESP+0x08] param_3
    int shadow_dx,        // [ESP+0x0C] param_4
    int shadow_dy,        // [ESP+0x10] param_5
    void* text_xform_vtbl,// [ESP+0x14] param_6  — vtable ptr for text color
    float text_r,         // [ESP+0x18] param_7
    float text_g,         // [ESP+0x1C] param_8
    float text_b,         // [ESP+0x20] param_9
    float text_a,         // [ESP+0x24] param_10
    void* shdw_xform_vtbl,// [ESP+0x28] param_11 — vtable ptr for shadow color
    float shadow_r,       // [ESP+0x2C] param_12
    float shadow_g,       // [ESP+0x30] param_13
    float shadow_b,       // [ESP+0x34] param_14
    float shadow_a        // [ESP+0x38] param_15
);
```

This is the lowest-level text function. It draws text with a drop-shadow.
The color params are part of 5-DWORD transform structs (vtable + 4 floats).

**This is the most complex function and hardest to call directly.**
Prefer the wrapper (§4.3) or Font_DrawCentered (§4.4) instead.

### 4.2 `UI_DrawTextCentered` — Centered with shadow (15 params + this)

**Address:** `0x00409C60`
**Calling convention:** `__thiscall` (ECX = Font*)
**RET:** `0x3C` (15 × 4 = 60 bytes cleaned)

Same signature as `UI_DrawTextShadow`. Internally:
1. Builds two 5-DWORD color structs on the stack, using `0x4CF300` as the
   vtable pointer (params 6 and 11 from the caller are **ignored/overwritten**)
2. Calls `Font_MeasureText(this, text)` to get text width
3. Subtracts width/2 from x to center
4. Calls `UI_DrawTextShadow` with the two internally-built structs

**The `this` pointer MUST be a Font\*, obtained via `*(App + 0x318)` etc.**

> **CRITICAL (verified from disassembly):** Params 6 and 11 are overwritten
> with the hardcoded absolute address `0x4CF300` inside the function body:
> ```
> 00409c7f: MOV dword ptr [EAX],0x4cf300   ; overwrites param_6 slot
> 00409cae: MOV dword ptr [EAX],0x4cf300   ; overwrites param_11 slot
> ```
> This means you do NOT need to pass a valid vtable pointer for params 6
> and 11 — they are replaced internally. Pass 0, NULL, or any value; it
> will be overwritten. The actual color data lives in params 7–10 (text
> RGBA) and 12–15 (shadow RGBA) as individual floats.

### 4.3 `UI_DrawTextShadow_Wrapper` — Easiest high-level (15 params + this)

**Address:** `0x00409B90`
**Calling convention:** `__thiscall` (ECX = Font*)
**RET:** `0x3C` (15 × 4 = 60 bytes cleaned)

```cpp
void __thiscall UI_DrawTextShadow_Wrapper(
    Font* this,           // ECX
    char* text,           // param_1  — text to draw
    int x,                // param_2  — screen X
    int y,                // param_3  — screen Y
    int shadow_dx,        // param_4  — shadow pixel offset X (typically 2-5)
    int shadow_dy,        // param_5  — shadow pixel offset Y (typically 2-5)
    void* unused_6,       // param_6  — IGNORED (replaced with &0x4CF300 internally)
    float text_r,         // param_7  — text red   (0.0-1.0)
    float text_g,         // param_8  — text green (0.0-1.0)
    float text_b,         // param_9  — text blue  (0.0-1.0)
    float text_a,         // param_10 — text alpha (0.0-1.0)
    void* unused_11,      // param_11 — IGNORED (replaced with &0x4CF300 internally)
    float shadow_r,      // param_12 — shadow red
    float shadow_g,       // param_13 — shadow green
    float shadow_b,       // param_14 — shadow blue
    float shadow_a        // param_15 — shadow alpha
);
```

**This is the RECOMMENDED function for DLL mods.** It auto-fills the vtable
pointers (`0x4CF300`) for both color structs, so you only pass raw RGBA floats.
Pass `0` (NULL) for params 6 and 11 — they're overwritten internally.

### 4.4 `Font_DrawCentered` — No shadow, auto-centered (8 params + this)

**Address:** `0x0042C870`
**Calling convention:** `__thiscall` (ECX = Font*)
**RET:** `0x20` (8 × 4 = 32 bytes cleaned)

```cpp
void __thiscall Font_DrawCentered(
    Font* this,           // ECX
    char* text,           // param_1
    int x,                // param_2  — center X
    int y,                // param_3  — Y
    void* unused_4,       // param_4  — IGNORED (overwritten with &0x4CF300)
    float r,              // param_5  — red   (0.0-1.0)
    float g,              // param_6  — green (0.0-1.0)
    float b,              // param_7  — blue  (0.0-1.0)
    float a               // param_8  — alpha (0.0-1.0)
);
```

This is the **simplest text function** — no shadow, auto-centered, auto-fills
the transform vtable. Internally calls `Font_MeasureText` then `Font_DrawGlyph`.

**Note on color:** When font scale (`Font+0x428`) == 1.0f (the default), the
color params are IGNORED — each glyph sprite is drawn via `Sprite_DrawRect`
which uses `Color_RandomRGBA()` internally. To get colored text, you must
either set `Font+0x428` to something other than 1.0 (which triggers the
`Scene_CreateObject4f` path that uses the color params), or accept the default
white text from the sprite textures.

### 4.5 `Font_DrawGlyph` — Raw glyph rendering (8 params + this)

**Address:** `0x00457440`
**Calling convention:** `__thiscall` (ECX = Font*)
**RET:** `0x20` (8 × 4 = 32 bytes cleaned)

```cpp
void __thiscall Font_DrawGlyph(
    Font* this,           // ECX
    char* text,           // param_1  — text (iterates each char)
    int x,                // param_2  — start X (left-aligned)
    int y,                // param_3  — start Y
    void* xform_vtbl,     // param_4  — vtable ptr (use 0x4CF300)
    float r,              // param_5
    float g,              // param_6
    float b,              // param_7
    float a               // param_8
);
```

This is the lowest-level text function. Iterates each character, looks up its
glyph at `Font + char × 0x14 + 0x42C`, and draws it via `Sprite_DrawRect`
(scale 1.0) or `Scene_CreateObject4f` (scaled). Same color caveat as
`Font_DrawCentered` — colors only work when scale ≠ 1.0.

### 4.6 `Font_MeasureText` — Measure string width

**Address:** `0x00456E20`
**Calling convention:** `__thiscall` (ECX = Font*)
**RET:** `0x04` (1 × 4 = 4 bytes cleaned)

```cpp
int __thiscall Font_MeasureText(Font* this, char* text);
```

Returns total advance width in pixels for the string. Uses `Font+0x428` (scale)
and each glyph's advance width at `Font + char × 0x14 + 0x430`.

### 4.7 `Font_DrawGlyph3D` — World-space text (18 params + this)

**Address:** `0x00457690`
**Calling convention:** `__thiscall` (ECX = Font*)
**RET:** `0x48` (18 × 4 = 72 bytes cleaned)

Draws text in 3D world space with arbitrary orientation vectors. Too complex
for simple HUD use. Not recommended for DLL mods.

---

## 5. Why `0x409C60` May Not Work

The function at `0x409C60` (`UI_DrawTextCentered`) is **correct and works**,
but there are several reasons it may fail in practice:

### 5.1 Wrong `this` pointer

The #1 cause of failure. The old version of this doc said `this` was
"GraphicsDevice/App" — **this is wrong**. The `this` pointer MUST be a `Font*`
obtained from `App+0x318` (or another font offset). Passing the App, Scene,
or GraphicsDevice pointer will crash inside `Font_DrawGlyph` when it tries to
access `this+0x42C` (the glyph table).

**Correct:**
```cpp
void* app  = *(void**)0x005341E0;
void* font = *(void**)((char*)app + 0x318);
UI_DrawTextCentered(font, "Hello", 400, 300, ...);
```

**Wrong (will crash):**
```cpp
void* app = *(void**)0x005341E0;
UI_DrawTextCentered(app, "Hello", 400, 300, ...);  // CRASH!
```

### 5.2 Too many parameters

`UI_DrawTextCentered` takes 15 stack params + ECX (this). If you're calling it
from C/C++ with `__thiscall`, you must push exactly 15 DWORDs. Many callers
get the count wrong. Use `Font_DrawCentered` (8 params) or
`UI_DrawTextShadow_Wrapper` (15 params but auto-fills vtable) instead.

### 5.3 Color struct vtable not set — **DEBUNKED**

> **Previous doc said:** "Params 6 and 11 must be a valid vtable pointer
> (`0x4CF300`) or the function will crash."
>
> **WRONG.** Verified from disassembly: `UI_DrawTextCentered` (0x409C60)
> overwrites params 6 and 11 with the hardcoded immediate `0x4CF300`:
> ```
> 00409c7f: MOV dword ptr [EAX],0x4cf300   ; param_6 overwritten
> 00409cae: MOV dword ptr [EAX],0x4cf300   ; param_11 overwritten
> ```
> The same is true for `UI_DrawTextShadow_Wrapper` (0x409B90).
> Params 6 and 11 are **always replaced internally** — you can pass 0, NULL,
> or garbage. The actual color data is the 4 floats after each vtable slot
> (params 7–10 and 12–15).

### 5.4 Font not loaded yet

The font pointers at `App+0x318` etc. are only valid after the resource
loader (`0x0042A8C0`) has completed. If you call during early initialization,
the pointer will be NULL. Hook after the loading screen completes.

### 5.5 Color struct not expanded to individual floats

If your mod API wrapper (`CallMethod` or similar) takes a `Color` struct
(e.g. `struct Color { float r, g, b, a; }`) as a single parameter, it may
pass it as a **pointer** (1 DWORD) instead of pushing 4 individual floats
onto the stack. This results in only 9 DWORDs on the stack instead of 15,
causing stack corruption and a crash when the function executes `RET 0x3C`
(it tries to clean 60 bytes but only 36 were pushed).

**Fix:** Pass each color component as a separate `float` argument:

```cpp
// WRONG — Color struct may be passed as pointer (1 DWORD):
CallMethod(0x409C60, font, "text", x, y, 5, 5,
    0, Color(0.5f, 0.5f, 0.5f, 0.8f),
    0, Color(0.0f, 0.0f, 0.0f, 1.0f));

// CORRECT — 15 individual DWORDs on the stack:
CallMethod(0x409C60, font, "text", x, y, 5, 5,
    0,                           // param_6 (ignored, overwritten)
    0.5f, 0.5f, 0.5f, 0.8f,     // params 7-10: text RGBA
    0,                           // param_11 (ignored, overwritten)
    0.0f, 0.0f, 0.0f, 1.0f);    // params 12-15: shadow RGBA
```

### 5.6 Y coordinate at screen edge

Setting `y=0` places text at the very top of the screen. Depending on the
font's glyph baseline offset and the viewport, the text may be partially or
fully clipped. Use `y=20` or higher to ensure visibility.

### 5.7 Failing call analysis (real example)

This call was reported as not working:

```cpp
DWORD vtable = baseAddr + 0xCF300;
void* font = *(void**)((char*) api->GetApp() + 0x318);
CallMethod(0x409C60, font, (char*)"69420", 437, 0, 5, 5,
    vtable, Color(.5f, .5f, .5f, .8f),
    vtable, Color(0.0f, 0.0f, 0.0f, 1.0f));
```

**Issues identified:**

1. **`vtable` param is unnecessary.** Params 6 and 11 are overwritten with
   `0x4CF300` inside the function. The `baseAddr + 0xCF300` calculation is
   wasted effort and irrelevant.

2. **`Color()` struct passing.** If `CallMethod` passes `Color()` as a struct
   by pointer (1 DWORD) instead of expanding to 4 floats (4 DWORDs), the
   stack is misaligned. This is the **most likely cause of failure**.
   Total params would be 9 instead of 15.

3. **`y=0`** may cause text to be clipped at the top of the screen.

4. **No null check on font.** If `App+0x318` is NULL (font not loaded yet),
   the call will crash.

**Recommended fix** — use `Font_DrawCentered` (8 params, simplest):

```cpp
void* font = *(void**)((char*)api->GetApp() + 0x318);
if (!font) return;
CallMethod(0x42C870, font, (char*)"69420", 437, 20,
    0,                    // ignored (overwritten with 0x4CF300)
    0.5f, 0.5f, 0.5f, 0.8f);  // RGBA (note: only works if font scale ≠ 1.0)
```

Or use `UI_DrawTextShadow_Wrapper` (15 params, with shadow):

```cpp
void* font = *(void**)((char*)api->GetApp() + 0x318);
if (!font) return;
CallMethod(0x409B90, font, (char*)"69420", 437, 20, 3, 3,
    0,                        // ignored
    0.5f, 0.5f, 0.5f, 0.8f,   // text RGBA
    0,                        // ignored
    0.0f, 0.0f, 0.0f, 1.0f);  // shadow RGBA
```

---

## 6. Recommended Approach for DLL Mods

### 6.1 Best: Use `UI_DrawTextShadow_Wrapper` (`0x409B90`)

This is the easiest function because it auto-fills the color struct vtable:

```cpp
typedef void (__thiscall *DrawTextShadowWrapper_t)(
    void* font, char* text, int x, int y,
    int sx, int sy, void* unused1,
    float tr, float tg, float tb, float ta,
    void* unused2, float sr, float sg, float sb, float sa);

// At address 0x409B90 in the original EXE
DrawTextShadowWrapper_t DrawTextShadowWrapper =
    (DrawTextShadowWrapper_t)0x00409B90;

void DrawHUDText(const char* text, int x, int y) {
    void* app  = *(void**)0x005341E0;
    void* font = *(void**)((char*)app + 0x318);  // showcardgothic28

    if (!font) return;  // font not loaded yet

    DrawTextShadowWrapper(
        font,                           // ECX = Font*
        (char*)text,                     // text
        x, y,                            // position
        3, 3,                            // shadow offset (3px right, 3px down)
        (void*)0,                        // unused (auto-filled)
        1.0f, 1.0f, 1.0f, 1.0f,         // text color: white, opaque
        (void*)0,                        // unused (auto-filled)
        0.0f, 0.0f, 0.0f, 1.0f          // shadow color: black, opaque
    );
}
```

### 6.2 Alternative: Use `Font_DrawCentered` (`0x0042C870`)

Simpler signature (8 params), no shadow, auto-centered:

```cpp
typedef void (__thiscall *FontDrawCentered_t)(
    void* font, char* text, int x, int y,
    void* unused, float r, float g, float b, float a);

FontDrawCentered_t FontDrawCentered =
    (FontDrawCentered_t)0x0042C870;

void DrawCenteredText(const char* text, int x, int y) {
    void* app  = *(void**)0x005341E0;
    void* font = *(void**)((char*)app + 0x318);

    if (!font) return;

    FontDrawCentered(
        font,                   // ECX = Font*
        (char*)text,             // text
        x, y,                    // center position
        (void*)0,                // unused (auto-filled with 0x4CF300)
        1.0f, 1.0f, 1.0f, 1.0f  // RGBA (only used if font scale != 1.0)
    );
}
```

### 6.3 Direct assembly call (for inline asm in DLL)

If calling from inline assembly in a bass.dll proxy:

```asm
; Example: Call UI_DrawTextShadow_Wrapper(font, "Hello", 400, 300, 3, 3, ...)
; ECX = Font*, 15 stack params

push_immediate 1.0          ; param_15: shadow_alpha (0x3f800000)
push_immediate 0.0          ; param_14: shadow_blue  (0x0)
push_immediate 0.0          ; param_13: shadow_green (0x0)
push_immediate 0.0          ; param_12: shadow_red   (0x0)
push_immediate 0x4cf300     ; param_11: shadow vtable (or 0 for wrapper)
push_immediate 1.0          ; param_10: text_alpha
push_immediate 1.0          ; param_9:  text_blue
push_immediate 1.0          ; param_8:  text_green
push_immediate 1.0          ; param_7:  text_red
push_immediate 0x4cf300     ; param_6:  text vtable (or 0 for wrapper)
push_immediate 3            ; param_5:  shadow_dy
push_immediate 3            ; param_4:  shadow_dx
push_immediate 300          ; param_3:  y
push_immediate 400          ; param_2:  x
push_offset hello_str       ; param_1:  text
mov  ecx, [font_ptr]       ; ECX = Font*
call dword ptr [0x00409B90] ; UI_DrawTextShadow_Wrapper
; No add esp needed — function cleans 0x3C bytes via RET 0x3C
```

---

## 7. Font Object Access Paths (for reference)

Different game contexts access the font through different object chains:

### 7.1 From Gadget-derived objects (Board, Menu, etc.)

All Gadget-derived objects store the App pointer at `+0x878`:

```
this+0x878 = App
Font = *(App + 0x318)   // or 0x31C, 0x324, 0x328, 0x320
```

Verified call sites:
- `ArenaBoard_Render` (`0x421910`): `MOV ECX,[ESI+0x878]; MOV ECX,[ECX+0x328]`
- `TourneyMenu_Render` (`0x00450AF0`): `UI_DrawTextShadow(*([this+0x878]+0x318), ...)`
- `HighScoreEntry_Render` (`0x0042BD40`): `*([this+0x878]+0x318)`
- `TourneyContinueDialog_Render` (`0x00445F50`): `*([this+0x878]+0x318)`
- `OkayDialog_ctor` (`0x00440E70`): `*([this+0x878]+0x318)`

### 7.2 From SimpleMenu-derived objects

SimpleMenu caches the font pointer at `+0x87C`:

```
this+0x87C = *(App + 0x318)  // set in SimpleMenu_ctor (0x00448F20)
```

Verified: `UIList_Render` (`0x449D40`) line: `MOV EBP,[EDI+0x87C]`

### 7.3 From ConfirmMenu_Ctor objects (RaceGoalReached)

```
this+0x0C = [parent+0x878] = App
Font = *(App + 0x318)
```

Verified: `ConfirmMenu_Render` (`0x44CD10`): `MOV EAX,[ESI+0x0C]; MOV ECX,[EAX+0x318]`

### 7.4 From CreditsScreen

```
this+0xCDC = App  (stored in ConfirmMenu_ctor)
Font = *(App + 0x318)
```

### 7.5 Direct from global App

```
void* g_App = *(void**)0x005341E0;
Font = *(void**)((char*)g_App + 0x318);
```

This is the most reliable approach for DLL mods — no need to trace through
object hierarchies.

---

## 8. Changing Font Scale for Colored Text

By default, font scale (`Font+0x428`) = 1.0f, which causes `Font_DrawGlyph`
to use the fast `Sprite_DrawRect` path that ignores color params. To get
colored text:

```cpp
// Temporarily change font scale to enable colored rendering
float* font_scale = (float*)((char*)font + 0x428);
float old_scale = *font_scale;
*font_scale = 1.2f;  // any value != 1.0 triggers the colored path

FontDrawCentered(font, text, x, y, 0, r, g, b, a);

*font_scale = old_scale;  // restore
```

The game itself does this in `HighScoreEntry_Render` (`0x0042BD40`):
```cpp
*(float*)(font + 0x428) = 0x3f400000;  // 0.75f — shrink for subtitle
// ... draw text ...
*(float*)(font + 0x428) = 0x3f800000;  // 1.0f — restore
```

---

## 9. String Formatting

Use `AthenaString_SprintfToBuffer` (`0x004BAE43`) for safe formatting:

```cpp
typedef void (*AthenaSprintf_t)(char* buffer, const char* fmt, ...);
AthenaSprintf_t AthenaSprintf = (AthenaSprintf_t)0x004BAE43;

char buf[256];
AthenaSprintf(buf, "Score: %d", score);
DrawHUDText(buf, 10, 10);
```

---

## 10. Practical Example: HUD Timer Overlay

```cpp
// In bass.dll proxy — hook during render frame
void OnRenderFrame() {
    void* app  = *(void**)0x005341E0;
    if (!app) return;

    void* font = *(void**)((char*)app + 0x318);  // showcardgothic28
    if (!font) return;

    // Get scene timer (arena mode: Scene+0x47AC)
    void* scene = *(void**)((char*)app + 0x184);
    if (!scene) return;

    int timer = *(int*)((char*)scene + 0x47AC);
    float seconds = timer / 60.0f;

    char buf[64];
    typedef void (*Sprintf_t)(char*, const char*, ...);
    Sprintf_t Sprintf = (Sprintf_t)0x004BAE43;
    Sprintf(buf, "TIME: %.1f", seconds);

    // Draw using wrapper (simplest API)
    typedef void (__thiscall *DrawText_t)(void*, char*, int, int, int, int,
        void*, float, float, float, float, void*, float, float, float, float);
    DrawText_t DrawText = (DrawText_t)0x00409B90;

    DrawText(font, buf, 400, 20, 3, 3,
        0, 1.0f, 1.0f, 1.0f, 1.0f,    // white text
        0, 0.0f, 0.0f, 0.0f, 1.0f);   // black shadow
}
```

---

## 11. Quick Reference

| Function | Address | Params (stack+this) | RET | Purpose |
|---|---|---|---|---|
| `UI_DrawTextShadow_Wrapper` | `0x00409B90` | 15+1 | `0x3C` | **Best for mods** — auto-fills vtable |
| `UI_DrawTextCentered` | `0x00409C60` | 15+1 | `0x3C` | Centered + shadow |
| `UI_DrawTextShadow` | `0x004012C0` | 15+1 | `0x3C` | Raw shadowed text |
| `Font_DrawCentered` | `0x0042C870` | 8+1 | `0x20` | Simplest — centered, no shadow |
| `Font_DrawGlyph` | `0x00457440` | 8+1 | `0x20` | Raw per-char rendering |
| `Font_MeasureText` | `0x00456E20` | 1+1 | `0x04` | Measure string width |
| `Font_DrawGlyph3D` | `0x00457690` | 18+1 | `0x48` | 3D world-space text (complex) |
| `LoadFont` | `0x00457130` | 2+1 | — | Load a font from disk |
| `AthenaString_SprintfToBuffer` | `0x004BAE43` | — | — | Safe printf |
| `UIList_SetTextByName` | `0x0044A8B0` | — | — | Change menu item text |
| `UIList_AddItem` | `0x004497F0` | — | — | Add menu item |

### Font offsets from App (`0x005341E0`)

| Offset | Font | Purpose |
|---|---|---|
| `+0x318` | showcardgothic28 | Main title / UI text |
| `+0x31C` | showcardgothic14 | Small label text |
| `+0x320` | showcardgothic16 | Info text |
| `+0x324` | arialnarrow12bold | UI detail text |
| `+0x328` | showcardgothic72 | Race timer digits |

### Font struct key offsets

| Offset | Type | Field |
|---|---|---|
| `+0x428` | `float` | Scale (1.0 = no color, ≠1.0 = colored) |
| `+0x424` | `int` | Line height |
| `+0x42C` | `char[]` | Per-glyph table start |

### Static identity transform

| Address | Value | Purpose |
|---|---|---|
| `0x004CF300` | vtable+floats | Default color/transform struct |

---

## 12. Common Pitfalls

1. **Wrong `this` pointer.** `UI_DrawTextCentered` and all text functions expect
   a **Font\*** as `this` (ECX), obtained via `*(App + 0x318)`. Passing the App,
   Scene, GraphicsDevice, or any other pointer will crash in `Font_DrawGlyph`.

2. **Too many/few params.** `UI_DrawTextCentered`/`UI_DrawTextShadow` take 15
   stack params (RET 0x3C). `Font_DrawCentered`/`Font_DrawGlyph` take 8 (RET 0x20).
   Mismatching the count corrupts the stack.

3. **Color struct vtable not set — DEBUNKED.** Params 6 and 11 of
   `UI_DrawTextShadow`/`UI_DrawTextCentered` are **overwritten internally** with
   `0x4CF300`. You can pass 0 for these params. They are NOT read from the
   caller's stack. (See §5.3 for disassembly proof.)

4. **Colors ignored at scale 1.0.** When `Font+0x428 == 1.0f`, the fast path
   (`Sprite_DrawRect`) is used and color params are ignored. Set scale ≠ 1.0
   temporarily for colored text (see §8).

5. **Color struct passed as pointer instead of 4 floats.** If your `CallMethod`
   wrapper receives a `Color` struct, it may push 1 pointer instead of 4 floats.
   This gives 9 stack params instead of 15 → stack corruption via `RET 0x3C`.
   Always expand `Color` into 4 individual `float` arguments. (See §5.5.)

6. **Font not loaded.** Font pointers at `App+0x318` etc. are NULL until the
   resource loader completes. Always null-check before drawing.

7. **Calling convention.** These are `__thiscall` — Font* goes in ECX, params
   go on stack right-to-left, callee cleans the stack (RET N). From C, use
   `__thiscall` typedefs or inline assembly.
