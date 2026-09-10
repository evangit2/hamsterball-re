/*
 * LevelFeatures_Loader - universal cross-level object injection (bass.dll proxy).
 *
 * Source is split into chapters in src/ (single translation unit).
 * LevelFeatures.c is only the aggregator: build command is unchanged,
 * output is still one bass.dll. Edit the chapter file, rebuild.
 *
 *  lf_00_core.c         - header, RVAs, offsets, ext heap, globals, helpers
 *  lf_01_bumpers.c      - bumper physics, game fn pointers, mesh path table
 *  lf_02_ctor.c         - UniversalBoardCtorLogic + install thunks
 *  lf_03_features.c     - InitBridge + Feature blocks
 *  lf_04_render_update.c - render + BoardUpdate + RaceState
 *  lf_05_create.c       - S1Ensure + UniversalCreateDynamicObjects
 *  lf_06_collision.c    - collision config + UniversalDispatchCollision
 *  lf_07_postscan.c     - PostSetup + S1/collision scans
 *  lf_08_hooksmain.c    - constructor + hooks + DebugLog + vtable + DllMain
 *  lf_09_pennants.c      - per-name pennant routing (FLAG*=rect, PENNANT*=triangle)
 *  lf_10_neon.c          - Neon glow follower lights, S3-gap gated (Step 6c)
 */
#include "src/lf_00_core.c"
#include "src/lf_01_bumpers.c"
#include "src/lf_02_ctor.c"
#include "src/lf_03_features.c"
#include "src/lf_04_render_update.c"
#include "src/lf_05_create.c"
#include "src/lf_06_collision.c"
#include "src/lf_07_postscan.c"
#include "src/lf_08_hooksmain.c"
#include "src/lf_09_pennants.c"
#include "src/lf_10_neon.c"
