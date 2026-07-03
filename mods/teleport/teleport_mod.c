/*
 * E:TELEPORT Mod for Hamsterball
 *
 * When the ball touches a floor panel tagged E:TELEPORT(LevelName),
 * the race ends silently (no music, no popups, no results screen)
 * and immediately loads and starts the specified level.
 *
 * Format: E:TELEPORT(3)       -> loads levels\level3.MESHWORLD (Intermediate)
 *         E:TELEPORT(7)       -> loads levels\level7.MESHWORLD (Neon)
 *         E:TELEPORT(neon)    -> same as E:TELEPORT(7)
 *         E:TELEPORT(level7)  -> same as E:TELEPORT(7)
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll teleport_mod.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 *
 * Rewritten as C (not C++) to match working jump_mod pattern.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ============================================================
 * Diagnostic logging — writes to teleport_log.txt next to bass.dll
 * ============================================================ */

static char g_logPath[MAX_PATH] = "";

static void diag_log(const char *msg) {
    if (g_logPath[0] == '\0') return;
    {
        HANDLE hFile = CreateFileA(g_logPath,
            FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written;
            SetFilePointer(hFile, 0, NULL, FILE_END);
            WriteFile(hFile, msg, (DWORD)strlen(msg), &written, NULL);
            WriteFile(hFile, "\r\n", 2, &written, NULL);
            CloseHandle(hFile);
        }
    }
}

static void diag_logf(const char *fmt, ...) {
    char buf[512];
    va_list args;
    if (g_logPath[0] == '\0') return;
    va_start(args, fmt);
    wvsprintfA(buf, fmt, args);
    va_end(args);
    diag_log(buf);
}

/* ============================================================
 * BASS Proxy Exports — match jump_mod pattern exactly
 * ============================================================ */

static HMODULE g_hRealBass = NULL;

typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}

typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c);
    return 1;
}

typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}

typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, DWORD, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, DWORD d, void* e) {
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 1;
}

typedef int  (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (real_BASS_Free) return real_BASS_Free();
    return 1;
}

typedef int  (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (real_BASS_Start) return real_BASS_Start();
    return 1;
}

typedef int  (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (real_BASS_Stop) return real_BASS_Stop();
    return 1;
}

typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}

typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int a, void* b, DWORD c, DWORD d, DWORD e, DWORD f) {
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f);
    return 0;
}

typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 1;
}

/* Extra BASS stubs — harmless if game doesn't import them */
__declspec(dllexport) void __stdcall BASS_Pause(void) {}
__declspec(dllexport) void __stdcall BASS_SetVolume(DWORD a) {}
__declspec(dllexport) DWORD __stdcall BASS_GetVolume(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_GetDevice(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_SetDevice(DWORD a) { return 1; }
__declspec(dllexport) void __stdcall BASS_GetInfo(void *a) {}
__declspec(dllexport) int __stdcall BASS_Update(DWORD a) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateFile(void *a, void *b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(int a, void *b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelPlay(DWORD a, BOOL b) { return 1; }
__declspec(dllexport) int __stdcall BASS_ChannelSetAttribute(DWORD a, DWORD b, float c) { return 1; }
__declspec(dllexport) int __stdcall BASS_ChannelGetAttribute(DWORD a, DWORD b, float *c) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetData(DWORD a, void *b, DWORD c) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetLevel(DWORD a) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelSetPosition(DWORD a, void *b, DWORD c) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetPosition(DWORD a, DWORD b) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelIsActive(DWORD a) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelRemoveSync(DWORD a, DWORD b) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelSetSync(DWORD a, DWORD b, DWORD c, void *d, void *e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleCreate(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleGetChannel(DWORD a, BOOL b) { return 0; }

static void load_real_bass(void)
{
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (g_hRealBass == NULL) {
        char path[MAX_PATH];
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&load_real_bass, &hSelf);
        if (hSelf && GetModuleFileNameA(hSelf, path, MAX_PATH)) {
            char *p = strrchr(path, '\\');
            if (p) { strcpy(p + 1, "bass_real.dll"); g_hRealBass = LoadLibraryA(path); }
        }
    }
    if (g_hRealBass) {
        real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
        real_BASS_MusicPlayEx          = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
        real_BASS_SetConfig             = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
        real_BASS_Init                  = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
        real_BASS_Free                  = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
        real_BASS_Start                 = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
        real_BASS_Stop                  = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
        real_BASS_ErrorGetCode          = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
        real_BASS_MusicLoad             = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
        real_BASS_ChannelStop            = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
    }
}

/* ============================================================
 * Game constants (verified via Ghidra decompilation)
 * ============================================================ */

#define EXE_BASE        0x00400000
#define APP_PTR         0x005341E0

/* Function addresses */
#define DISPATCH_COLLISION_EVENTS   0x0040C5D0
#define APP_START_RACE              0x004287C0
#define PLAYER_PROFILE_CTOR         0x00426F30
#define TOURNAMENT_ADVANCE_RACE     0x00427080

/* Board offsets */
#define BOARD_SCENE_PTR_OFFSET   0x878

/* App offsets */
#define APP_PROFILE_PTR          0x220
#define APP_ARENA_FLAG           0x237

/* PlayerProfile offsets */
#define PROFILE_RACE_INDEX       0x08
#define PROFILE_BOARD_PTR        0x0C

/* Ball offsets — verified from DispatchCollisionEvents disassembly:
 *   EBP=ball, ball+0x14=board, ball+0x18=playerIdx
 *   ball+0x2E9=death_pending (0=alive, 1=dying)
 *   board+0xCD0=goal_reached (set by N:GOAL handler) */
#define BALL_PLAYER_INDEX        0x18
#define BALL_DEATH_PENDING       0x2E9

/* Collision event pair offsets */
#define COLL_OBJ_NAME_OFFSET     0x864

/* Board goal-reached flag */
#define BOARD_GOAL_REACHED       0xCD0

/* Per-player flags */
#define APP_PLAYER_REACHED_GOAL_BASE   0x5D6
#define APP_PLAYER_STRIDE              0xA0
#define APP_PLAYER_SCORED_BASE         0x5FC
#define APP_PLAYER_NEWLY_GOAL_BASE     0x5F0

/* ============================================================
 * Level name -> race index mapping
 * ============================================================ */

typedef struct {
    const char* meshName;
    int raceIndex;
} LevelMapping;

static const LevelMapping levelMap[] = {
    {"level1", 1}, {"warmup", 1}, {"warm-up", 1},
    {"level2", 2}, {"beginner", 2}, {"cascade", 2},
    {"level3", 3}, {"intermediate", 3},
    {"level4", 4}, {"dizzy", 4},
    {"level5", 5}, {"tower", 5},
    {"level6", 6}, {"up", 6},
    {"level7", 7}, {"neon", 7},
    {"level8", 8}, {"expert", 8},
    {"level9", 9}, {"odd", 9},
    {"level10", 10}, {"toob", 10},
    {"level11", 11}, {"wobbly", 11},
    {"level12", 12}, {"glass", 12},
    {"level13", 13}, {"sky", 13},
    {"level14", 14}, {"master", 14},
    {"level15", 15}, {"impossible", 15},
    {NULL, 0}
};

static int findRaceIndex(const char *levelName) {
    if (levelName[0] >= '1' && levelName[0] <= '9') {
        int num = atoi(levelName);
        if (num >= 1 && num <= 15) return num;
    }
    {
        int j;
        for (j = 0; levelMap[j].meshName; j++) {
            if (_stricmp(levelName, levelMap[j].meshName) == 0)
                return levelMap[j].raceIndex;
        }
    }
    return -1;
}

/* ============================================================
 * Globals
 * ============================================================ */

static volatile int g_teleportActive = 0;
static volatile int g_teleportLevelIndex = -1;
static volatile int g_teleportFrameDelay = 0;

/* Original function bytes (8 bytes for DispatchCollisionEvents) */
/* Signature: 6A FF 64 A1 00 00 00 00 */
static unsigned char g_trampoline[16];
static void *g_origDispatch = NULL;

/* ============================================================
 * Memory helpers
 * ============================================================ */

static void WriteMemory(void *addr, const void *data, SIZE_T size) {
    DWORD oldProtect;
    if (VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy(addr, data, size);
        VirtualProtect(addr, size, oldProtect, &oldProtect);
    }
}

static void PatchMemory(void *addr, unsigned char value, SIZE_T size) {
    DWORD oldProtect;
    if (VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memset(addr, value, size);
        VirtualProtect(addr, size, oldProtect, &oldProtect);
    }
}

static int GetApp(void) {
    return *(int *)APP_PTR;
}

/* ============================================================
 * Parse level name from E:TELEPORT(LevelName)
 * ============================================================ */

static int isTeleportEvent(const char *eventName) {
    if (!eventName) return 0;
    return _strnicmp(eventName, "E:TELEPORT", 10) == 0;
}

static int parseTeleportLevel(const char *eventName, char *outLevelName, int outSize) {
    const char *start = strchr(eventName, '(');
    const char *end;
    int len;
    if (!start) return 0;
    start++;
    end = strchr(start, ')');
    if (!end) return 0;
    len = (int)(end - start);
    if (len <= 0 || len >= outSize) return 0;
    strncpy(outLevelName, start, len);
    outLevelName[len] = 0;
    return 1;
}

/* ============================================================
 * Set win state (replicate what N:GOAL does, minus audio/popups)
 * ============================================================ */

static void setWinState(void *board, int *ball) {
    int app = GetApp();
    int scene;
    int playerIdx;
    if (!app) return;

    scene = *(int *)((char *)board + BOARD_SCENE_PTR_OFFSET);
    if (!scene) return;

    playerIdx = ball[BALL_PLAYER_INDEX / 4];

    *((char *)board + BOARD_GOAL_REACHED) = 1;
    /* Don't set ball state — let the game's natural goal flow handle it */
    *((char *)app + APP_PLAYER_REACHED_GOAL_BASE + playerIdx * APP_PLAYER_STRIDE) = 1;

    if (*((char *)app + APP_PLAYER_SCORED_BASE + playerIdx * APP_PLAYER_STRIDE) == 0) {
        *((int *)((char *)app + APP_PLAYER_SCORED_BASE + playerIdx * APP_PLAYER_STRIDE)) = 1;
    }
    *((char *)app + APP_PLAYER_NEWLY_GOAL_BASE + playerIdx * APP_PLAYER_STRIDE) = 1;
}

/* ============================================================
 * Load and start the target level
 *
 * Tournament_AdvanceRace does: raceIndex = profile+0x08 + 1, then
 * switches on it. So to get case N, we need profile+0x08 = N-1.
 * ============================================================ */

typedef void (__fastcall *App_StartRace_t)(int app);
typedef void (__thiscall *PlayerProfile_ctor_t)(void *profile, int app, unsigned char partyFlag);
typedef void (__thiscall *Tournament_AdvanceRace_t)(void *profile, char param_1);

static void loadTargetLevel(int levelIndex) {
    int app;
    void *profileMem;
    unsigned char partyFlag;
    App_StartRace_t startRace;
    PlayerProfile_ctor_t profileCtor;
    Tournament_AdvanceRace_t advanceRace;

    if (levelIndex < 1 || levelIndex > 15) return;

    app = GetApp();
    if (!app) return;

    diag_logf("[loadTargetLevel] levelIndex=%d app=0x%08X", levelIndex, app);

    /* Step 1: Call App_StartRace(app) to clean up current race state */
    startRace = (App_StartRace_t)APP_START_RACE;
    startRace(app);

    /* Step 2: Clear arena flag */
    *((char *)app + APP_ARENA_FLAG) = 0;

    /* Step 3: Set up player slots (same as App_StartPracticeRace) */
    *((char *)app + 0x717) = 1;
    *((char *)app + 0x7B7) = 1;
    *((char *)app + 0x5D7) = 0;
    *((char *)app + 0x677) = 1;
    if (*((char *)app + 0x234) != 0) {
        *((char *)app + 0x677) = 0;
    }
    *((int *)app + 0x23C / 4) = 1;

    /* Step 4: Create PlayerProfile */
    profileMem = HeapAlloc(GetProcessHeap(), 0, 0x98);
    if (!profileMem) return;

    profileCtor = (PlayerProfile_ctor_t)PLAYER_PROFILE_CTOR;
    partyFlag = *((char *)app + 0x234);
    profileCtor(profileMem, app, partyFlag);

    *(void **)((char *)app + APP_PROFILE_PTR) = profileMem;

    /* Step 5: Set race index to levelIndex - 1 */
    *(int *)((char *)profileMem + PROFILE_RACE_INDEX) = levelIndex - 1;

    /* Set time-trial flag (same as practice mode) */
    *((char *)profileMem + 0x11) = 1;

    /* Step 6: Call Tournament_AdvanceRace(profile, 0) */
    advanceRace = (Tournament_AdvanceRace_t)TOURNAMENT_ADVANCE_RACE;
    diag_logf("[loadTargetLevel] Calling AdvanceRace(profile=0x%08X, 0)", (unsigned)profileMem);
    advanceRace(profileMem, 0);
    diag_log("[loadTargetLevel] AdvanceRace returned OK");
}

/* ============================================================
 * Hooked DispatchCollisionEvents
 *
 * We can't use __thiscall in C with MinGW reliably, so we use
 * __fastcall and extract board from ECX via inline asm.
 * Actually, since DispatchCollisionEvents is __thiscall
 * (ECX=board, stack: ball, collObj, RET 0x8), we can use a
 * naked function or __fastcall trick.
 *
 * Simplest approach: use a detour that saves ECX and calls
 * a C handler.
 * ============================================================ */

/* Called from the asm detour. ECX (board) is saved in g_savedBoard. */
static volatile void *g_savedBoard = NULL;
static volatile int *g_savedBall = NULL;
static volatile int *g_savedCollObj = NULL;

static volatile int g_hookFireCount = 0;
static volatile int g_teleportMatchCount = 0;

static void TeleportCollisionHandler(void) {
    void *board = (void *)g_savedBoard;
    int *ball = (int *)g_savedBall;
    int *collObj = (int *)g_savedCollObj;

    g_hookFireCount++;

    if (collObj && collObj[1]) {
        const char *eventName = *(const char **)((char *)collObj[1] + COLL_OBJ_NAME_OFFSET);
        if (eventName) {
            /* Log every event name so we can see what the game sends */
            if (g_hookFireCount <= 20 || (g_hookFireCount % 100) == 0) {
                diag_logf("[hook #%d] event=\"%s\"", g_hookFireCount, eventName);
            }
            if (isTeleportEvent(eventName)) {
                g_teleportMatchCount++;
                {
                    char levelName[128];
                    if (parseTeleportLevel(eventName, levelName, sizeof(levelName))) {
                        int raceIndex = findRaceIndex(levelName);
                        diag_logf("[TELEPORT MATCH #%d] event=\"%s\" level=\"%s\" raceIndex=%d",
                           g_teleportMatchCount, eventName, levelName, raceIndex);
                        if (raceIndex > 0) {
                            if (*((char *)board + BOARD_GOAL_REACHED) == 0 &&
                                *((char *)ball + BALL_DEATH_PENDING) == 0) {
                                setWinState(board, ball);
                                g_teleportLevelIndex = raceIndex;
                                g_teleportActive = 1;
                                g_teleportFrameDelay = 1;
                                diag_logf("[TELEPORT] Triggered! level=%d, deferred load next frame (main thread)", raceIndex);
                            } else {
                                diag_logf("[TELEPORT] SKIPPED: goal_reached=%d death_pending=%d",
                                   *((char *)board + BOARD_GOAL_REACHED),
                                   *((char *)ball + BALL_DEATH_PENDING));
                            }
                        }
                    }
                }
            }
        }
    } else {
        if (g_hookFireCount <= 5) {
            diag_logf("[hook #%d] collObj=%08X collObj[1]=%08X (null check failed)",
               g_hookFireCount, (unsigned)collObj, collObj ? (unsigned)collObj[1] : 0);
        }
    }
}

/* ============================================================
 * Main-thread per-frame hook for deferred level loading
 *
 * v1: Used background thread → race condition crash (Draw vs destroy).
 * v2: Hooked App_FrameUpdate ENTRY → loadTargetLevel ran BEFORE GameUpdate,
 *     modifying the scene graph mid-frame → crash inside CreateMechanicalObjects
 *     during the subsequent GameUpdate call (crash 0001:0001820B = 0x41820B).
 * v3: Hook App_FrameUpdate EPILOGUE (after GameUpdate returns). This ensures
 *     loadTargetLevel runs at the END of the frame, after all scene updates
 *     are complete. The new board will be fully constructed and ready for
 *     the NEXT frame's GameUpdate.
 *
 * App_FrameUpdate epilogue at 0x46C1F1:
 *   5E       POP ESI      (1 byte)
 *   83 C4 08 ADD ESP, 0x8 (3 bytes)
 *   C3       RET          (1 byte)
 * Total: 5 bytes — perfect for a 5-byte JMP (no NOPs needed).
 * ============================================================ */

#define APP_FRAME_UPDATE_EPILOGUE 0x0046C1F1

static unsigned char *g_frameUpdateDetour = NULL;

static void FrameUpdateHandler(void) {
    if (g_teleportActive && g_teleportFrameDelay > 0) {
        g_teleportFrameDelay--;
        if (g_teleportFrameDelay <= 0) {
            int levelIdx;
            g_teleportActive = 0;
            levelIdx = g_teleportLevelIndex;
            g_teleportLevelIndex = -1;
            loadTargetLevel(levelIdx);
        }
    }
}

/* ============================================================
 * Install hooks
 *
 * DispatchCollisionEvents at 0x40C5D0 is __thiscall:
 *   ECX = board, stack: [ball] [collObj] RET 0x8
 *
 * Original prologue (8 bytes):
 *   6A FF                 PUSH -1
 *   64 A1 00 00 00 00     MOV EAX, FS:[0]
 *
 * We patch with a 5-byte JMP to our detour, + 3 NOPs.
 * The detour saves registers, calls our C handler, then
 * executes the original prologue and JMPs back.
 * ============================================================ */

/* Detour code: save ECX/EDX, call C handler, restore, execute
 * original 8 bytes, JMP back.
 *
 * We allocate an executable buffer for this. */
static unsigned char *g_detourBuf = NULL;

static void InstallHooks(void) {
    unsigned char *dispatchAddr = (unsigned char *)DISPATCH_COLLISION_EVENTS;
    DWORD oldProtect;

    /* Verify signature */
    if (dispatchAddr[0] != 0x6A || dispatchAddr[1] != 0xFF ||
        dispatchAddr[2] != 0x64 || dispatchAddr[3] != 0xA1) {
        diag_log("[FATAL] Signature mismatch at 0x40C5D0! Hook NOT installed.");
        return;
    }
    diag_log("[InstallHooks] DispatchCollisionEvents signature OK at 0x40C5D0");

    /* Save original 8 bytes into trampoline */
    memcpy(g_trampoline, dispatchAddr, 8);

    /* Build trampoline: original 8 bytes + JMP to dispatchAddr+8 */
    g_trampoline[8] = 0xE9;  /* JMP rel32 */
    *(DWORD *)(g_trampoline + 9) = (DWORD)(dispatchAddr + 8) - (DWORD)(g_trampoline + 13);

    /* Make trampoline executable */
    VirtualProtect(g_trampoline, sizeof(g_trampoline), PAGE_EXECUTE_READWRITE, &oldProtect);
    g_origDispatch = g_trampoline;

    /* Allocate executable buffer for detour */
    g_detourBuf = (unsigned char *)VirtualAlloc(NULL, 256, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!g_detourBuf) return;

    /* Build detour code:
     *
     * The game calls DispatchCollisionEvents as:
     *   CALL [vtable+0x74]   ; ECX=board, stack: [ball] [collObj]
     *
     * When we JMP to our detour, the stack has:
     *   [return_addr] [ball] [collObj]
     * And ECX = board.
     *
     * We need to:
     * 1. Save ECX, EDX (fastcall uses them)
     * 2. Read [ESP+4]=ball, [ESP+8]=collObj from the stack
     * 3. Call TeleportCollisionHandler (which reads globals)
     * 4. Restore registers
     * 5. Execute original 8 bytes (PUSH -1; MOV EAX,FS:[0])
     * 6. JMP to dispatchAddr+8
     */

    {
        unsigned char *p = g_detourBuf;

        /* Save registers we'll clobber */
        /* PUSH ECX (board) */
        *p++ = 0x51;
        /* PUSH EDX */
        *p++ = 0x52;

        /* Store board (ECX) to g_savedBoard */
        /* MOV [g_savedBoard], ECX */
        *p++ = 0x89;  /* MOV r/m32, r32 */
        *p++ = 0x0D;  /* MOV ECX -> disp32 */
        *(DWORD *)p = (DWORD)&g_savedBoard; p += 4;

        /* Store ball ([ESP+12] = [original_esp+4] after 2 pushes = ball) */
        /* MOV EAX, [ESP+12] */
        *p++ = 0x8B; *p++ = 0x44; *p++ = 0x24; *p++ = 0x0C;
        /* MOV [g_savedBall], EAX */
        *p++ = 0xA3;
        *(DWORD *)p = (DWORD)&g_savedBall; p += 4;

        /* Store collObj ([ESP+16] = [original_esp+8] after 2 pushes) */
        /* MOV EAX, [ESP+16] */
        *p++ = 0x8B; *p++ = 0x44; *p++ = 0x24; *p++ = 0x10;
        /* MOV [g_savedCollObj], EAX */
        *p++ = 0xA3;
        *(DWORD *)p = (DWORD)&g_savedCollObj; p += 4;

        /* Call TeleportCollisionHandler */
        /* MOV EAX, TeleportCollisionHandler ; CALL EAX */
        *p++ = 0xB8;
        *(DWORD *)p = (DWORD)&TeleportCollisionHandler; p += 4;
        *p++ = 0xFF; *p++ = 0xD0;  /* CALL EAX */

        /* Restore registers */
        *p++ = 0x5A;  /* POP EDX */
        *p++ = 0x59;  /* POP ECX */

        /* Execute original 8 bytes (from trampoline copy) */
        /* PUSH -1 */
        *p++ = 0x6A; *p++ = 0xFF;
        /* MOV EAX, FS:[0] */
        *p++ = 0x64; *p++ = 0xA1; *p++ = 0x00; *p++ = 0x00; *p++ = 0x00; *p++ = 0x00;

        /* JMP to dispatchAddr+8 */
        *p++ = 0xE9;
        *(DWORD *)p = (DWORD)(dispatchAddr + 8) - (DWORD)(p + 4);
        p += 4;
    }

    /* Patch original function: JMP to detour */
    VirtualProtect(dispatchAddr, 8, PAGE_EXECUTE_READWRITE, &oldProtect);
    dispatchAddr[0] = 0xE9;  /* JMP rel32 */
    *(DWORD *)(dispatchAddr + 1) = (DWORD)g_detourBuf - (DWORD)(dispatchAddr + 5);
    dispatchAddr[5] = 0x90;
    dispatchAddr[6] = 0x90;
    dispatchAddr[7] = 0x90;
    VirtualProtect(dispatchAddr, 8, oldProtect, &oldProtect);

    diag_log("[InstallHooks] DispatchCollisionEvents hook installed OK.");

    /* ---- Hook App_FrameUpdate EPILOGUE (0x46C1F1) ----
     *
     * The epilogue runs AFTER GameUpdate returns, so the scene is in a
     * stable state. loadTargetLevel() can safely destroy the old board
     * and create a new one without corrupting the current frame.
     *
     * Original epilogue (5 bytes):
     *   5E          POP ESI       (1 byte)
     *   83 C4 08    ADD ESP, 0x8  (3 bytes)
     *   C3          RET           (1 byte)
     *
     * We patch all 5 bytes with a JMP to our detour. The detour:
     * 1. Calls FrameUpdateHandler() (checks teleport flag, loads level if needed)
     * 2. Executes the original 5 bytes (POP ESI, ADD ESP, RET)
     *
     * No register preservation needed beyond what the callee requires.
     * FrameUpdateHandler is __cdecl and only touches globals, so EAX/ECX/EDX
     * are clobbered (fine — they're dead at function return anyway).
     */

    {
        unsigned char *epiAddr = (unsigned char *)APP_FRAME_UPDATE_EPILOGUE;

        /* Verify signature: 5E 83 C4 08 C3 */
        if (epiAddr[0] != 0x5E || epiAddr[1] != 0x83 || epiAddr[2] != 0xC4 ||
            epiAddr[3] != 0x08 || epiAddr[4] != 0xC3) {
            diag_log("[FATAL] Signature mismatch at App_FrameUpdate epilogue (0x46C1F1)!");
            return;
        }
        diag_log("[InstallHooks] App_FrameUpdate epilogue signature OK at 0x46C1F1");

        /* Allocate executable buffer for the epilogue detour */
        g_frameUpdateDetour = (unsigned char *)VirtualAlloc(NULL, 256, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (!g_frameUpdateDetour) {
            diag_log("[FATAL] Failed to allocate memory for FrameUpdate epilogue detour");
            return;
        }

        {
            unsigned char *p = g_frameUpdateDetour;

            /* Call FrameUpdateHandler() — no args, reads globals.
             * At this point ESP still has the return address on top
             * (we haven't executed POP ESI yet). ESI still holds the
             * app pointer. We preserve ESI since we need it for the
             * original POP ESI instruction. */
            *p++ = 0x56;  /* PUSH ESI (preserve — FrameUpdateHandler might clobber it) */

            /* MOV EAX, FrameUpdateHandler ; CALL EAX */
            *p++ = 0xB8;
            *(DWORD *)p = (DWORD)&FrameUpdateHandler; p += 4;
            *p++ = 0xFF; *p++ = 0xD0;  /* CALL EAX */

            *p++ = 0x5E;  /* POP ESI (restore) */

            /* Execute original epilogue: POP ESI, ADD ESP 8, RET */
            *p++ = 0x5E;              /* POP ESI */
            *p++ = 0x83; *p++ = 0xC4; *p++ = 0x08;  /* ADD ESP, 0x8 */
            *p++ = 0xC3;              /* RET */
        }

        /* Patch original epilogue: 5-byte JMP (exactly fits, no NOPs) */
        VirtualProtect(epiAddr, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
        epiAddr[0] = 0xE9;  /* JMP rel32 */
        *(DWORD *)(epiAddr + 1) = (DWORD)g_frameUpdateDetour - (DWORD)(epiAddr + 5);
        VirtualProtect(epiAddr, 5, oldProtect, &oldProtect);

        diag_log("[InstallHooks] App_FrameUpdate epilogue hook installed OK. Level loading runs after GameUpdate.");
    }
}

/* ============================================================
 * Init thread
 * ============================================================ */

static DWORD WINAPI InitThread(LPVOID param) {
    (void)param;
    Sleep(2000);  /* wait for game to fully initialize */
    InstallHooks();
    return 0;
}

/* ============================================================
 * DllMain
 * ============================================================ */

BOOL APIENTRY DllMain(HMODULE hInst, DWORD reason, LPVOID lpReserved) {
    (void)lpReserved;
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInst);

        /* Set up log path next to bass.dll */
        {
            char mod_path[MAX_PATH];
            if (GetModuleFileNameA(hInst, mod_path, MAX_PATH)) {
                char *p = strrchr(mod_path, '\\');
                if (p) {
                    strcpy(p + 1, "teleport_log.txt");
                    strncpy(g_logPath, mod_path, MAX_PATH - 1);
                }
            }
        }

        diag_log("=== TELEPORT MOD LOADED ===");
        diag_logf("bass_real.dll load: %s", "starting...");

        load_real_bass();

        diag_logf("bass_real.dll handle: 0x%08X", (unsigned)g_hRealBass);

        CreateThread(NULL, 0, InitThread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
