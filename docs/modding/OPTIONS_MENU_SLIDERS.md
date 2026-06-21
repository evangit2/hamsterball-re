# Hamsterball Options Menu: Adding New Sliders and Decimal-Display Values

**Scope:** Original `Hamsterball.exe` (PE32, i386, Athena engine, VS2003).  
**Method:** Direct Ghidra decompilation and PE disassembly.  
**Last Updated:** 2026-06-14

---

## 1. Overview

The options menu is implemented as a `UIList`-based menu (`SimpleMenu` subclass).  
Each entry is a normal `UIListItem` with:

* A **display label** shown to the player (e.g. `"SOUND VOLUME:"`).
* A short **ID code** used internally to identify which entry is selected/acted on (e.g. `"SV"`, `"MV"`, `"TQ"`, `"MS"`).

The four existing slider-like items are:

| Label | ID | Staging value offset | Dirty flag | Applies to |
|---|---|---|---|---|
| `SOUND VOLUME:` | `SV` | `OptionsMenu + 0xD10` | `+0xD1C` | `SoundDevice + 0x838` |
| `MUSIC VOLUME:` | `MV` | `OptionsMenu + 0xD48` | `+0xD54` | `MusicDevice + 0x8` |
| `TEXTURE QUALITY:` | `TQ` | `OptionsMenu + 0xD80` | `+0xD8C` | `App + 0x174` graphics object, offset `+0x184` |
| `MOUSE SENSITIVITY:` | `MS` | `OptionsMenu + 0xDF0` | `+0xDFC` | `App + 0x84C` |

**Goal of this document:** show how to add a new slider entry, make it respond to left/right input, clamp and store its value, apply it to a subsystem, persist it to the registry, and display a **decimal numeric readout** such as `FOV: 0.7` instead of a plain bar.

---

## 2. Key Functions and Addresses

| Function | Address | Purpose |
|---|---|---|
| `OptionsMenu_ctor` | `0x00442CE0` | Builds the options list with `UIList_AddItem` calls. |
| `OptionsMenu_AdjustVolume` | `0x00442680` | Receives `(id_code, delta)` and updates the matching staging float. |
| `GraphicsOptionsMenu_Update` | `0x00441E70` | Applies staged values to real subsystems each frame. |
| `UIList_AddItem` | `0x004497F0` | Adds one menu entry.  Signature: `void __thiscall UIList_AddItem(void* this, char* display_text, char* id_code, ... int is_clickable)`. |
| `UIList_SetTextByName` | `0x0044A8B0` | Updates the display text of an existing item by its ID code. |
| `AthenaString_SprintfToBuffer` | `0x004BAE43` | `printf`-family formatter; supports `%d`, `%s`, and `%f` / `%.1f`. |
| `App_SaveAllConfig` | `0x004284C0` | Saves persistent settings to the registry on exit. |
| `App_Ctor` / registry init | `0x46DC40` region | Reads saved values back into `App` offsets at startup. |

---

## 3. How an Existing Slider Works

### 3.1 Value change

`OptionsMenu_AdjustVolume` (`0x00442680`) does the following for each known ID:

```cpp
if (__stricmp(param_1, "SV") == 0) {
    float new_val = (float)param_2 * 0.1f + *(float*)(this + 0xD10);
    if (new_val > 1.0f) new_val = 1.0f;
    if (new_val < 0.0f) new_val = 0.0f;
    *(float*)(this + 0xD10) = new_val;
    *(BYTE*)(this + 0xD1C) = 1;   // dirty flag
}
```

Observed from disassembly:

* The step constant `0.1` is the **double** at `_DAT_004CF308` (`0x004CF308`).
* The upper clamp is `_DAT_004CF310` = `1.0f` at `0x004CF310`.
* The lower clamp is `_DAT_004CF368` = `0.0f` at `0x004CF368`.
* `param_2` is the signed delta: `-1` for left, `+1` for right (actual input routing passes a scaled value; see section 8).

### 3.2 Value application

`GraphicsOptionsMenu_Update` (`0x00441E70`) reads each dirty flag and writes the staged float to the real subsystem.  For example:

```cpp
if (*(BYTE*)(this + 0xD1C)) {
    *(float*)(sound_device + 0x838) = *(float*)(this + 0xD10);
    *(BYTE*)(this + 0xD1C) = 0;
}
```

### 3.3 Value persistence

`App_SaveAllConfig` (`0x004284C0`) writes the active subsystem value to the registry.  Mouse sensitivity, for instance, is saved as a DWORD named `"MouseSensitivity"` from `App + 0x84C`.

---

## 4. Adding a Brand-New Slider

This example adds a hypothetical `FOV` slider (field-of-view scaling).  Pick whatever subsystem you actually want to control.

### 4.1 Reserve storage in the OptionsMenu struct

Choose unused space after the existing sliders.  Example offsets:

```cpp
struct OptionsMenu_extra {
    // ... existing fields up to +0xDFC ...
    float fov_staging;      // +0xE10 (example — verify no overlap)
    BYTE  fov_dirty;        // +0xE14
};
```

> **Always verify** the chosen offsets are not already used by `OptionsMenu_RenderControls` (`0x0042E840`) or by `GraphicsOptionsMenu_Update` (`0x00441E70`).  Use Ghidra to check cross-references before patching.

### 4.2 Add the menu item

In `OptionsMenu_ctor` (`0x00442CE0`), add another `UIList_AddItem` call near the other sliders:

```cpp
UIList_AddItem(this, "FOV:", "FOV",
               /* same trailing args as a neighboring slider */,
               /* param_8 = 1 if left/right should work on it */);
```

The ID code `"FOV"` is arbitrary but must be unique in this menu and must be uppercase if you want consistent `__stricmp` behavior.

### 4.3 Handle left/right changes

Patch `OptionsMenu_AdjustVolume` (`0x00442680`) with a new branch:

```cpp
if (__stricmp(param_1, "FOV") == 0) {
    float new_val = (float)param_2 * 0.1f + *(float*)(this + 0xE10);
    if (new_val > 1.0f) new_val = 1.0f;
    if (new_val < 0.0f) new_val = 0.0f;
    *(float*)(this + 0xE10) = new_val;
    *(BYTE*)(this + 0xE14) = 1;
}
```

### 4.4 Apply the value each frame

In `GraphicsOptionsMenu_Update` (`0x00441E70`), add:

```cpp
if (*(BYTE*)(this + 0xE14)) {
    float fov = *(float*)(this + 0xE10);
    // example: store in App + 0x???? or pass to graphics/camera code
    *(float*)(app_ptr + 0xYYY) = fov;
    *(BYTE*)(this + 0xE14) = 0;
}
```

### 4.5 Read and write the registry

In `App_SaveAllConfig` (`0x004284C0`) add a save line:

```cpp
RegKey_WriteDWORD(app + 0x54, "FieldOfView",
                  (DWORD)(*(float*)(app + 0xYYY) * 100.0f));
```

And add a matching read in the startup path (near where `MouseSensitivity` is read) to restore the value.

---

## 5. Displaying a Decimal Value

The existing sliders render only a proportional bar.  They do **not** display numbers.  To show a decimal number you must update the menu item's text each time the value changes.

### 5.1 Required functions

* `AthenaString_SprintfToBuffer(local_1024_buf, "%s: %.1f", label, value)` — formats the string.
* `UIList_SetTextByName(this, local_1024_buf, "FOV")` — pushes the formatted text to the menu entry whose ID code is `"FOV"`.

`UIList_SetTextByName` is at `0x0044A8B0`.  It frees the old text, allocates a copy, and updates the item.  It is safe to call every time the value changes.

### 5.2 Patch OptionsMenu_AdjustVolume to update the label

After the new `"FOV"` branch, immediately refresh the text:

```cpp
if (__stricmp(param_1, "FOV") == 0) {
    float new_val = (float)param_2 * 0.1f + *(float*)(this + 0xE10);
    if (new_val > 1.0f) new_val = 1.0f;
    if (new_val < 0.0f) new_val = 0.0f;
    *(float*)(this + 0xE10) = new_val;
    *(BYTE*)(this + 0xE14) = 1;

    char buf[1024];
    AthenaString_SprintfToBuffer(buf, "FOV: %.1f", new_val);
    UIList_SetTextByName(this, buf, "FOV");
}
```

This is the **minimal, safe** way to make the slider show a decimal readout.

### 5.3 Initialize the label in OptionsMenu_ctor

When the menu is first built, the text is still `"FOV:"`.  Set it to the current value as well:

```cpp
char buf[1024];
float current_fov = *(float*)(this + 0xE10);
AthenaString_SprintfToBuffer(buf, "FOV: %.1f", current_fov);
UIList_SetTextByName(this, buf, "FOV");
```

Place this right after the existing `UIList_SetTextByName` block (lines 100–112 of `OptionsMenu_ctor` in the decompilation).

### 5.4 Decimal precision

Use the normal `printf` precision specifiers:

| Format string | Output example | Use case |
|---|---|---|
| `"%.0f"` | `FOV: 7` | Whole numbers |
| `"%.1f"` | `FOV: 0.7` | One decimal place |
| `"%.2f"` | `FOV: 0.75` | Two decimal places |

The engine uses the standard CRT `vsprintf` (`CRT_vsprintf` at `0x004BC768`), so all normal float formatting works.

---

## 6. UI Layout Considerations

`UIList_AddItem` allocates a `UIListItem` of size `0x444` bytes.  The item stores:

* display text pointer (`+0x00`)
* id code pointer (`+0x04`)
* measured width/height
* selection state

The list itself is anchored at:

* `OptionsMenu + 0x44C` — item list
* `OptionsMenu + 0x88C` — secondary list / iterator backing
* `OptionsMenu + 0x864` — current selection index

When you add a new item, `UIList_Layout` (`0x00449D40`) and `UIList_Render` (`0x00449C20`) will automatically include it.  No manual layout math is required.

---

## 7. Persisting the New Setting

The registry path is:

```
HKEY_CURRENT_USER\Software\Raptisoft\Hamsterball
```

For a new float slider, the easiest compatible approach is to scale it to an integer and store as `REG_DWORD`:

```cpp
// Save: value is 0.0..1.0, store as 0..100
DWORD scaled = (DWORD)(value * 100.0f);
RegKey_WriteDWORD(app + 0x54, "FieldOfView", scaled);

// Load: reverse the scale
DWORD raw = RegKey_ReadDWORD(app + 0x54, "FieldOfView", default_value);
float value = raw / 100.0f;
```

If you want to store a raw float, use `Registry_SetValue` (`REG_BINARY`) with 4 bytes, as the game does for the `BestTime` and `Medals` blobs.  However, DWORD is simpler for human editing in `regedit`.

---

## 8. Left / Right Input Routing (what calls AdjustVolume)

The exact internal vtable dispatch is not fully decompiled here, but the practical path is:

1. Player presses Left or Right while a slider item is selected.
2. `UIList_HandleKeyNav` / `UIList_ActivateCurrentItem` (`0x00449750`) decides whether the current item is a left/right slider.
3. For slider items, it eventually calls into the menu's override of the input-handler vtable slot, which forwards to:
   * `OptionsMenu_AdjustVolume(this, id_code, delta)` at `0x00442680`.
4. `OptionsMenu_AdjustVolume` updates the staging value and dirty flag.

For modding, the only function you need to edit is `OptionsMenu_AdjustVolume`.  As long as your new ID code is unique and your item is added with the slider flag set, the existing routing will call your new branch automatically.

---

## 9. Build and Test

After editing the source patch or binary:

```bash
make clean && make
```

Or if you are patching the binary directly with Ghidra:

1. Apply patches in Ghidra.
2. Export the new PE.
3. Replace `Hamsterball.exe` and test on Wine or Windows.
4. Open **Options** menu, move selection to the new slider, press Left/Right, and confirm the decimal text updates.

---

## 10. Quick Reference: Useful Global Constants

| Address | Interpretation | Value |
|---|---|---|
| `_DAT_004CF308` (`0x004CF308`) | Slider step as double | `0.1` |
| `_DAT_004CF310` (`0x004CF310`) | Slider maximum as float | `1.0` |
| `_DAT_004CF368` (`0x004CF368`) | Slider minimum as float | `0.0` |
| `0x004D5EA4` | String `"TQ"` | ID code for texture quality |
| `0x004D5EA8` | String `"MS"` | ID code for mouse sensitivity |
| `0x004D5EAC` | String `"MV"` | ID code for music volume |
| `0x004D5EB0` | String `"SV"` | ID code for sound volume |

---

## 11. Example: Complete New Slider Patch Summary

This is the high-level change list for adding a `FOV` decimal slider:

1. `OptionsMenu_ctor` (`0x00442CE0`):
   * Add `UIList_AddItem(this, "FOV:", "FOV", ...);`
   * Add `AthenaString_SprintfToBuffer(buf, "FOV: %.1f", current); UIList_SetTextByName(this, buf, "FOV");`

2. `OptionsMenu_AdjustVolume` (`0x00442680`):
   * Add `__stricmp(id, "FOV")` branch with clamp and dirty flag.
   * Call `AthenaString_SprintfToBuffer` + `UIList_SetTextByName` to refresh the text.

3. `GraphicsOptionsMenu_Update` (`0x00441E70`):
   * Add dirty-flag branch that copies `OptionsMenu + 0xE10` to `App + 0xYYY`.

4. `App_SaveAllConfig` (`0x004284C0`):
   * Add `RegKey_WriteDWORD(..., "FieldOfView", scaled)`.

5. Startup read path (near `MouseSensitivity` read):
   * Add `RegKey_ReadDWORD(..., "FieldOfView")` and write to `App + 0xYYY`.

That is the full recipe.
