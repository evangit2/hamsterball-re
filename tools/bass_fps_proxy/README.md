# Hamsterball FPS Uncap — bass.dll Proxy

## What This Does

Hamsterball has a built-in FPS limiter that caps rendering at ~100 FPS (in practice ~75 FPS on most systems due to 15.6ms timer granularity). This proxy DLL replaces `bass.dll` — the game ships it and imports 10 audio functions from it — with a shim that forwards all audio calls to the original (`bass_real.dll`) while patching the game's rendering limiter in memory.

**Result:** the game renders as fast as the GPU can push frames — uncapped FPS, butter-smooth motion.

## Why bass.dll?

The game imports `bass.dll` for audio. Windows DLL search order loads our proxy first (same directory as the EXE), so our `bass.dll` loads instead of the original, then loads the original as `bass_real.dll` and forwards every function call to it. This is a classic "DLL proxy" technique — audio works identically, but our code runs inside the game's process with full memory access.

## The Failed Approaches (and why they failed)

Before arriving at the final solution, we went through several iterations. Understanding why each failed is critical for understanding why the final approach works.

### Attempt 1: Patch fps_target (100 → 1000)

**Theory:** The game has a struct field `fps_target` (at `App+0x830`) set to `100` in the constructor. Raising it to `1000` should raise the FPS cap.

**Why it failed:** `fps_target` is read by an `idiv` instruction in `App_Run` that computes `render_interval = time_slice / fps_target`. This `idiv` runs **once at the top of `App_Run`**, before the main loop begins. Our patches fire 500ms after DLL load (via a background thread) — by then, `App_Run` has already executed the `idiv` and stored `render_interval` in a stack variable. Patching the struct value after that point has zero effect on the running game.

### Attempt 2: Patch the idiv instruction itself

**Theory:** Instead of patching the struct value, patch the `idiv` instruction in the code so it divides by 1000 instead of 100.

**Why it failed:** Same timing problem. The `idiv` executes once at function entry. Even if we patch the instruction bytes, the computation has already happened — the result is sitting on the stack as a local variable. Changing the code after it runs is useless.

**Also:** raising `fps_target` *without* first fixing the timer causes the `frame_time` threshold to go negative, producing a **white screen**. The game's `GetTickCount()` returns 15.6ms-resolution values (Windows default), so when `fps_target` is high enough that the expected frame time (1/FPS) is smaller than the timer resolution, the subtraction underflows. This is why you can't just raise `fps_target` — you must also hook `GetTickCount` with `QueryPerformanceCounter` first.

### Attempt 3: Patch with short 3-byte patterns

**Theory:** Use byte-pattern matching to find and patch the `cmp eax, 10` (spin counter cap) instruction.

**Why it failed:** The 3-byte pattern `83 F8 0A` (`cmp eax, 10`) matches **13 locations** in the `.text` section — most of which have nothing to do with the frame limiter. Overwriting all of them corrupted audio, D3D8 initialization, and rendering code paths. The game showed a white screen with no sound.

**Fix:** Restricted to a 9-byte context pattern (`inc eax; cmp eax, 10; mov [esp+0x10], eax; jge`) that uniquely identifies the spin-counter location. This fixed the corruption but didn't fix the white screen (because the root cause was the idiv timing issue, not the spin counter).

### Attempt 4: The jbe NOP (WORKS)

**The breakthrough:** Stop trying to patch *computed values* that are baked into stack variables at init time. Instead, patch the **render decision** — a `jbe` (jump-if-below-or-equal) instruction that is checked **every single frame**.

At address `0x46BF55` in the game loop, there's a `jbe skip_render` instruction (`76 5D`). If the elapsed time since the last render is below the threshold, it jumps over the render path — skipping `Present()`. NOPing this jump (`90 90`) means the game **always falls through to the render path**, every single loop iteration, regardless of any timing value.

This works regardless of *when* our patch fires, because the instruction is re-evaluated every frame. It doesn't matter that `render_interval` was computed once at init — the `jbe` that gates rendering is checked continuously.

## The Final Solution (v7)

### Patches Applied (4 total)

| # | Patch | Address | Original Bytes | Patched Bytes | Purpose |
|---|-------|---------|---------------|---------------|---------|
| 1 | GetTickCount IAT hook | kernel32.dll IAT entry | (pointer) | `hooked_GetTickCount` | High-resolution timing — replaces 15.6ms GetTickCount with QPC-based sub-ms precision |
| 2 | Render-skip jbe NOP | VA `0x46BF55` | `76 5D` | `90 90` | Forces rendering every frame — the core FPS uncap |
| 3 | vsync disable (×2) | Pattern match | `C7 86 F8 01 00 00 01 00 00 00` | `C7 86 F8 01 00 00 00 00 00 80` | Sets `D3DPRESENT_INTERVAL_ONE` → `D3DPRESENT_INTERVAL_IMMEDIATE` (disables vsync, 2 locations) |
| 4 | timeBeginPeriod(1) | Win32 API call | N/A | N/A | Requests 1ms timer resolution from Windows |

### Why each patch is needed

1. **GetTickCount → QPC hook:** Without this, `timeBeginPeriod(1)` alone is unreliable on Windows 10 v2004+ / Windows 11. The game's `GetTickCount()` calls return 15.6ms-resolution values, and the frame-time threshold calculation can underflow (go negative) when FPS targets are high. The QPC hook gives us sub-millisecond precision (100ns on most systems).

2. **jbe NOP (the core patch):** This is the one that actually uncaps FPS. The game loop at `0x46BF55` checks whether enough time has passed since the last render. If not, it jumps over the `Present()` call. NOPing this jump forces a render every iteration. Because this instruction is checked every frame, it doesn't matter that our patch fires 500ms after startup.

3. **vsync disable:** The game calls `CreateDevice` with `D3DPRESENT_PARAMETERS.PresentationInterval = D3DPRESENT_INTERVAL_ONE` (vsync on). This caps FPS at the monitor refresh rate (60 Hz for most). Patching the constant from `1` to `0x80000000` (`D3DPRESENT_INTERVAL_IMMEDIATE`) disables vsync so the GPU can present as fast as it renders. There are 2 occurrences in the binary.

4. **timeBeginPeriod(1):** Requests that Windows use 1ms timer resolution instead of the default 15.6ms. This improves the resolution of `Sleep()`, `WaitForSingleObject()`, and other timing functions the game may use internally.

### What is NOT patched (and why)

- **fps_target (100):** Left at the original value. It's only read once at init to compute `render_interval`, and we bypass that entire code path with the jbe NOP. Patching it would risk the white-screen underflow bug if our QPC hook hasn't initialized yet.
- **fps_divisor (75):** Same — it's an init-time computed value. The jbe NOP makes it irrelevant.
- **Spin counter cap:** The game has a counter that force-renders after 10 skipped frames (a failsafe to prevent total freeze). Since we never skip renders, this counter never increments, so patching it is unnecessary.

## File Layout

```
tools/bass_fps_proxy/
├── bass_fps_proxy.c    — Full proxy DLL source (311 lines)
├── bass_exports.def    — Module definition file (exports the 10 BASS functions)
├── bass.dll            — Compiled proxy (statically linked)
└── README.md           — This file
```

## How to Build

### Prerequisites

- MinGW cross-compiler: `i686-w64-mingw32-gcc` (32-bit, since Hamsterball.exe is PE32 i386)
- On Ubuntu/Debian: `apt install gcc-mingw-w64-i686`

### Build Command

```bash
i686-w64-mingw32-gcc -shared -o bass.dll bass_fps_proxy.c \
    bass_exports.def -lwinmm \
    -Wl,--enable-stdcall-fixup -O2 \
    -static -static-libgcc -Wl,--add-stdcall-alias
```

**Critical flags:**
- `-static -static-libgcc`: Statically links libgcc, otherwise the DLL depends on `libgcc_s_dw2-1.dll` which won't exist on the target machine. **Must include both flags.**
- `-Wl,--enable-stdcall-fixup` and `-Wl,--add-stdcall-alias`: Ensures the exported function names match what the game imports (some imports use decorated names like `_BASS_Init@20`, some use plain names).
- `-lwinmm`: Links `timeBeginPeriod`/`timeEndPeriod`.
- `bass_exports.def`: Defines the 10 exports with their ordinals.

### Verify No External Dependencies

```bash
i686-w64-mingw32-objdump -p bass.dll | grep "DLL Name"
```

Should show only `kernel32.dll`, `user32.dll`, `winmm.dll`, and `bass_real.dll` — NOT `libgcc_s_dw2-1.dll`.

## How to Install

1. In your Hamsterball game folder, rename the original `bass.dll` to `bass_real.dll`
2. Copy the compiled `bass.dll` into the same folder
3. Launch the game

The proxy will:
- Load `bass_real.dll` and forward all 10 BASS audio functions to it (audio works normally)
- After 500ms, apply the 4 patches in a background thread
- Write a log file named `Hamsterball_fps.log` in the same folder

## How to Remove

Delete the proxy `bass.dll`, rename `bass_real.dll` back to `bass.dll`.

## Log Output

The DLL writes `Hamsterball_fps.log` next to the EXE:

```
Hamsterball FPS Uncap Proxy v7.0
=================================
Patches applied: 4
  GetTickCount IAT hook: OK (QPC-based sub-ms timing)
  render-skip jbe NOP @0x46BF55: OK (always render)
  vsync: INTERVAL_ONE -> INTERVAL_IMMEDIATE
  timer: timeBeginPeriod(1)
  fps_target: 100 (ORIGINAL - correct physics)
  QPC frequency: 10000000 Hz (100.000 ns resolution)
```

If the jbe patch fails (bytes don't match), it will show `FAILED (bytes mismatch)` — this means the game EXE is a different version than expected.

## Technical Details

### The jbe Instruction at 0x46BF55

In the game loop (`App_Run` / `GameLoop` at VA `0x46BD80`), after computing physics and input, the game checks whether it's time to render:

```asm
; ... time calculation ...
cmp eax, [render_interval]    ; has enough time elapsed?
jbe skip_render               ; 0x76 0x5D — if not, skip Present()
; ... D3D Present() ...
skip_render:
; ... continue loop ...
```

The `jbe` (jump if below or equal) at `0x46BF55` is the gatekeeper. NOPing it (`0x90 0x90`) makes the CPU unconditionally fall through to the render path.

### The vsync Pattern

```
Original:     C7 86 F8 01 00 00 01 00 00 00
              mov dword ptr [esi+0x1F8], 0x00000001   ; INTERVAL_ONE
Patched:      C7 86 F8 01 00 00 00 00 00 80
              mov dword ptr [esi+0x1F8], 0x80000000   ; INTERVAL_IMMEDIATE
```

This patches the `PresentationInterval` field of `D3DPRESENT_PARAMETERS` before `CreateDevice` is called. Two occurrences exist (likely one for each of the game's two `CreateDevice` paths — windowed and fullscreen).

### The GetTickCount IAT Hook

We walk the PE import table, find the `kernel32.dll` import for `GetTickCount`, and replace the function pointer with our own `hooked_GetTickCount()`. Our version uses `QueryPerformanceCounter` to get sub-millisecond precision:

```c
static DWORD WINAPI hooked_GetTickCount(void)
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (DWORD)((now.QuadPart * 1000ULL) / g_qpc_freq.QuadPart);
}
```

This is necessary because `timeBeginPeriod(1)` alone is insufficient on Windows 10 v2004+ / Windows 11 — the OS no longer reliably honors the 1ms request for `GetTickCount`.

### Key Addresses

All addresses assume image base `0x400000` (Hamsterball.exe default, no ASLR):

| Address | Description |
|---------|-------------|
| `0x46BD80` | Game loop (`GameLoop` / `App_Run` body) |
| `0x46BF55` | `jbe skip_render` — the frame render gate (patched to NOP) |
| `0x4278E0` | `WinMain` |
| `0x4FD680` | `App` global instance |
| `0x4BA57B` | Game's `operator_new` |

## Version History

| Version | Approach | Result |
|---------|----------|--------|
| v1-v2 | Patch fps_target + fps_divisor | White screen (timer underflow) |
| v3-v4 | Add GetTickCount QPC hook + timeBeginPeriod | Audio works, FPS still capped at ~75 |
| v5-v6 | Raise fps_target to 1000 + fix timer first | Audio works, ~111 FPS ceiling (vsync still on) |
| v7 (early) | Add vsync disable + short 3-byte patterns | White screen (pattern overmatch corruption) |
| v7 (mid) | Fix patterns to 9-byte context | White screen (idiv timing issue — patches fire too late) |
| v8 | Remove idiv/fps_target patches, keep only jbe NOP + vsync + timer | **Works** — uncapped FPS |
| v7.0 (final) | Clean rewrite with absolute addressing + jbe NOP | **Works** — uncapped FPS, clean code |
