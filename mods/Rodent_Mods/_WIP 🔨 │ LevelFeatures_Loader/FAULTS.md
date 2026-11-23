# LevelFeatures_Loader — User Fault Reports (RodentRacer, 2026-11-01)

Every fault below was reported by RodentRacer from real-Windows testing.
Status is updated as fixes ship and he confirms them. Do not remove entries —
mark them FIXED with the version instead.

| # | Race | Fault | Status |
|---|------|-------|--------|
| 1 | Beginner | Crash touching bumpers 4/5/6 (`0001:00054176`, Draw op). Bumpers 1–3 bounce but never glow white. | FIXED v13.3 (crash) + v13.4 (glow), user-confirmed |
| 2 | Intermediate | Bridge misplaced or absent. Sounds still work. | FIXED v13.9 (manual Rx carry + far-unmark), user-confirmed 2026-11-03 |
| 3 | Dizzy | Spiral warps ball incorrectly. Tippers invisible. Gluebies don't slow ball. Waterwheel incorrectly warps ball. | FIXED — all four user-confirmed 2026-11-07 (waterwheel/looper/tippers 11-03; glue bubbles + spiral 11-07) |
| 4 | Tower | Catapult crashes game (`BASS.dll 0001:000028B7`, Update op). | Catapult FIXED 2026-11-08 (Sound_PlayChannel stack shift, rig-verified). Trapdoor FIXED 2026-11-10 — USER-CONFIRMED WORKING 2026-11-19 (catapult + trapdoor + windmill all confirmed). Chomper still awaiting confirm (diag chomper_diag.txt requested 2026-11-10 — play Tower ~30s, send the file if the mouth is still dead/silent). |
| 5 | Up | Smaller lifters absent. | FIXED 2026-11-10 — user-marked complete. Root cause was zeroed create out-params: native Up create stores out1=obj/out2=[obj+0x10E0] (0x411985/87), native Expert spinner stores out1=obj (0x40E686) — zeroing pulled them out of CD4 (invisible-but-audible). Restored native out-params. Rig-verified: 3/3 in CD4, ride test ball carried ±110 units on visible yellow pad + magenta column, collision confirmed, no crash |
| 6 | Neon | Ball outline red instead of yellow. 8-ball doesn't glow. | v13.35 OUTLINE FIX (needs Windows eye-confirm) + v13.36 MARKER COLOR SHIPPED (rig-verified): any type-0 S3 light with pos=(0,0,0)+target=(0,0,0) is a glow marker, up-triple = color (0-1 clamped, emitter=colorx10, quad=RGBA). Zero-light files → yellow default; real-light files w/o marker → native. v13.37 8-BALL FIX SHIPPED (rig-verified pushes, needs Windows eye-confirm): sprite-balls render via 0x4011C0 hardcoded white, LFL rewrites the triple to the glow color while active. |
| 7 | Expert | Fans absent. Bridges are drawbridges (Intermediate type) instead of falling bridges (Expert type). | FIXED 2026-11-16 — fans re-enabled (g_enableExpertFans 0→1) + spinner mesh Level2→Level5 (both paths). Rig-verified: Expert loads clean, all 6 FAN S1s built (native Fan vtable), fan renders with housing/pole/3 blades (screenshot), sawblades/bonk/bell/judges present, minutes of runtime with zero crashes. The old full-set crash does not reproduce. Bridges: 2+2 spinners with native vtable, one half observed collapsing natively mid-run (saw/trigger chain works). USER-CONFIRMED WORKING 2026-11-19 (fans + bridges). |
| 8 | Odd | Gravity killplanes active in wrong gravity; gravity switch killed game once (unknown why). | PARTIALLY FIXED 2026-11-16 — wrong-gravity kill ROOT-CAUSED via objdump: global E:LIMIT (0x40C74B) kills UNGATED, LFL fell through to it after its own gravity-gated handling; native Odd (0x40F206+) never calls global for E:LIMIT. Fix is PRESENCE-driven, no content/slot IDs: boards that HAVE gravity planes (COLL_FLAG_GRAVITY, the same flag gating the handler) suppress the global call; all other boards keep legacy global behavior (Sky scoring, plain-level kill). E:LIMITX/Z/PIPE verified safe (global matcher is exact-match). Boot crash-test clean (43s). Awaiting user retest. Gravity-switch kill still OPEN — need DLL build date + CRASH_ADDRESS + last_touch.txt. |
| 9 | Toob | Branching paths don't work. | FIXED 2026-11-16 (verified live on rig). Root cause: native-ctor boards (contents 10-15, incl Toob) skipped ALL custom collision and fell through to global-only — native per-level handlers (E:BRANCH/B, saws, teeth, spinny) never ran, and LFL's own blocks were bypassed. Fix is content-driven delegation to the saved native slot-29 handler (same pattern as render/create/racestate). Plus a per-frame proximity driver for E:BRANCH(B) (trigger quad parsed from the level file at load, AABB-tested per ball, fires the shared teleport driver) as a safety net. Live proof: ball staged inside the trigger volume teleport-fired with no input — log shows BRANCH count=2 idx=0 dest=(2589,-2675,-1565)=POS01, ball arrived at the exit. Boot crash-test clean (42s). USER-CONFIRMED WORKING 2026-11-19. |
| 10 | Wobbly | Wobbly platforms don't move (wavy bridge does). | RIDDEN+VERIFIED 2026-11-16: full chain works under LFL — N:SQUAREWOBBLY touches reach native dispatch (delegate content=11), rider-add fires (riders=1 live), update tilts (tilt -10 to +9 tracking the ball, ball gets driven). Earlier flat readings were no-contact (ball on static track, never touching a deck). Shipped clean (temp DISP diag removed). USER-CONFIRMED WORKING 2026-11-19 (wobbly platforms move). |
| 11 | Glass | Glass has no reflection and normal friction. Smashers can't break. | FRICTION VERIFIED NATIVE 2026-11-16: low-friction = ball+0xC5C countdown (native 0x4177A2 sets 0xF on N:GLASS, native 0x405E00 decrements, native 0x40174A/0x4026AA apply glass multipliers 0.2/20.0/6.0 while nonzero). LFL only ever SETS +0xC5C (lf_06:1260 mirror + native delegate), never clears. Live rig: c5c pins at 15 on glass, decays off it; pushed ball glided 275u. Pre-2026-10-24 builds dropped ALL Glass touches (goto call_global) so c5c never set = normal friction exactly as reported — fixed by per-CONTENT delegate, just needs retest on current build. No code change (mechanism is native-identical). USER-CONFIRMED WORKING 2026-11-19 (glass friction + smashers). Residual: glass reflection unconfirmed — say if it's still flat on Windows. |
| 12 | Sky | Posts don't appear. Magnifying glass doesn't follow ball. Pegs don't pop up. Fun ball doesn't activate. | PARTIALLY FIXED 2026-11-16 (shipped, needs Windows retest): (1) Racestate now calls native 0x41FC90 directly for Sky-class boards (was skipped when FEAT_SKY_POPCYL set — LFL builds no Sky objects so native starved; pegs+magnifier frozen). Verified live: board+0x47F0 timer cycles 75->0 (native runs). (2) Replicated native loader 0x410900 S1-hook registration (105 refs into board+0x4394, was empty) for content-13; posts (pillars) + magnifier VISIBLE in screenshots. Cloudscape part disabled (0x4605E0 convention crashed postscan — decor only). LFL Sky feature stands down for 13 (native covers it). OPEN on rig: pegs static despite cycling timer (RNG/case chain needs Windows confirm or deeper RE); fun ball unidentified (static 0x4CF3A0 ball at (2278,-3058,-1854) — user please say what/where it should be). Boot flakes (D3D 2s dialogs) are environmental, retry boots. USER-CONFIRMED WORKING 2026-11-19 (magnifier + pegs/popcylinders + funball). |
| 13 | Master | Drawbridge doesn't move (but present). Chomper sounds (Tower) play instead of drawbridge sounds. Tippers work fine and are visible. Cannon doesn't work (cannot test farther). | FIXED 2026-11-17: LFL tipper/bridge code removed for Master (native owns them now). Rig verified: Master loads, renders, timer runs, ball rolls, goal triggers, stable 5+ min, no crash (old 0001:0004EAFF load crash gone). Cannon = E:LAUNCH launcher (native-owned). Needs Windows confirm on drawbridge/cannon/sounds. UPDATE v13.15: LFL mirrors now own Master tippers/drawbridge/BBRIDGE (native delegation bypassed, presence-routed via S1Inventory, Intermediate colors by design) — confirm tipper motion, drawbridge anim + sounds on Windows. UPDATE v13.16: drawbridge-at-origin fixed (NativeOwnsBoardFields — mirror no longer clobbers Master's live 0x4374/0x4378 objects; memory-proven on rig) — re-confirm bridge placement + motion on Windows. UPDATE v13.17: origin-spinner root-caused to OUR out1 (raw mesh, native returns NULL) — mirror out1 = 0 now, exact native parity; ctor natives audited inert under the bypass (functionally no natives) — confirm on Windows. UPDATE v13.19: deck invisible root-caused — dispatcher skips out2 when out1=0 (v13.17 regression), out1=mesh restored; native tail NOP'd (unpositioned 2PBridge pair was the origin ghost); natives fully out of scene, deck = our build at S1 pivot — confirm deck visible/tilting/sounding at the bridge on Windows. UPDATE v13.20: all Master-specific code disabled (native ctor dropped, Master builds universal; swirl/N:BRIDGE ungated); only popcylinder-special, cannon, BBRIDGE remain (shared paths) — retest the deck from clean, send lfdebug.log (has TAILNOP lines). UPDATE v13.21: pops (default variant + 0x5428), glue (0x6080), catapult (0x1100/0x584C/0x5848 + CATAPULT<n> power), break (0x540C + BreakGlass feature) all wired — retest all four + send log (BRFEAT/BridgeBreak lines prove per-frame drivers on a live race). UPDATE v13.22: all Master-specific code removed per user order (no native slots/calls/patches anywhere) — Master loads via generic branches only; retest all + send log. UPDATE v13.23: windmill = wrong rotation axis (timer+0xC0 flag 0 not 1); one-line fix, needs eye-confirm on the tilt. UPDATE v13.24: static-in-Master-slot = Master table row typo (bridgeParamOffset pointed at COUNTER not ANGLE, froze counter at 1.1B); one-field fix, rig shows machine cycling — confirm tilt + break on live race. UPDATE v13.25: propeller+swoop = windmill fall-through (CATAPULT S1 arms FEAT_WINDMILL, BONK_STORE fallback grabbed the deck mesh, unbounded angle); fallback removed, gated on real windmill render — confirm deck tilts and stays silent. UPDATE v13.26: feat now armed only by WINDMILL/CHOMPER S1s (Catapult lump removed); load-path safety moved to native-ctor flag — confirm Tower windmill still spins + Master deck tilts. UPDATE v13.27: native update pump for bonk/pops/halves (slot-11, same scan as tippers) + dead 0x3A44 gate removed + 0x2578 init — confirm bonk bobs, halves fall after break, pops bob. UPDATE v13.28: v13.27 pump double-drove where the native racestate already walks 0x2578 (Master-slot tippers 2x) — pump now stands down when the walk is present (Master slot), still drives no-walk slots. Confirm tippers 1x + bonk/pops/halves animate. UPDATE v13.29: slot-native racestate runs only for the slots own file (else base + universal pump, mutually exclusive by construction) — confirm Master dynamics + Up lifters + file-swapped loads. UPDATE v13.30: user DLL (pre-v13.22 wiring) still had working bonk/halves/pops — v13.22 stripped the native-slot wiring and broke all three. Scoped restore: pops build the Master class (0x36ee0) with 0x5420-mesh + 0x2578/0x5428 lists, halves mirror 0x5418/0x541C (+0x5410/0x5414 mesh pref) with the proximity break retired in favor of the native BONKPOPUP-touch break (0x436e70 args 0/1), bonk 0x540C store restored. Confirm bonk visible + hammer, halves break on popup (not early), pops visible on Windows. UPDATE v13.34: E:GRAVITY handler simplified (direct <TYPE> parse, no parser objects — suspected repeated-flip crash vector); v13.33 driver removed (touch path works). 40-flip rig soak clean — needs Windows real-play confirm + crash EIP if it still dies. UPDATE v13.33 SUPERSEDED: Odd gravity zones proximity-driven (ParseGravityZones resolves E:GRAVITY* AABBs+TYPEs, per-frame apply latching; rig-verified tilt+reset). Needs Windows feel-confirm. UPDATE v13.32: hammer triggers proximity-driven (S6 volume touches never arrive post-load — rig-proven zero dispatches; ParseHammerQuads resolves CALLHAMMER + 2x HAMMERCHASE AABBs at load, per-frame entry fires CreateBonkPopup/HammerChaseStart, popup-active + ball-near-bonk runs native 0x436e70 break; rig-verified +0x10FC 1->0 then halves 0->1). Confirm popup/chase/break on Windows. UPDATE v13.31: pops renamed BONKCYLINDER per user (adjusted Master file carries the new S1s) — Master class on name alone, banked + peg-seeded so the Sky RNG activator cycles them with Sky timing; stock POPCYLINDER S1s still build the same way. Confirm cycling matches Sky on Windows. SUPERSEDED v13.18 theory (no-deck-S1 was wrong, Master has BRIDGE): FEAT now deck-S1-only, BBRIDGE halves use native meshes/slots — confirm bridge visible + tilting with sounds at the bridge on Windows. CORRECTION 2026-11-18: the
  "no crash" claim above was wrong — 0001:0004EAFF open-crash REPRODUCED on rig
  with 36c96c2c (dies ~frame 4 in native update tail; E1/E2a bisects ruled out
  ctor overwrites and InitScene). See row 15 for root cause + v13.12 fix. |
| 14 | Impossible | Rotator doesn't carry ball (looper and pendulum neither). Gears don't carry ball correctly. Help inertia doesn't help inertia. | USER-CONFIRMED WORKING 2026-11-19 (rotator + looper + pendulums carry; help inertia works). Residual: gears carry correctness unconfirmed — say if a gear still throws wrong on Windows. |
| 15 | Master | Crash on open (0001:0004EAFF a few frames after load; user report 2026-11-18, rig-reproduced same day on ship + diag builds, slot 14 and slot-1 alike). | FIXED v13.12 (rig-verified, needs Windows confirm): native Master update (0x420DA0) calls FUN_0044fa90 on board+0x5C64 whenever RNG(0,0x14)==10; native Scene_Load (0x411F60) populates that list via S1 TarBubble* scan (0x4120C9-0x41211F), which LFL skipped — native create (0x4121D0) never appends there (6 appends go to 0x2578/0x5428) — so the list stayed empty and the first RNG-10 (frame ~4) read *(NULL+4). Fix replicates the scan post-Board_Setup (content-14 only) + dummy entry for TarBubble-less files. Verified: Master loads, runs 2+ min to goal stars, Warm-Up clean, boot clean. Refined v13.13: content gates (==4/==14) removed — one unified block detects the native consumer dynamically (scans board's saved native update for the 0x44FA90 call, takes list offset from nearest LEA; exe audit proves only Dizzy + Master call it) and ext feed follows UNI_NATIVE_CTOR, not race IDs. Rig-verified v13.13: Master 73s gameplay, Dizzy loads+runs, boot clean. |

## 2026-11-10 — Tower Update crash at BASS.dll+0xA8EA (user report, UNDER INVESTIGATION)
- Report: Tower, Update op, 26s runtime, user believed mace-trigger touch.
  0xA8EA in every shipped build = UniversalDispatchCollision E:BRANCH
  pipe-vector block (sprintf POS/VECTOR + float init). That block needs
  COLL_FLAG_BRANCH + touched name "E:BRANCH*" — both absent on stock Tower
  (rig live-table proof: 15 names, no BRANCH, flag 0; 200+ touch sweep,
  no crash). Cause unreproduced on rig.
- Diag shipped: bass.dll now writes last_touch.txt (game dir, rewritten
  only when the touched name changes — no per-frame I/O) with the exact
  last E:/N: touch. Rig-verified: logs E:TRAJECTORY then E:OPENSESAME on
  the trapdoor fall path; trapdoor fix intact (f4 0->10). Ask user to
  play normally, and after any crash send last_touch.txt — it names the
  real touch.

## 2026-11-10 — Tower 0xAA4A crash = E:BRANCH block tail (entry-walk, lf_06:1025-1028)
- User's touchlog-build crash at BASS.dll+0xAA4A. Debug-line mapping pins it
  to the E:BRANCH block's entry-walk loop (lines 1012-1029), same block as
  the earlier 0xA8EA (float-init). Both need COLL_FLAG_BRANCH + name
  "E:BRANCH*" — absent on stock Tower (rig live-table proof, 36-E: file
  census, no BRANCH anywhere). last_touch proves the crashing call's name
  was "E:MACETRIGGER".
- Working theory: the crashing touch carries a dangling/short name pointer
  (readable first bytes, garbage after) that passes the entry check and the
  gate on recycled heap bytes, then crashes in the block. The 4-byte entry
  probe was insufficient; the diag only logged readable names, so the file
  kept the previous valid touch.
- Fix shipped: entry now requires full 64-byte probe + NUL termination;
  failures write BADNAME to last_touch.txt and run native-only. Rig-verified:
  valid E: touches still log (E:TRAJECTORY), boot crash-clean.
- Next: user retests Tower. BADNAME + crash = theory confirmed (then gate
  needs pointer-validity, not just content). E:MACETRIGGER + crash = theory
  wrong, new data.

## 2026-11-10 — Tower crash: dispatcher does ZERO file I/O now (0xF6AA was diag WriteFile)
- 0xF6AA mapped (debug-line info) to the last_touch WriteFile in the
  dispatcher, not a collision branch. Name was valid (E:MACETRIGGER, probe
  passed), so the write itself was attempted and faulted there.
- Fix: dispatcher records touches to globals only (pure memcpy); the file
  is flushed from UniversalBoardUpdate on a clean stack. Rig-verified the
  flush works (E:TRAJECTORY logged, game alive, boot crash-clean).
- Next: user retests Tower. No crash = file-I/O-in-dispatcher was guilty.
  Crash + new address = look past the diag (native chain on mod-built
  Tower state is the remaining suspect).

## 2026-11-10 — Tower mace-trigger isolation (B/C builds, user testing)
- 0xF6AA (badname) = diag WriteFile; 0xF74A (noio, zero dispatcher I/O) =
  diag memcpy — both innocent instructions, so reported EIPs are not the
  fault site. Three builds, three innocent EIPs, all in dispatcher.
- Isolation: B = mod E:MACETRIGGER branch no-op'd (native still runs, maces
  still swing); C = B + skip native dispatch for mace touches (maces stay
  inert). If B clean = mod branch guilty; B crash + C clean = native/swing
  guilty; both crash = entry/environment.
- Repo stays at noio (B/C are throwaway, not committed).

## 2026-11-10 — Tower mace crash FIXED (unbounded GetIterator index)
- B clean (mod mace branch off) proved the mod's E:MACETRIGGER branch
  guilty. Root cause: GetIterator returns a stateful cursor that is NOT
  bounded by the list count; `*(list+8+iter*4)` reads/writes OOB when the
  cursor drifts (differs machine to machine — rig fine, user crashed).
  Same pattern in N:MACE, E:CATAPULTBOTTOM, N:TRAPDOOR, E:ALERTJUDGES,
  E:SCORE — all six rewritten as bounded for-loops over DATA[0..count)
  with count<64 + IsBadReadPtr guards, identical per-item behavior.
- last_touch stays memory-record + Update-flush (zero dispatcher file I/O).

## 2026-11-10 — Tower windmill absent (user confirms file present — code suspect, NOT missing file)
- User confirms Levels\Level4-Windmill.MESHWORLD is present; treat as mod-side
  load failure, not setup error. Root cause found 2026-11-10 via objdump
  (Tower 0x40D7C0): native WINDMILL returns out1=mesh (board+0x437C) for CD4
  registration; mod zeroed out1 per the wrong "only TURRET sets out-params"
  rule → dropped from CD4 = absent. Fixed: out1=mesh, out2=fresh render;
  removed bogus early-return; corrected the OUT-PARAM RULE comment.
- Rig crash-test clean (38s, no crash). Shipped 2026-11-10 — USER-CONFIRMED WORKING 2026-11-19 (windmill renders + spins).
- Chomper bite FIXED 2026-11-10 (objdump Tower 0x41E861+/0x40DFA0): the content-5
  early-return skipped the chomper state machine (jaw frozen shut → only lower
  jaw, never bites, no sound) and render read chompState from UNI_WINDMILL_SPEED
  instead of board+0x43A0. Added Feature_ChomperStates (0x43A0/0x43A8/0x43AC/0x43B0
  + bite sound app+0x4A8, exact native constants), render now reads board+0x43A0,
  init zeroes 0x43AC/0x43B0, windmill easy rate corrected 0.25→0.5 (native 0x4CF3F0).
  - Rig crash-test clean (38s). Shipped — PARTIALLY FIXED, awaiting user retest:
    upper jaw + bite anim + chomp sound?
  - Chomper still dead on user box with correct DLL (fully silent = states not
    running OR mesh missing, not a render tweak). Diag shipped 2026-11-10
    (CHOMP_DIAG_v1): unconditional board+0x4390 mirror (was conditional —
    mapped garbage could block the real mesh) + chomper states on the ext path
    too + throttled chomper_diag.txt (cf/lvl/feat/mesh/slots/snd/rng/pos).
    Ask: play Tower ~30s, send back chomper_diag.txt. No blind fixes until
    the file names the stage.
- Chomper ("purple chomping mouth") weirdness TBD — need a screenshot.

## 2026-11-19 — Neon home BadBall renders black (no emission, no outline)
- Rig screenshots: black disc at home instead of lit 8-ball. Binary-verified
  root cause: BadBall render v[2] (0x402DE0) draws its emission + outline
  sprites ONLY when ball+0xC80!=0; native Neon loader (0x41631D loop over
  board+0x2DEC) sets +0xC80=1 + yellow colors on EVERY ball incl BadBall.
  LFL only armed players (P1/P2 follower + glow quad) -> BadBall +0xC80=0
  (verified live) -> mesh body unlit in 0-light LevelDark + no glow.
- Fix: Neon_BadBallGlow (src/lf_10_neon.c, called from Neon_BuildLights):
  walks board+0x29D4, sets +0xC80=1 + (r,g,b) on vtable-0x4CF3A0 balls only
  (players untouched: +0xC80 is a live pointer for 0x4CF314 class).
- Rig verified: gate=1 color=(1,1,0) live, no crash, Neon loads. Screenshot
  shows BadBall now catches yellow light (lit crescent) but base still black:
  the 0x453150-derived sprite params (+0x1EC..+0x1F8/+0x1C0..+0x1C8) also need
  mirroring (native writes them in the same loop). NEXT: replicate the two
  0x453150 calls per BadBall, then outline should return too.
- Shipped 2026-11-19 (bass.dll + zip refreshed) — PARTIALLY FIXED, awaiting
  user retest on real Windows.
- Border follow-up (same day): vanilla-vs-LFL live comparison isolated it.
  Vanilla home BadBall carries baked RenderQuad params from the native
  loader loop: +0x1BC/+0x1C0/+0x1C4/+0x1C8=(1,1,0,1),
  +0x1EC/+0x1F0/+0x1F4/+0x1F8=(1,1,0,1), +0x204=0 (recorded live; border
  sprite material dif=(1,0,0,0) is STOCK in both, not LFL-caused).
  LFL wrote none of these -> border/glow sprites drew degenerate.
  Neon_BadBallGlow now writes the full stock set (hardcoded; marker-colored
  levels would need the 0x453150-call version). Rig: all values verified
  live, no crash; adjacent-balls screenshot shows a thin continuous yellow
  rim on the eastern ball, but rig resolution can't prove which ball it is.
  Shipped 2026-11-19 (bass.dll + zip refreshed) — PARTIALLY FIXED, awaiting
  user retest on real Windows: BadBall should show the yellow ring border.

## 2026-11-19 — player hamster renders pure green (selective tint)
- User (real Windows): entire hamster green, everything else yellow-correct.
- Rig cannot reproduce at useful res; eliminated: +0x1CC quad (vanilla player
  has it too = stock), marker (can't fire on 0-light stock file), sprite
  triple (never runs for the player: slot[7] gated by +0x2FC=1.0, sprite
  path only, third caller dead vtable), gate/params (vanilla-identical).
- Only remaining per-frame variable is the follower light color. Probe build
  2026-11-19 writes %TEMP%\lfl_neon_verdict.txt (path/has/marker/rgb) so we
  can see what LFL computed on the user's box. TEMP DIAG, remove after.
- Rig verdict: path='levels\leveldark' has=0 marker=0 rgb=(100,100,0).
- ROOT CAUSE FOUND 2026-11-19 via objdump: native Neon loader (0x41655B/
  0x4165C8/0x416632) writes ALL THREE player ball quads (1,1,0,1): quadA
  +0x1BC, quadB +0x1CC, quadC +0x1EC. Render draws the quadA sprite
  (lea 0x1B8 -> 0x45D660 at 0x4042AE). LFL's Neon_BallGlow wrote ONLY quadB,
  leaving quadA at universal-ctor defaults (red missing) = green hamster.
  Fix: Neon_BallGlow now writes all three sets, native-identical. (Kept: hamster still green, ball+ring yellow - quads exonerated by user description.)
- ROOT CAUSE #2, 2026-11-19 via objdump: Neon_PatchSpriteColor (v13.37 8-ball fix) wrote (r,g,b) in push order, but 0x453150 fills the ball struct REVERSED: last push 0x4011D5 -> +0x7B0, first 0x4011C9 -> +0x7B8. Mesh reads +0x7B0 as RED. Yellow (b=0) zeroed +0x7B0 = green hamster; ball shell+ring (quads) stayed yellow. Ball_Render calls 0x4011C0 unconditionally (0x4044EB), so the mesh path was always hit. Fix: write C9=b, D0=g, D5=r. USER-CONFIRMED WORKING 2026-11-19 (hamster yellow, diag code removed same day).
  2026-11-19 (bass.dll + zip refreshed) - needs Windows eye-confirm.
## 2026-11-20 - Toob bumpers bounce but never flash white (user report)
- File-driven: Beginner file glows in any slot, Toob file dark in any slot.
- Root cause (objdump): native Toob render (0x410670) reads bumper slots at
  board+0x438C (0x418 stride) + lit at board+0x644C, both native-filled. LFL
  replica reads ext slots (empty: stock Toob has no bumper S1s) + ext lit
  (never set: native-ctor delegates to native 0x410020 first). The native
  scene 0x40FA20 that links N:BUMPER names never runs on the fast path,
  so native 0x410020 skips its own lit write (bounce via global fallthrough).
- Fix: fixup in UniversalDispatchCollision for content-10 - bumper touches
  identified by buffer address (0x438C range), exact native lit set.
- Shipped 2026-11-20 (bass.dll + zip refreshed) - awaiting user retest.

## 2026-11-20 - Toob flipped to universal handlers (native build kept)
- Split UNI_NATIVE_CTOR (native-built) from new UNI_NATIVE_HANDLERS
  (delegate to native origs); content-10 runs universal handlers on its
  native build. Bumper lit is address-identified in the universal collide
  path (ext + native lit), decay/replica render wired for native-built
  content-10, native update's 4 object anims replicated, slot rebuild
  skipped by design. Awaiting user retest (flash + regressions).

## 2026-11-20 - E:BRANCH fixed on universal Toob (user-verified teleport)
- Flag was S1-gated; stock Toob has the branch only as S6 geom -> flag now
  set from S6 find. (A) is teleport+arm like native (was arm-only);
  suffix only selects saw-arm, both fall into shared POS/VECTOR teleport.
  Log confirms: (A) touch -> BRANCH count=2 dest + C3C=1, (B) fires too.

## 2026-11-20 - Sky flipped to universal board constructor
- UniversalSkyInit replicates native 0x41F930 (objdump-verified): Board_ctor,
  vtable 0x4D0FC8, "Board (Sky)"/"SKY RACE", app+0x858 diff flag,
  LoadRaceData("SKYRACE"), music "Bucky Break", meshes (skypillar +
  magnifyingglass via MeshNode_ctor; popcylinder1/2 + trapdoor via
  Level_MeshWorldCtor; clouds.png sprite -> +0x4374), zero +0x47AC/+0x47F0/+0x47F4.
- Native fast path for content-13 removed; create/collide/racestate/render run
  universal (UNI_NATIVE_CTOR=0). Racestate 13-branch now calls base +
  Feature_SkyPopcylinder (stand-down removed); TRAPDOOR create gate is
  content-driven. Boot crash-test clean (48s). Awaiting user retest on Windows:
  posts, magnifier follow, pegs pop, fun ball + lfdebug.log if anything is off.

## 2026-11-20 - Sky universal Draw crash fix (0001:00052087)
- User crash: Board (Sky), Draw op, d3d8.dll, FinishLoad(OK). Cause: native Sky
  scene (0x410830) appends every non-PILLAR S1 hook into board+0x4394 with NO
  null check — UniversalSkyInit missed the native ctor's AthenaListInit there,
  so the scene wrote through malloc garbage = heap corruption = first-Draw
  crash. Fix: AthenaListInit(+0x4394) in UniversalSkyInit (native parity).
  Also added content-13 to the Step 2/3 vtable+names exclusions (file-swapped
  Sky keeps its class/identity in any slot). Rig Sky-load NOT a valid gate:
  even the rollback build dies on rig Sky load (env limitation), boot test
  clean (48s). Awaiting Windows retest.

## 2026-11-20 - Sky Draw crash persists (diag build)
- uni3 (+0x4394 init, steps 2/3) still crashes at 0001:00052087 on real
  Windows, same address. Static audit holds (ctor/scene/slot-table parity).
- Shipped throwaway diag build (SkyDiag channel -> sky_diag.txt in game
  dir, Sky-content only): SKYINIT enter/done, SKYREPLICA hooks count,
  per-S1 CREATE census (content 13), SKYRS ticks (1 + every 600),
  SKYRENDER features/sprite/list (first + every 600). Run Sky ~15s, send
  sky_diag.txt + crash dialog.

## 2026-11-21 - Sky hybrid fix (universal ctor + native handlers)
- Root cause narrowed: 11-19 working build ran Sky fully native (ctor
  0x41F930 + scene + create + racestate + render). uni1-3 hand-replicated
  create/racestate/render; Draw faulted in d3d8 at 8s on real Windows.
  Static audit of the replica found no single smoking gun (meshes/sprite/
  calls all binary-verified), so restored the proven handlers instead.
- Fix: UniversalSkyInit still builds the board (native-equivalent DAT),
  but content-13 now sets UNI_NATIVE_CTOR=1 + UNI_NATIVE_HANDLERS=1, so
  create/racestate/render/collision delegate to the native content origs
  (0x410AD0/0x41FC90/0x410E80/dispatch) + additive feature blocks — the
  exact 11-19 combination. POPDOOR carved out of delegation (custom-name
  mirrors still build; vanilla TRAPDOOR goes native).
- Rig test (hybrid 0760c283, Wine): Sky loads (SKY-HYBRID + SKYINIT +
  replica hooks + native racestate in sky_diag, NO SKYRENDER = replica
  skipped), gameplay 28,800+ ticks no crash, blue-sky/checker/loop
  screenshot. Awaiting Windows retest — send sky_diag.txt + result.

## 2026-11-21 - Sky handler bisect build (find the guilty replica)
- Hybrid works (user-confirmed Sky loads/plays). To un-hybridize safely,
  shipped a bisect: 4 magic files (sky_uni_create/race/render/coll.txt,
  presence = that handler runs universal for Sky). Cumulative order
  create -> race -> render -> coll; first crash after adding a file =
  that replica guilty. SKY-FORCE line in sky_diag.txt proves the config.
- Rig-verified: boots clean, baseline Sky loads+plays (25,800 ticks),
  SKY-FORCE 0 0 0 0 logged. Per-flag loads need the user's fast box
  (rig nav too slow, and Wine can't reproduce the d3d8 fault anyway).

## 2026-11-21 - Sky Draw crash ROOT CAUSE + FIX (full universal)
- Bisect result: create/race/coll universal all clean on Wine; render=1
  crashed instantly in d3d8 (Board [Sky], same module as user's 0x52087).
- Step-diag isolated it to the Z-write block: native 0x410EDB pushes
  (subObj, 0xE, 0) with ECX=app + callee cleans 12 = __stdcall. The
  replica called vtable[0x32] as __thiscall (this in ECX, ignored), so
  the callee read 0xE as the device -> d3d8 AV on first frame. Fixed both
  sub-object calls (disable + restore) to __stdcall; verified 6,600+
  clean BOX frames + gameplay on Wine, all four handlers universal.
- Also fixed (visual parity, not the crash): sprite pos now from
  board+0x4378/7C/80 (ext slots were never filled = garbage), texture
  path case to native textures\clouds.png, projection const 1900.0
  ([0x4CF6F8], was misread 0x454D4143=3284.08).
- RULE (all future D3D work): engine-object vtables = __thiscall, RAW
  D3D-device vtables (gfx+0x154 class) = __stdcall, this on stack.
  The glass rs() call only works by accident (this already on stack).
- Hybrid retired: content-13 runs full universal; sky_uni_*.txt flags
  inert (kept for future bisects). Ship build = full universal Sky.
- SkyDiag handle lesson: persistent CREATE_ALWAYS handle orphans on rm
  (writes go to deleted inode). Read via /proc/PID/fd when missing.

## 2026-11-21 - Sky fog fix (cloud sprite position + draw order)
- User: fog way too close on real Windows. Two native-parity defects found:
- (1) Cloud sprite drew at garbage pos. board+0x4378/7C/80 held heap
  garbage (x~0 denormal, y/z=NaN) — Step-1b zeroing doesn't cover them and
  nothing filled them (CLOUDSCAPE lookup was #if-0 disabled since the
  2026-11-16 postscan crash). Native 0x4109FE fills via 0x4605E0
  (ECX=meshWorld, push(outbuf,name,0), callee-clean ret $0xC). Re-enabled
  with hand-rolled asm (MinGW __thiscall caller-cleans and would corrupt
  ESP — that was the 2026-11-16 crash). Verified SKY-CLOUDPOS filled,
  res=heap, game alive. New rule: callee-clean native fns need asm, never
  MinGW __thiscall.
- (2) Draw order. Native draws the box FIRST (far proj + Z-less sprite),
  then the level overdraws it. Mine drew it LAST, painting the Z-less
  sprite over the track. Reordered: Sky pre-runs SkyBoxDraw before the
  base render (post-base block skips via skyPreRan). Only Sky sets
  REND_SKY_SPRITE so no other race is affected.
- Rig shows clear views throughout; user retest decides the fog.

## 2026-11-21 - Sky poles restored (PILLAR hook inversion + render loop)
- User: poles missing. PILLAR01+ S1 refs exist; my scene replica SKIPPED them
  (my_strnicmp==0 continue) — native 0x410927 hooks PILLARs (strncmp==0
  falls through to the 0x453810 hook), skips the rest. Inverted to match.
- Hooks alone didn't draw: native render tail (0x410FBE) loops +0x4394 and
  per item runs Timer_Init + Timer-v[0x8] pose + skypillar-mesh-v[0x1C]
  render (board+0x436C = "meshes\\skypillar") + Timer_Cleanup. Replicated
  with hand-asm (both v-calls callee-clean ret12/ret8; MinGW __thiscall
  caller-cleans — verified by objdump, would corrupt ESP).
- Two deeper findings, both now rules: (1) Board_Setup re-inits +0x4394
  (native setup does the same), so pre-Setup hooks are wiped — scene tails
  run post-Setup (Step 6d). (2) 0x453810 APPENDS SUBLISTS (obj+4/obj+0x40C
  as count/items), not single items — post-setup S1 runtime objs aren't
  list-like, so hooks silently corrupt. Positions stored in a static array
  instead; render draws from it. No native-list dependency.
- Verified on rig: 52 pillars stored + rendered, poles visible under track.

## 2026-11-21 - Sky poles placed correctly (FILE-Y height fix)
- User: poles present but in wrong places (on pad, piercing, floating).
- Root cause: native pose 0x457B50 reads only S1 X/Z (ignores S1 Y, verified
  disasm + -500 Y test + timer matrix logs). Timer Y stays 0 = start level,
  so ALL 52 pillars rendered at start height — deep ones floated 200-1500
  above their track, overlapping the start area.
- Fix: override Y translation (timer+0x34) with file S1 Y post-pose, pre-render.
  Each pillar now sits at its designed height (deep ones drop out of view,
  near ones unchanged). Verified on rig: pad clear, pillars as supports, no
  piercing. Single-pillar and zone tests isolated it (south correct, north
  wrong = deep ones floating high).

## 2026-11-22 - Sky background pillars (revert fileY, hide far floaters)
- User: foreground fixed but background still broken.
- Root cause of background breakage was MY fileY patch (timer[13]=S1 Y):
  S1 Y is foundation depth (below surface), not surface height. PILLAR12
  (fileY -374, upper-track surface 0) rendered detached with a sky gap.
  [13] is Y-slot (vertical shift, no tilt — verified) but file Y is wrong
  height for upper-track pillars (deep foundations, short mesh can't reach).
- Fix reverted to native pose height H (surface, flush for upper/start).
  Far pillars (S1 Y<=-200, deep foundations) float at fixed H (pose ignores
  Y, binary-verified) — hiding them matches native (fog/cull). Near ones
  (y>-200, 7 S1s) render at native pose height (surface, flush, pad clear).
- Verified on rig: pad clear, 4-5 supports under platform/track, no piercing.

## 2026-11-22 - Sky pillars distance cull (fix crowded line)
- User: all poles squished onto one down-left line (GO! shot, matches their game).
- Cause: full 52 rendered, far ones crowd the track line (dense, overlapping)
  instead of hiding via fog/distance like native (7 visible, spaced).
- Fix: distance cull (dist<500 to pad) — 7 nearest S1s (P,01,03,04,06,12,46),
  spread W/C/E, 0 on pad. Verified on rig: 5 visible, spread 2D, no line.

## 2026-11-22 - Latest (dist<500, pose-native, 7 nearest)
- Slot audit: [12]=500 and [13]=500 both no-ops (inert, renderer ignores).
  [14]=fileX (X, spread, proven east-correct for PILLAR04),
  [15]=fileZ (Z, spread, verified, 0 zeros in 52).
  Manual D3D matrix misplaces east (X=[13] wrong slot) — native pose correct.
- Ships 7 nearest (dist<500, spread W/C/E, 0 on pad, parallax on rig).

## 2026-11-22 - Cull proven (44 skip, 8 render), ship clean
- SKIP log on user box: 44 skips (d2>250000), 8 render (0,1,3,4,6,12,45,46).
  Cull works. n=52 is total stored, not rendered count.
- Ship: dist<500, pose-native, no DIAG spam. Boot-clean.

## 2026-11-22 - X zeroed proof + fix (MAT01 matrix dump)
- MAT01 post-pose (PILLAR01 file -17,-8.6,-289):
  m12=0 (X ZEROED, should be -17), m13=-17 (fileX in Y slot, should be 0),
  m14=-289 (Z correct). User right (X, not Z).
- Pose 0x457B50 leaves m12=0 and writes fileX to m13. Fix post-pose:
  mm[13]=fileX (m12=X), mm[14]=0 (m13=Y/H). m14 (Z) already correct.
- Rig: parallax 3D spread, pad clear, no line/flat.

## 2026-11-22 - Y identical -> X-only fix (Y native varied)
- User: better (X/Z spread), but all Y identical (my m13=0 fixed H).
- Native Y=fileX (pose, varied heights). Reverted Y override, X-only:
  mm[13]=fileX (m12=X). Y/Z from pose (native).
- Rig: heights varied (tall foreground + short under-track), spread depth,
  no on-pad, all anchored.

## 2026-11-22 - Inverted Y test (user: heights seem inverted)
- Negated Y (m13=-fileX). Rig: no flip visible (near unchanged), better anchored.
- Ships for user compare (tall/short correct? Or still inverted?).

## 2026-11-22 - Full 52, X-fix, no inversion (per user)
- X-only (m12=fileX), Y/Z pose-native (no inversion), full 52 no cull.
- Boot-clean. Ships for user height compare.

## 2026-11-22 - Popcylinder blue/white swap (parity flip)
- User: blue/white swapped. Pop1=white [1,1,1,1], Pop2=blue [0.086,0.58,0.925,1].
- Flipped parity (idx&1 -> (idx+1)&1). Boot-clean.

## 2026-11-22 - Popcylinder path-based colors (A=white,B=blue)
- User: some flipped, some right (not global parity). Native Path A (EBX 0-3,8-11)
  vs Path B (4-7,12-15). Try A=Pop1 white, B=Pop2 blue. Boot-clean.

## 2026-11-22 - Popcylinder suffix odd/even (odd=white,even=blue)
- User: every other peg (odds one color, evens other). Verify.
- Suffix-based (raw&1): odd suffix->Pop1 white, even->Pop2 blue.
- Native uses 16 explicit slots (Path A/B ranges, no parity logic in code).
- If slots alternate, this matches. Boot-clean.

## 2026-11-22 - Popcylinder path-dependent parity (Path B flipped)
- User: 5-8,13-16 flipped (= Path B, EBX 4-7,12-15). Path A correct.
- Path A (0-3,8-11): suffix-odd->Pop1, even->Pop2. Path B flipped.
- Boot-clean. Matches native Path A/B banks (forward 0x4384[] vs backward 0x4388-[]).

## 2026-11-22 - Pop pattern case3 idx6->idx7 (Ghidra-verified)
- Decompiled SkyBoard_Update 0x41FC90 fully (cases 0-5). Case3 native:
  idx 0,1,2,3,4,7,8,11,12,13,14,15 (complement of case0 5,6,9,10).
- Mine had idx6, native idx7. Fixed. All other cases match exactly.

## 2026-11-22 - Impossible joins universal path (baseline v1)
- Content 15 off native fast path (0x424C20 disabled). UniversalImpossibleInit
  replicates ctor: Board_ctor, vtable 0x4D21C0, "Board (Impossible)",
  +0x4348=1, "IMPOSSIBLE RACE", IMPOSSIBLERACE, "Impossible Theme",
  5 meshes (Looper/Gear/BigGear/Rotator/Pendulum) to 0x436C-0x437C.
- Steps 2/3 keep Impossible vtable/names (added 15 to exclusions).
- S1 mirrors not yet (gears invisible in v1). Boot-clean (title).
- S1 census: 15 BigGear, 25 Gear, 1 Looper, 3 Pendulum, 1 Rotator, 15 Flag.

## 2026-11-22 - Impossible gears bank to 0x2578 (native parity)
- RE: Impossible_CreateDynamicObjects 0x417FE0 decompiled fully.
- Mirrors banked to ext UNI_OBJ_LIST (base update/render never pumps ext).
  Native banks all 5 kinds to board+0x2578. Switched to 0x2578.
- Out-params: native leaves unset except PENDULUM (sets out1/out2).
  Mirrors now match (drop for LOOPER/GEAR/BIGGEAR/ROTATOR, keep PENDULUM).
- Mesh sources: native uses board+0x436C-0x437C (ctor-loaded); mirrors use
  ext (same files, redundant loads). Equivalent, kept.
- Pendulum native passes NAME as y-arg (garbage y); mirrors pass real y
  (better than native, kept). Boot-clean.

## 2026-11-22 - Impossible gear spawn diag (ImpDiag, temporary)
- Added imp_diag.txt (ImpDiag, same pattern as SkyDiag, CREATE-only, safe).
- Logs each gear S1: SPAWN name x y z mesh obj (70 lines max).
- Bank is 0x2578 (native parity), out-params parity, but 0 gears at start
  on rig (closest S1 312 out, should be visible). Diag reveals which S1s
  spawn vs skip (mesh NULL? ctor fail? positions?). Throwaway, delete when green.

## 2026-11-22 - Impossible gear mesh case fix (Levels\\LevelImpossible-*)
- Root cause: UniversalImpossibleInit loaded gear meshes with lowercase
  paths ("levels\\levelimpossible-looper") but files are mixed-case
  (LevelImpossible-Looper). Wine/Linux case-sensitive -> load fails ->
  board slots garbage/NULL -> gears invisible.
- Fixed to "Levels\\LevelImpossible-Looper/Gear/BigGear/Rotator/Pendulum".
- Mirrors read board+0x436C-0x437C (native parity) + ImpDiag SPAWN lines.
- Boot-clean 45s title. User: load Impossible, send imp_diag.txt + screenshot.

## 2026-11-22 - ImpDiag positions as %d (wsprintfA has no float)
- wsprintfA %f prints garbage; use %d with int casts.
- 100ms xdotool presses are reliable single moves (taps eaten, 1s holds repeat).
- Rig Impossible slot-15: 0 gears at start, meshes garbage on Linux
  (case-sensitive paths) but Windows loads fine (case-insensitive).
- Next: user imp_diag.txt with positions maps visible vs invisible S1s.

## 2026-11-22 - Reverted to ext meshes (board slots give ZERO on Windows)
- Board-slot mirrors (0x436C-0x437C via UniversalImpossibleInit) spawn 0
  gears on user Windows. Ext S1EnsureMeshWorld gave SOME gears.
- Reverted all 5 to ext + kept ImpDiag %d positions + 0x2578 bank.
- Wrap-around: Up from WARM-UP -> PREVIOUS, one more Up -> IMPOSSIBLE.
- 100ms xdotool presses register exactly once (taps eaten, 1s repeat).

## 2026-11-22 - Impossible out-params fix (Ghidra 0x417FE0 RE)
- Native sets out1=obj, out2=obj[0x435] for ALL gears (else-branch).
  Mine set 0,0 for LOOPER/GEAR/BIGGEAR/ROTATOR -> caller skipped setup.
- Fixed all 4 to match native. Rig: 1 gear visible behind start tower.
- Sizes verified: LOOPER 0x1500, GEAR/BIGGEAR 0x1514, ROTATOR 0x1508.
- BIGGEAR [0x43D]=0.5f + TOUCH +0x544=1, ROTATOR [0x43A]=+/-1.0f match.

## 2026-11-22 - Impossible render RE (0x40C4A0 + slots, partial)
- Native 0x40C4A0 routes out1->board+0xD54, out2->board+0x10EC, calls
  slots 22 (CallRender) + 21 (CallUpdate w/Timer) with Gfx pose.
- Added routing + slot22: stable, 1-2 gears. Full pose (Timer+Gfx+slot21)
  crashes (0001:00098D9D) - per-vtable slot differences suspected.
- Ghidra flaky (dies repeatedly). Render RE ongoing.

## 2026-11-23 - Impossible gears: 0xD54 is not a list (objdump-proof, SHIPPED)
- Correction to the 11-22 note: native 0x40C4A0 per-S1 does slot33 ->
  pose (TimerInit 0x457AD0/0x457C60/SetPosition 0x457B50) ->
  out1->board+0xCD4 + meshdb+0x1C -> slot22 (0x58) -> slot21 (0x54) ->
  out2->board+0x10EC. There is NO 0xD54 anywhere in the dispatcher,
  and base Board_ctor (0x419030) never inits 0xD54 (inits
  0x884/0x898/0x8B8/0xCD4/0x10EC/0x2578/... only).
- So every Impossible spawn (looper/gear/biggear/rotator/pendulum) wrote
  one AthenaListAppend into garbage + double-listed the render in 0x10EC
  + double-fired slot22 unposed. Only 1-2 gears survived.
- Fix: deleted all five manual 0xD54/0x10EC/slot22 blocks. Mirrors now do
  0x2578 append only (exact native 0x417FE0 parity) + out1=obj/out2=[0x435];
  the dispatcher does 0xCD4/0x10EC/slot22/slot21 natively with pose.
- Boot crash-test clean (93s, no crash). Needs Windows confirm: all 25x
  GEAR + 15x BIGGEAR + looper/rotator/pendulums visible?

## 2026-11-23 - GEARPOS diag (v13.39, SHIPPED)
- User: gears spawn misplaced/misoriented, looper right but riding it
  instawins. Spawn data proven file-correct (47 SPAWN + rig mem-dump:
  obj+0x10D8 == file coords) -> per-frame pose/rider state suspect.
- Build logs runtime positions: UniversalBoardUpdate walks 0x2578 for
  vtable 0x4D5AD0 (GEAR+BIGGEAR+TOUCH), dumps obj/anchor/axle/spd/
  vis+vpos in centi-units at ~3s + ~10s into imp_diag.txt
  (GEARPOS begin/end). Join to S1 coords via obj addr (SPAWN lines).
- Crash-test clean (93s). Ask: play Impossible ~12s, send imp_diag.txt.

## 2026-11-23 - Impossible gears ROOT CAUSE + FIX (v13.41, SHIPPED)
- S1X2 (rig): runtime S1 triple2 = sane rot floats, e.g. GEAR
  (-90.0,-180.0,~0). But obj+0x10E4 held 0xFFFFFFA6/0xFFFFFF4C =
  (int)(-90.0)/(int)(-180.0) TRUNCATED, i.e. NaN as floats.
- Cause: Ctor7f_t typedef declared x2/y2/z2 as INT (lf_01_bumpers.c:39).
  MinGW converted the floats to ints at the call: -90.0f -> -90
  (0xFFFFFFA6) -> reinterpreted as float NaN by Gear_ctor 0x437590 and
  the per-frame pose (0x43E600 reads +0x10E4/8 as axle). One-word fix:
  x2/y2/z2 -> float. Looper/rotator/pendulum ctors take only
  (x,y,z,MESH) floats -> unaffected (looper placed right, as reported).
- Explains everything: NaN axle -> NaN pose matrix -> all 43 visuals
  stuck at origin (looks like "one gear"), wrong orientations, jank
  collisions, NaN carry/ride -> instawin. Anchors/lists/speeds were
  always correct.
- Fix build crash-test clean (93s). GEARPOS/S1X2 diags KEPT: sane axle
  lines in the log prove the new binary runs. Ask: visual confirm on
  Windows (gears spread over the track, rotating).
- RIG-CONFIRMED 2026-11-23: gears render on track (4+ visible in
  screenshots, red/white bullseye, on poles), axle = sane rot bits
  (0xC2B40001/-90.0 etc., zero NaN), GEARPOS axle sane centi-numbers.
  Still needs RodentRacer Windows confirm (gameplay + ride/instawin).

## 2026-11-23 - Expert judges fix (v13.42, SHIPPED)
- User: judges absent/invisible. Root cause (objdump): native Expert
  create appends each judge to board+0x4BBC AND runs 0x434C40 per
  0x4BBC item (zero +0x10F4 + dedup-append to 0x2578). LFL did the
  0x4BBC half only; with out-params NULL by design, judges had no
  update/render link at all. Fix replicates 0x434C40 inline in the
  JUDGE branch. Added JUDGE SPAWN line (dispatch proof).
- Crash-test clean (93s). Ask: visual confirm on Windows + imp_diag
  SPAWN JUDGE lines (expect 5: JUDGE..JUDGE04).

## 2026-11-23 - Odd lifter fix (v13.43, SHIPPED)
- User: Odd lifter invisible + touch crash (no crash EIP; custom file
  still has bare LIFTER, rename to DROPPER not done). Root causes:
  (1) bare LIFTER matches neither DROPPER nor LIFTER-digit branch ->
  falls through unbuilt (invisible); (2) E:DROPLIFT handler called
  g_DropLiftActivate(UNI_BONK_STORE=0) with no null check = crash.
- Fix: bare LIFTER on Odd content/slot routes into the DROPPER path
  (rename plan enforced in code; Up LIFTERn path untouched); handler
  null+validity-guarded (skips when nothing built).
- Crash-test clean (92s). NOTE: stock Level6 HAS the bare LIFTER but
  no E:DROPLIFT event, so the rig can prove build (SPAWN line) but not
  the touch crash — that needs RodentRacer Windows confirm on the
  custom file (visible lifter + safe touch). No EIP yet; send it if it
  still dies.
- CORRECTION: the first rig run (v13.43) did NOT fail the build — the
  DROPPER branch never had a SPAWN line, so zero SPAWNs was expected
  even on success (Odd has no gears/judges to log). S1X2 proved parse.
  v13.44 adds the SPAWN proof line; RIG-PASS 2026-11-23: SPAWN DROPPER
  x=-605 y=-2334 z=-2334 obj=0x5F98B80 (nonzero, race stable 48.8s).
  Touch crash still needs the custom file (stock has no E:DROPLIFT).

## 2026-11-23 - Judge fix RIG-CONFIRMED
- v13.42 on rig/Expert: 5/5 SPAWN JUDGE lines, 0x4BBC count=5 and
  0x2578 contains the same 5 (judge vtable 0x4D52B8 in both lists).
  Not visually seen (judges spawn at x~4600, far from start, no
  driving) — presence proven, Windows eyes still needed for visible.
