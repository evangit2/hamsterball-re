# Hamsterball UI Text Elements: Adding Custom On-Screen Text

**Scope:** Original `Hamsterball.exe` (PE32, i386, Athena engine, VS2003).  
**Method:** Direct Ghidra decompilation and PE disassembly.  
**Last Updated:** 2026-06-14

---

## 1. Overview

Hamsterball already has a small set of text-drawing helpers that render bitmap fonts in 2-D screen space.  This document explains how to add your own on-screen text elements — for example a HUD label, a timer overlay, a debug readout, or a custom message — using the engine's existing functions.

The two main approaches are:

1. **Menu-driven / UIList text** — change the text of an existing menu item with `UIList_SetTextByName`.
2. **Free 2-D screen text** — call `UI_DrawTextCentered`, `UI_DrawTextShadow`, or `Font_DrawCentered` from any render hook.

This document covers both.

---

## 2. Core Text-Drawing Functions

| Function | Address | Purpose |
|---|---|---|
| `UI_DrawTextShadow` | `0x004012C0` | Draws a string with a drop-shadow effect. |
| `UI_DrawTextCentered` | `0x00409C60` | Draws a string centered around `(x, y)`. |
| `UI_DrawTextCenteredAbsolute` | `0x004013A0` | Centered text with absolute coordinate semantics. |
| `UI_DrawTextShadow_Wrapper` | `0x00409B90` | Convenience wrapper around `UI_DrawTextShadow`. |
| `Font_DrawCentered` | `0x0042C870` | Lower-level: measures text, then calls `Font_DrawGlyph`. |
| `Font_MeasureText` | `0x00456E20` | Returns pixel width of a string. |
| `Font_DrawGlyph` | `0x00457440` | Single-character glyph renderer. |
| `UIList_SetTextByName` | `0x0044A8B0` | Updates text of an existing menu item by ID. |
| `AthenaString_SprintfToBuffer` | `0x004BAE43` | Safe `printf`-family string formatter. |
| `CRT_vsprintf` | `0x004BC768` | Standard CRT `vsprintf`. |

---

## 3. Free On-Screen Text

### 3.1 Simplest entry point: `UI_DrawTextCentered`

From the decompilation:

```cpp
void __thiscall
UI_DrawTextCentered(void *this, byte *text, int x, int y, int z_or_layer,
                   undefined4 param_6, undefined4 r, undefined4 g, undefined4 b,
                   undefined4 param_10, undefined4 param_11, undefined4 param_12,
                   undefined4 param_13, undefined4 param_14, undefined4 param_15);
```

* `this` — usually the active `GraphicsDevice` / `App` pointer.
* `text` — null-terminated ASCII string.
* `x`, `y` — screen pixel coordinates.
* `r`, `g`, `b` — color arguments (exact encoding is RGB triple or packed; verify in `Font_DrawGlyph`).
* The remaining arguments are transforms / font state / scale; in most cases you can pass identity/default values.

**Usage example:**

```cpp
char buf[256];
AthenaString_SprintfToBuffer(buf, "FPS: %.1f", current_fps);
UI_DrawTextCentered(app_ptr, buf, 400, 30, 0,
                  0, 0xFFFFFFFF, 0, 0, 0, 0, 0, 0, 0, 0);
```

> **Note:** The exact color argument packing is not fully documented here.  The engine's font functions accept either separate RGBA floats or a packed color depending on the call path.  Disassemble `Font_DrawGlyph` (`0x00457440`) for the precise layout when building a patch.

### 3.2 Shadowed text: `UI_DrawTextShadow`

```cpp
void __thiscall
UI_DrawTextShadow(void *this, byte *text, int x, int y, int shadow_dx, int shadow_dy,
                 undefined4 param_6, undefined4 shadow_color,
                 undefined4 r, undefined4 g, undefined4 b,
                 undefined4 param_10, undefined4 param_11,
                 undefined4 param_12, undefined4 param_13,
                 undefined4 param_14, undefined4 param_15);
```

This draws the same text twice: once offset by `(shadow_dx, shadow_dy)` with the shadow color, and once at `(x, y)` with the foreground color.  It is what the UI uses for readable labels over bright backgrounds.

### 3.3 Lower-level: `Font_DrawCentered`

```cpp
void __thiscall
Font_DrawCentered(void *this, byte *text, int x, int y,
                 undefined4 color_or_param4, undefined4 r, undefined4 g, undefined4 b,
                 undefined4 param_8);
```

This function measures the text with `Font_MeasureText`, computes the centered origin, and calls `Font_DrawGlyph`.  It is a good choice when you do **not** want a shadow and want direct color control.

---

## 4. Where to Call Text Functions

### 4.1 During a menu update

`GraphicsOptionsMenu_Update` (`0x00441E70`) is the per-frame update/render function for the options menu.  It is an ideal hook for text that must change every frame.

### 4.2 During gameplay

The scene's main render/update path is `Scene_UpdateBallsAndState` / `Scene_Render` (around `0x46BD80` region).  Hooking there lets you draw HUD text during a race.  The `App` object has an active `Scene` pointer at `App + 0x0C`, and the scene holds the ball list, timer, and score data.

### 4.3 A dedicated render callback

If you are writing a DLL mod or patch, the cleanest approach is:

1. Allocate a small struct in your injected code.
2. Hook the chosen render/update function via a 5-byte `jmp` or `__declspec(naked)` trampoline.
3. In your hook, format the string, call `UI_DrawTextCentered`, then jump back to the original function.

---

## 5. Formatting Strings: `AthenaString_SprintfToBuffer`

`AthenaString_SprintfToBuffer` (`0x004BAE43`) is the engine's safe wrapper around the CRT formatter.  It behaves like `sprintf` into a caller-provided buffer and supports all standard `printf` specifiers including `%f`.

Example formats useful for UI text:

```cpp
char buf[1024];

// Integer
AthenaString_SprintfToBuffer(buf, "Score: %d", score);

// Float with one decimal
AthenaString_SprintfToBuffer(buf, "Speed: %.1f", speed);

// String
AthenaString_SprintfToBuffer(buf, "Level: %s", level_name);

// Mixed
AthenaString_SprintfToBuffer(buf, "P1: %d  P2: %d  Time: %.1f",
                             p1_score, p2_score, timer);
```

The game uses this same pattern in `OptionsMenu_ctor` (lines 100–112 of the decompilation) to set dynamic menu labels such as `"Resolution: %d x %d"`.

---

## 6. Updating Existing Menu Text

If you want to show dynamic text inside an existing menu entry instead of drawing free-floating screen text, use `UIList_SetTextByName`:

```cpp
char buf[256];
AthenaString_SprintfToBuffer(buf, "Ping: %d ms", ping_ms);
UI_SetTextByName(menu_ptr, buf, "MYCUSTOMID");
```

Requirements:

* The menu entry with ID `"MYCUSTOMID"` must have been created with `UIList_AddItem(..., "MYCUSTOMID", ...)`.
* The ID string comparison is case-insensitive (`__stricmp`).
* The old string is freed and a new copy is allocated automatically.

This is the **safest** way to display changing text, because it reuses the existing menu layout, selection, and rendering code.

---

## 7. Measuring and Positioning Text

Use `Font_MeasureText` (`0x00456E20`) to compute pixel width before drawing if you need custom alignment:

```cpp
int width = Font_MeasureText(text);
int left_aligned_x = 0;
int right_aligned_x = screen_width - width;
int centered_x = screen_width / 2 - width / 2;
```

`Font_MeasureText` returns the total advance width in pixels for the current font.  The height of a line is controlled by the font's own metrics; there is no separate `Font_MeasureHeight` exposed in the simple helpers.

---

## 8. Font and Color Notes

* The game loads fonts through `LoadFont` (`0x00457130`) and stores them in a `FontList`.
* `Menu_AddFont` (`0x00475390`) is used during menu setup to register a font for UI rendering.
* Most UI text uses a single engine font.  If you add custom text during a menu, the current menu font is already active.
* Color handling varies by call path.  Some functions take packed `0xAARRGGBB`-style values, others take separate `r, g, b` floats in the range `0.0..1.0`.  Inspect the disassembly of `Font_DrawGlyph` (`0x00457440`) for the exact convention before hard-coding colors.

---

## 9. Practical Example: HUD Timer Overlay

Suppose you want to draw the current scene timer at the top center of the screen during gameplay.

```cpp
void __fastcall MyHudHook(void* scene_or_app)
{
    // Locate the timer value.  In arena/rumble mode, the timer is at Scene + 0x47AC.
    int timer_ticks = *(int*)((char*)scene_or_app + 0x47AC);
    float seconds = timer_ticks / 60.0f;

    char buf[64];
    AthenaString_SprintfToBuffer(buf, "TIME: %.1f", seconds);

    // Draw centered near the top (assume 800x600 screen)
    UI_DrawTextCentered(app_or_graphics_ptr, buf, 400, 20, 0,
                       0, 0xFFFFFFFF, 0, 0, 0, 0, 0, 0, 0, 0);
}
```

Then hook this function into `Scene_UpdateBallsAndState` or a comparable render path.

---

## 10. Common Pitfalls

1. **Wrong `this` pointer.**  `UI_DrawTextCentered` expects the graphics/font device, not a `Scene*` or `App*`.  Passing the wrong pointer will crash inside `Font_DrawGlyph`.
2. **Uninitialized font.**  Drawing text before the font list is loaded produces missing glyphs or crashes.  Hook after the menu/scene has finished initialization.
3. **Format-string buffer overflow.**  Always use `AthenaString_SprintfToBuffer` with a fixed-size stack buffer.  Do not call `CRT_vsprintf` directly unless you control the format string completely.
4. **Wrong color format.**  Verify whether a function wants packed RGBA or separate float channels.  The high-level wrappers and low-level glyph function may differ.
5. **Registry text not persisted.**  If you need your text content to survive restarts, store the underlying data in the registry via `App_SaveAllConfig` (`0x004284C0`), not the displayed string itself.

---

## 11. Quick Reference

| Task | Function | Address |
|---|---|---|
| Draw centered screen text | `UI_DrawTextCentered` | `0x00409C60` |
| Draw shadowed screen text | `UI_DrawTextShadow` | `0x004012C0` |
| Draw centered no-shadow | `Font_DrawCentered` | `0x0042C870` |
| Measure text width | `Font_MeasureText` | `0x00456E20` |
| Format a string | `AthenaString_SprintfToBuffer` | `0x004BAE43` |
| Change menu item text | `UIList_SetTextByName` | `0x0044A8B0` |
| Add a menu item | `UIList_AddItem` | `0x004497F0` |
| Save persistent values | `App_SaveAllConfig` | `0x004284C0` |

---

## 12. Summary

To add a custom UI text element:

1. **Pick a hook point** — menu update (`GraphicsOptionsMenu_Update`), scene render, or your own injected callback.
2. **Format the string** with `AthenaString_SprintfToBuffer`.
3. **Draw it** with `UI_DrawTextCentered` / `UI_DrawTextShadow` for free screen text, or `UIList_SetTextByName` for menu-integrated text.
4. **Use `Font_MeasureText`** if you need custom alignment.
5. **Persist underlying data** through `App_SaveAllConfig` if needed.

The engine already provides all necessary primitives; the main work is finding a stable render hook and passing the correct `this` pointer.
