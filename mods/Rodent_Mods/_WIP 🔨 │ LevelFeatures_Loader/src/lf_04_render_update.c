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
void ImpDiag(const char *msg);
/* Sky pre-base box (native order 0x410E80: BOX then Level_RenderDynamicObjects).
 * Native draws the far-projection + cloud sprite FIRST (Z off), then the level
 * overdraws it. Running it after the base paints the Z-less sprite OVER the
 * track (reads as near fog/haze). Sky runs this before the base call. */
static void SkyBoxDraw(void *board, void *ext)
{
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
                if (svtbl && !IsBadReadPtr(svtbl, 0xD0)) {
                    /* NATIVE SHAPE (0x410EDB-0x410EDC): pushes (subObj, 0xE,
                     * 0), ECX=app (NOT this) + callee cleans 12 = __stdcall.
                     * __thiscall passes this in ECX (ignored) so the callee
                     * reads 0xE as the device -> d3d8 AV (the Sky Draw
                     * crash). Match native exactly: stdcall, this on stack. */
                    void (__stdcall *fnC8)(DWORD, int, int) =
                        (void (__stdcall *)(DWORD, int, int))svtbl[0x32];
                    if (fnC8) fnC8(subObj, 0xe, 0);
                }
            }
            *(char *)((char *)gfx + 0x70C) = 0;
            (*(int *)((char *)gfx + 0x7C8))++;
        }

        /* Disable culling for cloud sprite */
        if (g_GraphicsSetCullMode2) g_GraphicsSetCullMode2(gfx, 0, 1);

        /* Render cloud sprite quad from dedicated REND_SKY_SPRITE.
         * Native parity (0x410F0F-0x410F31): sprite object is the board
         * slot, position comes from BOARD+0x4378/0x437C/0x4380 (filled by
         * the CLOUDSCAPE create branch), NOT ext (which nothing fills —
         * heap garbage -> NaN into D3D -> d3d8 crash, the Sky Draw fault). */
        if (g_SpriteRenderQuad) {
            DWORD sprite = *(DWORD *)((char *)ext + REND_SKY_SPRITE);
            float sp1 = *(float *)((char *)board + 0x4378);
            float sp2 = *(float *)((char *)board + 0x437C);
            float sp3 = *(float *)((char *)board + 0x4380);
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
                if (svtbl2 && !IsBadReadPtr(svtbl2, 0xD0)) {
                    /* Same stdcall shape as the disable call above. */
                    void (__stdcall *fnC8b)(DWORD, int, int) =
                        (void (__stdcall *)(DWORD, int, int))svtbl2[0x32];
                    if (fnC8b) fnC8b(subObj2, 0xe, 1);
                }
            }
            *(char *)((char *)gfx + 0x70C) = 1;
            (*(int *)((char *)gfx + 0x7C8))++;
        }

        /* Adjust projection from gfx field — native SkyBoard_RenderDynamic tail
         * (0x410F8E-0x410FAD, binary-verified): f=((f+f)*1100.0+1900.0);
         * SetProjection(gfx, 20.0, f+f). Constants are native ROM floats. */
        float fovAdjust = *(float *)((char *)gfx + 0x188);
        /* Native 0x410F8E-0x410FAD: f=((f+f)*[0x4CF6FC]+[0x4CF6F8]) doubled.
         * ROM floats: [0x4CF6FC]=1100.0, [0x4CF6F8]=1900.0 ( NOT 3284.08 —
         * 0x454D4143 was a misread address). */
        fovAdjust = (fovAdjust + fovAdjust) * 1100.0f + 1900.0f;
        if (g_GraphicsSetProjection)
            g_GraphicsSetProjection(gfx, 20.0f, fovAdjust + fovAdjust);
}

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
    if (ext && *(int *)((char *)ext + UNI_NATIVE_HANDLERS) &&
        !*(int *)((char *)ext + UNI_FORCE_UNI_RENDER)) {
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

    /* Glass slot-24: delegate to native 0x417C00 via orig[12]. It does base +
     * the full multi-buffer pane-timer sequence (3+ bufs, early v2, dual
     * dev-v7 per pane). The single-buffer replica was structurally wrong and
     * each partial fix risked wrong draws. DAT is hook-filled so native
     * inputs match vanilla exactly. Stock slot+content 12 only; swapped
     * slots keep the replica path below (guarded, no crash). */
    if (ext && level == 12 && ContentFileId(ext) == 12 &&
        *(BYTE *)((char *)ext + COLL_FLAG_GLASS) && g_origRender[12]) {
        g_origRender[12](board, unk);
        return;
    }

    /* Sky native order: box BEFORE the level (sprite overdrawn correctly).
     * The shared flow runs base first; Sky pre-runs its box here, then the
     * post-base block skips via skyPreRan. Only Sky sets REND_SKY_SPRITE. */
    int skyPreRan = 0;
    if (ext && *(DWORD *)((char *)ext + REND_SKY_SPRITE) != 0) {
        SkyBoxDraw(board, ext);
        skyPreRan = 1;
    }

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

    { static int skyRCount = 0; if (ContentFileId(ext) == 13 && (skyRCount == 0 || (++skyRCount % 600) == 0)) { char srd[128]; if (skyRCount == 0) skyRCount = 1; wsprintfA(srd, "SKYRENDER features=0x%X sprite=0x%08X list=%d mesh=0x%08X", features, *(DWORD *)((char *)ext + REND_SKY_SPRITE), *(int *)((char *)ext + REND_SKY_LIST + 4), *(DWORD *)((char *)ext + REND_SKY_MESH)); SkyDiag(srd); } }

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
        /* Faithful Tower render (native 0x40DFA0). The 68B timer T is a
         * composite: Gfx fns take SUB-OBJECT this-pointers, not T+0 —
         * rotations + draw matrix use T+4, wave uses T+8, scale/position
         * use T+12 (objdump: lea 0x18/0x1c/0x20(%esp) off the T at +0x14).
         * Passing T+0 everywhere misplaces every compose = upper jaw lost.
         * State floats come from ext mirrors (animated by update features);
         * object pointers/positions from native board slots. */

        char timerBuf[68];
        if (g_TimerInit && g_TimerCleanup && g_GfxScaleZ && g_GfxScaleX &&
            g_GfxSetPosition && g_WaveSin && g_Matrix44Zero) {
            g_TimerInit(timerBuf);
            void *rotM = (void *)(timerBuf + 4);   /* rotation/draw matrix */
            void *wavC = (void *)(timerBuf + 8);   /* wave component */
            void *posC = (void *)(timerBuf + 12);  /* scale/pos component */
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

            /* Throttled render-draw log (proves the draws fire + with what).
             * One line per ~600 frames to chomper_diag.txt: REND prefix. */
            {
                static int rn = 0;
                if (++rn >= 600 && chomperObj) {
                    rn = 0;
                    HANDLE rhf = CreateFileA("chomper_diag.txt", GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL);
                    if (rhf != INVALID_HANDLE_VALUE) {
                        char rl[256]; DWORD rbw = 0;
                        DWORD vt0 = IsBadReadPtr((void *)chomperObj, 4) ? 0 : *(DWORD *)chomperObj;
                        DWORD v7 = 0;
                        if (vt0 && !IsBadReadPtr((void *)vt0, 32)) v7 = ((DWORD *)vt0)[7];
                        float *m4 = (float *)rotM;
                        wsprintfA(rl, "REND obj=%d v7=0x%X jaw=%d px=%d py=%d pz=%d m0=%d m1=%d m2=%d m3=%d\r\n",
                            (int)chomperObj, (unsigned)v7,
                            (int)(chompState * 100.0f), (int)(px * 100.0f),
                            (int)(py * 100.0f), (int)(pz * 100.0f),
                            (int)(m4[0] * 1000.0f), (int)(m4[1] * 1000.0f),
                            (int)(m4[2] * 1000.0f), (int)(m4[3] * 1000.0f));
                        SetFilePointer(rhf, 0, NULL, FILE_END);
                        WriteFile(rhf, rl, strlen(rl), &rbw, NULL);
                        CloseHandle(rhf);
                    }
                }
            }

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
            /* Pass 2: reset the matrix (native 0x40E094), then rebuild:
             * ScaleX(180), offset jaw, same draws. */
            g_Matrix44Zero((int *)timerBuf);
            if (scaleFn) scaleFn(timerBuf, 1.15f, 1.15f, 1.15f);
            if (g_GfxScaleZ) g_GfxScaleZ(timerBuf, -chompState);
            if (g_GfxScaleX) g_GfxScaleX(timerBuf, 180.0f);
            if (g_GfxSetPosition) g_GfxSetPosition(timerBuf, -35.0f, 0.0f, 0.0f);
            if (posFn) posFn(timerBuf, px, py, pz);
            if (g_GfxSetPosition) g_GfxSetPosition(timerBuf, 0.0f, liftY, 0.0f);
            if (g_GfxSetPosition && g_WaveSin)
                g_GfxSetPosition(timerBuf, 0.0f, g_WaveSin((void *)0x4F7188, chompTimer) * 10.0f, 0.0f);
            /* Pass-2 draw with culling OFF for the 180-mirrored copy:
             * mirrored winding is backface-culled under LFL's phase
             * states (native draws with culling effectively off here). */
            {
                void *pgfx = NULL;
                DWORD papp = *(DWORD *)((char *)board + BOARD_APP_PTR);
                if (papp && !IsBadReadPtr((void *)papp, 0x600))
                    pgfx = *(void **)((char *)papp + 0x174);
                if (pgfx && g_GraphicsSetCullMode2) g_GraphicsSetCullMode2(pgfx, 0, 1);
                if (chomperObj && !IsBadReadPtr((void *)chomperObj, 4)) {
                    DWORD *cvtbl = *(DWORD **)chomperObj;
                    if (cvtbl && !IsBadReadPtr(cvtbl, 32) && cvtbl[7]) {
                        void (__thiscall *chompRender)(void *, void *, int) =
                            (void (__thiscall *)(void *, void *, int))cvtbl[7];
                        chompRender((void *)chomperObj, timerBuf, 0);
                    }
                }
                if (pgfx && g_GraphicsSetCullMode2) g_GraphicsSetCullMode2(pgfx, 1, 1);
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

        /* Render-gap fix (2026-11-20): native timer sequence replica. The
         * block's old Timer Scale/Position API corrupted Draw state; native
         * 0x417C00 uses 0x457AD0/0x457C60 + product-pos + device-select.
         * Per smasher: AD0(bufA); C60(bufA,float); v[2](bufA,x,y,z);
         * dev-v[7](dev,bufA,0). (bufB==bufA: native reuses one buffer.)
         * STABLE BASELINE (no reflection yet — hunt continues from here). */
        {
            typedef void (__thiscall *T57AD0)(void *);
            typedef DWORD (__thiscall *T57C60)(void *, float);
            T57AD0 fAD0 = (T57AD0)(g_moduleBase + 0x57AD0);
            T57C60 fC60 = (T57C60)(g_moduleBase + 0x57C60);
            /* Native 0x417C00 does Timer_Init(buf) BEFORE AD0 (Ghidra-verified):
             * Timer_Init writes ONLY buf[0] = timer vtable 0x4CF338. AD0 never
             * writes [0], so without init buf[0] is stack garbage and the
             * vtable-driven v[2]/dev-v[7] below guard out — pane finish never
             * draws (panes visible via base, no reflection). */
            typedef void (__thiscall *T57A40)(void *);
            T57A40 fInit = (T57A40)(g_moduleBase + 0x57A40);
            int i;
            for (i = 0; i < 2; i++) {
                float px = *(float *)((char *)board + (i ? 0x4378 : 0x436C));
                float py = *(float *)((char *)board + (i ? 0x437C : 0x4370));
                float pz = *(float *)((char *)board + (i ? 0x4380 : 0x4374));
                float pr = *(float *)((char *)board + (i ? 0x4388 : 0x4384));
                BYTE fg = *(BYTE *)((char *)board + (i ? 0x438D : 0x438C));
                char bufA[68];
                DWORD prod = 0;
                if (fAD0 && fC60) {
                    /* Native 0x417C00 issues dev-v[7] BEFORE Timer_Init
                     * (Ghidra+objdump verified: 0x417CE4 precedes the 0x457AD0
                     * call): BeginFrame+delegate primes the device, then init
                     * fills the buffer, C60 scales, v[2] positions, and the
                     * second dev-v[7] renders. Skipping the first call left
                     * the finish with no active frame (invisible). */
                    {
                        DWORD dev0 = *(DWORD *)(app + (fg ? 0x588 : 0x584));
                        if (dev0 && !IsBadReadPtr((void *)dev0, 0x20)) {
                            DWORD *dv0 = *(DWORD **)dev0;
                            if (dv0 && !IsBadReadPtr((void *)dv0, 0x20)) {
                                void (__thiscall *d70)(DWORD, DWORD, DWORD) =
                                    (void (__thiscall *)(DWORD, DWORD, DWORD))dv0[7];
                                if (d70) d70(dev0, (DWORD)bufA, 0);
                            }
                        }
                    }
                    if (fInit) fInit(bufA);
                    fAD0(bufA);
                    prod = fC60(bufA, pr);
                }
                if (!IsBadReadPtr(bufA, 0x10)) {
                    DWORD *pv = *(DWORD **)bufA;
                    if (pv && !IsBadReadPtr((void *)pv, 0x10)) {
                        void (__thiscall *p2)(DWORD, float, float, float) =
                            (void (__thiscall *)(DWORD, float, float, float))pv[2];
                        if (p2) p2((DWORD)bufA, px, py, pz);
                    }
                }
                {
                    DWORD dev = *(DWORD *)(app + (fg ? 0x588 : 0x584));
                    if (dev && !IsBadReadPtr((void *)dev, 0x20)) {
                        DWORD *dv = *(DWORD **)dev;
                        if (dv && !IsBadReadPtr((void *)dv, 0x20)) {
                            void (__thiscall *d7)(DWORD, DWORD, DWORD) =
                                (void (__thiscall *)(DWORD, DWORD, DWORD))dv[7];
                            if (d7) d7(dev, (DWORD)bufA, 0);
                        }
                    }
                }
                (void)prod;
            }
        }
        /* Native 0x417C00 tail (Ghidra-verified, was missing): after the pane
         * timer draws it restores RS(0x0E)=1 + 0x70C=1 (+0x7C8 tick). Without
         * this, RS(0x0E)=0 from the head persists into subsequent frames and
         * kills alpha blending for the fin draws (opaque fins, no glass). */
        {
            DWORD runk = (DWORD)unk;
            if (runk && !IsBadReadPtr((void *)runk, 0x800) &&
                *(BYTE *)(runk + 0x70C) != 1) {
                DWORD dev = *(DWORD *)(runk + 0x154);
                if (dev && !IsBadReadPtr((void *)dev, 0xCC)) {
                    DWORD *dv = *(DWORD **)dev;
                    if (dv && !IsBadReadPtr((void *)dv, 0xD0)) {
                        void (__thiscall *rs)(DWORD, DWORD, DWORD, DWORD) =
                            (void (__thiscall *)(DWORD, DWORD, DWORD, DWORD))dv[50];
                        if (rs) rs(dv, dev, 0x0E, 1);
                    }
                }
                *(BYTE *)(runk + 0x70C) = 1;
                *(DWORD *)(runk + 0x7C8) = *(DWORD *)(runk + 0x7C8) + 1;
            }
        }
    }

    sky_render:

    /* ── Sky Box (CLOUDSCAPE skybox) — far-clip + CLOUDSCAPE cloud sprite (points 1-3) ──
     * Only active if CLOUDSCAPE S1 / REND_SKY_SPRITE exists. */
    if ((features & REND_SKY_BOX) && !skyPreRan) {
        SkyBoxDraw(board, ext);
    }
    /* ── Sky Pillars (native render tail 0x410FBE-0x41107C, binary-verified) ──
     * Renders the skypillar mesh (board+0x436C) at each PILLAR S1 position
     * (g_skyPillarPos filled by scene-tails walk — the native +0x4394 hook
     * list can't be used: Setup re-inits it and 0x453810 merges sublists,
     * not items). Per pillar: Timer_Init, Timer-v[0x8] pose, mesh-v[0x1C]
     * render, Timer_Cleanup. Both v-calls are callee-clean (ret 12 / ret 8)
     * — hand-rolled asm, MinGW __thiscall caller-cleans and would corrupt
     * ESP (verified by objdump of thiscall shape). */
    if (features & REND_SKY_BOX) {
        extern float g_skyPillarPos[128 * 3];
        extern int g_skyPillarCount;
        DWORD pmesh = *(DWORD *)((char *)board + 0x436C);
        { static int pgOnce = 0; if (!pgOnce) { pgOnce = 1;
          char pg[96]; wsprintfA(pg, "SKY-PILLARS render n=%d mesh=0x%X",
            g_skyPillarCount, pmesh);
          SkyDiag(pg);
          { char av[128]; wsprintfA(av, "SKY-ARR0 xyz=%d,%d,%d",
            (int)g_skyPillarPos[0], (int)g_skyPillarPos[1], (int)g_skyPillarPos[2]);
            SkyDiag(av);
} } }
        if (pmesh && !IsBadReadPtr((void *)pmesh, 8) &&
            g_skyPillarCount > 0 && g_skyPillarCount <= 128) {
                DWORD *mVtbl = *(DWORD **)pmesh;
                if (mVtbl && !IsBadReadPtr(mVtbl, 0x20)) {
                    void *fnRender = (void *)mVtbl[7];
                    if (fnRender) {
                        int pi;
                        /* positions come from g_skyPillarPos (scene-tails walk);
                         * no native-list dependency. */
                        for (pi = 0; pi < g_skyPillarCount; pi++) {
/* Distance cull (RE 2026-11-22): native renders all 52 but far ones
                             * hide via fog/distance (7 visible). Render near ones
                             * (dist<500 to pad) at native pose height (surface, flush);
                             * skip far floaters (would crowd the track line). */
                            char timerBuf[68];
                            DWORD *tVtbl;
                            void *fnPose;
                            DWORD px, py, pz;
                            px = *(DWORD *)&g_skyPillarPos[pi * 3 + 0];
                            py = *(DWORD *)&g_skyPillarPos[pi * 3 + 1];
                            pz = *(DWORD *)&g_skyPillarPos[pi * 3 + 2];
/* Full 52, no cull (per user 2026-11-22). */

if (!g_TimerInit || !g_TimerCleanup) break;
                            g_TimerInit(timerBuf);
                            tVtbl = *(DWORD **)timerBuf;
                            if (!tVtbl || IsBadReadPtr(tVtbl, 12)) {
                                                        g_TimerCleanup(timerBuf);
                                continue;
                            }
                            fnPose = (void *)tVtbl[2];
                            if (!fnPose) {
                                g_TimerCleanup(timerBuf);
                                continue;
                            }
                            __asm__ volatile (
                                "pushl %[z]\n\t"
                                "pushl %[y]\n\t"
                                "pushl %[x]\n\t"
                                "movl %[t], %%ecx\n\t"
                                "call *%[f]\n\t"
                                : : [x] "m" (px), [y] "m" (py), [z] "m" (pz),
                                    [t] "r" (timerBuf), [f] "r" (fnPose)
                                : "eax", "ecx", "edx", "memory");
/* File XYZ (user: Y based on X is wrong). X=fileX (m12 fix),
                             * Y=fileY (m13, exact S1 height, not fileX),
                             * Z=fileZ (m14, pose). */
                            { float *mm = (float *)timerBuf;
                              mm[13] = *(float *)&px;
                              mm[14] = *(float *)&py; }
                            

                            __asm__ volatile (
                                "pushl $0\n\t"
                                "pushl %[t]\n\t"
                                "movl %[m], %%ecx\n\t"
                                "call *%[f]\n\t"
                                : : [t] "r" (timerBuf), [m] "r" (pmesh),
                                    [f] "r" (fnRender)
                                : "eax", "ecx", "edx", "memory");
                            g_TimerCleanup(timerBuf);
                        }
                    }
                }
        }
    }
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
                                /* Board slots ONLY (0x436C else 0x540C) —
                                 * mesh fallbacks are never bonk objects. */
                                int bonkObj = BoardHasOffset(board, 0x540C, 4) ? *(int *)((char *)board + 0x540C) : 0;
                                if (!bonkObj && BoardHasOffset(board, 0x436C, 4)) bonkObj = *(int *)((char *)board + 0x436C);
                                if (bonkObj) ((CreateBonkPopup_t)(g_moduleBase + RVA_CreateBonkPopup))(bonkObj);
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
                                    /* Board slots ONLY (0x436C else 0x540C). */
                                    int b2 = BoardHasOffset(board, 0x436C, 4) ? *(int *)((char *)board + 0x436C) : 0;
                                    if (!b2 && BoardHasOffset(board, 0x540C, 4)) b2 = *(int *)((char *)board + 0x540C);
                                    if (b2) ((Hammer_ChaseStart_t)(g_moduleBase + RVA_Hammer_ChaseStart))(b2);
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

    /* GEARPOS diag (v13.39): loaded-vs-supposed positions for Impossible gears.
     * Spawn (ImpDiag SPAWN lines) matches the file; gears render misplaced/
     * misoriented on real Windows. Dump each gear obj's runtime anchor +
     * axle + visual pos at ~3s and ~10s after load so S1 coords compare in
     * one file. Join key = obj addr (also in SPAWN lines). wsprintfA has no
     * float support: centi-units via (int)(x*100). Tick only advances on
     * frames where a gear exists (menu frames don't consume the dump). */
    {
        static DWORD gpBoard = 0;
        static int gpTick = 0, gpD1 = 0, gpD2 = 0;
        int gi, gfound = 0;
        int gcnt = 0; int *gitems = NULL;
        if ((DWORD)board != gpBoard) { gpBoard = (DWORD)board; gpTick = 0; gpD1 = 0; gpD2 = 0; }
        gcnt = *(int *)((char *)board + 0x2578 + 4);
        gitems = *(int **)((char *)board + 0x2578 + 0x40C);
        if (gcnt > 0 && gcnt <= 128 && gitems && !IsBadReadPtr(gitems, (unsigned)gcnt * 4)) {
            for (gi = 0; gi < gcnt; gi++) {
                int go = gitems[gi];
                if (!go || IsBadReadPtr((void *)go, 4)) continue;
                if (*(DWORD *)go == 0x4D5AD0) { gfound = 1; break; }
            }
        }
        if (gfound) gpTick++;
        if (gfound && ((gpTick == 180 && !gpD1) || (gpTick == 600 && !gpD2))) {
            char hb[96];
            if (gpTick == 180) gpD1 = 1; else gpD2 = 1;
            wsprintfA(hb, "GEARPOS begin tick=%d", gpTick); ImpDiag(hb);
            for (gi = 0; gi < gcnt; gi++) {
                int go = gitems[gi];
                char ib[192];
                int ax, ay, az, ex, ey, ez, spd, vx, vy, vz, vv;
                int vis;
                if (!go || IsBadReadPtr((void *)go, 0x1514)) continue;
                if (*(DWORD *)go != 0x4D5AD0) continue;
                ax = (int)(*(float *)(go + 0x10D8) * 100.0f);
                ay = (int)(*(float *)(go + 0x10DC) * 100.0f);
                az = (int)(*(float *)(go + 0x10E0) * 100.0f);
                ex = (int)(*(float *)(go + 0x10E4) * 100.0f);
                ey = (int)(*(float *)(go + 0x10E8) * 100.0f);
                ez = (int)(*(float *)(go + 0x10EC) * 100.0f);
                spd = (int)(*(float *)(go + 0x10F4) * 100.0f);
                vis = *(int *)(go + 0x10D4);
                vx = 0; vy = 0; vz = 0; vv = 0;
                if (vis && !IsBadReadPtr((void *)vis, 0x1100)) {
                    vv = *(int *)vis;
                    vx = (int)(*(float *)(vis + 0x10D8) * 100.0f);
                    vy = (int)(*(float *)(vis + 0x10DC) * 100.0f);
                    vz = (int)(*(float *)(vis + 0x10E0) * 100.0f);
                }
                wsprintfA(ib, "GEARPOS obj=0x%X anch=(%d,%d,%d) axle=(%d,%d,%d) spd=%d vis=0x%X(%X) vpos=(%d,%d,%d)",
                    go, ax, ay, az, ex, ey, ez, spd, vis, vv, vx, vy, vz);
                ImpDiag(ib);
            }
            ImpDiag("GEARPOS end");
        }
    }

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
    { void *sdx = GetBoardExt(board); if (sdx && ContentFileId(sdx) == 13 && (s_rsCount == 1 || s_rsCount % 600 == 0)) { char sd[96]; wsprintfA(sd, "SKYRS tick #%d board=0x%08X", s_rsCount, (DWORD)board); SkyDiag(sd); } }

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
        if (rsext && *(int *)((char *)rsext + UNI_NATIVE_HANDLERS) &&
            !*(int *)((char *)rsext + UNI_FORCE_UNI_RACE)) {
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
        /* 2026-11-20: Sky runs the universal path (UniversalSkyInit +
         * S1 mirrors). Base drives the shared scene; Feature_SkyPopcylinder
         * (racestate_features below) drives the ext popcylinder bank +
         * magnifier. Native 0x41FC90 no longer runs. */
        g_BoardUpdateRaceState(board);
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
