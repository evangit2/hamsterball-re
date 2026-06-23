/*
 * wall_bumper_mod.c — ALL WALLS ARE BUMPERS (v2)
 *
 * Hooks Ball_AI_ChaseNearest (0x00408390) — the player's per-frame update
 * function that IS called every frame for the player in race mode.
 * After the original runs (force applied, physics processed), we scan
 * the collision entry list on the PhysicsObject for type==2 (wall) hits.
 * For each wall hit, we push the ball away from the wall surface along
 * the collision normal and amplify velocity — simulating a pinball bumper.
 *
 * v1 FAILED: hooked Ball_FallUpdate (0x408830) — DEAD CODE, never called
 *             for the player in race mode (no code xrefs, only vtable DATA ref).
 * v2: Hooks Ball_AI_ChaseNearest (0x408390) which IS called for the player.
 *      Scans collision entries AFTER the original function returns. The
 *      collision entries from the previous frame's physics step persist
 *      in the PhysicsObject's collision list until the next frame.
 *
 * Also sets ball+0x808 (speed_boost counter) to briefly disable player
 * input during the bumper launch — same mechanism real Hamsterball bumpers use.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * HOOK DETAILS
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Target:   Ball_AI_ChaseNearest at 0x00408390 (__thiscall, ECX = ball)
 *
 * First instructions (6 bytes total — clean instruction boundary):
 *   0x408390: 83 EC 30           SUB ESP,0x30     (3 bytes)
 *   0x408393: 56                 PUSH ESI         (1 byte)
 *   0x408394: 8B F1              MOV ESI,ECX      (2 bytes)
 *
 * 5-byte JMP + 1 NOP = 6 bytes. Trampoline = 6 original bytes + JMP to 0x408396.
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

typedef void (WINAPI *BASS_FUNC)();

/* Forward declarations for BASS functions we proxy */
#define DEFINE_BASS_FORWARDED(name, ordinal) \
    static BASS_FUNC g_pfn_##name = NULL; \
    __declspec(dllexport) void WINAPI name() { \
        if (!g_pfn_##name) { \
            HMODULE h = LoadLibraryA("bass_real.dll"); \
            if (h) g_pfn_##name = (BASS_FUNC)GetProcAddress(h, #name); \
        } \
        if (g_pfn_##name) g_pfn_##name(); \
    }

/* Forward all BASS exports to bass_real.dll */
DEFINE_BASS_FORWARDED(BASS_Init, 1)
DEFINE_BASS_FORWARDED(BASS_Free, 2)
DEFINE_BASS_FORWARDED(BASS_GetVersion, 3)
DEFINE_BASS_FORWARDED(BASS_ErrorGetCode, 4)
DEFINE_BASS_FORWARDED(BASS_SetDevice, 5)
DEFINE_BASS_FORWARDED(BASS_GetDevice, 6)
DEFINE_BASS_FORWARDED(BASS_Start, 7)
DEFINE_BASS_FORWARDED(BASS_Stop, 8)
DEFINE_BASS_FORWARDED(BASS_Pause, 9)
DEFINE_BASS_FORWARDED(BASS_SetVolume, 10)
DEFINE_BASS_FORWARDED(BASS_GetVolume, 11)
DEFINE_BASS_FORWARDED(BASS_PluginLoad, 12)
DEFINE_BASS_FORWARDED(BASS_PluginFree, 13)
DEFINE_BASS_FORWARDED(BASS_MusicLoad, 14)
DEFINE_BASS_FORWARDED(BASS_MusicFree, 15)
DEFINE_BASS_FORWARDED(BASS_SampleLoad, 16)
DEFINE_BASS_FORWARDED(BASS_SampleCreate, 17)
DEFINE_BASS_FORWARDED(BASS_SampleFree, 18)
DEFINE_BASS_FORWARDED(BASS_SampleGetChannel, 19)
DEFINE_BASS_FORWARDED(BASS_SampleStop, 20)
DEFINE_BASS_FORWARDED(BASS_ChannelPlay, 21)
DEFINE_BASS_FORWARDED(BASS_ChannelStop, 22)
DEFINE_BASS_FORWARDED(BASS_ChannelPause, 23)
DEFINE_BASS_FORWARDED(BASS_ChannelSetPosition, 24)
DEFINE_BASS_FORWARDED(BASS_ChannelGetPosition, 25)
DEFINE_BASS_FORWARDED(BASS_ChannelSetSync, 26)
DEFINE_BASS_FORWARDED(BASS_ChannelRemoveSync, 27)
DEFINE_BASS_FORWARDED(BASS_ChannelGetLevel, 28)
DEFINE_BASS_FORWARDED(BASS_ChannelGetData, 29)
DEFINE_BASS_FORWARDED(BASS_ChannelSetAttribute, 30)
DEFINE_BASS_FORWARDED(BASS_ChannelGetAttribute, 31)
DEFINE_BASS_FORWARDED(BASS_ChannelGetInfo, 32)
DEFINE_BASS_FORWARDED(BASS_ChannelFlags, 33)
DEFINE_BASS_FORWARDED(BASS_ChannelUpdate, 34)
DEFINE_BASS_FORWARDED(BASS_StreamCreateFile, 35)
DEFINE_BASS_FORWARDED(BASS_StreamFree, 36)

/* ═══════════════════════════════════════════════════════════════════════════
 * Game Constants & Struct Offsets
 * ═══════════════════════════════════════════════════════════════════════════ */

#define APP_PTR                0x005341E0
#define HOOK_TARGET            0x00408390
#define HOOK_SIZE              6
#define TRAMPOLINE_SIZE        (HOOK_SIZE + 5)

/* Ball struct offsets */
#define BALL_PHYSICS           0x1A4   /* void* PhysicsObject */
#define BALL_POS_X             0x164   /* float */
#define BALL_POS_Y             0x168   /* float */
#define BALL_POS_Z             0x16C   /* float */
#define BALL_SPEED_BOOST       0x808   /* int: speed_boost counter */
#define BALL_FALLING_FLAG      0x2F9   /* byte: 1 when falling/despawning */

/* PhysicsObject struct offsets */
#define PHYS_COLLISION_COUNT   0x1C    /* int */
#define PHYS_COLLISION_ARRAY   0x424   /* void**: array of CollisionEntry* */
#define PHYS_SPEED             0xC64   /* float: speed magnitude */
#define PHYS_VEL_X             0xC98   /* float: velocity X */
#define PHYS_VEL_Y             0xC9C   /* float: velocity Y */
#define PHYS_VEL_Z             0xCA0   /* float: velocity Z */

/* CollisionEntry struct offsets */
#define ENTRY_TYPE             0x00    /* int32: 1=ball-ball, 2=wall, 5=floor */
#define ENTRY_NORMAL_X         0x20    /* float: surface normal X */
#define ENTRY_NORMAL_Y         0x24    /* float: surface normal Y */
#define ENTRY_NORMAL_Z         0x28    /* float: surface normal Z */

/* Bumper force presets */
static const float BUMPER_FORCES[] = { 40.0f, 60.0f, 80.0f, 120.0f, 200.0f };
#define NUM_FORCES (sizeof(BUMPER_FORCES) / sizeof(BUMPER_FORCES[0]))

/* ═══════════════════════════════════════════════════════════════════════════
 * Globals
 * ═══════════════════════════════════════════════════════════════════════════ */

static int g_mod_enabled = 1;
static int g_force_index = 2;  /* default: 80 */
static void *g_trampoline = NULL;
static int g_initialized = 0;

/* Keyboard state (DIK codes) */
#define DIK_F8  0x42
#define DIK_F9  0x43

/* ═══════════════════════════════════════════════════════════════════════════
 * Safe Memory Access
 * ═══════════════════════════════════════════════════════════════════════════ */

static int safe_read(void *addr, void *dest, int size) {
    if (!addr || IsBadReadPtr(addr, size)) return 0;
    memcpy(dest, addr, size);
    return 1;
}

static int safe_read_int(void *addr) {
    int val = 0;
    if (!safe_read(addr, &val, sizeof(val))) return 0;
    return val;
}

static float safe_read_float(void *addr) {
    float val = 0.0f;
    if (!safe_read(addr, &val, sizeof(val))) return 0.0f;
    return val;
}

/* Fast inverse square root (Quake III style) */
static float fast_inv_sqrt(float x) {
    union { float f; DWORD d; } inv;
    DWORD i = *(DWORD*)&x;
    i = 0x5f3759df - (i >> 1);
    inv.d = i;
    inv.f *= (1.5f - 0.5f * x * inv.f * inv.f);
    return inv.f;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook: Ball_AI_ChaseNearest
 *
 * Called every frame for the player ball in race mode.
 * After the original runs, we scan the PhysicsObject's collision list
 * for wall hits (type==2) and apply a pinball bumper effect.
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef void (__fastcall *BallAIChaseNearest_t)(void *ball, void *edx_dummy);
static BallAIChaseNearest_t g_orig_BallAIChaseNearest = NULL;

static void __fastcall hook_BallAIChaseNearest(void *ball, void *edx_dummy) {
    /* Call original function first (applies input force, AI logic, etc.) */
    if (g_orig_BallAIChaseNearest)
        g_orig_BallAIChaseNearest(ball, edx_dummy);

    if (!g_mod_enabled) return;
    if (!ball || IsBadReadPtr(ball, 0x200)) return;

    /* Don't process when ball is falling/despawning */
    if (safe_read_int((char*)ball + BALL_FALLING_FLAG))
        return;

    /* Get PhysicsObject from ball+0x1A4 */
    void *physics = NULL;
    if (!safe_read((char*)ball + BALL_PHYSICS, &physics, sizeof(physics)) || !physics)
        return;
    if (IsBadReadPtr(physics, 0x428)) return;

    /* Read collision count and array from PhysicsObject */
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
            if (mag > 0.0001f) {
                if (mag < 0.99f || mag > 1.01f) {
                    float inv = fast_inv_sqrt(mag);
                    nx *= inv;
                    ny *= inv;
                    nz *= inv;
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
            float inv = fast_inv_sqrt(mag);
            push_x *= inv;
            push_y *= inv;
            push_z *= inv;
        }
    }

    /* Push ball position away from wall (immediate displacement) */
    *(float*)((char*)ball + BALL_POS_X) += push_x * force;
    *(float*)((char*)ball + BALL_POS_Y) += push_y * force;
    *(float*)((char*)ball + BALL_POS_Z) += push_z * force;

    /* Overwrite velocity along wall normal (prevents the ball from
     * immediately flying back into the wall next frame) */
    if (!IsBadReadPtr(physics, 0xCA4)) {
        *(float*)((char*)physics + PHYS_VEL_X) = push_x * force;
        *(float*)((char*)physics + PHYS_VEL_Y) = push_y * force;
        *(float*)((char*)physics + PHYS_VEL_Z) = push_z * force;
        *(float*)((char*)physics + PHYS_SPEED) = force;
    }

    /* Set speed_boost counter — briefly disables player input,
     * simulating the "launch" effect of real pinball bumpers */
    *(int*)((char*)ball + BALL_SPEED_BOOST) = 3;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Detour Installation (6-byte: 5-byte JMP + 1 NOP)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void install_detour(void) {
    DWORD old_protect;
    BYTE *target = (BYTE*)HOOK_TARGET;
    BYTE jump_instr[5] = { 0xE9, 0, 0, 0, 0 };

    /* Allocate trampoline */
    g_trampoline = VirtualAlloc(NULL, TRAMPOLINE_SIZE + 16,
                                 MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_trampoline) return;

    BYTE *tramp = (BYTE*)g_trampoline;

    /* Copy original bytes to trampoline */
    memcpy(tramp, target, HOOK_SIZE);

    /* Add JMP from trampoline back to target+HOOK_SIZE */
    tramp[HOOK_SIZE] = 0xE9;
    DWORD rel_addr = (DWORD)target + HOOK_SIZE - ((DWORD)tramp + HOOK_SIZE + 5);
    *(DWORD*)(tramp + HOOK_SIZE + 1) = rel_addr;

    /* Set up function pointer to trampoline */
    g_orig_BallAIChaseNearest = (BallAIChaseNearest_t)tramp;

    /* Write JMP from target to hook function */
    DWORD hook_addr = (DWORD)hook_BallAIChaseNearest;
    rel_addr = hook_addr - ((DWORD)target + 5);
    jump_instr[1] = rel_addr & 0xFF;
    jump_instr[2] = (rel_addr >> 8) & 0xFF;
    jump_instr[3] = (rel_addr >> 16) & 0xFF;
    jump_instr[4] = (rel_addr >> 24) & 0xFF;

    VirtualProtect(target, HOOK_SIZE, PAGE_EXECUTE_READWRITE, &old_protect);
    memcpy(target, jump_instr, 5);
    target[5] = 0x90;  /* NOP */
    VirtualProtect(target, HOOK_SIZE, old_protect, &old_protect);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Keyboard Handling
 * ═══════════════════════════════════════════════════════════════════════════ */

static void handle_keyboard(void) {
    /* Check F8 (toggle) */
    if (GetAsyncKeyState(DIK_F8) & 0x8000) {
        g_mod_enabled = !g_mod_enabled;
        Sleep(200);  /* debounce */
    }

    /* Check F9 (cycle force) */
    if (GetAsyncKeyState(DIK_F9) & 0x8000) {
        g_force_index = (g_force_index + 1) % NUM_FORCES;
        Sleep(200);  /* debounce */
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Main Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI main_thread(LPVOID param) {
    /* Wait for game to fully load */
    Sleep(3000);

    /* Install detour */
    install_detour();
    g_initialized = 1;

    /* Main loop — handle keyboard */
    while (1) {
        handle_keyboard();
        Sleep(50);
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinst);
        CreateThread(NULL, 0, main_thread, NULL, 0, NULL);
    }
    return TRUE;
}
