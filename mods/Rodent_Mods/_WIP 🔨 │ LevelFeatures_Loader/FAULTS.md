# LevelFeatures_Loader — User Fault Reports (RodentRacer, 2026-11-01)

Every fault below was reported by RodentRacer from real-Windows testing.
Status is updated as fixes ship and he confirms them. Do not remove entries —
mark them FIXED with the version instead.

| # | Race | Fault | Status |
|---|------|-------|--------|
| 1 | Beginner | Crash touching bumpers 4/5/6 (`0001:00054176`, Draw op). Bumpers 1–3 bounce but never glow white. | FIXED v13.3 (crash) + v13.4 (glow), user-confirmed |
| 2 | Intermediate | Bridge misplaced or absent. Sounds still work. | FIXED v13.9 (manual Rx carry + far-unmark), user-confirmed 2026-11-03 |
| 3 | Dizzy | Spiral warps ball incorrectly. Tippers invisible. Gluebies don't slow ball. Waterwheel incorrectly warps ball. | FIXED — all four user-confirmed 2026-11-07 (waterwheel/looper/tippers 11-03; glue bubbles + spiral 11-07) |
| 4 | Tower | Catapult crashes game (`BASS.dll 0001:000028B7`, Update op). | Catapult FIXED 2026-11-08 (Sound_PlayChannel stack shift, rig-verified). Trapdoor FIXED 2026-11-10 — rollover never fired Activate (rig: +0x10F4 stuck 0 under ball) because the N:TRAPDOOR file-scan fallback only read the Sky file (zero tokens); Tower trigger plates live in Level4-Trapdoor1/2. Now scanned for any bare TRAPDOOR S1. Rig-verified: f4 0→10 on contact, door animates 79→0, ball drops through, no crash |
| 5 | Up | Smaller lifters absent. | FIXED 2026-11-10 — user-marked complete. Root cause was zeroed create out-params: native Up create stores out1=obj/out2=[obj+0x10E0] (0x411985/87), native Expert spinner stores out1=obj (0x40E686) — zeroing pulled them out of CD4 (invisible-but-audible). Restored native out-params. Rig-verified: 3/3 in CD4, ride test ball carried ±110 units on visible yellow pad + magenta column, collision confirmed, no crash |
| 6 | Neon | Ball outline red instead of yellow. 8-ball doesn't glow. | OPEN |
| 7 | Expert | Fans absent. Bridges are drawbridges (Intermediate type) instead of falling bridges (Expert type). | FIXED 2026-11-16 — fans re-enabled (g_enableExpertFans 0→1) + spinner mesh Level2→Level5 (both paths). Rig-verified: Expert loads clean, all 6 FAN S1s built (native Fan vtable), fan renders with housing/pole/3 blades (screenshot), sawblades/bonk/bell/judges present, minutes of runtime with zero crashes. The old full-set crash does not reproduce. Bridges: 2+2 spinners with native vtable, one half observed collapsing natively mid-run (saw/trigger chain works). Awaiting user gameplay confirm: fan blow force + bridge fall timing. |
| 8 | Odd | Gravity killplanes active in wrong gravity; gravity switch killed game once (unknown why). | PARTIALLY FIXED 2026-11-16 — wrong-gravity kill ROOT-CAUSED via objdump: global E:LIMIT (0x40C74B) kills UNGATED, LFL fell through to it after its own gravity-gated handling; native Odd (0x40F206+) never calls global for E:LIMIT. Fix is PRESENCE-driven, no content/slot IDs: boards that HAVE gravity planes (COLL_FLAG_GRAVITY, the same flag gating the handler) suppress the global call; all other boards keep legacy global behavior (Sky scoring, plain-level kill). E:LIMITX/Z/PIPE verified safe (global matcher is exact-match). Boot crash-test clean (43s). Awaiting user retest. Gravity-switch kill still OPEN — need DLL build date + CRASH_ADDRESS + last_touch.txt. |
| 9 | Toob | Branching paths don't work. | FIXED 2026-11-16 (verified live on rig). Root cause: native-ctor boards (contents 10-15, incl Toob) skipped ALL custom collision and fell through to global-only — native per-level handlers (E:BRANCH/B, saws, teeth, spinny) never ran, and LFL's own blocks were bypassed. Fix is content-driven delegation to the saved native slot-29 handler (same pattern as render/create/racestate). Plus a per-frame proximity driver for E:BRANCH(B) (trigger quad parsed from the level file at load, AABB-tested per ball, fires the shared teleport driver) as a safety net. Live proof: ball staged inside the trigger volume teleport-fired with no input — log shows BRANCH count=2 idx=0 dest=(2589,-2675,-1565)=POS01, ball arrived at the exit. Boot crash-test clean (42s). |
| 10 | Wobbly | Wobbly platforms don't move (wavy bridge does). | RIDDEN+VERIFIED 2026-11-16: full chain works under LFL — N:SQUAREWOBBLY touches reach native dispatch (delegate content=11), rider-add fires (riders=1 live), update tilts (tilt -10 to +9 tracking the ball, ball gets driven). Earlier flat readings were no-contact (ball on static track, never touching a deck). Shipped clean (temp DISP diag removed). If a ride still looks dead on Windows, say exactly what doesn't move (visual rock vs throw vs lean) + which deck. |
| 11 | Glass | Glass has no reflection and normal friction. Smashers can't break. | FRICTION VERIFIED NATIVE 2026-11-16: low-friction = ball+0xC5C countdown (native 0x4177A2 sets 0xF on N:GLASS, native 0x405E00 decrements, native 0x40174A/0x4026AA apply glass multipliers 0.2/20.0/6.0 while nonzero). LFL only ever SETS +0xC5C (lf_06:1260 mirror + native delegate), never clears. Live rig: c5c pins at 15 on glass, decays off it; pushed ball glided 275u. Pre-2026-10-24 builds dropped ALL Glass touches (goto call_global) so c5c never set = normal friction exactly as reported — fixed by per-CONTENT delegate, just needs retest on current build. No code change (mechanism is native-identical). Still open: break-visual + reflection need Windows natural-play confirm. |
| 12 | Sky | Posts don't appear. Magnifying glass doesn't follow ball. Pegs don't pop up. Fun ball doesn't activate. | PARTIALLY FIXED 2026-11-16 (shipped, needs Windows retest): (1) Racestate now calls native 0x41FC90 directly for Sky-class boards (was skipped when FEAT_SKY_POPCYL set — LFL builds no Sky objects so native starved; pegs+magnifier frozen). Verified live: board+0x47F0 timer cycles 75->0 (native runs). (2) Replicated native loader 0x410900 S1-hook registration (105 refs into board+0x4394, was empty) for content-13; posts (pillars) + magnifier VISIBLE in screenshots. Cloudscape part disabled (0x4605E0 convention crashed postscan — decor only). LFL Sky feature stands down for 13 (native covers it). OPEN on rig: pegs static despite cycling timer (RNG/case chain needs Windows confirm or deeper RE); fun ball unidentified (static 0x4CF3A0 ball at (2278,-3058,-1854) — user please say what/where it should be). Boot flakes (D3D 2s dialogs) are environmental, retry boots. |
| 13 | Master | Drawbridge doesn't move (but present). Chomper sounds (Tower) play instead of drawbridge sounds. Tippers work fine and are visible. Cannon doesn't work (cannot test farther). | FIXED 2026-11-17: LFL tipper/bridge code removed for Master (native owns them now). Rig verified: Master loads, renders, timer runs, ball rolls, goal triggers, stable 5+ min, no crash (old 0001:0004EAFF load crash gone). Cannon = E:LAUNCH launcher (native-owned). Needs Windows confirm on drawbridge/cannon/sounds. UPDATE v13.15: LFL mirrors now own Master tippers/drawbridge/BBRIDGE (native delegation bypassed, presence-routed via S1Inventory, Intermediate colors by design) — confirm tipper motion, drawbridge anim + sounds on Windows. UPDATE v13.16: drawbridge-at-origin fixed (NativeOwnsBoardFields — mirror no longer clobbers Master's live 0x4374/0x4378 objects; memory-proven on rig) — re-confirm bridge placement + motion on Windows. UPDATE v13.17: origin-spinner root-caused to OUR out1 (raw mesh, native returns NULL) — mirror out1 = 0 now, exact native parity; ctor natives audited inert under the bypass (functionally no natives) — confirm on Windows. UPDATE v13.19: deck invisible root-caused — dispatcher skips out2 when out1=0 (v13.17 regression), out1=mesh restored; native tail NOP'd (unpositioned 2PBridge pair was the origin ghost); natives fully out of scene, deck = our build at S1 pivot — confirm deck visible/tilting/sounding at the bridge on Windows. UPDATE v13.20: all Master-specific code disabled (native ctor dropped, Master builds universal; swirl/N:BRIDGE ungated); only popcylinder-special, cannon, BBRIDGE remain (shared paths) — retest the deck from clean, send lfdebug.log (has TAILNOP lines). UPDATE v13.21: pops (default variant + 0x5428), glue (0x6080), catapult (0x1100/0x584C/0x5848 + CATAPULT<n> power), break (0x540C + BreakGlass feature) all wired — retest all four + send log (BRFEAT/BridgeBreak lines prove per-frame drivers on a live race). UPDATE v13.22: all Master-specific code removed per user order (no native slots/calls/patches anywhere) — Master loads via generic branches only; retest all + send log. UPDATE v13.23: windmill = wrong rotation axis (timer+0xC0 flag 0 not 1); one-line fix, needs eye-confirm on the tilt. UPDATE v13.24: static-in-Master-slot = Master table row typo (bridgeParamOffset pointed at COUNTER not ANGLE, froze counter at 1.1B); one-field fix, rig shows machine cycling — confirm tilt + break on live race. UPDATE v13.25: propeller+swoop = windmill fall-through (CATAPULT S1 arms FEAT_WINDMILL, BONK_STORE fallback grabbed the deck mesh, unbounded angle); fallback removed, gated on real windmill render — confirm deck tilts and stays silent. UPDATE v13.26: feat now armed only by WINDMILL/CHOMPER S1s (Catapult lump removed); load-path safety moved to native-ctor flag — confirm Tower windmill still spins + Master deck tilts. UPDATE v13.27: native update pump for bonk/pops/halves (slot-11, same scan as tippers) + dead 0x3A44 gate removed + 0x2578 init — confirm bonk bobs, halves fall after break, pops bob. UPDATE v13.28: v13.27 pump double-drove where the native racestate already walks 0x2578 (Master-slot tippers 2x) — pump now stands down when the walk is present (Master slot), still drives no-walk slots. Confirm tippers 1x + bonk/pops/halves animate. SUPERSEDED v13.18 theory (no-deck-S1 was wrong, Master has BRIDGE): FEAT now deck-S1-only, BBRIDGE halves use native meshes/slots — confirm bridge visible + tilting with sounds at the bridge on Windows. CORRECTION 2026-11-18: the
  "no crash" claim above was wrong — 0001:0004EAFF open-crash REPRODUCED on rig
  with 36c96c2c (dies ~frame 4 in native update tail; E1/E2a bisects ruled out
  ctor overwrites and InitScene). See row 15 for root cause + v13.12 fix. |
| 14 | Impossible | Rotator doesn't carry ball (looper and pendulum neither). Gears don't carry ball correctly. Help inertia doesn't help inertia. | OPEN (3 sub-items) |
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
- Rig crash-test clean (38s, no crash). Shipped 2026-11-10 — PARTIALLY FIXED,
  awaiting user retest: does the windmill render + spin now?
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
