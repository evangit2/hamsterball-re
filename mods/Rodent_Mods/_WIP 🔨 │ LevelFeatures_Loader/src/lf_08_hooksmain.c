/* Chapter 8 - UniversalConstructor + hooks + DebugLog + vtable patches + PatchThread + DllMain (LevelFeatures.c lines 6105-6930) */
/* Chapter-9 pennants (lf_09_pennants.c, included after this file). */
void Pennant_PostSetup(void *board, void *ext, int raceIndex);
static void InstallFlagRenderHook(void);
static void UniversalConstructor(void *board, int raceIndex) {
    void* ext = EnsureBoardExt(board);
    char buf[256];
    wsprintfA(buf, "UniversalConstructor called: board=%p raceIndex=%d", board, raceIndex);
    DebugLog(buf);
    if (!board || raceIndex < 1 || raceIndex > 15) {
        DebugLog("UniversalConstructor: invalid params");
        return;
    }
    if (!g_operatorNew || !g_LevelMeshWorldCtor || !g_LevelRenderCtor ||
        !g_LevelInitScene) {
        DebugLog("UniversalConstructor: function pointers not resolved");
        return;
    }

    /* RaceFiles.txt overrides everything: Race 1..15 -> file. Then LevelData, then g_meshPaths.
     * Bare names like "LoopyRace" resolve to "levels\\LoopyRace\\LoopyRace" if that
     * subfolder file exists, otherwise fallback to "levels\\LoopyRace". */
    LoadRaceFiles();
    const char *rawPath = NULL;
    if (raceIndex>=1 && raceIndex<=15 && g_raceFiles[raceIndex][0]) rawPath = g_raceFiles[raceIndex];
    if (!rawPath || !*rawPath) rawPath = g_levelData[raceIndex].meshPath;
    if (!rawPath || !*rawPath) rawPath = g_meshPaths[raceIndex];
    if (!rawPath) return;
    char resolved[MAX_PATH]; ResolveRacePath(resolved, rawPath);
    const char *meshPath = resolved;
    {
        char dbg2[256]; wsprintfA(dbg2, "UniversalConstructor meshPath race=%d raw='%s' resolved='%s'", raceIndex, rawPath, meshPath);
        DebugLog(dbg2);
    }

    /* Extract level dir for texture/sound/sub-mesh fallback.
     * e.g. "levels\\MyLevel\\MyLevel" -> "levels\\MyLevel\\"
     *      "levels\\level1" -> "levels\\" (no extra dir, fallback is just levels\\) */
    {
        const char *lastSlash = strrchr(resolved, '\\');
        const char *lastSlash2 = strrchr(resolved, '/');
        if (lastSlash2 && (!lastSlash || lastSlash2 > lastSlash)) lastSlash = lastSlash2;
        if (lastSlash) {
            int dirLen = (int)(lastSlash - resolved) + 1; // include slash
            if (dirLen >= MAX_PATH) dirLen = MAX_PATH - 1;
            strncpy(g_levelDir, resolved, dirLen);
            g_levelDir[dirLen] = '\0';
        } else {
            g_levelDir[0] = '\0';
        }
        char dbg3[256]; wsprintfA(dbg3, "g_levelDir set to '%s'", g_levelDir);
        DebugLog(dbg3);
    }

    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x200)) return;
    void *gfx = *(void **)((char *)app + 0x174);
    if (!gfx) return;

    /* Step 1: MeshWorld */
    void *meshMem = g_operatorNew(0x10D0);
    if (!meshMem) return;
    void *meshWorld = g_LevelMeshWorldCtor(meshMem, gfx, meshPath);
    *(DWORD *)((char *)board + BOARD_MESHWORLD) = (DWORD)meshWorld;

    /* Step 1b: Extension heap allocation (Option B) — S1-driven. */
    {
        void* ext2 = EnsureBoardExt(board);
        (void)ext2;
        // S1 scan now that meshWorld exists — auto-enables feats for file-swapped levels
        if (ext && meshWorld) ScanS1AndAutoEnable(board, ext, meshWorld);
        // Targeted scan: only S1-referenced sub-meshes + main file (not wildcard whole folder)
        if (ext) ScanS1ReferencedMeshesForCollisions(board, ext, meshWorld, meshPath);
        // Build per-board cached collision flags (once, not per-collision)
        if (ext) BuildCollisionFlags(board, ext);
    }

    /* Step 2: RenderObj */
    void *renderMem = g_operatorNew(0x10D0);
    void *renderObj = NULL;
    if (renderMem) {
        renderObj = g_LevelRenderCtor(renderMem, meshWorld);
    }
    *(DWORD *)((char *)board + BOARD_RENDEROBJ) = (DWORD)renderObj;

    /* Step 3: InitScene */
    g_LevelInitScene(board);

    /* Step 4: Config-driven features (BEFORE Board_Setup)
     * The original game loads sub-meshes (bridge, tipper, etc.) in the
     * board ctor, BEFORE Board_Setup runs. Board_Setup calls
     * Scene_CreateDynamicObjects -> vtable[33] which reads these mesh
     * pointers. If we load them after Board_Setup, vtable[33] sees NULL
     * pointers and the dynamic objects get no mesh. */
    UniversalPostSetup(board);
    DebugLog("UniversalPostSetup done");

    /* Diagnostic: verify mesh pointers survived post-setup.
     * UNI_BONK_STORE (0x8620) and UNI_SAW2_OBJ (0x862C) are generic mesh
     * slots — they hold different objects per level (Tipper for Dizzy,
     * Bridge for Intermediate/Expert, Spinny for Toob, etc.) */
    {
        DWORD mesh0x8620 = *(DWORD *)((char *)ext + UNI_BONK_STORE);
        DWORD mesh0x862C = *(DWORD *)((char *)ext + UNI_SAW2_OBJ);
        char dbg[256];
        wsprintfA(dbg, "Pre-Board_Setup mesh check: [0x8620]=0x%08X [0x862C]=0x%08X", mesh0x8620, mesh0x862C);
        DebugLog(dbg);
    }

    /* Step 5: Board_Setup via vtable[0x80] */
    DWORD vtable = *(DWORD *)board;
    if (vtable && !IsBadReadPtr((void *)vtable, 0x84)) {
        void (__thiscall *boardSetup)(void *) = *(void (__thiscall **)(void *))((char *)vtable + 0x80);
        if (boardSetup) {
            DebugLog("Calling Board_Setup (vtable+0x80)...");
            boardSetup(board);
            DebugLog("Board_Setup done");
        }
    }

    /* Step 6: Per-level post-Board_Setup extras.
     * Dizzy's scene loader (0x40D390) scans MESHWORLD section 3 for
     * "TarBubble" objects and appends them to board+0x11E4. This feeds
     * the TarBubble particle effect system in DizzyBoard_Update.
     *
     * ORIGINAL GAME BUG: FUN_0044fa90 (called from DizzyBoard_Update) picks
     * a random TarBubble from the list. If the list is empty, it falls
     * through to iVar1=0 (NULL), then dereferences *(NULL+4) = crash at
     * address 0x4. This happens when a modded Dizzy level has no TarBubbles.
     *
     * FIX: If no TarBubbles are found, create a dummy object with a valid
     * name pointer and zero position so FUN_0044fa90 reads harmless data
     * instead of crashing. */
    if (raceIndex == 4) {
        int tarBubbleCount = 0;
        DWORD meshWorldPtr = *(DWORD *)((char *)board + BOARD_MESHWORLD);
        if (meshWorldPtr && !IsBadReadPtr((void *)meshWorldPtr, 0x500) &&
            g_AthenaListAppend && g_AthenaListGetIterator && g_AthenaListGetSize) {
            /* Access the MESHWORLD's section 3 object list:
             * meshWorld+0x480 = object database, +0x894 = iterator base,
             * +0x898 = count, +0xCA0 = array pointer */
            DWORD objDb = *(DWORD *)(meshWorldPtr + 0x480);
            if (objDb && !IsBadReadPtr((void *)objDb, 0x20)) {
                int iter = g_AthenaListGetIterator((void *)(objDb + 0x894));
                *(DWORD *)(objDb + 0x89C + iter * 4) = 0;
                int count = *(int *)(objDb + 0x898);
                if (count > 0) {
                    DWORD *array = *(DWORD **)(objDb + 0xCA0);
                    if (array && !IsBadReadPtr((void *)array, count * 4)) {
                        *(DWORD *)(objDb + 0x89C + iter * 4) = 1;
                        int idx = 0;
                        while (idx < count) {
                            DWORD *obj = (DWORD *)array[idx];
                            if (obj && !IsBadReadPtr((void *)obj, 4)) {
                                char *name = *(char **)obj;
                                if (name && !IsBadReadPtr(name, 9)) {
                                    if (my_strnicmp(name, "TarBubble", 9) == 0) {
                                        g_AthenaListAppend(
                                            (void *)((char *)ext + UNI_TARBUBBLE_LIST),
                                            (int)obj);
                                        tarBubbleCount++;
                                    }
                                }
                            }
                            idx++;
                            *(DWORD *)(objDb + 0x89C + iter * 4) = idx;
                        }
                    }
                }
            }
        }
        /* If no TarBubbles were found, create a dummy entry to prevent
         * FUN_0044fa90 from crashing on an empty list. The dummy needs:
         * +0x00 = char* name (points to a valid string)
         * +0x04 = float X position
         * +0x08 = float Y position
         * +0x0C = float Z position
         * FUN_0044fa90 reads these 4 fields when picking a random entry. */
        if (tarBubbleCount == 0 && g_AthenaListAppend && g_operatorNew) {
            /* Allocate a 0x10-byte dummy object */
            DWORD *dummy = (DWORD *)g_operatorNew(0x10);
            if (dummy) {
                memset(dummy, 0, 0x10);
                /* Set name pointer to a static string */
                static char dummyName[] = "TarBubble";
                dummy[0] = (DWORD)dummyName;
                /* Positions at +4, +8, +0xC are already 0 (memset above) */
                g_AthenaListAppend(
                    (void *)((char *)ext + UNI_TARBUBBLE_LIST),
                    (int)dummy);
                DebugLog("TarBubble scan: no TarBubbles found, added dummy entry");
            }
        }
    }

    /* Step 6b: Tower pennants + PENNANT* collection (see lf_09_pennants.c). */
    Pennant_PostSetup(board, ext, raceIndex);
}

/* Must be non-static for asm reference */
void __cdecl UniversalConstructorLogic(void *board, int raceIndex) {
    UniversalConstructor(board, raceIndex);
}

__attribute__((naked)) static void Hook_UniversalConstructor(void) {
    __asm__ __volatile__(
        "pushl %%ebp\n\t"
        "movl  %%esp, %%ebp\n\t"
        "pushl %%edx\n\t"
        "pushl %%ecx\n\t"

        "movl  0x08(%%esi), %%eax\n\t"
        "pushl %%eax\n\t"
        "pushl %%ecx\n\t"

        "call  _UniversalConstructorLogic\n\t"
        "addl  $8, %%esp\n\t"

        "popl  %%ecx\n\t"
        "popl  %%edx\n\t"
        "popl  %%ebp\n\t"

        "jmpl  *_g_ctorTrampoline\n\t"
        :: : "eax", "memory"
    );
}

/* Must be non-static for asm reference */
unsigned char *g_ctorTrampoline = NULL;

static void InstallUniversalConstructorHook(void) {
    DWORD targetAddr = g_moduleBase + 0x000273E0;
    unsigned char *orig = (unsigned char *)targetAddr;

    if (orig[0] != 0xFF || orig[1] != 0x52 || orig[2] != 0x48 ||
        orig[3] != 0x8B || orig[4] != 0x4E || orig[5] != 0x04) return;

    g_ctorTrampoline = VirtualAlloc(NULL, 16,
                        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_ctorTrampoline) return;

    g_ctorTrampoline[0] = 0x8B;  /* MOV ECX, [ESI+4] */
    g_ctorTrampoline[1] = 0x4E;
    g_ctorTrampoline[2] = 0x04;
    g_ctorTrampoline[3] = 0xE9;  /* JMP back to original+6 */
    *(DWORD *)(g_ctorTrampoline + 4) = (targetAddr + 6) - ((DWORD)g_ctorTrampoline + 8);

    DWORD oldProtect;
    VirtualProtect(orig, 6, PAGE_EXECUTE_READWRITE, &oldProtect);
    orig[0] = 0xE9;
    *(DWORD *)(orig + 1) = (DWORD)&Hook_UniversalConstructor - (targetAddr + 5);
    orig[5] = 0x90;
    VirtualProtect(orig, 6, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), orig, 6);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Allocation size patch
 * ═══════════════════════════════════════════════════════════════════════════ */

static const DWORD g_allocPatchRVAs[15] __attribute__((unused)) = {
    0x00027109, 0x00027136, 0x00027167, 0x00027198, 0x000271C9,
    0x000271FA, 0x0002722B, 0x0002725C, 0x0002728D, 0x000272BE,
    0x000272EF, 0x00027320, 0x00027351, 0x0002737B, 0x000273A5,
};

static void PatchAllocSizes(void) {
    /* Option B: board stays vanilla-sized; union lives in extension heap at board+EXT_PTR.
     * We no longer patch the 15 PUSH sites to UNION_SIZE (0xAB00). Leave them vanilla. */
    DebugLog("PatchAllocSizes: skipped (vanilla board size, ext heap used)");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Install collision hook
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Hook Tournament_AdvanceRace (0x00427080) to free ext on level unload */
static unsigned char* g_advanceTrampoline = NULL;
static void (__stdcall *g_origAdvanceRace)(DWORD);
static void __stdcall Hook_AdvanceRace(DWORD a1) {
    /* Free the outgoing board's ext *before* the call (App+0x178 is the cur board)
     * and sweep stales *after* the call. Covers both reuse and free cases.
     * App pointer is at absolute 0x005341E0 (RVA 0x1341E0 from g_moduleBase). */
    void* curBoard = NULL;
    DWORD appPtrAddr = g_moduleBase ? g_moduleBase + 0x1341E0 : 0x005341E0;
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery((void*)appPtrAddr, &mbi, sizeof(mbi)) && mbi.State==MEM_COMMIT) {
        DWORD app = *(DWORD*)appPtrAddr;
        if (app && VirtualQuery((void*)app, &mbi, sizeof(mbi)) && mbi.State==MEM_COMMIT) {
            /* BOARD_APP_PTR validated via BoardHasOffset-style check */
            if (BoardHasOffset((void*)app, 0x178, 4) || !HeapValidate(GetProcessHeap(),0,(void*)app)) {
                /* Fallback: raw read with SEH-style guard via VirtualQuery already */
                curBoard = *(void**)((char*)app + 0x178);
            } else {
                curBoard = *(void**)((char*)app + 0x178);
            }
        }
    }
    if (curBoard) {
        FreeBoardExt(curBoard);
    }
    if (g_origAdvanceRace) g_origAdvanceRace(a1);
    int j;
    for (j=0;j<MAX_EXT_MAP;j++) if (g_extMap[j].ext) {
        void* b = g_extMap[j].board;
        if (!b || !HeapValidate(GetProcessHeap(),0,b)) {
            /* Board was freed without AdvanceRace (e.g. immediate restart).
             * Use HeapValidate — IsBadReadPtr would pass on freed page. */
            HeapFree(GetProcessHeap(),0,g_extMap[j].ext);
            g_extMap[j].ext=NULL; g_extMap[j].extSize=0; g_extMap[j].feat=0; g_extMap[j].board=NULL;
        } else {
            /* Also sweep if board no longer points to a live Heap block that
             * contains our offset — VirtualQuery check */
            MEMORY_BASIC_INFORMATION mb2;
            if (!VirtualQuery(b, &mb2, sizeof(mb2)) || mb2.State!=MEM_COMMIT) {
                HeapFree(GetProcessHeap(),0,g_extMap[j].ext);
                g_extMap[j].ext=NULL; g_extMap[j].extSize=0; g_extMap[j].feat=0; g_extMap[j].board=NULL;
            }
        }
    }
}
static void InstallExtFreeHook(void) {
    DWORD targetAddr = g_moduleBase + 0x00027080;
    unsigned char* orig = (unsigned char*)targetAddr;
    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQuery(orig, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    if (!(mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))) return;
    // RVA 0x27080 prologue varies by build (dump shows 3C AE FD FF at 0x26480 file-off, not 55 8B EC)
    // Don't gate on exact bytes — log and continue
    if (orig[0]!=0x55 || orig[1]!=0x8B || orig[2]!=0xEC) {
        char dbg[64]; wsprintfA(dbg, "InstallExtFreeHook: prologue %02X %02X %02X (continuing)", orig[0], orig[1], orig[2]);
        DebugLog(dbg);
    }
    g_advanceTrampoline = VirtualAlloc(NULL, 32, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_advanceTrampoline) return;
    memcpy(g_advanceTrampoline, orig, 6);
    g_advanceTrampoline[6]=0xE9;
    *(DWORD*)(g_advanceTrampoline+7) = (targetAddr+6) - ((DWORD)g_advanceTrampoline+11);
    g_origAdvanceRace = (void (__stdcall *)(DWORD))g_advanceTrampoline;
    DWORD oldProtect;
    VirtualProtect(orig, 6, PAGE_EXECUTE_READWRITE, &oldProtect);
    orig[0]=0xE9;
    *(DWORD*)(orig+1) = (DWORD)Hook_AdvanceRace - (targetAddr+5);
    orig[5]=0x90;
    VirtualProtect(orig,6, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), orig, 6);
    DebugLog("InstallExtFreeHook: hooked Tournament_AdvanceRace");
}

static void InstallHook(void) {
    /* DEPRECATED: SEH trampoline on DispatchCollisionEvents (0x40C5D0) caused
     * stack/exception chain corruption (skill: Manual trampoline detour hooks
     * on SEH functions crash). UniversalDispatchCollision via vtable[29]
     * now solely handles collisions and calls the original directly.
     * Keep trampoline disabled — just set g_OriginalDispatch to the raw
     * function address. */
    g_OriginalDispatch = (DispatchCollisionEvents_t)(g_moduleBase + RVA_DispatchCollisionEvents);
    DebugLog("InstallHook: SEH trampoline disabled, vtable[29] handles collisions");
    (void)g_trampoline;
}

/* BASS proxy exports handled by bass.def - DLL forwarding to bass_real.dll */

/* ═══════════════════════════════════════════════════════════════════════════
 * File fallback hook — if game can't find texture/sound/mesh in its
 * default folder, retry in g_levelDir (the folder containing the main
 * MESHWORLD for this race). Covers:
 *   textures\\foo.png      -> g_levelDir\\foo.png
 *   levels\\Level3-Water  -> g_levelDir\\Level3-Water  (+ .MESHWORLD/.MESH)
 *   sounds\\foo.ogg       -> g_levelDir\\foo.ogg
 * Implemented as IAT patch on kernel32!CreateFileA/W so every file open
 * (D3DX, BASS, MESHWORLD) is covered without per-callsite hooks.
 * ═══════════════════════════════════════════════════════════════════════════ */

static HANDLE (WINAPI *g_origCreateFileA)(LPCSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE) = NULL;
static HANDLE (WINAPI *g_origCreateFileW)(LPCWSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE) = NULL;
static volatile LONG g_inFileHookFallback = 0;

static HANDLE WINAPI Hook_CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    if (InterlockedExchange(&g_inFileHookFallback, 1) != 0) {
        return g_origCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }
    HANDLE h = g_origCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    if (h != INVALID_HANDLE_VALUE) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    DWORD err = GetLastError();
    if (err != ERROR_FILE_NOT_FOUND && err != ERROR_PATH_NOT_FOUND) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    if (!lpFileName || !g_levelDir[0]) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    const char *ext = strrchr(lpFileName, '.');
    const char *slash = strrchr(lpFileName, '\\');
    const char *slash2 = strrchr(lpFileName, '/');
    if (slash2 && (!slash || slash2 > slash)) slash = slash2;
    const char *base = slash ? slash + 1 : lpFileName;
    if (!base || !*base) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    char trial[MAX_PATH];
    int dirLen = strlen(g_levelDir);
    int baseLen = strlen(base);
    if (dirLen + baseLen >= MAX_PATH) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    strcpy(trial, g_levelDir);
    strcat(trial, base);
    if (my_stricmp(trial, lpFileName) == 0) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    if (my_strnicmp(base, "lfdebug", 7)==0 || my_strnicmp(base, "LevelFeatures", 13)==0 || my_strnicmp(base, "RaceFiles", 9)==0) {
        InterlockedExchange(&g_inFileHookFallback, 0);
        return h;
    }
    HANDLE h2 = g_origCreateFileA(trial, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    if (h2 != INVALID_HANDLE_VALUE) {
        char dbg[512]; wsprintfA(dbg, "Fallback: '%s' -> '%s' (OK)", lpFileName, trial);
        DebugLog(dbg);
        InterlockedExchange(&g_inFileHookFallback, 0);
        return h2;
    }
    if (!ext) {
        if (dirLen + baseLen + 10 >= MAX_PATH) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
        strcpy(trial, g_levelDir);
        strcat(trial, base);
        strcat(trial, ".MESHWORLD");
        HANDLE h3 = g_origCreateFileA(trial, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
        if (h3 != INVALID_HANDLE_VALUE) {
            char dbg2[512]; wsprintfA(dbg2, "Fallback (+.MESHWORLD): '%s' -> '%s' (OK)", lpFileName, trial);
            DebugLog(dbg2);
            InterlockedExchange(&g_inFileHookFallback, 0);
            return h3;
        }
    }
    InterlockedExchange(&g_inFileHookFallback, 0);
    return h;
}

static HANDLE WINAPI Hook_CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    if (InterlockedExchange(&g_inFileHookFallback, 1) != 0) return g_origCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    HANDLE h = g_origCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    if (h != INVALID_HANDLE_VALUE) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    DWORD err = GetLastError();
    if (err != ERROR_FILE_NOT_FOUND && err != ERROR_PATH_NOT_FOUND) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    if (!lpFileName || !g_levelDir[0]) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    char ansi[MAX_PATH]; WideCharToMultiByte(CP_ACP, 0, lpFileName, -1, ansi, MAX_PATH, NULL, NULL);
    const char *slash = strrchr(ansi, '\\');
    const char *slash2 = strrchr(ansi, '/');
    if (slash2 && (!slash || slash2 > slash)) slash = slash2;
    const char *base = slash ? slash + 1 : ansi;
    if (!base || !*base) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    char trialAnsi[MAX_PATH];
    int dirLen = strlen(g_levelDir);
    int baseLen = strlen(base);
    if (dirLen + baseLen >= MAX_PATH) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    strcpy(trialAnsi, g_levelDir);
    strcat(trialAnsi, base);
    if (my_stricmp(trialAnsi, ansi)==0) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    if (my_strnicmp(base, "lfdebug", 7)==0) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    WCHAR trialW[MAX_PATH]; MultiByteToWideChar(CP_ACP, 0, trialAnsi, -1, trialW, MAX_PATH);
    HANDLE h2 = g_origCreateFileW(trialW, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    if (h2 != INVALID_HANDLE_VALUE) {
        char dbg[512]; wsprintfA(dbg, "FallbackW: '%s' -> '%s' (OK)", ansi, trialAnsi);
        DebugLog(dbg);
        InterlockedExchange(&g_inFileHookFallback, 0);
        return h2;
    }
    InterlockedExchange(&g_inFileHookFallback, 0);
    return h;
}

static void InstallFileFallbackHook(void) {
    if (g_origCreateFileA) return; // already installed
    HMODULE exe = GetModuleHandleA(NULL);
    if (!exe) exe = (HMODULE)g_moduleBase;
    if (!exe || IsBadReadPtr(exe, 0x100)) return;
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)exe;
    if (dos->e_magic != 0x5A4D) return;
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((char*)exe + dos->e_lfanew);
    if (IsBadReadPtr(nt, sizeof(IMAGE_NT_HEADERS))) return;
    if (nt->Signature != 0x00004550) return;
    DWORD importRVA = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    if (!importRVA) return;
    PIMAGE_IMPORT_DESCRIPTOR imp = (PIMAGE_IMPORT_DESCRIPTOR)((char*)exe + importRVA);
    for (; imp->Name; imp++) {
        char *dllName = (char*)exe + imp->Name;
        if (!dllName || IsBadReadPtr(dllName, 8)) continue;
        if (my_stricmp(dllName, "KERNEL32.dll") != 0 && my_stricmp(dllName, "kernel32.dll") != 0) continue;
        PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)((char*)exe + imp->FirstThunk);
        PIMAGE_THUNK_DATA origThunk = (PIMAGE_THUNK_DATA)((char*)exe + imp->OriginalFirstThunk);
        int useOrig = (imp->OriginalFirstThunk != 0);
        for (; thunk->u1.Function; thunk++, origThunk++) {
            PIMAGE_THUNK_DATA nameThunk = useOrig ? origThunk : thunk;
            if (IMAGE_SNAP_BY_ORDINAL(nameThunk->u1.Ordinal)) continue;
            PIMAGE_IMPORT_BY_NAME byName = (PIMAGE_IMPORT_BY_NAME)((char*)exe + nameThunk->u1.AddressOfData);
            if (IsBadReadPtr(byName, 4)) continue;
            char *funcName = (char*)byName->Name;
            if (!funcName || IsBadReadPtr(funcName, 4)) continue;
            DWORD oldProtect;
            if (strcmp(funcName, "CreateFileA")==0) {
                g_origCreateFileA = (void*)thunk->u1.Function;
                VirtualProtect(&thunk->u1.Function, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
                thunk->u1.Function = (DWORD)Hook_CreateFileA;
                VirtualProtect(&thunk->u1.Function, 4, oldProtect, &oldProtect);
                FlushInstructionCache(GetCurrentProcess(), &thunk->u1.Function, 4);
                DebugLog("InstallFileFallbackHook: CreateFileA hooked");
            } else if (strcmp(funcName, "CreateFileW")==0) {
                g_origCreateFileW = (void*)thunk->u1.Function;
                VirtualProtect(&thunk->u1.Function, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
                thunk->u1.Function = (DWORD)Hook_CreateFileW;
                VirtualProtect(&thunk->u1.Function, 4, oldProtect, &oldProtect);
                FlushInstructionCache(GetCurrentProcess(), &thunk->u1.Function, 4);
                DebugLog("InstallFileFallbackHook: CreateFileW hooked");
            }
        }
    }
    if (!g_origCreateFileA) DebugLog("InstallFileFallbackHook: CreateFileA not found in IAT");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Debug logging
 * ═══════════════════════════════════════════════════════════════════════════ */

void DebugLog(const char *msg) {
    /* Write next to the DLL, or fall back to game directory */
    char logPath[MAX_PATH];
    if (g_configPath[0]) {
        strcpy(logPath, g_configPath);
        char *p = strrchr(logPath, '\\');
        if (p) {
            strcpy(p + 1, "lfdebug.log");
        } else {
            strcpy(logPath, "lfdebug.log");
        }
    } else {
        /* GetConfigPath failed (common under Wine/BoxedWine) — use game dir */
        strcpy(logPath, "lfdebug.log");
    }
    HANDLE hFile = CreateFileA(logPath, GENERIC_WRITE,
                               FILE_SHARE_READ, NULL, OPEN_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    SetFilePointer(hFile, 0, NULL, FILE_END);
    DWORD written;
    WriteFile(hFile, msg, strlen(msg), &written, NULL);
    WriteFile(hFile, "\r\n", 2, &written, NULL);
    CloseHandle(hFile);
}

/* Forward declaration for auto-test (unused, kept for future testing) */

/* ═══════════════════════════════════════════════════════════════════════════
 * Vtable patching — replace slots 1, 19, 29, 33 in all 15 level vtables
 * with universal handlers. Saves original pointers for delegation.
 * ═══════════════════════════════════════════════════════════════════════════ */

static void InstallVtablePatches(void) {
    int i;
    for (i = 1; i <= 15; i++) {
        DWORD vtableAddr = g_levelVtables[i];
        if (!vtableAddr) continue;
        { MEMORY_BASIC_INFORMATION mbi; if (!VirtualQuery((void*)vtableAddr, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT || (DWORD)mbi.BaseAddress + mbi.RegionSize < vtableAddr + 0x88) continue; }

        DWORD oldProtect;

        /* Slot 1 (offset +0x04): Board_Update → UniversalBoardUpdate */
        {
            DWORD *slot = (DWORD *)(vtableAddr + 0x04);
            VirtualProtect(slot, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
            *slot = (DWORD)&UniversalBoardUpdate;
            VirtualProtect(slot, 4, oldProtect, &oldProtect);
            FlushInstructionCache(GetCurrentProcess(), slot, 4);
        }

        /* Slot 19 (offset +0x4C): RaceState → UniversalRaceState
         * Save original per-level handler — levels 2,6,7 have custom RaceState
         * that iterates per-level data (lifter list, render objs, timers). */
        {
            DWORD *slot = (DWORD *)(vtableAddr + 0x4C);
            /* Save original before overwriting (index by level 1-15) */
            if (i >= 1 && i <= 15) {
                g_origRaceState[i] = (RaceState_t)*slot;
            }
            VirtualProtect(slot, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
            *slot = (DWORD)&UniversalRaceState;
            VirtualProtect(slot, 4, oldProtect, &oldProtect);
            FlushInstructionCache(GetCurrentProcess(), slot, 4);
        }

        /* Slot 29 (offset +0x74): DispatchCollision → UniversalDispatchCollision
         * No need to save original — UniversalDispatchCollision calls g_OriginalDispatch directly */
        {
            DWORD *slot = (DWORD *)(vtableAddr + 0x74);
            VirtualProtect(slot, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
            *slot = (DWORD)&UniversalDispatchCollision;
            VirtualProtect(slot, 4, oldProtect, &oldProtect);
            FlushInstructionCache(GetCurrentProcess(), slot, 4);
        }

        /* Slot 33 (offset +0x84): CreateDynamicObjects → UniversalCreateDynamicObjects
         * No delegation — universal handler recognizes all object names */
        {
            DWORD *slot = (DWORD *)(vtableAddr + 0x84);
            VirtualProtect(slot, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
            *slot = (DWORD)&UniversalCreateDynamicObjects;
            VirtualProtect(slot, 4, oldProtect, &oldProtect);
            FlushInstructionCache(GetCurrentProcess(), slot, 4);
        }

        /* Slot 24 (offset +0x60): Per-level render — per-level RET convention.
         * Two call sites: 0x0046C8C7 (1-param RET 0) vs 0x0046C9F0 (2-param RET 4).
         * Only the 6 levels that use 2-param render (2=Beginner,5=Tower,10=Toob,12=Glass,13=Sky,14=Master) are patched. */
        {
            int is_two_param = (i==2 || i==5 || i==10 || i==12 || i==13 || i==14);
            if (is_two_param) {
                DWORD *slot = (DWORD *)(vtableAddr + 0x60);
                VirtualProtect(slot, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
                *slot = (DWORD)&UniversalRender;
                VirtualProtect(slot, 4, oldProtect, &oldProtect);
                FlushInstructionCache(GetCurrentProcess(), slot, 4);
            }
        }
    }
    DebugLog("Vtable slots [1,19,29,33] patched (slot 24 per-level: 2,5,10,12,13,14)");
}

static DWORD WINAPI PatchThread(LPVOID param) {
    DebugLog("=== PatchThread started ===");
    Sleep(1000); // 1s: early enough to beat first level load (~3-4s), late enough for BASS init
    DebugLog("Sleep done, resolving module base");
    g_moduleBase = (DWORD)GetModuleHandleA("Hamsterball.exe");
    if (!g_moduleBase) g_moduleBase = 0x00400000;

    g_operatorNew = (operator_new_t)(g_moduleBase + RVA_operator_new);
    g_LevelMeshWorldCtor = (Level_MeshWorldCtor_t)(g_moduleBase + RVA_Level_MeshWorldCtor);
    g_LevelRenderCtor = (Level_RenderCtor_t)(g_moduleBase + RVA_Level_RenderCtor);
    g_LevelInitScene = (Level_InitScene_t)(g_moduleBase + RVA_Level_InitScene);
    g_SoundPlay3D = (Sound_Play3D_t)(g_moduleBase + RVA_Sound_Play3D);
    g_CollectByNameFilter = (Scene_CollectByNameFilter_t)(g_moduleBase + RVA_Scene_CollectByNameFilter);
    g_AthenaListInit = (AthenaList_Init_t)(g_moduleBase + RVA_AthenaList_Init);
    g_BoardCtor = (Board_ctor_t)(g_moduleBase + RVA_Board_ctor);
    g_LoadRaceData = (LoadRaceData_t)(g_moduleBase + RVA_LoadRaceData);
    g_Vec3Init = (Vec3_Init_t)(g_moduleBase + RVA_Vec3_Init);
    g_MatrixIdentity = (Matrix_Identity_t)(g_moduleBase + RVA_Matrix_Identity);
    g_MeshNodeCtor = (MeshNode_ctor_t)(g_moduleBase + RVA_MeshNode_ctor);
    g_SpriteCtor = (Sprite_ctor_t)(g_moduleBase + RVA_Sprite_ctor);
    g_TipperVisualAttach = (TipperVisual_Attach_t)(g_moduleBase + RVA_TipperVisual_Attach);
    g_LevelAssignTex = (Level_AssignTex_t)(g_moduleBase + RVA_Level_AssignTexAndScales);
    g_SoundGetNextChannel = (Sound_GetNextChannel_t)(g_moduleBase + RVA_Sound_GetNextChannel);
    g_SceneRenderIfVisible = (Scene_RenderIfVisible_t)(g_moduleBase + RVA_Scene_RenderIfVisible);
    g_AthenaListAppend = (AthenaList_Append_t)(g_moduleBase + RVA_AthenaList_Append);
    g_AthenaListGetSize = (AthenaList_GetSize_t)(g_moduleBase + RVA_AthenaList_GetSize);
    g_AthenaListGetIterator = (AthenaList_GetIterator_t)(g_moduleBase + RVA_AthenaList_GetIterator);
    g_ehVectorCtor = (eh_vector_ctor_t)(g_moduleBase + RVA_eh_vector_ctor);
    g_ehVectorCtorFn = (void *)(g_moduleBase + RVA_FUN_0040a870);
    g_Vec3ListFree = (void *)(g_moduleBase + RVA_Vec3List_Free);
    g_LevelAssignTexScales = (Level_AssignTexScales_t)(g_moduleBase + RVA_Level_AssignTexScales);

    /* Resolve CreateDynamicObjects ctors */
    g_TipperCtor = (Ctor3_t)(g_moduleBase + RVA_Tipper_ctor);
    g_TipperVisualCtor = (TipperVisual_ctor_t)(g_moduleBase + RVA_TipperVisual_ctor);
    g_GluebieCtor = (Ctor3_t)(g_moduleBase + RVA_Gluebie_ctor);
    g_CatapultCtor = (Ctor3_t)(g_moduleBase + RVA_Catapult_ctor);
    g_MaceCtor = (Ctor3_t)(g_moduleBase + RVA_Mace_ctor);
    g_GlassLevelCtor = (Ctor3_t)(g_moduleBase + RVA_Glass_Level_ctor);
    g_TrapdoorCtor = (Ctor2_t)(g_moduleBase + RVA_Trapdoor_ctor);
    g_StandsCtor = (Stands_ctor_t)(g_moduleBase + RVA_Stands_ctor);
    g_BonkCtor = (Ctor4f_t)(g_moduleBase + RVA_Bonk_ctor);
    g_FanCtor = (Ctor5f_t)(g_moduleBase + RVA_Fan_ctor);
    g_SawBladeCtor = (Ctor4f_t)(g_moduleBase + RVA_SawBlade_ctor);
    g_SawBladeSetVariant = (SawBlade_SetVariant_t)(g_moduleBase + RVA_SawBlade_SetVariant);
    g_SpinnerLevelCtor = (Ctor5f_t)(g_moduleBase + RVA_Spinner_Level_ctor);
    g_GearLevelCtor = (Ctor4f_t)(g_moduleBase + RVA_Gear_Level_ctor);
    g_BellCtor = (Ctor4f_t)(g_moduleBase + RVA_Bell_ctor);
    g_OddLifterCtor = (Ctor4f_t)(g_moduleBase + RVA_Odd_Lifter_ctor);
    g_LifterCtor = (Ctor_Lifter_t)(g_moduleBase + RVA_Lifter_ctor);
    g_SpeedCylinderCtor = (Ctor_SpeedCyl_t)(g_moduleBase + RVA_SpeedCylinder_ctor);
    g_TimeButtonCtor = (Ctor3f_t)(g_moduleBase + RVA_TimeButton_ctor);
    g_RotatorImpossibleCtor = (Ctor3f_t)(g_moduleBase + RVA_Rotator_ctor_Impossible);
    g_SawCtor = (Ctor6f_t)(g_moduleBase + RVA_Saw_ctor);
    g_Saw2Ctor = (Ctor6f_t)(g_moduleBase + RVA_Saw2_ctor);
    g_FalloutCtor = (Ctor3f_t)(g_moduleBase + RVA_Fallout_ctor);
    g_BlockdawgCtor = (Ctor6f_t)(g_moduleBase + RVA_Blockdawg_ctor);
    g_GameLevelCtor = (Ctor3f_t)(g_moduleBase + RVA_GameLevel_ctor);
    g_WavyCtor = (Ctor_Str_t)(g_moduleBase + RVA_Wavy_ctor);
    g_WavyConfigure = (Wavy_Configure_t)(g_moduleBase + RVA_Wavy_Configure);
    g_NeonPlatformCtor = (Ctor3f_t)(g_moduleBase + RVA_NeonPlatform_ctor);
    g_ArenaStandsCtor = (Ctor3f_t)(g_moduleBase + RVA_ArenaStands_ctor);
    g_PopcylinderCtor = (Ctor3f_t)(g_moduleBase + RVA_Popcylinder_ctor);
    g_PopCylinderCtor = (Ctor3f_t)(g_moduleBase + RVA_PopCylinder_ctor);
    g_RotatorCtor = (Ctor_Rotator_t)(g_moduleBase + RVA_Rotator_ctor);
    g_LooperCtor = (Ctor3f_t)(g_moduleBase + RVA_Looper_ctor);
    g_GearCtor = (Ctor7f_t)(g_moduleBase + RVA_Gear_ctor);
    g_PendulumCtor = (Ctor3f_t)(g_moduleBase + RVA_Pendulum_ctor);
    g_BreakBridgeCtor = (Ctor3f_t)(g_moduleBase + RVA_BreakBridge_ctor);
    g_LevelFindObjectByName = (Level_FindObjectByName_t)(g_moduleBase + RVA_Level_FindObjectByName);
    g_SoundInitChannels = (Sound_InitChannels_t)(g_moduleBase + RVA_Sound_InitChannels);

    /* Resolve Board_Update function pointers */
    g_SceneUpdate = (Scene_Update_t)(g_moduleBase + RVA_Scene_Update);
    g_BoardUpdateRaceState = (Board_UpdateRaceState_t)(g_moduleBase + RVA_Board_UpdateRaceState);
    g_RenderDynamicObjects = (Level_RenderDynamicObjects_t)(g_moduleBase + RVA_Level_RenderDynamicObjects);
    g_GraphicsSetProjection = (Graphics_SetProjection_t)(g_moduleBase + RVA_Graphics_SetProjection);
    g_GraphicsSetCullMode2 = (Graphics_SetCullMode2_t)(g_moduleBase + RVA_Graphics_SetCullMode2);
    g_SpriteRenderQuad = (Sprite_RenderQuad_t)(g_moduleBase + RVA_Sprite_RenderQuad);
    g_RenderContextInit = (RenderContext_Init_t)(g_moduleBase + RVA_RenderContext_Init);
    g_Matrix4Identity = (Matrix4_Identity_t)(g_moduleBase + RVA_Matrix4_Identity);
    g_GfxScaleZ = (Gfx_ScaleFn_t)(g_moduleBase + RVA_Gfx_ScaleZ);
    g_GfxScaleY = (Gfx_ScaleFn_t)(g_moduleBase + RVA_Gfx_ScaleY);
    g_GfxScaleX = (Gfx_ScaleFn_t)(g_moduleBase + RVA_Gfx_ScaleX);
    g_GfxSetPosition = (Gfx_SetPosition_t)(g_moduleBase + RVA_Gfx_SetPosition);
    g_TimerInit = (Timer_Init_t)(g_moduleBase + RVA_Timer_Init);
    g_TimerCleanup = (Timer_Cleanup_t)(g_moduleBase + RVA_Timer_Cleanup);
    g_MatrixTransformVec3 = (Matrix_TransformVec3_t)(g_moduleBase + RVA_Matrix_TransformVec3);
    g_Matrix44Zero = (Matrix44_Zero_t)(g_moduleBase + RVA_Matrix44_Zero);
    g_SceneForEachBallSetVelocity = (Scene_ForEachBall_SetVelocity_t)(g_moduleBase + RVA_Scene_ForEachBall_SetVel);
    /* g_AthenaListGetIterator, g_AthenaListGetSize, g_AthenaListAppend already
     * resolved above — removed redundant re-resolution. */
    g_CreateTarBubble = (FUN_0044fa90_t)(g_moduleBase + RVA_FUN_0044fa90);
    g_CreateSplashParticle = (FUN_0044fb50_t)(g_moduleBase + RVA_FUN_0044fb50);
    g_RemoveBall = (FUN_00405190_t)(g_moduleBase + RVA_FUN_00405190);
    g_RNG = (CPUID_RNG_t)(g_moduleBase + RVA_CPUID_RNG);
    g_RNG_raw = (DWORD)(g_moduleBase + RVA_CPUID_RNG);
    g_BadBallCtor = (BadBall_ctor_t)(g_moduleBase + RVA_BadBall_ctor);
    g_BallSetTrajectory = (Ball_SetTrajectory_t)(g_moduleBase + RVA_Ball_SetTrajectory);
    g_BallSetVec3AtOffset = (Ball_SetVec3AtOffset_t)(g_moduleBase + RVA_Ball_SetVec3AtOffset);
    g_Vec3NormalizeAndScale = (Vec3_NormalizeAndScale_t)(g_moduleBase + RVA_Vec3_NormalizeAndScale);
    g_Vec3CopyUpd = (Vec3_Copy_t)(g_moduleBase + RVA_Vec3_Copy_Upd);
    g_SoundCalcDistAtten = (Sound_CalcDistAtten_t)(g_moduleBase + RVA_Sound_CalcDistAtten);
    g_SoundPlay3DAtPos = (Sound_Play3DAtPos_t)(g_moduleBase + RVA_Sound_Play3DAtPos);
    g_SceneSetRaceActive = (Scene_SetRaceActive_t)(g_moduleBase + RVA_Scene_SetRaceActive);
    g_SceneAddObject = (Scene_AddObject_t)(g_moduleBase + RVA_Scene_AddObject);

    /* Option B: Feature flags now per-board via ScanS1AndAutoEnable/OrBoardFeat.
     * g_updateFeatures/g_collisionEvents no longer used — S1-driven. */
    // memcpy(g_updateFeatures, ...) deprecated

    /* Resolve collision handler function pointers */
    g_SoundPlayChannel = (Sound_PlayChannel_t)(g_moduleBase + RVA_Sound_PlayChannel);
    g_BallDizzyImmunity = (Ball_DizzyImmunity_t)(g_moduleBase + RVA_Ball_DizzyImmunity);
    g_BallGrow = (Ball_Grow_t)(g_moduleBase + RVA_Ball_Grow);
    g_BallShrink = (Ball_Shrink_t)(g_moduleBase + RVA_Ball_Shrink);
    g_BallSetName = (Ball_SetName_t)(g_moduleBase + RVA_Ball_SetName);
    g_BallApplyTrajectory = (Ball_ApplyTrajectory_t)(g_moduleBase + RVA_Ball_ApplyTrajectory);
    g_BallSetTiltedGravity = (Ball_SetTiltedGravity_t)(g_moduleBase + RVA_Ball_SetTiltedGravity);
    g_BallSetFlatGravity = (Ball_SetFlatGravity_t)(g_moduleBase + RVA_Ball_SetFlatGravity);
    g_RotatorMarkTriggered = (Rotator_MarkTriggered_t)(g_moduleBase + RVA_Rotator_MarkTriggered);
    g_RotatorPlayCollisionSound = (Rotator_PlayCollisionSound_t)(g_moduleBase + RVA_Rotator_PlayCollisionSound);
    g_RotatorTriggerSound = (Rotator_TriggerSound_t)(g_moduleBase + RVA_Rotator_TriggerSound);
    g_RotatorStartSound = (Rotator_StartSound_t)(g_moduleBase + RVA_Rotator_StartSound);
    g_RotatorAddBall = (Rotator_AddBall_t)(g_moduleBase + RVA_Rotator_AddBall);
    g_CheckArenaUnlock = (CheckArenaUnlock_t)(g_moduleBase + RVA_CheckArenaUnlock);
    g_CatapultLaunch = (Catapult_Launch_t)(g_moduleBase + RVA_Catapult_Launch);
    g_TrapdoorOpen = (Trapdoor_Open_t)(g_moduleBase + RVA_Trapdoor_Open);
    g_TrapdoorActivate = (Trapdoor_Activate_t)(g_moduleBase + RVA_Trapdoor_Activate);
    g_SawAlertActivate = (Saw_AlertActivate_t)(g_moduleBase + RVA_Saw_AlertActivate);
    g_SawActivate = (Saw_Activate_t)(g_moduleBase + RVA_Saw_Activate);
    g_BellActivate = (Bell_Activate_t)(g_moduleBase + RVA_Bell_Activate);
    g_JudgeReset = (Judge_Reset_t)(g_moduleBase + RVA_Judge_Reset);
    g_ScoreDisplaySetTime = (ScoreDisplay_SetTime_t)(g_moduleBase + RVA_ScoreDisplay_SetTime);
    g_CreateBonkPopup = (CreateBonkPopup_t)(g_moduleBase + RVA_CreateBonkPopup);
    g_HammerChaseStart = (Hammer_ChaseStart_t)(g_moduleBase + RVA_Hammer_ChaseStart);
    g_PendulumPlayCollisionSound = (Pendulum_PlayCollisionSound_t)(g_moduleBase + RVA_Pendulum_PlayCollisionSound);
    g_PendulumAddIndex = (Pendulum_AddIndex_t)(g_moduleBase + RVA_Pendulum_AddIndex);
    g_ScoreObjectCtor = (ScoreObject_ctor_t)(g_moduleBase + RVA_ScoreObject_ctor);
    g_TimerDecrement = (Timer_Decrement_t)(g_moduleBase + RVA_Timer_Decrement);
    g_ArenaScoreParticleCtor = (ArenaScoreParticle_ctor_t)(g_moduleBase + RVA_ArenaScoreParticle_ctor);
    g_AthenaHashTableLookup = (AthenaHashTable_Lookup_t)(g_moduleBase + RVA_AthenaHashTable_Lookup);
    g_AthenaListContainsValue = (AthenaList_ContainsValue_t)(g_moduleBase + RVA_AthenaList_ContainsValue);
    g_SceneObjectSub1Ctor = (SceneObject_sub1_ctor_t)(g_moduleBase + RVA_SceneObject_sub1_ctor);
    g_AthenaStringSet = (AthenaString_Set_t)(g_moduleBase + RVA_AthenaString_Set);
    g_MWParserReadTag = (MWParser_ReadTag_t)(g_moduleBase + RVA_MWParser_ReadTag);
    g_StreamReaderDtor = (StreamReader_dtor_t)(g_moduleBase + RVA_StreamReader_dtor);
    g_AudioPlayMusic = (Audio_PlayMusic_t)(g_moduleBase + RVA_Audio_PlayMusic);
    g_DifficultyGetTimeModifier = (Difficulty_GetTimeModifier_t)(g_moduleBase + RVA_Difficulty_GetTimeModifier);
    g_AthenaStringFormat = (AthenaString_Format_t)(g_moduleBase + RVA_AthenaString_Format);
    g_AthenaStringSprintfToBuffer = (AthenaString_SprintfToBuffer_t)(g_moduleBase + RVA_AthenaString_SprintfToBuffer);
    g_WaveCos = (Wave_Fn_t)(g_moduleBase + RVA_Wave_Cos);
    g_WaveSin = (Wave_Fn_t)(g_moduleBase + RVA_Wave_Sin);
    g_SceneRegisterObject = (Scene_RegisterObject_t)(g_moduleBase + RVA_Scene_RegisterObject);
    g_AthenaListRemoveByValue = (AthenaList_RemoveByValue_t)(g_moduleBase + RVA_AthenaList_RemoveByValue);
    g_NeonPlatformActivate = (NeonPlatform_Activate_t)(g_moduleBase + RVA_NeonPlatform_Activate);
    g_SquareWobblyActivate = (SquareWobbly_Activate_t)(g_moduleBase + RVA_SquareWobbly_Activate);
    g_WavyActivate = (Wavy_Activate_t)(g_moduleBase + RVA_Wavy_Activate);
    g_SpinnerActivate = (Spinner_Activate_t)(g_moduleBase + RVA_Spinner_Activate);
    g_GearAddBall = (Gear_AddBall_t)(g_moduleBase + RVA_Gear_AddBall);
    g_NormalGravityReset = (NormalGravityReset_t)(g_moduleBase + RVA_NormalGravityReset);
    g_DropLiftActivate = (DropLift_Activate_t)(g_moduleBase + RVA_DropLift_Activate);
    g_CPUIDRNG = (CPUID_RNG_Fn_t)(g_moduleBase + RVA_CPUID_RNG_Fn);

    DebugLog("Function pointers resolved");

    GetConfigPath();
    {
        char dbg_cp[256];
        wsprintfA(dbg_cp, "GetConfigPath done: path='%s'", g_configPath[0] ? g_configPath : "(empty)");
        DebugLog(dbg_cp);
        char dbg_rf[256]; wsprintfA(dbg_rf, "RaceFiles path='%s'", g_raceFilesPath[0] ? g_raceFilesPath : "(empty)");
        DebugLog(dbg_rf);
    }
    // RaceFiles.txt: generate defaults if missing, then load
    {
        HANDLE hRf = CreateFileA(g_raceFilesPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hRf==INVALID_HANDLE_VALUE) {
            InitRaceFilesDefaults();
            GenerateRaceFiles();
            DebugLog("GenerateRaceFiles done (defaults)");
        } else CloseHandle(hRf);
    }
    LoadRaceFiles();
    DebugLog("LoadRaceFiles done");
    // Option B: LevelFeatures.txt deprecated — no LoadConfig
    DebugLog("LevelFeatures.txt deprecated (Option B, S1 collisions)");

    /* LevelData.txt — DEPRECATED Phase1: removed. g_levelData[] stays as
     * in-memory defaults only; spawns are S1-driven via ScanS1AndAutoEnable + S1Ensure*. */
    DebugLog("LevelData.txt deprecated — using in-memory defaults + S1 scan");

    InstallExtFreeHook();
    DebugLog("InstallExtFreeHook done");
    PatchAllocSizes();
    DebugLog("PatchAllocSizes done");
    InstallFileFallbackHook();
    DebugLog("InstallFileFallbackHook done");
    InstallBoardCtorHooks();
    DebugLog("InstallBoardCtorHooks done");
    InstallUniversalConstructorHook();
    DebugLog("InstallUniversalConstructorHook done");
    InstallVtablePatches();
    DebugLog("InstallVtablePatches done");
    InstallHook();
    DebugLog("InstallHook done");
    InstallFlagRenderHook();
    DebugLog("InstallFlagRenderHook done");
    DebugLog("=== PatchThread complete (v2 swirlfix) ===\n");
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        CreateThread(NULL, 0, PatchThread, NULL, 0, NULL);
    }
    return TRUE;
}
