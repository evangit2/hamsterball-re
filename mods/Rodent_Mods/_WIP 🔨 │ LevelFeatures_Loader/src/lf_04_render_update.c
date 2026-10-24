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

    int level = GetCurrentLevel(board);
    void *ext = GetBoardExt(board);
    /* Native-ctor fast path: run the NATIVE custom scene render (orig slot
     * 24) instead of the shared base render. The base render walks objects
     * the custom scenes need (Toob/Glass died: NULL object+0x154, wild
     * HeapAlloc ret). Skip additive extras — native scene is complete.
     * Origs by CONTENT (g_origRender[15] is NULL — 15 uses 1-param render
     * and is never patched; cross-slot 15 falls back to base render). */
    if (ext && *(int *)((char *)ext + UNI_NATIVE_CTOR)) {
        int cslot = ContentFileId(ext);
        int effSlot = (cslot == 5 || (cslot >= 10 && cslot <= 15)) ? cslot : level;
        if (effSlot >= 1 && effSlot <= 15 && g_origRender[effSlot]) {
            static int rdbg = 0;
            if (rdbg < 3 && effSlot == 5) { DebugLog("Tower render ENTER (orig)"); rdbg++; }
            g_origRender[effSlot](board, unk);
            if (effSlot == 5) DebugLog("Tower render EXIT (orig)");
            return;
        }
    }
    if (!ext || level == 0) return;

    /* Tower content (any slot): clean-room render below (faithful to native
     * 0x40DFA0 as re-derived 2026-10-23: method-this is the timer object,
     * chomper via board+0x4390 vtbl[7]). Native delegation removed. */
#if 0
    if (ContentFileId(GetBoardExt(board)) == 5 && g_TowerRender) {
        g_TowerRender(board, unk);
        return;
    }
#endif

    /* Call shared base render (Level_RenderDynamicObjects) — forward gfx ctx (native ret $0x4) */
    g_RenderDynamicObjects(board, unk);

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
        /* Faithful Tower render (native 0x40DFA0). Method-this is the 68B
         * timer object T — NOT gfx. Gfx fns take this=T. Chomper is the
         * board+0x4390 MeshNode, drawn via its vtbl[7](obj, T, 0).
         * State floats come from ext mirrors (animated by update features);
         * object pointers/positions from native board slots. */

        char timerBuf[68];
        char zeroScratch[64];
        if (g_TimerInit && g_TimerCleanup && g_GfxScaleZ && g_GfxScaleX &&
            g_GfxSetPosition && g_WaveSin && g_Matrix44Zero) {
            g_TimerInit(timerBuf);
            DWORD *timerVtbl = *(DWORD **)timerBuf;
            void (__thiscall *scaleFn)(void *, float, float, float) = NULL;
            void (__thiscall *posFn)(void *, float, float, float) = NULL;
            if (timerVtbl) {
                scaleFn = (void (__thiscall *)(void *, float, float, float))timerVtbl[6];
                posFn = (void (__thiscall *)(void *, float, float, float))timerVtbl[2];
            }
            float chompState = *(float *)((char *)ext + UNI_WINDMILL_SPEED);
            float chompTimer = *(float *)((char *)ext + UNI_CHOMP_TIMER);
            float px = *(float *)((char *)board + 0x4394);
            float py = *(float *)((char *)board + 0x4398);
            float pz = *(float *)((char *)board + 0x439C);
            float liftY = *(float *)((char *)board + 0x43B0);
            DWORD chomperObj = *(DWORD *)((char *)board + 0x4390);

            /* Pass 1 */
            if (scaleFn) scaleFn(timerBuf, 1.15f, 1.15f, 1.15f);
            if (g_GfxScaleZ) g_GfxScaleZ(timerBuf, -chompState);
            if (posFn) posFn(timerBuf, px, py, pz);
            if (g_GfxSetPosition) g_GfxSetPosition(timerBuf, 0.0f, liftY, 0.0f);
            if (g_GfxSetPosition && g_WaveSin)
                g_GfxSetPosition(timerBuf, 0.0f, g_WaveSin((void *)0x4F7188, chompTimer) * 10.0f, 0.0f);
            if (chomperObj && !IsBadReadPtr((void *)chomperObj, 4)) {
                DWORD *cvtbl = *(DWORD **)chomperObj;
                if (cvtbl && !IsBadReadPtr(cvtbl, 32) && cvtbl[7]) {
                    void (__thiscall *chompRender)(void *, void *, int) =
                        (void (__thiscall *)(void *, void *, int))cvtbl[7];
                    chompRender((void *)chomperObj, timerBuf, 0);
                }
            }
            /* Pass 2: reset matrix, ScaleX(180), offset jaw, same draws */
            g_Matrix44Zero((int *)zeroScratch);
            if (scaleFn) scaleFn(timerBuf, 1.15f, 1.15f, 1.15f);
            if (g_GfxScaleZ) g_GfxScaleZ(timerBuf, -chompState);
            if (g_GfxScaleX) g_GfxScaleX(timerBuf, 180.0f);
            if (g_GfxSetPosition) g_GfxSetPosition(timerBuf, -35.0f, 0.0f, 0.0f);
            if (posFn) posFn(timerBuf, px, py, pz);
            if (g_GfxSetPosition) g_GfxSetPosition(timerBuf, 0.0f, liftY, 0.0f);
            if (g_GfxSetPosition && g_WaveSin)
                g_GfxSetPosition(timerBuf, 0.0f, g_WaveSin((void *)0x4F7188, chompTimer) * 10.0f, 0.0f);
            if (chomperObj && !IsBadReadPtr((void *)chomperObj, 4)) {
                DWORD *cvtbl = *(DWORD **)chomperObj;
                if (cvtbl && !IsBadReadPtr(cvtbl, 32) && cvtbl[7]) {
                    void (__thiscall *chompRender)(void *, void *, int) =
                        (void (__thiscall *)(void *, void *, int))cvtbl[7];
                    chompRender((void *)chomperObj, timerBuf, 0);
                }
            }
            g_TimerCleanup(timerBuf);
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

            g_TimerCleanup(timerBuf2);
            g_TimerCleanup(timerBuf);
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
                if (g_TimerCleanup) g_TimerCleanup(timerBuf);

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

/* Forward: S1 audit scanner lives in lf_07 (same TU, defined later). */
static void ScanS1AndAutoEnable(void *board, void *ext, void *meshWorld);

void __fastcall UniversalBoardUpdate(void *board) {
    if (!g_SceneUpdate || !board) return;

    /* Tower full-native: run the NATIVE Tower update (Scene_Update +
     * Tower behaviors at 0x41E760). Our feature blocks stay off (feats
     * unset — native drives everything, exactly like vanilla). */
    {
        void *ux = GetBoardExt(board);
        if (ux && *(int *)((char *)ux + UNI_NATIVE_CTOR) &&
            ContentFileId(ux) == 5 && g_TowerNativeUpdate) {
            static void *s_towerAudited = NULL;
            /* Hail-mary 2026-10-24: the native tail never built Tower's
             * scene (mesh NULL, no ball, no creates) in this process.
             * Invoke the tail's scene builder (slot-0x48) directly here —
             * if it returns, the board becomes vanilla-complete. */
            {
                static int s_s48done = 0;
                if (!s_s48done) {
                    s_s48done = 1;
                    DWORD s48 = 0;
                    /* Read current scene products first: if the tail built
                     * the scene itself, do NOT re-run slot-0x48. */
                    DWORD preMesh = 0, preRender = 0;
                    if (!IsBadReadPtr((char*)board + BOARD_RENDEROBJ, 8)) {
                        preMesh = *(DWORD*)((char*)board + BOARD_MESHWORLD);
                        preRender = *(DWORD*)((char*)board + BOARD_RENDEROBJ);
                    }
                    { char se[96]; wsprintfA(se, "Tower pre-s48 mesh=0x%08X render=0x%08X", preMesh, preRender); DebugLog(se); }
                    if (!preMesh || !preRender) {
                        if (!IsBadReadPtr(board, 4) && !IsBadReadPtr(*(void**)board, 0x100))
                            s48 = *(DWORD*)(*(DWORD*)board + 0x48);
                        { char se[96]; wsprintfA(se, "Tower slot48 invoke target=0x%08X", s48); DebugLog(se); }
                        if (s48 == 0x40D6D0) {
                            DebugLog("Tower slot48 ENTER");
                            ((void(__thiscall*)(void*))s48)(board);
                            DebugLog("Tower slot48 EXIT");
                        }
                    } else {
                        DebugLog("Tower tail built scene; deferred s48 skipped");
                    }
                }
            }
            /* Allocator probe: does game operator_new still work here?
             * NULL = heap broken (explains tail's missing mesh/ball). */
            {
                static int s_allocTested = 0;
                if (!s_allocTested && g_operatorNew) {
                    s_allocTested = 1;
                    void *tp = g_operatorNew(0x10D0);
                    char ab[96]; wsprintfA(ab, "Tower alloc probe 0x10D0 -> 0x%08X", (DWORD)tp);
                    DebugLog(ab);
                }
            }
            /* Validate native-built lists BEFORE first update (one-shot).
             * Scene_Update walks these; a corrupt list here = native setup
             * exposed by heap layout (uninit read). */
            {
                static int s_lvDone = 0;
                if (!s_lvDone) {
                    s_lvDone = 1;
                    int li; DWORD loffs[] = {0x2578,0x43B8,0x47D0,0x4BE8,0x5000,0x8B8,0x10EC,0xCD4,0x29D4,0};
                    for (li = 0; loffs[li]; li++) {
                        char lb[160]; DWORD lc = 0; void *li2 = NULL; int ok = 0;
                        MEMORY_BASIC_INFORMATION mbi;
                        if (!IsBadReadPtr((char*)board + loffs[li], 0x410)) {
                            lc = *(DWORD*)((char*)board + loffs[li] + 4);
                            li2 = *(void**)((char*)board + loffs[li] + 0x40C);
                            if (li2 && VirtualQuery(li2, &mbi, sizeof(mbi)) && mbi.State == MEM_COMMIT) ok = (lc < 100000) ? 1 : 0;
                            else if (!li2 && lc == 0) ok = 1;
                        }
                        wsprintfA(lb, "Tower list +%X count=%u items=0x%08X %s", loffs[li], lc, (DWORD)li2, ok ? "OK" : "BAD");
                        DebugLog(lb);
                    }
                    /* Mesh/render slots: did the tail build the scene? */
                    {
                        char mb[160]; void *mwm2 = *(void**)((char*)board + BOARD_MESHWORLD);
                        void *rob = *(void**)((char*)board + BOARD_RENDEROBJ);
                        wsprintfA(mb, "Tower scene mesh=0x%08X render=0x%08X", (DWORD)mwm2, (DWORD)rob);
                        DebugLog(mb);
                    }
                    /* Tail-completion: slot-0x48 writes board+0x3F18 timer.
                     * NULL/garbage = slot-0x48 never ran or died early. */
                    {
                        char tb[160]; void *tmr = *(void**)((char*)board + 0x3F18);
                        DWORD t10 = 0, t14 = 0;
                        if (tmr && !IsBadReadPtr(tmr, 0x18)) {
                            t10 = *(DWORD*)((char*)tmr + 0x10);
                            t14 = *(DWORD*)((char*)tmr + 0x14);
                        }
                        wsprintfA(tb, "Tower tailcheck timer=0x%08X +10=0x%08X +14=0x%08X", (DWORD)tmr, t10, t14);
                        DebugLog(tb);
                    }
                    /* Vtable integrity: did anything retarget Tower's vtable?
                     * Expect vt=0x4D0A08, slot+0x48=0x40D6D0 (file-verified). */
                    {
                        char vb[160]; DWORD vt = *(DWORD*)board;
                        DWORD s48 = 0;
                        if (!IsBadReadPtr((void*)vt, 0x100)) s48 = *(DWORD*)(vt + 0x48);
                        wsprintfA(vb, "Tower vtable=0x%08X slot48=0x%08X", vt, s48);
                        DebugLog(vb);
                    }
                }
            }
            DebugLog("Tower native update ENTER");
            /* Full native update now that the board is complete (deferred
             * slot-0x48 built mesh/render/init/setup/objects). */
            g_TowerNativeUpdate(board);
            DebugLog("Tower native update EXIT");
            /* Deferred read-only S1REF audit: mesh exists only after the
             * native tail built it (post-Constructor), so scan here. */
            if (s_towerAudited != board) {
                void *mwm = *(void **)((char *)board + BOARD_MESHWORLD);
                if (mwm && !IsBadReadPtr(mwm, 4)) {
                    ScanS1AndAutoEnable(board, ux, mwm);
                    s_towerAudited = board;
                    DebugLog("Tower S1REF audit done (native mesh)");
                }
            }
            return;
        }
    }

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

typedef void (__fastcall *ToobClampZoom_t)(int board);
static ToobClampZoom_t g_ToobClampZoom = NULL;

void __fastcall UniversalRaceState(void *board) {
    if (!g_BoardUpdateRaceState || !board) return;

    /* First-seen board log: reveals native-vs-universal construction paths
     * (a native board here means the ctor dispatch was bypassed). */
    {
        static void *rs_lastBoard = NULL;
        if (board != rs_lastBoard) {
            rs_lastBoard = board;
            char bl[160];
            wsprintfA(bl, "[raceState] first-seen board=%p vt=0x%X level=%d content=%d",
                board, *(DWORD *)board, GetCurrentLevel(board),
                ContentFileId(GetBoardExt(board)));
            DebugLog(bl);
        }
    }
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
    /* Native-ctor fast path: native-built boards run the NATIVE per-level
     * RaceState (orig), which matches the native ctor/create objects exactly.
     * Custom-branch replication (e.g. Toob ClampZoom) is skipped — the orig
     * IS the custom handler for these boards. Additive feature blocks below
     * (racestate_features) still run — ext-driven, S1-based. */
    {
        void *rsext = GetBoardExt(board);
        if (rsext && *(int *)((char *)rsext + UNI_NATIVE_CTOR)) {
            int cslot = ContentFileId(rsext);
            int effSlot = (cslot == 5 || (cslot >= 10 && cslot <= 15)) ? cslot : level;
            if (effSlot >= 1 && effSlot <= 15 && g_origRaceState[effSlot]) {
                g_origRaceState[effSlot](board);
                goto racestate_features;
            }
        }
    }
    if (level == 2) {
        /* skip orig 0x420240 (decays dead board+0x642C) — base + ext decay instead */
        g_BoardUpdateRaceState(board);
    } else if (level == 7 || ContentFileId(GetBoardExt(board)) == 7) {
        /* Neon RaceState (0x424790) positions board+0x436C/0x4370 followers.
         * Only valid when Neon_PostSetup built them (S3-gap gate passed);
         * otherwise the slots are NULL/garbage or foreign (never touch). */
        if (Neon_IsActive(board) && g_origRaceState[level]) {
            g_origRaceState[level](board);
        } else {
            g_BoardUpdateRaceState(board);
        }
    } else if (ContentFileId(GetBoardExt(board)) == 10) {
        /* Toob RaceState (0x41F7E0) = Board_UpdateRaceState + zoom clamp of
         * board+0x644C..+0x6468 (intro camera). Calls base internally, so
         * call it instead of the slot orig (single-chain safe).
         * BISECT 2026-10-23: ClampZoom suspected in frame-1 crash; fall back
         * to base-only until the zoom words are proven initialized. */
        g_BoardUpdateRaceState(board);
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
        /* Tower content: orig Tower RaceState is the shared base handler;
         * call base directly (verified identical, avoids double-drive). */
        if (ContentFileId(GetBoardExt(board)) == 5) g_BoardUpdateRaceState(board);
        else g_origRaceState[level](board);
        if (s_rsCount <= 4) DebugLog("  [raceState] orig/base call returned");
    } else {
        g_BoardUpdateRaceState(board);
        if (s_rsCount <= 4) DebugLog("  [raceState] fallback call returned");
    }

    DWORD extFeat = GetBoardFeat(board);
    DWORD features = extFeat; // Option B: pure S1-driven, no g_updateFeatures fallback
    if (!features) return;
racestate_features:
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
