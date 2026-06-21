# Hamsterball — Adding New Controls to the Remap Menu (DLL Modding Guide)

**Scope:** Original `Hamsterball.exe` (PE32, i386, Athena engine).  
**Method:** MinHook-based DLL injection into live binary.  
**Last Updated:** 2026-06-02

## Table of Contents
1. [What This Guide Covers](#what-this-guide-covers)
2. [Architecture Summary](#architecture-summary)
3. [Prerequisite: Existing Knowledge](#prerequisite-existing-knowledge)
4. [The Six Hook Points](#the-six-hook-points)
5. [Hook Point 1 — Render Extra Control Rows](#hook-point-1--render-extra-control-rows)
6. [Hook Point 2 — Menu Selection Bounds](#hook-point-2--menu-selection-bounds)
7. [Hook Point 3 — Key Capture (Remap)](#hook-point-3--key-capture-remap)
8. [Hook Point 4 — Per-Frame Input Polling](#hook-point-4--per-frame-input-polling)
9. [Hook Point 5 — Action Execution](#hook-point-5--action-execution)
10. [Hook Point 6 — Registry Persistence](#hook-point-6--registry-persistence)
11. [Complete Example: Adding a "Brake" Key](#complete-example-adding-a-brake-key)
12. [MinHook Skeleton](#minhook-skeleton)
13. [Address Quick Reference](#address-quick-reference)

---

## What This Guide Covers

The original game hardcodes **exactly four** directional controls (left/right/up/down) in the remap menu, the registry, the physics code, and the UI renderer.  Adding a fifth control (e.g. brake, jump, camera reset) requires **six coordinated hook points** working together.  This document gives you every address, offset, and C++ snippet you need.

---

## Architecture Summary

The game’s input pipeline is single-threaded:

```
InputDevice_PollAndRelease (0x46EBD0)
    → fills 256-byte DI8 state buffer at InputDevice+0x0C

Ball_GetInputForce (0x46EC30)
    → reads 4 DIK codes from InputDevice+0x50C..0x518
    → writes (force_x, force_y) into Ball+0x170 (velocity)

OptionsMenu_RenderControls (0x42E840)
    → draws 4 rows of control icons at hard-coded Y intervals
```

The 4-key limit is baked into:
- **UI loop:** iterates `i = 0..3`
- **Registry:** only keys `CONTROL1`..`CONTROL4`
- **Struct:** `InputDevice+0x50C` is the start of a fixed 4-slot array
- **Physics:** `Ball_GetInputForce` only checks those 4 offsets

Your DLL must intercept all four layers.

---

## Prerequisite: Existing Knowledge

Before reading this guide you should already understand:
- `InputDevice` layout (size 0x91C, DIK codes at +0x50C/+0x510/+0x514/+0x518)
- `Ball` layout (velocity at +0x170, acceleration at +0x2B8)
- `App+0xB28..0xB34` stores the 4 `CONTROL` DWORDs
- `OptionsMenu_RenderControls` draws the remap UI
- The game uses **DirectInput 8** exclusively (single import at `0x47C7F0`)

If any of the above is unfamiliar, read `docs/INPUT_SYSTEM.md` first.

---

## The Six Hook Points

| # | Hook Target | Address | What You Change |
|---|-------------|---------|-----------------|
| 1 | `OptionsMenu_RenderControls` | **0x42E840** | Draw rows 5+ below the original 4 |
| 2 | Menu selection bounds check | **find via xref** | Change max index from 3 → N-1 |
| 3 | Key capture / remap writer | **find via xref** | Redirect index ≥4 to your custom storage |
| 4 | `InputDevice_PollAndRelease` | **0x46EBD0** | After original poll, scan your custom DIK codes |
| 5 | `Ball_GetInputForce` | **0x46EC30** | Apply custom action (e.g. decelerate) |
| 6 | Registry save/load | **0x4284C0** | Read/write extra keys alongside CONTROL1-4 |

> **Note:** Hook points 2 and 3 do not have a single canonical address because they are inlined into the menu state-machine.  We describe how to locate them with a pattern search.

---

## Hook Point 1 — Render Extra Control Rows

### Target
`OptionsMenu_RenderControls` at **0x42E840**.

### What the original does
The function iterates a 4-item loop.  For each slot `i = 0..3` it:
1. Reads `App+0xB28 + i*4` to get the bound device type
2. Draws the action name ("LEFT", "RIGHT", "UP", "DOWN")
3. Draws the bound key icon
4. If two slots share the same device, colours one red

### Your hook strategy
**Do not patch the loop count.**  Instead, let the original 4 rows render completely, then in your `post`-hook draw rows 5+ using the same coordinate math.

The vertical spacing between rows is a constant pixel offset (read from the original assembly or infer from the decompilation).  In the original, each row increments Y by roughly **32 pixels**.

### C++ Hook Body
```cpp
typedef void (__thiscall *tOptionsMenu_RenderControls)(void* pMenu);
tOptionsMenu_RenderControls oRenderControls;

void __fastcall hkRenderControls(void* pMenu) {
    // 1. Let original draw the 4 built-in rows
    oRenderControls(pMenu);

    // 2. Append custom rows
    //    pMenu is the OptionsMenu/SimpleMenu object.
    //    The renderer uses a coordinate system where (0,0) is top-left.
    //    Original row 3 ends at Y ≈ base_y + 3*32.
    //    We start custom rows at Y ≈ base_y + 4*32.
    for (int i = 0; i < g_CustomControlCount; ++i) {
        int rowY = g_BaseControlY + (4 + i) * g_RowHeight;

        // Draw action name (e.g. "BRAKE")
        DrawMenuString(pMenu, g_CustomControls[i].name,
                       g_NameColumnX, rowY,
                       g_NormalColor);

        // Draw bound key name (e.g. "SPACE")
        const char* keyName = DIKToString(g_CustomControls[i].dik);
        DrawMenuString(pMenu, keyName,
                       g_KeyColumnX, rowY,
                       g_NormalColor);
    }
}
```

### Where to get the draw helper
The game already has a string-rendering helper used by `OptionsMenu_RenderControls`.  Look for the call to `Font_DrawString` or `AthenaString_Render` inside the original function and call the same address from your hook.  The exact helper address varies by build; find it by reading the first few xrefs inside `0x42E840`.

---

## Hook Point 2 — Menu Selection Bounds

### Target
The menu navigation code that clamps the selected row index.

### How to find it
1. In Ghidra, xref `OptionsMenu_RenderControls` (0x42E840) backwards.
2. Look for the function that calls it — this is the menu update/tick function (likely named `OptionsMenu_Update` or simply a vtable slot on `SimpleMenu`).
3. Inside that function, search for an immediate comparison against **`3`** or **`4`** near code that reads keyboard input (DIK_UP / DIK_DOWN).
4. The pattern looks like:
   ```asm
   cmp  eax, 3        ; or cmp eax, 4
   jle  already_valid
   mov  eax, 3        ; clamp to max
   ```

### Patch strategy
**Option A — Inline patch (simplest)**
Change the immediate `3` to `N-1` (e.g. `4` if you added 1 custom control).  This is a single-byte patch at the comparison instruction.

**Option B — Hook the update function**
Hook the entire menu update vtable slot, call original, then if the returned index is ≥4 verify it against your custom count instead of the hardcoded 4.

> **Warning:** The menu also highlights the selected row with a flashing colour.  Make sure the highlight-draw code uses the same index bounds; it is usually right after the clamp check.

---

## Hook Point 3 — Key Capture (Remap)

### Target
The code that captures a keypress during remap mode and stores the DIK code.

### How to find it
When the user clicks a control row, the game enters a "waiting for key" state.  During this state it polls the DI8 buffer (`InputDevice+0x0C`) for any key that transitions from `0` → `0x80`.  Once found, it stores that DIK code into `InputDevice+0x50C + index*4`.

Search in the same menu update function for:
- A write to **`[reg+0x50C]`** or **`[reg+0x510]`**
- The index register is usually `eax` or `ecx` holding `0..3`
- The value being written is read from `InputDevice+0x0C + DIK`

### Patch strategy
Hook the write instruction.  If `index < 4`, pass through to original.  If `index >= 4`, write into your own array instead.

### C++ Hook Body (inline detour at the writer)
```cpp
// Global storage for custom bindings (max 8 extra controls)
struct CustomBinding {
    BYTE dik;        // DIK code (e.g. DIK_SPACE = 0x39)
    bool isDown;     // polled state
};
CustomBinding g_CustomControls[8];
int g_CustomControlCount = 1;  // e.g. 1 = only Brake

// Hook target: the instruction that does
//   mov [InputDevice+0x50C+index*4], newDik
void __stdcall hkWriteBinding(int pInputDevice, int index, BYTE newDik) {
    if (index < 4) {
        // Original 4 controls — write to game memory
        *(BYTE*)(pInputDevice + 0x50C + index*4) = newDik;
    } else {
        // Custom control — write to our array
        int customIdx = index - 4;
        if (customIdx < g_CustomControlCount) {
            g_CustomControls[customIdx].dik = newDik;
        }
    }
}
```

> **Duplicate-key check:** The original renderer flags duplicate bindings in red.  If you want the same behaviour for custom keys, also hook the duplicate-check loop (inside `OptionsMenu_RenderControls`) and extend it to scan your custom array.

---

## Hook Point 4 — Per-Frame Input Polling

### Target
`InputDevice_PollAndRelease` at **0x46EBD0**.

### Why hook here
This is the **only** place the game touches DirectInput each frame.  By hooking it, your custom keys are sampled at the exact same moment as the built-in keys, with the same device-state semantics (acquire/release, cooperative level, etc.).

### What the original does
```c
void InputDevice_PollAndRelease(int self) {
    // Keyboard
    int didev = *(int*)(self + 0x434);
    if (didev) {
        IDirectInputDevice8_GetDeviceState(didev, 0x100, self + 0x0C);
    }
    // ... gamepad polling omitted
}
```

### Your hook strategy
Call the original, then read the same 256-byte buffer (`self+0x0C`) for your custom DIK codes.

### C++ Hook Body
```cpp
typedef void (__thiscall *tPollAndRelease)(void* pDevice);
tPollAndRelease oPollAndRelease;

void __fastcall hkPollAndRelease(void* pDevice) {
    // 1. Let game poll keyboard + gamepads normally
    oPollAndRelease(pDevice);

    // 2. Read the freshly-polled DI8 buffer for our custom keys
    BYTE* dikBuffer = (BYTE*)pDevice + 0x0C;

    for (int i = 0; i < g_CustomControlCount; ++i) {
        BYTE dik = g_CustomControls[i].dik;
        g_CustomControls[i].isDown = (dikBuffer[dik] & 0x80) != 0;
    }
}
```

> **Do NOT create a separate thread.**  The game is single-threaded; a background `GetAsyncKeyState` loop would race with the physics update.  Always sample input inside the game's own poll hook.

---

## Hook Point 5 — Action Execution

### Target
`Ball_GetInputForce` at **0x46EC30**.

### What the original does
Reads the 4 directional keys and builds a 2-D force vector:
```c
// Inside Ball_GetInputForce case 1 (keyboard):
if (key_state[left_dik]  & 0x80) force_x -= 1.0f;
if (key_state[right_dik] & 0x80) force_x += 1.0f;
if (key_state[up_dik]    & 0x80) force_y -= 0.5f;
if (key_state[down_dik]  & 0x80) force_y += 1.0f;
// Then writes scale*force to Ball+0x170 (velocity)
```

### Your hook strategy
Call the original function so the 4 built-in directions keep working, then apply your custom action by mutating the Ball struct directly.

### C++ Hook Body — Brake Example
```cpp
typedef void (__thiscall *tBallGetInputForce)(void* pBall, float* outVec);
tBallGetInputForce oBallGetInputForce;

void __fastcall hkBallGetInputForce(void* pBall, float* outVec) {
    // 1. Original physics force from 4 directional keys
    oBallGetInputForce(pBall, outVec);

    // 2. Apply brake if custom key is held
    if (g_CustomControls[0].isDown) {  // assuming index 0 = Brake
        float* vel = (float*)((char*)pBall + 0x170);

        // Simple friction model: scale velocity by 0.85 each frame
        vel[0] *= 0.85f;
        vel[1] *= 0.85f;
        vel[2] *= 0.85f;

        // Optional: also reduce acceleration so the ball stops trying to speed up
        float* accel = (float*)((char*)pBall + 0x2B8);
        accel[0] = 0.0f;
        accel[1] = 0.0f;
        accel[2] = 0.0f;
    }
}
```

### Other action ideas
| Action | What to mutate | Notes |
|--------|---------------|-------|
| **Camera snap** | Ball+0xC88 matrix or Scene camera offsets | Rotates view to behind-the-ball |
| **Power-up trigger** | Call existing power-up function via its address | Find the power-up activate function in `FUNCTION_MAP.md` |
| **Jump** | Ball+0x170 Y component | Hard — original physics assumes ground contact; you may need to also hook collision logic |
| **Reset ball** | Ball+0x164 position | Teleport to last checkpoint (read from `Ball+0x2DC`) |

> **Important:** Jump is the hardest action because the ball physics does not have an "airborne" state.  Simply adding Y-velocity will make the ball float through floors unless you also disable ground-collision snapping while the jump is active.  This requires a second hook in the collision resolver.

---

## Hook Point 6 — Registry Persistence

### Target
`App_SaveAllConfig` at **0x4284C0** (save) and the registry-read code inside `InputHandler_ctor` / `App_Initialize_Full` (load).

### What the original does
Save:
```c
RegKey_WriteDWORD(reg, "2PController1", *(int*)(app + 0xB28));
RegKey_WriteDWORD(reg, "2PController2", *(int*)(app + 0xB2C));
RegKey_WriteDWORD(reg, "2PController3", *(int*)(app + 0xB30));
RegKey_WriteDWORD(reg, "2PController4", *(int*)(app + 0xB34));
```

Load:  
The 4 DWORDs are read during `InputHandler` construction and stored into the same `App+0xB28..0xB34` slots.

### Your hook strategy
**Option A — Hook save/load directly**
Hook `App_SaveAllConfig`: after the original 4 `RegKey_WriteDWORD` calls, add your own:
```cpp
for (int i = 0; i < g_CustomControlCount; ++i) {
    char keyName[32];
    sprintf(keyName, "CustomControl%d", i+1);
    RegKey_WriteDWORD(reg, keyName, g_CustomControls[i].dik);
}
```
And mirror this on load by hooking the load function (or reading the same keys in your `DllMain` after the game has initialised).

**Option B — External INI (recommended)**
Instead of fighting the registry, save your custom bindings to an INI file next to the EXE:
```cpp
// In DllMain or at first attach
WritePrivateProfileStringA("CustomControls", "Brake",
                           DIKToString(g_CustomControls[0].dik),
                           ".\hamsterball_mod.ini");
```
This avoids registry permission issues and makes portable installs easier.

---

## Complete Example: Adding a "Brake" Key

### Step-by-step checklist

1. **Allocate a new UI row**
   - Hook `OptionsMenu_RenderControls` (0x42E840).
   - After the original 4 rows, draw `"BRAKE"` and the bound key name.

2. **Extend selection bounds**
   - Patch the `cmp eax, 3` in the menu update function to `cmp eax, 4`.

3. **Capture a key for Brake**
   - Hook the DIK writer inside the menu update.
   - If `index == 4`, store DIK into `g_CustomControls[0].dik`.

4. **Poll the Brake key every frame**
   - Hook `InputDevice_PollAndRelease` (0x46EBD0).
   - After original poll, check `dikBuffer[g_CustomControls[0].dik] & 0x80`.

5. **Apply braking force**
   - Hook `Ball_GetInputForce` (0x46EC30).
   - If `g_CustomControls[0].isDown`, scale `Ball+0x170` velocity by 0.85.

6. **Save the binding**
   - On DLL detach (or in response to an in-game save event), write `g_CustomControls[0].dik` to `hamsterball_mod.ini`.

---

## MinHook Skeleton

```cpp
#include <windows.h>
#include <MinHook.h>

// ---------- global state ----------
struct CustomControl {
    const char* name;
    BYTE        dik;
    bool        isDown;
};
CustomControl g_CustomControls[8];
int g_CustomControlCount = 0;

// ---------- typedefs ----------
typedef void (__thiscall *tRenderControls)(void*);
typedef void (__thiscall *tPollAndRelease)(void*);
typedef void (__thiscall *tBallGetInputForce)(void*, float*);

tRenderControls    oRenderControls;
tPollAndRelease    oPollAndRelease;
tBallGetInputForce oBallGetInputForce;

// ---------- hook bodies ----------
void __fastcall hkRenderControls(void* pMenu) { /* ... see section 5 ... */ }
void __fastcall hkPollAndRelease(void* pDev)    { /* ... see section 7 ... */ }
void __fastcall hkBallGetInputForce(void* pBall, float* out)
                                                { /* ... see section 8 ... */ }

// ---------- dll entry ----------
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        MH_Initialize();

        MH_CreateHook((LPVOID)0x42E840, hkRenderControls,
                      (LPVOID*)&oRenderControls);
        MH_CreateHook((LPVOID)0x46EBD0, hkPollAndRelease,
                      (LPVOID*)&oPollAndRelease);
        MH_CreateHook((LPVOID)0x46EC30, hkBallGetInputForce,
                      (LPVOID*)&oBallGetInputForce);

        // Enable all hooks
        MH_EnableHook(MH_ALL_HOOKS);
    }
    return TRUE;
}
```

> **Compile with:** MSVC or MinGW, linking against `minhook.lib` (or `libminhook.a`).  Inject with your preferred loader (e.g. `dinput8.dll` proxy, `xinput1_3.dll` proxy, or an external injector like Process Hacker).

---

## Address Quick Reference

| Symbol | Address | Notes |
|--------|---------|-------|
| `OptionsMenu_RenderControls` | **0x42E840** | Draws 4 control rows; hook post-render |
| `OptionsMenu_ctor` | **0x442CE0** | Builds menu; useful for finding vtable |
| `UIList_AddItem` | **0x4497F0** | Menu item constructor |
| `InputDevice_PollAndRelease` | **0x46EBD0** | Polls DI8; hook for custom key sampling |
| `InputDevice_ctor` | **0x466620** | Sets default DIK codes |
| `Ball_GetInputForce` | **0x46EC30** | Physics force builder; hook for custom actions |
| `Input_IsKeyDown` | **0x46E0B0** | Generic key-state check |
| `App_SaveAllConfig` | **0x4284C0** | Registry save; hook for custom persistence |
| `App_Initialize_Full` | **0x429530** | Game init; creates InputHandler+devices |
| `DirectInput8Create` | **0x47C7F0** | Only DI8 import |
| `InputDevice+0x0C` | — | 256-byte DI8 keyboard state buffer |
| `InputDevice+0x50C` | — | `key_left` DIK code |
| `InputDevice+0x510` | — | `key_right` DIK code |
| `InputDevice+0x514` | — | `key_up` DIK code |
| `InputDevice+0x518` | — | `key_down` DIK code |
| `Ball+0x170` | — | `velocity` (Vec3) |
| `Ball+0x2B8` | — | `acceleration` (Vec3) |
| `Ball+0x164` | — | `position` (Vec3) |
| `Ball+0x2DC` | — | `last_checkpoint` index |
| `App+0xB28` | — | `CONTROL1` DWORD |
| `App+0xB2C` | — | `CONTROL2` DWORD |
| `App+0xB30` | — | `CONTROL3` DWORD |
| `App+0xB34` | — | `CONTROL4` DWORD |

---

## Common Pitfalls

1. **Do not resize the InputDevice struct.**  It is 0x91C bytes with hardcoded offsets throughout the EXE.  Store custom keys in your own DLL globals.

2. **Do not use a background thread for input.**  The game is single-threaded; race with physics will corrupt `Ball+0x170`.

3. **The original remap UI does not exist for rows 5+.**  You must draw them yourself; copy the same font/colour the game uses for rows 1-4.

4. **Duplicate-key detection is hardcoded to 4 slots.**  If you want red-warning for conflicting custom keys, extend the duplicate-check logic in your `OptionsMenu_RenderControls` hook.

5. **Registry keys beyond CONTROL4 do not exist in vanilla.**  Use an external INI or add keys under a different sub-key (e.g. `HKCU\Software\HamsterballMod`) to avoid clobbering the game's save format.

6. **Joystick mode complicates things.**  If a player binds your custom action to a joystick button, you must also poll the gamepad state array (at `InputHandler+0x40` gamepad array).  The DI8 keyboard buffer will not contain joystick buttons.

---

## Version History

| Date | Change |
|------|--------|
| 2026-06-02 | Initial document — 6 hook points, brake example, MinHook skeleton |
