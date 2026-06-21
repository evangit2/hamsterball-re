/*
 * jump_mod.c — BASS.dll proxy that lets Player 1 jump with the spacebar.
 *
 * v3: Fixed byte-sized jump overflow that caused NULL dereference crashes.
 * Short conditional jumps (74/75 xx) can only reach ±127 bytes. When the
 * .done label was >127 bytes from early checks, the displacement truncated,
 * sending execution to uninitialized memory (zeros) → crash at 0000:00000000.
 * Fix: use 32-bit near jumps (0F 84/0F 85 + rel32) for distant targets.
 *
 * Ground detection: collision type-5 (floor) result, no background thread.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll jump_mod.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
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
 * Jump Mod v3 — Fixed jump overflow + collision ground detection
 * ═══════════════════════════════════════════════════════════════════════════ */

#define G_APP_ADDR          0x005341E0

/* Hook 1: Type-5 floor collision handler at 0x407391 */
#define TYPE5_HOOK          0x00407391
#define TYPE5_ORIG_BYTES    7

/* Hook 2: Phase 15 vtable call at 0x407BB4 */
#define BALL_UPDATE_HOOK    0x00407BB4
#define HOOK2_ORIG_BYTES    6

/* Ball struct offsets */
#define BALL_VEL_X          0x170
#define BALL_VEL_Y          0x174
#define BALL_VEL_Z          0x178
#define BALL_PLAYER_IDX     0x018
#define BALL_FALL_MODE      0xC4C

/* App/Input offsets */
#define APP_INPUT_HANDLER   0x180
#define IH_KEYBOARD_DEV     0x434
#define KBD_KEY_BUFFER      0x00C
#define DIK_SPACE           0x039

/* Jump velocity */
static float g_jump_vel = 500.0f;

/* Shared state */
static volatile DWORD g_on_ground = 0;
static BYTE g_space_was_down = 0;
static volatile float g_air_vel_x = 0.0f;
static volatile float g_air_vel_z = 0.0f;
static volatile DWORD g_is_airborne = 0;
static volatile DWORD g_jump_count = 0;

/* ─── Helper: emit a near JZ (0F 84 + rel32) with placeholder ───
 * Writes 6 bytes starting at cave[p]. Does NOT advance p.
 * Returns offset of 4-byte displacement for fixup_near_jump. */
static int emit_jz_near(BYTE *cave, int p) {
    cave[p]   = 0x0F;
    cave[p+1] = 0x84;
    int disp_offset = p + 2;
    *(DWORD*)(cave + p + 2) = 0x12345678;  /* placeholder */
    return disp_offset;
}

/* ─── Helper: emit a near JNZ (0F 85 + rel32) with placeholder ───
 * Writes 6 bytes starting at cave[p]. Does NOT advance p.
 * Returns offset of 4-byte displacement for fixup_near_jump. */
static int emit_jnz_near(BYTE *cave, int p) {
    cave[p]   = 0x0F;
    cave[p+1] = 0x85;
    int disp_offset = p + 2;
    *(DWORD*)(cave + p + 2) = 0x12345678;  /* placeholder */
    return disp_offset;
}

/* ─── Helper: fix up a near-jump placeholder ─── */
static void fixup_near_jump(BYTE *cave, int placeholder_offset, int target_offset) {
    /* The 4-byte displacement is at placeholder_offset.
     * Displacement = target - (placeholder_offset + 4) */
    DWORD disp = (DWORD)(target_offset - (placeholder_offset + 4));
    *(DWORD*)(cave + placeholder_offset) = disp;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Cave 1 — Type-5 Floor Detection (at 0x407391)
 * ═══════════════════════════════════════════════════════════════════════════ */
static void install_type5_hook(void)
{
    BYTE *hook_addr = (BYTE*)TYPE5_HOOK;
    BYTE expected[] = { 0xC6, 0x86, 0xE9, 0x02, 0x00, 0x00, 0x01 };
    if (memcmp(hook_addr, expected, 7) != 0) return;

    BYTE *cave = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE);
    if (!cave) return;

    DWORD jmp_to_cave = (DWORD)(cave - hook_addr - 5);
    int p = 0;

    /* Original 7 bytes: MOV BYTE [ESI+0x2E9], 1 */
    cave[p++] = 0xC6; cave[p++] = 0x86;
    cave[p++] = 0xE9; cave[p++] = 0x02; cave[p++] = 0x00; cave[p++] = 0x00;
    cave[p++] = 0x01;

    /* Set g_on_ground = 1 */
    cave[p++] = 0xC7; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_on_ground; p += 4;
    cave[p++] = 0x01; cave[p++] = 0x00; cave[p++] = 0x00; cave[p++] = 0x00;

    /* JMP back to 0x407398 */
    cave[p++] = 0xE9;
    *(DWORD*)(cave + p) = (DWORD)(hook_addr + TYPE5_ORIG_BYTES) - (DWORD)(cave + p + 4);
    p += 4;

    /* Patch hook site */
    DWORD old_protect;
    VirtualProtect(hook_addr, TYPE5_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_to_cave;
    hook_addr[5] = 0x90;
    hook_addr[6] = 0x90;
    VirtualProtect(hook_addr, TYPE5_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, TYPE5_ORIG_BYTES);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Cave 2 — Jump Logic (at 0x407BB4, Phase 15)
 *
 * Uses NEAR jumps (0F 84/0F 85 + rel32) for ALL conditional branches
 * to avoid the ±127 byte overflow that crashed v2.
 * ═══════════════════════════════════════════════════════════════════════════ */
static void install_jump_hook(void)
{
    BYTE *hook_addr = (BYTE*)BALL_UPDATE_HOOK;

    BYTE *cave = (BYTE*)VirtualAlloc(NULL, 512, MEM_COMMIT | MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE);
    if (!cave) return;

    DWORD jmp_to_cave = (DWORD)(cave - hook_addr - 5);
    int p = 0;

    /* PUSH EAX, EDI */
    cave[p++] = 0x50;
    cave[p++] = 0x57;

    /* Original 6 bytes with ESP+8 offset */
    cave[p++] = 0x8B; cave[p++] = 0x4C; cave[p++] = 0x24;
    cave[p++] = 0x24;
    cave[p++] = 0x8B; cave[p++] = 0x11;

    /* ═══ CHECK: player_index == 0 ═══ */
    cave[p++] = 0x8B; cave[p++] = 0x86;
    cave[p++] = 0x18; cave[p++] = 0x00; cave[p++] = 0x00; cave[p++] = 0x00;
    cave[p++] = 0x85; cave[p++] = 0xC0;
    /* JNZ .done (near, 6 bytes) */
    int jnz_player = emit_jnz_near(cave, p);
    p += 6;

    /* ═══ CHECK: fall_mode == 0 ═══ */
    cave[p++] = 0x8A; cave[p++] = 0x86;
    cave[p++] = 0x4C; cave[p++] = 0x0C; cave[p++] = 0x00; cave[p++] = 0x00;
    cave[p++] = 0x84; cave[p++] = 0xC0;
    /* JNZ .done (near, 6 bytes) */
    int jnz_fallmode = emit_jnz_near(cave, p);
    p += 6;

    /* ═══ AIR MOMENTUM INJECTION ═══ */
    cave[p++] = 0xA1;
    *(DWORD*)(cave + p) = (DWORD)&g_is_airborne; p += 4;
    cave[p++] = 0x85; cave[p++] = 0xC0;
    /* JZ .check_ground (short — only 36 bytes, within range) */
    cave[p++] = 0x74; cave[p++] = 0x00;
    int jz_not_airborne = p - 1;

    /* FLD [g_air_vel_x] → FADD [ESI+0x170] → FSTP [ESI+0x170] */
    cave[p++] = 0xD9; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_air_vel_x; p += 4;
    cave[p++] = 0xD8; cave[p++] = 0x86;
    *(DWORD*)(cave + p) = BALL_VEL_X; p += 4;
    cave[p++] = 0xD9; cave[p++] = 0x9E;
    *(DWORD*)(cave + p) = BALL_VEL_X; p += 4;

    /* FLD [g_air_vel_z] → FADD [ESI+0x178] → FSTP [ESI+0x178] */
    cave[p++] = 0xD9; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_air_vel_z; p += 4;
    cave[p++] = 0xD8; cave[p++] = 0x86;
    *(DWORD*)(cave + p) = BALL_VEL_Z; p += 4;
    cave[p++] = 0xD9; cave[p++] = 0x9E;
    *(DWORD*)(cave + p) = BALL_VEL_Z; p += 4;

    /* .check_ground: */
    int check_ground_label = p;

    /* ═══ CHECK: g_on_ground ═══ */
    cave[p++] = 0xA1;
    *(DWORD*)(cave + p) = (DWORD)&g_on_ground; p += 4;
    cave[p++] = 0x85; cave[p++] = 0xC0;
    /* JZ .done (near, 6 bytes) */
    int jz_not_grounded = emit_jz_near(cave, p);
    p += 6;

    /* ═══ Grounded: clear airborne flag ═══ */
    cave[p++] = 0xC7; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_is_airborne; p += 4;
    cave[p++] = 0x00; cave[p++] = 0x00; cave[p++] = 0x00; cave[p++] = 0x00;

    /* ═══ Read keyboard ═══ */
    cave[p++] = 0xA1;
    *(DWORD*)(cave + p) = G_APP_ADDR; p += 4;
    cave[p++] = 0x85; cave[p++] = 0xC0;
    /* JZ .done (near, 6 bytes) */
    int jz_app = emit_jz_near(cave, p);
    p += 6;

    cave[p++] = 0x8B; cave[p++] = 0xB8;
    *(DWORD*)(cave + p) = APP_INPUT_HANDLER; p += 4;
    cave[p++] = 0x85; cave[p++] = 0xFF;
    /* JZ .done (near, 6 bytes) */
    int jz_ih = emit_jz_near(cave, p);
    p += 6;

    cave[p++] = 0x8B; cave[p++] = 0xBF;
    *(DWORD*)(cave + p) = IH_KEYBOARD_DEV; p += 4;
    cave[p++] = 0x85; cave[p++] = 0xFF;
    /* JZ .done (near, 6 bytes) */
    int jz_kbd = emit_jz_near(cave, p);
    p += 6;

    cave[p++] = 0x8A; cave[p++] = 0x87;
    *(DWORD*)(cave + p) = (KBD_KEY_BUFFER + DIK_SPACE); p += 4;
    cave[p++] = 0xA8; cave[p++] = 0x80;
    /* JZ .not_pressed (short — within range) */
    cave[p++] = 0x74; cave[p++] = 0x00;
    int jz_notpressed = p - 1;

    /* Edge detect */
    cave[p++] = 0x80; cave[p++] = 0x3D;
    *(DWORD*)(cave + p) = (DWORD)&g_space_was_down; p += 4;
    cave[p++] = 0x00;
    /* JNE .done (short — within range after keyboard checks) */
    cave[p++] = 0x75; cave[p++] = 0x00;
    int jne_already = p - 1;

    /* ═══ Rising edge! FADD jump impulse ═══ */
    cave[p++] = 0xD9; cave[p++] = 0x86;
    *(DWORD*)(cave + p) = BALL_VEL_Y; p += 4;
    cave[p++] = 0xD8; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_jump_vel; p += 4;
    cave[p++] = 0xD9; cave[p++] = 0x9E;
    *(DWORD*)(cave + p) = BALL_VEL_Y; p += 4;

    /* Save horizontal velocity */
    cave[p++] = 0x8B; cave[p++] = 0x86;
    *(DWORD*)(cave + p) = BALL_VEL_X; p += 4;
    cave[p++] = 0xA3;
    *(DWORD*)(cave + p) = (DWORD)&g_air_vel_x; p += 4;
    cave[p++] = 0x8B; cave[p++] = 0x86;
    *(DWORD*)(cave + p) = BALL_VEL_Z; p += 4;
    cave[p++] = 0xA3;
    *(DWORD*)(cave + p) = (DWORD)&g_air_vel_z; p += 4;
    cave[p++] = 0xC7; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_is_airborne; p += 4;
    cave[p++] = 0x01; cave[p++] = 0x00; cave[p++] = 0x00; cave[p++] = 0x00;

    cave[p++] = 0xC6; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_space_was_down; p += 4;
    cave[p++] = 0x01;

    cave[p++] = 0xA1;
    *(DWORD*)(cave + p) = (DWORD)&g_jump_count; p += 4;
    cave[p++] = 0x40;
    cave[p++] = 0xA3;
    *(DWORD*)(cave + p) = (DWORD)&g_jump_count; p += 4;

    /* JMP .done (short) */
    cave[p++] = 0xEB; cave[p++] = 0x00;
    int jmp_done = p - 1;

    /* .not_pressed: */
    int not_pressed_label = p;
    cave[p++] = 0xC6; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_space_was_down; p += 4;
    cave[p++] = 0x00;

    /* .done: */
    int done_label = p;

    /* Clear g_on_ground for next frame */
    cave[p++] = 0xC7; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_on_ground; p += 4;
    cave[p++] = 0x00; cave[p++] = 0x00; cave[p++] = 0x00; cave[p++] = 0x00;

    /* POP EDI, EAX */
    cave[p++] = 0x5F;
    cave[p++] = 0x58;

    /* JMP back to hook_addr + 6 */
    cave[p++] = 0xE9;
    *(DWORD*)(cave + p) = (DWORD)(hook_addr + HOOK2_ORIG_BYTES) - (DWORD)(cave + p + 4);
    p += 4;

    /* ═══ Fix up ALL jump placeholders ═══ */

    /* Near jumps: fixup with displacement offsets returned by emit_*. */
    fixup_near_jump(cave, jnz_player, done_label);
    fixup_near_jump(cave, jnz_fallmode, done_label);
    fixup_near_jump(cave, jz_not_grounded, done_label);
    fixup_near_jump(cave, jz_app, done_label);
    fixup_near_jump(cave, jz_ih, done_label);
    fixup_near_jump(cave, jz_kbd, done_label);

    /* Short jumps (74/75/EB + rel8): displacement byte is at (position - 1) */
    cave[jz_not_airborne] = (BYTE)(check_ground_label - (jz_not_airborne + 1));
    cave[jz_notpressed]   = (BYTE)(not_pressed_label - (jz_notpressed + 1));
    cave[jne_already]     = (BYTE)(done_label - (jne_already + 1));
    cave[jmp_done]        = (BYTE)(done_label - (jmp_done + 1));

    /* Patch hook site */
    DWORD old_protect;
    VirtualProtect(hook_addr, HOOK2_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_to_cave;
    hook_addr[5] = 0x90;
    VirtualProtect(hook_addr, HOOK2_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, HOOK2_ORIG_BYTES);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI patch_thread(LPVOID param)
{
    (void)param;
    Sleep(5000);

    BYTE *hook1 = (BYTE*)TYPE5_HOOK;
    BYTE exp1[] = { 0xC6, 0x86, 0xE9, 0x02, 0x00, 0x00, 0x01 };
    if (memcmp(hook1, exp1, 7) != 0) return 1;

    BYTE *hook2 = (BYTE*)BALL_UPDATE_HOOK;
    BYTE exp2[] = { 0x8B, 0x4C, 0x24, 0x1C, 0x8B, 0x11 };
    if (memcmp(hook2, exp2, 6) != 0) return 1;

    install_type5_hook();
    install_jump_hook();
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
        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
