/*
 * wall_bumper_mod.c — ALL WALLS ARE BUMPERS
 *
 * Hooks Ball_FallUpdate (0x00408830) — the player's physics/collision
 * function. After the original runs, we scan the collision entry list
 * for type==2 (wall) hits. For each wall hit, we push the ball away
 * from the wall surface along the collision normal, simulating a
 * pinball bumper effect on EVERY wall in the game.
 *
 * Also sets ball+0x808 (speed_boost counter) briefly to disable player
 * input during the bumper launch — just like real Hamsterball bumpers do.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * HOOK DETAILS
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Target:   Ball_FallUpdate at 0x00408830 (__thiscall, ECX = ball)
 *
 * First instructions (7 bytes total — crosses instruction boundary):
 *   0x408830: 6A FF                 PUSH -1           (2 bytes)
 *   0x408832: 68 57 94 4C 00        PUSH 0x004C9457   (5 bytes)
 *
 * A 5-byte JMP covers 0x408830-0x408834 (2+3 partial), so we need 7 bytes
 * total: 5-byte JMP + 2 NOPs. Trampoline = original 7 bytes + JMP to 0x408837.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * BUILD
 * ═══════════════════════════════════════════════════════════════════════════
 *
 *   i686-w64-mingw32-gcc -shared -o bass.dll wall_bumper_mod.c \
 *       -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *       -Wl,--add-stdcall-alias
 *
 * INSTALL:
 *   1. Rename original bass.dll → bass_real.dll in your Hamsterball folder
 *   2. Copy this bass.dll into the game folder
 *   3. Launch the game — all walls now act as bumpers
 *
 * CONTROLS:
 *   F8 — Toggle mod on/off (default: ON)
 *   F9 — Cycle bumper force (40 → 60 → 80 → 120 → 200)
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports — v3 lazy loader with stub fallback
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;
static int g_bass_tried_load = 0;

static void load_real_bass(void) {
    g_bass_tried_load = 1;
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char *slash = strrchr(path, '\\');
    if (slash) strcpy(slash + 1, "bass_real.dll");
    else strcpy(path, "bass_real.dll");
    g_hRealBass = LoadLibraryA(path);
}

#define DEFINE_BASS_FORWARDED(name, ret_type, params, args, stub_ret) \
    typedef ret_type (__stdcall *name##_t) params; \
    static name##_t real_##name = NULL; \
    __declspec(dllexport) ret_type __stdcall name params { \
        if (!g_bass_tried_load) load_real_bass(); \
        if (g_hRealBass && !real_##name) \
            real_##name = (name##_t)GetProcAddress(g_hRealBass, #name); \
        if (real_##name) return real_##name args; \
        return stub_ret; \
    }

DEFINE_BASS_FORWARDED(BASS_Init,
    int, (int a, DWORD b, DWORD c, HWND d, void* e), (a,b,c,d,e), 1)
DEFINE_BASS_FORWARDED(BASS_Free,
    int, (void), (), 1)
DEFINE_BASS_FORWARDED(BASS_Start,
    int, (void), (), 1)
DEFINE_BASS_FORWARDED(BASS_Stop,
    int, (void), (), 1)
DEFINE_BASS_FORWARDED(BASS_SetConfig,
    int, (DWORD a, DWORD b), (a,b), 1)
DEFINE_BASS_FORWARDED(BASS_ErrorGetCode,
    int, (void), (), 0)
DEFINE_BASS_FORWARDED(BASS_MusicLoad,
    DWORD, (int a, void* b, DWORD c, DWORD d, DWORD e, DWORD f), (a,b,c,d,e,f), 0)
DEFINE_BASS_FORWARDED(BASS_MusicPlayEx,
    int, (DWORD a, DWORD b, BOOL c), (a,b,c), 1)
DEFINE_BASS_FORWARDED(BASS_MusicFree,
    int, (DWORD a), (a), 1)
DEFINE_BASS_FORWARDED(BASS_ChannelSetAttributes,
    int, (DWORD a, float b, int c, int d), (a,b,c,d), 1)
DEFINE_BASS_FORWARDED(BASS_ChannelStop,
    int, (DWORD a), (a), 1)

/* ═══════════════════════════════════════════════════════════════════════════
 * Game Constants
 * ═══════════════════════════════════════════════════════════════════════════ */

#define IMAGE_BASE          0x00400000
#define ADDR_BallFallUpdate (IMAGE_BASE + 0x0008830)  /* 0x00408830 */
#define ADDR_App            0x005341E0

/* Ball struct offsets */
#define BALL_POS_X          0x164    /* float */
#define BALL_POS_Y          0x168    /* float */
#define BALL_POS_Z          0x16C    /* float */
#define BALL_PHYSICS        0x1A4    /* void* → PhysicsObject */
#define BALL_SPEED_BOOST    0x808    /* int: decrement each frame, disables input */

/* PhysicsObject struct offsets */
#define PHYS_COLLISION_COUNT  0x1C   /* int */
#define PHYS_COLLISION_ARRAY  0x424  /* void**: array of CollisionEntry* */

/* CollisionEntry struct offsets */
#define ENTRY_TYPE       0x00   /* int32: 1=ball-ball, 2=wall, 5=floor */
#define ENTRY_NORMAL_X   0x20   /* float */
#define ENTRY_NORMAL_Y   0x24   /* float */
#define ENTRY_NORMAL_Z   0x28   /* float */

/* Original bytes at Ball_FallUpdate entry (7 bytes = 2 instructions) */
static const BYTE ORIG_BYTES[7] = {
    0x6A, 0xFF,                          /* PUSH -1 */
    0x68, 0x57, 0x94, 0x4C, 0x00         /* PUSH 0x004C9457 */
};

/* Detour size: 5-byte JMP + 2 NOPs = 7 bytes */
#define DETOUR_SIZE 7
#define TRAMP_SIZE  16  /* 7 original + 5 JMP + padding */

/* ═══════════════════════════════════════════════════════════════════════════
 * State
 * ═══════════════════════════════════════════════════════════════════════════ */

static unsigned char g_trampoline[TRAMP_SIZE];
static int g_hook_installed = 0;

/* __thiscall workaround: __fastcall with dummy EDX */
typedef void (__fastcall *BallFallUpdate_t)(void *ball, void *edx_dummy);
static BallFallUpdate_t g_orig_BallFallUpdate = NULL;

/* Mod state */
static volatile int g_mod_enabled = 1;
static volatile int g_force_index = 0;
static const float BUMPER_FORCES[] = { 40.0f, 60.0f, 80.0f, 120.0f, 200.0f };
#define NUM_FORCES (sizeof(BUMPER_FORCES) / sizeof(BUMPER_FORCES[0]))

/* ═══════════════════════════════════════════════════════════════════════════
 * Safe Memory Read
 * ═══════════════════════════════════════════════════════════════════════════ */

static int safe_read(void *addr, void *dest, size_t len) {
    if (!addr || !dest || len == 0) return 0;
    if (IsBadReadPtr(addr, len)) return 0;
    memcpy(dest, addr, len);
    return 1;
}

static float safe_read_float(void *addr) {
    float val = 0.0f;
    safe_read(addr, &val, 4);
    return val;
}

static int safe_read_int(void *addr) {
    int val = 0;
    safe_read(addr, &val, 4);
    return val;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook: Ball_FallUpdate
 *
 * After the original function completes:
 * 1. Read collision entries from physics object
 * 2. For each type==2 (wall) entry, read the wall normal
 * 3. Push ball position along the normal by BUMPER_FORCE
 * 4. Set ball+0x808 (speed_boost) for brief input disable
 * ═══════════════════════════════════════════════════════════════════════════ */

static void __fastcall hook_BallFallUpdate(void *ball, void *edx_dummy) {
    /* Call original function via trampoline */
    if (g_orig_BallFallUpdate)
        g_orig_BallFallUpdate(ball, edx_dummy);

    if (!g_mod_enabled) return;
    if (!ball || IsBadReadPtr(ball, 0x200)) return;

    /* Get physics object from ball+0x1A4 */
    void *physics = NULL;
    if (!safe_read((char*)ball + BALL_PHYSICS, &physics, sizeof(physics)) || !physics)
        return;
    if (IsBadReadPtr(physics, 0x428)) return;

    /* Read collision count and array */
    int count = safe_read_int((char*)physics + PHYS_COLLISION_COUNT);
    if (count <= 0 || count > 200) return;

    void **entries = NULL;
    if (!safe_read((char*)physics + PHYS_COLLISION_ARRAY, &entries, sizeof(entries)) || !entries)
        return;
    if (IsBadReadPtr(entries, count * sizeof(void*))) return;

    /* Scan for wall collisions (type == 2) */
    int wall_hits = 0;
    float push_x = 0.0f, push_y = 0.0f, push_z = 0.0f;

    for (int i = 0; i < count; i++) {
        void *entry = NULL;
        if (!safe_read(&entries[i], &entry, sizeof(entry)) || !entry)
            continue;
        if (IsBadReadPtr(entry, 0x30))
            continue;

        int type = safe_read_int(entry);
        if (type == 2) {  /* Wall collision */
            float nx = safe_read_float((char*)entry + ENTRY_NORMAL_X);
            float ny = safe_read_float((char*)entry + ENTRY_NORMAL_Y);
            float nz = safe_read_float((char*)entry + ENTRY_NORMAL_Z);

            /* Normalize (game normals are usually already normalized, but be safe) */
            float mag = nx*nx + ny*ny + nz*nz;
            if (mag > 0.0001f && mag != 1.0f) {
                /* Already normalized if mag ≈ 1.0; skip sqrt if close */
                if (mag < 0.99f || mag > 1.01f) {
                    /* Approximate normalize without math.h */
                    union { float f; DWORD d; } inv_sqrt;
                    DWORD i = *(DWORD*)&mag;
                    i = 0x5f3759df - (i >> 1);
                    inv_sqrt.d = i;
                    inv_sqrt.f *= (1.5f - 0.5f * mag * inv_sqrt.f * inv_sqrt.f);
                    nx *= inv_sqrt.f;
                    ny *= inv_sqrt.f;
                    nz *= inv_sqrt.f;
                }
            }

            push_x += nx;
            push_y += ny;
            push_z += nz;
            wall_hits++;
        }
    }

    if (wall_hits == 0) return;

    /* Apply bumper force along accumulated wall normal */
    float force = BUMPER_FORCES[g_force_index];

    /* If multiple walls hit, normalize the accumulated push direction */
    if (wall_hits > 1) {
        float mag = push_x*push_x + push_y*push_y + push_z*push_z;
        if (mag > 0.0001f) {
            union { float f; DWORD d; } inv_sqrt;
            DWORD i = *(DWORD*)&mag;
            i = 0x5f3759df - (i >> 1);
            inv_sqrt.d = i;
            inv_sqrt.f *= (1.5f - 0.5f * mag * inv_sqrt.f * inv_sqrt.f);
            push_x *= inv_sqrt.f;
            push_y *= inv_sqrt.f;
            push_z *= inv_sqrt.f;
        }
    }

    /* Push ball away from wall */
    *(float*)((char*)ball + BALL_POS_X) += push_x * force;
    *(float*)((char*)ball + BALL_POS_Y) += push_y * force;
    *(float*)((char*)ball + BALL_POS_Z) += push_z * force;

    /* Set speed_boost counter — briefly disables player input,
     * simulating the "launch" effect of real pinball bumpers */
    *(int*)((char*)ball + BALL_SPEED_BOOST) = 3;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Detour Installation (7-byte: 5-byte JMP + 2 NOPs)
 * ═══════════════════════════════════════════════════════════════════════════ */

static int install_detour(void *target, void *hook, unsigned char *trampoline) {
    DWORD oldProtect;
    unsigned char *t = (unsigned char *)target;

    /* Verify original bytes match */
    if (memcmp(t, ORIG_BYTES, DETOUR_SIZE) != 0)
        return 0;

    if (!VirtualProtect(t, TRAMP_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect))
        return 0;

    /* Copy original 7 bytes to trampoline */
    memcpy(trampoline, t, DETOUR_SIZE);

    /* Append JMP rel32 back to target+7 */
    trampoline[DETOUR_SIZE] = 0xE9;  /* JMP rel32 */
    *(unsigned long *)(trampoline + DETOUR_SIZE + 1) =
        (unsigned long)((char *)target + DETOUR_SIZE - (char *)(trampoline + DETOUR_SIZE) - 5);

    /* Make trampoline executable */
    DWORD tp;
    VirtualProtect(trampoline, TRAMP_SIZE, PAGE_EXECUTE_READWRITE, &tp);

    /* Overwrite target: JMP rel32 to hook (5 bytes) + 2 NOPs */
    unsigned long rel = (unsigned long)((char *)hook - (char *)target - 5);
    t[0] = 0xE9;
    *(unsigned long *)(t + 1) = rel;
    t[5] = 0x90;  /* NOP */
    t[6] = 0x90;  /* NOP */

    FlushInstructionCache(GetCurrentProcess(), target, DETOUR_SIZE);
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Keyboard Polling Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI key_poll_thread(LPVOID param) {
    (void)param;
    int f8_was_down = 0;
    int f9_was_down = 0;

    while (1) {
        /* F8 — toggle mod on/off */
        int f8 = GetAsyncKeyState(VK_F8) & 0x8000;
        if (f8 && !f8_was_down) {
            g_mod_enabled = !g_mod_enabled;
            char msg[128];
            wsprintfA(msg, "Wall Bumpers: %s\nForce: %.0f",
                      g_mod_enabled ? "ON" : "OFF",
                      BUMPER_FORCES[g_force_index]);
            MessageBoxA(NULL, msg, "Wall Bumper Mod", MB_OK | MB_ICONINFORMATION);
        }
        f8_was_down = f8;

        /* F9 — cycle bumper force */
        int f9 = GetAsyncKeyState(VK_F9) & 0x8000;
        if (f9 && !f9_was_down) {
            g_force_index = (g_force_index + 1) % NUM_FORCES;
            char msg[128];
            wsprintfA(msg, "Bumper Force: %.0f", BUMPER_FORCES[g_force_index]);
            MessageBoxA(NULL, msg, "Wall Bumper Mod", MB_OK | MB_ICONINFORMATION);
        }
        f9_was_down = f9;

        Sleep(50);  /* poll every 50ms */
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI patch_thread(LPVOID param) {
    (void)param;

    /* Wait for game to fully load */
    int wait = 0;
    while (!(*(DWORD*)ADDR_App) && wait < 100) {
        Sleep(100);
        wait++;
    }
    if (!(*(DWORD*)ADDR_App)) return 0;
    Sleep(500);  /* extra safety margin */

    /* Install detour on Ball_FallUpdate */
    void *target = (void*)ADDR_BallFallUpdate;
    g_orig_BallFallUpdate = (BallFallUpdate_t)g_trampoline;

    if (install_detour(target, (void*)hook_BallFallUpdate, g_trampoline)) {
        g_hook_installed = 1;
        MessageBoxA(NULL,
            "Wall Bumper Mod installed!\n\n"
            "ALL walls now act as pinball bumpers.\n\n"
            "F8: Toggle on/off (currently ON)\n"
            "F9: Cycle bumper force (currently 40)",
            "Wall Bumper Mod", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxA(NULL,
            "Wall Bumper Mod: FAILED to install hook!\n"
            "Byte mismatch at Ball_FallUpdate (0x408830).\n"
            "The game may be a different version.",
            "Wall Bumper Mod Error", MB_OK | MB_ICONERROR);
    }

    /* Start keyboard polling thread */
    CreateThread(NULL, 0, key_poll_thread, NULL, 0, NULL);

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry Point
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved) {
    (void)reserved;
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInst);
        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        /* Restore original bytes */
        if (g_hook_installed) {
            DWORD oldProtect;
            void *target = (void*)ADDR_BallFallUpdate;
            if (VirtualProtect(target, DETOUR_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                memcpy(target, ORIG_BYTES, DETOUR_SIZE);
                VirtualProtect(target, DETOUR_SIZE, oldProtect, &oldProtect);
                FlushInstructionCache(GetCurrentProcess(), target, DETOUR_SIZE);
            }
        }
        break;
    }
    return TRUE;
}
