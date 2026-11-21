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
