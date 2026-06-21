/*
 * jump_mod.c — BASS.dll proxy that lets Player 1 jump with the spacebar.
 *
 * v5: Stripped down to MINIMAL to diagnose why spacebar doesn't work.
 * - Removed type-5 ground detection hook (Cave 1) entirely
 * - Removed g_on_ground check
 * - Removed air momentum injection
 * - Jump is now: FADD 20.0f to ball+0x168 (position Y) — crude but visible
 * - No ground detection: unlimited jumps while space held (rising edge only)
 * - This version PROVES the code cave and keyboard reading work
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
 * Jump Mod v5 — Minimal diagnostic version
 * ═══════════════════════════════════════════════════════════════════════════ */

#define G_APP_ADDR          0x005341E0
#define BALL_UPDATE_HOOK    0x00407BB4
#define HOOK_ORIG_BYTES     6

/* Ball struct offsets */
#define BALL_POS_Y          0x168
#define BALL_PLAYER_IDX     0x018
#define BALL_FALL_MODE      0xC4C

/* App/Input offsets */
#define APP_INPUT_HANDLER   0x180
#define IH_KEYBOARD_DEV     0x434
#define KBD_KEY_BUFFER      0x00C
#define DIK_SPACE           0x039

/* Jump nudge: add this to position Y on each rising-edge space press */
static float g_jump_nudge = 20.0f;

/* Edge detection */
static BYTE g_space_was_down = 0;

/* Debug counter — if this increments, the cave is working */
static volatile DWORD g_jump_count = 0;

/* ─── Helper: emit near JNZ (0F 85 + rel32) ─── */
static int emit_jnz_near(BYTE *cave, int p) {
    cave[p]   = 0x0F;
    cave[p+1] = 0x85;
    int disp_offset = p + 2;
    *(DWORD*)(cave + p + 2) = 0x12345678;
    return disp_offset;
}

/* ─── Helper: emit near JZ (0F 84 + rel32) ─── */
static int emit_jz_near(BYTE *cave, int p) {
    cave[p]   = 0x0F;
    cave[p+1] = 0x84;
    int disp_offset = p + 2;
    *(DWORD*)(cave + p + 2) = 0x12345678;
    return disp_offset;
}

/* ─── Helper: fix up near-jump displacement ─── */
static void fixup_near_jump(BYTE *cave, int disp_offset, int target_offset) {
    DWORD disp = (DWORD)(target_offset - (disp_offset + 4));
    *(DWORD*)(cave + disp_offset) = disp;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Code Cave — MINIMAL version
 *
 * At entry: ESI = ball pointer
 *
 * Flow:
 * 1. PUSH EAX, EDI
 * 2. Execute original 6 bytes (MOV ECX,[ESP+0x24]; MOV EDX,[ECX])
 * 3. Check player_index == 0
 * 4. Check fall_mode == 0
 * 5. Read keyboard: App → InputHandler → KeyboardDevice → DIK_SPACE
 * 6. Edge detect space
 * 7. On rising edge: FADD 20.0f to ball+0x168 (position Y nudge)
 * 8. POP EDI, EAX
 * 9. JMP back to hook_addr + 6
 * ═══════════════════════════════════════════════════════════════════════════ */
static void install_hook(void)
{
    BYTE *hook_addr = (BYTE*)BALL_UPDATE_HOOK;

    BYTE *cave = (BYTE*)VirtualAlloc(NULL, 512, MEM_COMMIT | MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE);
    if (!cave) return;

    DWORD jmp_to_cave = (DWORD)(cave - hook_addr - 5);
    int p = 0;

    /* PUSH EAX, EDI — save clobbered registers */
    cave[p++] = 0x50;  /* PUSH EAX */
    cave[p++] = 0x57;  /* PUSH EDI */

    /* Execute original 6 bytes with ESP+8 offset */
    cave[p++] = 0x8B; cave[p++] = 0x4C; cave[p++] = 0x24;
    cave[p++] = 0x24;  /* 0x1C + 8 = 0x24 */
    cave[p++] = 0x8B; cave[p++] = 0x11;

    /* ═══ CHECK: player_index == 0 ═══ */
    cave[p++] = 0x8B; cave[p++] = 0x86;
    cave[p++] = 0x18; cave[p++] = 0x00; cave[p++] = 0x00; cave[p++] = 0x00;
    cave[p++] = 0x85; cave[p++] = 0xC0;
    int jnz_player = emit_jnz_near(cave, p); p += 6;

    /* ═══ CHECK: fall_mode == 0 (byte) ═══ */
    cave[p++] = 0x8A; cave[p++] = 0x86;
    cave[p++] = 0x4C; cave[p++] = 0x0C; cave[p++] = 0x00; cave[p++] = 0x00;
    cave[p++] = 0x84; cave[p++] = 0xC0;
    int jnz_fall = emit_jnz_near(cave, p); p += 6;

    /* ═══ Read keyboard: App pointer ═══ */
    cave[p++] = 0xA1;
    *(DWORD*)(cave + p) = G_APP_ADDR; p += 4;
    cave[p++] = 0x85; cave[p++] = 0xC0;
    int jz_app = emit_jz_near(cave, p); p += 6;

    /* ═══ InputHandler = App+0x180 ═══ */
    cave[p++] = 0x8B; cave[p++] = 0xB8;
    *(DWORD*)(cave + p) = APP_INPUT_HANDLER; p += 4;
    cave[p++] = 0x85; cave[p++] = 0xFF;
    int jz_ih = emit_jz_near(cave, p); p += 6;

    /* ═══ KeyboardDevice = InputHandler+0x434 ═══ */
    cave[p++] = 0x8B; cave[p++] = 0xBF;
    *(DWORD*)(cave + p) = IH_KEYBOARD_DEV; p += 4;
    cave[p++] = 0x85; cave[p++] = 0xFF;
    int jz_kbd = emit_jz_near(cave, p); p += 6;

    /* ═══ DIK_SPACE state ═══ */
    cave[p++] = 0x8A; cave[p++] = 0x87;
    *(DWORD*)(cave + p) = (KBD_KEY_BUFFER + DIK_SPACE); p += 4;
    cave[p++] = 0xA8; cave[p++] = 0x80;  /* TEST AL, 0x80 */
    /* JZ .not_pressed (short) */
    cave[p++] = 0x74; cave[p++] = 0x00;
    int jz_notpressed = p - 1;

    /* ═══ Edge detect: CMP [g_space_was_down], 0 ═══ */
    cave[p++] = 0x80; cave[p++] = 0x3D;
    *(DWORD*)(cave + p) = (DWORD)&g_space_was_down; p += 4;
    cave[p++] = 0x00;
    /* JNE .done (short) */
    cave[p++] = 0x75; cave[p++] = 0x00;
    int jne_already = p - 1;

    /* ═══ RISING EDGE! FADD 20.0f to ball+0x168 (pos Y) ═══ */
    cave[p++] = 0xD9; cave[p++] = 0x86;
    *(DWORD*)(cave + p) = BALL_POS_Y; p += 4;   /* FLD [ESI+0x168] */
    cave[p++] = 0xD8; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_jump_nudge; p += 4;  /* FADD [g_jump_nudge] */
    cave[p++] = 0xD9; cave[p++] = 0x9E;
    *(DWORD*)(cave + p) = BALL_POS_Y; p += 4;   /* FSTP [ESI+0x168] */

    /* g_space_was_down = 1 */
    cave[p++] = 0xC6; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_space_was_down; p += 4;
    cave[p++] = 0x01;

    /* g_jump_count++ */
    cave[p++] = 0xA1;
    *(DWORD*)(cave + p) = (DWORD)&g_jump_count; p += 4;
    cave[p++] = 0x40;  /* INC EAX */
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

    /* POP EDI, EAX */
    cave[p++] = 0x5F;
    cave[p++] = 0x58;

    /* JMP back to hook_addr + 6 */
    cave[p++] = 0xE9;
    *(DWORD*)(cave + p) = (DWORD)(hook_addr + HOOK_ORIG_BYTES) - (DWORD)(cave + p + 4);
    p += 4;

    /* ═══ Fix up jumps ═══ */
    fixup_near_jump(cave, jnz_player, done_label);
    fixup_near_jump(cave, jnz_fall, done_label);
    fixup_near_jump(cave, jz_app, done_label);
    fixup_near_jump(cave, jz_ih, done_label);
    fixup_near_jump(cave, jz_kbd, done_label);

    cave[jz_notpressed] = (BYTE)(not_pressed_label - (jz_notpressed + 1));
    cave[jne_already]   = (BYTE)(done_label - (jne_already + 1));
    cave[jmp_done]      = (BYTE)(done_label - (jmp_done + 1));

    /* Patch hook site */
    DWORD old_protect;
    VirtualProtect(hook_addr, HOOK_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_to_cave;
    hook_addr[5] = 0x90;
    VirtualProtect(hook_addr, HOOK_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, HOOK_ORIG_BYTES);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI patch_thread(LPVOID param)
{
    (void)param;
    Sleep(5000);

    BYTE *hook = (BYTE*)BALL_UPDATE_HOOK;
    BYTE expected[] = { 0x8B, 0x4C, 0x24, 0x1C, 0x8B, 0x11 };
    if (memcmp(hook, expected, 6) != 0) return 1;

    install_hook();
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
