/*
 * LevelSpecials_Loader - Universal Bumper Collision Hook
 *
 * Makes N:BUMPER objects work on ALL levels, not just Beginner/Expert/Toob/Master/Sky.
 * Hooks DispatchCollisionEvents (0x0040C5D0) with a trampoline.
 *
 * For levels whose vtable[29] == DispatchCollisionEvents (no native bumper handler),
 * the hook checks if the collision object name starts with "N:BUMPER" and applies
 * the standard bumper bounce physics (velocity scale 4.0, clamp 5.0-10.0, play sound).
 *
 * Levels with custom collision handlers (Beginner=0x004111E0, Expert=0x00410020)
 * are skipped to avoid double-bounce.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>

/* QWORD typedef for BASS functions */
typedef unsigned long long QWORD;

/* ═══════════════════════════════════════════════════════════════════════════
 * Game addresses (RVAs — add module base at runtime)
 * ═══════════════════════════════════════════════════════════════════════════ */

#define RVA_DispatchCollisionEvents  0x000CC5D0  /* 0x0040C5D0 */
#define RVA_Sound_Play3D             0x00059860  /* 0x00459860 */
#define RVA_g_App                    0x001341E0  /* 0x005341E0 (global App ptr) */

/* Bumper physics constants (from game data section) */
#define BUMPER_VEL_SCALE  4.0f   /* 0x004CF41C */
#define BUMPER_MIN_SPEED  5.0f   /* 0x004CF55C */
#define BUMPER_MAX_SPEED  10.0f  /* 0x004CF9F8 */

/* Ball struct offsets */
#define BALL_POS_X        0x164
#define BALL_POS_Y        0x168
#define BALL_POS_Z        0x16C
#define BALL_PHYS_PTR     0x1A4

/* Physics struct offsets */
#define PHYS_VEL_X        0xCA4
#define PHYS_VEL_Y        0xCA8
#define PHYS_VEL_Z        0xCAC

/* App struct offsets */
#define APP_SOUNDFX_LIST  0x448  /* App+0x448 = sound effect list */

/* Collision pair offsets */
#define COLL_MESHBUF      0x4    /* pair+0x4 = MeshBuffer pointer */
#define MESHBUF_NAME      0x864  /* MeshBuffer+0x864 = name string char* */

/* Board struct offsets */
#define BOARD_VTABLE      0x0    /* board+0x0 = vtable pointer */
#define BOARD_APP_PTR     0x878  /* board+0x878 = App pointer */

/* Vtable layout */
#define VTABLE_DISPATCH_COLLISION  29  /* slot 29, byte offset 0x74 */

/* ═══════════════════════════════════════════════════════════════════════════
 * Function pointer types
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef void (__thiscall *DispatchCollisionEvents_t)(void *board, void *ball, void *collPair);
typedef void (__cdecl *Sound_Play3D_t)(void *soundList, float x, float y, float z, float volume);

/* ═══════════════════════════════════════════════════════════════════════════
 * Globals
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD g_moduleBase = 0;
static Sound_Play3D_t g_SoundPlay3D = NULL;
/* Must be non-static for asm to reference */
DispatchCollisionEvents_t g_OriginalDispatch = NULL;

/* Trampoline: 8 original bytes + JMP back */
static unsigned char *g_trampoline = NULL;

/* ═══════════════════════════════════════════════════════════════════════════
 * String helpers (avoid CRT dependencies where possible)
 * ═══════════════════════════════════════════════════════════════════════════ */

static int my_strnicmp(const char *a, const char *b, int n) {
    int i;
    for (i = 0; i < n; i++) {
        char ca = a[i], cb = b[i];
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return (unsigned char)ca - (unsigned char)cb;
        if (ca == 0) return 0;
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Apply bumper bounce physics
 *
 * Replicates the logic from FUN_004111E0 (Beginner) / FUN_00410020 (Expert):
 *   1. Play bumper sound at ball position
 *   2. Scale velocity X,Z by 4.0, zero Y
 *   3. If speed < 5.0, normalize to 5.0
 *   4. If speed > 10.0, clamp to 10.0
 * ═══════════════════════════════════════════════════════════════════════════ */

static void ApplyBumperBounce(void *board, void *ball, void *collPair) {
    DWORD *pair = (DWORD *)collPair;
    DWORD meshBuf = pair[COLL_MESHBUF / 4];
    if (!meshBuf) return;

    char *name = *(char **)(meshBuf + MESHBUF_NAME);
    if (!name) return;

    /* Verify this is N:BUMPER */
    if (my_strnicmp(name, "N:BUMPER", 8) != 0)
        return;

    /* Get ball position for 3D sound */
    float posX = *(float *)((char *)ball + BALL_POS_X);
    float posY = *(float *)((char *)ball + BALL_POS_Y);
    float posZ = *(float *)((char *)ball + BALL_POS_Z);

    /* Play bumper sound */
    DWORD *app = *(DWORD **)((char *)board + BOARD_APP_PTR);
    if (app && !IsBadReadPtr(app, 0x500)) {
        DWORD soundList = app[APP_SOUNDFX_LIST / 4];
        if (soundList && g_SoundPlay3D) {
            g_SoundPlay3D((void *)soundList, posX, posY, posZ, 1.0f);
        }
    }

    /* Get physics struct */
    DWORD phys = *(DWORD *)((char *)ball + BALL_PHYS_PTR);
    if (!phys || IsBadReadPtr((void *)phys, 0xCB0))
        return;

    float *velX = (float *)(phys + PHYS_VEL_X);
    float *velY = (float *)(phys + PHYS_VEL_Y);
    float *velZ = (float *)(phys + PHYS_VEL_Z);

    /* Read current velocity */
    float vx = *velX;
    float vz = *velZ;

    /* Scale by bumper constant, zero Y */
    vx *= BUMPER_VEL_SCALE;
    vz *= BUMPER_VEL_SCALE;
    float vy = 0.0f;

    /* Clamp to minimum speed (5.0) */
    float speedSq = vx * vx + vz * vz;
    if (speedSq < BUMPER_MIN_SPEED * BUMPER_MIN_SPEED) {
        if (speedSq > 0.0001f) {
            float speed = sqrtf(speedSq);
            float scale = BUMPER_MIN_SPEED / speed;
            vx *= scale;
            vz *= scale;
        }
    }

    /* Clamp to maximum speed (10.0) */
    speedSq = vx * vx + vz * vz;
    if (speedSq > BUMPER_MAX_SPEED * BUMPER_MAX_SPEED) {
        float speed = sqrtf(speedSq);
        if (speed > 0.0001f) {
            float scale = BUMPER_MAX_SPEED / speed;
            vx *= scale;
            vz *= scale;
        }
    }

    /* Write back velocity */
    *velX = vx;
    *velY = vy;
    *velZ = vz;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook function for DispatchCollisionEvents
 *
 * Original calling convention: __thiscall
 *   ECX = board (this pointer)
 *   [ESP+4] = ball (first stack param)
 *   [ESP+8] = collPair (second stack param)
 *   RET 0x8 (callee cleans 8 bytes)
 *
 * We use a naked thunk that saves registers, calls our C logic,
 * then tail-JMPs to the trampoline (which runs the original and RET 0x8).
 * The tail-JMP means the trampoline's RET 0x8 handles stack cleanup.
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Use __cdecl (not __fastcall) to avoid @16 name decoration in asm.
 * The naked thunk pushes all params on stack per cdecl convention. */
void __cdecl BumperCollisionLogic(void *board, void *ball, void *collPair) {
    /* Check if level has a native bumper handler */
    DWORD *vtable = *(DWORD **)board;
    DWORD dispatchAddr = vtable[VTABLE_DISPATCH_COLLISION];
    DWORD origAddr = g_moduleBase + RVA_DispatchCollisionEvents;

    /* Only apply bumper bounce if the level uses the DEFAULT dispatch handler */
    if (dispatchAddr == origAddr) {
        ApplyBumperBounce(board, ball, collPair);
    }

    /* Return to caller, which will tail-JMP to trampoline */
}

/* Naked thunk: intercepts the original __thiscall, calls C logic, then
 * tail-JMPs to the trampoline so the original's RET 0x8 handles cleanup.
 *
 * MinGW uses AT&T syntax and __asm__() not __asm {}.
 * We use GCC extended asm with naked attribute. */

__attribute__((naked)) static void Hook_DispatchCollisionEvents(void) {
    __asm__ __volatile__(
        "pushl %%ebp\n\t"
        "movl  %%esp, %%ebp\n\t"
        "pushl %%ecx\n\t"          /* save board (this) */
        "pushl %%edx\n\t"          /* save EDX */

        /* Call BumperCollisionLogic(board, ball, collPair) via cdecl */
        /* Params from stack: [ebp+8]=ball, [ebp+12]=collPair, saved ecx=board */
        "pushl 12(%%ebp)\n\t"      /* collPair (push right-to-left) */
        "pushl 8(%%ebp)\n\t"       /* ball */
        "pushl -4(%%ebp)\n\t"      /* board (saved on stack) */
        "call  _BumperCollisionLogic\n\t"
        "addl  $12, %%esp\n\t"     /* cdecl: caller cleans 3 params */

        "popl  %%edx\n\t"
        "popl  %%ecx\n\t"          /* restore board for trampoline */
        "popl  %%ebp\n\t"

        /* Tail-JMP to trampoline */
        "jmpl  *_g_OriginalDispatch\n\t"
        :: : "eax", "memory"
    );
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Install hook on DispatchCollisionEvents
 *
 * Original prologue (8 bytes):
 *   6A FF                 PUSH -1 (SEH frame)
 *   64 A1 00 00 00 00     MOV EAX, FS:[0]
 *
 * We copy 8 bytes to trampoline, add JMP back to original+8.
 * Patch original with JMP to our hook + 3 NOPs (8 bytes total).
 * ═══════════════════════════════════════════════════════════════════════════ */

static void InstallHook(void) {
    DWORD targetAddr = g_moduleBase + RVA_DispatchCollisionEvents;
    unsigned char *orig = (unsigned char *)targetAddr;

    /* Verify prologue signature */
    if (orig[0] != 0x6A || orig[1] != 0xFF ||
        orig[2] != 0x64 || orig[3] != 0xA1) {
        return;  /* Signature mismatch — don't hook */
    }

    /* Allocate executable trampoline (16 bytes) */
    g_trampoline = (unsigned char *)VirtualAlloc(NULL, 16,
                        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_trampoline) return;

    /* Copy 8 original bytes to trampoline */
    memcpy(g_trampoline, orig, 8);

    /* Append JMP rel32 back to original+8 */
    g_trampoline[8] = 0xE9;
    DWORD jmpTarget = (targetAddr + 8) - ((DWORD)g_trampoline + 8 + 5);
    *(DWORD *)(g_trampoline + 9) = jmpTarget;

    /* Set original function pointer to trampoline */
    g_OriginalDispatch = (DispatchCollisionEvents_t)g_trampoline;

    /* Patch original: JMP to hook + 3 NOPs (8 bytes) */
    DWORD oldProtect;
    VirtualProtect(orig, 16, PAGE_EXECUTE_READWRITE, &oldProtect);

    orig[0] = 0xE9;  /* JMP rel32 */
    *(DWORD *)(orig + 1) = (DWORD)&Hook_DispatchCollisionEvents - (targetAddr + 5);
    orig[5] = 0x90;  /* NOP */
    orig[6] = 0x90;  /* NOP */
    orig[7] = 0x90;  /* NOP */

    VirtualProtect(orig, 16, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), orig, 16);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS.dll proxy exports
 *
 * Forward all BASS functions to bass_real.dll (renamed original bass.dll)
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;

/* Function pointer typedefs for forwarded BASS functions */
typedef BOOL  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
typedef void  (__stdcall *BASS_Free_t)(void);
typedef BOOL  (__stdcall *BASS_Start_t)(void);
typedef void  (__stdcall *BASS_Stop_t)(void);
typedef BOOL  (__stdcall *BASS_Pause_t)(void);
typedef DWORD (__stdcall *BASS_GetVersion_t)(void);
typedef int   (__stdcall *BASS_ErrorGetCode_t)(void);
typedef BOOL  (__stdcall *BASS_SetVolume_t)(float);
typedef float (__stdcall *BASS_GetVolume_t)(void);
typedef BOOL  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
typedef DWORD (__stdcall *BASS_GetConfig_t)(DWORD);
typedef DWORD (__stdcall *BASS_GetDevice_t)(void);
typedef BOOL  (__stdcall *BASS_SetDevice_t)(DWORD);
typedef BOOL  (__stdcall *BASS_GetInfo_t)(void*);
typedef int   (__stdcall *BASS_GetDeviceInfo_t)(DWORD, void*);
typedef BOOL  (__stdcall *BASS_Update_t)(DWORD);
typedef DWORD (__stdcall *BASS_SampleLoad_t)(BOOL, const char*, QWORD, DWORD, DWORD, DWORD);
typedef DWORD (__stdcall *BASS_SampleCreate_t)(DWORD, DWORD, DWORD, DWORD, DWORD);
typedef BOOL  (__stdcall *BASS_SampleFree_t)(DWORD);
typedef DWORD (__stdcall *BASS_SampleGetChannel_t)(DWORD, BOOL);
typedef BOOL  (__stdcall *BASS_SampleGetInfo_t)(DWORD, void*);
typedef BOOL  (__stdcall *BASS_SampleSetInfo_t)(DWORD, void*);
typedef DWORD (__stdcall *BASS_StreamCreate_t)(DWORD, DWORD, DWORD, void*, void*);
typedef DWORD (__stdcall *BASS_StreamCreateFile_t)(BOOL, const void*, QWORD, QWORD, DWORD);
typedef DWORD (__stdcall *BASS_StreamCreateURL_t)(const char*, DWORD, DWORD, void*, void*);
typedef BOOL  (__stdcall *BASS_StreamFree_t)(DWORD);
typedef QWORD (__stdcall *BASS_StreamGetFilePosition_t)(DWORD, DWORD);
typedef BOOL  (__stdcall *BASS_ChannelPlay_t)(DWORD, BOOL);
typedef BOOL  (__stdcall *BASS_ChannelStop_t)(DWORD);
typedef BOOL  (__stdcall *BASS_ChannelPause_t)(DWORD);
typedef BOOL  (__stdcall *BASS_ChannelSetPosition_t)(DWORD, QWORD, DWORD);
typedef QWORD (__stdcall *BASS_ChannelGetPosition_t)(DWORD, DWORD);
typedef QWORD (__stdcall *BASS_ChannelGetLength_t)(DWORD, DWORD);
typedef DWORD (__stdcall *BASS_ChannelSetSync_t)(DWORD, DWORD, QWORD, void*, void*);
typedef BOOL  (__stdcall *BASS_ChannelRemoveSync_t)(DWORD, DWORD);
typedef BOOL  (__stdcall *BASS_ChannelSetAttribute_t)(DWORD, DWORD, float);
typedef BOOL  (__stdcall *BASS_ChannelGetAttribute_t)(DWORD, DWORD, float*);
typedef BOOL  (__stdcall *BASS_ChannelSlideAttribute_t)(DWORD, DWORD, float, DWORD);
typedef BOOL  (__stdcall *BASS_ChannelIsSliding_t)(DWORD, DWORD);
typedef BOOL  (__stdcall *BASS_ChannelGetInfo_t)(DWORD, void*);
typedef BOOL  (__stdcall *BASS_ChannelUpdate_t)(DWORD, DWORD);
typedef BOOL  (__stdcall *BASS_ChannelLock_t)(DWORD, BOOL);
typedef DWORD (__stdcall *BASS_MusicLoad_t)(BOOL, const void*, QWORD, DWORD, DWORD, DWORD);
typedef BOOL  (__stdcall *BASS_MusicFree_t)(DWORD);
typedef BOOL  (__stdcall *BASS_RecordInit_t)(int);
typedef BOOL  (__stdcall *BASS_RecordFree_t)(void);
typedef DWORD (__stdcall *BASS_RecordGetDevice_t)(void);
typedef BOOL  (__stdcall *BASS_RecordSetDevice_t)(DWORD);
typedef BOOL  (__stdcall *BASS_RecordGetInfo_t)(void*);
typedef DWORD (__stdcall *BASS_RecordStart_t)(DWORD, DWORD, DWORD, void*, void*);
typedef DWORD (__stdcall *BASS_ChannelGetLevel_t)(DWORD);
typedef BOOL  (__stdcall *BASS_PluginLoad_t)(const char*, DWORD);
typedef BOOL  (__stdcall *BASS_PluginFree_t)(DWORD);

/* Real function pointers */
static BASS_Init_t                 real_BASS_Init;
static BASS_Free_t                  real_BASS_Free;
static BASS_Start_t                 real_BASS_Start;
static BASS_Stop_t                  real_BASS_Stop;
static BASS_Pause_t                 real_BASS_Pause;
static BASS_GetVersion_t            real_BASS_GetVersion;
static BASS_ErrorGetCode_t          real_BASS_ErrorGetCode;
static BASS_SetVolume_t             real_BASS_SetVolume;
static BASS_GetVolume_t             real_BASS_GetVolume;
static BASS_SetConfig_t             real_BASS_SetConfig;
static BASS_GetConfig_t             real_BASS_GetConfig;
static BASS_GetDevice_t             real_BASS_GetDevice;
static BASS_SetDevice_t             real_BASS_SetDevice;
static BASS_GetInfo_t               real_BASS_GetInfo;
static BASS_GetDeviceInfo_t         real_BASS_GetDeviceInfo;
static BASS_Update_t                real_BASS_Update;
static BASS_SampleLoad_t            real_BASS_SampleLoad;
static BASS_SampleCreate_t         real_BASS_SampleCreate;
static BASS_SampleFree_t            real_BASS_SampleFree;
static BASS_SampleGetChannel_t      real_BASS_SampleGetChannel;
static BASS_SampleGetInfo_t        real_BASS_SampleGetInfo;
static BASS_SampleSetInfo_t        real_BASS_SampleSetInfo;
static BASS_StreamCreate_t          real_BASS_StreamCreate;
static BASS_StreamCreateFile_t      real_BASS_StreamCreateFile;
static BASS_StreamCreateURL_t       real_BASS_StreamCreateURL;
static BASS_StreamFree_t            real_BASS_StreamFree;
static BASS_StreamGetFilePosition_t real_BASS_StreamGetFilePosition;
static BASS_ChannelPlay_t           real_BASS_ChannelPlay;
static BASS_ChannelStop_t           real_BASS_ChannelStop;
static BASS_ChannelPause_t          real_BASS_ChannelPause;
static BASS_ChannelSetPosition_t    real_BASS_ChannelSetPosition;
static BASS_ChannelGetPosition_t    real_BASS_ChannelGetPosition;
static BASS_ChannelGetLength_t      real_BASS_ChannelGetLength;
static BASS_ChannelSetSync_t        real_BASS_ChannelSetSync;
static BASS_ChannelRemoveSync_t     real_BASS_ChannelRemoveSync;
static BASS_ChannelSetAttribute_t   real_BASS_ChannelSetAttribute;
static BASS_ChannelGetAttribute_t   real_BASS_ChannelGetAttribute;
static BASS_ChannelSlideAttribute_t real_BASS_ChannelSlideAttribute;
static BASS_ChannelIsSliding_t      real_BASS_ChannelIsSliding;
static BASS_ChannelGetInfo_t        real_BASS_ChannelGetInfo;
static BASS_ChannelUpdate_t         real_BASS_ChannelUpdate;
static BASS_ChannelLock_t           real_BASS_ChannelLock;
static BASS_MusicLoad_t             real_BASS_MusicLoad;
static BASS_MusicFree_t             real_BASS_MusicFree;
static BASS_RecordInit_t            real_BASS_RecordInit;
static BASS_RecordFree_t            real_BASS_RecordFree;
static BASS_RecordGetDevice_t       real_BASS_RecordGetDevice;
static BASS_RecordSetDevice_t       real_BASS_RecordSetDevice;
static BASS_RecordGetInfo_t         real_BASS_RecordGetInfo;
static BASS_RecordStart_t           real_BASS_RecordStart;
static BASS_ChannelGetLevel_t       real_BASS_ChannelGetLevel;
static BASS_PluginLoad_t            real_BASS_PluginLoad;
static BASS_PluginFree_t            real_BASS_PluginFree;

#define LOAD_FN(name) real_##name = (name##_t)GetProcAddress(g_hRealBass, #name)

static void load_real_bass(void) {
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (!g_hRealBass) {
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
    if (!g_hRealBass) return;

    LOAD_FN(BASS_Init);
    LOAD_FN(BASS_Free);
    LOAD_FN(BASS_Start);
    LOAD_FN(BASS_Stop);
    LOAD_FN(BASS_Pause);
    LOAD_FN(BASS_GetVersion);
    LOAD_FN(BASS_ErrorGetCode);
    LOAD_FN(BASS_SetVolume);
    LOAD_FN(BASS_GetVolume);
    LOAD_FN(BASS_SetConfig);
    LOAD_FN(BASS_GetConfig);
    LOAD_FN(BASS_GetDevice);
    LOAD_FN(BASS_SetDevice);
    LOAD_FN(BASS_GetInfo);
    LOAD_FN(BASS_GetDeviceInfo);
    LOAD_FN(BASS_Update);
    LOAD_FN(BASS_SampleLoad);
    LOAD_FN(BASS_SampleCreate);
    LOAD_FN(BASS_SampleFree);
    LOAD_FN(BASS_SampleGetChannel);
    LOAD_FN(BASS_SampleGetInfo);
    LOAD_FN(BASS_SampleSetInfo);
    LOAD_FN(BASS_StreamCreate);
    LOAD_FN(BASS_StreamCreateFile);
    LOAD_FN(BASS_StreamCreateURL);
    LOAD_FN(BASS_StreamFree);
    LOAD_FN(BASS_StreamGetFilePosition);
    LOAD_FN(BASS_ChannelPlay);
    LOAD_FN(BASS_ChannelStop);
    LOAD_FN(BASS_ChannelPause);
    LOAD_FN(BASS_ChannelSetPosition);
    LOAD_FN(BASS_ChannelGetPosition);
    LOAD_FN(BASS_ChannelGetLength);
    LOAD_FN(BASS_ChannelSetSync);
    LOAD_FN(BASS_ChannelRemoveSync);
    LOAD_FN(BASS_ChannelSetAttribute);
    LOAD_FN(BASS_ChannelGetAttribute);
    LOAD_FN(BASS_ChannelSlideAttribute);
    LOAD_FN(BASS_ChannelIsSliding);
    LOAD_FN(BASS_ChannelGetInfo);
    LOAD_FN(BASS_ChannelUpdate);
    LOAD_FN(BASS_ChannelLock);
    LOAD_FN(BASS_MusicLoad);
    LOAD_FN(BASS_MusicFree);
    LOAD_FN(BASS_RecordInit);
    LOAD_FN(BASS_RecordFree);
    LOAD_FN(BASS_RecordGetDevice);
    LOAD_FN(BASS_RecordSetDevice);
    LOAD_FN(BASS_RecordGetInfo);
    LOAD_FN(BASS_RecordStart);
    LOAD_FN(BASS_ChannelGetLevel);
    LOAD_FN(BASS_PluginLoad);
    LOAD_FN(BASS_PluginFree);
}

/* Exported BASS proxy functions */
__declspec(dllexport) BOOL  __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void *e) { return real_BASS_Init ? real_BASS_Init(a,b,c,d,e) : FALSE; }
__declspec(dllexport) void  __stdcall BASS_Free(void) { if (real_BASS_Free) real_BASS_Free(); }
__declspec(dllexport) BOOL  __stdcall BASS_Start(void) { return real_BASS_Start ? real_BASS_Start() : FALSE; }
__declspec(dllexport) void  __stdcall BASS_Stop(void) { if (real_BASS_Stop) real_BASS_Stop(); }
__declspec(dllexport) BOOL  __stdcall BASS_Pause(void) { return real_BASS_Pause ? real_BASS_Pause() : FALSE; }
__declspec(dllexport) DWORD __stdcall BASS_GetVersion(void) { return real_BASS_GetVersion ? real_BASS_GetVersion() : 0; }
__declspec(dllexport) int   __stdcall BASS_ErrorGetCode(void) { return real_BASS_ErrorGetCode ? real_BASS_ErrorGetCode() : 0; }
__declspec(dllexport) BOOL  __stdcall BASS_SetVolume(float a) { return real_BASS_SetVolume ? real_BASS_SetVolume(a) : FALSE; }
__declspec(dllexport) float __stdcall BASS_GetVolume(void) { return real_BASS_GetVolume ? real_BASS_GetVolume() : 0.0f; }
__declspec(dllexport) BOOL  __stdcall BASS_SetConfig(DWORD a, DWORD b) { return real_BASS_SetConfig ? real_BASS_SetConfig(a,b) : FALSE; }
__declspec(dllexport) DWORD __stdcall BASS_GetConfig(DWORD a) { return real_BASS_GetConfig ? real_BASS_GetConfig(a) : 0; }
__declspec(dllexport) DWORD __stdcall BASS_GetDevice(void) { return real_BASS_GetDevice ? real_BASS_GetDevice() : 0; }
__declspec(dllexport) BOOL  __stdcall BASS_SetDevice(DWORD a) { return real_BASS_SetDevice ? real_BASS_SetDevice(a) : FALSE; }
__declspec(dllexport) BOOL  __stdcall BASS_GetInfo(void *a) { return real_BASS_GetInfo ? real_BASS_GetInfo(a) : FALSE; }
__declspec(dllexport) int   __stdcall BASS_GetDeviceInfo(DWORD a, void *b) { return real_BASS_GetDeviceInfo ? real_BASS_GetDeviceInfo(a,b) : 0; }
__declspec(dllexport) BOOL  __stdcall BASS_Update(DWORD a) { return real_BASS_Update ? real_BASS_Update(a) : FALSE; }
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(BOOL a, const char *b, QWORD c, DWORD d, DWORD e, DWORD f) { return real_BASS_SampleLoad ? real_BASS_SampleLoad(a,b,c,d,e,f) : 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleCreate(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e) { return real_BASS_SampleCreate ? real_BASS_SampleCreate(a,b,c,d,e) : 0; }
__declspec(dllexport) BOOL  __stdcall BASS_SampleFree(DWORD a) { return real_BASS_SampleFree ? real_BASS_SampleFree(a) : FALSE; }
__declspec(dllexport) DWORD __stdcall BASS_SampleGetChannel(DWORD a, BOOL b) { return real_BASS_SampleGetChannel ? real_BASS_SampleGetChannel(a,b) : 0; }
__declspec(dllexport) BOOL  __stdcall BASS_SampleGetInfo(DWORD a, void *b) { return real_BASS_SampleGetInfo ? real_BASS_SampleGetInfo(a,b) : FALSE; }
__declspec(dllexport) BOOL  __stdcall BASS_SampleSetInfo(DWORD a, void *b) { return real_BASS_SampleSetInfo ? real_BASS_SampleSetInfo(a,b) : FALSE; }
__declspec(dllexport) DWORD __stdcall BASS_StreamCreate(DWORD a, DWORD b, DWORD c, void *d, void *e) { return real_BASS_StreamCreate ? real_BASS_StreamCreate(a,b,c,d,e) : 0; }
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateFile(BOOL a, const void *b, QWORD c, QWORD d, DWORD e) { return real_BASS_StreamCreateFile ? real_BASS_StreamCreateFile(a,b,c,d,e) : 0; }
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateURL(const char *a, DWORD b, DWORD c, void *d, void *e) { return real_BASS_StreamCreateURL ? real_BASS_StreamCreateURL(a,b,c,d,e) : 0; }
__declspec(dllexport) BOOL  __stdcall BASS_StreamFree(DWORD a) { return real_BASS_StreamFree ? real_BASS_StreamFree(a) : FALSE; }
__declspec(dllexport) QWORD __stdcall BASS_StreamGetFilePosition(DWORD a, DWORD b) { return real_BASS_StreamGetFilePosition ? real_BASS_StreamGetFilePosition(a,b) : 0; }
__declspec(dllexport) BOOL  __stdcall BASS_ChannelPlay(DWORD a, BOOL b) { return real_BASS_ChannelPlay ? real_BASS_ChannelPlay(a,b) : FALSE; }
__declspec(dllexport) BOOL  __stdcall BASS_ChannelStop(DWORD a) { return real_BASS_ChannelStop ? real_BASS_ChannelStop(a) : FALSE; }
__declspec(dllexport) BOOL  __stdcall BASS_ChannelPause(DWORD a) { return real_BASS_ChannelPause ? real_BASS_ChannelPause(a) : FALSE; }
__declspec(dllexport) BOOL  __stdcall BASS_ChannelSetPosition(DWORD a, QWORD b, DWORD c) { return real_BASS_ChannelSetPosition ? real_BASS_ChannelSetPosition(a,b,c) : FALSE; }
__declspec(dllexport) QWORD __stdcall BASS_ChannelGetPosition(DWORD a, DWORD b) { return real_BASS_ChannelGetPosition ? real_BASS_ChannelGetPosition(a,b) : 0; }
__declspec(dllexport) QWORD __stdcall BASS_ChannelGetLength(DWORD a, DWORD b) { return real_BASS_ChannelGetLength ? real_BASS_ChannelGetLength(a,b) : 0; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelSetSync(DWORD a, DWORD b, QWORD c, void *d, void *e) { return real_BASS_ChannelSetSync ? real_BASS_ChannelSetSync(a,b,c,d,e) : 0; }
__declspec(dllexport) BOOL  __stdcall BASS_ChannelRemoveSync(DWORD a, DWORD b) { return real_BASS_ChannelRemoveSync ? real_BASS_ChannelRemoveSync(a,b) : FALSE; }
__declspec(dllexport) BOOL  __stdcall BASS_ChannelSetAttribute(DWORD a, DWORD b, float c) { return real_BASS_ChannelSetAttribute ? real_BASS_ChannelSetAttribute(a,b,c) : FALSE; }
__declspec(dllexport) BOOL  __stdcall BASS_ChannelGetAttribute(DWORD a, DWORD b, float *c) { return real_BASS_ChannelGetAttribute ? real_BASS_ChannelGetAttribute(a,b,c) : FALSE; }
__declspec(dllexport) BOOL  __stdcall BASS_ChannelSlideAttribute(DWORD a, DWORD b, float c, DWORD d) { return real_BASS_ChannelSlideAttribute ? real_BASS_ChannelSlideAttribute(a,b,c,d) : FALSE; }
__declspec(dllexport) BOOL  __stdcall BASS_ChannelIsSliding(DWORD a, DWORD b) { return real_BASS_ChannelIsSliding ? real_BASS_ChannelIsSliding(a,b) : FALSE; }
__declspec(dllexport) BOOL  __stdcall BASS_ChannelGetInfo(DWORD a, void *b) { return real_BASS_ChannelGetInfo ? real_BASS_ChannelGetInfo(a,b) : FALSE; }
__declspec(dllexport) BOOL  __stdcall BASS_ChannelUpdate(DWORD a, DWORD b) { return real_BASS_ChannelUpdate ? real_BASS_ChannelUpdate(a,b) : FALSE; }
__declspec(dllexport) BOOL  __stdcall BASS_ChannelLock(DWORD a, BOOL b) { return real_BASS_ChannelLock ? real_BASS_ChannelLock(a,b) : FALSE; }
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(BOOL a, const void *b, QWORD c, DWORD d, DWORD e, DWORD f) { return real_BASS_MusicLoad ? real_BASS_MusicLoad(a,b,c,d,e,f) : 0; }
__declspec(dllexport) BOOL  __stdcall BASS_MusicFree(DWORD a) { return real_BASS_MusicFree ? real_BASS_MusicFree(a) : FALSE; }
__declspec(dllexport) BOOL  __stdcall BASS_RecordInit(int a) { return real_BASS_RecordInit ? real_BASS_RecordInit(a) : FALSE; }
__declspec(dllexport) BOOL  __stdcall BASS_RecordFree(void) { return real_BASS_RecordFree ? real_BASS_RecordFree() : FALSE; }
__declspec(dllexport) DWORD __stdcall BASS_RecordGetDevice(void) { return real_BASS_RecordGetDevice ? real_BASS_RecordGetDevice() : 0; }
__declspec(dllexport) BOOL  __stdcall BASS_RecordSetDevice(DWORD a) { return real_BASS_RecordSetDevice ? real_BASS_RecordSetDevice(a) : FALSE; }
__declspec(dllexport) BOOL  __stdcall BASS_RecordGetInfo(void *a) { return real_BASS_RecordGetInfo ? real_BASS_RecordGetInfo(a) : FALSE; }
__declspec(dllexport) DWORD __stdcall BASS_RecordStart(DWORD a, DWORD b, DWORD c, void *d, void *e) { return real_BASS_RecordStart ? real_BASS_RecordStart(a,b,c,d,e) : 0; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetLevel(DWORD a) { return real_BASS_ChannelGetLevel ? real_BASS_ChannelGetLevel(a) : 0; }
__declspec(dllexport) BOOL  __stdcall BASS_PluginLoad(const char *a, DWORD b) { return real_BASS_PluginLoad ? real_BASS_PluginLoad(a,b) : FALSE; }
__declspec(dllexport) BOOL  __stdcall BASS_PluginFree(DWORD a) { return real_BASS_PluginFree ? real_BASS_PluginFree(a) : FALSE; }

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch thread — waits for game to settle, then installs hook
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI PatchThread(LPVOID param) {
    Sleep(2000);

    /* Get module base */
    g_moduleBase = (DWORD)GetModuleHandleA("Hamsterball.exe");
    if (!g_moduleBase) g_moduleBase = 0x00400000;

    /* Resolve function pointers */
    g_SoundPlay3D = (Sound_Play3D_t)(g_moduleBase + RVA_Sound_Play3D);

    /* Install the DispatchCollisionEvents hook */
    InstallHook();

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain — entry point
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);

        /* Load real bass.dll */
        load_real_bass();

        /* Start patch thread */
        CreateThread(NULL, 0, PatchThread, NULL, 0, NULL);
    }
    return TRUE;
}
