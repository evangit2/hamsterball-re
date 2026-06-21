/*
 * jump_mod.c — BASS.dll proxy — v17 RAYCASTING GROUND DETECTION
 *
 * NO FLAGS. NO COOLDOWNS. JUST RAYCASTING.
 *
 * How it works:
 *   1. Input thread polls spacebar, sets g_want_jump=1
 *   2. PresentOrEnd hook (function-entry, safe to call C) does raycasting:
 *      - Cast ray from ball position straight down (0, -1, 0)
 *      - Call Mesh_FindClosestCollision (0x465D90) — same function the game uses
 *      - Check if hit point Y is within (ball.radius + epsilon) of ball Y
 *      - If grounded AND g_want_jump==1: set g_want_jump=2 (signal to apply)
 *      - If NOT grounded: clear g_want_jump (no jump for you)
 *   3. Phase 15 cave (0x407BB4): if g_want_jump==2, add impulse, clear flag
 *
 * The raycast uses the game's own collision system, so it's exactly as
 * accurate as the game's physics. No approximations, no sticky flags,
 * no cooldowns. On ground = can jump. Off ground = can't.
 *
 * Mesh_FindClosestCollision signature (verified from disasm at 0x465D90):
 *   __thiscall, ECX = scene mesh data ptr (from Scene+0x8B0)
 *   Stack: origin(3 floats), direction(3 floats), 1.0f(radius), out_vec3_ptr
 *   RET 0x20 (8 DWORDs callee-clean)
 *   Returns: pointer to hit vec3
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
 * Jump Mod v17 — RAYCASTING ground detection
 * ═══════════════════════════════════════════════════════════════════════════ */

/* ─── Hook addresses ──────────────────────────────────────────────────────── */
#define PRESENT_HOOK       0x00455A90   /* Graphics_PresentOrEnd — function entry */
#define PRESENT_ORIG_BYTES 6            /* first 6 bytes: 83 EC 0C 53 55 56      */
#define PHASE15_HOOK       0x00407BB4   /* Ball_Update Phase 15 point             */
#define PHASE15_ORIG_BYTES 6            /* 8B 4C 24 1C 8B 11                      */

/* ─── Game function addresses ─────────────────────────────────────────────── */
#define ADDR_Mesh_FindClosestCollision  0x00465D90
#define ADDR_App                        0x005341E0

/* ─── Parameters ──────────────────────────────────────────────────────────── */
static float g_jump_impulse = 20.0f;
static const float GROUND_RAY_EPSILON = 5.0f;  /* extra tolerance beyond radius */

/* ─── Shared state ────────────────────────────────────────────────────────── */
/* g_want_jump: 0=idle, 1=spacebar pressed (pending raycast), 2=raycast confirmed grounded */
static volatile DWORD g_want_jump = 0;
static volatile DWORD g_frame_count = 0;
static volatile DWORD g_jump_count = 0;
static volatile DWORD g_raycast_count = 0;
static volatile DWORD g_grounded_frames = 0;

/* ─── Mesh_FindClosestCollision typedef ─────────────────────────────────────
 *
 * __thiscall: ECX = scene mesh data pointer
 * Stack params (8 DWORDs, callee-clean RET 0x20):
 *   origin_x, origin_y, origin_z   (3 floats)
 *   dir_x, dir_y, dir_z            (3 floats)
 *   radius_scale                    (1 float = 1.0)
 *   out_hit_vec3_ptr                (1 pointer)
 * Returns: pointer to hit vec3 (3 floats: x, y, z)
 *
 * For MinGW, __thiscall is not directly supported. We use a __cdecl wrapper
 * that manually sets ECX and calls via inline asm. The function takes 8 stack
 * params + ECX, returns a float*, and cleans 0x20 bytes from the stack.
 */
typedef float* (__cdecl *Mesh_FindClosestCollision_cdecl_t)(
    void *ecx_val,
    float origin_x, float origin_y, float origin_z,
    float dir_x, float dir_y, float dir_z,
    float radius_scale,
    float *out_hit
);

/* We need to call the thiscall function. Since MinGW doesn't support __thiscall,
 * we write a small wrapper in inline asm.
 */
static float* do_raycast(void *mesh_data,
                          float ox, float oy, float oz,
                          float dx, float dy, float dz,
                          float radius_scale,
                          float *out_hit)
{
    /* The function is __thiscall with RET 0x20.
     * We push 8 DWORDs on the stack, set ECX, call, and the callee
     * will clean up the stack (RET 0x20).
     */
    float *result = NULL;
    __asm__ __volatile__ (
        "subl $0xc, %%esp\n\t"         /* make room for origin on stack */
        "fstps (%%esp)\n\t"            /* origin_z */
        "fstps 4(%%esp)\n\t"           /* origin_y */
        "fstps 8(%%esp)\n\t"           /* origin_x */
        "subl $0xc, %%esp\n\t"         /* make room for direction on stack */
        "fstps (%%esp)\n\t"            /* dir_z */
        "fstps 4(%%esp)\n\t"           /* dir_y */
        "fstps 8(%%esp)\n\t"           /* dir_x */
        "subl $0x8, %%esp\n\t"         /* room for radius_scale + out_hit */
        "fstps (%%esp)\n\t"            /* radius_scale (float) */
        "movl %[out], 4(%%esp)\n\t"    /* out_hit pointer */
        "movl %[mesh], %%ecx\n\t"      /* ECX = this */
        "call *%[fn]\n\t"              /* call Mesh_FindClosestCollision */
        /* callee did RET 0x20, so stack is already cleaned */
        "movl %%eax, %[res]\n\t"
        : [res] "=m" (result)
        : [mesh] "r" (mesh_data),
          [fn] "r" ((void*)(DWORD)ADDR_Mesh_FindClosestCollision),
          [out] "r" (out_hit)
        : "eax", "ecx", "edx", "memory", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
    );
    return result;
}

/* Actually, the inline asm approach is fragile with FPU regs.
 * Let's use a simpler approach: write a .asm stub file or just
 * use the __thiscall function pointer trick that works in MinGW.
 *
 * The trick: define a struct with a function pointer, and use
 * the "thiscall through member" pattern. But that still doesn't work
 * in MinGW because it doesn't support __thiscall.
 *
 * Simplest approach: write the wrapper as a naked function in a .S file.
 * But for a single .c file build, let's use a different approach:
 *
 * We'll write the call wrapper as raw machine code in an executable buffer.
 */

/* Actually, the SIMPLEST approach that definitely works: use the fact that
 * __thiscall is just __fastcall with this in ECX. For MinGW, we can use
 * __attribute__((fastcall)) which puts first 2 params in ECX and EDX.
 * So we pass (mesh_data, dummy) as first two fastcall params — mesh_data
 * goes in ECX (which is what we want), dummy goes in EDX (ignored).
 * Then the remaining 8 params go on the stack.
 *
 * BUT the callee does RET 0x20 (callee cleanup of 8 DWORDs). With fastcall,
 * GCC would expect the callee to clean those params, which it does via RET 0x20.
 * The issue is that fastcall by default uses RET (no stack cleanup) unless
 * we specify __attribute__((stdcall)) too.
 *
 * __attribute__((fastcall, stdcall)) would put first 2 in ECX/EDX and
 * callee-clean the remaining stack params. But GCC might complain about
 * combining these attributes.
 *
 * Most reliable: just do it all in inline asm with a C wrapper.
 */

/* ─── Get player ball pointer ─────────────────────────────────────────────── */
static DWORD get_player_ball(void)
{
    DWORD app = *(DWORD*)ADDR_App;
    if (!app) return 0;

    DWORD scene = *(DWORD*)(app + 0x14);
    if (!scene) return 0;

    /* AthenaList at Scene+0x29D4: count at +0x04, data at +0x40C */
    DWORD list_count = *(DWORD*)(scene + 0x29D4 + 0x04);
    if (list_count == 0) return 0;

    DWORD list_data = *(DWORD*)(scene + 0x29D4 + 0x40C);
    if (!list_data) return 0;

    /* Get first ball — for player 0 this is typically index 0 */
    DWORD ball = *(DWORD*)(list_data);
    return ball;
}

/* ─── Raycast ground check (called from Present hook in C context) ────────
 *
 * Returns 1 if ball is on the ground, 0 if airborne.
 * Uses the game's own Mesh_FindClosestCollision with direction (0, -1, 0).
 * This is exactly what Ball_Update does at 0x4064C2-0x406543.
 */
static int is_ball_grounded(DWORD ball)
{
    if (!ball) return 0;

    DWORD app = *(DWORD*)ADDR_App;
    if (!app) return 0;

    DWORD scene = *(DWORD*)(app + 0x14);
    if (!scene) return 0;

    /* Scene mesh data pointer at Scene+0x8B0 */
    DWORD mesh_data = *(DWORD*)(scene + 0x8B0);
    if (!mesh_data) return 0;

    /* Ball position */
    float ball_x = *(float*)(ball + 0x164);
    float ball_y = *(float*)(ball + 0x168);
    float ball_z = *(float*)(ball + 0x16C);

    /* Ball radius */
    float radius = *(float*)(ball + 0x284);

    /* Raycast: origin = ball position, direction = straight down (0, -1, 0) */
    float hit_result[3] = {0.0f, 0.0f, 0.0f};

    /* Call Mesh_FindClosestCollision via inline asm wrapper.
     *
     * The function is __thiscall:
     *   ECX = mesh_data (this)
     *   Stack (pushed right-to-left):
     *     origin_z, origin_y, origin_x   (3 floats — note: game pushes as
     *                                      SUB ESP,0xC then FSTP to each slot)
     *     dir_z, dir_y, dir_x            (3 floats)
     *     1.0f                            (radius scale)
     *     out_hit_ptr                     (output vec3)
     *   RET 0x20
     *
     * Looking at how Ball_Update calls it at 0x4064C2:
     *   PUSH 0x3f800000 (1.0f)
     *   SUB ESP,0xC  → direction = {0, -1.0, 0}
     *   SUB ESP,0xC  → origin = ball position
     *   LEA EAX,[ESP+0x78]  → output pointer
     *   PUSH EAX
     *   CALL 0x00465d90
     *
     * So the stack layout (ESP after all pushes, before CALL):
     *   ESP+0x00: origin_x  (ball+0x164)
     *   ESP+0x04: origin_y  (ball+0x168)
     *   ESP+0x08: origin_z  (ball+0x16C)
     *   ESP+0x0C: dir_x     (0.0)
     *   ESP+0x10: dir_y     (-1.0)
     *   ESP+0x14: dir_z     (0.0)
     *   ESP+0x18: 1.0f      (radius scale)
     *   ESP+0x1C: out_ptr
     * Total = 8 DWORDs = 0x20, matching RET 0x20
     */

    /* Use a raw machine-code trampoline to call this thiscall function.
     * We allocate a small code buffer that does:
     *   mov ecx, <mesh_data>
     *   push <out_ptr>
     *   push 1.0f
     *   push 0.0f   ; dir_z
     *   push -1.0f  ; dir_y
     *   push 0.0f   ; dir_x
     *   push ball_z
     *   push ball_y
     *   push ball_x
     *   call <fn_addr>
     *   ret
     */
    static BYTE *trampoline = NULL;
    if (!trampoline) {
        trampoline = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE,
                                          PAGE_EXECUTE_READWRITE);
        if (!trampoline) return 0;
    }

    /* Build the trampoline each time (params change each frame) */
    int tp = 0;

    /* MOV ECX, mesh_data */
    trampoline[tp++] = 0xB9;
    *(DWORD*)(trampoline + tp) = mesh_data; tp += 4;

    /* PUSH out_hit ptr (push the address of hit_result[0]) */
    trampoline[tp++] = 0x68;
    *(DWORD*)(trampoline + tp) = (DWORD)hit_result; tp += 4;

    /* PUSH 1.0f (0x3F800000) */
    trampoline[tp++] = 0x68;
    *(DWORD*)(trampoline + tp) = 0x3F800000; tp += 4;

    /* PUSH 0.0f (dir_z) */
    trampoline[tp++] = 0x68;
    *(DWORD*)(trampoline + tp) = 0x00000000; tp += 4;

    /* PUSH -1.0f (dir_y = 0xBF800000) */
    trampoline[tp++] = 0x68;
    *(DWORD*)(trampoline + tp) = 0xBF800000; tp += 4;

    /* PUSH 0.0f (dir_x) */
    trampoline[tp++] = 0x68;
    *(DWORD*)(trampoline + tp) = 0x00000000; tp += 4;

    /* PUSH ball_z */
    trampoline[tp++] = 0x68;
    *(DWORD*)(trampoline + tp) = *(DWORD*)&ball_z; tp += 4;

    /* PUSH ball_y */
    trampoline[tp++] = 0x68;
    *(DWORD*)(trampoline + tp) = *(DWORD*)&ball_y; tp += 4;

    /* PUSH ball_x */
    trampoline[tp++] = 0x68;
    *(DWORD*)(trampoline + tp) = *(DWORD*)&ball_x; tp += 4;

    /* CALL Mesh_FindClosestCollision */
    trampoline[tp++] = 0xE8;
    DWORD call_target = (DWORD)ADDR_Mesh_FindClosestCollision - (DWORD)(trampoline + tp + 4);
    *(DWORD*)(trampoline + tp) = call_target; tp += 4;

    /* RET */
    trampoline[tp++] = 0xC3;

    /* Call the trampoline */
    typedef float* (__cdecl *trampoline_fn_t)(void);
    trampoline_fn_t fn = (trampoline_fn_t)trampoline;
    float *result = fn();

    g_raycast_count++;

    if (!result) return 0;

    /* Check: abs(hit_y - ball_y) < radius + epsilon
     * This is the same check Ball_Update uses at 0x406524-0x406543 */
    float dy = hit_result[1] - ball_y;
    if (dy < 0) dy = -dy;  /* fabs */

    float threshold = radius + GROUND_RAY_EPSILON;

    return (dy < threshold) ? 1 : 0;
}

/* ─── Present hook C helper ─────────────────────────────────────────────────
 *
 * Called from the Present hook cave (function-entry, safe for C calls).
 * Does the raycast check and updates g_want_jump accordingly.
 */
static void __cdecl present_raycast_check(void)
{
    /* Only act if spacebar was pressed */
    if (g_want_jump != 1) return;

    DWORD ball = get_player_ball();
    if (!ball) {
        g_want_jump = 0;  /* no ball, no jump */
        return;
    }

    /* Check if ball is in active play */
    BYTE active = *(BYTE*)(ball + 0x769);
    if (!active) {
        g_want_jump = 0;
        return;
    }

    /* Check not in tube */
    BYTE in_tube = *(BYTE*)(ball + 0x324);
    if (in_tube) {
        g_want_jump = 0;
        return;
    }

    /* Check not in fall/respawn mode */
    DWORD fall_mode = *(DWORD*)(ball + 0xC4C);
    if (fall_mode != 0) {
        g_want_jump = 0;
        return;
    }

    /* RAYCAST: is the ball on the ground? */
    int grounded = is_ball_grounded(ball);

    if (grounded) {
        g_grounded_frames++;
        /* Ball IS on ground — confirm the jump */
        g_want_jump = 2;
    } else {
        /* Ball is AIRBORNE — deny the jump */
        g_want_jump = 0;
    }
}

/* Function pointer used by the Present hook cave to call the C helper */
static void (__cdecl *g_present_check_fn_ptr)(void) = NULL;

/* ─── Input polling thread ───────────────────────────────────────────────── */
static volatile int g_prev_space = 0;

static DWORD WINAPI input_thread(LPVOID param)
{
    (void)param;

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
            /* Spacebar just pressed — request jump (pending raycast) */
            if (g_want_jump == 0) {
                g_want_jump = 1;
            }
        }
        g_prev_space = space_down;
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook 1: Graphics_PresentOrEnd (0x455A90) — function-entry hook
 *
 * This is safe for calling C functions because no FPU/stack frame is live
 * at function entry. We do the raycast here and signal Phase 15.
 *
 * Original bytes: 83 EC 0C 53 55 56  =  SUB ESP,0xC; PUSH EBX; PUSH EBP; PUSH ESI
 * ═══════════════════════════════════════════════════════════════════════════ */
static BYTE *g_present_cave = NULL;

static void install_present_hook(void)
{
    BYTE *hook_addr = (BYTE*)PRESENT_HOOK;
    char buf[256];

    BYTE expected[] = { 0x83, 0xEC, 0x0C, 0x53, 0x55, 0x56 };
    wsprintfA(buf, "Present bytes: %02X %02X %02X %02X %02X %02X",
              hook_addr[0], hook_addr[1], hook_addr[2],
              hook_addr[3], hook_addr[4], hook_addr[5]);
    diag_log(buf);

    if (memcmp(hook_addr, expected, 6) != 0) {
        diag_log("PRESENT BYTE MISMATCH!");
        return;
    }

    g_present_cave = (BYTE*)VirtualAlloc(NULL, 512, MEM_COMMIT | MEM_RESERVE,
                                           PAGE_EXECUTE_READWRITE);
    if (!g_present_cave) { diag_log("present: VirtualAlloc FAILED"); return; }

    int p = 0;

    /* ─── Save registers ─── */
    /* PUSHAD (60) — saves all general regs */
    g_present_cave[p++] = 0x60;
    /* PUSHFD (9C) — saves flags */
    g_present_cave[p++] = 0x9C;

    /* ─── Check g_want_jump == 1 (spacebar was pressed) ─── */
    /* CMP [g_want_jump], 1 */
    g_present_cave[p++] = 0x83; g_present_cave[p++] = 0x3D;
    *(DWORD*)(g_present_cave + p) = (DWORD)&g_want_jump; p += 4;
    g_present_cave[p++] = 0x01;

    /* JNZ to .skip_raycast */
    int jnz_want_fixup = p;
    g_present_cave[p++] = 0x0F; g_present_cave[p++] = 0x85;
    *(DWORD*)(g_present_cave + p) = 0; p += 4;

    /* ─── Call present_raycast_check() via function pointer ─── */
    /* CALL [g_present_check_fn_ptr] */
    g_present_cave[p++] = 0xFF; g_present_cave[p++] = 0x15;
    *(DWORD*)(g_present_cave + p) = (DWORD)&g_present_check_fn_ptr; p += 4;

    /* ─── .skip_raycast: ─── */
    int skip_raycast_target = p;

    /* INC [g_frame_count] */
    g_present_cave[p++] = 0xFF; g_present_cave[p++] = 0x05;
    *(DWORD*)(g_present_cave + p) = (DWORD)&g_frame_count; p += 4;

    /* ─── Restore and execute original bytes ─── */
    /* POPFD (9D) */
    g_present_cave[p++] = 0x9D;
    /* POPAD (61) */
    g_present_cave[p++] = 0x61;

    /* Original 6 bytes */
    g_present_cave[p++] = 0x83; g_present_cave[p++] = 0xEC;
    g_present_cave[p++] = 0x0C; g_present_cave[p++] = 0x53;
    g_present_cave[p++] = 0x55; g_present_cave[p++] = 0x56;

    /* JMP back to hook_addr + 6 */
    g_present_cave[p++] = 0xE9;
    *(DWORD*)(g_present_cave + p) = (DWORD)(hook_addr + PRESENT_ORIG_BYTES) - (DWORD)(g_present_cave + p + 4);
    p += 4;

    /* ─── Fix up JNZ ─── */
    *(DWORD*)(g_present_cave + jnz_want_fixup + 2) =
        (DWORD)(g_present_cave + skip_raycast_target) - (DWORD)(g_present_cave + jnz_want_fixup + 6);

    /* ─── Patch hook site ─── */
    DWORD old_protect;
    VirtualProtect(hook_addr, PRESENT_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);

    DWORD jmp_offset = (DWORD)(g_present_cave - hook_addr - 5);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_offset;
    hook_addr[5] = 0x90;

    VirtualProtect(hook_addr, PRESENT_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, PRESENT_ORIG_BYTES);

    wsprintfA(buf, "PRESENT HOOK installed: %d bytes, cave=%08X", p, (DWORD)g_present_cave);
    diag_log(buf);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook 2: Phase 15 impulse (0x407BB4) — mid-function cave
 *
 * Original 6 bytes: 8B 4C 24 1C 8B 11  =  MOV ECX,[ESP+1C]; MOV EDX,[ECX]
 * ESI = ball pointer.
 *
 * If g_want_jump == 2 (raycast confirmed grounded): add impulse.
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

    /* ─── Check g_want_jump == 2 (raycast confirmed) ─── */
    /* CMP [g_want_jump], 2 */
    g_phase15_cave[p++] = 0x83; g_phase15_cave[p++] = 0x3D;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_want_jump; p += 4;
    g_phase15_cave[p++] = 0x02;

    /* JNZ to .no_jump */
    int jnz_confirm_fixup = p;
    g_phase15_cave[p++] = 0x0F; g_phase15_cave[p++] = 0x85;
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

    /* ─── Original 6 bytes ─── */
    g_phase15_cave[p++] = 0x8B; g_phase15_cave[p++] = 0x4C; g_phase15_cave[p++] = 0x24; g_phase15_cave[p++] = 0x1C;
    g_phase15_cave[p++] = 0x8B; g_phase15_cave[p++] = 0x11;

    /* JMP back to hook_addr + 6 */
    g_phase15_cave[p++] = 0xE9;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)(hook_addr + PHASE15_ORIG_BYTES) - (DWORD)(g_phase15_cave + p + 4);
    p += 4;

    /* ─── Fix up JNZ ─── */
    *(DWORD*)(g_phase15_cave + jnz_confirm_fixup + 2) =
        (DWORD)(g_phase15_cave + no_jump_target) - (DWORD)(g_phase15_cave + jnz_confirm_fixup + 6);

    /* ─── Patch hook site ─── */
    DWORD old_protect;
    VirtualProtect(hook_addr, PHASE15_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);

    DWORD jmp_offset = (DWORD)(g_phase15_cave - hook_addr - 5);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_offset;
    hook_addr[5] = 0x90;

    VirtualProtect(hook_addr, PHASE15_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, PHASE15_ORIG_BYTES);

    wsprintfA(buf, "PHASE15 HOOK installed: %d bytes, cave=%08X", p, (DWORD)g_phase15_cave);
    diag_log(buf);
}

/* ─── Patch thread ───────────────────────────────────────────────────────── */

static DWORD WINAPI patch_thread(LPVOID param)
{
    (void)param;
    char buf[256];

    diag_log("patch_thread: started");
    Sleep(5000);

    /* Initialize the C helper function pointer for the Present hook */
    g_present_check_fn_ptr = present_raycast_check;
    wsprintfA(buf, "present_check fn = %08X", (DWORD)present_raycast_check);
    diag_log(buf);

    install_present_hook();
    install_phase15_hook();

    CreateThread(NULL, 0, input_thread, NULL, 0, NULL);
    diag_log("input_thread launched");

    Sleep(8000);
    wsprintfA(buf, "After 8s: frames=%u jumps=%u raycasts=%u grounded=%u want=%u",
              g_frame_count, g_jump_count, g_raycast_count, g_grounded_frames, g_want_jump);
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

        diag_log("=== jump_mod v17 RAYCASTING loaded ===");

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
