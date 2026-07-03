/*
 * E:TELEPORT Mod for Hamsterball
 * 
 * When the ball touches a floor panel tagged E:TELEPORT(LevelName),
 * the race ends silently (no music, no popups, no results screen)
 * and immediately loads and starts the specified level.
 *
 * Format: E:TELEPORT(Level1)  → loads levels\level1.MESHWORLD
 *         E:TELEPORT(LevelNeon) → loads levels\level7.MESHWORLD
 *
 * Build: i686-w64-mingw32-g++ -shared -o bass.dll teleport_mod.cpp \
 *        -static -lwinmm -Wl,--enable-stdcall-fixup
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// ============================================================
// Game constants (verified via Ghidra decompilation)
// ============================================================

#define EXE_BASE        0x00400000
#define APP_PTR         0x005341E0  // global App pointer

// Function addresses (RVAs = addr - EXE_BASE)
#define DISPATCH_COLLISION_EVENTS_RVA  0x0000C5D0  // 0x40C5D0
#define APP_START_RACE_RVA             0x000087C0  // 0x4287C0
#define APP_START_PRACTICE_RACE_RVA    0x00008C50  // 0x428C50
#define APP_START_TOURNAMENT_RACE_RVA  0x000088B0  // 0x4288B0

// Board offsets
#define BOARD_SCENE_PTR_OFFSET   0x878   // board+0x878 = scene ptr

// App offsets
#define APP_PROFILE_PTR          0x220   // App+0x220 = PlayerProfile*
#define APP_ARENA_FLAG           0x237   // App+0x237 = arena mode flag

// PlayerProfile offsets
#define PROFILE_RACE_INDEX       0x08    // profile+0x08 = current race index
#define PROFILE_BOARD_PTR        0x0C    // profile+0x0C = current board ptr

// Ball offsets (param_1 in DispatchCollisionEvents)
#define BALL_STATE               0x30    // ball+0x30 = state (5 = finished)
#define BALL_ALREADY_GOAL        0x53    // ball+0x53 = already reached goal flag (byte)
#define BALL_ALIVE_FLAG          0x1DA   // ball+0x1DA = alive/not-in-death-state (byte)
#define BALL_PLAYER_INDEX        0x18    // ball+0x18 = player index (int, param_1[6])

// Collision event pair offsets
// param_2 is int* pair: [0]=type/board, [1]=MeshBuffer
// MeshBuffer+0x864 = event name string pointer
#define COLL_OBJ_NAME_OFFSET     0x864   // param_2[1]+0x864 = event name

// Board+0xCD0 = goal-reached flag (set once per race when N:GOAL is touched)
#define BOARD_GOAL_REACHED       0xCD0

// Per-player "reached goal" flag: App + playerIdx*0xA0 + 0x5D6
#define APP_PLAYER_REACHED_GOAL_BASE   0x5D6
#define APP_PLAYER_STRIDE              0xA0

// Per-player "scored" flag: App + playerIdx*0xA0 + 0x5FC
#define APP_PLAYER_SCORED_BASE         0x5FC

// Per-player "newly reached goal" flag: App + playerIdx*0xA0 + 0x5F0
#define APP_PLAYER_NEWLY_GOAL_BASE     0x5F0

// Scene+0x210 = status string pointer (used for debug, N:GOAL sets it)
#define SCENE_STATUS_STRING     0x210

// App+0x8B4 = gate counter (incremented by popups, decremented when done)
// We need to NOT increment this (popups do it, we skip popups)

// ============================================================
// Level name → race index mapping
// Tournament_AdvanceRace switch cases (1-15)
// ============================================================

typedef struct {
    const char* meshName;   // e.g. "level1", "levelneon", "levelcascade"
    int raceIndex;           // 1-15 (0 = warm-up, 14 = impossible)
} LevelMapping;

static LevelMapping levelMap[] = {
    {"level1",        1},   // Warm-Up
    {"levelwarmup",   1},
    {"levelwarm-up",  1},
    {"level2",        2},   // Beginner
    {"levelbeginner", 2},
    {"levelcascade",  2},   // Cascade = Beginner race
    {"level3",        3},   // Intermediate
    {"levelintermediate", 3},
    {"level4",        4},   // Dizzy
    {"leveldizzy",    4},
    {"level5",        5},   // Tower
    {"leveltower",    5},
    {"level6",        6},   // Up
    {"levelup",       6},
    {"level7",        7},   // Neon
    {"levelneon",     7},
    {"level8",        8},   // Expert
    {"levelexpert",   8},
    {"level9",        9},   // Odd
    {"levelodd",      9},
    {"level10",       10},  // Toob
    {"leveltoob",     10},
    {"level11",       11},  // Wobbly
    {"levelwobbly",   11},
    {"level12",       12},  // Glass
    {"levelglass",    12},
    {"level13",       13},  // Sky
    {"levelsky",      13},
    {"level14",       14},  // Master
    {"levelmaster",   14},
    {"level15",       15},  // Impossible
    {"levelimpossible", 15},
    {0, 0}
};

static int findRaceIndex(const char* levelName) {
    char lower[128];
    int i;
    for (i = 0; i < 127 && levelName[i]; i++) {
        lower[i] = (char)tolower((unsigned char)levelName[i]);
    }
    lower[i] = 0;
    
    for (int j = 0; levelMap[j].meshName; j++) {
        if (_stricmp(lower, levelMap[j].meshName) == 0) {
            return levelMap[j].raceIndex;
        }
    }
    return -1;  // not found
}

// ============================================================
// Function pointer types
// ============================================================

typedef void (__fastcall *DispatchCollisionEvents_t)(void* board, int* ball, int* collObj);
typedef void (__fastcall *App_StartRace_t)(int app);
typedef void (__thiscall *App_StartPracticeRace_t)(void* app, int levelIndex);

// ============================================================
// Globals
// ============================================================

static HMODULE g_hModule = NULL;
static DWORD g_oldProtect = 0;
static bool g_initialized = false;
static bool g_teleportActive = false;        // set when E:TELEPORT triggered
static int  g_teleportLevelIndex = -1;       // target level to load
static int  g_teleportFrameDelay = 0;        // delay frames before loading

// Original function bytes (8 bytes for DispatchCollisionEvents)
// Signature: 6A FF 64 A1 00 00 00 00
static BYTE g_origBytes[8];
static BYTE* g_patchAddr = NULL;

// Original DispatchCollisionEvents trampoline
static DispatchCollisionEvents_t g_origDispatch = NULL;

// ============================================================
// Memory helpers
// ============================================================

static void WriteMemory(void* addr, const void* data, SIZE_T size) {
    DWORD oldProtect;
    if (VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy(addr, data, size);
        VirtualProtect(addr, size, oldProtect, &oldProtect);
    }
}

static void PatchMemory(void* addr, BYTE value, SIZE_T size) {
    DWORD oldProtect;
    if (VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memset(addr, value, size);
        VirtualProtect(addr, size, oldProtect, &oldProtect);
    }
}

// ============================================================
// Get App pointer
// ============================================================

static int GetApp() {
    return *(int*)APP_PTR;
}

// ============================================================
// Parse level name from E:TELEPORT(LevelName)
// Extracts text between ( and )
// ============================================================

static bool parseTeleportLevel(const char* eventName, char* outLevelName, int outSize) {
    // Find opening paren
    const char* start = strchr(eventName, '(');
    if (!start) return false;
    start++;  // skip '('
    
    // Find closing paren
    const char* end = strchr(start, ')');
    if (!end) return false;
    
    int len = (int)(end - start);
    if (len <= 0 || len >= outSize) return false;
    
    strncpy(outLevelName, start, len);
    outLevelName[len] = 0;
    return true;
}

// ============================================================
// Check if event name starts with "E:TELEPORT"
// ============================================================

static bool isTeleportEvent(const char* eventName) {
    if (!eventName) return false;
    return _strnicmp(eventName, "E:TELEPORT", 10) == 0;
}

// ============================================================
// Set win state (replicate what N:GOAL does, minus audio/popups)
// ============================================================

static void setWinState(void* board, int* ball) {
    int app = GetApp();
    if (!app) return;
    
    int scene = *(int*)((char*)board + BOARD_SCENE_PTR_OFFSET);
    if (!scene) return;
    
    // Get player index from ball
    int playerIdx = ball[BALL_PLAYER_INDEX / 4];  // ball+0x18 = player index
    
    // Set board goal-reached flag (prevents N:GOAL from re-triggering)
    *((char*)board + BOARD_GOAL_REACHED) = 1;
    
    // Set ball state to 5 (finished)
    ball[BALL_STATE / 4] = 5;
    
    // Set per-player "reached goal" flag
    *((char*)app + APP_PLAYER_REACHED_GOAL_BASE + playerIdx * APP_PLAYER_STRIDE) = 1;
    
    // Set per-player "scored" flag (if not already set)
    if (*((char*)app + APP_PLAYER_SCORED_BASE + playerIdx * APP_PLAYER_STRIDE) == 0) {
        *((int*)(app + APP_PLAYER_SCORED_BASE + playerIdx * APP_PLAYER_STRIDE)) = 1;
    }
    
    // Set per-player "newly reached goal" flag
    *((char*)app + APP_PLAYER_NEWLY_GOAL_BASE + playerIdx * APP_PLAYER_STRIDE) = 1;
    
    // Copy camera distance values (N:GOAL does this for the goal zoom)
    // scene+0x294 = camera distance ptr, scene+0x29C = goal zoom distance ptr
    // We skip this — no visual zoom needed since we're immediately loading
}

// ============================================================
// Load and start the target level
// ============================================================

static void loadTargetLevel(int levelIndex) {
    int app = GetApp();
    if (!app || levelIndex < 1 || levelIndex > 15) return;
    
    // Call App_StartRace(app) to clean up current race state
    // This destroys old board, profile, results, resets music
    App_StartRace_t startRace = (App_StartRace_t)(EXE_BASE + APP_START_RACE_RVA);
    startRace(app);
    
    // Call App_StartPracticeRace(app, levelIndex) to load the target level
    // App_StartPracticeRace is __thiscall(this=app, param_1=levelIndex)
    // It creates a new PlayerProfile, sets race index, and calls Tournament_AdvanceRace
    // which creates the LevelBoard_*_ctor for the target level
    // Tournament_AdvanceRace switches on raceIndex: case 1=WarmUp, case 2=Beginner, etc.
    typedef void (__thiscall *StartPracticeFunc)(void*, int);
    StartPracticeFunc startPractice = (StartPracticeFunc)(EXE_BASE + APP_START_PRACTICE_RACE_RVA);
    startPractice((void*)app, levelIndex);
}

// ============================================================
// Hooked DispatchCollisionEvents
// ============================================================

// Trampoline: saved 8 bytes of original function, then JMP back to original+8
// 6A FF 64 A1 00 00 00 00 = 8 bytes (PUSH -1; MOV EAX, FS:[0])
static BYTE g_trampoline[16];  // 8 bytes original + 5 bytes JMP + padding

static void __fastcall HookedDispatchCollisionEvents(void* board, int* ball, int* collObj) {
    // Check collision event name
    if (collObj && collObj[1]) {
        const char* eventName = *(const char**)((char*)collObj[1] + COLL_OBJ_NAME_OFFSET);
        if (eventName && isTeleportEvent(eventName)) {
            // Parse level name from E:TELEPORT(LevelName)
            char levelName[128];
            if (parseTeleportLevel(eventName, levelName, sizeof(levelName))) {
                int raceIndex = findRaceIndex(levelName);
                if (raceIndex > 0) {
                    // Check ball is alive and hasn't already reached goal
                    if (*((char*)ball + BALL_ALREADY_GOAL) == 0 &&
                        *((char*)ball + BALL_ALIVE_FLAG) != 0) {
                        
                        // Set win state (no music, no popups)
                        setWinState(board, ball);
                        
                        // Queue the level load for next frame
                        // (can't load immediately — we're inside a collision callback)
                        g_teleportLevelIndex = raceIndex;
                        g_teleportActive = true;
                        g_teleportFrameDelay = 2;  // wait 2 frames for safety
                    }
                }
            }
        }
    }
    
    // Call original DispatchCollisionEvents
    g_origDispatch(board, ball, collObj);
}

// ============================================================
// Per-frame hook: check if we need to load a new level
// We hook App_StartRace to intercept after it runs, but actually
// we need a per-frame tick. Let's use a simple polling thread.
// ============================================================

static DWORD WINAPI TeleportPollThread(LPVOID param) {
    while (true) {
        if (g_teleportActive && g_teleportFrameDelay > 0) {
            g_teleportFrameDelay--;
            if (g_teleportFrameDelay <= 0) {
                g_teleportActive = false;
                int levelIdx = g_teleportLevelIndex;
                g_teleportLevelIndex = -1;
                loadTargetLevel(levelIdx);
            }
        }
        Sleep(16);  // ~60fps polling
    }
    return 0;
}

// ============================================================
// Install hooks
// ============================================================

static void InstallHooks() {
    DWORD oldProtect;
    
    // === Hook DispatchCollisionEvents (0x40C5D0) ===
    BYTE* dispatchAddr = (BYTE*)(EXE_BASE + DISPATCH_COLLISION_EVENTS_RVA);
    
    // Verify signature: 6A FF 64 A1 00 00 00 00
    if (dispatchAddr[0] != 0x6A || dispatchAddr[1] != 0xFF ||
        dispatchAddr[2] != 0x64 || dispatchAddr[3] != 0xA1) {
        return;  // signature mismatch, abort
    }
    
    // Save original 8 bytes
    memcpy(g_origBytes, dispatchAddr, 8);
    
    // Build trampoline: 8 bytes original code + JMP to original+8
    memcpy(g_trampoline, g_origBytes, 8);
    g_trampoline[8] = 0xE9;  // JMP rel32
    *(DWORD*)(g_trampoline + 9) = (DWORD)(dispatchAddr + 8) - (DWORD)(g_trampoline + 13);
    
    // Set original function pointer to trampoline
    g_origDispatch = (DispatchCollisionEvents_t)g_trampoline;
    
    // Make trampoline executable
    VirtualProtect(g_trampoline, sizeof(g_trampoline), PAGE_EXECUTE_READWRITE, &oldProtect);
    
    // Patch original function: JMP to our hook
    VirtualProtect(dispatchAddr, 8, PAGE_EXECUTE_READWRITE, &oldProtect);
    dispatchAddr[0] = 0xE9;  // JMP rel32
    *(DWORD*)(dispatchAddr + 1) = (DWORD)&HookedDispatchCollisionEvents - (DWORD)(dispatchAddr + 5);
    // NOP padding (3 bytes)
    dispatchAddr[5] = 0x90;
    dispatchAddr[6] = 0x90;
    dispatchAddr[7] = 0x90;
    VirtualProtect(dispatchAddr, 8, oldProtect, &oldProtect);
    
    // Start polling thread for deferred level loading
    CreateThread(NULL, 0, TeleportPollThread, NULL, 0, NULL);
    
    g_initialized = true;
}

static DWORD WINAPI InitThread(LPVOID param) {
    Sleep(1000);  // wait for game to initialize
    InstallHooks();
    return 0;
}

// ============================================================
// DLL Entry Point
// ============================================================

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        g_hModule = hinstDLL;
        DisableThreadLibraryCalls(hinstDLL);
        
        // Install hooks in a separate thread to avoid DllMain deadlock
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)InitThread, NULL, 0, NULL);
    }
    return TRUE;
}
