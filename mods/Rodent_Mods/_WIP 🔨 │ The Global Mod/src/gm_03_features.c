/* Chapter 3 - InitBridge + Feature blocks (bridge/swirl/windmill/badball/bumper/neon/sky) (LevelFeatures.c lines 2009-2814) */
/* ═══════════════════════════════════════════════════════════════════════════
 * InitBridge — replicates LevelBoard_Intermediate_ctor bridge setup
 *
 * Steps (from Ghidra decompilation of 0x0041cb20):
 *   1. operator_new(0x10d0) → Level_MeshWorldCtor(mem, gfx, "Levels\\Level2-Bridge") → board+UNI_BONK_STORE
 *   2. operator_new(0x10d0) → Level_RenderCtor(mem, meshWorld) → board+UNI_SAW1_OBJ
 *   3. TipperVisual_Attach(renderObj, meshWorld)
 *   4. board+UNI_BRIDGE_ANGLE = 0x42340000 (float 45.0)
 *   5. board+UNI_BRIDGE_STATE = 0
 *   6. board+UNI_BRIDGE_COUNTER = 0x32 (50)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void InitBridge(void *board) {
    if (!g_operatorNew || !g_LevelMeshWorldCtor || !g_LevelRenderCtor ||
        !g_TipperVisualAttach) return;
    void *ext = EnsureBoardExt(board);
    if (!ext) return;

    /* Don't create a second bridge if one is already loaded. */
    if (*(void **)((char *)ext + UNI_BONK_STORE) != NULL) {
        DebugLog("InitBridge: bridge already exists, skipping");
        return;
    }

    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x200)) return;
    void *gfx = *(void **)((char *)app + 0x174);
    if (!gfx) return;

    /* Step 1: MeshWorld — store at ext+UNI_BONK_STORE */
    void *meshMem = g_operatorNew(0x10D0);
    if (!meshMem) return;
    void *meshWorld = g_LevelMeshWorldCtor(meshMem, gfx, "Levels\\\\Level2-Bridge");
    *(void **)((char *)ext + UNI_BONK_STORE) = meshWorld;

    /* Step 2: RenderObj — store at ext+UNI_SAW1_OBJ */
    void *renderMem = g_operatorNew(0x10D0);
    void *renderObj = NULL;
    if (renderMem) {
        renderObj = g_LevelRenderCtor(renderMem, meshWorld);
    }
    *(void **)((char *)ext + UNI_SAW1_OBJ) = renderObj;

    /* Step 3: TipperVisual_Attach */
    if (renderObj && meshWorld) {
        g_TipperVisualAttach(renderObj, meshWorld);
    }

    /* Steps 4-6: Bridge config values — unified in ext */
    *(DWORD *)((char *)ext + BRIDGE_PARAM1) = 0x42340000;  /* 45.0f */
    *(DWORD *)((char *)ext + BRIDGE_PARAM2) = 0;
    *(DWORD *)((char *)ext + BRIDGE_PARAM3) = 0x32;       /* 50 */
    // OFF_* mirrors removed: UNI_* at ext+0x8634/38/3C is canonical inside ext heap.
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: Bridge Animation (Intermediate)
 * 4-state machine: wait → tilt down → wait → tilt back
 * Replicates FUN_0041CC90 (Intermediate Board_Update)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void Feature_BridgeAnimation(void *board, int level) {
    // Pause gate: board+0x874 !=0 means paused (ESC) — freeze anim + sound
    if (*(BYTE*)((char*)board+0x874)) return;
    if (!g_SceneUpdate) return;
    void* ext = GetBoardExt(board);
    int ballCount = g_AthenaListGetSize((void *)((char *)board + UNI_RACE_BALL_LIST));
    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (ballCount == 1 && app && !IsBadReadPtr((void *)app, 0x600)) {
        DWORD ball = *(DWORD *)(app + APP_BALL_PTR);
        if (ball && !IsBadReadPtr((void *)ball, 0x200) && *(char *)(ball + 0x14C))
            return;
    }
    DWORD renderObj;
    float *anglePtr; int *statePtr; int *counterPtr; float *pivotX; float *pivotY; float *pivotZ;
    if (!ext) return;
    // UNI_* is canonical — writers (InitBridge, LoadExtraMeshes) store at ext+UNI_*.
    renderObj = *(DWORD *)((char *)ext + UNI_BONK_STORE);
    anglePtr = (float*)((char*)ext + UNI_BRIDGE_ANGLE);
    statePtr = (int*)((char*)ext + UNI_BRIDGE_STATE);
    counterPtr = (int*)((char*)ext + UNI_BRIDGE_COUNTER);
    pivotX = (float*)((char*)ext + UNI_BRIDGE_PIVOT_X);
    pivotY = (float*)((char*)ext + UNI_BRIDGE_PIVOT_Y);
    pivotZ = (float*)((char*)ext + UNI_BRIDGE_PIVOT_Z);
    if (!renderObj) return;
    int state = *statePtr;
    switch (state) {
    case 0: { int c = *counterPtr - 1; *counterPtr = c; if (c < 1) { *statePtr = 1; } } break;
    case 1: {
            float angle = *anglePtr - 3.0f;
            *anglePtr = angle;
            if (angle < 0.0f) {
                *anglePtr = 0.0f; *counterPtr = 0x7D; *statePtr = 2;
                if (app && !IsBadReadPtr((void *)app, 0x800) && g_SoundPlay3D) {
                    DWORD snd = *(DWORD *)(app + APP_SOUNDFX_47C);
                    if (snd) g_SoundPlay3D((void *)snd, *pivotX, *pivotY, *pivotZ, 1.0f);
                }
                if (g_SceneForEachBallSetVelocity && g_Vec3CopyUpd) {
                    /* Native (0x41CDAE-0x41CDBF): Vec3_Copy(stackvec, &pivot)
                     * then call with (x, y, z) at stack top — full pivot, no
                     * radius. Passing (y, z, 0.5) marked the wrong balls. */
                    float pivot[3]; g_Vec3CopyUpd(pivot, pivotX);
                    g_SceneForEachBallSetVelocity(board, pivot[0], pivot[1], pivot[2]);
                }
            }
        } break;
    case 2: { int c = *counterPtr - 1; *counterPtr = c; if (c < 1) { *statePtr = 3; } } break;
    case 3: {
            float angle = *anglePtr + 0.5f;
            *anglePtr = angle;
            /* Rise creak-tick — native Intermediate Board_Update state 3
             * (0x41CDEF, disasm-verified 2026-09-11): Y=(angle)/7, q=(int)Y;
             * Play3D (App+0x478 sound at pivot, scale 0.5) iff Y==(float)q,
             * i.e. each time the rising angle hits a multiple of 7. */
            {
                float creakY = angle / 7.0f;
                int creakQ = (int)creakY;
                if (creakY == (float)creakQ && app && !IsBadReadPtr((void *)app, 0x800) && g_SoundPlay3D) {
                    DWORD snd = *(DWORD *)(app + 0x478);
                    if (snd) g_SoundPlay3D((void *)snd, *pivotX, *pivotY, *pivotZ, 1.0f);
                }
            }
            if (angle >= 45.0f) { *anglePtr = 45.0f; *counterPtr = 0x4B; *statePtr = 0; }
            /* Rider carry — native Intermediate Board_Update state 3
             * (0x41CE9D, Ghidra-verified 2026-09-11: 0x402BF0=Vec3_Copy,
             * 0x40A0B0=(mat,vec-inplace) stdcall, ECX officially scratch):
             * per ball with +0x778 set, rotate (pos-pivot) by -0.5 about Z
             * (matches the +0.5/frame rise rate), add pivot back. Runs only
             * while rising (states 0-2 jmp straight to end). Kinematic
             * carry — native pos-writes here; forces cannot do rigid carry,
             * so the native write is mirrored exactly (not accumulators). */
            if (g_TimerInit && g_TimerCleanup && g_GfxScaleZ && g_MatrixTransformVec3) {
                int bn = *(int *)((char *)board + UNI_BALL_COUNT);
                DWORD barr = *(DWORD *)((char *)board + UNI_BALL_ARRAY);
                if (bn > 0 && bn < 64 && barr && !IsBadReadPtr((void *)barr, bn * 4)) {
                    int bi;
                    for (bi = 0; bi < bn; bi++) {
                        DWORD cb = ((DWORD *)barr)[bi];
                        float cd[3]; char ctimer[256];
                        if (!cb || IsBadReadPtr((void *)cb, 0x800)) continue;
                        if (!*(BYTE *)((char *)cb + 0x778)) continue;
                        cd[0] = *(float *)((char *)cb + 0x164) - *pivotX;
                        cd[1] = *(float *)((char *)cb + 0x168) - *pivotY;
                        cd[2] = *(float *)((char *)cb + 0x16C) - *pivotZ;
                        /* FIX v13.8: +0x778 is sticky (native never clears it:
                         * two movb $1 sites, no clear). A ball marked by one
                         * touch is still marked 200+ units away next rise and
                         * gets swung in a huge arc (fling/teleport, then
                         * killplane respawn). Gate on the ForEachBall radius
                         * (500, 0x4CF3D8): too far to be on the deck means
                         * don't carry, and clear the stale mark so one touch
                         * doesn't doom every future rise. */
                        { float d2 = cd[0]*cd[0] + cd[1]*cd[1] + cd[2]*cd[2];
                          if (d2 > 250000.0f) {
                            *(BYTE *)((char *)cb + 0x778) = 0;
                            continue;
                          } }
                        /* (BRIDGECARRY per-frame log removed v13.13: file I/O on the
                         * game thread lagged the ride; carry verified working.) */
                        /* v13.9 MANUAL carry (pure C, exact): native ScaleZ output
                         * depends on its exact caller stack frame (uninitialized
                         * reads) and scrambles from our call site (SCALETEST).
                         * The deck pitches about X: x unchanged, YZ rotated by
                         * -0.5 deg/frame (ride log: rel rotates ~-38 deg in YZ
                         * over the 45 deg rise). */
                        { float mc = 0.9999619f, ms = -0.0087265f;
                          float oy = cd[1], oz = cd[2];
                          cd[1] = mc*oy - ms*oz; cd[2] = ms*oy + mc*oz; }
                        *(float *)((char *)cb + 0x164) = cd[0] + *pivotX;
                        *(float *)((char *)cb + 0x168) = cd[1] + *pivotY;
                        *(float *)((char *)cb + 0x16C) = cd[2] + *pivotZ;
                    }
                }
            }
        } break;
    }
    /* Pose EVERY frame (all states). Binary-verified against the last
     * working build (lfl_diag): DIRECT Scale call (compiler frame — the
     * hand-rolled wrapper frame crashed the render), EDX=vtable on the
     * first draw call. Deterministic seed (zero + TimerInit) so the Scale
     * output can't depend on our stack garbage (raw-stack seed rendered
     * the deck level-sized). */
    if (g_TimerInit && g_TimerCleanup && g_GfxScaleZ && g_GfxSetPosition && g_MatrixTransformVec3 && app) {
        void *gfx = *(void **)(app + 0x174);
        if (gfx) {
        char timerBuf[256];
        { int zi; for (zi = 0; zi < 256; zi++) timerBuf[zi] = 0; }
        g_TimerInit(timerBuf);
        /* Axis flag (binary-verified, Intermediate deck update): the dword
         * at timer+0xC0 selects the rotation axis for 0x457CC0. Native sets
         * 0 for the rider-carry call (0x41CF20, -0.5 deg) and 1 for the deck
         * pose (0x41CFCA, -angle). TimerInit zeroes it, so without this the
         * deck posed with the carry axis — spinning like a windmill
         * instead of pitching on its hinge. */
        *(int *)(timerBuf + 0xC0) = 1;
        g_GfxScaleZ(timerBuf, -*anglePtr);
        g_GfxSetPosition(timerBuf, *pivotX, *pivotY, *pivotZ);
        DWORD *renderVtbl = *(DWORD **)renderObj;
        if (renderVtbl) {
            void (__fastcall *fn58)(DWORD, DWORD) = (void (__fastcall *)(DWORD, DWORD))renderVtbl[0x16];
            void (__thiscall *fn54)(DWORD, void *) = (void (__thiscall *)(DWORD, void *))renderVtbl[0x15]; /* native: ECX=obj + push timer, ret $0x4 */
            if (fn58) fn58((DWORD)renderObj, (DWORD)renderVtbl);
            if (fn54) fn54((DWORD)renderObj, timerBuf);
        }
        g_TimerCleanup(timerBuf);
        }
    }
    /* Board mirror: native keeps angle/state/counter at board+0x4380/84/88
     * (the TipperVisual + game lists read BOARD fields). Mirror every frame
     * so the posed mesh never reads stale values — except where native
     * objects own those fields (Master: never clobber live slots). */
    if (BoardHasOffset(board, 0x4380, 12) && !NativeOwnsBoardFields(board)) {
        *(float *)((char *)board + 0x4380) = *anglePtr;
        *(int *)((char *)board + 0x4384) = *statePtr;
        *(int *)((char *)board + 0x4388) = *counterPtr;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: Tipper Visuals (Dizzy + any universal board with tippers)
 * Native render never draws OUR 0x2578 objects (same gap as the bridge
 * before its manual pose), so tipper planks are solid but invisible.
 * Draw each tipper's visual (obj+0x10D4) with its static yaw every frame:
 * TimerInit + ScaleY(yaw) + SetPosition(x,y,z) + obj v[0x54] + Cleanup —
 * the gluebie-tail draw pattern. Tip deflection animation later.
 * Skipped on native-ctor boards (native render draws those itself).
 * ═══════════════════════════════════════════════════════════════════════════ */
static void *Gm_DynSlotPtr(void *ext, int type, int idx);
static int Gm_DrawerCount(void *ext, int type);
void Gm_LogSkip(const char *line);
static int *Gm_DynCounts(void *ext);
/* TIPPER VISUALS (Ghidra-verified): native render never draws OUR 0x2578
 * objects, so tipper planks are solid but invisible. The tipper visual
 * (obj+0x10D4) carries cloned spatial trees (Stands_ctor/TipperVisual_ctor);
 * the shared update tail positions them via obj v[0x58]() (no args) then
 * obj v[0x54](obj, posed timer) -- trampolines through obj+0x434 (Stands
 * node, always valid). No render lists (board+0x8B0 is unbuilt garbage on
 * every board -- appending crashed/hung twice), no object walks (poses
 * stored at create). Per tipper: TimerInit + ScaleY(yaw) + SetPosition +
 * v[0x58] + v[0x54] + Cleanup. Static yaw for now; deflection later. */
/* v13.28: does the racestate that WILL run for this board already walk
 * board+0x2578 calling v[11] (native 0x41B130 shape: lea 0x2578 + call
 * *0x2c)? If yes, every 0x2578 member (tippers, bonk, pops, halves) is
 * natively driven and our pump must STAND DOWN (it double-drives: Master
 * slot tippers ran 2x). If no (Intermediate custom 0x41CC90 has no walk),
 * the pump is the sole driver. Mirrors UniversalRaceState's dispatch
 * exactly (same level/content chain) so the scanned fn is the one that
 * runs — behavior-gated, never slot-gated. Result cached per board. */
static int NativeWalkPresent(void *board) {
    void *ext = GetBoardExt(board);
    int *cache;
    void *fn = NULL;
    unsigned char *p;
    int i;
    if (!ext) return 0;
    cache = (int *)((char *)ext + UNI_PUMP_SKIP);
    if (*cache == 1) return 1;
    if (*cache == 2) return 0;
    /* Universal racestate (2026-11-25): every board runs base. The scan
     * below verifies base's shape (0x2578 walk present?) — no table. */
    fn = (void *)g_BoardUpdateRaceState;
    if (fn && !IsBadReadPtr((void *)fn, 0x1000)) {
        p = (unsigned char *)fn;
        for (i = 0; i < 0x1000 - 6; i++) {
            /* lea 0x2578(%reg): 8D /r 78 25 00 00 — direct walk */
            if (p[i] == 0x8D && p[i+2] == 0x78 && p[i+3] == 0x25 &&
                p[i+4] == 0x00 && p[i+5] == 0x00) {
                *cache = 1;
                return 1;
            }
            /* v13.29: call 0x41B130 (shared base): customs (Up 0x420660
             * etc.) walk via the base — same drive, must also stand down.
             * E8 rel32 at i targets fn+i+5+disp. */
            if (p[i] == 0xE8) {
                int disp;
                memcpy(&disp, p + i + 1, 4);
                if ((void *)(p + i + 5 + disp) == (void *)0x41B130) {
                    *cache = 1;
                    return 1;
                }
            }
        }
    }
    *cache = 2;
    return 0;
}

static void Feature_TipperVisual(void *board, int level) {
    /* v13.28: STAND DOWN when the native racestate already walks 0x2578
     * (base 0x41B130 shape). Native drives every member 1x/frame; our pump
     * on top made Master-slot tippers run 2x. No-walk slots (Intermediate
     * custom) still need the pump as sole driver. Behavior-gated, cached. */
    if (NativeWalkPresent(board)) return;
    /* Plan A (2026-11-03): drive native slot-11 update 0x437A40 per tipper.
     * Native owns the 0-3 fire/hold/blink machine, the Append/Remove of vis
     * on board+0x8B0+0x18, the Play3D fire sound, and the pose tail. Same
     * gates as bridge (pause + board+0x3A44 live flag + native-ctor skip).
     * If this crashes/misbehaves, Plan B = replicate the machine in C
     * (old manual static-yaw draw kept below under #if 0). */
    if (*(BYTE*)((char*)board+0x874)) return;
    if (!g_TipperUpdate) return;
    /* v13.27: the old board+0x3A44 live-flag gate is removed — Setup zeroes
     * 0x3A44 and nothing ever sets it, so the gate fired on every universal
     * board and the whole driver (tippers + native pump) never ran. Pause
     * byte above is the real liveness gate. */
    void* ext = GetBoardExt(board);
    if (!ext) return;
    if (*(int *)((char *)ext + UNI_NATIVE_HANDLERS)) return;
    if (!g_AthenaListGetSize) return;
    int ni = g_AthenaListGetSize((void *)((char *)board + UNI_OBJ_LIST));
    if (ni <= 0) return;
    DWORD *items = *(DWORD **)((char *)board + UNI_OBJ_LIST + 0x40C);
    if (!items || IsBadReadPtr((void *)items, ni * 4)) return;
    int i;
    /* v13.27: same scan also pumps native slot-11 updates for our other
     * dynamics (bonk bob 0x43F930, pop reposition 0x43DED0, halves fall).
     * Native calls these every frame; on universal boards nothing does, so
     * they freeze. vtbl read live per object (no hardcoded update addrs);
     * all slot-11s are __thiscall ECX-only by native convention. Halves are
     * static until BreakBridge sets +0x10FC (their update no-ops before). */
    DWORD halfVtbl = 0;
    {
        void *h1 = *(void **)((char *)ext + UNI_BBRIDGE1_OBJ);
        if (h1 && !IsBadReadPtr((void *)h1, 0x10FC) && *(DWORD *)h1 && !IsBadReadPtr(*(void **)h1, 0x30))
            halfVtbl = *(DWORD *)h1;
    }
    for (i = 0; i < ni; i++) {
        DWORD obj = items[i];
        DWORD vt;
        void (__thiscall *upd)(void *);
        if (!obj || IsBadReadPtr((void *)obj, 0x1104)) continue;
        vt = *(DWORD *)obj;
        if (vt == 0x4D4ED8) { g_TipperUpdate((void *)obj); continue; }
        if (vt != 0x4D58F0 && vt != 0x4D5120 && vt != halfVtbl) continue;
        if (IsBadReadPtr((void *)vt, 0x30)) continue;
        upd = (void (__thiscall *)(void *))((DWORD *)vt)[11];
        if (upd) upd((void *)obj);
    }
    return;
#if 0
    /* Plan B fallback: old manual static-yaw draw (no state machine). */
    if (!g_SceneUpdate) return;
    if (*(int *)((char *)board + 0x3A44) == 0) return;
    void* ext2 = GetBoardExt(board);
    if (!ext2) return;
    if (*(int *)((char *)ext2 + UNI_NATIVE_CTOR)) return;
    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x600)) return;
    if (!g_TimerInit || !g_TimerCleanup || !g_GfxScaleZ || !g_GfxScaleX || !g_GfxSetPosition) return;
    void *gfx = *(void **)(app + 0x174);
    if (!gfx) return;
    int ni2 = g_AthenaListGetSize ? g_AthenaListGetSize((void *)((char *)board + UNI_OBJ_LIST)) : 0;
    if (ni2 <= 0) return;
    DWORD *items2 = *(DWORD **)((char *)board + UNI_OBJ_LIST + 0x40C);
    if (!items2 || IsBadReadPtr((void *)items2, ni2 * 4)) return;
    int j;
    for (j = 0; j < ni2; j++) {
        DWORD obj = items2[j];
        if (!obj || IsBadReadPtr((void *)obj, 0x1100)) continue;
        if (*(DWORD *)obj != 0x4D4ED8) continue;
        DWORD vis = *(DWORD *)(obj + 0x10D4);
        if (!vis || IsBadReadPtr((void *)vis, 0x60)) continue;
        DWORD *visVtbl = *(DWORD **)vis;
        if (!visVtbl || IsBadReadPtr((void *)visVtbl, 0x60)) continue;
        float px = *(float *)(obj + 0x10D8);
        float py = *(float *)(obj + 0x10DC);
        float pz = *(float *)(obj + 0x10E0);
        float yaw = *(float *)(obj + 0x10E8);
        char timerBuf[256];
        { int zi; for (zi = 0; zi < 256; zi++) timerBuf[zi] = 0; }
        g_TimerInit(timerBuf);
        g_GfxScaleZ(timerBuf, 0.0f);
        g_GfxScaleX(timerBuf, 360.0f - yaw);
        g_GfxSetPosition(timerBuf, px, py, pz);
        {
            DWORD *ovt = *(DWORD **)obj;
            if (ovt && !IsBadReadPtr((void *)ovt, 0x160)) {
                void (__thiscall *fn58)(DWORD) = (void (__thiscall *)(DWORD))ovt[0x58];
                void (__thiscall *fn54)(DWORD, void *) = (void (__thiscall *)(DWORD, void *))ovt[0x54];
                if (fn58) fn58(obj);
                if (fn54) fn54(obj, timerBuf);
            }
        }
        g_TimerCleanup(timerBuf);
    }
#endif
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: Swirl Zones (Dizzy / Master)
 * Proximity check → velocity scale + tar bubbles + mesh rotation
 * Replicates DizzyBoard_Update (0x41D510) / Master Board_Update (0x420DA0)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Tiny sin/cos (degrees) — no libm in this DLL. Taylor to x^9/x^8 after
 * range reduction; plenty for per-frame game angles. */
static float my_sin_deg(float deg) {
    float a = deg;
    while (a > 180.0f) a -= 360.0f;
    while (a < -180.0f) a += 360.0f;
    float x = a * 0.01745329252f;
    float x2 = x * x;
    return x * (1.0f - x2 * (0.16666667f - x2 * (0.00833333f - x2 * 0.00019841f)));
}
static float my_cos_deg(float deg) {
    return my_sin_deg(deg + 90.0f);
}

/* (glue-weak diag counters retired 2026-11-07; block + increments removed) */

static void Feature_SwirlZones(void *board, int level) {
    // Pause gate: board+0x874 !=0 means paused (ESC) — freeze anim + sound
    if (*(BYTE*)((char*)board+0x874)) return;
    if (!g_AthenaListGetIterator || !g_operatorNew) return;
    void* ext = GetBoardExt(board);
    /* Get-only (no resurrect): teardown frames must not rebind zombie ext. */
    if (!ext) return;

    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x600)) return;

    /* Unified lists live in ext — one address for every race. The old
     * _M aliases are #defined to the same offsets (lf_00_core.c), so no
     * content fork is needed here. */
    int tarListOfs = BRD_TARBUBBLE_LIST;
    // tarListOfs is a UNI offset >=0x6500 — use ext

    /* Tar-content slow zones: the zone registry is drawer-driven (legacy
     * singleton UNI_GLUEBIE_OBJ + type-3 drawers); walking drawers reads
     * identical fields on every board with no content fork. */

    /* Ambient-tarbubble gate (native Dizzy update head): spawn only when
     * size(board+0x362C) != 1 or the player ball's +0x14C flag is clear.
     * Tar-content-gated, no level check: the gate reads only when the
     * tar list is non-empty (harvested from TARBUBBLE S1s at load —
     * stock: Dizzy + Master), so contentless boards never touch the
     * native slot. Empty list => block below can't fire regardless. */
    DWORD *tarListGate = (DWORD *)((char *)ext + tarListOfs);
    int tarCountGate = 0;
    if (!IsBadReadPtr(tarListGate, 0x410)) {
        tarCountGate = *(int *)(tarListGate + 1);  /* count at +0x04 */
    }
    int ambientGate = 1;
    if (tarCountGate > 0 && g_AthenaListGetSize) {
        int sz362C = g_AthenaListGetSize((void *)((char *)board + 0x362C));
        int p1ball = *(int *)(app + 0x5DC);
        char b14c = (p1ball && !IsBadReadPtr((void *)p1ball, 0x150)) ? *(char *)(p1ball + 0x14C) : 0;
        ambientGate = (sz362C != 1) || (b14c == 0);
    }


    /* TarBubble particle creation — replicates original DizzyBoard_Update.
     * RNG returns 10 (1 in 20 chance per frame) → create a tarbubble particle.
     * FUN_0044fa90 picks a random entry from the tarbubble list and reads
     * entry+0x04/0x08/0x0C as X/Y/Z floats. S1 ref points have this layout.
     * The particle is appended to UNI_PARTICLE_LIST for rendering. */
    static int swirlDbg = 0;
    if (swirlDbg < 3) DebugLog("  [swirl] step1: tarbubble check");
    if (ambientGate && g_CreateTarBubble && g_AthenaListAppend && g_operatorNew && g_RNG) {
        DWORD *tarList = (DWORD *)((char *)ext + tarListOfs);
        int tarCount = 0;
        if (!IsBadReadPtr(tarList, 0x410)) {
            tarCount = *(int *)(tarList + 1);  /* count at +0x04 */
        }
        if (swirlDbg < 3) {
            char dbg[128];
            wsprintfA(dbg, "  [swirl] step1: tarCount=%d tarListOfs=0x%X", tarCount, tarListOfs);
            DebugLog(dbg);
        }
        if (tarCount > 0) {
            int rngResult = -1;
            if (g_RNG) {
                rngResult = RNG_call((void *)0x4F7360, 0, 0x14, 0);
                if (swirlDbg < 3) {
                    char dbg2[128];
                    wsprintfA(dbg2, "  [swirl] step1: rngResult=%d (need 10)", rngResult);
                    DebugLog(dbg2);
                }
                if (rngResult == 10) {
                    void *tar = g_operatorNew(0x1C);
                    if (tar) {
                        if (swirlDbg < 3) DebugLog("  [swirl] step1: calling CreateTarBubble");
                        g_CreateTarBubble(tar, app, (int)((char *)ext + tarListOfs));
                        g_AthenaListAppend((void *)((char *)board + UNI_PARTICLE_LIST), (int)tar);
                        if (swirlDbg < 3) DebugLog("  [swirl] step1: CreateTarBubble done");
                    }
                }
            }
        }
    }
    if (swirlDbg < 3) DebugLog("  [swirl] step1 done");

    /* Swirl zone processing: iterate ball list, check proximity to swirl zones
     * Ball list at board+0x29D4 (AthenaList), array at board+0x2DE0 */
    if (swirlDbg < 3) DebugLog("  [swirl] step2: ball list iteration");
    if (swirlDbg < 3) DebugLog("  [swirl] step2a: get ball iterator");
    int ballIter = g_AthenaListGetIterator((void *)((char *)board + UNI_BALL_LIST));
    if (swirlDbg < 3) DebugLog("  [swirl] step2a done");
    *(int *)((char *)board + UNI_BALL_ITER + ballIter * 4) = 0;
    if (swirlDbg < 3) DebugLog("  [swirl] step2b: read ball count");
    int ballCount = *(int *)((char *)board + UNI_BALL_COUNT);
    if (swirlDbg < 3) DebugLog("  [swirl] step2b done");
    int ballIdx = 0;
    if (ballCount > 0) {
        if (swirlDbg < 3) DebugLog("  [swirl] step2c: read ball array");
        DWORD ballArrayPtr = *(DWORD *)((char *)board + UNI_BALL_ARRAY);
        if (ballArrayPtr && !IsBadReadPtr((void *)ballArrayPtr, 4)) {
            ballIdx = *(int *)ballArrayPtr;
            *(int *)((char *)board + UNI_BALL_ITER + ballIter * 4) = 1;
        }
        if (swirlDbg < 3) DebugLog("  [swirl] step2c done");
    }

    /* Zone registry: legacy singleton (drawer 0, UNI_GLUEBIE_OBJ) +
     * type-3 gluebie drawers (dynamic loader registry). Every gluebie on
     * every board holds a drawer with its obj pointer (create-time,
     * unconditional) — the same objects the native board+0x4378 list
     * holds on Dizzy, so walking drawers reads identical fields with no
     * level check. (Old native/ext fork removed 2026-11-26.) */
    int zones[65];
    int zoneCount = 0;
    {
        int gz0 = *(int *)((char *)ext + UNI_GLUEBIE_OBJ);
        if (gz0 && !IsBadReadPtr((void *)gz0, 0x1108) && zoneCount < 65)
            zones[zoneCount++] = gz0;
    }
    {
        int zd = Gm_DrawerCount(ext, 3);
        int zi;
        for (zi = 1; zi < zd && zi < 64 && zoneCount < 65; zi++) {
            void *slot = Gm_DynSlotPtr(ext, 3, zi);
            int zo = (slot && !IsBadReadPtr(slot, 16)) ? *(int *)((char *)slot + 8) : 0;
            if (zo && !IsBadReadPtr((void *)zo, 0x1108))
                zones[zoneCount++] = zo;
        }
    }

    /* NATIVE PARITY (Dizzy 0x41D510 lines 92-107): clear every zone's +0x1104
     * at frame start, before the ball loop sets the hit ones. LFL set 1 on
     * hit but never cleared — first touch froze that gluebie's anim forever
     * (native gluebie update reads the flag for its motion branch). */
    {
        int k;
        for (k = 0; k < zoneCount; k++) {
            int z = zones[k];
            if (z && !IsBadReadPtr((void *)z, 0x1108))
                *(BYTE *)((char *)z + 0x1104) = 0;
        }
    }

    while (ballIdx) {
        /* Skip balls in tar (ball+0x2CC != 0) — they get sinking logic */
        char inTar = *(char *)(ballIdx + BALL_IN_TAR_OFS);
        if (!inTar) {
            /* Per-ball hit flag (native ball+0x2BC: set when any zone hits
             * this frame, cleared otherwise — gates the contact sound). */
            int anyHit = 0;
            /* Check proximity to each swirl zone (drawer registry) */
            int zonePos = 0;
            int zoneIdx = (zoneCount > 0) ? zones[0] : 0;

            while (zoneIdx) {
                if (*(int *)(ballIdx + 0x18) >= 0) {
                    float ballX = *(float *)(ballIdx + BALL_POS_X_OFS);
                    float ballY = *(float *)(ballIdx + BALL_POS_Y_OFS);
                    float ballZ = *(float *)(ballIdx + BALL_POS_Z_OFS);
                    float zoneX, zoneY, zoneZ;
                    /* CENTER = LIVE POS (Dizzy 0x41D820-862: native reads
                     * obj+0x10E0/4/8 as the zone center). +0x10D4/8/C is the
                     * static S1 home. The gluebie update (0x43ECC0) zeroes
                     * +0x10E0 then recomputes it via Matrix_TransformVec3
                     * (0x401D60) from the anchor — so live == home value;
                     * reading either gives the same field. Keep the live read
                     * (native-exact) with a home fallback for the zero phase.
                     * NOTE 2026-11-04: verified live==home, so this branch is
                     * NOT the weak-slow cause — fields were already at the
                     * visuals (user log: 12 clean HITs). Cause hunt moved to
                     * scale-rate/persistence diag (g_scaledFrames). */
                    /* Live pos with S1-home fallback for the unposed first
                     * frame (native Dizzy form, valid on every board). */
                    {
                        float lx = *(float *)(zoneIdx + 0x10E0);
                        float ly = *(float *)(zoneIdx + 0x10E4);
                        float lz = *(float *)(zoneIdx + 0x10E8);
                        if (lx == 0.0f && ly == 0.0f && lz == 0.0f) {
                            zoneX = *(float *)(zoneIdx + 0x10D4);
                            zoneY = *(float *)(zoneIdx + 0x10D8);
                            zoneZ = *(float *)(zoneIdx + 0x10DC);
                        } else { zoneX = lx; zoneY = ly; zoneZ = lz; }
                    }
                    float radius = *(float *)(zoneIdx + 0x1100) * 60.0f;

                    float dx = zoneX - ballX;
                    float dy = zoneY - ballY;
                    float dz = zoneZ - ballZ;
                    float distSq = dx*dx + dy*dy + dz*dz;

                    if (distSq < radius * radius) {
                        /* Zone hit — mark ball +0x2BC and zone +0x1104
                         * (native: contact sound + particles fire once). */
                        anyHit = 1;
                        *(BYTE *)((char *)zoneIdx + 0x1104) = 1;
                        /* Scale ball velocity */
                        DWORD phys = *(DWORD *)(ballIdx + BALL_PHYS_PTR_OFS);
                        if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
                            float vx = *(float *)(phys + BALL_PHYS_VEL_X);
                            float vy = *(float *)(phys + BALL_PHYS_VEL_Y);
                            float vz = *(float *)(phys + BALL_PHYS_VEL_Z);
                            float speedSq = vx*vx + vy*vy + vz*vz;
                            float speed = 0.0f;
                            if (speedSq > 0.0f) speed = sqrtf(speedSq);
                            if (speed > 0.0f) {
                                float scale = (speed * 0.95f) / speed;
                                vx *= scale; vy *= scale; vz *= scale;
                                *(float *)(phys + BALL_PHYS_VEL_X) = vx;
                                *(float *)(phys + BALL_PHYS_VEL_Y) = vy;
                                *(float *)(phys + BALL_PHYS_VEL_Z) = vz;
                            }
                            /* Play sound + spawn particles on first contact */
                            if (!*(char *)(ballIdx + BALL_TAR_SOUND_FLAG)) {
                                if (g_SoundPlay3D) {
                                    DWORD snd = *(DWORD *)(app + APP_SOUNDFX_484);
                                    if (snd)
                                        g_SoundPlay3D((void *)snd, ballX, ballY, ballZ, 1.0f);
                                }
                                /* Spawn 3 particles */
                                int p;
                                for (p = 0; p < 3 && g_operatorNew && g_AthenaListAppend; p++) {
                                    float *part = (float *)g_operatorNew(0x14);
                                    if (part) {
                                        /* Native particle init: dword[3]=6 (bits of
                                         * 8.407791e-45, NOT 1), byte+16=0. */
                                        ((int *)part)[3] = 6;
                                        *(char *)((char *)part + 16) = 0;
                                        /* Random direction (normalized) */
                                        if (g_RNG) {
                                            part[0] = (float)RNG_call((void *)0x4F7360, 0, 100, 1);
                                            part[1] = (float)RNG_call((void *)0x4F7360, 0, 100, 1);
                                            part[2] = (float)RNG_call((void *)0x4F7360, 0, 100, 1);
                                            float lenSq = part[0]*part[0] + part[1]*part[1] + part[2]*part[2];
                                            if (lenSq > 0.0f) {
                                                float len = sqrtf(lenSq);
                                                float s = 1.0f / len;
                                                part[0] *= s; part[1] *= s; part[2] *= s;
                                            }
                                        }
                                        int particleListSize = g_AthenaListGetSize(
                                            (void *)(ballIdx + 0x810));
                                        if (particleListSize < 30) {
                                            g_AthenaListAppend(
                                                (void *)(ballIdx + 0x810), (int)part);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                zonePos++;
                if (zonePos >= zoneCount) break;
                zoneIdx = zones[zonePos];
            }
            /* Native ball+0x2BC write: 1 when any zone hit this frame, else 0. */
            *(BYTE *)((char *)ballIdx + BALL_TAR_SOUND_FLAG) = (BYTE)anyHit;
        } else {
            /* Ball is in tar — sink and potentially remove */
            *(float *)(ballIdx + BALL_POS_Y_OFS) -= 0.25f;
            /* Splash particle */
            if (g_CreateSplashParticle && g_AthenaListAppend && g_RNG) {
                int rng1 = RNG_call((void *)0x4F7360, 0, 0xF, 0);
                if (rng1 == 1) {
                    float rx = (float)RNG_call((void *)0x4F7360, 0, 100, 1);
                    float rz = (float)RNG_call((void *)0x4F7360, 0, 100, 1);
                    float lenSq = rx*rx + rz*rz;
                    float len = (lenSq > 0.0f) ? sqrtf(lenSq) : 0.0f;
                    float scale = (len > 0.0f) ?
                        (10.0f + *(float *)(ballIdx + 0x284)) / len : 0.0f;
                    rx *= scale; rz *= scale;
                    void *splash = g_operatorNew(0x1C);
                    if (splash) {
                        g_CreateSplashParticle(splash, app,
                            rz + *(float *)(ballIdx + BALL_POS_X_OFS),
                            *(DWORD *)(ballIdx + 0x2D0),
                            rx + *(float *)(ballIdx + BALL_POS_Z_OFS));
                        g_AthenaListAppend((void *)((char *)board + UNI_PARTICLE_LIST), (int)splash);
                    }
                }
            }
            /* Remove ball if below threshold */
            float threshold = *(float *)(ballIdx + 0x2D0) -
                *(float *)(ballIdx + 0x284) * 2.5f;
            if (*(float *)(ballIdx + BALL_POS_Y_OFS) < threshold) {
                if (g_RemoveBall) g_RemoveBall(ballIdx);
            }
        }
        int nextBall = *(int *)((char *)board + UNI_BALL_ITER + ballIter * 4);
        if (*(int *)((char *)board + UNI_BALL_COUNT) <= nextBall) break;
        DWORD ballArr = *(DWORD *)((char *)board + UNI_BALL_ARRAY);
        if (!ballArr || IsBadReadPtr((void *)ballArr, (nextBall + 1) * 4)) break;
        ballIdx = *(int *)(ballArr + nextBall * 4);
        *(int *)((char *)board + UNI_BALL_ITER + ballIter * 4) = nextBall + 1;
    }
    if (swirlDbg < 3) DebugLog("  [swirl] step2 done");

    /* Swirl mesh rotation — shared path, runs wherever swirl data exists */
    if (swirlDbg < 3) DebugLog("  [swirl] step3: mesh rotation");
    if (g_TimerInit && g_TimerCleanup && g_GfxScaleY &&
        g_GfxSetPosition && g_Matrix44Zero && app) {
        void *gfx = *(void **)(app + 0x174);
        if (gfx) {
        char timerBuf[68];
        g_TimerInit(timerBuf);

        /* Primary swirl mesh rotation — compose into timerBuf (drawn below),
         * never the persistent gfx object (same class as bridge/windmill). */
        if (swirlDbg < 3) DebugLog("  [swirl] step3a: primary mesh rotation");
        /* Angle lives in ext on every board (drawer model, seeded from
         * S1 at create). Native slots retired Phase B (Dizzy-verified). */
        float *pAngW = (float *)((char *)ext + OFF_SWIRL_ANGLE1);
        float angle1 = *pAngW - 0.5f;
        *(float*)((char*)ext + OFF_SWIRL_SPEED)=0.5f; *pAngW=angle1;
        if (swirlDbg < 3) DebugLog("  [swirl] step3a: calling Matrix44Zero");
        g_Matrix44Zero((int *)timerBuf);
        if (swirlDbg < 3) DebugLog("  [swirl] step3a: calling GfxScaleY");
        {
            float rx = *(float*)((char*)ext + OFF_WATER_ROT_X);
            float ry = *(float*)((char*)ext + OFF_WATER_ROT_Y);
            float rz = *(float*)((char*)ext + OFF_WATER_ROT_Z);
            if (rx!=0 || ry!=0 || rz!=0) {
                if (ry!=0) g_GfxScaleY(timerBuf, angle1 * ry); else g_GfxScaleY(timerBuf, angle1);
                if (rx!=0) g_GfxScaleX(timerBuf, angle1 * rx);
                if (rz!=0) g_GfxScaleZ(timerBuf, angle1 * rz);
            } else g_GfxScaleY(timerBuf, angle1);
        }
        if (swirlDbg < 3) DebugLog("  [swirl] step3a: calling GfxSetPosition");
        g_GfxSetPosition(timerBuf, *(float*)((char*)ext + OFF_WHEEL_EMBED_X), *(float*)((char*)ext + OFF_WHEEL_EMBED_Y), *(float*)((char*)ext + OFF_WHEEL_EMBED_Z));
        if (swirlDbg < 3) DebugLog("  [swirl] step3a: Gfx calls done, calling render");
        // Re-enabled after ext-heap fix — heap overflow was corrupting spatial tree at +0x18
        {
            /* Draw the ext-seeded mesh (create mirrors S1 mesh/pos into
             * ext on every board); native slots stay fresh via the mirrors
             * in create + the angle mirror above. */
            DWORD drawObj = *(DWORD*)((char*)ext + UNI_MESH_1);
            /* Native wheel center for sound below. */
            float wcx = *(float*)((char*)ext + OFF_WHEEL_EMBED_X);
            float wcy = *(float*)((char*)ext + OFF_WHEEL_EMBED_Y);
            float wcz = *(float*)((char*)ext + OFF_WHEEL_EMBED_Z);
            /* Wheel creak (native block 3 tail): distance attenuation at the
             * wheel center, played through the ctor-grabbed channel +0x4BDC.
             * Presence-gated on the channel itself (wherever a waterwheel
             * was built and grabbed one) — not on Dizzy content. */
            if (g_SoundCalcDistAtten && g_SoundPlay3DAtPos) {
                float atten = g_SoundCalcDistAtten(*(int *)(app + 0x178), wcx, wcy, wcz);
                int wch = BoardHasOffset(board, 0x4BDC, 4) ? *(int *)((char *)board + 0x4BDC) : 0;
                if (wch) g_SoundPlay3DAtPos(wch, atten);
            }
            DWORD waterRender = drawObj;
            if (waterRender && !IsBadReadPtr((void*)waterRender, 4)) {
                DWORD *vtbl = *(DWORD**)waterRender;
                if (vtbl && !IsBadReadPtr(vtbl, 0x60)) {
                    void (__thiscall *fn16)(DWORD) = (void (__thiscall*)(DWORD))vtbl[0x16];
                    void (__thiscall *fn15)(DWORD, void*) = (void (__thiscall*)(DWORD,void*))vtbl[0x15];
                    if (fn16) fn16(waterRender);
                    if (fn15) fn15(waterRender, timerBuf);
                }
            }
        }
        if (swirlDbg < 3) DebugLog("  [swirl] step3a done");

        g_TimerCleanup(timerBuf);
        }
        DebugLog("  [swirl] step3a done (full)");
        /* Global Mod multi-wheel: drawers #1+ spin at their own S1 positions
         * (Dizzy-native path above owns drawer 0 via board slots; custom
         * levels with 2+ WATERWHEEL S1s get extra spinning wheels here). */
        {
            int *dc = Gm_DynCounts(ext);
            int nw = dc ? dc[1] : 0;
            if (nw > 1 && g_TimerInit && g_TimerCleanup && g_GfxScaleY &&
                g_GfxSetPosition && g_Matrix44Zero && app) {
                void *gfxM = *(void **)(app + 0x174);
                if (gfxM) {
                    int wi;
                    DWORD sharedMesh = *(DWORD*)((char*)ext + UNI_MESH_1);
                    for (wi = 1; wi < nw && wi < 64; wi++) {
                        void *slot = Gm_DynSlotPtr(ext, 1, wi);
                        char mtimer[68];
                        float ang;
                        float px, py, pz, brx, bry, brz;
                        if (!slot) continue;
                        if (((DWORD *)slot)[2]==0) {
                            /* First frame: adopt create-time obj if list has it.
                             * Wheel instances are native-driven; drawer obj may
                             * be 0 when create ran before native build. */
                        }
                        ang = ((float *)slot)[10] - 0.5f;
                        ((float *)slot)[10] = ang;
                        px = ((float *)slot)[3]; py = ((float *)slot)[4]; pz = ((float *)slot)[5];
                        brx = ((float *)slot)[6]; bry = ((float *)slot)[7]; brz = ((float *)slot)[8];
                        g_TimerInit(mtimer);
                        g_Matrix44Zero((int *)mtimer);
                        if (brx!=0 || bry!=0 || brz!=0) {
                            if (bry!=0) g_GfxScaleY(mtimer, ang * bry); else g_GfxScaleY(mtimer, ang);
                            if (brx!=0) g_GfxScaleX(mtimer, ang * brx);
                            if (brz!=0) g_GfxScaleZ(mtimer, ang * brz);
                        } else g_GfxScaleY(mtimer, ang);
                        g_GfxSetPosition(mtimer, px, py, pz);
                        if (sharedMesh && !IsBadReadPtr((void*)sharedMesh, 4)) {
                            DWORD *vt = *(DWORD**)sharedMesh;
                            if (vt && !IsBadReadPtr(vt, 0x60)) {
                                void (__thiscall *f16)(DWORD) = (void (__thiscall*)(DWORD))vt[0x16];
                                void (__thiscall *f15)(DWORD, void*) = (void (__thiscall*)(DWORD,void*))vt[0x15];
                                if (f16) f16(sharedMesh);
                                if (f15) f15(sharedMesh, mtimer);
                            }
                        }
                        g_TimerCleanup(mtimer);
                    }
                    { char mdbg[96]; wsprintfA(mdbg, "  [swirl] multi-wheel drawers=%d done", nw-1); DebugLog(mdbg); }
                }
            }
        }
    }

    /* Secondary swirl mesh rotation (Gfx_ScaleX) — shared path */
    if (g_TimerInit && g_TimerCleanup && g_GfxScaleX &&
        g_GfxSetPosition && g_Matrix44Zero && app) {
        void *gfx = *(void **)(app + 0x174);
        if (gfx) {
        if (swirlDbg < 3) DebugLog("  [swirl] step3b: secondary mesh rotation");
        char timerBuf[68];
        g_TimerInit(timerBuf);

        float swirlSpeed = (*(int *)(app + APP_DIFFICULTY) == 0) ? 0.25f : 0.5f;
        /* Angle lives in ext on every board (native slots retired). */
        float *pAngS = (float *)((char *)ext + OFF_SWIRL_ANGLE2);
        *pAngS = *pAngS + swirlSpeed;
        if (swirlDbg < 3) DebugLog("  [swirl] step3b: calling Matrix44Zero");
        g_Matrix44Zero((int *)timerBuf);
        if (swirlDbg < 3) DebugLog("  [swirl] step3b: calling GfxScaleX");
        g_GfxScaleX(timerBuf, *pAngS);
        if (swirlDbg < 3) DebugLog("  [swirl] step3b: calling GfxSetPosition");
        g_GfxSetPosition(timerBuf,
            *(float *)((char *)ext + OFF_SWIRL_POS_X),
            *(float *)((char *)ext + OFF_SWIRL_POS_Y),
            *(float *)((char *)ext + OFF_SWIRL_POS_Z));
        if (swirlDbg < 3) DebugLog("  [swirl] step3b: Gfx calls done, calling render");
        {
            /* Draw the ext-seeded swirl mesh (see step3a). */
            DWORD drawS = *(DWORD*)((char*)ext + UNI_MESH_7);
            DWORD swirlRender = drawS;
            if (swirlRender && !IsBadReadPtr((void*)swirlRender, 4)) {
                DWORD *vtbl = *(DWORD**)swirlRender;
                if (vtbl && !IsBadReadPtr(vtbl, 0x60)) {
                    void (__thiscall *fn16)(DWORD) = (void (__thiscall*)(DWORD))vtbl[0x16];
                    void (__thiscall *fn15)(DWORD, void*) = (void (__thiscall*)(DWORD,void*))vtbl[0x15];
                    if (fn16) fn16(swirlRender);
                    if (fn15) fn15(swirlRender, timerBuf);
                }
            }
        }
        if (swirlDbg < 3) DebugLog("  [swirl] step3b done");

        g_TimerCleanup(timerBuf);
        }
        if (swirlDbg < 3) DebugLog("  [swirl] step3b done");
        /* Global Mod: secondary-swirl drawers #1+ (own pos + angle each). */
        {
            int ns = Gm_DrawerCount(ext, 2);
            if (ns > 1 && g_GfxScaleX && app) {
                void *gfxS = *(void **)(app + 0x174);
                if (gfxS && !IsBadReadPtr(gfxS, 4)) {
                    int wi;
                    float sspd = (*(int *)(app + APP_DIFFICULTY) == 0) ? 0.25f : 0.5f;
                    DWORD sDraw = *(DWORD*)((char*)ext + UNI_MESH_7);
                    for (wi = 1; wi < ns && wi < 64; wi++) {
                        void *slot = Gm_DynSlotPtr(ext, 2, wi);
                        char st[68];
                        float *w;
                        if (!slot) continue;
                        w = (float *)slot;
                        w[10] += sspd;
                        g_TimerInit(st);
                        g_Matrix44Zero((int *)st);
                        g_GfxScaleX(st, w[10]);
                        g_GfxSetPosition(st, w[3], w[4], w[5]);
                        if (sDraw && !IsBadReadPtr((void*)sDraw, 4)) {
                            DWORD *vt = *(DWORD**)sDraw;
                            if (vt && !IsBadReadPtr(vt, 0x60)) {
                                void (__thiscall *f16)(DWORD) = (void (__thiscall*)(DWORD))vt[0x16];
                                void (__thiscall *f15)(DWORD, void*) = (void (__thiscall*)(DWORD,void*))vt[0x15];
                                if (f16) f16(sDraw);
                                if (f15) f15(sDraw, st);
                            }
                        }
                        g_TimerCleanup(st);
                    }
                    if (swirlDbg < 3) DebugLog("  [swirl] step3b-multi done");
                }
            }
        }
    }
    /* Rider carry (drawer-driven, every board): the ball's center-relative
     * offset is rotated kinematically (native Dizzy blocks 4/5 constants);
     * custom levels marked riders (+0x778/+0x779) but never carried them
     * (and +0x778 is sticky). Drive every wheel/swirl drawer center here:
     * nearest center within 500u carries, none in range clears the mark. */
    if (Gm_DrawerCount(ext, 1) > 0 || Gm_DrawerCount(ext, 2) > 0) {
        int ci = g_AthenaListGetIterator((void *)((char *)board + UNI_BALL_LIST));
        *(int *)((char *)board + UNI_BALL_ITER + ci * 4) = 0;
        int cn = *(int *)((char *)board + UNI_BALL_COUNT);
        DWORD carr = 0;
        if (cn > 0) {
            DWORD ca = *(DWORD *)((char *)board + UNI_BALL_ARRAY);
            if (ca && !IsBadReadPtr((void *)ca, 4)) {
                carr = *(int *)ca;
                *(int *)((char *)board + UNI_BALL_ITER + ci * 4) = 1;
            }
        }
        while (carr) {
            int hasW = *(char *)(carr + 0x778);
            int hasS = *(char *)(carr + 0x779);
            char inTar = *(char *)(carr + BALL_IN_TAR_OFS);
            if ((hasW || (hasS && !inTar))) {
                float bx = *(float *)(carr + BALL_POS_X_OFS);
                float by = *(float *)(carr + BALL_POS_Y_OFS);
                float bz = *(float *)(carr + BALL_POS_Z_OFS);
                /* nearest wheel drawer (0 = singleton ext pos) */
                float nwx = 0, nwy = 0, nwz = 0; float nwd = 1e30f; int wfound = 0;
                /* nearest swirl drawer (0 = singleton ext pos) */
                float nsx = 0, nsy = 0, nsz = 0; float nsd = 1e30f; int sfound = 0;
                { int nwt = Gm_DrawerCount(ext, 1);
                  if (nwt > 0 && hasW) {
                      float cx = *(float*)((char*)ext + OFF_WHEEL_EMBED_X);
                      float cy = *(float*)((char*)ext + OFF_WHEEL_EMBED_Y);
                      float cz = *(float*)((char*)ext + OFF_WHEEL_EMBED_Z);
                      { float dx = bx - cx, dy = by - cy, dz = bz - cz;
                        float d2 = dx*dx + dy*dy + dz*dz;
                        if (d2 < nwd) { nwd = d2; nwx = cx; nwy = cy; nwz = cz; wfound = 1; } }
                      { int wi2; for (wi2 = 1; wi2 < nwt && wi2 < 64; wi2++) {
                          void *sl2 = Gm_DynSlotPtr(ext, 1, wi2);
                          if (!sl2) continue;
                          { float cx2 = ((float*)sl2)[3], cy2 = ((float*)sl2)[4], cz2 = ((float*)sl2)[5];
                            float dx = bx - cx2, dy = by - cy2, dz = bz - cz2;
                            float d2 = dx*dx + dy*dy + dz*dz;
                            if (d2 < nwd) { nwd = d2; nwx = cx2; nwy = cy2; nwz = cz2; wfound = 1; } }
                      } }
                  } }
                { int nst = Gm_DrawerCount(ext, 2);
                  if (nst > 0 && hasS && !inTar) {
                      float cx = *(float*)((char*)ext + OFF_SWIRL_POS_X);
                      float cy = *(float*)((char*)ext + OFF_SWIRL_POS_Y);
                      float cz = *(float*)((char*)ext + OFF_SWIRL_POS_Z);
                      { float dx = bx - cx, dy = by - cy, dz = bz - cz;
                        float d2 = dx*dx + dy*dy + dz*dz;
                        if (d2 < nsd) { nsd = d2; nsx = cx; nsy = cy; nsz = cz; sfound = 1; } }
                      { int si2; for (si2 = 1; si2 < nst && si2 < 64; si2++) {
                          void *sl2 = Gm_DynSlotPtr(ext, 2, si2);
                          if (!sl2) continue;
                          { float cx2 = ((float*)sl2)[3], cy2 = ((float*)sl2)[4], cz2 = ((float*)sl2)[5];
                            float dx = bx - cx2, dy = by - cy2, dz = bz - cz2;
                            float d2 = dx*dx + dy*dy + dz*dz;
                            if (d2 < nsd) { nsd = d2; nsx = cx2; nsy = cy2; nsz = cz2; sfound = 1; } }
                      } }
                  } }
                /* wheel carry: -0.5 deg/frame about X (native 0x41DC80 constants) */
                if (hasW) {
                    if (!wfound || nwd > 250000.0f) { *(char *)(carr + 0x778) = 0; }
                    else {
                        float ry = by - nwy, rz = bz - nwz;
                        *(float *)(carr + BALL_POS_Y_OFS) = nwy + 0.9999619f * ry + 0.0087265f * rz;
                        *(float *)(carr + BALL_POS_Z_OFS) = nwz - 0.0087265f * ry + 0.9999619f * rz;
                    }
                }
                /* swirl capture/carry: +0.5 Ry step (native block-5 constants) */
                if (hasS && !inTar) {
                    if (!sfound || nsd > 250000.0f) { *(char *)(carr + 0x779) = 0; }
                    else {
                        int cd = *(int *)(carr + 0x78C);
                        if (cd != 0) {
                            cd--; *(int *)(carr + 0x78C) = cd;
                            if (cd == 0) {
                                float cap = (float)(*(int *)(carr + 0x788)) * 0.0001f;
                                DWORD phys = *(DWORD *)(carr + BALL_PHYS_PTR_OFS);
                                if (phys && !IsBadReadPtr((void *)phys, 0xCB0) && cap > 0.0f) {
                                    float vx = *(float *)(phys + BALL_PHYS_VEL_X);
                                    float vy = *(float *)(phys + BALL_PHYS_VEL_Y);
                                    float vz = *(float *)(phys + BALL_PHYS_VEL_Z);
                                    float sp = sqrtf(vx*vx + vy*vy + vz*vz);
                                    if (sp > cap && sp > 0.0f) { float k = cap / sp;
                                        *(float *)(phys + BALL_PHYS_VEL_X) = vx * k;
                                        *(float *)(phys + BALL_PHYS_VEL_Y) = vy * k;
                                        *(float *)(phys + BALL_PHYS_VEL_Z) = vz * k; }
                                }
                            }
                        } else {
                            *(int *)(carr + 0x78C) = 5;
                            { float rx = bx - nsx, ry2 = by - nsy, rz2 = bz - nsz;
                              float o0 = rx, o2 = rz2;
                              float nx = 0.9999619f * rx + 0.0087265f * rz2;
                              float nz = -0.0087265f * rx + 0.9999619f * rz2;
                              rx = nx; rz2 = nz;
                              *(float *)(carr + BALL_POS_X_OFS) = nsx + rx;
                              *(float *)(carr + BALL_POS_Y_OFS) = nsy + ry2;
                              *(float *)(carr + BALL_POS_Z_OFS) = nsz + rz2;
                              { DWORD phys = *(DWORD *)(carr + BALL_PHYS_PTR_OFS);
                                if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
                                    float vx = *(float *)(phys + BALL_PHYS_VEL_X);
                                    float vy = *(float *)(phys + BALL_PHYS_VEL_Y);
                                    float vz = *(float *)(phys + BALL_PHYS_VEL_Z);
                                    float osp = sqrtf(vx*vx + vy*vy + vz*vz);
                                    *(int *)(carr + 0x788) = (int)(osp * 10000.0f);
                                    { float nvx = vx + (rx - o0) * 0.1f;
                                      float nvy = vy;
                                      float nvz = vz + (rz2 - o2) * 0.1f;
                                      float nsp = sqrtf(nvx*nvx + nvy*nvy + nvz*nvz);
                                      if (nsp > osp && osp > 0.0f && nsp > 0.0f) { float k = osp / nsp;
                                        nvx *= k; nvy *= k; nvz *= k; }
                                      *(float *)(phys + BALL_PHYS_VEL_X) = nvx;
                                      *(float *)(phys + BALL_PHYS_VEL_Y) = nvy;
                                      *(float *)(phys + BALL_PHYS_VEL_Z) = nvz; }
                                } }
                            }
                        }
                    }
                }
            }
            { int rn = *(int *)((char *)board + UNI_BALL_ITER + ci * 4);
              if (*(int *)((char *)board + UNI_BALL_COUNT) <= rn) break;
              { DWORD ca2 = *(DWORD *)((char *)board + UNI_BALL_ARRAY);
                if (!ca2 || IsBadReadPtr((void *)ca2, (rn + 1) * 4)) break;
                carr = *(int *)(ca2 + rn * 4);
                *(int *)((char *)board + UNI_BALL_ITER + ci * 4) = rn + 1; } }
        }
    }
    if (swirlDbg < 3) DebugLog("  [swirl] Feature_SwirlZones complete");
    swirlDbg++;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: Windmill (Tower)
 * Rotation + 4-state machine (spin up → creak → spin down → pause)
 * Replicates FUN_0041E760 (Tower Board_Update)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Chomper bite state machine (native Tower 0x41E861+, objdump 2026-11-10).
 * Drives board+0x43A0 (jaw open 0..25), 0x43A8 (state 0..3), 0x43AC
 * (counter), 0x43B0 (liftY). The content-5 early-return skipped this
 * entirely → jaw frozen shut (only lower jaw), never bites, no sound. */
/* Global Mod: windmill drawer #1+ driver (own angle/speed/state/counter/
 * decay + pos, shared render). Drawer 0 stays on the singleton path. */
static void Gm_WindmillDrawer(void *board, void *ext, DWORD app, void *slot, DWORD renderObj) {
    float *w = (float *)slot;
    float rotSpeed;
    if (!w || !app || IsBadReadPtr((void *)app, 0x500)) return;
    rotSpeed = (*(int *)(app + APP_DIFFICULTY) == 0) ? 0.5f : 1.0f;
    w[10] += rotSpeed;
    { int ai = (int)w[10];
      if (ai % 0x5A == 0x2D && g_SoundPlay3D) {
          DWORD snd = *(DWORD *)(app + 0x4A4);
          if (snd) g_SoundPlay3D((void *)snd, w[3], w[4], w[5], 1.0f);
      } }
    if (g_TimerInit && g_TimerCleanup && g_GfxScaleY && g_GfxSetPosition) {
        void *gfx = *(void **)(app + 0x174);
        if (gfx && !IsBadReadPtr(gfx, 4)) {
            char tb[68];
            g_TimerInit(tb);
            g_GfxScaleY(tb, w[10]);
            g_GfxSetPosition(tb, w[3], w[4], w[5]);
            if (renderObj && !IsBadReadPtr((void*)renderObj, 4)) {
                DWORD *vt = *(DWORD **)renderObj;
                if (vt && !IsBadReadPtr(vt, 0x60)) {
                    void (__fastcall *f58)(DWORD, DWORD) = (void (__fastcall *)(DWORD, DWORD))vt[0x16];
                    void (__thiscall *f54)(DWORD, void *) = (void (__thiscall *)(DWORD, void *))vt[0x15];
                    if (f58) f58(renderObj, (DWORD)vt);
                    if (f54) f54(renderObj, tb);
                }
            }
            g_TimerCleanup(tb);
        }
    }
    { int st = (int)w[12];
      switch (st) {
      case 0: { float sp = w[11]; if (sp == 0.0f) sp = 0.25f; sp *= 1.2f; w[11] = sp;
                if (sp > 25.0f) { w[11] = 25.0f; w[12] = 1.0f; w[13] = 25.0f; w[14] = 50.0f; } } break;
      case 1: { int c = (int)w[13] - 1; w[13] = (float)c;
                if (c < 1) { w[12] = 2.0f;
                    if (g_SoundPlay3D) { DWORD snd = *(DWORD *)(app + 0x4A8);
                        if (snd) g_SoundPlay3D((void *)snd, w[3], w[4], w[5], 1.0f); } } } break;
      case 2: { float sp = w[11] * 0.25f; w[11] = sp;
                if (sp < 1.0f) { w[11] = 0.0f;
                    if (g_RNG) { int r = RNG_call((void *)0x4F7360, 0, 2, 0);
                        if (r != 0) { w[12] = 3.0f; w[13] = (float)(RNG_call((void *)0x4F7360, 0, 100, 0) + 100); }
                        else w[12] = 0.0f; } } } break;
      case 3: { float d = w[14] - 2.0f; if (d < 0.0f) d = 0.0f; w[14] = d;
                int c = (int)w[13] - 1; w[13] = (float)c; if (c < 1) w[12] = 0.0f; } break;
      } }
}

/* Global Mod: drive windmill drawers #1+ with a shared render. */
static void Gm_WindmillDrawers(void *board, void *ext, DWORD app, DWORD renderObj) {
    int nw = Gm_DrawerCount(ext, 7);
    if (nw > 1) {
        int i;
        for (i = 1; i < nw && i < 64; i++) {
            void *slot = Gm_DynSlotPtr(ext, 7, i);
            if (slot) Gm_WindmillDrawer(board, ext, app, slot, renderObj);
        }
        { char wd[80]; wsprintfA(wd, "windmill drawers=%d done", nw - 1); DebugLog(wd); }
    }
}

static void Feature_ChomperStates(void *board, DWORD app) {
    if (!board) return;
    *(float *)((char *)board + 0x43A4) += 3.0f; /* bob timer, native 0x41E861 */
    int state = *(int *)((char *)board + 0x43A8);
    if (state < 0 || state > 3) { *(int *)((char *)board + 0x43A8) = 0; state = 0; }
    switch (state) {
    case 0: {
        float cs = *(float *)((char *)board + 0x43A0);
        if (!(cs > 0.0f)) cs = 0.25f; /* native: <0 or NaN → 0.25; LFL
         * also re-arms from exact 0.0 (native re-arms via its reset path
         * 0x40EB57 which LFL doesn't run — without this the jaw bites
         * once per load then sticks shut forever). */
        cs *= 1.2f; /* native double 0x4D0A98 */
        *(float *)((char *)board + 0x43A0) = cs;
        /* Smooth lift: rise with the jaw (+2/frame to 50, mirroring
         * state 3's -2/frame fall) instead of native's 0→50 single-frame
         * snap (0x41E8E7) — the snap reads as a teleport-up jerk when
         * the jaw cycles continuously. */
        {
            float ly = *(float *)((char *)board + 0x43B0);
            if (ly < 50.0f) {
                ly += 2.0f;
                if (ly > 50.0f) ly = 50.0f;
                *(float *)((char *)board + 0x43B0) = ly;
            }
        }
        if (cs >= 25.0f) { /* native fcomps 0x4CFECC */
            *(float *)((char *)board + 0x43A0) = 25.0f;
            *(int *)((char *)board + 0x43A8) = 1;
            *(int *)((char *)board + 0x43AC) = 0x19;
        }
        break;
    }
    case 1: {
        int c = *(int *)((char *)board + 0x43AC) - 1;
        *(int *)((char *)board + 0x43AC) = c;
        if (c <= 0) {
            *(int *)((char *)board + 0x43A8) = 2;
            if (g_SoundPlay3D) { /* native bite sound, app+0x4A8 at 0x41E954 */
                DWORD snd = (app && !IsBadReadPtr((void *)app, 0x500)) ? *(DWORD *)(app + 0x4A8) : 0;
                if (snd) g_SoundPlay3D((void *)snd,
                    *(float *)((char *)board + 0x4394),
                    *(float *)((char *)board + 0x4398),
                    *(float *)((char *)board + 0x439C), 1.0f);
            }
        }
        break;
    }
    case 2: {
        float cs = *(float *)((char *)board + 0x43A0) * 0.5f; /* native fmuls 0x4CF3F0 */
        *(float *)((char *)board + 0x43A0) = cs;
        if (cs <= 1.0f) { /* native fcomps 0x4CF310 */
            *(float *)((char *)board + 0x43A0) = 0.0f;
            if (g_RNG) {
                int rng = RNG_call((void *)0x4F7360, 0, 2, 0);
                if (rng == 0) *(int *)((char *)board + 0x43A8) = 0;
                else {
                    *(int *)((char *)board + 0x43A8) = 3;
                    int rng2 = RNG_call((void *)0x4F7360, 0, 100, 0);
                    *(int *)((char *)board + 0x43AC) = rng2 + 100;
                }
            }
        }
        break;
    }
    case 3: {
        float ly = *(float *)((char *)board + 0x43B0) - 2.0f; /* native fsubs 0x4CF48C */
        if (!(ly >= 0.0f)) ly = 0.0f;
        *(float *)((char *)board + 0x43B0) = ly;
        int c = *(int *)((char *)board + 0x43AC) - 1;
        *(int *)((char *)board + 0x43AC) = c;
        if (c <= 0) *(int *)((char *)board + 0x43A8) = 0;
        break;
    }
    }
    /* Global Mod: chomper drawers #1+ (own jaw/lift/sound pos each). Mesh is
     * shared (Meshes\Chomper); drawer pos from S1. Rendered in gm_04. */
    {
        void *ext = GetBoardExt(board);
        int nc = Gm_DrawerCount(ext, 9);
        if (nc > 1) {
            int i;
            for (i = 1; i < nc && i < 64; i++) {
                void *slot = Gm_DynSlotPtr(ext, 9, i);
                float *w;
                if (!slot) continue;
                w = (float *)slot;
                /* init-on-first-frame: NaN-arm like native 0x40EB57 */
                if (!(w[10] > 0.0f) && !(w[10] < 0.0f)) { w[10] = 0.0f; }
                { int st = (int)w[12];
                  switch (st) {
                  case 0: { float cs = w[10]; if (!(cs > 0.0f)) cs = 0.25f; cs *= 1.2f; w[10] = cs;
                            if (w[14] < 50.0f) { w[14] += 2.0f; if (w[14] > 50.0f) w[14] = 50.0f; }
                            if (cs >= 25.0f) { w[10] = 25.0f; w[12] = 1.0f; w[13] = 25.0f; } } break;
                  case 1: { int cc = (int)w[13] - 1; w[13] = (float)cc;
                            if (cc <= 0) { w[12] = 2.0f;
                                if (g_SoundPlay3D) { DWORD snd = (app && !IsBadReadPtr((void *)app, 0x500)) ? *(DWORD *)(app + 0x4A8) : 0;
                                    if (snd) g_SoundPlay3D((void *)snd, w[3], w[4], w[5], 1.0f); } } } break;
                  case 2: { float cs = w[10] * 0.5f; w[10] = cs;
                            if (cs <= 1.0f) { w[10] = 0.0f;
                                if (g_RNG) { int r = RNG_call((void *)0x4F7360, 0, 2, 0);
                                    if (r == 0) w[12] = 0.0f;
                                    else { w[12] = 3.0f; w[13] = (float)(RNG_call((void *)0x4F7360, 0, 100, 0) + 100); } } } } break;
                  case 3: { float ly = w[14] - 2.0f; if (!(ly >= 0.0f)) ly = 0.0f; w[14] = ly;
                            int cc = (int)w[13] - 1; w[13] = (float)cc; if (cc <= 0) w[12] = 0.0f; } break;
                  default: w[12] = 0.0f; break;
                  } }
            }
            { char cd[80]; wsprintfA(cd, "chomper drawers=%d done", nc - 1); DebugLog(cd); }
        }
    }
}

/* TEMP DIAG chomper v1: RETIRED 2026-11-25 (jaw confirmed working on user
 * box, logs dropped per request). No-op stub. */
static void DiagChomper(void *board, DWORD app) {
    (void)board; (void)app;
}
/* (old body deleted 2026-11-25) */

static void Feature_Windmill(void *board, int level) {
    // Pause gate: board+0x874 !=0 means paused (ESC) — freeze anim + sound
    if (*(BYTE*)((char*)board+0x874)) return;
    /* Windmill (Tower-family S1s, any content, any slot): drive the native
     * board slots (native Tower update never runs under the mod). Replicates
     * the native windmill sequence 1:1 — angle advance, sound at 45 mod 90
     * via app+0x4A4, matrix compose + vtbl[0x58]/[0x54] on the board+0x437C
     * object. Angle source of truth is ext (create-seeded, mirrored to the
     * board slot); draw target prefers the board object, ext render fallback.
     * Native Tower render reads these slots, so the windmill visibly spins. */
    /* Single presence-gated driver (all contents, any slot): replicate the
     * native Tower update windmill sequence 1:1 (verified vs native
     * 0x41E7C4-0x41E85E): angle advance, swoosh at angle%90==45 via
     * app+0x4A4, matrix compose + vtbl[0x58]/[0x54] on the board+0x437C
     * object. Old content==5/ext-mirror fork removed — the same file now
     * behaves identically in any slot. */
    {
        /* NOTE: native Tower ctor leaves board+0x878 (app) as 0 — fall back
         * to the app captured at Logic time, else every app read fails. */
        extern int g_lastAppParam;
        DWORD app5 = *(DWORD *)((char *)board + BOARD_APP_PTR);
        if (!app5) app5 = (DWORD)g_lastAppParam;
        if (!app5 || IsBadReadPtr((void *)app5, 0x500)) return;
        void *ext5 = GetBoardExt(board);
        if (!ext5) return;
        /* Angle lives in ext (create-seeded); mirror to the native board
         * slot every frame for native readers. */
        float *extAng = (float *)((char *)ext5 + UNI_WINDMILL_ANGLE);
        float rate = (*(int *)(app5 + APP_DIFFICULTY) == 0) ? 0.5f : 1.0f;
        *extAng = *extAng + rate;
        if (BoardHasOffset(board, 0x438C, 4))
            *(float *)((char *)board + 0x438C) = *extAng;
        /* Draw target: native board+0x437C object when valid (today's Tower
         * visual), else the ext REND_TOWER_WINDMILL render. */
        DWORD wmr = 0;
        if (BoardHasOffset(board, 0x437C, 4)) {
            DWORD bm = *(DWORD *)((char *)board + 0x437C);
            if (bm && !IsBadReadPtr((void *)bm, 4)) wmr = bm;
        }
        if (!wmr) wmr = *(DWORD *)((char *)ext5 + REND_TOWER_WINDMILL);
        {
            int ai = (int)*extAng;
            if (ai % 0x5A == 0x2D && g_SoundPlay3D) {
                DWORD snd = *(DWORD *)(app5 + 0x4A4);
                if (snd) g_SoundPlay3D((void *)snd,
                    *(float *)((char *)board + 0x4380),
                    *(float *)((char *)board + 0x4384),
                    *(float *)((char *)board + 0x4388), 1.0f);
            }
        }
        if (wmr && !IsBadReadPtr((void *)wmr, 4) &&
            g_TimerInit && g_TimerCleanup && g_GfxScaleY && g_GfxSetPosition) {
            /* Native Tower (0x41E80B-0x41E85E): TimerInit(T) -> ScaleY(T,angle)
             * -> SetPosition(T,x,y,z) -> draw wmr with T. Single buffer.
             * Mirror board+0x43A4 advance (native 0x41E861 fadds _DAT_004CF418
             * = 3.0): chomper bob timer must advance here too, else the
             * content-5 early-return freezes it at sin(0)*10. */
            { void *ext5 = GetBoardExt(board);
              if (ext5) *(float *)((char *)ext5 + UNI_CHOMP_TIMER) += 3.0f; }
            char timerBuf[68];
            g_TimerInit(timerBuf);
            g_GfxScaleY(timerBuf, *extAng);
            g_GfxSetPosition(timerBuf,
                *(float *)((char *)board + 0x4380),
                *(float *)((char *)board + 0x4384),
                *(float *)((char *)board + 0x4388));
            DWORD *vt = *(DWORD **)wmr;
            if (vt) {
                void (__fastcall *fn58)(DWORD, DWORD) = (void (__fastcall *)(DWORD, DWORD))vt[0x16];
                void (__thiscall *fn54)(DWORD, void *) = (void (__thiscall *)(DWORD, void *))vt[0x15];
                if (fn58) fn58(wmr, (DWORD)vt);
                if (fn54) fn54(wmr, timerBuf);
            }
            g_TimerCleanup(timerBuf);
        }
        /* Native 0x41E760 falls through to the chomper states. Presence-
         * gated: only drive them when a chomper was actually built. */
        Gm_WindmillDrawers(board, ext5, app5, wmr);
        if (*(DWORD *)((char *)ext5 + REND_TOWER_CHOMPER)) {
            Feature_ChomperStates(board, app5);
            DiagChomper(board, app5);
        }
        return;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: BadBall Spawner (Odd)
 * Timer-based spawning of BadBall enemies
 * Replicates FUN_0041EE80 (Odd Board_Update)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void Feature_BadBallSpawner(void *board, int level) {
    // Pause gate: board+0x874 !=0 means paused (ESC) — freeze anim + sound
    if (*(BYTE*)((char*)board+0x874)) return;
    if (!g_RNG || !g_BadBallSpawnCtor || !g_operatorNew || !g_AthenaListAppend) return;
    void* ext = GetBoardExt(board);
    /* Get-only (no resurrect): teardown frames must not rebind zombie ext. */
    if (!ext) return;

    char spawnFlag = *(char *)((char *)ext + BRD_BB_FLAG);
    /* 8-ball tripwire (Odd chrome-ball hunt 2026-11-25): ball+0x808 nonzero
     * = native 8-ball mode. Log transitions only (no per-frame I/O). */
    {
        static int last808[4] = { -1, -1, -1, -1 };
        DWORD bl = *(DWORD *)((char *)board + 0x29D4);
        if (bl && !IsBadReadPtr((void *)bl, 0x410)) {
            int n = *(int *)(bl + 4);
            DWORD items = *(DWORD *)(bl + 0x40C);
            int i;
            if (n > 4) n = 4;
            for (i = 0; i < n; i++) {
                DWORD b = 0;
                if (!IsBadReadPtr((void *)(items + i * 4), 4)) b = *(DWORD *)(items + i * 4);
                if (!b || IsBadReadPtr((void *)b, 0x80C)) continue;
                {
                    int cur = *(int *)((char *)b + 0x808);
                    if (last808[i] != -1 && cur != last808[i]) {
                        char wl[128];
                        wsprintfA(wl, "BALL808 ball%d %d -> %d pos=%d,%d,%d",
                            i, last808[i], cur, (int)*(float *)((char *)b + 0x164),
                            (int)*(float *)((char *)b + 0x168), (int)*(float *)((char *)b + 0x16C));
                        Gm_LogSkip(wl);
                    }
                    last808[i] = cur;
                }
            }
        }
    }
    if (!spawnFlag) return;

    int counter = *(int *)((char *)ext + BRD_BB_COUNTER) - 1;
    *(int *)((char *)ext + BRD_BB_COUNTER) = counter;
    if (counter >= 1) return;

    /* Check limits — ball list at board+0x29D4 (was 0xA75 in Ghidra DWORD index) */
    int ballCount = g_AthenaListGetSize((void *)((char *)board + UNI_BALL_LIST));
    int totalSpawned = *(int *)((char *)ext + BRD_BB_TOTAL);
    if (ballCount >= 10 || totalSpawned >= 100) return;

    /* Set next spawn timer */
    int nextDelay = RNG_call((void *)0x4F7360, 0, 0x19, 0);
    *(int *)((char *)ext + BRD_BB_COUNTER) = nextDelay + 0x19;

    /* Pick random spawn position (3-slot table) */
    int posIdx;
    do {
        posIdx = RNG_call((void *)0x4F7360, 0, 3, 0);
    } while (posIdx == *(int *)((char *)ext + BRD_BB_LAST_IDX));
    *(int *)((char *)ext + BRD_BB_LAST_IDX) = posIdx;

    float *posTable = (float *)((char *)ext + BRD_BB_POS_TABLE);
    float spawnX = posTable[posIdx * 3];
    float spawnY = posTable[posIdx * 3 + 1];
    float spawnZ = posTable[posIdx * 3 + 2];

    /* Play spawn sound */
    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (app && !IsBadReadPtr((void *)app, 0x600) && g_SoundPlay3D) {
        DWORD snd = *(DWORD *)(app + 0x4D0);
        if (snd) g_SoundPlay3D((void *)snd, spawnX, spawnY, spawnZ, 1.0f);
    }

    /* Create BadBall */
    void *mem = g_operatorNew(0xC64);
    if (!mem) return;
    void *badball = g_BadBallSpawnCtor(mem, (int)board);
    if (!badball) return;

    /* Native calls vtable[1] on the fresh badball (state init) before
     * aiming it. */
    { DWORD *bbv = *(DWORD **)badball;
      if (bbv && !IsBadReadPtr(bbv, 8)) {
          void (__thiscall *bbInit)(DWORD) = (void (__thiscall *)(DWORD))bbv[1];
          if (bbInit) bbInit((DWORD)badball);
      } }

    /* Set trajectory — native Scene_SpawnBadBall (0x41EE86, Ghidra-verified
     * 2026-11-26): args are (0x41EF8A, spawnX, spawnY, ~0, ~0). The 0x401040
     * Vec3_Init temp is dead (overwritten, never feeds the call), so mesh
     * floats must NOT be passed as args 4/5. */
    g_BallSetTrajectory(badball, 0x41EF8A, spawnX, spawnY, 0.0f, 0.0f);

    /* Set badball fields — store IEEE 754 bits, NOT int casts */
    int *bb = (int *)badball;
    float spawnYPlus24 = spawnY + 24.0f;
    memcpy(&bb[0x5A], &spawnYPlus24, sizeof(int));
    memcpy(&bb[0x59], &spawnX, sizeof(int));
    memcpy(&bb[0x5B], &spawnZ, sizeof(int));
    bb[0x9E] = 0x3F000000;  /* 0.5f */
    bb[0x9F] = 0x3DCCCCCD;  /* 0.1f */
    bb[0xA1] = 0x41C00000;  /* 24.0f */
    bb[0x62] = 0x40A00000;  /* 5.0f */
    bb[6] = -1;

    /* Random direction */
    float dirX, dirZ, lenSq;
    do {
        dirX = (float)RNG_call((void *)0x4F7360, 0, 0x19, 1);
        dirZ = (float)RNG_call((void *)0x4F7360, 0, 0x32, 1);
        lenSq = dirX * dirX + dirZ * dirZ;
    } while (lenSq <= 0.0f || sqrtf(lenSq) == 0.0f);

    /* Normalize and scale to 2.5 */
    float len = sqrtf(lenSq);
    dirX = (dirX / len) * 2.5f;
    dirZ = (dirZ / len) * 2.5f;

    /* Set velocity via Ball_SetVec3AtOffset.
     * CRITICAL: check bb[0x69] (physics pointer) for NULL BEFORE calling
     * g_BallSetVec3AtOffset — __thiscall dereferences `this` (ECX) to read
     * the vtable, so a NULL pointer crashes immediately. */
    DWORD physObj = bb[0x69];
    if (physObj && g_BallSetVec3AtOffset) {
        float velVec[3] = { dirX, 12.0f, dirZ };
        g_BallSetVec3AtOffset((void *)physObj, velVec);
        /* Call vtable[0x4] on the physics object */
        DWORD *vtbl = *(DWORD **)physObj;
        if (vtbl) {
            void (__fastcall *fn10)(DWORD) = (void (__fastcall *)(DWORD))vtbl[0x4];
            if (fn10) fn10((DWORD)physObj);
        }
    }

    /* Restore list membership (NOAPPEND bisect reverted 2026-11-26): native
     * spawner appends ONLY to 0x29D4. Without this the balls never render
     * and BadBall_Update starves — "still 8-balls" is expected. */
    g_AthenaListAppend((void *)((char *)board + UNI_BALL_LIST), (int)badball);
    *(int *)((char *)ext + BRD_BB_TOTAL) = totalSpawned + 1;
    /* Build tag 5D90 in the log line proves the new ctor shipped. */
    { char spw[96]; wsprintfA(spw, "ODDSPAWN total=%d bc=%d spawn=5D90", totalSpawned + 1, ballCount + 1); Gm_LogSkip(spw); }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: Bumper Lit Decay (Beginner / Toob / Master)
 * Decays 4-8 float "lit" values by 0.05/frame, clamped to 0.0
 * Replicates Beginner slot[19] / Toob slot[19] / Master vtable[0x94]
 * ═══════════════════════════════════════════════════════════════════════════ */

static void Feature_BumperDecay(void *board, int level) {
    // Pause gate: board+0x874 !=0 means paused (ESC) — freeze anim + sound
    if (*(BYTE*)((char*)board+0x874)) return;
    void* ext = GetBoardExt(board);
    if (!ext) return;
    int baseOfs;
    int count;

    /* Dynamic capacity: decay however many bumpers collect found
     * (UNI_BUMPER_COUNT). Native per-level counts (8/8/4) no longer apply —
     * the mod owns all slots now. */
    baseOfs = BRD_BUMPER_DECAY_BEG;
    count = *(int *)((char *)ext + UNI_BUMPER_COUNT);
    /* No fallback guess: COUNT comes from the same collect the render
     * reads, so a valid count is always present when this runs. (Old
     * level==2/10/14 fallback to 8 removed 2026-11-25.) */
    if (count <= 0 || count > BUMPER_LIT_MAX) {
        return;
    }

    int i;
    float rate = *(float *)((char *)ext + UNI_BUMPER_DECAYRATE);
    if (!(rate > 0.0f)) rate = 0.05f;
    for (i = 0; i < count; i++) {
        float val = *(float *)((char *)ext + baseOfs + i * 4);
        val -= rate;
        if (val <= 0.0f) val = 0.0f;
        *(float *)((char *)ext + baseOfs + i * 4) = val;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: Neon Camera (Neon)
 * Positions render objects relative to ball
 * Replicates FUN_00424790 (Neon slot[19])
 * ═══════════════════════════════════════════════════════════════════════════ */

/* VAC tube driver (native Up RaceState 0x420660 walk, disasm-verified):
 * calls v[0] (vacuum update 0x43D160 — suction/transport) on each tube
 * face. Native walked board+0x436C; faces live in the ext VAC list now
 * (same objects, same call shape). Presence-gated by FEAT_VACUUM. */
static void Feature_Vacuum(void *board, int level) {
    void *ext;
    int n, i;
    (void)level;
    if (*(BYTE*)((char*)board+0x874)) return;
    ext = GetBoardExt(board);
    if (!ext) return;
    n = *(int *)((char *)ext + UNI_VAC_COUNT);
    if (n <= 0 || n > 8) return;
    for (i = 0; i < n; i++) {
        DWORD face = *(DWORD *)((char *)ext + UNI_VAC_FACES + i * 4);
        if (!face || IsBadReadPtr((void *)face, 4)) continue;
        {
            DWORD vt = *(DWORD *)face;
            if (!vt || IsBadReadPtr((void *)vt, 4)) continue;
            {
                typedef void (__thiscall *VacTick_t)(void *);
                VacTick_t tick = (VacTick_t)*(DWORD *)vt;
                if (tick) tick((void *)face);
            }
        }
    }
}

static void Feature_NeonCamera(void *board, int level) {
    // Pause gate: board+0x874 !=0 means paused (ESC) — freeze anim + sound
    if (*(BYTE*)((char*)board+0x874)) return;
    /* S3-gap gate (lf_10): only reposition followers we actually built.
     * Native objects live at board+0x436C/0x4370 (NOT ext mesh slots). */
    if (!Neon_IsActive(board)) return;
    void* ext = GetBoardExt(board);
    if (!ext) return;
    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x600)) return;

    DWORD ball = *(DWORD *)(app + APP_BALL_PTR);
    if (!ball || IsBadReadPtr((void *)ball, 0x200)) return;

    float ballX = *(float *)(ball + BALL_POS_X_OFS);
    float ballY = *(float *)(ball + BALL_POS_Y_OFS);
    float ballZ = *(float *)(ball + BALL_POS_Z_OFS);

    /* Position P1 follower light (board+0x436C, built by Neon_PostSetup) */
    DWORD render1 = BoardHasOffset(board, 0x436C, 4) ?
        *(DWORD *)((char *)board + 0x436C) : 0;
    if (render1) {
        DWORD *vtbl = *(DWORD **)render1;
        if (vtbl) {
            void (__thiscall *setPos)(DWORD, float, float, float) =
                (void (__thiscall *)(DWORD, float, float, float))vtbl[0x1]; /* vtable[+4] */
            if (setPos) setPos((DWORD)render1, ballX + 20.0f, ballY + 30.0f, ballZ - 20.0f);
        }
    }

    /* Position P2 follower light (board+0x4370) if App+0x677 is 0 */
    if (!*(char *)(app + 0x677)) {
        DWORD ball2 = *(DWORD *)(app + 0x67C);
        if (ball2 && !IsBadReadPtr((void *)ball2, 0x200)) {
            float b2X = *(float *)(ball2 + BALL_POS_X_OFS);
            float b2Y = *(float *)(ball2 + BALL_POS_Y_OFS);
            float b2Z = *(float *)(ball2 + BALL_POS_Z_OFS);
            DWORD render2 = BoardHasOffset(board, 0x4370, 4) ?
                *(DWORD *)((char *)board + 0x4370) : 0;
            if (render2) {
                DWORD *vtbl2 = *(DWORD **)render2;
                if (vtbl2) {
                    void (__thiscall *setPos)(DWORD, float, float, float) =
                        (void (__thiscall *)(DWORD, float, float, float))vtbl2[0x1];
                    if (setPos) setPos((DWORD)render2, b2X + 20.0f, b2Y + 30.0f, b2Z - 20.0f);
                }
            }
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: Sky Popcylinder Activator (Sky)
 * Randomly activates popcylinders via Scene_SetRaceActive
 * Replicates SkyBoard_Update (0x41FC90)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void Feature_SkyPopcylinder(void *board, int level) {
    // Pause gate: board+0x874 !=0 means paused (ESC) — freeze anim + sound
    if (*(BYTE*)((char*)board+0x874)) return;
    if (!g_RNG || !g_SceneSetRaceActive) return;
    void* ext = GetBoardExt(board);
    if (!ext) return;
    /* 2026-11-20: Sky runs the universal path (UniversalSkyInit + S1
     * mirrors), so this replica drives the ext popcylinder bank +
     * magnifier. Native 0x41FC90 no longer runs for Sky. */

    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x600)) return;

    /* Magnifier update tick — native Sky Board_Update 0x41FCA7 (disasm-verified
     * 2026-09-11): difficulty (App+0x23C) != 0 -> magnifier->vtable[1]().
     * (Previous gate was inverted; harmless while the ext slot was unwritten.) */
    if (*(int *)(app + APP_DIFFICULTY) != 0) {
        /* Call vtable[+4] on the scene object at board+UNI_MAGNIFYING_GLASS */
        DWORD sceneObj = *(DWORD *)((char *)ext + UNI_MAGNIFYING_GLASS);
        if (sceneObj) {
            DWORD *vtbl = *(DWORD **)sceneObj;
            if (vtbl) {
                void (__fastcall *fn4)(DWORD) = (void (__fastcall *)(DWORD))vtbl[0x1];
                if (fn4) fn4((DWORD)sceneObj);
            }
        }
    }

    /* Check timer */
    if (!*(int *)((char *)ext + UNI_PEG_COUNT) || *(int *)(app + APP_DIFFICULTY) == 0) return;

    int counter = *(int *)((char *)ext + UNI_SKY_TIMER) - 1;
    *(int *)((char *)ext + UNI_SKY_TIMER) = counter;
    if (counter >= 1) return;

    /* Reset counter and activate random popcylinders */
    *(int *)((char *)ext + UNI_SKY_TIMER) = 0x4B; /* 75 */
    int rngCase = RNG_call((void *)0x4F7360, 0, 6, 0);

    /* Play sound at popcylinder idx8 position (native SkyBoard_Update plays
     * at 0x47D0==idx8 via App+0x480 in ALL 6 cases, Ghidra-verified) */
    DWORD pop8 = *(DWORD *)((char *)ext + UNI_SKY_POPCYL_BASE + 32);
    if (pop8 && g_SoundPlay3D) {
        DWORD snd = *(DWORD *)(app + 0x480);
        if (snd) {
            g_SoundPlay3D((void *)snd,
                *(float *)(pop8 + 0x10D4),
                *(float *)(pop8 + 0x10D8),
                *(float *)(pop8 + 0x10DC), 1.0f);
        }
    }

    /* Activate popcylinders per RNG case (native switch, Ghidra-verified
     * 2026-09-11; bank UNI_SKY_POPCYL_BASE+idx*4 == native 0x47B0+idx*4).
     * NULL-guarded: file-swapped levels may lack S1s (vanilla would crash;
     * LFL skips instead). */
#define SKY_POP(i) do { int o_ = *(int *)((char *)ext + UNI_SKY_POPCYL_BASE + (i) * 4); if (o_) g_SceneSetRaceActive(o_); } while (0)
    switch (rngCase) {
    case 0:
        SKY_POP(5);
        SKY_POP(6);
        SKY_POP(9);
        SKY_POP(10);
        break;
    case 1: {
        int i3 = 0;
        int rb = RNG_call((void *)0x4F7360, 0, 5, 0);
        if (rb != -3 && -1 < rb + 3) {
            do {
                int ri = RNG_call((void *)0x4F7360, 0, 0x10, 0);
                SKY_POP(ri);
                i3++;
                rb = RNG_call((void *)0x4F7360, 0, 5, 0);
            } while (i3 < rb + 3);
        }
        break;
    }
    case 2: {
        int i;
        for (i = 0; i < 0x10; i++)
            SKY_POP(i);
        break;
    }
    case 3:
        SKY_POP(0);
        SKY_POP(1);
        SKY_POP(2);
        SKY_POP(3);
        SKY_POP(4);
        SKY_POP(7);
        SKY_POP(8);
        SKY_POP(11);
        SKY_POP(12);
        SKY_POP(13);
        SKY_POP(14);
        SKY_POP(15);
        break;
    case 4: {
        int idx = RNG_call((void *)0x4F7360, 0, 4, 0);
        SKY_POP(idx * 4);
        SKY_POP(idx * 4 + 1);
        SKY_POP(idx * 4 + 2);
        SKY_POP(idx * 4 + 3);
        break;
    }
    case 5: {
        int idx = RNG_call((void *)0x4F7360, 0, 4, 0);
        SKY_POP(idx);
        SKY_POP(4 + idx);
        SKY_POP(8 + idx);
        SKY_POP(12 + idx);
        break;
    }
    }
#undef SKY_POP
}
