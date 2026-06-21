/*
 * jump_mod.c — BASS.dll proxy — v14b RAYCAST via game thread
 *
 * v14 called Ball_FindMeshCollision from the input thread — CRASHED
 * because game functions that create/destroy internal objects (AthenaList,
 * CollisionMesh, SpatialTree) are NOT thread-safe.
 *
 * v14b uses TWO hooks:
 *   1. Ball_Update entry (0x405E00) — GAME THREAD, safe to call C
 *      Checks g_want_jump, does raycast, sets g_jump_approved
 *   2. Phase 15 cave (0x407BB4) — adds impulse if g_jump_approved
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
 * Jump Mod v14b — raycast on game thread, impulse on game thread
 * ═══════════════════════════════════════════════════════════════════════════ */

/* ─── Hook addresses ─────────────────────────────────────────────────────── */
#define BALL_UPDATE_ENTRY    0x00405E00
#define BALL_UPDATE_RELOC    8    /* bytes to relocate (push -1 + mov eax,fs:[0]) */
#define PHASE15_HOOK         0x00407BB4
#define PHASE15_RELOC        6    /* bytes to relocate (MOV ECX,[ESP+1C]; MOV EDX,[ECX]) */

/* ─── Jump parameters ────────────────────────────────────────────────────── */
static float g_jump_impulse = 20.0f;
#define GROUND_TOLERANCE     2.0f

/* ─── Shared state between hooks ─────────────────────────────────────────── */
static volatile DWORD g_want_jump = 0;      /* set by input thread */
static volatile DWORD g_jump_approved = 0;   /* set by entry hook if grounded */
static volatile DWORD g_frame_count = 0;
static volatile DWORD g_jump_count = 0;

/* ─── Raycast function ───────────────────────────────────────────────────── */

typedef struct { float x, y, z; } Vec3;

typedef Vec3* (__thiscall *FindMeshCollision_fn)(
    void *mesh,
    Vec3 *output,
    float originX, float originY, float originZ,
    float dirX,    float dirY,    float dirZ,
    float param8
);

#define FN_FIND_MESH_COLLISION ((FindMeshCollision_fn)0x403980)

static Vec3 g_raycast_hit;

/* Called from Ball_Update entry hook — on the GAME THREAD.
 * ECX (ball pointer) is passed as the parameter. */
static void __cdecl do_raycast_and_approve(DWORD ball)
{
    char buf[256];

    /* Only process if input thread requested a jump */
    if (g_want_jump == 0) return;

    /* Only process player ball (player_index >= 0 at ball+0x18) */
    if (*(int*)(ball + 0x18) < 0) return;

    /* Get mesh from Scene+0x8B0 */
    DWORD scene = *(DWORD*)(ball + 0x14);
    if (!scene) return;
    DWORD mesh = *(DWORD*)(scene + 0x8B0);
    if (!mesh) return;

    float ball_x = *(float*)(ball + 0x164);
    float ball_y = *(float*)(ball + 0x168);
    float ball_z = *(float*)(ball + 0x16C);
    float ball_radius = *(float*)(ball + 0x284);

    Vec3 result;

    /* Cast ray straight down from ball position */
    FN_FIND_MESH_COLLISION(
        (void*)mesh,
        &result,
        ball_x, ball_y, ball_z,
        0.0f, -1.0f, 0.0f,
        0.0f
    );

    g_raycast_hit = result;

    /* Distance from ball center to ground hit (Y axis) */
    float dy = ball_y - result.y;
    if (dy < 0) dy = -dy;

    if (dy <= ball_radius + GROUND_TOLERANCE) {
        /* Grounded — approve the jump */
        g_jump_approved = 1;
        g_jump_count++;

        wsprintfA(buf, "JUMP #%u: raycast GROUNDED (ball_y=%.1f hit_y=%.1f dy=%.1f r=%.1f)",
                  g_jump_count, ball_y, result.y, ball_y - result.y, ball_radius);
        diag_log(buf);
    } else {
        /* Airborne — deny */
        wsprintfA(buf, "DENIED: AIRBORNE (ball_y=%.1f hit_y=%.1f dy=%.1f r=%.1f)",
                  ball_y, result.y, dy, ball_radius);
        diag_log(buf);
    }

    g_want_jump = 0;
}

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

        if (space_down && !g_prev_space && g_want_jump == 0) {
            g_want_jump = 1;
        }
        g_prev_space = space_down;
    }
    return 0;
}

/* ─── Hook 1: Ball_Update entry (0x405E00) ─────────────────────────────────
 * At function entry it's safe to call C functions.
 * Ball_Update is __thiscall: ECX = ball pointer.
 *
 * Original 8 bytes:
 *   6AFF            push -1
 *   64A100000000    mov eax, fs:[0]
 *
 * Cave layout:
 *   PUSHAD
 *   PUSHFD
 *   PUSH ECX            ; ball pointer
 *   CALL do_raycast_and_approve
 *   ADD ESP, 4
 *   POPFD
 *   POPAD
 *   ; original 8 bytes
 *   push -1
 *   mov eax, fs:[0]
 *   JMP back to 0x405E08
 */

static BYTE *g_cave1 = NULL;

static void install_entry_hook(void)
{
    BYTE *hook_addr = (BYTE*)BALL_UPDATE_ENTRY;
    char buf[256];

    g_cave1 = (BYTE*)VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE,
                                   PAGE_EXECUTE_READWRITE);
    if (!g_cave1) { diag_log("entry hook: VirtualAlloc FAILED"); return; }

    wsprintfA(buf, "entry cave: %08X", (DWORD)g_cave1);
    diag_log(buf);

    int p = 0;

    /* PUSHAD — save all general registers */
    g_cave1[p++] = 0x60;

    /* PUSHFD — save flags */
    g_cave1[p++] = 0x9C;

    /* PUSH ECX (ball pointer — the __thiscall 'this') */
    g_cave1[p++] = 0x51;

    /* CALL do_raycast_and_approve — relative call */
    g_cave1[p++] = 0xE8;
    *(DWORD*)(g_cave1 + p) = (DWORD)do_raycast_and_approve - (DWORD)(g_cave1 + p + 4);
    p += 4;

    /* ADD ESP, 4 — clean up the parameter */
    g_cave1[p++] = 0x83; g_cave1[p++] = 0xC4; g_cave1[p++] = 0x04;

    /* POPFD — restore flags */
    g_cave1[p++] = 0x9D;

    /* POPAD — restore all general registers */
    g_cave1[p++] = 0x61;

    /* Original 8 bytes: push -1; mov eax, fs:[0] */
    g_cave1[p++] = 0x6A; g_cave1[p++] = 0xFF;
    g_cave1[p++] = 0x64; g_cave1[p++] = 0xA1;
    *(DWORD*)(g_cave1 + p) = 0x00000000; p += 4;

    /* JMP back to 0x405E08 */
    g_cave1[p++] = 0xE9;
    *(DWORD*)(g_cave1 + p) = (DWORD)(hook_addr + BALL_UPDATE_RELOC) - (DWORD)(g_cave1 + p + 4);
    p += 4;

    /* Patch hook site: E9 <offset> + 3 NOPs */
    DWORD old_protect;
    VirtualProtect(hook_addr, BALL_UPDATE_RELOC, PAGE_EXECUTE_READWRITE, &old_protect);

    DWORD jmp_offset = (DWORD)(g_cave1 - hook_addr - 5);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_offset;
    hook_addr[5] = 0x90;
    hook_addr[6] = 0x90;
    hook_addr[7] = 0x90;

    VirtualProtect(hook_addr, BALL_UPDATE_RELOC, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, BALL_UPDATE_RELOC);

    wsprintfA(buf, "ENTRY HOOK installed: %d bytes, cave=%08X", p, (DWORD)g_cave1);
    diag_log(buf);
}

/* ─── Hook 2: Phase 15 cave (0x407BB4) ─────────────────────────────────────
 * Same as v12/v13: checks g_jump_approved and adds impulse to ball+0x174.
 *
 * Original 6 bytes: MOV ECX,[ESP+1C]; MOV EDX,[ECX]
 * ESI = ball pointer
 */

static BYTE *g_cave2 = NULL;

static void install_phase15_hook(void)
{
    BYTE *hook_addr = (BYTE*)PHASE15_HOOK;
    char buf[256];

    g_cave2 = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE,
                                   PAGE_EXECUTE_READWRITE);
    if (!g_cave2) { diag_log("phase15 hook: VirtualAlloc FAILED"); return; }

    int p = 0;

    /* CMP [g_jump_approved], 0 */
    g_cave2[p++] = 0x83; g_cave2[p++] = 0x3D;
    *(DWORD*)(g_cave2 + p) = (DWORD)&g_jump_approved; p += 4;
    g_cave2[p++] = 0x00;

    /* JZ skip — jump not approved */
    g_cave2[p++] = 0x74;
    g_cave2[p++] = 28;

    /* FLD [ESI+0x174] — load Y force accumulator */
    g_cave2[p++] = 0xD9; g_cave2[p++] = 0x86;
    *(DWORD*)(g_cave2 + p) = 0x174; p += 4;

    /* FADD [g_jump_impulse] — add upward impulse */
    g_cave2[p++] = 0xD8; g_cave2[p++] = 0x05;
    *(DWORD*)(g_cave2 + p) = (DWORD)&g_jump_impulse; p += 4;

    /* FSTP [ESI+0x174] — store modified Y force */
    g_cave2[p++] = 0xD9; g_cave2[p++] = 0x9E;
    *(DWORD*)(g_cave2 + p) = 0x174; p += 4;

    /* MOV [g_jump_approved], 0 — consume the approval */
    g_cave2[p++] = 0xC7; g_cave2[p++] = 0x05;
    *(DWORD*)(g_cave2 + p) = (DWORD)&g_jump_approved; p += 4;
    *(DWORD*)(g_cave2 + p) = 0; p += 4;

    /* skip: */

    /* Original 6 bytes: MOV ECX,[ESP+1C]; MOV EDX,[ECX] */
    g_cave2[p++] = 0x8B; g_cave2[p++] = 0x4C; g_cave2[p++] = 0x24; g_cave2[p++] = 0x1C;
    g_cave2[p++] = 0x8B; g_cave2[p++] = 0x11;

    /* INC [g_frame_count] */
    g_cave2[p++] = 0xFF; g_cave2[p++] = 0x05;
    *(DWORD*)(g_cave2 + p) = (DWORD)&g_frame_count; p += 4;

    /* JMP back to hook_addr + 6 */
    g_cave2[p++] = 0xE9;
    *(DWORD*)(g_cave2 + p) = (DWORD)(hook_addr + PHASE15_RELOC) - (DWORD)(g_cave2 + p + 4);
    p += 4;

    /* Patch hook site */
    DWORD old_protect;
    VirtualProtect(hook_addr, PHASE15_RELOC, PAGE_EXECUTE_READWRITE, &old_protect);

    DWORD jmp_offset = (DWORD)(g_cave2 - hook_addr - 5);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_offset;
    hook_addr[5] = 0x90;

    VirtualProtect(hook_addr, PHASE15_RELOC, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, PHASE15_RELOC);

    wsprintfA(buf, "PHASE15 HOOK installed: %d bytes, cave=%08X", p, (DWORD)g_cave2);
    diag_log(buf);
}

/* ─── Patch thread ───────────────────────────────────────────────────────── */

static DWORD WINAPI patch_thread(LPVOID param)
{
    (void)param;
    char buf[256];

    diag_log("patch_thread: started");
    Sleep(5000);

    /* Verify bytes at both hook sites */
    BYTE *entry_hook = (BYTE*)BALL_UPDATE_ENTRY;
    BYTE entry_expected[] = { 0x6A, 0xFF, 0x64, 0xA1, 0x00, 0x00, 0x00, 0x00 };
    wsprintfA(buf, "Entry bytes: %02X %02X %02X %02X %02X %02X %02X %02X",
              entry_hook[0], entry_hook[1], entry_hook[2], entry_hook[3],
              entry_hook[4], entry_hook[5], entry_hook[6], entry_hook[7]);
    diag_log(buf);
    if (memcmp(entry_hook, entry_expected, 8) != 0) {
        diag_log("ENTRY BYTE MISMATCH!");
        return 1;
    }

    BYTE *phase15_hook = (BYTE*)PHASE15_HOOK;
    BYTE phase15_expected[] = { 0x8B, 0x4C, 0x24, 0x1C, 0x8B, 0x11 };
    wsprintfA(buf, "Phase15 bytes: %02X %02X %02X %02X %02X %02X",
              phase15_hook[0], phase15_hook[1], phase15_hook[2],
              phase15_hook[3], phase15_hook[4], phase15_hook[5]);
    diag_log(buf);
    if (memcmp(phase15_hook, phase15_expected, 6) != 0) {
        diag_log("PHASE15 BYTE MISMATCH!");
        return 1;
    }

    install_entry_hook();
    install_phase15_hook();

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

        diag_log("=== jump_mod v14b loaded ===");

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
