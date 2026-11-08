# LevelFeatures_Loader — User Fault Reports (RodentRacer, 2026-11-01)

Every fault below was reported by RodentRacer from real-Windows testing.
Status is updated as fixes ship and he confirms them. Do not remove entries —
mark them FIXED with the version instead.

| # | Race | Fault | Status |
|---|------|-------|--------|
| 1 | Beginner | Crash touching bumpers 4/5/6 (`0001:00054176`, Draw op). Bumpers 1–3 bounce but never glow white. | FIXED v13.3 (crash) + v13.4 (glow), user-confirmed |
| 2 | Intermediate | Bridge misplaced or absent. Sounds still work. | FIXED v13.9 (manual Rx carry + far-unmark), user-confirmed 2026-11-03 |
| 3 | Dizzy | Spiral warps ball incorrectly. Tippers invisible. Gluebies don't slow ball. Waterwheel incorrectly warps ball. | FIXED — all four user-confirmed 2026-11-07 (waterwheel/looper/tippers 11-03; glue bubbles + spiral 11-07) |
| 4 | Tower | Catapult crashes game (`BASS.dll 0001:000028B7`, Update op). Cannot test farther objects. | OPEN |
| 5 | Up | Smaller lifters absent. | OPEN |
| 6 | Neon | Ball outline red instead of yellow. 8-ball doesn't glow. | OPEN |
| 7 | Expert | Fans absent. Bridges are drawbridges (Intermediate type) instead of falling bridges (Expert type). | OPEN |
| 8 | Odd | Crash at gravity flip (2454000:00000400, Update op). Cannot test farther objects. | PARTIALLY FIXED — tag-dtor call passed this=1+no stack arg (MinGW 1-arg thiscall); now TagDtor(tag,1) matching native 0x40EE29 (codegen-verified). Shipped 2026-11-07, awaiting user retest |
| 9 | Toob | Branching paths don't work. | OPEN |
| 10 | Wobbly | Wobbly platforms don't move (wavy bridge does). | OPEN |
| 11 | Glass | Glass has no reflection and normal friction. Smashers can't break. | OPEN (3 sub-items) |
| 12 | Sky | Posts don't appear. Magnifying glass doesn't follow ball. Pegs don't pop up. Fun ball doesn't activate. | OPEN (4 sub-items) |
| 13 | Master | Drawbridge doesn't move (but present). Chomper sounds (Tower) play instead of drawbridge sounds. Tippers work fine and are visible. Cannon doesn't work (cannot test farther). | OPEN (4 sub-items; tippers OK) |
| 14 | Impossible | Rotator doesn't carry ball (looper and pendulum neither). Gears don't carry ball correctly. Help inertia doesn't help inertia. | OPEN (3 sub-items) |
