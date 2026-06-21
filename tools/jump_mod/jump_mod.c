/*
 * jump_mod.c — BASS.dll proxy — v13 GROUND-DETECTED JUMP
 *
 * v12 works but allows mid-air jumping. v13 adds ground detection.
 *
 * Ground detection: reads ball+0x180 (Y displacement per frame).
 *   - On ground: Y disp ≈ 0 (ball is stationary or rolling horizontally)
 *   - In air rising: Y disp > 0
 *   - In air falling: Y disp < -0.5
 *   - Jump only allowed when abs(Y_disp) < GROUND_THRESHOLD
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
 * Jump Mod v13 — velocity impulse with ground detection
 * ═══════════════════════════════════════════════════════════════════════════ */

#define PHASE15_HOOK       0x00407BB4
#define HOOK_ORIG_BYTES    6

static float g_jump_impulse = 20.0f;

/* Ground detection threshold for ball+0x180 (Y displacement).
 * On ground: |Y_disp| < threshold. In air: |Y_disp| >= threshold. */
#define GROUND_THRESHOLD   0.5f

static volatile DWORD g_jump_requested = 0;
static volatile DWORD g_frame_count = 0;
static volatile DWORD g_jump_count = 0;

/* ─── Input polling thread with ground detection ─────────────────────────── */
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
            /* Spacebar just pressed — check if ball is on ground */
            DWORD scene = *(DWORD*)(app + 0x1A4);
            if (scene) {
                /* Get ball list from Scene+0x29D4 and find player ball */
                DWORD ball_list = scene + 0x29D4;
                int count = *(int*)(ball_list + 0x1C);
                if (count > 0) {
                    /* First ball in the list is usually the player */
                    DWORD *balls = *(DWORD**)(ball_list + 0x424);
                    if (balls && balls[0]) {
                        DWORD ball = balls[0];
                        /* Read Y displacement at ball+0x180 */
                        float y_disp = *(float*)(ball + 0x180);
                        float abs_y = y_disp < 0 ? -y_disp : y_disp;

                        if (abs_y < GROUND_THRESHOLD) {
                            g_jump_requested = 1;
                            g_jump_count++;

                            if (log_counter < 50) {
                                wsprintfA(buf, "JUMP #%u: y_disp=%.3f (grounded!)", g_jump_count, y_disp);
                                diag_log(buf);
                                log_counter++;
                            }
                        } else {
                            if (log_counter < 50) {
                                wsprintfA(buf, "JUMP DENIED: y_disp=%.3f (airborne)", y_disp);
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

    wsprintfA(buf, "HOOK v13 INSTALLED! impulse=%f threshold=%f cave=%08X",
              g_jump_impulse, GROUND_THRESHOLD, (DWORD)cave);
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

        diag_log("=== jump_mod v13 loaded ===");

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
