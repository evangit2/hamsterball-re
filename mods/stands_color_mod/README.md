# StandsColorMod.CEA

Runtime STANDS color override for Arena modes in Hamsterball.

## What it does

Hooks `App_ResetFrame` (0x0046C200, called every frame) and walks the pointer chain to the arena stands MeshWorld material array, overwriting diffuse and ambient colors with custom values.

## Pointer chain (Ghidra-verified)

```
[0x005341E0]         → App
[App + 0x178]       → board/scene
[board + 0x4390]    → stands Level object (set by CameraLookAt @ 0x00413280)
[standsLevel + 0x08] → MeshWorld
[MeshWorld + 0x24]   → material count
[MeshWorld + 0x28]   → material array (each 0x50 bytes)
```

## Material structure (0x50 bytes, D3DMATERIAL8 at +0x04)

| Offset | Field | Size |
|--------|-------|------|
| +0x04 | Diffuse RGBA | 16 bytes |
| +0x14 | Ambient RGBA | 16 bytes |
| +0x24 | Specular RGBA | 16 bytes |
| +0x34 | Emissive RGBA | 16 bytes |
| +0x44 | Power | 4 bytes |
| +0x48 | Texture ptr | 4 bytes (NULL=no texture) |

## Configuration

Edit `newDiffuse` and `newAmbient` in the CEA script:

```
newDiffuse:
  dd 3F800000  // R = 1.0
  dd 00000000  // G = 0.0
  dd 00000000  // B = 0.0
  dd 3F800000  // A = 1.0
```

IEEE 754 hex reference:
- 1.0 = `3F800000`
- 0.5 = `3F000000`
- 0.25 = `3E800000`
- 0.0 = `00000000`

## Texture note

If stands have a checker texture (material+0x48 != NULL), diffuse color MODULATES the texture. For solid color, uncomment the texture-clear line in the loop.

## Stands lifetime

Stands are recreated every arena match (CameraLookAt → Level_MeshWorldCtor). The script patches every frame so it survives reloads automatically.
