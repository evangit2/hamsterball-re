# Rodent_tri_limit

Removes the 65,534 combined vertex limit in Hamsterball MESHWORLD level loading.

## Problem

Hamsterball concatenates ALL MeshBuffers' submesh vertex data into ONE combined D3D8 vertex buffer (`MeshWorld_BuildVertexBuffer` @ `0x0046f8d0`). If the total exceeds 65,534 vertices, the combined VB can be created but rendering breaks because 16-bit indices cannot reference vertices past 65,534. Worse, the original function **frees the per-submesh CPU vertex data before attempting VB creation** — so when rendering fails, the data is lost and the level can't render at all.

## Solution

Hooks `MeshWorld_BuildVertexBuffer` via a 5-byte JMP detour (bass.dll proxy pattern). The hook:

1. Counts total vertices across all optimized MeshBuffers' submeshes (without freeing anything)
2. If total ≤ 65,534: calls the original function (combined VB works fine)
3. If total > 65,534: sets `meshworld+0x459 = 0` (force fallback render path) and returns **without calling the original** — preserving the per-submesh CPU vertex data

The game's own fallback render path in `SceneObject_RenderFull` handles rendering via `DrawPrimitiveUP` with the preserved CPU data. No render hook needed.

## Installation

1. Rename the original `bass.dll` in the game folder to `bass_real.dll`
2. Copy this mod's `bass.dll` into the game folder

## Technical Details

- **Hook target**: `MeshWorld_BuildVertexBuffer` @ `0x0046f8d0`
- **Prologue**: `83 EC 10 53 55` (SUB ESP,0x10; PUSH EBX; PUSH EBP — 5 bytes)
- **Limit**: 65,534 (0xFFFE) — the max vertices indexable with 16-bit indices
- **Per-MeshBuffer limit**: NOT touched (inherent to D3DFMT_INDEX16)
- **Log file**: `Hamsterball_tri_limit.log` in the game folder

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll Rodent_tri_limit.cpp \
  bass_exports.def -O2 -static -static-libgcc \
  -Wl,--enable-stdcall-fixup -Wl,--add-stdcall-alias
```

## Author

Hamsterbot for RodentRacer
