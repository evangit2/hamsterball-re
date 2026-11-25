# Level-Gate Removal TODO

Goal: no `if (level == X)` / `ContentFileId == X` behavior gates. Every
difference moves into the MESHWORLD file (S1 presence / S1 params) or a
load-time capability probe whose result lives in ext. Handlers read ext,
never level numbers.

Guards (`level == 0`, range checks) stay — they are not hardcoding.

## 1. Race-state update dispatch (DONE 2026-11-25)
- Universal cut: EVERY board runs base + feature blocks. Slot-19 audit
  (exe .rdata): 10 of 15 levels already ran base 0x41B130 natively.
- Customs absorbed: Beginner 0x420240 decayed dead +0x642C timers (mod
  decays ext UNI_BUMPER_LIT instead); Toob 0x41F7E0 zoom clamp dropped
  (user to verify intro visuals); Up 0x420660 walked always-empty +0x436C
  calling deleting dtor v[0] (landmine, never a driver — lifters live in
  0x2578 via v[11] walk); Neon 0x424790 double-positioned followers the
  mod already drives; Sky 0x41FC90 never ran (universal path).
- Dead code noted: UNI_NATIVE_HANDLERS fast path (nativeCtor always NULL,
  both assignments if(0)) — left in place, never fires.
- REMAINING: none. Both dispatch sites + pump mirror collapsed.
- `gm_03_features.c:263-274` (NativeWalkPresent scan fn)
- `gm_04_render_update.c:1081-1110` (UniversalRaceState proper)
- Rows: Beginner(2)->base | Neon(7)->native-if-followers-built |
  Toob(10)->base | Sky(13)->base | Tower(5)->base |
  else native-iff-content==level else base.
- CUT 2026-11-25: Tower row -> Tower-vtable check -> base (orig identical
  to base, board-type not slot number). Sky row -> Sky-vtable check only
  (dropped level/content halves). Neon row -> Neon_IsActive ext flag only
  (native 0x424790 call deleted; followers already driven mod-side by
  Feature_NeonCamera, same offsets — native call was double-positioning).
- REMAINING: Beginner row, Toob row, generic content==level row (audit
  each native orig's effects, absorb into feature blocks, then cut).

## 2. Beginner/Toob/Master color tables
- `gm_04_render_update.c:224-226` (render + ContentFileId forms)
- Replace with: S4/mat params or per-file palette marker in MESHWORLD.
- Status: TODO

## 3. Toob(10) native-ctor short-circuits (5 sites, one pattern)
- `gm_04:181,227` `gm_07:518` `gm_06:390` `gm_08:216`
- Replace with: single `UNI_NATIVE_CTOR` ext flag (already exists —
  just delete the ContentFileId half of each condition).
- Status: TODO (easy)

## 4. Sky(13) tails
- `gm_04:208` `gm_08:204`
- Replace with: S1-presence (PILLAR/CLOUD in file?) — SkySceneTails
  already walks scene state; gate on what it finds, not slot 13.
- Status: TODO

## 5. Odd(9) BadBall table variant
- `gm_05_create.c:1072` (isOdd)
- Replace with: file contains LAUNCH01-03 S1s (S1 census already counts
  them — gate on the count, not the slot).
- Status: TODO

## 6. Pin behavior: Master(14) vs BonkCyl
- `gm_05_create.c:1387`
- Replace with: mesh param on the pin S1 (BONK vs TOOB variant tag in
  MESHWORLD) or presence of the BonkCyl parent object.
- Status: TODO

## 7. Sky(13) trapdoor-as-popdoor
- `gm_05_create.c:1456`
- Replace with: DAT flag on the S1 ref (door behavior tag), read at spawn.
- Status: TODO

## 8. Master(14) velocity constants
- `gm_06_collision.c:460-461` (5.0/12.0 vs 4.0/10.0)
- Replace with: `N:VELOCITY(max,scale)`-style S1 params, defaults = base
  values. Any level can then tune without code.
- Status: TODO

## 9. Impossible(15) kill-floor exception (x2)
- `gm_06_collision.c:703,710`
- Replace with: kill-floor S1/DAT opt-out tag (MESHWORLD declares its own
  kill planes; absence = no kill).
- Status: TODO

## 10. Odd(9) tournament difficulty check
- `gm_06_collision.c:1116`
- Replace with: DAT param on the relevant S1 (difficulty threshold travels
  with the object, not the slot).
- Status: TODO

## 11. Dizzy(4) flag
- `gm_03_features.c:447` (isDizzy)
- Replace with: S1 presence that the Dizzy path actually needs
  (WHEELEMBED/JUMPPIPE S1s — gate on them directly).
- Status: TODO

## 12. Bumper decay on Beginner/Toob/Master
- `gm_03_features.c:1662-1664`
- Replace with: decay params on bumper S1s (per-bumper rate in MESHWORLD),
  defaults = current base values.
- Status: TODO

## 13. Odd(9) postscan block
- `gm_07_postscan.c:246`
- Replace with: same S1-presence the block scans for (LAUNCH table build
  runs when LAUNCH S1s exist).
- Status: TODO

## 14. Glass(12) render path
- `gm_04_render_update.c:152`
- Replace with: material/render flags from the file (glass S4 mats?),
  not slot 12.
- Status: TODO
