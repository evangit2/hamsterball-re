/*
 * lava_rising — Rising/falling lava with heat zone system
 *
 * Built on the custom lifter state machine. The lifter represents lava
 * that rises and falls on a timing cycle. When the ball is within the
 * lifter's X/Z footprint, the vertical gap (ball.y - lava.y) determines
 * the heat level:
 *   - Close to lava → ball turns red, speed increases
 *   - Far from lava  → ball cools, returns to normal
 *   - Touching lava  → ball dies (shatter)
 *   - Near ice/cold  → ball turns blue, speed decreases (future)
 *
 * Heat is calculated via Option 2: X/Z bounds check + Y gap.
 * Ball tint via ball+0x2AC (R) / 0x2B0 (G) / 0x2B4 (B) color multipliers.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef unsigned __int64 QWORD;

/* ═══════════════════════════════════════════════════════════════════
 *  BASS proxy exports (10 required game imports)
 * ═══════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;
static void lazy_load_real_bass(void);  /* forward decl */

typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    lazy_load_real_bass();
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}

typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    lazy_load_real_bass();
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c);
    return 1;
}

typedef int  (__stdcall *BASS_MusicLoad_t)(int, const char*, QWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) int __stdcall BASS_MusicLoad(int a, const char* b, QWORD c, DWORD d, DWORD e, DWORD f) {
    lazy_load_real_bass();
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f);
    return 0;
}

typedef int  (__stdcall *BASS_SampleLoad_t)(int, const char*, QWORD, DWORD, DWORD, DWORD);
static BASS_SampleLoad_t real_BASS_SampleLoad = NULL;
__declspec(dllexport) int __stdcall BASS_SampleLoad(int a, const char* b, QWORD c, DWORD d, DWORD e, DWORD f) {
    lazy_load_real_bass();
    if (real_BASS_SampleLoad) return real_BASS_SampleLoad(a, b, c, d, e, f);
    return 0;
}

typedef int  (__stdcall *BASS_StreamCreateFile_t)(int, const char*, QWORD, QWORD, DWORD);
static BASS_StreamCreateFile_t real_BASS_StreamCreateFile = NULL;
__declspec(dllexport) int __stdcall BASS_StreamCreateFile(int a, const char* b, QWORD c, QWORD d, DWORD e) {
    lazy_load_real_bass();
    if (real_BASS_StreamCreateFile) return real_BASS_StreamCreateFile(a, b, c, d, e);
    return 0;
}

typedef int  (__stdcall *BASS_SampleGetChannel_t)(DWORD, BOOL);
static BASS_SampleGetChannel_t real_BASS_SampleGetChannel = NULL;
__declspec(dllexport) int __stdcall BASS_SampleGetChannel(DWORD a, BOOL b) {
    lazy_load_real_bass();
    if (real_BASS_SampleGetChannel) return real_BASS_SampleGetChannel(a, b);
    return 0;
}

typedef int  (__stdcall *BASS_ChannelPlay_t)(DWORD, BOOL);
static BASS_ChannelPlay_t real_BASS_ChannelPlay = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelPlay(DWORD a, BOOL b) {
    lazy_load_real_bass();
    if (real_BASS_ChannelPlay) return real_BASS_ChannelPlay(a, b);
    return 1;
}

typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    lazy_load_real_bass();
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 1;
}

typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, DWORD, DWORD);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, DWORD d, DWORD e) {
    lazy_load_real_bass();
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 1;
}

typedef int  (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) int __stdcall BASS_Free(void) {
    lazy_load_real_bass();
    if (real_BASS_Free) return real_BASS_Free();
    return 1;
}

typedef int  (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    lazy_load_real_bass();
    if (real_BASS_Stop) return real_BASS_Stop();
    return 1;
}

typedef int  (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;
__declspec(dllexport) int __stdcall BASS_Start(void) {
    lazy_load_real_bass();
    if (real_BASS_Start) return real_BASS_Start();
    return 1;
}

typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    lazy_load_real_bass();
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}

typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    lazy_load_real_bass();
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════
 *  Lifter mod
 * ═══════════════════════════════════════════════════════════════════ */

/* Rotator struct offsets (byte offsets) */
#define OFF_BOARD      0x10D0
#define OFF_POS_X      0x10D4
#define OFF_POS_Y      0x10D8   /* this is what moves up/down */
#define OFF_POS_Z      0x10DC
#define OFF_CHILD_MESH 0x10E0
#define OFF_STATE      0x10E4
#define OFF_TIMER      0x10E8
#define OFF_PAUSE_DUR  0x10EC
#define OFF_DIRTY      0x10F0

/* Rotator vtable address (in .rdata) */
#define ROTATOR_VTABLE  0x004D5770
/* vtable[0x0B] = index 11 = byte offset 0x2C */
#define VTABLE_0B_OFFSET 0x2C

/* Original function pointer (saved before patching) */
typedef int (__thiscall *RenderFn_t)(void *thisptr);
static RenderFn_t g_OriginalRender = NULL;

/* Timer durations (frames) */
#define T_BOTTOM_PAUSE_1  300
#define T_RISE            1000
#define T_TOP_PAUSE       300
#define T_FALL            1000
#define T_BOTTOM_PAUSE_2  400

/* Movement per sub-step */
#define RISE_STEP  0.25f
#define FALL_STEP  0.25f
#define SUB_STEPS  3

/* Ball-carry constants (from original decompilation at 0x0043D420) */
#define BALL_Y_THRESHOLD  50.0f   /* _DAT_004D5D10 (double) */
#define BALL_XZ_RANGE     75.0f   /* _DAT_004CF480 */

/* Ball struct offsets */
#define BALL_X      0x164
#define BALL_Y      0x168
#define BALL_Z      0x16C

/* Ball color multiplier offsets (read by Ball_Render every frame) */
#define BALL_COLOR_R  0x2AC
#define BALL_COLOR_G  0x2B0
#define BALL_COLOR_B  0x2B4

/* Ball max_speed (used for speed scaling) */
#define BALL_MAX_SPEED  0x188

/* Ball death function (shatter) */
#define BALL_SHATTER_ADDR  0x00408D70

/* Heat zone parameters */
#define HEAT_CUTOFF       200.0f   /* beyond this distance, no heat effect */
#define HEAT_DEATH_DIST   26.0f    /* ball radius — touching lava = death */
#define HEAT_FULL_DIST    30.0f   /* distance at which heat is max (1.0) */

/* Normal ball values (for restoration) */
#define NORMAL_MAX_SPEED  1000.0f

/* Board struct offsets for ball list (AthenaList) */
#define BOARD_BALL_LIST   0x29D4
#define BOARD_BALL_COUNT  0x29D8
#define BOARD_BALL_ARRAY  0x2DE0

/* Game function addresses */
#define FUNC_AthenaList_Lock  0x004532B0  /* FUN_004532b0 — iterator lock */

/* Function pointer type for AthenaList lock */
typedef int (__cdecl *AthenaListLock_t)(int list_ptr);

/*
 * Heat zone system: raycast downward from the ball against all N:LAVA meshes.
 *
 * Scans the MeshWorld's MeshBuffer list for names starting with "N:LAVA",
 * then calls Mesh_FindClosestCollision (0x465D90) on each, casting a ray
 * straight down from the ball. Uses the closest hit distance for heat.
 *
 * Works for both rising lava (on lifters) and static lava meshes.
 */

/* Mesh_FindClosestCollision: __thiscall(ECX=mesh, RET 0x20)
 *   Stack params: float *outHit, float origin[3], float dir[3], float tolerance
 *   Returns outHit (3 floats = hit position XYZ)
 */
typedef void * (__thiscall *MeshRaycast_t)(void *mesh, float *outHit, float *origin, float *dir, float tolerance);
#define FUNC_MeshRaycast  ((MeshRaycast_t)0x00465D90)

/* Scene/MeshWorld/MeshBuffer struct offsets */
#define G_SCENE_PTR       0x005341E4
#define SCENE_MESHWORLD   0x8AC
#define MW_MESHBUFFER_LIST 0x2C    /* AthenaList embedded in MeshWorld */
#define MW_MB_COUNT        0x04    /* count in AthenaList */
#define MW_MB_ARRAY        0x40C   /* items pointer in AthenaList */
#define MB_NAME            0x864   /* MeshBuffer name (char*) */

static void apply_heat_zone(BYTE *lifter)
{
    /* Get the scene → meshworld → meshbuffer list */
    int scene = *(int *)G_SCENE_PTR;
    if (!scene || IsBadReadPtr((void *)scene, 0x8B0)) return;

    int level = *(int *)(scene + SCENE_MESHWORLD);
    if (!level || IsBadReadPtr((void *)level, 0x500)) return;

    /* MeshWorld's AthenaList of MeshBuffers is at level+0x2C */
    int *athenaList = (int *)((char *)level + MW_MESHBUFFER_LIST);
    int mb_count = *(int *)((char *)athenaList + MW_MB_COUNT);
    int *mb_array = *(int **)((char *)athenaList + MW_MB_ARRAY);
    if (!mb_array || mb_count < 1) return;

    /* Get the board's ball list */
    int board = *(int *)(lifter + OFF_BOARD);
    if (!board) return;

    int ball_count = *(int *)(board + BOARD_BALL_COUNT);
    if (ball_count < 1) return;

    int *ball_array = *(int **)(board + BOARD_BALL_ARRAY);
    if (!ball_array) return;

    /* For each ball, raycast downward against all N:LAVA meshes */
    for (int i = 0; i < ball_count; i++) {
        int ball = ball_array[i];
        if (!ball) continue;
        if (IsBadReadPtr((void *)ball, 0x2B8)) continue;

        float bx = *(float *)(ball + BALL_X);
        float by = *(float *)(ball + BALL_Y);
        float bz = *(float *)(ball + BALL_Z);

        /* Raycast straight down from the ball */
        float origin[3] = { bx, by, bz };
        float dir[3]    = { 0.0f, -1.0f, 0.0f };  /* straight down */
        float hitPos[3]  = { 0, 0, 0 };

        float closestDist = HEAT_CUTOFF + 1.0f;  /* beyond cutoff = no heat */
        int foundHit = 0;

        /* Scan all meshbuffers for N:LAVA prefix */
        for (int j = 0; j < mb_count; j++) {
            int mb = mb_array[j];
            if (!mb) continue;
            if (IsBadReadPtr((void *)mb, 0x870)) continue;

            char *name = *(char **)(mb + MB_NAME);
            if (!name) continue;
            if (IsBadReadPtr(name, 8)) continue;

            /* Check if name starts with "N:LAVA" (case-insensitive) */
            if (_strnicmp(name, "N:LAVA", 6) != 0) continue;

            /* Raycast against this mesh buffer */
            float hit[3] = { 0, 0, 0 };
            void *result = FUNC_MeshRaycast((void *)mb, hit, origin, dir, 0.01f);
            if (result) {
                /* hit is the intersection point; distance = |hit.y - ball.y| */
                float dist = by - hit[1];
                if (dist < 0) dist = -dist;
                if (dist < closestDist) {
                    closestDist = dist;
                    foundHit = 1;
                }
            }
        }

        if (!foundHit) {
            /* No lava below — restore normal */
            *(float *)(ball + BALL_COLOR_R) = 1.0f;
            *(float *)(ball + BALL_COLOR_G) = 1.0f;
            *(float *)(ball + BALL_COLOR_B) = 1.0f;
            *(float *)(ball + BALL_MAX_SPEED) = NORMAL_MAX_SPEED;
            continue;
        }

        /* Touching lava = death */
        if (closestDist <= HEAT_DEATH_DIST) {
            typedef void (__thiscall *ShatterFn_t)(void *);
            ShatterFn_t shatter = (ShatterFn_t)BALL_SHATTER_ADDR;
            shatter((void *)ball);
            continue;
        }

        /* Compute heat level (0.0 = no heat, 1.0 = max heat) */
        float heat = 0.0f;
        if (closestDist < HEAT_FULL_DIST) {
            heat = 1.0f;
        } else if (closestDist < HEAT_CUTOFF) {
            heat = (HEAT_CUTOFF - closestDist) / (HEAT_CUTOFF - HEAT_FULL_DIST);
        }

        if (heat > 0.0f) {
            /* Tint ball red: increase R, decrease G and B */
            *(float *)(ball + BALL_COLOR_R) = 1.0f;
            *(float *)(ball + BALL_COLOR_G) = 1.0f - heat * 0.8f;
            *(float *)(ball + BALL_COLOR_B) = 1.0f - heat * 0.8f;

            /* Speed boost: scale max_speed from normal to 1.5x based on heat */
            *(float *)(ball + BALL_MAX_SPEED) = NORMAL_MAX_SPEED * (1.0f + heat * 0.5f);
        } else {
            /* Restore normal values when far from lava */
            *(float *)(ball + BALL_COLOR_R) = 1.0f;
            *(float *)(ball + BALL_COLOR_G) = 1.0f;
            *(float *)(ball + BALL_COLOR_B) = 1.0f;
            *(float *)(ball + BALL_MAX_SPEED) = NORMAL_MAX_SPEED;
        }
    }
}

/*
 * Carry balls on the lifter by the same delta as the lifter movement.
 * Mirrors the original game's ball-carry logic from Rotator_vtable0B:
 *   1. Lock the board's ball AthenaList
 *   2. Iterate all balls
 *   3. If ball Y is within BALL_Y_THRESHOLD of lifter Y
 *      AND ball X/Z is within BALL_XZ_RANGE of lifter X/Z
 *   4. Move ball Y by the same delta as the lifter
 */
static void carry_balls(BYTE *thisptr, float delta_y)
{
    int board = *(int *)(thisptr + OFF_BOARD);
    if (!board) return;

    int ball_count = *(int *)(board + BOARD_BALL_COUNT);
    if (ball_count < 1) return;

    int *ball_array = *(int **)(board + BOARD_BALL_ARRAY);
    if (!ball_array) return;

    float lifter_x = *(float *)(thisptr + OFF_POS_X);
    float lifter_y = *(float *)(thisptr + OFF_POS_Y);
    float lifter_z = *(float *)(thisptr + OFF_POS_Z);

    for (int i = 0; i < ball_count; i++) {
        int ball = ball_array[i];
        if (!ball) continue;
        if (IsBadReadPtr((void *)ball, 0x180)) continue;

        float bx = *(float *)(ball + BALL_X);
        float by = *(float *)(ball + BALL_Y);
        float bz = *(float *)(ball + BALL_Z);

        /* Check: ball is on the lifter platform */
        float y_diff = by - lifter_y;
        if (y_diff < 0) y_diff = -y_diff;  /* fabsf */

        if (y_diff < BALL_Y_THRESHOLD &&
            bx > lifter_x - BALL_XZ_RANGE && bx < lifter_x + BALL_XZ_RANGE &&
            bz > lifter_z - BALL_XZ_RANGE && bz < lifter_z + BALL_XZ_RANGE)
        {
            *(float *)(ball + BALL_Y) = by + delta_y;
        }
    }
}

/*
 * Custom lifter render function.
 * Replaces Rotator vtable[0x0B] (original at 0x0043D420).
 *
 * Strategy: Call the original function with a temporary no-op state
 * (state=0, timer=99999) so it handles the dirty-flag mesh repositioning
 * (Timer_Init, Gfx_SetPosition, vtable calls, Timer_Cleanup) without
 * running the original state machine. Then restore our real state/timer/
 * pos_y and run the custom state machine.
 */
__attribute__((used, noinline))
int __thiscall CustomLifter_Render(void *thisptr)
{
    BYTE  *bytes  = (BYTE *)thisptr;

    /* ── Save our real state/timer/pos_y ────────────────────────── */
    int   saved_state  = *(int  *)(bytes + OFF_STATE);
    int   saved_timer  = *(int  *)(bytes + OFF_TIMER);
    float saved_pos_y  = *(float*)(bytes + OFF_POS_Y);

    /* ── Temporarily set no-op state so original does nothing ───── */
    *(int *)(bytes + OFF_STATE) = 0;       /* state 0 = idle, just timer countdown */
    *(int *)(bytes + OFF_TIMER) = 99999;  /* huge timer → no state transition */

    /* ── Call original: handles dirty flag + mesh repositioning ── */
    /* Original in state 0 with timer 99999: decrements timer, no
       movement, no sound, no ball scan. Dirty flag is processed at
       the top before the switch, so mesh gets repositioned if needed. */
    g_OriginalRender(thisptr);

    /* ── Restore our real values ───────────────────────────────── */
    *(int  *)(bytes + OFF_STATE) = saved_state;
    *(int  *)(bytes + OFF_TIMER) = saved_timer;
    *(float*)(bytes + OFF_POS_Y) = saved_pos_y;

    /* ── Run custom state machine ──────────────────────────────── */
    int   *state  = (int  *)(bytes + OFF_STATE);
    int   *timer  = (int  *)(bytes + OFF_TIMER);
    float *pos_y  = (float*)(bytes + OFF_POS_Y);
    BYTE  *dirty  = (BYTE *)(bytes + OFF_DIRTY);

    switch (*state) {

    /* ── State 0: IDLE (bottom pause) ───────────────────────────── */
    case 0:
        (*timer)--;
        if (*timer < 1) {
            *state = 1;          /* → rising */
            *timer = T_RISE;
        }
        break;

    /* ── State 1: RISING ────────────────────────────────────────── */
    case 1:
        for (int i = 0; i < SUB_STEPS; i++) {
            *pos_y += RISE_STEP;
            carry_balls(bytes, RISE_STEP);
        }
        *dirty = 1;              /* mark mesh for repositioning next frame */
        /* No clunk sound */
        (*timer)--;
        if (*timer < 1) {
            *state = 2;          /* → top pause */
            *timer = T_TOP_PAUSE;
        }
        break;

    /* ── State 2: IDLE (top pause) ──────────────────────────────── */
    case 2:
        (*timer)--;
        if (*timer < 1) {
            *state = 3;          /* → falling */
            *timer = T_FALL;
        }
        break;

    /* ── State 3: FALLING ──────────────────────────────────────── */
    case 3:
        for (int i = 0; i < SUB_STEPS; i++) {
            *pos_y -= FALL_STEP;
            carry_balls(bytes, -FALL_STEP);
        }
        *dirty = 1;
        (*timer)--;
        if (*timer < 1) {
            *state = 0;          /* → bottom pause */
            *timer = T_BOTTOM_PAUSE_2;
            /* No arrival sound */
        }
        break;
    }

    /* ── Apply heat zone effects ──────────────────────────────────── */
    apply_heat_zone(bytes);

    return 1;
}

/* ═══════════════════════════════════════════════════════════════════
 *  Hook installation
 * ═══════════════════════════════════════════════════════════════════ */

static void install_lifter_hook(void)
{
    /* The Rotator vtable is at 0x004D5770 in .rdata.
       vtable[0x0B] is at offset 0x2C = address 0x004D579C.
       Original function at 0x0043D420. */

    DWORD *vtable_entry = (DWORD *)(ROTATOR_VTABLE + VTABLE_0B_OFFSET);
    DWORD old_protect;

    /* Verify the original function pointer matches expected value */
    DWORD expected = 0x0043D420;
    if (*vtable_entry != expected) {
        /* Vtable entry doesn't match — wrong game version or already patched */
        return;
    }

    /* Save original */
    g_OriginalRender = (RenderFn_t)(*vtable_entry);

    /* Make vtable entry writable */
    VirtualProtect(vtable_entry, 4, PAGE_EXECUTE_READWRITE, &old_protect);

    /* Patch with our function */
    *vtable_entry = (DWORD)&CustomLifter_Render;

    /* Restore protection */
    VirtualProtect(vtable_entry, 4, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), vtable_entry, 4);
}

/* ═══════════════════════════════════════════════════════════════════
 *  BASS proxy initialization
 * ═══════════════════════════════════════════════════════════════════ */

/* Lazy initialization: loads real bass.dll on first BASS proxy call.
 * This runs synchronously during the game's BASS_Init (App::Initialize),
 * so the real DLL is available before any audio operations. */
static void lazy_load_real_bass(void)
{
    if (g_hRealBass) return;  /* already loaded */

    char path[MAX_PATH];

    /* Try game directory first (bass_real.dll — user renames original) */
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char *slash = strrchr(path, '\\');
    if (slash) {
        strcpy(slash + 1, "bass_real.dll");
        g_hRealBass = LoadLibraryA(path);
    }

    /* Fallback: system directory */
    if (!g_hRealBass) {
        GetSystemDirectoryA(path, MAX_PATH);
        lstrcatA(path, "\\bass.dll");
        g_hRealBass = LoadLibraryA(path);
    }

    if (!g_hRealBass) return;

    real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)
        GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
    real_BASS_MusicPlayEx = (BASS_MusicPlayEx_t)
        GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
    real_BASS_MusicLoad = (BASS_MusicLoad_t)
        GetProcAddress(g_hRealBass, "BASS_MusicLoad");
    real_BASS_SampleLoad = (BASS_SampleLoad_t)
        GetProcAddress(g_hRealBass, "BASS_SampleLoad");
    real_BASS_StreamCreateFile = (BASS_StreamCreateFile_t)
        GetProcAddress(g_hRealBass, "BASS_StreamCreateFile");
    real_BASS_SampleGetChannel = (BASS_SampleGetChannel_t)
        GetProcAddress(g_hRealBass, "BASS_SampleGetChannel");
    real_BASS_ChannelPlay = (BASS_ChannelPlay_t)
        GetProcAddress(g_hRealBass, "BASS_ChannelPlay");
    real_BASS_ChannelStop = (BASS_ChannelStop_t)
        GetProcAddress(g_hRealBass, "BASS_ChannelStop");
    real_BASS_Init = (BASS_Init_t)
        GetProcAddress(g_hRealBass, "BASS_Init");
    real_BASS_Free = (BASS_Free_t)
        GetProcAddress(g_hRealBass, "BASS_Free");
    real_BASS_Stop = (BASS_Stop_t)
        GetProcAddress(g_hRealBass, "BASS_Stop");
    real_BASS_Start = (BASS_Start_t)
        GetProcAddress(g_hRealBass, "BASS_Start");
    real_BASS_SetConfig = (BASS_SetConfig_t)
        GetProcAddress(g_hRealBass, "BASS_SetConfig");
    real_BASS_ErrorGetCode = (BASS_ErrorGetCode_t)
        GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
}

static DWORD WINAPI patch_thread(LPVOID param)
{
    (void)param;
    /* Wait for game to finish loading */
    Sleep(2000);

    install_lifter_hook();

    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    (void)hinstDLL;
    (void)lpvReserved;

    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
    }
    return TRUE;
}
