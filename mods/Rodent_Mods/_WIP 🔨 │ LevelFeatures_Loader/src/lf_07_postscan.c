/* Chapter 7 - UniversalPostSetup + S1/collision scans (LevelFeatures.c lines 5675-6104) */
/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Post-Setup — config-driven feature initialization
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Find an S1 ref point's position by exact name (verbatim 3 floats at
 * entry +0x04/08/0x0C — byte order preserved, never reordered).
 * Mirrors native 0x4605E0 S1-name lookup used by the Odd scene loader
 * (0x40EAA0 tail) to fill the BadBall spawn table. Returns 1 if found. */
static int FindS1Position(void *meshWorld, const char *want, float *out) {
    MEMORY_BASIC_INFORMATION mbi;
    DWORD objDb;
    int iter, count, idx;
    DWORD *array;
    DWORD savedIter;
    if (!meshWorld || !want || !out) return 0;
    if (!VirtualQuery(meshWorld, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return 0;
    objDb = *(DWORD *)((char *)meshWorld + 0x480);
    if (!objDb) return 0;
    if (!VirtualQuery((void*)objDb, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return 0;
    if (!g_AthenaListGetIterator || !g_AthenaListGetSize) return 0;
    if (!VirtualQuery((void*)(objDb + 0x894), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return 0;
    iter = g_AthenaListGetIterator((void *)(objDb + 0x894));
    if (iter <0 || iter>16) return 0;
    if (!VirtualQuery((void*)(objDb + 0x89C + iter*4), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return 0;
    savedIter = *(DWORD*)(objDb + 0x89C + iter*4);
    *(DWORD *)(objDb + 0x89C + iter*4) = 0;
    if (!VirtualQuery((void*)(objDb + 0x898), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) { *(DWORD*)(objDb + 0x89C + iter*4)=savedIter; return 0; }
    count = *(int *)(objDb + 0x898);
    if (count <=0 || count>8192) return 0;
    if (!VirtualQuery((void*)(objDb + 0xCA0), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return 0;
    array = *(DWORD **)(objDb + 0xCA0);
    if (!array) return 0;
    if (!VirtualQuery(array, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return 0;
    *(DWORD *)(objDb + 0x89C + iter*4) = 1;
    for (idx=0; idx<count; idx++) {
        DWORD *obj = (DWORD *)array[idx];
        char *name;
        if (!obj) continue;
        if (!VirtualQuery(obj, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) continue;
        name = *(char **)obj;
        if (!name) continue;
        if (!VirtualQuery(name, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) continue;
        if (my_stricmp(name, want) == 0) {
            if (!VirtualQuery((void*)((char*)obj + 0x04), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) break;
            out[0] = *(float *)((char *)obj + 0x04);
            out[1] = *(float *)((char *)obj + 0x08);
            out[2] = *(float *)((char *)obj + 0x0C);
            *(DWORD*)(objDb + 0x89C + iter*4)=savedIter;
            return 1;
        }
    }
    *(DWORD*)(objDb + 0x89C + iter*4)=savedIter;
    return 0;
}

/* Up VAC transport tubes (native Up scene loader 0x411620 tail,
 * disasm-verified 2026-09-11 via objdump): for each S1 "VAC-IN*" (len-6
 * prefix), suffix is reused verbatim ("-1", "-02" both work); "VAC-OUT*"
 * + "VAC-VEC*" resolved by name; op_new(0x38) + VacFace_ctor; appended
 * to the NATIVE board+0x436C list (orig Up RaceState iterates it — LFL
 * calls orig for level 6, so faces must sit exactly where vanilla puts
 * them). Two-pass (collect then build): FindS1Position nests AthenaList
 * iterator use and must not run inside an open walk. List (re)inited
 * here — PostSetup always precedes CreateDynamicObjects appends, so it
 * is empty at this point; bounds-guarded for file-swapped small boards. */
static void Vac_BuildTubes(void *board, void *ext) {
    MEMORY_BASIC_INFORMATION mbi;
    DWORD meshWorld, objDb, *array, savedIter;
    int iter, count, idx, built = 0;
    char suffixes[8][44];
    float inPos[8][3];
    int nIn = 0;
    if (!board || !ext) return;
    if (!g_operatorNew || !g_VacFaceCtor || !g_AthenaListAppend ||
        !g_AthenaListInit || !g_AthenaListGetIterator) return;
    meshWorld = *(DWORD *)((char *)board + BOARD_MESHWORLD);
    if (!meshWorld) return;
    if (!VirtualQuery((void *)meshWorld, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    objDb = *(DWORD *)((char *)meshWorld + 0x480);
    if (!objDb) return;
    if (!VirtualQuery((void*)objDb, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    if (!VirtualQuery((void*)(objDb + 0x894), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    iter = g_AthenaListGetIterator((void *)(objDb + 0x894));
    if (iter <0 || iter>16) return;
    if (!VirtualQuery((void*)(objDb + 0x89C + iter*4), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    savedIter = *(DWORD*)(objDb + 0x89C + iter*4);
    *(DWORD *)(objDb + 0x89C + iter*4) = 0;
    if (!VirtualQuery((void*)(objDb + 0x898), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) { *(DWORD*)(objDb + 0x89C + iter*4)=savedIter; return; }
    count = *(int *)(objDb + 0x898);
    if (count <=0 || count>8192) return;
    if (!VirtualQuery((void*)(objDb + 0xCA0), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    array = *(DWORD **)(objDb + 0xCA0);
    if (!array) return;
    if (!VirtualQuery(array, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    *(DWORD *)(objDb + 0x89C + iter*4) = 1;
    /* Pass 1: collect VAC-IN suffixes + entry positions */
    for (idx=0; idx<count && nIn<8; idx++) {
        DWORD *obj = (DWORD *)array[idx];
        char *name;
        int slen, si;
        if (!obj) continue;
        if (!VirtualQuery(obj, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) continue;
        name = *(char **)obj;
        if (!name) continue;
        if (!VirtualQuery(name, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) continue;
        if (my_strnicmp(name, "VAC-IN", 6) != 0) continue;
        if (!VirtualQuery((void*)((char*)obj + 0x04), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) continue;
        slen = 0; while (name[6+slen] && slen < 40) slen++;
        for (si=0; si<slen; si++) suffixes[nIn][si] = name[6+si];
        suffixes[nIn][slen] = '\0';
        inPos[nIn][0] = *(float *)((char *)obj + 0x04);
        inPos[nIn][1] = *(float *)((char *)obj + 0x08);
        inPos[nIn][2] = *(float *)((char *)obj + 0x0C);
        nIn++;
    }
    *(DWORD*)(objDb + 0x89C + iter*4)=savedIter;
    if (nIn == 0) return;
    /* List must fit: native Up board holds it; file-swapped small boards skip */
    if (!BoardHasOffset(board, 0x436C, 0x410)) { DebugLog("VAC: board too small, tubes skipped"); return; }
    g_AthenaListInit((void *)((char *)board + 0x436C), 0);
    /* Pass 2: resolve OUT/VEC + build faces */
    for (idx=0; idx<nIn; idx++) {
        char outName[64], vecName[64];
        float outP[3], vecP[3];
        void *mem, *face;
        const char *p1 = "VAC-OUT", *p2 = "VAC-VEC";
        int pi = 0, si = 0;
        while (p1[pi] && pi < 60) { outName[pi] = p1[pi]; pi++; }
        si = 0; while (suffixes[idx][si] && pi < 60) { outName[pi++] = suffixes[idx][si++]; }
        outName[pi] = '\0';
        pi = 0; while (p2[pi] && pi < 60) { vecName[pi] = p2[pi]; pi++; }
        si = 0; while (suffixes[idx][si] && pi < 60) { vecName[pi++] = suffixes[idx][si++]; }
        vecName[pi] = '\0';
        if (!FindS1Position((void *)meshWorld, outName, outP)) continue;
        if (!FindS1Position((void *)meshWorld, vecName, vecP)) continue;
        mem = g_operatorNew(0x38);
        if (!mem) continue;
        face = g_VacFaceCtor(mem, board,
            inPos[idx][0], inPos[idx][1], inPos[idx][2],
            outP[0], outP[1], outP[2], vecP[0], vecP[1], vecP[2]);
        if (!face) continue;
        g_AthenaListAppend((void *)((char *)board + 0x436C), (int)face);
        built++;
    }
    {
        char vdbg[64];
        wsprintfA(vdbg, "VAC: built %d/%d tubes", built, nIn);
        DebugLog(vdbg);
    }
}

/* Sky magnifier (native Sky loader 0x410900 tail): MAGNIFYER S1 +op_new
 * (0x444) + Magnifier_ctor, stored board+0x47AC; natively gated on
 * difficulty (App+0x23C) != 0. Exactly 1 MAGNIFYER in stock Level9, so
 * first-match == native. Ext slot always written (LFL readers use ext);
 * native slot mirrored when it fits. */
static void Magnifier_Build(void *board, void *ext) {
    float p[3];
    DWORD meshWorld, app, mem, obj;
    if (!board || !ext || !g_operatorNew || !g_MagnifierCtor) return;
    meshWorld = *(DWORD *)((char *)board + BOARD_MESHWORLD);
    if (!meshWorld) return;
    if (!FindS1Position((void *)meshWorld, "MAGNIFYER", p)) return;
    app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x240)) return;
    if (*(int *)(app + APP_DIFFICULTY) == 0) return;
    mem = (DWORD)g_operatorNew(0x444);
    if (!mem) return;
    obj = (DWORD)g_MagnifierCtor((void *)mem, board, p[0], p[1], p[2]);
    if (!obj) return;
    *(DWORD *)((char *)ext + UNI_MAGNIFYING_GLASS) = obj;
    if (BoardHasOffset(board, 0x47AC, 4))
        *(DWORD *)((char *)board + 0x47AC) = obj;
    DebugLog("Sky: magnifier built");
}

static void UniversalPostSetup(void *board) {
    void* ext = EnsureBoardExt(board);
    if (!ext) return;
    int level = GetCurrentLevel(board);
    if (level == 0) return;

    // Option B: LevelFeatures.txt deprecated — no LoadConfig, pure S1-driven
    // Bridge init is now S1-driven via ScanS1AndAutoEnable + lazy S1Ensure in CreateDynamicObjects
    // Do not call InitBridge here; CreateDynamicObjects handles BRIDGE/BUMPERS on demand
    // Keep bumpers S1-driven: scan for N:BUMPER presence
    int hasBumper = (*(BYTE*)((char*)ext + COLL_FLAG_BUMPER));
    if (hasBumper) {
        DWORD meshWorld = *(DWORD *)((char *)board + BOARD_MESHWORLD);
        if (!meshWorld || IsBadReadPtr((void *)meshWorld, 0x430)) return;

        int i;
        for (i = 0; i < 8; i++) {
            char nameBuf[16];
            const char *prefix = "N:BUMPER";
            int p = 0, j;
            for (j = 0; prefix[j]; j++) nameBuf[p++] = prefix[j];
            int num = i + 1;
            if (num >= 10) { nameBuf[p++] = '0' + (num / 10); num %= 10; }
            nameBuf[p++] = '0' + num;
            nameBuf[p] = '\0';

            void *dest = (char *)ext + BUMPER_SLOT_BASE + i * BUMPER_SLOT_STRIDE;

            if (g_AthenaListInit) {
                g_AthenaListInit(dest, 0);
            }
            if (g_CollectByNameFilter) {
                g_CollectByNameFilter((void *)meshWorld, nameBuf, dest);
            }
            *(DWORD *)((char *)ext + BUMPER_LIT_BASE + i * BUMPER_LIT_STRIDE) = 0;
        }
    }

    /* Odd BadBall spawn table (native Odd scene loader, Ghidra-verified
     * 2026-09-11 via decomp: flag byte +0x10DC=0, counter +0x4374=200,
     * total +0x4378=0, last +0x43A0=-1, table +0x437C/88/94 = S1 positions
     * of LAUNCH01 / LAUNCH02 / LAUNCH03. CHROMESHADOW resolves to a SEPARATE
     * slot (+0x43A4, consumer unknown — NOT a spawn location).
     * The ONLY writer of +0x10DC in .text is this init-0: no trigger event
     * (Odd E: set is BELL/SCORE/JUDGES/SAW/HAMMER only) ever sets the flag,
     * so the vanilla timer-spawner never fires. Replicated dormant-faithful
     * here (flag=0); starting it (flag=1 at load, or wiring a custom E:
     * trigger to set it) is a gameplay call, not a fix. */
    if (level == 9) {
        DWORD oddMW = *(DWORD *)((char *)board + BOARD_MESHWORLD);
        if (oddMW && !IsBadReadPtr((void *)oddMW, 0x430)) {
            const char *bbSlots[3] = { "LAUNCH01", "LAUNCH02", "LAUNCH03" };
            int bi, bfound = 0;
            *(char *)((char *)ext + UNI_BB_FLAG) = 0;
            *(int *)((char *)ext + UNI_BB_COUNTER) = 200;
            *(int *)((char *)ext + UNI_BB_TOTAL) = 0;
            *(int *)((char *)ext + UNI_BB_LAST_IDX) = -1;
            for (bi = 0; bi < 3; bi++) {
                float p[3] = { 0.0f, 0.0f, 0.0f };
                if (FindS1Position((void *)oddMW, bbSlots[bi], p)) {
                    ((float *)((char *)ext + UNI_BB_POS_TABLE))[bi * 3] = p[0];
                    ((float *)((char *)ext + UNI_BB_POS_TABLE))[bi * 3 + 1] = p[1];
                    ((float *)((char *)ext + UNI_BB_POS_TABLE))[bi * 3 + 2] = p[2];
                    bfound++;
                }
            }
            OrBoardFeat(board, FEAT_BADBALL);
            {
                char bbdbg[96];
                wsprintfA(bbdbg, "Odd: BB table init %d/3 (dormant, flag=0 per native)", bfound);
                DebugLog(bbdbg);
            }
        }
    }

    /* Up VAC tubes + Sky magnifier (native scene-loader tails, see builders
     * above). Content-gated: no-op on levels without VAC-IN/MAGNIFYER S1s. */
    Vac_BuildTubes(board, ext);
    Magnifier_Build(board, ext);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Scene Constructor — REPLACES vtable[0x48] (Scene_LoadLevel*)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* S1-driven auto-enable — makes any MESHWORLD file-swappable.
 * Scans meshWorld+0x480 S1 refs after Level_MeshWorldCtor and OrBoardFeat for
 * bridge/swirl/windmill so a Dizzy MESHWORLD dropped into WarmUp slot
 * auto-enables its features without LevelData.txt edit. Heap size is
 * fixed 0xC000, so no dynamic sizing needed — just feat bits. */
static void AddS1CollisionToExt(void *board, void *ext, const char *name) {
    if (!board || !ext || !name) return;
    if (name[0]!='N' && name[0]!='E') return;
    if (name[1]!=':') return;
    // dedup
    int count = *(int*)((char*)ext + OFF_COLLISION_COUNT);
    if (count <0 || count >= MAX_S1_COLLISIONS) { if (count>=MAX_S1_COLLISIONS) DebugLog("AddS1Collision: overflow, dropping"); return; }
    char *base = (char*)ext + OFF_COLLISION_NAMES;
    for (int i=0;i<count;i++) if (my_stricmp(base + i*S1_COLLISION_NAME_LEN, name)==0) return;
    my_strncpy(base + count*S1_COLLISION_NAME_LEN, name, S1_COLLISION_NAME_LEN);
    *(int*)((char*)ext + OFF_COLLISION_COUNT) = count+1;
}
static int IsS1CollisionEnabled(void *board, const char *eventName) {
    if (!board || !eventName) return 0;
    void *ext = GetBoardExt(board);
    if (!ext) return 0;
    int count = *(int*)((char*)ext + OFF_COLLISION_COUNT);
    if (count<=0 || count>MAX_S1_COLLISIONS) return 0;
    char *base = (char*)ext + OFF_COLLISION_NAMES;
    // exact match
    for (int i=0;i<count;i++) if (my_stricmp(base + i*S1_COLLISION_NAME_LEN, eventName)==0) return 1;
    // generic prefix match: stored variant vs canonical event (E:BRANCH(A) vs E:BRANCH)
    // covers N:BUMPER1, E:BRANCH(A)/(B), E:SCORE*, N:NEONPLATFORM*, N:SPINNY*, etc.
    // dispatch uses my_strnicmp(name, event, len)==0, so stored "E:BRANCH(A)" must enable canonical "E:BRANCH"
    { int elen=0; while(eventName[elen]) elen++;
      for (int i=0;i<count;i++) if (my_strnicmp(base + i*S1_COLLISION_NAME_LEN, eventName, elen)==0) return 1;
    }
    return 0;
}
static void BuildCollisionFlags(void *board, void *ext) {
    if (!board || !ext) return;
    // Clear flags
    memset((char*)ext + OFF_COLLISION_FLAGS, 0, COLL_FLAG_COUNT);
    // Set each based on IsS1CollisionEnabled (called once per event at load, not per collision)
    if (IsS1CollisionEnabled(board, "N:BUMPER")) *(BYTE*)((char*)ext + COLL_FLAG_BUMPER)=1;
    if (IsS1CollisionEnabled(board, "N:BRIDGE")) *(BYTE*)((char*)ext + COLL_FLAG_BRIDGE)=1;
    if (IsS1CollisionEnabled(board, "N:WATERWHEEL")) *(BYTE*)((char*)ext + COLL_FLAG_WATERWHEEL)=1;
    if (IsS1CollisionEnabled(board, "N:WHEELEMBED")) *(BYTE*)((char*)ext + COLL_FLAG_WHEELEMBED)=1;
    if (IsS1CollisionEnabled(board, "N:SWIRL")) *(BYTE*)((char*)ext + COLL_FLAG_SWIRL)=1;
    if (IsS1CollisionEnabled(board, "E:CATAPULTBOTTOM")) *(BYTE*)((char*)ext + COLL_FLAG_CATAPULTBOTTOM)=1;
    if (IsS1CollisionEnabled(board, "E:OPENSESAME")) *(BYTE*)((char*)ext + COLL_FLAG_OPENSESAME)=1;
    if (IsS1CollisionEnabled(board, "N:TRAPDOOR")) *(BYTE*)((char*)ext + COLL_FLAG_TRAPDOOR)=1;
    if (IsS1CollisionEnabled(board, "E:BITE")) *(BYTE*)((char*)ext + COLL_FLAG_BITE)=1;
    if (IsS1CollisionEnabled(board, "E:MACETRIGGER")) *(BYTE*)((char*)ext + COLL_FLAG_MACETRIGGER)=1;
    if (IsS1CollisionEnabled(board, "N:MACE")) *(BYTE*)((char*)ext + COLL_FLAG_MACE)=1;
    if (IsS1CollisionEnabled(board, "E:HELPINERTIA")) *(BYTE*)((char*)ext + COLL_FLAG_HELPINERTIA)=1;
    if (IsS1CollisionEnabled(board, "E:UNHELPINERTIA")) *(BYTE*)((char*)ext + COLL_FLAG_UNHELPINERTIA)=1;
    if (IsS1CollisionEnabled(board, "E:VACPOPOUT")) *(BYTE*)((char*)ext + COLL_FLAG_VACPOPOUT)=1;
    if (IsS1CollisionEnabled(board, "N:SPEEDCYLINDER")) *(BYTE*)((char*)ext + COLL_FLAG_SPEEDCYLINDER)=1;
    if (IsS1CollisionEnabled(board, "N:EXTRATIME")) *(BYTE*)((char*)ext + COLL_FLAG_EXTRATIME)=1;
    if (IsS1CollisionEnabled(board, "N:NEONPLATFORM")) *(BYTE*)((char*)ext + COLL_FLAG_NEONPLATFORM)=1;
    if (IsS1CollisionEnabled(board, "E:ZOOP")) *(BYTE*)((char*)ext + COLL_FLAG_ZOOP)=1;
    if (IsS1CollisionEnabled(board, "E:LIGHTSOFF")) *(BYTE*)((char*)ext + COLL_FLAG_LIGHTSOFF)=1;
    if (IsS1CollisionEnabled(board, "E:LIGHTSON")) *(BYTE*)((char*)ext + COLL_FLAG_LIGHTSON)=1;
    if (IsS1CollisionEnabled(board, "E:CALLHAMMER")) *(BYTE*)((char*)ext + COLL_FLAG_CALLHAMMER)=1;
    if (IsS1CollisionEnabled(board, "E:HAMMERCHASE")) *(BYTE*)((char*)ext + COLL_FLAG_HAMMERCHASE)=1;
    if (IsS1CollisionEnabled(board, "E:ALERTSAW1")) *(BYTE*)((char*)ext + COLL_FLAG_ALERTSAW1)=1;
    if (IsS1CollisionEnabled(board, "E:ALERTSAW2")) *(BYTE*)((char*)ext + COLL_FLAG_ALERTSAW2)=1;
    if (IsS1CollisionEnabled(board, "E:ALERTSAW3")) *(BYTE*)((char*)ext + COLL_FLAG_ALERTSAW3)=1;
    if (IsS1CollisionEnabled(board, "E:ACTIVATESAW1")) *(BYTE*)((char*)ext + COLL_FLAG_ACTIVATESAW1)=1;
    if (IsS1CollisionEnabled(board, "E:ACTIVATESAW2")) *(BYTE*)((char*)ext + COLL_FLAG_ACTIVATESAW2)=1;
    if (IsS1CollisionEnabled(board, "E:ALERTJUDGES")) *(BYTE*)((char*)ext + COLL_FLAG_ALERTJUDGES)=1;
    if (IsS1CollisionEnabled(board, "E:SCORE")) *(BYTE*)((char*)ext + COLL_FLAG_SCORE)=1;
    if (IsS1CollisionEnabled(board, "E:BELL")) *(BYTE*)((char*)ext + COLL_FLAG_BELL)=1;
    if (IsS1CollisionEnabled(board, "E:GRAVITY")) *(BYTE*)((char*)ext + COLL_FLAG_GRAVITY)=1;
    if (IsS1CollisionEnabled(board, "N:JUMPFIRST")) *(BYTE*)((char*)ext + COLL_FLAG_JUMPFIRST)=1;
    if (IsS1CollisionEnabled(board, "N:JUMPSECOND")) *(BYTE*)((char*)ext + COLL_FLAG_JUMPSECOND)=1;
    if (IsS1CollisionEnabled(board, "E:SHRINK")) *(BYTE*)((char*)ext + COLL_FLAG_SHRINK)=1;
    if (IsS1CollisionEnabled(board, "E:GROWSOUND")) *(BYTE*)((char*)ext + COLL_FLAG_GROWSOUND)=1;
    if (IsS1CollisionEnabled(board, "E:GROW")) *(BYTE*)((char*)ext + COLL_FLAG_GROW)=1;
    if (IsS1CollisionEnabled(board, "E:DROPLIFT")) *(BYTE*)((char*)ext + COLL_FLAG_DROPLIFT)=1;
    if (IsS1CollisionEnabled(board, "E:PIPERANDOM")) *(BYTE*)((char*)ext + COLL_FLAG_PIPERANDOM)=1;
    if (IsS1CollisionEnabled(board, "E:LIMIT")) *(BYTE*)((char*)ext + COLL_FLAG_LIMIT)=1;
    if (IsS1CollisionEnabled(board, "E:LIMITX")) *(BYTE*)((char*)ext + COLL_FLAG_LIMITX)=1;
    if (IsS1CollisionEnabled(board, "E:LIMITZ")) *(BYTE*)((char*)ext + COLL_FLAG_LIMITZ)=1;
    if (IsS1CollisionEnabled(board, "E:LIMITPIPE1")) *(BYTE*)((char*)ext + COLL_FLAG_LIMITPIPE1)=1;
    if (IsS1CollisionEnabled(board, "E:SWALLOW")) *(BYTE*)((char*)ext + COLL_FLAG_SWALLOW)=1;
    if (IsS1CollisionEnabled(board, "E:LIMITPIPE2")) *(BYTE*)((char*)ext + COLL_FLAG_LIMITPIPE2)=1;
    if (IsS1CollisionEnabled(board, "E:BRANCH")) *(BYTE*)((char*)ext + COLL_FLAG_BRANCH)=1;
    if (IsS1CollisionEnabled(board, "N:SPINNY")) *(BYTE*)((char*)ext + COLL_FLAG_SPINNY)=1;
    if (IsS1CollisionEnabled(board, "N:SAWTEETH")) *(BYTE*)((char*)ext + COLL_FLAG_SAWTEETH)=1;
    if (IsS1CollisionEnabled(board, "N:SPINNER")) *(BYTE*)((char*)ext + COLL_FLAG_SPINNER)=1;
    if (IsS1CollisionEnabled(board, "E:LAUNCH")) *(BYTE*)((char*)ext + COLL_FLAG_LAUNCH)=1;
    if (IsS1CollisionEnabled(board, "N:SQUAREWOBBLY")) *(BYTE*)((char*)ext + COLL_FLAG_SQUAREWOBBLY)=1;
    if (IsS1CollisionEnabled(board, "N:WAVY")) *(BYTE*)((char*)ext + COLL_FLAG_WAVY)=1;
    if (IsS1CollisionEnabled(board, "N:GLASS")) *(BYTE*)((char*)ext + COLL_FLAG_GLASS)=1;
    if (IsS1CollisionEnabled(board, "N:TENBONUS1")) *(BYTE*)((char*)ext + COLL_FLAG_TENBONUS1)=1;
    if (IsS1CollisionEnabled(board, "N:TENBONUS2")) *(BYTE*)((char*)ext + COLL_FLAG_TENBONUS2)=1;
    if (IsS1CollisionEnabled(board, "E:PEGS")) *(BYTE*)((char*)ext + COLL_FLAG_PEGS)=1;
    if (IsS1CollisionEnabled(board, "E:TRAPPOP")) *(BYTE*)((char*)ext + COLL_FLAG_TRAPPOP)=1;
    if (IsS1CollisionEnabled(board, "E:NOPEGS")) *(BYTE*)((char*)ext + COLL_FLAG_NOPEGS)=1;
    if (IsS1CollisionEnabled(board, "E:HEATON")) *(BYTE*)((char*)ext + COLL_FLAG_HEATON)=1;
    if (IsS1CollisionEnabled(board, "E:HEATOFF")) *(BYTE*)((char*)ext + COLL_FLAG_HEATOFF)=1;
    if (IsS1CollisionEnabled(board, "N:BOUNCE")) *(BYTE*)((char*)ext + COLL_FLAG_BOUNCE)=1;
    if (IsS1CollisionEnabled(board, "N:ONROTATOR")) *(BYTE*)((char*)ext + COLL_FLAG_ONROTATOR)=1;
    if (IsS1CollisionEnabled(board, "N:ONGEAR")) *(BYTE*)((char*)ext + COLL_FLAG_ONGEAR)=1;
    /* Feature wiring (S1-driven, no level hardcode):
     * N:BUMPER presence => bumper-lit decay (render reads ext+UNI_BUMPER_LIT).
     * Without this FEAT_BUMPER_DECAY never sets and Beginner (native orig
     * skipped in UniversalRaceState) loses decay entirely. */
    if (*(BYTE*)((char*)ext + COLL_FLAG_BUMPER)) OrBoardFeat(board, FEAT_BUMPER_DECAY);
    { char dbg[96]; int cnt=0; for(int i=0;i<COLL_FLAG_COUNT;i++) if(*(BYTE*)((char*)ext+OFF_COLLISION_FLAGS+i)) cnt++; wsprintfA(dbg,"BuildCollisionFlags: %d/%d enabled",cnt,COLL_FLAG_COUNT); DebugLog(dbg); }
}

static void ScanS1AndAutoEnable(void *board, void *ext, void *meshWorld) {
    if (!board || !ext || !meshWorld) return;
    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQuery(meshWorld, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    /* meshWorld+0x480 must be readable */
    DWORD objDb = *(DWORD *)((char *)meshWorld + 0x480);
    if (!objDb) return;
    if (!VirtualQuery((void*)objDb, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    if (!g_AthenaListGetIterator || !g_AthenaListGetSize) return;
    /* Validate AthenaList header without IsBad* */
    if (!VirtualQuery((void*)(objDb + 0x894), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    int iter = g_AthenaListGetIterator((void *)(objDb + 0x894));
    if (iter <0 || iter>16) return;
    if (!VirtualQuery((void*)(objDb + 0x89C + iter*4), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    DWORD savedIter = *(DWORD*)(objDb + 0x89C + iter*4);
    *(DWORD *)(objDb + 0x89C + iter*4) = 0;
    if (!VirtualQuery((void*)(objDb + 0x898), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) { *(DWORD*)(objDb + 0x89C + iter*4)=savedIter; return; }
    int count = *(int *)(objDb + 0x898);
    if (count <=0 || count>8192) return;
    if (!VirtualQuery((void*)(objDb + 0xCA0), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    DWORD *array = *(DWORD **)(objDb + 0xCA0);
    if (!array) return;
    if (!VirtualQuery(array, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    /* Quick sanity: array should hold count pointers */
    *(DWORD *)(objDb + 0x89C + iter*4) = 1;
    int idx=0;
    // Scan ALL entries (not capped at 64) — large custom levels may have windmill late
    int scan = count;
    for (idx=0; idx<scan; idx++) {
        DWORD *obj = (DWORD *)array[idx];
        if (!obj) continue;
        if (!VirtualQuery(obj, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) continue;
        char *name = *(char **)obj;
        if (!name) continue;
        if (!VirtualQuery(name, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) continue;
        // name is S1 ref string, e.g. "Levels\\Level3-WaterWheel" or "BRIDGE" etc.
        // Phase1 full registry: every S1 prefix auto-enables its feat / ensures mesh
        if (my_strnicmp(name, "BRIDGE", 6)==0 || my_strnicmp(name, "BBRIDGE", 7)==0) {
            OrBoardFeat(board, FEAT_BRIDGE_ANIM);
        }
        if (my_strnicmp(name, "WATERWHEEL", 10)==0 || my_strnicmp(name, "WHEELEMBED", 10)==0 || my_strnicmp(name, "SWIRL", 5)==0 || my_strnicmp(name, "TarBubble", 9)==0 || my_strnicmp(name, "GLUEBIE", 7)==0 || my_strnicmp(name, "TIPPER", 6)==0) {
            OrBoardFeat(board, FEAT_SWIRL);
        }
        if (my_strnicmp(name, "WINDMILL", 8)==0 || my_strnicmp(name, "CHOMPER", 7)==0 || my_strnicmp(name, "TURRET", 6)==0 || my_strnicmp(name, "CATAPULT", 8)==0 || my_strnicmp(name, "MACE", 4)==0 || my_strnicmp(name, "DRAWBRIDGE", 10)==0 || my_strnicmp(name, "TRAPDOOR", 8)==0) {
            OrBoardFeat(board, FEAT_WINDMILL);
        }
        if (my_strnicmp(name, "WOBBLY", 6)==0 || my_strnicmp(name, "WAVY", 4)==0) {
            // Wobbly family — no feat flag but S1 presence proves level intent
        }
        if (my_strnicmp(name, "POPCYLINDER", 11)==0 || my_strnicmp(name, "POPDOOR", 7)==0 || my_strnicmp(name, "CLOUDSCAPE", 10)==0) {
            OrBoardFeat(board, FEAT_SKY_POPCYL);
        }
        if (my_strnicmp(name, "BONK", 4)==0 || my_strnicmp(name, "FAN", 3)==0 || my_strnicmp(name, "SAWBLADE", 8)==0 || my_strnicmp(name, "BELL", 4)==0 || my_strnicmp(name, "JUDGE", 5)==0 || my_strnicmp(name, "SPINNER", 7)==0 || my_strnicmp(name, "LOOPER", 6)==0 || my_strnicmp(name, "GEAR", 4)==0 || my_strnicmp(name, "PENDULUM", 8)==0 || my_strnicmp(name, "ROTATOR", 7)==0) {
            // Expert/Impossible family — handled via S1Ensure* in CreateDynamicObjects
        }
        if (my_strnicmp(name, "CLOUDSCAPE", 10)==0) {
            S1EnsureSprite(board, ext, REND_SKY_SPRITE, "Textures\\Clouds.png");
        }
        // Option B: every N:/E: S1 ref auto-enables its collision event for this board
        if ((name[0]=='N' || name[0]=='E') && name[1]==':') {
            AddS1CollisionToExt(board, ext, name);
        }
    }
    *(DWORD*)(objDb + 0x89C + iter*4)=savedIter;
    char dbg[128];
    DWORD feat = GetBoardFeat(board);
    wsprintfA(dbg, "S1 scan: count=%d feat=0x%X", count, feat);
    DebugLog(dbg);
}

/* ── File byte-scan for N:/E: hidden in sub-meshes ──
 * Some N:/E: collision meshes live inside sub-mesh .MESHWORLD/.MESH
 * files (e.g. Level3-WaterWheel) not in the main level S1/S6.
 * User requested: scan ALL mesh/meshworld files in the level folder.
 * Generic extractor: finds N:XXX / E:XXX tokens in raw bytes and
 * AddS1CollisionToExt for this board. Dedup handled by AddS1.
 * Called once per level load, not per frame. */
static int IsCollNameChar(char c) {
    return (c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='_'||c==':'||c=='-'||c=='('||c==')';
}
static void ScanFileForCollisions(void *board, void *ext, const char *filePath) {
    if (!board || !ext || !filePath || !filePath[0]) return;
    HANDLE h = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h==INVALID_HANDLE_VALUE) return;
    DWORD fsize = GetFileSize(h, NULL);
    if (fsize==0 || fsize==(DWORD)-1 || fsize>0x500000) { CloseHandle(h); return; }
    char *buf = (char*)HeapAlloc(GetProcessHeap(), 0, fsize);
    if (!buf) { CloseHandle(h); return; }
    DWORD br=0; if (!ReadFile(h, buf, fsize, &br, NULL) || br!=fsize) { HeapFree(GetProcessHeap(),0,buf); CloseHandle(h); return; }
    CloseHandle(h);
    int added=0;
    for (DWORD i=0; i+2 < fsize; ) {
        if ((buf[i]=='N' || buf[i]=='E') && buf[i+1]==':' && IsCollNameChar(buf[i+2])) {
            DWORD j=0;
            while (i+j < fsize && j < 31 && IsCollNameChar(buf[i+j])) j++;
            if (j>=3 && j<=31) {
                char name[32]; int k; for (k=0;k<(int)j && k<31;k++) name[k]=buf[i+k];
                name[k]='\0';
                // validate: at least N:X or E:XX and not just prefix
                if ((name[0]=='N' || name[0]=='E') && name[1]==':' && name[2]) {
                    int before = (int)*(int*)((char*)ext + OFF_COLLISION_COUNT);
                    AddS1CollisionToExt(board, ext, name);
                    int after = (int)*(int*)((char*)ext + OFF_COLLISION_COUNT);
                    if (after>before) added++;
                }
            }
            i += (j?j:1);
        } else i++;
    }
    HeapFree(GetProcessHeap(),0,buf);
    if (added) { char lg[96]; wsprintfA(lg, "FileScan: '%s' +%d N:/E:", filePath, added); DebugLog(lg); }
}
// Targeted scan: only scan mesh files actually referenced by S1 (not whole folder wildcard).
// Prevents hitch from scanning 30+ files every level load. Falls back to basePath file.
static void ScanS1ReferencedMeshesForCollisions(void *board, void *ext, void *meshWorld, const char *basePath) {
    if (!board || !ext) return;
    // Always scan the main level file itself
    if (basePath && basePath[0]) {
        ScanFileForCollisions(board, ext, basePath);
        char withExt[MAX_PATH]; strcpy(withExt, basePath);
        int l=strlen(withExt);
        if (l+10 < MAX_PATH && my_strnicmp(withExt+l-10, ".MESHWORLD", 10)!=0 && my_strnicmp(withExt+l-5, ".MESH", 5)!=0) {
            strcat(withExt, ".MESHWORLD");
            ScanFileForCollisions(board, ext, withExt);
        }
    }
    if (!meshWorld) return;
    DWORD objDb = *(DWORD *)((char *)meshWorld + 0x480);
    if (!objDb) return;
    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQuery((void*)objDb, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    if (!g_AthenaListGetIterator || !g_AthenaListGetSize) return;
    if (!VirtualQuery((void*)(objDb + 0x894), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    int iter = g_AthenaListGetIterator((void *)(objDb + 0x894));
    if (iter <0 || iter>16) return;
    if (!VirtualQuery((void*)(objDb + 0x89C + iter*4), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    DWORD savedIter = *(DWORD*)(objDb + 0x89C + iter*4);
    *(DWORD *)(objDb + 0x89C + iter*4) = 0;
    if (!VirtualQuery((void*)(objDb + 0x898), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) { *(DWORD*)(objDb + 0x89C + iter*4)=savedIter; return; }
    int count = *(int *)(objDb + 0x898);
    if (count <=0 || count>8192) { *(DWORD*)(objDb + 0x89C + iter*4)=savedIter; return; }
    if (!VirtualQuery((void*)(objDb + 0xCA0), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) { *(DWORD*)(objDb + 0x89C + iter*4)=savedIter; return; }
    DWORD *array = *(DWORD **)(objDb + 0xCA0);
    if (!array || !VirtualQuery(array, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) { *(DWORD*)(objDb + 0x89C + iter*4)=savedIter; return; }
    *(DWORD *)(objDb + 0x89C + iter*4) = 1;
    for (int idx=0; idx<count; idx++) {
        DWORD *obj = (DWORD *)array[idx];
        if (!obj || !VirtualQuery(obj, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) continue;
        char *name = *(char **)obj;
        if (!name || !VirtualQuery(name, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) continue;
        // Heuristic: S1 strings that look like file paths (contain slash or known prefix) and not N:/E: themselves
        if (name[0]=='N' && name[1]==':') continue;
        if (name[0]=='E' && name[1]==':') continue;
        int isPath = 0;
        for (char *p=name; *p; p++) if (*p=='\\' || *p=='/') { isPath=1; break; }
        if (!isPath) {
            if (my_strnicmp(name, "Levels\\", 7)==0) isPath=1;
            if (my_strnicmp(name, "Meshes\\", 7)==0) isPath=1;
            if (my_strnicmp(name, "levels\\", 7)==0) isPath=1;
        }
        if (!isPath) {
            /* Bare S1 names own sub-mesh files carrying N:/E: tokens (vanilla mains
             * contain zero Levels\\ path strings). Fall back to the known owner file. */
            static const char *s_subMeshFallback =
                "CATAPULT\0Levels\\Level4-Catapult\0" "MACE\0Levels\\Level4-Mace\0" "DRAWBRIDGE\0Levels\\Level4-Drawbridge\0" "WINDMILL\0Levels\\Level4-Windmill\0"
                "TRAPDOOR\0Levels\\Level9-TrapDoor\0" "TURRET\0Levels\\Level4-Turret\0" "CHOMPER\0Meshes\\Chomper\0" "TIPPER\0Levels\\Level3-Tipper\0"
                "GLUEBIE\0Levels\\Level3-Gluebie\0" "SWIRL\0Levels\\Level3-Swirl\0" "WATERWHEEL\0Levels\\Level3-WaterWheel\0" "WHEELEMBED\0Levels\\Level3-WaterWheel\0"
                "SAWBRIDGE\0Levels\\Level2-Bridge\0" "SAW\0Levels\\Level8-Saw\0" "BBRIDGE\0Levels\\Level10-Bridge1\0" "SPINNY\0Levels\\Level8-Spinny\0"
                "SAW\0Levels\\Level8-Saw\0" "FALLOUT\0Levels\\Level8-Fallout\0" "BLOCKDAWG\0Levels\\Level8-BlockDawg1\0" "LIFTER\0Levels\\LevelUp-Lifter\0"
                "DROPPER\0Levels\\LevelUp-Lifter\0" "SPEEDCYLINDER\0Levels\\LevelUp-SpeedCylinder\0" "TIMEBUTTON\0Levels\\LevelUp-Button\0" "NEONPLATFORM\0Levels\\LevelDark-NeonPlatform\0"
                "DFLOOR\0Levels\\LevelDark-DFloor1\0" "TRODE\0Levels\\LevelDark-Trode\0" "POPCYLINDER\0Levels\\Level9-PopCylinder1\0" "POPDOOR\0Levels\\Level9-TrapDoor\0"
                "CLOUDSCAPE\0Levels\\Level9-PopCylinder1\0" "WOBBLY\0Levels\\Level7-Wobbly1\0" "WAVY\0Levels\\Level7-Wavy1\0" "LOOPER\0Levels\\LevelImpossible-Looper\0"
                "GEAR\0Levels\\LevelImpossible-Gear\0" "BIGGEAR\0Levels\\LevelImpossible-BigGear\0" "ROTATOR\0Levels\\LevelImpossible-Rotator\0" "PENDULUM\0Levels\\LevelImpossible-Pendulum\0"
                "BONK\0Levels\\Level2-Bridge\0" "FAN\0Levels\\Level2-Bridge\0" "BELL\0Levels\\Level2-Bridge\0" "JUDGE\0Levels\\Level2-Bridge\0";
            for (const char *f = s_subMeshFallback; *f; ) {
                int pl = strlen(name) < strlen(f) ? strlen(name) : strlen(f);
                (void)pl;
                size_t pn = 0; while (f[pn]) pn++;
                if (my_strnicmp(name, f, pn) == 0) {
                    const char *mp = f + pn + 1;
                    ScanFileForCollisions(board, ext, mp);
                    { char me[MAX_PATH]; strcpy(me, mp); strcat(me, ".MESHWORLD");
                      ScanFileForCollisions(board, ext, me); }
                    break;
                }
                f += pn + 1; { size_t ml = strlen(f); f += ml + 1; }
            }
            continue;
        }
        // name is like "Levels\\Level3-WaterWheel" or "Meshes\\Chomper" etc. Scan that file.
        char tryPath[MAX_PATH];
        // If name already contains "levels\", use as-is; else prepend "levels\"
        if (my_strnicmp(name, "levels\\", 7)==0 || my_strnicmp(name, "levels/", 7)==0) {
            strcpy(tryPath, name);
        } else {
            // S1 refs are already PascalCase ("Levels\\...", "Meshes\\...") matching disk.
            // Do NOT lowercase: ext4/Wine is case-sensitive. Use name as-is.
            if (0) {
                strcpy(tryPath, "levels\\");
                strcat(tryPath, name+7);
            } else if (0) {
                strcpy(tryPath, "meshes\\");
                strcat(tryPath, name+7);
            } else {
                strcpy(tryPath, name);
            }
        }
        ScanFileForCollisions(board, ext, tryPath);
        // also try with .MESHWORLD extension if no dot
        if (!strchr(tryPath, '.')) {
            char withExt2[MAX_PATH]; strcpy(withExt2, tryPath); strcat(withExt2, ".MESHWORLD");
            ScanFileForCollisions(board, ext, withExt2);
            strcpy(withExt2, tryPath); strcat(withExt2, ".MESH");
            ScanFileForCollisions(board, ext, withExt2);
        }
        *(DWORD *)(objDb + 0x89C + iter*4) = idx+1;
    }
    *(DWORD *)(objDb + 0x89C + iter*4)=savedIter; // restore
}

/* DEPRECATED: wildcard folder sweep — replaced by ScanS1ReferencedMeshesForCollisions (targeted)
 * Kept as #if 0 to avoid future hitch; do not re-enable without profiling.
 */
#if 0
static void ScanLevelFolderForCollisions(void *board, void *ext, const char *basePath) {
    if (!board || !ext || !basePath || !basePath[0]) return;
    // extract dir from basePath (up to last slash)
    char dir[MAX_PATH]=""; const char *last=NULL; for (const char *q=basePath; *q; q++) if (*q=='\\' || *q=='/') last=q;
    if (last) { int len=(int)(last - basePath)+1; if (len>=MAX_PATH) len=MAX_PATH-1; strncpy(dir, basePath, len); dir[len]='\0'; }
    else { strcpy(dir, "levels\\"); }
    // also ensure dir ends with slash
    int dlen=strlen(dir); if (dlen && dir[dlen-1]!='\\' && dir[dlen-1]!='/') { if (dlen+1<MAX_PATH){dir[dlen]='\\';dir[dlen+1]='\0';} }
    // helper to scan wildcard in a given dir
    const char *pats[4]={"*.MESHWORLD","*.MESH","*.meshworld","*.mesh"};
    char globalDir[MAX_PATH]="levels\\";
    // scan requested dir
    for (int pi=0; pi<4; pi++) {
        char pat[MAX_PATH]; strcpy(pat, dir); strcat(pat, pats[pi]);
        WIN32_FIND_DATAA fd; HANDLE fh=FindFirstFileA(pat, &fd);
        if (fh==INVALID_HANDLE_VALUE) continue;
        do {
            if (fd.cFileName[0]=='.') continue;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
            char full[MAX_PATH]; strcpy(full, dir); strcat(full, fd.cFileName);
            ScanFileForCollisions(board, ext, full);
        } while (FindNextFileA(fh, &fd));
        FindClose(fh);
    }
    // also scan global levels\ — always (secret objects + shared sub-meshes live there)
    if (my_stricmp(dir, globalDir)!=0) /* also scan global Levels folder for shared sub-meshes + secret objects */ {
        for (int pi=0; pi<4; pi++) {
            char pat[MAX_PATH]; strcpy(pat, globalDir); strcat(pat, pats[pi]);
            WIN32_FIND_DATAA fd; HANDLE fh=FindFirstFileA(pat, &fd);
            if (fh==INVALID_HANDLE_VALUE) continue;
            do {
                if (fd.cFileName[0]=='.') continue;
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
                char full[MAX_PATH]; strcpy(full, globalDir); strcat(full, fd.cFileName);
                ScanFileForCollisions(board, ext, full);
            } while (FindNextFileA(fh, &fd));
            FindClose(fh);
        }
    }
    // also explicitly scan the basePath file itself (covers bare-name resolves without slash)
    {
        char tryPath[MAX_PATH]; strcpy(tryPath, basePath);
        ScanFileForCollisions(board, ext, tryPath);
        // try with extension
        int tlen=strlen(tryPath);
        if (tlen+10 < MAX_PATH && my_strnicmp(tryPath+tlen-10, ".MESHWORLD", 10)!=0 && my_strnicmp(tryPath+tlen-5, ".MESH", 5)!=0) {
            strcat(tryPath, ".MESHWORLD");
            ScanFileForCollisions(board, ext, tryPath);
        }
    }
    char lg2[96]; int cnt=*(int*)((char*)ext + OFF_COLLISION_COUNT);
    wsprintfA(lg2, "FolderScan done dir='%s' total N:/E:=%d", dir, cnt); DebugLog(lg2);
}
#endif // 0 — ScanLevelFolderForCollisions deprecated
