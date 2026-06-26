# Universal Safespots

A bass.dll proxy mod that adds a "universal" SAFESPOT type to Hamsterball.

## What It Does

Place a `SAFESPOT(*)` reference point in your level's MESHWORLD file. When the ball
has an active checkpoint filter (e.g. `(B)` from an `E:SAFESWITCH(B)` trigger), normal
SAFESPOTs with non-matching letters are rejected by the respawn search. This mod
intercepts that rejection and checks if the SAFESPOT name contains `(*)`. If found,
the SAFESPOT is accepted regardless of the current filter — it competes with
matching SAFESPOTs purely on distance.

## How It Works

In `Ball_FindClosestRespawnPoint` (0x405190), the filter check at 0x405894 does:

```
jne 0x4058C2    ; reject if strnicmp != 0 (letters don't match)
```

The mod replaces this 2-byte `jne` (plus 3 bytes of the accept path that follow it)
with a 5-byte JMP to a code cave. The code cave:

1. If `strnicmp` matched (eax==0): accept (original behavior)
2. If `strnicmp` didn't match: calls `strstr(safespot_name, "(*)")`
   - If found: accept (override — universal safespot)
   - If not found: reject (original behavior)

The `strstr` function (0x4BAC20) is statically linked in the EXE and preserves `edi`.

## Hook Details

| Address | Original Bytes | Description |
|---------|---------------|-------------|
| 0x405894 | `75 2C` | `jne 0x4058C2` (reject non-matching filter) |
| 0x405896 | `8B 07` | `mov eax, [edi]` (accept path: load safespot name) |
| 0x405898 | `68 90 F4 4C 00` | `push 0x4CF490` (accept path: push "[Z]" string) |

5 bytes at 0x405894 are replaced with `E9 XX XX XX XX` (JMP to code cave).

## Usage

1. Rename the original `bass.dll` to `bass_real.dll` in the game folder
2. Copy this mod's `bass.dll` into the game folder
3. Add `SAFESPOT(*)` reference points in your level's MESHWORLD file
4. The universal SAFESPOTs will be accepted regardless of active checkpoint filter

## Safety

- No threads, no IAT hooks
- No C function calls from the code cave (pure assembly, calls statically linked `strstr`)
- `[Z]`/`[X]` gravity checks still apply after acceptance
- Multiplayer proximity checks (2P mode) still apply

## Compilation

```bash
i686-w64-mingw32-gcc -shared -o bass.dll universal_safespots.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```
