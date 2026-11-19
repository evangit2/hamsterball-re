# LevelFeatures_Loader — universal cross-level objects for Hamsterball

A `bass.dll` proxy mod. Replaces all 15 per-level board constructors **and**
4 vtable slots with universal handlers, so any level's objects (bridges,
tippers, bumpers, windmills, …) work on any other level. Fully S1-driven:
drop a `.MESHWORLD` into any slot and its `N:`/`E:` refs bring their own
meshes and behaviors — no config editing.

Game version: V3.6c. Load mechanism: `bass.dll` proxy (rename original to
`bass_real.dll`).

## How it works

**Universal constructor.** A 6-byte JMP detour on the scene-loader call
(`CALL [EDX+0x48]` at `0x4273E0`) routes every level through
`UniversalConstructor`, which runs the same 4 steps every native loader
runs: `Level_MeshWorldCtor` → `Level_RenderCtor` → `Level_InitScene` →
per-level tail replication (Tower pennants, Neon glow lights, Up VAC tubes,
Sky magnifier) → `UniversalPostSetup` → `Board_Setup` (`vtable+0x80`).

**Per-board extension heap (Option B).** Boards stay vanilla-sized. All
mod state lives in a `0xC000` heap block per board (`g_extMap`, no board
mirrors). `ScanS1AndAutoEnable` reads the loaded file's S1 refs after
`MeshWorldCtor` and sets per-board `FEAT_*` flags, so file-swapped levels
just work.

**Vtable patching** (`InstallVtablePatches`, ~1s after load, before first
level):

| Slot | Offset | Handler | Strategy |
|------|--------|---------|----------|
| 1 | +0x04 | `UniversalBoardUpdate` | Calls shared `Scene_Update` (0x419C00) first, then feature blocks |
| 19 | +0x4C | `UniversalRaceState` | Calls the saved original **or** the shared base exactly once, then feature blocks (Beginner skipped — `Feature_BumperDecay` owns it; Sky skipped when ext feat active) |
| 24 | +0x60 | `UniversalRender` (RET-4 thunk) | Only the 6 wrapper levels {2,5,10,12,13,14}; the other 9 keep native shared render (all 15 natives are `(board,gfx)` RET 4, verified by disasm) |
| 29 | +0x74 | `UniversalDispatchCollision` | Reimplements all 62 collision events inline (gated on per-board flags built once at load), then calls global `DispatchCollisionEvents` |
| 33 | +0x84 | `UniversalCreateDynamicObjects` | Matches S1 names, calls entity ctors directly (no delegation — originals only know their own level's names) |

Sub-meshes lazy-load via `S1EnsureMeshWorld` inside CreateDynamicObjects —
no static preload. `Tournament_AdvanceRace` (0x427080) is hooked with an
ECX-preserving wrapper to free the outgoing board's ext block.

## Feature blocks

| Feature | Levels | Source | Description |
|---------|--------|--------|-------------|
| `FEAT_BRIDGE_ANIM` | 3, 14 | Intermediate (0x41CC90) | 4-state bridge tilt machine |
| `FEAT_SWIRL` | 4, 14 | Dizzy (0x41D510), Master (0x420DA0) | Swirl zones + tar bubbles |
| `FEAT_WINDMILL` | 5 | Tower (0x41E760) | Windmill rotation + speed states |
| `FEAT_BADBALL` | 9 | Odd (0x41EE80) | Timer-based BadBall spawner (dormant-faithful: native flag is pinned 0 in vanilla) |
| `FEAT_BUMPER_DECAY` | 2, 10, 14 | Beginner/Toob/Master | Bumper "lit" float decay |
| `FEAT_NEON_CAM` | 7 | Neon (0x424790) | Ball-following glow lights (S3-gap gated, see below) |
| `FEAT_SKY_POPCYL` | 13 | Sky (0x41FC90) | Random popcylinder activation |

## Per-name pennants

Name a flag mesh `FLAG*` for a rectangle, `PENNANT*` for a triangle — on
any level, in any slot (`src/lf_09_pennants.c`). The game natively runs
every flag through one shared waver (all-rectangle, or all-pennant on
Tower); the mod builds a second pennant waver and routes each flag by name
at render time. Check `lfdebug.log` for `Pennant:` lines to confirm pickup.

## Neon glow gate

Neon follower lights build **only** when the level MESHWORLD has no S3
light of its own (`src/lf_10_neon.c`). Stock Neon (`LevelDark`, S3=0) gets
followers; every other stock race keeps its native lights. Custom authors
opt out by adding any S3 light.

## RaceFiles.txt

Maps `Race 1`..`Race 15` to a file — swap any race to any `.MESHWORLD`
without renaming files:

```ini
Race 1: Level1          # WarmUp slot loads levels\level1
Race 4: Level3          # Dizzy file in the WarmUp slot (always unlocked in Time Trials)
```

Bare names auto-prepend `levels\`; `.MESHWORLD` extension auto-stripped.
Re-read on every level load. This is the **only** config file —
`LevelFeatures.txt` (v11) and `LevelData.txt` (v10) are deprecated and no
longer read; S1 `N:`/`E:` refs are the source of truth.

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy `bass.dll` and `RaceFiles.txt` from the zip into the game folder

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll LevelFeatures.c \
  bass.def -lwinmm -static-libgcc -Wl,--enable-stdcall-fixup -O2
```

Source lives in `src/` as 11 chapters (one translation unit —
`LevelFeatures.c` only `#include`s them, output is still one `bass.dll`):

| Chapter | Contents |
|---|---|
| `lf_00_core.c` | RVAs, offsets, ext heap, globals, string/level/racefile helpers |
| `lf_01_bumpers.c` | Bumper physics constants, game fn pointers, mesh path table |
| `lf_02_ctor.c` | UniversalBoardCtorLogic + install thunks |
| `lf_03_features.c` | InitBridge + Feature blocks (bridge/swirl/windmill/…) |
| `lf_04_render_update.c` | Render + BoardUpdate + RaceState |
| `lf_05_create.c` | S1Ensure + UniversalCreateDynamicObjects |
| `lf_06_collision.c` | Collision flags + UniversalDispatchCollision |
| `lf_07_postscan.c` | PostSetup + S1/collision scans |
| `lf_08_hooksmain.c` | Constructor + hooks + DebugLog + vtable + DllMain |
| `lf_09_pennants.c` | Per-name pennant routing (`FLAG*`=rect, `PENNANT*`=triangle) |
| `lf_10_neon.c` | Neon glow follower lights, S3-gap gated |

## Files

- `bass.dll` — compiled mod
- `RaceFiles.txt` — slot→file map (the only config file)
- `LevelFeatures.txt` — **deprecated**, no longer read (kept for reference)
- `LevelFeatures.xml` — reference catalog of injectable objects (docs only)
- `bass.def` — export definitions for the bass.dll proxy
- `LevelFeatures_Loader.zip` — release artifact (`bass.dll` + `README.md`)

## History

- **v13.30** — Master bonk/halves/pops re-wired (user report: bonk + pops
invisible, halves shatter early — his DLL still had the pre-v13.22 wiring).
Root causes, all objdump-verified: (1) Pops built with the Sky ctor
(0x363f0); Master native uses PopCylinder_ctor (0x36ee0) — content-14 now
builds the Master class (0x10E8) with native-mesh preference (board+0x5420,
file fallback) + both native lists (0x2578 and 0x5428, own FEAT_POP_LISTS
init bit). Sky path untouched. (2) Halves never saw the native break:
v13.22 dropped the board+0x5418/0x541C mirrors, so the BONKPOPUP path was
dead and the proximity-to-Bonk fallback fired early — mirrors restored
(native-mesh preference 0x5410/0x5414 + file fallback), proximity retired,
BONKPOPUP touch now calls native 0x436e70 on both halves (args 0/1, exact
Master-0x412f80 parity; ext slots as fallback). (3) Bonk hammer dead:
board+0x540C store restored (native 0x4123C7 parity, guarded). Scoped
restore only — glue/catapult 0x6080/0x584C wiring stays out per v13.22.
Rig: boot crash-test clean; Master gameplay confirm needed on Windows.
- **v13.29** — We run the drive, native stands down on foreign files. Slot-native
racestate now runs ONLY for the slot's own file (content == level); anything
else gets shared base + our universal pump (every 0x2578 member via live
slot-11, sole driver, 1x). Fixes the file-swap hazard (slot-native walks and
touches objects a foreign file doesn't have) and the 2x tippers (pump + walk
are now mutually exclusive by construction). Walk detector also catches
customs that walk via calling the base (Up etc.). Rig: swapped Master file in
Up slot and slot 3 both run base + skip with dynamics cycling 1x, no crash.
- **v13.28** — Tippers back to 1x (were 2x on Master slot). Root cause: the
native racestate (base 0x41B130) already walks board+0x2578 calling every
member's slot-11 each frame, so native drives all dynamics 1x wherever that
walk runs. Our v13.27 pump added a second drive on top. Fix: before pumping,
scan the exact racestate fn UniversalRaceState will call (same level/content
chain) for the walk's byte shape (lea 0x2578) and stand down when present;
no-walk slots (Intermediate custom) keep the pump as sole driver. Gated on
which code runs, never on slot — cached per board. Rig: no-walk slot pumps
1 call/tick, no dupes; walk-slot skip verified by construction (vtable slot
19 + walk bytes + dispatch chain all proven).
- **v13.27** — Master dynamics (bonk, halves, pops) get their native per-frame
drive back. On universal boards nothing calls object slot-11 updates, so every
native dynamic froze after creation. Fix: the tipper scan now also pumps
native v[11] for pop (0x4D58F0), bonk (0x4D5120) and halves (live vtable)
objects — same __thiscall ECX-only shape native uses, no per-class RE.
Two dead gates removed along the way: board+0x3A44 is always 0 (Setup zeroes
it, single writer) so it killed the whole driver, and board+0x2578 gets an
explicit AthenaList_Init (same missing-init class as 0x5C64/0x436C/0x4378).
Rig: 5+ min stable, no crash; motion is ball-proximity-gated so the parked
rig ball can't confirm it — needs live play to verify bob/fall/pop.
- **v13.26** — Windmill feat armed only by actual windmills. The postscan
grouped TURRET/CATAPULT/MACE/DRAWBRIDGE/TRAPDOOR with WINDMILL/CHOMPER (legacy
Tower-family lump), so Catapult levels entered the windmill feature — the
v13.25 propeller. Trimmed to WINDMILL + CHOMPER (the only objects the
function reads). Load-path safety moved to BoardMeshOrZero keyed on the
native-ctor flag (universal boards never ran the preload → ext-load; native
boards keep trusting slots). Rig: Tower windmill spinning (angle 3234),
Master windmill angle 0.0, deck cycling, feat bit clean.
- **v13.25** — Constant-velocity deck propeller + swoop on Master file
root-caused to the windmill fall-through, not the deck code. Postscan arms
FEAT_WINDMILL for CATAPULT/DRAWBRIDGE/TRAPDOOR/MACE/TURRET/CHOMPER S1s, so
Master (Catapult S1) enters Feature_Windmill; content != 5 skips the native
path but the fall-through posed REND_TOWER_WINDMILL with a BONK_STORE
fallback — and BONK_STORE holds the tilting deck mesh. Unbounded angle at
constant velocity + 45-mod-90 swoop on the deck = the reported propeller.
Fix: fallback removed, early return when no real windmill render (chomper
tail keeps its own gate). Rig proof on Master file: windmill angle frozen
0.0, deck still cycling, live timer.
- **v13.24** — Static deck in Master slot root-caused to a one-field data bug,
not a code gate. The Master row of g_levelData set bridgeParamOffset =
UNI_BRIDGE_COUNTER instead of UNI_BRIDGE_ANGLE, so ctor step 9f wrote
(45.0, 0, 50) at ext+0x863C/40/44 instead of +0x8634/38/3C. The create-mirror
init saw counter = 45.0f-bits (1110682897, not 0) and skipped, leaving
angle/state/counter = 0/0/1110682897 — state 0 needs 1.1B decrements, so the
deck sat static essentially forever. Every other slot uses its own row
(Intermediate's correctly says UNI_BRIDGE_ANGLE), which is exactly why only
the Master slot froze. One-field fix; rig proof on Intermediate-in-slot-14
(ext angle/state/counter now live, machine cycling) and normal Master
(angle 18.5, state 3/rising, live timer, CD4=32).
- **v13.23** — Windmill deck root-caused to a wrong rotation axis. Native
Intermediate deck update poses via 0x457CC0 with the dword at timer+0xC0 = 1
(0x41CFCA; the rider-carry call at 0x41CF20 uses 0). TimerInit zeroes it, so
our shared pose ran with the carry axis — deck spinning instead of pitching.
One-line fix: set timer+0xC0 = 1 before GfxScaleZ (fn/address/args already
matched native: TimerInit = 0x457AD0, ScaleZ = 0x457CC0, SetPosition =
0x457B50). Same fix applies to Intermediate automatically (shared feature).
Rig: Master loads, live timer runs, 60s+, CD4=32. Needs your eyes to confirm
the tilt (rig races stay frozen, can't render motion).
- **v13.22** — All Master-specific code removed per user direction. Master now
loads everything through the exact same generic branches as every other level:
deleted the BBRIDGE native-slot replication (0x5410/0x5414 mesh prefer +
0x5418/0x541C obj mirror), bonk board+0x540C store, pop 0x5420-mesh/0x5428-list,
gluebie 0x6080-list, catapult 0x1100/0x5848/0x584C, the 0x436E70 break call,
the exe tail NOP, the 9a-extra5 native-list inits, and the tarbubble native
feed + native dummy. Halves/bonk/break are now fully generic: file meshes
into ext, objs in ext slots (UNI_BBRIDGE1/2_OBJ, UNI_BONK_SAVE), presence-gated
(S1P_BBRIDGE from the S1 walk, no slot/content checks), break done with inline
field writes + slam sound. Pops/glue/catapult/deck/bonk are the same
name-matched branches every level uses (no content gates anywhere in create).
Rig: Master loads, live timer runs, 60s+, CD4=32 identical composition
(12 pops, halves, bonk, catapult, 7 gluebies). Two deliberate non-removals:
the bumper render/decay `level == 14` ternaries are per-race DATA (4 vs 8,
like mesh paths) inside shared features, not Master code paths.
- **v13.21** — Four Master regressions fixed (user report), all root-caused
to native state our universal build stopped providing: (1) Popcylinders:
our mirror skipped numberless POPCYLINDER0 (atol=0→idx=-1, the 4 missing)
while native matches the bare prefix — default variant now; mesh prefers
native board+0x5420, file fallback; obj also appended to the native 2nd
list board+0x5428 (its walkers drive the specials). (2) Gluebies inert:
native Master create appends board+0x6080 (proximity list its walkers
scan) + 0x2578 — we only appended 0x2578 (+0x4378 Dizzy-only). Now
appending 0x6080 always. (3) Catapult never fires: missing obj+0x1100=1
armed flag + board+0x584C 2nd-list membership + board+0x5848 mesh mirror
(all in native create 0x4126D8). Plus dynamic fling power per user ask:
CATAPULT<n> S1 suffix = percent of native 50 (CATAPULT150 = 75), applied
after Launch (which resets 0x10F4); no suffix = native behavior.
(4) Bridge never breaks: bonk obj now stored at board+0x540C (native
create 0x4123C7 parity) + ext mirror (board slot observed cleared
post-create; clearer unidentified, ext copy feeds the feature); new
Feature_BridgeBreak replicates native 0x436E70 on both halves (+0x10FC=1,
+0x10F8=∓5, +0x10E8=(±5,20,0)) + slam sound when a ball touches the bonk.
Also 9a-extra5 now inits +0x5C64 (tarbubble consumer was garbage → 53
wild appends). Rig: Master loads + 60s+, CD4=32 (all 12 pops), live race
timer runs. Deck circles: unchanged this build — still needs your
log+screenshot (BRFEAT lines? pivot?) to separate "feature mis-posed"
from "unlisted render".
- **v13.20** — Master-specific code disabled per user direction (only the
three keepers remain: popcylinder-special, cannon/Catapult, BBRIDGE — all
shared paths, untouched). Removed: (1) native Master ctor fast path
(0x4206D0) — Master now builds via g_BoardCtor + S1 mirrors like the
shared-scene races, so no native Master board code runs at all (bridge
pair, tipper pair, BreakBridge states never built); UNI_NATIVE_CTOR stays
0 so create/collide/racestate/render never delegate to native origs.
(2) Swirl mesh-rotation Master skips — uniform path (NULL-guarded no-op
where no swirl meshes). (3) N:BRIDGE collision Master exclusion — shared
deck collision now applies on Master. Kept deliberately: tail NOP (the
native Scene_Load tail still runs and would list empty slots), tarbubble
dynamic block (auto-adapts: ext list fed for manual update, no native
consumer found on universal boards), bumper physics/count data (Master
physically has 4 bumpers + own velocities — data, not code paths).
Rig: universal-built Master loads + 60s+ stable, Intermediate loads;
BBRIDGE halves use file meshes (native slots NULL at create, as on the
user's build). New TAILNOP applied/skipped lines in lfdebug.log prove
the exe patch state on any machine.
- **v13.19** — Deck invisible root-caused (user: tilting deck absent, halves
fine; diag log proved our deck path runs: inventory sees BRIDGE, mirror
runs, pivot nonzero, render built, feature posing). Two-part fix: (1) the
scene dispatcher (0x40C4A0) skips out2 registration when out1 is NULL —
v13.17's out1 = 0 unlisted the deck render. Reverted to out1 = mesh.
(Intermediate never complained because the same mechanism always worked
there.) (2) The native bridge tail (0x412158-0x4121BA) lists the ctor
2PBridge pair (never given the S1 position since native BRIDGE create is
bypassed) — that unpositioned native pair is the origin ghost (sounds +
shake at 0,0,0). NOP its 4 appends, verify-or-skip on the E8 opcode,
idempotent, native app+0x234 gate keeps it scoped. Natives now fully out
of the scene; deck = our Intermediate-colored build, posed + sounded at
the S1 pivot. Rig: Master 60s+ stable, boot clean.
- **v13.18** — Absent bridge + sounds-at-origin fixed (user: bridge gone but
audible at the origin). Two defects, both Master-only consequences of
universal code: (1) Master has BBRIDGE1/2 S1s but no plain BRIDGE S1, so
the deck mirror never ran — yet BBRIDGE presence armed FEAT_BRIDGE_ANIM
(CORRECTED v13.19: Master DOES have a BRIDGE S1 — the deck mirror runs
fine per the diag log; the real defect was v13.17's out1 = 0),
and the feature posed the SAWBRIDGE-shared Level5-Bridge mesh at the zero
pivot with bridge sounds. Fix: FEAT arms on deck S1 (K_BRIDGE) only, never
BBRIDGE; inventory sets no FEATs (mirrors + postscan own them). (2) Our
BBRIDGE halves ignored the native drive slots. Fix: mesh from native
board+0x5410/0x5414 when valid (file fallback), obj mirrored to
board+0x5418/0x541C (native create stores EDI there; the drive reads those
slots), append stays native board+0x2578. Memory-proven on rig: native
meshes live, our objs in 0x5418/0x541C, Master 60s+/87s+ stable.
- **v13.17** — Drawbridge origin-spinner root-caused to OUR out1 (user: no
natives at all). Native Master create returns out1 = board+0x436C which
the ctor leaves NULL (xor-zeroed, never written) — the scene never gets a
raw mesh. The v13.15 mirror returned the Level2-Bridge mesh as out1 and
the scene drew it unposed at the origin. Fix: mirror out1 = 0 (exact
native parity, NULL-tolerated — vanilla does this every Master load).
Audit findings: 0x436C/0x4370 = drawbridge pair, 0x4374/0x4378 =
Level10-2PBridge render+controller, 0x4394/0x4398 = Level3-Tipper
render+controller. Native create builds fresh per-S1 tippers/BBRIDGEs
(bypassed by our mirrors) and returns the ctor drawbridge pair for BRIDGE
(bypassed). With the bypass + out1=0, ctor natives are inert (update has
zero slot refs, teardown NULL-safe) — functionally no natives. Rig:
Master 60s+ stable, native slots intact, boot clean.
- **v13.16** — Drawbridge-at-origin fixed (user: bridge sat at world origin
spinning). Root cause: the drawbridge mirror wrote S1 xyz floats into
board+0x4374/0x4378, but Master's ctor builds live heap objects there
(0x420870/0x4208A6) — the write orphaned the natives and fed float-bits
to the native scene tail as pointers (junk rendered at the origin). Fix:
NativeOwnsBoardFields() object-presence test — mirrors and the per-frame
bridge board-mirror skip board+0x4374-0x4388 writes wherever live objects
sit there. Intermediate unchanged (its slots are NULL → writes happen).
Memory-proven on rig: Master board+0x4374/0x4378 hold live objects with
the fix. Master 100s+ stable, no crash/duplicates; Intermediate loads.
- **v13.15** — Master tippers/drawbridge owned by LFL mirrors (user: our
code takes care of everything on Master, no palette matching). Tipper,
drawbridge and BBRIDGE pieces now bypass native delegation and run the
universal mirrors on every race carrying those S1s (Intermediate colors
on Master by design). Routing is presence-based: one S1Inventory walk of
the loaded object database at load records tipper/bridge kinds; delegate
bypass, mirrors, features and postscan flags key off the kind enum and
presence mask — no slot IDs, no per-name dispatch (single S1KindFor
codec; S6 hook reserved, S1 covers every ref). Safe because mirrors build
real native-class objects (native ctors) and native 0x4374/0x4378 slots
are ctor-set, not create-set. Rig-verified: Master runs 100s+ with LFL
tippers/drawbridge, no crash/duplicates; Intermediate + Dizzy clean.
- **v13.14** — Dead content forks removed (same user request): the swirl/
tar ext-list ternaries (==14 ? _M : base) were no-ops — both aliases are
#defined to the same offsets — so they're now single defines. Sizing was
already dynamic: AthenaList_Append grows per entry (1 ref or 100), the
only cap is the 8192 sanity bound. NATIVE lists (Dizzy 0x4790 / Master
0x5C64) stay exe-side and cannot merge without patching native reads;
the v13.13 detector covers both. One real fork left, untouched by design:
Feature_SwirlZones reads Dizzy zones from native board+0x4378
(isDizzy) vs ext elsewhere — unifying that changes Dizzy swirl sourcing.
- **v13.13** — TarBubble fix gone universal (user: no per-race gates).
Replaced the content-4/content-14 scan blocks with one gate-free block:
it scans the board's own saved native update for a direct call to
FUN_0044fa90 and takes the list offset from the nearest disp32 LEA above
it (exe audit: only Dizzy 0x4790 + Master 0x5C64 call it; all others get
no append). Ext feed follows the UNI_NATIVE_CTOR capability flag, not a
race ID. Rig-verified: Master 73s+ gameplay, Dizzy loads+runs, boot clean.
- **v13.12** — Master open-crash FIXED (0001:0004EAFF a few frames after
load). Root cause: native Master update (0x420DA0) calls FUN_0044fa90 on
board+0x5C64 whenever RNG(0,0x14)==10; native Scene_Load (0x411F60)
populates that list by scanning S1 TarBubble* refs, which LFL skipped
(native create never appends there — its 6 appends go to 0x2578/0x5428).
Replicated the scan post-Board_Setup (content-14 only) + dummy entry for
TarBubble-less files (Dizzy-0x11E4 precedent). Rig-verified: Master loads,
runs 2+ min to goal stars, Warm-Up clean, boot clean.
- **v13.11** — Pose timer buffer zeroed (unzeroed buffer crashed Intermediate
on open); state-transition lines + heartbeat in log.
- **v13.10** — Dizzy wheel + swirl carries are manual pure-C (measured ScaleY/
ScaleX matrices; accumulated angle for swirl). Tippers/gluebies still open.
- **v13.9** — Bridge carry is manual pure-C Rx(-0.5°/frame) + far-unmark
(diag BRIDGECARRY/BRIDGEAFTER lines in; boot probe OUT). PARTIALLY FIXED.
- **v13.8** — Superseded by v13.9 (native ScaleZ scrambles from our frame).
- **v13.7** — Bridge rider mark passed (y, z, 0.5), native passes (x, y, z):
wrong balls carried (teleports/stuck). Fixed to full pivot.
- **v13.6** — Bridge posed every frame (was rise-only: snap raised→flat,
invisible fall+pause). Carry verified native-exact (-0.5f push present in
binary). Rebuild with bass.def (a bad build dropped all BASS forwards).
- **v13.5** — Intermediate bridge: return (mesh, render) like native
(Scene_CreateDynamicObjects 0x40C430 drops out2 when out1 NULL — the
absent bridge); S1 pivot + 45/0/50 angle/state/counter mirrored to
board+0x4374/80; ext angle init 45 (was 0, started flat).
- **v13.4** — Bumper flash reads OUR postscan mesh lists (ext
UNI_EHVECTOR+i*0x418), not the native board+0x436C slots the generic
replica ctor never fills (that was the missing glow: chain ran, slots
empty, gate skipped every frame). Same meshes/stride/vtbl call.
- **v13.3** — Bumper flash render rewritten to native-exact
(Scene_RenderReflectiveObjects7 0x411380): per-bumper slots, ctx 0x60 bytes
(was 64B stack smash vs 78 native), native field layout (lit +0x10/+0x20,
byte +0x4C), ctx pointer on GFX+0x7C0 (was board+0x7C0, never read);
empty slots skipped (swap-slot safe). Fixed Beginner 4-6 Draw crash.
- **v13.2** — Expert/Tower/Wobbly slot-1 stabilization: JUDGE outs (NULL,
NULL) + 0x4BBC board link (game-side JUDGE consumer faults on non-null
out1); zeroed hammyjudge write removed (untested, not needed); Expert fans
+ board mirrors deferred (`g_enableExpertFans/Mirrors = 0`, full-set
interaction still open); Tower MACE Tower-ctor + ext trust-gate + YellowLink
presence + AthenaLists presence; Wobbly exact-native per-wNum constants;
BRIDGEn digit-routed to spinner path; all diag probes/audits removed
- **v13.1** — Neon scene-loader tail (Step 6c) with S3-gap gate
- **v13** — Option B: per-board ext heap, vanilla-sized boards, S1 feat scan
- **v12.3** — Tower/Sky render split
- **v12** — Level-folder sub-mesh scan
- **v11** — S1-driven collisions, `LevelFeatures.txt` deprecated
- **v9** — `RaceFiles.txt` swappable races
- **v8** — TURRET Timer-vtable fix, dedicated mesh slots

## Testing notes

- Crash test = launch on Wine/Xvfb, survive 35s past title (catches bad
hooks/stack corruption). Wine renders black on llvmpipe and DirectInput
ignores synthetic keys, so level-load paths need real Windows testing.
- Debug log: `lfdebug.log` next to the config (INIT line lists paths).
