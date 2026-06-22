// ============================================================
// Hamsterball Pinball Mode - bass.dll proxy (v2)
// Uses .def file forwarding for BASS functions (zero crash risk).
// Only contains pinball hook code + keyboard polling.
// F8: Toggle pinball mode on/off
// F9: Cycle bounce multiplier (2x -> 3x -> 5x -> 10x -> 2x)
// ============================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>

/* BASS forwarding is handled ENTIRELY by the .def file:
 *   BASS_Init = bass_real.BASS_Init
 *   BASS_Free = bass_real.BASS_Free
 *   ...etc...
 * The OS loader resolves these at load time. No LoadLibraryA,
 * no race conditions, no stack corruption from wrong param counts.
 * This DLL only needs the pinball hook code. */

/* ════════════════════════════════════════════════════════════════════
 * Pinball Mode — see bass_pinball.c v1 for full design doc.
 * Hook 1 (0x407300): detect wall collision type==2, set flag
 * Hook 2 (0x407BB4): amplify bounce velocity by multiplier
 * ════════════════════════════════════════════════════════════════════ */

static const float BOUNCE_MULTS[] = {2.0f, 3.0f, 5.0f, 10.0f};
static const int NUM_MULTS = 4;

static volatile DWORD g_pinball_enabled = 0;
static volatile DWORD g_wall_hit = 0;
static volatile float g_bounce_mult = 3.0f;
static volatile int g_mult_index = 1;

/* Original bytes */
static unsigned char orig_typecheck[] = {0x8B, 0x07, 0x83, 0xF8, 0x02};
static unsigned char orig_phase15[]  = {0x8B, 0x4C, 0x24, 0x1C, 0x8B, 0x11};

/* --- Memory helpers --- */

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

/* --- Code Caves --- */

static unsigned char* cave_typecheck = NULL;
static unsigned char* cave_phase15  = NULL;

static void BuildCaveTypeCheck() {
    cave_typecheck = (unsigned char*)VirtualAlloc(NULL, 256, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_typecheck, 0x90, 256);

    unsigned char* buf = cave_typecheck;
    int i = 0;

    /* Original 5 bytes */
    buf[i++] = 0x8B; buf[i++] = 0x07;
    buf[i++] = 0x83; buf[i++] = 0xF8; buf[i++] = 0x02;

    buf[i++] = 0x9C;  /* PUSHFD */
    buf[i++] = 0x83; buf[i++] = 0xF8; buf[i++] = 0x02; /* CMP EAX,2 */
    buf[i++] = 0x75; buf[i++] = 0x00;  /* JNZ skip */
    int jnz_pos = i - 1;

    buf[i++] = 0xC7; buf[i++] = 0x05;
    *(unsigned int*)(buf + i) = (unsigned int)&g_wall_hit; i += 4;
    *(unsigned int*)(buf + i) = 1; i += 4;

    int skip_pos = i;
    cave_typecheck[jnz_pos] = (unsigned char)(skip_pos - (jnz_pos + 1));

    buf[i++] = 0x9D;  /* POPFD */

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

    /* Check g_pinball_enabled */
    buf[i++] = 0x83; buf[i++] = 0x3D;
    *(unsigned int*)(buf + i) = (unsigned int)&g_pinball_enabled; i += 4;
    buf[i++] = 0x00;
    buf[i++] = 0x74; buf[i++] = 0x00;
    int jz1_pos = i - 1;

    /* Check g_wall_hit */
    buf[i++] = 0x83; buf[i++] = 0x3D;
    *(unsigned int*)(buf + i) = (unsigned int)&g_wall_hit; i += 4;
    buf[i++] = 0x00;
    buf[i++] = 0x74; buf[i++] = 0x00;
    int jz2_pos = i - 1;

    /* MOV EAX, [ESI + 0x1A4] (collision_node) */
    buf[i++] = 0x8B; buf[i++] = 0x86;
    *(unsigned int*)(buf + i) = 0x1A4; i += 4;

    /* speed *= g_bounce_mult */
    buf[i++] = 0xD9; buf[i++] = 0x80;
    *(unsigned int*)(buf + i) = 0xC64; i += 4;
    buf[i++] = 0xD8; buf[i++] = 0x0D;
    *(unsigned int*)(buf + i) = (unsigned int)&g_bounce_mult; i += 4;
    buf[i++] = 0xD9; buf[i++] = 0x98;
    *(unsigned int*)(buf + i) = 0xC64; i += 4;

    /* velX = speed * dirX */
    buf[i++] = 0xD9; buf[i++] = 0x80;
    *(unsigned int*)(buf + i) = 0xC64; i += 4;
    buf[i++] = 0xD8; buf[i++] = 0x88;
    *(unsigned int*)(buf + i) = 0xC8C; i += 4;
    buf[i++] = 0xD9; buf[i++] = 0x98;
    *(unsigned int*)(buf + i) = 0xC98; i += 4;

    /* velY = speed * dirY */
    buf[i++] = 0xD9; buf[i++] = 0x80;
    *(unsigned int*)(buf + i) = 0xC64; i += 4;
    buf[i++] = 0xD8; buf[i++] = 0x88;
    *(unsigned int*)(buf + i) = 0xC90; i += 4;
    buf[i++] = 0xD9; buf[i++] = 0x98;
    *(unsigned int*)(buf + i) = 0xC9C; i += 4;

    /* velZ = speed * dirZ */
    buf[i++] = 0xD9; buf[i++] = 0x80;
    *(unsigned int*)(buf + i) = 0xC64; i += 4;
    buf[i++] = 0xD8; buf[i++] = 0x88;
    *(unsigned int*)(buf + i) = 0xC94; i += 4;
    buf[i++] = 0xD9; buf[i++] = 0x98;
    *(unsigned int*)(buf + i) = 0xCA0; i += 4;

    /* Clear g_wall_hit */
    buf[i++] = 0xC7; buf[i++] = 0x05;
    *(unsigned int*)(buf + i) = (unsigned int)&g_wall_hit; i += 4;
    *(unsigned int*)(buf + i) = 0; i += 4;

    int skip_pos = i;
    cave_phase15[jz1_pos] = (unsigned char)(skip_pos - (jz1_pos + 1));
    cave_phase15[jz2_pos] = (unsigned char)(skip_pos - (jz2_pos + 1));

    /* Original 6 bytes */
    buf[i++] = 0x8B; buf[i++] = 0x4C; buf[i++] = 0x24; buf[i++] = 0x1C;
    buf[i++] = 0x8B; buf[i++] = 0x11;

    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x407BBA - ((unsigned int)cave_phase15 + i + 4);
    i += 4;

    WriteJump((void*)0x407BB4, cave_phase15);
    WriteNops((void*)0x407BB9, 1);
}

/* --- Keyboard polling --- */

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

/* --- Installation --- */

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

/* --- DLL Entry Point --- */

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInst);
        /* BASS forwarding is handled by the .def file — nothing to init here.
         * Just spawn the hook installation thread. */
        CreateThread(NULL, 0, InstallPatches, NULL, 0, NULL);
    }
    return TRUE;
}
