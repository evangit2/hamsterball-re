# Hamsterball Registry System

## Overview

Hamsterball stores all persistent settings (display, audio, unlocks, controls, best times) in the **Windows Registry** under `HKEY_CURRENT_USER\Software\Raptisoft\Hamsterball`.  
The game wraps raw Win32 ADVAPI32 calls in a thin `RegKey_*` helper layer.  This document covers the exact key path, value names, data types, App offsets, and how to read/write registry data the same way the engine does.

**Last verified against:** `Hamsterball.exe` (Athena engine, VS2003) via Ghidra decompilation.

---

## Registry Key Path

```
HKEY_CURRENT_USER\Software\Raptisoft\Hamsterball
```

The path is built at runtime using the format string at **`0x4D3978`**:
- `"Raptisoft\\%s"` where `%s` = `"Hamsterball"`

The game opens this key once at startup and caches the `HKEY` handle at **`App+0x54`**.

---

## Win32 Registry API Imports

From `ADVAPI32.dll` (standard Windows registry API):

| Import # | API Name | Purpose |
|----------|----------|---------|
| 480 | `RegOpenKeyA` | Open existing key |
| 481 | `RegOpenKeyExA` | Open with options |
| 459 | `RegCreateKeyA` | Create if missing |
| 491 | `RegQueryValueExA` | Read value |
| 504 | `RegSetValueExA` | Write value |
| 456 | `RegCloseKey` | Close handle |

---

## Engine Wrapper Functions

The game does **not** call Win32 APIs directly from every save site.  Instead it uses these wrapper helpers (addresses inferred from `App_SaveAllConfig` usage):

### `RegKey_Open` — Open/Creates the Key
- **Address:** Inlined / called from `App_SaveAllConfig`
- **Input:** `int* reg_handle_ptr` (points to `App+0x54`)
- **Behavior:** Calls `RegCreateKeyA` to create `Software\Raptisoft\Hamsterball` if missing.

### `RegKey_WriteDWORD`
- **Input:** `(void* handle, const char* name, DWORD value)`
- **Behavior:** Calls `RegSetValueExA(handle, name, 0, REG_DWORD, &value, 4)`

### `RegKey_WriteBool`
- **Input:** `(void* handle, const char* name, BYTE value)`
- **Behavior:** Same as `RegKey_WriteDWORD` but writes a 1-byte `REG_DWORD` (0 or 1).

### `Registry_SetValue`
- **Input:** `(void* handle, const char* name, BYTE* data, DWORD size)`
- **Behavior:** Calls `RegSetValueExA(handle, name, 0, REG_BINARY, data, size)`

### `RegKey_Close`
- **Input:** `(int handle)`
- **Behavior:** Calls `RegCloseKey(handle)` and invalidates the cached handle.

---

## App Object Registry Offsets

The `App` struct (base at `App` or `App_ptr`) stores all values that are mirrored to the registry.  Here are the confirmed offsets used by `App_SaveAllConfig`:

| Offset | Type | Registry Name | Description |
|--------|------|---------------|-------------|
| `+0x054` | `HKEY` | *(handle)* | Cached registry key handle |
| `+0x238` | `BYTE` | `RightButtonPause` | Right-click pauses the game |
| `+0x84C` | `DWORD` | `MouseSensitivity`` | Mouse sensitivity (0–10 typical) |
| `+0x850` | `BYTE` | `MirrorTournament` | Mirror-mode tournament flag |
| `+0x851` | `BYTE` | `DizzyRace` | Unlock: Dizzy race |
| `+0x852` | `BYTE` | `TowerRace` | Unlock: Tower race |
| `+0x853` | `BYTE` | `UpRace` | Unlock: Up race |
| `+0x854` | `BYTE` | `ExpertRace` | Unlock: Expert race |
| `+0x855` | `BYTE` | `OddRace` | Unlock: Odd race |
| `+0x856` | `BYTE` | `ToobRace` | Unlock: Toob race |
| `+0x857` | `BYTE` | `WobblyRace` | Unlock: Wobbly race |
| `+0x858` | `BYTE` | `SkyRace` | Unlock: Sky race |
| `+0x859` | `BYTE` | `MasterRace` | Unlock: Master race |
| `+0x85A` | `BYTE` | `DizzyArena` | Unlock: Dizzy arena |
| `+0x85B` | `BYTE` | `TowerArena` | Unlock: Tower arena |
| `+0x85C` | `BYTE` | `UpArena` | Unlock: Up arena |
| `+0x85D` | `BYTE` | `ExpertArena` | Unlock: Expert arena |
| `+0x85E` | `BYTE` | `OddArena` | Unlock: Odd arena |
| `+0x85F` | `BYTE` | `ToobArena` | Unlock: Toob arena |
| `+0x860` | `BYTE` | `WobblyArena` | Unlock: Wobbly arena |
| `+0x861` | `BYTE` | `SkyArena` | Unlock: Sky arena |
| `+0x862` | `BYTE` | `MasterArena` | Unlock: Master arena |
| `+0x863` | `BYTE` | `NeonRace` | Unlock: Neon race |
| `+0x864` | `BYTE` | `GlassRace` | Unlock: Glass race |
| `+0x865` | `BYTE` | `ImpossibleRace` | Unlock: Impossible race |
| `+0x866` | `BYTE` | `NeonArena` | Unlock: Neon arena |
| `+0x867` | `BYTE` | `GlassArena` | Unlock: Glass arena |
| `+0x868` | `BYTE` | `ImpossibleArena` | Unlock: Impossible arena |
| `+0x86C` | `BYTE[0x50]` | `BestTime` | 80-byte blob of best race times |
| `+0x8BC` | `BYTE[0x50]` | `Medals` | 80-byte blob of medal bitmasks |
| `+0xB28` | `DWORD` | `2PController1` | 2P mapping slot 1 |
| `+0xB2C` | `DWORD` | `2PController2` | 2P mapping slot 2 |
| `+0xB30` | `DWORD` | `2PController3` | 2P mapping slot 3 |
| `+0xB34` | `DWORD` | `2PController4` | 2P mapping slot 4 |

---

## Save Function Reference

### `App_SaveAllConfig` — **`0x4284C0`**
The master save routine.  Called on game exit and whenever settings change.

```cpp
void __fastcall App_SaveAllConfig(void* app)
{
    App_WriteDisplaySettings(app);          // 0x?????? — saves resolution/quality

    RegKey_Open(*(int*)(app + 0x54));      // Ensure key is open

    // ---- Scalar settings ----
    RegKey_WriteDWORD(app + 0x54, "MouseSensitivity",      *(DWORD*)(app + 0x84C));
    RegKey_WriteBool (app + 0x54, "MirrorTournament",      *(BYTE* )(app + 0x850));
    RegKey_WriteBool (app + 0x54, "RightButtonPause",      *(BYTE* )(app + 0x238));

    // ---- Race unlocks (12) ----
    RegKey_WriteBool(app + 0x54, "DizzyRace",      *(BYTE*)(app + 0x851));
    RegKey_WriteBool(app + 0x54, "TowerRace",      *(BYTE*)(app + 0x852));
    RegKey_WriteBool(app + 0x54, "UpRace",         *(BYTE*)(app + 0x853));
    RegKey_WriteBool(app + 0x54, "ExpertRace",     *(BYTE*)(app + 0x854));
    RegKey_WriteBool(app + 0x54, "OddRace",        *(BYTE*)(app + 0x855));
    RegKey_WriteBool(app + 0x54, "ToobRace",       *(BYTE*)(app + 0x856));
    RegKey_WriteBool(app + 0x54, "WobblyRace",     *(BYTE*)(app + 0x857));
    RegKey_WriteBool(app + 0x54, "SkyRace",        *(BYTE*)(app + 0x858));
    RegKey_WriteBool(app + 0x54, "MasterRace",     *(BYTE*)(app + 0x859));
    RegKey_WriteBool(app + 0x54, "NeonRace",       *(BYTE*)(app + 0x863));
    RegKey_WriteBool(app + 0x54, "GlassRace",      *(BYTE*)(app + 0x864));
    RegKey_WriteBool(app + 0x54, "ImpossibleRace", *(BYTE*)(app + 0x865));

    // ---- Arena unlocks (12) ----
    RegKey_WriteBool(app + 0x54, "DizzyArena",      *(BYTE*)(app + 0x85A));
    RegKey_WriteBool(app + 0x54, "TowerArena",      *(BYTE*)(app + 0x85B));
    RegKey_WriteBool(app + 0x54, "UpArena",         *(BYTE*)(app + 0x85C));
    RegKey_WriteBool(app + 0x54, "ExpertArena",     *(BYTE*)(app + 0x85D));
    RegKey_WriteBool(app + 0x54, "OddArena",        *(BYTE*)(app + 0x85E));
    RegKey_WriteBool(app + 0x54, "ToobArena",       *(BYTE*)(app + 0x85F));
    RegKey_WriteBool(app + 0x54, "WobblyArena",     *(BYTE*)(app + 0x860));
    RegKey_WriteBool(app + 0x54, "SkyArena",        *(BYTE*)(app + 0x861));
    RegKey_WriteBool(app + 0x54, "MasterArena",     *(BYTE*)(app + 0x862));
    RegKey_WriteBool(app + 0x54, "NeonArena",       *(BYTE*)(app + 0x866));
    RegKey_WriteBool(app + 0x54, "GlassArena",      *(BYTE*)(app + 0x867));
    RegKey_WriteBool(app + 0x54, "ImpossibleArena", *(BYTE*)(app + 0x868));

    // ---- Binary blobs (best times & medals) ----
    Registry_SetValue(app + 0x54, "BestTime", (BYTE*)(app + 0x86C), 0x50);
    Registry_SetValue(app + 0x54, "Medals",   (BYTE*)(app + 0x8BC), 0x50);

    // ---- 2P controller mappings ----
    RegKey_WriteDword(app + 0x54, "2PController1", *(DWORD*)(app + 0xB28));
    RegKey_WriteDword(app + 0x54, "2PController2", *(DWORD*)(app + 0xB2C));
    RegKey_WriteDword(app + 0x54, "2PController3", *(DWORD*)(app + 0xB30));
    RegKey_WriteDword(app + 0x54, "2PController4", *(DWORD*)(app + 0xB34));

    RegKey_Close(*(int*)(app + 0x54));
}
```

---

## Load Function Reference

### `LoadOrSaveConfig` — **`0x4279F0`**
Called during startup to read (or create) the configuration.  It mirrors the save structure but uses `RegQueryValueExA` instead of `RegSetValueExA`.

*(Ghidra decompilation is large; the relevant registry-read logic is structurally identical to `App_SaveAllConfig` but reads into the same App offsets instead of writing.)*

---

## Display Settings

Resolution, color depth, texture quality, and full-screen mode are saved by **`App_WriteDisplaySettings`** (called from `App_SaveAllConfig`).  The exact value names for display settings live in the string table near **`0x4D5EB8`** (`"Resolution: %d x %d"`) and are written as a separate sub-key or value blob.

---

## How to Add Custom Registry Values (Modding Guide)

If you are injecting code or writing a trainer/mod, follow the same pattern the engine uses:

### 1. Open the key
```cpp
// app = pointer to App object (passed to most __fastcall functions)
int hKey = *(int*)((BYTE*)app + 0x54);
RegKey_Open(hKey);   // wrapper at inferred address; or call RegCreateKeyA yourself
```

### 2. Write a scalar
```cpp
// DWORD example
DWORD myValue = 42;
RegSetValueExA(hKey, "MyModValue", 0, REG_DWORD, (BYTE*)&myValue, 4);

// BOOL example (same as DWORD but 0/1)
BYTE myFlag = 1;
RegSetValueExA(hKey, "MyModFlag", 0, REG_DWORD, &myFlag, 4);
```

### 3. Write binary data
```cpp
BYTE myBlob[64] = { ... };
RegSetValueExA(hKey, "MyModData", 0, REG_BINARY, myBlob, sizeof(myBlob));
```

### 4. Close
```cpp
RegKey_Close(hKey);   // or RegCloseKey(hKey)
```

### Important Notes
- The game uses **`REG_DWORD`** for both true DWORDs and booleans (not `REG_SZ`).
- Binary blobs (`BestTime`, `Medals`) use **`REG_BINARY`** with exact 80-byte (`0x50`) sizes.
- The registry handle at `App+0x54` is cached; **do not** leak it—always close after use.
- Display settings are saved separately; if you are adding display-related values, hook `App_WriteDisplaySettings` instead.

---

## String Table References (Binary Offsets)

| String | Binary Offset | Usage |
|--------|---------------|-------|
| `"Raptisoft\\%s"` | `0x4D3978` | Registry key format |
| `"MouseSensitivity"` | `0x4D2898` | Save/load |
| `"MirrorTournament"` | `0x4D2884` | Save/load |
| `"BestTime"` | `0x4D274C` | Save/load |
| `"Medals"` | `0x4D2744` | Save/load |
| `"2PController1"` | `0x4D2734` | Save/load |
| `"Resolution: %d x %d"` | `0x4D5EB8` | Display UI |
| `"Texture Quality"` | `0x4D8780` | Display UI |

---

## Quick Reference: Reading from Outside the Game

If you want to read Hamsterball settings from an external tool (Python / C# / etc.):

```python
import winreg

key = winreg.OpenKey(winreg.HKEY_CURRENT_USER, r"Software\Raptisoft\Hamsterball")

# Read a DWORD (e.g., MouseSensitivity)
val, typ = winreg.QueryValueEx(key, "MouseSensitivity")
print(val)  # int

# Read a binary blob (e.g., BestTime)
val, typ = winreg.QueryValueEx(key, "BestTime")
print(len(val))  # 80 bytes

winreg.CloseKey(key)
```

---

## See Also

- `App_SaveAllConfig` — master save routine @ `0x4284C0`
- `LoadOrSaveConfig` — startup load/create @ `0x4279F0`
- `OptionsMenu_RenderControls` — control binding UI @ `0x42E840`
- `SoundDevice_ReadVolume` — audio registry read @ `0x466570`
- `SoundDevice_dtor` — audio registry save @ `0x4668A0`
