/*
 * ghost_triggers.c — Ghost Ball Proximity Triggers
 *
 * Scans S1 ref points in level MESHWORLD data for entries prefixed "GT:".
 * Each frame, checks if the ghost ball (scene+0x361C) is within a trigger's
 * radius. When the ghost ball enters a trigger zone, logs the event.
 *
 * S1 ref point name format:  GT:EventName
 *   - The part after "GT:" is the event name to log (and eventually pass to DCE)
 *   - The S1 point's position (pos_x, pos_y, pos_z) defines the trigger center
 *   - The S1 point's rot_y field (puVar5[5]) is repurposed as the trigger radius
 *     (since rot_y is unused for invisible trigger points)
 *
 * S1 entry layout (DWORD array):
 *   [0] = char* name
 *   [1] = float pos_x
 *   [2] = float pos_y
 *   [3] = float pos_z
 *   [4] = float rot_x (unused)
 *   [5] = float rot_y → repurposed as trigger radius (default 50.0 if 0)
 *   [6] = float rot_z (unused)
 *
 * Hook: Frame epilogue at 0x46C1F1 (POP ESI / ADD ESP,8 / RET)
 *   - Runs once per frame after all game logic + rendering
 *   - Ghost ball position is fresh from PlaybackSnapshot
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll ghost_triggers.c \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* BASS type definitions */
typedef unsigned long long QWORD;
typedef DWORD HSTREAM;
typedef DWORD HMUSIC;
typedef DWORD HFX;

/* ---- Game addresses ---- */
#define EXE_BASE        0x00400000
#define APP_PTR         0x005341E0
#define APP_FRAME_UPDATE_EPILOGUE 0x0046C1F1

/* ---- App offsets ---- */
#define APP_178         0x178    /* App+0x178 = scene ptr */

/* ---- Scene offsets ---- */
#define SCENE_GHOST_BALL 0x361C   /* scene+0x361C = ghost Ball* */
#define SCENE_8AC        0x8AC    /* scene+0x8AC = Level* (SceneObject) */

/* ---- Level/SceneObject offsets ---- */
/* SceneObject+0x480 = pointer to the MESHWORLD data object */
/* That object+0x894 = AthenaList (EMBEDDED, not pointer!) containing S1 entries */
/* That object+0x898 = AthenaList count */
/* That object+0xCA0 = AthenaList list_array (DWORD*[]) */
#define MW_S1_LIST_OFFSET 0x894   /* offset from MW data object base */
#define MW_S1_COUNT_OFFSET 0x898
#define MW_S1_ARRAY_OFFSET 0xCA0

/* ---- S1 entry layout (each entry is a DWORD array) ---- */
#define S1_NAME    0    /* [0] = char* name */
#define S1_POS_X   1    /* [1] = float x */
#define S1_POS_Y   2    /* [2] = float y */
#define S1_POS_Z   3    /* [3] = float z */
#define S1_ROT_X   4    /* [4] = float rot_x (unused) */
#define S1_ROT_Y   5    /* [5] = float rot_y → repurposed as trigger radius */
#define S1_ROT_Z   6    /* [6] = float rot_z (unused) */
#define S1_SIZE    7    /* 7 DWORDs per entry = 28 bytes */

/* ---- Ball offsets ---- */
#define BALL_POS_X  0x164
#define BALL_POS_Y  0x168
#define BALL_POS_Z  0x16C

/* ---- Constants ---- */
#define MAX_TRIGGERS 64
#define DEFAULT_RADIUS 50.0f
#define TRIGGER_COOLDOWN_FRAMES 60   /* 1 second at 60fps */
#define TRIGGER_PREFIX "GT:"

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
#define LOG(...)   do {} while (0)
#define LOGS(s)    do {} while (0)
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
void __stdcall BASS_Free(void) { if (real_BASS_Free) real_BASS_Free(); }
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
void __stdcall BASS_Start(void) { if (real_BASS_Start) real_BASS_Start(); }
void __stdcall BASS_Stop(void) { if (real_BASS_Stop) real_BASS_Stop(); }
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
    if (!real_BASS_ChannelSetAttributes)
        real_BASS_ChannelSetAttributes = (void*)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttribute");
    real_BASS_MusicPlayEx      = (void*)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
    real_BASS_ChannelGetData  = (void*)GetProcAddress(g_hRealBass, "BASS_ChannelGetData");
    real_BASS_ChannelSetFX    = (void*)GetProcAddress(g_hRealBass, "BASS_ChannelSetFX");
    real_BASS_Start           = (void*)GetProcAddress(g_hRealBass, "BASS_Start");
    real_BASS_Stop            = (void*)GetProcAddress(g_hRealBass, "BASS_Stop");
    real_BASS_SetConfig       = (void*)GetProcAddress(g_hRealBass, "BASS_SetConfig");
    real_BASS_ErrorGetCode    = (void*)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
    real_BASS_ChannelStop     = (void*)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
}

/* ---- Utility ---- */
static void patch_bytes(void *addr, const void *data, DWORD size) {
    DWORD old;
    VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &old);
    memcpy(addr, data, size);
    VirtualProtect(addr, size, old, &old);
}
static void *alloc_executable(DWORD size) {
    return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}

/* ---- Trigger data ---- */
typedef struct {
    float pos_x, pos_y, pos_z;   /* trigger center (world space) */
    float radius;                /* trigger radius */
    float radius_sq;             /* radius squared (for distance check) */
    char  name[64];              /* event name (after "GT:" prefix) */
    int   inside;                /* 1 = ghost ball currently inside, 0 = outside */
    int   cooldown;              /* frames remaining before can retrigger */
} Trigger;

static Trigger g_triggers[MAX_TRIGGERS];
static int g_triggerCount = 0;
static int g_currentLevel = -1;     /* tracks which level we've scanned */
static int g_frameCount = 0;

/* ---- S1 ref point scanning ----
 *
 * The S1 AthenaList is EMBEDDED in the MW data object, NOT a pointer.
 * Access chain:
 *   scene+0x8AC  → Level (SceneObject*)
 *   Level+0x480  → MW data object (the parsed MeshWorld)
 *   MW+0x894     = AthenaList (embedded) — vtable at +0x894, count at +0x898
 *   MW+0xCA0     = list_array (DWORD**) — array of S1 entry pointers
 *
 * Each S1 entry is an array of DWORDs (see S1_* constants above).
 * The name at entry[0] is a char* — we strnicmp it for "GT:" prefix.
 */
static void scan_s1_ref_points(DWORD scene) {
    g_triggerCount = 0;

    if (!scene || IsBadReadPtr((void*)scene, 0x4000)) return;

    DWORD level = *(DWORD*)(scene + SCENE_8AC);
    if (!level || IsBadReadPtr((void*)level, 0x1000)) return;

    DWORD mwData = *(DWORD*)(level + 0x480);
    if (!mwData || IsBadReadPtr((void*)mwData, 0x1000)) return;

    /* AthenaList count at MW+0x898 (embedded list, NOT a pointer deref) */
    int s1Count = *(int*)(mwData + MW_S1_COUNT_OFFSET);
    DWORD s1Array = *(DWORD*)(mwData + MW_S1_ARRAY_OFFSET);

    LOG("scan_s1: scene=0x%X level=0x%X mw=0x%X s1Count=%d s1Array=0x%X",
        scene, level, mwData, s1Count, s1Array);

    if (s1Count <= 0 || s1Count > 1000) return;
    if (!s1Array || IsBadReadPtr((void*)s1Array, s1Count * 4)) return;

    for (int i = 0; i < s1Count && g_triggerCount < MAX_TRIGGERS; i++) {
        DWORD entry = *(DWORD*)(s1Array + i * 4);
        if (!entry || IsBadReadPtr((void*)entry, S1_SIZE * 4)) continue;

        char *name = *(char**)(entry + S1_NAME * 4);
        if (!name || IsBadReadPtr(name, 4)) continue;

        if (_strnicmp(name, TRIGGER_PREFIX, 3) == 0) {
            Trigger *t = &g_triggers[g_triggerCount];

            /* Extract event name (everything after "GT:") */
            const char *eventName = name + 3;
            strncpy(t->name, eventName, sizeof(t->name) - 1);
            t->name[sizeof(t->name) - 1] = '\0';

            /* Position from S1 entry */
            t->pos_x = *(float*)(entry + S1_POS_X * 4);
            t->pos_y = *(float*)(entry + S1_POS_Y * 4);
            t->pos_z = *(float*)(entry + S1_POS_Z * 4);

            /* Radius from rot_y field (repurposed). If 0, use default. */
            t->radius = *(float*)(entry + S1_ROT_Y * 4);
            if (t->radius <= 0.0f || t->radius > 5000.0f) t->radius = DEFAULT_RADIUS;
            t->radius_sq = t->radius * t->radius;

            t->inside = 0;
            t->cooldown = 0;

            LOG("Found trigger #%d: name='%s' pos=(%.1f,%.1f,%.1f) radius=%.1f",
                g_triggerCount, t->name, t->pos_x, t->pos_y, t->pos_z, t->radius);

            g_triggerCount++;
        }
    }

    LOG("scan_s1: found %d GT: triggers", g_triggerCount);
}

/* ---- Frame epilogue handler (called from raw byte stub) ----
 * Runs once per frame after all game logic + rendering.
 * Ghost ball position has been set by PlaybackSnapshot earlier in the frame.
 */
void __cdecl frame_epilogue_handler(void) {
    g_frameCount++;

    DWORD app = *(DWORD*)APP_PTR;
    if (!app || IsBadReadPtr((void*)app, 0x1000)) return;

    DWORD scene = *(DWORD*)(app + APP_178);
    if (!scene || IsBadReadPtr((void*)scene, 0x4000)) {
        /* Scene not loaded — reset trigger scan */
        if (g_currentLevel != -1) {
            g_currentLevel = -1;
            g_triggerCount = 0;
        }
        return;
    }

    /* Detect level change by tracking scene pointer.
     * When scene changes, re-scan S1 ref points. */
    if ((int)scene != g_currentLevel) {
        g_currentLevel = (int)scene;
        scan_s1_ref_points(scene);
    }

    if (g_triggerCount == 0) return;

    /* Get ghost ball */
    DWORD ghostBall = *(DWORD*)(scene + SCENE_GHOST_BALL);
    if (!ghostBall || IsBadReadPtr((void*)ghostBall, 0x200)) return;

    /* Read ghost ball position */
    float bx = *(float*)(ghostBall + BALL_POS_X);
    float by = *(float*)(ghostBall + BALL_POS_Y);
    float bz = *(float*)(ghostBall + BALL_POS_Z);

    /* Check each trigger */
    for (int i = 0; i < g_triggerCount; i++) {
        Trigger *t = &g_triggers[i];

        /* Decrement cooldown */
        if (t->cooldown > 0) t->cooldown--;

        /* Squared distance from ghost ball to trigger center */
        float dx = bx - t->pos_x;
        float dy = by - t->pos_y;
        float dz = bz - t->pos_z;
        float dist_sq = dx*dx + dy*dy + dz*dz;

        int wasInside = t->inside;
        t->inside = (dist_sq < t->radius_sq) ? 1 : 0;

        /* Trigger on entry (was outside, now inside) */
        if (!wasInside && t->inside && t->cooldown == 0) {
            float dist = sqrtf(dist_sq);
            LOG("TRIGGER FIRED: '%s' — ghost ball entered zone at (%.1f, %.1f, %.1f), dist=%.1f, radius=%.1f, frame=%d",
                t->name, bx, by, bz, dist, t->radius, g_frameCount);
            t->cooldown = TRIGGER_COOLDOWN_FRAMES;
        }
    }
}

/* ---- Raw byte stub for frame epilogue hook ----
 * Replaces 5 bytes at 0x46C1F1: 5E 83 C4 08 C3 (POP ESI / ADD ESP,8 / RET)
 * Saves all registers, calls handler, restores, executes original 5 bytes, RET
 */
static BYTE *g_frame_stub = NULL;

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

static void install_frame_hook(void) {
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
    LOGS("ghost_triggers mod initializing");
    install_frame_hook();
    LOGS("ghost_triggers mod initialized — scanning for GT: ref points");
    return 0;
}

/* ---- DllMain ---- */
BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInst);

#ifdef ENABLE_LOGGING
        char modPath[MAX_PATH];
        GetModuleFileNameA(hInst, modPath, MAX_PATH);
        char *p = strrchr(modPath, '\\');
        if (p) {
            strcpy(p + 1, "ghost_triggers_log.txt");
            strncpy(g_logPath, modPath, MAX_PATH - 1);
        }
#endif

        load_real_bass();
        LOG("DllMain: bass_real=%s, log=%s", g_hRealBass ? "OK" : "FAIL", g_logPath);
        CreateThread(NULL, 0, init_thread, NULL, 0, NULL);
    }
    return TRUE;
}
