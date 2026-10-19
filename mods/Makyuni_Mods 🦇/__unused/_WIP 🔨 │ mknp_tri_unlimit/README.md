# mkn_plus_tri_unlimit

Removes D3D8 triangle/vertex rendering limits in Hamsterball. Bass.dll proxy mod.

## What it does

| Limit | Cause | Fix |
|-------|-------|-----|
| ~22K triangles per MeshBuffer | D3DFMT_INDEX16 (16-bit indices, max 65534 verts ÷ 3 = 21845 tris) | Patch to D3DFMT_INDEX32 + remove 0xFFFE checks + fix buffer size (×6→×12) |
| ~65K total vertices | Combined D3D8 VB creation fails when total > 65534 | Force fallback render path (DrawPrimitiveUP) |

## Patches applied

### 1. MeshBuffer_Allocate (0x00480c4d)

- **NOP 0xFFFE vertex check** at 0x00480c8e: `0F 83 12 04 00 00` → `90 90 90 90 90 90`
- **NOP 0xFFFE face check** at 0x00480c97: `0F 83 09 04 00 00` → `90 90 90 90 90 90`
- **INDEX16→INDEX32 + size fix** at 0x00480cb4: 7-byte code cave detour replacing:
  - `6A 65` (PUSH 0x65 / D3DFMT_INDEX16) → `6A 66` (PUSH 0x66 / D3DFMT_INDEX32)
  - `D1 E2` (SHL EDX,1 / ×2 for 2-byte indices) → `C1 E2 02` (SHL EDX,2 / ×4 for 4-byte indices)

### 2. MeshWorld_BuildVertexBuffer (0x0046f8d0)

5-byte JMP detour. When total vertex count across all MeshBuffers exceeds 65534, sets `meshworld+0x459=0` and clears the combined VB pointers. This preserves per-submesh CPU vertex data for the fallback render path (DrawPrimitiveUP).

## Installation

1. Rename original `mknp_tri_unlimit.dll` to `bass_real.dll`
2. Copy `mkn_plus_tri_unlimit.dll` as `mknp_tri_unlimit.dll` into the game folder

## Build

```
i686-w64-mingw32-gcc -shared -o mkn_plus_tri_unlimit.dll source/mkn_plus_tri_unlimit.c \
  source/bass.def -O2 -static -static-libgcc \
  -Wl,--enable-stdcall-fixup -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
```

## Compatibility

- Game version: V3.6.c
- Use case: Custom levels with high-poly meshes (>22K triangles per GEOMOBJECT, or >65K total vertices)
- Crash-tested: 35s startup survival on Wine/Xvfb ✓

## Source

Based on the Rodent_tri_v2 approach. All patch addresses verified against the binary.
