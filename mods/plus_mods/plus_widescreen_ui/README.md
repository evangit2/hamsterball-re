# Widescreen UI Fix

A Hamsterball Plus mod that corrects UI stretching when running the game in widescreen resolutions.

## Problem

Hamsterball uses a single perspective projection matrix for ALL rendering — 3D world, menus, HUD, text, everything. When you force a widescreen resolution, the aspect ratio changes, and UI elements stretch horizontally because there's no separate UI projection.

## Solution

This mod hooks `Graphics_SetViewport` (0x454f10) and `Scene_Render` (0x41a2e0) to distinguish between 3D and UI rendering passes. When the game is about to render UI elements (timer, position indicator, menus, etc.), the mod overrides the D3D projection matrix and viewport to 4:3 aspect, creating a pillarbox effect for UI only. 3D world rendering keeps the full widescreen field of view.

## How It Works

1. **Scene_Render hook** tracks rendering phase per frame — first `(0,0)` call = 3D world, second `(0,0)` call = UI
2. **Graphics_SetViewport hook** intercepts each viewport reset. After the original function runs:
   - **3D pass**: leaves the widescreen perspective matrix untouched (full FOV)
   - **UI pass**: overrides to a 4:3 perspective matrix + centered 4:3 D3D viewport (pillarbox)
3. Split-screen 2P: first viewport calls use split params, final `(0,0)` = UI override
4. Menus (0 player count): all calls are `(0,0)`, first = 3D (background), second = UI

## Controls

Toggle in Options menu: **Widescreen UI Fix** (default: ON)

## Technical Details

- **D3D8 vtable indices used**: SetTransform=[37] (D3DTS_PROJECTION=3), SetViewport=[40]
- **Calling convention**: `__stdcall` for D3D8 COM methods (device pointer on stack, not in ECX)
- **Perspective matrix**: Same formula as the game's `Matrix_BuildPerspectiveFOV` (FOV=π/4, aspect=4/3, near/far from gfx struct)
- **Gfx struct offsets**: +0x154=IDirect3DDevice8*, +0x5c→present params→+0x15c=bbWidth/+0x160=bbHeight, +0x790=nearPlane, +0x794=farPlane

## Installation

1. Compile with Visual Studio (32-bit/x86)
2. Place `WidescreenUIFix.dll` in game's `Mods\` folder
3. Enable in Options menu

## Source

`WidescreenUIFix.cpp` — single file, ~170 lines

## Author

BookwormKevin
