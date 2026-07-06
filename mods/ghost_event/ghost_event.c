/*
 * ghost_event.c — E:GHOST custom event mod
 *
 * Creates a custom collision event "E:GHOST(filename)" that loads a .ghost
 * file (binary format compatible with ghost_saver v22) and plays it back
 * as a ghost ball, regardless of game mode (Time Trial or Tournament).
 *
 * HOW IT WORKS:
 * 1. DispatchCollisionEvents hook intercepts E:GHOST events
 * 2. Level_UpdateAndRender patch removes Time Trial + party mode checks
 *    so the game's own ghost playback rendering code runs in any mode
 * 3. App_FrameUpdate epilogue hook:
 *    - On first E:GHOST trigger: creates ghost ball (replicating Board_ctor's
 *      ghost ball init), loads .ghost file into a BestTimeTracker, injects
 *      into App+0x910
 *    - Every frame: increments playback index (advances ghost one frame)
 *
 * Ghost file format (binary, ghost_saver v22 compatible):
 *   [4 bytes] frameCount (int32)
 *   [4 bytes] finishTime (int32, milliseconds)
 *   [frameCount × 0x28 bytes] BallSnapshot entries
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll ghost_event.c -I../shared \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

/* BASS type definitions */
typedef unsigned long long QWORD;
typedef DWORD HSTREAM;
typedef DWORD HMUSIC;
typedef DWORD HFX;

/* ---- Game addresses ---- */
#define EXE_BASE        0x00400000
#define APP_PTR         0x005341E0
#define DISPATCH_COLLISION_EVENTS 0x0040C5D0
#define LEVEL_UPDATE_AND_RENDER   0x0040B600
#define APP_FRAME_UPDATE_EPILOGUE 0x0046C1F1
#define BALL_CTOR        0x004039E0
#define BALL_SET_TRAJECTORY 0x00403850
#define OPERATOR_NEW     0x004BA57B
#define OPERATOR_DELETE  0x004BA74D  /* CRT _free — pairs with operator_new's _malloc */
#define BTT_VTABLE_ADDR  0x004D262C
#define ATHENALIST_VT    0x004D875C
#define ADDR_BTT_CTOR    0x00427660
#define ADDR_BTT_DTOR    0x004278C0  /* vtable[0] — deleting destructor: __thiscall(this, flags), RET 0x4 */
#define ADDR_ALIST_APPEND 0x00453780

/* ---- Struct sizes ---- */
#define BTT_SIZE        0x528
#define SNAP_SIZE       0x28
#define BALL_SIZE       0xC60
#define NO_TIME         9999999

/* ---- App offsets ---- */
#define APP_90C         0x90C  /* playback active flag — game checks this to call PlaybackSnapshot */
#define APP_910         0x910
#define APP_5DC         0x5DC
#define APP_5D6         0x5D6
#define APP_234         0x234
#define APP_220         0x220  /* PlayerProfile* */

/* ---- PlayerProfile offsets ---- */
#define PROFILE_BOARD   0x0C   /* Board* (verified: Board_ctor reads *(param_1+0x220)+0xC) */

/* ---- Board/Scene offsets ---- */
#define BOARD_APP       0x878  /* Board+0x878 = App* (set in Board_ctor) */
#define BOARD_GHOST_BALL 0x361C /* Board+0x361C = ghost Ball* */

/* ---- BTT offsets ---- */
#define BTT_VTABLE      0x00
#define BTT_AL_VTABLE   0x04
#define BTT_AL_COUNT    0x08
#define BTT_LIST_ARRAY  0x410
#define BTT_PLAYBACK_IDX 0x41C
#define BTT_RACE_TIME   0x420
#define BTT_RACE_NAME   0x424
#define BTT_BEST_TIME   0x524
#define BTT_NAME        0x424   /* same as BTT_RACE_NAME — char[128] race name */

/* ---- Ball offsets ---- */
#define BALL_VTABLE     0x00
#define BALL_BOARD      0x14
#define BALL_PLAYER_ID  0x18
#define BALL_POS_X      0x164
#define BALL_POS_Y      0x168
#define BALL_POS_Z      0x16C
#define BALL_ALPHA      0x2FC
#define BALL_GRAVITY    0x278
#define BALL_GRAVITY_SCALE 0x27C  /* Board_ctor sets 0.1 (0x3DCCCCCD) */
#define BALL_MAXSPEED   0x188
#define BALL_RADIUS     0x284
#define BALL_MASS       0x1A0  /* Board_ctor sets 0.8 (0x3F4CCCCD) */
#define BALL_RESPAWN_FLAG 0x281  /* Board_ctor sets 0 */

/* ---- Ghost file magic ---- */
#define GHOST_MAGIC     0x47485347  /* "GHSG" — matches ghost_saver v22+ */
#define GHOST_VERSION   1

/* ---- Logging ---- */
#define ENABLE_LOGGING

#ifdef ENABLE_LOGGING
static char g_logPath[MAX_PATH] = "";
static void diag_log(const char *msg) {
    if (!g_logPath[0]) return;
    HANDLE h = CreateFileA(g_logPath, FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD w; SetFilePointer(h, 0, NULL, FILE_END);
        WriteFile(h, msg, strlen(msg), &w, NULL);
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

/* ---- BASS proxy ---- */
static HMODULE g_hRealBass = NULL;

static BOOL (__stdcall *real_BASS_Init)(int, int, DWORD, HWND, const void*);
static void (__stdcall *real_BASS_Free)(void);
static HSTREAM (__stdcall *real_BASS_StreamCreateFile)(BOOL, const void*, QWORD, QWORD, DWORD);
static BOOL (__stdcall *real_BASS_StreamFree)(HSTREAM);
static HMUSIC (__stdcall *real_BASS_MusicLoad)(BOOL, const void*, DWORD, DWORD, DWORD, DWORD);
static BOOL (__stdcall *real_BASS_MusicFree)(HMUSIC);
static BOOL (__stdcall *real_BASS_ChannelSetAttributes)(DWORD, int, float, int);
static BOOL (__stdcall *real_BASS_MusicPlayEx)(DWORD, DWORD, int, DWORD);
static DWORD (__stdcall *real_BASS_ChannelGetData)(DWORD, void*, DWORD);
static HFX (__stdcall *real_BASS_ChannelSetFX)(DWORD, DWORD, int);
static void (__stdcall *real_BASS_Start)(void);
static void (__stdcall *real_BASS_Stop)(void);
static int  (__stdcall *real_BASS_SetConfig)(DWORD, DWORD);
static int  (__stdcall *real_BASS_ErrorGetCode)(void);
static int  (__stdcall *real_BASS_ChannelStop)(DWORD);

BOOL __stdcall BASS_Init(int device, int freq, DWORD flags, HWND win, const void *dsguid) {
    if (real_BASS_Init) return real_BASS_Init(device, freq, flags, win, dsguid);
    return TRUE;
}
void __stdcall BASS_Free(void) {
    if (real_BASS_Free) real_BASS_Free();
}
HSTREAM __stdcall BASS_StreamCreateFile(BOOL mem, const void *file, QWORD offset, QWORD length, DWORD flags) {
    if (real_BASS_StreamCreateFile) return real_BASS_StreamCreateFile(mem, file, offset, length, flags);
    return 0;
}
BOOL __stdcall BASS_StreamFree(HSTREAM handle) {
    if (real_BASS_StreamFree) return real_BASS_StreamFree(handle);
    return FALSE;
}
HMUSIC __stdcall BASS_MusicLoad(BOOL mem, const void *file, DWORD offset, DWORD length, DWORD flags, DWORD freq) {
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(mem, file, offset, length, flags, freq);
    return (HMUSIC)1;
}
BOOL __stdcall BASS_MusicFree(HMUSIC handle) {
    if (real_BASS_MusicFree) return real_BASS_MusicFree(handle);
    return FALSE;
}
BOOL __stdcall BASS_ChannelSetAttributes(DWORD handle, int freq, float volume, int pan) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(handle, freq, volume, pan);
    return FALSE;
}
BOOL __stdcall BASS_MusicPlayEx(DWORD handle, DWORD flags, DWORD freq, BOOL ramp) {
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(handle, flags, (int)freq, ramp);
    return TRUE;
}
DWORD __stdcall BASS_ChannelGetData(DWORD handle, void *buffer, DWORD length) {
    if (real_BASS_ChannelGetData) return real_BASS_ChannelGetData(handle, buffer, length);
    return 0;
}
HFX __stdcall BASS_ChannelSetFX(DWORD handle, DWORD type, int priority) {
    if (real_BASS_ChannelSetFX) return real_BASS_ChannelSetFX(handle, type, priority);
    return 0;
}
void __stdcall BASS_Start(void) {
    if (real_BASS_Start) real_BASS_Start();
}
void __stdcall BASS_Stop(void) {
    if (real_BASS_Stop) real_BASS_Stop();
}
int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}
int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}
int __stdcall BASS_ChannelStop(DWORD handle) {
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(handle);
    return 1;
}

static void load_real_bass(void) {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char *p = strrchr(path, '\\');
    if (p) strcpy(p + 1, "bass_real.dll");
    else strcpy(path, "bass_real.dll");
    g_hRealBass = LoadLibraryA(path);
    if (!g_hRealBass) return;
    real_BASS_Init            = (void*)GetProcAddress(g_hRealBass, "BASS_Init");
    real_BASS_Free           = (void*)GetProcAddress(g_hRealBass, "BASS_Free");
    real_BASS_StreamCreateFile = (void*)GetProcAddress(g_hRealBass, "BASS_StreamCreateFile");
    real_BASS_StreamFree     = (void*)GetProcAddress(g_hRealBass, "BASS_StreamFree");
    real_BASS_MusicLoad       = (void*)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
    real_BASS_MusicFree       = (void*)GetProcAddress(g_hRealBass, "BASS_MusicFree");
    real_BASS_ChannelSetAttributes = (void*)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
    real_BASS_MusicPlayEx      = (void*)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
    real_BASS_ChannelGetData  = (void*)GetProcAddress(g_hRealBass, "BASS_ChannelGetData");
    real_BASS_ChannelSetFX    = (void*)GetProcAddress(g_hRealBass, "BASS_ChannelSetFX");
    real_BASS_Start           = (void*)GetProcAddress(g_hRealBass, "BASS_Start");
    real_BASS_Stop            = (void*)GetProcAddress(g_hRealBass, "BASS_Stop");
    real_BASS_SetConfig       = (void*)GetProcAddress(g_hRealBass, "BASS_SetConfig");
    real_BASS_ErrorGetCode    = (void*)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
    real_BASS_ChannelStop     = (void*)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
    if (!real_BASS_ChannelSetAttributes) {
        real_BASS_ChannelSetAttributes = (void*)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttribute");
    }
}

/* ---- Utility ---- */
static void patch_bytes(void *addr, const void *data, DWORD size) {
    DWORD old;
    VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &old);
    memcpy(addr, data, size);
    VirtualProtect(addr, size, old, &old);
}
static void patch_byte(void *addr, BYTE val) {
    patch_bytes(addr, &val, 1);
}
static void *alloc_executable(DWORD size) {
    return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}

/* ---- Inline asm wrappers for __thiscall game functions ----
 * MinGW __thiscall function pointers silently fail — must use asm.
 * (Proven pattern from ghost_saver v3+)
 */

/* Call BTT constructor: __thiscall(btt), ECX=btt, RET 0 */
static void call_btt_ctor(void *btt) {
    DWORD ctorAddr = ADDR_BTT_CTOR;
    __asm__ volatile(
        "movl %0, %%ecx\n\t"
        "call *%1\n\t"
        : : "r"(btt), "r"(ctorAddr)
        : "eax", "ecx", "edx", "memory"
    );
}

/* Call BTT deleting destructor: __thiscall(btt, flags=1), RET 0x4.
 * flags=1 calls internal dtor (frees snapshots+list) then operator delete
 * (frees the BTT struct itself) — full destroy matching operator_new. */
static void call_btt_dtor(void *btt) {
    DWORD dtorAddr = ADDR_BTT_DTOR;
    __asm__ volatile(
        "push $1\n\t"
        "movl %0, %%ecx\n\t"
        "call *%1\n\t"
        : : "r"(btt), "r"(dtorAddr)
        : "eax", "ecx", "edx", "memory"
    );
}

/* Call AthenaList_Append: __thiscall(list, item), RET 0x4.
 * Callee cleans up the 4-byte stack param itself — do NOT add $4 to ESP. */
static void call_alist_append(DWORD *list, void *item) {
    DWORD appendAddr = ADDR_ALIST_APPEND;
    __asm__ volatile(
        "push %1\n\t"
        "movl %0, %%ecx\n\t"
        "call *%2\n\t"
        : : "r"(list), "r"(item), "r"(appendAddr)
        : "eax", "ecx", "edx", "memory"
    );
}

/* ---- Utility: get board pointer ----
 * Verified chain: App(0x5341E0) -> +0x220 (PlayerProfile*) -> +0xC (Board*)
 * This is the same chain used by ghost_saver and confirmed in Board_ctor.
 * Board+0x878 = App* (back-pointer, set in Board_ctor).
 * App+0x178 is SoundDevice, NOT the scene — do not use it.
 */
static DWORD get_board(DWORD app) {
    if (!app || IsBadReadPtr((void*)app, 0x1000)) return 0;
    DWORD profile = *(DWORD*)(app + APP_220);
    if (!profile || IsBadReadPtr((void*)profile, 0x100)) return 0;
    DWORD board = *(DWORD*)(profile + PROFILE_BOARD);
    if (!board || IsBadReadPtr((void*)board, 0x4000)) return 0;
    /* Sanity: Board+0x878 should point back to App */
    if (*(DWORD*)(board + BOARD_APP) != app) return 0;
    return board;
}

/* ---- State ---- */
static DWORD g_moduleBase = 0x00400000;
static DWORD g_loadedBTT = 0;
static BOOL  g_ghostActive = FALSE;
static char  g_pendingGhostFile[256] = "";
static BOOL  g_ghostBallCreated = FALSE;  /* TRUE if WE created the ghost ball (not the game) */
static BOOL  g_ghostFromEvent = FALSE;    /* TRUE if current ghost was loaded by an E:GHOST plane trigger.
                                           * While TRUE, subsequent E:GHOST collisions skip the reload
                                           * to avoid rebuilding 778 snapshots every frame.
                                           * Cleared when playback finishes (last frame reached) or
                                           * when the board is lost, so the ghost can be re-triggered. */

/* Ball vtable[0] = deleting destructor at 0x402A50
 * __thiscall(ball, flags), RET 0x4. flags=1: dtor + free struct */
#define BALL_DELETING_DTOR 0x402A50
#define OPERATOR_DELETE    0x004BA74D

/* ---- DCE handler (called from raw byte stub) ----
 * DCE is __thiscall: ECX=board, stack: [ball, collEntry], RET 0x8
 * Collision entry pair: pair[0]=type/board, pair[1]=MeshBuffer
 * Event name at: MeshBuffer + 0x864
 */
static int g_dceCallCount = 0;

void __cdecl dce_handler(DWORD board, DWORD ball, DWORD collEntry) {
    g_dceCallCount++;
    if (!collEntry) {
        if (g_dceCallCount <= 3) LOG("DCE[%d]: null collEntry (board=0x%X ball=0x%X)", g_dceCallCount, board, ball);
        return;
    }
    DWORD *pair = (DWORD*)collEntry;
    if (IsBadReadPtr(pair, 8)) {
        if (g_dceCallCount <= 3) LOG("DCE[%d]: bad pair ptr 0x%X", g_dceCallCount, collEntry);
        return;
    }
    if (!pair[1]) {
        if (g_dceCallCount <= 3) LOG("DCE[%d]: pair[1] null (pair[0]=0x%X)", g_dceCallCount, pair[0]);
        return;
    }
    if (IsBadReadPtr((void*)pair[1], 0x868)) {
        if (g_dceCallCount <= 3) LOG("DCE[%d]: bad meshbuf 0x%X", g_dceCallCount, pair[1]);
        return;
    }
    const char *eventName = NULL;
    {
        /* MeshBuffer+0x864 stores a char* pointer to the name string,
         * NOT an inline char array. Must dereference to get the actual string.
         * (Verified from working water mod + collision_hook tool.) */
        DWORD namePtr = *(DWORD*)((BYTE*)pair[1] + 0x864);
        if (!namePtr || IsBadReadPtr((void*)namePtr, 1)) {
            if (g_dceCallCount <= 3) LOG("DCE[%d]: bad name ptr (meshbuf=0x%X, val=0x%X)", g_dceCallCount, pair[1], namePtr);
            return;
        }
        eventName = (const char*)namePtr;
    }
    if (!eventName[0]) {
        if (g_dceCallCount <= 3) LOG("DCE[%d]: empty event name (meshbuf=0x%X)", g_dceCallCount, pair[1]);
        return;
    }

    /* Log first 20 events, then every 100th */
    if (g_dceCallCount <= 20 || (g_dceCallCount % 100) == 0) {
        LOG("DCE[%d]: event='%s' (board=0x%X ball=0x%X meshbuf=0x%X)", g_dceCallCount, eventName, board, ball, pair[1]);
    }

    if (_strnicmp(eventName, "E:GHOST", 7) == 0) {
        /* Guard: if a ghost loaded by an E:GHOST plane is still playing,
         * skip the reload to avoid rebuilding 778 snapshots every frame.
         * The flag is cleared when playback finishes or the board is lost,
         * so the ghost can be re-triggered by hitting the plane again. */
        if (g_ghostFromEvent && g_ghostActive && g_loadedBTT) {
            /* Still playing — skip reload */
            return;
        }
        LOG("DCE: E:GHOST match found! full='%s'", eventName);
        const char *p1 = strchr(eventName, '(');
        if (p1) {
            const char *p2 = strchr(p1, ')');
            if (p2 && (p2 - p1 - 1) > 0) {
                size_t len = p2 - p1 - 1;
                if (len > 250) len = 250;
                memcpy(g_pendingGhostFile, p1 + 1, len);
                g_pendingGhostFile[len] = '\0';
                LOG("DCE: E:GHOST parsed filename='%s'", g_pendingGhostFile);
            } else {
                LOG("DCE: E:GHOST missing ')' or empty parens");
            }
        } else {
            LOG("DCE: E:GHOST missing '('");
        }
    }
}

/* ---- Frame epilogue handler (called from raw byte stub) ---- */
void __cdecl frame_epilogue_handler(void);

/* ---- Ghost file loading ---- */
#pragma pack(push, 1)
typedef struct {
    DWORD magic;
    DWORD version;
    DWORD time;        /* game ticks (finishTime) — matches ghost_saver field order */
    DWORD frameCount;
} GhostFileHeader;
#pragma pack(pop)

static int load_ghost_file(const char *filename, DWORD **outSnapshots, DWORD *outCount, DWORD *outFinishTime) {
    char path[MAX_PATH];
    DWORD bytesRead;
    GhostFileHeader header;

    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&load_ghost_file, &hSelf);
    GetModuleFileNameA(hSelf, path, MAX_PATH);
    char *p = strrchr(path, '\\');
    if (p) {
        strcpy(p + 1, "Ghosts\\");
        strncat(path, filename, MAX_PATH - strlen(path) - 1);
        /* Append .ghost extension if not already present */
        size_t plen = strlen(path);
        if (plen < MAX_PATH - 7 && (plen < 6 || _stricmp(path + plen - 6, ".ghost") != 0)) {
            strcat(path, ".ghost");
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
        /* Legacy format: first 8 bytes = frameCount + finishTime (no magic/version).
         * We read 16 bytes (sizeof GhostFileHeader) but legacy data only has
         * 8 header bytes — the next 8 bytes we consumed are actually snapshot data.
         * Seek back 8 bytes so snapshot reading starts at the right offset. */
        LOG("No magic, legacy format (magic=0x%X) — seeking back 8 bytes", header.magic);
        DWORD count = header.magic;      /* first DWORD = frameCount */
        DWORD time = header.version;     /* second DWORD = finishTime */
        header.frameCount = count;
        header.time = time;
        /* Seek to offset 8 (after the 2 DWORD legacy header) */
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

/* ---- BTT construction ----
 * Uses the game's own BTT constructor (0x427660) + operator_new for heap
 * consistency. Each snapshot is individually allocated via operator_new and
 * appended via AthenaList_Append — this way the game's BTT destructor can
 * safely _free() each snapshot without heap corruption.
 * (Pattern proven in ghost_saver v18+.)
 */
static DWORD create_btt_from_ghost(DWORD *snapshots, DWORD count, DWORD finishTime, const char *raceName) {
    /* Allocate BTT via game's operator_new so the game can safely free it later */
    typedef void* (__cdecl *operator_new_t)(size_t);
    operator_new_t game_operator_new = (operator_new_t)OPERATOR_NEW;
    DWORD bttAddr = (DWORD)game_operator_new(BTT_SIZE);
    if (!bttAddr) {
        LOG("operator_new failed for BTT");
        return 0;
    }
    LOG("BTT allocated at 0x%08X (via game operator_new)", bttAddr);

    /* Call the REAL BTT constructor — sets up vtable, AthenaList, internals */
    call_btt_ctor((void*)bttAddr);

    /* Verify constructor succeeded (vtable must be 0x4D262C) */
    DWORD vtable = *(DWORD*)bttAddr;
    if (vtable != BTT_VTABLE_ADDR) {
        LOG("ERROR: BTT ctor failed — vtable=0x%X (expected 0x%X)", vtable, BTT_VTABLE_ADDR);
        /* Constructor failed — calling dtor would crash on uninitialized state.
         * Free via CRT _free (0x4BA74D) which pairs with operator_new's _malloc. */
        typedef void (__cdecl *game_free_t)(void*);
        game_free_t game_free = (game_free_t)OPERATOR_DELETE;
        game_free((void*)bttAddr);
        return 0;
    }
    LOG("BTT ctor OK (vtable verified)");

    /* Set best time and race name */
    *(DWORD*)(bttAddr + BTT_BEST_TIME) = finishTime;
    if (raceName && raceName[0]) {
        char *bttName = (char*)(bttAddr + BTT_NAME);
        strncpy(bttName, raceName, 127);
        bttName[127] = '\0';
    }

    /* Append each snapshot individually via operator_new + AthenaList_Append.
     * This ensures each snapshot is a separate heap allocation that the game's
     * BTT destructor can safely _free() individually. */
    DWORD *alist = (DWORD*)(bttAddr + 0x04);  /* embedded AthenaList at BTT+4 */
    DWORD appended = 0;
    for (DWORD i = 0; i < count; i++) {
        DWORD *snap = (DWORD*)game_operator_new(SNAP_SIZE);
        if (!snap) {
            LOG("ERROR: operator_new failed for snapshot %d", i);
            continue;
        }
        memcpy(snap, (BYTE*)snapshots + i * SNAP_SIZE, SNAP_SIZE);
        call_alist_append(alist, snap);
        appended++;
    }
    LOG("Appended %d snapshots via AthenaList_Append", appended);

    /* Reset playback index to 0 */
    *(DWORD*)(bttAddr + BTT_PLAYBACK_IDX) = 0;

    LOG("BTT created at 0x%08X: count=%d, time=%d, name='%s'",
        bttAddr, appended, finishTime, raceName ? raceName : "(none)");
    return bttAddr;
}

/* ---- Ghost ball creation ----
 * Replicates Board_ctor's ghost ball init (at 0x419636):
 * 1. operator_new(0xC60)
 * 2. Ball_ctor(ball, board) — __thiscall, ECX=ball, stack: board, RET 0x4
 *    Ball_ctor sets ball+0x14=board, ball+0x10=board+0x878(App)
 * 3. ball->vtable[1]() — Ball_SetupCollisionRender, __thiscall, ECX=ball
 * 4. Ball_SetTrajectory(ball, board+0x3F20, ...) — sets trajectory matrices
 * 5. Set ghost fields matching Board_ctor:
 *    playerID=-1, gravity=0.5, gravity_scale=0.1, radius=26.0,
 *    mass=0.8, maxspeed=1000.0, respawn_flag=0
 * 6. Store at board+0x361C
 */
static DWORD create_ghost_ball(DWORD board) {
    /* Step 1: operator_new(0xC60) */
    typedef void* (__cdecl *operator_new_t)(size_t);
    operator_new_t game_operator_new = (operator_new_t)OPERATOR_NEW;
    DWORD ballAddr = (DWORD)game_operator_new(BALL_SIZE);
    if (!ballAddr) {
        LOG("operator_new failed for ghost ball");
        return 0;
    }
    LOG("Ghost ball allocated at 0x%08X, board=0x%08X", ballAddr, board);

    /* Step 2: Ball_ctor(ball, board) — __thiscall with 1 stack param */
    LOG("Step 2: calling Ball_ctor(0x%08X, 0x%08X)...", ballAddr, board);
    DWORD ctorAddr = BALL_CTOR;
    __asm__ volatile (
        "push %1\n\t"
        "movl %0, %%ecx\n\t"
        "call *%2\n\t"
        :
        : "r"(ballAddr), "r"(board), "r"(ctorAddr)
        : "eax", "ecx", "edx", "memory"
    );
    LOG("Step 2: Ball_ctor returned OK");

    /* Verify Ball_ctor succeeded — vtable must be set.
     * Ball vtable is at 0x4CF3A0 (confirmed from Ball_Update decomp). */
    DWORD ballVtable = *(DWORD*)ballAddr;
    if (!ballVtable || IsBadReadPtr((void*)ballVtable, 4)) {
        LOG("ERROR: Ball_ctor failed — vtable=0x%X (bad ptr)", ballVtable);
        return 0;
    }
    LOG("Step 2: ball vtable=0x%08X (verified OK)", ballVtable);

    /* Step 3: ball->vtable[1]() — Ball_SetupCollisionRender */
    DWORD func1 = *(DWORD*)(ballVtable + 0x04);
    LOG("Step 3: calling vtable[1]=0x%08X on ball 0x%08X...", func1, ballAddr);
    __asm__ volatile (
        "movl %0, %%ecx\n\t"
        "call *%1\n\t"
        :
        : "r"(ballAddr), "r"(func1)
        : "eax", "ecx", "edx", "memory"
    );
    LOG("Step 3: vtable[1] returned OK");

    /* Step 4: Ball_SetTrajectory — SKIP the call, write fields directly.
     * Board_ctor creates a 4x4 scale matrix via Matrix_Scale4x4(1.0, 1.0, EBX, 0.35)
     * then calls Ball_SetTrajectory(ball, matrix[0..3], matrix[4]).
     * Ball_SetTrajectory just stores 5 DWORDs into ball+0x2AC..0x2BC.
     * We can write them directly without the matrix call. */
    *(float*)(ballAddr + 0x2AC) = 1.0f;    /* scale X (from Matrix_Scale4x4) */
    *(float*)(ballAddr + 0x2B0) = 1.0f;    /* scale Y */
    *(float*)(ballAddr + 0x2B4) = 0.35f;   /* scale Z */
    *(DWORD*)(ballAddr + 0x2B8) = 0;       /* unused */
    *(DWORD*)(ballAddr + 0x2BC) = 0;       /* unused */
    LOG("Step 4: trajectory fields written directly (skipped Ball_SetTrajectory call)");

    /* Step 5: Set ghost-specific fields (matching Board_ctor exactly) */
    *(DWORD*)(ballAddr + BALL_PLAYER_ID) = 0xFFFFFFFF;   /* -1 */
    *(float*)(ballAddr + BALL_GRAVITY) = 0.5f;           /* 0x3F000000 */
    *(float*)(ballAddr + BALL_GRAVITY_SCALE) = 0.1f;    /* 0x3DCCCCCD */
    *(float*)(ballAddr + BALL_RADIUS) = 26.0f;           /* 0x41D00000 */
    *(float*)(ballAddr + BALL_MASS) = 0.8f;              /* 0x3F4CCCCD */
    *(float*)(ballAddr + BALL_MAXSPEED) = 1000.0f;       /* 0x4479C000 */
    *(BYTE*)(ballAddr + BALL_RESPAWN_FLAG) = 0;
    *(float*)(ballAddr + BALL_ALPHA) = 0.45f;            /* set by Level_UpdateAndRender each frame */
    *(float*)(ballAddr + BALL_POS_X) = 0.0f;
    *(float*)(ballAddr + BALL_POS_Y) = 0.0f;
    *(float*)(ballAddr + BALL_POS_Z) = 0.0f;

    /* Step 6: Store at board+0x361C */
    *(DWORD*)(board + BOARD_GHOST_BALL) = ballAddr;

    LOG("Ghost ball created at 0x%08X in board 0x%08X", ballAddr, board);
    return ballAddr;
}

/* ---- Cleanup previous ghost resources ----
 * Destroys old ghost ball (if we created it) and destroys old BTT via the
 * game's own destructor (which safely frees all snapshots individually).
 * Called before loading a new ghost, when the board is lost, and on DLL detach.
 *
 * Ball vtable[0] = 0x402A50 = deleting destructor:
 *   __thiscall(ball, flags=1), RET 0x4
 *   Calls GameObject_dtor (frees child objects, timers, etc.) then frees the struct.
 *
 * BTT cleanup: call BTT vtable[0] (0x4278C0) deleting destructor with flags=1.
 * This iterates the AthenaList and _free()s each snapshot individually, then
 * frees the BTT struct itself — all via the game's own heap. Safe because we
 * allocated each snapshot individually via operator_new + AthenaList_Append.
 */
static void cleanup_previous_ghost(DWORD app) {
    DWORD board = get_board(app);

    /* 1. Destroy old ghost ball if we created it */
    if (g_ghostBallCreated) {
        if (board) {
            DWORD ghostBall = *(DWORD*)(board + BOARD_GHOST_BALL);
            if (ghostBall && !IsBadReadPtr((void*)ghostBall, 0x100)) {
                DWORD dtorAddr = BALL_DELETING_DTOR;
                DWORD flags = 1;
                __asm__ volatile (
                    "push %1\n\t"
                    "movl %0, %%ecx\n\t"
                    "call *%2\n\t"
                    : : "r"(ghostBall), "r"(flags), "r"(dtorAddr)
                    : "eax", "ecx", "edx", "memory"
                );
                *(DWORD*)(board + BOARD_GHOST_BALL) = 0;
                LOG("Old ghost ball destroyed at 0x%08X", ghostBall);
            }
        }
        g_ghostBallCreated = FALSE;
    }

    /* 2. Clear App+0x910 and App+0x90C if they point to our BTT */
    if (g_loadedBTT && app) {
        if (*(DWORD*)(app + APP_910) == g_loadedBTT) {
            *(DWORD*)(app + APP_910) = 0;
            *(DWORD*)(app + APP_90C) = 0;
            LOG("Cleared App+0x910+0x90C (was our BTT 0x%08X)", g_loadedBTT);
        }
    }

    /* 3. Destroy old BTT via game's deleting destructor (flags=1).
     * This safely frees all snapshots + list + BTT struct via the game's heap.
     * Safe because snapshots were individually allocated via operator_new. */
    if (g_loadedBTT) {
        if (!IsBadReadPtr((void*)g_loadedBTT, 0x100)) {
            DWORD vt = *(DWORD*)g_loadedBTT;
            if (vt == BTT_VTABLE_ADDR) {
                call_btt_dtor((void*)g_loadedBTT);
                LOG("Old BTT destroyed via game dtor at 0x%08X", g_loadedBTT);
            } else {
                LOG("Old BTT vtable mismatch (0x%X) — freeing via CRT _free", vt);
                typedef void (__cdecl *game_free_t)(void*);
                game_free_t game_free = (game_free_t)OPERATOR_DELETE;
                game_free((void*)g_loadedBTT);
            }
        }
        g_loadedBTT = 0;
    }

    g_ghostActive = FALSE;
    g_ghostFromEvent = FALSE;
}

/* ---- Frame epilogue handler ---- */
void __cdecl frame_epilogue_handler(void) {
    DWORD app = *(DWORD*)APP_PTR;
    if (!app || IsBadReadPtr((void*)app, 0x1000)) return;

    /* Check for pending ghost file load */
    if (g_pendingGhostFile[0]) {
        char filename[256];
        strncpy(filename, g_pendingGhostFile, sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
        g_pendingGhostFile[0] = '\0';

        LOG("Processing ghost file: %s", filename);

        DWORD board = get_board(app);
        if (!board) {
            LOG("No valid board (get_board failed)");
            return;
        }

        /* Cleanup previous ghost resources (destroy old ghost ball + free old BTT) */
        if (g_ghostActive || g_loadedBTT) {
            cleanup_previous_ghost(app);
        }

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

        /* Inject new BTT into App+0x910 (playback buffer) */
        *(DWORD*)(app + APP_910) = newBTT;
        g_loadedBTT = newBTT;

        /* Create ghost ball (old one was destroyed in cleanup if it existed) */
        DWORD ghostBall = *(DWORD*)(board + BOARD_GHOST_BALL);
        if (!ghostBall || IsBadReadPtr((void*)ghostBall, 0x100)) {
            ghostBall = create_ghost_ball(board);
            if (ghostBall) {
                g_ghostBallCreated = TRUE;
            } else {
                LOG("Failed to create ghost ball");
            }
        } else {
            /* Ghost ball already exists (game-created in Time Trial), reuse it */
            g_ghostBallCreated = FALSE;
            LOG("Ghost ball already exists (game-created), reusing: 0x%08X", ghostBall);
        }

        /* Reset playback index to 0 */
        *(DWORD*)(newBTT + BTT_PLAYBACK_IDX) = 0;

        g_ghostActive = TRUE;
        g_ghostFromEvent = TRUE;  /* Mark as event-loaded so re-collisions skip reload */
        /* Force App+0x90C so the game calls PlaybackSnapshot every frame.
         * In Tournament/Party modes the game never sets this flag, so without
         * forcing it the ghost ball renders but never moves. */
        *(DWORD*)(app + APP_90C) = 1;
        LOG("Ghost playback started: BTT=0x%08X, ball=0x%08X", newBTT, ghostBall);
    }

    /* Check if ghost is still active — clean up if board or ball is lost,
     * or if playback has finished (reached the last frame).
     * This handles level transitions and allows re-triggering after playback ends. */
    if (g_ghostActive && g_loadedBTT) {
        DWORD app2 = *(DWORD*)APP_PTR;
        if (app2 && !IsBadReadPtr((void*)app2, 0x1000)) {
            /* Force playback flag every frame — game may clear it in non-TT modes */
            *(DWORD*)(app2 + APP_90C) = 1;
            DWORD board = get_board(app2);
            if (!board) {
                /* Board is gone — level transition or race end.
                 * Must clear App+0x910 and destroy BTT before new level loads,
                 * otherwise the game's own cleanup will try to _free() our
                 * BTT snapshots and crash. */
                LOG("Board lost — cleaning up ghost resources");
                cleanup_previous_ghost(app2);
            } else {
                DWORD ghostBall = *(DWORD*)(board + BOARD_GHOST_BALL);
                if (!ghostBall || IsBadReadPtr((void*)ghostBall, 0x100)) {
                    /* Ghost ball was destroyed by the game but BTT remains.
                     * Clear BTT to prevent dangling playback. */
                    LOG("Ghost ball lost — cleaning up ghost resources");
                    cleanup_previous_ghost(app2);
                } else if (g_ghostFromEvent) {
                    /* Check if playback has finished: the game's PlaybackSnapshot
                     * advances the index every frame. When it reaches the count,
                     * the ghost is done playing. Clear the flag and clean up so
                     * the ghost can be re-triggered by hitting the plane again. */
                    DWORD playIdx = *(DWORD*)(g_loadedBTT + BTT_PLAYBACK_IDX);
                    DWORD count = *(DWORD*)(g_loadedBTT + BTT_AL_COUNT);
                    if (count > 0 && playIdx >= count) {
                        LOG("Ghost playback finished (idx=%d/%d) — cleaning up for re-trigger",
                            playIdx, count);
                        cleanup_previous_ghost(app2);
                    }
                }
            }
        } else {
            /* App itself is gone — shouldn't happen but be safe */
            LOG("App lost — deactivating ghost");
            g_ghostActive = FALSE;
            g_ghostFromEvent = FALSE;
        }
    }
}

/* ---- Raw byte stubs for hooks ---- */

/* DCE hook stub:
 * At entry: ECX=board, [ESP+4]=ball, [ESP+8]=collEntry, RET 0x8
 * pushad (32) + pushfd (4) = 36 bytes
 * [ESP+36] = return addr (from JMP)
 * [ESP+40] = ball
 * [ESP+44] = collEntry
 *
 * We push board(ECX), ball, collEntry, call handler, cleanup, restore, jmp trampoline
 */
static BYTE *g_dce_stub = NULL;
static BYTE g_dce_original[8];
static void *g_dce_trampoline = NULL;

static void build_dce_trampoline(void) {
    BYTE *code = (BYTE*)alloc_executable(16);
    memcpy(code, g_dce_original, 8);
    code[8] = 0xE9;
    *(DWORD*)(code + 9) = (DWORD)(DISPATCH_COLLISION_EVENTS + 8) - (DWORD)(code + 13);
    g_dce_trampoline = code;
}

static void build_dce_stub(void) {
    /* Layout:
     * At entry: ECX=board, [ESP+4]=ball, [ESP+8]=collEntry, RET 0x8
     * pushad (32) + pushfd (4) = 36 bytes on stack
     *   [ESP+36] = return addr (from JMP)
     *   [ESP+40] = ball
     *   [ESP+44] = collEntry
     *
     * dce_handler is __cdecl(board, ball, collEntry) — __cdecl expects
     * arguments pushed right-to-left: collEntry, ball, board.
     *
     * We push in reverse order: collEntry, ball, board(ECX).
     * After pushad(32)+pushfd(4) = 36 bytes:
     *   [ESP+36] = return addr, [ESP+40] = ball, [ESP+44] = collEntry
     */
    BYTE *code = (BYTE*)alloc_executable(64);
    int i = 0;
    code[i++] = 0x60;  /* pushad */
    code[i++] = 0x9C;  /* pushfd */
    /* push [esp+0x2C] (collEntry at offset 44=0x2C) — push FIRST (rightmost arg) */
    code[i++] = 0xFF; code[i++] = 0x74; code[i++] = 0x24; code[i++] = 0x2C;
    /* After pushing collEntry, ESP dropped by 4. ball was at 0x28(40), now at 0x2C */
    code[i++] = 0xFF; code[i++] = 0x74; code[i++] = 0x24; code[i++] = 0x2C;
    /* push ecx (board — still in ECX at entry) — push LAST (leftmost arg) */
    code[i++] = 0x51;
    /* call dce_handler (relative) */
    code[i++] = 0xE8;
    *(DWORD*)(code + i) = (DWORD)&dce_handler - (DWORD)(code + i + 4);
    i += 4;
    /* add esp, 12 (cleanup 3 pushed params) */
    code[i++] = 0x83; code[i++] = 0xC4; code[i++] = 0x0C;
    /* popfd */
    code[i++] = 0x9D;
    /* popad */
    code[i++] = 0x61;
    /* jmp to trampoline: mov eax, addr; jmp eax */
    code[i++] = 0xB8;
    *(DWORD*)(code + i) = (DWORD)g_dce_trampoline;
    i += 4;
    code[i++] = 0xFF; code[i++] = 0xE0;

    g_dce_stub = code;
}

/* Frame epilogue stub:
 * Replaces 5 bytes at 0x46C1F1 (POP ESI / ADD ESP,8 / RET)
 * Saves state, calls handler, restores, executes original 5 bytes, RET
 */
static BYTE *g_frame_stub = NULL;
static BYTE g_frame_original[5];

static void build_frame_stub(void) {
    BYTE *code = (BYTE*)alloc_executable(64);
    int i = 0;
    code[i++] = 0x60;  /* pushad */
    code[i++] = 0x9C;  /* pushfd */
    /* call frame_epilogue_handler (relative) */
    code[i++] = 0xE8;
    *(DWORD*)(code + i) = (DWORD)&frame_epilogue_handler - (DWORD)(code + i + 4);
    i += 4;
    /* popfd */
    code[i++] = 0x9D;
    /* popad */
    code[i++] = 0x61;
    /* original 5 bytes: POP ESI / ADD ESP,8 / RET */
    code[i++] = 0x5E;  /* pop esi */
    code[i++] = 0x83; code[i++] = 0xC4; code[i++] = 0x08;  /* add esp, 8 */
    code[i++] = 0xC3;  /* ret */

    g_frame_stub = code;
}

/* ---- Level_UpdateAndRender patch ----
 * NOP out the Time Trial check (JZ at 0x40B7F5) and party mode check (JNZ at 0x40B7FF)
 * so ghost playback rendering runs in any mode.
 */
static void patch_level_update_and_render(void) {
    BYTE *jz_addr = (BYTE*)(g_moduleBase + 0x000B7F5);
    patch_byte(jz_addr,     0x90);
    patch_byte(jz_addr + 1, 0x90);

    BYTE *jnz_addr = (BYTE*)(g_moduleBase + 0x000B7FF);
    patch_byte(jnz_addr,     0x90);
    patch_byte(jnz_addr + 1, 0x90);

    LOGS("Level_UpdateAndRender patched");
}

/* ---- Install hooks ---- */
static void install_dce_hook(void) {
    memcpy(g_dce_original, (void*)DISPATCH_COLLISION_EVENTS, 8);
    build_dce_trampoline();
    build_dce_stub();

    /* Install JMP+NOP to our stub */
    BYTE patch[8];
    patch[0] = 0xE9;
    *(DWORD*)(patch + 1) = (DWORD)g_dce_stub - DISPATCH_COLLISION_EVENTS - 5;
    patch[5] = 0x90;
    patch[6] = 0x90;
    patch[7] = 0x90;
    patch_bytes((void*)DISPATCH_COLLISION_EVENTS, patch, 8);
    LOGS("DCE hook installed");
}

static void install_frame_hook(void) {
    memcpy(g_frame_original, (void*)APP_FRAME_UPDATE_EPILOGUE, 5);
    build_frame_stub();

    BYTE patch[5];
    patch[0] = 0xE9;
    *(DWORD*)(patch + 1) = (DWORD)g_frame_stub - APP_FRAME_UPDATE_EPILOGUE - 5;
    patch_bytes((void*)APP_FRAME_UPDATE_EPILOGUE, patch, 5);
    LOGS("Frame epilogue hook installed");
}

/* ---- Init thread ---- */
static DWORD WINAPI init_thread(LPVOID param) {
    (void)param;
    Sleep(2000);
    HMODULE hExe = GetModuleHandleA(NULL);
    g_moduleBase = (DWORD)hExe;
    LOG("init_thread: moduleBase=0x%X", g_moduleBase);

    patch_level_update_and_render();
    install_dce_hook();
    install_frame_hook();

    LOGS("ghost_event mod initialized — waiting for E:GHOST events");
    return 0;
}

/* ---- DllMain ---- */
BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved) {
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInst);

#ifdef ENABLE_LOGGING
        char modPath[MAX_PATH];
        GetModuleFileNameA(hInst, modPath, MAX_PATH);
        char *p = strrchr(modPath, '\\');
        if (p) {
            strcpy(p + 1, "ghost_event_log.txt");
            strncpy(g_logPath, modPath, sizeof(g_logPath) - 1);
            g_logPath[sizeof(g_logPath) - 1] = '\0';
        }
#endif

        load_real_bass();
        CreateThread(NULL, 0, init_thread, NULL, 0, NULL);
    }
    else if (reason == DLL_PROCESS_DETACH) {
        /* Clean up our BTT before the game's shutdown destroys it.
         * Since snapshots are now individually allocated via operator_new
         * and appended via AthenaList_Append, we can safely call the game's
         * BTT destructor (which iterates and frees each snapshot). */
        if (g_loadedBTT) {
            DWORD app = *(DWORD*)APP_PTR;
            if (app && !IsBadReadPtr((void*)app, 0x1000)) {
                DWORD curr910 = *(DWORD*)(app + APP_910);
                if (curr910 == g_loadedBTT) {
                    *(DWORD*)(app + APP_910) = 0;
                    *(DWORD*)(app + APP_90C) = 0;
                    LOG("DLL_PROCESS_DETACH: cleared App+0x910+0x90C (was our BTT 0x%08X)", g_loadedBTT);
                }
            }
            /* Destroy BTT via game destructor — safe because snapshots are
             * individually allocated via operator_new. Vtable check prevents
             * crash on partially-initialized BTT. */
            if (!IsBadReadPtr((void*)g_loadedBTT, 0x100)) {
                DWORD vt = *(DWORD*)g_loadedBTT;
                if (vt == BTT_VTABLE_ADDR) {
                    call_btt_dtor((void*)g_loadedBTT);
                    LOG("DLL_PROCESS_DETACH: BTT destroyed via game dtor at 0x%08X", g_loadedBTT);
                } else {
                    LOG("DLL_PROCESS_DETACH: BTT vtable mismatch (0x%X) — CRT _free", vt);
                    typedef void (__cdecl *game_free_t)(void*);
                    game_free_t game_free = (game_free_t)OPERATOR_DELETE;
                    game_free((void*)g_loadedBTT);
                }
            }
            g_loadedBTT = 0;
        }
    }
    return TRUE;
}
