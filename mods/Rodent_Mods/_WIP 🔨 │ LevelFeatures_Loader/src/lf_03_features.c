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
                    if (snd) g_SoundPlay3D((void *)snd, *pivotX, *pivotY, *pivotZ);
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
            if (angle >= 45.0f) { *anglePtr = 45.0f; *counterPtr = 0x4B; *statePtr = 0; }
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
                    void (__fastcall *fn54)(DWORD, char *) = (void (__fastcall *)(DWORD, char *))renderVtbl[0x15];
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
    if (!ext) ext = EnsureBoardExt(board);
    if (!ext) return;

    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x600)) return;

    /* Unified lists live in ext */
    int swirlListOfs = (level == 14) ? BRD_SWIRL_LIST_M : BRD_SWIRL_LIST;
    int tarListOfs = (level == 14) ? BRD_TARBUBBLE_LIST_M : BRD_TARBUBBLE_LIST;
    // swirlListOfs/tarListOfs are UNI offsets >=0x6500 — use ext


    /* TarBubble particle creation — replicates original DizzyBoard_Update.
     * RNG returns 10 (1 in 20 chance per frame) → create a tarbubble particle.
     * FUN_0044fa90 picks a random entry from the tarbubble list and reads
     * entry+0x04/0x08/0x0C as X/Y/Z floats. S1 ref points have this layout.
     * The particle is appended to UNI_PARTICLE_LIST for rendering. */
    static int swirlDbg = 0;
    if (swirlDbg < 3) DebugLog("  [swirl] step1: tarbubble check");
    if (g_CreateTarBubble && g_AthenaListAppend && g_operatorNew && g_RNG) {
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

    while (ballIdx) {
        /* Skip balls in tar (ball+0x2CC != 0) — they get sinking logic */
        char inTar = *(char *)(ballIdx + BALL_IN_TAR_OFS);
        if (!inTar) {
            /* Check proximity to each swirl zone */
            int zoneIter = g_AthenaListGetIterator((void *)((char *)ext + swirlListOfs));
            *(int *)((char *)ext + swirlListOfs + 8 + zoneIter * 4) = 0;
            int zoneCount = *(int *)((char *)ext + swirlListOfs + 4);
            int zoneIdx = 0;
            if (zoneCount > 0) {
                DWORD zoneArrayPtr = *(DWORD *)((char *)ext + swirlListOfs + 0x40C);
                if (zoneArrayPtr && !IsBadReadPtr((void *)zoneArrayPtr, 4)) {
                    zoneIdx = *(int *)zoneArrayPtr;
                    *(int *)((char *)ext + swirlListOfs + 8 + zoneIter * 4) = 1;
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
                                        g_SoundPlay3D((void *)snd, ballX, ballY, ballZ);
                                }
                                /* Spawn 3 particles */
                                int p;
                                for (p = 0; p < 3 && g_operatorNew && g_AthenaListAppend; p++) {
                                    float *part = (float *)g_operatorNew(0x14);
                                    if (part) {
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
                int next = *(int *)((char *)ext + swirlListOfs + 8 + zoneIter * 4);
                if (*(int *)((char *)ext + swirlListOfs + 4) <= next) break;
                DWORD zoneArr = *(DWORD *)((char *)ext + swirlListOfs + 0x40C);
                if (!zoneArr || IsBadReadPtr((void *)zoneArr, (next + 1) * 4)) break;
                zoneIdx = *(int *)(zoneArr + next * 4);
                *(int *)((char *)ext + swirlListOfs + 8 + zoneIter * 4) = next + 1;
            }
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
        float angle1 = *(float*)((char*)ext + OFF_SWIRL_ANGLE1) - 0.5f;
        *(float*)((char*)ext + OFF_SWIRL_SPEED)=0.5f; *(float*)((char*)ext + OFF_SWIRL_ANGLE1)=angle1;
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
            DWORD waterRender = *(DWORD*)((char*)ext + UNI_MESH_1);
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
        *(float*)((char*)ext + OFF_SWIRL_ANGLE2) = *(float*)((char*)ext + OFF_SWIRL_ANGLE2) + swirlSpeed;
        if (swirlDbg < 3) DebugLog("  [swirl] step3b: calling Matrix44Zero");
        g_Matrix44Zero((int *)timerBuf);
        if (swirlDbg < 3) DebugLog("  [swirl] step3b: calling GfxScaleX");
        g_GfxScaleX(gfx, *(float *)((char *)ext + OFF_SWIRL_ANGLE2));
        if (swirlDbg < 3) DebugLog("  [swirl] step3b: calling GfxSetPosition");
        g_GfxSetPosition(gfx,
            *(float *)((char *)ext + OFF_SWIRL_POS_X),
            *(float *)((char *)ext + OFF_SWIRL_POS_Y),
            *(float *)((char *)ext + OFF_SWIRL_POS_Z));
        if (swirlDbg < 3) DebugLog("  [swirl] step3b: Gfx calls done, calling render");
        {
            DWORD swirlRender = *(DWORD*)((char*)ext + UNI_MESH_7);
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
                if (snd) g_SoundPlay3D((void *)snd, *posX, *posY, *posZ);
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
                void (__fastcall *fn54)(DWORD, char *) = (void (__fastcall *)(DWORD, char *))vtbl[0x15];
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
    case 1: { int c = *counterPtr - 1; *counterPtr = c; if (c < 1) { *statePtr = 2; if (g_SoundPlay3D) { DWORD snd = *(DWORD *)(app + 0x4A8); if (snd) g_SoundPlay3D((void *)snd, *posX, *posY, *posZ); } } } break;
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
    if (!ext) ext = EnsureBoardExt(board);
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
        if (snd) g_SoundPlay3D((void *)snd, spawnX, spawnY, spawnZ);
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

    /* Only activate when difficulty != 0 */
    if (*(int *)(app + APP_DIFFICULTY) == 0) {
        /* Still call vtable[+4] on the scene object at board+UNI_MAGNIFYING_GLASS */
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

    /* Play sound at rotator position */
    DWORD rotator = *(DWORD *)((char *)ext + UNI_TRAPDOOR_LIST);
    if (rotator && g_SoundPlay3D) {
        DWORD snd = *(DWORD *)(app + 0x480);
        if (snd) {
            g_SoundPlay3D((void *)snd,
                *(float *)(rotator + 0x10D4),
                *(float *)(rotator + 0x10D8),
                *(float *)(rotator + 0x10DC));
        }
    }

    /* Activate popcylinders based on RNG case */
    switch (rngCase) {
    case 0:
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_CATAPULT_DATA));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 24));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_TRAPDOOR_COUNT));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_TRAPDOOR_LIST + 8));
        break;
    case 1: {
        int n = RNG_call((void *)0x4F7360, 0, 5, 0);
        int i;
        for (i = 0; i < n + 3; i++) {
            int idx = RNG_call((void *)0x4F7360, 0, 0x10, 0);
            g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + idx * 4));
        }
        break;
    }
    case 2: {
        int i;
        for (i = 0; i < 0x10; i++)
            g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + i * 4));
        break;
    }
    case 3:
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 4));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 8));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 12));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 16));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 28));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_TRAPDOOR_LIST));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 44));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 48));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 52));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 56));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 60));
        break;
    case 4: {
        int idx = RNG_call((void *)0x4F7360, 0, 4, 0);
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + idx * 0x10));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 4 + idx * 0x10));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 8 + idx * 0x10));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 12 + idx * 0x10));
        break;
    }
    case 5: {
        int idx = RNG_call((void *)0x4F7360, 0, 4, 0);
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + idx * 4));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 16 + idx * 4));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_TRAPDOOR_LIST + idx * 4));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 48 + idx * 4));
        break;
    }
    }
}
