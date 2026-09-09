/* Chapter 5 - S1Ensure helpers + UniversalCreateDynamicObjects (LevelFeatures.c lines 3418-4417) */
/* ═══════════════════════════════════════════════════════════════════════════
 * Universal CreateDynamicObjects (Slot 33) — replaces all 15 per-level handlers
 *
 * The game calls this for each named mesh object found in the MESHWORLD.
 * Each per-level handler matches its own object names (BRIDGE, TIPPER, MACE, etc.)
 * and creates the appropriate game object via its ctor.
 *
 * Since N:/E: names are unique per level's mesh, there are zero conflicts.
 * We save the original per-level handlers and delegate to them, so each
 * level's objects are created exactly as the original game intended.
 * ═══════════════════════════════════════════════════════════════════════════ */

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal CreateDynamicObjects (Slot 33) — replaces all 15 per-level handlers
 *
 * Matches S1 object names and calls the appropriate ctor directly.
 * This enables cross-level object injection — any object from any level
 * can be used on any other level via LevelData.txt mesh configuration.
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* S1EnsureMeshWorld(void* board, void* ext, DWORD offset, const char* path) {
    void* cur = *(void**)((char*)ext + offset);
    if (cur) return cur;
    DWORD app = *(DWORD*)((char*)board + BOARD_APP_PTR);
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
    DWORD app = *(DWORD*)((char*)board + BOARD_APP_PTR);
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
    DWORD app = *(DWORD*)((char*)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void*)app, 0x200)) return NULL;
    void* gfx = *(void**)((char*)app + 0x174);
    if (!gfx || !g_operatorNew || !g_SpriteCtor) return NULL;
    void* mem = g_operatorNew(0x110);
    if (!mem) return NULL;
    void* spr = g_SpriteCtor(mem, gfx, path);
    if (spr) *(void**)((char*)ext + offset) = spr;
    return spr;
}

void __thiscall UniversalCreateDynamicObjects(void *board, char *name, void *out1, void *out2, int *s1data) {
    if (!name || !out1 || !out2 || !s1data) return;
    int level = GetCurrentLevel(board);
    if (level == 0 || level > 15) { *(int*)out1 = 0; *(int*)out2 = 0; return; }
    void* ext = EnsureBoardExt(board);
    if (!ext) { *(int*)out1 = 0; *(int*)out2 = 0; return; }

    {
        char dbg[256];
        wsprintfA(dbg, "CreateDynamicObjects: level=%d name='%s' board=0x%08X s1data=0x%08X", level, name, (DWORD)board, (DWORD)s1data);
        DebugLog(dbg);
    }

    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
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
        if (!obj) { obj = *(void **)((char *)ext + UNI_MESH_0); renderOut = *(int *)((char *)ext + UNI_MESH_1); }
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
        obj = *(void **)((char *)ext + UNI_MESH_6);
        renderOut = *(int *)((char *)ext + UNI_MESH_7);
        /* Store swirl position for step3b mesh rotation */
        *(float *)((char *)ext + UNI_MESH_15) = x;
        *(float *)((char *)ext + UNI_MESH_12) = y;
        *(float *)((char *)ext + UNI_MESH_13) = z;
        OrBoardFeat(board, FEAT_SWIRL);
        // Mirror for legacy OFF_* readers (alias, same addr) — use outer ext, no shadow.
        if (ext) { *(void**)((char*)ext+OFF_SWIRL_MESH)=obj; *(int*)((char*)ext+OFF_SWIRL_RENDER)=renderOut; *(float*)((char*)ext+OFF_SWIRL_POS_X)=x; *(float*)((char*)ext+OFF_SWIRL_POS_Y)=y; *(float*)((char*)ext+OFF_SWIRL_POS_Z)=z; }
        // Create swirl collision zone for Feature_SwirlZones proximity check
        {
            void* zoneMem = g_operatorNew(0x1110);
            if (zoneMem) {
                memset(zoneMem, 0, 0x1110);
                *(float*)((char*)zoneMem + 0x10E0) = x;
                *(float*)((char*)zoneMem + 0x10E4) = y;
                *(float*)((char*)zoneMem + 0x10E8) = z;
                *(float*)((char*)zoneMem + 0x1100) = 2.5f; // radius factor -> 150 units (*60)
                if (ext) g_AthenaListAppend((void*)((char*)ext + UNI_SWIRL_LIST), (int)zoneMem);
                else g_AthenaListAppend((void*)((char*)board + UNI_SWIRL_LIST), (int)zoneMem);
            }
        }
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
        void *mem = g_operatorNew(0x110C);
        if (mem) {
            obj = g_GluebieCtor(mem, (int)board, meshVal);
            DWORD *o = (DWORD *)obj;
            memcpy(&o[0x435], &x, 4); memcpy(&o[0x436], &y, 4); memcpy(&o[0x437], &z, 4);
            /* Original appends to board+0x4378 (Gluebie list) + board+0x2578 (obj list).
             * UNI_MESH_3 (0x85EC) is a mesh SLOT, not an AthenaList — appending to it
             * clobbers mesh pointers at 0x8620-0x862C via the iter array. */
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            /* Restore missing Gluebie list append (board+0x4378) — original does both */
            g_AthenaListAppend((void*)((char*)board + 0x4378), (int)obj);
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── CATAPULT (Tower) ── */
    if (my_strnicmp(name, "CATAPULT", 8) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_CATAPULT_MESH, "Levels\\Level4-Catapult");
        int meshOff = UNI_CATAPULT_MESH;
        int meshVal = *(int*)((char*)ext + meshOff);
        if (!meshVal) { DebugLog("CATAPULT: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1108);
        if (mem) {
            obj = g_CatapultCtor(mem, (int)board, meshVal);
            DWORD *o = (DWORD *)obj;
            memcpy(&o[0x436], &x, 4); memcpy(&o[0x437], &y, 4); memcpy(&o[0x438], &z, 4);
            int listOff = UNI_CATAPULT_LIST;
            g_AthenaListAppend((void*)((char*)ext + listOff), (int)obj);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = o[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── MACE (Tower) ── */
    if (my_strnicmp(name, "MACE", 4) == 0 && difficulty != 0) {
        S1EnsureMeshWorld(board, ext, UNI_MACE_MESH, "Levels\\Level4-Mace");
        int meshVal = *(int*)((char*)ext + UNI_MACE_MESH);
        if (!meshVal) { DebugLog("MACE: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x110C);
        if (mem) {
            obj = g_MaceCtor(mem, (int)board, meshVal);
            DWORD *o = (DWORD *)obj;
            memcpy(&o[0x436], &x, 4); memcpy(&o[0x437], &y, 4); memcpy(&o[0x438], &z, 4);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            g_AthenaListAppend((void*)((char*)ext + UNI_MACE_LIST), (int)obj);
            if (g_AthenaListGetSize((void*)((char*)ext + UNI_MACE_LIST)) == 1) {
                o[0x43A] = 0x42A00000; o[0x43D] = 1; o[0x43E] = 0x32;
            }
            renderOut = o[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── DRAWBRIDGE (Tower) ── */
    if (my_strnicmp(name, "DRAWBRIDGE", 10) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_DRAWBRIDGE_MESH, "Levels\\Level4-Drawbridge");
        int meshVal = *(int*)((char*)ext + UNI_DRAWBRIDGE_MESH);
        if (!meshVal) { DebugLog("DRAWBRIDGE: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x113C);
        if (mem) {
            obj = g_GlassLevelCtor(mem, (int)board, meshVal);
            DWORD *o = (DWORD *)obj;
            memcpy(&o[0x436], &x, 4); memcpy(&o[0x437], &y, 4); memcpy(&o[0x438], &z, 4);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            g_AthenaListAppend((void*)((char*)ext + UNI_DRAWBRIDGE_LIST), (int)obj);
            renderOut = o[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── WINDMILL (Tower) ── */
    if (my_strnicmp(name, "WINDMILL", 8) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_WINDMILL_MESH, "Levels\\Level4-Windmill");
        int mesh = *(int *)((char *)ext + UNI_WINDMILL_MESH);
        if (!mesh) { DebugLog("WINDMILL: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x10D0);
        if (mem) {
            void *render = g_LevelRenderCtor(mem, (void*)mesh);
            g_TipperVisualAttach(render, (void*)mesh);
            renderOut = (int)render;
            /* Store render obj in dedicated render offset for REND_WINDMILL */
            *(DWORD *)((char *)ext + REND_TOWER_WINDMILL) = (DWORD)render;
        }
        *(float *)((char *)ext + UNI_WINDMILL_X) = x;
        *(float *)((char *)ext + UNI_WINDMILL_Y) = y;
        *(float *)((char *)ext + UNI_WINDMILL_Z) = z;
        if (g_RNG) *(float *)((char *)ext + UNI_WINDMILL_ANGLE) = (float)RNG_call((void*)0x4F7360, 0, 0x168, 0);
        *(int*)out1 = mesh; *(int*)out2 = renderOut;
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
        /* Store position in dedicated tower render offsets */
        *(float *)((char *)ext + REND_TOWER_CHOMP_X) = x;
        *(float *)((char *)ext + REND_TOWER_CHOMP_Y) = y;
        *(float *)((char *)ext + REND_TOWER_CHOMP_Z) = z;
        /* Adjust Y by constant (original subtracts _DAT_004CF370) */
        float adj = *(float *)(g_moduleBase + 0xCF370);
        *(float *)((char *)ext + REND_TOWER_CHOMP_Y) -= adj;
        *(int*)out1 = 0; *(int*)out2 = 0;
        return;
    }

    /* ── TURRET tower (Tower) — not a gun, it's a tower ──
     * Original game (Tower_CreateDynamicObjects at 0x0040d7c0):
     *   1. operator_new(0x10D0) → Stands_ctor(mem, meshPtr)
     *   2. Timer_Init(stack local) — creates a Timer object
     *   3. Copy position (x,y,z) from S1 data to stack local struct
     *   4. Call TIMER vtable[2] (Gfx_SetPosition) with (x,y,z) as 3 float args
     *      Stands vtable[2] is SceneObject_BuildStrips — calling it hangs!
     *   5. Call STANDS vtable[0x15] (slot 21) with pointer to position struct
     *   6. Level_RenderCtor + TipperVisual_Attach
     *   7. Timer_Cleanup */
    if (my_strnicmp(name, "TURRET", 6) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_TURRET_MESH, "Levels\\Level4-Turret");
        void *meshPtr = *(void **)((char *)ext + UNI_TURRET_MESH);
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
            /* Call STANDS vtable[0x15] (slot 21) with pointer to position struct */
            DWORD *standsVtbl = *(DWORD **)stands;
            if (standsVtbl) {
                void (__fastcall *fn54)(DWORD, float *) = (void (__fastcall *)(DWORD, float *))standsVtbl[0x15];
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
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── SAWBLADE (Expert) ── */
    if (my_strnicmp(name, "SAWBLADE", 8) == 0 && difficulty != 0) {
        void *mem = g_operatorNew(0x111C);
        if (mem) {
            obj = g_SawBladeCtor(mem, (int)board, x, y, z);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            if (strstr(name, "1")) { g_SawBladeSetVariant(obj, 1); *(void **)((char *)ext + UNI_SAWBLADE1_OBJ) = obj; }
            if (strstr(name, "2")) { g_SawBladeSetVariant(obj, 2); *(void **)((char *)ext + UNI_SAWBLADE2_OBJ) = obj; }
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── SAWBRIDGE (Expert cut-away spinner) — renamed from BRIDGE (level==8) ──
     * S1: SAWBRIDGE1 / SAWBRIDGE2 (+ NEG) — was BRIDGE1/2 on Expert
     * BRIDGE (drawbridge) is now Intermediate/Master only, level-gate removed */
    if (my_strnicmp(name, "SAWBRIDGE", 9) == 0) {
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
        OrBoardFeat(board, FEAT_BRIDGE_ANIM);
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
            OrBoardFeat(board, FEAT_BRIDGE_ANIM);
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
        obj = *(void **)((char *)ext + UNI_BONK_STORE);
        *(float *)((char *)ext + BRD_BRIDGE_PIVOT_X) = x;
        *(float *)((char *)ext + BRD_BRIDGE_PIVOT_Y) = y;
        *(float *)((char *)ext + BRD_BRIDGE_PIVOT_Z) = z;
        if (!strstr(name, "(NOCOLLIDE)"))
            renderOut = *(int *)((char *)ext + UNI_SAW1_OBJ);
        OrBoardFeat(board, FEAT_BRIDGE_ANIM);
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── JUDGE (Expert) ── */
    if (my_strnicmp(name, "JUDGE", 5) == 0) {
        void *mem = g_operatorNew(0x1100);
        if (mem) {
            obj = g_GearLevelCtor(mem, (int)board, x, y, z);
            g_AthenaListAppend((void*)((char*)ext + UNI_JUDGE_LIST), (int)obj);
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
            *(void **)((char *)ext + UNI_BONK_STORE) = obj;
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── LIFTER (Odd vanilla alias) ── — vanilla Odd S1 is bare LIFTER -> Odd_Lifter_ctor when level==9 */
    if (my_strnicmp(name, "LIFTER", 6) == 0 && GetCurrentLevel(board) == 9 && my_strnicmp(name, "DROPPER", 7) != 0) {
        void *mem = g_operatorNew(0x10FC);
        if (mem) {
            obj = g_OddLifterCtor(mem, (int)board, x, y, z);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
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
        void *mem = g_operatorNew(0x10F4);
        if (mem) {
            obj = g_LifterCtor(mem, (int)board, x, y, z, meshVal, num);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            /* Dual-append to legacy board+0x436C for Up's RaceState handler.
             * Name-driven now — any LIFTER in any slot needs the list if meshed as Up.
             * Keep unconditional (Up RaceState iterates 0x436C to call Lifter_Update). */
            g_AthenaListAppend((void*)((char*)board + 0x436C), (int)obj);
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
        int pathObj = g_LevelFindObjectByName(meshWorld, "SAWPATH");
        S1EnsureMeshWorld(board, ext, UNI_SAW_MESH, "Levels\\Level8-Saw");
        int meshVal = *(int*)((char*)ext + UNI_SAW_MESH);
        if (!meshVal) { DebugLog("SAW: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1110);
        if (mem) {
            obj = g_SawCtor(mem, (int)board, x, y, z, meshVal, pathObj);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            *(void **)((char *)ext + UNI_SAW_TOOB_OBJ) = obj;
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── SAW2 (Toob) ── */
    if (my_stricmp(name, "SAW2") == 0 && difficulty != 0) {
        int pathObj = g_LevelFindObjectByName(meshWorld, "SMALLSAWPATH");
        S1EnsureMeshWorld(board, ext, UNI_SAW_MESH, "Levels\\Level8-Saw");
        int meshVal = *(int*)((char*)ext + UNI_SAW_MESH);
        if (!meshVal) { DebugLog("SAW2: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1118);
        if (mem) {
            obj = g_Saw2Ctor(mem, (int)board, x, y, z, meshVal, pathObj);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
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
            *(void **)((char *)ext + UNI_FALLOUT_OBJ) = obj;
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── BLOCKDAWG1/2/3 (Toob) ── */
    if (my_strnicmp(name, "BLOCKDAWG", 9) == 0 && difficulty != 0) {
        int dawgNum = name[9] - '0';
        int meshOff = (dawgNum==1) ? UNI_BLOCKDAWG1_MESH : (dawgNum==3) ? UNI_BLOCKDAWG3_MESH : UNI_BLOCKDAWG2_MESH;
        { const char *_bdPath = (dawgNum==1) ? "Levels\\Level8-BlockDawg1" : (dawgNum==3) ? "Levels\\Level8-BlockDawg2" : "Levels\\Level8-BlockDawg2"; S1EnsureMeshWorld(board, ext, meshOff, _bdPath); }
        void* bExt2 = GetBoardExt(board); if (!bExt2) bExt2 = ext;
        int meshVal = bExt2 ? *(int*)((char*)bExt2 + meshOff) : *(int*)((char*)ext + meshOff);
        if (!meshVal) { DebugLog("BLOCKDAWG: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        char pathName[] = "DAWGPATH0";
        pathName[8] = '0' + dawgNum;
        int pathObj = g_LevelFindObjectByName(meshWorld, pathName);
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
            /* Per-wobbly constants */
            static const DWORD w43a[] = {0,0,0x41000000,0,0x41700000,0x41700000,0x41200000,0x41A00000};
            static const DWORD w43b[] = {0,0,0,0x41C80000,0,0,0x41200000,0};
            static const DWORD w440[] = {0,0,0x42C80000,0x43160000,0x432F0000,0x431B0000,0x43160000,0x42C80000};
            static const int w1105[] = {0,0,1,0,1,1,0,1};
            o[0x43A] = w43a[wNum]; o[0x43B] = w43b[wNum]; o[0x440] = w440[wNum];
            if (w1105[wNum]) *(char*)((char*)obj + 0x1105) = 1;
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
            if (dNum == 4) {
                *(void **)((char *)ext + UNI_NEON_DARK_COUNT) = obj;
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
        void *mem = g_operatorNew(0x150C);
        if (mem) {
            /* numArg from __ftol2 — use 0 as fallback */
            obj = g_SpeedCylinderCtor(mem, (int)board, x, y, z, 0, meshVal);
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
        *(int*)out1 = 0; *(int*)out2 = 0;
        return;
    }
    if (my_strnicmp(name, "SMASHER2", 8) == 0) {
        *(float *)((char *)ext + REND_GLASS_S2_X) = x;
        *(float *)((char *)ext + REND_GLASS_S2_Y) = y;
        *(float *)((char *)ext + REND_GLASS_S2_Z) = z;
        *(DWORD *)((char *)ext + REND_GLASS_TRANSP2) = 0xC2B40000; /* -90.0f */
        *(char *)((char *)ext + REND_GLASS_FLAG2) = 0;
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

    /* Unknown object — no-op */
    *(int*)out1 = 0; *(int*)out2 = 0;
}
