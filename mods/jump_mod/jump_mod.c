/*
 * jump_mod.c — BASS.dll proxy that lets Player 1 jump with the spacebar.
 *
 * APPROACH: Binary hook (code cave) at the end of Ball_Update (0x004082B6).
 * The cave runs AFTER Ball_Update's physics + collision detection completes,
 * so the jump velocity persists for one full frame before the next Ball_Update
 * zeroes it for collision.
 *
 * Keyboard reading: Uses the game's own DirectInput8 buffer.
 *   App = *(DWORD*)0x005341E0
 *   InputHandler = *(DWORD*)(App + 0x180)
 *   KeyboardDevice = *(DWORD*)(InputHandler + 0x434)
 *   DIK_SPACE state = *(BYTE*)(KeyboardDevice + 0xC + 0x39)
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll jump_mod.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports — forward all game imports to bass_real.dll
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
/* Extra stubs */
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
 * Jump Mod — Binary Hook into Ball_Update epilogue
 * ═══════════════════════════════════════════════════════════════════════════ */

#define G_APP_ADDR          0x005341E0
#define BALL_UPDATE_HOOK    0x004082B6   /* MOV [ESP+0x944],ECX — just before epilogue */
#define HOOK_ORIG_BYTES     7            /* 89 8C 24 44 09 00 00 */

/* Ball struct offsets */
#define BALL_PLAYER_IDX     0x018
#define BALL_VEL_Y          0x174
#define BALL_IS_FALLING     0x281
#define BALL_FALL_MODE      0xC4C

/* App struct offsets */
#define APP_INPUT_HANDLER   0x180

/* InputHandler offsets */
#define IH_KEYBOARD_DEV    0x434

/* KeyboardDevice offsets */
#define KBD_KEY_BUFFER     0x00C
#define DIK_SPACE          0x039

/* Jump velocity: 500.0f = 0x43FA0000 */
#define JUMP_VELOCITY_BITS 0x43FA0000

/* Edge detection state */
static BYTE g_space_was_down = 0;

/* Debug counter */
static volatile DWORD g_jump_count = 0;

/* Code cave: hand-assembled x86 machine code.
 *
 * At entry: ESI = ball pointer (this)
 *           ESP at the hook point in Ball_Update's epilogue
 *
 * The cave:
 *   1. Saves registers (EAX, ECX, EDX, EDI)
 *   2. Executes the original instruction: MOV [ESP+0x954], ECX
 *      (+0x10 offset from 4 PUSHes)
 *   3. Checks ball+0x18 == 0 (Player 1)
 *   4. Reads App → InputHandler(App+0x180) → KeyboardDevice(IH+0x434)
 *   5. Reads DIK_SPACE from KeyboardDevice+0xC+0x39
 *   6. Edge-detects key press (rising edge only)
 *   7. Checks fall_mode == 0 and is_falling == 0
 *   8. If all conditions met, sets ball+0x174 = JUMP_VELOCITY
 *   9. Restores registers
 *   10. JMP back to hook_addr + 7
 */
static void install_hook(void)
{
    BYTE *hook_addr = (BYTE*)BALL_UPDATE_HOOK;

    BYTE *cave = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE);
    if (!cave) return;

    DWORD jmp_to_cave = (DWORD)(cave - hook_addr - 5);

    int p = 0;

    /* PUSH EAX, ECX, EDX, EDI — save registers */
    cave[p++] = 0x50;  /* PUSH EAX */
    cave[p++] = 0x51;  /* PUSH ECX */
    cave[p++] = 0x52;  /* PUSH EDX */
    cave[p++] = 0x57;  /* PUSH EDI */

    /* Execute original instruction: MOV [ESP+0x954], ECX
     * Original was [ESP+0x944], but we pushed 4 regs = +0x10 */
    cave[p++] = 0x89; cave[p++] = 0x8C; cave[p++] = 0x24;
    cave[p++] = 0x54; cave[p++] = 0x09; cave[p++] = 0x00; cave[p++] = 0x00;

    /* --- Check player_index == 0 (ball+0x18) --- */
    /* MOV EAX, [ESI+0x18] */
    cave[p++] = 0x8B; cave[p++] = 0x86;
    cave[p++] = 0x18; cave[p++] = 0x00; cave[p++] = 0x00; cave[p++] = 0x00;
    /* TEST EAX, EAX */
    cave[p++] = 0x85; cave[p++] = 0xC0;
    /* JNZ .done */
    cave[p++] = 0x75; cave[p++] = 0x00;
    int jnz_player = p - 1;

    /* --- Check fall_mode == 0 (ball+0xC4C, byte) --- */
    /* MOV AL, [ESI+0xC4C] */
    cave[p++] = 0x8A; cave[p++] = 0x86;
    cave[p++] = 0x4C; cave[p++] = 0x0C; cave[p++] = 0x00; cave[p++] = 0x00;
    /* TEST AL, AL */
    cave[p++] = 0x84; cave[p++] = 0xC0;
    /* JNZ .done */
    cave[p++] = 0x75; cave[p++] = 0x00;
    int jnz_fallmode = p - 1;

    /* --- Check is_falling == 0 (ball+0x281, byte) --- */
    /* MOV AL, [ESI+0x281] */
    cave[p++] = 0x8A; cave[p++] = 0x86;
    cave[p++] = 0x81; cave[p++] = 0x02; cave[p++] = 0x00; cave[p++] = 0x00;
    /* TEST AL, AL */
    cave[p++] = 0x84; cave[p++] = 0xC0;
    /* JNZ .done */
    cave[p++] = 0x75; cave[p++] = 0x00;
    int jnz_isfalling = p - 1;

    /* --- Read App pointer: MOV EAX, [0x005341E0] --- */
    cave[p++] = 0xA1;
    *(DWORD*)(cave + p) = G_APP_ADDR; p += 4;
    /* TEST EAX, EAX */
    cave[p++] = 0x85; cave[p++] = 0xC0;
    /* JZ .done */
    cave[p++] = 0x74; cave[p++] = 0x00;
    int jz_app = p - 1;

    /* --- InputHandler = [EAX+0x180]: MOV EDI, [EAX+0x180] --- */
    cave[p++] = 0x8B; cave[p++] = 0xB8;
    *(DWORD*)(cave + p) = APP_INPUT_HANDLER; p += 4;
    /* TEST EDI, EDI */
    cave[p++] = 0x85; cave[p++] = 0xFF;
    /* JZ .done */
    cave[p++] = 0x74; cave[p++] = 0x00;
    int jz_ih = p - 1;

    /* --- KeyboardDevice = [EDI+0x434]: MOV EDI, [EDI+0x434] --- */
    cave[p++] = 0x8B; cave[p++] = 0xBF;
    *(DWORD*)(cave + p) = IH_KEYBOARD_DEV; p += 4;
    /* TEST EDI, EDI */
    cave[p++] = 0x85; cave[p++] = 0xFF;
    /* JZ .done */
    cave[p++] = 0x74; cave[p++] = 0x00;
    int jz_kbd = p - 1;

    /* --- Read DIK_SPACE: MOV AL, [EDI+0x45] (0xC+0x39=0x45) --- */
    cave[p++] = 0x8A; cave[p++] = 0x87;
    *(DWORD*)(cave + p) = (KBD_KEY_BUFFER + DIK_SPACE); p += 4;
    /* TEST AL, 0x80 */
    cave[p++] = 0xA8; cave[p++] = 0x80;
    /* JZ .not_pressed */
    cave[p++] = 0x74; cave[p++] = 0x00;
    int jz_notpressed = p - 1;

    /* --- Key IS pressed: edge detect --- */
    /* CMP byte ptr [g_space_was_down], 0 */
    cave[p++] = 0x80; cave[p++] = 0x3D;
    *(DWORD*)(cave + p) = (DWORD)&g_space_was_down; p += 4;
    cave[p++] = 0x00;
    /* JNE .done (already pressed, skip) */
    cave[p++] = 0x75; cave[p++] = 0x00;
    int jne_already = p - 1;

    /* --- Rising edge! Apply jump: MOV DWORD PTR [ESI+0x174], 0x43FA0000 --- */
    cave[p++] = 0xC7; cave[p++] = 0x86;
    *(DWORD*)(cave + p) = BALL_VEL_Y; p += 4;
    *(DWORD*)(cave + p) = JUMP_VELOCITY_BITS; p += 4;

    /* Set g_space_was_down = 1 */
    cave[p++] = 0xC6; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_space_was_down; p += 4;
    cave[p++] = 0x01;

    /* Increment g_jump_count */
    /* MOV EAX, [g_jump_count] */
    cave[p++] = 0xA1;
    *(DWORD*)(cave + p) = (DWORD)&g_jump_count; p += 4;
    /* INC EAX */
    cave[p++] = 0x40;
    /* MOV [g_jump_count], EAX */
    cave[p++] = 0xA3;
    *(DWORD*)(cave + p) = (DWORD)&g_jump_count; p += 4;

    /* JMP .done */
    cave[p++] = 0xEB; cave[p++] = 0x00;
    int jmp_done = p - 1;

    /* .not_pressed: */
    int not_pressed_label = p;
    /* Set g_space_was_down = 0 */
    cave[p++] = 0xC6; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_space_was_down; p += 4;
    cave[p++] = 0x00;

    /* .done: */
    int done_label = p;

    /* POP EDI, EDX, ECX, EAX */
    cave[p++] = 0x5F;  /* POP EDI */
    cave[p++] = 0x5A;  /* POP EDX */
    cave[p++] = 0x59;  /* POP ECX */
    cave[p++] = 0x58;  /* POP EAX */

    /* JMP back to hook_addr + 7 */
    cave[p++] = 0xE9;
    DWORD jmp_back = (DWORD)(hook_addr + HOOK_ORIG_BYTES) - (DWORD)(cave + p + 4);
    *(DWORD*)(cave + p) = jmp_back; p += 4;

    /* Fix up all placeholder jumps */
    cave[jnz_player]     = (BYTE)(done_label - (jnz_player + 1));
    cave[jnz_fallmode]   = (BYTE)(done_label - (jnz_fallmode + 1));
    cave[jnz_isfalling]  = (BYTE)(done_label - (jnz_isfalling + 1));
    cave[jz_app]         = (BYTE)(done_label - (jz_app + 1));
    cave[jz_ih]          = (BYTE)(done_label - (jz_ih + 1));
    cave[jz_kbd]         = (BYTE)(done_label - (jz_kbd + 1));
    cave[jz_notpressed]  = (BYTE)(not_pressed_label - (jz_notpressed + 1));
    cave[jne_already]    = (BYTE)(done_label - (jne_already + 1));
    cave[jmp_done]       = (BYTE)(done_label - (jmp_done + 1));

    /* Patch the hook site: JMP + 2 NOPs */
    DWORD old_protect;
    VirtualProtect(hook_addr, HOOK_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_to_cave;
    hook_addr[5] = 0x90;
    hook_addr[6] = 0x90;
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

    /* Verify hook site has expected bytes */
    BYTE *hook_addr = (BYTE*)BALL_UPDATE_HOOK;
    BYTE expected[] = { 0x89, 0x8C, 0x24, 0x44, 0x09, 0x00, 0x00 };
    if (memcmp(hook_addr, expected, 7) != 0) {
        return 1;
    }

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
