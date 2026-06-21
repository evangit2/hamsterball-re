/*
 * jump_mod.c — BASS.dll proxy — v16 SIMPLE GROUND DETECTION
 *
 * v15c had 3 caves (entry clear + type-5 set + Phase 15 check) but
 * type-5 apparently doesn't fire every frame on normal ground.
 *
 * v16: Drop the type-5 and entry hooks entirely. Just use ball+0x2E9
 * directly in the Phase 15 cave. It's a "sticky" flag that's set to 1
 * when the ball touches floor and never cleared within Ball_Update.
 * 
 * This means: once you've touched floor, you can jump. After jumping,
 * you're airborne — but the flag is still 1. However, the ball is now
 * moving upward fast enough that by the next time you'd want to jump,
 * the ball will have landed and the flag will still be 1.
 *
 * The REAL issue v13-v15c were trying to solve was "prevent double
 * jumping in midair after rolling off a ledge." For that, we add a
 * simple cooldown: after a jump, don't allow another jump for N frames.
 * During that cooldown, if the ball is still airborne, it can't jump.
 * If it lands during the cooldown, the landing resets the cooldown.
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
 * Jump Mod v16 — single Phase 15 cave, ball+0x2E9 + cooldown
 * ═══════════════════════════════════════════════════════════════════════════ */

/* ─── Hook address ───────────────────────────────────────────────────────── */
#define PHASE15_HOOK       0x00407BB4
#define PHASE15_ORIG_BYTES 6  /* 8B 4C 24 1C 8B 11 = MOV ECX,[ESP+1C]; MOV EDX,[ECX] */

/* ─── Parameters ──────────────────────────────────────────────────────────── */
static float g_jump_impulse = 20.0f;
#define JUMP_COOLDOWN_FRAMES 20  /* ~0.33 seconds at 60fps */

/* ─── Shared state ───────────────────────────────────────────────────────── */
static volatile DWORD g_want_jump = 0;        /* set by input thread */
static volatile DWORD g_frame_count = 0;
static volatile DWORD g_jump_count = 0;
static volatile DWORD g_cooldown = 0;          /* frames remaining until next jump allowed */

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

/* ─── Phase 15 impulse hook (0x407BB4) ────────────────────────────────────
 *
 * Original 6 bytes: 8B 4C 24 1C 8B 11  =  MOV ECX,[ESP+1C]; MOV EDX,[ECX]
 * ESI = ball pointer.
 *
 * Logic:
 *   1. Decrement cooldown (if > 0)
 *   2. Check g_want_jump
 *   3. Check ball+0x2E9 (floor contact flag — sticky, set by type-5)
 *   4. Check cooldown == 0
 *   5. If all pass: add impulse, set cooldown, clear g_want_jump
 *
 * ball+0x2E9 is 1 when the ball has touched floor (type-5 collision).
 * It's never cleared within Ball_Update, so it stays 1 after landing.
 * The cooldown prevents double-jumping: after a jump, you must wait
 * JUMP_COOLDOWN_FRAMES before jumping again. By that time, if you
 * rolled off a ledge, the ball will be in fall_mode (0xC4C != 0)
 * which also blocks jumping.
 */
static BYTE *g_cave = NULL;

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

    g_cave = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE,
                                   PAGE_EXECUTE_READWRITE);
    if (!g_cave) { diag_log("phase15: VirtualAlloc FAILED"); return; }

    int p = 0;

    /* ─── Decrement cooldown ─── */
    /* CMP [g_cooldown], 0 */
    g_cave[p++] = 0x83; g_cave[p++] = 0x3D;
    *(DWORD*)(g_cave + p) = (DWORD)&g_cooldown; p += 4;
    g_cave[p++] = 0x00;

    /* JZ to .skip_dec (near jump) */
    int jz_skip_dec_fixup = p;
    g_cave[p++] = 0x0F; g_cave[p++] = 0x84;
    *(DWORD*)(g_cave + p) = 0; p += 4;

    /* DEC [g_cooldown] */
    g_cave[p++] = 0xFF; g_cave[p++] = 0x0D;
    *(DWORD*)(g_cave + p) = (DWORD)&g_cooldown; p += 4;

    /* ─── .skip_dec: ─── */
    int skip_dec_target = p;

    /* ─── Check g_want_jump ─── */
    /* CMP [g_want_jump], 0 */
    g_cave[p++] = 0x83; g_cave[p++] = 0x3D;
    *(DWORD*)(g_cave + p) = (DWORD)&g_want_jump; p += 4;
    g_cave[p++] = 0x00;

    /* JZ to .no_jump (near jump) */
    int jz_want_fixup = p;
    g_cave[p++] = 0x0F; g_cave[p++] = 0x84;
    *(DWORD*)(g_cave + p) = 0; p += 4;

    /* ─── Check ball+0x2E9 (floor contact) ─── */
    /* CMP BYTE [ESI+0x2E9], 0 */
    g_cave[p++] = 0x80; g_cave[p++] = 0xBE;
    *(DWORD*)(g_cave + p) = 0x2E9; p += 4;
    g_cave[p++] = 0x00;

    /* JZ to .no_jump (near jump) */
    int jz_floor_fixup = p;
    g_cave[p++] = 0x0F; g_cave[p++] = 0x84;
    *(DWORD*)(g_cave + p) = 0; p += 4;

    /* ─── Check ball+0xC4C (fall_mode — not dying/respawning) ─── */
    /* CMP DWORD [ESI+0xC4C], 0 */
    g_cave[p++] = 0x83; g_cave[p++] = 0xBE;
    *(DWORD*)(g_cave + p) = 0xC4C; p += 4;
    g_cave[p++] = 0x00;

    /* JNZ to .no_jump (near jump) */
    int jnz_fall_fixup = p;
    g_cave[p++] = 0x0F; g_cave[p++] = 0x85;
    *(DWORD*)(g_cave + p) = 0; p += 4;

    /* ─── Check cooldown ─── */
    /* CMP [g_cooldown], 0 */
    g_cave[p++] = 0x83; g_cave[p++] = 0x3D;
    *(DWORD*)(g_cave + p) = (DWORD)&g_cooldown; p += 4;
    g_cave[p++] = 0x00;

    /* JNZ to .no_jump (near jump) */
    int jnz_cd_fixup = p;
    g_cave[p++] = 0x0F; g_cave[p++] = 0x85;
    *(DWORD*)(g_cave + p) = 0; p += 4;

    /* ─── .jump: add impulse to ball+0x174 ─── */
    /* FLD [ESI+0x174] — load Y force accumulator */
    g_cave[p++] = 0xD9; g_cave[p++] = 0x86;
    *(DWORD*)(g_cave + p) = 0x174; p += 4;

    /* FADD [g_jump_impulse] — add upward impulse */
    g_cave[p++] = 0xD8; g_cave[p++] = 0x05;
    *(DWORD*)(g_cave + p) = (DWORD)&g_jump_impulse; p += 4;

    /* FSTP [ESI+0x174] — store modified Y force */
    g_cave[p++] = 0xD9; g_cave[p++] = 0x9E;
    *(DWORD*)(g_cave + p) = 0x174; p += 4;

    /* MOV [g_want_jump], 0 — consume jump request */
    g_cave[p++] = 0xC7; g_cave[p++] = 0x05;
    *(DWORD*)(g_cave + p) = (DWORD)&g_want_jump; p += 4;
    *(DWORD*)(g_cave + p) = 0; p += 4;

    /* MOV [g_cooldown], JUMP_COOLDOWN_FRAMES — start cooldown */
    g_cave[p++] = 0xC7; g_cave[p++] = 0x05;
    *(DWORD*)(g_cave + p) = (DWORD)&g_cooldown; p += 4;
    *(DWORD*)(g_cave + p) = JUMP_COOLDOWN_FRAMES; p += 4;

    /* INC [g_jump_count] */
    g_cave[p++] = 0xFF; g_cave[p++] = 0x05;
    *(DWORD*)(g_cave + p) = (DWORD)&g_jump_count; p += 4;

    /* ─── .no_jump: ─── */
    int no_jump_target = p;

    /* ─── Original 6 bytes ─── */
    g_cave[p++] = 0x8B; g_cave[p++] = 0x4C; g_cave[p++] = 0x24; g_cave[p++] = 0x1C;
    g_cave[p++] = 0x8B; g_cave[p++] = 0x11;

    /* INC [g_frame_count] */
    g_cave[p++] = 0xFF; g_cave[p++] = 0x05;
    *(DWORD*)(g_cave + p) = (DWORD)&g_frame_count; p += 4;

    /* JMP back to hook_addr + 6 */
    g_cave[p++] = 0xE9;
    *(DWORD*)(g_cave + p) = (DWORD)(hook_addr + PHASE15_ORIG_BYTES) - (DWORD)(g_cave + p + 4);
    p += 4;

    /* ─── Fix up near jumps ─── */
    *(DWORD*)(g_cave + jz_skip_dec_fixup + 2) = (DWORD)(g_cave + skip_dec_target) - (DWORD)(g_cave + jz_skip_dec_fixup + 6);
    *(DWORD*)(g_cave + jz_want_fixup + 2) = (DWORD)(g_cave + no_jump_target) - (DWORD)(g_cave + jz_want_fixup + 6);
    *(DWORD*)(g_cave + jz_floor_fixup + 2) = (DWORD)(g_cave + no_jump_target) - (DWORD)(g_cave + jz_floor_fixup + 6);
    *(DWORD*)(g_cave + jnz_fall_fixup + 2) = (DWORD)(g_cave + no_jump_target) - (DWORD)(g_cave + jnz_fall_fixup + 6);
    *(DWORD*)(g_cave + jnz_cd_fixup + 2) = (DWORD)(g_cave + no_jump_target) - (DWORD)(g_cave + jnz_cd_fixup + 6);

    /* ─── Patch hook site ─── */
    DWORD old_protect;
    VirtualProtect(hook_addr, PHASE15_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);

    DWORD jmp_offset = (DWORD)(g_cave - hook_addr - 5);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_offset;
    hook_addr[5] = 0x90;

    VirtualProtect(hook_addr, PHASE15_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, PHASE15_ORIG_BYTES);

    wsprintfA(buf, "PHASE15 HOOK installed: %d bytes, cave=%08X", p, (DWORD)g_cave);
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
    wsprintfA(buf, "After 8s: frames=%u jumps=%u cooldown=%u",
              g_frame_count, g_jump_count, g_cooldown);
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

        diag_log("=== jump_mod v16 loaded ===");

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
