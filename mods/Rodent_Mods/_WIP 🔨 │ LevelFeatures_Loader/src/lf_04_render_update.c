static void FireBranchTeleport(void *board, int *ball, const char *branchName, int isBranchB);
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
void ToobDiag(const char *line);
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
    if (ext && *(int *)((char *)ext + UNI_NATIVE_HANDLERS)) {
        int cslot = ContentFileId(ext);
        int effSlot = (cslot >= 10 && cslot <= 15) ? cslot : level;
        if (effSlot >= 1 && effSlot <= 15 && g_origRender[effSlot]) {
            g_origRender[effSlot](board, unk);
            return;
        }
    }
    if (!ext || level == 0) {
        /* Degrade to base render, never blank: ext can miss on teardown
         * races / map pressure while the board is still drawn. */
        if (!ext && board) g_RenderDynamicObjects(board, unk);
        return;
    }

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

    /* REND_BUMPER: active when N:BUMPER collision event is enabled for this level.
     * Universal Toob (native-built content-10) has no bumper S1s — its slots
     * are native-built — so the render runs on native-built content-10 too. */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_BUMPER)) ||
        (ContentFileId(ext) == 10 && *(int *)((char *)ext + UNI_NATIVE_CTOR)))
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
     * Faithful to native Scene_RenderReflectiveObjects7 (0x411380): one
     * slot per bumper, lit drives the flash, ctx layout matches native,
     * ctx pointer on GFX+0x7C0. Slots come from OUR postscan collect
     * (UniversalPostSetup: AthenaList-init + Scene_CollectByNameFilter
     * "N:BUMPER%d" into ext UNI_EHVECTOR+i*0x418) — the replica board
     * ctor is generic and never fills the native board+0x436C slots, so
     * reading those is always empty (the missing-glow bug). Same meshes,
     * same stride, same vtbl[0x14] call. Old replica used a fixed ext
     * slot + undersized ctx + board+0x7C0 (Draw crash + no flash). */
    if ((features & REND_BUMPER) &&
        (level == 2 || level == 10 || level == 14 ||
         ContentFileId(ext) == 2 || ContentFileId(ext) == 10 ||
         ContentFileId(ext) == 14)) {
        int bumperCount = (level == 14 || ContentFileId(ext) == 14) ? 4 : 8;
        float *litPtr = (float *)((char *)ext + UNI_BUMPER_LIT);
        /* Universal Toob: stock Toob has no bumper meshes, so the postscan
         * collect finds nothing — but the bumper slot structs ARE native-built
         * at board+0x438C (0x418 stride, same structs the native render passes
         * to vtbl[0x14]). Render those instead of the empty ext slots. */
        int useNativeSlots = (ContentFileId(ext) == 10 && *(int *)((char *)ext + UNI_NATIVE_CTOR) &&
            BoardHasOffset(board, 0x438C + 7 * 0x418 + 0x410, 4));
        int i;
        for (i = 0; i < bumperCount; i++) {
            if (litPtr[i] != 0.0f) {
                /* Presence gate: our postscan collect fills ext slots with
                 * AthenaLists (count + readable items). Skip anything
                 * else rather than rendering a hollow/garbage slot. */
                char *slotPtr = (char *)ext + UNI_EHVECTOR + i * 0x418;
                if (useNativeSlots) {
                    slotPtr = (char *)board + 0x438C + i * 0x418;
                } else {
                    if (IsBadReadPtr(slotPtr, 0x410)) continue;
                    if (*(int *)(slotPtr + 0x04) <= 0) continue;
                    { DWORD itArr = *(DWORD *)(slotPtr + 0x40C);
                      if (!itArr || IsBadReadPtr((void *)itArr, 4)) continue; }
                }
                /* Native ctx is 0x4E bytes (Init writes vtable+17 dwords+
                 * 2 bytes); 64-byte buffer smashed the stack. */
                char renderCtx[0x60];
                if (g_RenderContextInit) g_RenderContextInit(renderCtx);
                *(float *)(renderCtx + 0x10) = litPtr[i];  /* lit */
                *(float *)(renderCtx + 0x20) = litPtr[i];  /* lit */
                *(DWORD *)(renderCtx + 0x04) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x08) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x0C) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x14) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x18) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x1C) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x34) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x38) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x3C) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x40) = 0x3F800000;
                *(char *)(renderCtx + 0x4C) = (litPtr[i] != 1.0f) ? 1 : 0;
                /* Ctx pointer belongs on GFX+0x7C0 (native reads it there). */
                if (!IsBadReadPtr((void *)unk, 0x800))
                    *(void **)((char *)unk + 0x7C0) = renderCtx;
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
        if (!IsBadReadPtr((void *)unk, 0x800))
            *(void **)((char *)unk + 0x7C0) = NULL;
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
            float chompState = *(float *)((char *)board + 0x43A0);
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

        /* Native step 3 (0x417C2C): consume the +0x70C flag on the render
         * ctx — device state sync the material loop expects. Skipping it
         * crashed Draw (NULL+0x154 at 0x46F160). */
        {
            DWORD runk = (DWORD)unk;
            if (runk && !IsBadReadPtr((void *)runk, 0x800) &&
                *(BYTE *)(runk + 0x70C)) {
                DWORD dev = *(DWORD *)(runk + 0x154);
                if (dev && !IsBadReadPtr((void *)dev, 0xCC)) {
                    DWORD *dv = *(DWORD **)dev;
                    if (dv && !IsBadReadPtr((void *)dv, 0xD0)) {
                        void (__thiscall *rs)(DWORD, DWORD, DWORD, DWORD) =
                            (void (__thiscall *)(DWORD, DWORD, DWORD, DWORD))dv[50];
                        if (rs) rs(dv, dev, 0x0E, 0);
                    }
                }
                *(DWORD *)(runk + 0x7C8) = *(DWORD *)(runk + 0x7C8) + 1;
                *(BYTE *)(runk + 0x70C) = 0;
            }
        }

        char timerBuf[68];
        if (g_TimerInit && g_TimerCleanup) {
            g_TimerInit(timerBuf);

            /* Smasher 1: scale the FRESH timer matrix (native pattern: Scale/
             * Position always target a per-frame buffer, never the persistent
             * gfx object — scaling gfx accumulates across frames). */
            float transp1 = *(float *)((char *)ext + REND_GLASS_TRANSP1);
            if (g_GfxScaleX) g_GfxScaleX(timerBuf, transp1);
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

            /* Smasher 2: same fresh-buffer pattern (never scale persistent gfx). */
            char timerBuf2[68];
            g_TimerInit(timerBuf2);
            float transp2 = *(float *)((char *)ext + REND_GLASS_TRANSP2);
            if (g_GfxScaleX) g_GfxScaleX(timerBuf2, transp2);
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

        /* Toggle Z-write off around skybox/clouds (native SkyBoard_RenderDynamic
         * 0x410EC5: same 0x70C gate, fn on the gfx+0x154 sub-object's vtable
         * slot 0x32 with (0xe, 0)). Call it on the SUB-OBJECT, not gfx —
         * binary-verified: native loads [gfx+0x154] then its vtable. */
        if (*(char *)((char *)gfx + 0x70C) != 0) {
            DWORD subObj = *(DWORD *)((char *)gfx + 0x154);
            if (subObj && !IsBadReadPtr((void *)subObj, 4)) {
                DWORD *svtbl = *(DWORD **)subObj;
                if (svtbl) {
                    void (__thiscall *fnC8)(DWORD, int, int) =
                        (void (__thiscall *)(DWORD, int, int))svtbl[0x32];
                    if (fnC8) fnC8(subObj, 0xe, 0);
                }
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

        /* Toggle render state back (native 0x410F55: same gate, sub-object
         * vtable slot 0x32 with (0xe, 1)). */
        if (*(char *)((char *)gfx + 0x70C) != 1) {
            DWORD subObj2 = *(DWORD *)((char *)gfx + 0x154);
            if (subObj2 && !IsBadReadPtr((void *)subObj2, 4)) {
                DWORD *svtbl2 = *(DWORD **)subObj2;
                if (svtbl2) {
                    void (__thiscall *fnC8b)(DWORD, int, int) =
                        (void (__thiscall *)(DWORD, int, int))svtbl2[0x32];
                    if (fnC8b) fnC8b(subObj2, 0xe, 1);
                }
            }
            *(char *)((char *)gfx + 0x70C) = 1;
            (*(int *)((char *)gfx + 0x7C8))++;
        }

        /* Adjust projection from gfx field — native SkyBoard_RenderDynamic tail
         * (0x410F8E-0x410FAD, binary-verified): f=((f+f)*1100.0+0x454D4143);
         * SetProjection(gfx, 20.0, f+f). Constants are native ROM floats, not
         * reciprocals. */
        float fovAdjust = *(float *)((char *)gfx + 0x188);
        { DWORD cb = 0x454D4143; float m2; memcpy(&m2, &cb, 4);
          fovAdjust = (fovAdjust + fovAdjust) * 1100.0f + m2; }
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

typedef void (__thiscall *BoardUpdate_t)(void *board); /* native slot-1 orig (ECX=board, no stack args) */

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Board_Update — replaces all 15 per-level Board_Update functions
 *
 * Calls Scene_Update first, then dispatches to feature blocks based on
 * the per-level feature flags in g_updateFeatures[].
 * ═══════════════════════════════════════════════════════════════════════════ */

void __fastcall UniversalBoardUpdate(void *board) {
    if (!g_SceneUpdate || !board) return;

    /* Native-ctor fast path: native-built boards run their NATIVE slot-1
     * handler (orig), which drives level-specific machinery the shared base
     * lacks — e.g. content-10 Toob orig runs object setup the shared
     * base lacks. (Most 10-15 origs ARE the base: no-op.) The 4 other
     * customs call the base internally first (single-chain safe, verified in
     * disasm); the 10 shared levels' orig IS the base. Additive LFL blocks
     * below (proximity + features) still run — ext-driven, S1-based. */
    {
        void *uext = GetBoardExt(board);
        if (uext && *(int *)((char *)uext + UNI_NATIVE_HANDLERS)) {
            int cslot = ContentFileId(uext);
            int effSlot = (cslot >= 10 && cslot <= 15) ? cslot : GetCurrentLevel(board);
            BoardUpdate_t origU = NULL;
            if (effSlot >= 1 && effSlot <= 15) origU = (BoardUpdate_t)g_origUpdate[effSlot];
            if (origU) origU(board);
            else g_SceneUpdate(board);
            goto board_additive;
        }
    }
    /* Universal Toob (native-built): replicate the native update orig's
     * per-object anims (0x41F750: update(1) on non-NULL board+0x4370/74/78/7C,
     * NULL-guarded exactly like native). Runs BEFORE the shared base update
     * (native order: objects, then Scene_Update). The orig's per-frame bumper
     * slot rebuild is skipped on purpose — slots stay ctor-built and lit is
     * ext-driven now, so there is nothing to rebuild. */
    {
        void *uext2 = GetBoardExt(board);
        if (uext2 && ContentFileId(uext2) == 10 && *(int *)((char *)uext2 + UNI_NATIVE_CTOR) &&
            BoardHasOffset(board, 0x4370 + 3 * 4 + 4, 4)) {
            int oi;
            typedef void (__thiscall *ObjUpdate1_t)(void *o, int a);
            for (oi = 0; oi < 4; oi++) {
                void *o = *(void **)((char *)board + 0x4370 + oi * 4);
                if (o && !IsBadReadPtr(o, 4)) {
                    DWORD vt = *(DWORD *)o;
                    if (vt && !IsBadReadPtr((void *)vt, 4)) {
                        ObjUpdate1_t fn = *(ObjUpdate1_t *)vt; /* vtable[0], thiscall (obj, 1) */
                        if (fn) fn(o, 1);
                    }
                }
            }
        }
    }
    g_SceneUpdate(board);

board_additive:
    /* E:BRANCH(B) proximity driver — boards whose dispatcher touches
     * never arrive (native-ctor fast path bypass) still teleport when
     * the ball enters the trigger volume. C3C guard prevents double-fire
     * with the dispatcher path; 0x2CD bit prevents per-frame refire. */
    {
        void *pex = GetBoardExt(board);
        if (pex && *(int *)((char *)pex + OFF_BRANCH_QUAD_VALID) && !*(BYTE *)((char *)board + 0x874)) {
            int bc = *(int *)((char *)board + 0x29D4 + 4);
            int *bi = *(int **)((char *)board + 0x29D4 + 0x40C);
            float *qmin = (float *)((char *)pex + OFF_BRANCH_QUAD_MIN);
            float *qmax = (float *)((char *)pex + OFF_BRANCH_QUAD_MAX);
            if (bc >= 1 && bc <= 4 && bi && !IsBadReadPtr(bi, (unsigned)bc * 4)) {
                int k;
                for (k = 0; k < bc; k++) {
                    int b = bi[k];
                    if (!b || IsBadReadPtr((void *)b, 0xD00)) continue;
                    float bx = *(float *)(b + 0x164), by = *(float *)(b + 0x168), bz = *(float *)(b + 0x16C);
                    int inside = (bx > qmin[0]-40 && bx < qmax[0]+40 &&
                                  by > qmin[1]-40 && by < qmax[1]+40 &&
                                  bz > qmin[2]-40 && bz < qmax[2]+40);
                    int was = (*(BYTE *)(b + 0x2CD)) & 1;
                    if (inside && !was && *(BYTE *)(b + 0xC3C) == 0) {
                        FireBranchTeleport(board, (int *)b, "BRANCH(B)", 1);
                        *(BYTE *)(b + 0x2CD) |= 1;
                    } else if (!inside && was) {
                        *(BYTE *)(b + 0x2CD) &= (BYTE)~1;
                    }
                }
            }
        }
    }

    /* Master hammer triggers (E:CALLHAMMER/E:HAMMERCHASE) proximity driver —
     * S6 trigger-volume touches never arrive post-load (only the load-time
     * initial sweep dispatches), so the touch handlers in lf_06 never run:
     * no bonk popup/chase, no sound, bridge never breaks. Per-frame AABB
     * entry test runs the same native calls instead. 0x2CD bits 1/2/4
     * edge-trigger per ball; STATE bits re-arm on exit. Double-fire with
     * the dispatcher path is benign (popup/chase natives are idempotent). */
    {
        typedef void (__fastcall *CreateBonkPopup_t)(int obj);
        typedef void (__fastcall *Hammer_ChaseStart_t)(int obj);
        typedef void (__thiscall *BreakBridge_Break_t)(void *obj, int dir);
        void *pex = GetBoardExt(board);
        if (pex && (*(int *)((char *)pex + OFF_HAMMER_TRIG_VALID)) && !*(BYTE *)((char *)board + 0x874)) {
            DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
            int difficulty = (app && !IsBadReadPtr((void *)app, 0x800)) ? *(int *)(app + APP_DIFFICULTY) : 0;
            if (difficulty != 0) {
                int bc = *(int *)((char *)board + 0x29D4 + 4);
                int *bi = *(int **)((char *)board + 0x29D4 + 0x40C);
                if (bc >= 1 && bc <= 4 && bi && !IsBadReadPtr(bi, (unsigned)bc * 4)) {
                    int k;
                    for (k = 0; k < bc; k++) {
                        int b = bi[k];
                        if (!b || IsBadReadPtr((void *)b, 0xD00)) continue;
                        float bx = *(float *)(b + 0x164), by = *(float *)(b + 0x168), bz = *(float *)(b + 0x16C);
                        int st = *(DWORD *)((char *)pex + OFF_HAMMER_TRIG_STATE);
                        /* E:CALLHAMMER: bit0 valid, bit0 armed, 0x2CD bit1 edge */
                        if ((*(int *)((char *)pex + OFF_HAMMER_TRIG_VALID)) & 1) {
                            float *qmin = (float *)((char *)pex + OFF_CALLHAMMER_MIN);
                            float *qmax = (float *)((char *)pex + OFF_CALLHAMMER_MAX);
                            int inside = (bx > qmin[0]-40 && bx < qmax[0]+40 &&
                                          by > qmin[1]-40 && by < qmax[1]+40 &&
                                          bz > qmin[2]-40 && bz < qmax[2]+40);
                            int was = (*(BYTE *)(b + 0x2CD)) & 2;
                            if (inside && !was && (st & 1)) {
                                int bonkObj = BoardHasOffset(board, 0x540C, 4) ? *(int *)((char *)board + 0x540C) : 0;
                                if (!bonkObj) bonkObj = *(int *)((char *)pex + UNI_BONK_MESH);
                                if (bonkObj && !IsBadReadPtr((void *)bonkObj, 0x1108))
                                    ((CreateBonkPopup_t)(g_moduleBase + RVA_CreateBonkPopup))(bonkObj);
                                *(DWORD *)((char *)pex + OFF_HAMMER_TRIG_STATE) &= (DWORD)~1;
                                *(BYTE *)(b + 0x2CD) |= 2;
                            } else if (!inside && was) {
                                *(BYTE *)(b + 0x2CD) &= (BYTE)~2;
                                *(DWORD *)((char *)pex + OFF_HAMMER_TRIG_STATE) |= 1;
                            }
                        }
                        /* E:HAMMERCHASE (2 quads): bit1/bit2 valid+armed, 0x2CD bit2 edge */
                        {
                            int qi;
                            for (qi = 0; qi < 2; qi++) {
                                DWORD vbit = (DWORD)(2 << qi);
                                DWORD minOff = (qi == 0) ? OFF_HAMMERCHASE_MIN : OFF_HAMMERCHASE2_MIN;
                                DWORD maxOff = (qi == 0) ? OFF_HAMMERCHASE_MAX : OFF_HAMMERCHASE2_MAX;
                                if (!((*(int *)((char *)pex + OFF_HAMMER_TRIG_VALID)) & (int)vbit)) continue;
                                float *qmin = (float *)((char *)pex + minOff);
                                float *qmax = (float *)((char *)pex + maxOff);
                                int inside = (bx > qmin[0]-40 && bx < qmax[0]+40 &&
                                              by > qmin[1]-40 && by < qmax[1]+40 &&
                                              bz > qmin[2]-40 && bz < qmax[2]+40);
                                int was = (*(BYTE *)(b + 0x2CD)) & 4;
                                st = *(DWORD *)((char *)pex + OFF_HAMMER_TRIG_STATE);
                                if (inside && !was && (st & (int)vbit)) {
                                    int b2 = BoardHasOffset(board, 0x436C, 4) ? *(int *)((char *)board + 0x436C) : 0;
                                    if (!b2) b2 = *(int *)((char *)pex + UNI_BONK_MESH);
                                    if (!b2) b2 = *(int *)((char *)pex + UNI_BONK_STORE);
                                    if (b2 && !IsBadReadPtr((void *)b2, 0x1108))
                                        ((Hammer_ChaseStart_t)(g_moduleBase + RVA_Hammer_ChaseStart))(b2);
                                    *(DWORD *)((char *)pex + OFF_HAMMER_TRIG_STATE) &= (DWORD)~vbit;
                                    *(BYTE *)(b + 0x2CD) |= 4;
                                } else if (!inside && was) {
                                    *(BYTE *)(b + 0x2CD) &= (BYTE)~4;
                                    *(DWORD *)((char *)pex + OFF_HAMMER_TRIG_STATE) |= vbit;
                                }
                            }
                        }
                        /* Stage 2: break the halves once the popup is up.
                         * The BONKPOPUP touch never arrives (same dead
                         * volume-touch path), so when the popup is active
                         * (bonk +0x10FC==0) and the ball reaches the bonk,
                         * run the native break directly (idempotent: halves
                         * latch +0x10FC, skip once broken). */
                        {
                            int bonkObj = BoardHasOffset(board, 0x540C, 4) ? *(int *)((char *)board + 0x540C) : 0;
                            if (!bonkObj) bonkObj = *(int *)((char *)pex + UNI_BONK_MESH);
                            if (bonkObj && !IsBadReadPtr((void *)bonkObj, 0x1108) && *(int *)(bonkObj + 0x10FC) == 0) {
                                float hx = *(float *)(bonkObj + 0x10D4), hy = *(float *)(bonkObj + 0x10D8), hz = *(float *)(bonkObj + 0x10DC);
                                float ddx = bx-hx, ddy = by-hy, ddz = bz-hz;
                                if (ddx*ddx + ddy*ddy + ddz*ddz < 100.0f*100.0f) {
                                    int h1 = BoardHasOffset(board, 0x5418, 4) ? *(int *)((char *)board + 0x5418) : 0;
                                    int h2 = BoardHasOffset(board, 0x541C, 4) ? *(int *)((char *)board + 0x541C) : 0;
                                    if (!h1) h1 = *(int *)((char *)pex + UNI_BBRIDGE1_OBJ);
                                    if (!h2) h2 = *(int *)((char *)pex + UNI_BBRIDGE2_OBJ);
                                    if (h1 && !IsBadReadPtr((void *)h1, 0x1100) && *(int *)(h1 + 0x10FC) == 0)
                                        ((BreakBridge_Break_t)(g_moduleBase + RVA_BreakBridge_break))((void *)h1, 0);
                                    if (h2 && !IsBadReadPtr((void *)h2, 0x1100) && *(int *)(h2 + 0x10FC) == 0)
                                        ((BreakBridge_Break_t)(g_moduleBase + RVA_BreakBridge_break))((void *)h2, 1);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* Get level and dispatch features */
    int level = GetCurrentLevel(board);
    if (level == 0) return;
    // Pause already handled per-feature, but gate here too
    if (*(BYTE*)((char*)board+0x874)) return;
    DWORD extFeat = GetBoardFeat(board);
    DWORD features = extFeat; // Option B: pure S1-driven, no g_updateFeatures fallback
    if (!features) return;

    static int featDbg = 0;
    /* Bridge animation (presence-routed: runs wherever S1Inventory found
     * drawbridge kinds — mirrors own them on every race now) */
    if (features & FEAT_BRIDGE_ANIM) {
        if (featDbg < 3) DebugLog("  [update] Feature_BridgeAnimation...");
        Feature_BridgeAnimation(board, level);
        if (featDbg < 3) DebugLog("  [update] Feature_BridgeAnimation done");
    }

    /* NOTE 2026-11-19: the old per-frame ball-near-Bonk proximity break
     * (Feature_BridgeBreak) is retired — it shattered the halves early.
     * Breaks now fire only on the native BONKPOPUP touch (lf_06), which is
     * the exact native timing (Master 0x412f80). */

    /* Tipper visuals (presence-routed: runs wherever S1Inventory found
     * TIPPER kinds — mirrors own them on every race now) */
    if (features & FEAT_TIPPER_VIS) {
        Feature_TipperVisual(board, level);
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
        if (rsext && *(int *)((char *)rsext + UNI_NATIVE_HANDLERS)) {
            int cslot = ContentFileId(rsext);
            int effSlot = (cslot >= 10 && cslot <= 15) ? cslot : level;
            if (effSlot >= 1 && effSlot <= 15 && g_origRaceState[effSlot]) {
                g_origRaceState[effSlot](board);
                goto racestate_features;
            }
        }
    }
    if (level == 2 || ContentFileId(GetBoardExt(board)) == 2) {
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

    } else if (level == 13 || ContentFileId(GetBoardExt(board)) == 13 ||
               (board && !IsBadReadPtr(board, (void*)4) && *(DWORD *)board == 0x4D0FC8)) {
        /* Sky RaceState (0x41FC90) drives native popcylinders (board+0x47B0)
         * + native magnifier-follow (board+0x47AC). LFL builds no Sky
         * objects (Sky uses custom S1 loop 0x410900, bypassing LFL's
         * per-name builders), so skipping orig starves them (pegs never
         * pop, magnifier never follows). Call native directly by RVA
         * (bypasses orig-array indexing — always exact for Sky-class). */
        {
            typedef void (__thiscall *SkyRS_t)(void *board);
            SkyRS_t skyRS = (SkyRS_t)(g_moduleBase + 0x1FC90);
            if (skyRS) skyRS(board);
            else if (g_origRaceState[level]) g_origRaceState[level](board);
            else g_BoardUpdateRaceState(board);
        }
    } else if (level >= 1 && level <= 15 && g_origRaceState[level]) {
        /* v13.29: slot-native orig runs ONLY when the loaded file IS the
         * slot's own (content == level — objects it expects exist, proven
         * safe). A foreign file gets base + our own drivers (universal
         * pump) instead — slot-native on a foreign file walks/touches
         * objects that aren't there. Tower content: orig IS the shared
         * base handler; call base directly (verified identical). */
        if (ContentFileId(GetBoardExt(board)) == 5) g_BoardUpdateRaceState(board);
        else if (ContentFileId(GetBoardExt(board)) == level) g_origRaceState[level](board);
        else g_BoardUpdateRaceState(board);
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
