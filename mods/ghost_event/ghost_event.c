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
#define OPERATOR_NEW     0x004BA57B
#define BTT_VTABLE_ADDR  0x004D262C
#define ATHENALIST_VT    0x004D875C

/* ---- Struct sizes ---- */
#define BTT_SIZE        0x528
#define SNAP_SIZE       0x28
#define BALL_SIZE       0xC60
#define NO_TIME         9999999

/* ---- App offsets ---- */
#define APP_90C         0x90C
#define APP_910         0x910
#define APP_5DC         0x5DC
#define APP_5D6         0x5D6
#define APP_234         0x234
#define APP_220         0x220
#define APP_178         0x178

/* ---- Scene offsets ---- */
#define SCENE_GHOST_BALL 0x361C

/* ---- BTT offsets ---- */
#define BTT_VTABLE      0x00
#define BTT_AL_VTABLE   0x04
#define BTT_AL_COUNT    0x08
#define BTT_LIST_ARRAY  0x410
#define BTT_PLAYBACK_IDX 0x41C
#define BTT_RACE_TIME   0x420
#define BTT_RACE_NAME   0x424
#define BTT_BEST_TIME   0x524

/* ---- Ball offsets ---- */
#define BALL_VTABLE     0x00
#define BALL_BOARD      0x14
#define BALL_PLAYER_ID  0x18
#define BALL_POS_X      0x164
#define BALL_POS_Y      0x168
#define BALL_POS_Z      0x16C
#define BALL_ALPHA      0x2FC
#define BALL_GRAVITY    0x278
#define BALL_MAXSPEED   0x188
#define BALL_RADIUS     0x284

/* ---- Ghost file magic ---- */
#define GHOST_MAGIC     0x47484F53  /* "GHOS" */
#define GHOST_VERSION   1

/* ---- Logging ---- */
/* #define ENABLE_LOGGING */

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

static void (__stdcall *real_BASS_Init)(int, int, DWORD, HWND, const void*);
static void (__stdcall *real_BASS_Free)(void);
static HSTREAM (__stdcall *real_BASS_StreamCreateFile)(BOOL, const void*, QWORD, QWORD, DWORD);
static BOOL (__stdcall *real_BASS_StreamFree)(HSTREAM);
static HMUSIC (__stdcall *real_BASS_MusicLoad)(BOOL, const void*, QWORD, DWORD, DWORD, DWORD);
static BOOL (__stdcall *real_BASS_MusicFree)(HMUSIC);
static BOOL (__stdcall *real_BASS_ChannelSetAttributes)(DWORD, int, float, int);
static BOOL (__stdcall *real_BASS_ChannelPlay)(DWORD, BOOL);
static DWORD (__stdcall *real_BASS_ChannelGetData)(DWORD, void*, DWORD);
static HFX (__stdcall *real_BASS_ChannelSetFX)(DWORD, DWORD, int);

void __stdcall BASS_Init(int device, int freq, DWORD flags, HWND win, const void *dsguid) {
    if (real_BASS_Init) real_BASS_Init(device, freq, flags, win, dsguid);
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
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(mem, file, (QWORD)offset, length, flags, freq);
    return 0;
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
    if (real_BASS_ChannelPlay) return real_BASS_ChannelPlay(handle, ramp);
    return FALSE;
}
DWORD __stdcall BASS_ChannelGetData(DWORD handle, void *buffer, DWORD length) {
    if (real_BASS_ChannelGetData) return real_BASS_ChannelGetData(handle, buffer, length);
    return 0;
}
HFX __stdcall BASS_ChannelSetFX(DWORD handle, DWORD type, int priority) {
    if (real_BASS_ChannelSetFX) return real_BASS_ChannelSetFX(handle, type, priority);
    return 0;
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
    real_BASS_ChannelPlay     = (void*)GetProcAddress(g_hRealBass, "BASS_ChannelPlay");
    real_BASS_ChannelGetData  = (void*)GetProcAddress(g_hRealBass, "BASS_ChannelGetData");
    real_BASS_ChannelSetFX    = (void*)GetProcAddress(g_hRealBass, "BASS_ChannelSetFX");
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

/* ---- State ---- */
static DWORD g_moduleBase = 0x00400000;
static DWORD g_loadedBTT = 0;
static DWORD g_oldPlaybackBTT = 0;
static BOOL  g_ghostActive = FALSE;
static char  g_pendingGhostFile[256] = "";
static BOOL  g_ghostBallCreated = FALSE;

/* ---- DCE handler (called from raw byte stub) ----
 * DCE is __thiscall: ECX=board, stack: [ball, collEntry], RET 0x8
 * Collision entry pair: pair[0]=type/board, pair[1]=MeshBuffer
 * Event name at: MeshBuffer + 0x864
 */
void __cdecl dce_handler(DWORD board, DWORD ball, DWORD collEntry) {
    if (!collEntry) return;
    DWORD *pair = (DWORD*)collEntry;
    if (IsBadReadPtr(pair, 8) || !pair[1]) return;
    if (IsBadReadPtr((void*)pair[1], 0x868)) return;
    const char *eventName = (const char*)(pair[1] + 0x864);
    if (IsBadReadPtr(eventName, 1) || !eventName[0]) return;

    if (_strnicmp(eventName, "E:GHOST", 7) == 0) {
        const char *p1 = strchr(eventName, '(');
        if (p1) {
            const char *p2 = strchr(p1, ')');
            if (p2 && (p2 - p1 - 1) > 0) {
                size_t len = p2 - p1 - 1;
                if (len > 250) len = 250;
                memcpy(g_pendingGhostFile, p1 + 1, len);
                g_pendingGhostFile[len] = '\0';
                LOG("E:GHOST detected: file=%s", g_pendingGhostFile);
            }
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
    DWORD frameCount;
    DWORD finishTime;
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
    } else {
        snprintf(path, MAX_PATH, "Ghosts\\%s", filename);
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
        /* Legacy format: first 8 bytes = frameCount + finishTime (no magic) */
        LOG("No magic, legacy format");
        DWORD count = header.magic;
        DWORD time = header.version;
        header.frameCount = count;
        header.finishTime = time;
    } else {
        LOG("Magic OK: v%d frames=%d time=%d", header.version, header.frameCount, header.finishTime);
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
    *outFinishTime = header.finishTime;
    LOG("Ghost loaded: %d frames", header.frameCount);
    return 1;
}

/* ---- BTT construction ---- */
static DWORD create_btt_from_ghost(DWORD *snapshots, DWORD count, DWORD finishTime) {
    /* Allocate BTT via operator_new (game heap) */
    typedef void* (__cdecl *operator_new_t)(size_t);
    operator_new_t game_operator_new = (operator_new_t)OPERATOR_NEW;
    DWORD bttAddr = (DWORD)game_operator_new(BTT_SIZE);
    if (!bttAddr) {
        bttAddr = (DWORD)malloc(BTT_SIZE);
        if (!bttAddr) return 0;
    }

    memset((void*)bttAddr, 0, BTT_SIZE);

    /* Set vtable */
    *(DWORD*)bttAddr = BTT_VTABLE_ADDR;

    /* Initialize embedded AthenaList */
    *(DWORD*)(bttAddr + BTT_AL_VTABLE) = ATHENALIST_VT;
    *(DWORD*)(bttAddr + BTT_AL_COUNT) = count;
    *(DWORD*)(bttAddr + BTT_PLAYBACK_IDX) = 0;
    *(DWORD*)(bttAddr + BTT_RACE_TIME) = finishTime;
    *(DWORD*)(bttAddr + BTT_BEST_TIME) = finishTime;

    /* Allocate list_array: array of DWORD pointers, one per snapshot */
    DWORD *listArray = (DWORD*)malloc(count * 4);
    if (!listArray) { free((void*)bttAddr); return 0; }

    for (DWORD i = 0; i < count; i++) {
        listArray[i] = (DWORD)((BYTE*)snapshots + i * SNAP_SIZE);
    }

    *(DWORD*)(bttAddr + BTT_LIST_ARRAY) = (DWORD)listArray;

    LOG("BTT created at 0x%08X: count=%d, array=0x%08X", bttAddr, count, listArray);
    return bttAddr;
}

/* ---- Ghost ball creation ----
 * Replicates Board_ctor's ghost ball init at 0x419636:
 * 1. operator_new(0xC60)
 * 2. Ball_ctor(ball, scene) — __thiscall, ECX=ball, stack: scene, RET 0x4
 * 3. ball->vtable[1]() — Ball_SetupCollisionRender, __thiscall, ECX=ball
 * 4. Set ghost fields: playerID=-1, gravity=0.5, radius=26.0, maxspeed=1000.0, alpha=0.45
 * 5. Store at scene+0x361C
 */
static DWORD create_ghost_ball(DWORD sceneAddr) {
    /* Step 1: operator_new(0xC60) */
    typedef void* (__cdecl *operator_new_t)(size_t);
    operator_new_t game_operator_new = (operator_new_t)OPERATOR_NEW;
    DWORD ballAddr = (DWORD)game_operator_new(BALL_SIZE);
    if (!ballAddr) {
        LOG("operator_new failed for ghost ball");
        return 0;
    }
    LOG("Ghost ball allocated at 0x%08X", ballAddr);

    /* Step 2: Ball_ctor(ball, scene) — __thiscall with 1 stack param
     * Must use inline asm because __fastcall typedefs put 2nd arg in EDX */
    DWORD ctorAddr = BALL_CTOR;
    __asm__ volatile (
        "push %1\n\t"
        "movl %0, %%ecx\n\t"
        "call *%2\n\t"
        :
        : "r"(ballAddr), "r"(sceneAddr), "r"(ctorAddr)
        : "eax", "ecx", "edx", "memory"
    );

    /* Step 3: ball->vtable[1]() — Ball_SetupCollisionRender */
    DWORD vtable = *(DWORD*)ballAddr;
    DWORD func1 = *(DWORD*)(vtable + 0x04);
    __asm__ volatile (
        "movl %0, %%ecx\n\t"
        "call *%1\n\t"
        :
        : "r"(ballAddr), "r"(func1)
        : "eax", "ecx", "edx", "memory"
    );

    /* Step 4: Set ghost-specific fields */
    *(DWORD*)(ballAddr + BALL_PLAYER_ID) = 0xFFFFFFFF;
    *(float*)(ballAddr + BALL_GRAVITY) = 0.5f;
    *(float*)(ballAddr + BALL_RADIUS) = 26.0f;
    *(float*)(ballAddr + BALL_MAXSPEED) = 1000.0f;
    *(float*)(ballAddr + BALL_ALPHA) = 0.45f;
    *(float*)(ballAddr + BALL_POS_X) = 0.0f;
    *(float*)(ballAddr + BALL_POS_Y) = 0.0f;
    *(float*)(ballAddr + BALL_POS_Z) = 0.0f;

    /* Step 5: Store at scene+0x361C */
    *(DWORD*)(sceneAddr + SCENE_GHOST_BALL) = ballAddr;

    LOG("Ghost ball created at 0x%08X in scene 0x%08X", ballAddr, sceneAddr);
    return ballAddr;
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

        DWORD scene = *(DWORD*)(app + APP_178);
        if (!scene || IsBadReadPtr((void*)scene, 0x4000)) {
            LOG("No valid scene");
            return;
        }

        DWORD *snapshots = NULL;
        DWORD count = 0;
        DWORD finishTime = 0;
        if (!load_ghost_file(filename, &snapshots, &count, &finishTime)) {
            LOG("Failed to load ghost file: %s", filename);
            return;
        }

        DWORD newBTT = create_btt_from_ghost(snapshots, count, finishTime);
        if (!newBTT) {
            free(snapshots);
            LOG("Failed to create BTT");
            return;
        }

        /* Save old App+0x910 for potential cleanup */
        g_oldPlaybackBTT = *(DWORD*)(app + APP_910);

        /* Inject new BTT */
        *(DWORD*)(app + APP_910) = newBTT;
        g_loadedBTT = newBTT;

        /* Create ghost ball if it doesn't exist */
        DWORD ghostBall = *(DWORD*)(scene + SCENE_GHOST_BALL);
        if (!ghostBall || IsBadReadPtr((void*)ghostBall, 0x100)) {
            ghostBall = create_ghost_ball(scene);
            if (ghostBall) {
                g_ghostBallCreated = TRUE;
            } else {
                LOG("Failed to create ghost ball");
            }
        } else {
            g_ghostBallCreated = FALSE;
            LOG("Ghost ball already exists at 0x%08X", ghostBall);
        }

        /* Reset playback index to 0 */
        *(DWORD*)(newBTT + BTT_PLAYBACK_IDX) = 0;

        g_ghostActive = TRUE;
        LOG("Ghost playback started: BTT=0x%08X, ball=0x%08X", newBTT, ghostBall);
    }

    /* Advance playback index every frame */
    if (g_ghostActive && g_loadedBTT) {
        DWORD app2 = *(DWORD*)APP_PTR;
        if (app2 && !IsBadReadPtr((void*)app2, 0x1000)) {
            DWORD scene = *(DWORD*)(app2 + APP_178);
            if (scene && !IsBadReadPtr((void*)scene, 0x4000)) {
                DWORD ghostBall = *(DWORD*)(scene + SCENE_GHOST_BALL);
                if (ghostBall && !IsBadReadPtr((void*)ghostBall, 0x100)) {
                    DWORD *playbackIdx = (DWORD*)(g_loadedBTT + BTT_PLAYBACK_IDX);
                    DWORD count = *(DWORD*)(g_loadedBTT + BTT_AL_COUNT);
                    (*playbackIdx)++;
                    if (count > 0 && *playbackIdx >= count) {
                        *playbackIdx = count - 1;
                    }
                } else {
                    LOG("Ghost ball lost, deactivating");
                    g_ghostActive = FALSE;
                }
            } else {
                LOG("Scene lost, deactivating");
                g_ghostActive = FALSE;
            }
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
     * 60           pushad
     * 9C          pushfd
     * 51           push ecx (board — still in ECX at entry)
     * FF 74 24 28  push [esp+0x28]  (ball: pushad=32, pushfd=4, push ecx=4 → retaddr at 40=0x28, ball at 44=0x2C)
     * Wait: after pushad(32)+pushfd(4)+push ecx(4) = 40 bytes on stack
     *   [ESP+0]  = ecx (board)
     *   [ESP+4]  = eflags
     *   [ESP+8..36] = pushad regs (EDI..EAX)
     *   [ESP+40] = return addr
     *   [ESP+44] = ball
     *   [ESP+48] = collEntry
     * So ball is at [ESP+0x2C] (44), collEntry at [ESP+0x30] (48)
     */
    BYTE *code = (BYTE*)alloc_executable(64);
    int i = 0;
    code[i++] = 0x60;  /* pushad */
    code[i++] = 0x9C;  /* pushfd */
    code[i++] = 0x51;  /* push ecx (board) */
    /* push [esp+0x2C] (ball) */
    code[i++] = 0xFF; code[i++] = 0x74; code[i++] = 0x24; code[i++] = 0x2C;
    /* push [esp+0x30] (collEntry) */
    code[i++] = 0xFF; code[i++] = 0x74; code[i++] = 0x24; code[i++] = 0x30;
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
    Sleep(2000);
    HMODULE hExe = GetModuleHandleA(NULL);
    g_moduleBase = (DWORD)hExe;

    patch_level_update_and_render();
    install_dce_hook();
    install_frame_hook();

    LOGS("ghost_event mod initialized");
    return 0;
}

/* ---- DllMain ---- */
BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInst);
        load_real_bass();

#ifdef ENABLE_LOGGING
        char modPath[MAX_PATH];
        GetModuleFileNameA(hInst, modPath, MAX_PATH);
        char *p = strrchr(modPath, '\\');
        if (p) {
            strcpy(p + 1, "ghost_event_log.txt");
            strncpy(g_logPath, modPath, MAX_PATH - 1);
        }
#endif
        CreateThread(NULL, 0, init_thread, NULL, 0, NULL);
    }
    return TRUE;
}
