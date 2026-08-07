v55n_38 - TimeButton popup shows "+5" instead of "+0" (ScoreObject slot fix)
------------------------------------------------------------
BUG: The "EXTRA TIME:" popup showed "+0" even though the +500 bonus was granted
(confirmed by press.log: REWARD +500, countdown 5313->5813).

ROOT CAUSE (Ghidra, ScoreObject render 0x44C160):
  ScoreObject's render reads *(param_2 + 0x20) as the displayed number, then
  divides by 100. Native UpRaceCollisionEvents passes param_2 = App+PID*0xA0+0x5CC
  (the TIMER BASE), so +0x20 lands on the +0x5EC bonus field = 500 -> "+5".
  v55n_37 passed param_2 = App+PID*0xA0+0x5EC (the bonus field directly),
  so +0x20 read +0x60C (garbage -> 0 -> "+0").

FIX (v55n_38):
  Pass base + 0x5CC (the timer base, same as native) to ScoreObject_ctor.
  The popup now reads the correct +0x5EC bonus = 500/100 = "+5".
  Added native Timer_Decrement parity (+0x10 = +0x1C-100, +0x2A=1) so the
  popup auto-dies after 100 ticks instead of lingering.

NOW: press the button -> popup shows "EXTRA TIME: +5", countdown jumps +500.
v55n_37 - TimeButton EXTRA TIME is now VISIBLE: bump the real clock + ScoreObject popup
------------------------------------------------------------
USER (v55n_35 logs): reward logged but no visible extra time. v55n_36 dropped the
single-player gate but still wrote the WRONG field.

ROOT CAUSE (Ghidra, Board_UpdateRaceState 0x41B080):
  The visible race clock / HUD countdown is NOT App+PID*0xA0+0x5EC.
  board+0x362C holds per-player timer POINTERS (App+PID*0xA0+0x5CC). The game
  DECREMENTS the field at ptr+0x1C = App+PID*0xA0+0x5E8 every frame, and THAT
  (+0x5E8) is what the HUD displays. The +0x5EC field we were writing is only a
  game-over GATE (checked <1), never shown on screen.
  Native UpRaceCollisionEvents writes +0x5EC=500 AND pops a visible
  EXTRA TIME ScoreObject (ScoreObject_ctor 0x44BE80, appended to board+0x8b8).
  The popup is the ONLY visible feedback on press.

FIX (v55n_37):
  1) ADD 500 to the countdown slot App+PID*0xA0+0x5E8 (the real visible clock).
  2) Write +0x5EC=500 too (native parity).
  3) Spawn the native EXTRA TIME ScoreObject popup via ScoreObject_ctor
     (0x44BE80) and append it to board+0x8b8 (the game reward/render list) so
     the reward is actually SEEN.
  No single-player gate: works in Warm-Up, Time Trial, AND Tournament.

NOW: press the button -> the on-screen clock jumps +500 and a floating
EXTRA TIME popup appears. Wine title 47s clean. bass.dll md5 5a7d55cb.
v55n_36 — TimeButton: tournament EXTRA TIME + reused-board respawn fix
------------------------------------------------------------
USER (v55n_35): everything works, no crash, but no EXTRA TIME in Tournament;
and the button was missing during one tournament run (appeared on restart).

ROOT CAUSES (from the press.log: 2 PRESSED lines, only 1 REWARD):
  1. EXTRA TIME skipped in tournament: the reward write was gated on
     profile+0x10==0 && profile+0x11==0 (native single-player gate). In
     tournament one of those flags is set, so the +500 write silently
     never ran (the 2nd PRESSED in the log got no REWARD line).
     FIX: drop the single-player gate. Always grant +500 to the pressing
     player (player_idx*0xA0 + 0x5EC + App), with player_idx range-guard
     and a player-0 fallback if the slot is unreadable. The timer-slot
     field is the same the game decrements in every mode.
  2. Button missing on some tournament loads: the spawn gate keyed only on
     board == g_spawned_board, but board pointers are REUSED across
     tournament races, so a same-board new race looked already-spawned.
     FIX: also track g_spawned_level and gate on BOTH (level is read fresh
     before the gate). A new race with a reused board gets a new level
     pointer -> re-spawns.

bass.dll md5 5c7a55438cf3005678b3c7958e829a03. Wine title 56s clean.
v55n_35 — TimeButton: FULL native press behavior re-enabled
------------------------------------------------------------
USER (v55n_34): press log appeared, NO crash. Root cause confirmed:
the stray shared-logf write in the press branch (two-thread FILE* race)
was the crash. v55n_34 = stable + press fires.

v55n_35 re-enables the REAL TimeButton behavior (was #if 0 since v55n_33):
  - +0x10E4 = 1 latch, +0x10E5 = 1 pressed pose, +0x10D8 -= 20.0 sink
  - press sound on the NATIVE channel (App + 0x510, per Rotator_TriggerSound
    0x436CF0; v55n_29 used the wrong +0x4A8 chomper channel)
  - single-player reward: timer slot (player_idx*0xA0 + 0x5EC + App) = +500
    EXTRA TIME
All logging stays in the thread-private press.log (shared-logf race must
not return). tb->pressed mod flag restored.

bass.dll md5 7d287522349f6edd5de59030aca53537. Wine title 52s clean.
v55n_34 — TimeButton root cause: stray shared-logf write in press branch
------------------------------------------------------------
v55n_33 STILL crashed (PRESSED fired at heart=728, crash ~6s later, ntdll
0001:0003F0D5, Draw, 18s). Root cause of the crash is now traced precisely:

The press branch had TWO writes to the SHARED main logf FILE* running on
the PRESENT (main) thread:
  1. the fprintf(logf,...) I removed in v55n_33 (the first one, replaced by
     press.log), AND
  2. a SECOND stray fprintf(logf, "TimeButton pressed by proximity"...) at
     the very end of the press branch (line ~5505) that I MISSED.

The background (entity) thread ALSO writes logf every frame with NO lock.
TWO threads writing the same msvcrt FILE* without synchronization races its
internal buffer/offset pointers -> silent heap corruption that does not trip
immediately; it corrupts an allocation and crashes LATER in ntdll during a
subsequent Draw (the 0001:0003F0D5 ~6s after the press fired). The catapult
helper (proven stable) never writes shared logf from Present; only TimeButton
did.

v55n_34 removes that final stray fprintf(logf,...). Now the PROXIMITY PRESS
does NOT touch the shared logf at all; the only file write is the dedicated
thread-private mknp_custom_entities_press.log (open/write/close in-branch).

  - Expected: drop on button -> PRESSED fires (press.log) -> NO crash.
  - If it STILL crashes, the remaining press-time writes are only
    tb->pressed=1 (mod-static) + one self-contained press.log write, so it
    would then be the proximity logic itself, but given catapult/other
    helpers run the same proximity pattern crash-free, the shared-logf race
    is the overwhelmingly likely root cause.

bass.dll md5 5cefde22fc10d0079f1f638257be8d64. Wine title 49s clean.
v55n_33 — TimeButton: press re-enabled, ALL shared-logf/tb.log I/O removed
------------------------------------------------------------
USER (v55n_32): dropped on button -> dizzy, NOTHING else, NO crash.
   => The helper was NOT called at all and the button stayed fully solid.
      The button + collider + ball-resting-on-it are EXONERATED (confirmed
      twice: v55n_27 fell on it dizzy no crash; v55n_32 same).
   => The crash is ENTIRELY inside the press-helper code path.

v55n_31 (press enabled) crashed: the press branch wrote the SHARED main
logf FILE* from the Present hook, while the background entity thread ALSO
writes logf every frame with no lock. v55n_32 (helper off) had ZERO file
I/O -> stable. v55n_33 removes ALL tb.log churn (df stays NULL) and logs
the press to a dedicated thread-private mknp_custom_entities_press.log,
NEVER touching the shared logf from the Present hook.

  - If STABLE (press fires, no crash) -> racing FILE* writes were the bug.
  - If still CRASHES -> the remaining press-time writes are just
    tb->pressed=1 (mod-static) + one fopen/fprintf/fclose on press.log,
    narrowing it further.

bass.dll md5 81344e93dfec9340b669b3f081bbf4b4. Wine title 46s clean.
v55n_32 — TimeButton A/B: press helper DISABLED (collider kept solid)
------------------------------------------------------------
USER (v55n_31): no sound, still crashes 0001:0003F0D5 ntdll at 18s on press.
The tb.log heartbeat stopped at frame 720 (~12s) then the game froze and
crashed at 18s.

Three hard facts:
  1. v55n_31 press writes NOTHING to game memory (no latch, no reward, no
     sound, no sink, no pose - only tb->pressed mod-struct + a log line).
     Yet it still crashed. So the crash is NOT any press write.
  2. N:EXTRATIME has only ONE xref in the exe -> UpRaceCollisionEvents.
     Warm-Up never routes it, so the earlier rename idea would be a no-op
     on Warm-Up and I removed it (also I cannot confirm press fired, because
     the main-log PRESSED fprintf is buffered and lost on hard crash).
  3. Your v55n_27 already proved ball resting ON the solid button is safe
     (you fell on it, got dizzy, no crash).

So the remaining question is PRESS-PATH vs COLLIDER. v55n_32 answers it:
the button stays SOLID and registered exactly as v55n_24-31, but the
proximity-press helper call is disabled entirely. 
  - If STABLE  -> the press code path (even log-only/no-write) is the trigger.
  - If CRASHES -> it is the ball interacting with the registered collider,
                  and solidity must be rebuilt without the case-39 registration.

bass.dll md5 864648dcddbe3e10f60b228fe6741bb3. Wine title 44s clean.

v55n_31 — TimeButton press: DECISIVE isolation (press writes NOTHING but log)
------------------------------------------------------------
USER (v55n_30): no sound plays; STILL crashes 0001:0003F0D5 ntdll at ~17s.
v55n_30 had latch + reward only (sound #if 0, no sink, no pose) -> crash
persists identically. That exonerates sound, sink, and pressed-pose.

IMPORTANT CORRECTION on the crash address: MODULE = ntdll.dll, so
0001:0003F0D5 is an ntdll-only heap offset (corruption inside an alloc/free
routine during Draw). It does NOT map to the exe (my earlier Catapult_Update
0x43F080+0x55 read was WRONG - that only makes sense if the crash were
module 1 = exe, but the header names ntdll.dll). So this is generic heap
corruption surfacing in Draw, consistent with a write-behind-object.

v55n_31 therefore does the CLEANEST possible test: on press it writes
NOTHING to memory except the mod-side tb->pressed flag + a log line.
NO +0x10E4 latch, NO reward write, NO sound, NO sink, NO pose.
The entity stays completely untouched. If it STILL crashes on press ->
the crash is NOT from any press-time memory write; it is from the ball
CONTACTING the solid TimeButton mesh during Draw (collision/render path).
If it does NOT crash -> the latch or the reward write (or a combo) was the
culprit and we re-add ONE at a time.

NOTE: press now logs unconditionally to the MAIN log so a fired press that
crash-renders is still captured (subject to stdio buffer flush on crash).

bass.dll md5 e8a458c1c697b02474403538e669073c. Wine title 44s clean.

v55n_30 — TimeButton press: ISOLATION TEST (must find real crash cause)
------------------------------------------------------------
User (v55n_29): press fires but crashes. ntdll 0001:0003F0D5, Draw, ~16s.
KEY EVIDENCE: crash address is IDENTICAL across v55n_28 (press channel
+0x510) and v55n_29 (chomper channel App+0x4A8). Two DIFFERENT sound
channels, SAME crash -> the sound channel is NOT the culprit. v55n_29 even
played the chomper sound (App+0x4A8 = chomper channel) then crashed the
same way. The common press action across both builds:
  +0x10E4=1 (latch), +0x10E5=1 (pressed-pose flag), +0x10D8-=20 (sink).

Also noted: v55n_29 tb log never showed a PRESSED line, yet the sound
played -> the press fired on a NON-30-frame tick when the throttled tb log
(df) was NULL, so the press line was missed. Press now ALWAYS logs to main
log too.

v55n_30 ISOLATES: press now does ONLY latch +0x10E4=1 + reward. NO sound,
NO +0x10E5 pressed-pose flag, NO +0x10D8 sink. If it stops crashing -> the
pressed-pose flag or sink (or a Sound_Play3D side-effect) was the cause;
then re-add one at a time. If it STILL crashes -> the mere act of latching
a SpeedCylinder-constructed object (or the reward write) is the cause.

bass.dll md5 2565fef18e5ffa8adf35d305e15c33e3. Wine title 44s clean.
tb/main log will now definitively show the press each time.

v55n_29 — TimeButton press: FIX wrong sound channel (crash on press)
------------------------------------------------------------
User (v55n_28): press FIRED (heard the sound, latch set) but the game
crashed the moment it happened:
  ntdll.dll / CRASH_ADDRESS 0001:0003F0D5 / CURRENTOPERATION: Draw / ~22s

ROOT CAUSE: the press sound used the WRONG channel resolution.
  board+0x878 IS the App pointer (BOARD_APP = 0x878), NOT a sound list.
  My press block did:
     snd_list = *(DWORD*)(board + 0x878);   // this is APP
     ch = *(DWORD*)(snd_list + 0x510);      // App+0x510 = garbage
  and passed that garbage as the sound channel to Sound_Play3D(0x459860),
  corrupting the heap -> ntdll crash during Draw.

FIX: mirror the PROVEN catapult pattern (cEnt_catapult_present_check lines
8093/8106, known-working):
     app = *(DWORD*)(board + 0x878);        // BOARD_APP
     ch  = *(DWORD*)(app + 0x4A8);
     if (ch valid) Sound_Play3D(ch, ...)

The reward block was already correct (app_tb = board+0x878). Only the sound
channel was wrong. Same wrong-channel code also exists in the DEAD
DispatchCollisionEvents N:EXTRATIME handler (line ~2020), but that hook is
NEVER installed in this build (install_bonk_collision_hook orphaned), so it
cannot fire and is harmless.

So the ~30u-arc fix from v55n_28 CONFIRMED the trigger works and the press
latched; this build makes the accompanying sound safe so it does not crash.

bass.dll md5 afb779dfe7b255a4ce0c0804f89eaa13. Wine title 44s clean.
EXPECT on Warm-Up: roll over button -> PRESSED, sound plays, NO crash,
EXTRA TIME reward (single player).

v55n_28 — TimeButton press: widen vertical+radius (diag proved why)
------------------------------------------------------------
User tb log (v55n_27) is GOLD — it proved the press gate math, not the hook:
  TB: 810 ball=(767.4,-114.4,-508.7) horiz=17.7  <- ball DEAD-CENTER over button
  TB:   [0] vertical window miss (dy=115.9 need [-60,60])

The ball was right on top of the button (17 units horizontally!) but failed
the vertical window: ball center sat up to ~116 units above the button ref Y
(-230.4) as it arced over. My [-60,+60] window was too tight to ever catch it.

FIX: TIMEBUTTON_PRESS_RADIUS_SQ 900->1600 (40u), DY range [-60,+60]->[-160,+160].
Frame 840 (dy=61.7, horiz=27.3) now presses. Trace shows the ball clearly
passes through the gorilla-tape-sized envelope on its arc.

Also: (r=0) in log is just %d printing the float constant 900.0f as int --
real compare drove the log correctly (horiz actually vs 900 -> the failures
were real distance too far, not int-truncation). Cosmetic only, harmless.

bass.dll md5 c01af6572039cebfd14148a9cc18504e. Wine 55s clean.
PRESS still via main-thread proximity, one-shot latch. Expect: roll over
button -> PRESSED line + sink + EXTRA TIME reward on this build.

v55n_26 — TimeButton press fixed via MAIN-THREAD PROXIMITY (no game detour)
------------------------------------------------------------
STATE: button is SOLID, renders, does NOT rotate (v55n_25). Only press was dead.

ROOT CAUSE of dead press:
  1. install_bonk_collision_hook() (which patches DispatchCollisionEvents
     0x40C5D0 and contains the N:EXTRATIME press handler) is defined but
     NEVER called anywhere in the code — only uninstall is. So the hook is
     never installed and the press handler never runs.
  2. EVEN IF installed: my v53g-2 notes prove a MANUAL trampoline on that SEH
     function (PUSH 0xFF + MOV EAX,FS:[0] prologue, 8 bytes) crashes when the
     hook fires. The proper fix needs MinHook, not a hand-built trampoline.

FIX (v55n_26): replicate the press by PROXIMITY on the MAIN THREAD in the
Present hook — the exact safe pattern catapult/speedcyl/waterwheel already
use (gated on board+0x874 pause flag). When the ball is within ~30 units
horizontal and a vertical window of the button (sink depth 20.0f -> 60-unit
window), it presses:
  +0x10E4 = 1 (latch), +0x10E5 = 1 (pressed pose),
  +0x10D8 -= 20.0f (sink), press sound (sound list +0x510),
  single-player reward: timer slot (player_idx*0xA0+0x5EC+App) = 500.
One-shot (latch stays 1, matching native one-shot buttons).

Expected on Warm-Up / any level:
  - Roll over / near the button -> it sinks, presses
  - "EXTRA TIME:" reward + timer +500 (single player)
  - Still solid, still no rotation
Binaries: bass.dll md5 cd6f8e19b71ec3e91f0fc5e905e53285. Wine title 49s ALIVE.

v55n_25 — TimeButton (SpeedCyl shape) + native TimeButton vtable override
------------------------------------------------------------
v55n_24 WORKS: your log showed the button spawns, is solid, level runs, NO
crash. Only bug: it rotated slowly (SpeedCylinder idle spin). Root cause: v55n_24
used SpeedCylinder_ctor, whose vtable (0x4D57D0) drives the continuous rotation
(slot 11 = 0x43D8C0) — exactly what a speedcylinder does when idle.

v55n_25 keeps the PROVEN stable SpeedCylinder_ctor construction (alloc 0x150C,
Stands_ctor, Level_RenderCtor collision Level at +0x10E0, full case-39
registration: board+0x10EC + scene tree + MeshBuffer+0x47C + self-ref) but
overrides the vtable to the native TimeButton vtable (0x4D5830) so the
per-frame Update uses TimeButton slots (RenderOnce 0x43DC40) instead of the
SpeedCylinder spin. obj is 0x150C bytes >= TimeButton 0x10E8, so every
TimeButton slot reads in-bounds. Also restored +0x10E5=1 (render-once flag)
that SpeedCylinder_ctor overwrote, matching the native TimeButton_ctor state.

Press still works: the mod-side N:EXTRATIME handler matches vtable 0x4D5830
and the +0x10E4=0 unpressed latch byte (0 from the speed float write).

Expected on Warm-Up / any level:
  - Button appears, does NOT rotate
  - Solid (press it, ball bounces off)
  - Pressing triggers EXTRA TIME: popup + timer (500)
Binaries: bass.dll md5 1ec117e78a809e6a2910a539332f9316. Wine title 42s ALIVE.

v55n_24 — TimeButton via PROVEN SpeedCylinder_ctor (decisive test + candidate fix)
------------------------------------------------------------
Your v55n_23 log was the pivot. Mesh swap CHANGED the crash:
  LevelUp-Button      -> ntdll heap corruption 0001:0004717E (~8s)
  LevelUp-SpeedCylinder -> exe crash 0001:000527F1 = VA 0x4527F1 (corrupt EIP,
                           objdump: lands mid-data = EIP pointing at garbage)
Both at Update ~8s, both with the NATIVE TimeButton_ctor (0x436C10).

Decompiled both ctors side-by-side. They are nearly IDENTICAL:
  TimeButton_ctor 0x436C10: Stands_ctor -> vtable 0x4D5830 -> pos ->
                            Level_RenderCtor collision Level at +0x10E0 ->
                            +0x10E4=0, +0x10E5=1. NEITHER appends a board list.
  SpeedCylinder_ctor 0x436A20: same Stands_ctor + Level_RenderCtor at +0x10E0,
                            +0x150C bytes.
Only real differences: vtable + a few field inits.

CONCLUSION: the native TimeButton_ctor path itself is broken when spawned on a
NON-race board (Warm-Up / any level). The mesh only changes WHICH heap block
breaks. Since SpeedCylinder_ctor is PROVEN crash-free + solid on ANY level
(case 39 works on your machine), v55n_24 spawns the TimeButton using
SpeedCylinder_ctor with FULL case-39 registration, feeds it the BUTTON mesh,
and tracks it as a TimeButton (mod-side N:EXTRATIME press handler).

Also fixed: an orphaned duplicate alloc (TIMEBUTTON_SIZE + SPEEDCYLINDER_SIZE)
that a poor edit had left — would HAVE leaked heap; removed. (This bug was NOT
in shipped v55n_22/23; it only appeared during this edit.)

Outcome oracle:
  - STABLE -> TimeButton_ctor was the culprit; keep this. Button solid+pressable
              on any level.
  - CRASHES -> the button MESH itself (LevelUp-Button geometry) is broken on
              non-race levels -> swap TimeButton to a custom geometry.
Binaries: bass.dll md5 b3bd13acfa07499ddf64c977d2bc589a. Wine title 42s ALIVE.

v55n_23 — mesh isolation: TimeButton uses the WORKING SpeedCylinder mesh
------------------------------------------------------------
Your v55n_22 log + crash header was the key correction:
  MODULE: C:\Windows\SYSTEM32\ntdll.dll
  CRASH_ADDRESS: 0001:0004717E
So 0001:0004717E IS ntdll.dll. My "it is EXE 0x44717E (ScoreDisplay dtor)"
correction was WRONG — I took the 0x4717E offset and blindly added 0x400000,
but module index 1 in the crash header = ntdll, not the exe. The original
"ntdll at Update" reading was right. Sorry for the detour.

What v55n_22 PROVED (ZERO registration, still crashed same addr ~8s):
  - MeshBuffer+0x47C write       -> NOT the cause
  - board+0x10EC + scene tree    -> NOT the cause
  - self-ref +0x47C              -> NOT the cause
  - translate / board+0x2578 / press path (already ruled out earlier)

What is LEFT that differs from the WORKING SpeedCylinder (case 39):
  - ctor fn      : TimeButton_ctor 0x436C10  vs SpeedCylinder_ctor 0x436A20
  - mesh         : levels\LevelUp-Button vs levels\LevelUp-SpeedCylinder

v55n_23 changes ONLY the mesh to LevelUp-SpeedCylinder (keeps TimeButton_ctor
+ native vtable + zero registration). Outcome:
  - STOPS crashing -> LevelUp-Button geometry is the culprit
  - STILL crashes  -> TimeButton_ctor/vtable path is the culprit

Binaries: bass.dll md5 ba53204e0fb653751b3bec0965fc8057. Wine title-screen
46s ALIVE (cannot reach level start; real verdict on real Windows).

v55n_22 — TimeButton ZERO-REGISTRATION isolation test (trial-and-error mode)
------------------------------------------------------------
MAJOR CORRECTION: 0001:0004717E is NOT ntdll.dll. Module 1 = the MAIN EXE,
so VA = 0x44717E. Disasm shows 0x44717B = `movb $0x3,0x14(%esp)`, crash at
byte 4 = MID-INSTRUCTION / EIP corruption. The function is ScoreDisplay_dtor
(0x4470D0, vtable 0x4D67E8) = the "EXTRA TIME:" score popup teardown. It walks
obj+0x1140/0x1124/0x1138/0x110C/0x10EC/0x10D0 calling free + BaseObject_Cleanup.
It runs a frame AFTER spawn (log ends "No GRID points found"). So the game is
destroying a ScoreDisplay at spawn and the return-address chain is corrupted.

Trial-and-error premise: every one of v55n_16..21 had SOME mod registration
active (MeshBuffer+0x47C write, board+0x10EC+scene append, self-ref). We have
NEVER tested the native TimeButton_ctor + mesh + native vtable COMPLETELY ALONE
with zero mod interference. SpeedCylinder registers and works, so registration
alone is not the cause, but v55n_22 removes EVERYTHING:
  - alloc + memset + pfn_TimeButton_ctor(obj,board,px,py,pz,mesh)
  - write +0x10D4/8/C = spawn pos
  - track in g_timebuttons[]
  - NO MeshBuffer+0x47C write, NO board+0x10EC/scene-tree append, NO self-ref.

Outcome oracle:
  - STOPS crashing -> registration step is the trigger -> re-add one piece at
    a time (MeshBuffer+0x47C next).
  - STILL crashes  -> the native ctor+mesh+vtable path itself is broken here
    -> abandon native TimeButton_ctor; use the crash-safe PopCylinder-visual +
    mod-side-press pattern.
Binaries: bass.dll md5 1c3189a6aabb0c3f6cff36dbcfeb838e (source only; Wine
title-screen 44s ALIVE -- Wine cannot reach the level-start spawn path, so the
real verdict is only readable on real Windows).

v55n_21 — TimeButton: TRUE root cause = board+0x2578 append (not geometry)
------------------------------------------------------------
Your v55n_20 log was the breakthrough. It showed:
  `TimeButton OBJ 0x0C6A8FF0 appended to board+0x2578 (native parity)` firing
  RIGHT BEFORE the crash, plus 22x `TBtx tree count=0 (bad)`.

Two facts the log proved:
1. The geometry translate is a NO-OP -- the built collision Level's mw+0x18
   tree is EMPTY at spawn (`tree count=0`). It writes NOTHING, so it cannot be
   the crash. Removing all translate work.
2. The crash line is the board+0x2578 append. That was the "native parity"
   idea from v55n_18 -- WRONG. My own block comment admits it:
   "Board_UpdateRaceState (vtable[19], 0x41B080) iterates board+0x2578 EVERY
   FRAME calling vtable slot 11 (0x43DC40) on each object." On the cEnt object
   that once-guarded chain corrupts the stack -> heap -> ntdll 0001:0004717E.

The proven crash-free sibling SpeedCylinder (case 39) does NONE of this:
- Does NOT append to board+0x2578.
- Does NOT use a private vtable / NOPs / render hook.
- Uses its NATIVE vtable.
- Solidity comes ENTIRELY from registering obj+0x10E0 into board+0x10EC +
  scene tree + MeshBuffer+0x47C + self-ref. NO geometry translate.

Fix (v55n_21):
- Dropped the board+0x2578 append (the actual crash).
- Dropped the private vtable copy + NOPs + render hook -- use native vtable.
- Mirror SpeedCylinder exactly: register obj+0x10E0 into board+0x10EC +
  scene tree, set MeshBuffer+0x47C + self-ref.
- Disabled the no-op translate (log spam + SpeedCylinder does none).

Crash-test: title screen survived 42s. Real Warm-Up test needs user machine.

v55n_20 — TimeButton solidity: switch to catapult-proven saved-originals tree translate
------------------------------------------------------------
The v55n_19 revert was WRONG in the opposite direction. User log for v55n_19
showed a NEW crash: `0001:000570A4` EXE, Draw, `fcomps 0x4(%edi)` — the v55n_15
signature, from the PRE-CTOR cEnt_translate_meshworld_verts(mesh,...) writing
the SOURCE mesh's +0x18 tree items IN PLACE. That corrupts a tree the board
still walks during Draw. ("v55n_14 crash-free" was a false assumption — Wine
never reaches level start, so it was never verified.)

Root cause clarity (two crash states oscillated across versions):
  - Write SOURCE mesh tree in place (pre-ctor)  -> 0001:000570A4 EXE Draw
  - Write BUILT Level strips/buffers per-frame (Present-driver) -> ntdll 0001:0004717E Update
  The PROVEN catapult avoids BOTH: it translates the BUILT collision Level's
  mw+0x18 tree items from SAVED ORIGINALS (idempotent, non-cumulative, never
  touches source mesh or strips). That's cEnt_timebutton_translate_tree.

Fix:
  - REMOVED pre-ctor cEnt_translate_meshworld_verts(mesh,...) (caused 0001:000570A4).
  - RE-ENABLED the Present-driver, now calling cEnt_timebutton_translate_tree
    (the catapult-proven mechanism) instead of cEnt_translate_collision_strips.
    It translates the built collision Level's mw+0x18 tree items from saved
    originals so the collider sits at the spawn pos. Retries until tree items exist.
  - Kept private vtable (slots 1+11 noop), render hook, +0x10EC registration,
    native +0x2578 append.

Crash-test: title screen survived 42s. Godspeed — real Warm-Up button test
still requires user's machine (Wine can't reach level start).

v55n_19 — TimeButton crash FIXED (evidence-backed revert to crash-free baseline)
------------------------------------------------------------
Root cause (git 91f4097e = crash-free baseline, user-confirmed):
  v55n_14 relied on PRE-CTOR cEnt_translate_meshworld_verts(mesh, px,py,pz) and
  had the Present-driver translate #if 0'd  -> NO crash (only non-solid).
  v55n_16/17/18 REMOVED the pre-ctor translate and ACTIVATED a Present-driver
  loop that WRITES to the BUILT collision Level (obj+0x10E0) mesh/tree every
  frame  -> reintroduced ntdll 0001:0004717E at Update (the persistent crash).
  The private vtable copy + render hook + +0x10EC registration were IDENTICAL
  in both and are NOT the cause.
Fix:
  - RESTORED pre-ctor cEnt_translate_meshworld_verts(mesh, px,py,pz) in case 45,
    so Stands_ctor (0x462850) clones the collision trees at the spawn position.
  - DISABLED the Present-driver cEnt_translate_collision_strips loop (#if 0) on
    the built Level — matches v55n_14's proven crash-free state.
  - Kept native-parity +0x2578 OBJ append + +0x10EC registration + render hook,
    other entities untouched.
Definitive evidence: SpeedCylinder (case 39), the crash-free sibling, uses the
native vtable with no Present-driver write; v55n_14 had this loop off.
Crash-test: title screen survived 48s (past audio-init window). Real Warm-Up
button-spawn crash still requires user's machine (Wine can't reach level start).

## v55n_18 — TimeButton CRASH FIX: NATIVE PARITY (real root cause found via Ghidra)
- MAKYUNI tested v55n_17: STILL crashed (ntdll 0001:0004717E, RUNTIME 00:00:08, Update,
  FinishLoad(OK), board=0x0AFB0048). The translate now ran but still silently no-op'd
  (mw+0x30 count=0 -> early return, no "geom translated" line). Same crash as v55n_16 =
  the GEOMETRY PATTERN WAS NEVER THE CAUSE.
- REAL ROOT CAUSE (Ghidra Up_CreateDynamicObjects 0x411911-0x41197D): native TimeButton
  creation does ONLY AthenaList_Append(board+0x2578, OBJ). It NEVER registers obj+0x10E0
  (a Level_RenderCtor output) into board+0x10EC or the scene tree. Native solidity comes
  from the Stands_ctor-cloned obj+0x18 trees reached via the +0x2578 entry during Ball_Update.
- FAIL: from v55n_5 on, the mod injected tb_col (+0x10E0) into board+0x10EC + scene tree.
  Ball_Update iterates board+0x10EC during Update and treated this bare render-ctor
  output as a collision object -> heap corruption -> ntdll 0001:0004717E every time, ~1s
  after spawn (the button renders, then the ball reaches it and Update walks the bogus entry).
  This is the EXACT documented "Level-family col_off=0" crime (render-ctor output != coll obj).
- FIX (v55n_18): append the OBJ to board+0x2578 (native parity), drop the board+0x10EC +
  scene-tree injections entirely. Keep vtable[11] NOP (0x43DC40 crashes on cEnt bare-mesh
  object), keep %u+0x47C refs for reward dispatch.
- APOLOGY: misdiagnosed as geometry for 5 versions; the log's constant ntdll 0001:0004717E
  at Update with every geometry change was the tell I missed.
- md5: `9f79c9726e53ba54453b62d009db950c`. Wine ALIVE 42s (title only — button un-reachable).
## v55n_17 — TimeButton solidity: GHIDRA-CONFIRMED vertex-source translate (PROVEN pattern)
- MAKYUNI tested v55n_16: still crashed (ntdll 0001:0004717E, ~1s after spawn, Update,
  no "TimeButton pressed" in log). Log showed 26x "TBtx tree count=0 (bad)".
- ROOT CAUSE (v55n_16): my tree-translate read mw+0x18, which is EMPTY on the BUILT
  collision Level -> every call no-ops (count=0) -> button never solid AND the loop itself
  was the wrong structure. APOLOGY: I misdiagnosed by reasoning from memory across several
  versions instead of decompiling. Ghidra settles it.
- GROUND TRUTH (Ghidra D3DXSkinMesh_CopyStripData 0x45E0E0): the render walks the built
  Level's MeshBuffer list at mw+0x2C (count mw+0x30, items mw+0x438) — and it IS populated
  (that's how the button draws). The collision query reads those sub-mesh/strip SOURCE verts.
- FIX (v55n_17): translate the BUILT collision Level's vertex SOURCE (sub-mesh +0x448 +
  strips) ONCE from the spawn offset, retried each Present frame until buffers build, gated
  on pause. NO game-owned tree writes (the v55n_8/11/15 crash cause). Uses the Ghidra-confirmed
  offsets. This is the proven catapult mechanism.
- HONESTY: the v55n_16 crash module is the USER's Windows ntdll; Wine's ntdll at the same RVA
  is a different build, so I cannot claim the exact instruction. The solidity fix is now
  grounded in Ghidra regardless.
- md5: `6bc3e497bbd61ed50c54ce630fdb390b`. Wine ALIVE 42s (title only — button un-reachable).

## v55n_16 — TimeButton solidity via PROVEN catapult pattern; REVERTED the crashing v55n_15 write
- MAKYUNI tested v55n_15: CRASH `0001:000570A4` (FinishLoad, 8s). Note 0001:000570A4 = VA
  0x40570A4 = `fcomps 0x4(%edi)` — an FPU compare against a tree-item pointer.
- ROOT CAUSE + REGRESSION: v55n_15 translated the freshly-loaded SOURCE mesh's +0x18 tree items
  IN PLACE pre-ctor. That corrupts a tree the board still walks -> FinishLoad faults on the items.
  v55n_14 (tree write never ran — gated behind mb_count=0) was NON-SOLID but had NO crash. The
  in-place source-mesh tree write IS the crash, same class as v55n_8/v55n_11 HEAP CORRUPTION.
- FIX: removed the pre-ctor source-mesh translate (case 45). Ported the PROVEN catapult pattern:
  translate the BUILT collision Level's mw+0x18 tree items (mw = *(DWORD*)(collLevel+0x08))
  NON-CUMULATIVELY from saved originals every frame (idempotent, crash-free). New fn
  cEnt_timebutton_translate_tree(), driven from Present hook, gated on board+0x874 (pause).
- This mirrors how the catapult (PROVEN solid, zero crashes) writes collision — it NEVER mutates
  a mesh in place; it rewrites the built Level's tree from saved copies.
- md5: `542510425fb77704bdd9df3e0136bdc9`. Wine ALIVE 42s (title only — button un-reachable).

## v55n_15 — TimeButton SOLID — translate the +0x18 COLLISION TREE (not the strips) pre-ctor
- MAKYUNI tested v55n_14: STILL NON-SOLID. Log: `TBtx mb_count=0 (AthenaList@+0x2C count+0x4)` then silent return.
- ROOT CAUSE (proven from native binary, NOT a guess): Stands_ctor (0x462850) at 0x462937 does
  `add $0x18,%edi` then reads `[edi+0x4]` (count) and `[edi+0x40c]` (items) — i.e. **mesh+0x18 is
  an EMBEDDED AthenaList (count +0x4, items +0x40C)** and Stands_ctor CLONES every item from it into
  the built obj+0x18 tree. Those cloned items ARE the collision (broad-phase + exact test).
- THE v55n_14 BUG: the translate fn checked the **+0x2C MeshBuffer/strip list FIRST**, found it EMPTY
  at load, and `return 0`ed BEFORE ever reaching the +0x18 tree block. So the tree was never translated.
- v55n_15 fix: reorder so **+0x18 collision tree translates FIRST and UNCONDITIONALLY** (count +0x4,
  items +0x40C, each item +0/+4/+8 = world pos; add dx/dy/dz). +0x2C strip/submesh translate still
  runs after as belt-and-suspenders but no longer gates correctness. +0x18 and +0x2C are DIFFERENT
  structures — never gate one on the other.
- md5: `d4c59e2925b5c1bd1319308b43485f15`. Wine ALIVE 42s (title only — button spawn not reachable).

## v55n_14 — TimeButton SOLID — found the offending offset via MAKYUNI's log; fixed +0x2C list + tree pre-ctor
- MAKYUNI tested v55n_13: STILL NON-SOLID. Her log captured the EXACT failure:
  `TBtranslate meshworld enter mw=0x0C78EE50` → `TBtx mb_count=0` → silent return.
  The pre-ctor translate FIRED but read the WRONG MeshBuffer list offset.
- ROOT CAUSE READ FROM LOG: I translated the mesh before the ctor (good — Stands_ctor
  clones its trees at ctor), but read the MeshBuffer list at mw+0x30/mw+0x438 — which on a
  standalone loaded MeshWorld reads a PACKED-buffer/PADDING field = 0. So `mb_count=0`,
  no strips/tree translated, button stayed baked at origin -> non-solid.
- FIX (v55n_14):
  1. MeshBuffer list read as the EMBEDDED ATHENALIST at mw+0x2C (count +0x4, items +0x40C)
     — the PROVEN catapult offsets (cEnt_catapult_rotate_collision_verts).
  2. ALSO translate the mesh's +0x18 spatial TREE items — Stands_ctor (0x462850) CLONES
     mesh+0x18 into obj+0x18 at 0x462937/0x462951 (call 0x4532b0); that clone is the
     solidity source. Translating strips alone still leaves the cloned tree at origin.
     (Tree items: embedded AthenaList count +0x4, items +0x40C, each +0/+4/+8 = pos.)
  3. Pre-ctor only (kept) — translates BEFORE TimeButton_ctor so Stands_ctor builds the
     tree at the spawn point. Safe: own freshly-loaded mesh, not yet in any game list.
  4. Present-driver translate (#if 0 DISABLED) — now redundant; would double-translate.
- Zero game-owned tree writes (no crash; v55n_8/v55n_11 cause avoided). Wine ALIVE 42s.
- NOTE: v55n_13 shipped BEFORE this analysis was possible — its only functional change was
  granular logging. v55n_14 is the real fix.
- MAKYUNI tested v55n_12: NO crash, but STILL NON-SOLID. Log had no "geom translated"
  line (the Present-driver translate silently early-returned / never reached).
- DECISIVE native decompile (TimeButton_ctor 0x436C10 + render-once 0x43DC40):
  * Solidity comes from Stands_ctor (call at 0x436C32) cloning the mesh's spatial
    trees into obj+0x18 AT THE TIME OF CONSTRUCTION, at the mesh's MODELED position.
  * The native render-once (0x43DC40) only repositions via a Timer world-matrix; it
    does NOT touch collision geometry at all.
  * So a button is solid iff its MESH is modeled where it spawns. The cEnt
    LevelUp-Button mesh is baked near-origin but spawned at (778.5,-230.4,-522.5),
    so obj+0x18 collision trees conclude AT ORIGIN -> non-solid.
- WHY earlier attempts failed:
  * v55n_8/v55n_11 wrote COLLISION TREE ITEMS after the tree was built -> moved the
    items far outside the octree's cached AABB bounds -> the query's broad-phase
    pruning read them invalidly -> heap corruption crash 0001:0004717E. The catapult
    survives only because it ROTATES in place (items stay near the original bounds).
  * v55n_9/v55n_12 translated source AFTER ctor (too late — tree already built) OR
    from a render/Present hook that never fired/silently failed.
- FIX: NEW cEnt_translate_meshworld_verts() translates the loaded MeshWorld's vertex
  source (sub-mesh +0x448 + strips) by (px,py,pz) BEFORE calling TimeButton_ctor.
  Stands_ctor then clones obj+0x18 trees AT THE SPAWN POSITION -> SOLID, zero
  game-owned tree writes, zero crash, no hook needed. Octree bounds are correct the
  whole time. Each cEnt spawn allocates its own mesh, so this is instance-safe.
- Added GRANULAR diagnostic logging ("TB translate meshworld enter / mb_count / geom
  translated N verts") so if it still fails the next log reveals the exact offset.
- Wine title-screen ALIVE 42s. User on real Windows must re-confirm solid.

## v55n_12 — TimeButton crash FIXED (0001:0004717E) — removed the game-owned collision TREE-item writes
- MAKYUNI tested v55n_11: it CRASHED on level start (ntdll 0001:0004717E, Warm-Up,
  Update, FinishLoad OK). The v55n_11 log was the smoking gun: it translated 8 tree
  items, then immediately crashed.
- Root cause confirmed via objdump: crash EIP 0x44717E is byte-3 of `c6 44 24 14 03`
  (mid-instruction) = SEH-resume of HEAP corruption caused by my writes to the game's
  collision TREE items (coll_level+0x18/0x848/mw+0x18). Writing those EVERY time it runs
  corrupts the heap (v55n_8 crash + v55n_11 crash = same family). The catapult's "tree
  rotation" works on a differently-owned / per-frame-rebuilt structure — copying its
  write pattern to a one-shot translate was WRONG.
- FIX: REMOVED ALL collision-tree writes. v55n_12 translates ONLY the SAFE vertex-source
  arrays (sub-mesh +0x448 source verts + MeshBuffer strip verts) via the guaranteed
  Present hook (gluebie_present_helper), gated board+0x874, retrying until verts>0. The
  collision tree is rebuilt every frame from these source arrays (SpatialTree_ctor
  0x463330 / Ball_Update 0x405E00), so correct source positions = correct tree, with
  zero writes to game-owned tree memory.
- Wine title-screen ALIVE 42s. User on real Windows must re-confirm solid vs non-solid.

## v55n_11 — TimeButton SOLID — root cause found via user's v55n_9 log: render hook never fires
- MAKYUNI's v55n_9 test log was the smoking gun: the button spawns + registers its
  collision Level, but there is NO "tree translated"/"geom translated" line, and the log
  ends at "No GRID points found." => the one-shot geometry translation, parked in the
  vtable[18] render hook (cEnt_timebutton_render), NEVER RAN on a real game.
- Root cause: the cEnt TimeButton (case 45) is never added to board+0xCD4 (render list)
  or board+0x2578 (update list), so its hooked render is never invoked -> the collision
  tree stays baked at origin -> non-solid at the ref point. My v55n_9/v55n_10 fixes were
  in the wrong place (they only ran translation IF that hook fired).
- FIX (v55n_11): drive the one-shot collision-geometry translation from the GUARANTEED
  every-frame Present hook (gluebie_present_helper), gated on board+0x874 not-paused.
  It translates all 3 collision tree lists (coll_level+0x18, coll_level+0x848, mw+0x18)
  + sub-mesh +0x448 source arrays + strips via cEnt_translate_collision_strips, then
  latches geom_translated=1 (retries on 0 built verts, so the render-hook latch can't
  pin it prematurely). Verified SpatialTree_ctor (0x463330) + Ball_Update (0x405E00):
  the collision tree is rebuilt every frame from board+0x8B0+0x18 appended objects'
  +0x18 clones, so a one-shot translate of the Level's own tracks persists across
  rebuilds (the source is already translated).
- Removed the translation from the render hook (it could latch geom_translated with 0
  built verts, blocking the Present driver); the hook keeps only the world-matrix
  position for visuals.
- Log files renamed: `mknp_custom_entities.log` (+ _catapult, _debug) per MAKYUNI.

## v55n_10 — TimeButton SOLID — port the catapult's proven collision translation (all 3 tree lists + submesh + strips)
- MAKYUNI: "port all its functions to our cEnt Timebutton" — mirrored the PROVEN-solid
  catapult collision translation (cEnt_catapult_rotate_collision_verts).
- v55n_9 was STILL NOT SOLID because it only translated sub-mesh +0x448 arrays + strips,
  but SKIPPED the collision tree items — the ACTUAL world-space triangle positions the
  ball query (Mesh_FindClosestCollision 0x465D90) compares against.
- v55n_10 translates ALL THREE tree lists the catapult rotates, at first-render timing
  (safe — tree fully built, via geom_translated flag):
    * coll_level+0x18 (embedded AthenaList, count +0x4, items +0x40C)
    * coll_level+0x848 (same layout)
    * mw+0x18 (MeshWorld's own collision tree, same layout)
  each item's +0/+4/+8 is a world-space position -> all add (dx,dy,dz).
- v55n_8 crashed because it wrote tree items at CTOR time (during FinishLoad, tree
  partially built). render-time is the catapult's safe timing.
- Log files renamed: `mknp_custom_entities.log` (+ _catapult, _debug) per MAKYUNI.

## v55n_9 — TimeButton solidity — REAL geometry translation (no more octree mutation)
- v55n_8's spatial-tree ITEM translation was WRONG and CRASHED at start
  (0001:00043353, 00:00:06, Update) — it mutated octree node data in
  colLevel+0x18/0x848/mw+0x18 which corrupted the collision tree.
- Root cause of "STILL NOT SOLID" found via Ghidra MeshWorld_BuildVertexBuffer
  (0x46F8D0): v55n_6/v55n_7 strip translation read the WRONG list offsets
  (MeshWorld+0x2C count +0x04/items +0x40C are NOT the AthenaList fields).
  Correct: count +0x30, items +0x438. The old code translated 0 verts every time.
- v55n_9 translates the REAL collision geometry at first render (catapult-proven
  timing): sub-mesh +0x448 source vertex arrays (the SpatialTree triangle source)
  + transient +0x10 arrays + MeshBuffer strip vertices, all offset by the spawn
  position. One-shot via geom_translated flag in the render hook.
- ALSO fixed: the MeshBuffer+0x47C entity-write loop used the same wrong
  +0x04/+0x40C offsets, so N:EXTRATIME never found the button entity.
- Crash regression: v55n_9 passes 42s Wine title-screen (v55n_8 was 6s ntdll crash).

## v55n_8 — TimeButton SOLID via spatial-tree translate (real mechanism)

**Still not solid → REAL cause found by decompiling the collision query.** v55n_6
translated the mesh STRIPS, but I verified via Ghidra that
`Ball_AdvancePositionOrCollision` (0x4564C0) and `Mesh_FindClosestCollision`
(0x465D90) read **WORLD-space spatial-TREE ITEM positions** (each item's +0/+4/+8
compared directly against the ball) at `colLevel+0x18`, `colLevel+0x848` and
`mw+0x18` (embedded AthenaList: count +0x4, items +0x40C) — NOT the strips.

This is the SAME mechanism the catapult (confirmed SOLID) uses via
`cEnt_catapult_rotate_collision_verts` — it rewrites those tree-item positions.

Fix: `cEnt_translate_collision_strips` now ALSO translates the tree-item positions
in all three tree lists by (px,py,pz), on top of the strips. Now collision follows
the render hook's (px,py,pz), so the ball hits where the button looks.

## v55n_7 — TimeButton solid (final) + quit crash root-cause fix + X keybind

**Not solid (follow-up)** — the v55n_6 strip-translate fix was correct in concept but the
user still reported non-solid. This release keeps the strip-translate AND adds a
one-shot **quit-time despawn**: TimeButton/SpeedCylinder +0x10E0 collision Levels are
registered in board+0x10EC + scene tree, but a normal Pause→Quit never fires
cEnt_despawn_all_rotaters (that only runs on board CHANGE). So on quit the game's
board teardown freed board+0x10EC while the +0x10E0 Levels were still registered →
double-free → ntdll. v55n_7 calls cEnt_despawn_all_rotaters() once when
game_is_quitting() first becomes true (in gluebie_present_helper), unhooking them
before the board teardown.

Railroaded into the same Present hook so it also covers SpeedCylinder (which has the
identical registration pattern).

**X keybind** — table visibility is now toggled by BOTH T (0x54) and X (0x58),
same 500ms rate limit.

## v55n_6 — TimeButton not solid FIXED (+ quit crash re-fixed properly)

**Not solid** — the button was visible at the ref point (render hook) but the ball
flew through. Root cause: the render hook moves the VISUAL to (px,py,pz) via the
world matrix, but the ball's collision query (Mesh_FindClosestCollision 0x465D90)
reads the +0x10E0 collision Level's RAW strip vertex coords, which are baked at
the mesh's local position (~near origin), NOT the ref point. So collision stayed
at ~(0,0,0) while the visual was at the ref.

FIX: added `cEnt_translate_collision_strips(dx,dy,dz)` — walks the +0x10E0 Level's
MeshWorld meshbuffers and offsets every strip vertex X/Y/Z by (px,py,pz) (same
proven strip-walk pattern as cEnt_catapult_rotate_collision_verts). Now collision
agrees with the render. Called right after the collision Level registration.

**Quit crash (proper fix)** — v55n_5 got the +0x10E5 guard backwards. Ghidra:
`Rotator_RemoveAndFree (0x436FC0)` guard is `if (+0x10E5 == 0) { cleanup }` —
cleanup ONLY runs when the flag is 0. v55n_5 left +0x10E5=1 (ctor default), so at
despawn RemoveAndFree no-oped, the +0x10E0 Level stayed registered in
board+0x10EC + scene tree, and the game's quit teardown double-freed it ->
ntdll 0001:000410C3. v55n_6 CLEARS +0x10E5=0 at spawn (replicating the render-once
transition) AND adds a proper TimeButton despawn to cEnt_despawn_all_rotaters that
calls 0x436FC0 (RemoveAndFree, now active with +0x10E5==0) + the dtor 0x43DC20.

## v55n_5 — TimeButton spawn position + quit crash FIXED

Two regressions from v55n_4, both traced to Ghidra:

1. **Spawned at 0,0,0**: NOP'ing vtable[11] (0x43DC40) also removed the render
   positioning it did via the timer vtable[2] (Gfx_SetPosition). Fix: hook
   vtable[18] (render 0x45E0E0) on the private vtable copy and write the world
   matrix translation (m[12]/13/14) from obj+0x10D4/8/C before calling the
   original — same proven matrix pattern as cEnt_catapult_render.

2. **Quit crash (ntdll 0001:00041106, Pause Menu/Background)**: v55n_3/v55n_4
   removed the +0x10E0 collision-Level registration into board+0x10EC +
   scene tree to stop the in-game crash. But the TimeButton dtor
   (FUN_0043dc20 -> Rotator_Cleanup) ALWAYS frees obj+0x10E0, and
   Rotator_RemoveAndFree (0x436FC0) removes that Level from board+0x8B0+0x18
   and board+0x10EC. With no registration, the scene teardown double-freed the
   Level. Fix: re-register obj+0x10E0 into board+0x10EC + board+0x8B0+0x18 —
   EXACTLY like SpeedCylinder (case 39) — so RemoveAndFree cleanly unhooks it
   once. The in-game crash stays fixed because the vtable[11] NOP prevents
   0x43DC40 from ever firing (registration is safe to re-add).

3. **N:EXTRATIME entity lookup**: the handler matched the button by
   `*(DWORD*)ent == 0x4D5830`, but the private vtable copy breaks that
   equality. Now also accepts a private copy whose slot 0 == 0x43DC20 (dtor).

## v55n_4 — TimeButton crash FIXED (real root cause: vtable[11] 0x43DC40)

The board+0x2578 update loop (Board_UpdateRaceState 0x41B080) calls vtable[0x2C]
(= slot 11) on every object each frame. For TimeButton that is 0x43DC40 (render-once).
Gated on +0x10E5 (set to 1 by the ctor), it fires once and calls a PATCHED slot-22
thunk (0x46DF80: ADD ESP,0x24 + RET 8) that corrupts the caller's stack frame ->
heap corruption -> ntdll.dll 0001:0004717E ~1s after the button spawns. The native
Up button survives because its board+0x478C mesh is pre-loaded with a valid
SceneObject; the cEnt bare MeshWorld is not.

FIX: NOP'd vtable[1] AND vtable[11] (0x43DC40) on a private vtable copy, and cleared
+0x10E5 so the render-once chain can never fire. The no-op returns 1 (keep in list).
Button still renders (vtable[18] static path) and collides (obj+0x18 trees).

## v55n_3 fix (crash) — TimeButton vtable[1] no-op (root cause: Rotator_Update heap corruption)

MAKYUNI reported the cEnt TimeButton crashing ~1s after it appears:
`MODULE: ntdll.dll, CRASH_ADDRESS: 0001:0004717E, CURRENTOPERATION: Update,
CURRENTOBJECT: Board (Warm-Up), FinishLoad(OK)`.

Root cause (Ghidra): the game's update list (`board+0x2578`) calls
`vtable[1]` on every registered object each frame. For the TimeButton
(vtable 0x4D5830) that slot is **Rotator_Update (0x4606D0)**, a vertex-deform
function that allocates `SceneObject+0x43C * 0x60` bytes and writes mesh
vertices into it. The native game survives because its TimeButton mesh
(`board+0x478C`) carries proper vertex/count data. The cEnt loads the button
mesh as a bare `Level_MeshWorldCtor` (0x461510) MeshWorld whose SceneObject
has a 0 vertex count at +0x43C -> Rotator_Update allocates a 0-size buffer
and writes into it -> heap corruption -> ntdll crash during Update.

Fix: replace the TimeButton's `vtable[1]` with a no-op on a private vtable
copy (same pattern as Chomper). The button needs no vertex deformation; its
render (vtable[18] 0x45E0E0) is safe because it checks SceneObject+0x434==0
and falls back to a normal static `SceneObject_RenderFull`. The press sink
(+0x10D8 -= 20) and collision (spatial trees) are unaffected.

Crash-tested on Wine/Xvfb: alive 51s+, clean load. (The Wine env can't reach
the in-race button spawn, so the definitive proof must come from MAKYUNI's
home test.)

## v55n_3 — cEnt TimeButton is now solid and pressable (native Up race logic)

MAKYUNI asked how the Up race TimeButton works; Ghidra deep-dive of the native
chain (TimeButton_ctor 0x436C10, UpRaceCollisionEvents 0x4119B0, press fn
Rotator_TriggerSound 0x436CF0) showed the press is **pure collision**, no force
threshold: the ball touching the N:EXTRATIME tagged mesh presses it, if the
button's +0x10E4 latch is still 0.

Native press (Rotator_TriggerSound):
- +0x10E4 = 1 (one-shot latch)
- +0x10E5 = 1 (render pressed pose)
- +0x10D8 -= 20.0f (button sinks 20 units)
- press sound on sound list channel +0x510

Reward (single-player only, profile+0x10==0 && profile+0x11==0):
- timer slot (player_idx*0xA0 + 0x5EC + App) SET to 500
- "EXTRA TIME:" ScoreObject popup

What the cEnt (ai_type 45) now does, mirroring the proven SpeedCylinder
(v55n_2) native-parity fix:
1. Spawns the real TimeButton_ctor (0x436C10, 0x10E8 bytes, vtable 0x4D5830)
2. Registers the Level_RenderCtor collision Level at obj+0x10E0 into
   board+0x10EC + scene collision tree (SOLID)
3. Sets MeshBuffer+0x47C = entity on the collision mesh MeshBuffers so the
   N:EXTRATIME handler finds the button entity (same link SpeedCylinder needed)
4. Sets entity+0x47C self-ref
5. Tracks buttons in g_timebuttons[]; the DispatchCollisionEvents hook now
   replicates the native N:EXTRATIME press + timer reward on ANY level

Note: the native handler writes the timer to a FIXED 500 (not +500). The cEnt
replicates that exact behavior.

## v55n_2 — SpeedCylinder is now solid and has its native spin/launch behavior

The cEnt SpeedCylinder (ai_type 39) was missing its behavior and was
non-solid. Compared against the native Up race (ctor 0x436A20, vtable
0x4D57D0, 0x150C bytes). Native registers it ONLY in board+0x2578 (update
list); collision comes from the spatial trees cloned into obj+0x18 by
Stands_ctor; the launch behavior lives in vtable slot 11 (0x43D8C0).

The mod's cEnt was missing three things vs native:

1. **MeshBuffer+0x47C -> entity link was never set.** The collision handler
   (UpRaceCollisionEvents 0x4119B0) resolves the entity via
   `[[MeshBuffer]+0x47C]` then calls Pendulum_PlayCollisionSound (0x436B70).
   The mod loads the mesh standalone, so this link was 0 -> handler got a
   NULL entity -> no sound, no ball tracking.
2. **Collision Level at obj+0x10E0 was never registered.** SpeedCylinder_ctor
   creates a Level_RenderCtor at +0x10E0 (like Catapult's +0x10D4). The
   common registration skipped it (col_off=0 for non-Rotator) -> non-solid.
3. **No per-frame slot 11 driver.** The spin-up (0.25 -> 20.0), 175-frame
   hold, and launch at 65.0 + star trail live in vtable slot 11 (0x43D8C0).
   The mod's entities are only in the update list (vtable[1] deformation),
   not slot 11 -> no behavior.

Changes (mirrors the proven-working Catapult pattern):

- **case 39** now registers obj+0x10E0 into board+0x10EC + scene collision
  tree (makes it SOLID), sets MeshBuffer+0x47C = entity on all collision
  MeshBuffers (makes the handler find the entity), sets entity+0x47C
  self-ref, and tracks the entity in `g_speedcyls[]`.
- **Present hook** now drives vtable slot 11 (0x43D8C0) per-frame on each
  tracked SpeedCylinder, gated on board+0x874 (not paused) — the native
  spin-up/launch/star-trail behavior.
- `g_speedcyl_count` reset on level unload.

## v55n_1 — Tarbubble is now a DECORATIVE floating bubble (Ghidra-verified)

Native TarBubble was misunderstood: it is **not** a tar trap. Deep-dive of
DizzyBoard_Update (0x41D512) + Master (0x420DA0) + the bubble object
(vtable 0x4D6E48, ctor 0x44FB50) shows it is a purely **decorative** S1 ref
point that occasionally spawns a 0x1C-byte bubble which shrinks, floats up,
and pops. The slowdown is Gluebie (ai_type 43); the sinking is Tarpit
(ai_type 44).

Changes:

- **ai_type 25 (Tarbubble)** no longer sinks/traps the ball. It now spawns
  a real native bubble object (ctor 0x44FB50, vtable 0x4D6E48) at the ref
  point, self-driven from the Present hook on ALL boards:
  - ~12%/frame spawn chance, 6-25 frame cooldown between bubbles
  - each bubble: scale ×0.95/frame, lifetime 25-50 frames, rises
    scale×60/frame, pops (bubble1 sound via app+0x488) when done
  - native dtor (0x44FD40) frees the object on pop AND on level unload
    (no leaks)
  - pause gate (board+0x874) freezes bubbles during ESC menu
- **Gluebie (43)** unchanged — still slows the ball (velocity ×0.95)
- **Tarpit (44)** unchanged — still sinks + in_tar death

## v55m_51 — Debug tables show the LIVE rotation accumulator

The debug string tables showed mostly `1.00` for rotation values because they
displayed `obj+0x10E8` (the per-frame angle delta, capped at exactly 1.0 by
v55m_50) and `obj+0x10EC` (direction, rewritten to 1.0 every frame). But the
value that **actually drives the visible spin** is `obj+0x10E4` — the
accumulator in degrees — which wasn't being read.

Now:

- **Angle X** (properties table) shows the live accumulator `obj+0x10E4`
  (degrees) — it climbs as the object spins.
- **Rotation angle** (properties table) also shows the accumulator.
- **Accumulated angle** (docs table, `3 - Updates`) now reads the accumulator
  too (it was wrongly showing the delta before).
- **Angle Y** keeps the per-frame delta, **Angle Z** keeps the direction
  multiplier, and **Scale X/Y/Z** stays at the S1 ref-point scale (native
  default 1.0 — it's genuinely constant unless the level sets a scale).

## v55m_50 — cEnt Rotator max speed cap raised 20 → 250

The cEnt Rotator's per-frame render angle cap (`NATIVE_ROTATOR_MAX_SPEED`)
was raised from **20.0 → 250.0**. Since the native render advances the angle
by `direction × 0.004` per frame, the cap now allows:

- `250 × 0.004 = 1.0 rad/frame` max (was `20 × 0.004 = 0.08` rad/frame)
- **12.5× faster** constant spin for cEnt Rotators with `ROS_Y=0`
  (constant-rotation mode) before the runaway-acceleration clamp kicks in.

This only affects the cEnt **Rotator** (AI 1) constant rotation. Other
entities, oscillation ranges (`ros_*`), and the native game's own Rotator
(`obj+0x10EC` direction clamp at 20.0 in 0x0043D8C0) are untouched.

## v55m_49 — One Custom Entity table per cEnt (A/D switching)

The debug view now creates **one Custom Entity table per cEnt** found in the
level's MESHWORLD section 3. Previously there were only two property tables
(switched with P). Now:

- Every `cEnt_XXX` entry on the level gets its **own** table — a level with
  `cEnt_001` and `cEnt_002` has two tables, a level with `cEnt_001` and
  `cEnt_025` also has exactly two tables. **The numbers don't need to be
  continuous.**
- **A key** = previous table, **D key** = next table (they wrap around).
  Pressing is rate-limited to 0.2s.
- The table title shows the position: `Custom Entity 001 - Swirl (1/2)`.
- The **P key** still toggles between the two property views *inside* the
  currently selected table (0 = hierarchical docs, 1 = live properties).

## v55m_44x — Rotator debug view: operator_new memory size

The sub-line under `1 - Constructors (object creation)` now shows the exact
bytes reserved by `operator_new` for the Rotator:
```
rotator
(empty line)
1 - Constructors (object creation)
  Operator_new (memory reserve): 5384 bytes
```
- The byte count is pulled from `ROTATER_SIZE` (0x1508 = 5384 bytes) at
  runtime, so it stays accurate if the constant ever changes.

## v55m_44w — Rotator debug view: section title + sub-line

In the `"Rotator"` debug state, the debug text now shows a hierarchical
breakdown instead of the default labels:
```
rotator
(empty line)
1 - Constructors (object creation)
  subline            ← indented 15px to the right
```
- Line 2 is now `1 - Constructors (object creation)` (was `hampter: yes`).
- A new sub-line `subline` is drawn below it, indented 15px to the right
  (sub-lines are offset 15px from main lines).

## v55m_44v — Runtime debug config file (mknp_custom_entities_debug.txt)

The game now reads a debug config file **next to the DLL**:
`mknp_custom_entities_debug.txt`. If the file doesn't exist, the mod creates
it with the default content `- debug: "yes"`.

**The file is read at runtime every frame** — editing it takes effect
immediately (no restart needed).

Content format: `- debug: "<value>"` where `<value>` is one of:
- **`"no"`** — don't draw the debug texts at all (they won't appear).
- **`"yes"`** or **`""`** (blank) — draw the debug texts in their default
  state (the `hello world` / `hampter: yes` / `ballz: no` lines).
- **`"Rotator"`** — draw the debug texts, but replace the first line
  (`hello world`) with `rotator`.

## v55m_44u — AI 1 Rotator constant rotation now actually works

The `cEnt_update_constant_rotations()` function was dead code — it was
defined but never called, so AI 1 (Rotator, ROS_Y=0) still oscillated like
AI 6 instead of spinning continuously.

**Fix:**
- Now called **every frame from the present hook** (`gluebie_present_helper`,
  slot 9, before the native object render at slot 10). It rewrites
  `obj+0x10EC` (direction) to `rot_y` each frame before the native render's
  ±2.0 flip check runs, so the native reversal never takes effect →
  constant one-way rotation.
- **Removed the angle clamp** from `cEnt_update_constant_rotations`. The old
  clamp (`angle > 1.99 → angle = -1.99`) caused a visible snap (~114° jump).
  Since the direction is rewritten every frame anyway, the native flip is
  already neutralized — the angle grows unboundedly but `Gfx_Scale` uses
  sin/cos, so large angles are fine.

## v55m_44t — Label/value lines with colors + empty line

Debug text now shows:
```
hello world
(empty line)
hampter: yes
ballz: no
```
- Added an empty line between "hello world" and "hampter".
- "hampter" → **"hampter: yes"** with "yes" in **green** (R=0, G=1.0, B=0).
- "ballz" → **"ballz: no"** with "no" in **red** (R=1.0, G=0, B=0).
- The label ("hampter: " / "ballz: ") stays white; the value is drawn
  immediately after it, colored. Uses `Font_MeasureText` (0x456E20) to get
  the label width so the value sits right next to it.

## v55m_44s — debugTextSpacing changed to 20 pixels

`debugTextSpacing` (the gap between the debug text lines) changed from
**10** to **20 pixels**.

## v55m_44r — Debug text lines: "hampter" + "ballz" with debugTextSpacing

Added two more debug text lines below "hello world": **"hampter"** and
**"ballz"**. Lines are evenly spaced using a new internal variable
`debugTextSpacing` (currently **10 pixels**), so each line is drawn 10px
below the previous one.

## v55m_44q — In-game text uses the score-counter font (showcardgothic16)

The in-game text ("hello world") was rendering too large because it used the
main title font (showcardgothic28, `App+0x318`). Changed it to the same font
the score counter uses — **showcardgothic16** (`App+0x320`), the info-text
font. Same size as the score HUD at the top-left.

## v55m_44p — In-game text system (score-HUD style "hello world")

**NEW FEATURE — the first milestone of the in-game text/HUD system.** This
adds a text layer that draws notifications on screen (like the score text at
the top-left), driven by a new dedicated hook. For now it just draws a fixed
"hello world" string to prove the pipeline works; the next step is wiring it
up to report actual mod events (spawns, collisions, AI state changes, etc.).

**Implementation:**
- **New hook: `Graphics_PresentOrEnd` (0x455A90).** The mod's existing
  "present hook" at 0x46C200 is actually `App_ResetFrame` = the viewport
  clear — text drawn there is erased. 0x455A90 runs AFTER the viewport clear
  and BEFORE Present, so text drawn here is visible on top of everything,
  exactly like the score HUD.
- **Hook type:** 7-byte trampoline (MOV AL,[ESP+4]; SUB ESP,0x20) with the
  same PUSHAD/PUSHFD → CALL C fn → POPFD/POPAD + original bytes + JMP-back
  pattern as the existing frame hooks. Installed in DllMain, uninstalled in
  DLL_PROCESS_DETACH.
- **Text drawing:** `UI_DrawTextShadow_Wrapper` (0x409B90), `__thiscall`
  RET 0x3C (15 params) — the score-HUD-style shadowed text. Font is read
  live every frame from `App+0x318` (showcardgothic28) and null-checked;
  the gate is font validity (not get_board(), which returns 0 on some Wine
  setups where the profile is NULL).
- **Draw call:** `hello world` at (20, 12), white text with black shadow,
  shadow offset (3,3) — matching the score-text look.

**Verification:** build EXIT 0 (322,094 B), hook confirmed installed at
0x455A90 (JMP) via live memory read, crash test passed (game survives). Note:
the game's logic loop does not advance into a level on this headless
Wine/llvmpipe setup (input is blocked by the mod's stdio logging), so a
visual screenshot in this environment is not possible — the hook and draw
pipeline are verified via memory inspection. User tests on real Windows.

## v55m_44o — waterwheel fix round 3 (STILL CRASHES — superseded, do not re-ship)

**IMPORTANT — USER RETEST (msg 1533631877636816896): 44o STILL crashed at level start.** Same crash family as 44i–44n (0x465777 CollisionLevel render walk). The no-registration/manual-render approach did NOT fix it either. MAKYUNI paused work on Custom Entities after this.

**Key diagnostic for the next attempt (never tested):** run the same build with `levels\Level3-WaterWheel` instead of `levels\Waterwheel` — this isolates file-vs-code in one test. 44a–44e (native file) had no crash reports; 44f+ (user's Waterwheel.MESHWORLD) crashed consistently at 0x465777. The user's file: 64,328 B, 20 meshbuffers (8 N:WATERWHEEL + 4 N:WHEELEMBED + 8 S:AXLE), 1832 verts, BRANCH root. Native file: 71,544 B, 20 meshbuffers, 2028 verts. Both parse valid; loader (0x461510) and 2nd param verified identical.

**Structural facts confirmed:** 0x470150 (SceneObject render) = `__thiscall(ECX, 1 arg)` RET 4. Native waterwheel = mesh at bare slot board+0x4374, NEVER in any list; sound via 0x434770→Sound_Play3D(0x459860). Registering the mesh anywhere (render list / scene tree) lets FinishLoad build a CollisionLevel → 0x465650 walk faults. Even with NO registration (44o), crash persists → remaining suspects: (a) the file's octree/strip layout difference (8 leaves vs 3 leaves), (b) something else in the mod's spawn (creak sound, mesh vtable hook, matrix write) that still triggers it.

## v55m_44o — FINAL (for real): waterwheel fix round 3 — NO registration, manual Present-hook render

44n still crashed at the same address (0x465777). The fix was incomplete: registering the MeshWorld in `board+0xCD4` + `sceneobj+0x1C` made the game's FinishLoad create a CollisionLevel for the mesh's component child anyway — same crash, different wrapper.

**44o: the mesh is NOT registered in ANY game list.** It lives in a bare slot (`ww->pc_obj`, mirroring the native `board+0x4374` bare slot). The Present hook renders it manually each frame via `cEnt_waterwheel_present_render` — which applies the X-axis rotation matrix to the mesh's world transform (`mesh+0x4`) and calls the mesh's own vtable[18] (0x470150 SceneObject render) directly. No game list, no FinishLoad processing, no CollisionLevel.

Also fixed: the vtable[18] call signature was wrong in 44n — `0x470150` is `__thiscall(ECX=this, arg1)` with `RET $0x4` (cleans 1 arg), not 2 args. Passing 2 args would corrupt the stack.

Now: same loader (0x461510), same mesh, same rotation — but **kept invisible to the game**, rendered manually. No registration → no CollisionLevel → no crash.

## v55m_44n — FINAL waterwheel fix: remove PopCylinder entirely (native-style mesh render)

MAKYUNI's question was the key: *"why can't you load the code that lastly worked with Level3-WaterWheel.MESHWORLD, but make it use my Waterwheel.MESHWORLD instead?"*

- **Root cause of the whole 44i–44m saga:** the mod wrapped the waterwheel mesh in `PopCylinder_ctor` (0x436EE0), which internally creates a CollisionLevel (0x465080, vtable 0x4D9068). That CollisionLevel's render (vtable[18]=0x465650) walks its COMPONENT meshbuffers (0x7C bytes, NO strip arrays) → `+0x418` strip-array deref is OUT OF BOUNDS → AV at 0x465789/0x465777. Every "fix" (44i disable collision, 44j neutralize, 44k recursive, 44l Present-hook, 44m selective) was fighting a problem the mod itself created.
- **The native game NEVER does this.** Dizzy loads `Level3-WaterWheel.MESHWORLD` via the SAME loader (0x461510 Level_MeshWorldCtor, 2nd param = `[board+0x878]+0x174`), stores the mesh at `board+0x4374`, and the board renders it directly. No PopCylinder, no CollisionLevel, no crash. The loader code is IDENTICAL for both files — verified: both `Level3-WaterWheel.MESHWORLD` and the user's `Waterwheel.MESHWORLD` parse perfectly (all meshbuffers have strips, both BRANCH octrees, valid S5 vertex refs).
- **44n fix:** waterwheel = plain MeshWorld (vtable 0x4D8FB0, [18]=0x470150 SceneObject render which recurses into octree children). Hook [18] with the X-axis rotation matrix (same 0.5°/frame as native), register the MESH in `board+0xCD4` render list + `sceneobj+0x1C`. NO PopCylinder → NO CollisionLevel → crash path structurally gone. All neutralization code (44i-44m) removed. This is EXACTLY "the code that worked with Level3-WaterWheel, pointed at your Waterwheel.MESHWORLD" — same loader, same native-style wrapper.
- **Verified:** build EXIT 0 (320,344 B, md5 f16091f9), banner `v55m_44n`, Wine smoke test survived 45s.

## v55m_44m — Fix 44l regression: SELECTIVE board-walk (don't kill level geometry)

- **FIXED:** 44l's log shows the crash is GONE (`Present-hook re-neutralization active (15 nodes)` — the main-thread pass runs; level loads, no crash) — **but 44l's blanket board-walk neutralized the board's OWN CollisionLevel tree (11 nodes: 0x0C77AAE8→0x0C7A28F0 = the level's ground/walls geometry)**. Setting `+0x430=1` + zeroing component meshbuffer counts on those nodes killed the level's collision AND lighting → **infinite fall through the ground + pale waterwheel color**.
- **Why:** 44l's "board coverage" walked `board+0x8B0`'s tree and neutralized EVERY 0x4D9068 node it found — but that tree contains the level's legitimate collision geometry (which has VALID meshbuffers and must render), not just the wheel's broken CollisionLevel.
- **Fix (44m): SELECTIVE board-walk.** The board-tree walk now only neutralizes a node if:
  1. it's one of the **wheel's own recorded nodes** (addresses captured during the wheel-tree walk into `g_wheel_nodes[]`, cleared on level load), OR
  2. it **probes broken**: component meshbuffer list has ≥1 meshbuffer with strip count > 0 whose `+0x418` strip pointer is unmapped (the exact OOB condition the render walk faults on).
  Level-geometry nodes have valid `+0x418` strip pointers → left alone → ground/walls/lights intact.
- Also: wheel-node address set resets on level load (stale addresses from previous board), and the Present-hook proof log re-arms per level.
- Result: wheel's broken CollisionLevel still neutralized (crash stays fixed), level geometry untouched (no more infinite fall / pale wheel).

## v55m_44l — FINAL level-start crash fix: main-thread re-neutralization (Present hook) + board CollisionLevel coverage

- **FIXED:** The 44k crash log (user's second run) shows `v55m_44k` banner, **4 nodes render-neutralized** (root 0x0C7AF550 + 3 children), yet the crash **still occurred** at `0001:00065789` during `Background / FinishLoad(OK)` — the SAME CollisionLevel render walk (`0x465789`).
- **Why 44k was still insufficient:** 44k's per-frame re-neutralization ran in the **mod's background thread** (16ms tick loop). The game's FinishLoad runs on the **game's own loading thread** and renders the collision level **between the background thread's ticks** — a newly registered/rebuilt CollisionLevel node with `+0x430=0` hits the renderer before the next mod-thread pass. The 4 neutralized nodes prove the spawn-time + background passes ran; the crashing node was created/registered **after** them.
- **Fix (44l):**
  1. **Main-thread re-neutralization via the Present hook** (`gluebie_present_helper`, installed at `App_ResetFrame 0x46C200`): every frame, on the main thread, **before any D3D render**, re-walks and re-neutralizes every active waterwheel's CollisionLevel tree. The Present hook runs synchronously with rendering — a node registered in the same frame is neutralized before the renderer reaches it. Closes the background-thread race for good.
  2. **Board CollisionLevel coverage**: the neutralizer now also walks `board+0x8B0`'s CollisionLevel tree (the board's own collision render path, which recurses into sub-list children during FinishLoad). If the wheel's CollisionLevel (or a FinishLoad clone of it) is ever appended there, it's `+0x430=1` before the renderer walks its broken component meshbuffers.
  3. **One-shot proof log**: `WATERWHEEL: Present-hook re-neutralization active (N nodes)` — written once when the Present pass first neutralizes nodes, so the user's next log proves the main-thread pass is running.
- All writes remain byte-flag safe (`+0x430=1` only; `+0x434` pointer never touched). Idempotent + cheap (bounded tree walks over ≤8 wheels + 1 board tree per frame).

## v55m_44k — Fix level-start crash for real: RECURSIVE + per-frame CollisionLevel neutralization

- **FIXED:** 44j still crashed at `0001:0006578C` — but the crash **moved from Draw (44i) to Background/FinishLoad (44j)**. The 44j log proved the root + 3 children were neutralized, yet the render still walked a broken node.
- **Why 44j was insufficient:** the CollisionLevel tree is **deeper than 2 levels**. Post-init `0x465860` builds it recursively: `0x465903 mov 0x430(%edi),%al` — **each child copies `+0x430` from its PARENT at construction time**, so every node built under a `+0x430=0` parent has `+0x430=0`. The render at `0x4657fc` recurses through `+0x1C`/`+0x424` sub-lists calling `vtable[18]` on **every** child at **any** depth — a single grandchild/leaf with `+0x430=0` walks its broken 0x7C component meshbuffers → AV. 44j's shallow 2-level scan missed them.
- **Fix (44k):**
  1. **Recursive full-tree walk** (`cEnt_neutralize_collision_tree`): from each CollisionLevel root, walks the entire subtree (bounded 512-node stack, 64-node visited set, cycle-safe), sets `+0x430=1` and zeroes the component meshbuffer count (`[node+0x08]+0x30`) on **every** CollisionLevel node (vtable `0x4D9068`).
  2. **Per-frame re-neutralization** in the polling thread: every 16ms, for every active waterwheel, re-walks and re-neutralizes. The game re-registers/re-builds CollisionLevel nodes during FinishLoad **after** the spawn-time patch — one-shot neutralization can never cover nodes that don't exist yet. Per-frame re-application catches them before the renderer reaches them. (Idempotent, cheap: bounded tree walk over ≤8 wheels, no logging with NULL logf.)
- All writes are safe: `+0x430` is a byte FLAG (byte-sized accesses only, set to 1 at `0x462363` when a meshbuffer list has items); `+0x434` is a render Level POINTER — never touched.
- New log lines: `WATERWHEEL: collision level 0x... render-neutralized (root)` + `collision node 0x... render-neutralized (total N)` per node (spawn-time pass only; per-frame passes log nothing).

## v55m_44j — Fix level-start crash for real: neutralize the CollisionLevel render

- **FIXED:** Level-start crash `0001:00065789` (VA `0x465789`) still happened in 44i — the same CollisionLevel render walk, ~7s into Draw after `FinishLoad(OK)`.
- **Why 44i was insufficient:** 44i only disabled the MOD's manual collision registration (`pc_obj+0x10E0` → `board+0x10EC` / `scene_col+0x18`). But **`PopCylinder_ctor` (0x436EE0) itself creates the CollisionLevel** (call `0x465080`, vtable `0x4D9068`) at `pc_obj+0x10E0`, and the **game's own registration fn `0x436FC0`** (which appends `+0x10E0` to `board+0x8B0+0x18` and `board+0x10EC` then zeroes `+0x10E0`) only runs on objects in `board+0x5428` — **the mod's wheel is NOT in that list**, so `pc_obj+0x10E0` stays valid and the CollisionLevel stays alive in memory, reachable from render sub-lists.
- **The actual crash:** The CollisionLevel render (`vtable[18]=0x465650`) walks its component meshbuffers (0x7C bytes, NO strip arrays):
  - `cmp [mb+0x10],0` → strip count happens nonzero
  - `mov 0x418(%esi),%edx; mov (%edx),%ecx` → **`+0x418` OOB** → garbage strip pointer → AV at `0x465789`
- **Fix (render-neutralization):** after `PopCylinder_ctor`, the mod now:
  1. Sets **`CollisionLevel+0x430 = 1`** — the render guard at `0x46568f` (`cmp [this+0x430],0; jne 0x4657fc`) then **skips the broken meshbuffer walk** and jumps to the (empty) `+0x424` sub-list recursion.
  2. Zeroes the component meshbuffer **count at `[col+0x08]+0x30`** — belt-and-suspenders so the walk can never run even if `+0x430` is reset.
  3. Scans the `+0x18` render sub-lists (count `+0x1C`, items `+0x424`) of **both pc_obj and the CollisionLevel** and render-neutralizes any child CollisionLevels (vtable `0x4D9068`) the same way.
- All writes are safe: `+0x430` is a byte FLAG (verified — all accesses are byte-sized `mov %al/imm8,0x430(%reg)`; set to 1 at `0x462363` when a meshbuffer list has items). `+0x434` is a render Level POINTER — deliberately NOT touched.
- New log lines: `WATERWHEEL: collision level 0x... render-neutralized` + per-child lines.
- The wheel still renders + rotates visually (vtable[18] hook unaffected); collision remains off (same as native Dizzy wheel — the level file's own `N:WATERWHEEL` geometry provides collision).

## v55m_44i — Fix level-start crash: disable waterwheel collision object

- **FIXED:** Level-start crash (`0001:0006578C` = VA `0x46578C`) once the waterwheel spawns. The wheel appeared for a millisecond then the game crashed during `FinishLoad`.
- **Root cause (traced end-to-end in the binary):** `PopCylinder_ctor` (`0x436EE0`) creates a **CollisionLevel** (`0x465080`, vtable `0x4D9068`) whose post-init (`0x465860`) loads the mesh via `0x4706e0` — the **component SceneObject loader** (vtable `0x4D9CDC`, **0x7C-byte meshbuffers, NO strip arrays**). The collision render (vtable[18] = `0x465650`) then walks each meshbuffer:
  - `cmp [mb+0x10],0` (strip count — happens to be nonzero on a 0x7C component meshbuffer)
  - `mov 0x418(%esi),%edx; mov (%edx),%ecx` (strip array — **`+0x418` is out of bounds on a 0x7C meshbuffer**)
  - → reads a garbage strip pointer → AV at `0x465780`; crash EIP `0x46578C` = SEH-resumed mid-instruction (`74 35` = 2nd byte of `je`).
- **Fix:** Stop registering the PopCylinder's collision object (`pc_obj+0x10E0`) in the board collision lists. The **native Dizzy waterwheel has NO spawned collision object** — collision comes from the level file's own `N:WATERWHEEL` geometry (`Level_MeshWorldCtor → board+0x4BA8`, verified in the dizzy-waterwheel-system reference). The mod replicates that: the wheel still renders + rotates visually, and the user's level geometry provides collision.
- Collision registration code is commented out (v55m_44b block) with the full explanation.

## v55m_44h — Fix waterwheel never spawning (44g validation too strict)

- **FIXED:** The v55m_44g mesh-validation rejected `meshbuffer count=0` and fell back to `_default` — but the user's log showed BOTH `levels\Waterwheel` AND `levels\_default` load with `count=0`, and the wheel never spawned ("it's not appearing").
- **Why count=0 is valid:** The PopCylinder render `0x45E0E0` handles an empty meshbuffer list natively — `cmp [eax+4],0; jg` skips the strip loop (`xor edi,edi`), then falls through to the SceneObject render `0x470150` which draws from the mesh's own vertex data. So count=0 meshes render fine; my 44g check wrongly killed them.
- **Fix:** validation now only rejects negative counts, absurd counts (>10000), or genuinely unreadable items pointers. `count=0` is accepted and logged.
- No crash — the 44f crash (`0001:00065789`) is gone; the wheel simply needs its spawn accepted.

## v55m_44g — Waterwheel mesh validation + _default fallback on invalid/corrupt file

- **FIXED:** A malformed/corrupt custom `Waterwheel.MESHWORLD` could crash the level render at `0001:00065789` (VA `0x465789` — render reads strip array at `mb+0x418` of a meshbuffer, garbage data → crash).
- **Root cause:** The mod renders the waterwheel mesh via `0x45E0E0` which expects full-level `0x874`-byte CreateMeshBuffers (strips at `+0x418`, name at `+0x864`). A custom mesh file that loads with an invalid/empty meshbuffer list makes the render read out of bounds.
- **Fix:** After `MeshWorld_ctor`, the mod now **validates the loaded mesh's meshbuffer list** (count 1–10000, items pointer readable). If invalid → logs `meshbuffer list invalid` and **falls back to `levels\_default`** and retries. If both fail → logs and skips spawning (no crash).
- **ALSO FIXED:** The entity table's `Waterwheel` entry now uses `levels\Waterwheel` (was `levels\Level3-WaterWheel`) — so the new default actually activates (v55m_44f's default was silently overridden by the table).
- New diagnostics: `WATERWHEEL: mesh '<path>' meshbuffers=%d` and invalid/fallback messages.

## v55m_44f — Waterwheel default mesh + fallback

- Waterwheel (ai_type 26) now defaults to `levels\Waterwheel` (user provides `Waterwheel.MESHWORLD`) instead of `levels\Level3-WaterWheel`.
- If `Waterwheel.MESHWORLD` is missing, falls back to `levels\_default` (same pattern as Cloudscape case 28).
- A MESH= tag override still wins over the default.

## v55m_44e — Fix level-start crash (truncated vtable copy)

- **FIXED:** Crash at level start (`0001:000587E7` = VA `0x40587E7`, mid-instruction EIP) during `FinishLoad` background op on Warm-Up.
- **Root cause:** Waterwheel + Chomper each created a **private vtable copy of only 256 bytes (64 entries)**, but the PopCylinder vtable (`0x4D58F0`) is **168 entries = 672 bytes**. During level load (FinishLoad), the game calls vtable slots beyond 64 → reads garbage past the truncated copy → EIP corruption → crash.
- **Fix:** enlarged both private vtable copies from 256 → `0x400` (1024 bytes), matching the proven catapult pattern (v55m_43h). Now covers the full 168-entry vtable + headroom.
- This is the same class of bug as the catapult vtable-size truncation pitfall (documented in entity-constructor-discovery skill).

## v55m_44d — Waterwheel stops when paused

- **FIXED:** Waterwheel kept rotating (and playing WheelCreak) while the game was paused.
- Native Dizzy's waterwheel update lives inside `DizzyBoard_Update` (0x41D512), which does **not** run while paused (Scene_Update reads `board+0x874`; Scene_CreateGameOverMenu sets it).
- The mod's background thread called `cEnt_waterwheel_update` unconditionally → wheel kept spinning + creaking during pause.
- Added the same pause gate the catapult already uses (`board+0x874 != 0 → return`) at the top of `cEnt_waterwheel_update`, so rotation AND sound both freeze while paused and resume on unpause.

## v55m_44c — Waterwheel reverse rotation + WheelCreak sound

- **CHANGED:** Waterwheel now rotates in the **opposite direction**. Native Dizzy decrements (`angle -= 0.5`/frame), user requested the reverse → now `angle += 0.5`/frame.
- **ADDED:** WheelCreak sound — replicates the native Dizzy system exactly:
  - Native Dizzy loads `sounds\wheelcreak` into audio slot `App+0x490` at startup (0x402A820), then at board setup grabs a playable channel: `MOV ECX,[board+0x878]+0x490; CALL 0x459810` → cached at `board+0x4BDC` (0x41D34C).
  - Per-frame it plays that channel via `Sound_Play3D` (0x459860) with distance attenuation (0x41DC22).
  - Mod mirrors this at waterwheel spawn: reads the game's own loaded wheelcreak slot `[App+0x490]`, calls `Sound_GetChannel` (0x459810) to get a playable channel, caches it in `ww->creak_channel`, and plays it per-frame from the update loop via `Sound_Play3D(channel, x, y, z, 1.0)`.
- **SAFETY (no crash):** every pointer deref is NULL + `IsBadReadPtr` guarded; if the wheelcreak slot or channel is unavailable (non-Dizzy level / sound not loaded), the mod simply leaves `creak_channel = 0` and never calls `Sound_Play3D`. Runs on the background thread — the same proven-safe location as the tar sound.

## v55m_44b — Waterwheel collision fix (non-solid → solid)

- **FIXED:** Waterwheel was non-solid because the spawn code read the collision object from `pc_obj + 0x10D4` — but for PopCylinder that offset holds the **position X float**, so `IsBadReadPtr` rejected it and collision registration was skipped.
- Verified in native binary (`PopCylinder_ctor` 0x436EE0):
  - `+0x10D4` = position floats (written at 0x436F2D–0x436F3E)
  - `+0x10E0` = the real collision Level, created by `call 0x465080` (CollisionLevel, vtable 0x4D9068) at 0x436F5E
- Fixed to read `+0x10E0`, matching the working Rotator pattern (line ~2295).
- Native PopCylinder also self-registers collision (`0x436FC0` appends `+0x10E0` to scene tree `board+0x8B0+0x18` and board collision list `board+0x10EC`), so the wheel is now solid.

## v55m_44a — Waterwheel X-axis rotation (native Dizzy replication)

- **FIXED:** cEnt Waterwheel now rotates around the **X axis** (previously Y-axis), matching the native Dizzy Race waterwheel exactly.
- Verified against native binary:
  - `angle -= 0.5` degrees per frame (constant `0x4CF3F0`)
  - Native rotation builder at `0x45AE27` builds m[5]=cos, m[6]=sin, m[9]=−sin, m[10]=cos → rotation about **X**
  - Reached via `0x457C90` (angle × deg→rad `0x4D8E58` → X-axis builder via thunk `0x4F7208`)
- **FIXED:** The old hook used direct `D3D SetTransform` with a rotation-only matrix, which **discarded the object's position and scale** from the world matrix.
- Now uses the proven catapult pattern (v55m_43h): composes the rotation with the object's own world matrix at `renderLevel+0x4`, pivoting around the object's center `(ww->x, ww->y, ww->z)`.
- The render Level is read from `this_+0x434` (same as catapult — confirmed PopCylinder stores it there at `0x436F67`).

## v55m_42t — Arm render-list only

- Removed arm from `BOARD_UPDATE_LIST` and scene tree to isolate whether the crash is list-related or render-hook related.

## v55m_42s — Remove arm from collision lists

- PopCylinder collision object offset is `+0x10D4`, not `+0x10E0`.
- v55m_42r added the arm's `obj+0x10E0` to collision lists, which read a bad pointer/position float and corrupted the scene tree → crash in Draw.
- v55m_42s leaves the arm visual-only: update list + render list + scene tree. The base catapult still handles collision.

## v55m_42r — Fix arm mesh loading crash

- v55m_42q loaded the arm mesh using `board+0x878` as an `App*` pointer, but `board+0x878` is actually `SoundList*`.
- This caused the arm's `PopCylinder_ctor` to receive a corrupted Gfx/MeshWorld pointer.
- v55m_42r reuses the already-loaded base catapult MeshWorld pointer (loaded with the correct global `App` Gfx device).
- Arm now shares the same mesh data as the base; it is still a separate object with its own Y-axis rotation.

## v55m_42q — Separate arm object with Chomper-style direct D3D rotation

- v55m_42p rotated the catapult base itself; this caused visual issues because the base includes the bowl/collision geometry.
- Now spawns a **second** PopCylinder object using `levels\\Level4-Catapult` mesh as the **arm**.
- The base catapult keeps its original render unchanged.
- The arm object's `vtable[18]` is hooked and applies a **Y-axis rotation** via direct `D3D SetTransform` (same working pattern as Chomper/Waterwheel).
- Pivot is the catapult position `(cs->x, cs->y, cs->z)`.
- Arm angle is still driven by the existing 50-frame windup countdown (0 to -45 degrees), snaps back to 0 after launch.
- Arm object is registered on board lists and despawned with the rotater cleanup on level unload.

## v55m_42p — Catapult arm rotation via Timer+Gfx

- Added `arm_angle` and `orig_vtable18` to `CatapultState`.
- Hooked catapult vtable[18] to apply Z rotation during render.
- Arm rotates from 0 to -45 degrees over the 50-frame windup.
- Uses the native Timer_Init/Gfx_ScaleZ/Gfx_SetPosition/Timer_Cleanup pattern (same as working Bridgeslam/Chomper hooks).
- Avoids direct D3D SetTransform, which crashed in v55m_42k/l.

## v55m_42f — Tighten catapult trigger + halve launch force again

- Native Catapult_Update has no separate 'enter force'; the only force is 90.0 direct-velocity launch after 50 ticks.
- Trigger zone tightened to horiz<60 and dy -10..+15 so launch only fires inside the bowl.
- Launch force halved again: horizontal 37.5 -> 18.75, vertical 22.5 -> 11.25.

## v55m_42e — Halve catapult launch force to match native feel

- Native Catapult_Update fires at 90.0 total velocity via Scene_ForEachBall_SetVelocity (direct velocity set).
- Our custom catapult uses physics force accumulators (ball+0x170/174/178), so same numbers feel much stronger.
- Halved launch force: horizontal 75.0 -> 37.5, vertical 45.0 -> 22.5.

## v55m_42d — Switch catapult sound to older BASS_SamplePlay + tighten trigger zone

- Added BASS_SamplePlay and BASS_StreamPlay/CreateFile function loading.
- cEnt_play_dropin_sound now tries BASS_SamplePlay first, then SampleGetChannel+ChannelPlay, then StreamCreateFile+StreamPlay.
- Reduced catapult trigger radius from 120 to 90 and tightened vertical window (dy -30..+50) so it only fires when on the bowl.

## v55m_42c — Load BASS function pointers with stdcall decorated fallback

- Added decorated-name fallback (_Name@N) for BASS_SampleGetChannel, BASS_ChannelPlay, etc.
- Loaded BASS_StreamCreateFile as additional fallback.

## v55m_42b — Add BASS error logging to catapult sound

- Log BASS error codes after SampleLoad/SampleGetChannel/ChannelPlay.
- Helps diagnose why loaded sample produces no audio.

## v55m_42 — Play dropin sound via BASS directly

- Load real BASS function pointers: BASS_SampleLoad, BASS_SampleGetChannel, BASS_ChannelPlay, BASS_SampleFree.
- Load 'sounds\dropin' sample once per level at catapult spawn.
- Play it on launch via BASS_SampleGetChannel + BASS_ChannelPlay.
- Free sample on level unload to avoid leak.

## v55m_41 — Diagnostic build: catapult sound disabled

- Used to confirm if the ntdll crash is caused by sound load/play.

## v55m_40 — Fix sound crash: use correct board+0x464 slot for dropin

- Native catapult sound loading uses board+0x464 for 'sounds\dropin'.
- v55m_39 wrote to board+0x460, corrupting another sound slot.

## v55m_39 — Use native sound manager virtual call for dropin sound

- Replaced manual Sound_LoadOggOrWav with board+0x22C sound manager vtable[24].
- Native pattern loads 'sounds\dropin' into board+0x460 slot, then Sound_Play3D.

## v55m_38 — Fix catapult sound crash: use App+0x460 as SoundList

- v55m_37 passed App* (board+0x878) to Sound_LoadOggOrWav; now uses App+0x460.
- Added diagnostic logs for sound_list and loaded channel.

## v55m_37 — Stronger catapult launch + safe dropin sound

- Increased launch force from 35/25 to 75/45 (closer to native 90 velocity).
- Load dropin sound via Sound_LoadOggOrWav(board+0x878) instead of cached App+0x460 pointer.
- Play via Sound_Play3D to avoid Sound_PlayChannel crash.

## v55m_36 — Disable catapult sound + star trail to isolate crash

- Catapult trigger now fires correctly.
- Removed sound play and star trail writes (caused 0x00000000 crash after launch).
- Keep launch force only for stability test.

## v55m_35 — Restore v55m_28m radius catapult trigger, remove SEH hook

- Removed `board+0x43B8` append (caused heap corruption on non-Tower levels).
- Removed `install_bonk_collision_hook()` (SEH trampoline at `DispatchCollisionEvents` caused Draw crashes at 0x452376).
- Restored Present-hook radius trigger from v55m_28m.

## v55m_34 — Remove unsafe catapult MeshWorld scan

- v55m_33 scanning the catapult collision Level's MeshWorld caused crash at 0x45237E.
- Replaced with simple `cat_col_obj` pointer log.

## v55m_33 — Add catapult diagnostic logs

- Log whether `E:CATAPULTBOTTOM` mesh exists in the spawned catapult collision level.
- Log when the player ball collides with `E:CATAPULTBOTTOM`.
- Log when `E:CATAPULTBOTTOM` triggers `Catapult_Launch`.

## v55m_32 — Fix catapult E:CATAPULTBOTTOM trigger

- Install DispatchCollisionEvents hook when a Catapult is spawned (not just Bonk).
- Match `collision_data[0]` (entity pointer) against `catapult+0x10D4`, not `collision_data[1]`.

## v55m_31 — Revert collision-site hooks to v55m_29 baseline

- Reverted v55m_30/v55m_30a collision-site hook rewrite; it caused heap corruption (crash 0x45FB03).
- Restored native E:CATAPULTBOTTOM DispatchCollisionEvents approach from v55m_29.

# Version Changelog

## v55m_29 — Catapult uses native E:CATAPULTBOTTOM collision event

- **Problem:** v55m_28m catapult fired via radius trigger, but the zone was too wide and did not match the native Tower behavior.
- **Fix:**
  - Removed the manual radius trigger + force push in `cEnt_catapult_present_check`.
  - Re-enabled adding the spawned `Catapult_ctor` object to the native catapult update list at `board+0x43B8`, so the original wind-up/launch state machine (`Catapult_vtable11` / `Catapult_Update`) runs.
  - In the `DispatchCollisionEvents` detour, `E:CATAPULTBOTTOM` now directly calls `Catapult_Launch(obj)` on the matching tracked catapult instead of just setting a `collided` flag.
  - Falls back to launching the closest tracked catapult if the collision pointer does not match exactly.
- **Behavior:** Entering the `E:CATAPULTBOTTOM` collision mesh plays the native 50-frame wind-up animation, then launches the ball with the original Tower force/direction.

## v55m_28m — Catapult trigger diagnostics + wider zone

- **Problem:** v55m_28l catapult tracked and heartbeat logged, but no TRIGGER/LAUNCH lines. User entered the catapult and nothing happened.
- **Diagnosis:** `custom_entities_catapult.log` showed heartbeat every 60 frames for 780+ frames with no trigger. Either the ball was outside the radius 80 / dy [-100,+40] window, or `get_ball_ptr` returned a non-player ball.
- **Fix:**
  - Widened trigger zone to radius 120, dy [-120,+80].
  - Widened reset zone to radius 250, dy [-180,+120] (must be larger than trigger).
  - Added per-frame proximity logging: ball position, catapult position, horizontal distance, dy, trigger/reset flags, cooldown, `was_in_zone`.
  - Logging now happens every 30 frames and also whenever the ball is inside the reset zone, so the user can see exactly why the trigger does or does not fire.
- **Crash test:** pending.

## v55m_14 — Chomper crash fix (0000:00000010 during Draw)

- **Root cause:** Chomper added `coll` (Level_RenderCtor result) to
  board+0xCD4 (render list) and sceneobj+0x1C (spatial tree).
  The game's Draw iterates the render list and calls vtable[2]
  (SceneObject_BuildStrips) on each item. BuildStrips calls
  Font_RenderToTextureComplex with the MeshWorld's vertex count.
  The Chomper's `coll` has an EMPTY MeshWorld (Level_LoadMeshes
  creates a new MeshWorld via MeshWorld_ctor_simple but the parent
  mesh_world+0x08 is NULL because vtable[14] loads .MESHWORLD binary
  data without setting +0x08). With 0 vertices, Font_RenderToTextureComplex
  creates a 0x0 D3D texture -> NULL -> reads NULL+0x10 -> crash at
  0x00000010.
- **Fix:** Removed coll from board+0xCD4 (render list), board+0x8B8
  (update list), and sceneobj+0x1C (spatial tree). The Present hook
  (cEnt_chomper_update) already handles all rendering via vtable[0x16]
  (SceneObject_CallUpdate) + vtable[0x15] (SceneObject_CallRender) with
  Timer_Init/Gfx_ScaleZ. No game render pipeline involvement needed.
- **Ghidra analysis:** LoadMeshWorld (0x45DE30) has two paths:
  (1) File exists: calls vtable[14] (0x4629E0, binary file loader) +
  vtable[15] (0x460DA0, Scene_RenderFrame). vtable[14] sets this+0x47C
  = self, this+0x431=1, this+0x434=Timer, but does NOT set this+0x08
  (MeshWorld*). (2) File not found: creates MeshWorld at +0x08 via
  operator_new(0x488)+MeshWorld_ctor+MeshWorld_Parse. Level_LoadMeshes
  reads parent+0x08 to copy mesh data -> NULL -> empty child MeshWorld.

## v55j_8 — Gluebie: match native Dizzy behavior (Ghidra-verified)

- **Gluebie proximity behavior fixed to match native DizzyBoard_Update exactly.**
  Deep Ghidra decompilation of DizzyBoard_Update (0x41D512), Gluebie_ctor
  (0x437CB0), Gluebie vtable[11] (0x43ECC0), and Dizzy_CreateDynamicObjects
  (0x40A5F0) revealed 5 behavioral differences:
  - **BUG 1 (wrong flag):** Mod set ball+0x260 (tar render flag) — native
    Gluebie does NOT touch ball+0x260. That flag is set by Ball_Update when
    the ball physically touches the tar SURFACE (3.0 units). Mod was showing
    tar splotch at 45-60 units distance (way too early) and hiding it when
    leaving range. Native sets ball+0x2BC (sound/particle cooldown) instead.
  - **BUG 2 (wrong clear):** Mod cleared ball+0x260=0 when ball left range.
    Native NEVER clears ball+0x2BC — it stays 1 until ball dies/respawns.
  - **BUG 3 (wrong radius):** Mod used hardcoded 60.0. Native uses
    obj+0x1100 * 60.0, where Gluebie_ctor inits +0x1100 to
    (RNG(25)+75)*0.01 = 0.75-1.0, giving radius 45-60.
    Mod comment said "ctor doesn't init +0x1100" — WRONG, it does.
  - **BUG 4 (double processing):** Present hook ran Gluebie check on Dizzy
    Race, where native DizzyBoard_Update ALREADY handles it → double
    velocity scaling. Added gluebie_is_dizzy() check to skip on Dizzy.
  - **BUG 5 (missing cooldown flag):** Mod used internal static cooldown
    counter instead of ball+0x2BC. Native checks ball+0x2BC==0 before
    playing sound, then sets ball+0x2BC=1 (once per entry, no counter).
- **Velocity scaling confirmed correct:** Native does normalize velocity,
  multiply by (speed * 0.95) / speed = 0.95, which IS just *= 0.95 with
  zero-velocity guard. Mod's simple *= 0.95 is functionally equivalent.
- **Constants verified (Ghidra memory reads):**
  - _DAT_004d0930 = 60.0 (outer radius multiplier)
  - _DAT_004d092c = 0.95 (velocity scale factor)
  - _DAT_004cf368 = 0.0 (epsilon)
  - _DAT_004cf380 = 0.25 (tar sink rate)
  - _DAT_004cf480 = 75.0 (radius RNG offset)
  - _DAT_004cf524 = 0.01 (radius RNG scale)
- **Not yet implemented (native has, mod doesn't):**
  - Tar splotch particles (3x operator_new(0x14), random direction,
    appended to ball+0x810 AthenaList, max 30). Visual only, no gameplay
    impact. Will add if user reports missing particles.
- **Crash test:** PASS (12.1s, no crash, DLL restored).

## v55g — Catapult: full system port (solid + launch + state machine)

- **Catapult (ai_type 35) now has full native behavior ported.**
  - **Problem:** Catapult was spawning but non-solid and static — no collision,
    no wind-up, no launch, no per-frame state machine.
  - **Root cause:** Catapult_ctor calls Stands_ctor which clones spatial trees
    into obj+0x18 (making it solid), but the collision/render Level was never
    created. Without it, the collision object at obj+0x10D4 was NULL, so the
    collision list (board+0x10EC) was empty — ball passed through.
  - **Fix — 5 components ported from native (Ghidra-verified):**
    1. **Collision/render Level:** Create via `Level_RenderCtor(mesh)` and store
       at obj+0x10D4 (same pattern as TipperVisual). Added to collision list
       (board+0x10EC) and scene collision (sceneobj+0x18).
    2. **Per-frame state machine:** `Catapult_vtable11` (0x437F10, vtable[11])
       handles wind-up + release. Native game calls this via Board_UpdateRaceState,
       but Catapult is NOT in the Scene_Update list (board+0x8B8). Added manual
       per-frame call in `entity_thread` for each tracked Catapult.
    3. **E:CATAPULTBOTTOM trigger:** Native game only checks this on Tower Race
       (race 4). Instead of hooking DispatchCollisionEvents (SEH trampoline crash
       risk — confirmed in v53g-2), use per-frame proximity check: when ball is
       within 40 units of catapult base (Y offset -10 for bottom plate), call
       `Catapult_Launch` (0x434290) which sets launching flag + 50-tick countdown.
    4. **Tracking:** Catapult objects tracked in `g_catapults[]` array (max 16)
       for per-frame updates. Reset on level unload.
    5. **Collision list fix:** Updated col_off logic to include type 35
       (Catapult) alongside PopCylinder(0) and Rotator(1-6) for collision
       object at +0x10D4.
  - **Functions ported (all Ghidra-verified):**
    - `Catapult_ctor` (0x437E10, 3 params: this, board, mesh) — already called
    - `Catapult_Launch` (0x434290, __fastcall, 1 param: this) — trigger
    - `Catapult_vtable11` (0x437F10, __fastcall, 1 param: this) — state machine
    - `Catapult_Update` (0x43F080, __fastcall, 1 param: this) — vtable[61], animation
      (called by render loop via board+0xCD4 render list, no manual call needed)
  - **Note:** `logf` not used in per-frame Catapult code (dangling pointer after
    initial fclose in entity_thread — pre-existing issue).
  - **AI list entry updated:** type changed from 0 to 35 (Catapult).

## v55f — WaterWheel: mesh loaded + rotated per-frame (no entity)

- **WaterWheel (ai_type 26) now loads its mesh and rotates it each frame.**
  - Native game (DizzyBoard_ctor at 0x41D067) creates a mesh via `MeshWorld_ctor("Levels\\Level3-WaterWheel")`
    stored at `board+0x4BA8`, position at `board+0x4BB0`, angle at `board+0x4BBC`.
  - DizzyBoard_Update (0x41D512) rotates: `angle -= 0.5/frame` (constant at 0x4CF3F0),
    then applies via `Gfx_RotateY(stack_matrix, angle)` (0x457C90) + mesh vtable[22] (SetTransform)
    + mesh vtable[21] (SetPosition).
  - **Old behavior:** spawned PopCylinder with the mesh path (wrong — non-solid, static).
  - **New behavior:** creates mesh via `MeshWorld_ctor`, stores in `g_waterwheels[]` array,
    rotates per-frame via `cEnt_waterwheel_update()` which calls `Gfx_RotateY` + mesh vtable[22]+[21].
  - Mesh is added to board render list (`board+0xCD4`) and scene spatial tree (`sceneobj+0x1C`)
    for visibility and collision.
- **Added `cEnt_waterwheel_update()` — per-frame rotation.**
  - Calls `Gfx_RotateY(rot_matrix, angle)` to build Y-rotation matrix.
  - Calls `mesh->vtable[22]()` (SetTransform, no params) then `mesh->vtable[21](&rot_matrix)` (SetPosition, 1 param).
- **AI list entry updated:** mesh path stays `levels\\Level3-WaterWheel` (used by `MeshWorld_ctor`).

## v55e — TarBubble: no entity, position-only marker

- **TarBubble no longer spawns a PopCylinder entity.**
  - Native game has NO TarBubble constructor — S1 ref points named "TarBubble"
    are stored in `board+0x4790` AthenaList as position markers.
  - DizzyBoard_Update (0x41D512) creates a collision traversal object (0x44FA90)
    that sinks the ball 0.25/frame when inside the tar radius.
  - Old behavior: spawned PopCylinder with `meshes\tarbubble` mesh (wrong).
  - New behavior: stores position in `g_tarbubble_pos[]` array, no entity spawned.
- **Added `cEnt_tarbubble_proximity_check()` — replicates native tar sinking.**
  - Per-frame check: iterates balls, computes 3D distance to each TarBubble position.
  - When ball is within 3.0 units: sinks ball Y by 0.25/frame (native constant 0x4CF380),
    sets `ball+0x2CC` (in_tar flag — disables control, decays spin 0.85x/frame),
    sets `ball+0x2BC` (tar render flag).
  - Radius 3.0 matches native Ball_Update tar surface contact distance.
- **AI list entry changed:** `Tarbubble` mesh path → `NULL` (no mesh file needed).

## v55d — Gluebie tar sound + proximity fix

- **Gluebie tar sound: Fixed crash + implemented sound playback.**
  - Root cause: `Sound_Play3D` typedef was missing the 4th parameter (float scale=1.0).
    Native function has `RET 0x10` (4 params = 16 bytes). Old typedef only passed 3
    params → stack corruption → crash at 0x4065F2 inside Ball_ctor.
  - Sound is now queued via `g_gluebie_sound_pending` flag when ball enters Gluebie
    range, then played from the background thread loop with proper 4-param call:
    `Sound_Play3D([App+0x484], x, y, z, 1.0f)`.
  - App accessed via `board+0x878` (same as native DizzyBoard_Update at 0x41D9B3).
  - Added `IsBadReadPtr` safety checks on sound channel pointer.
- **Gluebie proximity radius: Changed from 45-60 to 3.0 (user request).**
  - Native has TWO proximity checks:
    1. `DizzyBoard_Update`: `obj+0x1100 * 60.0` = 45-60 units (center-to-center, velocity slowdown)
    2. `Ball_Update`: `3.0` units (distance to tar surface, sets tar flag)
  - The 45-60 value was too large for non-Dizzy levels (smaller Gluebies).
  - Using 3.0 as requested — matches the native inner zone.
- **Bridgeslam sound: Also fixed to pass 4th param (1.0f) + IsBadReadPtr check.**
  - Same `Sound_Play3D` signature bug affected bridgeslam sound (latent crash).

## v55c — Gluebie collision + behavior fix

- **Gluebie (ai_type 43): Fixed missing collision + behavior + cross-level proximity.**
  - **Root cause:** Gluebie was ai_type=0 (PopCylinder fallback). No Gluebie_ctor
    was called, so it had no Gluebie vtable (0x4D4F38), no vtable[11] update
    function (0x43ECC0), and no proximity behavior.
  - **Fix 1 (ctor):** Added ai_type 43 with Gluebie_ctor (0x437CB0, 0x110C bytes).
    - Gluebie_ctor calls Stands_ctor (clones spatial trees from Level3-Gluebie mesh)
    - vtable[11] (0x43ECC0) handles rendering + animation (scale, position, particles)
    - Position stored at obj+0x10D4/10D8/10DC (matching native Dizzy_CreateDynamicObjects)
  - **Fix 2 (cross-level proximity):** Native Gluebie behavior is in DizzyBoard_Update
    which iterates board+0x4378 (Dizzy-only AthenaList — Tower/Expert/Toob use 0x4378
    as a Level pointer, so it can't be safely initialized on other levels).
    Added mod-side proximity check (cEnt_gluebie_proximity_check) that runs every
    frame from the mod's background thread:
    - Iterates board+0x29D4 (ball AthenaList) and mod's g_gluebie_objs array
    - Checks 3D distance: sqrt((gluebie+0x10E0 - ball+0x164)^2 + ...) < gluebie+0x1100 * 60.0
    - If in range: scales ball velocity (collisionMesh+0xCA4/CA8/CAC) by 0.95
    - Sets gluebie+0x1104 = 1 (active flag)
    - ball+0x1A4 = collision mesh pointer (set by Ball_ctor via CollisionMesh_ctor)
  - **Native Gluebie behavior** (DizzyBoard_Update, decompiled):
    - Iterates board+0x4378 (Gluebie list)
    - Checks ball proximity: distance(gluebie_pos, ball_pos) < gluebie+0x1100 * 60.0
    - If ball in range: scales ball velocity by 0.95, plays sound, creates 3 particles
    - Sets gluebie+0x1104=1 (active flag)

## v55b — Tipper collision fix

- **Tipper (ai_type 37): Fixed missing collision + crash on level start.**
  - **Crash cause:** TipperVisual_Attach (0x465200) was declared as `__cdecl` but is
    actually `__thiscall` (starts with `MOV ESI,ECX`, `RET 0x4`). With `__cdecl`,
    ECX was not set to the visual pointer → ESI = garbage → access violation reading
    ESI+0x431 at 0x465203. Crash report: 0001:00064203.
  - **Collision root cause:** Tipper_ctor creates the behavior object but sets
    obj+0x10D4 = 0. The Tipper's vtable[11] (0x437A40) is a 4-state machine that
    dynamically adds/removes obj+0x10D4 (the TipperVisual) from the scene collision
    list (board+0x8B0+0x18) as the tipper raises/lowers. Without TipperVisual,
    obj+0x10D4=0 (NULL) → AthenaList_Append(board+0x8B0+0x18, NULL) → no collision.
  - **Fix 1 (crash):** Changed TipperVisual_Attach typedef from `__cdecl` to `__thiscall`.
  - **Fix 2 (collision):** After Tipper_ctor, create TipperVisual matching native
    Dizzy_CreateDynamicObjects flow:
    1. Level_RenderCtor from Level3-Tipper mesh → render Level
    2. TipperVisual_ctor from render Level → visual object
    3. Store visual at obj+0x10D4 (DWORD index 0x435)
    4. TipperVisual_Attach(visual, obj) — links visual to behavior
  - **Native vtable[11] state machine:**
    - State 0: Countdown → AthenaList_Append(collision_list, obj+0x10D4) → state 1
    - State 1: Raise animation (scale × 1.05/frame) → state 2
    - State 2: Wait (100 frames) → state 3
    - State 3: Lower animation (scale × 0.85/frame) → AthenaList_Remove(collision_list, obj+0x10D4) → state 0

## v55 — Major fix: Static Swirl + crashes

- **Static Swirl fix (10 entities): 8ball, Bell, Chomper, Fan, Funball, Glassbreaker, Judge, Mag, Sawblade, Tarbubble**
  - Root cause: if/else-if chain bug (fixed in v54) was still sending .MESH entities
    through the .MESH swap code with Swirl mesh instead of their correct mesh.
  - v55 fix: Removed ai_types 10, 11, 15, 22 from path=NULL override. These entities
    have .MESH file paths (meshes\*) and should go through the .MESH swap code which:
    1. Loads the correct .MESH via MeshNode_ctor
    2. Loads Swirl as PopCylinder base (proper vtable, position, collision)
    3. Swaps obj+0x08 to the entity's MeshWorld (correct visual model)
    4. NO vtable override (prevents stack corruption from v54d crash)
  - Result: All 10 entities should now show their correct mesh model instead of Static Swirl.

- **Crash fixes (11 entities → 4 crash patterns resolved):**
  - MeshArchive_ctor crash (Bridge, Chomper, Wobbly): Changed ai_type to 0 (PopCylinder fallback).
    These entities' native constructors (GameLevel_ctor, BreakBridge_ctor) call MeshArchive_ctor
    which crashes without board pre-loaded mesh data. Now they use PopCylinder with their .MESHWORLD mesh.
  - Level_ctor crash (8Ball, Spinner): Fixed by removing ai_type 15 and 27 from path=NULL override.
    Now .MESH swap code handles them — no native constructor called.
  - MeshWorld_Parse crash (Flag, Flag2): Changed ai_type from 14 (Wavy_ctor) to 0 (PopCylinder).
    Flag.MESHWORLD doesn't exist, causing Wavy_ctor to crash during parsing.
  - Gear/Looper/Cloudscape/Drawbridge crashes: Changed ai_type to 0 (PopCylinder fallback).
    Native constructors crash without board pre-loaded data.

- **Chrome fix:** Changed mesh path from _default.MESHWORLD to Sphere.MESH.

- **Also includes all v54d fixes:**
  - if/else-if chain bug fix (path=NULL overrides now work)
  - Dizzy Arena double-Swirl fix (my_stristr for "Dizzy" in level name)
  - Bell/Fan/SawBlade case handlers with .MESH swap (no vtable override)

- Crash test: 38.8s OK

- **CRASH FIX: Removed vtable overrides for Bell/Fan/SawBlade**
  - v54 added vtable overrides to give PopCylinder objects native Bell/Fan/SawBlade behavior.
  - Crash at 0x0046186E (inside Level_ctor) — mid-instruction EIP = stack corruption.
  - Root cause: Bell/Fan/SawBlade vtable functions have different calling conventions
    (different RET N) than PopCylinder's vtable functions. When the game calls these
    overridden vtable functions on a PopCylinder object, the stack gets corrupted.
  - Same pattern as v53f: "NEVER override a vtable on an object allocated with a
    different size/constructor than the vtable's expected struct."
  - Fix: Removed all vtable overrides. Bell/Fan/SawBlade now use PopCylinder's
    native vtable (static mesh, no native animation/behavior). The mesh swap at
    obj+0x08 gives the correct visual appearance.
- **Also includes all v54 fixes:**
  - if/else-if chain bug fix (path=NULL overrides now work)
  - 8Ball mesh fix (App+0x268 → App+0x248, ball+0x754=1)
  - Bell: PopCylinder + .MESH swap (meshes\Bell)
  - Fan: PopCylinder + .MESH swap (meshes\fanbody)
  - SawBlade: PopCylinder + Level8-Saw.MESHWORLD
- Crash test: 38.6s OK

## v54 — MESH File Hotfix

- **CRITICAL FIX: if/else-if chain bug caused ALL path=NULL overrides to be skipped!**
  - The path determination code uses `if (mesh_path) { path = mesh_path; } else if (ai_type == 15) { path = NULL; }`
  - When mesh_path is non-NULL (which it always is for 8ball, Bell, Fan, etc.),
    the first branch sets `path = mesh_path` and ALL else-if branches are SKIPPED.
  - The `ai_type == 15 → path = NULL` override NEVER RAN!
  - Fix: Added a separate override AFTER the if/else-if chain that forces
    `path = NULL` for all entity types that handle their own mesh loading
    (types 12-16, 22, 28, 30-33, 41, 42).
- **8Ball (type 15): Fixed Static Swirl issue!**
  - Root cause 1: if/else-if bug (above) — path was never set to NULL, so
    the .MESH swap code created a PopCylinder with Swirl mesh instead of
    running case 15 (BadBall_ctor).
  - Root cause 2: Ball_Render reads mesh from `App+0x244[ball+0x754 * 4]`,
    NOT from `ball+0x10`. The old code stored MeshNode at the wrong offset.
  - Fix: Copy 8Ball mesh pointer from `App+0x268` to `App+0x248` (slot 1),
    set `ball+0x754 = 1`. Same pattern as `cEnt_process_custom_tags`.
- **Bell (type 30): Fixed!**
  - Bell_ctor calls `Level_ctor` (no mesh). vtable[1] = Rotator_Update needs vertex data.
  - v54 uses PopCylinder_ctor with `meshes\Bell` .MESH swap + vtable override to 0x004D5330.
- **Fan (type 31): Fixed!**
  - Same Level_ctor issue. Uses `meshes\fanbody` .MESH swap + vtable override to 0x004D5180.
- **SawBlade (type 32): Fixed!**
  - Same Level_ctor issue. Uses `levels\Level8-Saw` .MESHWORLD + vtable override to 0x004D5240.
- **Also includes all v53g-5 fixes:**
  - Fixed critical despawn bug (entities destroyed on No-GRID levels)
  - Fixed Trapdoor/Odd_Lifter mesh leak + NULL guard
  - All 34 constructor addresses verified via Ghidra
- Crash test: 38.6s OK

## v53g-5

- **CRITICAL FIX: Entities despawned immediately on levels without GRID points**
  - The "No GRID points found" branch called `cEnt_despawn_all_rotaters()` which
    calls `vtable[11]` (RemoveAndFree) on ALL custom entities that were just
    spawned by `process_rotaters()`. This destroyed every custom entity on
    levels that have no GRID reference points.
  - Fix: Removed the `cEnt_despawn_all_rotaters()` call from the No-GRID branch.
    Entities are still properly cleaned up on level exit via the normal path.
- **FIX: Trapdoor/Odd_Lifter mesh leak and NULL mesh guard**
  - Trapdoor_ctor (0x438290) and Odd_Lifter_ctor (0x434E60) read mesh from
    `board+0x878+0x594/0x5C8` (App mesh table), NOT from the mesh parameter.
    The spawn function was loading a mesh file that was never used (memory leak).
  - Fix: Set `path=NULL` for types 41 and 42 so no mesh file is loaded.
  - Added safety guard: checks if App mesh table entry is valid before calling
    the constructor. If NULL, the entity is skipped (logged) instead of crashing.
- **Verified via Ghidra decompilation**:
  - All 34 constructor addresses confirmed correct
  - All 24 alloc size constants confirmed correct
  - All 8 Stands_ctor family calling conventions confirmed (v53g-4 fixes hold)
  - Stands_ctor (0x462850) internally calls SpriteAnim_Ctor, AthenaList_Init,
    Timer_Init, and SpatialTree_CloneToLevel — all verified safe
  - Collision offset logic (col_off=0 for all except Rotator 1-6) confirmed
  - Board list additions (0x2578, 0xCD4, 0x8B8, scene+0x1C) verified correct
  - Despawn logic (skip types 30-33) verified correct
- Crash test: 39.7s, no crash

## v53g-4

- **REVERTED v53f crash regression**: use_board_level_as_mesh and visual mesh swap
  caused 11 entities to crash (Rotator, Pendulum, Looper, Gear, Swirl, Flickfloor1/2,
  Flickring, Spinner, Trode, Bonk). All entities now load their own MESHWORLD files
  directly (back to v53e behavior).
- **Fixed leftover 0x95E0**: Per-frame monitoring code had a leftover board+0x95E0
  reference that corrupted the game's update list, causing crashes at 0x452376.
- **Fixed Flag/Flag2 crashes**: FlagWaver_Ctor creates a global renderer, not a
  per-entity object. Changed Flag from type 12 (FlagWaver) to type 14 (Wavy_ctor)
  with Flag.MESHWORLD path, same as Flag2.
- **Wobbly wobble**: Added type 8 (GameLevel) to board+0x8B8 (Scene_Update list)
  so Rotator_Update (vtable[1]) is called per-frame for vertex deformation.
- **Bridge tilt animation**: Changed Bridge from type 16 (custom PopCylinder) to
  type 34 (BreakBridge_ctor, 0x436D70) with Pendulum vtable and Rotator_Update.
  Added to board+0x8B8 for per-frame vertex deformation.
- **_default.MESHWORLD placeholder**: Entities with no real mesh (Bumper, Tarpit,
  Chrome) use levels\_default as placeholder.
- Known issues: .MESH entities (8ball, Bell, Fan, Funball) show as static Swirl
  due to MeshWorld structure mismatch. Needs further investigation.

## v53f

- **CRITICAL FIX: Crash root cause found and fixed**
  - Constructors like ArenaStands_ctor, Rotator_ctor, Looper_ctor, etc. internally call
    Stands_ctor/Level_RenderCtor which dereference SceneObject+0x440 (vertex data)
  - Separately-loaded .MESHWORLD meshes had NULL vertex data at this offset
  - Fix: pass the board's own Level (board+0x8AC) as the mesh parameter
  - Board's Level has fully-loaded vertex data - no more NULL dereference crashes
- **Visual mesh swap after construction**
  - After constructing with board Level, swap obj+0x08 (MeshWorld*) to desired visual mesh
  - This gives correct appearance while keeping valid collision/vtable from board Level
- **Rotator oscillation fixed**
  - Native render checks "if angle > 2.0" and "if angle < -2.0" to reverse direction
  - Fix: clamp angle to [-1.99, 1.99] every frame to prevent reversal
  - Constant rotation now works correctly

## v53e

- All mod functions renamed with `cEnt_` prefix to separate from game's originals
- Cloned all constructors/wrappers - mod now works with copies, not game functions directly
- README pipe alignment fixed
- README restructured: version changelogs moved to this file, README keeps only reference tables

## v53d

- Fixed 8ball/BadBall: now loads 8ball.MESH via cEnt_MeshNode_ctor and stores at ball+0x10
- Fixed BadBall alloc size: 0xC98 (was 0xC70)
- Added cEnt_Bonk_ctor (type 33, 0x438850, 0x1200) - Warm-Up Bonk, self-loads level5-bonk
- Bell/Fan/SawBlade: reverted to PopCylinder (type 0) - their ctors call Level_ctor (no mesh)
  and crash during board update because the vtable update method calls LoadMesh with invalid state

## v53c

- Isolated and cloned behaviors: Chomper, Chrome, Funball, Tarbubble, Waterwheel
- Fixed cEnt_Gear_ctor (9 params, was using 6-param Rotator typedef - would crash!)
- Fixed Looper size (0x1500, was 0x1508)
- Added cEnt_Spinner_Level_ctor (Expert Race "BRIDGE")
- Added Cloudscape entity (Sky Race clouds, cEnt_Sprite_ctor)
- Named cEnt_ wrappers for all Neon Race objects (cEnt_DFloor1-4, cEnt_FlickRing_ctor, cEnt_Trode_ctor)

## v53b

- **Deep Ghidra decompilation** of every `CreateDynamicObjects` function in the game
- Added 7 new constructor types with correct alloc sizes and calling conventions
- Fixed 10 entities that were using the wrong `_ctor`
- NULL mesh paths now use `levels\_default` as placeholder (your `_default.MESHWORLD` file)

### Entities fixed in v53b

| Entity       | Old _ctor   | New _ctor             | Address  | Size   |
| ------------ | ----------- | --------------------- | -------- | ------ |
| Drawbridge   | PopCylinder | cEnt_Glass_Level_ctor | 0x4384A0 | 0x113C |
| Flag         | PopCylinder | cEnt_FlagWaver_Ctor   | 0x46AF30 | 0x8C   |
| Flickfloor1  | PopCylinder | cEnt_ArenaStands_ctor | 0x43E450 | 0x1104 |
| Flickfloor2  | PopCylinder | cEnt_ArenaStands_ctor | 0x43E450 | 0x1104 |
| Flickring    | PopCylinder | cEnt_ArenaStands_ctor | 0x43E450 | 0x1104 |
| Glassbreaker | PopCylinder | cEnt_Secret_ctor      | 0x43DFB0 | 0x10EC |
| Judge        | PopCylinder | cEnt_Gear_Level_ctor  | 0x43A150 | 0x1100 |
| Sign         | PopCylinder | cEnt_Sign_ctor        | 0x443B90 | 0x10FC |
| Trode        | PopCylinder | cEnt_ArenaStands_ctor | 0x43E450 | 0x1104 |
| Wobbly       | PopCylinder | cEnt_GameLevel_ctor   | 0x4351F0 | 0x1524 |
