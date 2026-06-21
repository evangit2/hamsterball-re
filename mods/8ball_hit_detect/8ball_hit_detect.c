/*
 * 8ball_hit_detect.c — BASS.dll proxy that detects player→8-ball collisions.
 *
 * HOOK POINT: 0x00406FD1 inside Ball_Update (0x405E00)
 *   Original instruction: fld dword [edi+0x284]  (6 bytes: D9 87 84 02 00 00)
 *
 * At the hook point:
 *   ESI = this ball (running Ball_Update)
 *   EDI = other ball (the collision partner)
 *   We are inside the ball-ball collision response (collision type 1, past
 *   the piVar16 type check). Both balls are valid pointers.
 *
 * DETECTION:
 *   ball+0x18 = player_index:  0-3 = Player 1-4,  -1 = NPC 8-ball
 *   A "player hits 8-ball" event = one ball has player_index >= 0 and the
 *   other has player_index == -1.
 *
 * EFFECT:
 *   - Increments g_hit_count (readable via registered symbol / debugger)
 *   - Appends a line to hitlog.txt in the game directory on every hit
 *   - Uses pointer comparison (ESI < EDI) to count each collision once,
 *     since Ball_Update runs for both balls (symmetric double-fire)
 *   - Pure detection only — no gameplay changes
 *
 * BUILD:
 *   i686-w64-mingw32-gcc -shared -o bass.dll 8ball_hit_detect.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias
 *
 * INSTALL:
 *   1. Rename original bass.dll → bass_real.dll in the Hamsterball directory
 *   2. Copy this compiled bass.dll to the same directory
 *   3. Launch Hamsterball.exe
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>

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
 * 8-Ball Hit Detection — Binary Hook into Ball_Update
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Hook target: the fld dword [edi+0x284] at 0x406FD1
 * This is the start of the ball-ball collision scoring section in Ball_Update.
 * At this point: ESI = this ball, EDI = other ball, both valid.
 * Original bytes: D9 87 84 02 00 00 (6 bytes)
 */
#define HOOK_ADDR          0x00406FD1
#define HOOK_ORIG_BYTES    6
/* Original instruction: fld dword [edi+0x284] */
static const BYTE HOOK_ORIG[] = { 0xD9, 0x87, 0x84, 0x02, 0x00, 0x00 };

/* Ball struct offsets */
#define BALL_PLAYER_IDX     0x018    /* int: 0-3 = Player 1-4, -1 = NPC 8-ball */

/* Hit counter — readable via debugger or CE */
static volatile DWORD g_hit_count = 0;

/*
 * log_hit — called from the code cave when a player→8-ball collision is
 * detected. Writes a line to hitlog.txt in the game directory.
 *   idx1 = this ball's player_index (from ESI+0x18)
 *   idx2 = other ball's player_index (from EDI+0x18)
 * One is >= 0 (the player), the other is -1 (the 8-ball).
 */
static void __cdecl log_hit(int idx1, int idx2)
{
    int player = (idx1 >= 0) ? idx1 : idx2;

    char buf[160];
    int n = snprintf(buf, sizeof(buf),
        "[Hit %lu] Player %d struck an 8-ball\r\n",
        (unsigned long)g_hit_count, player + 1);

    if (n > 0) {
        HANDLE hFile = CreateFileA("hitlog.txt", FILE_APPEND_DATA,
            FILE_SHARE_READ, NULL, OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written;
            WriteFile(hFile, buf, (DWORD)n, &written, NULL);
            CloseHandle(hFile);
        }
    }
}

/*
 * Code cave layout (x86, hand-assembled):
 *
 * Entry: ESI = this ball, EDI = other ball (from Ball_Update collision loop)
 *
 *   PUSHAD                          ; save all registers
 *
 *   ; --- Read both balls' player_index ---
 *   MOV EAX, [ESI+0x18]             ; this ball's player_index
 *   MOV EBX, [EDI+0x18]             ; other ball's player_index
 *
 *   ; --- Check: is this a player→8-ball collision? ---
 *   ; Case 1: ESI is player (EAX != -1), EDI is 8-ball (EBX == -1)
 *   CMP EAX, 0xFFFFFFFF
 *   JE  .check_case2                ; this ball is 8-ball → check other
 *   CMP EBX, 0xFFFFFFFF
 *   JNE .done                       ; both players → skip
 *   ; Player (ESI) hit 8-ball (EDI) — player is the hitter
 *   JMP .hit_detected
 *
 *   .check_case2:
 *   ; ESI is 8-ball (EAX == -1), check if EDI is player (EBX != -1)
 *   CMP EBX, 0xFFFFFFFF
 *   JE  .done                       ; both 8-balls → skip
 *   ; Player (EDI) hit 8-ball (ESI) — player is the hitter
 *
 *   .hit_detected:
 *   ; --- Avoid double-counting (Ball_Update runs for both balls) ---
 *   ; Only count when ESI < EDI (lower address wins)
 *   CMP ESI, EDI
 *   JAE .done                       ; let the other Ball_Update handle it
 *
 *   ; --- Increment hit counter ---
 *   INC DWORD [g_hit_count]
 *
 *   ; --- Log to hitlog.txt ---
 *   PUSH EBX                       ; arg2 = other ball's player_index
 *   PUSH EAX                       ; arg1 = this ball's player_index
 *   CALL log_hit
 *   ADD ESP, 8                     ; cdecl cleanup
 *
 *   .done:
 *   POPAD                           ; restore all registers
 *
 *   ; --- Execute original instruction: fld dword [edi+0x284] ---
 *   DB 0xD9, 0x87, 0x84, 0x02, 0x00, 0x00
 *
 *   ; --- JMP back to HOOK_ADDR + 6 ---
 *   JMP (HOOK_ADDR + 6)
 */
static void install_hook(void)
{
    BYTE *hook_addr = (BYTE*)HOOK_ADDR;
    BYTE *cave = (BYTE*)VirtualAlloc(NULL, 512, MEM_COMMIT | MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE);
    if (!cave) return;

    DWORD jmp_to_cave = (DWORD)(cave - hook_addr - 5);

    int p = 0;

    /* PUSHAD (save all registers) */
    cave[p++] = 0x60;

    /* MOV EAX, [ESI+0x18] — this ball's player_index */
    cave[p++] = 0x8B; cave[p++] = 0x86;
    *(DWORD*)(cave + p) = BALL_PLAYER_IDX; p += 4;

    /* MOV EBX, [EDI+0x18] — other ball's player_index */
    cave[p++] = 0x8B; cave[p++] = 0x9F;
    *(DWORD*)(cave + p) = BALL_PLAYER_IDX; p += 4;

    /* CMP EAX, 0xFFFFFFFF — is this ball an 8-ball? */
    cave[p++] = 0x3D;
    *(DWORD*)(cave + p) = 0xFFFFFFFF; p += 4;
    /* JE .check_case2 */
    cave[p++] = 0x74; cave[p++] = 0x00;
    int je_check_case2 = p - 1;

    /* CMP EBX, 0xFFFFFFFF — is other ball an 8-ball? */
    cave[p++] = 0x81; cave[p++] = 0xFB;
    *(DWORD*)(cave + p) = 0xFFFFFFFF; p += 4;
    /* JNE .done — both players, not a player→8-ball hit */
    cave[p++] = 0x75; cave[p++] = 0x00;
    int jne_both_players = p - 1;

    /* JMP .hit_detected (fall through = case 1: player hit 8-ball) */
    cave[p++] = 0xEB; cave[p++] = 0x00;
    int jmp_hit1 = p - 1;

    /* .check_case2: */
    int check_case2_label = p;
    /* CMP EBX, 0xFFFFFFFF — is other ball also 8-ball? */
    cave[p++] = 0x81; cave[p++] = 0xFB;
    *(DWORD*)(cave + p) = 0xFFFFFFFF; p += 4;
    /* JE .done — both 8-balls, skip */
    cave[p++] = 0x74; cave[p++] = 0x00;
    int je_both_8balls = p - 1;

    /* .hit_detected: */
    int hit_detected_label = p;

    /* CMP ESI, EDI — avoid double-counting (only count when ESI < EDI) */
    cave[p++] = 0x39; cave[p++] = 0xFE;  /* CMP ESI, EDI */
    /* JAE .done */
    cave[p++] = 0x73; cave[p++] = 0x00;
    int jae_double = p - 1;

    /* INC DWORD [g_hit_count] */
    cave[p++] = 0xFF; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_hit_count; p += 4;

    /* --- Call log_hit(idx1, idx2) — cdecl ---
     * EAX still has this ball's player_index, EBX has other ball's.
     * PUSH EBX (arg2), PUSH EAX (arg1), CALL, ADD ESP,8 */
    /* PUSH EBX */
    cave[p++] = 0x53;
    /* PUSH EAX */
    cave[p++] = 0x50;
    /* CALL log_hit (near, relative) */
    cave[p++] = 0xE8;
    {
        DWORD call_addr = (DWORD)(cave + p + 4);
        DWORD target = (DWORD)&log_hit;
        *(DWORD*)(cave + p) = target - call_addr;
    }
    p += 4;
    /* ADD ESP, 8 — cdecl cleanup */
    cave[p++] = 0x83; cave[p++] = 0xC4; cave[p++] = 0x08;

    /* .done: */
    int done_label = p;

    /* POPAD (restore all registers) */
    cave[p++] = 0x61;

    /* Execute original instruction: fld dword [edi+0x284] */
    cave[p++] = 0xD9; cave[p++] = 0x87;
    cave[p++] = 0x84; cave[p++] = 0x02;
    cave[p++] = 0x00; cave[p++] = 0x00;

    /* JMP back to HOOK_ADDR + HOOK_ORIG_BYTES */
    cave[p++] = 0xE9;
    DWORD jmp_back = (DWORD)(hook_addr + HOOK_ORIG_BYTES) - (DWORD)(cave + p + 4);
    *(DWORD*)(cave + p) = jmp_back; p += 4;

    /* --- Fix up all placeholder jumps --- */
    cave[je_check_case2]     = (BYTE)(check_case2_label - (je_check_case2 + 1));
    cave[jne_both_players]   = (BYTE)(done_label - (jne_both_players + 1));
    cave[jmp_hit1]           = (BYTE)(hit_detected_label - (jmp_hit1 + 1));
    cave[je_both_8balls]     = (BYTE)(done_label - (je_both_8balls + 1));
    cave[jae_double]         = (BYTE)(done_label - (jae_double + 1));

    /* --- Patch the hook site: JMP + NOP --- */
    DWORD old_protect;
    VirtualProtect(hook_addr, HOOK_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_to_cave;
    hook_addr[5] = 0x90;  /* NOP */
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
    BYTE *hook_addr = (BYTE*)HOOK_ADDR;
    if (memcmp(hook_addr, HOOK_ORIG, HOOK_ORIG_BYTES) != 0) {
        return 1;
    }

    install_hook();
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
    (void)hInst; (void)reserved;

    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInst);
        load_real_bass();
        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
    }

    return TRUE;
}
