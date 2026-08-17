# Vortex — architecture, findings & plan (Option A)

Status: **OPTION A BUILT AND ENABLED (2026-08-17).** The consolidated
procedural-composite vortex is implemented, compiles clean, passes the
hbtestd crash test, and is **live in the current shipping DLL**. Added the
A/B/C/D hardening fixes on 2026-08-17 (same build): (A) the shared-vtable reveal
is no longer latched off after the first earn (bug: `g_revealArmedVtbl=0`
after race A's reveal killed every later race's diamond), (B) the weasel
texture capture now try-reads `D3DLOCK_READONLY` then falls back to a plain
lock and detects a blank (all-transparent) capture, (C) the composite
canvas is power-of-two rounded and the sprite box is derived from the actual
canvas↔weasel pixel ratio so the centered weasel stays exactly on the anchor
regardless of POT padding, and (D) the composite draw uses a **persistent
reused buffer** instead of `VirtualAlloc`/`VirtualFree` every frame — the
SEH-frame draw path now does zero per-frame allocation (previously up to a
4 MB alloc+zero+free per frame for ~20+ reveal frames). This doc records the
design, the verified D3D8 slots, and the state of the build so the work can
be resumed/verified across sessions.

Date: 2026-08-17 — written after a long investigation session (thread
`1529299247516749886`), following a real-Windows crash discovering the
`0x46C1F1` present-hook wasn't the root cause. Updated to status BUILT after
the Option-A composite-vortex implementation.

---

## TL;DR

The vortex is **built and enabled** in the current shipping DLL. The earlier
raw-`DrawPrimitiveUP` D3D8 version **crashed real Windows** at frame ~57
(`eax=0` inside `d3d8.dll`) and was temporarily `VORTEX_OFF`. RodentRacer then
chose to **rebuild the vortex on the game's own sprite system** (`Sprite_ctor`
+ the proven `ctx+0x37C` swap), **Option A**: generate the vortex image
**procedurally at runtime into an in-memory D3D texture — no PNG file, no disk
write, no encoder, no SEH cave.** The raw-D3D version stayed disabled; the
procedural-composite rebuild is what ships live.

This doc records the verified decompilation that makes Option A safe and
concrete, plus the resulting build state.

---

## 1. Why the vortex was briefly `VORTEX_OFF` (historical — raw-D3D version)

> **Update (2026-08-17): the shipped build has the procedural-composite vortex
> LIVE (`VORTEX_OFF` not defined). The `VORTEX_OFF` disable below applied only
> to the earlier raw-`DrawPrimitiveUP` version, which has been replaced.**

Commit `8d0d0aca` — "OPTION-1 — disable vortex raw-D3D draw (`VORTEX_OFF`)".

**Root cause of the real-Windows crash:** the vortex drew via raw
`DrawPrimitiveUP` on the D3D8 device, dispatching device methods through
hardcoded `vt + D3D_DEV_*` vtable offsets. On real Windows the draw crashed at
frame ~57 with `eax=0` inside `d3d8.dll` — the classic signature of a **mismatched
`__stdcall`/`__thiscall` vtable slot** (one wrong offset passes garbage/zeroed
device pointer or corrupts the stack). Wine tolerates it; real GPUs do not. The
same *class* of bug was hit and fixed before (`9efc2403` — "wrong D3D8
GetRenderState vtable slot"): one wrong slot == crash.

Key principle that governed the whole investigation: **a screen-scoped visual
that must layer over a specific sprite is best done as per-sprite / device-state
changes from the modal screen's OWN update host (which provably renders), NOT a
global post-render hook and NOT hand-rolled raw-D3D with unverified slots.**

---

## 2. Investigation history (what was learned, in order)

1. **`0x455A90` (Graphics_PresentOrEnd) present hook** — crashed real Windows at
   boot (LoadingScreen, RUNTIME 00:00:01, `0001:0000284F`). Cold install of a
   JMP→heap redirect at boot is the vector. Wine tolerates; real Windows doesn't.
2. **`0x46C1F1` (GameUpdate frame epilogue)** — thought to be the fix, then
   found it does NOT run on the modal award screen (zero `present-tick FIRES`),
   so a present-hook overlay can never draw there.
3. **Root-cause reframe (user's insight, verified):** the crash was never the
   host site — it's the **content of the tick** (a raw D3D draw via unverified
   vtable slots). Do not blame / swap the host; diff the handler.
4. **`0x44D760` award-update host (vtable[1] override)** — the ONLY host that
   provably runs during the award screen (it drives the `weasel_mult` frames).
   This is where all reveal logic now runs.
5. **The trophy swap** (`37a2a4ea`, "redirect the game's own renderer"): swap
   the sprite pointer at `ctx+0x37C` (ctx = `*(results+0xC)`) to a diamond
   `Sprite_ctor` sprite; the game's own render fn (`0x44DF70`, draw at
   `0x44E139`) then draws it. **No SEH cave, no raw D3D.** This is the proven
   safe pattern, and it's the basis for the vortex rebuild.

---

## 3. Verified decompilation (the basis for Option A)

### The award render function `0x44DF70` (vtable slot[2] of the award/results object)
- **SEH-wrapped** (prologue `push -1; push 0x4cc828; mov fs:[0],esp`). Any code
  we run inside it is in an SEH frame — avoid interior loops.
- Reads exactly **one** weasel/vortex sprite slot: `ctx+0x37C`, and makes exactly
  **one** `Sprite_DrawRect` call for it at `0x44E139`:
  ```
  44e12c: mov ecx, [eax+0x37C]   ; ctx+0x37C = weasel/medal sprite slot
  44e132: push 0x63
  44e134: push 0x208
  44e139: call 0x42c7c0          ; Sprite_DrawRect(sprite, 0x208, 0x63)
  ```
- Other sprite slots read: `+0x318`, `+0x31c`, `+0x354` (text/other medals).
- **There is NO iterable sprite list** — each slot is drawn by an explicit call.
  (So we cannot "append 20 streak sprites to a list the game draws." That is why
  Option B's independent-20-streaks needs either ONE texture that encodes the
  field, or a risky SEH loop cave. Option A sidesteps this.)

### `Sprite_DrawRect` `0x42c7c0`
- `__thiscall(ecx=sprite, int x, int y)`, `RET 0x8` (3 args: this + x + y).
- Calls `0x45d300(sprite, x, y)`.

### `0x45d300` (the world→screen sprite draw)
- Reads the sprite's own **`+0xC8` (width)** and **`+0xCC` (height)** (set by
  `Sprite_ctor`) and builds the on-screen quad via the game's world→screen
  transform `Gfx_TransformX/Y` (`0x453e90`/`0x453eb0`):
  ```
  screenX = Gfx_TransformX(gfx, x)
  screenY = Gfx_TransformY(gfx, y)
  cornerX = Gfx_TransformX(gfx, x + [sp+0xC8])
  cornerY = Gfx_TransformY(gfx, y + [sp+0xCC])
  ```
- Then applies the sprite material (`+0x8`..) via `Graphics_ApplyMaterialAndDraw`
  (`0x455110`) and a per-frame vertex draw. The actual D3D texture that gets
  sampled is whatever `IDirect3DTexture8` is bound (see below).
- **This confirms:** a `Sprite_ctor` sprite draws a fixed box at a top-left world
  pos, sized by `+0xC8/+0xCC`. It is D3D-agnostic about where the texture came from.

### The texture binding path
- `0x455c50` (texture manager, SEH-wrapped): walks a texture list, calls
  `0x4c760e` per entry, ultimately `0x476770`.
- `0x476770` = **`D3DXCreateTextureFromFileEx`** — the FILE-based texture loader
  (from an earlier memory note). Returns an **`IDirect3DTexture8`**.
- `0x455c50` then `AthenaList_Append` (`0x453780`) the texture into the list.
- **The sprite stores its D3D texture at `+0x50`.** Verified: the sprite sub-ctor
  `0x457fa0` inits material (`+0x8..+0x14`) and blend flags (`+0x4c/+0x4d`) but
  does **NOT** set `+0x50` — `+0x50` is populated ONLY by the texture-loader path
  and is exactly what the renderer consumes.

### Conclusion of the decompilation
The renderer draws whatever **`IDirect3DTexture8`** is inside the sprite's `+0x50`
slot. It does **not** care how that texture was created — file (`0x476770`) or
device-in-memory. **Therefore we can bypass the file entirely and bind our own
in-memory texture.** This is the verified basis of Option A.

---

## 4. Option A design (runtime procedural texture, NO file)

**Goal:** generate the vortex streak image at reveal-time as raw RGBA pixels in a
`malloc`'d buffer, upload it to a D3D texture via `CreateTexture` + `LockRect`
(purely in memory), bind it to a sprite, and let the existing renderer draw it —
exactly like the diamond swap. No PNG, no disk, no encoder, no SEH cave.

### The steps (all proven except the two §6 verifications)
1. **Reach the D3D device** — `app = get_app()` (`0x5341E0`); `gfx = [app+0x178??]`
   → the graphics object; device = `[gfx+0x154]` (confirmed used everywhere, e.g.
   `0x45d40f`: `mov eax,[ecx+0x154]`). NOTE: confirm `app+APP_GFX` → `gfx` →
   `gfx+0x154` is the device (this exact chain the existing sprite/mult code uses).
2. **Procedurally generate RGBA** — fill a small buffer (e.g. 64×64 or 32×32)
   with the vortex streak field: white streaks along radial directions, gradient
   alpha, inward-pointing. Parameters (streak count, length, thickness, alpha
   profile) live in the mod, so the image is data-driven.
3. **`IDirect3DDevice8::CreateTexture`** (vtable slot **39**, offset **0x9C**) —
   create `D3DFMT_A8R8G8B8` texture of our size.
4. **`LockRect`** (texture vtable, offset **0x54**) → `memcpy` the RGBA buffer →
   `UnlockRect`.
5. **Bind** — create/populate a `Sprite` struct (via `Sprite_ctor 0x45d0c0` is
   the easy way, but it pulls the FILE loader; for Option A we instead allocate a
   minimal sprite struct and set `+0x50 = texture`, `+0xC8/+0xCC = box size`,
   material defaults, vtable `0x4d8f84` — matching what `Sprite_ctor` produces).
   *Alternative:* call `Sprite_ctor` then overwrite `+0x50` with our texture.
6. **Swap `ctx+0x37C`** → our vortex sprite (the proven diamond mechanism,
   `37a2a4ea` style, with restore on disarm / object change).
7. **Animate from the award-update host** (`0x44D760` vtable[1] override) — scale
   via `+0xC8/+0xCC`, and fade via the white-fade `diffuse`/`diamond_set_add`
   mechanism over the reveal window. The game's single draw call
   (`0x44E139`) renders it.

### Why this is safe (vs the raw-D3D vortex)
- Draws through **the game's own `Sprite_DrawRect`** → `0x45d300` →
  material/draw path — no hand-rolled `DrawPrimitiveUP`, no unverified device
  slot dispatch for the *draw*.
- Only two D3D device calls used (`CreateTexture`, `LockRect`) — and their slots
  are standard D3D8 (see §6).
- **No code cave inside SEH** `0x44DF70` — we patch a DATA slot (`ctx+0x37C`),
  the render fn stays 100% original (the exact proven pattern).
- **No file/PNG/encoder** at all.

---

## 5. Why NOT Option B (20 independent streak sprites)

- The render fn `0x44DF70` has **no sprite list** to extend — only fixed explicit
  slots. 20 separate moving streaks need 20 `Sprite_DrawRect` calls, which only a
  loop cave inside the SEH render fn could add → the crash class we've avoided
  all along.
- The earlier "Sprite-based" instinct (B) collapses into A once you realize the
  render does ONE call at ONE slot: **encode the whole field into one texture
  (A)**, not 20 sprites (B).

---

## 6. Verified D3D8 slots (resolved 2026-08-17 — from MinGW d3d8.h)

Both open items are RESOLVED by reading the authoritative `d3d8.h`
(`/usr/i686-w64-mingw32/include/d3d8.h`):

1. **D3D8 interface vtable slots (CORRECTED — the doc's original guesses were
   wrong, and this is likely what sank the raw-D3D vortex):**
   - `IDirect3DDevice8::CreateTexture` = **slot 20 / offset `0x50`**
     (the doc guessed slot 39 / `0x9C`; `0x9C` is actually `MultiplyTransform`)
   - `IDirect3DTexture8::LockRect` = **slot 16 / offset `0x40`**
   - `IDirect3DTexture8::UnlockRect` = **slot 17 / offset `0x44`**
   - `IDirect3DTexture8::GetLevelDesc` = **slot 14 / offset `0x38`**
2. **Device pointer chain** — confirmed: `app(+0x174)→gfx→gfx+0x154` is the
   `IDirect3DDevice8*` (used everywhere, e.g. `0x45d40f`). The sprite stores its
   own `gfx` at `sprite+4`, and `Graphics_ApplyMaterialAndDraw` reads
   `+0x154` → device.

Resolution of the bind question: the build allocates a **minimal sprite**
(0xD4 buffer) with vtable `0x4D8F84`, `+0x50` = our texture, `+0xC8/+0xCC` =
the weasel's original draw box, and material defaults — then swaps it into
`ctx+0x37C`. This avoids the `Sprite_ctor` FILE loader entirely and is purely a
data write, matching the proven diamond-swap pattern.

---

## 7. Current build state (this session's shipping DLL)

**Shipped (2026-08-17):** `bass.dll` from commit `aea9b030` ("2x canvas vortex +
medal-anchor shift") — the **procedural-composite vortex is LIVE** in this
build (`diamond_vortex_tick` is called from the reveal driver; `VORTEX_OFF` is
**not** defined). The raw-`DrawPrimitiveUP` vortex version (which crashed real
Windows) is gone; the composite draws through the game's own sprite renderer,
so it is safe and is what ships.

Normal build flags (already-defined feature set — the current shipped build):
```
-DDIAMOND_VTABLE_OVERRIDE -DDIAMOND_TT_WRAPPER
```
Add `-DVORTEX_OFF` only to compile the composite vortex out (white-fade +
diamond swap stay enabled, no vortex/white-lerp).

Earlier probe builds (historical, superseded): `diamond_weasel_bass_c6f1probe.dll`
(commit `75f6b0b2`, the `DIAMOND_C6F1_PROBE` probe) and the `DIAMOND_CB90_PROBE`
build had no vortex.

Vortex state/constants (valid for the current live build):
```
VORTEX_MAX      20   streaks at a time
VORTEX_FRAMES   100  active cycle (result-frames)
VORTEX_TAIL     30   no-new-spawn tail, streaks fade
VORTEX_STRETCH  22.0 streak length (px)
VORTEX_SEGS     8    subdivisions along streak (alpha gradient)
VORTEX_CENTER_FADE 12.0 alpha-fade as inner tip nears center
Trophy world pos: (0x208, 0x63); center via sprite +0xC8/+0xCC; Gfx_TransformX/Y
  replicated in C (vortex_compute_center) — no inline-asm FPU.
Vortex tick: `diamond_vortex_tick(results)`, called from the reveal driver
  behind `#ifndef VORTEX_OFF`. The Option-A composite (runtime procedural
  texture + `ctx+0x37C` swap) is implemented and live in the shipped build.
```

---

## 8. The winning principles from this saga (write these down)

1. **Never install a mod JMP→heap redirect at boot** — real Windows crashes the
   LoadingScreen regardless of the tick's early-return gate. Wine tolerates it.
2. **Diff the handler content, don't blame the host site.** The `0x46C1F1`
   present host is a legitimate NON-SEH function; what crashed was the raw-D3D
   tick moved onto it.
3. **The modal award screen is driven by its own vtable[1]/[2]** (`0x44D760`
   update, `0x44DF70` draw). Global GameUpdate/present boundaries do NOT fire on
   it. Put per-frame effect work on `0x44D760`, and draw via the game's own
   renderer (data swap), not a present hook.
4. **A modal rendering is composited in the RENDER fn.** Drawing from the UPDATE
   host does not show. Swap a data slot (`ctx+0x37C`) the render fn reads.
5. **Avoid interior-of-SEH mod-C caves.** The winning pattern is always a DATA
   change the SEH-wrapped code already reads, or a harmless non-SEH fn hook — not
   running mod logic inside a `fs:[0]` frame.
6. **Verify D3D8 vtable slots against the interface before every use.** One wrong
   slot == a real-Windows-only crash Wine can't catch.
7. **Procedural D3D texture from memory (`CreateTexture`+`LockRect`) is the way
   to draw a runtime-generated image with ZERO png/file/disk/size.** The renderer
   only needs a valid `IDirect3DTexture8`.