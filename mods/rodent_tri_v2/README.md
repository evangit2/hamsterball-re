# Rodent_tri_v2

Removes ALL triangle/vertex rendering limits in Hamsterball by combining two fixes:

1. **Per-MeshBuffer 16-bit index limit (22K tris)** — Patches `MeshBuffer_Allocate` (0x00480c4d) to use D3DFMT_INDEX32 instead of D3DFMT_INDEX16, and removes the 0xFFFE vertex/face count checks.

2. **Combined vertex buffer limit (65K verts)** — Hooks `MeshWorld_BuildVertexBuffer` (0x0046f8d0) to force the fallback render path (DrawPrimitiveUP) when total vertices exceed 65534.

## What it fixes

| Limit | Cause | Fix |
|-------|-------|-----|
| ~22K triangles per MeshBuffer | D3DFMT_INDEX16 (16-bit indices, max 65534 vertices ÷ 3 = 21845 tris) | Patch to D3DFMT_INDEX32 + remove 0xFFFE check + fix buffer size (×6→×12) |
| ~65K total vertices | Combined D3D8 VB creation fails when total > 65534 | Force fallback render path (DrawPrimitiveUP) |

## Patches applied

### MeshBuffer_Allocate (0x00480c4d)

1. **NOP 0xFFFE vertex check** at 0x00480c8e: `0F 83 12 04 00 00` → `90 90 90 90 90 90`
2. **NOP 0xFFFE face check** at 0x00480c97: `0F 83 09 04 00 00` → `90 90 90 90 90 90`
3. **INDEX16→INDEX32 + size fix** at 0x00480cb4: 7-byte code cave detour replacing:
   - `6A 65` (PUSH 0x65 / D3DFMT_INDEX16) → `6A 66` (PUSH 0x66 / D3DFMT_INDEX32)
   - `D1 E2` (SHL EDX,1 / ×2 for 2-byte indices) → `C1 E2 02` (SHL EDX,2 / ×4 for 4-byte indices)

### MeshWorld_BuildVertexBuffer (0x0046f8d0)

5-byte JMP detour. When total vertex count across all MeshBuffers exceeds 65534, sets `meshworld+0x459=0` and returns without calling the original function. This preserves per-submesh CPU vertex data for the fallback render path.

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy `bass.dll` from this mod into the game folder

## Build

```
i686-w64-mingw32-gcc -shared -o bass.dll Rodent_tri_v2.cpp \
  bass_exports.def -O2 -static -static-libgcc \
  -Wl,--enable-stdcall-fixup -Wl,--add-stdcall-alias
```

## Compatibility

- Game version: V3.6.c
- Use case: Custom levels with high-poly meshes (>22K triangles per GEOMOBJECT, or >65K total vertices)

## Log

The mod writes to `Hamsterball_tri_v2.log` in the game folder.
