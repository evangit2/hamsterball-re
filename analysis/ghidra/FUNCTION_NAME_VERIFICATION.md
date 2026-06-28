# Function Name Verification Report

**Total functions decompiled:** 3,978
**Functions analyzed for name accuracy:** All 3,978
**Confirmed misnomers:** 7

## Summary

All 3,305 previously-undecompiled functions have been decompiled from GhidraMCP and saved to `analysis/ghidra/decompilations/batch_auto/`. Each function was analyzed for name/behavior mismatches using the following methods:

- String literal analysis (what does the function actually reference?)
- Call graph analysis (what functions does it call?)
- Cross-reference verification (who calls it and how?)
- Constructor call counting (does 'Create' actually create multiple types?)
- Rendering call detection (is a 'Tick' actually doing rendering?)

## Confirmed Misnomers

### TimerDisplay (0x004298c0)

- **Severity:** CRITICAL
- **Suggested name:** `LoadingScreenGadget_Factory`
- **Description:** Named 'TimerDisplay' but is actually the LoadingScreenGadget factory function. Allocates 0x3628 bytes for a LoadingScreenGadget object and loads ALL game resources: 5 fonts (showcardgothic28/72/14/16, arialnarrow12bold), 40+ textures (hammy1-3.png, blueblot.png, goal.png, locktile.png, arrow1.png, etc.), 14 meshes, 7 levels, 55 sounds. The only timer-related string is 'timerblot.png'. This is the main game asset loading function, not a timer display.
- **Evidence:** operator_new(0x3628) + LoadingScreenGadget_Ctor + 199 string literals for fonts/textures/meshes

### RegisterDialog_Render (0x00447920)

- **Severity:** MODERATE
- **Suggested name:** `PurchaseScreen_Render`
- **Description:** Named 'RegisterDialog_Render' suggesting a generic dialog, but this is specifically the game PURCHASE/REGISTRATION screen. Shows 'REGISTER HAMSTERBALL!', 'CLICK HERE TO BUY!', customer name/serial number input fields, and 'UNLOCK!' button. The name is technically correct (it renders a registration dialog) but 'PurchaseScreen_Render' would be more descriptive.
- **Evidence:** Strings: 'REGISTER HAMSTERBALL!', 'CLICK HERE TO BUY!', 'CUSTOMER NAME:', 'SERIAL NUMBER:', 'UNLOCK!'

### DispatchCollisionEvents (0x0040c5d0)

- **Severity:** CRITICAL
- **Suggested name:** `LevelObjectCollisionHandler`
- **Description:** Named 'DispatchCollisionEvents' after just ONE of ~15 event types it handles. This is actually the GENERAL level object collision/event handler called from 28+ sites. Handles: N:SECRET, N:UNLOCKSECRET, E:NODIZZY, E:SAFESWITCH, E:LIMIT, E:BREAK, E:JUMP, E:ACTION, E:TRAJECTORY, N:NOCONTROL, N:WATER, N:TARPIT, DROPIN, PIPEBONK, POPOUT, N:GOAL, N:MOUSETRAP. Does NOT create anything - it's a collision response handler.
- **Evidence:** 28 xrefs (all UNCONDITIONAL_CALL), 0 ctor calls, 18 __stricmp calls dispatching events

### RaceGoalReached_Tick (0x0044df70)

- **Severity:** MODERATE
- **Suggested name:** `RaceResultsScreen_Render`
- **Description:** Named 'Tick' but is primarily a RENDER function. Displays the race results screen with 'BEST RACE TIME:', 'WEASEL\'S TIME:', 'BROKEN BALLS:', 'DIZZIED BALLS:', 'BRONZE TIME:', 'SILVER TIME:' and 'Click the mouse to continue!'. Has 56 Matrix_Scale4x4 calls and 14 UI_DrawTextCentered calls. Should be named RaceResultsScreen_Render.
- **Evidence:** 56 Matrix_Scale4x4 + 14 UI_DrawTextCentered + race result strings

### CreateExpertLevelObjects (0x0040e250)

- **Severity:** MODERATE
- **Suggested name:** `CreateMechanicalObjects2`
- **Description:** Named 'CreateExpertLevelObjects' but creates 6 different object types: BONK, SLOW/SUPER, UP, SAWBLADE, BRIDGE, and calls Bonk_ctor, TowerLevel_Ctor, Sawblade_Level_Ctor, Spinner_Level_ctor, Gear_Level_ctor, Tipper_Level_Ctor. Should be named after the category of objects it creates, not just one.
- **Evidence:** 6 distinct _ctor calls, handles BONK/SLOW/SUPER/UP/SAWBLADE/BRIDGE/NEG/JUDGE/BELL strings

### CreateUpLevelObjects (0x004117b0)

- **Severity:** MODERATE
- **Suggested name:** `CreateRotatorsAndPendulums`
- **Description:** Named 'CreateUpLevelObjects' but creates Rotator and Pendulum objects. Handles LIFTER, SPEEDCYLINDER, TIMEBUTTON strings but the actual objects created are Rotator_ctor_sound, Rotator_ctor_nosound, and Pendulum_ctor.
- **Evidence:** 3 distinct _ctor calls: Rotator_ctor_sound, Rotator_ctor_nosound, Pendulum_ctor

### HandleArenaCollisionEvents (0x00412850)

- **Severity:** MODERATE
- **Suggested name:** `CreateArenaObjects`
- **Description:** Named 'HandleArenaCollisionEvents' but handles N:SPINNER, N:BUMPER, E:LAUNCH, LAUNCHPOINT, EXPLODEHELPER, E:CALLHAMMER, E:HAMMERCHASE, E:CATAPULTBOTTOM. Only creates ArenaScoreParticle_ctor. This is an arena-specific object factory, not just a spinner creator.
- **Evidence:** Handles 8 different event/object types, only 1 ctor call (ArenaScoreParticle_ctor)

## Notes

- Many 'Render' functions were flagged as suspicious for 'no draw calls', but this is a FALSE POSITIVE: D3D8 rendering uses vtable dispatches (`(**code**)(...)` patterns) which don't show up as named function calls. These names are CORRECT.
- CRT library functions (scanf, printf, pow, RaiseException) and codec functions (IDCT, IMDCT, Inflate, Vorbis) are correctly named — they're VS2003 CRT and bundled codec libraries.
- D3DX functions (CreateTextureFromFile, CreateMeshFromFormat, OptimizeMesh, WeldVertices, SkinMesh) are correctly named — they're the D3DX utility library bundled with the game.
- The `Create*` function naming pattern is systematically misleading: each 'Create' function is actually a factory for MULTIPLE object types, named after just one. This appears to be how the original developers named them (after the first/primary object type in the switch statement).
