/*
 * Universal_Safespots — bass.dll proxy mod for Hamsterball
 *
 * Adds a "universal" SAFESPOT type: SAFESPOT(*) in the MESHWORLD file.
 * When the ball has an active checkpoint filter (e.g. "(B)" from E:SAFESWITCH(B)),
 * normal SAFESPOTs with non-matching letters are rejected. This mod intercepts
 * that rejection and checks if the SAFESPOT name contains "(*)". If found,
 * the SAFESPOT is accepted regardless of the current filter — it competes
 * with matching SAFESPOTs on distance alone.
 *
 * HOW IT WORKS:
 *   In Ball_FindClosestRespawnPoint (0x405190), the filter check at 0x405894
 *   does: jne 0x4058C2 (reject if strnicmp != 0). We replace that 2-byte jne
 *   plus the 3 bytes after it with a 5-byte JMP to a code cave. The code cave:
 *     1. If strnicmp matched (eax==0): accept (original behavior)
 *     2. If strnicmp didn't match: call strstr(name, "(*)")
 *        a. If found: accept (override — universal safespot)
 *        b. If not found: reject (original behavior)
 *
 * HOOK POINT: 0x405894 (5 bytes overwritten: 75 2C 8B 07 68)
 *   75 2C           = jne 0x4058C2 (reject)
 *   8B 07           = mov eax, [edi] (accept path: load safespot name)
 *   68 90 F4 4C 00  = push 0x4CF490 (accept path: push "[Z]" string)
 *
 * KEY ADDRESSES (all RVA from module base 0x400000):
 *   strstr:        0x004BAC20 (statically linked __cdecl, preserves edi)
 *   "[Z]" string: 0x004CF490
 *   Accept cont:   0x0040589D (continue after overwritten accept bytes)
 *   Reject target: 0x004058C2
 *
 * COMPILE:
 *   i686-w64-mingw32-gcc -shared -o bass.dll universal_safespots.c \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>

/* ── BASS proxy exports (REQUIRED — all 10 game imports) ────────────── */
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

typedef int  (__stdcall *BASS_MusicLoad_t)(const char*, DWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) int __stdcall BASS_MusicLoad(const char* a, DWORD b, DWORD c, DWORD d, DWORD e) {
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e);
    return 0;
}

typedef int  (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (real_BASS_Free) return real_BASS_Free();
    return 1;
}

typedef int  (__stdcall *BASS_Init_t)(int, int, int, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, int b, int c, HWND d, void* e) {
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 1;
}

typedef void (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) void __stdcall BASS_ChannelStop(DWORD a) {
    if (real_BASS_ChannelStop) real_BASS_ChannelStop(a);
}

typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}

typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
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

/* ── Universal SAFESPOT code cave patch ─────────────────────────────── */

static void apply_patches(void) {
    HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
    if (!hExe) hExe = GetModuleHandleA(NULL);
    DWORD base = (DWORD)hExe;

    /* Key addresses (RVA from module base) */
    DWORD hook_addr   = base + 0x5894;   /* 0x405894: jne + overwritten bytes */
    DWORD strstr_addr  = base + 0xBAC20;  /* 0x4BAC20: statically linked strstr */
    DWORD zstring_addr = base + 0xCF490;  /* 0x4CF490: "[Z]" string in .rdata */
    DWORD accept_cont  = base + 0x589D;   /* 0x40589D: continue accept path */
    DWORD reject_addr  = base + 0x58C2;   /* 0x4058C2: reject path */

    /* Allocate RWX page for code cave + marker string */
    BYTE *cave = (BYTE *)VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE,
                                       PAGE_EXECUTE_READWRITE);
    if (!cave) return;

    /* Place "(*)\0" marker string near end of page */
    const char marker[] = "(*)";
    DWORD marker_addr = (DWORD)(cave + 4090);
    memcpy(cave + 4090, marker, 4);

    /*
     * Build code cave. Layout (all offsets from cave start):
     *
     * pos 0:  85 C0              test eax, eax
     * pos 2:  74 1A              jz accept_path (offset 26 → pos 30)
     * pos 4:  50                 push eax          ; save strnicmp result
     * pos 5:  68 XX XX XX XX     push marker_addr  ; "(*)" — arg 2 (substr)
     * pos 10: FF 37              push dword [edi]  ; safespot name — arg 1 (str)
     * pos 12: E8 XX XX XX XX     call strstr
     * pos 17: 83 C4 08           add esp, 8        ; cleanup (cdecl)
     * pos 20: 85 C0              test eax, eax
     * pos 22: 58                 pop eax           ; restore strnicmp result
     * pos 23: 75 05              jnz accept_path (offset 5 → pos 30)
     * pos 25: E9 XX XX XX XX     jmp reject_addr   ; original reject
     * pos 30: accept_path:
     * pos 30: 8B 07              mov eax, [edi]    ; overwritten byte 1-2
     * pos 32: 68 XX XX XX XX     push zstring_addr ; overwritten byte 3-7
     * pos 37: E9 XX XX XX XX     jmp accept_cont   ; continue original
     * Total: 42 bytes
     */
    int p = 0;

    /* test eax, eax */
    cave[p++] = 0x85; cave[p++] = 0xC0;

    /* jz accept_path (offset = 30 - 4 = 26 = 0x1A) */
    cave[p++] = 0x74; cave[p++] = 0x1A;

    /* push eax (save strnicmp result) */
    cave[p++] = 0x50;

    /* push marker_addr ("(*)") — arg 2 of strstr (substr) */
    cave[p++] = 0x68;
    *(DWORD *)(cave + p) = marker_addr; p += 4;

    /* push dword [edi] (safespot name) — arg 1 of strstr (str) */
    cave[p++] = 0xFF; cave[p++] = 0x37;

    /* call strstr (E8 rel32) */
    cave[p++] = 0xE8;
    *(DWORD *)(cave + p) = strstr_addr - (DWORD)(cave + p + 4); p += 4;

    /* add esp, 8 (cleanup __cdecl args) */
    cave[p++] = 0x83; cave[p++] = 0xC4; cave[p++] = 0x08;

    /* test eax, eax */
    cave[p++] = 0x85; cave[p++] = 0xC0;

    /* pop eax (restore strnicmp result) */
    cave[p++] = 0x58;

    /* jnz accept_path (offset = 30 - 25 = 5) */
    cave[p++] = 0x75; cave[p++] = 0x05;

    /* jmp reject_addr (E9 rel32) */
    cave[p++] = 0xE9;
    *(DWORD *)(cave + p) = reject_addr - (DWORD)(cave + p + 4); p += 4;

    /* === accept_path (pos 30) === */

    /* mov eax, [edi] — re-execute overwritten byte */
    cave[p++] = 0x8B; cave[p++] = 0x07;

    /* push zstring_addr ("[Z]") — re-execute overwritten byte */
    cave[p++] = 0x68;
    *(DWORD *)(cave + p) = zstring_addr; p += 4;

    /* jmp accept_cont (0x40589D) — continue original accept path */
    cave[p++] = 0xE9;
    *(DWORD *)(cave + p) = accept_cont - (DWORD)(cave + p + 4); p += 4;

    /* Verify we wrote exactly 42 bytes */
    /* p should be 42 here */

    /* Patch hook point: replace 5 bytes with JMP to code cave */
    DWORD oldProt;
    if (VirtualProtect((void *)hook_addr, 5, PAGE_READWRITE, &oldProt)) {
        *(BYTE *)hook_addr = 0xE9;  /* JMP rel32 */
        *(DWORD *)(hook_addr + 1) = (DWORD)cave - (hook_addr + 5);
        VirtualProtect((void *)hook_addr, 5, oldProt, &oldProt);
    }
}

/* ── BASS proxy init ────────────────────────────────────────────────── */

static void init_bass_proxy(void) {
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (!g_hRealBass) return;

    real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
    real_BASS_MusicPlayEx          = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
    real_BASS_MusicLoad             = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
    real_BASS_Free                  = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
    real_BASS_Init                  = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
    real_BASS_ChannelStop           = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
    real_BASS_ErrorGetCode          = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
    real_BASS_SetConfig             = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
    real_BASS_Start                 = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
    real_BASS_Stop                  = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            init_bass_proxy();
            apply_patches();
            break;
    }
    return TRUE;
}
