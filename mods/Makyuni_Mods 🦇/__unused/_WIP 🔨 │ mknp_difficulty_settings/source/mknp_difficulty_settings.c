/*
 * mknp_difficulty_settings.c — Difficulty-based level file redirect mod (v3.1)
 *
 * COMPLETE REWRITE. Instead of modifying entity names, this mod redirects
 * which MESHWORLD file gets loaded based on tournament difficulty.
 *
 * HOW IT WORKS:
 *   1. Hooks LoadMeshWorld (0x0045DE30) — the function that takes a level
 *      name like "levels\level1" and loads "levels\level1.meshworld".
 *      Before calling the original, the mod checks:
 *        - Are we in Tournament mode? (profile+0x10==0 && profile+0x11==0)
 *        - What difficulty? (App+0x23C: 0=Pipsqueak, 1=Normal, 2=Frenzied)
 *      If in Tournament mode:
 *        - Pipsqueak (0): redirect "levels\level1" → "levels\level1-easy"
 *        - Normal (1):    no redirect (use default file)
 *        - Frenzied (2):  redirect "levels\level1" → "levels\level1-hard"
 *      If NOT in Tournament mode (Practice/Party):
 *        - Try "levels\level1-hard", fallback to default if missing.
 *
 *      If the -easy or -hard variant doesn't exist, falls back to default.
 *
 *   2. NOPs the difficulty gates in Board_Setup so entities always spawn
 *      regardless of difficulty. This ensures 8-balls/Mousetraps appear on
 *      Pipsqueak difficulty (they're in the level file, just gated out).
 *        - 0x0041C9E4: 74 07 → 90 90 (CreateBadBalls gate)
 *        - 0x0041CA05: 74 07 → 90 90 (CreateMouseTrap gate)
 *
 * TOURNAMENT DETECTION:
 *   App_StartTournamentRace: does NOT set profile+0x10 or profile+0x11
 *   App_StartPracticeRace: sets profile+0x11 = 1
 *   App_StartPartyRace: sets profile+0x10 = 1
 *   So: is_tournament = (profile+0x10==0 && profile+0x11==0)
 *
 * BUILD:
 *   i686-w64-mingw32-gcc -shared -o bass.dll mknp_difficulty_settings.c \
 *     -I../shared -lwinmm -Wl,--enable-stdcall-fixup -O2 -static \
 *     -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 */

#include "bass_proxy.h"

/* ═══════════════════════════════════════════════════════════════════════════
 * Game addresses
 * ═══════════════════════════════════════════════════════════════════════════ */

#define APP_DIFFICULTY_OFFSET  0x23C
#define APP_PROFILE_OFFSET     0x220
#define PROFILE_PARTY_FLAG     0x10   /* profile+0x10 = 1 if Party/2P mode */
#define PROFILE_PRACTICE_FLAG  0x11   /* profile+0x11 = 1 if Practice/TimeTrial */

/* LoadMeshWorld — __thiscall(this, char* levelName)
 * Takes a level name like "levels\level1", formats as "%s.meshworld", loads file.
 * Entry: 0x0045DE30
 * Prologue: MOV EAX,FS:[0] (6 bytes) + PUSH -1 (2 bytes) = 8 bytes */
#define LOADMESHWORLD_ADDR 0x0045DE30

/* Board_Setup difficulty gates — JZ instructions that skip entity factories
 * on Pipsqueak difficulty. NOP them so factories always run.
 * 0x0041C9E4: 74 07 → 90 90 (CreateBadBalls gate)
 * 0x0041CA05: 74 07 → 90 90 (CreateMouseTrap gate) */
#define GATE_CREATEBADBALLS   0x0041C9E4
#define GATE_CREATEMOUSETRAP  0x0041CA05

/* ═══════════════════════════════════════════════════════════════════════════
 * Tournament detection
 * ═══════════════════════════════════════════════════════════════════════════ */

static int is_tournament_mode(void) {
    if (IsBadReadPtr((void*)GLOBAL_APP_PTR, 4)) return 0;
    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
    if (!app || app < 0x10000) return 0;

    if (IsBadReadPtr((void*)(app + APP_PROFILE_OFFSET), 4)) return 0;
    DWORD profile = *(DWORD*)(app + APP_PROFILE_OFFSET);
    if (!profile || profile < 0x10000) return 0;

    if (IsBadReadPtr((void*)(profile + PROFILE_PARTY_FLAG), 1)) return 0;
    if (IsBadReadPtr((void*)(profile + PROFILE_PRACTICE_FLAG), 1)) return 0;

    /* Tournament: neither party nor practice flag is set */
    if (*(BYTE*)(profile + PROFILE_PARTY_FLAG) != 0) return 0;
    if (*(BYTE*)(profile + PROFILE_PRACTICE_FLAG) != 0) return 0;

    return 1;
}

static int get_difficulty(void) {
    if (IsBadReadPtr((void*)GLOBAL_APP_PTR, 4)) return 1;
    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
    if (!app || app < 0x10000) return 1;
    if (IsBadReadPtr((void*)(app + APP_DIFFICULTY_OFFSET), 4)) return 1;
    int diff = *(int*)(app + APP_DIFFICULTY_OFFSET);
    if (diff < 0 || diff > 2) return 1;
    return diff;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Level name redirect
 *
 * Rules:
 *   Tournament Pipsqueak (diff=0): append "-easy"
 *   Tournament Normal (diff=1):    no suffix (default file)
 *   Tournament Frenzied (diff=2):  append "-hard"
 *   Practice/Party:                append "-hard", fallback to default if missing
 *
 * Returns: 1=redirected (name in outBuf), 0=no redirect needed
 * ═══════════════════════════════════════════════════════════════════════════ */

static int redirect_level_name(const char *original, char *outBuf, int bufSize) {
    int diff = get_difficulty();
    int tournament = is_tournament_mode();
    const char *suffix = NULL;

    if (tournament) {
        switch (diff) {
            case 0:  suffix = "-easy"; break;  /* Pipsqueak */
            case 1:  suffix = NULL;   break;  /* Normal — default file */
            case 2:  suffix = "-hard"; break;  /* Frenzied */
            default: suffix = NULL;   break;
        }
    } else {
        /* Practice/Party: try -hard suffix, fallback to default */
        suffix = "-hard";
    }

    if (!suffix) return 0;

    /* Build redirected name: "levels\level1" + "-easy" = "levels\level1-easy" */
    int origLen = strlen(original);
    int suffixLen = strlen(suffix);
    if (origLen + suffixLen + 1 > bufSize) return 0;

    /* Check if the name already has a suffix (avoid double-suffixing) */
    if (origLen >= 5 && _strnicmp(original + origLen - 5, "-easy", 5) == 0) return 0;
    if (origLen >= 5 && _strnicmp(original + origLen - 5, "-hard", 5) == 0) return 0;
    if (origLen >= 7 && _strnicmp(original + origLen - 7, "-normal", 7) == 0) return 0;

    /* Also skip arena/stands/spawnplatform files (not race levels) */
    if (_strnicmp(original, "levels\\arena", 12) == 0) return 0;
    if (_strnicmp(original, "Levels\\Arena", 12) == 0) return 0;

    strcpy(outBuf, original);
    strcat(outBuf, suffix);

    /* Check if the redirected .meshworld file exists.
     * LoadMeshWorld formats as "%s.meshworld", so we check that path. */
    char checkPath[MAX_PATH];
    _snprintf(checkPath, MAX_PATH, "%s.meshworld", outBuf);
    if (GetFileAttributesA(checkPath) == INVALID_FILE_ATTRIBUTES) {
        /* File doesn't exist — fall back to default */
        return 0;
    }

    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Detour hook for LoadMeshWorld
 *
 * Target: LoadMeshWorld at 0x0045DE30
 * Calling convention: __thiscall(this, char* levelName)
 *   ECX = this (level object)
 *   stack param 1 = levelName (char*)
 *
 * Prologue: MOV EAX,FS:[0] (6 bytes) + PUSH -1 (2 bytes) = 8 bytes
 * We copy 8 bytes to trampoline, patch 5-byte JMP + 3 NOPs.
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef void (__fastcall *LoadMeshWorld_t)(void *this_, void *edx, char *levelName);
static LoadMeshWorld_t g_origLoadMeshWorld = NULL;

/* Stable buffer for the redirected name (must persist until original call returns).
 * LoadMeshWorld is synchronous, so a static buffer is safe. */
static char g_redirectedName[MAX_PATH];

void __fastcall hook_LoadMeshWorld(void *this_, void *edx, char *levelName) {
    char redirected[MAX_PATH];

    if (redirect_level_name(levelName, redirected, MAX_PATH)) {
        /* Use redirected name */
        strncpy(g_redirectedName, redirected, MAX_PATH - 1);
        g_redirectedName[MAX_PATH - 1] = '\0';
        levelName = g_redirectedName;
    }

    /* Call original LoadMeshWorld via trampoline */
    g_origLoadMeshWorld(this_, edx, levelName);
}

static void install_loadmeshworld_hook(void) {
    DWORD target = LOADMESHWORLD_ADDR;
    unsigned char *src = (unsigned char*)target;

    /* Allocate executable page for trampoline */
    void *trampMem = VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE,
                                  PAGE_EXECUTE_READWRITE);
    if (!trampMem) return;
    unsigned char *tramp = (unsigned char*)trampMem;

    /* LoadMeshWorld prologue:
     *   64 A1 00 00 00 00  MOV EAX, FS:[0]   (6 bytes)
     *   6A FF              PUSH -1           (2 bytes)
     * Total: 8 bytes for 2 complete instructions.
     *
     * Copy 8 bytes to trampoline, then append JMP back to target+8.
     */
    DWORD old;
    VirtualProtect(src, 16, PAGE_EXECUTE_READWRITE, &old);
    memcpy(tramp, src, 8);               /* copy original 8 bytes */
    tramp[8] = 0xE9;                      /* JMP rel32 */
    DWORD back = (target + 8) - ((DWORD)(tramp + 8) + 5);
    memcpy(tramp + 9, &back, 4);
    VirtualProtect(src, 16, old, &old);
    FlushInstructionCache(GetCurrentProcess(), src, 16);

    /* Set up function pointer to trampoline */
    g_origLoadMeshWorld = (LoadMeshWorld_t)tramp;

    /* Install 5-byte JMP + 3 NOPs at target (8 bytes total) */
    VirtualProtect(src, 8, PAGE_EXECUTE_READWRITE, &old);
    src[0] = 0xE9;                        /* JMP rel32 */
    DWORD hookAddr = (DWORD)hook_LoadMeshWorld;
    DWORD offset = hookAddr - (target + 5);
    memcpy(src + 1, &offset, 4);
    src[5] = 0x90;                        /* NOP */
    src[6] = 0x90;                        /* NOP */
    src[7] = 0x90;                        /* NOP */
    VirtualProtect(src, 8, old, &old);
    FlushInstructionCache(GetCurrentProcess(), src, 8);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Difficulty gate patches
 *
 * NOPs the JZ instructions in Board_Setup that skip CreateBadBalls
 * and CreateMouseTrap on Pipsqueak difficulty. This ensures entities
 * spawn on all difficulties (they're in the level file, just gated out).
 * ═══════════════════════════════════════════════════════════════════════════ */

static void patch_difficulty_gates(void) {
    /* Gate 1: CreateBadBalls at 0x0041C9E4 (74 07 → 90 90) */
    patch_byte(GATE_CREATEBADBALLS, 0x90);
    patch_byte(GATE_CREATEBADBALLS + 1, 0x90);

    /* Gate 2: CreateMouseTrap at 0x0041CA05 (74 07 → 90 90) */
    patch_byte(GATE_CREATEMOUSETRAP, 0x90);
    patch_byte(GATE_CREATEMOUSETRAP + 1, 0x90);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);

        load_real_bass();

        /* Patch difficulty gates so entities always spawn */
        patch_difficulty_gates();

        /* Hook LoadMeshWorld to redirect level files based on difficulty */
        install_loadmeshworld_hook();
    }
    return TRUE;
}
