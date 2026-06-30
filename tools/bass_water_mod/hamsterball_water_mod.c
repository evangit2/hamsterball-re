/*
 * hamsterball_water_mod.c — v2: Velocity-based water physics via Phase 15 hook.
 *
 * BUILD (Linux -> Windows): make
 *
 * INSTALLATION (Windows):
 *   1. In the Hamsterball game folder rename original bass.dll -> bass_real.dll
 *   2. Copy this proxy bass.dll + hamsterball_water.ini into the game folder
 *   3. Place invisible collision planes named "E:WATER" in custom levels, OR
 *      set water plane Y coordinates in the INI file
 *   4. Run Hamsterball.exe normally
 *
 * HOW IT WORKS (v2 — FIXED):
 *   - Forwards all BASS audio calls to bass_real.dll.
 *   - Hooks Ball_Update at Phase 15 (0x407BB4) via code cave — same proven
 *     approach as the jump mod and power bounce mod.
 *   - The code cave calls a C helper function that:
 *       1. Reads the ball's position and velocity from the physics struct
 *       2. Checks if the ball is touching/inside a water plane
 *       3. If in water, modifies VELOCITY directly:
 *          a. Entry damping (scale velocity_Y on first contact while falling)
 *          b. Drag (scale all velocity axes per frame)
 *          c. Horizontal drag (extra scaling on X/Z only)
 *          d. Buoyancy (add upward velocity proportional to submersion depth)
 *       4. Writes modified velocity back to the physics struct
 *   - The velocity changes persist and affect next frame's position integration,
 *     which is correct physics: forces modify velocity, velocity modifies position.
 *
 * FIXES FROM v1:
 *   - Modifies VELOCITY (phys+0xCA4/CA8/CAC) instead of position delta
 *     → drag/buoyancy actually affect the ball's momentum, not just visual position
 *   - Fixes ball+0x14 = Board (not Scene) — original mod mislabeled this
 *   - Uses Phase 15 code cave (proven approach from jump/power_bounce mods)
 *     instead of vtable hook with save-call-modify pattern
 *   - Buoyancy is now velocity-based (acceleration), not position offset
 *     → ball reaches stable float instead of oscillating
 *   - Entry damping reduces velocity (persists), not position delta (one-frame)
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll hamsterball_water_mod.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports — forward all game imports to bass_real.dll
 * ═══════════════════════════════════════════════════════════════════════════ */

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

/* Extra stubs for completeness */
__declspec(dllexport) void __stdcall BASS_Pause(void) {}
__declspec(dllexport) void __stdcall BASS_SetVolume(DWORD a) {}
__declspec(dllexport) DWORD __stdcall BASS_GetVolume(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_GetDevice(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_SetDevice(DWORD a) { return 1; }
__declspec(dllexport) void __stdcall BASS_GetInfo(void *a) {}
__declspec(dllexport) int __stdcall BASS_Update(DWORD a) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateFile(void *a, void* b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(int a, void* b, DWORD c, DWORD d, DWORD e) { return 0; }
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

/* ═══════════════════════════════════════════════════════════════════════════
 * Diagnostic logging
 * ═══════════════════════════════════════════════════════════════════════════ */

static char g_logpath[MAX_PATH] = "";

static void diag_log(const char *msg)
{
    if (g_logpath[0] == '\0') return;
    HANDLE hFile = CreateFileA(g_logpath, FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        SetFilePointer(hFile, 0, NULL, FILE_END);
        WriteFile(hFile, msg, (DWORD)strlen(msg), &written, NULL);
        WriteFile(hFile, "\r\n", 2, &written, NULL);
        CloseHandle(hFile);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Memory layout constants (verified against Hamsterball.exe via Ghidra)
 * ═══════════════════════════════════════════════════════════════════════════ */

#define IMAGE_BASE              0x00400000

/* Hook address — Phase 15 in Ball_Update, same as jump mod + power bounce */
#define PHASE15_HOOK            0x00407BB4
#define PHASE15_ORIG_BYTES      6
/* Original bytes: 8B 4C 24 1C 8B 11 = MOV ECX,[ESP+1C]; MOV EDX,[ECX] */
/* At this point ESI = ball pointer */

/* Ball struct offsets */
#define BALL_BOARD              0x014   /* Board* (NOT Scene — ball+0x14 is the Board) */
#define BALL_POS_X              0x164
#define BALL_POS_Y              0x168
#define BALL_POS_Z              0x16C
#define BALL_FORCE_Y            0x174   /* Y force accumulator (used by jump mod) */
#define BALL_PHYS_PTR           0x1A4   /* Physics struct pointer */
#define BALL_RADIUS             0x284
#define BALL_PLAYER_IDX         0x018   /* int: 0-3 = player, -1 = NPC */

/* Physics struct offsets (at ball+0x1A4) */
#define PHYS_VEL_X              0xCA4
#define PHYS_VEL_Y              0xCA8
#define PHYS_VEL_Z              0xCAC

/* Board struct offsets */
#define BOARD_SCENE             0x878   /* Scene* back-pointer */
#define BOARD_COLLISION_MESH    0x8B0   /* collision mesh data (for raycasting) */

/* AthenaList layout */
#define ATHENA_COUNT_OFFSET     0x04
#define ATHENA_DATA_OFFSET      0x40C

/* MeshBuffer struct offsets */
#define OFF_MB_FACE_LIST        0x0C
#define OFF_MB_NAME              0x864

/* CollisionFace struct offsets */
#define OFF_FACE_V0_Y           0x04

/* ═══════════════════════════════════════════════════════════════════════════
 * Config
 * ═══════════════════════════════════════════════════════════════════════════ */

#define MAX_WATER_PLANES 16

typedef struct {
    float entry_damping;       /* velocity multiplier on first contact while falling (0-1) */
    float drag;                /* per-frame velocity drag on all axes (0-1) */
    float horizontal_drag;     /* extra drag on X/Z axes (0-1) */
    float buoyancy_strength;   /* upward acceleration per frame at full submersion */
    int   debug;               /* write log file */
    int   plane_count;         /* fallback water planes from INI */
    float plane_y[MAX_WATER_PLANES];
} water_cfg_t;

static water_cfg_t g_cfg = {
    0.70f,   /* entry_damping: velocity reduced to 70% on first contact */
    0.03f,   /* drag: 3% velocity reduction per frame on all axes */
    0.04f,   /* horizontal_drag: extra 4% on X/Z */
    0.45f,   /* buoyancy_strength: upward acceleration at full submersion */
    1,       /* debug */
    0,       /* plane_count */
    {0}      /* plane_y */
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Water plane cache
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    float surface_y;
    int   found;
} water_plane_t;

static water_plane_t g_planes[MAX_WATER_PLANES];
static int g_plane_count = 0;
static int g_scan_done = 0;
static DWORD g_last_board = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * Per-ball water state
 * ═══════════════════════════════════════════════════════════════════════════ */

#define MAX_BALLS 32

typedef struct {
    DWORD ball;              /* ball pointer (key); 0 = unused */
    int   in_water;          /* currently in water? */
    int   entry_damped;      /* already applied entry damping this water session? */
} water_state_t;

static water_state_t g_states[MAX_BALLS];

static water_state_t *get_ball_state(DWORD ball)
{
    int free_idx = -1;
    for (int i = 0; i < MAX_BALLS; i++) {
        if (g_states[i].ball == ball) return &g_states[i];
        if (free_idx == -1 && g_states[i].ball == 0) free_idx = i;
    }
    if (free_idx >= 0) {
        memset(&g_states[free_idx], 0, sizeof(water_state_t));
        g_states[free_idx].ball = ball;
        return &g_states[free_idx];
    }
    return NULL;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * INI config loading
 * ═══════════════════════════════════════════════════════════════════════════ */

static float read_ini_float(const char *path, const char *section, const char *key, float def)
{
    char buf[64];
    DWORD n = GetPrivateProfileStringA(section, key, "", buf, sizeof(buf), path);
    if (n == 0) return def;
    return (float)atof(buf);
}

static int read_ini_int(const char *path, const char *section, const char *key, int def)
{
    char buf[32];
    DWORD n = GetPrivateProfileStringA(section, key, "", buf, sizeof(buf), path);
    if (n == 0) return def;
    return atoi(buf);
}

static void load_config(const char *ini_path)
{
    DWORD attr = GetFileAttributesA(ini_path);
    if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY)) return;

    g_cfg.entry_damping     = read_ini_float(ini_path, "WaterPhysics", "EntryDamping", 0.70f);
    g_cfg.drag              = read_ini_float(ini_path, "WaterPhysics", "Drag", 0.03f);
    g_cfg.horizontal_drag   = read_ini_float(ini_path, "WaterPhysics", "HorizontalDrag", 0.04f);
    g_cfg.buoyancy_strength = read_ini_float(ini_path, "WaterPhysics", "BuoyancyStrength", 0.45f);
    g_cfg.debug             = read_ini_int(ini_path, "WaterPhysics", "Debug", 1);

    if (g_cfg.entry_damping < 0.0f) g_cfg.entry_damping = 0.0f;
    if (g_cfg.entry_damping > 1.0f) g_cfg.entry_damping = 1.0f;
    if (g_cfg.drag < 0.0f) g_cfg.drag = 0.0f;
    if (g_cfg.drag > 1.0f) g_cfg.drag = 1.0f;
    if (g_cfg.horizontal_drag < 0.0f) g_cfg.horizontal_drag = 0.0f;
    if (g_cfg.horizontal_drag > 1.0f) g_cfg.horizontal_drag = 1.0f;

    g_cfg.plane_count = read_ini_int(ini_path, "WaterPlanes", "Count", 0);
    if (g_cfg.plane_count < 0) g_cfg.plane_count = 0;
    if (g_cfg.plane_count > MAX_WATER_PLANES) g_cfg.plane_count = MAX_WATER_PLANES;
    for (int i = 0; i < g_cfg.plane_count; i++) {
        char key[16];
        wsprintfA(key, "Y%d", i);
        g_cfg.plane_y[i] = read_ini_float(ini_path, "WaterPlanes", key, 0.0f);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Water plane discovery
 *
 * Scans the Board's collision mesh for objects named "E:WATER".
 * Falls back to INI-configured Y coordinates if no E:WATER objects found.
 *
 * ball+0x14 = Board (NOT Scene — this was a bug in v1)
 * board+0x8B0 = collision mesh data (used by Mesh_FindClosestCollision)
 * ═══════════════════════════════════════════════════════════════════════════ */

static int athena_get_count(void *list_ptr)
{
    if (!list_ptr || IsBadReadPtr(list_ptr, 0x414)) return 0;
    return *(int *)((char *)list_ptr + ATHENA_COUNT_OFFSET);
}

static void **athena_get_data(void *list_ptr)
{
    if (!list_ptr || IsBadReadPtr(list_ptr, 0x414)) return NULL;
    void **data = *(void ***)((char *)list_ptr + ATHENA_DATA_OFFSET);
    if (!data || IsBadReadPtr(data, sizeof(void*))) return NULL;
    return data;
}

static float read_meshbuffer_face_y(void *meshbuf)
{
    if (!meshbuf || IsBadReadPtr(meshbuf, 0x874)) return 0.0f;

    void *face_list = (char *)meshbuf + OFF_MB_FACE_LIST;
    int face_count = athena_get_count(face_list);
    if (face_count < 1) return 0.0f;

    void **face_data = athena_get_data(face_list);
    if (!face_data) return 0.0f;

    void *first_face = face_data[0];
    if (!first_face || IsBadReadPtr(first_face, 0x60)) return 0.0f;

    return *(float *)((char *)first_face + OFF_FACE_V0_Y);
}

static const char *get_meshbuffer_name(void *meshbuf)
{
    if (!meshbuf || IsBadReadPtr(meshbuf, 0x874)) return NULL;
    char *name = *(char **)((char *)meshbuf + OFF_MB_NAME);
    if (!name || IsBadReadPtr(name, 1)) return NULL;
    return name;
}

static int is_water_name(const char *name)
{
    if (!name) return 0;
    return (_strnicmp(name, "E:WATER", 7) == 0);
}

/* Scan a MeshWorld's object list for E:WATER objects */
static int scan_object_list(void *meshworld, const char *label)
{
    if (!meshworld || IsBadReadPtr(meshworld, 0x488)) {
        diag_log("scan: meshworld is NULL or unreadable");
        return 0;
    }

    void *obj_list = (char *)meshworld + 0x2C;  /* AthenaList object_list */
    int count = athena_get_count(obj_list);
    void **data = athena_get_data(obj_list);

    if (count < 1 || !data) return 0;

    int found = 0;
    for (int i = 0; i < count && found < MAX_WATER_PLANES; i++) {
        if (IsBadReadPtr(&data[i], sizeof(void*))) break;
        void *obj = data[i];
        if (!obj || IsBadReadPtr(obj, 0x874)) continue;

        const char *name = get_meshbuffer_name(obj);
        if (!name) continue;

        if (is_water_name(name)) {
            float face_y = read_meshbuffer_face_y(obj);
            if (found < MAX_WATER_PLANES) {
                g_planes[found].surface_y = face_y;
                g_planes[found].found = 1;
                found++;
            }
        }
    }
    return found;
}

static void scan_for_water_planes(DWORD board)
{
    memset(g_planes, 0, sizeof(g_planes));
    g_plane_count = 0;

    if (!board || IsBadReadPtr((void*)board, 0x1000)) {
        diag_log("scan: board is NULL or unreadable");
        g_scan_done = 1;
        return;
    }

    /* Try CollisionLevel at Board+0x8B0 → +0x08 as collision MeshWorld */
    int found = 0;
    DWORD collision_level = *(DWORD*)(board + BOARD_COLLISION_MESH);
    if (collision_level && !IsBadReadPtr((void*)collision_level, 0x10D0)) {
        DWORD cl_mw = *(DWORD*)(collision_level + 0x08);
        if (cl_mw && !IsBadReadPtr((void*)cl_mw, 0x488)) {
            found = scan_object_list((void*)cl_mw, "collvl+0x08");
        }
    }

    g_plane_count = found;

    /* Fallback to INI-configured planes */
    if (g_plane_count == 0 && g_cfg.plane_count > 0) {
        for (int i = 0; i < g_cfg.plane_count && g_plane_count < MAX_WATER_PLANES; i++) {
            g_planes[g_plane_count].surface_y = g_cfg.plane_y[i];
            g_planes[g_plane_count].found = 1;
            g_plane_count++;
        }
    }

    {
        char buf[256];
        wsprintfA(buf, "scan_for_water_planes: board=%08X found=%d (ini_fallback=%d)",
                  board, g_plane_count, g_plane_count > 0 ? 0 : (g_cfg.plane_count > 0));
        diag_log(buf);
    }

    g_scan_done = 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Water physics — velocity modification
 *
 * Called from the Phase 15 code cave via C function pointer.
 * Receives the ball pointer (ESI at the hook point).
 *
 * Modifies velocity directly in the physics struct (phys+0xCA4/CA8/CAC).
 * These changes persist and affect next frame's position integration,
 * which is correct physics: forces modify velocity, velocity modifies position.
 *
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Compute submersion fraction: 0 = just touching, 0.5 = half, 1 = fully under.
 * Engine uses Y-up: positive Y is up, gravity pulls toward -Y.
 * Bottom of ball (deepest) = ball_y - radius.
 * Top of ball (shallowest) = ball_y + radius. */
static float compute_submersion(float ball_y, float radius, float surface_y)
{
    float bottom_y = ball_y - radius;
    float top_y = ball_y + radius;

    if (bottom_y >= surface_y) return 0.0f;
    if (top_y <= surface_y) return 1.0f;

    float submerged = (surface_y - bottom_y) / (2.0f * radius);
    if (submerged < 0.0f) submerged = 0.0f;
    if (submerged > 1.0f) submerged = 1.0f;
    return submerged;
}

/* Check if ball is in water. Returns surface_y or -99999.0f if not in water. */
static float check_in_water(float ball_y, float radius)
{
    float bottom_y = ball_y - radius;
    for (int i = 0; i < g_plane_count; i++) {
        if (!g_planes[i].found) continue;
        if (bottom_y < g_planes[i].surface_y) {
            return g_planes[i].surface_y;
        }
    }
    return -99999.0f;
}

/* Main water physics function — called from Phase 15 code cave.
 * Uses only integer math to check conditions, then float math to modify velocity.
 * Safe to call from code cave with PUSHAD/PUSHFD saved state (same pattern as
 * power_bounce mod's check_power_bounce). */
static void __cdecl apply_water_physics(DWORD ball)
{
    if (!ball || IsBadReadPtr((void*)ball, 0x300)) return;
    if (g_plane_count == 0) return;

    /* Read ball position and radius */
    float ball_y = *(float*)(ball + BALL_POS_Y);
    float radius = *(float*)(ball + BALL_RADIUS);
    if (radius <= 0.0f || radius > 1000.0f) return;  /* sanity check */

    /* Check if ball is in water */
    float surface_y = check_in_water(ball_y, radius);
    if (surface_y < -99998.0f) {
        /* Not in water — clear state if was previously in water */
        water_state_t *st = get_ball_state(ball);
        if (st && st->in_water) {
            st->in_water = 0;
            st->entry_damped = 0;
        }
        return;
    }

    /* Get physics struct */
    DWORD phys = *(DWORD*)(ball + BALL_PHYS_PTR);
    if (!phys || IsBadReadPtr((void*)phys, 0xCB0)) return;

    /* Get velocity pointers */
    float *vel_x = (float*)(phys + PHYS_VEL_X);
    float *vel_y = (float*)(phys + PHYS_VEL_Y);
    float *vel_z = (float*)(phys + PHYS_VEL_Z);

    /* Get per-ball state */
    water_state_t *st = get_ball_state(ball);
    if (!st) return;

    /* Just entered water */
    if (!st->in_water) {
        st->in_water = 1;
        st->entry_damped = 0;
    }

    /* Compute submersion depth */
    float submersion = compute_submersion(ball_y, radius, surface_y);
    if (submersion <= 0.0f) {
        if (st->in_water) {
            st->in_water = 0;
            st->entry_damped = 0;
        }
        return;
    }

    /* 1. Entry damping: on first contact while falling (vel_y < 0 = downward in Y-up),
     *    reduce vertical velocity by entry_damping factor.
     *    This modifies VELOCITY (persists across frames), not position delta. */
    if (!st->entry_damped && *vel_y < 0.0f) {
        *vel_y *= g_cfg.entry_damping;
        st->entry_damped = 1;
    }

    /* 2. Drag: reduce all velocity components per frame.
     *    This is a velocity scale, so it accumulates — the ball actually slows down. */
    float vscale = 1.0f - g_cfg.drag;
    float hscale = 1.0f - (g_cfg.drag + g_cfg.horizontal_drag);
    if (vscale < 0.0f) vscale = 0.0f;
    if (hscale < 0.0f) hscale = 0.0f;

    /* 3. Horizontal drag: extra velocity reduction on X and Z axes.
     *    Makes horizontal movement sluggish in water. */
    *vel_x *= hscale;
    *vel_z *= hscale;

    /* 4. Buoyancy: upward acceleration proportional to submersion depth.
     *    At submerged=0 (just touching): no buoyancy, full gravity.
     *    At submerged=0.5 (half): buoyancy = buoyancy_strength, roughly cancels gravity.
     *    At submerged=1.0 (fully under): buoyancy = 2*bouyancy_strength, net upward.
     *
     *    This is added to VELOCITY (acceleration), so the ball decelerates going down,
     *    stops, then accelerates upward — reaching a stable float at the surface. */
    float buoyancy = g_cfg.buoyancy_strength * submersion * 2.0f;
    *vel_y *= vscale;       /* apply drag to Y first */
    *vel_y += buoyancy;      /* then add buoyancy as upward acceleration */
}

/* Function pointer for code cave to call our C helper */
static void (__cdecl *g_water_fn_ptr)(DWORD) = NULL;

/* ═══════════════════════════════════════════════════════════════════════════
 * Phase 15 Code Cave (0x407BB4)
 *
 * Original 6 bytes: 8B 4C 24 1C 8B 11 = MOV ECX,[ESP+1C]; MOV EDX,[ECX]
 * At this point: ESI = ball pointer
 *
 * The cave:
 *   1. Saves all registers (PUSHAD + PUSHFD)
 *   2. Stores ESI (ball) for water plane scanning
 *   3. Scans for water planes if not done or board changed
 *   4. Calls apply_water_physics(ESI) — modifies velocity in phys struct
 *   5. Restores all registers (POPFD + POPAD)
 *   6. Executes original 6 bytes
 *   7. Jumps back to hook_addr + 6
 *
 * Same pattern as power_bounce mod's collision cave.
 * ═══════════════════════════════════════════════════════════════════════════ */

static BYTE *g_phase15_cave = NULL;
static volatile DWORD g_ball_ptr = 0;
static volatile DWORD g_hook_calls = 0;

static void install_phase15_hook(void)
{
    BYTE *hook_addr = (BYTE*)PHASE15_HOOK;
    char buf[256];

    BYTE expected[] = { 0x8B, 0x4C, 0x24, 0x1C, 0x8B, 0x11 };
    wsprintfA(buf, "Phase15 bytes: %02X %02X %02X %02X %02X %02X",
              hook_addr[0], hook_addr[1], hook_addr[2],
              hook_addr[3], hook_addr[4], hook_addr[5]);
    diag_log(buf);

    if (memcmp(hook_addr, expected, PHASE15_ORIG_BYTES) != 0) {
        diag_log("PHASE15 BYTE MISMATCH!");
        return;
    }

    g_phase15_cave = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE,
                                           PAGE_EXECUTE_READWRITE);
    if (!g_phase15_cave) { diag_log("phase15: VirtualAlloc FAILED"); return; }

    int p = 0;

    /* PUSHAD (save all general-purpose registers) */
    g_phase15_cave[p++] = 0x60;

    /* PUSHFD (save flags) */
    g_phase15_cave[p++] = 0x9C;

    /* MOV [g_ball_ptr], ESI — save ball pointer for water plane scanning */
    g_phase15_cave[p++] = 0x89; g_phase15_cave[p++] = 0x35;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_ball_ptr; p += 4;

    /* Check if we need to scan for water planes:
     * CMP DWORD [g_scan_done], 0
     * JNE .skip_scan
     * (also check if board changed)
     *
     * Actually, let's just call scan every time the board changes.
     * We check g_last_board vs current board. But reading ball+0x14
     * requires the ball pointer which is in ESI. We already saved it.
     * Let's just do the scan check in C. */

    /* PUSH ESI (arg: ball pointer) */
    g_phase15_cave[p++] = 0x56;

    /* CALL [g_water_fn_ptr] — calls apply_water_physics(ball) */
    g_phase15_cave[p++] = 0xFF; g_phase15_cave[p++] = 0x15;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_water_fn_ptr; p += 4;

    /* ADD ESP, 4 (cdecl cleanup: 1 DWORD arg) */
    g_phase15_cave[p++] = 0x83; g_phase15_cave[p++] = 0xC4;
    g_phase15_cave[p++] = 0x04;

    /* INC DWORD [g_hook_calls] */
    g_phase15_cave[p++] = 0xFF; g_phase15_cave[p++] = 0x05;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_hook_calls; p += 4;

    /* POPFD (restore flags) */
    g_phase15_cave[p++] = 0x9D;

    /* POPAD (restore all registers) */
    g_phase15_cave[p++] = 0x61;

    /* Original 6 bytes: MOV ECX,[ESP+1C]; MOV EDX,[ECX] */
    g_phase15_cave[p++] = 0x8B; g_phase15_cave[p++] = 0x4C;
    g_phase15_cave[p++] = 0x24; g_phase15_cave[p++] = 0x1C;
    g_phase15_cave[p++] = 0x8B; g_phase15_cave[p++] = 0x11;

    /* JMP back to hook_addr + PHASE15_ORIG_BYTES */
    g_phase15_cave[p++] = 0xE9;
    *(DWORD*)(g_phase15_cave + p) =
        (DWORD)(hook_addr + PHASE15_ORIG_BYTES) - (DWORD)(g_phase15_cave + p + 4);
    p += 4;

    /* Patch the hook site */
    DWORD old_protect;
    VirtualProtect(hook_addr, PHASE15_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);

    DWORD jmp_offset = (DWORD)(g_phase15_cave - hook_addr - 5);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_offset;
    hook_addr[5] = 0x90;  /* NOP byte 6 */

    VirtualProtect(hook_addr, PHASE15_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, PHASE15_ORIG_BYTES);

    wsprintfA(buf, "PHASE15 HOOK installed: %d bytes, cave=%08X", p, (DWORD)g_phase15_cave);
    diag_log(buf);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Water plane scanning thread — runs in C context (not code cave)
 *
 * Periodically checks if we need to scan for water planes using the ball
 * pointer saved by the Phase 15 cave. This runs in a background thread so
 * the code cave doesn't need to do complex scanning.
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI scan_thread(LPVOID param)
{
    (void)param;
    while (1) {
        Sleep(100);  /* check every 100ms */

        DWORD ball = g_ball_ptr;
        if (!ball || IsBadReadPtr((void*)ball, 0x300)) continue;

        /* Get board pointer (ball+0x14) */
        DWORD board = *(DWORD*)(ball + BALL_BOARD);
        if (!board || IsBadReadPtr((void*)board, 0x1000)) continue;

        /* Re-scan if board changed or not yet scanned */
        if (!g_scan_done || board != g_last_board) {
            g_last_board = board;
            scan_for_water_planes(board);
        }
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI patch_thread(LPVOID param)
{
    (void)param;
    char buf[256];

    diag_log("=== Water mod v2 loaded (velocity-based, Phase 15 hook) ===");
    Sleep(5000);  /* wait for game to finish loading */

    /* Initialize the C helper function pointer */
    g_water_fn_ptr = apply_water_physics;
    wsprintfA(buf, "water_fn = %08X", (DWORD)apply_water_physics);
    diag_log(buf);

    wsprintfA(buf, "Config: entry_damp=%.2f drag=%.3f hdrag=%.3f buoy=%.3f ini_planes=%d",
              g_cfg.entry_damping, g_cfg.drag, g_cfg.horizontal_drag,
              g_cfg.buoyancy_strength, g_cfg.plane_count);
    diag_log(buf);

    /* Install Phase 15 hook */
    install_phase15_hook();

    /* Start background scan thread */
    CreateThread(NULL, 0, scan_thread, NULL, 0, NULL);
    diag_log("scan_thread launched");

    /* Status check after 8 seconds */
    Sleep(8000);
    wsprintfA(buf, "After 8s: hook_calls=%u planes=%d ball=%08X",
              g_hook_calls, g_plane_count, g_ball_ptr);
    diag_log(buf);

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hInst, DWORD reason, LPVOID lpReserved)
{
    (void)lpReserved;
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInst);

        /* Set up log path next to bass.dll */
        GetModuleFileNameA(hInst, g_logpath, MAX_PATH);
        {
            char *p = strrchr(g_logpath, '\\');
            if (p) strcpy(p + 1, "water_mod_log.txt");
        }

        diag_log("=== Water mod v2 DLL attaching ===");

        load_real_bass();
        {
            char buf[128];
            wsprintfA(buf, "bass_real = %08X", (DWORD)g_hRealBass);
            diag_log(buf);
        }

        /* Load INI config */
        {
            char ini_path[MAX_PATH];
            GetModuleFileNameA(hInst, ini_path, MAX_PATH);
            char *p = strrchr(ini_path, '\\');
            if (p) strcpy(p + 1, "hamsterball_water.ini");
            load_config(ini_path);
        }

        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
