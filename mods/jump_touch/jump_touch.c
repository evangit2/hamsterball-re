/*
 * jump_touch.c — BASS.dll proxy — Jump mod for touch/Android
 *
 * Tap the screen to jump! Uses left mouse button (Winlator maps touch to click).
 *
 * How it works:
 *   1. Background thread polls left-click (touch tap) every 16ms
 *   2. On tap: runs raycast straight down from ball position
 *      If ball is on ground → set g_want_jump=1
 *   3. Phase 15 code cave (pure asm): if g_want_jump!=0, add upward impulse
 *
 * Safety:
 *   - Phase 15 cave is PURE ASM (FSTP/FLD/FADD/MOV/JMP) — no C calls mid-function
 *   - Raycast runs in background thread (safe C context)
 *   - No IAT hooks, no GetTickCount
 *   - Three gates: countdown done, race not ended, player not finished
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll jump_touch.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>
#include <string.h>

#define IMAGE_BASE 0x00400000

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;

static void load_real_bass(void)
{
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (!g_hRealBass) {
        char path[MAX_PATH];
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&load_real_bass, &hSelf);
        if (GetModuleFileNameA(hSelf, path, MAX_PATH)) {
            char *p = strrchr(path, '\\');
            if (p) { strcpy(p + 1, "bass_real.dll"); g_hRealBass = LoadLibraryA(path); }
        }
    }
}

typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
    if (g_hRealBass) { if (!real_BASS_Init) real_BASS_Init = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init"); if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e); } return 1;
}
typedef int  (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (g_hRealBass) { if (!real_BASS_Free) real_BASS_Free = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free"); if (real_BASS_Free) return real_BASS_Free(); } return 1;
}
typedef int  (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (g_hRealBass) { if (!real_BASS_Start) real_BASS_Start = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start"); if (real_BASS_Start) return real_BASS_Start(); } return 1;
}
typedef int  (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (g_hRealBass) { if (!real_BASS_Stop) real_BASS_Stop = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop"); if (real_BASS_Stop) return real_BASS_Stop(); } return 1;
}
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (g_hRealBass) { if (!real_BASS_SetConfig) real_BASS_SetConfig = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig"); if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b); } return 1;
}
typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (g_hRealBass) { if (!real_BASS_ErrorGetCode) real_BASS_ErrorGetCode = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode"); if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode(); } return 0;
}
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    if (g_hRealBass) { if (!real_BASS_MusicPlayEx) real_BASS_MusicPlayEx = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx"); if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c); } return 1;
}
typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (g_hRealBass) { if (!real_BASS_ChannelSetAttributes) real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes"); if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d); } return 1;
}
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (g_hRealBass) { if (!real_BASS_ChannelStop) real_BASS_ChannelStop = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop"); if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a); } return 1;
}
typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int a, void* b, DWORD c, DWORD d, DWORD e, DWORD f) {
    if (g_hRealBass) { if (!real_BASS_MusicLoad) real_BASS_MusicLoad = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad"); if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f); } return 0;
}

/* Extra stubs */
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

/* ═══════════════════════════════════════════════════════════════════════════
 * Jump Mod
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Hook address */
#define PHASE15_HOOK       0x00407BB4
#define PHASE15_ORIG_BYTES 6

/* Game functions */
#define ADDR_Mesh_FindClosestCollision  0x00465D90
#define ADDR_App                        0x005341E0

/* Jump parameters */
static float g_jump_impulse = 20.0f;
static const float GROUND_SLOPE_FACTOR = 1.45f;

/* Shared state */
static volatile DWORD g_want_jump = 0;
static volatile DWORD g_ball_ptr = 0;

/* Mesh_FindClosestCollision wrapper — __thiscall via function pointer */
typedef float* (__attribute__((thiscall)) *raycast_fn_t)(
    void *mesh_data, float *out_hit,
    float ox, float oy, float oz,
    float dx, float dy, float dz,
    float radius
);

static float* do_raycast(void *mesh_data,
                          float ox, float oy, float oz,
                          float dx, float dy, float dz,
                          float radius_scale,
                          float *out_hit)
{
    raycast_fn_t fn = (raycast_fn_t)(DWORD)ADDR_Mesh_FindClosestCollision;
    return fn(mesh_data, out_hit, ox, oy, oz, dx, dy, dz, radius_scale);
}

/* Get player ball pointer */
static DWORD get_player_ball(void)
{
    DWORD app = *(DWORD*)ADDR_App;
    if (!app) return 0;

    DWORD scene = *(DWORD*)(app + 0x178);
    if (!scene) return 0;

    DWORD list_count = *(DWORD*)(scene + 0x29D4 + 0x04);
    if (list_count == 0) return 0;

    DWORD list_data = *(DWORD*)(scene + 0x29D4 + 0x40C);
    if (!list_data) return 0;

    return *(DWORD*)(list_data);
}

/* Raycast ground check — returns 1 if grounded, 0 if airborne */
static int is_ball_grounded(DWORD ball)
{
    if (!ball) return 0;
    if (IsBadReadPtr((void*)ball, 0xD00)) return 0;

    DWORD scene = *(DWORD*)(ball + 0x14);
    if (!scene) return 0;
    if (IsBadReadPtr((void*)scene, 0x1000)) return 0;

    DWORD mesh_data = *(DWORD*)(scene + 0x8B0);
    if (!mesh_data) return 0;
    if (IsBadReadPtr((void*)mesh_data, 0x100)) return 0;

    float ball_x = *(float*)(ball + 0x164);
    float ball_y = *(float*)(ball + 0x168);
    float ball_z = *(float*)(ball + 0x16C);
    float radius = *(float*)(ball + 0x284);
    if (radius <= 0.0f || radius > 1000.0f) return 0;

    float hit_result[3] = {0.0f, 0.0f, 0.0f};

    do_raycast((void*)mesh_data,
               ball_x, ball_y, ball_z,
               0.0f, -1.0f, 0.0f,
               1.0f,
               hit_result);

    float hit_y = hit_result[1];
    float dist = fabsf(hit_y - ball_y);
    float threshold = radius * GROUND_SLOPE_FACTOR;

    return (dist < threshold) ? 1 : 0;
}

/* Input polling thread — touch via left mouse button */
static volatile int g_prev_tap = 0;

static DWORD WINAPI input_thread(LPVOID param)
{
    Sleep(5000);  /* Wait for game to fully load */

    for (;;) {
        Sleep(16);  /* ~60fps */

        /* Check left mouse button (Winlator maps touch to left-click) */
        SHORT key_state = GetAsyncKeyState(VK_LBUTTON);
        int tap_down = (key_state & 0x8000) != 0;

        if (tap_down && !g_prev_tap) {
            /* Screen tapped! Check gates before allowing jump. */

            /* Gate 1: Countdown — Scene+0x3A4C must be 1 (countdown done) */
            DWORD ball = g_ball_ptr;
            if (!ball) ball = get_player_ball();
            if (!ball) goto next;

            if (IsBadReadPtr((void*)ball, 0xD00)) goto next;

            DWORD scene = *(DWORD*)(ball + 0x14);
            if (!scene || IsBadReadPtr((void*)scene, 0x4000)) goto next;

            BYTE countdown_done = *(BYTE*)(scene + 0x3A4C);
            if (!countdown_done) goto next;

            /* Gate 2: Race end — App+0x5D6 (player finished flag) */
            DWORD app = *(DWORD*)ADDR_App;
            if (app && !IsBadReadPtr((void*)app, 0x600)) {
                BYTE finished = *(BYTE*)(app + 0x5D6);
                if (finished) goto next;

                /* Gate 3: App+0x5D5 */
                BYTE flag2 = *(BYTE*)(app + 0x5D5);
                if (flag2) goto next;
            }

            /* Raycast: is the ball on the ground? */
            if (is_ball_grounded(ball)) {
                g_want_jump = 1;
            }
        }
        next:
        g_prev_tap = tap_down;
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Phase 15 impulse code cave — PURE ASM (no C function calls)
 *
 * Original 6 bytes: 8B 4C 24 1C 8B 11 = MOV ECX,[ESP+1C]; MOV EDX,[ECX]
 * ESI = ball pointer.
 *
 * If g_want_jump != 0: FADD impulse to ball+0x174 (Y force), clear flag.
 * Then: save ball pointer, run original bytes, JMP back.
 * ═══════════════════════════════════════════════════════════════════════════ */

static BYTE *g_phase15_cave = NULL;

static void install_phase15_hook(void)
{
    BYTE *hook_addr = (BYTE*)PHASE15_HOOK;

    BYTE expected[] = { 0x8B, 0x4C, 0x24, 0x1C, 0x8B, 0x11 };
    if (memcmp(hook_addr, expected, 6) != 0) return;

    g_phase15_cave = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE,
                                           PAGE_EXECUTE_READWRITE);
    if (!g_phase15_cave) return;

    int p = 0;

    /* CMP DWORD PTR [g_want_jump], 0 */
    g_phase15_cave[p++] = 0x83; g_phase15_cave[p++] = 0x3D;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_want_jump; p += 4;
    g_phase15_cave[p++] = 0x00;

    /* JZ to .no_jump */
    int jz_fixup = p;
    g_phase15_cave[p++] = 0x0F; g_phase15_cave[p++] = 0x84;
    *(DWORD*)(g_phase15_cave + p) = 0; p += 4;

    /* FLD [ESI+0x174] — load current Y force */
    g_phase15_cave[p++] = 0xD9; g_phase15_cave[p++] = 0x86;
    *(DWORD*)(g_phase15_cave + p) = 0x174; p += 4;

    /* FADD [g_jump_impulse] — add upward impulse */
    g_phase15_cave[p++] = 0xD8; g_phase15_cave[p++] = 0x05;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_jump_impulse; p += 4;

    /* FSTP [ESI+0x174] — store modified Y force */
    g_phase15_cave[p++] = 0xD9; g_phase15_cave[p++] = 0x9E;
    *(DWORD*)(g_phase15_cave + p) = 0x174; p += 4;

    /* MOV [g_want_jump], 0 — consumed */
    g_phase15_cave[p++] = 0xC7; g_phase15_cave[p++] = 0x05;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_want_jump; p += 4;
    *(DWORD*)(g_phase15_cave + p) = 0; p += 4;

    /* .no_jump: */
    int no_jump_target = p;

    /* MOV [g_ball_ptr], ESI — save ball pointer for input thread */
    g_phase15_cave[p++] = 0x89; g_phase15_cave[p++] = 0x35;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_ball_ptr; p += 4;

    /* Original 6 bytes */
    g_phase15_cave[p++] = 0x8B; g_phase15_cave[p++] = 0x4C; g_phase15_cave[p++] = 0x24; g_phase15_cave[p++] = 0x1C;
    g_phase15_cave[p++] = 0x8B; g_phase15_cave[p++] = 0x11;

    /* JMP back to hook_addr + 6 */
    g_phase15_cave[p++] = 0xE9;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)(hook_addr + PHASE15_ORIG_BYTES) - (DWORD)(g_phase15_cave + p + 4);
    p += 4;

    /* Fix up JZ */
    *(DWORD*)(g_phase15_cave + jz_fixup + 2) =
        (DWORD)(g_phase15_cave + no_jump_target) - (DWORD)(g_phase15_cave + jz_fixup + 6);

    /* Patch hook site: JMP to cave */
    DWORD old_protect;
    VirtualProtect(hook_addr, PHASE15_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);

    DWORD jmp_offset = (DWORD)(g_phase15_cave - hook_addr - 5);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_offset;
    hook_addr[5] = 0x90;

    VirtualProtect(hook_addr, PHASE15_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, PHASE15_ORIG_BYTES);
}

/* Patch thread: install hook then launch input thread */
static DWORD WINAPI patch_thread(LPVOID param)
{
    Sleep(5000);  /* Wait for game to load */
    install_phase15_hook();
    CreateThread(NULL, 0, input_thread, NULL, 0, NULL);
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hInst, DWORD reason, LPVOID lpReserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInst);
        load_real_bass();
        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
        break;
    }
    return TRUE;
}
