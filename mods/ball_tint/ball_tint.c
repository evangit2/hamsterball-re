/*
 * ball_tint.c — BASS.dll proxy mod (v2 — board color table method)
 *
 * Tints player 1's ball to a hex color read from ball_tint.txt.
 *
 * How it works:
 *   1. On load: creates ball_tint.txt next to the DLL (if missing)
 *   2. Background thread polls every ~60ms
 *   3. Reads hex color from ball_tint.txt (e.g. "FF6B35" or "#FF6B35")
 *   4. Finds the board via App+0x220 → PlayerProfile+0xC → board
 *   5. Writes RGBA floats directly into the board's player ball color
 *      table at board+0x3AB0 (player 1). These are the same color
 *      values initialized by Board_ctor's four Vec3_Init calls:
 *        board+0x3AB0 = (1.0, 1.0, 1.0) white   (Player 1) ← we write here
 *        board+0x3AC4 = (0.0, 0.5, 1.0) blue    (Player 2)
 *        board+0x3AD8 = (1.0, 0.25, 0.25) salmon (Player 3)
 *        board+0x3AEC = (1.0, 1.0, 0.0) yellow  (Player 4)
 *      Each entry is 4 floats (R, G, B, A) = 16 bytes, spaced 0x14 apart.
 *
 * Color file format:
 *   - Plain text, one line
 *   - Hex color: "FF6B35", "#FF6B35", "0xFF6B35" (case-insensitive)
 *   - Invalid/missing: defaults to white (FFFFFF)
 *   - Re-reads every poll, so you can change colors at runtime
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll ball_tint.c -lwinmm \
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
 * Ball Tint Mod — Board Color Table Method
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Game addresses */
#define APP_PTR_ADDR       0x005341E0   /* Global pointer to App struct */

/* App struct offsets */
#define APP_PROFILE_OFFSET 0x220        /* App+0x220 = PlayerProfile pointer */

/* PlayerProfile struct offsets */
#define PROFILE_BOARD_OFFSET 0x0C      /* profile+0x0C = current Board pointer */

/* Board struct offsets — player ball color table (set by Board_ctor Vec3_Init) */
#define BOARD_COLOR_BASE    0x3AB0     /* Player 1 color RGBA (4 floats) */
#define BOARD_COLOR_STRIDE  0x14       /* 20 bytes per player entry (0x3AB0→0x3AC4→...) */

/* Fallback: ball list for board-scanning method */
#define SCENE_BALL_LIST     0x29D4     /* Board+0x29D4 = AthenaList of balls */
#define ATHENA_COUNT_OFFSET 0x004      /* count at list+0x04 */
#define ATHENA_ARRAY_OFFSET 0x40C      /* array ptr at list+0x40C */

static char g_config_path[MAX_PATH] = {0};
static DWORD g_last_color = 0xFFFFFFFF;  /* Force initial read */

/* Get DLL directory path and build config file path */
static void init_config_path(void)
{
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&init_config_path, &hSelf);
    GetModuleFileNameA(hSelf, g_config_path, MAX_PATH);
    char *p = strrchr(g_config_path, '\\');
    if (p) {
        strcpy(p + 1, "ball_tint.txt");
    } else {
        strcpy(g_config_path, "ball_tint.txt");
    }
}

/* Create default config file if it doesn't exist */
static void create_default_config(void)
{
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        const char *default_content =
            "FFFFFF\n"
            "# Ball Tint Color (hex RGB, no alpha)\n"
            "# Examples: FF6B35 (orange), 4A90D9 (blue), 2ECC71 (green)\n"
            "# Lines starting with # are ignored\n"
            "# Change this value at runtime — mod re-reads every 60ms\n";
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

/* Parse hex color from text. Returns 0xRRGGBB. */
static DWORD parse_hex_color(const char *text)
{
    const char *p = text;

    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (*p == '#') p++;
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) p += 2;

    DWORD hex = 0;
    int digits = 0;
    while (digits < 6 && *p) {
        char c = *p;
        if (c >= '0' && c <= '9') {
            hex = (hex << 4) | (c - '0');
            digits++;
        } else if (c >= 'a' && c <= 'f') {
            hex = (hex << 4) | (c - 'a' + 10);
            digits++;
        } else if (c >= 'A' && c <= 'F') {
            hex = (hex << 4) | (c - 'A' + 10);
            digits++;
        } else {
            break;
        }
        p++;
    }

    if (digits >= 6) {
        return hex;
    } else if (digits >= 3) {
        DWORD r = ((hex >> 8) & 0xF) * 0x11;
        DWORD g = ((hex >> 4) & 0xF) * 0x11;
        DWORD b = (hex & 0xF) * 0x11;
        return (r << 16) | (g << 8) | b;
    }

    return 0xFFFFFF;
}

/* Read color from config file. Returns 0xRRGGBB. */
static DWORD read_color_from_file(void)
{
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return 0xFFFFFF;

    char buf[256] = {0};
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(h);

    if (bytesRead == 0) return 0xFFFFFF;

    char *line = buf;
    while (*line) {
        while (*line == ' ' || *line == '\t') line++;
        if (*line == '#' || *line == '\r' || *line == '\n' || *line == '\0') {
            while (*line && *line != '\n') line++;
            if (*line == '\n') line++;
            continue;
        }
        return parse_hex_color(line);
    }

    return 0xFFFFFF;
}

/*
 * Find the current board (scene) pointer.
 * Primary: App+0x220 → PlayerProfile+0xC → board
 * Fallback: scan App for a pointer with a valid AthenaList at +0x29D4
 */
static DWORD find_board(DWORD app)
{
    /* Primary path: App → profile → board */
    if (!IsBadReadPtr((void*)(app + APP_PROFILE_OFFSET), 4)) {
        DWORD profile = *(DWORD*)(app + APP_PROFILE_OFFSET);
        if (profile && profile > 0x10000 && !IsBadReadPtr((void*)(profile + PROFILE_BOARD_OFFSET), 4)) {
            DWORD board = *(DWORD*)(profile + PROFILE_BOARD_OFFSET);
            if (board && board > 0x10000 && !IsBadReadPtr((void*)board, 0x4000)) {
                return board;
            }
        }
    }

    /* Fallback: scan App for board via AthenaList at +0x29D4 */
    for (int off = 0x100; off < 0xA00; off += 4) {
        DWORD candidate = *(DWORD*)((BYTE*)app + off);
        if (candidate == 0 || candidate < 0x10000) continue;
        if (IsBadReadPtr((void*)candidate, 0x4000)) continue;
        DWORD list_base = candidate + SCENE_BALL_LIST;
        if (IsBadReadPtr((void*)list_base, 0x10)) continue;
        DWORD count = *(DWORD*)(list_base + ATHENA_COUNT_OFFSET);
        DWORD array = *(DWORD*)(list_base + ATHENA_ARRAY_OFFSET);
        if (count > 0 && count < 100 && array != 0 && !IsBadReadPtr((void*)array, 4)) {
            return candidate;
        }
    }

    return 0;
}

/*
 * Write RGBA floats into the board's player ball color table.
 * board+0x3AB0 = Player 1 color (R, G, B, A) — 4 floats
 * board+0x3AC4 = Player 2, board+0x3AD8 = Player 3, board+0x3AEC = Player 4
 */
static void set_board_ball_color(DWORD board, int player_index, float r, float g, float b)
{
    DWORD color_addr = board + BOARD_COLOR_BASE + (player_index * BOARD_COLOR_STRIDE);

    if (IsBadWritePtr((void*)color_addr, 16)) return;

    *(float*)(color_addr + 0x00) = r;   /* R */
    *(float*)(color_addr + 0x04) = g;   /* G */
    *(float*)(color_addr + 0x08) = b;   /* B */
    *(float*)(color_addr + 0x0C) = 1.0f;/* A */
}

/* Background thread: poll and apply tint */
static DWORD WINAPI tint_thread(LPVOID param)
{
    Sleep(3000);  /* Wait for game to fully load */

    for (;;) {
        Sleep(60);

        /* Read color from file */
        DWORD color = read_color_from_file();
        if (color == g_last_color) continue;
        g_last_color = color;

        /* Convert hex to floats */
        float r = ((color >> 16) & 0xFF) / 255.0f;
        float g = ((color >> 8)  & 0xFF) / 255.0f;
        float b = ( color        & 0xFF) / 255.0f;

        /* Find App */
        DWORD app = *(DWORD*)APP_PTR_ADDR;
        if (!app || app < 0x10000) continue;
        if (IsBadReadPtr((void*)app, 0x300)) continue;

        /* Find board */
        DWORD board = find_board(app);
        if (!board) continue;

        /* Write color into board's player 1 ball color slot */
        set_board_ball_color(board, 0, r, g, b);
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
        HANDLE hThread = CreateThread(NULL, 0, tint_thread, NULL, 0, NULL);
        if (hThread) CloseHandle(hThread);
        break;
    }
    return TRUE;
}
