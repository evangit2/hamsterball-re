# mknp_battyball_events (HB+ v2.1)

Second mod in the **battyball** series — companion to `mknp_battyball_entities`.

Custom `E:` event planes live here. v1j adds the `pause` event property.

## What it does (v1j)

- Reads `mknp_battyball_events_set.jsonc` next to the DLL (re-read every level start).
- Each `"E:name", { ... }` entry fires when the player ball touches that event
  plane in the MESHWORLD file — except `trigger_type` 2/3, which fire at
  level start and never on touch.
- `"sound": "Name"` plays one sound; `"sound1".."sound9"` picks randomly
  between up to 9 variants on every touch (native and custom mix freely).
- `"trigger_type": 0` fires every touch (default); `1` fires once, then never
  again until the level is re-entered; `2` fires at level start and loops;
  `3` fires once at level start.
- `"loop_sound": true` replays a fresh random variant each cycle, when the
  previous one ends plus the `"loop_delay"` gap.
- `"loop_delay"` is seconds (decimals ok, e.g. `2.5`): the period between
  loop starts. Shorter than the sound = overlap. Default 0 = back-to-back
  by sound length. Fastest possible is 1 frame (~16ms).
- `"pause": true` (default) = the event's sounds stay silent while the game
  is paused and resume on unpause (loop clocks freeze, touches are skipped).
  `"pause": false` = the event ignores pause and keeps playing through it.
- Native `sound` (any of the 61 game sounds): played from its game slot.
- Custom `sound` (`Sounds/<name>.ogg`/`.wav`, not in the game): preloaded at
  level start through the game's own loader, then played 3D positioned.
- One play per touch (1s re-fire gate while resting on the plane).
- Missing file logs `SFX preload <name> FAIL` and retries next level.
- Missing set file is auto-created with the `E:sfx_bell` example.

## Set file

```jsonc
[
    "E:sfx_bell", { "trigger_type": 2, "loop_delay": 200, "sound1": "Testrand1", "sound2": "Testrand2", "sound3": "Testrand3" }
]
```

## HB+ Options

| Control | ID             | Default | Range | Description                                |
|---------|----------------|---------|-------|--------------------------------------------|
| Toggle  | `BATTY_EVENTS` | ON      | -     | Master switch. When OFF, events are quiet. |

## Log

```
INIT Battyball Events v1j (mod loaded)
SET event E:sfx_bell sound=Testrand1 off=0x0
SET event E:sfx_bell sound=Testrand2 off=0x0
SET event E:sfx_bell sound=Testrand3 off=0x0
SET event E:sfx_bell trig=2 loop=0 delay=200000ms pause=1 vars=3
SET init: 1 sound events
LEVEL start
SFX preload Testrand1 OK
SFX preload Testrand2 OK
SFX preload Testrand3 OK
SFX dur Testrand1 850ms
SFX autoloop start Testrand1 for E:sfx_bell (#1)
SCENE end
```

## Build

```bash
cd source && ./build.sh
```

MinGW `i686-w64-mingw32-g++`, same flags as `mknp_battyball_entities`.
