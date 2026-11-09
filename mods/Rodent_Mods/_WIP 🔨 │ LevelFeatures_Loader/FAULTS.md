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
| 5 | Up | Smaller lifters absent. | PARTIALLY FIXED 2026-11-08 — two bugs: (1) LFL Up-lifter branch required a digit suffix, so stock Up's 2 bare LIFTER S1s never built (only LIFTER225 existed) — now accepts bare LIFTER (Odd's bare branch first, safe); (2) LFL returned out1=obj/out2=obj[0x438] but native Up create leaves both untouched — dispatcher then appended lifter to CD4 + called v[0x58]/v[0x54] with garbage (visuals parked at origin) — now zeros both. Rig-verified: all 3 lifters (225/150/150 travel) built, states cycling, bases oscillating around S1 pos, no crash. 2026-11-09: bare LIFTER dead again per rename plan (digits-only, LIFTER2/3…) — stock Up's 2 bare S1s stay unbuilt until user ships renamed meshworld; Odd bare branch untouched. Visual confirm pending user retest |
| 6 | Neon | Ball outline red instead of yellow. 8-ball doesn't glow. | OPEN |
| 7 | Expert | Fans absent. Bridges are drawbridges (Intermediate type) instead of falling bridges (Expert type). | OPEN |
| 8 | Odd | Crash at gravity flip (2454000:00000400, Update op). Cannot test farther objects. | PARTIALLY FIXED — tag-dtor call passed this=1+no stack arg (MinGW 1-arg thiscall); now TagDtor(tag,1) matching native 0x40EE29 (codegen-verified). Shipped 2026-11-07, awaiting user retest |
| 9 | Toob | Branching paths don't work. | OPEN |
| 10 | Wobbly | Wobbly platforms don't move (wavy bridge does). | OPEN |
| 11 | Glass | Glass has no reflection and normal friction. Smashers can't break. | OPEN (3 sub-items) |
| 12 | Sky | Posts don't appear. Magnifying glass doesn't follow ball. Pegs don't pop up. Fun ball doesn't activate. | OPEN (4 sub-items) |
| 13 | Master | Drawbridge doesn't move (but present). Chomper sounds (Tower) play instead of drawbridge sounds. Tippers work fine and are visible. Cannon doesn't work (cannot test farther). | OPEN (4 sub-items; tippers OK) |
| 14 | Impossible | Rotator doesn't carry ball (looper and pendulum neither). Gears don't carry ball correctly. Help inertia doesn't help inertia. | OPEN (3 sub-items) |
