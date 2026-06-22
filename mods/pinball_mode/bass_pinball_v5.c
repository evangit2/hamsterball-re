// ============================================================
// Hamsterball Pinball Mode - bass.dll proxy (v5)
//
// FIXES over v4:
//   - Completely wrong hook point fixed. v3/v4 modified physics_obj
//     internal fields (+0xC64 speed, +0xC98/C9C/CA0 velocity) which
//     corrupted collision math → ball stuck to ground randomly.
//   - v5 hooks at 0x407CE0, AFTER the collision callback has set
//     the bounce offset at ball+0x2C0/+0x2C4/+0x2C8 but BEFORE those
//     offsets are added to the ball's position. This is the ACTUAL
//     bounce displacement field — amplifying it = stronger bounce.
//   - No physics_obj fields touched at all. Zero interference with
//     collision math.
//
// Hook 1 (0x407300): detect wall collision (type==2), set g_wall_hit flag
// Hook 2 (0x407CE0): if g_wall_hit, multiply ball+0x2C0/2C4/2C8 by bounce_mult,
//   then execute original FLD [EDI]; FADD [ESI+0x164] and continue.
//
// F8: Toggle pinball mode on/off (silent)
// F9: Cycle bounce multiplier 2x -> 3x -> 5x -> 10x -> 2x (silent)
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
 *
 * Ball struct offsets:
 *   +0x164/+0x168/+0x16C = position X/Y/Z (float)
 *   +0x2C0/+0x2C4/+0x2C8 = bounce offset X/Y/Z (float, added to position)
 *
 * The bounce offset is set by the collision callback at 0x407BBC.
 * It's applied to position at 0x407CE0-0x407D15:
 *   pos += bounce_offset
 *   bounce_offset = 0 (cleared after use)
 *
 * v5 amplifies the bounce_offset AFTER the callback sets it but BEFORE
 * it's applied to position. Zero interference with collision math.
 * ════════════════════════════════════════════════════════════════════ */

static const float BOUNCE_MULTS[] = {2.0f, 3.0f, 5.0f, 10.0f};
static const int NUM_MULTS = 4;

static volatile DWORD g_pinball_enabled = 0;
static volatile DWORD g_wall_hit = 0;
static volatile float g_bounce_mult = 3.0f;
static volatile int g_mult_index = 1;

/* Original bytes at hook points */
/* 0x407300: MOV EAX,[EDI]; CMP EAX,2 (5 bytes) */
static unsigned char orig_typecheck[] = {0x8B, 0x07, 0x83, 0xF8, 0x02};
/* 0x407CE0: FLD [EDI]; FADD [ESI+0x164] (8 bytes) */
static unsigned char orig_bounceapply[] = {0xD9, 0x07, 0xD8, 0x86, 0x64, 0x01, 0x00, 0x00};

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
static unsigned char* cave_bounceapply = NULL;

/* ── Hook 1: Wall collision type check at 0x407300 ──
 *
 * Original: MOV EAX,[EDI]; CMP EAX,2 (5 bytes, then JZ at 0x407305)
 * Cave: same instructions + set g_wall_hit=1 if type==2
 * PUSHFD/POPFD to preserve flags for the original JZ.
 * JMP back to 0x407305.
 */
static void BuildCaveTypeCheck() {
    cave_typecheck = (unsigned char*)VirtualAlloc(NULL, 256, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_typecheck, 0x90, 256);
    unsigned char* buf = cave_typecheck;
    int i = 0;

    /* Original: MOV EAX, [EDI] */
    buf[i++] = 0x8B; buf[i++] = 0x07;
    /* Original: CMP EAX, 2 */
    buf[i++] = 0x83; buf[i++] = 0xF8; buf[i++] = 0x02;

    /* Save flags from CMP */
    buf[i++] = 0x9C;  /* PUSHFD */

    /* Re-compare to set g_wall_hit */
    buf[i++] = 0x83; buf[i++] = 0xF8; buf[i++] = 0x02;  /* CMP EAX, 2 */
    buf[i++] = 0x75; buf[i++] = 0x00;  /* JNZ skip */
    int jnz_pos = i - 1;

    /* MOV g_wall_hit, 1 */
    buf[i++] = 0xC7; buf[i++] = 0x05;
    *(unsigned int*)(buf + i) = (unsigned int)&g_wall_hit; i += 4;
    *(unsigned int*)(buf + i) = 1; i += 4;

    int skip_pos = i;
    cave_typecheck[jnz_pos] = (unsigned char)(skip_pos - (jnz_pos + 1));

    /* Restore flags */
    buf[i++] = 0x9D;  /* POPFD */

    /* JMP 0x407305 */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x407305 - ((unsigned int)cave_typecheck + i + 4);
    i += 4;

    WriteJump((void*)0x407300, cave_typecheck);
}

/* ── Hook 2: Amplify bounce offset at 0x407CE0 ──
 *
 * Original (8 bytes):
 *   0x407CE0: FLD [EDI]              ; D9 07         (load bounce_offset_X)
 *   0x407CE2: FADD [ESI+0x164]       ; D8 86 64010000 (pos_X += bounce_offset_X)
 *
 * After this, Y and Z are handled inline at 0x407CF7-0x407D15.
 *
 * Cave: if pinball_enabled && wall_hit:
 *   multiply [ESI+0x2C0], [ESI+0x2C4], [ESI+0x2C8] by g_bounce_mult
 *   clear g_wall_hit
 * Then execute original FLD [EDI]; FADD [ESI+0x164]
 * JMP to 0x407CE8 (LEA EDX,[ESP+0xA4] — the instruction after FADD)
 *
 * Register state at 0x407CE0:
 *   ESI = ball struct (preserved)
 *   EDI = ESI+0x2C0 (pointer to bounce_offset_X, set at 0x407C50)
 *   FPU stack: empty (previous FSTP at 0x407C80 popped the last value)
 *
 * We use EAX as scratch (it's not needed until 0x407C86 where it's reloaded).
 * FPU operations: each FLD+FMUL+FSTP is balanced (push, multiply, store+pop).
 *
 * Encoding:
 *   D9 86 xx xx xx xx  = FLD [ESI+disp32]     (push to FPU)
 *   D8 0D xx xx xx xx  = FMUL [disp32]        (multiply st0 by memory)
 *   D9 9E xx xx xx xx  = FSTP [ESI+disp32]    (store+pop from FPU)
 */
static void BuildCaveBounceApply() {
    cave_bounceapply = (unsigned char*)VirtualAlloc(NULL, 512, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_bounceapply, 0x90, 512);
    unsigned char* buf = cave_bounceapply;
    int i = 0;

    /* Check g_pinball_enabled, if 0 skip to original code */
    buf[i++] = 0x83; buf[i++] = 0x3D;  /* CMP dword ptr [addr], 0 */
    *(unsigned int*)(buf + i) = (unsigned int)&g_pinball_enabled; i += 4;
    buf[i++] = 0x00;
    buf[i++] = 0x74; buf[i++] = 0x00;  /* JZ skip */
    int jz1_pos = i - 1;

    /* Check g_wall_hit, if 0 skip to original code */
    buf[i++] = 0x83; buf[i++] = 0x3D;
    *(unsigned int*)(buf + i) = (unsigned int)&g_wall_hit; i += 4;
    buf[i++] = 0x00;
    buf[i++] = 0x74; buf[i++] = 0x00;  /* JZ skip */
    int jz2_pos = i - 1;

    /* --- Multiply bounce_offset_X at [ESI+0x2C0] --- */
    /* FLD [ESI+0x2C0] */
    buf[i++] = 0xD9; buf[i++] = 0x86;
    *(unsigned int*)(buf + i) = 0x2C0; i += 4;
    /* FMUL [g_bounce_mult] */
    buf[i++] = 0xD8; buf[i++] = 0x0D;
    *(unsigned int*)(buf + i) = (unsigned int)&g_bounce_mult; i += 4;
    /* FSTP [ESI+0x2C0] */
    buf[i++] = 0xD9; buf[i++] = 0x9E;
    *(unsigned int*)(buf + i) = 0x2C0; i += 4;

    /* --- Multiply bounce_offset_Y at [ESI+0x2C4] --- */
    buf[i++] = 0xD9; buf[i++] = 0x86;
    *(unsigned int*)(buf + i) = 0x2C4; i += 4;
    buf[i++] = 0xD8; buf[i++] = 0x0D;
    *(unsigned int*)(buf + i) = (unsigned int)&g_bounce_mult; i += 4;
    buf[i++] = 0xD9; buf[i++] = 0x9E;
    *(unsigned int*)(buf + i) = 0x2C4; i += 4;

    /* --- Multiply bounce_offset_Z at [ESI+0x2C8] --- */
    buf[i++] = 0xD9; buf[i++] = 0x86;
    *(unsigned int*)(buf + i) = 0x2C8; i += 4;
    buf[i++] = 0xD8; buf[i++] = 0x0D;
    *(unsigned int*)(buf + i) = (unsigned int)&g_bounce_mult; i += 4;
    buf[i++] = 0xD9; buf[i++] = 0x9E;
    *(unsigned int*)(buf + i) = 0x2C8; i += 4;

    /* Clear g_wall_hit */
    buf[i++] = 0xC7; buf[i++] = 0x05;
    *(unsigned int*)(buf + i) = (unsigned int)&g_wall_hit; i += 4;
    *(unsigned int*)(buf + i) = 0; i += 4;

    /* skip target: original instructions */
    int skip_pos = i;
    cave_bounceapply[jz1_pos] = (unsigned char)(skip_pos - (jz1_pos + 1));
    cave_bounceapply[jz2_pos] = (unsigned char)(skip_pos - (jz2_pos + 1));

    /* Original: FLD [EDI] */
    buf[i++] = 0xD9; buf[i++] = 0x07;
    /* Original: FADD [ESI+0x164] */
    buf[i++] = 0xD8; buf[i++] = 0x86;
    *(unsigned int*)(buf + i) = 0x164; i += 4;

    /* JMP 0x407CE8 (LEA EDX,[ESP+0xA4]) */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x407CE8 - ((unsigned int)cave_bounceapply + i + 4);
    i += 4;

    /* Overwrite 0x407CE0 with JMP (5 bytes) + 3 NOPs */
    WriteJump((void*)0x407CE0, cave_bounceapply);
    WriteNops((void*)0x407CE5, 3);
}

/* ── Key polling: silent toggle, no popups ── */
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
        }
        prev_f8 = f8_down;

        BYTE f9_down = keys[DIK_F9] & 0x80;
        if (f9_down && !prev_f9) {
            g_mult_index = (g_mult_index + 1) % NUM_MULTS;
            g_bounce_mult = BOUNCE_MULTS[g_mult_index];
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
        MessageBoxA(NULL, "Hook 1: byte mismatch at 0x407300", "Pinball v5", MB_OK | MB_ICONERROR);
        return 0;
    }
    if (!VerifyBytes((void*)0x407CE0, orig_bounceapply, 8)) {
        MessageBoxA(NULL, "Hook 2: byte mismatch at 0x407CE0", "Pinball v5", MB_OK | MB_ICONERROR);
        return 0;
    }

    BuildCaveTypeCheck();
    BuildCaveBounceApply();
    CreateThread(NULL, 0, KeyPollThread, NULL, 0, NULL);
    return 0;
}

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInst);
        CreateThread(NULL, 0, InstallPatches, NULL, 0, NULL);
    }
    return TRUE;
}
