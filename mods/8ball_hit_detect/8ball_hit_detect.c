/*
 * 8ball_hit_detect.c — BASS.dll proxy that detects player→8-ball collisions.
 *
 * HOOK POINT: 0x00406FD1 inside Ball_Update (0x405E00)
 *   Original instruction: fld dword [edi+0x284]  (6 bytes: D9 87 84 02 00 00)
 *
 * At the hook point:
 *   ESI = this ball (running Ball_Update)
 *   EDI = other ball (the collision partner)
 *   We are inside the ball-ball collision scoring section.
 *
 * DETECTION:
 *   ball+0x18 = player_index:  0-3 = Player 1-4,  -1 = NPC 8-ball
 *   A "player hits 8-ball" event = one ball has player_index >= 0 and the
 *   other has player_index == -1.
 *
 * EFFECT:
 *   - Increments g_hit_count (readable via debugger/CE)
 *   - Appends a line to hitlog.txt in the game directory on every hit
 *   - Uses pointer comparison (ESI < EDI) to count each collision once,
 *     since Ball_Update runs for both balls (symmetric double-fire)
 *   - Pure detection only — no gameplay changes
 *
 * ARCHITECTURE:
 *   The code cave does minimal work: checks player indices, increments
 *   g_hit_count, and sets g_hit_pending (a volatile DWORD). A polling
 *   thread in the DLL checks g_hit_pending every 50ms and writes the log.
 *   This avoids calling C functions from hand-assembled code (which caused
 *   FPU/stack corruption in v4).
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

/* Pending hit flag — set by code cave, read by polling thread.
 * 0 = no pending hit. Non-zero = player_index + 1 of the hitter. */
static volatile DWORD g_hit_pending = 0;

/*
 * write_log_line — appends a string to hitlog.txt. Pure Win32 API.
 */
static void write_log_line(const char *msg, int len)
{
    HANDLE hFile = CreateFileA("hitlog.txt", FILE_APPEND_DATA,
        FILE_SHARE_READ, NULL, OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hFile, msg, (DWORD)len, &written, NULL);
        CloseHandle(hFile);
    }
}

/*
 * Polling thread — checks g_hit_pending every 50ms and writes to hitlog.txt.
 * This runs on a separate thread, so file I/O and wsprintfA can't corrupt
 * the game's FPU state or stack.
 */
static DWORD WINAPI log_thread(LPVOID param)
{
    (void)param;
    for (;;) {
        DWORD pending = g_hit_pending;
        if (pending != 0) {
            /* Clear the flag first (interlocked) so we don't double-log */
            InterlockedExchange(&g_hit_pending, 0);

            DWORD count = g_hit_count;
            int player = (int)pending - 1;  /* convert back to 0-based */

            char buf[128];
            int n = wsprintfA(buf, "[Hit %lu] Player %d struck an 8-ball\r\n",
                (unsigned long)count, player + 1);
            write_log_line(buf, n);
        }
        Sleep(50);
    }
    return 0;
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
 *   CMP EAX, 0xFFFFFFFF
 *   JE  .check_case2                ; this ball is 8-ball → check other
 *   CMP EBX, 0xFFFFFFFF
 *   JNE .done                       ; both players → skip
 *   JMP .hit_detected
 *
 *   .check_case2:
 *   CMP EBX, 0xFFFFFFFF
 *   JE  .done                       ; both 8-balls → skip
 *
 *   .hit_detected:
 *   ; --- Avoid double-counting (Ball_Update runs for both balls) ---
 *   CMP ESI, EDI
 *   JAE .done                       ; let the other Ball_Update handle it
 *
 *   ; --- Increment hit counter ---
 *   INC DWORD [g_hit_count]
 *
 *   ; --- Set pending flag for polling thread ---
 *   ; player_index is in EAX (if player) or EBX (if other is player)
 *   ; Use: if EAX >= 0, player = EAX; else player = EBX
 *   CMP EAX, 0xFFFFFFFF
 *   JNE .esi_is_player
 *   ; EDI is the player — use EBX
 *   MOV ECX, EBX
 *   JMP .set_flag
 *   .esi_is_player:
 *   MOV ECX, EAX
 *   .set_flag:
 *   INC ECX                         ; convert to 1-based (0 = no pending)
 *   MOV DWORD [g_hit_pending], ECX
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

    /* --- Set g_hit_pending = player_index + 1 ---
     * If EAX >= 0 (ESI is player), use EAX. Else use EBX (EDI is player).
     * CMP EAX, 0xFFFFFFFF */
    cave[p++] = 0x3D;
    *(DWORD*)(cave + p) = 0xFFFFFFFF; p += 4;
    /* JNE .esi_is_player */
    cave[p++] = 0x75; cave[p++] = 0x00;
    int jne_esi_player = p - 1;

    /* EDI is the player — MOV ECX, EBX */
    cave[p++] = 0x89; cave[p++] = 0xD9;  /* MOV ECX, EBX */
    /* JMP .set_flag */
    cave[p++] = 0xEB; cave[p++] = 0x00;
    int jmp_set_flag = p - 1;

    /* .esi_is_player: */
    int esi_player_label = p;
    /* MOV ECX, EAX */
    cave[p++] = 0x89; cave[p++] = 0xC1;  /* MOV ECX, EAX */

    /* .set_flag: */
    int set_flag_label = p;
    /* INC ECX (convert to 1-based: 0 = no pending, 1-4 = player 1-4) */
    cave[p++] = 0x41;  /* INC ECX */
    /* MOV DWORD [g_hit_pending], ECX */
    cave[p++] = 0x89; cave[p++] = 0x0D;
    *(DWORD*)(cave + p) = (DWORD)&g_hit_pending; p += 4;

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
    cave[jne_esi_player]     = (BYTE)(esi_player_label - (jne_esi_player + 1));
    cave[jmp_set_flag]       = (BYTE)(set_flag_label - (jmp_set_flag + 1));

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

    /* Diagnostic: confirm DLL loaded and patch thread is running */
    write_log_line("[MOD] 8ball_hit_detect DLL loaded\r\n", 33);

    /* Verify hook site has expected bytes */
    BYTE *hook_addr = (BYTE*)HOOK_ADDR;
    if (memcmp(hook_addr, HOOK_ORIG, HOOK_ORIG_BYTES) != 0) {
        write_log_line("[MOD] ERROR: Hook site bytes mismatch — wrong game version?\r\n", 61);
        return 1;
    }

    write_log_line("[MOD] Hook site verified, installing hook...\r\n", 44);
    install_hook();
    write_log_line("[MOD] Hook installed successfully\r\n", 34);

    /* Start the polling thread that writes hitlog.txt entries */
    CreateThread(NULL, 0, log_thread, NULL, 0, NULL);
    write_log_line("[MOD] Log polling thread started\r\n", 33);

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
