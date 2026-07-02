/*
 * global_objects_mkn.c — Global BUMPER Mod for Hamsterball
 *
 * Makes BUMPER objects work on ANY race/level, not just Beginner/Master/Toob.
 *
 * BUMPER objects are mesh-level collision objects named "N:BUMPER%d" (1-8).
 * They exist in the MESHWORLD files of many levels but only have collision
 * EFFECTS (velocity boost + sound + hit animation) on levels whose board
 * vtable dispatches to a collision handler that recognizes "N:BUMPER".
 *
 * This mod hooks DispatchCollisionEvents (0x40C5D0) — the BASE collision
 * dispatcher called by ALL board vtables. When an "N:BUMPER" collision event
 * is detected on a level that doesn't natively handle it, we apply the same
 * physics as the game's own bumper handlers:
 *
 *   1. Play bumper sound (Sound_Play3D at App+0x448)
 *   2. Scale XZ velocity by 4.0x (_DAT_004cf41c)
 *   3. Set Y velocity to 0 (flat bounce)
 *   4. Clamp speed to [5.0, 10.0] (_DAT_004cf55c / _DAT_004cf9f8)
 *   5. Set bumper hit flag = 1.0f for bumper animation
 *
 * The hit flag offset varies by board type:
 *   Race boards:  board + atol(N) * 4 + 0x6448
 *   Arena boards: board + (atol(N)-1) * 4 + 0x53FC
 *
 * We detect board type by checking the board vtable pointer.
 *
 * Build: i686-w64-mingw32-gcc -shared -o bass.dll global_objects_mkn.c -lwinmm \
 *        -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 *
 * Install: Rename original bass.dll to bass_real.dll, place this bass.dll in game folder.
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* ========== Game function addresses (VA, image base 0x400000) ========== */
#define ADDR_DISPATCH_COLLISION 0x0040C5D0  /* __thiscall: ECX=board, [ball, entry], RET 8 */
#define ADDR_SOUND_PLAY_3D      0x00459860  /* __cdecl: (sound_ptr, x, y, z) */
#define ADDR_VEC3_NORMALIZE_SCALE 0x00401AA0 /* __thiscall: ECX=&vec3, float scale */
#define ADDR_ATHENASTRING_FORMAT 0x00466C70  /* __fastcall: ECX=buffer, EDX=format_str */

/* ========== Struct offsets ========== */
#define APP_BASE_ADDR           0x005341E0   /* Global App pointer address */
#define APP_GFX_DEVICE          0x174        /* App+0x174 = Graphics* (D3D device) */
#define BOARD_APP_PTR           0x878        /* board+0x878 = App* */

/* Ball offsets */
#define BALL_POS_X              0x164        /* ball+0x164 = position X (float) */
#define BALL_POS_Y              0x168        /* ball+0x168 = position Y (float) */
#define BALL_POS_Z              0x16C        /* ball+0x16C = position Z (float) */
#define BALL_PHYS_PTR           0x1A4        /* ball+0x1A4 = Physics* (param_2[0x69] in decomp) */
#define PHYS_VEL_X              0xCA4        /* phys+0xCA4 = velocity X (float) */
#define PHYS_VEL_Y              0xCA8        /* phys+0xCA8 = velocity Y (float) */
#define PHYS_VEL_Z              0xCAC        /* phys+0xCAC = velocity Z (float) */

/* Collision entry offsets */
#define COLLISION_ENTRY_NAME    0x864        /* collision_data+0x864 = entity name string ptr */

/* Sound slot for bumper SFX */
#define APP_BUMPER_SOUND        0x448        /* App+0x448 = bumper sound pointer */

/* Bumper physics constants (verified from .rdata) */
#define BUMPER_VEL_SCALE        4.0f        /* _DAT_004cf41c: velocity multiplier */
#define BUMPER_MIN_SPEED        5.0f        /* _DAT_004cf55c: minimum launch speed */
#define BUMPER_MAX_SPEED_RACE   10.0f       /* _DAT_004cf9f8: max speed for race boards */
#define BUMPER_MAX_SPEED_ARENA  12.0f       /* _DAT_004cf3dc: max speed for arena boards */

/* Bumper hit flag offsets per board type */
#define BUMPER_HIT_RACE         0x6448      /* Race boards: board + N*4 + 0x6448 */
#define BUMPER_HIT_ARENA        0x53FC      /* Arena boards: board + (N-1)*4 + 0x53FC */

/* Known board vtables for type detection */
#define VTABLE_LEVEL_CASCADE    0x004D10E0  /* Warm-Up Race */
#define VTABLE_LEVEL8            0x004D0EC0  /* Beginner Race */
#define VTABLE_LEVEL10           0x004D0F40  /* Master Race */

/* ========== Function pointer types ========== */
typedef void (__thiscall *dispatch_collision_t)(void *board, void *ball, void *entry);
typedef void (__cdecl *sound_play_3d_t)(void *sound_ptr, float x, float y, float z);
typedef void (__thiscall *vec3_normalize_scale_t)(float *vec, float scale);

/* ========== Function pointers ========== */
static dispatch_collision_t  pfn_dispatch_collision;
static sound_play_3d_t       pfn_sound_play_3d;
static vec3_normalize_scale_t pfn_vec3_normalize_scale;

/* ========== Hook state ========== */
static unsigned char *g_collision_tramp = NULL;
static int g_hooked = 0;

/* ========== Utility ========== */
static int safe_read(void *addr, unsigned int size)
{
    return !IsBadReadPtr(addr, size);
}

/* Detect if board is an arena board by checking vtable.
 * Arena boards use HandleArenaCollisionEvents (0x412850) as vtable[0x1D].
 * Race boards use either FUN_00410020 or FUN_004111E0.
 * We check the vtable[0x1D] slot (offset 0x74) to determine board type.
 */
static int is_arena_board(void *board)
{
    if (!board || !safe_read(board, 4)) return 0;
    void **vt = *(void ***)board;
    if (!vt || !safe_read(vt, 0x78)) return 0;

    /* vtable[0x1D] = offset 0x74 = collision handler */
    void *collision_handler = vt[0x1D];
    if (!collision_handler) return 0;

    /* HandleArenaCollisionEvents = 0x412850 (arena boards) */
    if ((DWORD)collision_handler == 0x412850) return 1;

    /* Also check some arena vtables directly by board vtable address */
    if ((DWORD)vt == 0x4D11E0 || (DWORD)vt == 0x4D1200 ||
        (DWORD)vt == 0x4D1220 || (DWORD)vt == 0x4D1240)
        return 1;

    return 0;
}

/* Check if the board's collision handler already handles N:BUMPER natively.
 * If it does, we skip our injection to avoid double-processing.
 *
 * Boards with native BUMPER handling:
 *   - Warm-Up (Cascade): vtable[0x1D] = FUN_004111E0 (0x4111E0)
 *   - Beginner/Master:   vtable[0x1D] = FUN_00410020 (0x410020) 
 *   - Arena-Beginner/Toob: vtable[0x1D] = HandleArenaCollisionEvents (0x412850)
 *
 * All other boards → DispatchCollisionEvents (0x40C5D0) which does NOT handle BUMPER.
 */
static int board_handles_bumper_natively(void *board)
{
    if (!board || !safe_read(board, 4)) return 0;
    void **vt = *(void ***)board;
    if (!vt || !safe_read(vt, 0x78)) return 0;

    void *collision_handler = vt[0x1D];
    if (!collision_handler) return 0;

    /* FUN_004111E0 (Warm-Up), FUN_00410020 (Beginner/Master), HandleArenaCollisionEvents (Arena) */
    if ((DWORD)collision_handler == 0x004111E0 ||
        (DWORD)collision_handler == 0x00410020 ||
        (DWORD)collision_handler == 0x00412850)
        return 1;

    return 0;
}

/* Apply bumper physics to ball.
 * This replicates the exact logic from FUN_004111E0 and FUN_00410020.
 *
 * Physics flow:
 * 1. Get ball position (for sound)
 * 2. Get physics velocity (XZ plane)
 * 3. Scale by BUMPER_VEL_SCALE (4.0)
 * 4. Zero out Y velocity
 * 5. If speed < min (5.0): normalize to min
 * 6. If speed > max (10.0 race / 12.0 arena): normalize to max
 * 7. Write back to physics velocity
 * 8. Set bumper hit flag on board
 */
static void apply_bumper_physics(void *board, void *ball, const char *bumper_name)
{
    float pos_x, pos_y, pos_z;
    float vel_x, vel_z, speed_sq, speed;
    float max_speed, scale_factor;
    int phys_ptr;
    int arena;
    long bumper_idx;
    void *app;
    void *sound_ptr;

    if (!ball || !safe_read(ball, 0x200)) return;
    if (!board || !safe_read(board, 0x6500)) return;

    /* Get ball position */
    pos_x = *(float *)((unsigned char *)ball + BALL_POS_X);
    pos_y = *(float *)((unsigned char *)ball + BALL_POS_Y);
    pos_z = *(float *)((unsigned char *)ball + BALL_POS_Z);

    /* Get App and sound pointer */
    app = *(void **)((unsigned char *)board + BOARD_APP_PTR);
    if (!app || !safe_read(app, APP_BUMPER_SOUND + 4)) return;
    sound_ptr = *(void **)((unsigned char *)app + APP_BUMPER_SOUND);

    /* Play bumper sound at ball position */
    if (sound_ptr && pfn_sound_play_3d) {
        pfn_sound_play_3d(sound_ptr, pos_x, pos_y, pos_z);
    }

    /* Get physics pointer */
    phys_ptr = *(int *)((unsigned char *)ball + BALL_PHYS_PTR);
    if (!phys_ptr || !safe_read((void *)phys_ptr, PHYS_VEL_Z + 4)) return;

    /* Read current XZ velocity */
    vel_x = *(float *)((unsigned char *)phys_ptr + PHYS_VEL_X);
    vel_z = *(float *)((unsigned char *)phys_ptr + PHYS_VEL_Z);

    /* Scale by bumper velocity multiplier */
    vel_x *= BUMPER_VEL_SCALE;
    vel_z *= BUMPER_VEL_SCALE;

    /* Zero out Y velocity (flat bounce) */
    *(float *)((unsigned char *)phys_ptr + PHYS_VEL_Y) = 0.0f;

    /* Clamp speed: min = 5.0 */
    speed_sq = vel_x * vel_x + vel_z * vel_z;
    if (speed_sq < BUMPER_MIN_SPEED * BUMPER_MIN_SPEED) {
        /* Too slow — normalize to min speed */
        if (speed_sq > 0.0f) {
            float inv = BUMPER_MIN_SPEED / sqrtf(speed_sq);
            vel_x *= inv;
            vel_z *= inv;
        } else {
            /* Zero velocity — can't normalize, skip */
            vel_x = 0.0f;
            vel_z = 0.0f;
        }
    }

    /* Clamp speed: max */
    arena = is_arena_board(board);
    max_speed = arena ? BUMPER_MAX_SPEED_ARENA : BUMPER_MAX_SPEED_RACE;

    speed_sq = vel_x * vel_x + vel_z * vel_z;
    if (speed_sq > max_speed * max_speed) {
        if (speed_sq > 0.0f) {
            float inv = max_speed / sqrtf(speed_sq);
            vel_x *= inv;
            vel_z *= inv;
        }
    }

    /* Write back scaled velocity */
    *(float *)((unsigned char *)phys_ptr + PHYS_VEL_X) = vel_x;
    *(float *)((unsigned char *)phys_ptr + PHYS_VEL_Z) = vel_z;

    /* Set bumper hit flag for animation.
     * The bumper name is "N:BUMPER%d" where %d is 1-8.
     * Extract the number and compute the flag offset.
     */
    bumper_idx = atol(bumper_name + 8); /* skip "N:BUMPER" (8 chars) */
    if (bumper_idx >= 1 && bumper_idx <= 8) {
        DWORD flag_offset;
        if (arena) {
            /* Arena: board + (N-1)*4 + 0x53FC */
            flag_offset = BUMPER_HIT_ARENA + (bumper_idx - 1) * 4;
        } else {
            /* Race: board + N*4 + 0x6448 */
            flag_offset = BUMPER_HIT_RACE + bumper_idx * 4;
        }
        /* Set hit flag to 1.0f (0x3F800000) */
        *(unsigned int *)((unsigned char *)board + flag_offset) = 0x3F800000;
    }
}

/* ========== Detour: DispatchCollisionEvents Hook ==========
 *
 * DispatchCollisionEvents (0x40C5D0) is the BASE collision dispatcher.
 * It's called by ALL board vtables (vtable[0x1D] = offset 0x74).
 *
 * Some boards override vtable[0x1D] with their own handler that:
 *   1. Checks for specific collision names (N:BUMPER, N:SWIRL, etc.)
 *   2. Applies physics effects
 *   3. Falls through to DispatchCollisionEvents for common handling
 *
 * Other boards use DispatchCollisionEvents directly, which does NOT handle
 * N:BUMPER — so bumpers exist as visual meshes but have no effect.
 *
 * Our hook intercepts the BASE DispatchCollisionEvents. If the board already
 * has a native bumper handler, we let it pass through untouched. If not, we
 * check for N:BUMPER collisions and apply the physics ourselves before calling
 * the original.
 *
 * Original prologue (8 bytes): 6A FF 64 A1 00 00 00 00
 *   PUSH -1; MOV EAX, FS:[0]
 */
static void __fastcall collision_hook(void *board, void *unused, void *ball, void *entry)
{
    /* Only process if this board doesn't handle bumpers natively */
    if (!board_handles_bumper_natively(board)) {
        /* Check collision entry for N:BUMPER */
        if (entry && safe_read(entry, 8)) {
            int *pair = (int *)entry;
            int collision_data_ptr = pair[1];
            if (collision_data_ptr && safe_read((void *)collision_data_ptr, COLLISION_ENTRY_NAME + 16)) {
                char *event_name = *(char **)((unsigned char *)collision_data_ptr + COLLISION_ENTRY_NAME);
                if (event_name && safe_read(event_name, 9)) {
                    if (_strnicmp(event_name, "N:BUMPER", 8) == 0) {
                        /* This is a bumper collision on a non-native board!
                         * Apply bumper physics. */
                        apply_bumper_physics(board, ball, event_name);
                    }
                }
            }
        }
    }

    /* Call original DispatchCollisionEvents */
    {
        typedef void (__fastcall *orig_t)(void *, void *, void *, void *);
        ((orig_t)g_collision_tramp)(board, unused, ball, entry);
    }
}

/* ========== Detour installation ========== */
static void install_detour(DWORD target_addr, void *hook_fn,
                           const unsigned char *orig_bytes, int orig_len,
                           unsigned char **out_tramp)
{
    DWORD old_prot;
    unsigned char *tramp;
    unsigned char jmp_patch[5];
    int patch_len;

    /* Allocate executable trampoline: orig_len bytes + 5 for JMP back */
    tramp = (unsigned char *)VirtualAlloc(NULL, orig_len + 5,
                MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!tramp) return;

    /* Copy original prologue to trampoline */
    memcpy(tramp, orig_bytes, orig_len);

    /* Add JMP back to target+orig_len */
    tramp[orig_len] = 0xE9; /* JMP rel32 */
    *(DWORD *)(tramp + orig_len + 1) =
        (target_addr + orig_len) - ((DWORD)tramp + orig_len + 5);

    /* Patch original: JMP to hook */
    VirtualProtect((void *)target_addr, orig_len, PAGE_EXECUTE_READWRITE, &old_prot);

    jmp_patch[0] = 0xE9; /* JMP rel32 */
    *(DWORD *)(jmp_patch + 1) = (DWORD)hook_fn - (target_addr + 5);
    memcpy((void *)target_addr, jmp_patch, 5);

    /* Fill remaining bytes with NOPs */
    patch_len = orig_len - 5;
    if (patch_len > 0) {
        memset((void *)(target_addr + 5), 0x90, patch_len);
    }

    VirtualProtect((void *)target_addr, orig_len, old_prot, &old_prot);
    FlushInstructionCache(GetCurrentProcess(), (void *)target_addr, orig_len);

    *out_tramp = tramp;
}

static void install_hooks(void)
{
    /* Verify original bytes at DispatchCollisionEvents (0x40C5D0) */
    static const unsigned char COLL_PROLOGUE[8] = {
        0x6A, 0xFF, 0x64, 0xA1, 0x00, 0x00, 0x00, 0x00
    };

    unsigned char *p;

    /* Verify collision dispatch prologue */
    p = (unsigned char *)ADDR_DISPATCH_COLLISION;
    if (!safe_read(p, 8)) return;
    if (memcmp(p, COLL_PROLOGUE, 8) != 0) return;

    /* Install collision dispatch detour (8-byte prologue) */
    install_detour(ADDR_DISPATCH_COLLISION, collision_hook,
                   COLL_PROLOGUE, 8, &g_collision_tramp);

    g_hooked = 1;
}

/* ========== Init function pointers ========== */
static void init_function_pointers(void)
{
    pfn_dispatch_collision   = (dispatch_collision_t)ADDR_DISPATCH_COLLISION;
    pfn_sound_play_3d        = (sound_play_3d_t)ADDR_SOUND_PLAY_3D;
    pfn_vec3_normalize_scale = (vec3_normalize_scale_t)ADDR_VEC3_NORMALIZE_SCALE;
}

/* ========== BASS Proxy Stubs ========== */
/* Minimal BASS proxy — lazy-loads bass_real.dll if present */

static HMODULE g_real_bass = NULL;

static void lazy_load_bass(void)
{
    if (g_real_bass) return;
    g_real_bass = LoadLibraryA("bass_real.dll");
}

/* Game imports exactly these 10 BASS functions */
__declspec(dllexport) int __stdcall BASS_Init(int a, int b, int c, int d, void *e)
{
    (void)a; (void)b; (void)c; (void)d; (void)e;
    lazy_load_bass();
    if (g_real_bass) {
        typedef int (__stdcall *fn_t)(int, int, int, int, void *);
        fn_t fn = (fn_t)GetProcAddress(g_real_bass, "BASS_Init");
        if (fn) return fn(a, b, c, d, e);
    }
    return 1; /* TRUE — no crash on missing bass_real */
}

__declspec(dllexport) void __stdcall BASS_Free(void)
{
    lazy_load_bass();
    if (g_real_bass) {
        typedef void (__stdcall *fn_t)(void);
        fn_t fn = (fn_t)GetProcAddress(g_real_bass, "BASS_Free");
        if (fn) fn();
    }
}

__declspec(dllexport) int __stdcall BASS_Stop(void)
{
    lazy_load_bass();
    if (g_real_bass) {
        typedef int (__stdcall *fn_t)(void);
        fn_t fn = (fn_t)GetProcAddress(g_real_bass, "BASS_Stop");
        if (fn) return fn();
    }
    return 1;
}

__declspec(dllexport) int __stdcall BASS_Start(void)
{
    lazy_load_bass();
    if (g_real_bass) {
        typedef int (__stdcall *fn_t)(void);
        fn_t fn = (fn_t)GetProcAddress(g_real_bass, "BASS_Start");
        if (fn) return fn();
    }
    return 1;
}

__declspec(dllexport) int __stdcall BASS_SetConfig(int a, int b)
{
    (void)a; (void)b;
    lazy_load_bass();
    if (g_real_bass) {
        typedef int (__stdcall *fn_t)(int, int);
        fn_t fn = (fn_t)GetProcAddress(g_real_bass, "BASS_SetConfig");
        if (fn) return fn(a, b);
    }
    return 1;
}

__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void)
{
    lazy_load_bass();
    if (g_real_bass) {
        typedef int (__stdcall *fn_t)(void);
        fn_t fn = (fn_t)GetProcAddress(g_real_bass, "BASS_ErrorGetCode");
        if (fn) return fn();
    }
    return 0;
}

__declspec(dllexport) void * __stdcall BASS_MusicLoad(int a, void *b, int c, int d, int e, int f)
{
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)f;
    lazy_load_bass();
    if (g_real_bass) {
        typedef void * (__stdcall *fn_t)(int, void *, int, int, int, int);
        fn_t fn = (fn_t)GetProcAddress(g_real_bass, "BASS_MusicLoad");
        if (fn) return fn(a, b, c, d, e, f);
    }
    return (void *)1; /* non-zero — game crashes if NULL */
}

__declspec(dllexport) int __stdcall BASS_MusicPlayEx(void *a, int b, int c)
{
    (void)a; (void)b; (void)c;
    lazy_load_bass();
    if (g_real_bass) {
        typedef int (__stdcall *fn_t)(void *, int, int);
        fn_t fn = (fn_t)GetProcAddress(g_real_bass, "BASS_MusicPlayEx");
        if (fn) return fn(a, b, c);
    }
    return 1;
}

__declspec(dllexport) int __stdcall BASS_ChannelGetLength(void *a, int b)
{
    (void)a; (void)b;
    lazy_load_bass();
    if (g_real_bass) {
        typedef int (__stdcall *fn_t)(void *, int);
        fn_t fn = (fn_t)GetProcAddress(g_real_bass, "BASS_ChannelGetLength");
        if (fn) return fn(a, b);
    }
    return 0;
}

__declspec(dllexport) int __stdcall BASS_ChannelBytes2Seconds(void *a, int b)
{
    (void)a; (void)b;
    lazy_load_bass();
    if (g_real_bass) {
        typedef int (__stdcall *fn_t)(void *, int);
        fn_t fn = (fn_t)GetProcAddress(g_real_bass, "BASS_ChannelBytes2Seconds");
        if (fn) return fn(a, b);
    }
    return 0;
}

/* ========== DllMain ========== */
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
    (void)hinst; (void)reserved;
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        init_function_pointers();
        install_hooks();
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
