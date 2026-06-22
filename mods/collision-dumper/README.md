# Collision Dumper

Dumps all fields of every collision entry from the game's PhysicsObject to a text file, for reverse-engineering unknown struct fields.

## What It Does

- Hooks `Ball_Update` (0x405E00) with a 5-byte JMP detour
- After each `Ball_Update` completes, walks the PhysicsObject's collision entry list
- Dumps 0x80 bytes (32 DWORDs) per entry as hex + float, with known-field annotations
- Throttled to ~1 dump per second to avoid flooding
- Output: `collision_dump.txt` in the game directory

## Entry Fields (Known)

| Offset | Type | Field |
|--------|------|-------|
| +0x00 | int32 | type (1=ball-ball, 2=wall, 5=floor) |
| +0x0C | void* | other_ball (type==1 only) |
| +0x20 | float | normal_x |
| +0x24 | float | normal_y |
| +0x28 | float | normal_z |
| +0x2C | float | collision_pt |
| +0x30 | float | normal2_x |
| +0x34 | float | normal2_y |
| +0x38 | float | normal2_z |
| +0x64 | int32 | collision_id |

All other offsets are UNKNOWN — that's what this dumper is designed to test.

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll collision_dumper.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias
```

## Install

1. Rename original `bass.dll` → `bass_real.dll` in the Hamsterball directory
2. Copy this compiled `bass.dll` to the same directory
3. Launch `Hamsterball.exe`
4. A MessageBox confirms the hook is installed
5. Play the game — collide with walls and balls
6. Check `collision_dump.txt` for output
