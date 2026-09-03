# WobblyBridgeFix — Invisible Wavy Bridge Fix (Wobbly Race)

Fixes the wavy bridge in Wobbly Race (Level 7) that is invisible on some computers (Intel integrated, Wine/Proton) while collision still works.

## Root cause

`Wavy_Update` at `0x440390` does `IDirect3DVertexBuffer8::Lock` with `D3DLOCK_DISCARD (0x2000)` at `0x440752`:

```
0x440752: 68 00 20 00 00  PUSH 0x2000
0x440757: 50             PUSH EAX (VB ptr)
0x440758: FF 51 38       CALL [ECX+0x38]  ; Lock
```

The vertex buffer (342 verts, 87 tris, `BrightGreenChecker.bmp`, bounds `[-75,-25,-700]→[75,0,0]`, `Wavy_ctor 0x43AD40` / `Wavy_Configure 0x435440`) was **not** created with `D3DUSAGE_DYNAMIC`. Strict drivers reject `DISCARD` on non-dynamic VBs → `Lock` fails → the `REP MOVSD` copy at `0x440773` writes to stack garbage → `Unlock` at `0x44078B` leaves the GPU VB with stale zeros → invisible.

## Fix

Patch `PUSH 0x2000` → `PUSH 0` at `0x440752` (single byte `0x20→0x00` at `0x440754`). Plain lock (`flags=0`) succeeds on all drivers. Cost: one 342-vertex lock every 3 frames — negligible.

Applied 500 ms after `DLL_PROCESS_ATTACH` via `VirtualProtect(PAGE_EXECUTE_READWRITE)`. Verified with `WobblyBridgeFix.log` next to the DLL.

## Install

1. Rename original `bass.dll` → `bass_real.dll` (keep a backup).
2. Copy `WobblyBridgeFix.dll` → `bass.dll` in the Hamsterball folder.
3. Or if you have a mod loader that loads `WobblyBridgeFix.dll` by name, drop it as-is.

## Build

```
i686-w64-mingw32-gcc -shared -o bass.dll WobblyBridgeFix.c bass.def -lwinmm -lshlwapi -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
cp bass.dll WobblyBridgeFix.dll
```

## Addresses

* `Wavy_ctor 0x43AD40` (vtable `0x4D5458`, slot 11 = `0x440390`)
* `Wavy_Configure 0x435440`
* `Wavy_Update 0x440390` (Lock `0x440752`, Unlock `0x44078B`, `Level7-Wavy1.MESHWORLD` 342 verts)
* Patch site: `0x440752` `68 00 20 00 00` → `68 00 00 00 00`

