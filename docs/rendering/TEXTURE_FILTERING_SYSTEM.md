# D3D8 Texture Filtering System

## Overview

Hamsterball uses Direct3D 8's `SetTextureStageState` API to control texture
filtering. The game defaults to `D3DTEXF_LINEAR` (smooth/blurry) for all
filter types. This document covers the D3D8 texture filtering API, how the
game uses it, and how the `sharp_textures` mod overrides it.

## D3D8 Texture Filtering API

In D3D8, texture filtering is controlled per-stage via
`IDirect3DDevice8::SetTextureStageState`:

- **Vtable index:** 63 (byte offset 0xFC in the device vtable)
- **Calling convention:** `__stdcall` — the `this` pointer is pushed on the
  stack as the first parameter (NOT `__thiscall` where `this` goes in ECX)
- **Signature:**
  ```c
  HRESULT __stdcall SetTextureStageState(
      IDirect3DDevice8* this,   // pushed on stack
      DWORD Stage,               // 0-7 (D3D8 supports 8 texture stages)
      DWORD Type,                // D3DTEXTURESTAGESTATETYPE
      DWORD Value                // filter mode
  );
  ```

### Texture Stage State Types

| Constant | Value | Description |
|----------|-------|-------------|
| `D3DTSS_MAGFILTER` | 16 | Magnification filter (texture appears larger than original) |
| `D3DTSS_MINFILTER` | 17 | Minification filter (texture appears smaller than original) |
| `D3DTSS_MIPFILTER` | 18 | Mipmap filter (between mip levels) |

### Filter Mode Values

| Constant | Value | Effect |
|----------|-------|--------|
| `D3DTEXF_NONE` | 0 | No filtering (mipmaps disabled for MIPFILTER) |
| `D3DTEXF_POINT` | 1 | Nearest-neighbor — sharp/pixelated, no smoothing |
| `D3DTEXF_LINEAR` | 2 | Bilinear/trilinear — smooth/blurry (game default) |
| `D3DTEXF_ANISOTROPIC` | 3 | Anisotropic — high quality, directional filtering |

> **Note:** D3D8 does NOT have `SetSamplerState` — that is a D3D9+ API.
> In D3D8, sampler state is set via `SetTextureStageState`.

## Game's Default Behavior

The game sets up render states in two functions:

1. **Graphics_Defaults** (0x00455A60) — Calls the D3D8 device's
   `SetRenderState` (vtable offset 0xC8) to configure depth buffering,
   alpha blending, and fog.
2. **Graphics_InitRenderStates** (0x0045A439) — Called via
   `Graphics_SetRenderState`, configures shader profiles via
   `D3DX_DetectShaderProfile`.

The game does NOT explicitly call `SetTextureStageState` for filtering
in any located function. D3D8's default filter mode is `D3DTEXF_LINEAR`
(2) for MAGFILTER and MINFILTER, and `D3DTEXF_NONE` (0) for MIPFILTER
when mipmaps are not generated. The game relies on these defaults,
producing smooth/blurry textures.

## Graphics_BeginFrame (0x00453B50)

Called every frame. This is the optimal hook point for per-frame
texture state overrides.

### Disassembly (function prologue)
```asm
00453b50: 53              PUSH EBX
00453b51: 8B D9           MOV EBX,ECX
00453b53: 8B 4C 24 08     MOV ECX,[ESP+8]
00453b57: 59              PUSH ESI
00453b5a: 57              PUSH EDI
00453b5b: 75 06           JNZ 0x00453b63
00453b5d: 8D 4B 4C 07    LEA ECX,[EBX+0x74C]
00453b63: 8B 43 54 01    MOV EAX,[EBX+0x154]  ; D3D8 device
00453b69: 8D 71 04       LEA ESI,[ECX+4]
00453b6c: 8B 08          MOV ECX,[EAX]         ; vtable
00453b6e: 56              PUSH ESI             ; param 3
00453b6f: 6A 00 01       PUSH 0x100           ; param 2
00453b74: 50              PUSH EAX             ; this (ON STACK)
00453b75: FF 51 94       CALL [ECX+0x94]       ; vtable[37] = SetTransform
00453b7b: ...
```

Key observations:
- `this` (Graphics struct) is in ECX on entry (`__thiscall` for game functions)
- D3D8 device is at `Graphics+0x154`
- D3D8 device methods are called with `this` pushed on the stack (`__stdcall`)
- The prologue is 7 bytes: `53 8B D9 8B 4C 24 08`

## D3D8 Device Vtable

The IDirect3DDevice8 vtable is defined in `d3d8.h`. Key entries:

| Index | Offset | Method |
|-------|--------|--------|
| 0-2 | 0x00-0x08 | QueryInterface, AddRef, Release |
| 3 | 0x0C | TestCooperativeLevel |
| 37 | 0x94 | SetTransform |
| 50 | 0xC8 | SetRenderState |
| 51 | 0xCC | GetRenderState |
| 61 | 0xF0 | GetTexture |
| 62 | 0xF4 | SetTexture |
| 63 | 0xFC | **SetTextureStageState** |
| 64 | 0x100 | GetTextureStageState |

## Critical Pitfall: Calling Convention

### The Bug

The initial `sharp_textures` mod used `__thiscall` for the
`SetTextureStageState` function pointer:

```c
// WRONG — crashes in d3d8.dll at 1 second
typedef int (__thiscall *SetTSS_t)(void*, DWORD, DWORD, DWORD);
```

With `__thiscall`, the `this` pointer goes in ECX. But D3D8 COM methods
expect `this` on the **stack** (`__stdcall`). The result:

- The D3D8 method receives garbage for the `this` pointer
- The `Stage` parameter is misaligned, read as `this`
- Crash in `d3d8.dll` at `0001:00052508`, runtime 00:00:01
- Crash object: `LoadingScreen Gadget` (first thing rendered)

### The Fix

```c
// CORRECT — D3D8 COM methods are __stdcall
typedef int (__stdcall *SetTSS_t)(void*, DWORD, DWORD, DWORD);
```

With `__stdcall`, the compiler pushes all four parameters (including
`this`) onto the stack in right-to-left order, matching what the D3D8
method expects.

### How to Verify

The game's own disassembly confirms this — in `Graphics_BeginFrame`:
```asm
PUSH EAX          ; this pointer pushed ON STACK
CALL [ECX+0x94]   ; calling D3D8 vtable method
```
If D3D8 methods were `__thiscall`, the game would use `MOV ECX,EAX`
before the call. It doesn't — it pushes `this` on the stack.

## sharp_textures Mod

### How It Works

1. **Hook:** 7-byte detour on `Graphics_BeginFrame` (0x00453B50)
2. **Per frame:** After original function runs, retrieves D3D8 device from
   `Graphics+0x154`
3. **Applies filters:** Calls `SetTextureStageState` on all 8 stages (0-7)
   with configured filter values
4. **Config:** `sharp_textures.txt` auto-generated with plain values

### Config File
```
# Values: 0=none, 1=point(sharp), 2=linear(smooth), 3=anisotropic
MAGFILTER = 1
MINFILTER = 1
MIPFILTER = 1
```

### Safety
- No IAT hooks (Android/Wine safe)
- No background threads
- `IsBadReadPtr` checks before all pointer dereferences
- Trampoline preserves original 7-byte prologue

## References

- MinGW d3d8.h: `/usr/i686-w64-mingw32/include/d3d8.h` (line 898 for SetTextureStageState)
- MinGW d3d8types.h: Filter constants (lines 870-877)
- Ghidra disassembly: `Graphics_BeginFrame` at 0x00453B50
- Mod source: `mods/sharp_textures/sharp_textures.c`
