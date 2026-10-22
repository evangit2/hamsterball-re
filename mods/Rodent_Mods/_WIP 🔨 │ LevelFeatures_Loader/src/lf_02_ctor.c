/* Chapter 2 - UniversalBoardCtorLogic + install thunks (LevelFeatures.c lines 1662-2008) */
/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Board Constructor — REPLACES all 15 per-level board constructors
 *
 * Called via 15 naked thunks that set g_pendingRaceIndex.
 * Does the same steps every LevelBoard_*_ctor does:
 *   1. Board_ctor(mem, app)
 *   2. Set vtable
 *   3. Set board name (+0x868), race title (+0x29B4)
 *   4. Set +0x870 from app+0x1DC
 *   5. Vec3_Init with per-level color, store at +0x1508
 *   6. LoadRaceData(mem, raceName)
 *   7. Set music name (+0x4344)
 *   8. Load extra meshes (per-level)
 *   9. Set unlock flags (per-level)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* UniversalTowerInit — content-driven Tower board setup, no native ctor.
 * Replicates Tower native ctor (0x41E340, Ghidra-verified) field-by-field:
 * base Board_ctor, Tower vtable, name strings, difficulty flag,
 * LoadRaceData("TOWERRACE"). +0x870 comes from Step 4 (proven app-source),
 * +0x1508 from Step 5 (slot colors, proven), lists from Step 9a-extra3.
 * Mesh slots (+0x436C..+0x43B4) stay empty — S1-driven create loads and
 * mirrors them (BoardMeshOrZero: Tower content never trusts board slots). */
static void UniversalTowerInit(void *mem, int app) {
    g_BoardCtor(mem, app);
    *(DWORD *)mem = 0x4D0A08; /* Tower board vtable */
    *(char **)((char *)mem + 0x868) = "Board (Tower)";
    *(char **)((char *)mem + 0x29B4) = "TOWER RACE";
    *(char **)((char *)mem + 0x4344) = "Happy Rush";
    *(DWORD *)((char *)mem + 0x43A4) = 0;
    *(DWORD *)((char *)mem + 0x43A0) = 0;
    *(DWORD *)((char *)mem + 0x43A8) = 0;
    { int diff = *(int *)(app + 0x23C);
      DWORD prof = *(DWORD *)(app + 0x220);
      if (diff != 0 && prof && !IsBadReadPtr((void *)(prof + 0x10), 1) &&
          *(char *)(prof + 0x10) == 0)
          *(BYTE *)(app + 0x852) = 1; }
    g_LoadRaceData(mem, "TOWERRACE");
    DebugLog("UniversalTowerInit done (no native ctor)");
}

/* UniversalUpInit — content-driven Up board setup, no native ctor.
 * Replicates Up native ctor (0x420390, Ghidra-verified) field-by-field:
 * base Board_ctor, Up vtable (0x4D11A0 — owns slot 18 vacuum scene setup
 * 0x411540, slot 19 lifter RaceState 0x420660, slot 29 Up collision
 * 0x4119B0), name strings, difficulty flag, LoadRaceData("UPRACE").
 * +0x870 comes from Step 4 (proven app-source), +0x1508 from Step 5 (table
 * color {1,0,1} matches native Vec3), +0x4344 from Step 7 (table music
 * "Up Race" matches native), 0x436C list from Step 9a-extra (content-6
 * gated). Mesh slots (+0x4784/88/8C) stay empty — S1-driven create loads
 * and mirrors them (same pattern as Tower). */
static void UniversalUpInit(void *mem, int app) {
    g_BoardCtor(mem, app);
    *(DWORD *)mem = 0x4D11A0; /* Up board vtable */
    *(char **)((char *)mem + 0x868) = "Board (Up)";
    *(char **)((char *)mem + 0x29B4) = "UP RACE";
    *(char **)((char *)mem + 0x4344) = "Up Race";
    { int diff = *(int *)(app + 0x23C);
      DWORD prof = *(DWORD *)(app + 0x220);
      if (diff != 0 && prof && !IsBadReadPtr((void *)(prof + 0x10), 1) &&
          *(char *)(prof + 0x10) == 0)
          *(BYTE *)(app + 0x853) = 1; }
    g_LoadRaceData(mem, "UPRACE");
    DebugLog("UniversalUpInit done (no native ctor)");
}

void __cdecl UniversalBoardCtorLogic(void *mem, int app) {
    int raceIndex = g_pendingRaceIndex;
    g_lastAppParam = app;
    char buf[256];
    wsprintfA(buf, "UniversalBoardCtorLogic called: mem=%p app=0x%X raceIndex=%d", mem, app, raceIndex);
    DebugLog(buf);
    if (!mem || raceIndex < 1 || raceIndex > 15) {
        DebugLog("UniversalBoardCtorLogic: invalid params, returning");
        return;
    }
    if (!g_BoardCtor || !g_LoadRaceData) {
        DebugLog("UniversalBoardCtorLogic: function pointers not resolved");
        return;
    }

    LevelData *ld = &g_levelData[raceIndex];

    /* Step 0: Content identity (file, never slot). MUST precede Step 1 —
     * the ctor delegation below reads g_loadingContentId. */
    {
        const char *rp0 = NULL;
        char rsl0[MAX_PATH]; rsl0[0] = '\0';
        LoadRaceFiles();
        if (raceIndex>=1 && raceIndex<=15 && g_raceFiles[raceIndex][0]) rp0 = g_raceFiles[raceIndex];
        if (!rp0 || !*rp0) rp0 = g_levelData[raceIndex].meshPath;
        if (!rp0 || !*rp0) rp0 = g_meshPaths[raceIndex];
        if (rp0 && *rp0) {
            ResolveRacePath(rsl0, rp0);
            g_loadingContentId = ContentIdFromPath(rsl0);
        } else g_loadingContentId = 0;
    }

    /* Step 1: base setup. Tower content (any slot): UniversalTowerInit —
     * content-driven replica of the native Tower ctor (no native code runs).
     * Mesh slots are filled later by S1-driven create (native-prefer). */
    if (g_loadingContentId == 5) {
        DebugLog("Calling UniversalTowerInit...");
        UniversalTowerInit(mem, app);
    } else if (g_loadingContentId == 6) {
        DebugLog("Calling UniversalUpInit...");
        UniversalUpInit(mem, app);
    } else {
        DebugLog("Calling Board_ctor...");
        g_BoardCtor(mem, app);
        DebugLog("Board_ctor done");
    }

    /* Ext heap: fresh lifecycle per construction. Drop any stale entry for
     * this address first (heap reuse after quit-to-menu/restart paths that
     * bypass AdvanceRace). Otherwise a new board inherits dead mesh pointers
     * and feat bits -> use-after-free on race open (fix 2026-09-18). */
    FreeBoardExt(mem);
    void *ext = EnsureBoardExt(mem);
    if (!ext) {
        DebugLog("UniversalBoardCtorLogic: EnsureBoardExt failed");
        return;
    }

    /* Step 0 (cont.): stamp per-board content id (ext exists from here). */
    {
        const char *rp = NULL;
        char rsl[MAX_PATH]; rsl[0] = '\0';
        LoadRaceFiles();
        if (raceIndex>=1 && raceIndex<=15 && g_raceFiles[raceIndex][0]) rp = g_raceFiles[raceIndex];
        if (!rp || !*rp) rp = g_levelData[raceIndex].meshPath;
        if (!rp || !*rp) rp = g_meshPaths[raceIndex];
        if (rp && *rp) {
            ResolveRacePath(rsl, rp);
            g_loadingContentId = ContentIdFromPath(rsl);
        } else g_loadingContentId = 0;
        *(int *)((char *)ext + UNI_CONTENT_FILE) = g_loadingContentId;
        { char cd[160]; wsprintfA(cd, "ContentId=%d path='%s' mem=0x%08X slot=%d", g_loadingContentId, rsl, (DWORD)mem, raceIndex); DebugLog(cd); }
    }

    /* Step 1b: Zero-fill old per-level data offsets (0x436C-0x6500).
     * The mod moved ehVector, bumper lit, and render data to the unified
     * zone (0x6500+). But per-level RENDER functions (vtable slot 24)
     * still read from the OLD per-level offsets (e.g. Beginner reads
     * board+0x436C for ehVector, board+0x642C for bumper lit).
     * operator_new uses malloc (no zeroing), so these offsets contain
     * garbage. Zeroing them ensures:
     *   - Bumper lit floats = 0.0 → render function skips (no reflective material)
     *   - Mesh slot pointers = NULL → render function does nothing
     * This prevents crashes from garbage pointers being dereferenced.
     *
     * IMPORTANT: Start at 0x436C, NOT 0x4300. Board_ctor writes critical
     * data in 0x4300-0x436B that must be preserved:
     *   0x4340 = 1.0f (scale factor)
     *   0x434C = NULL pointer check
     *   0x4350 = -150.0f (camera offset)
     *   0x4358 = demo timer flag
     *   0x435C = demo timer count (0x9c4 = 2500)
     *   0x4368 = 0 (byte flag)
     * Zeroing these causes crashes in Scene_Update's vtable sub-functions. */
    // Vanilla board stays small; unified data now lives in ext (HEAP_ZERO_MEMORY).
    // Previous memset to 0x6000 overflowed WarmUp (~0x4400) -> heap corruption -> 0x452783 crash.
    // Safe fix: do NOT memset board+0x436C at all. Ext is already zeroed. If a level needs
    // legacy zero at 0x436C (Up lifter list), that is handled in step 9a-extra instead.
    // (Kept as no-op to avoid OOB on small boards.)
    {
        char dbg2[128];
        wsprintfA(dbg2, "memset skipped (OOB fix, board=0x%08X)", (DWORD)mem);
        DebugLog(dbg2);
    }

    /* Step 2: Set vtable. Tower/Up content keeps the vtable its
     * Universal*Init set (native class for content); other content gets
     * its slot vtable. */
    if (g_loadingContentId != 5 && g_loadingContentId != 6)
        *(DWORD *)mem = g_levelVtables[raceIndex];
    DebugLog("Vtable set");

    /* Step 3: Set board name and race title (slot table — skipped for Tower
     * and Up content, whose content names the Universal*Init already set). */
    if (g_loadingContentId != 5 && g_loadingContentId != 6) {
        *(char **)((char *)mem + UNI_BOARD_NAME) = ld->boardName;
        *(char **)((char *)mem + UNI_RACE_TITLE) = ld->raceTitle;
    }
    DebugLog("Names set");

    /* Step 4: Set +0x870 from app+0x1DC (proven source; all working runs). */
    *(DWORD *)((char *)mem + UNI_BOARD_APPVAL) = *(DWORD *)(app + 0x1DC);
    DebugLog("+0x870 set");

    /* Step 5: Write per-level color directly to board+0x1508 */
    /* Original game uses Vec3_Init+Matrix_Identity, but we write directly
       to avoid calling convention issues. Board+0x1508 is a 4-float RGBA. */
    *(float *)((char *)mem + UNI_COLOR_R) = ld->color[0];
    *(float *)((char *)mem + UNI_COLOR_G) = ld->color[1];
    *(float *)((char *)mem + UNI_COLOR_B) = ld->color[2];
    *(float *)((char *)mem + UNI_COLOR_A) = 1.0f;  /* alpha */
    DebugLog("Color set");

    /* Step 6: LoadRaceData(mem, raceName) */
    DebugLog("Calling LoadRaceData...");
    g_LoadRaceData(mem, ld->raceData);
    DebugLog("LoadRaceData done");

    /* Step 7: Set music name */
    *(char **)((char *)mem + UNI_MUSIC_NAME) = ld->musicName;
    DebugLog("Music set");

    /* Step 8: Load extra meshes — DEPRECATED Phase1 S1-driven.
     * Static g_levelData[].meshes preload removed. All meshes now lazy-loaded
     * via S1Ensure* inside UniversalCreateDynamicObjects on demand. */
    DebugLog("Step 8: S1-driven meshes (no static preload)");

    /* Step 8b: For Dizzy, also write mesh pointers to ORIGINAL offsets.
     * Unified storage is now in ext; copy to vanilla offsets for native code.
     * Guard: vanilla Dizzy board is ~0x6000, but file-swapped Dizzy-in-WarmUp
     * board is ~0x4400 → 0x4BA8 is OOB. Use BoardHasOffset (VirtualQuery+HeapSize),
     * NOT IsBadWritePtr (which only checks page writability). */
    if (g_loadingContentId == 4) {
        if (BoardHasOffset(mem, 0x4BC8, 4) && BoardHasOffset(mem, 0x4BD8, 4) && ExtHasOffset(ext, UNI_TIPPER_MESH, 4)) {
            /* Tipper mesh+render — load from ext dedicated slots */
            *(DWORD *)((char *)mem + 0x436C) = *(DWORD *)((char *)ext + UNI_TIPPER_MESH);
            *(DWORD *)((char *)mem + 0x4370) = *(DWORD *)((char *)ext + UNI_TIPPER_RENDER);
            /* WaterWheel mesh+render */
            *(DWORD *)((char *)mem + 0x4BA8) = *(DWORD *)((char *)ext + UNI_MESH_0);
            *(DWORD *)((char *)mem + 0x4BAC) = *(DWORD *)((char *)ext + UNI_MESH_1);
            /* Swirl mesh+render */
            *(DWORD *)((char *)mem + 0x4BC4) = *(DWORD *)((char *)ext + UNI_MESH_6);
            *(DWORD *)((char *)mem + 0x4BC8) = *(DWORD *)((char *)ext + UNI_MESH_11);
            /* Gluebie mesh */
            *(DWORD *)((char *)mem + 0x4374) = *(DWORD *)((char *)ext + UNI_GLUEBIE_MESH);
            /* Init angle/scale fields (original offsets) */
            *(DWORD *)((char *)mem + 0x4BC0) = 0;  /* WaterWheel scale */
            *(DWORD *)((char *)mem + 0x4BD8) = 0;  /* Swirl angle */
            DebugLog("Step 8b: Dizzy mesh pointers dual-written to original offsets");
        } else {
            DebugLog("Step 8b: Dizzy dual-write skipped (vanilla board too small for 0x4BA8)");
        }
    }

    /* Step 9: Set unlock flags */
    if (ld->unlockFlagOffset) {
        DWORD appVal = *(DWORD *)((char *)mem + BOARD_APP_PTR);
        if (appVal && !IsBadReadPtr((void *)appVal, 0x860)) {
            DWORD diff = *(DWORD *)(appVal + 0x23C);
            if (diff != 0) {
                DWORD gameMode = *(DWORD *)(appVal + 0x220);
                if (gameMode && !IsBadReadPtr((void *)gameMode, 0x20)) {
                    if (*(char *)(gameMode + 0x10) == 0) {
                        *(char *)(appVal + ld->unlockFlagOffset) = 1;
                    }
                }
            }
        }
    }
    DebugLog("Step 9 (unlock flags) done");

    /* Step 9: Per-level structural init (AthenaLists, eh_vector arrays, zero-fills) */

    /* 9a: AthenaList_Init — unified offsets >=0x6500 live in ext */
    if (g_AthenaListInit) {
        int ai;
        for (ai = 0; ai < 8 && ld->athenaListOffsets[ai]; ai++) {
            DWORD off = ld->athenaListOffsets[ai];
            void *dst = (off >= 0x6500) ? (void *)((char *)ext + off) : (void *)((char *)mem + off);
            g_AthenaListInit(dst, 0);
        }
    }
    DebugLog("Step 9a (AthenaList_Init) done");

    /* 9a-extra: Up race (level 6) needs a legacy AthenaList at board+0x436C.
     * The original Up RaceState (0x00420660) iterates this list and calls
     * each Lifter's Update() every frame.  Without it, vacuum tubes never
     * animate and E:HELPINERTIA/E:VACPOPOUT events never fire. */
    if (g_loadingContentId == 6 && g_AthenaListInit) {
        g_AthenaListInit((void *)((char *)mem + 0x436C), 0);
        DebugLog("Step 9a-extra: legacy AthenaList at board+0x436C for Up");
    }

    /* 9a-extra2: Dizzy race (level 4) needs a gluebie object list at
     * board+0x4378. Native Dizzy ctor inits it (base Board_ctor does NOT —
     * verified by disasm: it inits 0x8B8/0xCD4/0x10EC/0x2578/... but never
     * 0x436C/0x4378). Without this, GLUEBIE appends to garbage → heap
     * corruption on every Dizzy race open (fix 2026-09-18). */
    if (g_loadingContentId == 4 && g_AthenaListInit) {
        g_AthenaListInit((void *)((char *)mem + 0x4378), 0);
        DebugLog("Step 9a-extra2: gluebie list at board+0x4378 for Dizzy");
    }

    /* 9a-extra3: Tower race (level 5) needs native object lists at
     * board+0x43B8 (catapult), 0x47D0 (trapdoor), 0x4BE8 (drawbridge),
     * 0x5000 (mace). Base Board_ctor never inits them (verified live:
     * they hold small-int garbage, not list heads); appends corrupted the
     * heap on every Tower open (fix 2026-10-21). */
    if (g_loadingContentId == 5 && g_AthenaListInit) {
        g_AthenaListInit((void *)((char *)mem + 0x43B8), 0);
        g_AthenaListInit((void *)((char *)mem + 0x47D0), 0);
        g_AthenaListInit((void *)((char *)mem + 0x4BE8), 0);
        g_AthenaListInit((void *)((char *)mem + 0x5000), 0);
        DebugLog("Step 9a-extra3: Tower native lists 43B8/47D0/4BE8/5000");
    }
    /* Expert (level 8) spinner lists: board+0x4380 ("1") / +0x4798 ("2").
     * Base Board_ctor never inits them; native slot33 appended directly.
     * Init here or the spinner append corrupts the heap at load. */
    if (g_loadingContentId == 8 && g_AthenaListInit) {
        g_AthenaListInit((void *)((char *)mem + 0x4380), 0);
        g_AthenaListInit((void *)((char *)mem + 0x4798), 0);
        DebugLog("Step 9a-extra4: Expert native lists 4380/4798");
    }

    /* 9b: eh_vector — lives in ext if offset >=0x6500 */
    if (g_ehVectorCtor && g_ehVectorCtorFn && g_Vec3ListFree &&
        ld->ehVectorOffset && ld->ehVectorCount > 0) {
        void *dst = (ld->ehVectorOffset >= 0x6500) ? (void *)((char *)ext + ld->ehVectorOffset) : (void *)((char *)mem + ld->ehVectorOffset);
        g_ehVectorCtor(dst,
                       ld->ehVectorStride, ld->ehVectorCount,
                       g_ehVectorCtorFn, g_Vec3ListFree);
    }
    DebugLog("Step 9b (ehVector) done");

    /* 9c: Zero-fill — unified offsets go to ext */
    {
        int zi;
        for (zi = 0; zi < 8 && ld->zeroFillOffsets[zi]; zi++) {
            DWORD off = ld->zeroFillOffsets[zi];
            if (off >= 0x6500) *(DWORD *)((char *)ext + off) = 0;
            else *(DWORD *)((char *)mem + off) = 0;
        }
    }
    DebugLog("Step 9c (zero-fills) done");

    /* 9d: Level_AssignTexturesAndScales — mesh ptr may be in ext */
    if (g_LevelAssignTexScales) {
        int ti;
        for (ti = 0; ti < 8 && ld->assignTexOffsets[ti]; ti++) {
            DWORD off = ld->assignTexOffsets[ti];
            DWORD meshPtr = (off >= 0x6500) ? *(DWORD *)((char *)ext + off) : *(DWORD *)((char *)mem + off);
            if (meshPtr && !IsBadReadPtr((void *)meshPtr, 4)) {
                g_LevelAssignTexScales(mem, (void *)meshPtr);
            }
        }
    }
    DebugLog("Step 9d (textures) done");

    /* 9e: Sound channel — unified offset goes to ext */
    if (ld->soundChannelOffset && g_SoundGetNextChannel) {
        DWORD appVal = *(DWORD *)((char *)mem + BOARD_APP_PTR);
        if (appVal && !IsBadReadPtr((void *)appVal, 0x500)) {
            DWORD soundDevice = *(DWORD *)(appVal + 0x490);
            if (soundDevice) {
                int channel = g_SoundGetNextChannel((void *)soundDevice);
                if (ld->soundChannelOffset >= 0x6500) *(int *)((char *)ext + ld->soundChannelOffset) = channel;
                else *(int *)((char *)mem + ld->soundChannelOffset) = channel;
                if (channel && g_SceneRenderIfVisible)
                    g_SceneRenderIfVisible(channel, 0.0f);
            }
        }
    }
    DebugLog("Step 9e (sound) done");

    /* 9f: Bridge params — unified goes to ext */
    if (ld->bridgeParamOffset) {
        void *base = (ld->bridgeParamOffset >= 0x6500) ? ext : mem;
        DWORD off = ld->bridgeParamOffset;
        *(DWORD *)((char *)base + off)     = 0x42340000;  /* 45.0f */
        *(DWORD *)((char *)base + off + 4) = 0;
        *(DWORD *)((char *)base + off + 8) = 0x32;
    }
    DebugLog("Step 9f (bridge params) done");

    /* 9g: Special init — unified goes to ext */
    if (ld->specialByteOffset) {
        if (ld->specialByteOffset >= 0x6500) *(BYTE *)((char *)ext + ld->specialByteOffset) = ld->specialByteValue;
        else *(BYTE *)((char *)mem + ld->specialByteOffset) = ld->specialByteValue;
    }
    if (ld->specialDwordOffset) {
        if (ld->specialDwordOffset >= 0x6500) *(DWORD *)((char *)ext + ld->specialDwordOffset) = ld->specialDwordValue;
        else *(DWORD *)((char *)mem + ld->specialDwordOffset) = ld->specialDwordValue;
    }
    DebugLog("Step 9g (special init) done — UniversalBoardCtorLogic complete");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Board Constructor Entry — naked, handles __thiscall convention
 * ECX=mem, [ESP+4]=app, returns board ptr in EAX
 * ═══════════════════════════════════════════════════════════════════════════ */

__attribute__((naked)) void UniversalBoardCtorEntry(void) {
    __asm__ __volatile__(
        "pushl %%ebp\n\t"
        "movl  %%esp, %%ebp\n\t"
        "pushl %%ecx\n\t"          /* save mem */
        "pushl 8(%%ebp)\n\t"       /* app */
        "pushl %%ecx\n\t"          /* mem (from saved) */
        "call  _UniversalBoardCtorLogic\n\t"
        "addl  $8, %%esp\n\t"
        "popl  %%ecx\n\t"          /* restore mem to ECX */
        "movl  %%ecx, %%eax\n\t"   /* return mem in EAX */
        "popl  %%ebp\n\t"
        "ret   $4\n\t"             /* __thiscall: callee cleans 4 bytes */
        :: : "eax", "ecx", "edx", "memory"
    );
}

/* ═══════════════════════════════════════════════════════════════════════════
 * 15 naked thunks — each sets race index then JMPs to UniversalBoardCtorEntry
 * ═══════════════════════════════════════════════════════════════════════════ */

#define DEFINE_LEVEL_THUNK(NUM) \
__attribute__((naked)) static void Thunk_Level##NUM(void) { \
    __asm__ __volatile__( \
        "movl $" #NUM ", _g_pendingRaceIndex\n\t" \
        "jmp _UniversalBoardCtorEntry\n\t" \
    ); \
}

DEFINE_LEVEL_THUNK(1)
DEFINE_LEVEL_THUNK(2)
DEFINE_LEVEL_THUNK(3)
DEFINE_LEVEL_THUNK(4)
DEFINE_LEVEL_THUNK(5)
DEFINE_LEVEL_THUNK(6)
DEFINE_LEVEL_THUNK(7)
DEFINE_LEVEL_THUNK(8)
DEFINE_LEVEL_THUNK(9)
DEFINE_LEVEL_THUNK(10)
DEFINE_LEVEL_THUNK(11)
DEFINE_LEVEL_THUNK(12)
DEFINE_LEVEL_THUNK(13)
DEFINE_LEVEL_THUNK(14)
DEFINE_LEVEL_THUNK(15)

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch all 15 CALL LevelBoard_*_ctor instructions
 * ═══════════════════════════════════════════════════════════════════════════ */

static const DWORD g_ctorCallRVAs[15] = {
    0x0002712C,  /* 1=WarmUp */
    0x0002715D,  /* 2=Beginner */
    0x0002718E,  /* 3=Intermediate */
    0x000271BF,  /* 4=Dizzy */
    0x000271F0,  /* 5=Tower */
    0x00027221,  /* 6=Up */
    0x00027252,  /* 7=Neon */
    0x00027283,  /* 8=Expert */
    0x000272B4,  /* 9=Odd */
    0x000272E5,  /* 10=Toob */
    0x00027316,  /* 11=Wobbly */
    0x00027347,  /* 12=Glass */
    0x00027374,  /* 13=Sky */
    0x0002739E,  /* 14=Master */
    0x000273C8,  /* 15=Impossible */
};

static void *g_thunkPtrs[16] = {
    NULL,
    Thunk_Level1, Thunk_Level2, Thunk_Level3, Thunk_Level4,
    Thunk_Level5, Thunk_Level6, Thunk_Level7, Thunk_Level8,
    Thunk_Level9, Thunk_Level10, Thunk_Level11, Thunk_Level12,
    Thunk_Level13, Thunk_Level14, Thunk_Level15,
};

/* Universal board size: every level board allocates 0x7000 bytes (>= the
 * largest native board, Master 0x6498, plus margin). Any content in any slot
 * gets a board big enough for the largest per-level layout (Tower lists at
 * 0x5000, Dizzy fields, etc.) — no OOB writes on file/slot swaps.
 * Runs inside InstallBoardCtorHooks: for each of the 15 ctor CALL sites,
 * scan back for the operator_new CALL and rewrite its preceding push $SIZE.
 * Verification is strict: CALL target must be operator_new exactly, and the
 * 5 bytes before it must be a push imm32 (0x68). */
#define UNI_BOARD_SIZE 0x7000

static void InstallUniversalBoardSize(void) {
    DWORD newAddr = g_moduleBase + RVA_operator_new;
    int i;
    for (i = 0; i < 15; i++) {
        unsigned char *call = (unsigned char *)(g_moduleBase + g_ctorCallRVAs[i]);
        int back;
        if (IsBadReadPtr(call, 5)) continue;
        if (call[0] != 0xE8) continue;
        for (back = 6; back <= 48; back++) {
            unsigned char *c = call - back;
            if (IsBadReadPtr(c, 5)) break;
            if (c[0] != 0xE8) continue;
            if ((DWORD)(c + 5 + *(int *)(c + 1)) != newAddr) continue;
            /* c = CALL operator_new; preceding 5 bytes must be push imm32 */
            if (c[-5] != 0x68) continue;
            { DWORD oldProtect;
              VirtualProtect(c - 5, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
              *(DWORD *)(c - 4) = UNI_BOARD_SIZE;
              VirtualProtect(c - 5, 5, oldProtect, &oldProtect);
              FlushInstructionCache(GetCurrentProcess(), c - 5, 5); }
            break;
        }
    }
    DebugLog("Universal board size installed (0x7000 x15)");
}

static void InstallBoardCtorHooks(void) {
    int i;
    InstallUniversalBoardSize();
    for (i = 0; i < 15; i++) {
        unsigned char *site = (unsigned char *)(g_moduleBase + g_ctorCallRVAs[i]);
        if (IsBadReadPtr(site, 5)) continue;
        if (site[0] != 0xE8) continue;  /* verify CALL opcode */

        DWORD target = (DWORD)g_thunkPtrs[i + 1];
        DWORD rel32 = target - ((DWORD)site + 5);

        DWORD oldProtect;
        VirtualProtect(site, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
        *(DWORD *)(site + 1) = rel32;
        VirtualProtect(site, 5, oldProtect, &oldProtect);
        FlushInstructionCache(GetCurrentProcess(), site, 5);
    }
}
