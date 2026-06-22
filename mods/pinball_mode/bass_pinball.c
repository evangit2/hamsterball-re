// ============================================================
// Hamsterball Pinball Mode - bass.dll proxy
// Amplifies wall bounce velocity universally across all levels.
// F8: Toggle pinball mode on/off
// F9: Cycle bounce multiplier (2x -> 3x -> 5x -> 10x -> 2x)
// ============================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>

/* ---- BASS Proxy: Forward to bass_real.dll ---- */

static HMODULE g_hRealBass = NULL;

/* Only the 10 functions the game actually imports need forwarding.
 * All others are empty stubs (the game never calls them). */

typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 1;
}

typedef void (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) void __stdcall BASS_Free(void) {
    if (real_BASS_Free) real_BASS_Free();
}

typedef int (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (real_BASS_Start) return real_BASS_Start();
    return 1;
}

typedef int (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (real_BASS_Stop) return real_BASS_Stop();
    return 0;
}

typedef int (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}

typedef int (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}

/* BASS_MusicLoad: BOOL mem, const void *file, QWORD offset, DWORD length, DWORD flags, DWORD freq
 * 6 params = 28 bytes on 32-bit stack (QWORD = 8 bytes = 2 DWORDs).
 * WRONG param count = stack corruption = crash! Must match exactly. */
typedef int (__stdcall *BASS_MusicLoad_t)(BOOL, const void*, unsigned long long, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) int __stdcall BASS_MusicLoad(BOOL a, const void* b, unsigned long long c, DWORD d, DWORD e, DWORD f) {
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f);
    return 0;
}

typedef int (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c);
    return 1;
}

typedef int (__stdcall *BASS_MusicFree_t)(DWORD);
static BASS_MusicFree_t real_BASS_MusicFree = NULL;
__declspec(dllexport) int __stdcall BASS_MusicFree(DWORD a) {
    if (real_BASS_MusicFree) return real_BASS_MusicFree(a);
    return 0;
}

typedef int (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}

typedef int (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 1;
}

static void init_bass_proxy(void) {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char* slash = strrchr(path, '\\');
    if (slash) {
        strcpy(slash + 1, "bass_real.dll");
    } else {
        strcpy(path, "bass_real.dll");
    }
    g_hRealBass = LoadLibraryA(path);
    if (g_hRealBass) {
        real_BASS_Init              = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
        real_BASS_Free              = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
        real_BASS_Start             = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
        real_BASS_Stop              = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
        real_BASS_SetConfig         = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
        real_BASS_ErrorGetCode      = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
        real_BASS_MusicLoad         = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
        real_BASS_MusicPlayEx       = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
        real_BASS_MusicFree         = (BASS_MusicFree_t)GetProcAddress(g_hRealBass, "BASS_MusicFree");
        real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
        real_BASS_ChannelStop       = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
    }
}

/* ════════════════════════════════════════════════════════════════════
 * Pinball Mode
 *
 * HOW IT WORKS:
 * Ball_Update (0x405E00) processes all physics for a ball each frame.
 * Inside, a collision loop at 0x407300 iterates collision entries:
 *   type 1 = ball-ball, type 2 = wall, type 5 = floor
 *
 * When type==2 (wall) is found, the game computes a bounce response:
 *   - Reflects velocity along the wall normal
 *   - Computes new speed (collision_node+0xC64)
 *   - Writes velocity vector (c98/c9c/ca0 = speed * direction)
 *   - Breaks out of the collision loop
 *
 * All collision processing paths converge at 0x407BB4 (Phase 15),
 * where the vtable[0] physics integration call happens.
 *
 * HOOK 1 (0x407300, 5 bytes): Detect wall collision type==2,
 *   set g_wall_hit=1. Preserves flags for the original JZ.
 *
 * HOOK 2 (0x407BB4, 6 bytes): At Phase 15 convergence, if
 *   g_wall_hit, multiply collision_node speed (c64) by
 *   g_bounce_mult, recompute velocity = speed * direction.
 *   Clear g_wall_hit.
 *
 * Toggle: F8 = on/off, F9 = cycle multiplier (2x/3x/5x/10x)
 * ════════════════════════════════════════════════════════════════════ */

/* --- Configuration --- */
static const float BOUNCE_MULTS[] = {2.0f, 3.0f, 5.0f, 10.0f};
static const int NUM_MULTS = 4;

/* --- Globals --- */
static volatile DWORD g_pinball_enabled = 0;  /* off by default */
static volatile DWORD g_wall_hit = 0;          /* set per-collision, cleared at Phase 15 */
static volatile float g_bounce_mult = 3.0f;     /* default 3x */
static volatile int g_mult_index = 1;          /* index into BOUNCE_MULTS */

/* --- Original bytes --- */

/* Hook 1: Collision type check (0x407300)
 * 5 bytes: 8B 07 83 F8 02
 * MOV EAX,[EDI]; CMP EAX,2 */
static unsigned char orig_typecheck[] = {0x8B, 0x07, 0x83, 0xF8, 0x02};

/* Hook 2: Phase 15 convergence (0x407BB4)
 * 6 bytes: 8B 4C 24 1C 8B 11
 * MOV ECX,[ESP+0x1C]; MOV EDX,[ECX] */
static unsigned char orig_phase15[] = {0x8B, 0x4C, 0x24, 0x1C, 0x8B, 0x11};

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

/* --- Caves --- */

static unsigned char* cave_typecheck = NULL;
static unsigned char* cave_phase15 = NULL;

/* Hook 1: Collision type check (0x407300)
 * Original: MOV EAX,[EDI]; CMP EAX,2  (5 bytes)
 * EDI = collision entry pointer, [EDI] = type (1=ball,2=wall,5=floor)
 *
 * Cave: execute original, save flags, check if type==2, set g_wall_hit=1,
 * restore flags, JMP back to 0x407305 (original JZ instruction). */
static void BuildCaveTypeCheck() {
    cave_typecheck = (unsigned char*)VirtualAlloc(NULL, 256, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_typecheck, 0x90, 256);

    unsigned char* buf = cave_typecheck;
    int i = 0;

    /* Original 5 bytes: MOV EAX,[EDI]; CMP EAX,2 */
    buf[i++] = 0x8B; buf[i++] = 0x07;              /* MOV EAX,[EDI] */
    buf[i++] = 0x83; buf[i++] = 0xF8; buf[i++] = 0x02; /* CMP EAX,2 */

    /* Save flags (needed by JZ at 0x407305) */
    buf[i++] = 0x9C;  /* PUSHFD */

    /* Check if type == 2 (wall collision) */
    buf[i++] = 0x83; buf[i++] = 0xF8; buf[i++] = 0x02; /* CMP EAX,2 */
    buf[i++] = 0x75; buf[i++] = 0x00;  /* JNZ skip - fill later */
    int jnz_pos = i - 1;

    /* MOV DWORD [g_wall_hit], 1 */
    buf[i++] = 0xC7; buf[i++] = 0x05;
    *(unsigned int*)(buf + i) = (unsigned int)&g_wall_hit;
    i += 4;
    *(unsigned int*)(buf + i) = 1;
    i += 4;

    /* skip: */
    int skip_pos = i;
    cave_typecheck[jnz_pos] = (unsigned char)(skip_pos - (jnz_pos + 1));

    /* Restore flags */
    buf[i++] = 0x9D;  /* POPFD */

    /* JMP to 0x407305 (original JZ instruction) */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x407305 - ((unsigned int)cave_typecheck + i + 4);
    i += 4;

    WriteJump((void*)0x407300, cave_typecheck);
}

/* Hook 2: Phase 15 convergence (0x407BB4)
 * Original: MOV ECX,[ESP+0x1C]; MOV EDX,[ECX]  (6 bytes)
 * ESI = ball pointer, collision_node = [ESI+0x1A4]
 *
 * Collision node layout:
 *   +0xC64 = speed (float)
 *   +0xC8C = direction X (float)
 *   +0xC90 = direction Y (float)
 *   +0xC94 = direction Z (float)
 *   +0xC98 = velocity X (float) = speed * dirX
 *   +0xC9C = velocity Y (float) = speed * dirY
 *   +0xCA0 = velocity Z (float) = speed * dirZ
 *
 * Cave: if g_pinball_enabled && g_wall_hit:
 *   speed *= g_bounce_mult
 *   velocity = speed * direction (recompute all 3 components)
 *   clear g_wall_hit
 * Then execute original 6 bytes, JMP to 0x407BBA. */
static void BuildCavePhase15() {
    cave_phase15 = (unsigned char*)VirtualAlloc(NULL, 512, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_phase15, 0x90, 512);

    unsigned char* buf = cave_phase15;
    int i = 0;

    /* Check g_pinball_enabled */
    buf[i++] = 0x83; buf[i++] = 0x3D;  /* CMP DWORD [imm32], 0 */
    *(unsigned int*)(buf + i) = (unsigned int)&g_pinball_enabled;
    i += 4;
    buf[i++] = 0x00;
    buf[i++] = 0x74; buf[i++] = 0x00;  /* JZ skip - fill later */
    int jz1_pos = i - 1;

    /* Check g_wall_hit */
    buf[i++] = 0x83; buf[i++] = 0x3D;  /* CMP DWORD [imm32], 0 */
    *(unsigned int*)(buf + i) = (unsigned int)&g_wall_hit;
    i += 4;
    buf[i++] = 0x00;
    buf[i++] = 0x74; buf[i++] = 0x00;  /* JZ skip - fill later */
    int jz2_pos = i - 1;

    /* Load collision_node pointer: MOV EAX, [ESI + 0x1A4] */
    buf[i++] = 0x8B; buf[i++] = 0x86;
    *(unsigned int*)(buf + i) = 0x1A4;
    i += 4;
    /* EAX = collision_node */

    /* Amplify speed: FLD [EAX+0xC64]; FMUL [g_bounce_mult]; FSTP [EAX+0xC64] */
    buf[i++] = 0xD9; buf[i++] = 0x80;  /* FLD DWORD [EAX+imm32] */
    *(unsigned int*)(buf + i) = 0xC64;
    i += 4;

    buf[i++] = 0xD8; buf[i++] = 0x0D;  /* FMUL DWORD [imm32] */
    *(unsigned int*)(buf + i) = (unsigned int)&g_bounce_mult;
    i += 4;

    buf[i++] = 0xD9; buf[i++] = 0x98;  /* FSTP DWORD [EAX+imm32] */
    *(unsigned int*)(buf + i) = 0xC64;
    i += 4;

    /* Recompute velocity = speed * direction */
    /* velX = speed * dirX: FLD [EAX+0xC64]; FMUL [EAX+0xC8C]; FSTP [EAX+0xC98] */
    buf[i++] = 0xD9; buf[i++] = 0x80;  /* FLD DWORD [EAX+imm32] */
    *(unsigned int*)(buf + i) = 0xC64;
    i += 4;
    buf[i++] = 0xD8; buf[i++] = 0x88;  /* FMUL DWORD [EAX+imm32] */
    *(unsigned int*)(buf + i) = 0xC8C;
    i += 4;
    buf[i++] = 0xD9; buf[i++] = 0x98;  /* FSTP DWORD [EAX+imm32] */
    *(unsigned int*)(buf + i) = 0xC98;
    i += 4;

    /* velY = speed * dirY: FLD [EAX+0xC64]; FMUL [EAX+0xC90]; FSTP [EAX+0xC9C] */
    buf[i++] = 0xD9; buf[i++] = 0x80;  /* FLD DWORD [EAX+imm32] */
    *(unsigned int*)(buf + i) = 0xC64;
    i += 4;
    buf[i++] = 0xD8; buf[i++] = 0x88;  /* FMUL DWORD [EAX+imm32] */
    *(unsigned int*)(buf + i) = 0xC90;
    i += 4;
    buf[i++] = 0xD9; buf[i++] = 0x98;  /* FSTP DWORD [EAX+imm32] */
    *(unsigned int*)(buf + i) = 0xC9C;
    i += 4;

    /* velZ = speed * dirZ: FLD [EAX+0xC64]; FMUL [EAX+0xC94]; FSTP [EAX+0xCA0] */
    buf[i++] = 0xD9; buf[i++] = 0x80;  /* FLD DWORD [EAX+imm32] */
    *(unsigned int*)(buf + i) = 0xC64;
    i += 4;
    buf[i++] = 0xD8; buf[i++] = 0x88;  /* FMUL DWORD [EAX+imm32] */
    *(unsigned int*)(buf + i) = 0xC94;
    i += 4;
    buf[i++] = 0xD9; buf[i++] = 0x98;  /* FSTP DWORD [EAX+imm32] */
    *(unsigned int*)(buf + i) = 0xCA0;
    i += 4;

    /* Clear g_wall_hit: MOV DWORD [g_wall_hit], 0 */
    buf[i++] = 0xC7; buf[i++] = 0x05;
    *(unsigned int*)(buf + i) = (unsigned int)&g_wall_hit;
    i += 4;
    *(unsigned int*)(buf + i) = 0;
    i += 4;

    /* skip: (target of both JZ above) */
    int skip_pos = i;
    cave_phase15[jz1_pos] = (unsigned char)(skip_pos - (jz1_pos + 1));
    cave_phase15[jz2_pos] = (unsigned char)(skip_pos - (jz2_pos + 1));

    /* Original 6 bytes: MOV ECX,[ESP+0x1C]; MOV EDX,[ECX] */
    buf[i++] = 0x8B; buf[i++] = 0x4C; buf[i++] = 0x24; buf[i++] = 0x1C; /* MOV ECX,[ESP+0x1C] */
    buf[i++] = 0x8B; buf[i++] = 0x11;  /* MOV EDX,[ECX] */

    /* JMP to 0x407BBA */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x407BBA - ((unsigned int)cave_phase15 + i + 4);
    i += 4;

    WriteJump((void*)0x407BB4, cave_phase15);
    /* NOP the 6th byte (we replaced 6 bytes with 5-byte JMP + 1 NOP) */
    WriteNops((void*)0x407BB9, 1);
}

/* --- Keyboard polling thread --- */

#define DIK_F8  0x42
#define DIK_F9  0x43

static DWORD WINAPI KeyPollThread(LPVOID param) {
    DWORD app, input_handler, kb_dev;
    BYTE prev_f8 = 0, prev_f9 = 0;

    Sleep(2000);  /* wait for game to initialize */

    while (1) {
        Sleep(30);

        /* Keyboard chain: App(0x5341E0) -> +0x180 -> InputHandler -> +0x434 -> KeyboardDevice -> +0xC = key buffer */
        app = *(DWORD*)0x5341E0;
        if (!app) continue;
        input_handler = *(DWORD*)(app + 0x180);
        if (!input_handler) continue;
        kb_dev = *(DWORD*)(input_handler + 0x434);
        if (!kb_dev) continue;

        BYTE* keys = (BYTE*)(kb_dev + 0xC);

        /* F8: Toggle pinball on/off (rising edge) */
        BYTE f8_down = keys[DIK_F8] & 0x80;
        if (f8_down && !prev_f8) {
            g_pinball_enabled = !g_pinball_enabled;
            char msg[64];
            const char* state = g_pinball_enabled ? "ON" : "OFF";
            wsprintfA(msg, "Pinball Mode: %s (%.0fx)", state, g_bounce_mult);
            MessageBoxA(NULL, msg, "Pinball Mode", MB_OK | MB_TOPMOST);
        }
        prev_f8 = f8_down;

        /* F9: Cycle multiplier (rising edge, only when enabled) */
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
    /* BASS proxy is already initialized in DllMain — no need to redo it.
     * Wait for the game's code to be fully loaded — App must be allocated
     * before we can safely patch. App is at 0x5341E0. */
    int wait_count = 0;
    while (!(*(DWORD*)0x5341E0) && wait_count < 100) {
        Sleep(100);
        wait_count++;
    }
    if (!(*(DWORD*)0x5341E0)) {
        /* App not initialized after 10s — bail out silently */
        return 0;
    }

    /* Small extra delay to ensure all code sections are mapped */
    Sleep(500);

    /* Verify original bytes */
    if (!VerifyBytes((void*)0x407300, orig_typecheck, 5)) {
        MessageBoxA(NULL, "Hook 1: byte mismatch at 0x407300", "Pinball Mode", MB_OK | MB_ICONERROR);
        return 0;
    }
    if (!VerifyBytes((void*)0x407BB4, orig_phase15, 6)) {
        MessageBoxA(NULL, "Hook 2: byte mismatch at 0x407BB4", "Pinball Mode", MB_OK | MB_ICONERROR);
        return 0;
    }

    /* Build code caves */
    BuildCaveTypeCheck();
    BuildCavePhase15();

    /* Start keyboard polling thread */
    CreateThread(NULL, 0, KeyPollThread, NULL, 0, NULL);
    return 0;
}

/* --- DLL Entry Point --- */

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInst);
        /* Initialize BASS forwarding IMMEDIATELY — the game calls BASS_Init
         * and BASS_MusicLoad during App.Initialize(), which runs before our
         * patch thread even wakes up. If bass_real.dll isn't loaded yet,
         * the stub returns wrong values and the game crashes. */
        init_bass_proxy();
        /* Spawn the hook installation thread (delayed) */
        CreateThread(NULL, 0, InstallPatches, NULL, 0, NULL);
    }
    return TRUE;
}
