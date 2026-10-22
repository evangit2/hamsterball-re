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
    case 0: { int c = *counterPtr - 1; *counterPtr = c; if (c < 1) *statePtr = 1; } break;
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
                    float pivot[3]; g_Vec3CopyUpd(pivot, pivotX);
                    g_SceneForEachBallSetVelocity(board, pivot[1], pivot[2], 0.5f);
                }
            }
        } break;
    case 2: { int c = *counterPtr - 1; *counterPtr = c; if (c < 1) *statePtr = 3; } break;
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
                        float cd[3]; char ctimer[68];
                        if (!cb || IsBadReadPtr((void *)cb, 0x800)) continue;
                        if (!*(BYTE *)((char *)cb + 0x778)) continue;
                        cd[0] = *(float *)((char *)cb + 0x164) - *pivotX;
                        cd[1] = *(float *)((char *)cb + 0x168) - *pivotY;
                        cd[2] = *(float *)((char *)cb + 0x16C) - *pivotZ;
                        g_TimerInit(ctimer);
                        g_GfxScaleZ(ctimer, -0.5f);
                        g_MatrixTransformVec3((float *)ctimer, cd);
                        *(float *)((char *)cb + 0x164) = cd[0] + *pivotX;
                        *(float *)((char *)cb + 0x168) = cd[1] + *pivotY;
                        *(float *)((char *)cb + 0x16C) = cd[2] + *pivotZ;
                        g_TimerCleanup(ctimer);
                    }
                }
            }
            if (g_TimerInit && g_TimerCleanup && g_GfxScaleZ && g_GfxSetPosition && g_MatrixTransformVec3 && app) {
                void *gfx = *(void **)(app + 0x174);
                if (gfx) {
                char timerBuf[68];
                g_TimerInit(timerBuf);
                g_GfxScaleZ(gfx, -*anglePtr);
                g_GfxSetPosition(gfx, *pivotX, *pivotY, *pivotZ);
                DWORD *renderVtbl = *(DWORD **)renderObj;
                if (renderVtbl) {
                    void (__fastcall *fn58)(DWORD) = (void (__fastcall *)(DWORD))renderVtbl[0x16];
                    void (__thiscall *fn54)(DWORD, void *) = (void (__thiscall *)(DWORD, void *))renderVtbl[0x15]; /* native: ECX=obj + push timer, ret $0x4 */
                    if (fn58) fn58((DWORD)renderObj);
                    if (fn54) fn54((DWORD)renderObj, timerBuf);
                }
                g_TimerCleanup(timerBuf);
                }
            }
        } break;
    }
    // No board mirror — ext is source of truth
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: Swirl Zones (Dizzy / Master)
 * Proximity check → velocity scale + tar bubbles + mesh rotation
 * Replicates DizzyBoard_Update (0x41D510) / Master Board_Update (0x420DA0)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void Feature_SwirlZones(void *board, int level) {
    // Pause gate: board+0x874 !=0 means paused (ESC) — freeze anim + sound
    if (*(BYTE*)((char*)board+0x874)) return;
    if (!g_AthenaListGetIterator || !g_operatorNew) return;
    void* ext = GetBoardExt(board);
    /* Get-only (no resurrect): teardown frames must not rebind zombie ext. */
    if (!ext) return;

    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x600)) return;

    /* Unified lists live in ext */
    int swirlListOfs = (level == 14) ? BRD_SWIRL_LIST_M : BRD_SWIRL_LIST;
    int tarListOfs = (level == 14) ? BRD_TARBUBBLE_LIST_M : BRD_TARBUBBLE_LIST;
    // swirlListOfs/tarListOfs are UNI offsets >=0x6500 — use ext

    /* Dizzy-class boards use NATIVE slots/list walks (native Dizzy update
     * parity); other levels keep the ext-unified path. Content-keyed so a
     * Dizzy file in any slot behaves identically. */
    int isDizzy = (level == 4) || (ContentFileId(ext) == 4);

    /* Native ambient-tarbubble gate (Dizzy update head): spawn only when
     * size(board+0x362C) != 1 or the player ball's +0x14C flag is clear.
     * Board_ctor inits the list the same under the mod, so the gate reads
     * identically. Non-Dizzy keeps the legacy always-try behavior. */
    int ambientGate = 1;
    if (isDizzy && g_AthenaListGetSize) {
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

    /* Zone source: native board+0x4378 gluebie list on Dizzy-class boards
     * (native behavior — gluebies ARE the proximity zones, with per-gluebie
     * radius at +0x1100); ext unified zones on other levels. */
    void *zoneBase = isDizzy ? board : ext;
    DWORD zoneOff = isDizzy ? 0x4378 : (DWORD)swirlListOfs;

    while (ballIdx) {
        /* Skip balls in tar (ball+0x2CC != 0) — they get sinking logic */
        char inTar = *(char *)(ballIdx + BALL_IN_TAR_OFS);
        if (!inTar) {
            /* Per-ball hit flag (native ball+0x2BC: set when any zone hits
             * this frame, cleared otherwise — gates the contact sound). */
            int anyHit = 0;
            /* Check proximity to each swirl zone */
            int zoneIter = g_AthenaListGetIterator((void *)((char *)zoneBase + zoneOff));
            *(int *)((char *)zoneBase + zoneOff + 8 + zoneIter * 4) = 0;
            int zoneCount = *(int *)((char *)zoneBase + zoneOff + 4);
            int zoneIdx = 0;
            if (zoneCount > 0) {
                DWORD zoneArrayPtr = *(DWORD *)((char *)zoneBase + zoneOff + 0x40C);
                if (zoneArrayPtr && !IsBadReadPtr((void *)zoneArrayPtr, 4)) {
                    zoneIdx = *(int *)zoneArrayPtr;
                    *(int *)((char *)zoneBase + zoneOff + 8 + zoneIter * 4) = 1;
                }
            }

            while (zoneIdx) {
                if (*(int *)(ballIdx + 0x18) >= 0) {
                    float ballX = *(float *)(ballIdx + BALL_POS_X_OFS);
                    float ballY = *(float *)(ballIdx + BALL_POS_Y_OFS);
                    float ballZ = *(float *)(ballIdx + BALL_POS_Z_OFS);
                    float zoneX = *(float *)(zoneIdx + 0x10E0);
                    float zoneY = *(float *)(zoneIdx + 0x10E4);
                    float zoneZ = *(float *)(zoneIdx + 0x10E8);
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
                                        /* Native particle init: dword[3]=1 (bits
                                         * 0x00000001), byte+16=0. */
                                        ((int *)part)[3] = 1;
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
                int next = *(int *)((char *)zoneBase + zoneOff + 8 + zoneIter * 4);
                if (*(int *)((char *)zoneBase + zoneOff + 4) <= next) break;
                DWORD zoneArr = *(DWORD *)((char *)zoneBase + zoneOff + 0x40C);
                if (!zoneArr || IsBadReadPtr((void *)zoneArr, (next + 1) * 4)) break;
                zoneIdx = *(int *)(zoneArr + next * 4);
                *(int *)((char *)zoneBase + zoneOff + 8 + zoneIter * 4) = next + 1;
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

    /* Dizzy-only: mesh rotation (Master doesn't rotate meshes) */
    if (swirlDbg < 3) DebugLog("  [swirl] step3: mesh rotation");
    if (level != 14 && g_TimerInit && g_TimerCleanup && g_GfxScaleY &&
        g_GfxSetPosition && g_Matrix44Zero && app) {
        void *gfx = *(void **)(app + 0x174);
        if (gfx) {
        char timerBuf[68];
        g_TimerInit(timerBuf);

        /* Primary swirl mesh rotation (Gfx_ScaleY) */
        if (swirlDbg < 3) DebugLog("  [swirl] step3a: primary mesh rotation");
        /* Native angle lives at board+0x4BBC on Dizzy-class boards (zeroed
         * per WATERWHEEL S1 at create, decremented here); ext mirror else. */
        float *pAngW = isDizzy ? (float *)((char *)board + 0x4BBC)
                               : (float *)((char *)ext + OFF_SWIRL_ANGLE1);
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
                if (ry!=0) g_GfxScaleY(gfx, angle1 * ry); else g_GfxScaleY(gfx, angle1);
                if (rx!=0) g_GfxScaleX(gfx, angle1 * rx);
                if (rz!=0) g_GfxScaleZ(gfx, angle1 * rz);
            } else g_GfxScaleY(gfx, angle1);
        }
        if (swirlDbg < 3) DebugLog("  [swirl] step3a: calling GfxSetPosition");
        g_GfxSetPosition(gfx, *(float*)((char*)ext + OFF_WHEEL_EMBED_X), *(float*)((char*)ext + OFF_WHEEL_EMBED_Y), *(float*)((char*)ext + OFF_WHEEL_EMBED_Z));
        if (swirlDbg < 3) DebugLog("  [swirl] step3a: Gfx calls done, calling render");
        // Re-enabled after ext-heap fix — heap overflow was corrupting spatial tree at +0x18
        {
            /* Dizzy-class: draw the NATIVE mesh (board+0x4BA8, vtbl 0x58/0x54
             * — exactly the native update's draw calls); else ext render. */
            DWORD drawObj = isDizzy ? *(DWORD *)((char *)board + 0x4BA8)
                                    : *(DWORD*)((char*)ext + UNI_MESH_1);
            /* Native wheel center for sound below. */
            float wcx = isDizzy ? *(float *)((char *)board + 0x4BB0) : *(float*)((char*)ext + OFF_WHEEL_EMBED_X);
            float wcy = isDizzy ? *(float *)((char *)board + 0x4BB4) : *(float*)((char*)ext + OFF_WHEEL_EMBED_Y);
            float wcz = isDizzy ? *(float *)((char *)board + 0x4BB8) : *(float*)((char*)ext + OFF_WHEEL_EMBED_Z);
            /* Wheel creak (native block 3 tail): distance attenuation at the
             * wheel center, played through the ctor-grabbed channel +0x4BDC. */
            if (isDizzy && g_SoundCalcDistAtten && g_SoundPlay3DAtPos) {
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
    }

    /* Dizzy: secondary swirl mesh rotation (Gfx_ScaleX) */
    if (level != 14 && g_TimerInit && g_TimerCleanup && g_GfxScaleX &&
        g_GfxSetPosition && g_Matrix44Zero && app) {
        void *gfx = *(void **)(app + 0x174);
        if (gfx) {
        if (swirlDbg < 3) DebugLog("  [swirl] step3b: secondary mesh rotation");
        char timerBuf[68];
        g_TimerInit(timerBuf);

        float swirlSpeed = (*(int *)(app + APP_DIFFICULTY) == 0) ? 0.25f : 0.5f;
        /* Native angle lives at board+0x4BD8 on Dizzy-class boards; ext else. */
        float *pAngS = isDizzy ? (float *)((char *)board + 0x4BD8)
                               : (float *)((char *)ext + OFF_SWIRL_ANGLE2);
        *pAngS = *pAngS + swirlSpeed;
        if (swirlDbg < 3) DebugLog("  [swirl] step3b: calling Matrix44Zero");
        g_Matrix44Zero((int *)timerBuf);
        if (swirlDbg < 3) DebugLog("  [swirl] step3b: calling GfxScaleX");
        g_GfxScaleX(gfx, *pAngS);
        if (swirlDbg < 3) DebugLog("  [swirl] step3b: calling GfxSetPosition");
        g_GfxSetPosition(gfx,
            isDizzy ? *(float *)((char *)board + 0x4BCC) : *(float *)((char *)ext + OFF_SWIRL_POS_X),
            isDizzy ? *(float *)((char *)board + 0x4BD0) : *(float *)((char *)ext + OFF_SWIRL_POS_Y),
            isDizzy ? *(float *)((char *)board + 0x4BD4) : *(float *)((char *)ext + OFF_SWIRL_POS_Z));
        if (swirlDbg < 3) DebugLog("  [swirl] step3b: Gfx calls done, calling render");
        {
            /* Dizzy-class: draw the NATIVE mesh (board+0x4BC4); else ext render. */
            DWORD drawS = isDizzy ? *(DWORD *)((char *)board + 0x4BC4)
                                  : *(DWORD*)((char*)ext + UNI_MESH_7);
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
    }
    /* Dizzy riders: wheel (ball+0x778) and swirl (ball+0x779) carry.
     * Native blocks 4/5: the ball's center-relative offset is rotated
     * through the live Gfx matrix and the ball repositioned (kinematic
     * carry); the swirl also steers velocity with a release countdown at
     * ball+0x78C and a capture cap at ball+0x788. */
    if (isDizzy && g_TimerInit && g_TimerCleanup && g_GfxScaleY && g_GfxScaleX &&
        g_MatrixTransformVec3 && app) {
        void *gfxR = *(void **)(app + 0x174);
        if (gfxR && !IsBadReadPtr(gfxR, 4)) {
        float wcx2 = *(float *)((char *)board + 0x4BB0);
        float wcy2 = *(float *)((char *)board + 0x4BB4);
        float wcz2 = *(float *)((char *)board + 0x4BB8);
        float scx = *(float *)((char *)board + 0x4BCC);
        float scy = *(float *)((char *)board + 0x4BD0);
        float scz = *(float *)((char *)board + 0x4BD4);
        float sdelta = (*(int *)(app + APP_DIFFICULTY) == 0) ? 0.25f : 0.5f;
        int ri = g_AthenaListGetIterator((void *)((char *)board + UNI_BALL_LIST));
        *(int *)((char *)board + UNI_BALL_ITER + ri * 4) = 0;
        int rcount = *(int *)((char *)board + UNI_BALL_COUNT);
        int rb = 0;
        if (rcount > 0) {
            DWORD rarr = *(DWORD *)((char *)board + UNI_BALL_ARRAY);
            if (rarr && !IsBadReadPtr((void *)rarr, 4)) {
                rb = *(int *)rarr;
                *(int *)((char *)board + UNI_BALL_ITER + ri * 4) = 1;
            }
        }
        while (rb) {
            /* Wheel carry (native block 4: ScaleY(-0.5) + transform). */
            if (*(char *)(rb + 0x778)) {
                float rel[3];
                float out[16];
                char tB[68];
                rel[0] = *(float *)(rb + BALL_POS_X_OFS) - wcx2;
                rel[1] = *(float *)(rb + BALL_POS_Y_OFS) - wcy2;
                rel[2] = *(float *)(rb + BALL_POS_Z_OFS) - wcz2;
                g_TimerInit(tB);
                g_GfxScaleY(gfxR, -0.5f);
                g_MatrixTransformVec3(out, rel);
                *(float *)(rb + BALL_POS_X_OFS) = wcx2 + out[0];
                *(float *)(rb + BALL_POS_Y_OFS) = wcy2 + out[1];
                *(float *)(rb + BALL_POS_Z_OFS) = wcz2 + out[2];
                g_TimerCleanup(tB);
            }
            /* Swirl capture / carry (native block 5). */
            if (*(char *)(rb + 0x779) == 0) {
                int cd = *(int *)(rb + 0x78C);
                if (cd != 0) {
                    cd--;
                    *(int *)(rb + 0x78C) = cd;
                    if (cd == 0) {
                        float cap = (float)(*(int *)(rb + 0x788)) * 0.0001f;
                        DWORD phys = *(DWORD *)(rb + BALL_PHYS_PTR_OFS);
                        if (phys && !IsBadReadPtr((void *)phys, 0xCB0) && cap > 0.0f) {
                            float vx = *(float *)(phys + BALL_PHYS_VEL_X);
                            float vy = *(float *)(phys + BALL_PHYS_VEL_Y);
                            float vz = *(float *)(phys + BALL_PHYS_VEL_Z);
                            float sp = sqrtf(vx*vx + vy*vy + vz*vz);
                            if (sp > cap && sp > 0.0f) {
                                float s = cap / sp;
                                *(float *)(phys + BALL_PHYS_VEL_X) = vx * s;
                                *(float *)(phys + BALL_PHYS_VEL_Y) = vy * s;
                                *(float *)(phys + BALL_PHYS_VEL_Z) = vz * s;
                            }
                        }
                    }
                }
            } else {
                *(int *)(rb + 0x78C) = 5;
                {
                float rel[3];
                float out[16];
                char tB[68];
                float ox = *(float *)(rb + BALL_POS_X_OFS);
                float oy = *(float *)(rb + BALL_POS_Y_OFS);
                float oz = *(float *)(rb + BALL_POS_Z_OFS);
                rel[0] = ox - scx; rel[1] = oy - scy; rel[2] = oz - scz;
                g_TimerInit(tB);
                g_GfxScaleX(gfxR, sdelta);
                g_MatrixTransformVec3(out, rel);
                g_TimerCleanup(tB);
                *(float *)(rb + BALL_POS_X_OFS) = scx + out[0];
                *(float *)(rb + BALL_POS_Y_OFS) = scy + out[1];
                *(float *)(rb + BALL_POS_Z_OFS) = scz + out[2];
                DWORD phys = *(DWORD *)(rb + BALL_PHYS_PTR_OFS);
                if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
                    float vx = *(float *)(phys + BALL_PHYS_VEL_X);
                    float vy = *(float *)(phys + BALL_PHYS_VEL_Y);
                    float vz = *(float *)(phys + BALL_PHYS_VEL_Z);
                    float osp = sqrtf(vx*vx + vy*vy + vz*vz);
                    *(int *)(rb + 0x788) = (int)(osp * 10000.0f);
                    float nvx = vx + (out[0] - rel[0]) * 0.1f;
                    float nvy = vy + (out[1] - rel[1]) * 0.1f;
                    float nvz = vz + (out[2] - rel[2]) * 0.1f;
                    float nsp = sqrtf(nvx*nvx + nvy*nvy + nvz*nvz);
                    if (nsp > osp && osp > 0.0f && nsp > 0.0f) {
                        float s = osp / nsp;
                        nvx *= s; nvy *= s; nvz *= s;
                    }
                    *(float *)(phys + BALL_PHYS_VEL_X) = nvx;
                    *(float *)(phys + BALL_PHYS_VEL_Y) = nvy;
                    *(float *)(phys + BALL_PHYS_VEL_Z) = nvz;
                }
                }
            }
            int rn = *(int *)((char *)board + UNI_BALL_ITER + ri * 4);
            if (*(int *)((char *)board + UNI_BALL_COUNT) <= rn) break;
            { DWORD rarr2 = *(DWORD *)((char *)board + UNI_BALL_ARRAY);
            if (!rarr2 || IsBadReadPtr((void *)rarr2, (rn + 1) * 4)) break;
            rb = *(int *)(rarr2 + rn * 4);
            *(int *)((char *)board + UNI_BALL_ITER + ri * 4) = rn + 1; }
        }
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

static void Feature_Windmill(void *board, int level) {
    // Pause gate: board+0x874 !=0 means paused (ESC) — freeze anim + sound
    if (*(BYTE*)((char*)board+0x874)) return;
    /* Tower content (any slot): drive the NATIVE board slots (native Tower
     * update never runs under the mod). Replicates Tower slot-1 windmill
     * sequence 1:1 — angle on +0x438C, sound at 45 mod 90 via app+0x4A4,
     * matrix compose + vtbl[0x16]/[0x15] on the native +0x437C render.
     * Native Tower render reads these slots, so the windmill visibly spins. */
    if (ContentFileId(GetBoardExt(board)) == 5) {
        /* NOTE: native Tower ctor leaves board+0x878 (app) as 0 — fall back
         * to the app captured at Logic time, else every app read fails. */
        extern int g_lastAppParam;
        DWORD app5 = *(DWORD *)((char *)board + BOARD_APP_PTR);
        if (!app5) app5 = (DWORD)g_lastAppParam;
        if (!app5 || IsBadReadPtr((void *)app5, 0x500)) return;
        float *bAng = (float *)((char *)board + 0x438C);
        float rate = (*(int *)(app5 + APP_DIFFICULTY) == 0) ? 0.25f : 1.0f;
        *bAng = *bAng + rate;
        {
            int ai = (int)*bAng;
            if (ai % 0x5A == 0x2D && g_SoundPlay3D) {
                DWORD snd = *(DWORD *)(app5 + 0x4A4);
                if (snd) g_SoundPlay3D((void *)snd,
                    *(float *)((char *)board + 0x4380),
                    *(float *)((char *)board + 0x4384),
                    *(float *)((char *)board + 0x4388), 1.0f);
            }
        }
        DWORD wmr = *(DWORD *)((char *)board + 0x437C);
        if (wmr && !IsBadReadPtr((void *)wmr, 4) &&
            g_Matrix4Identity && g_GfxScaleY && g_GfxSetPosition) {
            char tA[68], tB[68], tC[68];
            g_Matrix4Identity(tA);
            g_GfxScaleY(tB, *bAng);
            g_GfxSetPosition(tC,
                *(float *)((char *)board + 0x4380),
                *(float *)((char *)board + 0x4384),
                *(float *)((char *)board + 0x4388));
            DWORD *vt = *(DWORD **)wmr;
            if (vt) {
                void (__fastcall *fn58)(DWORD) = (void (__fastcall *)(DWORD))vt[0x16];
                void (__thiscall *fn54)(DWORD, void *) = (void (__thiscall *)(DWORD, void *))vt[0x15];
                if (fn58) fn58(wmr);
                if (fn54) fn54(wmr, tA);
            }
        }
        return;
    }
    void* ext = GetBoardExt(board);
    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x500)) return;
    float *anglePtr; float *speedPtr; int *statePtr; int *counterPtr; float *decayPtr; float *posX; float *posY; float *posZ;
    DWORD *renderPtr;
    if (!ext) return;
    anglePtr = (float*)((char*)ext + UNI_WINDMILL_ANGLE);
    speedPtr = (float*)((char*)ext + UNI_WINDMILL_SPEED);
    statePtr = (int*)((char*)ext + UNI_WINDMILL_STATE);
    counterPtr = (int*)((char*)ext + UNI_WINDMILL_COUNTER);
    decayPtr = (float*)((char*)ext + UNI_WINDMILL_DECAY);
    posX = (float*)((char*)ext + UNI_WINDMILL_X);
    posY = (float*)((char*)ext + UNI_WINDMILL_Y);
    posZ = (float*)((char*)ext + UNI_WINDMILL_Z);
    renderPtr = (DWORD*)((char*)ext + REND_TOWER_WINDMILL);
    /* Fallback for legacy BONK_STORE alias during migration. */
    if (!*renderPtr && *(DWORD*)((char*)ext+UNI_BONK_STORE)) *renderPtr = *(DWORD*)((char*)ext+UNI_BONK_STORE);
    float rotSpeed = (*(int *)(app + APP_DIFFICULTY) == 0) ? 0.25f : 1.0f;
    *anglePtr = *anglePtr + rotSpeed;
    *(float*)((char*)ext + UNI_CHOMP_TIMER) += 3.0f; /* mirror board+0x43A4 (_DAT_004CF418) */
    {
        int angleInt = (int)*anglePtr;
        if (angleInt % 0x5A == 0x2D) {
            if (g_SoundPlay3D) {
                DWORD snd = *(DWORD *)(app + 0x4A4);
                if (snd) g_SoundPlay3D((void *)snd, *posX, *posY, *posZ, 1.0f);
            }
        }
    }
    if (g_TimerInit && g_TimerCleanup && g_GfxScaleY && g_GfxSetPosition && app) {
        void *gfx = *(void **)(app + 0x174);
        if (gfx) {
        char timerBuf[68];
        g_TimerInit(timerBuf);
        g_GfxScaleY(gfx, *anglePtr);
        g_GfxSetPosition(gfx, *posX, *posY, *posZ);
        DWORD renderObj = *renderPtr;
        if (renderObj) {
            DWORD *vtbl = *(DWORD **)renderObj;
            if (vtbl) {
                void (__fastcall *fn58)(DWORD) = (void (__fastcall *)(DWORD))vtbl[0x16];
                void (__thiscall *fn54)(DWORD, void *) = (void (__thiscall *)(DWORD, void *))vtbl[0x15]; /* native: ECX=obj + push timer, ret $0x4 */
                if (fn58) fn58((DWORD)renderObj);
                if (fn54) fn54((DWORD)renderObj, timerBuf);
            }
        }
        g_TimerCleanup(timerBuf);
        }
    }
    int wmState = *statePtr;
    switch (wmState) {
    case 0: { float speed = *speedPtr; if (speed == 0.0f) speed = 0.25f; speed *= 1.2f; *speedPtr = speed; if (speed > 25.0f) { *speedPtr = 25.0f; *statePtr = 1; *counterPtr = 0x19; *decayPtr = 50.0f; } } break;
    case 1: { int c = *counterPtr - 1; *counterPtr = c; if (c < 1) { *statePtr = 2; if (g_SoundPlay3D) { DWORD snd = *(DWORD *)(app + 0x4A8); if (snd) g_SoundPlay3D((void *)snd, *posX, *posY, *posZ, 1.0f); } } } break;
    case 2: { float speed = *speedPtr * 0.25f; *speedPtr = speed; if (speed < 1.0f) { *speedPtr = 0.0f; if (g_RNG) { int rng = RNG_call((void *)0x4F7360, 0, 2, 0); if (rng != 0) { *statePtr = 3; int rng2 = RNG_call((void *)0x4F7360, 0, 100, 0); *counterPtr = rng2 + 100; } else { *statePtr = 0; } } } } break;
    case 3: { float decay = *decayPtr - 2.0f; if (decay < 0.0f) decay = 0.0f; *decayPtr = decay; int c = *counterPtr - 1; *counterPtr = c; if (c < 1) *statePtr = 0; } break;
    }
    // No board mirror — ext is source of truth
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: BadBall Spawner (Odd)
 * Timer-based spawning of BadBall enemies
 * Replicates FUN_0041EE80 (Odd Board_Update)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void Feature_BadBallSpawner(void *board, int level) {
    // Pause gate: board+0x874 !=0 means paused (ESC) — freeze anim + sound
    if (*(BYTE*)((char*)board+0x874)) return;
    if (!g_RNG || !g_BadBallCtor || !g_operatorNew || !g_AthenaListAppend) return;
    void* ext = GetBoardExt(board);
    /* Get-only (no resurrect): teardown frames must not rebind zombie ext. */
    if (!ext) return;

    char spawnFlag = *(char *)((char *)ext + BRD_BB_FLAG);
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
    void *badball = g_BadBallCtor(mem, (int)board);
    if (!badball) return;

    /* Set trajectory */
    g_BallSetTrajectory(badball, 0x41EF8A, spawnX, spawnY, spawnZ, 0.0f);

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

    g_AthenaListAppend((void *)((char *)board + UNI_BALL_LIST), (int)badball);
    *(int *)((char *)ext + BRD_BB_TOTAL) = totalSpawned + 1;
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

    if (level == 2) {        /* Beginner */
        baseOfs = BRD_BUMPER_DECAY_BEG;
        count = 8;
    } else if (level == 10) { /* Toob */
        baseOfs = BRD_BUMPER_DECAY_TOOB;
        count = 8;
    } else if (level == 14) { /* Master */
        baseOfs = BRD_BUMPER_DECAY_MAST;
        count = 4;
    } else {
        return;
    }

    int i;
    for (i = 0; i < count; i++) {
        float val = *(float *)((char *)ext + baseOfs + i * 4);
        val -= 0.05f;
        if (val <= 0.0f) val = 0.0f;
        *(float *)((char *)ext + baseOfs + i * 4) = val;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: Neon Camera (Neon)
 * Positions render objects relative to ball
 * Replicates FUN_00424790 (Neon slot[19])
 * ═══════════════════════════════════════════════════════════════════════════ */

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
        SKY_POP(6);
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
