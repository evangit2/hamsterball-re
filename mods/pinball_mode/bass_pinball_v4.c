// ============================================================
// Hamsterball Pinball Mode - bass.dll proxy (v4)
//
// FIXES over v3:
//   - Modifies velocity vector (+0xC98/+0xC9C/+0xCA0) instead of
//     normals (+0xC8C/+0xC90/+0xC94). v3 multiplied normals by
//     speed which corrupted them for subsequent collisions AND
//     had no effect on actual ball bounce velocity.
//   - Removes MessageBoxA popups (user: "rly annnoying").
//     F8 now silently toggles, F9 silently cycles multiplier.
//   - Also multiplies speed scalar (+0xC64) for consistency.
//
// Uses lazy loading: bass_real.dll is loaded on first BASS call,
// NOT in DllMain (avoids loader lock deadlock on real Windows).
// If bass_real.dll is missing, falls back to stubs (no audio, no crash).
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
 *
 * Hook 1 (0x407300): detect wall collision type==2, set flag
 * Hook 2 (0x407BB4): amplify bounce VELOCITY VECTOR by multiplier
 *
 * Physics object layout (at ball+0x1A4):
 *   +0xC64 = speed scalar (bounce magnitude)
 *   +0xC8C = collision normal X (unit vector)  -- DO NOT MODIFY
 *   +0xC90 = collision normal Y                -- DO NOT MODIFY
 *   +0xC94 = collision normal Z                -- DO NOT MODIFY
 *   +0xC98 = velocity X = speed * normal_X     -- MODIFY THIS
 *   +0xC9C = velocity Y = speed * normal_Y     -- MODIFY THIS
 *   +0xCA0 = velocity Z = speed * normal_Z     -- MODIFY THIS
 *
 * The velocity vector is computed BEFORE 0x407BB4 as speed*normal.
 * The collision callback at 0x407BBC uses these values.
 * v3 bug: modified normals instead of velocity → no effect on bounce.
 * ════════════════════════════════════════════════════════════════════ */

static const float BOUNCE_MULTS[] = {2.0f, 3.0f, 5.0f, 10.0f};
static const int NUM_MULTS = 4;

static volatile DWORD g_pinball_enabled = 0;
static volatile DWORD g_wall_hit = 0;
static volatile float g_bounce_mult = 3.0f;
static volatile int g_mult_index = 1;

/* Original bytes at hook points */
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

/* ── Hook 1: Wall collision type check at 0x407300 ──
 *
 * Original: MOV EAX,[EDI]; CMP EAX,2; JZ 0x40730C
 * Cave: same instructions + set g_wall_hit=1 if type==2
 * Then POPFD to restore flags for the original JZ, JMP back to 0x407305
 */
static void BuildCaveTypeCheck() {
    cave_typecheck = (unsigned char*)VirtualAlloc(NULL, 256, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_typecheck, 0x90, 256);
    unsigned char* buf = cave_typecheck;
    int i = 0;

    /* Original instructions */
    buf[i++] = 0x8B; buf[i++] = 0x07;                    /* MOV EAX, [EDI] */
    buf[i++] = 0x83; buf[i++] = 0xF8; buf[i++] = 0x02;   /* CMP EAX, 2 */

    /* Save flags (from CMP EAX,2 above) */
    buf[i++] = 0x9C;                                       /* PUSHFD */

    /* Re-compare to set g_wall_hit (flags already saved) */
    buf[i++] = 0x83; buf[i++] = 0xF8; buf[i++] = 0x02;   /* CMP EAX, 2 */
    buf[i++] = 0x75; buf[i++] = 0x00;                     /* JNZ skip */
    int jnz_pos = i - 1;

    /* MOV g_wall_hit, 1 */
    buf[i++] = 0xC7; buf[i++] = 0x05;
    *(unsigned int*)(buf + i) = (unsigned int)&g_wall_hit; i += 4;
    *(unsigned int*)(buf + i) = 1; i += 4;

    int skip_pos = i;
    cave_typecheck[jnz_pos] = (unsigned char)(skip_pos - (jnz_pos + 1));

    /* Restore flags (from original CMP EAX,2) */
    buf[i++] = 0x9D;                                       /* POPFD */

    /* Jump back to 0x407305 (original JZ instruction) */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x407305 - ((unsigned int)cave_typecheck + i + 4);
    i += 4;

    WriteJump((void*)0x407300, cave_typecheck);
}

/* ── Hook 2: Amplify bounce velocity at 0x407BB4 ──
 *
 * Original: MOV ECX,[ESP+0x1C]; MOV EDX,[ECX]; PUSH 1; CALL [EDX]
 *
 * Cave checks g_pinball_enabled + g_wall_hit, then multiplies:
 *   physics_obj+0xC64 (speed)       by bounce_mult
 *   physics_obj+0xC98 (velocity_X)  by bounce_mult
 *   physics_obj+0xC9C (velocity_Y)  by bounce_mult
 *   physics_obj+0xCA0 (velocity_Z)  by bounce_mult
 * Then clears g_wall_hit and executes original instructions.
 *
 * ESI = ball struct pointer (preserved by cave)
 * EAX = temp register (clobbered, but original code doesn't need it until
 *       0x407BC6 where it's overwritten by MOV EAX,[ECX])
 * FPU stack: each FLD+FMUL+FSTP is balanced (push, multiply, store+pop)
 *
 * Encoding reference:
 *   D9 80 xx xx xx xx  = FLD [EAX+disp32]      (push to FPU)
 *   D8 0D xx xx xx xx  = FMUL [disp32]         (multiply st0 by memory)
 *   D9 98 xx xx xx xx  = FSTP [EAX+disp32]     (store+pop from FPU)
 */
static void BuildCavePhase15() {
    cave_phase15 = (unsigned char*)VirtualAlloc(NULL, 512, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_phase15, 0x90, 512);
    unsigned char* buf = cave_phase15;
    int i = 0;

    /* Check g_pinball_enabled, if 0 skip to original code */
    buf[i++] = 0x83; buf[i++] = 0x3D;                    /* CMP dword ptr [addr], 0 */
    *(unsigned int*)(buf + i) = (unsigned int)&g_pinball_enabled; i += 4;
    buf[i++] = 0x00;
    buf[i++] = 0x74; buf[i++] = 0x00;                    /* JZ skip */
    int jz1_pos = i - 1;

    /* Check g_wall_hit, if 0 skip to original code */
    buf[i++] = 0x83; buf[i++] = 0x3D;
    *(unsigned int*)(buf + i) = (unsigned int)&g_wall_hit; i += 4;
    buf[i++] = 0x00;
    buf[i++] = 0x74; buf[i++] = 0x00;                    /* JZ skip */
    int jz2_pos = i - 1;

    /* Load physics_obj pointer: MOV EAX, [ESI+0x1A4] */
    buf[i++] = 0x8B; buf[i++] = 0x86;
    *(unsigned int*)(buf + i) = 0x1A4; i += 4;

    /* --- Multiply speed scalar at +0xC64 --- */
    /* FLD [EAX+0xC64] */
    buf[i++] = 0xD9; buf[i++] = 0x80;
    *(unsigned int*)(buf + i) = 0xC64; i += 4;
    /* FMUL [g_bounce_mult] */
    buf[i++] = 0xD8; buf[i++] = 0x0D;
    *(unsigned int*)(buf + i) = (unsigned int)&g_bounce_mult; i += 4;
    /* FSTP [EAX+0xC64] */
    buf[i++] = 0xD9; buf[i++] = 0x98;
    *(unsigned int*)(buf + i) = 0xC64; i += 4;

    /* --- Multiply velocity_X at +0xC98 --- */
    buf[i++] = 0xD9; buf[i++] = 0x80;
    *(unsigned int*)(buf + i) = 0xC98; i += 4;
    buf[i++] = 0xD8; buf[i++] = 0x0D;
    *(unsigned int*)(buf + i) = (unsigned int)&g_bounce_mult; i += 4;
    buf[i++] = 0xD9; buf[i++] = 0x98;
    *(unsigned int*)(buf + i) = 0xC98; i += 4;

    /* --- Multiply velocity_Y at +0xC9C --- */
    buf[i++] = 0xD9; buf[i++] = 0x80;
    *(unsigned int*)(buf + i) = 0xC9C; i += 4;
    buf[i++] = 0xD8; buf[i++] = 0x0D;
    *(unsigned int*)(buf + i) = (unsigned int)&g_bounce_mult; i += 4;
    buf[i++] = 0xD9; buf[i++] = 0x98;
    *(unsigned int*)(buf + i) = 0xC9C; i += 4;

    /* --- Multiply velocity_Z at +0xCA0 --- */
    buf[i++] = 0xD9; buf[i++] = 0x80;
    *(unsigned int*)(buf + i) = 0xCA0; i += 4;
    buf[i++] = 0xD8; buf[i++] = 0x0D;
    *(unsigned int*)(buf + i) = (unsigned int)&g_bounce_mult; i += 4;
    buf[i++] = 0xD9; buf[i++] = 0x98;
    *(unsigned int*)(buf + i) = 0xCA0; i += 4;

    /* Clear g_wall_hit flag */
    buf[i++] = 0xC7; buf[i++] = 0x05;
    *(unsigned int*)(buf + i) = (unsigned int)&g_wall_hit; i += 4;
    *(unsigned int*)(buf + i) = 0; i += 4;

    /* skip target: original instructions */
    int skip_pos = i;
    cave_phase15[jz1_pos] = (unsigned char)(skip_pos - (jz1_pos + 1));
    cave_phase15[jz2_pos] = (unsigned char)(skip_pos - (jz2_pos + 1));

    /* Original: MOV ECX, [ESP+0x1C] */
    buf[i++] = 0x8B; buf[i++] = 0x4C; buf[i++] = 0x24; buf[i++] = 0x1C;
    /* Original: MOV EDX, [ECX] */
    buf[i++] = 0x8B; buf[i++] = 0x11;

    /* Jump to 0x407BBA (PUSH 1; CALL [EDX]) */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x407BBA - ((unsigned int)cave_phase15 + i + 4);
    i += 4;

    WriteJump((void*)0x407BB4, cave_phase15);
    WriteNops((void*)0x407BB9, 1);
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
        CreateThread(NULL, 0, InstallPatches, NULL, 0, NULL);
    }
    return TRUE;
}
