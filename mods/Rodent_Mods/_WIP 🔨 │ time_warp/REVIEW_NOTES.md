# Time Warp — Review Notes (2026-09-03)

Files examined:
- `mods/Rodent_Mods/_WIP 🔨 │ time_warp/time_warp.c` (3789 lines, bass.dll proxy)
- `mods/Rodent_Mods/_WIP 🔨 │ time_warp/ARCHITECTURE.md` (420 lines)
- `mods/- 🟣 │ PLUS mods/plus_time_warp/source/TimeWarpMod.cpp` + `README.md` (HB+ v2.1 port)

## What it is

One mod combining four systems on a single frame-epilogue dispatch:
1. Ghost Saver — records TT runs, saves `Ghosts/<Race>.ghost` + `Previous_Run.ghost` (GHSG binary: magic 0x47485347, version, time, frame_count, 40B snapshots x N)
2. Ghost Event — `E:GHOST(name)` collision loads a `.ghost` into App+0x910 playback slot
3. Ghost Triggers — `GT:` S1 ref points (COLOR / SPEED / STOP / START / RESET), entry-trigger + 60-frame cooldown, checks both ghosts
4. Warp — `WARP(N)` S1 ref points, player ball only, 25-unit trigger, 2s cooldown
5. Same-level warp spawns Ghost 2 (heliotrope #db03fc) replaying the segment just driven

Warp phases: RUMBLE 2000ms (color lerp to purple, alpha 1.0->0.5, steering off, music fade 3000ms) -> FLASH 150ms (instant white, fade out, ball invisible, in_tar=1, timer freeze) -> HOLD 1000ms -> FADE 2000ms to white -> LOAD -> REVEAL 1000ms.

Multi-segment: `Level[N].ghost` temp brackets, `Level(N).ghost` confirmed parens. On goal: total time (App+0x5E8 elapsed) vs best; promote or discard. Ghost 1 chains segments sequentially with playback-idx save/restore across warp; Ghost 2 resets to frame 0 each warp.

## Hooks (bass build)

- Frame epilogue 0x46C1F1, 5B JMP, sig-checked (5E 83 C4 08 C3), pushad/pushfd stub — shared dispatch for all subsystems
- App_StartPracticeRace 0x428C50, 7B detour + trampoline (ghost saver pre-inject)
- App_StartTournamentRace 0x4288B0 hook (creates BTT via operator_new 0x528 + BTT_Ctor 0x427660, vtable check 0x4D262C)
- DispatchCollisionEvents 0x40C5D0, 8B JMP stub (E:GHOST parse)
- Level_UpdateAndRender 0x40B7F5 + 0x40B7FF NOPs (ghost mode checks)
- Timer caves 0x41B3E5 (9B) + 0x41B50C (5B) via g_freezeTimer (passive, like N:GOAL)
- TT recording NOP 0x41B690, 7B (NOPs TT check, keeps party check at 0x41B697)
- Pause blocks 0x19D5B, 0x130B5, 0x0B405

Ghost 2 create: operator_new 0xC60 + Ball_ctor 0x4039E0, vtable[1] setup, playerID -1, gravity 0.5, alpha 0.45, append board+0x29D4. Matches proven second-ghost pattern (Ball_Render renders all balls in list).

HB+ port: same logic via onGameUpdate / onEventPlaneCollide / onSceneEnd. onSceneEnd has midWarp guard so warp capture (ghost2_capture) survives the race-start teardown and is consumed after new board loads. Timer caves + TT NOP installed once in Initialize, never restored mid-session.

## What's good

- Single epilogue hook instead of four. Right call — 0x46C1F1 is the safe host (not 0x455A90).
- __thiscall via asm (mov ecx + call), not C function pointers. Avoids MinGW silent failure.
- TT NOP keeps party check. NOP-the-check, don't set profile+0x11 — correct instinct.
- Ghost 2 color tied to RUMBLE lerp color. Good visual continuity.
- [N]/(N) + Ghost 1 chaining is the standout design — real multi-lap haunting, not single replay.
- HB+ midWarp preservation is subtle and correct.

## Risks / verify before ship

1. DCE manual trampoline on SEH function. 0x40C5D0 prologue (PUSH 0xFF + MOV EAX,FS:[0]) sets up SEH; copying 8B to a trampoline breaks the chain -> known crash at Level_RenderObjects+0x62. HB+ build avoids it entirely. Bass build should use MinHook or move E:GHOST handling in-epilogue.
2. No pause gate. Epilogue + warp machine never check board+0x874, so phases advance during ESC pause while native entities freeze. One early-return at handler top.
3. No quit gate. DETACH cleanup is too late (board/ball freed first) -> epilogue use-after-free on exit. Check App+0x159 at handler entry.
4. get_board path App+0x220 -> +0x0C differs from usual App+0x178. The board+0x878==app sanity saves crashes but failure is silent (no ghosts, no warp). Verify per mode: TT / Tournament / Party.
5. Footprint: 7+ patches, owns 0x46C1F1, so conflicts with standalone ghost_saver / ghost_event / warp mods. Document as either-or.
6. WARP_TRIGGER_DIST_SQ 625 (25 units) is tight at speed; Tournament BTT-creation timing is the fragile link. Crash-test warp-during-tournament transitions, not just 35s title survival.

## Recommendation

Ship the HB+ build as primary (no SEH trampoline, no loader-lock init thread, free menu toggle). Keep bass build only for no-HB+ users. Next small patches: pause + quit gates (2 lines each), DCE replacement, per-mode board verification.
