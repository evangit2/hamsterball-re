# Portal Mod (bass.dll proxy)

E:PORTAL(N) teleportation system for Hamsterball. When the ball touches an `E:PORTAL(N)` collision event, it teleports to the `PORTALPOS(N)` S1 ref point. Optional `PORTALVEC(N)` sets exit direction — ball preserves incoming speed but redirects along the portal vector.

## How it works

1. Hooks `DispatchCollisionEvents` (0x0040C5D0) via trampoline
2. Intercepts `E:PORTAL(N)` collision events
3. Looks up `PORTALPOS(N)` in the level's S1 ref point table
4. Sets native teleport flag (ball+0xC3C=1) + destination coords (ball+0xC40/44/48)
5. If `PORTALVEC(N)` exists: redirects velocity along (PORTALVEC - PORTALPOS), preserving speed
6. If no `PORTALVEC`: keeps input direction and speed
7. 30-frame cooldown per player to prevent rapid re-triggering

## MESHWORLD setup

Add to your level's S1 ref points:
```
PORTALPOS(1)    - destination position
PORTALVEC(1)    - exit direction marker (optional)
```

Add to your level's S6 meshbuffer names:
```
E:PORTAL(1)     - trigger plane (collision event)
```

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Put this `bass.dll` in the game folder

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll portal_mod.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
```

## Differences from HB+ version

This is the bass.dll proxy version of the HB+ PortalMod. The core logic is identical — same offsets, same ref point lookup, same teleport flag system. The only difference is the hook mechanism:

| HB+ version | bass.dll proxy version |
|---|---|
| `onEventPlaneCollide` callback | `DispatchCollisionEvents` trampoline hook |
| `api->GetScene()` | Global `g_Scene` at 0x005341E4 |
| `api->Play3dSoundEffect()` | (omitted — no sound in proxy version) |
| `CreateModInstance` export | `bass.def` forwarding + `DllMain` thread |

Author: RodentRacer / Hamsterbot
