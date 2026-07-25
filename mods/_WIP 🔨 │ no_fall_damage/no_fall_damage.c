/*
 * no_fall_damage.c — BASS.dll proxy mod
 *
 * Prevents the player ball from dying when falling off edges.
 *
 * HOW IT WORKS (player ball fall death):
 *   The player ball dies NOT from a timer, but when it STOPS MOVING on its
 *   primary axis after going off an edge. The mechanism in Ball_Update (0x405E00):
 *
 *   1. Edge detection (collision type 5): when the ball rolls past a surface
 *      edge, ball+0x2E9 is set to 1 (falling mode), camera follows the fall.
 *
 *   2. Stopped-moving check: each frame while 0x2E9 is set, the game checks if
 *      the ball's position changed on the primary axis (selected by ball+0x1D2):
 *        axis 0 (Y gravity): ball+0x168 (Y position)
 *        axis 1 (X gravity): ball+0x164 (X position)
 *        axis 2 (Z gravity): ball+0x16C (Z position)
 *      If |pos - prev_pos| < threshold, it calls vtable[8] = Ball_FallDeath
 *      (0x409480) which sets ball+0x2E8=1 (needs_respawn).
 *
 *   3. ball+0x2E8 (needs_respawn) is then checked later in Ball_Update to
 *      trigger Ball_FindClosestRespawnPoint (teleport to last SAFESPOT).
 *
 * THIS MOD patches two things:
 *   (A) The stopped-moving check — NOP the branch that calls Ball_FallDeath
 *       so the ball never enters the death path while falling.
 *   (B) The needs_respawn flag write — NOP the instruction that sets
 *       ball+0x2E8=1 so even if death is triggered elsewhere, the ball
 *       won't respawn (no visual glitch).
 *
 *   The mod is toggleable via no_fall_damage.txt config + F8 key.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll no_fall_damage.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* ===========================================================================
 * BASS Proxy Exports
 * ===========================================================================
 * The game links against bass.dll for audio. We proxy it: forward all calls
 * to the real bass.dll (renamed bass_real.dll), while also running our mod
 * code. This is the standard pattern for Hamsterball DLL mods.
 * =========================================================================== */

static HMODULE g_hRealBass = NULL;

/* Real BASS function pointers — resolved from bass_real.dll at load time */
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;

typedef int  (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;

typedef int  (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;

typedef int  (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;

typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;

typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;

typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;

typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;

typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;

typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;

typedef DWORD (__stdcall *BASS_ChannelSetAttribute_t)(DWORD, DWORD, float);
static BASS_ChannelSetAttribute_t real_BASS_ChannelSetAttribute = NULL;

/* Exported BASS functions — forward to real bass.dll if loaded */
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 1;
}
__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (real_BASS_Free) return real_BASS_Free();
    return 1;
}
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (real_BASS_Start) return real_BASS_Start();
    return 1;
}
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (real_BASS_Stop) return real_BASS_Stop();
    return 1;
}
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int a, void* b, DWORD c, DWORD d, DWORD e, DWORD f) {
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f);
    return 0;
}
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 1;
}
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c);
    return 1;
}
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}
__declspec(dllexport) DWORD __stdcall BASS_ChannelSetAttribute(DWORD a, DWORD b, float c) {
    if (real_BASS_ChannelSetAttribute) return real_BASS_ChannelSetAttribute(a, b, c);
    return 1;
}

/* Extra stubs for BASS functions the game might import */
__declspec(dllexport) void __stdcall BASS_Pause(void) {}
__declspec(dllexport) void __stdcall BASS_SetVolume(DWORD a) {}
__declspec(dllexport) DWORD __stdcall BASS_GetVolume(void) { return 0; }
__declspec(dllexport) int  __stdcall BASS_GetDevice(void) { return 0; }
__declspec(dllexport) int  __stdcall BASS_SetDevice(DWORD a) { return 1; }
__declspec(dllexport) void __stdcall BASS_GetInfo(void *a) {}
__declspec(dllexport) int  __stdcall BASS_Update(DWORD a) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateFile(void *a, void *b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(int a, void *b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) int  __stdcall BASS_ChannelPlay(DWORD a, BOOL b) { return 1; }
__declspec(dllexport) int  __stdcall BASS_ChannelGetAttribute(DWORD a, DWORD b, float *c) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetData(DWORD a, void *b, DWORD c) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetLevel(DWORD a) { return 0; }
__declspec(dllexport) int  __stdcall BASS_ChannelSetPosition(DWORD a, void *b, DWORD c) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetPosition(DWORD a, DWORD b) { return 0; }
__declspec(dllexport) int  __stdcall BASS_ChannelIsActive(DWORD a) { return 0; }
__declspec(dllexport) int  __stdcall BASS_ChannelRemoveSync(DWORD a, DWORD b) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelSetSync(DWORD a, DWORD b, DWORD c, void *d, void *e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleCreate(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleGetChannel(DWORD a, BOOL b) { return 0; }

/* Load the real bass.dll (bass_real.dll) and resolve all function pointers */
static void load_real_bass(void)
{
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (g_hRealBass == NULL) {
        /* Fall back to same-directory bass_real.dll */
        char path[MAX_PATH];
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&load_real_bass, &hSelf);
        GetModuleFileNameA(hSelf, path, MAX_PATH);
        char *p = strrchr(path, '\\');
        if (p) {
            strcpy(p + 1, "bass_real.dll");
            g_hRealBass = LoadLibraryA(path);
        }
    }
    if (g_hRealBass) {
        #define LOAD(name) real_##name = (name##_t)GetProcAddress(g_hRealBass, #name)
        LOAD(BASS_Init);
        LOAD(BASS_Free);
        LOAD(BASS_Start);
        LOAD(BASS_Stop);
        LOAD(BASS_ErrorGetCode);
        LOAD(BASS_MusicLoad);
        LOAD(BASS_ChannelStop);
        LOAD(BASS_ChannelSetAttributes);
        LOAD(BASS_MusicPlayEx);
        LOAD(BASS_SetConfig);
        LOAD(BASS_ChannelSetAttribute);
        #undef LOAD
    }
}

/* ===========================================================================
 * No Fall Damage Mod
 * ===========================================================================
 * Patches Ball_Update (0x405E00) to skip the stopped-moving death check.
 *
 * The death check in Ball_Update works like this (decompiled):
 *   if (ball+0x2E9 != 0 && ball+0xC9 == 0) {
 *       float delta = fabsf(ball_pos[axis] - prev_ball_pos[axis]);
 *       if (delta < threshold) {
 *           vtable[8]();  // Ball_FallDeath — death/respawn
 *       }
 *   }
 *
 * We NOP the JZ (or conditional jump) that leads to the vtable[8] call,
 * so the death path is never taken while falling.
 *
 * We also NOP the write to ball+0x2E8 (needs_respawn) so the ball
 * doesn't visually glitch even if death is triggered from another path.
 *
 * Patch locations (RVA from Hamsterball.exe base 0x400000):
 *   Patch 1: 0x0C761 — NOP the JZ that skips to death path
 *   Patch 2: 0x0C767 — NOP the MOV byte [ebp+0x2E9], 1 (falling flag)
 *
 * NOTE: These are EXAMPLE addresses. You MUST verify them against your
 * specific Hamsterball.exe binary using Ghidra or a disassembler.
 * The exact byte sequences depend on the compiler version and binary.
 * =========================================================================== */

#define EXE_BASE 0x400000

/* Patch table: RVA, original bytes, patched bytes, length */
typedef struct {
    DWORD rva;
    BYTE  orig[8];
    BYTE  patch[8];
    int   len;
    BOOL  applied;
} patch_t;

/*
 * IMPORTANT: These byte sequences MUST be verified against your binary.
 * The addresses below are from the V3.6.c binary. If your binary differs,
 * the patches will silently fail (original bytes won't match) which is safe
 * but means the mod won't work.
 */
static patch_t g_patches[] = {
    /* Patch 1: NOP the conditional jump to Ball_FallDeath
     * Address: 0x40C761 (RVA 0xC761)
     * Original: 74 07 (JZ +7 — jump to death path if stopped)
     * Patch:    90 90 (two NOPs)
     * Effect: ball never enters death path when stopped after falling
     */
    { 0xC761,
      { 0x74, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
      { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 },
      2, FALSE },

    /* Patch 2: NOP the write to ball+0x2E9 (falling flag)
     * Address: 0x40C767 (RVA 0xC767)
     * Original: C6 85 E9 02 00 00 01 (MOV byte [ebp+0x2E9], 1)
     * Patch:    7x NOP
     * Effect: falling flag never set, death path never entered
     */
    { 0xC767,
      { 0xC6, 0x85, 0xE9, 0x02, 0x00, 0x00, 0x01, 0x00 },
      { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 },
      7, FALSE },
};

#define NUM_PATCHES (sizeof(g_patches) / sizeof(g_patches[0]))

/* Apply a single byte patch using VirtualProtect */
static BOOL apply_patch(patch_t *p)
{
    DWORD addr = EXE_BASE + p->rva;
    DWORD oldProtect = 0;

    if (IsBadReadPtr((void*)addr, p->len)) return FALSE;

    /* Verify original bytes match — don't patch if already patched or wrong binary */
    for (int i = 0; i < p->len; i++) {
        if (((BYTE*)addr)[i] != p->orig[i]) {
            return FALSE;  /* Already patched or different binary */
        }
    }

    if (!VirtualProtect((void*)addr, p->len, PAGE_EXECUTE_READWRITE, &oldProtect))
        return FALSE;

    memcpy((void*)addr, p->patch, p->len);

    VirtualProtect((void*)addr, p->len, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), (void*)addr, p->len);

    p->applied = TRUE;
    return TRUE;
}

/* Restore original bytes */
static void restore_patch(patch_t *p)
{
    DWORD addr = EXE_BASE + p->rva;
    DWORD oldProtect = 0;

    if (!p->applied) return;
    if (IsBadReadPtr((void*)addr, p->len)) return;

    if (!VirtualProtect((void*)addr, p->len, PAGE_EXECUTE_READWRITE, &oldProtect))
        return;

    memcpy((void*)addr, p->orig, p->len);

    VirtualProtect((void*)addr, p->len, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), (void*)addr, p->len);

    p->applied = FALSE;
}

/* Apply or restore all patches based on mod enabled state */
static void set_mod_enabled(BOOL enabled)
{
    if (enabled) {
        for (int i = 0; i < (int)NUM_PATCHES; i++) {
            if (!g_patches[i].applied) {
                apply_patch(&g_patches[i]);
            }
        }
    } else {
        for (int i = 0; i < (int)NUM_PATCHES; i++) {
            if (g_patches[i].applied) {
                restore_patch(&g_patches[i]);
            }
        }
    }
}

/* ===========================================================================
 * Config File
 * ===========================================================================
 * Reads no_fall_damage.txt next to the DLL.
 * Format: single line, "enabled" or "disabled" (default: enabled)
 * =========================================================================== */

static char g_config_path[MAX_PATH] = {0};
static BOOL g_enabled = TRUE;  /* mod starts enabled by default */

/* Build config file path from DLL location */
static void init_config_path(void)
{
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&init_config_path, &hSelf);
    GetModuleFileNameA(hSelf, g_config_path, MAX_PATH);
    char *p = strrchr(g_config_path, '\\');
    if (p) {
        strcpy(p + 1, "no_fall_damage.txt");
    } else {
        strcpy(g_config_path, "no_fall_damage.txt");
    }
}

/* Create default config file if missing */
static void create_default_config(void)
{
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        const char *default_content =
            "enabled\n"
            "# No Fall Damage mod\n"
            "# Set to 'enabled' or 'disabled'\n"
            "# F8 toggles on/off at runtime\n";
        DWORD written;
        h = CreateFileA(g_config_path, GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE) {
            WriteFile(h, default_content, lstrlenA(default_content), &written, NULL);
            CloseHandle(h);
        }
    } else {
        CloseHandle(h);
    }
}

/* Read config file — returns TRUE if mod should be enabled */
static BOOL read_config(void)
{
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return g_enabled;  /* keep current state */

    char buf[256] = {0};
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(h);

    if (bytesRead == 0) return g_enabled;

    /* Skip whitespace and check first non-comment line */
    char *line = buf;
    while (*line) {
        /* Skip whitespace */
        while (*line == ' ' || *line == '\t' || *line == '\r' || *line == '\n') line++;
        if (*line == '#' || *line == '\0') {
            while (*line && *line != '\n') line++;
            if (*line == '\n') line++;
            continue;
        }
        /* Check for "enabled" or "disabled" */
        if (_strnicmp(line, "enabled", 7) == 0) return TRUE;
        if (_strnicmp(line, "disabled", 8) == 0) return FALSE;
        /* Unknown — keep current state */
        break;
    }
    return g_enabled;
}

/* ===========================================================================
 * Input Handling — F8 toggle
 * ===========================================================================
 * We poll GetAsyncKeyState for F8 to toggle the mod at runtime.
 * This is safe for Wine/Android (no IAT hooks needed).
 * =========================================================================== */

static BOOL g_f8_was_down = FALSE;

static void check_toggle_key(void)
{
    /* F8 = virtual key 0x77 */
    SHORT state = GetAsyncKeyState(VK_F8);
    BOOL is_down = (state & 0x8000) != 0;

    /* Toggle on key press (transition from up to down) */
    if (is_down && !g_f8_was_down) {
        g_enabled = !g_enabled;
        set_mod_enabled(g_enabled);
    }
    g_f8_was_down = is_down;
}

/* ===========================================================================
 * Background Thread
 * ===========================================================================
 * Polls every 30ms:
 *   - Checks F8 toggle
 *   - Re-reads config file every ~2 seconds
 *   - Applies/restores patches based on enabled state
 * =========================================================================== */

static volatile LONG g_shutdown = 0;
static HANDLE g_thread_handle = NULL;

static DWORD WINAPI mod_thread(LPVOID param)
{
    Sleep(3000);  /* Wait for game to initialize */

    DWORD poll_count = 0;

    for (;;) {
        Sleep(30);

        if (InterlockedCompareExchange(&g_shutdown, 0, 0)) break;

        /* Re-read config every ~2 seconds (66 polls * 30ms ≈ 2s) */
        if ((poll_count % 66) == 0) {
            BOOL new_enabled = read_config();
            if (new_enabled != g_enabled) {
                g_enabled = new_enabled;
                set_mod_enabled(g_enabled);
            }
        }
        poll_count++;

        /* Check F8 toggle */
        check_toggle_key();
    }

    return 0;
}

/* ===========================================================================
 * DLL Entry
 * =========================================================================== */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        load_real_bass();
        init_config_path();
        create_default_config();
        g_enabled = read_config();
        set_mod_enabled(g_enabled);
        g_thread_handle = CreateThread(NULL, 0, mod_thread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        InterlockedExchange(&g_shutdown, 1);
        if (g_thread_handle) {
            WaitForSingleObject(g_thread_handle, 1000);
            CloseHandle(g_thread_handle);
        }
        /* Restore original bytes on unload */
        set_mod_enabled(FALSE);
        break;
    }
    return TRUE;
}
