/* Chapter 5 - S1Ensure helpers + UniversalCreateDynamicObjects (LevelFeatures.c lines 3418-4417) */
/* ═══════════════════════════════════════════════════════════════════════════
 /* Universal CreateDynamicObjects (Slot 33) — replaces all 15 per-level handlers
  *
  * Matches S1 object names and calls the appropriate ctor directly
  * (no delegation to originals — the originals only know their own level's
  * names). This enables cross-level object injection: any object from any
  * level can be used on any other level, driven by the loaded file's S1 refs.
  * ═══════════════════════════════════════════════════════════════════════════ */

/* Board app pointer with fallback: native per-level ctors leave board+0x878
 * as 0 at runtime, so create-time loaders use the app captured in Logic. */
static DWORD GetBoardApp(void *board) {
    extern int g_lastAppParam;
    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app) app = (DWORD)g_lastAppParam;
    return app;
}

/* Board object-slot validation. IsBadReadPtr alone is unreliable: malloc
 * garbage that happens to be mapped passes it (Tower +0x4378 = 0x03000000
 * hung Stands_ctor in a 4-billion-iteration clone loop, 2026-10-22).
 * Real game objects have a vtable in the exe image; garbage does not.
 * Tower content (no native ctor) never trusts board slots — ext owns them. */
static int BoardMeshOrZero(void *board, DWORD slotOff) {
    int v = *(int *)((char *)board + slotOff);
    if (ContentFileId(GetBoardExt(board)) == 5) return 0;
    if (!v || IsBadReadPtr((void *)v, 4)) return 0;
    { int vt = *(int *)v;
      if (vt < 0x400000 || vt >= 0x500000) return 0; }
    return v;
}

static void* S1EnsureMeshWorld(void* board, void* ext, DWORD offset, const char* path) {
    void* cur = *(void**)((char*)ext + offset);
    if (cur) return cur;
    DWORD app = GetBoardApp(board);
    if (!app || IsBadReadPtr((void*)app, 0x200)) return NULL;
    void* gfx = *(void**)((char*)app + 0x174);
    if (!gfx || !g_operatorNew || !g_LevelMeshWorldCtor) return NULL;
    void* mem = g_operatorNew(0x10D0);
    if (!mem) return NULL;
    void* mw = g_LevelMeshWorldCtor(mem, gfx, path);
    if (mw) *(void**)((char*)ext + offset) = mw;
    return mw;
}
static void* S1EnsureRender(void* board, void* ext, DWORD renderOff, DWORD meshOff) {
    void* cur = *(void**)((char*)ext + renderOff);
    if (cur) return cur;
    void* mesh = *(void**)((char*)ext + meshOff);
    if (!mesh || !g_operatorNew || !g_LevelRenderCtor) return NULL;
    void* mem = g_operatorNew(0x10D0);
    if (!mem) return NULL;
    void* robj = g_LevelRenderCtor(mem, mesh);
    if (robj) *(void**)((char*)ext + renderOff) = robj;
    return robj;
}
static void* S1EnsureMeshNode(void* board, void* ext, DWORD offset, const char* path) {
    void* cur = *(void**)((char*)ext + offset);
    if (cur) return cur;
    DWORD app = GetBoardApp(board);
    if (!app || IsBadReadPtr((void*)app, 0x200)) return NULL;
    void* gfx = *(void**)((char*)app + 0x174);
    if (!gfx || !g_operatorNew || !g_MeshNodeCtor) return NULL;
    void* mem = g_operatorNew(0x18);
    if (!mem) return NULL;
    void* node = g_MeshNodeCtor(mem, gfx, path);
    if (node) *(void**)((char*)ext + offset) = node;
    return node;
}
static void* S1EnsureSprite(void* board, void* ext, DWORD offset, const char* path) {
    void* cur = *(void**)((char*)ext + offset);
    if (cur) return cur;
    DWORD app = GetBoardApp(board);
    if (!app || IsBadReadPtr((void*)app, 0x200)) return NULL;
    void* gfx = *(void**)((char*)app + 0x174);
    if (!gfx || !g_operatorNew || !g_SpriteCtor) return NULL;
    void* mem = g_operatorNew(0x110);
    if (!mem) return NULL;
    void* spr = g_SpriteCtor(mem, gfx, path);
    if (spr) *(void**)((char*)ext + offset) = spr;
    return spr;
}

void __thiscall UniversalCreateDynamicObjectsImpl(void *board, char *name, void *out1, void *out2, int *s1data) {
    /* OUT-PARAM RULE (blank-world fix 2026-10-22): out1 must be 0 or a REAL
     * game object — NEVER a mesh/render pointer. The native Setup caller
     * appends out1 to an object list; a mesh there poisons the D3D device
     * and blanks the whole frame. Native Tower create sets out params ONLY
     * for TURRET (stands,render); every other S1 leaves them untouched. */
    if (!name || !out1 || !out2 || !s1data) return;
    int level = GetCurrentLevel(board);
    if (level == 0 || level > 15) { *(int*)out1 = 0; *(int*)out2 = 0; return; }
    void* ext = EnsureBoardExt(board);
    if (!ext) { *(int*)out1 = 0; *(int*)out2 = 0; return; }

    /* Native-ctor fast path: native-built boards get NATIVE objects via the
     * saved orig create (our replicating handler misses engine fields).
     * Origs are looked up by CONTENT (native slot for that content), so
     * cross-slot swaps delegate to the content's own native handler. */
    if (*(int *)((char *)ext + UNI_NATIVE_CTOR)) {
        int cslot = ContentFileId(ext);
        int effSlot = (cslot >= 10 && cslot <= 15) ? cslot : level;
        if (effSlot >= 1 && effSlot <= 15 && g_origCreate[effSlot]) {
            g_origCreate[effSlot](board, name, out1, out2, s1data);
            return;
        }
    }

    {
        char dbg[256];
        wsprintfA(dbg, "CreateDynamicObjects: level=%d name='%.64s' board=0x%08X s1data=0x%08X", level, name, (DWORD)board, (DWORD)s1data);
        DebugLog(dbg);
    }

    DWORD app = GetBoardApp(board);
    int difficulty = (app && !IsBadReadPtr((void*)app, 0x500)) ? *(int *)(app + APP_DIFFICULTY) : 0;
    int meshWorld = *(int *)((char *)board + BOARD_MESHWORLD);

    /* Extract position from S1 data: +4=X, +8=Y, +0xC=Z, +0x10=X2, +0x14=Y2, +0x18=Z2 */
    float x = *(float *)(s1data + 1);
    float y = *(float *)(s1data + 2);
    float z = *(float *)(s1data + 3);
    float x2 = *(float *)(s1data + 4);
    float y2 = *(float *)(s1data + 5);
    float z2 = *(float *)(s1data + 6);
    float fparam = *(float *)(s1data + 5);  /* same as y2 for some ctors */

    void *obj = NULL;
    int renderOut = 0;

    /* ── SECRET / SECRETUNLOCK (shared CreateSecretObjects, Ghidra-verified 2026-09-11) ──
     * Native order: SECRETUNLOCK (12-char match) first, then SECRET (6-char).
     * Gate: difficulty != 0 && App+0x234 == 0 && profile(App+0x220)+0x11 == 0.
     * Mesh: App+0x580 for UNLOCK, App+0x57c for SECRET.
     * Secret_ctor(mem, board, x,y,z, mesh), size 0x10EC; appends to board+0xCD4,
     * meshdb+0x1C, board+0x8B0+0x18 (render at obj+0x10E0), board+0x2578.
     * Returns 0,0 (native per-level create ignores SECRET S1s; shared fn owns them). */
    if (my_strnicmp(name, "SECRETUNLOCK", 12) == 0 || my_strnicmp(name, "SECRET", 6) == 0) {
        int isUnlock = (my_strnicmp(name, "SECRETUNLOCK", 12) == 0);
        if (difficulty != 0 && app && !IsBadReadPtr((void*)app, 0x600) && g_SecretCtor &&
            g_operatorNew && g_AthenaListAppend && *(char *)(app + 0x234) == 0) {
            DWORD profile = *(DWORD *)(app + 0x220);
            if (profile && !IsBadReadPtr((void*)profile, 0x20) && *(char *)(profile + 0x11) == 0) {
                void *seMesh = *(void **)((char *)app + (isUnlock ? 0x580 : 0x57C));
                if (seMesh) {
                    void *seMem = g_operatorNew(0x10EC);
                    if (seMem) {
                        float meshBits;
                        memcpy(&meshBits, &seMesh, 4);
                        void *sobj = g_SecretCtor(seMem, (int)board, x, y, z, meshBits);
                        if (sobj) {
                            g_AthenaListAppend((void*)((char*)board + 0xCD4), (int)sobj);
                            if (meshWorld && !IsBadReadPtr((void*)meshWorld, 0x500)) {
                                DWORD mdb = *(DWORD *)((char *)meshWorld + 0x480);
                                if (mdb && !IsBadReadPtr((void*)mdb, 0x30))
                                    g_AthenaListAppend((void*)(mdb + 0x1C), (int)sobj);
                            }
                            DWORD b8b0 = *(DWORD *)((char *)board + 0x8B0);
                            if (b8b0 && !IsBadReadPtr((void*)b8b0, 0x30))
                                g_AthenaListAppend((void*)(b8b0 + 0x18), ((DWORD*)sobj)[0x438]);
                            g_AthenaListAppend((void*)((char*)board + 0x2578), (int)sobj);
                        }
                    }
                }
            }
        }
        *(int*)out1 = 0; *(int*)out2 = 0;
        return;
    }

    /* ── TIPPER (Dizzy) ── */
    // S1 ensure for swapped files

    if (my_strnicmp(name, "TIPPER", 6) == 0 && difficulty != 0) {
        S1EnsureMeshWorld(board, ext, UNI_TIPPER_MESH, "Levels\\Level3-Tipper");
        S1EnsureRender(board, ext, UNI_TIPPER_RENDER, UNI_TIPPER_MESH);
        int meshOff = UNI_TIPPER_MESH;
        int renderOff = UNI_TIPPER_RENDER;
        int meshVal = *(int*)((char*)ext + meshOff);
        if (!meshVal) { DebugLog("TIPPER: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        {
            char dbg[256];
            wsprintfA(dbg, "TIPPER: meshOff=0x%X meshVal=0x%X renderOff=0x%X renderVal=0x%X level=%d rot=%.1f,%.1f,%.1f",
                      meshOff, meshVal, renderOff, *(int*)((char*)ext + renderOff), level, x2,y2,z2);
            DebugLog(dbg);
        }
        void *mem = g_operatorNew(0x1104);
        if (mem) {
            /* Mirror into the native mesh slot first (native Dizzy create
             * reads board+0x436C as the Tipper_ctor mesh arg). */
            if (BoardHasOffset(board, 0x436C, 4)) {
                int natM = *(int *)((char *)board + 0x436C);
                if (!natM || IsBadReadPtr((void *)natM, 4))
                    *(int *)((char *)board + 0x436C) = meshVal;
            }
            obj = g_TipperCtor(mem, (int)board, meshVal);
            DWORD *o = (DWORD *)obj;
            memcpy(&o[0x436], &x, 4); memcpy(&o[0x437], &y, 4); memcpy(&o[0x438], &z, 4);
            memcpy(&o[0x439], &x2, 4); memcpy(&o[0x43A], &y2, 4); memcpy(&o[0x43B], &z2, 4);
            if (ext) {
                *(float*)((char*)ext + UNI_TIPPER_ROT_X) = x2;
                *(float*)((char*)ext + UNI_TIPPER_ROT_Y) = y2;
                *(float*)((char*)ext + UNI_TIPPER_ROT_Z) = z2;
            }
            void *vmem = g_operatorNew(0x10D0);
            if (vmem) {
                void *vis = g_TipperVisualCtor(vmem, *(int*)((char*)ext + renderOff));
                o[0x435] = (DWORD)vis;
                g_TipperVisualAttach(vis, (void*)obj);
            }
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
        }
        OrBoardFeat(board, FEAT_SWIRL);
        *(int*)out1 = (int)obj; *(int*)out2 = (int)renderOut;
        return;
    }


    /* ── WATERWHEEL (Dizzy) ── */
    if (my_strnicmp(name, "WATERWHEEL", 10) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_MESH_0, "Levels\\Level3-WaterWheel");
        S1EnsureRender(board, ext, UNI_MESH_1, UNI_MESH_0);
        // Canonical UNI_* inside ext (OFF_* aliases same).
        *(float *)((char *)ext + UNI_WHEELEMBED_X) = x;
        *(float *)((char *)ext + UNI_WHEELEMBED_Y) = y;
        *(float *)((char *)ext + UNI_WHEELEMBED_Z) = z;
        *(float*)((char*)ext + UNI_WATER_ROT_X) = x2;
        *(float*)((char*)ext + UNI_WATER_ROT_Y) = y2;
        *(float*)((char*)ext + UNI_WATER_ROT_Z) = z2;
        /* Mirror into native slots (ctor Step 8b ran when ext was empty, so
         * native slots hold NULLs — re-mirror now that meshes are loaded).
         * Native Dizzy create reads +0x4BA8 as the mesh and writes S1 pos
         * to +0x4BB0/4/8, zeroing +0x4BBC (wheel angle) per S1. */
        if (BoardHasOffset(board, 0x4BA8, 4) && BoardHasOffset(board, 0x4BBC, 4)) {
            *(int *)((char *)board + 0x4BA8) = *(int *)((char *)ext + UNI_MESH_0);
            *(int *)((char *)board + 0x4BAC) = *(int *)((char *)ext + UNI_MESH_1);
            *(float *)((char *)board + 0x4BB0) = x;
            *(float *)((char *)board + 0x4BB4) = y;
            *(float *)((char *)board + 0x4BB8) = z;
            *(int *)((char *)board + 0x4BBC) = 0;
        }
        if (!obj) { obj = *(void **)((char *)board + 0x4BA8); renderOut = *(int *)((char *)board + 0x4BAC); }
        *(DWORD *)((char *)ext + UNI_JUDGE_LIST) = 0;
        OrBoardFeat(board, FEAT_SWIRL);
        {
            char dbg[256];
            wsprintfA(dbg, "WATERWHEEL: pos=%.1f,%.1f,%.1f rot=%.1f,%.1f,%.1f feat SWIRL auto-enabled", x,y,z,x2,y2,z2);
            DebugLog(dbg);
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── SWIRL (Dizzy) ── */
    if (my_strnicmp(name, "SWIRL", 5) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_MESH_6, "Levels\\Level3-Swirl");
        S1EnsureRender(board, ext, UNI_MESH_7, UNI_MESH_6);
        /* Original: obj = board+0x4BC4 (Swirl mesh), renderOut = board+0x4BC8 (render obj)
         * Mod: Swirl mesh at UNI_MESH_6 (0x85F8), render at UNI_MESH_7 (0x85FC) */
        /* Store swirl position for step3b mesh rotation */
        *(float *)((char *)ext + UNI_MESH_15) = x;
        *(float *)((char *)ext + UNI_MESH_12) = y;
        *(float *)((char *)ext + UNI_MESH_13) = z;
        /* Mirror into native slots (Step 8b ran on empty ext — re-mirror now).
         * Native Dizzy create reads +0x4BC4 as the mesh, writes S1 pos to
         * +0x4BCC/D0/D4, returns (mesh, render) as outs, no appends. */
        if (BoardHasOffset(board, 0x4BC4, 4) && BoardHasOffset(board, 0x4BCC, 4)) {
            *(int *)((char *)board + 0x4BC4) = *(int *)((char *)ext + UNI_MESH_6);
            *(int *)((char *)board + 0x4BC8) = *(int *)((char *)ext + UNI_MESH_7);
            *(float *)((char *)board + 0x4BCC) = x;
            *(float *)((char *)board + 0x4BD0) = y;
            *(float *)((char *)board + 0x4BD4) = z;
        }
        obj = *(void **)((char *)board + 0x4BC4);
        renderOut = *(int *)((char *)board + 0x4BC8);
        OrBoardFeat(board, FEAT_SWIRL);
        // Mirror for legacy OFF_* readers (alias, same addr) — use outer ext, no shadow.
        if (ext) { *(void**)((char*)ext+OFF_SWIRL_MESH)=obj; *(int*)((char*)ext+OFF_SWIRL_RENDER)=renderOut; *(float*)((char*)ext+OFF_SWIRL_POS_X)=x; *(float*)((char*)ext+OFF_SWIRL_POS_Y)=y; *(float*)((char*)ext+OFF_SWIRL_POS_Z)=z; }
        /* NOTE: no ext zone object — Feature_SwirlZones walks the NATIVE
         * board+0x4378 gluebie list (native Dizzy behavior; the gluebies ARE
         * the proximity zones). Ext zones were misplaced (SWIRL S1 pos, not
         * gluebie pos) and are retired. */
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── GLUEBIE (Dizzy) ── */
    if (my_strnicmp(name, "GLUEBIE", 7) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_GLUEBIE_MESH, "Levels\\Level3-Gluebie");
        if (difficulty == 0) { *(int*)out1 = 0; *(int*)out2 = 0; return; }
        int meshOff = UNI_GLUEBIE_MESH;
        int meshVal = *(int*)((char*)ext + meshOff);
        if (!meshVal) { DebugLog("GLUEBIE: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        /* Mirror into the native mesh slot (native Dizzy ctor +0x4374;
         * native create reads it as the Gluebie_ctor mesh arg). */
        if (BoardHasOffset(board, 0x4374, 4)) {
            int natM = *(int *)((char *)board + 0x4374);
            if (!natM || IsBadReadPtr((void *)natM, 4))
                *(int *)((char *)board + 0x4374) = meshVal;
        }
        void *mem = g_operatorNew(0x110C);
        if (mem) {
            obj = g_GluebieCtor(mem, (int)board, meshVal);
            DWORD *o = (DWORD *)obj;
            memcpy(&o[0x435], &x, 4); memcpy(&o[0x436], &y, 4); memcpy(&o[0x437], &z, 4);
            /* Original appends to board+0x4378 (Gluebie list) + board+0x2578 (obj list).
             * UNI_MESH_3 (0x85EC) is a mesh SLOT, not an AthenaList — appending to it
             * clobbers mesh pointers at 0x8620-0x862C via the iter array. */
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            /* board+0x4378 is Dizzy's gluebie list (inited in 9a-extra2 for
             * Dizzy content). Native Dizzy create appends here AND 0x2578 —
             * the update walks 0x4378 for tar/swirl proximity. Gate on
             * content (not slot): Dizzy file in any slot drives it. */
            if (GetCurrentLevel(board) == 4 || ContentFileId(GetBoardExt(board)) == 4)
                g_AthenaListAppend((void*)((char*)board + 0x4378), (int)obj);
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* Tower bisect neuter REMOVED 2026-10-21 — drawbridge fixed, all live. */
    /* ── CATAPULT (Tower) ──
     * Native: Catapult_ctor(new(0x1108), board, board+0x436C mesh). Mirror
     * our mesh into the native slot (preload gap, same as WINDMILL). */
    if (my_strnicmp(name, "CATAPULT", 8) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_CATAPULT_MESH, "Levels\\Level4-Catapult");
        int meshOff = UNI_CATAPULT_MESH;
        int meshVal = *(int*)((char*)ext + meshOff);
        /* Always mirror into native slot (preload never runs under mod). */
        *(int *)((char *)board + 0x436C) = meshVal;
        if (!meshVal) { DebugLog("CATAPULT: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1108);
        if (mem) {
            obj = g_CatapultCtor(mem, (int)board, meshVal);
            DWORD *o = (DWORD *)obj;
            memcpy(&o[0x436], &x, 4); memcpy(&o[0x437], &y, 4); memcpy(&o[0x438], &z, 4);
            int listOff = UNI_CATAPULT_LIST;
            g_AthenaListAppend((void*)((char*)ext + listOff), (int)obj);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            g_AthenaListAppend((void*)((char*)board + 0x43B8), (int)obj); /* native 2nd list (inited Step 9a-extra3) */
            renderOut = o[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── MACE (Tower) ──
     * Native: Mace_ctor(new(0x110C), board, board+0x4378 mesh). Mirror
     * our mesh into the native slot (preload gap, same as WINDMILL). */
    if (my_strnicmp(name, "MACE", 4) == 0 && difficulty != 0) {
        /* Tower native ctor fills board+0x4378 itself - use it when valid,
         * else fall back to ext load+mirror (other levels / no delegation). */
        int meshVal = BoardMeshOrZero(board, 0x4378);
        if (!meshVal) {
            S1EnsureMeshWorld(board, ext, UNI_MACE_MESH, "Levels\\Level4-Mace");
            meshVal = *(int*)((char*)ext + UNI_MACE_MESH);
            /* Always mirror into native slot (preload never runs under mod). */
            *(int *)((char *)board + 0x4378) = meshVal;
        }
        if (!meshVal) { DebugLog("MACE: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        /* YellowLink chain mesh (native Tower ctor +0x4374, MeshNode 0x18).
         * Tower content only; shared Meshes asset so safe for any slot.
         * Cleared: the 0001:000570A4 hang was garbage-mesh trust (board
         * +0x4378 = 0x03000000 passed IsBadReadPtr), now fixed by
         * BoardMeshOrZero — YellowLink was never the culprit. */
        if (ContentFileId(GetBoardExt(board)) == 5 && BoardHasOffset(board, 0x4374, 4)) {
            int yExt = *(int *)((char *)ext + UNI_YELLOWLINK_MESH);
            if (!yExt || IsBadReadPtr((void *)yExt, 4)) {
                S1EnsureMeshNode(board, ext, UNI_YELLOWLINK_MESH, "Meshes\\YellowLink");
                yExt = *(int *)((char *)ext + UNI_YELLOWLINK_MESH);
            }
            if (yExt) *(int *)((char *)board + 0x4374) = yExt;
        }
        void *mem = g_operatorNew(0x110C);
        if (mem) {
            /* Tower MACE S1s build via 0x438750 (own class); other
             * levels via 0x437CB0. Verified in native Tower create. */
            if (ContentFileId(GetBoardExt(board)) == 5 && g_MaceTowerCtor) obj = g_MaceTowerCtor(mem, (int)board, meshVal);
            else obj = g_MaceCtor(mem, (int)board, meshVal);
            DWORD *o = (DWORD *)obj;
            memcpy(&o[0x436], &x, 4); memcpy(&o[0x437], &y, 4); memcpy(&o[0x438], &z, 4);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            g_AthenaListAppend((void*)((char*)board + 0x5000), (int)obj); /* native 2nd list (inited Step 9a-extra3) */
            g_AthenaListAppend((void*)((char*)ext + UNI_MACE_LIST), (int)obj);
            if (g_AthenaListGetSize((void*)((char*)board + 0x5000)) == 1) {
                o[0x43A] = 0x42A00000; o[0x43D] = 1; o[0x43E] = 0x32;
            }
            renderOut = o[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── DRAWBRIDGE (Tower) ──
     * Native: Glass_Level_ctor(new(0x113C), board, board+0x4370 mesh);
     * pos → obj+0x436/7/8; lists board+0x2578 + board+0x4BE8; out=(obj,obj[0x435]).
     * The board+0x4370 slot holds preload garbage under the mod — mirror our
     * mesh into it (same fix as WINDMILL/TURRET). */
    if (my_strnicmp(name, "DRAWBRIDGE", 10) == 0) {
        /* Delegation test DISABLED 2026-10-22: native 0x40D7C0 call suspected
         * in Tower wild-EIP crashes (per-call side effects?). Our own Glass
         * ctor path instead (native-parity per comments below). */
        /* Tower native ctor fills board+0x4370 itself - use when valid. */
        int dExt = BoardMeshOrZero(board, 0x4370);
        if (!dExt) {
            S1EnsureMeshWorld(board, ext, UNI_DRAWBRIDGE_MESH, "Levels\\Level4-Drawbridge");
            dExt = *(int*)((char*)ext + UNI_DRAWBRIDGE_MESH);
            *(int *)((char *)board + 0x4370) = dExt;
        }
#if 1 /* delegation off 2026-10-22, own Glass path */
        int meshVal = dExt;
        if (!meshVal) { DebugLog("DRAWBRIDGE: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x113C);
        if (mem) {
            obj = g_GlassLevelCtor(mem, (int)board, meshVal);
            DWORD *o = (DWORD *)obj;
            memcpy(&o[0x436], &x, 4); memcpy(&o[0x437], &y, 4); memcpy(&o[0x438], &z, 4);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            /* Native 2nd list: required by the update loop (empty list =
             * update crash). Setup kill earlier was the garbage mesh, not this. */
            g_AthenaListAppend((void*)((char*)board + 0x4BE8), (int)obj);
            g_AthenaListAppend((void*)((char*)ext + UNI_DRAWBRIDGE_LIST), (int)obj);
            renderOut = o[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
#endif
    }

    /* ── WINDMILL (Tower) ──
     * Native (Tower_CreateDynamicObjects, Ghidra 2026-10-21): mesh =
     * pre-loaded board+0x437c (no fresh load), render = Level_RenderCtor,
     * pos → board+0x4380/84/88, RNG(0x168) → board+0x438c, out=(mesh,render). */
    if (my_strnicmp(name, "WINDMILL", 8) == 0) {
        /* BLANK-WORLD BISECT: windmill re-enabled (turret still skipped). */
        /* Native Tower ctor owns +0x437C (render) — use it when valid.
         * Never overwrite with our mesh (native update/render vtbl-calls it). */
        {
            int natR = BoardMeshOrZero(board, 0x437C);
            if (natR) {
                *(int*)out1 = 0; *(int*)out2 = natR; return;
            }
        }
        /* Native board+0x437C is filled by the static preload our Step 8
         * replaced — under the mod it holds garbage (0xFE...). Mirror our
         * loaded mesh into the native slot so game code sees a valid mesh. */
        S1EnsureMeshWorld(board, ext, UNI_WINDMILL_MESH, "Levels\\Level4-Windmill");
        int extMesh = *(int *)((char *)ext + UNI_WINDMILL_MESH);
        /* Always mirror: native preload never runs under the mod. */
        *(int *)((char *)board + 0x437C) = extMesh;
        int mesh = extMesh;
        if (!mesh) { DebugLog("WINDMILL: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x10D0);
        if (mem) {
            void *render = g_LevelRenderCtor(mem, (void*)mesh);
            g_TipperVisualAttach(render, (void*)mesh);
            renderOut = (int)render;
            /* Store render obj in dedicated render offset for REND_WINDMILL */
            *(DWORD *)((char *)ext + REND_TOWER_WINDMILL) = (DWORD)render;
        }
        /* Native board slots first (game update reads these), ext mirrors for our driver. */
        *(float *)((char *)board + 0x4380) = x;
        *(float *)((char *)board + 0x4384) = y;
        *(float *)((char *)board + 0x4388) = z;
        *(float *)((char *)ext + UNI_WINDMILL_X) = x;
        *(float *)((char *)ext + UNI_WINDMILL_Y) = y;
        *(float *)((char *)ext + UNI_WINDMILL_Z) = z;
        if (g_RNG) {
            float ra = (float)RNG_call((void*)0x4F7360, 0, 0x168, 0);
            *(float *)((char *)board + 0x438C) = ra;
            *(float *)((char *)ext + UNI_WINDMILL_ANGLE) = ra;
        }
        /* Native returns (mesh, render) — the NULL out1 was our invention. */
        *(int*)out1 = 0; *(int*)out2 = renderOut;
        return;
    }

    /* ── TRAPDOOR (Tower, level!=13) ── — Sky TRAPDOOR (level 13) uses Rotator POPDOOR path below */
    if (my_strnicmp(name, "TRAPDOOR", 8) == 0 && GetCurrentLevel(board) != 13) {
        void *mem = g_operatorNew(0x10F8);
        if (mem) {
            obj = g_TrapdoorCtor(mem, (int)board);
            DWORD *o = (DWORD *)obj;
            memcpy(&o[0x438], &x, 4); memcpy(&o[0x439], &y, 4); memcpy(&o[0x43A], &z, 4);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            g_AthenaListAppend((void*)((char*)board + 0x47D0), (int)obj); /* native 2nd list (inited Step 9a-extra3) */
            g_AthenaListAppend((void*)((char*)ext + UNI_TRAPDOOR_LIST), (int)obj);
            renderOut = o[0x435];
            g_AthenaListAppend((void*)((char*)board + UNI_TRAPDOOR_MESH_LIST), o[0x436]);
            g_AthenaListAppend((void*)((char*)board + UNI_TRAPDOOR_RENDER_LIST), o[0x437]);
            if (meshWorld) {
                int mw = *(int *)(meshWorld + 0x480);
                if (mw) g_AthenaListAppend((void*)(mw + 0x1C), o[0x436]);
                int ro = *(int *)((char *)board + BOARD_RENDEROBJ);
                if (ro) g_AthenaListAppend((void*)(ro + 0x18), o[0x437]);
            }
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── CHOMPER (Tower) ── */
    if (my_strnicmp(name, "CHOMPER", 7) == 0) {
        /* Load Chomper mesh if not already loaded */
        S1EnsureMeshNode(board, ext, UNI_CHOMPER_MESH, "Meshes\\Chomper");
        void *chomperMesh = *(void **)((char *)ext + UNI_CHOMPER_MESH);
        if (chomperMesh) *(DWORD *)((char *)ext + REND_TOWER_CHOMPER) = (DWORD)chomperMesh;
        /* Mirror the MeshNode into the native render slot — the clean-room
         * Tower render draws board+0x4390 via vtbl[7] (native 0x40DFA0). */
        if (chomperMesh && BoardHasOffset(board, 0x4390, 4)) {
            int natC = *(int *)((char *)board + 0x4390);
            if (!natC || IsBadReadPtr((void *)natC, 4))
                *(DWORD *)((char *)board + 0x4390) = (DWORD)chomperMesh;
        }
        /* Native writes board+0x4394/98/9c (game reads these); ext mirrors for our driver. */
        *(float *)((char *)board + 0x4394) = x;
        *(float *)((char *)board + 0x4398) = y;
        *(float *)((char *)board + 0x439C) = z;
        /* Adjust Y by constant (original subtracts _DAT_004CF370) */
        float adj = *(float *)(g_moduleBase + 0xCF370);
        *(float *)((char *)board + 0x4398) -= adj;
        *(float *)((char *)ext + REND_TOWER_CHOMP_X) = x;
        *(float *)((char *)ext + REND_TOWER_CHOMP_Y) = y - adj;
        *(float *)((char *)ext + REND_TOWER_CHOMP_Z) = z;
        *(int*)out1 = 0; *(int*)out2 = 0;
        return;
    }

    /* ── TURRET tower (Tower) — not a gun, it's a tower ──
     * Native (Tower_CreateDynamicObjects, Ghidra 2026-10-21):
     *   Stands_ctor(new(0x10D0), board+0x43b4 mesh); Timer_Init(stack);
     *   timer vtbl[2] thiscall (x,y,z by value); Stands vtbl[0x15] thiscall
     *   (ECX=stands, one ptr arg, RET4 — NOT fastcall); Level_RenderCtor;
     *   TipperVisual_Attach; Timer_Cleanup(stack); out=(stands,render). */
    if (my_strnicmp(name, "TURRET", 6) == 0) {
        /* Same preload gap as WINDMILL: board+0x43B4 holds garbage under
         * the mod. Mirror our loaded mesh into the native slot. */
        S1EnsureMeshWorld(board, ext, UNI_TURRET_MESH, "Levels\\Level4-Turret");
        int tExt = *(int *)((char *)ext + UNI_TURRET_MESH);
        /* Always mirror: native preload never runs under the mod. */
        *(int *)((char *)board + 0x43B4) = tExt;
        void *meshPtr = (void *)tExt;
        if (!meshPtr) { *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x10D0);
        if (mem) {
            int stands = (int)(DWORD)g_StandsCtor(mem, (int)(DWORD)meshPtr);
            char timerBuf[68];
            g_TimerInit(timerBuf);
            /* Copy position from S1 data to stack struct (matching original) */
            float pos[3];
            pos[0] = x; pos[1] = y; pos[2] = z;
            /* Call TIMER vtable[2] (Gfx_SetPosition) with 3 floats by value */
            DWORD *timerVtbl = *(DWORD **)timerBuf;
            if (timerVtbl) {
                void (__thiscall *timerSetPos)(void*, float, float, float) = (void (__thiscall *)(void*, float, float, float))timerVtbl[2];
                if (timerSetPos) timerSetPos(timerBuf, pos[0], pos[1], pos[2]);
            }
            /* STANDS vtbl[0x15] (slot 21): native thiscall (ECX=stands, one
             * ptr arg, RET4). The old __fastcall here imbalanced the stack
             * and killed Tower setup. */
            DWORD *standsVtbl = *(DWORD **)stands;
            if (standsVtbl) {
                void (__thiscall *fn54)(DWORD, float *) = (void (__thiscall *)(DWORD, float *))standsVtbl[0x15];
                if (fn54) fn54((DWORD)stands, pos);
            }
            void *rmem = g_operatorNew(0x10D0);
            if (rmem) {
                void *render = g_LevelRenderCtor(rmem, (void*)stands);
                g_TipperVisualAttach(render, (void*)stands);
                obj = (void*)stands;
                renderOut = (int)render;
                *(DWORD *)((char *)ext + REND_TOWER_TURRET) = (DWORD)render;
            }
            g_TimerCleanup(timerBuf);
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── BONK (Expert) ── */
    if (my_strnicmp(name, "BONK", 4) == 0 && difficulty != 0) {
        void *mem = g_operatorNew(0x1200);
        if (mem) {
            obj = g_BonkCtor(mem, (int)board, x, y, z);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            /* Native stores the bonk obj at board+0x436C (read by
             * E:CALLHAMMER/E:HAMMERCHASE). Mirror it. */
            if (BoardHasOffset(board, 0x436C, 4)) *(void **)((char *)board + 0x436C) = obj;
            DWORD *o = (DWORD *)obj;
            renderOut = o[0x43E];
            int storeOff = UNI_BONK_MESH;
            void* bExt = GetBoardExt(board); if(!bExt) bExt=EnsureBoardExt(board);
            if(bExt) *(void **)((char *)bExt + storeOff) = obj;
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── FAN (Expert) ── */
    if (my_strnicmp(name, "FAN", 3) == 0 && difficulty != 0) {
        void *mem = g_operatorNew(0x1188);
        if (mem) {
            obj = g_FanCtor(mem, (int)board, x, y, z, fparam);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            if (strstr(name, "SLOW")) ((DWORD*)obj)[0x43B] = 1;
            if (strstr(name, "SUPER")) *(char*)((char*)obj + 0x10ED) = 1;
            if (strstr(name, "UP") && g_SoundInitChannels) g_SoundInitChannels(obj, 1);
        }
        /* Native FAN out2 = 0 (local_14 untouched by the FAN branch). */
        *(int*)out1 = (int)obj; *(int*)out2 = 0;
        return;
    }

    /* ── SAWBLADE (Expert) ── */
    if (my_strnicmp(name, "SAWBLADE", 8) == 0 && difficulty != 0) {
        void *mem = g_operatorNew(0x111C);
        if (mem) {
            obj = g_SawBladeCtor(mem, (int)board, x, y, z);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            /* Native stores the saw obj in board+0x4370 ("1") / +0x4374 ("2")
             * (Expert slot33 0x40E487/0x40E4A8) - the saw update reads its slot. */
            if (strstr(name, "1")) { g_SawBladeSetVariant(obj, 1); *(void **)((char *)ext + UNI_SAWBLADE1_OBJ) = obj; *(void **)((char *)board + 0x4370) = obj; }
            if (strstr(name, "2")) { g_SawBladeSetVariant(obj, 2); *(void **)((char *)ext + UNI_SAWBLADE2_OBJ) = obj; *(void **)((char *)board + 0x4374) = obj; }
        }
        /* Native SAWBLADE out2 = 0 (local_14 untouched by the SAW branch). */
        *(int*)out1 = (int)obj; *(int*)out2 = 0;
        return;
    }

    /* ── SAWBRIDGE (Expert cut-away spinner) — renamed from BRIDGE (level==8) ──
     * S1: SAWBRIDGE1 / SAWBRIDGE2 (+ NEG) — was BRIDGE1/2 on Expert
     * BRIDGE (drawbridge) is now Intermediate/Master only, level-gate removed */
    if (my_strnicmp(name, "SAWBRIDGE", 9) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_BONK_STORE, "Levels\\Level2-Bridge");
        void* brMesh = *(void**)((char*)ext + UNI_BONK_STORE);
        /* Native Spinner_Level_ctor reads its mesh from board+0x4378
         * (preload gap under mod - mirror ours in, Tower pattern). */
        if (brMesh) *(void**)((char*)board + 0x4378) = brMesh;
        void* brRender = *(void**)((char*)ext + UNI_SAW1_OBJ);
        if (!brRender && brMesh) {
            void* mem = g_operatorNew(0x10D0);
            if (mem) {
                void* robj = g_LevelRenderCtor(mem, brMesh);
                if (robj) {
                    g_TipperVisualAttach(robj, brMesh);
                    *(void**)((char*)ext + UNI_SAW1_OBJ) = robj;
                }
            }
        }
        /* Spinner_Level_ctor */
        void *mem = g_operatorNew(0x10FC);
        if (mem) {
            obj = g_SpinnerLevelCtor(mem, (int)board, x, y, z, fparam);
            DWORD *o = (DWORD *)obj;
            renderOut = o[0x43D];
            if (strstr(name, "1")) g_AthenaListAppend((void*)((char*)ext + UNI_LIST_1), (int)obj);
            if (strstr(name, "2")) g_AthenaListAppend((void*)((char*)ext + UNI_LIST_2), (int)obj);
            if (strstr(name, "NEG")) o[0x43E] = 0xBF800000;
        }
        /* Native appends the spinner to board+0x4380 ("1") / board+0x4798 ("2")
         * (Expert slot33 0x40E549/0x40E567, substring test) - never 0x2578.
         * The native list driver (still running) picks it up from there.
         /* Init lazily here (not ctor): native Board_Setup overwrites these
         * slots after the ctor hook runs, so ctor-time init is wiped. */
                if (obj && !(GetBoardFeat(board) & FEAT_EXPERT_LISTS)) {
                       if (g_AthenaListInit) { g_AthenaListInit((void*)((char*)board + 0x4380), 0); g_AthenaListInit((void*)((char*)board + 0x4798), 0); }
            OrBoardFeat(board, FEAT_EXPERT_LISTS);
                   }
                if (obj && strstr(name, "1")) g_AthenaListAppend((void*)((char*)board + 0x4380), (int)obj);
        if (obj && strstr(name, "2")) g_AthenaListAppend((void*)((char*)board + 0x4798), (int)obj);
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── BRIDGE (Intermediate/Master drawbridge, position-only) ──
     * S1: BRIDGE / BRIDGE(NOCOLLIDE) — SAWBRIDGE is Expert spinner, handled above
     * Vanilla level5 has S1 name "BRIDGE" but expects Spinner_Level_ctor;
     * Intermediate BRIDGE is pos-only. Dispatch by level to handle both files. */
    if (my_strnicmp(name, "BRIDGE", 6) == 0) {
        int _lvl = GetCurrentLevel(board);
        if (_lvl == 8 && my_strnicmp(name, "SAWBRIDGE", 9) != 0) {
                       /* Expert vanilla "BRIDGE" — treat as SAWBRIDGE spinner */
            S1EnsureMeshWorld(board, ext, UNI_BONK_STORE, "Levels\\Level2-Bridge");
            void* brMesh2 = *(void**)((char*)ext + UNI_BONK_STORE);
            /* Native Spinner_Level_ctor reads its mesh from board+0x4378
             * (preload gap under mod - mirror ours in, Tower pattern). */
            if (brMesh2) *(void**)((char*)board + 0x4378) = brMesh2;
            void* brRender2 = *(void**)((char*)ext + UNI_SAW1_OBJ);
            if (!brRender2 && brMesh2) {
                void* mem = g_operatorNew(0x10D0);
                if (mem) {
                    void* robj = g_LevelRenderCtor(mem, brMesh2);
                    if (robj) {
                        g_TipperVisualAttach(robj, brMesh2);
                        *(void**)((char*)ext + UNI_SAW1_OBJ) = robj;
                    }
                }
            }
            void *mem = g_operatorNew(0x10FC);
            if (mem) {
                obj = g_SpinnerLevelCtor(mem, (int)board, x, y, z, fparam);
                               DWORD *o = (DWORD *)obj;
                renderOut = o[0x43D];
                /* Vanilla BRIDGE has no 1/2 suffix — append to LIST_1 by default */
                if (strstr(name, "2")) g_AthenaListAppend((void*)((char*)ext + UNI_LIST_2), (int)obj);
                else g_AthenaListAppend((void*)((char*)ext + UNI_LIST_1), (int)obj);
                if (strstr(name, "NEG")) o[0x43E] = 0xBF800000;
            }
            /* Native appends the spinner to board+0x4380 ("1") / board+0x4798 ("2")
             * (Expert slot33 0x40E549/0x40E567, substring test) - never 0x2578.
             * The native list driver (still running) picks it up from there.
             /* Init lazily here (not ctor): native Board_Setup overwrites these
             * slots after the ctor hook runs, so ctor-time init is wiped. */
                    if (obj && !(GetBoardFeat(board) & FEAT_EXPERT_LISTS)) {
                if (g_AthenaListInit) { g_AthenaListInit((void*)((char*)board + 0x4380), 0); g_AthenaListInit((void*)((char*)board + 0x4798), 0); }
                OrBoardFeat(board, FEAT_EXPERT_LISTS);
            }
                        if (obj && strstr(name, "2")) g_AthenaListAppend((void*)((char*)board + 0x4798), (int)obj);
            else if (obj) g_AthenaListAppend((void*)((char*)board + 0x4380), (int)obj);
            *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
            return;
        }
        S1EnsureMeshWorld(board, ext, UNI_BONK_STORE, "Levels\\Level2-Bridge");
        void* brMesh = *(void**)((char*)ext + UNI_BONK_STORE);
        void* brRender = *(void**)((char*)ext + UNI_SAW1_OBJ);
        if (!brRender && brMesh) {
            void* mem = g_operatorNew(0x10D0);
            if (mem) {
                void* robj = g_LevelRenderCtor(mem, brMesh);
                if (robj) {
                    g_TipperVisualAttach(robj, brMesh);
                    *(void**)((char*)ext + UNI_SAW1_OBJ) = robj;
                }
            }
        }
        /* Intermediate/Master: position only — store pivot, no level check */
        *(float *)((char *)ext + BRD_BRIDGE_PIVOT_X) = x;
        *(float *)((char *)ext + BRD_BRIDGE_PIVOT_Y) = y;
        *(float *)((char *)ext + BRD_BRIDGE_PIVOT_Z) = z;
        /* Fix 2026-10-21: arm N:BRIDGE here, at creation time. The S1-name
         * table (BuildCollisionFlags, postscan) can never enable it:
         * Intermediate's bridge S1 is named 'BRIDGE' (no N: prefix) while
         * N:BRIDGE exists only as an S6 event — and postscan runs before
         * this lazy creation anyway. Without the flag the rider mark
         * (ball+0x778) never fires and the bridge tilts but carries nothing. */
        if (brMesh) *(BYTE*)((char*)ext + COLL_FLAG_BRIDGE) = 1;
        if (!strstr(name, "(NOCOLLIDE)"))
            renderOut = *(int *)((char *)ext + UNI_SAW1_OBJ);
        OrBoardFeat(board, FEAT_BRIDGE_ANIM);
        /* Fix 2026-10-21: return (NULL, render) — pos-only. Returning the
         * MeshWorld* as obj poisoned the game's object list (wrong type in
         * a game-object list) and killed both balls on Intermediate. */
        obj = NULL;
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── JUDGE (Expert) ── */
    if (my_strnicmp(name, "JUDGE", 5) == 0) {
        void *mem = g_operatorNew(0x1100);
        if (mem) {
            obj = g_GearLevelCtor(mem, (int)board, x, y, z);
            g_AthenaListAppend((void*)((char*)ext + UNI_JUDGE_LIST), (int)obj);
            /* Native appends judges to board+0x4BBC (the live judge list,
             * walked by native scene code). Mirror it — ext-only judges
             * are never driven. */
            if (BoardHasOffset(board, 0x4BBC, 4) && g_AthenaListAppend) {
                if (!(GetBoardFeat(board) & FEAT_EXPERT_LISTS) && g_AthenaListInit) {
                    g_AthenaListInit((void*)((char*)board + 0x4380), 0);
                    g_AthenaListInit((void*)((char*)board + 0x4798), 0);
                    g_AthenaListInit((void*)((char*)board + 0x4BBC), 0);
                    OrBoardFeat(board, FEAT_EXPERT_LISTS);
                }
                g_AthenaListAppend((void*)((char*)board + 0x4BBC), (int)obj);
            }
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── BELL (Expert) ── */
    if (my_strnicmp(name, "BELL", 4) == 0) {
        void *mem = g_operatorNew(0x10E8);
        if (mem) {
            obj = g_BellCtor(mem, (int)board, x, y, z);
            *(void **)((char *)ext + UNI_BELL_OBJ) = obj;
            /* Native stores the bell at board+0x4FD4. Mirror it. */
            if (BoardHasOffset(board, 0x4FD4, 4)) *(void **)((char *)board + 0x4FD4) = obj;
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── DROPPER (Odd single lifer) — renamed from LIFTER (level==9) ──
     * S1: DROPPER — was bare LIFTER on Odd */
    if (my_strnicmp(name, "DROPPER", 7) == 0) {
        void *mem = g_operatorNew(0x10FC);
        if (mem) {
            obj = g_OddLifterCtor(mem, (int)board, x, y, z);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            /* Native Odd create stores the lifter at board+0x436C (read by
             * E:DROPLIFT 0x435170). Mirror it (preload-gap pattern). */
            if (BoardHasOffset(board, 0x436C, 4)) *(void **)((char *)board + 0x436C) = obj;
            *(void **)((char *)ext + UNI_BONK_STORE) = obj;
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── LIFTER (Odd vanilla alias) ── — vanilla Odd S1 is bare LIFTER -> Odd_Lifter_ctor when level==9 ── */
    if (my_strnicmp(name, "LIFTER", 6) == 0 && GetCurrentLevel(board) == 9 && my_strnicmp(name, "DROPPER", 7) != 0) {
        void *mem = g_operatorNew(0x10FC);
        if (mem) {
            obj = g_OddLifterCtor(mem, (int)board, x, y, z);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            if (BoardHasOffset(board, 0x436C, 4)) *(void **)((char *)board + 0x436C) = obj;
            *(void **)((char *)ext + UNI_BONK_STORE) = obj;
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── LIFTER (Up tubes) ──
     * S1: LIFTER2, LIFTER3... — Up only, level-gate removed (name-driven) */
    if (my_strnicmp(name, "LIFTER", 6) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_LIFTER_MESH, "Levels\\LevelUp-Lifter");
        long num = atol(name + 6);
        int meshVal = *(int*)((char*)ext + UNI_LIFTER_MESH);
        if (!meshVal) { DebugLog("LIFTER: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        /* Mirror into the native mesh slot (native Up ctor +0x4784). Native
         * Up create reads this slot as the Lifter_ctor mesh arg. */
        if (BoardHasOffset(board, 0x4784, 4)) {
            int natM = *(int *)((char *)board + 0x4784);
            if (!natM || IsBadReadPtr((void *)natM, 4))
                *(int *)((char *)board + 0x4784) = meshVal;
        }
        void *mem = g_operatorNew(0x10F4);
        if (mem) {
            obj = g_LifterCtor(mem, (int)board, x, y, z, meshVal, num);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            /* NO dual-append to board+0x436C: native Up create appends
             * lifters ONLY to 0x2578 — the 0x436C list stays empty natively.
             * Up RaceState (0x420660) calls vtable[0] (deleting dtor
             * 0x43D400!) on each 0x436C member with a garbage flag, so
             * appending live lifters there deletes them and crashes.
             * Lifters are driven via the shared 0x2578 walk (vtable[11]). */
            renderOut = ((DWORD*)obj)[0x438];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── SPINNY (Toob) ── */
    if (my_strnicmp(name, "SPINNY", 6) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_SPINNY_MESH, "Levels\\Level8-Spinny");
        int meshVal = *(int*)((char*)ext + UNI_SPINNY_MESH);
        if (!meshVal) { DebugLog("SPINNY: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1508);
        if (mem) {
            obj = g_RotatorImpossibleCtor(mem, (int)board, x, y, z, meshVal);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── SAW (Toob) ── */
    if (my_stricmp(name, "SAW") == 0 && difficulty != 0) {
        int pathObj = meshWorld ? g_LevelFindObjectByName(meshWorld, "SAWPATH") : 0;
        S1EnsureMeshWorld(board, ext, UNI_SAW_MESH, "Levels\\Level8-Saw");
        int meshVal = *(int*)((char*)ext + UNI_SAW_MESH);
        if (!meshVal) { DebugLog("SAW: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1110);
        if (mem) {
            obj = g_SawCtor(mem, (int)board, x, y, z, meshVal, pathObj);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            /* Native stores the saw at board+0x4380 (read by E:BRANCH).
             * Mirror it (preload-gap pattern). */
            if (BoardHasOffset(board, 0x4380, 4)) *(void **)((char *)board + 0x4380) = obj;
            *(void **)((char *)ext + UNI_SAW_TOOB_OBJ) = obj;
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── SAW2 (Toob) ── */
    if (my_stricmp(name, "SAW2") == 0 && difficulty != 0) {
        int pathObj = meshWorld ? g_LevelFindObjectByName(meshWorld, "SMALLSAWPATH") : 0;
        S1EnsureMeshWorld(board, ext, UNI_SAW_MESH, "Levels\\Level8-Saw");
        int meshVal = *(int*)((char*)ext + UNI_SAW_MESH);
        if (!meshVal) { DebugLog("SAW2: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1118);
        if (mem) {
            obj = g_Saw2Ctor(mem, (int)board, x, y, z, meshVal, pathObj);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            /* Native stores saw2 at board+0x4384 (read by E:ALERTSAW2). */
            if (BoardHasOffset(board, 0x4384, 4)) *(void **)((char *)board + 0x4384) = obj;
            *(void **)((char *)ext + UNI_SAW2_TOOB_OBJ) = obj;
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── FALLOUT1 (Toob) ── */
    if (my_strnicmp(name, "FALLOUT1", 8) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_FALLOUT_MESH, "Levels\\Level8-Fallout");
        int meshVal = *(int*)((char*)ext + UNI_FALLOUT_MESH);
        if (!meshVal) { DebugLog("FALLOUT1: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x10E8);
        if (mem) {
            obj = g_FalloutCtor(mem, (int)board, x, y, z, meshVal);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            /* Native stores the fallout at board+0x4388. */
            if (BoardHasOffset(board, 0x4388, 4)) *(void **)((char *)board + 0x4388) = obj;
            *(void **)((char *)ext + UNI_FALLOUT_OBJ) = obj;
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── BLOCKDAWG1/2/3 (Toob) ──
     * Native mesh mapping (Ghidra-verified 0x40FB30): dawg1 AND dawg2 both
     * ride the Blockdawg1 mesh (+0x4378); dawg3 rides Blockdawg2 (+0x437C).
     * Native stores NOTHING to board (append 0x2578 only); dawg3 gets
     * +0x1152=1 and is the only branch that sets out params. */
    if (my_strnicmp(name, "BLOCKDAWG", 9) == 0 && difficulty != 0) {
        int dawgNum = name[9] - '0';
        int meshOff = (dawgNum==3) ? UNI_BLOCKDAWG3_MESH : UNI_BLOCKDAWG1_MESH;
        { const char *_bdPath = (dawgNum==3) ? "Levels\\Level8-BlockDawg2" : "Levels\\Level8-BlockDawg1"; S1EnsureMeshWorld(board, ext, meshOff, _bdPath); }
        void* bExt2 = GetBoardExt(board); if (!bExt2) bExt2 = ext;
        int meshVal = bExt2 ? *(int*)((char*)bExt2 + meshOff) : *(int*)((char*)ext + meshOff);
        if (!meshVal) { DebugLog("BLOCKDAWG: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        char pathName[] = "DAWGPATH0";
        pathName[8] = '0' + dawgNum;
        int pathObj = meshWorld ? g_LevelFindObjectByName(meshWorld, pathName) : 0;
        void *mem = g_operatorNew(0x1154);
        if (mem) {
            obj = g_BlockdawgCtor(mem, (int)board, x, y, z, meshVal, pathObj);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x435];
            if (dawgNum == 3) *(char*)((char*)obj + 0x1152) = 1;
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── WOBBLY1-7 (Wobbly) ── */
    if (my_strnicmp(name, "WOBBLY", 6) == 0 && name[6] >= '1' && name[6] <= '7') {
        int wNum = name[6] - '0';
        int meshOff = UNI_WOBBLY_BASE + (wNum-1) * 4;
        { const char *_wPath = (wNum==1) ? "Levels\\Level7-Wobbly1" : (wNum==2) ? "Levels\\Level7-Wobbly2" : (wNum==3) ? "Levels\\Level7-Wobbly3" : (wNum==4) ? "Levels\\Level7-Wobbly4" : (wNum==5) ? "Levels\\Level7-Wobbly5" : (wNum==6) ? "Levels\\Level7-Wobbly6" : "Levels\\Level7-Wobbly7"; S1EnsureMeshWorld(board, ext, meshOff, _wPath); }
        void* wExt = GetBoardExt(board); if (!wExt) wExt = ext;
        int meshVal = wExt ? *(int*)((char*)wExt + meshOff) : *(int*)((char*)ext + meshOff);
        if (!meshVal) { DebugLog("WOBBLY: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1524);
        if (mem) {
            obj = g_GameLevelCtor(mem, (int)board, x, y, z, meshVal);
            DWORD *o = (DWORD *)obj;
            renderOut = o[0x435];
            /* Per-wobbly constants (Ghidra-verified 0x40F420). NOTE WOBBLY3
             * sets byte +0x1104 (NOT +0x1105 like the others). */
            static const DWORD w43a[] = {0,0,0x41000000,0,0x41700000,0x41700000,0x41200000,0x41A00000};
            static const DWORD w43b[] = {0,0,0,0x41C80000,0,0,0x41200000,0};
            static const DWORD w440[] = {0,0,0x42C80000,0x43160000,0x432F0000,0x431B0000,0x43160000,0x42C80000};
            static const int w1105[] = {0,0,1,0,1,1,0,1};
            o[0x43A] = w43a[wNum]; o[0x43B] = w43b[wNum]; o[0x440] = w440[wNum];
            if (wNum == 3) *(char*)((char*)obj + 0x1104) = 1;
            else if (w1105[wNum]) *(char*)((char*)obj + 0x1105) = 1;
            /* Native ctor slot feeds GameLevel_ctor — mirror it. */
            if (BoardHasOffset(board, 0x436C + (wNum-1) * 4, 4))
                *(void **)((char *)board + 0x436C + (wNum-1) * 4) = (void *)meshVal;
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── WAVY1 (Wobbly) ── */
    if (my_strnicmp(name, "WAVY1", 5) == 0) {
        int gfx = *(int *)(app + 0x174);
        if (gfx) *(char *)(gfx + 2000) = 1;
        void *mem = g_operatorNew(0x1AE7C);
        if (mem) {
            obj = g_WavyCtor(mem, (int)board, x, y, z, "Levels\\Level7-Wavy1");
            g_WavyConfigure(obj, 0x1C, 0x41A00000, 0x40000000, 0xC0400000);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x435];
        }
        if (gfx) *(char *)(gfx + 2000) = 0;
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── NEONPLATFORM (Neon) ── */
    if (my_strnicmp(name, "NEONPLATFORM", 12) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_SAW2_OBJ, "Levels\\LevelDark-NeonPlatform");
        int meshVal = *(int*)((char*)ext + UNI_SAW2_OBJ);
        if (!meshVal) { DebugLog("NEONPLATFORM: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        /* Native ctor slot board+0x4374 feeds NeonPlatform_ctor. Mirror it. */
        if (BoardHasOffset(board, 0x4374, 4)) *(void **)((char *)board + 0x4374) = (void *)meshVal;
        void *mem = g_operatorNew(0x10EC);
        if (mem) {
            obj = g_NeonPlatformCtor(mem, (int)board, x, y, z, meshVal);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── DFLOOR1-4 (Neon) ── */
    if (my_strnicmp(name, "DFLOOR", 6) == 0 && name[6] >= '1' && name[6] <= '4') {
        int dNum = name[6] - '0';
        int meshOff = UNI_DFLOOR_BASE + (dNum-1) * 4;
        { const char *_dfPath = (dNum==1) ? "Levels\\LevelDark-DFloor1" : (dNum==2) ? "Levels\\LevelDark-DFloor2" : (dNum==3) ? "Levels\\LevelDark-DFloor3" : "Levels\\LevelDark-DFloor4"; S1EnsureMeshWorld(board, ext, meshOff, _dfPath); }
        void *dExt = GetBoardExt(board); if (!dExt) dExt = ext;
        int meshVal = dExt ? *(int*)((char*)dExt + meshOff) : *(int*)((char*)ext + meshOff);
        if (!meshVal) { DebugLog("DFLOOR: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1104);
        if (mem) {
            obj = g_ArenaStandsCtor(mem, (int)board, x, y, z, meshVal);
            DWORD *o = (DWORD *)obj;
            renderOut = o[0x43A];
            /* Native ctor slots +0x4378/7C/80/84 feed the DFLOOR ctors.
             * Mirror them (preload-gap pattern). */
            if (BoardHasOffset(board, 0x4378 + (dNum-1) * 4, 4))
                *(void **)((char *)board + 0x4378 + (dNum-1) * 4) = (void *)meshVal;
            if (dNum == 4) {
                *(void **)((char *)ext + UNI_NEON_DARK_COUNT) = obj;
                /* Native stores DFLOOR4 obj at board+0x438C (re-appended by
                 * E:LIGHTSOFF when the dark counter cycles). Mirror it. */
                if (BoardHasOffset(board, 0x438C, 4)) *(void **)((char *)board + 0x438C) = obj;
                o[0x437] = 2;
                if (obj) {
                    *(DWORD *)((char *)obj + 0x10E0) = 0;
                }
            } else {
                g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            }
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── TRODE (Neon) ── */
    if (my_strnicmp(name, "TRODE", 5) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_BRIDGE_COUNTER, "Levels\\LevelDark-Trode");
        int meshVal = *(int*)((char*)ext + UNI_BRIDGE_COUNTER);
        if (!meshVal) { DebugLog("TRODE: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        /* Native ctor slot board+0x4388 feeds the Trode ctor. Mirror it. */
        if (BoardHasOffset(board, 0x4388, 4)) *(void **)((char *)board + 0x4388) = (void *)meshVal;
        void *mem = g_operatorNew(0x1104);
        if (mem) {
            obj = g_ArenaStandsCtor(mem, (int)board, x, y, z, meshVal);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x43A];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── POPCYLINDER (Sky) ── */
    if (my_strnicmp(name, "POPCYLINDER", 11) == 0) {
        if (difficulty != 0) {
            /* Sky */
            long idx = atol(name + 11) - 1;
            if (idx >= 0 && idx < 16) {
                int meshIdx = idx & 1;
                int meshOff = UNI_POPCYL_MESH_BASE + meshIdx * 4;
                { const char *_pcPath = (meshIdx==0) ? "Levels\\Level9-PopCylinder1" : "Levels\\Level9-PopCylinder2"; S1EnsureMeshWorld(board, ext, meshOff, _pcPath); }
                void* pExt = GetBoardExt(board); if (!pExt) pExt = ext;
                int meshVal = pExt ? *(int*)((char*)pExt + meshOff) : *(int*)((char*)ext + meshOff);
                if (!meshVal) { DebugLog("POPCYLINDER: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
                void *mem = g_operatorNew(0x10F4);
                if (mem) {
                    obj = g_PopcylinderCtor(mem, (int)board, x, y, z, meshVal);
                    *(void **)((char *)ext + UNI_SKY_POPCYL_BASE + idx * 4) = obj;
                    g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
                    renderOut = ((DWORD*)obj)[0x438];
                }
            }
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── POPDOOR/TRAPDOOR (Sky Rotator, level==13) ──
     * S1 vanilla name is TRAPDOOR (Sky_CreateDynamicObjects 0x410AD0 line 79); POPDOOR accepted for new files */
    if ((my_strnicmp(name, "POPDOOR", 7) == 0) || (my_strnicmp(name, "TRAPDOOR", 8) == 0 && GetCurrentLevel(board) == 13)) {
        float dat = *(float *)(g_moduleBase + 0xCF44C);
        S1EnsureMeshWorld(board, ext, UNI_SKY_TRAPDOOR, "Levels\\Level9-TrapDoor");
        int meshVal = *(int*)((char*)ext + UNI_SKY_TRAPDOOR);
        if (!meshVal) { DebugLog("POPDOOR: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x10F4);
        if (mem) {
            obj = g_RotatorCtor(mem, (int)board, x, y, z, dat - fparam, meshVal);
            *(void **)((char *)ext + UNI_NEON_TRAPDOOR) = obj;
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            g_AthenaListAppend((void*)((char*)ext + UNI_TRAPDOOR_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x43C];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── SPEEDCYLINDER (Up) ── */
    if (my_strnicmp(name, "SPEEDCYLINDER", 13) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_SPEEDCYLINDER_MESH, "Levels\\LevelUp-SpeedCylinder");
        int meshVal = *(int*)((char*)ext + UNI_SPEEDCYLINDER_MESH);
        if (!meshVal) { DebugLog("SPEEDCYLINDER: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        /* Mirror into the native mesh slot (native Up ctor +0x4788). */
        if (BoardHasOffset(board, 0x4788, 4)) {
            int natM = *(int *)((char *)board + 0x4788);
            if (!natM || IsBadReadPtr((void *)natM, 4))
                *(int *)((char *)board + 0x4788) = meshVal;
        }
        void *mem = g_operatorNew(0x150C);
        if (mem) {
            /* numArg is (int)(360.0 - s1_y2): native does FLD [0x4CF44C]
             * (=360.0) / FSUB [s1data+0x14] / __ftol2 (Up create 0x41189B).
             * fparam here is the same s1data+0x14 float. */
            int numArg = (int)(360.0f - fparam);
            obj = g_SpeedCylinderCtor(mem, (int)board, x, y, z, numArg, meshVal);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x438];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── TIMEBUTTON (Up) ── */
    if (my_strnicmp(name, "TIMEBUTTON", 10) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_TIMEBUTTON_MESH, "Levels\\LevelUp-Button");
        int meshVal = *(int*)((char*)ext + UNI_TIMEBUTTON_MESH);
        if (!meshVal) { DebugLog("TIMEBUTTON: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        /* Mirror into the native mesh slot (native Up ctor +0x478C). */
        if (BoardHasOffset(board, 0x478C, 4)) {
            int natM = *(int *)((char *)board + 0x478C);
            if (!natM || IsBadReadPtr((void *)natM, 4))
                *(int *)((char *)board + 0x478C) = meshVal;
        }
        void *mem = g_operatorNew(0x10E8);
        if (mem) {
            obj = g_TimeButtonCtor(mem, (int)board, x, y, z, meshVal);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x438];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── LOOPER (Impossible) ── */
    if (my_strnicmp(name, "LOOPER", 6) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_LOOPER_MESH, "Levels\\LevelImpossible-Looper");
        int meshVal = *(int*)((char*)ext + UNI_LOOPER_MESH);
        if (!meshVal) { DebugLog("LOOPER: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1500);
        if (mem) {
            obj = g_LooperCtor(mem, (int)board, x, y, z, meshVal);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── GEAR (Impossible) ── */
    if (my_strnicmp(name, "GEAR", 4) == 0 && my_strnicmp(name, "BIGGEAR", 7) != 0) {
        S1EnsureMeshWorld(board, ext, UNI_GEAR_MESH, "Levels\\LevelImpossible-Gear");
        int meshVal = *(int*)((char*)ext + UNI_GEAR_MESH);
        if (!meshVal) { DebugLog("GEAR: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1514);
        if (mem) {
            obj = g_GearCtor(mem, (int)board, x, y, z, x2, y2, z2, meshVal);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── BIGGEAR (Impossible) ── */
    if (my_strnicmp(name, "BIGGEAR", 7) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_BIGGEAR_MESH, "Levels\\LevelImpossible-BigGear");
        int meshVal = *(int*)((char*)ext + UNI_BIGGEAR_MESH);
        if (!meshVal) { DebugLog("BIGGEAR: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1514);
        if (mem) {
            obj = g_GearCtor(mem, (int)board, x, y, z, x2, y2, z2, meshVal);
            DWORD *o = (DWORD *)obj;
            o[0x43D] = 0x3F000000;
            if (strstr(name, "TOUCH")) *(char*)((char*)obj + 0x544) = 1;
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = o[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── ROTATOR (Impossible) ── */
    if (my_strnicmp(name, "ROTATOR", 7) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_ROTATOR_MESH, "Levels\\LevelImpossible-Rotator");
        int meshVal = *(int*)((char*)ext + UNI_ROTATOR_MESH);
        if (!meshVal) { DebugLog("ROTATOR: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1508);
        if (mem) {
            obj = g_RotatorImpossibleCtor(mem, (int)board, x, y, z, meshVal);
            DWORD *o = (DWORD *)obj;
            o[0x43A] = 0x3F800000;
            if (g_RNG && RNG_call((void*)0x4F7360, 0, 2, 0) == 0)
                o[0x43A] = 0xBF800000;
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = o[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── PENDULUM (Impossible) ── */
    if (my_strnicmp(name, "PENDULUM", 8) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_PENDULUM_MESH, "Levels\\LevelImpossible-Pendulum");
        int meshVal = *(int*)((char*)ext + UNI_PENDULUM_MESH);
        if (!meshVal) { DebugLog("PENDULUM: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1504);
        if (mem) {
            obj = g_PendulumCtor(mem, (int)board, x, y, z, meshVal);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── BBRIDGE1/2 (Master) ── */
    if (my_strnicmp(name, "BBRIDGE", 7) == 0) {
        int bNum = name[7] - '0';
        int meshOff = (bNum == 1) ? UNI_BBRIDGE1_MESH : UNI_BBRIDGE2_MESH;
        int storeOff = (bNum == 1) ? UNI_BBRIDGE1_OBJ : UNI_BBRIDGE2_OBJ;
        S1EnsureMeshWorld(board, ext, meshOff, (bNum == 1) ? "Levels\\Level10-Bridge1" : "Levels\\Level10-Bridge2");
        void* bbExt2 = GetBoardExt(board); if (!bbExt2) bbExt2 = ext;
        int meshVal = bbExt2 ? *(int*)((char*)bbExt2 + meshOff) : *(int*)((char*)ext + meshOff);
        if (!meshVal) { DebugLog("BBRIDGE: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1100);
        if (mem) {
            obj = g_BreakBridgeCtor(mem, (int)board, x, y, z, meshVal);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            void* bbExt = GetBoardExt(board); if(!bbExt) bbExt=ext;
            if(bbExt) *(void **)((char *)bbExt + storeOff) = obj;
            renderOut = ((DWORD*)obj)[0x438];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── SMASHER1/2 (Glass) ──
     * Writes position floats to dedicated REND_GLASS_* offsets.
     * Original game stores 3 floats (X,Y,Z) at board+0x436C/0x4370/0x4374.
     * Render block passes them to Timer vtable[0x08](timer, X, Y, Z). */
    if (my_strnicmp(name, "SMASHER1", 8) == 0) {
        *(float *)((char *)ext + REND_GLASS_S1_X) = x;
        *(float *)((char *)ext + REND_GLASS_S1_Y) = y;
        *(float *)((char *)ext + REND_GLASS_S1_Z) = z;
        *(DWORD *)((char *)ext + REND_GLASS_TRANSP1) = 0;
        *(char *)((char *)ext + REND_GLASS_FLAG1) = 0;
        /* Native mirrors (Ghidra-verified 0x40AD80): S1 DAT floats to
         * board+0x436C/70/74, zero +0x4384, clear flag +0x438C. */
        if (BoardHasOffset(board, 0x436C, 16)) {
            *(float *)((char *)board + 0x436C) = x;
            *(float *)((char *)board + 0x4370) = y;
            *(float *)((char *)board + 0x4374) = z;
            *(DWORD *)((char *)board + 0x4384) = 0;
        }
        if (BoardHasOffset(board, 0x438C, 1)) *(BYTE *)((char *)board + 0x438C) = 0;
        *(int*)out1 = 0; *(int*)out2 = 0;
        return;
    }
    if (my_strnicmp(name, "SMASHER2", 8) == 0) {
        *(float *)((char *)ext + REND_GLASS_S2_X) = x;
        *(float *)((char *)ext + REND_GLASS_S2_Y) = y;
        *(float *)((char *)ext + REND_GLASS_S2_Z) = z;
        *(DWORD *)((char *)ext + REND_GLASS_TRANSP2) = 0xC2B40000; /* -90.0f */
        *(char *)((char *)ext + REND_GLASS_FLAG2) = 0;
        /* Native mirrors: S1 DAT floats to +0x4378/7C/80, -90.0f timer
         * +0x4388, clear flag +0x438D. */
        if (BoardHasOffset(board, 0x4378, 16)) {
            *(float *)((char *)board + 0x4378) = x;
            *(float *)((char *)board + 0x437C) = y;
            *(float *)((char *)board + 0x4380) = z;
            *(float *)((char *)board + 0x4388) = -90.0f;
        }
        if (BoardHasOffset(board, 0x438D, 1)) *(BYTE *)((char *)board + 0x438D) = 0;
        *(int*)out1 = 0; *(int*)out2 = 0;
        return;
    }

    /* ── TARBUBBLE (Dizzy) ──
     * S1: TARBUBBLE — DizzyBoard_Update uses board+0x11E4 list.
     * Mod stores into ext+UNI_TARBUBBLE_LIST (0x9430) so Feature_SwirlZones can spawn bubbles. */
    if (my_strnicmp(name, "TARBUBBLE", 9) == 0) {
        void *entry = g_operatorNew(0x20);
        if (entry) {
            memset(entry, 0, 0x20);
            *(float *)((char *)entry + 0x04) = x;
            *(float *)((char *)entry + 0x08) = y;
            *(float *)((char *)entry + 0x0C) = z;
            g_AthenaListAppend((void *)((char *)ext + UNI_TARBUBBLE_LIST), (int)entry);
        }
        *(int*)out1 = 0; *(int*)out2 = 0;
        return;
    }

    /* Unknown object — no-op, but label it (entry log at fn top already
     * printed the full name; this marks the fallthrough as a deliberate
     * skip so future S1 additions are diagnosable, not silent). */
    {
        char udbg[96];
        int ulen = 0;
        while (name[ulen] && ulen < 48) ulen++;
        {
            int ui;
            char *dst = udbg;
            const char *prefix = "CreateDynamicObjects: unknown S1 '";
            while (*prefix) *dst++ = *prefix++;
            for (ui = 0; ui < ulen; ui++) *dst++ = name[ui];
            *dst++ = '\''; *dst++ = '\0';
        }
        DebugLog(udbg);
    }
    *(int*)out1 = 0; *(int*)out2 = 0;
}

/* CENSUS wrapper (object audit 2026-10-24): every CreateDynamicObjects
 * call on every board funnels through here, so the log holds a complete
 * per-race object census (EXPECTED = S1REF lines, ACTUAL = these lines).
 * vtable slot 33 points at this wrapper; it delegates to the Impl above
 * (fast-path native orig or our mirrors) then logs name + out params. */
void __thiscall UniversalCreateDynamicObjects(void *board, char *name, void *out1, void *out2, int *s1data) {
    if (!name || !out1 || !out2 || !s1data) return;
    UniversalCreateDynamicObjectsImpl(board, name, out1, out2, s1data);
    {
        void *censext = GetBoardExt(board);
        int cslot = censext ? ContentFileId(censext) : 0;
        int clevel = GetCurrentLevel(board);
        char censdbg[256];
        wsprintfA(censdbg, "CREATE census: name='%.48s' slot=%d content=%d out1=0x%08X out2=0x%08X",
            name, clevel, cslot, *(DWORD *)out1, *(DWORD *)out2);
        DebugLog(censdbg);
    }
}
