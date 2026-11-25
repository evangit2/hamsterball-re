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

## 2. Beginner/Toob/Master color tables (DONE 2026-11-25)
- The gate was the bumper reflective render (`gm_04:224`), not a color
  table — mislabeled in the original list.
- File audit: bumper S1 refs exist ONLY in LevelCascade (Beginner, 6) and
  Level8 (Toob, 13). No other stock file has them, so the presence flag
  (COLL_FLAG_BUMPER from N:BUMPER S1s, set in gm_07_postscan:449) is the
  complete gate. The level==2/10/14 list was redundant; Master(14) never
  even reached the block (no bumper S1s in its file, flag stays 0).
- Cut to `if (features & REND_BUMPER)`. Comments updated.
- REMAINING: none.

## 3. Toob(10) native-ctor short-circuits (DONE 2026-11-25)
- All 5 sites gated on UNI_NATIVE_CTOR, which is always 0 (both
  nativeCtor assignments are if(0) — every board is replica-built).
  Deleted: gm_04:227 native-slots render branch, gm_06:390 unnamed-touch
  fixup, gm_08:216 84-line saw-mirror block, gm_04:181 + gm_07:518
  second-half conditions. Plus gm_03 decay level==2/10/14 fallback->8
  (COUNT from the collect is always valid when decay runs).
- File proof: Toob's N:BUMPER1-8 live in the file bytes, harvested by
  the file-scan like everyone else — never native slots.
- REMAINING: gm_06:476 Toob lit-mirror (live code, writes board+0x6448
  nobody reads on replica boards — harmless; folds into item 8).

## 4. Sky(13) tails (DONE 2026-11-25)
- gm_04:208 SKYRENDER diag: called the retired SkyDiag stub — pure dead
  logging, deleted.
- gm_08:204 SkySceneTails gate: function is fully S1-driven internally
  (walks objDb, acts only on PILLAR-prefixed refs, 6+ early-returns).
  File proof: only Level9 contains PILLAR (52 refs) — every other file
  is a no-op by content. Gate removed, runs on every board.
- REMAINING: none.

## 5. Odd(9) BadBall table variant (DONE 2026-11-25, pending file rename)
- DROPPER block is pure-name now (isOdd/isBareLifterOdd deleted).
- LIFTER block accepts bare (=150, native ctor 0x43698f writes 0x96 on
  num==0 — verified) or LIFTERn; LIFTERFOO still never builds.
- REMAINING (user file edit): rename Odd's bare LIFTER S1 to DROPPER in
  the meshworld. Until then stock Odd routes its lifter nowhere (falls
  through unbuilt) — do NOT ship this build to Odd players yet.

## 6. Pin behavior: Master(14) vs BonkCyl (DONE 2026-11-26, user verifies)
- Gate cut to `isBonkCyl` only (content-14 half removed). BONKCYLINDER
  selects Master class on name alone, anywhere. POPCYLINDER always
  means Sky class now — the rename is the disambiguator.
- NOT rig-verified: rig carries stock Master (POPCYLINDER S1s, now
  Sky-path by design). Needs user's adjusted file (BONKCYLINDER).
  User runs Master: confirm pins + close behavior (close-crash still
  open, untouched by this change).

## 7. Sky(13) trapdoor-as-popdoor (DONE 2026-11-26, pending file rename)
- Sky block is POPDOOR-only now (TRAPDOOR-on-13 half removed). Same
  rename shape as #5/#6: Sky's door = POPDOOR, Tower's = TRAPDOOR
  (line 640 block untouched — its native name, no gate needed there
  once Sky's S1 is renamed).
- REMAINING (user file edit): rename Sky's single TRAPDOOR S1 to
  POPDOOR in Level9. Until then stock Sky's door falls through to the
  unknown-S1 log (no crash — the most-exercised path in the mod).

## 8. Master(14) velocity constants (DONE 2026-11-26)
- Master's 5.0/12.0 were file-unreachable (its file has no bumper S1s);
  gate cut. Scale+max now come from ext, fed at load by optional S1
  N:VELOCITY(max,scale) (defaults 4.0/10.0 = old base values).
- Parser harness-tested (7/7: decimals, ints, malformed-reject,
  case-insensitive). Rig Beginner clean (defaults path live).
- REMAINING: none. If your Master file ever gains bumpers and you want
  5/12 back, add one N:VELOCITY(12.0,5.0) S1 to it.
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
