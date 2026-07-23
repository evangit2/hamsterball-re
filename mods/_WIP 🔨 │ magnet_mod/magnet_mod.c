/*
 * magnet_mod.c — Magnet zones for Hamsterball (v2: force accumulator approach)
 *
 * S1 ref points named MAGNET(P) attract the ball.
 * S1 ref points named MAGNET(N) repel the ball.
 *
 * Force is applied through the game's own physics engine by writing to
 * the force accumulators (ball+0x170/0x174/0x178) at the Phase 15
 * convergence point (0x407BB4) — same technique as the jump mod.
 * This gives smooth physics integration with proper collision response.
 *
 * Architecture (same as jump_mod):
 *   1. Background thread: scans S1 ref points for MAGNET(P)/MAGNET(N)
 *   2. Present hook (0x455A90): computes magnet force in safe C context,
 *      stores in globals (g_force_x/y/z, g_apply_force)
 *   3. Phase 15 cave (0x407BB4): reads globals, adds to ball+0x170/0x174/0x178
 *      via FLD/FADD/FSTP — physics engine then consumes them
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll magnet_mod.c \
 *     -I"../_WIP 🔨 │ shared" -lwinmm -Wl,--enable-stdcall-fixup -O2 \
 *     -static -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 */

#include "bass_proxy.h"
#include <math.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook Addresses
 * ═══════════════════════════════════════════════════════════════════════════ */

#define PRESENT_HOOK       0x00455A90   /* Graphics_PresentOrEnd — function entry */
#define PRESENT_ORIG_BYTES 7            /* 8A 44 24 04 83 EC 20 */
#define PHASE15_HOOK       0x00407BB4   /* Ball_Update Phase 15 convergence */
#define PHASE15_ORIG_BYTES 6            /* 8B 4C 24 1C 8B 11 */
#define ADDR_App           0x005341E0

/* ═══════════════════════════════════════════════════════════════════════════
 * Magnet State
 * ═══════════════════════════════════════════════════════════════════════════ */

#define MAX_MAGNETS 64

typedef struct {
    float x, y, z;
    int   is_positive;  /* 1 = attract (P), 0 = repel (N) */
} MagnetPoint;

static MagnetPoint g_magnets[MAX_MAGNETS];
static int   g_magnet_count = 0;
static DWORD g_last_board   = 0;

/* Config */
static float g_range     = 300.0f;
static float g_strength   = 0.5f;

/* Computed force (written by Present hook, read by Phase 15 cave) */
static volatile float g_force_x = 0.0f;
static volatile float g_force_y = 0.0f;
static volatile float g_force_z = 0.0f;
static volatile DWORD g_apply_force = 0;  /* 0=no, 1=yes */

/* ═══════════════════════════════════════════════════════════════════════════
 * Game Accessors
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD get_board_v2(void) {
    if (IsBadReadPtr((void*)ADDR_App, 4)) return 0;
    DWORD app = *(DWORD*)ADDR_App;
    if (!app || app < 0x10000) return 0;
    if (IsBadReadPtr((void*)(app + 0x178), 4)) return 0;
    DWORD board = *(DWORD*)(app + 0x178);
    if (!board || board < 0x10000) return 0;
    return board;
}

static DWORD get_player_ball_v2(void) {
    if (IsBadReadPtr((void*)ADDR_App, 4)) return 0;
    DWORD app = *(DWORD*)ADDR_App;
    if (!app || app < 0x10000) return 0;
    if (IsBadReadPtr((void*)(app + 0x5DC), 4)) return 0;
    DWORD ball = *(DWORD*)(app + 0x5DC);
    if (!ball || ball < 0x10000) return 0;
    if (IsBadReadPtr((void*)ball, 4)) return 0;
    return ball;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * S1 Ref Point Scanner
 *
 * Chain: board+0x8AC → Level → Level+0x480 → SceneObject
 *        SceneObject+0x898 = count, SceneObject+0xCA0 = items array ptr
 * Each item: [0]=name_ptr, [1]=X(float), [2]=Y(float), [3]=Z(float)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void scan_magnets(DWORD board) {
    g_magnet_count = 0;
    g_last_board   = board;

    if (IsBadReadPtr((void*)(board + 0x8AC), 4)) return;
    DWORD level = *(DWORD*)(board + 0x8AC);
    if (!level || level < 0x10000) return;
    if (IsBadReadPtr((void*)(level + 0x480), 4)) return;
    DWORD sceneobj = *(DWORD*)(level + 0x480);
    if (!sceneobj || sceneobj < 0x10000) return;

    if (IsBadReadPtr((void*)(sceneobj + 0x898), 4)) return;
    int count = *(int*)(sceneobj + 0x898);
    if (count <= 0 || count > 2000) return;

    if (IsBadReadPtr((void*)(sceneobj + 0xCA0), 4)) return;
    DWORD items_array = *(DWORD*)(sceneobj + 0xCA0);
    if (!items_array || items_array < 0x10000) return;

    for (int i = 0; i < count && g_magnet_count < MAX_MAGNETS; i++) {
        if (IsBadReadPtr((void*)(items_array + i * 4), 4)) break;
        DWORD item = *(DWORD*)(items_array + i * 4);
        if (!item || item < 0x10000) continue;
        if (IsBadReadPtr((void*)item, 16)) continue;

        char *name = *(char **)item;
        if (!name || IsBadReadPtr(name, 10)) continue;

        if (_strnicmp(name, "MAGNET(", 7) != 0) continue;

        char type = name[7];
        if (type != 'P' && type != 'p' && type != 'N' && type != 'n') continue;

        MagnetPoint *m   = &g_magnets[g_magnet_count++];
        m->x             = *(float *)(item + 4);
        m->y             = *(float *)(item + 8);
        m->z             = *(float *)(item + 12);
        m->is_positive   = (type == 'P' || type == 'p') ? 1 : 0;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config Reader
 * ═══════════════════════════════════════════════════════════════════════════ */

static void read_config(void) {
    char path[MAX_PATH];
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                        (LPCSTR)&read_config, &hSelf);
    GetModuleFileNameA(hSelf, path, MAX_PATH);
    char *p = strrchr(path, '\\');
    if (p) strcpy(p + 1, "magnet_config.txt");
    else   strcat(path, "\\magnet_config.txt");

    FILE *f = fopen(path, "r");
    if (!f) {
        f = fopen(path, "w");
        if (f) {
            fprintf(f, "# Magnet Mod Config\n");
            fprintf(f, "# S1 points: MAGNET(P)=attract, MAGNET(N)=repel\n\n");
            fprintf(f, "range = 300.0\n");
            fprintf(f, "strength = 0.5\n");
            fclose(f);
        }
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char *s = line;
        while (*s == ' ' || *s == '\t') s++;
        if (*s == '#' || *s == '\n' || *s == '\r' || *s == '\0') continue;

        char key[64];
        float val;
        if (sscanf(s, "%63[^=] = %f", key, &val) == 2) {
            char *k = key;
            while (*k == ' ') k++;
            char *end = k + strlen(k) - 1;
            while (end > k && (*end == ' ' || *end == '\t')) *end-- = '\0';

            if (_stricmp(k, "range")     == 0) g_range     = val;
            else if (_stricmp(k, "strength") == 0) g_strength   = val;
        }
    }
    fclose(f);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Present hook C helper — compute magnet force in safe C context
 *
 * Called from the Present hook cave (function-entry, safe for C calls).
 * Reads ball position, computes total magnet force, stores in globals
 * for the Phase 15 cave to apply.
 * ═══════════════════════════════════════════════════════════════════════════ */

static void __cdecl magnet_compute_force(void) {
    g_apply_force = 0;
    g_force_x = 0.0f;
    g_force_y = 0.0f;
    g_force_z = 0.0f;

    if (g_magnet_count == 0) return;

    DWORD ball = get_player_ball_v2();
    if (!ball) return;

    /* Check ball is in active play */
    if (IsBadReadPtr((void*)(ball + 0x769), 1)) return;
    BYTE active = *(BYTE*)(ball + 0x769);
    if (!active) return;

    /* Skip if in tube/fall/respawn */
    if (IsBadReadPtr((void*)(ball + 0x324), 1)) return;
    if (*(BYTE*)(ball + 0x324)) return;  /* in tube */
    if (IsBadReadPtr((void*)(ball + 0xC4C), 4)) return;
    if (*(DWORD*)(ball + 0xC4C) != 0) return;  /* fall mode */

    if (IsBadReadPtr((void*)(ball + 0x164), 12)) return;
    float bx = *(float*)(ball + 0x164);
    float by = *(float*)(ball + 0x168);
    float bz = *(float*)(ball + 0x16C);

    /* Skip if ball position not yet initialized */
    if (bx == 0.0f && by == 0.0f && bz == 0.0f) return;

    float total_fx = 0.0f, total_fy = 0.0f, total_fz = 0.0f;

    for (int i = 0; i < g_magnet_count; i++) {
        MagnetPoint *m = &g_magnets[i];

        float dx = m->x - bx;
        float dy = m->y - by;
        float dz = m->z - bz;
        float dist_sq = dx * dx + dy * dy + dz * dz;

        if (dist_sq > g_range * g_range) continue;
        if (dist_sq < 1.0f) continue;

        float dist = sqrtf(dist_sq);

        /* Linear falloff: full force at dist=0, zero at dist=range */
        float falloff = 1.0f - (dist / g_range);
        if (falloff <= 0.0f) continue;

        float force = g_strength * falloff;
        float inv_dist = 1.0f / dist;

        if (m->is_positive) {
            /* Attract: pull ball toward magnet */
            total_fx += dx * inv_dist * force;
            total_fy += dy * inv_dist * force;
            total_fz += dz * inv_dist * force;
        } else {
            /* Repel: push ball away from magnet */
            total_fx -= dx * inv_dist * force;
            total_fy -= dy * inv_dist * force;
            total_fz -= dz * inv_dist * force;
        }
    }

    if (total_fx != 0.0f || total_fy != 0.0f || total_fz != 0.0f) {
        g_force_x = total_fx;
        g_force_y = total_fy;
        g_force_z = total_fz;
        g_apply_force = 1;
    }
}

/* Function pointer for the Present hook cave to call the C helper */
static void (__cdecl *g_compute_force_fn)(void) = NULL;

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook 1: Graphics_PresentOrEnd (0x455A90) — function-entry hook
 *
 * Safe for calling C functions at function entry.
 * Original 7 bytes: 8A 44 24 04 83 EC 20
 *   MOV AL,[ESP+4]; SUB ESP,0x20
 * ═══════════════════════════════════════════════════════════════════════════ */

static BYTE *g_present_cave = NULL;

static void install_present_hook(void) {
    BYTE *hook_addr = (BYTE*)PRESENT_HOOK;

    BYTE expected[] = { 0x8A, 0x44, 0x24, 0x04, 0x83, 0xEC, 0x20 };
    if (memcmp(hook_addr, expected, 7) != 0) return;

    g_present_cave = (BYTE*)VirtualAlloc(NULL, 512, MEM_COMMIT | MEM_RESERVE,
                                           PAGE_EXECUTE_READWRITE);
    if (!g_present_cave) return;

    int p = 0;

    /* PUSHAD + PUSHFD (save all regs + flags) */
    g_present_cave[p++] = 0x60;  /* PUSHAD */
    g_present_cave[p++] = 0x9C;  /* PUSHFD */

    /* CALL [g_compute_force_fn] */
    g_present_cave[p++] = 0xFF; g_present_cave[p++] = 0x15;
    *(DWORD*)(g_present_cave + p) = (DWORD)&g_compute_force_fn; p += 4;

    /* POPFD + POPAD (restore) */
    g_present_cave[p++] = 0x9D;  /* POPFD */
    g_present_cave[p++] = 0x61;  /* POPAD */

    /* Original 7 bytes: 8A 44 24 04 83 EC 20 */
    g_present_cave[p++] = 0x8A; g_present_cave[p++] = 0x44;
    g_present_cave[p++] = 0x24; g_present_cave[p++] = 0x04;
    g_present_cave[p++] = 0x83; g_present_cave[p++] = 0xEC;
    g_present_cave[p++] = 0x20;

    /* JMP back to hook_addr + 7 */
    g_present_cave[p++] = 0xE9;
    *(DWORD*)(g_present_cave + p) = (DWORD)(hook_addr + PRESENT_ORIG_BYTES)
                                  - (DWORD)(g_present_cave + p + 4);
    p += 4;

    /* Patch hook site */
    DWORD old_protect;
    VirtualProtect(hook_addr, PRESENT_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);
    DWORD jmp_offset = (DWORD)(g_present_cave - hook_addr - 5);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_offset;
    hook_addr[5] = 0x90;
    hook_addr[6] = 0x90;
    VirtualProtect(hook_addr, PRESENT_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, PRESENT_ORIG_BYTES);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook 2: Phase 15 impulse (0x407BB4) — mid-function cave
 *
 * Original 6 bytes: 8B 4C 24 1C 8B 11 = MOV ECX,[ESP+1C]; MOV EDX,[ECX]
 * ESI = ball pointer at this point.
 *
 * If g_apply_force == 1: add magnet force to ball+0x170/0x174/0x178
 * using FLD/FADD/FSTP — same technique as the jump mod.
 * The physics engine then consumes these accumulators via vtable[0].
 * ═══════════════════════════════════════════════════════════════════════════ */

static BYTE *g_phase15_cave = NULL;

static void install_phase15_hook(void) {
    BYTE *hook_addr = (BYTE*)PHASE15_HOOK;

    BYTE expected[] = { 0x8B, 0x4C, 0x24, 0x1C, 0x8B, 0x11 };
    if (memcmp(hook_addr, expected, 6) != 0) return;

    g_phase15_cave = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE,
                                           PAGE_EXECUTE_READWRITE);
    if (!g_phase15_cave) return;

    int p = 0;

    /* Check g_apply_force == 1 */
    /* CMP [g_apply_force], 1 */
    g_phase15_cave[p++] = 0x83; g_phase15_cave[p++] = 0x3D;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_apply_force; p += 4;
    g_phase15_cave[p++] = 0x01;

    /* JNZ to .no_force */
    int jnz_fixup = p;
    g_phase15_cave[p++] = 0x0F; g_phase15_cave[p++] = 0x85;
    *(DWORD*)(g_phase15_cave + p) = 0; p += 4;

    /* ─── Apply force to ball+0x170 (X force accumulator) ─── */
    /* FLD [ESI+0x170] — load current X force */
    g_phase15_cave[p++] = 0xD9; g_phase15_cave[p++] = 0x86;
    *(DWORD*)(g_phase15_cave + p) = 0x170; p += 4;

    /* FADD [g_force_x] — add magnet X force */
    g_phase15_cave[p++] = 0xD8; g_phase15_cave[p++] = 0x05;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_force_x; p += 4;

    /* FSTP [ESI+0x170] — store back */
    g_phase15_cave[p++] = 0xD9; g_phase15_cave[p++] = 0x9E;
    *(DWORD*)(g_phase15_cave + p) = 0x170; p += 4;

    /* ─── Apply force to ball+0x174 (Y force accumulator) ─── */
    /* FLD [ESI+0x174] */
    g_phase15_cave[p++] = 0xD9; g_phase15_cave[p++] = 0x86;
    *(DWORD*)(g_phase15_cave + p) = 0x174; p += 4;

    /* FADD [g_force_y] */
    g_phase15_cave[p++] = 0xD8; g_phase15_cave[p++] = 0x05;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_force_y; p += 4;

    /* FSTP [ESI+0x174] */
    g_phase15_cave[p++] = 0xD9; g_phase15_cave[p++] = 0x9E;
    *(DWORD*)(g_phase15_cave + p) = 0x174; p += 4;

    /* ─── Apply force to ball+0x178 (Z force accumulator) ─── */
    /* FLD [ESI+0x178] */
    g_phase15_cave[p++] = 0xD9; g_phase15_cave[p++] = 0x86;
    *(DWORD*)(g_phase15_cave + p) = 0x178; p += 4;

    /* FADD [g_force_z] */
    g_phase15_cave[p++] = 0xD8; g_phase15_cave[p++] = 0x05;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_force_z; p += 4;

    /* FSTP [ESI+0x178] */
    g_phase15_cave[p++] = 0xD9; g_phase15_cave[p++] = 0x9E;
    *(DWORD*)(g_phase15_cave + p) = 0x178; p += 4;

    /* Clear g_apply_force */
    g_phase15_cave[p++] = 0xC7; g_phase15_cave[p++] = 0x05;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_apply_force; p += 4;
    *(DWORD*)(g_phase15_cave + p) = 0; p += 4;

    /* ─── .no_force: ─── */
    int no_force_target = p;

    /* Original 6 bytes: 8B 4C 24 1C 8B 11 */
    g_phase15_cave[p++] = 0x8B; g_phase15_cave[p++] = 0x4C;
    g_phase15_cave[p++] = 0x24; g_phase15_cave[p++] = 0x1C;
    g_phase15_cave[p++] = 0x8B; g_phase15_cave[p++] = 0x11;

    /* JMP back to hook_addr + 6 */
    g_phase15_cave[p++] = 0xE9;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)(hook_addr + PHASE15_ORIG_BYTES)
                                  - (DWORD)(g_phase15_cave + p + 4);
    p += 4;

    /* Fix up JNZ */
    *(DWORD*)(g_phase15_cave + jnz_fixup + 2) =
        (DWORD)(g_phase15_cave + no_force_target)
      - (DWORD)(g_phase15_cave + jnz_fixup + 6);

    /* Patch hook site */
    DWORD old_protect;
    VirtualProtect(hook_addr, PHASE15_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);
    DWORD jmp_offset = (DWORD)(g_phase15_cave - hook_addr - 5);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_offset;
    hook_addr[5] = 0x90;
    VirtualProtect(hook_addr, PHASE15_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, PHASE15_ORIG_BYTES);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Background thread: scan magnets on level load
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI MagnetScanThread(LPVOID param) {
    /* Wait for App global */
    int wait = 0;
    while (!(*(DWORD *)ADDR_App) && wait < 100) {
        Sleep(100); wait++;
    }
    if (!(*(DWORD *)ADDR_App)) return 0;
    Sleep(1000);

    read_config();

    while (1) {
        Sleep(200);  /* check every 200ms for board changes */

        DWORD board = get_board_v2();
        if (!board) continue;

        if (board != g_last_board) {
            scan_magnets(board);
        }
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch thread: install hooks after game stabilizes
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI PatchThread(LPVOID param) {
    /* Wait for App global */
    int wait = 0;
    while (!(*(DWORD *)ADDR_App) && wait < 100) {
        Sleep(100); wait++;
    }
    if (!(*(DWORD *)ADDR_App)) return 0;
    Sleep(500);

    /* Initialize C helper function pointer */
    g_compute_force_fn = magnet_compute_force;

    install_present_hook();
    install_phase15_hook();

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry Point
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        load_real_bass();
        CreateThread(NULL, 0, PatchThread,    NULL, 0, NULL);
        CreateThread(NULL, 0, MagnetScanThread, NULL, 0, NULL);
    }
    return TRUE;
}
