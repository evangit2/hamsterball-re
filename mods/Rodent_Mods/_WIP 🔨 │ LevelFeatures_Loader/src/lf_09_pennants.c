/* Chapter 9 - per-mesh-name pennant routing (LevelFeatures.c split, new file).
 *
 * Native model (verified in binary): ONE FlagWaver (0x8C) at board+0x3F18 is
 * shared by every FLAG mesh on the level. CreateFlags (0x40C0F0) builds it
 * once (NULL-guarded) and appends each FLAG S1 ref to board+0x2160. The
 * shared renderer walks board+0x2160 and calls FlagRender (0x46B070) per
 * entry, re-reading board+0x3F18 each time. Tower's scene loader tail
 * (0x40D785+) flips that one waver to pennant/125x75, so all Tower flags
 * pennant. There is exactly one pennant write in the whole game.
 *
 * This chapter adds designer control without touching native paths:
 *  - FLAG*    -> native waver (rectangle on every level, Tower included)
 *  - PENNANT* -> dedicated pennant waver (triangle on every level)
 * Native CreateFlags only collects the FLAG prefix, so PENNANT* S1 refs are
 * appended to board+0x2160 here (pointer-deduped). A 5-byte CALL detour at
 * the sole FlagRender call site (0x40B512) routes PENNANT* entries to the
 * dedicated waver. Every fallback keeps the incoming (native) waver, so a
 * missed name lookup degrades to current behavior, never a crash.
 */

#define BOARD_FLAG_WAVER  0x3F18  /* single shared FlagWaver* (0x8C obj) */
#define BOARD_FLAG_LIST   0x2160  /* AthenaList of flag S1 refs */
#define FLAGWAVER_SIZE    0x8C
/* UNI_PENNANT_WAVER (0xB9FC ext slot) is defined in lf_00_core.c */

#define RVA_FlagWaver_ctor      0x0006AF30  /* __thiscall(mem, gfx) RET 4 */
#define RVA_FlagRebuild         0x0006A8A0  /* __thiscall(obj) RET 0 */
#define RVA_FlagRender          0x0006B070  /* __thiscall(waver, buf, entry) RET 8 */
#define RVA_FlagRenderCallSite  0x0000B512  /* sole caller: E8 59 FB 05 00 */

typedef void* (__thiscall *FlagWaverCtor_t)(void *mem, void *gfx);
typedef void (__thiscall *FlagRebuild_t)(void *obj);
typedef void (__thiscall *FlagRender_t)(void *waver, void *buf, void *entry);

static FlagWaverCtor_t p_FlagWaverCtor = NULL;
static FlagRebuild_t   p_FlagRebuild = NULL;
static FlagRender_t    p_OrigFlagRender = NULL;
static void *g_pennantBoard = NULL;
static int   g_pennantRace = 0;
static int   g_flagHookInstalled = 0;

static void Pennant_InitPointers(void) {
    if (p_FlagWaverCtor && p_FlagRebuild && p_OrigFlagRender) return;
    if (!g_moduleBase) return;
    p_FlagWaverCtor  = (FlagWaverCtor_t)(g_moduleBase + RVA_FlagWaver_ctor);
    p_FlagRebuild    = (FlagRebuild_t)(g_moduleBase + RVA_FlagRebuild);
    p_OrigFlagRender = (FlagRender_t)(g_moduleBase + RVA_FlagRender);
}

/* Dedicated pennant waver for the active board (NULL = use native). */
static void *Pennant_GetWaver(void) {
    if (!g_pennantBoard || IsBadReadPtr(g_pennantBoard, 0x40)) return NULL;
    void *ext = GetBoardExt(g_pennantBoard);
    if (!ext) return NULL;
    if (IsBadReadPtr((char*)ext + UNI_PENNANT_WAVER, 4)) return NULL;
    void *w = *(void**)((char*)ext + UNI_PENNANT_WAVER);
    if (!w || IsBadReadPtr(w, FLAGWAVER_SIZE)) return NULL;
    return w;
}

/* Detour for FlagRender: same __thiscall signature, callee cleans 8 bytes.
 * entryPlus2C = S1 entry + 0x2C (render loop pushes esi after +0x2C advance),
 * so the S1 name sits at *(entryPlus2C - 0x2C). Unknown/unreadable -> native. */
static void __thiscall Hook_FlagRender(void *waver, void *buf, void *entryPlus2C) {
    void *use = waver;
    if (entryPlus2C) {
        char *base = (char*)entryPlus2C - 0x2C;
        if (!IsBadReadPtr(base, 4)) {
            char *nm = *(char**)base;
            if (nm && !IsBadReadPtr(nm, 8) && my_strnicmp(nm, "PENNANT", 7) == 0) {
                void *pw = Pennant_GetWaver();
                if (pw) use = pw;
            }
        }
    }
    if (p_OrigFlagRender) p_OrigFlagRender(use, buf, entryPlus2C);
}

static void InstallFlagRenderHook(void) {
    if (g_flagHookInstalled) return;
    if (!g_moduleBase) return;
    Pennant_InitPointers();
    unsigned char *site = (unsigned char*)(g_moduleBase + RVA_FlagRenderCallSite);
    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQuery(site, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT) return;
    if (site[0] != 0xE8 || site[1] != 0x59 || site[2] != 0xFB ||
        site[3] != 0x05 || site[4] != 0x00) {
        DebugLog("InstallFlagRenderHook: unexpected bytes, skipping");
        return;
    }
    DWORD old;
    if (!VirtualProtect(site, 5, PAGE_EXECUTE_READWRITE, &old)) return;
    site[0] = 0xE8;
    *(DWORD*)(site + 1) = (DWORD)((char*)&Hook_FlagRender - (char*)(site + 5));
    VirtualProtect(site, 5, old, &old);
    FlushInstructionCache(GetCurrentProcess(), site, 5);
    g_flagHookInstalled = 1;
    DebugLog("InstallFlagRenderHook: FlagRender routed via Hook_FlagRender");
}

/* True if obj is already listed in board+0x2160 (avoids double-render). */
static int Pennant_InList(void *board, void *obj) {
    if (IsBadReadPtr((char*)board + BOARD_FLAG_LIST, 0x410)) return 0;
    int cnt = *(int*)((char*)board + BOARD_FLAG_LIST + 4);
    void **arr = *(void***)((char*)board + BOARD_FLAG_LIST + 0x40C);
    if (cnt < 0 || cnt > 1024 || !arr || IsBadReadPtr(arr, 4)) return 0;
    int i;
    for (i = 0; i < cnt; i++) {
        void *it = NULL;
        if (!IsBadReadPtr(&arr[i], 4)) it = arr[i];
        if (it && it == obj) return 1;
    }
    return 0;
}

/* Post-Board_Setup extras: Tower native look + PENNANT* collection + waver.
 * Called from UniversalConstructor Step 6. All-native fallbacks on failure. */
void Pennant_PostSetup(void *board, void *ext, int raceIndex) {
    g_pennantBoard = board;
    g_pennantRace = raceIndex;
    (void)raceIndex;  /* names decide shapes; no per-race exceptions */
    Pennant_InitPointers();
    if (!board || !ext) return;

    /* (1) No Tower exception: FLAG* stays rectangular even on Tower race 5.
     * (The native scene-loader tail pennanted the shared waver; we
     * deliberately do not replicate it — the mesh name is the only switch.) */

    /* (2) Collect PENNANT* S1 refs into board+0x2160 (native only takes
     * FLAG*). Same S1-walk shape as the TarBubble Step-6 block. */
    int pennCount = 0;
    DWORD meshWorld = 0;
    if (!IsBadReadPtr((char*)board + BOARD_MESHWORLD, 4))
        meshWorld = *(DWORD*)((char*)board + BOARD_MESHWORLD);
    if (meshWorld && !IsBadReadPtr((void*)meshWorld, 0x500) &&
        g_AthenaListAppend && g_AthenaListGetIterator) {
        DWORD objDb = *(DWORD*)(meshWorld + 0x480);
        if (objDb && !IsBadReadPtr((void*)objDb, 0x20)) {
            int iter = g_AthenaListGetIterator((void*)(objDb + 0x894));
            *(DWORD*)(objDb + 0x89C + iter * 4) = 0;
            int count = *(int*)(objDb + 0x898);
            if (count > 0) {
                DWORD *array = *(DWORD**)(objDb + 0xCA0);
                if (array && !IsBadReadPtr((void*)array, count * 4)) {
                    *(DWORD*)(objDb + 0x89C + iter * 4) = 1;
                    int idx = 0;
                    while (idx < count) {
                        DWORD *obj = (DWORD*)array[idx];
                        if (obj && !IsBadReadPtr((void*)obj, 4)) {
                            char *name = *(char**)obj;
                            if (name && !IsBadReadPtr(name, 8) &&
                                my_strnicmp(name, "PENNANT", 7) == 0) {
                                if (!Pennant_InList(board, (void*)obj)) {
                                    g_AthenaListAppend(
                                        (void*)((char*)board + BOARD_FLAG_LIST),
                                        (int)obj);
                                    pennCount++;
                                }
                            }
                        }
                        idx++;
                        *(DWORD*)(objDb + 0x89C + iter * 4) = idx;
                    }
                }
            }
        }
    }

    /* (3) Dedicated pennant waver, only when PENNANT* refs exist. */
    if (pennCount > 0 && p_FlagWaverCtor && p_FlagRebuild && g_operatorNew) {
        void *slot = NULL;
        if (!IsBadReadPtr((char*)ext + UNI_PENNANT_WAVER, 4))
            slot = *(void**)((char*)ext + UNI_PENNANT_WAVER);
        if (!slot || IsBadReadPtr(slot, FLAGWAVER_SIZE)) {
            DWORD app = 0;
            if (!IsBadReadPtr((char*)board + BOARD_APP_PTR, 4))
                app = *(DWORD*)((char*)board + BOARD_APP_PTR);
            void *gfx = NULL;
            if (app && !IsBadReadPtr((void*)app, 0x200))
                gfx = *(void**)((char*)app + 0x174);
            if (gfx) {
                void *mem = g_operatorNew(FLAGWAVER_SIZE);
                void *w = NULL;
                if (mem) w = (void*)p_FlagWaverCtor(mem, gfx);
                if (w && !IsBadReadPtr(w, FLAGWAVER_SIZE)) {
                    *(BYTE*)((char*)w + 0x80) = 1;
                    *(DWORD*)((char*)w + 0x10) = 0x42FA0000;
                    *(DWORD*)((char*)w + 0x14) = 0x42960000;
                    p_FlagRebuild(w);
                    *(void**)((char*)ext + UNI_PENNANT_WAVER) = w;
                    {
                        char lg[96];
                        wsprintfA(lg, "Pennant: dedicated waver built, refs=%d", pennCount);
                        DebugLog(lg);
                    }
                }
            }
        }
    }
}
