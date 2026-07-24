/*
 * portal_mod.c — E:PORTAL(N) teleportation mod (bass.dll proxy version)
 *
 * E:PORTAL(N) collision event triggers teleport to PORTALPOS(N) S1 ref point.
 * Optional PORTALVEC(N) sets exit direction. Preserves incoming speed.
 * Plays zip sound at destination.
 *
 * Based on the HB+ version (PortalMod.cpp) but uses a bass.dll proxy shell
 * with a DispatchCollisionEvents trampoline hook instead of HB+ callbacks.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll portal_mod.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 *
 * Install:
 *   Rename original bass.dll to bass_real.dll, put this bass.dll in game folder.
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ---- BASS types ---- */
typedef unsigned long long QWORD;
typedef DWORD HSTREAM;
typedef DWORD HMUSIC;
typedef DWORD HFX;

/* ---- Game addresses ---- */
#define EXE_BASE                0x00400000
#define APP_PTR                  0x005341E0
#define SCENE_PTR                0x005341E4
#define DISPATCH_COLLISION_EVENTS 0x0040C5D0

/* ---- Ball offsets ---- */
#define BALL_PLAYER_ID           0x018
#define BALL_PHYS_PTR            0x1A4
#define BALL_TELEPORT_FLAG       0xC3C
#define BALL_TELEPORT_X          0xC40
#define BALL_TELEPORT_Y          0xC44
#define BALL_TELEPORT_Z          0xC48
#define BALL_RADIUS              0x284

/* ---- Physics offsets ---- */
#define PHYS_VEL_X               0xCA4
#define PHYS_VEL_Y               0xCA8
#define PHYS_VEL_Z               0xCAC

/* ---- Scene/Level offsets ---- */
#define SCENE_LEVEL_PTR          0x8AC
#define LEVEL_SCENEOBJ           0x480
#define SCENEOBJ_COUNT           0x898
#define SCENEOBJ_ARRAY           0xCA0

/* ---- Ref point entry layout ---- */
#define REF_NAME                 0x00
#define REF_X                    0x04
#define REF_Y                    0x08
#define REF_Z                    0x0C

/* ---- App offsets (for sound) ---- */
#define APP_SOUNDS_PTR           0x178  /* App+0x178 = Graphics/Sounds ptr */
#define APP_ZIP_SOUND_CHANNEL    0x4CC  /* App+0x4CC = zip SoundChannel* (loaded in TimerDisplay) */
#define SOUND_PLAY_3D            0x00459860  /* __thiscall(ECX=soundChannel, float x, float y, float z), RET 0x10 */

/* Sound_Play3D wrapper — implemented in portal_sound.asm to avoid GCC
 * inline asm register constraints issues. */
extern void __cdecl portal_play_zip(DWORD soundChannel, float x, float y, float z);

static void call_sound_play3d(DWORD soundChannel, float x, float y, float z) {
    if (!soundChannel) return;
    if (IsBadReadPtr((void*)soundChannel, 0x20)) return;
    portal_play_zip(soundChannel, x, y, z);
}

/* ================================================================ */
/* BASS PROXY (v3 lazy loader)                                       */
/* ================================================================ */

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
    return FALSE;
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

/* ================================================================ */
/* UTILITY                                                           */
/* ================================================================ */

static void patch_bytes(void *addr, const void *data, DWORD size) {
    DWORD old;
    VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &old);
    memcpy(addr, data, size);
    VirtualProtect(addr, size, old, &old);
}

static void *alloc_executable(DWORD size) {
    return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}

static inline float portal_sqrtf(float x) {
    float result;
    __asm__ __volatile__("sqrtss %1, %0" : "=x"(result) : "x"(x));
    return result;
}

/* ---- Minimal string helpers (no CRT dependency needed) ---- */

static int my_strnicmp(const char *a, const char *b, int n) {
    for (int i = 0; i < n; i++) {
        char ca = a[i], cb = b[i];
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return ca - cb;
        if (ca == 0) return 0;
    }
    return 0;
}

static int my_stricmp(const char *a, const char *b) {
    while (*a && *b) {
        char ca = *a, cb = *b;
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

static int my_atoi(const char *s) {
    int result = 0;
    while (*s >= '0' && *s <= '9') {
        result = result * 10 + (*s - '0');
        s++;
    }
    return result;
}

static int my_snprintf(char *buf, int size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    typedef int (WINAPI *wvsprintfA_t)(LPSTR, LPCSTR, va_list);
    static wvsprintfA_t pWvsprintfA = NULL;
    if (!pWvsprintfA) {
        HMODULE h = GetModuleHandleA("user32.dll");
        if (h) pWvsprintfA = (wvsprintfA_t)GetProcAddress(h, "wvsprintfA");
    }
    if (!pWvsprintfA) { buf[0] = 0; va_end(args); return 0; }
    int result = pWvsprintfA(buf, fmt, args);
    va_end(args);
    if (result >= size) { buf[size-1] = 0; result = size - 1; }
    return result;
}

/* ================================================================ */
/* PORTAL LOGIC                                                      */
/* ================================================================ */

static int g_cooldowns[4] = {0, 0, 0, 0};
static int g_dceCallCount = 0;

/* Look up a ref point by name from the level's S1 ref point table.
 * level+0x480 = sceneobj
 * sceneobj+0x898 = count
 * *(sceneobj+0xCA0) = double-deref to array of entry pointers
 * Each entry: [0]=name ptr, [4]=x(float), [8]=y(float), [0xC]=z(float) */
static int lookupRefPoint(DWORD levelPtr, const char *name, float *outX, float *outY, float *outZ) {
    if (IsBadReadPtr((void*)(levelPtr + LEVEL_SCENEOBJ), 4)) return 0;
    DWORD sceneobj = *(DWORD*)(levelPtr + LEVEL_SCENEOBJ);
    if (!sceneobj || sceneobj < 0x10000) return 0;

    if (IsBadReadPtr((void*)(sceneobj + SCENEOBJ_COUNT), 4)) return 0;
    int count = *(int*)(sceneobj + SCENEOBJ_COUNT);
    if (count <= 0 || count > 10000) return 0;

    if (IsBadReadPtr((void*)(sceneobj + SCENEOBJ_ARRAY), 4)) return 0;
    DWORD arrayBase = *(DWORD*)(sceneobj + SCENEOBJ_ARRAY);
    if (!arrayBase || arrayBase < 0x10000) return 0;

    DWORD *entries = (DWORD*)arrayBase;
    if (!entries || IsBadReadPtr(entries, count * 4)) return 0;

    for (int i = 0; i < count; i++) {
        DWORD entry = entries[i];
        if (!entry || entry < 0x10000) continue;
        if (IsBadReadPtr((void*)entry, 16)) continue;

        char *entryName = *(char**)(entry + REF_NAME);
        if (!entryName || IsBadReadPtr(entryName, 64)) continue;

        if (my_stricmp(entryName, name) == 0) {
            *outX = *(float*)(entry + REF_X);
            *outY = *(float*)(entry + REF_Y);
            *outZ = *(float*)(entry + REF_Z);
            return 1;
        }
    }
    return 0;
}

/* ================================================================ */
/* DCE HANDLER (called from raw byte stub)                           */
/*                                                                  */
/* DCE is __thiscall: ECX=board, stack: [ball, collEntry], RET 0x8   */
/* Collision entry pair: pair[0]=type/board, pair[1]=MeshBuffer      */
/* Event name at: *(MeshBuffer + 0x864) = char* pointer             */
/* ================================================================ */

void __cdecl dce_handler(DWORD board, DWORD ball, DWORD collEntry) {
    g_dceCallCount++;
    if (!collEntry) return;

    DWORD *pair = (DWORD*)collEntry;
    if (IsBadReadPtr(pair, 8)) return;
    if (!pair[1]) return;
    if (IsBadReadPtr((void*)pair[1], 0x868)) return;

    /* Get event name from MeshBuffer+0x864 (char* pointer, not inline) */
    DWORD namePtr = *(DWORD*)((BYTE*)pair[1] + 0x864);
    if (!namePtr || IsBadReadPtr((void*)namePtr, 1)) return;
    const char *eventName = (const char*)namePtr;
    if (!eventName[0]) return;

    /* Check for E:PORTAL( */
    if (my_strnicmp(eventName, "E:PORTAL(", 9) != 0) return;

    int portalNum = my_atoi(eventName + 9);
    if (portalNum < 1) return;

    /* Get scene -> level ptr */
    DWORD sceneAddr = *(DWORD*)SCENE_PTR;
    if (!sceneAddr || sceneAddr < 0x10000) return;
    if (IsBadReadPtr((void*)(sceneAddr + SCENE_LEVEL_PTR), 4)) return;
    DWORD levelPtr = *(DWORD*)(sceneAddr + SCENE_LEVEL_PTR);
    if (!levelPtr || levelPtr < 0x10000) return;

    /* Look up PORTALPOS(N) */
    char posName[64];
    my_snprintf(posName, sizeof(posName), "PORTALPOS(%d)", portalNum);
    float destX, destY, destZ;
    if (!lookupRefPoint(levelPtr, posName, &destX, &destY, &destZ)) return;

    /* Look up PORTALVEC(N) for exit direction */
    char vecName[64];
    my_snprintf(vecName, sizeof(vecName), "PORTALVEC(%d)", portalNum);
    float vecX = 0, vecY = 0, vecZ = 0;
    int hasVec = lookupRefPoint(levelPtr, vecName, &vecX, &vecY, &vecZ);

    /* Read current velocity to preserve speed magnitude */
    char *ballPtr = (char*)ball;
    int playerID = *(int*)(ballPtr + BALL_PLAYER_ID);
    if (playerID < 0 || playerID >= 4) return;
    if (g_cooldowns[playerID] > 0) return;

    DWORD physPtr = *(DWORD*)(ballPtr + BALL_PHYS_PTR);
    float velX = 0, velY = 0, velZ = 0;
    float currentSpeed = 0;

    if (physPtr && !IsBadReadPtr((void*)physPtr, 0xCB0)) {
        velX = *(float*)(physPtr + PHYS_VEL_X);
        velY = *(float*)(physPtr + PHYS_VEL_Y);
        velZ = *(float*)(physPtr + PHYS_VEL_Z);
        currentSpeed = portal_sqrtf(velX*velX + velY*velY + velZ*velZ);
    }

    /* Use native teleport flag system */
    *(BYTE*)(ballPtr + BALL_TELEPORT_FLAG) = 1;
    *(float*)(ballPtr + BALL_TELEPORT_X) = destX;
    *(float*)(ballPtr + BALL_TELEPORT_Y) = destY + *(float*)(ballPtr + BALL_RADIUS) + 1.0f;
    *(float*)(ballPtr + BALL_TELEPORT_Z) = destZ;

    /* Set exit velocity */
    if (physPtr && !IsBadReadPtr((void*)physPtr, 0xCB0)) {
        if (hasVec) {
            /* Direction = PORTALVEC - PORTALPOS, normalized */
            float dx = vecX - destX;
            float dy = vecY - destY;
            float dz = vecZ - destZ;
            float len = portal_sqrtf(dx*dx + dy*dy + dz*dz);

            if (len > 0.001f) {
                float scale = currentSpeed / len;
                *(float*)(physPtr + PHYS_VEL_X) = dx * scale;
                *(float*)(physPtr + PHYS_VEL_Y) = dy * scale;
                *(float*)(physPtr + PHYS_VEL_Z) = dz * scale;
            }
        } else {
            /* No PORTALVEC — keep input direction, preserve speed */
            *(float*)(physPtr + PHYS_VEL_X) = velX;
            *(float*)(physPtr + PHYS_VEL_Y) = velY;
            *(float*)(physPtr + PHYS_VEL_Z) = velZ;
        }
    }

    g_cooldowns[playerID] = 30;

    /* Play zip sound at destination */
    DWORD app = *(DWORD*)APP_PTR;
    if (app && !IsBadReadPtr((void*)app, 0x600)) {
        DWORD zipChannel = *(DWORD*)(app + APP_ZIP_SOUND_CHANNEL);
        if (zipChannel && !IsBadReadPtr((void*)zipChannel, 0x20)) {
            call_sound_play3d(zipChannel, destX, destY, destZ);
        }
    }
}

/* ================================================================ */
/* RAW BYTE STUBS FOR DCE HOOK                                      */
/* ================================================================ */

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
    /* At entry: ECX=board, [ESP+4]=ball, [ESP+8]=collEntry, RET 0x8
     * pushad (32) + pushfd (4) = 36 bytes on stack
     *   [ESP+36] = return addr (from JMP)
     *   [ESP+40] = ball
     *   [ESP+44] = collEntry
     *
     * dce_handler is __cdecl(board, ball, collEntry)
     * Push in reverse order: collEntry, ball, board(ECX).
     */
    BYTE *code = (BYTE*)alloc_executable(64);
    int i = 0;
    code[i++] = 0x60;  /* pushad */
    code[i++] = 0x9C;  /* pushfd */
    /* push [esp+0x2C] (collEntry at offset 44=0x2C) */
    code[i++] = 0xFF; code[i++] = 0x74; code[i++] = 0x24; code[i++] = 0x2C;
    /* push [esp+0x2C] (ball was at 40=0x28, now shifted by 4 to 0x2C) */
    code[i++] = 0xFF; code[i++] = 0x74; code[i++] = 0x24; code[i++] = 0x2C;
    /* push ecx (board) */
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

static void install_dce_hook(void) {
    memcpy(g_dce_original, (void*)DISPATCH_COLLISION_EVENTS, 8);
    build_dce_trampoline();
    build_dce_stub();

    BYTE patch[8];
    patch[0] = 0xE9;
    *(DWORD*)(patch + 1) = (DWORD)g_dce_stub - DISPATCH_COLLISION_EVENTS - 5;
    patch[5] = 0x90;
    patch[6] = 0x90;
    patch[7] = 0x90;
    patch_bytes((void*)DISPATCH_COLLISION_EVENTS, patch, 8);
}

/* ================================================================ */
/* INIT THREAD                                                       */
/* ================================================================ */

static DWORD WINAPI init_thread(LPVOID param) {
    (void)param;
    Sleep(2000);
    install_dce_hook();
    return 0;
}

/* ================================================================ */
/* DLLMAIN                                                           */
/* ================================================================ */

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved) {
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInst);
        load_real_bass();
        CreateThread(NULL, 0, init_thread, NULL, 0, NULL);
    }
    return TRUE;
}
