# custom_events

Custom E: collision events for Hamsterball — define your own event types with DLL-side handlers.

## v1: E:SOUND

Plays a custom sound file from the `Sounds/` folder when the ball touches an `E:SOUND` collision object in a MESHWORLD level file. Works exactly like `E:POPOUT` (cooldown timer + dizzy immunity) but with a user-specified sound instead of the hardcoded popout sound.

### MESHWORLD Format

Name a collision plane in your level MESHWORLD:

```
E:SOUND<file>mysfx</file>
```

When the ball touches this plane, the mod will:
1. Load `Sounds/mysfx.ogg` (or `Sounds/mysfx.wav` if .ogg not found)
2. Play it via BASS
3. Set a 50-frame cooldown (same as E:POPOUT)
4. Grant 100 frames of dizzy immunity (same as E:POPOUT)

Sound files are cached after first load — subsequent collisions play instantly.

### Installation

1. Rename original `bass.dll` to `bass_real.dll` (if not already done)
2. Copy this mod's `bass.dll` into the game folder
3. Create a `Sounds/` folder in the game directory (if it doesn't exist)
4. Place `.ogg` or `.wav` files there

### Sound File Location

```
Hamsterball/
├── Hamsterball.exe
├── bass.dll          ← this mod
├── bass_real.dll     ← original BASS
├── Sounds/
│   ├── mysfx.ogg     ← your custom sounds
│   ├── explosion.wav
│   └── ...
└── sounds/           ← original game sounds (separate folder)
    ├── collide.ogg
    └── ...
```

Note: `Sounds/` (capital S) is for custom sounds. The game's original sounds are in `sounds/` (lowercase). Windows is case-insensitive so both work, but keeping them separate is cleaner.

### Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll custom_events.c \
  -I../shared -lwinmm -Wl,--enable-stdcall-fixup \
  -O2 -static -static-libgcc -Wl,--add-stdcall-alias \
  -msse2 -mfpmath=sse
```

### How It Works

The mod hooks `DispatchCollisionEvents` (0x40C5D0) — the game's base collision handler. Every collision event flows through this function, with the event name string accessible at `collision_entry[1]+0x864`.

When the event name starts with `E:SOUND`, the mod:
1. Parses the `<file>...</file>` XML-like tag
2. Loads the named sound file via `BASS_StreamCreateFile` (from bass_real.dll)
3. Plays it via `BASS_StreamPlay` (BASS 2.0) or `BASS_ChannelPlay` (BASS 2.4)
4. Applies the same cooldown + dizzy immunity as E:POPOUT

### BASS Version Detection

The mod auto-detects BASS version by checking for `BASS_ChannelSetAttribute` (2.4-only):
- **BASS 2.0** (original): Uses `BASS_StreamCreateFile` (DWORD params) + `BASS_StreamPlay`
- **BASS 2.4** (updated): Uses `BASS_StreamCreateFile` (QWORD params) + `BASS_ChannelPlay`

### Adding More Custom Events

The hook intercepts ALL collision events. To add a new event type (e.g. `E:PARTICLE`), add a new check in `SoundCollisionHandler`:

```c
if (_strnicmp(eventName, "E:PARTICLE", 10) == 0) {
    /* your custom logic here */
}
```

### Log File

The mod writes `custom_events_log.txt` next to `bass.dll` with initialization info and collision event logs (first 10 events).

### Technical Details

| Property | Value |
|----------|-------|
| Hook target | `DispatchCollisionEvents` (0x40C5D0) |
| Hook type | 8-byte detour (JMP + 3 NOPs) |
| Calling convention | `__thiscall` (ECX=board, stack: ball, collObj, RET 0x8) |
| Event name offset | `collision_entry[1] + 0x864` |
| Cooldown field | `ball + 0x7CC` (50 frames, same as E:POPOUT) |
| Dizzy immunity | `Ball_DizzyImmunity(ball, 100)` — same as E:POPOUT |
| Sound cache | 32 entries max, keyed by filename |
| BASS functions | Loaded from `bass_real.dll` at runtime |
