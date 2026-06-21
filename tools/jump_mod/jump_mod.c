/*
 * jump_mod.c — BASS.dll proxy — v14 RAYCAST GROUND DETECTION
 *
 * v13 used Y displacement which fails on slopes.
 * v14 uses ACTUAL RAYCASTING via Ball_FindMeshCollision (0x403980)
 * to cast a ray downward from the ball and detect ground contact.
 *
 * Ground detection: cast ray from ball position straight down (0,-1,0).
 * If the closest collision point is within ball_radius + tolerance,
 * the ball is on the ground. Works on ALL surfaces including slopes.
 *
 * Hook: 0x407BB4 — adds impulse to ball+0x174 (Y force accumulator)
 * before Phase 15 physics integration.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll jump_mod.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports
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

/* ═══════════════════════════════════════════════════════════════════════════
 * Diagnostic logging
 * ═══════════════════════════════════════════════════════════════════════════ */

static char g_logpath1[MAX_PATH] = "";
static char g_logpath2[MAX_PATH] = "";

static void diag_log(const char *msg)
{
    const char *paths[] = { g_logpath1, g_logpath2 };
    int i;
    for (i = 0; i < 2; i++) {
        if (paths[i][0] == '\0') continue;
        HANDLE hFile = CreateFileA(paths[i],
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

/* ═══════════════════════════════════════════════════════════════════════════
 * Jump Mod v14 — velocity impulse with raycast ground detection
 * ═══════════════════════════════════════════════════════════════════════════ */

#define PHASE15_HOOK       0x00407BB4
#define HOOK_ORIG_BYTES    6

static float g_jump_impulse = 20.0f;

/* Ground detection tolerance: if raycast hit is within
 * ball_radius + GROUND_TOLERANCE below ball center, ball is grounded. */
#define GROUND_TOLERANCE   2.0f

static volatile DWORD g_jump_requested = 0;
static volatile DWORD g_frame_count = 0;
static volatile DWORD g_jump_count = 0;

/* ─── Raycast function type ──────────────────────────────────────────────── */

typedef struct { float x, y, z; } Vec3;

/* Ball_FindMeshCollision (0x403980) — thin wrapper over Mesh_FindClosestCollision.
 * __thiscall: ECX = this (mesh pointer from Scene+0x8B0)
 *   ret 0x20 (callee cleans 8 DWORD params from stack) */
typedef Vec3* (__thiscall *FindMeshCollision_fn)(
    void *mesh,
    Vec3 *output,
    float originX, float originY, float originZ,
    float dirX,    float dirY,    float dirZ,
    float param8
);

#define FN_FIND_MESH_COLLISION ((FindMeshCollision_fn)0x403980)

/* ─── Raycast ground detection ───────────────────────────────────────────── */

static Vec3 g_raycast_hit;
static volatile int g_raycast_ok = 0;

static int do_raycast_ground_check(DWORD ball)
{
    DWORD scene = *(DWORD*)(ball + 0x14);
    if (!scene) return 0;

    DWORD mesh = *(DWORD*)(scene + 0x8B0);
    if (!mesh) return 0;

    float ball_x = *(float*)(ball + 0x164);
    float ball_y = *(float*)(ball + 0x168);
    float ball_z = *(float*)(ball + 0x16C);
    float ball_radius = *(float*)(ball + 0x284);

    Vec3 result;

    /* Cast ray straight down from ball position */
    Vec3 *ret = FN_FIND_MESH_COLLISION(
        (void*)mesh,
        &result,
        ball_x, ball_y, ball_z,    /* origin: ball position */
        0.0f, -1.0f, 0.0f,         /* direction: straight down */
        0.0f                        /* param8: unused */
    );

    if (!ret) return 0;

    g_raycast_hit = result;
    g_raycast_ok = 1;

    /* Distance from ball center to ground hit (Y axis only) */
    float dy = ball_y - result.y;
    /* On ground: dy ≈ ball_radius (ball sitting on surface)
     * In air:    dy >> ball_radius (ground is far below) */
    if (dy < 0) dy = -dy;

    return (dy <= ball_radius + GROUND_TOLERANCE);
}

/* ─── Input polling thread with raycast ground detection ─────────────────── */
static volatile int g_prev_space = 0;

static DWORD WINAPI input_thread(LPVOID param)
{
    (void)param;
    char buf[256];
    int log_counter = 0;

    while (1) {
        Sleep(16);

        DWORD app = *(DWORD*)0x005341E0;
        if (!app) continue;

        DWORD ih = *(DWORD*)(app + 0x180);
        if (!ih) continue;

        DWORD kb = *(DWORD*)(ih + 0x434);
        if (!kb) continue;

        BYTE *keys = (BYTE*)(kb + 0xC);
        int space_down = (keys[0x39] & 0x80) != 0;

        if (space_down && !g_prev_space) {
            /* Spacebar just pressed — raycast ground check */
            DWORD scene = *(DWORD*)(app + 0x1A4);
            if (scene) {
                DWORD ball_list = scene + 0x29D4;
                int count = *(int*)(ball_list + 0x1C);
                if (count > 0) {
                    DWORD *balls = *(DWORD**)(ball_list + 0x424);
                    if (balls && balls[0]) {
                        DWORD ball = balls[0];
                        int on_ground = do_raycast_ground_check(ball);

                        if (on_ground) {
                            g_jump_requested = 1;
                            g_jump_count++;

                            if (log_counter < 100) {
                                wsprintfA(buf,
                                    "JUMP #%u: raycast GROUNDED (ball_y=%.1f hit_y=%.1f dist=%.1f radius=%.1f)",
                                    g_jump_count,
                                    *(float*)(ball + 0x168),
                                    g_raycast_hit.y,
                                    *(float*)(ball + 0x168) - g_raycast_hit.y,
                                    *(float*)(ball + 0x284));
                                diag_log(buf);
                                log_counter++;
                            }
                        } else {
                            if (log_counter < 100) {
                                wsprintfA(buf,
                                    "DENIED: AIRBORNE (ball_y=%.1f hit_y=%.1f dist=%.1f radius=%.1f)",
                                    *(float*)(ball + 0x168),
                                    g_raycast_hit.y,
                                    *(float*)(ball + 0x168) - g_raycast_hit.y,
                                    *(float*)(ball + 0x284));
                                diag_log(buf);
                                log_counter++;
                            }
                        }
                    }
                }
            }
        }
        g_prev_space = space_down;
    }
    return 0;
}

/* ─── Hook installation ──────────────────────────────────────────────────── */

static void install_hook(void)
{
    BYTE *hook_addr = (BYTE*)PHASE15_HOOK;
    char buf[256];

    BYTE *cave = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE);
    if (!cave) {
        diag_log("install_hook: VirtualAlloc FAILED!");
        return;
    }

    wsprintfA(buf, "install_hook: cave=%08X", (DWORD)cave);
    diag_log(buf);

    int p = 0;

    /* ─── 1. CMP [g_jump_requested], 0 ─── */
    cave[p++] = 0x83; cave[p++] = 0x3D;
    *(DWORD*)(cave + p) = (DWORD)&g_jump_requested; p += 4;
    cave[p++] = 0x00;

    /* ─── 2. JZ skip ─── */
    cave[p++] = 0x74;
    cave[p++] = 28;

    /* ─── 3. FLD [ESI+0x174] — load Y force accumulator ─── */
    cave[p++] = 0xD9; cave[p++] = 0x86;
    *(DWORD*)(cave + p) = 0x174; p += 4;

    /* ─── 4. FADD [g_jump_impulse] — add upward impulse ─── */
    cave[p++] = 0xD8; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_jump_impulse; p += 4;

    /* ─── 5. FSTP [ESI+0x174] — store modified Y force ─── */
    cave[p++] = 0xD9; cave[p++] = 0x9E;
    *(DWORD*)(cave + p) = 0x174; p += 4;

    /* ─── 6. MOV [g_jump_requested], 0 — consume the request ─── */
    cave[p++] = 0xC7; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_jump_requested; p += 4;
    *(DWORD*)(cave + p) = 0; p += 4;

    /* ─── skip: ─── */

    /* ─── 7. Original 6 bytes: MOV ECX,[ESP+1C]; MOV EDX,[ECX] ─── */
    cave[p++] = 0x8B; cave[p++] = 0x4C; cave[p++] = 0x24; cave[p++] = 0x1C;
    cave[p++] = 0x8B; cave[p++] = 0x11;

    /* ─── 8. INC [g_frame_count] ─── */
    cave[p++] = 0xFF; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_frame_count; p += 4;

    /* ─── 9. JMP back to hook_addr + 6 (= 0x407BBA) ─── */
    cave[p++] = 0xE9;
    *(DWORD*)(cave + p) = (DWORD)(hook_addr + HOOK_ORIG_BYTES) - (DWORD)(cave + p + 4);
    p += 4;

    wsprintfA(buf, "cave: %d bytes, installing...", p);
    diag_log(buf);

    /* Patch hook site */
    DWORD old_protect;
    VirtualProtect(hook_addr, HOOK_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);

    DWORD jmp_to_cave = (DWORD)(cave - hook_addr - 5);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_to_cave;
    hook_addr[5] = 0x90;

    VirtualProtect(hook_addr, HOOK_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, HOOK_ORIG_BYTES);

    wsprintfA(buf, "HOOK v14 INSTALLED! impulse=%f tolerance=%f cave=%08X",
              g_jump_impulse, GROUND_TOLERANCE, (DWORD)cave);
    diag_log(buf);
}

static DWORD WINAPI patch_thread(LPVOID param)
{
    (void)param;
    char buf[256];

    diag_log("patch_thread: started");
    Sleep(5000);

    BYTE *hook = (BYTE*)PHASE15_HOOK;
    BYTE expected[] = { 0x8B, 0x4C, 0x24, 0x1C, 0x8B, 0x11 };

    wsprintfA(buf, "Actual:   %02X %02X %02X %02X %02X %02X",
              hook[0], hook[1], hook[2], hook[3], hook[4], hook[5]);
    diag_log(buf);

    if (memcmp(hook, expected, 6) != 0) {
        diag_log("BYTE MISMATCH!");
        return 1;
    }

    install_hook();

    CreateThread(NULL, 0, input_thread, NULL, 0, NULL);
    diag_log("input_thread launched");

    Sleep(8000);
    wsprintfA(buf, "After 8s: frames=%u jumps=%u", g_frame_count, g_jump_count);
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

        GetTempPathA(MAX_PATH, g_logpath1);
        lstrcatA(g_logpath1, "hamsterball_jump_debug.txt");

        GetModuleFileNameA(hInst, g_logpath2, MAX_PATH);
        {
            char *p = strrchr(g_logpath2, '\\');
            if (p) strcpy(p + 1, "jump_debug.txt");
        }

        diag_log("=== jump_mod v14 loaded ===");

        load_real_bass();
        {
            char buf[128];
            wsprintfA(buf, "bass_real = %08X", (DWORD)g_hRealBass);
            diag_log(buf);
        }

        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
