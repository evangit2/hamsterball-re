# LevelFeatures_Loader — User Fault Reports (RodentRacer, 2026-11-01)

Every fault below was reported by RodentRacer from real-Windows testing.
Status is updated as fixes ship and he confirms them. Do not remove entries —
mark them FIXED with the version instead.

| # | Race | Fault | Status |
|---|------|-------|--------|
| 1 | Beginner | Crash touching bumpers 4/5/6 (`0001:00054176`, Draw op). Bumpers 1–3 bounce but never glow white. | FIXED v13.3 (crash) + v13.4 (glow), user-confirmed |
| 2 | Intermediate | Bridge misplaced or absent. Sounds still work. | FIXED v13.9 (manual Rx carry + far-unmark), user-confirmed 2026-11-03 |
| 3 | Dizzy | Spiral warps ball incorrectly. Tippers invisible. Gluebies don't slow ball. Waterwheel incorrectly warps ball. | FIXED — all four user-confirmed 2026-11-07 (waterwheel/looper/tippers 11-03; glue bubbles + spiral 11-07) |
| 4 | Tower | Catapult crashes game (`BASS.dll 0001:000028B7`, Update op). Cannot test farther objects. | FIXED 2026-11-08 — root cause was NOT the catapult: Sound_PlayChannel typedef took 1 arg but native callee cleans 4 (RET 4), shifting ESP+4 on every catapult-touch sound → next esp-relative read faulted (rig repro: 2 crashes at same cmpb, both on bottom touch). Now called with (ch, 1.0f) matching native push. Rig-verified: repeated bottom contact + full launch cycle, no crash |
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
