/*
 * jump_mod_raycast.c — BASS.dll proxy — v12 + RAYCASTING GROUND DETECTION
 *
 * Based on working v12 (single hook at 0x407BB4, no Present hook).
 * Adds raycast ground check in the input thread before allowing jump.
 *
 * How it works:
 *   1. Input thread polls spacebar every 16ms
 *   2. On rising-edge keypress: run raycast (Mesh_FindClosestCollision)
 *      straight down from ball position. If hit is within radius+epsilon,
 *      ball is grounded → set g_want_jump=1 (allow jump).
 *      If airborne → g_want_jump stays 0 (deny jump).
 *   3. Phase 15 cave (0x407BB4): if g_want_jump!=0, FADD impulse to
 *      ball+0x174, clear g_want_jump=0. Same as v12.
 *
 * The raycast runs in the input thread (normal C context, safe to call
 * C functions). The Phase 15 cave stays pure asm (no C calls), exactly
 * like v12. No Present hook needed.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll jump_mod_raycast.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <math.h>

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
 * Jump Mod — v12 + Raycast
 * ═══════════════════════════════════════════════════════════════════════════ */

/* ─── Hook addresses ──────────────────────────────────────────────────────── */
#define PHASE15_HOOK       0x00407BB4   /* Ball_Update Phase 15 point             */
#define PHASE15_ORIG_BYTES 6            /* 8B 4C 24 1C 8B 11                      */

/* ─── Game function addresses ─────────────────────────────────────────────── */
#define ADDR_Mesh_FindClosestCollision  0x00465D90
#define ADDR_App                        0x005341E0

/* ─── Parameters ──────────────────────────────────────────────────────────── */
static float g_jump_impulse = 20.0f;
/* Slope-aware threshold: on a slope of angle θ, a straight-down raycast
 * hits at distance r/cos(θ) from ball center (not r). Using radius*1.4
 * covers slopes up to ~44° (cos(44°)≈0.719, r/0.719≈1.39r). */
static const float GROUND_SLOPE_FACTOR = 1.4f;

/* ─── Shared state ────────────────────────────────────────────────────────── */
/* g_want_jump: 0=idle, 1=spacebar pressed AND grounded (ready to jump) */
static volatile DWORD g_want_jump = 0;
static volatile DWORD g_frame_count = 0;
static volatile DWORD g_jump_count = 0;
static volatile DWORD g_raycast_count = 0;
static volatile DWORD g_grounded_count = 0;
/* Ball pointer captured from Phase 15 cave (ESI=ball). Set every frame. */
static volatile DWORD g_ball_ptr = 0;

/* ─── Mesh_FindClosestCollision wrapper ────────────────────────────────────
 *
 * __thiscall: ECX = mesh_data (this pointer)
 * Stack params (callee-clean RET 0x20, 8 DWORDs = 0x20 bytes):
 *   [ESP+0x00] out_hit_vec3_ptr  (pushed last)
 *   [ESP+0x04] origin_x
 *   [ESP+0x08] origin_y
 *   [ESP+0x0C] origin_z
 *   [ESP+0x10] dir_x
 *   [ESP+0x14] dir_y
 *   [ESP+0x18] dir_z
 *   [ESP+0x1C] radius_scale
 * Returns: out_hit pointer (always non-NULL if function doesn't crash)
 *
 * Verified via GhidraMCP: the game's own call at 0x4064b9-0x40651f pushes
 * args in this exact order. ECX = scene+0x8B0 (mesh_data). Callee does
 * RET 0x20 (callee-clean, pops 32 bytes).
 *
 * Uses __attribute__((thiscall)) function pointer — MinGW 13-win32 generates
 * correct callee-clean code (sets ECX, pushes args, no caller cleanup).
 * Verified by cross-compiling and disassembling the output.
 *
 * DO NOT use inline asm for this call! Previous versions used inline asm
 * with manual `sub esp, 0x20` + `mov [esp+N]`, which corrupted GCC's stack
 * frame tracking — when GCC inlines do_raycast into is_ball_grounded at -O2,
 * the manual ESP manipulation causes all subsequent local variable reads
 * (ball_x, ball_y, radius, hit_result) to be at wrong stack offsets,
 * producing garbage 0x80000000 values for everything.
 */
typedef float* (__attribute__((thiscall)) *raycast_fn_t)(
    void *mesh_data,   /* this → ECX */
    float *out_hit,    /* [ESP+0x00] */
    float ox,           /* [ESP+0x04] */
    float oy,           /* [ESP+0x08] */
    float oz,           /* [ESP+0x0C] */
    float dx,           /* [ESP+0x10] */
    float dy,           /* [ESP+0x14] */
    float dz,           /* [ESP+0x18] */
    float radius        /* [ESP+0x1C] */
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

/* ─── Get player ball pointer ─────────────────────────────────────────────── */
static DWORD get_player_ball(void)
{
    DWORD app = *(DWORD*)ADDR_App;
    if (!app) return 0;

    /* Scene pointer is at App+0x178 */
    DWORD scene = *(DWORD*)(app + 0x178);
    if (!scene) return 0;

    /* AthenaList at Scene+0x29D4: count at +0x04, data at +0x40C */
    DWORD list_count = *(DWORD*)(scene + 0x29D4 + 0x04);
    if (list_count == 0) return 0;

    DWORD list_data = *(DWORD*)(scene + 0x29D4 + 0x40C);
    if (!list_data) return 0;

    DWORD ball = *(DWORD*)(list_data);
    return ball;
}

/* ─── Raycast ground check ─────────────────────────────────────────────────
 *
 * Returns 1 if ball is on the ground, 0 if airborne.
 * Uses the game's own Mesh_FindClosestCollision with direction (0, -1, 0).
 */
static int is_ball_grounded(DWORD ball)
{
    if (!ball) return 0;

    DWORD scene = *(DWORD*)(ball + 0x14);
    if (!scene) {
        diag_log("DBG: ball+0x14 (scene) = NULL");
        return 0;
    }

    DWORD mesh_data = *(DWORD*)(scene + 0x8B0);
    if (!mesh_data) {
        diag_log("DBG: scene+0x8B0 (mesh_data) = NULL");
        return 0;
    }

    float ball_x = *(float*)(ball + 0x164);
    float ball_y = *(float*)(ball + 0x168);
    float ball_z = *(float*)(ball + 0x16C);
    float radius = *(float*)(ball + 0x284);

    float hit_result[3] = {0.0f, 0.0f, 0.0f};

    float *ret = do_raycast((void*)mesh_data,
                             ball_x, ball_y, ball_z,
                             0.0f, -1.0f, 0.0f,
                             1.0f,
                             hit_result);

    g_raycast_count++;

    /* Diagnostic dump */
    {
        char buf[512];
        int bx = (int)(ball_x * 100.0f);
        int by = (int)(ball_y * 100.0f);
        int bz = (int)(ball_z * 100.0f);
        int hy = (int)(hit_result[1] * 100.0f);
        int rad = (int)(radius * 100.0f);
        sprintf(buf, "DBG: ball=%08X mesh=%08X ret=%08X pos=(%d,%d,%d) hit_y=%d radius=%d",
                ball, mesh_data, (DWORD)ret, bx, by, bz, hy, rad);
        diag_log(buf);
    }

    float hit_y = hit_result[1];
    float dist = fabsf(hit_y - ball_y);
    float threshold = radius * GROUND_SLOPE_FACTOR;

    if (dist < threshold) {
        g_grounded_count++;
        return 1;
    }

    return 0;
}

/* ─── Input polling thread ─────────────────────────────────────────────────── */
static volatile int g_prev_space = 0;

static DWORD WINAPI input_thread(LPVOID param)
{
    (void)param;
    char buf[256];

    while (1) {
        Sleep(16);

        DWORD app = *(DWORD*)ADDR_App;
        if (!app) continue;

        DWORD ih = *(DWORD*)(app + 0x180);
        if (!ih) continue;

        DWORD kb = *(DWORD*)(ih + 0x434);
        if (!kb) continue;

        /* KeyboardDevice+0x45 = key buffer (at +0xC) + DIK_SPACE (0x39) */
        BYTE key_state = *(BYTE*)(kb + 0x45);
        int space_down = (key_state & 0x80) != 0;

        if (space_down && !g_prev_space) {
            /* Spacebar just pressed — check if grounded via raycast */
            DWORD ball = g_ball_ptr;
            if (ball) {
                int grounded = is_ball_grounded(ball);
                if (grounded) {
                    g_want_jump = 1;  /* grounded — allow jump */
                    wsprintfA(buf, "JUMP: grounded! raycasts=%u grounded=%u want=%u",
                             g_raycast_count, g_grounded_count, g_want_jump);
                    diag_log(buf);
                } else {
                    wsprintfA(buf, "DENY: airborne. raycasts=%u grounded=%u",
                             g_raycast_count, g_grounded_count);
                    diag_log(buf);
                }
            } else {
                /* Ball pointer not yet captured — allow jump as fallback (v12 behavior) */
                g_want_jump = 1;
                diag_log("JUMP: ball_ptr not set, fallback allow");
            }
        }
        g_prev_space = space_down;
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook: Phase 15 impulse (0x407BB4) — mid-function cave
 *
 * Original 6 bytes: 8B 4C 24 1C 8B 11  =  MOV ECX,[ESP+1C]; MOV EDX,[ECX]
 * ESI = ball pointer.
 *
 * If g_want_jump != 0 (spacebar pressed AND grounded): add impulse.
 * This is EXACTLY the same cave as v12 — no changes to the impulse mechanism.
 * ═══════════════════════════════════════════════════════════════════════════ */
static BYTE *g_phase15_cave = NULL;

static void install_phase15_hook(void)
{
    BYTE *hook_addr = (BYTE*)PHASE15_HOOK;
    char buf[256];

    BYTE expected[] = { 0x8B, 0x4C, 0x24, 0x1C, 0x8B, 0x11 };
    wsprintfA(buf, "Phase15 bytes: %02X %02X %02X %02X %02X %02X",
              hook_addr[0], hook_addr[1], hook_addr[2],
              hook_addr[3], hook_addr[4], hook_addr[5]);
    diag_log(buf);

    if (memcmp(hook_addr, expected, 6) != 0) {
        diag_log("PHASE15 BYTE MISMATCH!");
        return;
    }

    g_phase15_cave = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE,
                                           PAGE_EXECUTE_READWRITE);
    if (!g_phase15_cave) { diag_log("phase15: VirtualAlloc FAILED"); return; }

    int p = 0;

    /* ─── Check g_want_jump != 0 ─── */
    /* CMP DWORD PTR [g_want_jump], 0 */
    g_phase15_cave[p++] = 0x83; g_phase15_cave[p++] = 0x3D;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_want_jump; p += 4;
    g_phase15_cave[p++] = 0x00;

    /* JNZ to .no_jump (skip impulse if g_want_jump == 0) */
    /* Actually: JZ to .no_jump — if g_want_jump==0, skip the impulse */
    int jz_fixup = p;
    g_phase15_cave[p++] = 0x0F; g_phase15_cave[p++] = 0x84;
    *(DWORD*)(g_phase15_cave + p) = 0; p += 4;

    /* ─── .jump: add impulse to ball+0x174 (Y force accumulator) ─── */
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

    /* INC [g_jump_count] */
    g_phase15_cave[p++] = 0xFF; g_phase15_cave[p++] = 0x05;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_jump_count; p += 4;

    /* ─── .no_jump: ─── */
    int no_jump_target = p;

    /* Save ball pointer (ESI) for input thread to use for raycast */
    /* MOV [g_ball_ptr], ESI */
    g_phase15_cave[p++] = 0x89; g_phase15_cave[p++] = 0x35;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_ball_ptr; p += 4;

    /* INC [g_frame_count] */
    g_phase15_cave[p++] = 0xFF; g_phase15_cave[p++] = 0x05;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_frame_count; p += 4;

    /* ─── Original 6 bytes ─── */
    g_phase15_cave[p++] = 0x8B; g_phase15_cave[p++] = 0x4C; g_phase15_cave[p++] = 0x24; g_phase15_cave[p++] = 0x1C;
    g_phase15_cave[p++] = 0x8B; g_phase15_cave[p++] = 0x11;

    /* JMP back to hook_addr + 6 */
    g_phase15_cave[p++] = 0xE9;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)(hook_addr + PHASE15_ORIG_BYTES) - (DWORD)(g_phase15_cave + p + 4);
    p += 4;

    /* ─── Fix up JZ ─── */
    *(DWORD*)(g_phase15_cave + jz_fixup + 2) =
        (DWORD)(g_phase15_cave + no_jump_target) - (DWORD)(g_phase15_cave + jz_fixup + 6);

    /* ─── Patch hook site ─── */
    DWORD old_protect;
    VirtualProtect(hook_addr, PHASE15_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);

    DWORD jmp_offset = (DWORD)(g_phase15_cave - hook_addr - 5);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_offset;
    hook_addr[5] = 0x90;

    VirtualProtect(hook_addr, PHASE15_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, PHASE15_ORIG_BYTES);

    wsprintfA(buf, "HOOK v12+RAYCAST INSTALLED! impulse=%f cave=%08X", g_jump_impulse, (DWORD)g_phase15_cave);
    diag_log(buf);
}

/* ─── Patch thread ───────────────────────────────────────────────────────── */

static DWORD WINAPI patch_thread(LPVOID param)
{
    (void)param;
    char buf[256];

    diag_log("patch_thread: started");
    Sleep(5000);

    install_phase15_hook();

    CreateThread(NULL, 0, input_thread, NULL, 0, NULL);
    diag_log("input_thread launched");

    Sleep(8000);
    wsprintfA(buf, "After 8s: frames=%u jumps=%u raycasts=%u grounded=%u want=%u",
              g_frame_count, g_jump_count, g_raycast_count, g_grounded_count, g_want_jump);
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

        {
            char mod_path[MAX_PATH];
            if (GetModuleFileNameA(hInst, mod_path, MAX_PATH)) {
                char *p = strrchr(mod_path, '\\');
                if (p) {
                    strcpy(p + 1, "hamsterball_jump_debug.txt");
                    strncpy(g_logpath1, mod_path, MAX_PATH-1);
                }
            }
        }

        diag_log("=== jump_mod v20+RAYCAST (slope-aware threshold) loaded ===");

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
