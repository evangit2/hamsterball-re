# Light Platforms Mod

Controls ArenaStands (DFLOOR1-4) platform visibility based on the Neon Race
light state. When the Neon light is ON, platforms stay visible. When OFF,
platforms disappear — with a native flicker effect during transitions.

## How it works

ArenaStands objects have a 4-state visibility cycle:

| State | Description | In render list? | Renders? |
|-------|-------------|-----------------|----------|
| 0 | Solid visible (stable) | Yes | Yes |
| 1 | Flicker before disappearing | Yes | Flicker* |
| 2 | Invisible (stable) | No | No |
| 3 | Flicker after reappearing | Yes | Flicker* |

*During flicker states, a ToggleTimer (obj+0x10EC) toggles a visible flag
every 100 frames. The render function (vtable slot 18 = 0x437560) skips
rendering when the flag is 0, causing the visual flicker. The object stays
in the render list during flicker, so **collision remains active** —
platforms are physically tangible while flickering.

## Mod behavior

- Light ON + platform invisible: set state=3 (flicker → reappear)
- Light ON + platform solid: pin timer=75 (stay visible)
- Light OFF + platform solid: set state=1 (flicker → disappear)
- Light OFF + platform invisible: pin timer=75 (stay invisible)
- During flicker states: let native state machine run naturally

The light state is read from the Neon Race SceneObject at board+0x436C,
field +0x88 (visible flag, set by E:LIGHTSON/E:LIGHTSOFF collision events).

## Hook

Hooks Graphics_RenderScene entry (0x454BC0) — runs after Board_UpdateRaceState
so the native state machine ticks first, then we override.

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll light_platforms.c \
  -I"../../_WIP 🔨 │ shared" -lwinmm -Wl,--enable-stdcall-fixup -O2 \
  -static -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
```
