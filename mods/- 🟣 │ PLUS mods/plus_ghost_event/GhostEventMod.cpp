#include "HamsterballAPI.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

/*
 * Ghost Event for HB+ v2.1
 *
 * Port of ghost_event (bass.dll proxy) to HB+ API.
 *
 * Differences from bass.dll version:
 *  - No BASS proxy shell.
 *  - DispatchCollisionEvents hook replaced by onEventPlaneCollide callback.
 *  - App_FrameUpdate epilogue hook replaced by onGameUpdate callback.
 *  - Memory patches applied through IModAPI::PatchMemory.
 *
 * Behavior preserved:
 *  - Detects E:GHOST(filename) event planes.
 *  - Loads .ghost files from a Ghosts/ directory next to the DLL.
 *  - Creates a BestTimeTracker and injects it into App+0x910 (playback).
 *  - Creates/reuses the ghost ball at board+0x361C.
 *  - Renders the ghost in any mode by NOP'ing the TT/party checks in
 *    Level_UpdateAndRender (0x40B7F5, 0x40B7FF).
 *  - Manually advances the playback index in non-Time-Trial modes.
 */

#define ENABLE_LOGGING

/* ---- Game function addresses (absolute) ---- */
#define ADDR_OPERATOR_NEW   0x004BA57B
#define ADDR_OPERATOR_DELETE 0x004BA74D
#define ADDR_BALL_CTOR      0x004039E0
#define ADDR_BALL_DTOR      0x00402A50
#define ADDR_BTT_CTOR       0x00427660
#define ADDR_BTT_DTOR       0x004278C0
#define ADDR_ALIST_APPEND   0x00453780

#define BTT_SIZE        0x528
#define SNAP_SIZE       0x28
#define BALL_SIZE       0xC60

/* ---- App offsets ---- */
#define APP_PROFILE_PTR     0x220
#define APP_BTT_PLAYBACK    0x910
#define APP_PARTY_FLAG      0x234

/* ---- Profile / Board offsets ---- */
#define PROFILE_BOARD_PTR   0x0C
#define BOARD_GHOST_BALL    0x361C

/* ---- BTT offsets ---- */
#define BTT_VTABLE          0x00
#define BTT_ALIST_VTABLE    0x04
#define BTT_ALIST_COUNT     0x08
#define BTT_LIST_ARRAY      0x410
#define BTT_PLAYBACK_IDX    0x41C
#define BTT_RACE_TIME       0x420
#define BTT_NAME            0x424
#define BTT_BEST_TIME       0x524

/* ---- Ball offsets ---- */
#define BALL_VTABLE         0x00
#define BALL_PLAYER_ID      0x018
#define BALL_POS_X          0x164
#define BALL_POS_Y          0x168
#define BALL_POS_Z          0x16C
#define BALL_MAX_SPEED      0x188
#define BALL_MASS           0x1A0
#define BALL_GRAVITY        0x278
#define BALL_GRAVITY_SCALE  0x27C
#define BALL_RADIUS         0x284
#define BALL_RESPAWN_FLAG   0x281
#define BALL_ALPHA          0x2FC
#define BALL_COLOR_R        0x2AC
#define BALL_COLOR_G        0x2B0
#define BALL_COLOR_B        0x2B4

#define BTT_VTABLE_ADDR     0x004D262C
#define ATHENALIST_VT       0x004D875C

/* ---- Ghost file magic ---- */
#define GHOST_MAGIC     0x47485347  /* "GHSG" */
#define GHOST_VERSION   1

#pragma pack(push, 1)
typedef struct {
    DWORD magic;
    DWORD version;
    DWORD time;
    DWORD frameCount;
} GhostFileHeader;
#pragma pack(pop)

/* ---- Logging ---- */
#ifdef ENABLE_LOGGING
static char g_logPath[MAX_PATH] = "";
static void diag_log(const char *msg) {
    if (!g_logPath[0]) return;
    HANDLE h = CreateFileA(g_logPath, FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD w; SetFilePointer(h, 0, NULL, FILE_END);
        WriteFile(h, msg, (DWORD)strlen(msg), &w, NULL);
        WriteFile(h, "\r\n", 2, &w, NULL);
        CloseHandle(h);
    }
}
static void diag_logf(const char *fmt, ...) {
    char buf[512]; va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    diag_log(buf);
}
#define LOG(...)   diag_logf(__VA_ARGS__)
#define LOGS(s)    diag_log(s)
#else
#define LOG(...)   do {} while(0)
#define LOGS(s)    do {} while(0)
#endif

/* ---- Inline asm wrappers for game functions ---- */

static DWORD game_operator_new(DWORD size) {
    DWORD addr = ADDR_OPERATOR_NEW;
    DWORD result;
    __asm__ volatile (
        "push %1\n\t"
        "call *%2\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result)
        : "r"(size), "r"(addr)
        : "eax", "ecx", "edx", "memory"
    );
    return result;
}

static void game_operator_delete(DWORD ptr) {
    DWORD addr = ADDR_OPERATOR_DELETE;
    __asm__ volatile (
        "push %0\n\t"
        "call *%1\n\t"
        : : "r"(ptr), "r"(addr)
        : "eax", "ecx", "edx", "memory"
    );
}

static void call_ball_ctor(DWORD ball, DWORD board) {
    DWORD addr = ADDR_BALL_CTOR;
    __asm__ volatile (
        "push %1\n\t"
        "movl %0, %%ecx\n\t"
        "call *%2\n\t"
        : : "r"(ball), "r"(board), "r"(addr)
        : "eax", "ecx", "edx", "memory"
    );
}

static void call_ball_vtable1(DWORD ball) {
    if (!ball) return;
    DWORD vt = *(DWORD*)ball;
    DWORD fn = *(DWORD*)(vt + 4);
    __asm__ volatile (
        "movl %0, %%ecx\n\t"
        "call *%1\n\t"
        : : "r"(ball), "r"(fn)
        : "eax", "ecx", "edx", "memory"
    );
}

static void call_ball_dtor(DWORD ball) {
    DWORD addr = ADDR_BALL_DTOR;
    __asm__ volatile (
        "push $1\n\t"
        "movl %0, %%ecx\n\t"
        "call *%1\n\t"
        : : "r"(ball), "r"(addr)
        : "eax", "ecx", "edx", "memory"
    );
}

static void call_btt_ctor(DWORD btt) {
    DWORD addr = ADDR_BTT_CTOR;
    __asm__ volatile (
        "movl %0, %%ecx\n\t"
        "call *%1\n\t"
        : : "r"(btt), "r"(addr)
        : "eax", "ecx", "edx", "memory"
    );
}

static void call_btt_dtor(DWORD btt) {
    DWORD addr = ADDR_BTT_DTOR;
    __asm__ volatile (
        "push $1\n\t"
        "movl %0, %%ecx\n\t"
        "call *%1\n\t"
        : : "r"(btt), "r"(addr)
        : "eax", "ecx", "edx", "memory"
    );
}

static void call_alist_append(DWORD list, DWORD item) {
    DWORD addr = ADDR_ALIST_APPEND;
    __asm__ volatile (
        "push %1\n\t"
        "movl %0, %%ecx\n\t"
        "call *%2\n\t"
        : : "r"(list), "r"(item), "r"(addr)
        : "eax", "ecx", "edx", "memory"
    );
}

/* ---- Helpers ---- */

static DWORD get_board(DWORD app) {
    if (!app || IsBadReadPtr((void*)app, 0x1000)) return 0;
    DWORD profile = *(DWORD*)(app + APP_PROFILE_PTR);
    if (!profile || IsBadReadPtr((void*)profile, 0x100)) return 0;
    DWORD board = *(DWORD*)(profile + PROFILE_BOARD_PTR);
    if (!board || IsBadReadPtr((void*)board, 0x4000)) return 0;
    return board;
}

/* ---- Mod state ---- */

static DWORD g_loadedBTT = 0;
static BOOL  g_ghostActive = FALSE;
static BOOL  g_ghostBallCreated = FALSE;
static BOOL  g_ghostFromEvent = FALSE;
static BOOL  g_needManualAdvance = FALSE;
static char  g_pendingGhostFile[256] = "";

/* ---- Ghost file loading ---- */

static int load_ghost_file(const char *filename, DWORD **outSnapshots, DWORD *outCount, DWORD *outFinishTime) {
    char path[MAX_PATH];
    DWORD bytesRead;
    GhostFileHeader header;

    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&load_ghost_file, &hSelf);
    if (hSelf && GetModuleFileNameA(hSelf, path, MAX_PATH)) {
        char *p = strrchr(path, '\\');
        if (p) {
            strcpy(p + 1, "Ghosts\\");
            strncat(path, filename, MAX_PATH - strlen(path) - 1);
            size_t plen = strlen(path);
            if (plen < MAX_PATH - 7 && (plen < 6 || _stricmp(path + plen - 6, ".ghost") != 0)) {
                strcat(path, ".ghost");
            }
        } else {
            snprintf(path, MAX_PATH, "Ghosts\\%s.ghost", filename);
        }
    } else {
        snprintf(path, MAX_PATH, "Ghosts\\%s.ghost", filename);
    }

    LOG("Loading ghost file: %s", path);
    HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        LOG("Failed to open: %s (err=%d)", path, GetLastError());
        return 0;
    }

    if (!ReadFile(hFile, &header, sizeof(header), &bytesRead, NULL) || bytesRead != sizeof(header)) {
        LOG("Header read failed (%d bytes)", bytesRead);
        CloseHandle(hFile);
        return 0;
    }

    if (header.magic != GHOST_MAGIC) {
        LOG("No magic, legacy format (magic=0x%X) — seeking back 8 bytes", header.magic);
        DWORD count = header.magic;
        DWORD time = header.version;
        header.frameCount = count;
        header.time = time;
        SetFilePointer(hFile, 8, NULL, FILE_BEGIN);
    } else {
        LOG("Magic OK: v%d time=%d frames=%d", header.version, header.time, header.frameCount);
    }

    if (header.frameCount == 0 || header.frameCount > 50000) {
        LOG("Invalid frame count: %d", header.frameCount);
        CloseHandle(hFile);
        return 0;
    }

    DWORD dataSize = header.frameCount * SNAP_SIZE;
    DWORD *snapshots = (DWORD*)malloc(dataSize);
    if (!snapshots) { CloseHandle(hFile); return 0; }

    if (!ReadFile(hFile, snapshots, dataSize, &bytesRead, NULL) || bytesRead != dataSize) {
        LOG("Data read failed (%d of %d)", bytesRead, dataSize);
        free(snapshots);
        CloseHandle(hFile);
        return 0;
    }

    CloseHandle(hFile);
    *outSnapshots = snapshots;
    *outCount = header.frameCount;
    *outFinishTime = header.time;
    LOG("Ghost loaded: %d frames, time=%d", header.frameCount, header.time);
    return 1;
}

/* ---- BTT construction ---- */

static DWORD create_btt_from_ghost(DWORD *snapshots, DWORD count, DWORD finishTime, const char *raceName) {
    DWORD bttAddr = game_operator_new(BTT_SIZE);
    if (!bttAddr) {
        LOG("operator_new failed for BTT");
        return 0;
    }
    LOG("BTT allocated at 0x%08X", bttAddr);

    call_btt_ctor(bttAddr);

    DWORD vtable = *(DWORD*)bttAddr;
    if (vtable != BTT_VTABLE_ADDR) {
        LOG("ERROR: BTT ctor failed — vtable=0x%X (expected 0x%X)", vtable, BTT_VTABLE_ADDR);
        game_operator_delete(bttAddr);
        return 0;
    }
    LOG("BTT ctor OK (vtable verified)");

    *(DWORD*)(bttAddr + BTT_BEST_TIME) = finishTime;
    if (raceName && raceName[0]) {
        char *bttName = (char*)(bttAddr + BTT_NAME);
        strncpy(bttName, raceName, 127);
        bttName[127] = '\0';
    }

    DWORD alist = bttAddr + 0x04;
    DWORD appended = 0;
    for (DWORD i = 0; i < count; i++) {
        DWORD *snap = (DWORD*)game_operator_new(SNAP_SIZE);
        if (!snap) {
            LOG("ERROR: operator_new failed for snapshot %d", i);
            continue;
        }
        memcpy(snap, (BYTE*)snapshots + i * SNAP_SIZE, SNAP_SIZE);
        call_alist_append(alist, (DWORD)snap);
        appended++;
    }
    LOG("Appended %d snapshots via AthenaList_Append", appended);

    *(DWORD*)(bttAddr + BTT_PLAYBACK_IDX) = 0;
    LOG("BTT created at 0x%08X: count=%d, time=%d, name='%s'",
        bttAddr, appended, finishTime, raceName ? raceName : "(none)");
    return bttAddr;
}

/* ---- Ghost ball creation ---- */

static DWORD create_ghost_ball(DWORD board) {
    DWORD ballAddr = game_operator_new(BALL_SIZE);
    if (!ballAddr) {
        LOG("operator_new failed for ghost ball");
        return 0;
    }
    LOG("Ghost ball allocated at 0x%08X, board=0x%08X", ballAddr, board);

    call_ball_ctor(ballAddr, board);

    DWORD ballVtable = *(DWORD*)ballAddr;
    if (!ballVtable || IsBadReadPtr((void*)ballVtable, 4)) {
        LOG("ERROR: Ball_ctor failed — vtable=0x%X", ballVtable);
        return 0;
    }
    LOG("Step 2: ball vtable=0x%08X", ballVtable);

    call_ball_vtable1(ballAddr);
    LOG("Step 3: vtable[1] returned OK");

    *(float*)(ballAddr + BALL_COLOR_R) = 1.0f;
    *(float*)(ballAddr + BALL_COLOR_G) = 1.0f;
    *(float*)(ballAddr + BALL_COLOR_B) = 0.35f;

    *(DWORD*)(ballAddr + BALL_PLAYER_ID) = 0xFFFFFFFF;
    *(float*)(ballAddr + BALL_GRAVITY) = 0.5f;
    *(float*)(ballAddr + BALL_GRAVITY_SCALE) = 0.1f;
    *(float*)(ballAddr + BALL_RADIUS) = 26.0f;
    *(float*)(ballAddr + BALL_MASS) = 0.8f;
    *(float*)(ballAddr + BALL_MAX_SPEED) = 1000.0f;
    *(BYTE*)(ballAddr + BALL_RESPAWN_FLAG) = 0;
    *(float*)(ballAddr + BALL_ALPHA) = 0.45f;
    *(float*)(ballAddr + BALL_POS_X) = 0.0f;
    *(float*)(ballAddr + BALL_POS_Y) = 0.0f;
    *(float*)(ballAddr + BALL_POS_Z) = 0.0f;

    *(DWORD*)(board + BOARD_GHOST_BALL) = ballAddr;
    LOG("Ghost ball created at 0x%08X in board 0x%08X", ballAddr, board);
    return ballAddr;
}

/* ---- Cleanup ---- */

static void cleanup_previous_ghost(DWORD app) {
    DWORD board = get_board(app);

    if (g_ghostBallCreated && board) {
        DWORD ghostBall = *(DWORD*)(board + BOARD_GHOST_BALL);
        if (ghostBall && !IsBadReadPtr((void*)ghostBall, 0x100)) {
            call_ball_dtor(ghostBall);
            *(DWORD*)(board + BOARD_GHOST_BALL) = 0;
            LOG("Old ghost ball destroyed at 0x%08X", ghostBall);
        }
        g_ghostBallCreated = FALSE;
    }

    if (g_loadedBTT && app) {
        if (*(DWORD*)(app + APP_BTT_PLAYBACK) == g_loadedBTT) {
            *(DWORD*)(app + APP_BTT_PLAYBACK) = 0;
            LOG("Cleared App+0x910 (playback) — was BTT 0x%08X", g_loadedBTT);
        }
    }

    if (g_loadedBTT) {
        if (!IsBadReadPtr((void*)g_loadedBTT, 0x100)) {
            DWORD vt = *(DWORD*)g_loadedBTT;
            if (vt == BTT_VTABLE_ADDR) {
                call_btt_dtor(g_loadedBTT);
                LOG("Old BTT destroyed via game dtor at 0x%08X", g_loadedBTT);
            } else {
                LOG("Old BTT vtable mismatch (0x%X) — CRT _free", vt);
                game_operator_delete(g_loadedBTT);
            }
        }
        g_loadedBTT = 0;
    }

    g_ghostActive = FALSE;
    g_ghostFromEvent = FALSE;
    g_needManualAdvance = FALSE;
}

/* ---- Helpers ---- */

static void set_log_path(void) {
#ifdef ENABLE_LOGGING
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&set_log_path, &hSelf);
    if (hSelf) {
        char path[MAX_PATH];
        if (GetModuleFileNameA(hSelf, path, MAX_PATH)) {
            char *p = strrchr(path, '\\');
            if (p) strcpy(p + 1, "ghost_event_log.txt");
            else strcpy(path, "ghost_event_log.txt");
            strncpy(g_logPath, path, MAX_PATH - 1);
            g_logPath[MAX_PATH - 1] = '\0';
        }
    }
#endif
}

/* ---- Mod class ---- */

class GhostEventMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;

public:
    const char* GetModName() override      { return "Ghost Event"; }
    const char* GetAuthorName() override   { return "RodentRacer"; }
    const char* GetContributors() override { return "Hamsterbot"; }
    int GetApiVersion() override           { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;
        set_log_path();
        LOGS("=== Ghost Event (HB+) loaded ===");

        /* Patch Level_UpdateAndRender to render ghost in any mode */
        if (api) {
            api->PatchMemory(0x40B7F5, "\x90\x90", 2); /* Time Trial JZ */
            api->PatchMemory(0x40B7FF, "\x90\x90", 2); /* Party JNZ */
            LOGS("Level_UpdateAndRender patched (TT/party checks NOP'd)");
        }
    }

    void onEventPlaneCollide(Ball* collidingBall, char* eventPlaneID) override {
        if (!eventPlaneID || !eventPlaneID[0]) return;
        if (_strnicmp(eventPlaneID, "E:GHOST", 7) != 0) return;

        if (g_ghostFromEvent && g_ghostActive && g_loadedBTT) return;

        LOG("onEventPlaneCollide: E:GHOST full='%s'", eventPlaneID);
        const char *p1 = strchr(eventPlaneID, '(');
        if (p1) {
            const char *p2 = strchr(p1, ')');
            if (p2 && (p2 - p1 - 1) > 0) {
                size_t len = p2 - p1 - 1;
                if (len > 250) len = 250;
                memcpy(g_pendingGhostFile, p1 + 1, len);
                g_pendingGhostFile[len] = '\0';
                LOG("Parsed filename='%s'", g_pendingGhostFile);
            }
        }
    }

    void onGameUpdate() override {
        if (!api) return;

        App* appObj = api->GetApp();
        DWORD app = (DWORD)appObj;
        if (!app || IsBadReadPtr((void*)app, 0x1000)) return;

        /* Process pending ghost file load */
        if (g_pendingGhostFile[0]) {
            char filename[256];
            strncpy(filename, g_pendingGhostFile, sizeof(filename) - 1);
            filename[sizeof(filename) - 1] = '\0';
            g_pendingGhostFile[0] = '\0';

            DWORD board = get_board(app);
            if (!board) {
                LOG("No valid board (get_board failed)");
                return;
            }

            if (g_ghostActive || g_loadedBTT) cleanup_previous_ghost(app);

            DWORD *snapshots = NULL;
            DWORD count = 0;
            DWORD finishTime = 0;
            if (!load_ghost_file(filename, &snapshots, &count, &finishTime)) {
                LOG("Failed to load ghost file: %s", filename);
                return;
            }

            DWORD newBTT = create_btt_from_ghost(snapshots, count, finishTime, filename);
            if (!newBTT) {
                free(snapshots);
                LOG("Failed to create BTT");
                return;
            }

            *(DWORD*)(app + APP_BTT_PLAYBACK) = newBTT;
            g_loadedBTT = newBTT;

            DWORD ghostBall = *(DWORD*)(board + BOARD_GHOST_BALL);
            if (!ghostBall || IsBadReadPtr((void*)ghostBall, 0x100)) {
                ghostBall = create_ghost_ball(board);
                if (ghostBall) g_ghostBallCreated = TRUE;
                else LOG("Failed to create ghost ball");
            } else {
                g_ghostBallCreated = FALSE;
                LOG("Ghost ball already exists, reusing: 0x%08X", ghostBall);
            }

            *(DWORD*)(newBTT + BTT_PLAYBACK_IDX) = 0;
            g_ghostActive = TRUE;
            g_ghostFromEvent = TRUE;

            DWORD partyMode = 0;
            if (!IsBadReadPtr((void*)(app + APP_PARTY_FLAG), 4))
                partyMode = *(DWORD*)(app + APP_PARTY_FLAG);
            g_needManualAdvance = (partyMode != 0);
            LOG("Ghost playback started: BTT=0x%08X, ball=0x%08X, manualAdvance=%d (partyMode=%d)",
                newBTT, ghostBall, g_needManualAdvance, partyMode);
        }

        /* Active ghost maintenance */
        if (g_ghostActive && g_loadedBTT) {
            DWORD board = get_board(app);
            if (!board) {
                LOG("Board lost — cleaning up ghost resources");
                cleanup_previous_ghost(app);
                return;
            }

            DWORD ghostBall = *(DWORD*)(board + BOARD_GHOST_BALL);
            if (!ghostBall || IsBadReadPtr((void*)ghostBall, 0x100)) {
                LOG("Ghost ball lost — cleaning up ghost resources");
                cleanup_previous_ghost(app);
                return;
            }

            if (g_needManualAdvance) {
                DWORD playIdx = *(DWORD*)(g_loadedBTT + BTT_PLAYBACK_IDX);
                DWORD count   = *(DWORD*)(g_loadedBTT + BTT_ALIST_COUNT);
                if (count > 0 && playIdx < count - 1) {
                    *(DWORD*)(g_loadedBTT + BTT_PLAYBACK_IDX) = playIdx + 1;
                }
            }

            if (g_ghostFromEvent) {
                DWORD playIdx = *(DWORD*)(g_loadedBTT + BTT_PLAYBACK_IDX);
                DWORD count = *(DWORD*)(g_loadedBTT + BTT_ALIST_COUNT);
                if (count > 0 && playIdx >= count - 1) {
                    LOG("Ghost playback finished (idx=%d/%d) — cleaning up for re-trigger",
                        playIdx, count);
                    cleanup_previous_ghost(app);
                }
            }
        }
    }

    void onSceneEnd() override {
        if (api) {
            App* appObj = api->GetApp();
            if (appObj) cleanup_previous_ghost((DWORD)appObj);
        }
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new GhostEventMod();
}
