/*
 * ball_tint.c — BASS.dll proxy mod (v5 — P1-P4 colors, ArenaBoard fix)
 *
 * Tints player balls to hex colors read from ball_tint.txt.
 * Supports custom colors for all 4 players across 1P/2P/4P modes.
 *
 * Config file (ball_tint.txt), 9 lines:
 *   Line 1: Player 1 color in 1-player mode
 *   Line 2: Player 1 color in 2-player mode
 *   Line 3: Player 1 color in 4-player mode (Arena/Rumble)
 *   Line 4: Player 2 color in 2-player mode
 *   Line 5: Player 2 color in 4-player mode (Arena/Rumble)
 *   Line 6: Player 3 color in 4-player mode (Arena/Rumble)
 *   Line 7: Player 4 color in 4-player mode (Arena/Rumble)
 *
 * How it works:
 *   1. On load: creates ball_tint.txt next to the DLL (if missing)
 *   2. Background thread polls every ~30ms
 *   3. Re-reads colors from ball_tint.txt every ~2 seconds
 *   4. Finds the board via App+0x220 → PlayerProfile+0xC → board
 *   5. Validates board by checking vtable pointer is in .rdata range
 *   6. Counts active players (flag at App+0x5D7+slot*0xA0, ZERO = active)
 *   7. Selects colors based on player count for each active player
 *   8. Writes RGBA floats to board color table EVERY poll (no skip)
 *
 * Board color table (set by Board_ctor's Vec3_Init calls):
 *   board+0x3AB0 = (1.0, 1.0, 1.0, 1.0) white   (Player 1)
 *   board+0x3AC4 = (0.0, 0.5, 1.0, 1.0) blue      (Player 2)
 *   board+0x3AD8 = (1.0, 0.25, 0.25, 1.0) salmon   (Player 3)
 *   board+0x3AEC = (1.0, 1.0, 0.0, 1.0) yellow    (Player 4)
 * Each entry is 4 floats (R, G, B, A) = 16 bytes, spaced 0x14 apart.
 *
 * v5 changes:
 *   - Extended BOARD_VTABLE_MAX to 0x4D2000 (ArenaBoard derived vtables
 *     go up to 0x4D1B00+, previous 0x4D1400 was too low)
 *   - Added Player 3 and Player 4 custom colors for 4P mode
 *   - 9 config lines instead of 5
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
typedef int  (__stdcall *BASS_ChannelSetAttribute_t)(DWORD, DWORD, float);
static BASS_ChannelSetAttribute_t real_BASS_ChannelSetAttribute = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttribute(DWORD a, DWORD b, float c) {
    if (real_BASS_ChannelSetAttribute) return real_BASS_ChannelSetAttribute(a, b, c);
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
        LOAD(BASS_ChannelSetAttribute);
        #undef LOAD
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Ball Tint Mod v5 — P1-P4 Multi-Mode Color Support
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Game addresses */
#define APP_PTR_ADDR       0x005341E0   /* Global pointer to App struct */

/* App struct offsets */
#define APP_PROFILE_OFFSET 0x220        /* App+0x220 = PlayerProfile pointer */

/* PlayerProfile struct offsets */
#define PROFILE_BOARD_OFFSET 0x0C      /* profile+0x0C = current Board pointer */

/* Board struct offsets — player ball color table (set by Board_ctor Vec3_Init) */
#define BOARD_COLOR_BASE    0x3AB0     /* Player 1 color RGBA (4 floats) */
#define BOARD_COLOR_STRIDE  0x14       /* 20 bytes per player entry */

/* App struct — player data slots for counting active players */
#define APP_PLAYER_DATA_BASE 0x5CC     /* App+0x5CC = first player_data slot */
#define APP_PLAYER_STRIDE     0xA0     /* 160 bytes per player slot */
#define APP_PLAYER_ACTIVE_OFF 0x0B    /* slot+0x0B = active flag (ZERO = active) */

/* Board vtable range for validation (prevents use-after-free crash).
 * Covers ALL board types:
 *   Board base:     0x4D0260
 *   LevelBoards:    0x4D04A8 - 0x4D0BC0
 *   ArenaBoards:     0x4D1358 - 0x4D1B00+ (WarmUp=0x4D1428, Beginner=0x4D14F0, etc.)
 * Use broad range 0x4D0000-0x4D2000 to cover all derived board vtables. */
#define BOARD_VTABLE_MIN  0x4D0000
#define BOARD_VTABLE_MAX  0x4D2000

static char g_config_path[MAX_PATH] = {0};

/*
 * Nine colors:
 * [0] = P1 color in 1P mode
 * [1] = P1 color in 2P mode
 * [2] = P1 color in 4P mode
 * [3] = P2 color in 2P mode
 * [4] = P2 color in 4P mode
 * [5] = P3 color in 4P mode
 * [6] = P4 color in 4P mode
 */
#define NUM_COLORS 7
static DWORD g_colors[NUM_COLORS] = {
    0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
    0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF
};

/* Shutdown flag — set by DLL_PROCESS_DETACH to stop the background thread */
static volatile LONG g_shutdown = 0;
static HANDLE g_thread_handle = NULL;

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
            "4A90D9\n"
            "2ECC71\n"
            "FF6B35\n"
            "E74C3C\n"
            "9B59B6\n"
            "F1C40F\n"
            "# Ball Tint Colors — one per line\n"
            "# Line 1: Player 1 color in SINGLE-PLAYER mode\n"
            "# Line 2: Player 1 color in 2-PLAYER mode\n"
            "# Line 3: Player 1 color in 4-PLAYER mode (Arena/Rumble)\n"
            "# Line 4: Player 2 color in 2-PLAYER mode\n"
            "# Line 5: Player 2 color in 4-PLAYER mode (Arena/Rumble)\n"
            "# Line 6: Player 3 color in 4-PLAYER mode (Arena/Rumble)\n"
            "# Line 7: Player 4 color in 4-PLAYER mode (Arena/Rumble)\n"
            "# Hex RGB: FF6B35 (orange), 4A90D9 (blue), 2ECC71 (green)\n"
            "# Lines starting with # are ignored\n"
            "# Change values at runtime — mod re-reads every 2 seconds\n";
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

/* Read up to 7 hex colors from config file. */
static void read_colors_from_file(void)
{
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    char buf[1024] = {0};
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(h);

    if (bytesRead == 0) return;

    DWORD colors[NUM_COLORS];
    int i;
    for (i = 0; i < NUM_COLORS; i++) colors[i] = 0xFFFFFF;

    int color_idx = 0;
    char *line = buf;
    while (*line && color_idx < NUM_COLORS) {
        while (*line == ' ' || *line == '\t') line++;
        if (*line == '#' || *line == '\r' || *line == '\n' || *line == '\0') {
            while (*line && *line != '\n') line++;
            if (*line == '\n') line++;
            continue;
        }
        colors[color_idx] = parse_hex_color(line);
        color_idx++;
        while (*line && *line != '\n') line++;
        if (*line == '\n') line++;
    }

    memcpy(g_colors, colors, sizeof(colors));
}

/*
 * Count active players by scanning App's player_data slots.
 * Each slot is 0xA0 bytes. A slot is active when the byte at
 * slot+0x0B (App+0x5D7 for player 1) is ZERO (per Board_ctor).
 * Returns 1, 2, 3, or 4.
 */
static int count_active_players(DWORD app)
{
    int count = 0;
    for (int i = 0; i < 4; i++) {
        DWORD slot_addr = app + APP_PLAYER_DATA_BASE + (i * APP_PLAYER_STRIDE);
        if (IsBadReadPtr((void*)slot_addr, APP_PLAYER_STRIDE)) break;
        BYTE active = *(BYTE*)(slot_addr + APP_PLAYER_ACTIVE_OFF);
        if (active == 0) count++;
    }
    if (count < 1) count = 1;
    return count;
}

/*
 * Validate that a board pointer is still a live Board object.
 * Checks that the vtable pointer is in the game's .rdata range.
 */
static int validate_board(DWORD board)
{
    if (!board || board < 0x10000) return 0;
    if (IsBadReadPtr((void*)board, 4)) return 0;
    DWORD vtable = *(DWORD*)board;
    if (vtable < BOARD_VTABLE_MIN || vtable > BOARD_VTABLE_MAX) return 0;
    return 1;
}

/*
 * Write RGBA floats into the board's player ball color table.
 * board+0x3AB0 = Player 1, +0x3AC4 = P2, +0x3AD8 = P3, +0x3AEC = P4
 */
static void set_board_ball_color(DWORD board, int player_index, float r, float g, float b)
{
    DWORD color_addr = board + BOARD_COLOR_BASE + (player_index * BOARD_COLOR_STRIDE);
    if (IsBadWritePtr((void*)color_addr, 16)) return;
    *(float*)(color_addr + 0x00) = r;
    *(float*)(color_addr + 0x04) = g;
    *(float*)(color_addr + 0x08) = b;
    *(float*)(color_addr + 0x0C) = 1.0f;
}

/* Helper: convert hex color to RGB floats and write to board */
static void apply_color(DWORD board, int player_idx, DWORD hex)
{
    float r = ((hex >> 16) & 0xFF) / 255.0f;
    float g = ((hex >> 8)  & 0xFF) / 255.0f;
    float b = ( hex        & 0xFF) / 255.0f;
    set_board_ball_color(board, player_idx, r, g, b);
}

/* Background thread: poll and apply tint */
static DWORD WINAPI tint_thread(LPVOID param)
{
    Sleep(3000);

    DWORD poll_count = 0;

    for (;;) {
        Sleep(30);

        if (g_shutdown) break;

        /* Re-read colors every ~2 seconds */
        if ((poll_count % 66) == 0) {
            read_colors_from_file();
        }
        poll_count++;

        /* Find App */
        DWORD app = *(DWORD*)APP_PTR_ADDR;
        if (!app || app < 0x10000) continue;
        if (IsBadReadPtr((void*)app, 0x300)) continue;

        /* Find board — primary path only */
        DWORD board = 0;
        if (!IsBadReadPtr((void*)(app + APP_PROFILE_OFFSET), 4)) {
            DWORD profile = *(DWORD*)(app + APP_PROFILE_OFFSET);
            if (profile && profile > 0x10000 &&
                !IsBadReadPtr((void*)(profile + PROFILE_BOARD_OFFSET), 4)) {
                board = *(DWORD*)(profile + PROFILE_BOARD_OFFSET);
                if (board && board > 0x10000 && validate_board(board) &&
                    !IsBadReadPtr((void*)board, 0x4000) &&
                    !IsBadWritePtr((void*)(board + BOARD_COLOR_BASE), 16)) {
                    /* Valid board */
                } else {
                    board = 0;
                }
            }
        }
        if (!board) continue;

        if (g_shutdown) break;

        /* Count active players */
        int num_players = count_active_players(app);

        /* Apply P1 color based on mode */
        int p1_idx;
        if (num_players <= 1) p1_idx = 0;       /* 1P */
        else if (num_players == 2) p1_idx = 1;   /* 2P */
        else p1_idx = 2;                         /* 4P */
        apply_color(board, 0, g_colors[p1_idx]);

        /* Apply P2 color if 2+ players */
        if (num_players >= 2) {
            int p2_idx = (num_players >= 3) ? 4 : 3;
            apply_color(board, 1, g_colors[p2_idx]);
        }

        /* Apply P3 color if 3+ players */
        if (num_players >= 3) {
            apply_color(board, 2, g_colors[5]);  /* P3: 4P mode only */
        }

        /* Apply P4 color if 4 players */
        if (num_players >= 4) {
            apply_color(board, 3, g_colors[6]);  /* P4: 4P mode only */
        }
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
        g_thread_handle = CreateThread(NULL, 0, tint_thread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        InterlockedExchange(&g_shutdown, 1);
        if (g_thread_handle) {
            WaitForSingleObject(g_thread_handle, 2000);
            CloseHandle(g_thread_handle);
            g_thread_handle = NULL;
        }
        break;
    }
    return TRUE;
}
