/*
 * menu_colors.c — BASS.dll proxy mod
 *
 * Reads menu_colors.txt and patches the "Click here to play!" button
 * colors on the loading/title screen.
 *
 * Config format (menu_colors.txt):
 *   - LOADING SCREEN -
 *   "Click here" button - Off = #FFFFFF
 *   "Click here" button - On = #FFFFFF
 *
 * How it works:
 *   1. On load: creates menu_colors.txt next to the DLL (if missing)
 *   2. Patches two Vec3_Init call sites in FUN_0042d270 (title renderer)
 *      with code caves that PUSH custom float values from global variables
 *   3. Background thread re-reads config every ~2 seconds
 *   4. When config changes, global floats update → next frame uses new colors
 *
 * Original code (Off button, 0x0042d5fd, 9 bytes):
 *   PUSH 0x3f800000  ; Blue = 1.0f  (5 bytes: 68 00 00 80 3F)
 *   PUSH 0x0          ; Green = 0.0f (2 bytes: 6A 00)
 *   PUSH 0x0          ; Red = 0.0f   (2 bytes: 6A 00)
 *   LEA ECX,[ESP+0x28]; CALL Vec3_Init
 *
 * Original code (On button, 0x0042d624, 9 bytes):
 *   PUSH 0x3f000000  ; Blue = 0.5f  (5 bytes: 68 00 00 00 3F)
 *   PUSH 0x0          ; Green = 0.0f (2 bytes: 6A 00)
 *   PUSH 0x0          ; Red = 0.0f   (2 bytes: 6A 00)
 *   LEA ECX,[ESP+0x64]; CALL Vec3_Init
 *
 * Green/Red use PUSH imm8 (2 bytes) — can't hold arbitrary floats.
 * Code caves redirect to: PUSH [global_float] (6 bytes each) + JMP back.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll menu_colors.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports (all 10 game imports + extras)
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
        GetModuleFileNameA(hSelf, path, MAX_PATH);
        char *p = strrchr(path, '\\');
        if (p) {
            strcpy(p + 1, "bass_real.dll");
            g_hRealBass = LoadLibraryA(path);
        }
    }
    if (g_hRealBass) {
        #define LOAD(name) real_##name = (name##_t)GetProcAddress(g_hRealBass, #name)
        LOAD(BASS_ChannelSetAttributes);
        LOAD(BASS_MusicPlayEx);
        LOAD(BASS_SetConfig);
        LOAD(BASS_Init);
        LOAD(BASS_Free);
        LOAD(BASS_Start);
        LOAD(BASS_Stop);
        LOAD(BASS_ErrorGetCode);
        LOAD(BASS_MusicLoad);
        LOAD(BASS_ChannelStop);
        #undef LOAD
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Menu Colors Mod
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Original code addresses (Hamsterball.exe, base 0x400000) */
#define OFF_PATCH_ADDR   0x0042d5fd   /* Start of 3 PUSHes for Off button */
#define OFF_RETURN_ADDR  0x0042d606   /* LEA ECX after the 3 PUSHes */
#define OFF_PATCH_SIZE   9            /* 5 + 2 + 2 bytes */

#define ON_PATCH_ADDR    0x0042d624   /* Start of 3 PUSHes for On button */
#define ON_RETURN_ADDR   0x0042d62d   /* LEA ECX after the 3 PUSHes */
#define ON_PATCH_SIZE    9            /* 5 + 2 + 2 bytes */

/* Each code cave: 3 × PUSH [addr] (6 bytes each) + JMP rel32 (5 bytes) = 23 bytes */
#define CAVE_SIZE        23

/* Global color values — updated by config reader, read by code caves.
 * Defaults are #FFFFFF (white) to match the config file default. */
static float g_off_r = 1.0f, g_off_g = 1.0f, g_off_b = 1.0f;
static float g_on_r  = 1.0f, g_on_g  = 1.0f, g_on_b  = 1.0f;

static char g_config_path[MAX_PATH] = {0};
static volatile LONG g_shutdown = 0;
static HANDLE g_thread_handle = NULL;

/* ── Config file management ──────────────────────────────────────────────── */

static void init_config_path(void)
{
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&init_config_path, &hSelf);
    GetModuleFileNameA(hSelf, g_config_path, MAX_PATH);
    char *p = strrchr(g_config_path, '\\');
    if (p) {
        strcpy(p + 1, "menu_colors.txt");
    } else {
        strcpy(g_config_path, "menu_colors.txt");
    }
}

static void create_default_config(void)
{
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        const char *default_content =
            "- LOADING SCREEN -\r\n"
            "\"Click here\" button - Off = #FFFFFF\r\n"
            "\"Click here\" button - On = #FFFFFF\r\n"
            "\r\n"
            "\r\n"
            "- MAIN MENU -\r\n"
            "soon.\r\n"
            "\r\n"
            "\r\n"
            "- TOURNEY MENU -\r\n"
            "soon.\r\n"
            "\r\n"
            "\r\n"
            "- MIRROR MENU -\r\n"
            "soon.\r\n"
            "\r\n"
            "\r\n"
            "- TIME TRIALS MENU -\r\n"
            "soon.\r\n"
            "\r\n"
            "\r\n"
            "- 2P MENU -\r\n"
            "soon.\r\n"
            "\r\n"
            "\r\n"
            "- 4P MENU -\r\n"
            "soon.\r\n"
            "\r\n"
            "\r\n"
            "- OTHER -\r\n"
            "soon.\r\n";
        DWORD written;
        h = CreateFileA(g_config_path, GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE) {
            WriteFile(h, default_content, lstrlenA(default_content), &written, NULL);
            CloseHandle(h);
        }
    } else {
        CloseHandle(h);
    }
}

/* ── Hex color parser ────────────────────────────────────────────────────── */

static DWORD parse_hex_color(const char *text)
{
    const char *p = text;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '#') p++;
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) p += 2;

    DWORD hex = 0;
    int digits = 0;
    while (digits < 6 && *p) {
        char c = *p;
        if (c >= '0' && c <= '9') { hex = (hex << 4) | (c - '0'); digits++; }
        else if (c >= 'a' && c <= 'f') { hex = (hex << 4) | (c - 'a' + 10); digits++; }
        else if (c >= 'A' && c <= 'F') { hex = (hex << 4) | (c - 'A' + 10); digits++; }
        else break;
        p++;
    }
    if (digits >= 6) return hex;
    if (digits >= 3) {
        DWORD r = ((hex >> 8) & 0xF) * 0x11;
        DWORD g = ((hex >> 4) & 0xF) * 0x11;
        DWORD b = (hex & 0xF) * 0x11;
        return (r << 16) | (g << 8) | b;
    }
    return 0xFFFFFF;
}

/* Convert hex to float RGB and store in target variables */
static void hex_to_floats(DWORD hex, float *r, float *g, float *b)
{
    *r = ((hex >> 16) & 0xFF) / 255.0f;
    *g = ((hex >> 8)  & 0xFF) / 255.0f;
    *b = ( hex        & 0xFF) / 255.0f;
}

/* ── Config reader ───────────────────────────────────────────────────────── */

/* Scan a line for a hex color after '=' sign. Returns 1 if found. */
static int extract_color_from_line(const char *line, DWORD *out_hex)
{
    const char *eq = strchr(line, '=');
    if (!eq) return 0;
    eq++;
    while (*eq == ' ' || *eq == '\t') eq++;
    if (*eq == '\0' || *eq == '\r' || *eq == '\n') return 0;

    /* Check if it looks like "soon" or empty */
    if (strnicmp(eq, "soon", 4) == 0) return 0;

    *out_hex = parse_hex_color(eq);
    return 1;
}

/* Check if a line contains a case-insensitive substring */
static int line_contains(const char *line, const char *needle)
{
    /* Simple case-insensitive search */
    int needle_len = lstrlenA(needle);
    int line_len = lstrlenA(line);
    int i, j;
    for (i = 0; i <= line_len - needle_len; i++) {
        int match = 1;
        for (j = 0; j < needle_len; j++) {
            char a = line[i + j];
            char b = needle[j];
            if (a >= 'A' && a <= 'Z') a += 32;
            if (b >= 'A' && b <= 'Z') b += 32;
            if (a != b) { match = 0; break; }
        }
        if (match) return 1;
    }
    return 0;
}

/* Read menu_colors.txt and update global color variables */
static void read_config(void)
{
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    char buf[4096] = {0};
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(h);
    if (bytesRead == 0) return;

    float new_off_r = g_off_r, new_off_g = g_off_g, new_off_b = g_off_b;
    float new_on_r  = g_on_r,  new_on_g  = g_on_g,  new_on_b  = g_on_b;

    char *line = buf;
    while (*line) {
        /* Get line length */
        char *nl = line;
        while (*nl && *nl != '\n') nl++;
        int len = nl - line;
        /* Trim trailing \r */
        if (len > 0 && line[len-1] == '\r') len--;

        /* Check for "Off" with a color */
        if (line_contains(line, "off") && len < 200) {
            DWORD hex;
            if (extract_color_from_line(line, &hex)) {
                hex_to_floats(hex, &new_off_r, &new_off_g, &new_off_b);
            }
        }
        /* Check for "On" with a color (but not "button" alone) */
        else if (line_contains(line, "on") && line_contains(line, "=") && len < 200) {
            DWORD hex;
            if (extract_color_from_line(line, &hex)) {
                hex_to_floats(hex, &new_on_r, &new_on_g, &new_on_b);
            }
        }

        if (*nl == '\n') nl++;
        line = nl;
        if (*line == '\0') break;
    }

    /* Atomically update globals */
    g_off_r = new_off_r; g_off_g = new_off_g; g_off_b = new_off_b;
    g_on_r  = new_on_r;  g_on_g  = new_on_g;  g_on_b  = new_on_b;
}

/* ── Code cave builder ───────────────────────────────────────────────────── */

/*
 * Build a code cave that replaces 3 consecutive PUSH instructions.
 *
 * Original (9 bytes): PUSH imm32(blue); PUSH imm8(green); PUSH imm8(red)
 * Cave (23 bytes):
 *   PUSH dword ptr [blue_global]    ; FF 35 <addr32>  (6 bytes)
 *   PUSH dword ptr [green_global]   ; FF 35 <addr32>  (6 bytes)
 *   PUSH dword ptr [red_global]     ; FF 35 <addr32>  (6 bytes)
 *   JMP return_addr                  ; E9 <rel32>      (5 bytes)
 *
 * The PUSH order matches the original: Blue (z) first, then Green (y), then Red (x).
 * Vec3_Init takes (x=Red, y=Green, z=Blue) as cdecl stack params, pushed in reverse.
 */
static void build_cave(unsigned char *cave, float *r, float *g, float *b, DWORD ret_addr)
{
    int idx = 0;

    /* PUSH dword ptr [blue] — pushed first (z argument, highest on stack) */
    cave[idx++] = 0xFF;
    cave[idx++] = 0x35;
    *(DWORD*)(cave + idx) = (DWORD)b;
    idx += 4;

    /* PUSH dword ptr [green] — pushed second (y argument) */
    cave[idx++] = 0xFF;
    cave[idx++] = 0x35;
    *(DWORD*)(cave + idx) = (DWORD)g;
    idx += 4;

    /* PUSH dword ptr [red] — pushed third (x argument, lowest on stack) */
    cave[idx++] = 0xFF;
    cave[idx++] = 0x35;
    *(DWORD*)(cave + idx) = (DWORD)r;
    idx += 4;

    /* JMP ret_addr — relative jump back to original code */
    cave[idx++] = 0xE9;
    DWORD cave_base = (DWORD)cave;
    /* offset = target - address_of_next_instruction = ret_addr - (cave_base + idx + 4) */
    *(DWORD*)(cave + idx) = ret_addr - (cave_base + idx + 4);
    idx += 4;

    /* Total: 23 bytes */
}

/* ── Code patcher ────────────────────────────────────────────────────────── */

/*
 * Patch original code: replace 9-byte PUSH sequence with JMP + NOPs.
 * 5-byte JMP (E9 rel32) + 4 NOPs (0x90) = 9 bytes total.
 */
static int patch_code(DWORD addr, int patch_size, DWORD cave_addr)
{
    DWORD old_protect;
    if (!VirtualProtect((void*)addr, patch_size, PAGE_EXECUTE_READWRITE, &old_protect))
        return 0;

    /* Write JMP to cave (5 bytes) */
    *(unsigned char*)addr = 0xE9;
    *(DWORD*)(addr + 1) = cave_addr - (addr + 5);

    /* NOP remaining bytes */
    int i;
    for (i = 5; i < patch_size; i++)
        *(unsigned char*)(addr + i) = 0x90;

    VirtualProtect((void*)addr, patch_size, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), (void*)addr, patch_size);
    return 1;
}

/* Install both code caves (Off button + On button) */
static void install_patches(void)
{
    /* Allocate executable memory for both caves (23 bytes each, round up to 64) */
    unsigned char *cave_mem = (unsigned char*)VirtualAlloc(
        NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!cave_mem) return;

    /* Build Off button cave (reads from g_off_r/g/b) */
    build_cave(cave_mem, &g_off_r, &g_off_g, &g_off_b, OFF_RETURN_ADDR);

    /* Build On button cave (reads from g_on_r/g/b) */
    build_cave(cave_mem + CAVE_SIZE, &g_on_r, &g_on_g, &g_on_b, ON_RETURN_ADDR);

    /* Patch Off button code → redirect to cave */
    patch_code(OFF_PATCH_ADDR, OFF_PATCH_SIZE, (DWORD)cave_mem);

    /* Patch On button code → redirect to cave */
    patch_code(ON_PATCH_ADDR, ON_PATCH_SIZE, (DWORD)(cave_mem + CAVE_SIZE));
}

/* ── Background thread ───────────────────────────────────────────────────── */

static DWORD WINAPI config_thread(LPVOID param)
{
    Sleep(2000);

    for (;;) {
        Sleep(30);
        if (g_shutdown) break;

        /* Re-read config every ~2 seconds (66 polls × 30ms ≈ 2s) */
        static DWORD poll_count = 0;
        if ((poll_count % 66) == 0) {
            read_config();
        }
        poll_count++;
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        load_real_bass();
        init_config_path();
        create_default_config();
        read_config();           /* Read config before first render */
        install_patches();        /* Patch code caves */
        g_thread_handle = CreateThread(NULL, 0, config_thread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        InterlockedExchange(&g_shutdown, 1);
        if (g_thread_handle) {
            WaitForSingleObject(g_thread_handle, 2000);
            CloseHandle(g_thread_handle);
        }
        break;
    }
    return TRUE;
}
