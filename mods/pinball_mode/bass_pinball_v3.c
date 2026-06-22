// ============================================================
// Hamsterball Pinball Mode - bass.dll proxy (v3 lazy loader)
//
// Uses lazy loading: bass_real.dll is loaded on first BASS call,
// NOT in DllMain (avoids loader lock deadlock on real Windows).
// If bass_real.dll is missing, falls back to stubs (no audio, no crash).
//
// Installation:
//   1. Rename your original bass.dll to bass_real.dll
//   2. Put this bass.dll in the game folder
//   3. If bass_real.dll is missing, game runs without audio (no crash)
//
// F8: Toggle pinball mode on/off
// F9: Cycle bounce multiplier (2x -> 3x -> 5x -> 10x -> 2x)
// ============================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>

/* ---- BASS Proxy: Lazy load bass_real.dll on first call ---- */

static HMODULE g_hRealBass = NULL;
static int g_bass_tried_load = 0;

static void load_real_bass(void) {
    if (g_bass_tried_load) return;
    g_bass_tried_load = 1;
    
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char* slash = strrchr(path, '\\');
    if (slash) {
        strcpy(slash + 1, "bass_real.dll");
    } else {
        strcpy(path, "bass_real.dll");
    }
    g_hRealBass = LoadLibraryA(path);
}

/* Macro to define a forwarding function with lazy loading + stub fallback */
#define DEFINE_BASS_FORWARDED(name, ret_type, params, args, stub_ret) \
    typedef ret_type (__stdcall *name##_t) params; \
    static name##_t real_##name = NULL; \
    __declspec(dllexport) ret_type __stdcall name params { \
        if (!g_bass_tried_load) load_real_bass(); \
        if (g_hRealBass && !real_##name) \
            real_##name = (name##_t)GetProcAddress(g_hRealBass, #name); \
        if (real_##name) return real_##name args; \
        return stub_ret; \
    }

DEFINE_BASS_FORWARDED(BASS_Init, int, (int a, DWORD b, DWORD c, HWND d, void* e), (a, b, c, d, e), 1)
DEFINE_BASS_FORWARDED(BASS_Free, void, (void), (), )
DEFINE_BASS_FORWARDED(BASS_Start, int, (void), (), 1)
DEFINE_BASS_FORWARDED(BASS_Stop, int, (void), (), 0)
DEFINE_BASS_FORWARDED(BASS_SetConfig, int, (DWORD a, DWORD b), (a, b), 1)
DEFINE_BASS_FORWARDED(BASS_ErrorGetCode, int, (void), (), 0)
DEFINE_BASS_FORWARDED(BASS_MusicLoad, int, (BOOL a, const void* b, unsigned long long c, DWORD d, DWORD e, DWORD f), (a, b, c, d, e, f), 0)
DEFINE_BASS_FORWARDED(BASS_MusicPlayEx, int, (DWORD a, DWORD b, BOOL c), (a, b, c), 1)
DEFINE_BASS_FORWARDED(BASS_MusicFree, int, (DWORD a), (a), 0)
DEFINE_BASS_FORWARDED(BASS_ChannelSetAttributes, int, (DWORD a, float b, int c, int d), (a, b, c, d), 1)
DEFINE_BASS_FORWARDED(BASS_ChannelStop, int, (DWORD a), (a), 1)

/* ════════════════════════════════════════════════════════════════════
 * Pinball Mode
 * Hook 1 (0x407300): detect wall collision type==2, set flag
 * Hook 2 (0x407BB4): amplify bounce velocity by multiplier
 * ════════════════════════════════════════════════════════════════════ */

static const float BOUNCE_MULTS[] = {2.0f, 3.0f, 5.0f, 10.0f};
static const int NUM_MULTS = 4;

static volatile DWORD g_pinball_enabled = 0;
static volatile DWORD g_wall_hit = 0;
static volatile float g_bounce_mult = 3.0f;
static volatile int g_mult_index = 1;

static unsigned char orig_typecheck[] = {0x8B, 0x07, 0x83, 0xF8, 0x02};
static unsigned char orig_phase15[]  = {0x8B, 0x4C, 0x24, 0x1C, 0x8B, 0x11};

static void WriteBytes(void* addr, const void* data, int len) {
    DWORD old;
    VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &old);
    memcpy(addr, data, len);
    VirtualProtect(addr, len, old, &old);
    FlushInstructionCache(GetCurrentProcess(), addr, len);
}

static void WriteNops(void* addr, int len) {
    static const unsigned char nops[16] = {0x90};
    WriteBytes(addr, nops, len);
}

static int VerifyBytes(void* addr, const unsigned char* expected, int len) {
    return memcmp(addr, expected, len) == 0;
}

static void WriteJump(void* from, void* to) {
    unsigned char jmp[5];
    DWORD old;
    VirtualProtect(from, 5, PAGE_EXECUTE_READWRITE, &old);
    jmp[0] = 0xE9;
    *(unsigned int*)(jmp + 1) = (unsigned int)to - ((unsigned int)from + 5);
    memcpy(from, jmp, 5);
    VirtualProtect(from, 5, old, &old);
    FlushInstructionCache(GetCurrentProcess(), from, 5);
}

static unsigned char* cave_typecheck = NULL;
static unsigned char* cave_phase15  = NULL;

static void BuildCaveTypeCheck() {
    cave_typecheck = (unsigned char*)VirtualAlloc(NULL, 256, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_typecheck, 0x90, 256);
    unsigned char* buf = cave_typecheck;
    int i = 0;

    buf[i++] = 0x8B; buf[i++] = 0x07;
    buf[i++] = 0x83; buf[i++] = 0xF8; buf[i++] = 0x02;
    buf[i++] = 0x9C;
    buf[i++] = 0x83; buf[i++] = 0xF8; buf[i++] = 0x02;
    buf[i++] = 0x75; buf[i++] = 0x00;
    int jnz_pos = i - 1;
    buf[i++] = 0xC7; buf[i++] = 0x05;
    *(unsigned int*)(buf + i) = (unsigned int)&g_wall_hit; i += 4;
    *(unsigned int*)(buf + i) = 1; i += 4;
    int skip_pos = i;
    cave_typecheck[jnz_pos] = (unsigned char)(skip_pos - (jnz_pos + 1));
    buf[i++] = 0x9D;
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x407305 - ((unsigned int)cave_typecheck + i + 4);
    i += 4;
    WriteJump((void*)0x407300, cave_typecheck);
}

static void BuildCavePhase15() {
    cave_phase15 = (unsigned char*)VirtualAlloc(NULL, 512, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_phase15, 0x90, 512);
    unsigned char* buf = cave_phase15;
    int i = 0;

    buf[i++] = 0x83; buf[i++] = 0x3D;
    *(unsigned int*)(buf + i) = (unsigned int)&g_pinball_enabled; i += 4;
    buf[i++] = 0x00;
    buf[i++] = 0x74; buf[i++] = 0x00;
    int jz1_pos = i - 1;

    buf[i++] = 0x83; buf[i++] = 0x3D;
    *(unsigned int*)(buf + i) = (unsigned int)&g_wall_hit; i += 4;
    buf[i++] = 0x00;
    buf[i++] = 0x74; buf[i++] = 0x00;
    int jz2_pos = i - 1;

    buf[i++] = 0x8B; buf[i++] = 0x86;
    *(unsigned int*)(buf + i) = 0x1A4; i += 4;

    buf[i++] = 0xD9; buf[i++] = 0x80;
    *(unsigned int*)(buf + i) = 0xC64; i += 4;
    buf[i++] = 0xD8; buf[i++] = 0x0D;
    *(unsigned int*)(buf + i) = (unsigned int)&g_bounce_mult; i += 4;
    buf[i++] = 0xD9; buf[i++] = 0x98;
    *(unsigned int*)(buf + i) = 0xC64; i += 4;

    buf[i++] = 0xD9; buf[i++] = 0x80;
    *(unsigned int*)(buf + i) = 0xC64; i += 4;
    buf[i++] = 0xD8; buf[i++] = 0x88;
    *(unsigned int*)(buf + i) = 0xC8C; i += 4;
    buf[i++] = 0xD9; buf[i++] = 0x98;
    *(unsigned int*)(buf + i) = 0xC98; i += 4;

    buf[i++] = 0xD9; buf[i++] = 0x80;
    *(unsigned int*)(buf + i) = 0xC64; i += 4;
    buf[i++] = 0xD8; buf[i++] = 0x88;
    *(unsigned int*)(buf + i) = 0xC90; i += 4;
    buf[i++] = 0xD9; buf[i++] = 0x98;
    *(unsigned int*)(buf + i) = 0xC9C; i += 4;

    buf[i++] = 0xD9; buf[i++] = 0x80;
    *(unsigned int*)(buf + i) = 0xC64; i += 4;
    buf[i++] = 0xD8; buf[i++] = 0x88;
    *(unsigned int*)(buf + i) = 0xC94; i += 4;
    buf[i++] = 0xD9; buf[i++] = 0x98;
    *(unsigned int*)(buf + i) = 0xCA0; i += 4;

    buf[i++] = 0xC7; buf[i++] = 0x05;
    *(unsigned int*)(buf + i) = (unsigned int)&g_wall_hit; i += 4;
    *(unsigned int*)(buf + i) = 0; i += 4;

    int skip_pos = i;
    cave_phase15[jz1_pos] = (unsigned char)(skip_pos - (jz1_pos + 1));
    cave_phase15[jz2_pos] = (unsigned char)(skip_pos - (jz2_pos + 1));

    buf[i++] = 0x8B; buf[i++] = 0x4C; buf[i++] = 0x24; buf[i++] = 0x1C;
    buf[i++] = 0x8B; buf[i++] = 0x11;
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x407BBA - ((unsigned int)cave_phase15 + i + 4);
    i += 4;

    WriteJump((void*)0x407BB4, cave_phase15);
    WriteNops((void*)0x407BB9, 1);
}

#define DIK_F8  0x42
#define DIK_F9  0x43

static DWORD WINAPI KeyPollThread(LPVOID param) {
    DWORD app, input_handler, kb_dev;
    BYTE prev_f8 = 0, prev_f9 = 0;
    Sleep(2000);
    while (1) {
        Sleep(30);
        app = *(DWORD*)0x5341E0;
        if (!app) continue;
        input_handler = *(DWORD*)(app + 0x180);
        if (!input_handler) continue;
        kb_dev = *(DWORD*)(input_handler + 0x434);
        if (!kb_dev) continue;
        BYTE* keys = (BYTE*)(kb_dev + 0xC);

        BYTE f8_down = keys[DIK_F8] & 0x80;
        if (f8_down && !prev_f8) {
            g_pinball_enabled = !g_pinball_enabled;
            char msg[64];
            const char* state = g_pinball_enabled ? "ON" : "OFF";
            wsprintfA(msg, "Pinball Mode: %s (%.0fx)", state, g_bounce_mult);
            MessageBoxA(NULL, msg, "Pinball Mode", MB_OK | MB_TOPMOST);
        }
        prev_f8 = f8_down;

        BYTE f9_down = keys[DIK_F9] & 0x80;
        if (f9_down && !prev_f9) {
            g_mult_index = (g_mult_index + 1) % NUM_MULTS;
            g_bounce_mult = BOUNCE_MULTS[g_mult_index];
            char msg[64];
            wsprintfA(msg, "Bounce: %.0fx", g_bounce_mult);
            MessageBoxA(NULL, msg, "Pinball Mode", MB_OK | MB_TOPMOST);
        }
        prev_f9 = f9_down;
    }
    return 0;
}

static DWORD WINAPI InstallPatches(LPVOID param) {
    int wait_count = 0;
    while (!(*(DWORD*)0x5341E0) && wait_count < 100) {
        Sleep(100);
        wait_count++;
    }
    if (!(*(DWORD*)0x5341E0)) return 0;
    Sleep(500);

    if (!VerifyBytes((void*)0x407300, orig_typecheck, 5)) {
        MessageBoxA(NULL, "Hook 1: byte mismatch at 0x407300", "Pinball Mode", MB_OK | MB_ICONERROR);
        return 0;
    }
    if (!VerifyBytes((void*)0x407BB4, orig_phase15, 6)) {
        MessageBoxA(NULL, "Hook 2: byte mismatch at 0x407BB4", "Pinball Mode", MB_OK | MB_ICONERROR);
        return 0;
    }

    BuildCaveTypeCheck();
    BuildCavePhase15();
    CreateThread(NULL, 0, KeyPollThread, NULL, 0, NULL);
    return 0;
}

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInst);
        /* BASS proxy loads bass_real.dll lazily on first call — safe, no
         * loader lock. If bass_real.dll is missing, stubs return success
         * and the game runs without audio. */
        CreateThread(NULL, 0, InstallPatches, NULL, 0, NULL);
    }
    return TRUE;
}
