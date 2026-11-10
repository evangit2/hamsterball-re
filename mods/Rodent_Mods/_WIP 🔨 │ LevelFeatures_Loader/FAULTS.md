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
| 7 | Expert | Fans absent. Bridges are drawbridges (Intermediate type) instead of falling bridges (Expert type). | OPEN — bridges 2026-11-09: full set verified built (4 spinners 0x4380/0x4798, 5 judges 0x4BBC, bell, 2 saws 0x2578) with correct vtables; spinner out-params restored to native (out1=obj per 0x40E686) 2026-11-10 after the Up-lifter root cause (zeroing = invisible-but-audible). Idle-frozen is BY DESIGN (spinner +0x10F0=100 skips update until the ACTIVATESAW→saw→RNG→0x439A30 chain arms it; ball never left the pad on rig so no events fired). LFL forwards ACTIVATESAW1/2, ALERTSAW1/2, ALERTJUDGES to the native handlers on the right objects — chain looks complete, needs gameplay verification (drive ball into triggers). Both rig crashes this session (0x45D8D9, 0x45DBF2) followed my own memory pokes, not LFL code. Fans: intentionally disabled via g_enableExpertFans (ship-state deferral), needs enabling + crash-test, not a mystery. |
| 8 | Odd | Crash at gravity flip (2454000:00000400, Update op). Cannot test farther objects. | PARTIALLY FIXED — tag-dtor call passed this=1+no stack arg (MinGW 1-arg thiscall); now TagDtor(tag,1) matching native 0x40EE29 (codegen-verified). Shipped 2026-11-07, awaiting user retest |
| 9 | Toob | Branching paths don't work. | OPEN |
| 10 | Wobbly | Wobbly platforms don't move (wavy bridge does). | OPEN |
| 11 | Glass | Glass has no reflection and normal friction. Smashers can't break. | OPEN (3 sub-items) |
| 12 | Sky | Posts don't appear. Magnifying glass doesn't follow ball. Pegs don't pop up. Fun ball doesn't activate. | OPEN (4 sub-items) |
| 13 | Master | Drawbridge doesn't move (but present). Chomper sounds (Tower) play instead of drawbridge sounds. Tippers work fine and are visible. Cannon doesn't work (cannot test farther). | OPEN (4 sub-items; tippers OK) |
| 14 | Impossible | Rotator doesn't carry ball (looper and pendulum neither). Gears don't carry ball correctly. Help inertia doesn't help inertia. | OPEN (3 sub-items) |

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

## 2026-11-10 — Tower windmill absent (missing mesh file, not code)
- Windmill create is fine (native slot or ext-load). It needs
  Levels\Level4-Windmill.MESHWORLD in the game dir; user never got it
  (all 3 repo copies identical eeebab1c, 17KB — established file).
  Resent 2026-11-10. If still absent with the file present, suspect load
  failure (DebugLog off, can't confirm remotely).
- Chomper ("purple chomping mouth") weirdness TBD — need a screenshot.
