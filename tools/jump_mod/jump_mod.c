/*
 * jump_mod.c — BASS.dll proxy — v7 DIAGNOSTIC
 *
 * Writes diagnostic info to C:\hamsterball_jump_debug.txt
 * so we can see exactly where things fail.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll jump_mod.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias
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

static void diag_log(const char *msg)
{
    HANDLE hFile = CreateFileA("C:\\hamsterball_jump_debug.txt",
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

/* ═══════════════════════════════════════════════════════════════════════════
 * Jump Mod v7 — diagnostic version
 * ═══════════════════════════════════════════════════════════════════════════ */

#define BALL_UPDATE_HOOK    0x00407BB4
#define HOOK_ORIG_BYTES     6

static volatile DWORD g_frame_count = 0;
static float g_nudge = 2.0f;

static void install_hook(void)
{
    BYTE *hook_addr = (BYTE*)BALL_UPDATE_HOOK;

    diag_log("install_hook: allocating cave...");

    BYTE *cave = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE);
    if (!cave) {
        diag_log("install_hook: VirtualAlloc FAILED!");
        return;
    }

    char buf[128];
    wsprintfA(buf, "install_hook: cave at %08X, hook at %08X", (DWORD)cave, (DWORD)hook_addr);
    diag_log(buf);

    DWORD jmp_to_cave = (DWORD)(cave - hook_addr - 5);

    /* Log the jump displacement */
    wsprintfA(buf, "install_hook: jmp_to_cave disp = %08X", jmp_to_cave);
    diag_log(buf);

    int p = 0;

    /* Execute original 6 bytes: MOV ECX,[ESP+0x1C]; MOV EDX,[ECX] */
    cave[p++] = 0x8B; cave[p++] = 0x4C; cave[p++] = 0x24; cave[p++] = 0x1C;
    cave[p++] = 0x8B; cave[p++] = 0x11;

    /* FLD [ESI+0x168] — load ball position Y */
    cave[p++] = 0xD9; cave[p++] = 0x86;
    *(DWORD*)(cave + p) = 0x168; p += 4;

    /* FADD [g_nudge] — add 2.0f */
    cave[p++] = 0xD8; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_nudge; p += 4;

    /* FSTP [ESI+0x168] — store back */
    cave[p++] = 0xD9; cave[p++] = 0x9E;
    *(DWORD*)(cave + p) = 0x168; p += 4;

    /* INC [g_frame_count] — proves cave runs */
    cave[p++] = 0xFF; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_frame_count; p += 4;

    /* JMP back to hook_addr + 6 */
    cave[p++] = 0xE9;
    *(DWORD*)(cave + p) = (DWORD)(hook_addr + HOOK_ORIG_BYTES) - (DWORD)(cave + p + 4);
    p += 4;

    wsprintfA(buf, "install_hook: cave is %d bytes, patching hook site...", p);
    diag_log(buf);

    /* Patch hook site */
    DWORD old_protect;
    BOOL vp = VirtualProtect(hook_addr, HOOK_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);
    if (!vp) {
        wsprintfA(buf, "install_hook: VirtualProtect FAILED! err=%d", GetLastError());
        diag_log(buf);
        return;
    }

    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_to_cave;
    hook_addr[5] = 0x90;

    VirtualProtect(hook_addr, HOOK_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, HOOK_ORIG_BYTES);

    diag_log("install_hook: HOOK INSTALLED SUCCESSFULLY");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI patch_thread(LPVOID param)
{
    (void)param;
    char buf[128];

    diag_log("=== jump_mod v7 diagnostic ===");
    diag_log("patch_thread: started, sleeping 5000ms...");

    Sleep(5000);

    diag_log("patch_thread: awake, checking hook bytes...");

    BYTE *hook = (BYTE*)BALL_UPDATE_HOOK;
    BYTE expected[] = { 0x8B, 0x4C, 0x24, 0x1C, 0x8B, 0x11 };

    wsprintfA(buf, "patch_thread: actual bytes: %02X %02X %02X %02X %02X %02X",
              hook[0], hook[1], hook[2], hook[3], hook[4], hook[5]);
    diag_log(buf);

    wsprintfA(buf, "patch_thread: expected bytes: %02X %02X %02X %02X %02X %02X",
              expected[0], expected[1], expected[2], expected[3], expected[4], expected[5]);
    diag_log(buf);

    if (memcmp(hook, expected, 6) != 0) {
        diag_log("patch_thread: BYTE MISMATCH — hook NOT installed!");
        return 1;
    }

    diag_log("patch_thread: bytes match, calling install_hook...");
    install_hook();

    /* Wait a bit then log frame count to verify cave runs */
    Sleep(3000);
    wsprintfA(buf, "patch_thread: after 3s, g_frame_count = %u", g_frame_count);
    diag_log(buf);

    Sleep(5000);
    wsprintfA(buf, "patch_thread: after 8s total, g_frame_count = %u", g_frame_count);
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
        load_real_bass();
        {
            char buf[128];
            wsprintfA(buf, "DllMain: bass_real = %08X", (DWORD)g_hRealBass);
            diag_log(buf);
        }
        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
