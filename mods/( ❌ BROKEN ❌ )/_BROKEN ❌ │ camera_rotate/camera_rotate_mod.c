/*
 * Camera_Rotate Mod for Hamsterball
 *
 * Adds a custom E: plane E:ROTCAM that rotates the camera 90 degrees
 * around the Y axis over 3 seconds with sinusoidal ease-in/out.
 *
 *   E:ROTCAM(R)  → rotate right (clockwise, +90°)
 *   E:ROTCAM(L)  → rotate left  (counter-clockwise, -90°)
 *
 * The rotation uses a cosine ease-in-out curve:
 *   angle(t) = startAngle + totalRotation * (1 - cos(π * t)) / 2
 * where t goes from 0 to 1 over 3 seconds.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll camera_rotate_mod.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* ============================================================
 * Diagnostic logging — writes to camera_rotate_log.txt
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

/* Extra BASS stubs */
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

#define APP_PTR                 0x005341E0
#define DISPATCH_COLLISION_EVENTS   0x0040C5D0

/* Ball offsets */
#define BALL_PLAYER_INDEX        0x18
#define BALL_DEATH_PENDING       0x2E9

/* Board offsets */
#define BOARD_SCENE_PTR_OFFSET   0x878

/* Scene offsets — verified from Scene_SetCamera (0x419FA0) disassembly */
#define SCENE_ORBIT_ANGLE        0x29BC   /* float, radians */
#define SCENE_ORBIT_DISTANCE     0x29C0   /* float */

/* Collision event pair offsets */
#define COLL_OBJ_NAME_OFFSET     0x864

/* ============================================================
 * Rotation state
 * ============================================================ */

/* Rotation animation state */
static volatile int g_rotActive = 0;        /* 1 = currently rotating */
static volatile float g_rotStartAngle = 0.0f;  /* scene+0x29BC at start */
static volatile float g_rotTargetDelta = 0.0f; /* +PI/2 or -PI/2 */
static volatile DWORD g_rotStartTime = 0;   /* GetTickCount() at start */
static volatile DWORD g_rotDuration = 3000;  /* 3 seconds in ms */

#define PI_F       3.14159265358979323846f
#define HALF_PI_F  (PI_F * 0.5f)

/* Stats */
static volatile int g_hookFireCount = 0;
static volatile int g_rotcamMatchCount = 0;

/* Saved args from asm detour */
static volatile void *g_savedBoard = NULL;
static volatile int *g_savedBall = NULL;
static volatile int *g_savedCollObj = NULL;

/* ============================================================
 * Memory helpers
 * ============================================================
 */

static int GetApp(void) {
    return *(int *)APP_PTR;
}

static float ReadSceneAngle(void) {
    int app = GetApp();
    int profile, board, scene;
    if (!app) return 0.0f;
    profile = *(int *)((char *)app + 0x220);
    if (!profile) return 0.0f;
    board = *(int *)((char *)profile + 0x0C);
    if (!board) return 0.0f;
    scene = *(int *)((char *)board + BOARD_SCENE_PTR_OFFSET);
    if (!scene) return 0.0f;
    return *(float *)((char *)scene + SCENE_ORBIT_ANGLE);
}

static void WriteSceneAngle(float angle) {
    int app = GetApp();
    int profile, board, scene;
    if (!app) return;
    profile = *(int *)((char *)app + 0x220);
    if (!profile) return;
    board = *(int *)((char *)profile + 0x0C);
    if (!board) return;
    scene = *(int *)((char *)board + BOARD_SCENE_PTR_OFFSET);
    if (!scene) return;
    *(float *)((char *)scene + SCENE_ORBIT_ANGLE) = angle;
}

/* ============================================================
 * Event parsing
 * ============================================================
 */

static int isRotcamEvent(const char *eventName) {
    if (!eventName) return 0;
    return _strnicmp(eventName, "E:ROTCAM", 8) == 0;
}

static int parseRotcamDirection(const char *eventName) {
    /* Returns +1 for R (right), -1 for L (left), 0 on error */
    const char *start = strchr(eventName, '(');
    if (!start) return 0;
    start++;
    if (*start == 'R' || *start == 'r') return 1;
    if (*start == 'L' || *start == 'l') return -1;
    return 0;
}

/* ============================================================
 * Collision handler — called from asm detour
 * ============================================================
 */

static void CameraRotateCollisionHandler(void) {
    void *board = (void *)g_savedBoard;
    int *ball = (int *)g_savedBall;
    int *collObj = (int *)g_savedCollObj;

    g_hookFireCount++;

    if (collObj && collObj[1]) {
        const char *eventName = *(const char **)((char *)collObj[1] + COLL_OBJ_NAME_OFFSET);
        if (eventName) {
            /* Log first few events */
            if (g_hookFireCount <= 10 || (g_hookFireCount % 100) == 0) {
                diag_logf("[hook #%d] event=\"%s\"", g_hookFireCount, eventName);
            }
            if (isRotcamEvent(eventName)) {
                int dir = parseRotcamDirection(eventName);
                g_rotcamMatchCount++;
                diag_logf("[ROTCAM MATCH #%d] event=\"%s\" dir=%d",
                           g_rotcamMatchCount, eventName, dir);
                if (dir != 0 && !g_rotActive) {
                    /* Start rotation if ball is alive */
                    if (*((char *)ball + BALL_DEATH_PENDING) == 0) {
                        g_rotStartAngle = ReadSceneAngle();
                        g_rotTargetDelta = (float)dir * HALF_PI_F;
                        g_rotStartTime = GetTickCount();
                        g_rotActive = 1;
                        diag_logf("[ROTCAM] STARTED! startAngle=%.4f delta=%.4f dir=%s",
                           g_rotStartAngle, g_rotTargetDelta, dir > 0 ? "RIGHT" : "LEFT");
                    } else {
                        diag_log("[ROTCAM] SKIPPED: ball dying");
                    }
                } else if (g_rotActive) {
                    diag_log("[ROTCAM] SKIPPED: rotation already in progress");
                }
            }
        }
    }
}

/* ============================================================
 * Per-frame polling thread — handles rotation animation
 * ============================================================
 */

static DWORD WINAPI CameraRotatePollThread(LPVOID param) {
    (void)param;
    while (1) {
        if (g_rotActive) {
            DWORD elapsed = GetTickCount() - g_rotStartTime;
            float t;
            float easedT;
            float newAngle;

            if (elapsed >= g_rotDuration) {
                /* Animation complete — set final angle */
                newAngle = g_rotStartAngle + g_rotTargetDelta;
                WriteSceneAngle(newAngle);
                g_rotActive = 0;
                diag_logf("[ROTCAM] COMPLETE! finalAngle=%.4f", newAngle);
            } else {
                /* t goes 0→1 over duration */
                t = (float)elapsed / (float)g_rotDuration;
                /* Sinusoidal ease-in-out: (1 - cos(π*t)) / 2 */
                easedT = (1.0f - cosf(PI_F * t)) * 0.5f;
                newAngle = g_rotStartAngle + g_rotTargetDelta * easedT;
                WriteSceneAngle(newAngle);
            }
        }
        Sleep(16);  /* ~60fps */
    }
    return 0;
}

/* ============================================================
 * Install hooks — same detour pattern as level warp mod
 * ============================================================
 */

static unsigned char g_trampoline[16];
static unsigned char *g_detourBuf = NULL;

static void InstallHooks(void) {
    unsigned char *dispatchAddr = (unsigned char *)DISPATCH_COLLISION_EVENTS;
    DWORD oldProtect;

    /* Verify signature: 6A FF 64 A1 00 00 00 00 */
    if (dispatchAddr[0] != 0x6A || dispatchAddr[1] != 0xFF ||
        dispatchAddr[2] != 0x64 || dispatchAddr[3] != 0xA1) {
        diag_log("[FATAL] Signature mismatch at 0x40C5D0! Hook NOT installed.");
        return;
    }
    diag_log("[InstallHooks] DispatchCollisionEvents signature OK at 0x40C5D0");

    /* Save original 8 bytes */
    memcpy(g_trampoline, dispatchAddr, 8);

    /* Build trampoline: original 8 bytes + JMP to dispatchAddr+8 */
    g_trampoline[8] = 0xE9;
    *(DWORD *)(g_trampoline + 9) = (DWORD)(dispatchAddr + 8) - (DWORD)(g_trampoline + 13);

    VirtualProtect(g_trampoline, sizeof(g_trampoline), PAGE_EXECUTE_READWRITE, &oldProtect);

    /* Allocate executable detour buffer */
    g_detourBuf = (unsigned char *)VirtualAlloc(NULL, 256, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!g_detourBuf) {
        diag_log("[FATAL] VirtualAlloc failed for detour buffer!");
        return;
    }

    /* Build detour:
     *   PUSH ECX (board)
     *   PUSH EDX
     *   MOV [g_savedBoard], ECX
     *   MOV EAX, [ESP+12]  ; ball (after 2 pushes)
     *   MOV [g_savedBall], EAX
     *   MOV EAX, [ESP+16]  ; collObj
     *   MOV [g_savedCollObj], EAX
     *   MOV EAX, CameraRotateCollisionHandler
     *   CALL EAX
     *   POP EDX
     *   POP ECX
     *   PUSH -1            ; original bytes
     *   MOV EAX, FS:[0]
     *   JMP dispatchAddr+8
     */
    {
        unsigned char *p = g_detourBuf;

        *p++ = 0x51;  /* PUSH ECX */
        *p++ = 0x52;  /* PUSH EDX */

        /* MOV [g_savedBoard], ECX */
        *p++ = 0x89; *p++ = 0x0D;
        *(DWORD *)p = (DWORD)&g_savedBoard; p += 4;

        /* MOV EAX, [ESP+12] */
        *p++ = 0x8B; *p++ = 0x44; *p++ = 0x24; *p++ = 0x0C;
        /* MOV [g_savedBall], EAX */
        *p++ = 0xA3;
        *(DWORD *)p = (DWORD)&g_savedBall; p += 4;

        /* MOV EAX, [ESP+16] */
        *p++ = 0x8B; *p++ = 0x44; *p++ = 0x24; *p++ = 0x10;
        /* MOV [g_savedCollObj], EAX */
        *p++ = 0xA3;
        *(DWORD *)p = (DWORD)&g_savedCollObj; p += 4;

        /* MOV EAX, CameraRotateCollisionHandler ; CALL EAX */
        *p++ = 0xB8;
        *(DWORD *)p = (DWORD)&CameraRotateCollisionHandler; p += 4;
        *p++ = 0xFF; *p++ = 0xD0;

        /* POP EDX, POP ECX */
        *p++ = 0x5A;
        *p++ = 0x59;

        /* Original 8 bytes: PUSH -1 ; MOV EAX, FS:[0] */
        *p++ = 0x6A; *p++ = 0xFF;
        *p++ = 0x64; *p++ = 0xA1; *p++ = 0x00; *p++ = 0x00; *p++ = 0x00; *p++ = 0x00;

        /* JMP to dispatchAddr+8 */
        *p++ = 0xE9;
        *(DWORD *)p = (DWORD)(dispatchAddr + 8) - (DWORD)(p + 4);
        p += 4;
    }

    /* Patch original function: JMP to detour + 3 NOPs */
    VirtualProtect(dispatchAddr, 8, PAGE_EXECUTE_READWRITE, &oldProtect);
    dispatchAddr[0] = 0xE9;
    *(DWORD *)(dispatchAddr + 1) = (DWORD)g_detourBuf - (DWORD)(dispatchAddr + 5);
    dispatchAddr[5] = 0x90;
    dispatchAddr[6] = 0x90;
    dispatchAddr[7] = 0x90;
    VirtualProtect(dispatchAddr, 8, oldProtect, &oldProtect);

    /* Start polling thread */
    CreateThread(NULL, 0, CameraRotatePollThread, NULL, 0, NULL);

    diag_log("[InstallHooks] Hook installed OK. Polling thread started.");
}

/* ============================================================
 * Init thread
 * ============================================================
 */

static DWORD WINAPI InitThread(LPVOID param) {
    (void)param;
    Sleep(2000);
    InstallHooks();
    return 0;
}

/* ============================================================
 * DllMain
 * ============================================================
 */

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
                    strcpy(p + 1, "camera_rotate_log.txt");
                    strncpy(g_logPath, mod_path, MAX_PATH - 1);
                }
            }
        }

        diag_log("=== CAMERA_ROTATE MOD LOADED ===");

        load_real_bass();

        diag_logf("bass_real.dll handle: 0x%08X", (unsigned)g_hRealBass);

        CreateThread(NULL, 0, InitThread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
