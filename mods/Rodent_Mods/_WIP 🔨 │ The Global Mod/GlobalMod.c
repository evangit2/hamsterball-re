/*
 * The Global Mod - universal cross-level object injection (bass.dll proxy).
 *
 * Source is split into chapters in src/ (single translation unit).
 * GlobalMod.c is only the aggregator: build command is unchanged,
 * output is still one bass.dll. Edit the chapter file, rebuild.
 *
 *  gm_00_core.c         - header, RVAs, offsets, ext heap, globals, helpers
 *  gm_01_bumpers.c      - bumper physics, game fn pointers, mesh path table
 *  gm_02_ctor.c         - UniversalBoardCtorLogic + install thunks
 *  gm_03_features.c     - InitBridge + Feature blocks
 *  gm_04_render_update.c - render + BoardUpdate + RaceState
 *  gm_05_create.c       - S1Ensure + UniversalCreateDynamicObjects
 *  gm_06_collision.c    - collision config + UniversalDispatchCollision
 *  gm_07_postscan.c     - PostSetup + S1/collision scans
 *  gm_08_hooksmain.c    - constructor + hooks + DebugLog + vtable + DllMain
 *  gm_09_pennants.c      - per-name pennant routing (FLAG*=rect, PENNANT*=triangle)
 *  gm_10_neon.c          - Neon glow follower lights, S3-gap gated (Step 6c)
 *  gm_11_dynamic.c       - Dynamic drawer cabinet: S1 counts, ext growth, slot table (Global Mod v01)
 */
#include "src/gm_00_core.c"
#include "src/gm_01_bumpers.c"
#include "src/gm_02_ctor.c"
#include "src/gm_03_features.c"
#include "src/gm_04_render_update.c"
#include "src/gm_05_create.c"
#include "src/gm_06_collision.c"
#include "src/gm_07_postscan.c"
#include "src/gm_08_hooksmain.c"
#include "src/gm_09_pennants.c"
#include "src/gm_10_neon.c"
#include "src/gm_11_dynamic.c"
