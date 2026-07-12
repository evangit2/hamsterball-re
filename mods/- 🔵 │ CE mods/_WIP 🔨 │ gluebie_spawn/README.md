# Gluebie Spawn Mod

Spawns animated Gluebie (tar blob) objects at Player 1's position on hotkey press.
Works globally in any race or arena.

## What It Does

- Press the CE hotkey to spawn a Gluebie at Player 1's exact position
- Spawned Gluebies animate (wobble, rotate, scale) like original game Gluebies
- Balls within ~30 units of a spawned Gluebie get the tar effect:
  - Ball slows down (velocity × 0.85 per frame)
  - Tar sound plays on first contact
  - When ball leaves range: recovers (flags cleared, normal speed restored)
- Up to 16 Gluebies can be spawned simultaneously
- Gluebie array clears on level/board change

## How It Works

### The Bug In v1
The original script used C array indices (`0xB3`, `0x1DA`) as byte offsets in CE assembly.
In C, `param_1[0xB3]` accesses byte offset `0xB3*4 = 0x2CC`, but in x86 assembly
`[esi+0xB3]` accesses byte offset `0xB3` directly. The correct byte offsets are:

| C Index | Byte Offset | Field |
|---------|-------------|-------|
| 0xB3    | 0x2CC       | tar_collision_flag |
| 0x1DA   | 0x768       | physics_enabled    |

### What Was Fixed

1. **Byte offset correction**: `[esi+0xB3]` → `[esi+0x2CC]`, `[esi+0x1DA]` → `[esi+0x768]`
2. **Tar sound effect**: Calls `Sound_Play3D` (0x459860) on first contact with a Gluebie
   - Gets sound resource via: `ball+0x14` → board → `+0x878` → scene → `+0x484`
   - Pushes ball X/Y/Z position as 3 floats + 1.0f (volume)
   - Only plays when `ball+0x2CC == 0` (first contact, not already tarred)
3. **Recovery on leaving range**: When ball moves away from all Gluebies:
   - Clears `ball+0x2CC = 0` (tar_collision_flag)
   - Restores `ball+0x768 = 1` (physics_enabled = normal)
   - This lets the ball recover speed after leaving the tar area

### Tar Physics (in Ball_Update at 0x4081D9)

```asm
; Check physics_enabled flag
0x004081D9: MOV AL, [ESI+0x768]
0x004081DF: TEST AL, AL
0x004081E1: JNZ  0x004081F7      ; if 1 (normal) → speed boost

; TAR PATH (0x768 == 0): multiply blend by 0.85
0x004081E3: FLD  [ESI+0x764]     ; load vel_blend_factor
0x004081E9: FMUL [0x004CF4C0]    ; × 0.85
0x004081EF: FSTP [ESI+0x764]
0x004081F5: JMP  0x0040823D

; NORMAL PATH (0x768 != 0): multiply blend by 1.1
0x004081F7: FLD  [ESI+0x764]
0x004081FD: FCOMP [0x004CF538]   ; compare with 89128.96
0x0040820A: MOV  [ESI+0x764], 0x3C23D70A  ; cap to 0.01
0x0040821A: FMUL [0x004CF4B8]   ; × 1.1 (double)
0x00408233: MOV  [ESI+0x764], 0x3F800000  ; cap to 1.0
```

## Addresses

| Address | Function | Purpose |
|---------|----------|---------|
| 0x405E22 | Ball_Update hook point | Per-frame ball physics tick |
| 0x437CB0 | Gluebie_ctor | Constructs Gluebie object (0x110C bytes) |
| 0x4BA57B | operator_new | Memory allocation |
| 0x461510 | MeshWorld_ctor | Loads Level3-Gluebie mesh |
| 0x453810 | AthenaList_Append | Adds to render/update lists |
| 0x459860 | Sound_Play3D | Plays 3D positioned sound |
| 0x4D0728 | "Level3-Gluebie" string | Mesh file name |

## Ball Struct Offsets

| Offset | Type | Field |
|--------|------|-------|
| +0x014 | ptr  | Board/level pointer |
| +0x018 | int  | Player index (0 = Player 1) |
| +0x164 | float | Ball X position |
| +0x168 | float | Ball Y position |
| +0x16C | float | Ball Z position |
| +0x2CC | byte | tar_collision_flag (1 = tarred) |
| +0x768 | byte | physics_enabled (1 = normal, 0 = tar) |
| +0x764 | float | vel_blend_factor (decayed by 0.85 in tar) |

## Gluebie Struct Offsets

| Offset | Type | Field |
|--------|------|-------|
| +0x10D0 | ptr  | Scene pointer |
| +0x10D4 | float | Position X |
| +0x10D8 | float | Position Y |
| +0x10DC | float | Position Z |
| +0x10F0 | float | Random rotation (0-360) |
| +0x10F4 | float | Direction (1.0 or -1.0) |
| +0x10F8 | float | Wobble speed |
| +0x1108 | float | Scale factor (1.0) |

## Constants

| Address | Type | Value | Purpose |
|---------|------|-------|---------|
| 0x4CF4C0 | float32 | 0.85 | Tar friction multiplier |
| 0x4CF4B8 | float64 | 1.1  | Normal speed multiplier |
| 0x4CF538 | float64 | 89128.96 | Speed cap threshold |
| 0x4CF310 | float32 | 1.0  | Max blend factor |
