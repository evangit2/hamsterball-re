/* Chapter 4 - UniversalRenderImpl + BoardUpdate + RaceState (LevelFeatures.c lines 2815-3417) */
/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Render (Slot 24) — replaces all 15 per-level render functions
 *
 * Calls Level_RenderDynamicObjects first (the shared base render), then
 * dispatches to per-level render feature blocks based on g_renderFeatures[].
 *
 * Render blocks:
 *   REND_BUMPER:   Bumper reflective material (Beginner, Toob, Master)
 *   REND_WINDMILL: Tower windmill + chomper render with projection adjust
 *   REND_GLASS:    Glass transparent smasher render with render-state toggles
 *   REND_SKY_CAM:  Sky camera setup + cloud sprite + transparent objects
 * ═══════════════════════════════════════════════════════════════════════════ */

/* UniversalRender implementation — does the actual rendering work.
 * Called by the naked thunk below which handles RET 4.
 * Must be non-static for asm reference from the naked thunk. */
void UniversalRenderImpl(void *board, int unk) {
    if (!board || !g_RenderDynamicObjects) return;

    /* Call shared base render (Level_RenderDynamicObjects) — forward gfx ctx (native ret $0x4) */
    g_RenderDynamicObjects(board, unk);

    int level = GetCurrentLevel(board);
    void* ext = GetBoardExt(board);
    if (!ext) return;
    if (level == 0) return;

    DWORD meshWorld = *(DWORD *)((char *)board + BOARD_MESHWORLD);

    /* Compute render features dynamically based on what's actually enabled.
     * No hardcoded per-level defaults — any render feature can be used on any
     * level as long as the required objects/events are present. */
    DWORD features = 0;

    /* REND_BUMPER: active when N:BUMPER collision event is enabled for this level */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_BUMPER)))
        features |= REND_BUMPER;

    /* REND_WINDMILL/CHOMPER/TURRET: each tower segment gated on its own S1 / render pointer.
     * WINDMILL S1 -> REND_TOWER_WINDMILL, CHOMPER S1 -> REND_TOWER_CHOMPER, TURRET* S1 -> REND_TOWER_TURRET */
    if (*(DWORD *)((char *)ext + REND_TOWER_WINDMILL) != 0)
        features |= REND_WINDMILL;
    if (*(DWORD *)((char *)ext + REND_TOWER_CHOMPER) != 0)
        features |= REND_CHOMPER;
    if (*(DWORD *)((char *)ext + REND_TOWER_TURRET) != 0)
        features |= REND_TURRET;

    /* REND_GLASS: active when N:GLASS collision event is enabled for this level */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_GLASS)))
        features |= REND_GLASS;

    /* REND_SKY_BOX: skybox + cloud sprite (CLOUDSCAPE S1). Points 1-3: projection, cull, sprite quad */
    if (*(DWORD *)((char *)ext + REND_SKY_SPRITE) != 0)
        features |= REND_SKY_BOX;
    /* REND_SKY_LIST: transparent object list (point 4) — only if list has objects or mesh exists */
    {
        int skyCount = *(int *)((char *)ext + REND_SKY_LIST + 4);
        DWORD skyMesh = *(DWORD *)((char *)ext + REND_SKY_MESH);
        if (skyCount > 0 || skyMesh != 0)
            features |= REND_SKY_LIST;
    }

    if (!features) return;

    /* ── Bumper Reflective Render (Beginner, Toob, Master) ──
     * Iterates ehVector slots, renders bumpers with reflective material
     * when their lit value is non-zero. Reads from unified offsets. */
    if (features & REND_BUMPER) {
        int bumperCount = (level == 14) ? 4 : 8;
        float *litPtr = (float *)((char *)ext + UNI_BUMPER_LIT);
        char *slotPtr = (char *)ext + UNI_EHVECTOR;
        int i;
        for (i = 0; i < bumperCount; i++) {
            if (litPtr[i] != 0.0f) {
                /* Set up render context with lit value as material multiplier */
                char renderCtx[64];
                if (g_RenderContextInit) g_RenderContextInit(renderCtx);
                *(float *)(renderCtx + 0x10) = litPtr[i];  /* lit float */
                *(float *)(renderCtx + 0x14) = litPtr[i];  /* duplicate */
                /* Set all material components to 1.0 */
                *(DWORD *)(renderCtx + 0x04) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x08) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x0C) = 0x3F800000;
                *(char *)(renderCtx + 0x20) = (litPtr[i] != 1.0f) ? 1 : 0;
                *(DWORD *)(renderCtx + 0x24) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x28) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x2C) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x30) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x34) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x38) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x3C) = 0x3F800000;
                /* Store render context pointer on board */
                *(void **)((char *)board + 0x7C0) = renderCtx;
                /* Call meshWorld->vtable[0x14](slotPtr) to render bumper */
                if (meshWorld && !IsBadReadPtr((void *)meshWorld, 0x54)) {
                    DWORD *vtbl = *(DWORD **)meshWorld;
                    if (vtbl) {
                        void (__thiscall *fn)(DWORD, DWORD) = (void (__thiscall *)(DWORD, DWORD))vtbl[0x14];
                        if (fn) fn((DWORD)meshWorld, (DWORD)slotPtr); /* native: ECX=meshWorld + push slot */
                    }
                }
                /* Reset render context */
                if (g_Matrix4Identity) g_Matrix4Identity(renderCtx);
            }
        }
        /* Clear render context pointer */
        *(void **)((char *)board + 0x7C0) = NULL;
    }

    /* ── Tower Render — split per reference (WINDMILL / CHOMPER / TURRET tower) ──
     * Each segment only runs if its S1 was present (WINDMILL, CHOMPER, TURRET*).
     * Original combined them under one flag; now gated individually.
     * Inner per-pointer checks stay, outer is OR of all three so a level with
     * only CHOMPER still renders it. */
    if (features & (REND_WINDMILL | REND_CHOMPER | REND_TURRET)) {
        DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
        if (!app || IsBadReadPtr((void *)app, 0x200)) goto glass_render;
        void *gfx = *(void **)((char *)app + 0x174);
        if (!gfx) goto glass_render;

        char timerBuf[68];
        if (g_TimerInit && g_TimerCleanup) {
            g_TimerInit(timerBuf);

            /* Scale matrix (1.15, 1.15, 1.15) */
            DWORD *timerVtbl = *(DWORD **)timerBuf;
            if (timerVtbl) {
                void (__thiscall *scaleFn)(DWORD, float, float, float) =
                    (void (__thiscall *)(DWORD, float, float, float))timerVtbl[6];
                if (scaleFn) scaleFn((DWORD)timerBuf, 1.15f, 1.15f, 1.15f); /* was int 0x3f933333 = 1.06e9f, not 1.15f */
            }

            /* Gfx_ScaleZ(-board[0x43A0]) — chomper-state float, NOT windmill angle (0x40DFA0) */
            float chompState = *(float *)((char *)ext + UNI_WINDMILL_SPEED);
            if (g_GfxScaleZ) g_GfxScaleZ(gfx, -chompState);

            /* Render windmill render obj via vtable[0x08] */
            DWORD windmillRender = *(DWORD *)((char *)ext + REND_TOWER_WINDMILL);
            if (windmillRender && !IsBadReadPtr((void *)windmillRender, 4)) {
                DWORD *vtbl = *(DWORD **)windmillRender;
                if (vtbl) {
                    void (__thiscall *fn8)(DWORD) = (void (__thiscall *)(DWORD))vtbl[2];
                    if (fn8) fn8(windmillRender);
                }
            }

            /* Gfx_SetPosition(0, board[0x43B0], 0) then (0, Wave_Sin(0x4F7188, 0x43A4)*10.0, 0) (0x40DFA0) */
            if (g_GfxSetPosition) g_GfxSetPosition(gfx, 0.0f, *(float *)((char *)ext + UNI_WINDMILL_DECAY), 0.0f);
            if (g_GfxSetPosition && g_WaveSin) g_GfxSetPosition(gfx, 0.0f, g_WaveSin((void *)0x4F7188, *(float *)((char *)ext + UNI_CHOMP_TIMER)) * 10.0f, 0.0f);

            /* Render Chomper mesh via meshWorld->vtable[0x1C] */
            DWORD chomperMesh = *(DWORD *)((char *)ext + REND_TOWER_CHOMPER);
            if (meshWorld && !IsBadReadPtr((void *)meshWorld, 0x1C) && chomperMesh) {
                DWORD *vtbl = *(DWORD **)meshWorld;
                if (vtbl) {
                    void (__thiscall *fn1C)(DWORD) = (void (__thiscall *)(DWORD))vtbl[7];
                    if (fn1C) fn1C(chomperMesh);
                }
            }

            /* Reset: scale matrix again + Gfx_ScaleZ(-rotation) + Gfx_ScaleX(180) */
            if (timerVtbl) {
                void (__thiscall *scaleFn)(DWORD, float, float, float) =
                    (void (__thiscall *)(DWORD, float, float, float))timerVtbl[6];
                if (scaleFn) scaleFn((DWORD)timerBuf, 1.15f, 1.15f, 1.15f); /* was int 0x3f933333 = 1.06e9f, not 1.15f */
            }
            if (g_GfxScaleZ) g_GfxScaleZ(gfx, -chompState);
            if (g_GfxScaleX) g_GfxScaleX(gfx, 180.0f);
            if (g_GfxSetPosition) g_GfxSetPosition(gfx, -35.0f, 0.0f, 0.0f); /* 0xC20C0000 (0x40DFA0) */

            /* Render turret render obj from dedicated offset */
            DWORD turretRender = *(DWORD *)((char *)ext + REND_TOWER_TURRET);
            if (turretRender && !IsBadReadPtr((void *)turretRender, 4)) {
                DWORD *vtbl = *(DWORD **)turretRender;
                if (vtbl) {
                    void (__thiscall *fn8)(DWORD) = (void (__thiscall *)(DWORD))vtbl[2];
                    if (fn8) fn8(turretRender);
                }
            }

            /* Reset position + Wave_Sin bob + render Chomper again (0x40DFA0) */
            if (g_GfxSetPosition) g_GfxSetPosition(gfx, 0.0f, *(float *)((char *)ext + UNI_WINDMILL_DECAY), 0.0f);
            if (g_GfxSetPosition && g_WaveSin) g_GfxSetPosition(gfx, 0.0f, g_WaveSin((void *)0x4F7188, *(float *)((char *)ext + UNI_CHOMP_TIMER)) * 10.0f, 0.0f);
            if (meshWorld && !IsBadReadPtr((void *)meshWorld, 0x1C) && chomperMesh) {
                DWORD *vtbl = *(DWORD **)meshWorld;
                if (vtbl) {
                    void (__thiscall *fn1C)(DWORD) = (void (__thiscall *)(DWORD))vtbl[7];
                    if (fn1C) fn1C(chomperMesh);
                }
            }

            g_TimerCleanup(timerBuf, timerBuf);
        }
    }

    glass_render:

    /* ── Glass Transparent Render ──
     * Renders two transparent glass smasher meshes with render-state toggles.
     * Reads from dedicated REND_GLASS_* offsets so it can coexist with any
     * other render features on the same level. */
    if (features & REND_GLASS) {
        DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
        if (!app || IsBadReadPtr((void *)app, 0x600)) goto sky_render;
        void *gfx = *(void **)((char *)app + 0x174);
        if (!gfx) goto sky_render;

        char timerBuf[68];
        if (g_TimerInit && g_TimerCleanup) {
            g_TimerInit(timerBuf);

            /* Smasher 1: Gfx_ScaleX(transparency), then call Timer vtable[0x08]
             * with position floats (X, Y, Z) — NOT a mesh pointer.
             * Original: vtable[0x08](timerObj, posX, posY, posZ) */
            float transp1 = *(float *)((char *)ext + REND_GLASS_TRANSP1);
            if (g_GfxScaleX) g_GfxScaleX(gfx, transp1);
            DWORD *timerVtbl = *(DWORD **)timerBuf;
            if (timerVtbl) {
                void (__thiscall *fn8)(DWORD, float, float, float) = (void (__thiscall *)(DWORD, float, float, float))timerVtbl[2];
                if (fn8) {
                    fn8((DWORD)timerBuf,
                        *(float *)((char *)ext + REND_GLASS_S1_X),
                        *(float *)((char *)ext + REND_GLASS_S1_Y),
                        *(float *)((char *)ext + REND_GLASS_S1_Z));
                }
            }

            /* Select render device based on flag */
            BYTE flag1 = *(BYTE *)((char *)ext + REND_GLASS_FLAG1);
            DWORD renderDev;
            if (flag1 == 0) {
                renderDev = *(DWORD *)(app + 0x584);
            } else {
                renderDev = *(DWORD *)(app + 0x588);
            }
            if (renderDev && !IsBadReadPtr((void *)renderDev, 0x1C)) {
                DWORD *vtbl = *(DWORD **)renderDev;
                if (vtbl) {
                    void (__thiscall *fn1C)(DWORD) = (void (__thiscall *)(DWORD))vtbl[7];
                    if (fn1C) fn1C(renderDev);
                }
            }

            /* Smasher 2: Gfx_ScaleX(transparency2), render mesh */
            char timerBuf2[68];
            g_TimerInit(timerBuf2);
            float transp2 = *(float *)((char *)ext + REND_GLASS_TRANSP2);
            if (g_GfxScaleX) g_GfxScaleX(gfx, transp2);
            DWORD *timerVtbl2 = *(DWORD **)timerBuf2;
            if (timerVtbl2) {
                void (__thiscall *fn8)(DWORD, float, float, float) = (void (__thiscall *)(DWORD, float, float, float))timerVtbl2[2];
                if (fn8) {
                    fn8((DWORD)timerBuf2,
                        *(float *)((char *)ext + REND_GLASS_S2_X),
                        *(float *)((char *)ext + REND_GLASS_S2_Y),
                        *(float *)((char *)ext + REND_GLASS_S2_Z));
                }
            }

            BYTE flag2 = *(BYTE *)((char *)ext + REND_GLASS_FLAG2);
            if (flag2 == 0) {
                renderDev = *(DWORD *)(app + 0x584);
            } else {
                renderDev = *(DWORD *)(app + 0x588);
            }
            if (renderDev && !IsBadReadPtr((void *)renderDev, 0x1C)) {
                DWORD *vtbl = *(DWORD **)renderDev;
                if (vtbl) {
                    void (__thiscall *fn1C)(DWORD) = (void (__thiscall *)(DWORD))vtbl[7];
                    if (fn1C) fn1C(renderDev);
                }
            }

            g_TimerCleanup(timerBuf2, timerBuf2);
            g_TimerCleanup(timerBuf, timerBuf);
        }
    }

    sky_render:

    /* ── Sky Box (CLOUDSCAPE skybox) — far-clip + CLOUDSCAPE cloud sprite (points 1-3) ──
     * Only active if CLOUDSCAPE S1 / REND_SKY_SPRITE exists. */
    if (features & REND_SKY_BOX) {
        DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
        if (!app || IsBadReadPtr((void *)app, 0x200)) return;
        void *gfx = *(void **)((char *)app + 0x174);
        if (!gfx) return;

        /* Set far-clip projection: near=20.0, far=50000.0 */
        if (g_GraphicsSetProjection)
            g_GraphicsSetProjection(gfx, 20.0f, 50000.0f);

        /* Toggle render state (gfx+0x70C) — D3D state management */
        if (*(char *)((char *)gfx + 0x70C) != 0) {
            DWORD *gfxVtbl = *(DWORD **)gfx;
            if (gfxVtbl) {
                void (__thiscall *fnC8)(DWORD, int, int) =
                    (void (__thiscall *)(DWORD, int, int))gfxVtbl[0x32];
                if (fnC8) fnC8((DWORD)gfx, 0xe, 0);
            }
            *(char *)((char *)gfx + 0x70C) = 0;
            (*(int *)((char *)gfx + 0x7C8))++;
        }

        /* Disable culling for cloud sprite */
        if (g_GraphicsSetCullMode2) g_GraphicsSetCullMode2(gfx, 0, 1);

        /* Render cloud sprite quad from dedicated REND_SKY_SPRITE */
        if (g_SpriteRenderQuad) {
            DWORD sprite = *(DWORD *)((char *)ext + REND_SKY_SPRITE);
            float sp1 = *(float *)((char *)ext + REND_SKY_SPRITE_X);
            float sp2 = *(float *)((char *)ext + REND_SKY_SPRITE_Y);
            float sp3 = *(float *)((char *)ext + REND_SKY_SPRITE_Z);
            if (sprite) g_SpriteRenderQuad((void *)sprite, sp1, sp2, sp3, 23.0f, 0);
        }

        /* Re-enable culling */
        if (g_GraphicsSetCullMode2) g_GraphicsSetCullMode2(gfx, 1, 1);

        /* Toggle render state back */
        if (*(char *)((char *)gfx + 0x70C) != 1) {
            DWORD *gfxVtbl = *(DWORD **)gfx;
            if (gfxVtbl) {
                void (__thiscall *fnC8)(DWORD, int, int) =
                    (void (__thiscall *)(DWORD, int, int))gfxVtbl[0x32];
                if (fnC8) fnC8((DWORD)gfx, 0xe, 1);
            }
            *(char *)((char *)gfx + 0x70C) = 1;
            (*(int *)((char *)gfx + 0x7C8))++;
        }

        /* Adjust projection based on gfx field */
        float fovAdjust = *(float *)((char *)gfx + 0x188);
        fovAdjust = (fovAdjust + fovAdjust) * 0.0009765625f + 0.00048828125f;
        if (g_GraphicsSetProjection)
            g_GraphicsSetProjection(gfx, 20.0f, fovAdjust + fovAdjust);
    }
    /* ── Sky Transparent List (point 4) — only if respective objects exist ──
     * Iterates REND_SKY_LIST (UNI_LIST_7) of transparent popcylinders etc.
     * Gated on REND_SKY_LIST (count>0 or mesh). */
    if (features & REND_SKY_LIST) {
        DWORD app2 = *(DWORD *)((char *)board + BOARD_APP_PTR);
        void *gfx2 = (app2 && !IsBadReadPtr((void *)app2, 0x200)) ? *(void **)((char *)app2 + 0x174) : NULL;
        if (gfx2) {
        /* Level_RenderDynamicObjects was already called above.
         * Now iterate the Sky render object AthenaList and render transparent objects.
         * Uses dedicated REND_SKY_LIST (UNI_LIST_7) — separate from swirl's UNI_LIST_6. */
        if (g_AthenaListGetIterator && g_AthenaListGetSize && meshWorld) {
            int iter = g_AthenaListGetIterator((void *)((char *)ext + REND_SKY_LIST));
            *(int *)((char *)ext + REND_SKY_LIST + 8 + iter * 4) = 0;
            int count = *(int *)((char *)ext + REND_SKY_LIST + 4);
            int item = 0;
            if (count > 0) {
                DWORD arr = *(DWORD *)((char *)ext + REND_SKY_LIST + 0x40C);
                if (arr && !IsBadReadPtr((void *)arr, 4)) {
                    item = *(int *)arr;
                    *(int *)((char *)ext + REND_SKY_LIST + 8 + iter * 4) = 1;
                }
            }
            while (item) {
                char timerBuf[68];
                if (g_TimerInit) g_TimerInit(timerBuf);
                DWORD *tVtbl = *(DWORD **)timerBuf;
                if (tVtbl) {
                    void (__thiscall *fn8)(DWORD, DWORD, DWORD, DWORD) =
                        (void (__thiscall *)(DWORD, DWORD, DWORD, DWORD))tVtbl[2];
                    if (fn8) {
                        fn8((DWORD)timerBuf,
                            *(DWORD *)(item + 4),
                            *(DWORD *)(item + 8),
                            *(DWORD *)(item + 0xC));
                    }
                }
                /* Render via transparent mesh from dedicated REND_SKY_MESH */
                DWORD mesh = *(DWORD *)((char *)ext + REND_SKY_MESH);
                if (mesh && !IsBadReadPtr((void *)mesh, 0x1C)) {
                    DWORD *mVtbl = *(DWORD **)mesh;
                    if (mVtbl) {
                        void (__thiscall *fn1C)(DWORD, int) =
                            (void (__thiscall *)(DWORD, int))mVtbl[7];
                        if (fn1C) fn1C(mesh, 0);
                    }
                }
                if (g_TimerCleanup) g_TimerCleanup(timerBuf, timerBuf);

                int next = *(int *)((char *)ext + REND_SKY_LIST + 8 + iter * 4);
                if (count <= next) break;
                DWORD arr2 = *(DWORD *)((char *)ext + REND_SKY_LIST + 0x40C);
                if (!arr2 || IsBadReadPtr((void *)arr2, (next + 1) * 4)) break;
                item = *(int *)(arr2 + next * 4);
                *(int *)((char *)ext + REND_SKY_LIST + 8 + iter * 4) = next + 1;
            }
        }
        } // gfx2
    } // REND_SKY_LIST
} // UniversalRenderImpl

/* Naked thunk for UniversalRender — forwards (ECX=board, stack gfx) to
 * UniversalRenderImpl and cleans the 1 stack param (RET 4, __thiscall).
 * Binary-verified 2026-09-12: ALL 15 native renders take (board, gfx) with
 * RET 4 (shared 0x40B420 ends ret $0x4; wrappers pass gfx through), and the
 * main Board dispatch (0x46C9F0) pushes gfx before call [eax+0x60]. Only the
 * 6 wrapper levels {2,5,10,12,13,14} are patched; the other 9 keep native.
 */
__attribute__((naked)) void UniversalRender(void) {
    __asm__ __volatile__(
        "pushl %%ebp\n\t"
        "movl  %%esp, %%ebp\n\t"
        "pushl %%edx\n\t"          /* save EDX (unused but preserved) */
        "pushl 8(%%ebp)\n\t"       /* forward caller's gfx stack arg */
        "pushl %%ecx\n\t"          /* forward board (ECX) */
        "call  _UniversalRenderImpl\n\t"
        "addl  $8, %%esp\n\t"
        "popl  %%edx\n\t"
        "popl  %%ebp\n\t"
        "ret   $4\n\t"             /* __thiscall: callee cleans 4 bytes */
        :: : "eax", "ecx", "memory"
    );
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Board_Update — replaces all 15 per-level Board_Update functions
 *
 * Calls Scene_Update first, then dispatches to feature blocks based on
 * the per-level feature flags in g_updateFeatures[].
 * ═══════════════════════════════════════════════════════════════════════════ */

void __fastcall UniversalBoardUpdate(void *board) {
    if (!g_SceneUpdate || !board) return;

    static int s_updateCount = 0;
    s_updateCount++;
    if (s_updateCount <= 3) {
        char dbg[128];
        wsprintfA(dbg, "UniversalBoardUpdate #%d (board=0x%08X)", s_updateCount, (DWORD)board);
        DebugLog(dbg);
    }

    /* Call base Scene_Update */
    DebugLog("  [update] calling Scene_Update...");
    g_SceneUpdate(board);
    DebugLog("  [update] Scene_Update done");

    /* Get level and dispatch features */
    int level = GetCurrentLevel(board);
    if (level == 0) return;

    // Pause already handled per-feature, but gate here too
    if (*(BYTE*)((char*)board+0x874)) return;
    DWORD extFeat = GetBoardFeat(board);
    DWORD features = extFeat; // Option B: pure S1-driven, no g_updateFeatures fallback
    if (!features) return;

    static int featDbg = 0;
    /* Bridge animation (Intermediate + Master bridge) */
    if (features & FEAT_BRIDGE_ANIM) {
        if (featDbg < 3) DebugLog("  [update] Feature_BridgeAnimation...");
        Feature_BridgeAnimation(board, level);
        if (featDbg < 3) DebugLog("  [update] Feature_BridgeAnimation done");
    }

    /* Swirl zones (Dizzy + Master) */
    if (features & FEAT_SWIRL) {
        if (featDbg < 3) DebugLog("  [update] Feature_SwirlZones...");
        Feature_SwirlZones(board, level);
        if (featDbg < 3) DebugLog("  [update] Feature_SwirlZones done");
    }

    /* Windmill (Tower) */
    if (features & FEAT_WINDMILL) {
        if (featDbg < 3) DebugLog("  [update] Feature_Windmill...");
        Feature_Windmill(board, level);
        if (featDbg < 3) DebugLog("  [update] Feature_Windmill done");
    }

    /* BadBall spawner (Odd) */
    if (features & FEAT_BADBALL) {
        if (featDbg < 3) DebugLog("  [update] Feature_BadBallSpawner...");
        Feature_BadBallSpawner(board, level);
        if (featDbg < 3) DebugLog("  [update] Feature_BadBallSpawner done");
    }
    if (featDbg < 3) featDbg++;

    /* Note: Bumper decay, neon camera, and sky popcylinder are handled in
       UniversalRaceState (slot 19), NOT here. In the original game, these
       are in the RaceState handler, not Board_Update. */
}

/* Naked thunk for vtable slot 1 (Board_Update) */
/* Replaces the original __fastcall Board_Update(board) with UniversalBoardUpdate */
/* Must be callable via vtable indirect call: CALL [vtable+0x4] */
/* We patch each level's vtable[1] to point to UniversalBoardUpdate */

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Race State (Slot 19) — replaces all 15 per-level RaceState handlers
 *
 * Calls Board_UpdateRaceState first, then dispatches to feature blocks
 * that belong in RaceState (bumper decay, neon camera, sky popcylinders).
 * ═══════════════════════════════════════════════════════════════════════════ */

void __fastcall UniversalRaceState(void *board) {
    if (!g_BoardUpdateRaceState || !board) return;

    static int s_rsCount = 0;
    s_rsCount++;
    if (s_rsCount <= 3) {
        char dbg[128];
        wsprintfA(dbg, "  [raceState] #%d calling Board_UpdateRaceState...", s_rsCount);
        DebugLog(dbg);
    }

    /* Single-chain rule (2026-09-12 audit): every saved orig either IS the
     * base (10 shared levels) or calls it internally first (all 5 customs,
     * verified in disasm), so the dispatch below runs orig-or-base exactly
     * once — never both (double-run was 2x timers/RNG/lifters). */

    /* Get level and dispatch features */
    int level = GetCurrentLevel(board);
    if (level == 0) return;

    /* Call the ORIGINAL per-level RaceState handler.  Several levels have
     * custom RaceState code that iterates per-level data at board+0x436C:
     *   Up  (lvl 6):  iterates lifter AthenaList, calls each Lifter_Update
     *   Neon(lvl 7):  calls vtable[1] on render objects at board+0x436C/0x4370
     *   Beginner(2):  decays 8 float timers at board+0x642C-0x6448
     * Without this call, lifters don't animate, vacuum events never fire, etc.
     * Beginner is skipped — orig decays board+0x642C while Feature_BumperDecay
     * decays ext+0x85C0 (UNI_BUMPER_LIT); calling both double-decays and desyncs. */
    if (level == 2) {
        /* skip orig 0x420240 (decays dead board+0x642C) — base + ext decay instead */
        g_BoardUpdateRaceState(board);
    } else if (level == 7) {
        /* Neon RaceState (0x424790) positions board+0x436C/0x4370 followers.
         * Only valid when Neon_PostSetup built them (S3-gap gate passed);
         * otherwise the slots are NULL/garbage or foreign (never touch). */
        if (Neon_IsActive(board) && g_origRaceState[level]) {
            g_origRaceState[level](board);
        } else {
            g_BoardUpdateRaceState(board);
        }
    } else if (level == 13) {
        /* Sky RaceState (0x41FC90) RNG-activates NATIVE-slot popcylinders.
         * Universal objects live in ext slots via Feature_SkyPopcylinder;
         * calling both double-activates (2x rate + doubled sounds) and runs
         * orig on native slots the universal builder never filled. Skip orig
         * when the ext feature will run; keep it as fallback otherwise. */
        DWORD skyFeat = GetBoardFeat(board);
        if (!(skyFeat & FEAT_SKY_POPCYL) && g_origRaceState[level]) {
            g_origRaceState[level](board);
        } else {
            g_BoardUpdateRaceState(board);
        }
    } else if (level >= 1 && level <= 15 && g_origRaceState[level]) {
        g_origRaceState[level](board);
    } else {
        g_BoardUpdateRaceState(board);
    }

    DWORD extFeat = GetBoardFeat(board);
    DWORD features = extFeat; // Option B: pure S1-driven, no g_updateFeatures fallback
    if (!features) return;
    if (*(BYTE*)((char*)board+0x874)) return;
    if (features & FEAT_BUMPER_DECAY)
        Feature_BumperDecay(board, level);

    /* Neon camera follow (Neon) */
    if (features & FEAT_NEON_CAM)
        Feature_NeonCamera(board, level);

    /* Sky popcylinder activator (Sky) */
    if (features & FEAT_SKY_POPCYL)
        Feature_SkyPopcylinder(board, level);
}
