/*
 * jump_mod.c — BASS.dll proxy — v15 TYPE-5 GROUND DETECTION
 *
 * v14b used a Ball_Update entry hook to call Ball_FindMeshCollision for
 * raycast ground detection — this FROZE the game because calling
 * Mesh_FindClosestCollision from within Ball_Update causes reentrancy
 * issues with the game's internal collision state.
 *
 * v15 uses Pattern 5: type-5 collision hook. No game function calls.
 * Cave 1 at 0x407391 observes the existing type-5 floor collision write
 * (ball+0x2E9 = 1) and also sets g_on_ground = 1.
 * Cave 2 at 0x407BB4 (Phase 15) checks g_on_ground, adds impulse if set,
 * then CLEARS g_on_ground = 0 at end of frame.
 *
 * This is thread-safe (all on game thread) and has zero performance cost.
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
 * Jump Mod v15 — type-5 ground detection + Phase 15 impulse
 * ═══════════════════════════════════════════════════════════════════════════ */

/* ─── Hook addresses ─────────────────────────────────────────────────────── */
#define TYPE5_HOOK         0x00407391
#define TYPE5_ORIG_BYTES   7  /* C6 86 E9 02 00 00 01 = MOV BYTE [ESI+0x2E9], 1 */

#define PHASE15_HOOK       0x00407BB4
#define PHASE15_ORIG_BYTES 6  /* 8B 4C 24 1C 8B 11 = MOV ECX,[ESP+1C]; MOV EDX,[ECX] */

/* ─── Parameters ──────────────────────────────────────────────────────────── */
static float g_jump_impulse = 20.0f;

/* ─── Shared state ───────────────────────────────────────────────────────── */
static volatile DWORD g_on_ground = 0;        /* set by type-5 cave, cleared by Phase 15 cave */
static volatile DWORD g_want_jump = 0;        /* set by input thread */
static volatile DWORD g_frame_count = 0;
static volatile DWORD g_jump_count = 0;

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

/* ─── Cave 1: Type-5 floor collision hook (0x407391) ───────────────────────
 *
 * Original 7 bytes: C6 86 E9 02 00 00 01  =  MOV BYTE [ESI+0x2E9], 1
 * This fires when Ball_Update detects a type-5 (floor) collision.
 * ESI = ball pointer.
 *
 * Cave: execute original 7 bytes, then also set g_on_ground = 1.
 */
static BYTE *g_cave1 = NULL;

static void install_type5_hook(void)
{
    BYTE *hook_addr = (BYTE*)TYPE5_HOOK;
    char buf[256];

    BYTE expected[] = { 0xC6, 0x86, 0xE9, 0x02, 0x00, 0x00, 0x01 };
    wsprintfA(buf, "Type5 bytes: %02X %02X %02X %02X %02X %02X %02X",
              hook_addr[0], hook_addr[1], hook_addr[2], hook_addr[3],
              hook_addr[4], hook_addr[5], hook_addr[6]);
    diag_log(buf);

    if (memcmp(hook_addr, expected, 7) != 0) {
        diag_log("TYPE5 BYTE MISMATCH!");
        return;
    }

    g_cave1 = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE,
                                   PAGE_EXECUTE_READWRITE);
    if (!g_cave1) { diag_log("type5: VirtualAlloc FAILED"); return; }

    int p = 0;

    /* Execute original 7 bytes: MOV BYTE [ESI+0x2E9], 1 */
    g_cave1[p++] = 0xC6; g_cave1[p++] = 0x86;
    g_cave1[p++] = 0xE9; g_cave1[p++] = 0x02;
    g_cave1[p++] = 0x00; g_cave1[p++] = 0x00;
    g_cave1[p++] = 0x01;

    /* MOV DWORD [g_on_ground], 1 */
    g_cave1[p++] = 0xC7; g_cave1[p++] = 0x05;
    *(DWORD*)(g_cave1 + p) = (DWORD)&g_on_ground; p += 4;
    g_cave1[p++] = 0x01; g_cave1[p++] = 0x00;
    g_cave1[p++] = 0x00; g_cave1[p++] = 0x00;

    /* JMP back to hook_addr + 7 */
    g_cave1[p++] = 0xE9;
    *(DWORD*)(g_cave1 + p) = (DWORD)(hook_addr + TYPE5_ORIG_BYTES) - (DWORD)(g_cave1 + p + 4);
    p += 4;

    /* Patch hook site: E9 <offset> + 2 NOPs (7 bytes total) */
    DWORD old_protect;
    VirtualProtect(hook_addr, TYPE5_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);

    DWORD jmp_offset = (DWORD)(g_cave1 - hook_addr - 5);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_offset;
    hook_addr[5] = 0x90;
    hook_addr[6] = 0x90;

    VirtualProtect(hook_addr, TYPE5_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, TYPE5_ORIG_BYTES);

    wsprintfA(buf, "TYPE5 HOOK installed: %d bytes, cave=%08X", p, (DWORD)g_cave1);
    diag_log(buf);
}

/* ─── Cave 2: Phase 15 impulse hook (0x407BB4) ────────────────────────────
 *
 * Original 6 bytes: 8B 4C 24 1C 8B 11  =  MOV ECX,[ESP+1C]; MOV EDX,[ECX]
 * ESI = ball pointer.
 *
 * Cave: check g_want_jump AND g_on_ground. If both set, add impulse.
 * Always clear g_on_ground at end of frame.
 */
static BYTE *g_cave2 = NULL;

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

    g_cave2 = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE,
                                   PAGE_EXECUTE_READWRITE);
    if (!g_cave2) { diag_log("phase15: VirtualAlloc FAILED"); return; }

    int p = 0;

    /* ─── Check g_want_jump ─── */
    /* CMP [g_want_jump], 0 */
    g_cave2[p++] = 0x83; g_cave2[p++] = 0x3D;
    *(DWORD*)(g_cave2 + p) = (DWORD)&g_want_jump; p += 4;
    g_cave2[p++] = 0x00;

    /* JZ to .no_jump (near jump — target fixed up below) */
    int jz_want_fixup = p;
    g_cave2[p++] = 0x0F; g_cave2[p++] = 0x84;
    *(DWORD*)(g_cave2 + p) = 0;  /* placeholder */
    p += 4;

    /* ─── Check g_on_ground ─── */
    /* CMP [g_on_ground], 0 */
    g_cave2[p++] = 0x83; g_cave2[p++] = 0x3D;
    *(DWORD*)(g_cave2 + p) = (DWORD)&g_on_ground; p += 4;
    g_cave2[p++] = 0x00;

    /* JZ to .no_jump (near jump) */
    int jz_ground_fixup = p;
    g_cave2[p++] = 0x0F; g_cave2[p++] = 0x84;
    *(DWORD*)(g_cave2 + p) = 0;  /* placeholder */
    p += 4;

    /* ─── .jump: add impulse to ball+0x174 ─── */
    /* FLD [ESI+0x174] — load Y force accumulator */
    g_cave2[p++] = 0xD9; g_cave2[p++] = 0x86;
    *(DWORD*)(g_cave2 + p) = 0x174; p += 4;

    /* FADD [g_jump_impulse] — add upward impulse */
    g_cave2[p++] = 0xD8; g_cave2[p++] = 0x05;
    *(DWORD*)(g_cave2 + p) = (DWORD)&g_jump_impulse; p += 4;

    /* FSTP [ESI+0x174] — store modified Y force */
    g_cave2[p++] = 0xD9; g_cave2[p++] = 0x9E;
    *(DWORD*)(g_cave2 + p) = 0x174; p += 4;

    /* MOV [g_want_jump], 0 — consume jump request */
    g_cave2[p++] = 0xC7; g_cave2[p++] = 0x05;
    *(DWORD*)(g_cave2 + p) = (DWORD)&g_want_jump; p += 4;
    *(DWORD*)(g_cave2 + p) = 0; p += 4;

    /* INC [g_jump_count] */
    g_cave2[p++] = 0xFF; g_cave2[p++] = 0x05;
    *(DWORD*)(g_cave2 + p) = (DWORD)&g_jump_count; p += 4;

    /* ─── .no_jump: ─── */
    int no_jump_target = p;

    /* Clear g_on_ground for next frame */
    /* MOV DWORD [g_on_ground], 0 */
    g_cave2[p++] = 0xC7; g_cave2[p++] = 0x05;
    *(DWORD*)(g_cave2 + p) = (DWORD)&g_on_ground; p += 4;
    *(DWORD*)(g_cave2 + p) = 0; p += 4;

    /* ─── Original 6 bytes ─── */
    g_cave2[p++] = 0x8B; g_cave2[p++] = 0x4C; g_cave2[p++] = 0x24; g_cave2[p++] = 0x1C;
    g_cave2[p++] = 0x8B; g_cave2[p++] = 0x11;

    /* INC [g_frame_count] */
    g_cave2[p++] = 0xFF; g_cave2[p++] = 0x05;
    *(DWORD*)(g_cave2 + p) = (DWORD)&g_frame_count; p += 4;

    /* JMP back to hook_addr + 6 */
    g_cave2[p++] = 0xE9;
    *(DWORD*)(g_cave2 + p) = (DWORD)(hook_addr + PHASE15_ORIG_BYTES) - (DWORD)(g_cave2 + p + 4);
    p += 4;

    /* ─── Fix up near jumps ─── */
    *(DWORD*)(g_cave2 + jz_want_fixup + 2) = (DWORD)(g_cave2 + no_jump_target) - (DWORD)(g_cave2 + jz_want_fixup + 6);
    *(DWORD*)(g_cave2 + jz_ground_fixup + 2) = (DWORD)(g_cave2 + no_jump_target) - (DWORD)(g_cave2 + jz_ground_fixup + 6);

    /* ─── Patch hook site ─── */
    DWORD old_protect;
    VirtualProtect(hook_addr, PHASE15_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);

    DWORD jmp_offset = (DWORD)(g_cave2 - hook_addr - 5);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_offset;
    hook_addr[5] = 0x90;

    VirtualProtect(hook_addr, PHASE15_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, PHASE15_ORIG_BYTES);

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

    install_type5_hook();
    install_phase15_hook();

    CreateThread(NULL, 0, input_thread, NULL, 0, NULL);
    diag_log("input_thread launched");

    Sleep(8000);
    wsprintfA(buf, "After 8s: frames=%u jumps=%u grounded=%u",
              g_frame_count, g_jump_count, g_on_ground);
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

        diag_log("=== jump_mod v15 loaded ===");

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
