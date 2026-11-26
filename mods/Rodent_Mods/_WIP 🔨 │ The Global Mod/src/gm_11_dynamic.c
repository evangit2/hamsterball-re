/* Chapter 11 - Dynamic drawer cabinet (The Global Mod v01).
 * S1-driven: count instances per type, grow ext beyond 0xC000, index table
 * remembers which drawer each S1 instance lives in. Legacy singletons stay
 * as index-0 aliases so existing feature blocks keep working.
 */
#define GM_DYN_BASE   0xBD00  /* header inside fixed ext (free: 0xBBC8-0xC000) */
#define GM_DYN_MAGIC  0x474D4459  /* 'GMDY' */
#define GM_DYN_MAXTYPES 16
#define GM_DYN_PERSLOT 64     /* overflow drawers per family (Neon needs 22+, Impossible ~45) */
static void Gm_LogCreated(const char *line);
#define GM_DYN_OVF_PTR  (GM_DYN_BASE + 0x60 + 16 * 4)  /* void* overflow block */
#define GM_DYN_OVF_CAP  (GM_DYN_BASE + 0x60 + 16 * 4 + 4)  /* int slots per family */
/* Drawer slot = 64B (16 words). Word map (family overlays share slots):
 *  W0 mesh  W1 render  W2 obj  W3 x  W4 y  W5 z  W6 rx  W7 ry  W8 rz
 *  W9 s1idx  W10 A  W11 B  W12 C  W13 D  W14 E  W15 spare
 * Overlays: WHEEL: A=angle. SWIRL2: A=angle. WINDMILL: A=angle B=speed
 *  C=state D=counter E=decay. CHOMPER: A=cs B=liftY C=state D=counter.
 *  BRIDGE: A=angle C=state D=counter, pivot xyz in W3-5, deck render in W1. */
#define GM_DYN_SLOT_SIZE 64

/* type ids: each gets its own drawer family */
#define GMD_TIPPER    0
#define GMD_WATER     1
#define GMD_SWIRL     2
#define GMD_GLUEBIE   3
#define GMD_CATAPULT  4
#define GMD_MACE      5
#define GMD_DRAWBRIDGE 6
#define GMD_WINDMILL  7
#define GMD_TRAPDOOR  8
#define GMD_CHOMPER   9
#define GMD_TURRET    10
#define GMD_BONK      11  /* BONK/BRIDGE/SPINNER family */
#define GMD_FANSAW    12  /* FAN/SAWBLADE/BELL/JUDGE family */
#define GMD_WOBBLY    13  /* WOBBLY/WAVY */
#define GMD_POPCYL    14  /* POPCYLINDER/POPDOOR/CLOUDSCAPE */
#define GMD_LIFTER    15  /* LIFTER/SPEEDCYL/TIMEBUTTON/LOOPER/GEAR/PEND/ROT */

static const char *Gm_TypeName(int t) {
    switch (t) {
        case 0: return "TIPPER";
        case 1: return "WATER";
        case 2: return "SWIRL";
        case 3: return "GLUEBIE";
        case 4: return "CATAPULT";
        case 5: return "MACE";
        case 6: return "DRAWBRIDGE";
        case 7: return "WINDMILL";
        case 8: return "TRAPDOOR";
        case 9: return "CHOMPER";
        case 10: return "TURRET";
        case 11: return "BONK";
        case 12: return "FANSAW";
        case 13: return "WOBBLY";
        case 14: return "POPCYL";
        case 15: return "LIFTER";
        default: return "?";
    }
}

static int Gm_HasSub(const char *hay, const char *needle) {
    int hl, nl, i;
    if (!hay || !needle || !needle[0]) return 0;
    hl = strlen(hay); nl = strlen(needle);
    if (nl > hl) return 0;
    for (i = 0; i <= hl - nl; i++) {
        if (my_strnicmp(hay + i, needle, nl) == 0) return 1;
    }
    return 0;
}

static int Gm_TypeForName(const char *name) {
    if (!name) return -1;
    /* S1 names are mesh paths (Levels\\Level3-Tipper) or N:/E: refs.
     * Match by substring so both forms route. */
    if (Gm_HasSub(name, "TIPPER")) return GMD_TIPPER;
    if (Gm_HasSub(name, "WATERWHEEL") || Gm_HasSub(name, "WHEELEMBED")) return GMD_WATER;
    if (Gm_HasSub(name, "SWIRL") || Gm_HasSub(name, "TarBubble")) return GMD_SWIRL;
    if (Gm_HasSub(name, "GLUEBIE")) return GMD_GLUEBIE;
    if (Gm_HasSub(name, "CATAPULT")) return GMD_CATAPULT;
    if (Gm_HasSub(name, "MACE")) return GMD_MACE;
    if (Gm_HasSub(name, "DRAWBRIDGE")) return GMD_DRAWBRIDGE;
    if (Gm_HasSub(name, "WINDMILL")) return GMD_WINDMILL;
    if (Gm_HasSub(name, "TRAPDOOR")) return GMD_TRAPDOOR;
    if (Gm_HasSub(name, "CHOMPER")) return GMD_CHOMPER;
    if (Gm_HasSub(name, "TURRET")) return GMD_TURRET;
    if (Gm_HasSub(name, "BONK") || Gm_HasSub(name, "BRIDGE") || Gm_HasSub(name, "SPINNER")) return GMD_BONK;
    if (Gm_HasSub(name, "FAN") || Gm_HasSub(name, "SAW") || Gm_HasSub(name, "BELL") || Gm_HasSub(name, "JUDGE")) return GMD_FANSAW;
    if (Gm_HasSub(name, "WOBBLY") || Gm_HasSub(name, "WAVY")) return GMD_WOBBLY;
    if (Gm_HasSub(name, "POPCYLINDER") || Gm_HasSub(name, "POPDOOR") || Gm_HasSub(name, "BONKCYLINDER") || Gm_HasSub(name, "CLOUDSCAPE")) return GMD_POPCYL;
    if (Gm_HasSub(name, "LIFTER") || Gm_HasSub(name, "SPEEDCYL") || Gm_HasSub(name, "TIMEBUTTON") || Gm_HasSub(name, "LOOPER") || Gm_HasSub(name, "GEAR") || Gm_HasSub(name, "PENDULUM") || Gm_HasSub(name, "ROTATOR") || Gm_HasSub(name, "BLOCKDAWG") || Gm_HasSub(name, "FALLOUT") || Gm_HasSub(name, "NEONPLATFORM") || Gm_HasSub(name, "DFLOOR") || Gm_HasSub(name, "TRODE") || Gm_HasSub(name, "MAGNIF")) return GMD_LIFTER;
    return -1;
}

/* Count S1 instances per drawer family. Mirrors ScanS1 walk (objDb+0x894 list). */
static void Gm_CountS1(void *meshWorld, int *counts) {
    int t;
    for (t = 0; t < GM_DYN_MAXTYPES; t++) counts[t] = 0;
    if (!meshWorld) return;
    {
        MEMORY_BASIC_INFORMATION mbi;
        DWORD objDb;
        int iter, count, idx, scan;
        DWORD *array;
        DWORD savedIter;
        if (!VirtualQuery(meshWorld, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT) return;
        objDb = *(DWORD *)((char *)meshWorld + 0x480);
        if (!objDb) return;
        if (!VirtualQuery((void *)objDb, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT) return;
        if (!g_AthenaListGetIterator || !g_AthenaListGetSize) return;
        if (!VirtualQuery((void *)(objDb + 0x894), &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT) return;
        iter = g_AthenaListGetIterator((void *)(objDb + 0x894));
        if (iter < 0 || iter > 16) return;
        if (!VirtualQuery((void *)(objDb + 0x89C + iter * 4), &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT) return;
        savedIter = *(DWORD *)(objDb + 0x89C + iter * 4);
        *(DWORD *)(objDb + 0x89C + iter * 4) = 0;
        if (!VirtualQuery((void *)(objDb + 0x898), &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT) { *(DWORD *)(objDb + 0x89C + iter * 4) = savedIter; return; }
        count = *(int *)(objDb + 0x898);
        if (count <= 0 || count > 8192) { *(DWORD *)(objDb + 0x89C + iter * 4) = savedIter; return; }
        if (!VirtualQuery((void *)(objDb + 0xCA0), &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT) { *(DWORD *)(objDb + 0x89C + iter * 4) = savedIter; return; }
        array = *(DWORD **)(objDb + 0xCA0);
        if (!array) { *(DWORD *)(objDb + 0x89C + iter * 4) = savedIter; return; }
        if (!VirtualQuery(array, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT) { *(DWORD *)(objDb + 0x89C + iter * 4) = savedIter; return; }
        *(DWORD *)(objDb + 0x89C + iter * 4) = 1;
        scan = count;
        for (idx = 0; idx < scan; idx++) {
            DWORD *obj = (DWORD *)array[idx];
            char *name;
            int ty;
            if (!obj) continue;
            if (!VirtualQuery(obj, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT) continue;
            name = *(char **)obj;
            if (!name) continue;
            if (!VirtualQuery(name, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT) continue;
            ty = Gm_TypeForName(name);
            if (ty >= 0 && ty < GM_DYN_MAXTYPES) {
                if (counts[ty] < 64) counts[ty]++;
            }
        }
        *(DWORD *)(objDb + 0x89C + iter * 4) = savedIter;
    }
}

/* Header lives at ext+GM_DYN_BASE (fixed). Overflow drawers live at ext+0xC000. */
static int *Gm_DynCounts(void *ext) { return (int *)((char *)ext + GM_DYN_BASE + 8); }
static int Gm_DynExtra(void *ext) { return *(int *)((char *)ext + GM_DYN_BASE + 8 + 16 * 4); }
/* Separate overflow block (NOT ext growth): allocated on first multi-
 * instance create, pointer kept in fixed ext. Safe at any time — ext never
 * moves, so no stale-pointer class. Layout: type*16 slots of 64B. */
static void *Gm_EnsureOverflow(void *ext) {
    void *blk;
    if (!ext) return NULL;
    if (*(DWORD *)((char *)ext + GM_DYN_BASE) != GM_DYN_MAGIC) return NULL;
    blk = *(void **)((char *)ext + GM_DYN_OVF_PTR);
    if (blk) return blk;
    blk = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
        (DWORD)(GM_DYN_MAXTYPES * GM_DYN_PERSLOT * GM_DYN_SLOT_SIZE));
    if (!blk) return NULL;
    *(void **)((char *)ext + GM_DYN_OVF_PTR) = blk;
    *(int *)((char *)ext + GM_DYN_OVF_CAP) = GM_DYN_PERSLOT;
    { char ob[96]; wsprintfA(ob, "DYN: overflow block=%p cap=%d/family",
        blk, GM_DYN_PERSLOT); DebugLog(ob); Gm_LogCreated(ob); }
    return blk;
}
static void *Gm_DynSlotPtr(void *ext, int type, int idx) {
    /* idx 0 = legacy singleton (caller uses existing UNI_* slot).
     * idx>=1 = overflow block slot (type*16 + idx-1). */
    void *blk;
    int cap;
    if (!ext || type < 0 || type >= GM_DYN_MAXTYPES || idx < 1) return NULL;
    if (*(DWORD *)((char *)ext + GM_DYN_BASE) != GM_DYN_MAGIC) return NULL;
    blk = *(void **)((char *)ext + GM_DYN_OVF_PTR);
    cap = *(int *)((char *)ext + GM_DYN_OVF_CAP);
    if (!blk || cap <= 0) return NULL;
    if (idx > cap) return NULL;
    return (void *)((char *)blk +
        (DWORD)((type * GM_DYN_PERSLOT + (idx - 1)) * GM_DYN_SLOT_SIZE));
}


#define GM_DYN_CURSOR_BASE (GM_DYN_BASE + 0x60)  /* 16 x int create cursors */

/* Double-fire suppression (Up-crash fix 2026-11-25): the same board+type+obj
 * arriving twice in a row (native re-create path) reuses its idx instead of
 * appending a duplicate drawer. Board-scoped so address reuse can't skip a
 * legit first create. NULL objs never suppress (deferred-obj instances). */
static void *g_dynLastBoard[GM_DYN_MAXTYPES] = {0};
static DWORD g_dynLastObj[GM_DYN_MAXTYPES] = {0};
static int g_dynLastIdx[GM_DYN_MAXTYPES] = {0};

/* Created-log: game-dir file log (DebugLog is globally off).
 * Header per level load + one line per successful drawer registration. */
static void Gm_LogCreated(const char *line) {
    HANDLE hf;
    DWORD bw = 0;
    if (!line || !line[0]) return;
    hf = CreateFileA("globalmod_created.txt", GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hf == INVALID_HANDLE_VALUE) return;
    SetFilePointer(hf, 0, NULL, FILE_END);
    WriteFile(hf, line, strlen(line), &bw, NULL);
    WriteFile(hf, "\r\n", 2, &bw, NULL);
    CloseHandle(hf);
}

/* Per-create registration: remembers which drawer each S1 instance got.
 * idx 0 = legacy singleton (existing UNI_* slots drive it).
 * idx>=1 = overflow drawer written here. Returns idx, -1 on error. */
extern int g_loadingContentId; /* set per load in UniversalConstructor (gm_00_core.c) */
static const char *Gm_RaceName(int id) {
    switch (id) {
        case 1: return "WARM-UP"; case 2: return "BEGINNER";
        case 3: return "INTERMEDIATE"; case 4: return "DIZZY";
        case 5: return "TOWER"; case 6: return "UP";
        case 7: return "NEON"; case 8: return "EXPERT";
        case 9: return "ODD"; case 10: return "TOOB";
        case 11: return "WOBBLY"; case 12: return "GLASS";
        case 13: return "SKY"; case 14: return "MASTER";
        case 15: return "IMPOSSIBLE"; default: return "?";
    }
}
/* Public skip-log line (mesh-missing etc.): gm_05_create.c fwd-declares this.
 * DebugLog is globally off, so skips would otherwise vanish silently. */
void Gm_LogSkip(const char *line) { Gm_LogCreated(line); }
static int Gm_CreateSlot(void *board, void *ext, int type,
    float x, float y, float z, float rx, float ry, float rz, void *obj,
    const char *s1name) {
    int *cursors;
    int idx, *counts;
    void *slot;
    char dbg[160];
    if (!ext || type < 0 || type >= GM_DYN_MAXTYPES) return -1;
    if (*(DWORD *)((char *)ext + GM_DYN_BASE) != GM_DYN_MAGIC) return -1;
    /* Ext<->board binding check (Up-crash fix 2026-11-25): hooks fall back
     * to a passed-in ext when GetBoardExt fails closed on a dangling block
     * (freed-but-committed). Writing drawers into a dead ext heap-corrupts
     * and the crash lands pages later in unrelated menu code. Refuse
     * orphan/stale ext — re-resolve from the map, else log + skip. */
    {
        int ei, bound = 0;
        for (ei = 0; ei < MAX_EXT_MAP; ei++)
            if (g_extMap[ei].ext == ext) { bound = (g_extMap[ei].board == board); break; }
        if (!bound) {
            void *live = GetBoardExt(board);
            if (!live || *(DWORD *)((char *)live + GM_DYN_BASE) != GM_DYN_MAGIC) {
                char nb[160];
                wsprintfA(nb, "DYNCREATE: %s REJECTED stale/orphan ext=0x%08X board=0x%08X",
                    Gm_TypeName(type), (DWORD)ext, (DWORD)board);
                DebugLog(nb);
                Gm_LogCreated(nb);
                return -1;
            }
            ext = live;
        }
    }
    /* Same board+type+obj twice in a row = native double-fire: reuse idx. */
    if (obj && g_dynLastBoard[type] == board && g_dynLastObj[type] == (DWORD)obj)
        return g_dynLastIdx[type];
    cursors = (int *)((char *)ext + GM_DYN_CURSOR_BASE);
    counts = Gm_DynCounts(ext);
    idx = cursors[type];
    /* Clamp: never exceed scanned count, never exceed 64. */
    if (idx < 0) idx = 0;
    if (idx >= 64) return -1;
    if (idx < counts[type]) {
        /* valid instance */
    } else {
        /* Unscanned/late S1 (sub-mesh refs the scan missed) — still track. */
    }
    cursors[type] = idx + 1;
    if (idx == 0) {
        /* Legacy singleton owns drawer 0 — nothing extra to write. */
        g_dynLastBoard[type] = board;
        g_dynLastObj[type] = (DWORD)obj;
        g_dynLastIdx[type] = 0;
        { char dbg0[192];
          wsprintfA(dbg0, "DYNCREATE: %s #0 obj=0x%08X pos=%d,%d,%d s1='%.48s'",
            Gm_TypeName(type), (DWORD)obj, (int)x, (int)y, (int)z,
            s1name ? s1name : "?");
          DebugLog(dbg0);
          Gm_LogCreated(dbg0); }
        return 0;
    }
    if (!Gm_EnsureOverflow(ext)) return -1;
    slot = Gm_DynSlotPtr(ext, type, idx);
    if (!slot) {
        /* Over cap (shouldn't happen now that PERSLOT==cursor clamp): log it
         * so silent drawer drops never hide again. */
        char dp[128];
        wsprintfA(dp, "DYNCREATE: %s #%d DROPPED cap=%d s1='%.48s'",
            Gm_TypeName(type), idx, GM_DYN_PERSLOT, s1name ? s1name : "?");
        DebugLog(dp);
        Gm_LogCreated(dp);
        return -1;
    }
    /* 64B word map: 0 mesh,1 render,2 obj,3-5 xyz,6-8 rxyz,9 s1idx,10-15 state */
    ((DWORD *)slot)[0] = 0;            /* mesh (shared singleton used) */
    ((DWORD *)slot)[1] = 0;            /* render */
    ((DWORD *)slot)[2] = (DWORD)obj;   /* instance object */
    ((float *)slot)[3] = x;
    ((float *)slot)[4] = y;
    ((float *)slot)[5] = z;
    ((float *)slot)[6] = rx;
    ((float *)slot)[7] = ry;
    ((float *)slot)[8] = rz;
    ((int *)slot)[9] = idx;
    ((float *)slot)[10] = 0.0f;
    ((float *)slot)[11] = 0.0f;
    ((float *)slot)[12] = 0.0f;
    ((float *)slot)[13] = 0.0f;
    ((float *)slot)[14] = 0.0f;
    ((float *)slot)[15] = 0.0f;
    wsprintfA(dbg, "DYNCREATE: %s #%d obj=0x%08X pos=%d,%d,%d s1='%.48s'",
        Gm_TypeName(type), idx, (DWORD)obj, (int)x, (int)y, (int)z,
        s1name ? s1name : "?");
    DebugLog(dbg);
    Gm_LogCreated(dbg);
    g_dynLastBoard[type] = board;
    g_dynLastObj[type] = (DWORD)obj;
    g_dynLastIdx[type] = idx;
    return idx;
}

/* Reset create cursors at level start (create runs fresh per board). */
static void Gm_ResetCursors(void *ext) {
    int t;
    int *cursors;
    if (!ext) return;
    if (*(DWORD *)((char *)ext + GM_DYN_BASE) != GM_DYN_MAGIC) return;
    cursors = (int *)((char *)ext + GM_DYN_CURSOR_BASE);
    for (t = 0; t < GM_DYN_MAXTYPES; t++) cursors[t] = 0;
}

static void Gm_SetSlotRender(void *ext, int type, int idx, void *render) {
    void *slot;
    if (!ext || idx < 1) return;
    slot = Gm_DynSlotPtr(ext, type, idx);
    if (slot) ((DWORD *)slot)[1] = (DWORD)render;
}
static void Gm_SetSlotMesh(void *ext, int type, int idx, void *mesh) {
    void *slot;
    if (!ext || idx < 1) return;
    slot = Gm_DynSlotPtr(ext, type, idx);
    if (slot) ((DWORD *)slot)[0] = (DWORD)mesh;
}
static int Gm_DrawerCount(void *ext, int type) {
    /* Live truth = create cursors (actual registrations), NOT the Step-1b
     * census (the S1 list fills during Setup, after the census runs). */
    int *cursors;
    if (!ext || type < 0 || type >= GM_DYN_MAXTYPES) return 0;
    if (*(DWORD *)((char *)ext + GM_DYN_BASE) != GM_DYN_MAGIC) return 0;
    cursors = (int *)((char *)ext + GM_DYN_CURSOR_BASE);
    return cursors[type];
}
/* End-of-load summary: true drawer totals per family. */
static void Gm_WriteSummary(void *board, void *ext) {
    if (!board || !ext) return;
    if (*(DWORD *)((char *)ext + GM_DYN_BASE) != GM_DYN_MAGIC) return;
    { int *c = (int *)((char *)ext + GM_DYN_CURSOR_BASE);
      char sm[288];
      wsprintfA(sm, "SUMMARY board=%p (TIPPER=%d WATER=%d SWIRL=%d GLUE=%d CAT=%d MACE=%d DRAW=%d WIND=%d TRAP=%d CHOMP=%d TUR=%d BONK=%d FANS=%d WOB=%d POP=%d LIFT=%d)",
        board, c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7],
        c[8], c[9], c[10], c[11], c[12], c[13], c[14], c[15]);
      DebugLog(sm);
      Gm_LogCreated(sm); }
}

/* Stamp the cabinet header + S1 census (informational). Drawers allocate
 * on demand via the separate overflow block — ext never grows/moves, so no
 * stale-pointer class at any hook point. Returns ext unchanged. */
static void *Gm_EnsureDynamic(void *board, void *ext, void *meshWorld) {
    int counts[16];
    int t;
    char dbg[256];
    if (!board || !ext || !meshWorld) return ext;
    Gm_CountS1(meshWorld, counts);
    /* Stamp header in fixed region. */
    *(DWORD *)((char *)ext + GM_DYN_BASE) = GM_DYN_MAGIC;
    *(int *)((char *)ext + GM_DYN_BASE + 4) = GM_DYN_MAXTYPES;
    for (t = 0; t < GM_DYN_MAXTYPES; t++) Gm_DynCounts(ext)[t] = counts[t];
    { int zi; int *zc = (int *)((char *)ext + GM_DYN_CURSOR_BASE); for (zi = 0; zi < GM_DYN_MAXTYPES; zi++) zc[zi] = 0; }
    *(void **)((char *)ext + GM_DYN_OVF_PTR) = NULL;
    *(int *)((char *)ext + GM_DYN_OVF_CAP) = 0;
    wsprintfA(dbg, "DYN: census (TIPPER=%d WATER=%d SWIRL=%d GLUE=%d CAT=%d MACE=%d DRAW=%d WIND=%d TRAP=%d CHOMP=%d TUR=%d BONK=%d FANS=%d WOB=%d POP=%d LIFT=%d) overflow=on-demand",
        counts[0], counts[1], counts[2], counts[3], counts[4], counts[5],
        counts[6], counts[7], counts[8], counts[9], counts[10], counts[11],
        counts[12], counts[13], counts[14], counts[15]);
    DebugLog(dbg);
    { char hdr[288];
      wsprintfA(hdr, "LEVEL board=%p census (TIPPER=%d WATER=%d SWIRL=%d GLUE=%d CAT=%d MACE=%d DRAW=%d WIND=%d TRAP=%d CHOMP=%d TUR=%d BONK=%d FANS=%d WOB=%d POP=%d LIFT=%d)",
        board,
        counts[0], counts[1], counts[2], counts[3], counts[4], counts[5],
        counts[6], counts[7], counts[8], counts[9], counts[10], counts[11],
        counts[12], counts[13], counts[14], counts[15]);
      Gm_LogCreated(hdr); }
    { char rl[64];
      wsprintfA(rl, "RACE content=%d name=%s", g_loadingContentId,
        Gm_RaceName(g_loadingContentId));
      Gm_LogCreated(rl); }
    return ext;
}