/*
 * ball_tint.c — BASS.dll proxy mod (v4 — P1+P2 colors, reliable reapply, crash fix)
 *
 * Tints player balls to hex colors read from ball_tint.txt.
 * Supports custom colors for Player 1 (1P/2P/4P modes) and Player 2 (2P/4P modes).
 *
 * Config file (ball_tint.txt), 5 lines:
 *   Line 1: Player 1 color in 1-player mode
 *   Line 2: Player 1 color in 2-player mode
 *   Line 3: Player 1 color in 4-player mode (Arena/Rumble)
 *   Line 4: Player 2 color in 2-player mode
 *   Line 5: Player 2 color in 4-player mode (Arena/Rumble)
 *
 * How it works:
 *   1. On load: creates ball_tint.txt next to the DLL (if missing)
 *   2. Background thread polls every ~60ms
 *   3. Re-reads 5 hex colors from ball_tint.txt each poll
 *   4. Finds the board via App+0x220 → PlayerProfile+0xC → board
 *   5. Validates board by checking vtable pointer is in game's .rdata range
 *   6. Counts active players (flag at App+0x5D7+slot*0xA0, ZERO = active)
 *   7. Selects P1 color based on player count, P2 color for 2P/4P
 *   8. Writes RGBA floats to board color table EVERY poll (no skip optimization)
 *
 * Board color table (set by Board_ctor's Vec3_Init calls):
 *   board+0x3AB0 = (1.0, 1.0, 1.0, 1.0) white   (Player 1)
 *   board+0x3AC4 = (0.0, 0.5, 1.0, 1.0) blue      (Player 2)
 *   board+0x3AD8 = (1.0, 0.25, 0.25, 1.0) salmon   (Player 3)
 *   board+0x3AEC = (1.0, 1.0, 0.0, 1.0) yellow    (Player 4)
 * Each entry is 4 floats (R, G, B, A) = 16 bytes, spaced 0x14 apart.
 *
 * v4 changes:
 *   - Fixed 1P/4P player count detection (was inverted: 0=active, not non-zero)
 *   - Added Player 2 custom colors for 2P and 4P modes
 *   - Tint applied EVERY poll (fixes random loss on race restart)
 *   - Board vtable validation + DLL_PROCESS_DETACH shutdown (fixes Quit crash)
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
 * Ball Tint Mod v4 — P1+P2 Multi-Mode Color Support + Crash Fix
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

/* App struct — player data slots for counting active players */
#define APP_PLAYER_DATA_BASE 0x5CC     /* App+0x5CC = first player_data slot */
#define APP_PLAYER_STRIDE     0xA0     /* 160 bytes per player slot */
#define APP_PLAYER_ACTIVE_OFF 0x0B    /* slot+0x0B = active flag (ZERO = active, per Board_ctor) */

/* Board vtable range for validation (prevents use-after-free crash) */
/* Board base vtable = 0x4D0260, derived board vtables up to ~0x4D0BC0 */
#define BOARD_VTABLE_MIN  0x4D0200
#define BOARD_VTABLE_MAX  0x4D0C00

/* Fallback: ball list for board-scanning method */
#define SCENE_BALL_LIST     0x29D4     /* Board+0x29D4 = AthenaList of balls */
#define ATHENA_COUNT_OFFSET 0x004      /* count at list+0x04 */
#define ATHENA_ARRAY_OFFSET 0x40C      /* array ptr at list+0x40C */

static char g_config_path[MAX_PATH] = {0};

/* Five colors:
 * [0] = P1 color in 1P mode
 * [1] = P1 color in 2P mode
 * [2] = P1 color in 4P mode
 * [3] = P2 color in 2P mode
 * [4] = P2 color in 4P mode
 */
#define NUM_COLORS 5
static DWORD g_colors[NUM_COLORS] = { 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF };

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
            "# Ball Tint Colors — one per line\n"
            "# Line 1: Player 1 color in SINGLE-PLAYER mode\n"
            "# Line 2: Player 1 color in 2-PLAYER mode\n"
            "# Line 3: Player 1 color in 4-PLAYER mode (Arena/Rumble)\n"
            "# Line 4: Player 2 color in 2-PLAYER mode\n"
            "# Line 5: Player 2 color in 4-PLAYER mode (Arena/Rumble)\n"
            "# Hex RGB: FF6B35 (orange), 4A90D9 (blue), 2ECC71 (green)\n"
            "# Lines starting with # are ignored\n"
            "# Change values at runtime — mod re-reads every 60ms\n";
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

/* Read up to 5 hex colors from config file (first 5 non-comment lines). */
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

    DWORD colors[NUM_COLORS] = { 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF };
    int color_idx = 0;

    char *line = buf;
    while (*line && color_idx < NUM_COLORS) {
        /* Skip whitespace */
        while (*line == ' ' || *line == '\t') line++;
        /* Skip comment/empty lines */
        if (*line == '#' || *line == '\r' || *line == '\n' || *line == '\0') {
            while (*line && *line != '\n') line++;
            if (*line == '\n') line++;
            continue;
        }
        /* Parse hex color from this line */
        colors[color_idx] = parse_hex_color(line);
        color_idx++;
        /* Advance to next line */
        while (*line && *line != '\n') line++;
        if (*line == '\n') line++;
    }

    /* Atomic copy to g_colors */
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
        if (active == 0) count++;  /* ZERO = active (verified from Board_ctor) */
    }
    if (count < 1) count = 1;  /* Safety: at least player 1 */
    return count;
}

/*
 * Validate that a board pointer is still a live Board object.
 * Checks that the vtable pointer (first 4 bytes) is in the expected
 * range for Board vtables. When the board is freed, the vtable pointer
 * becomes garbage — this prevents writing to freed memory.
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
            if (board && board > 0x10000 && validate_board(board)) {
                return board;
            }
        }
    }

    /* Fallback: scan App for board via AthenaList at +0x29D4 */
    for (int off = 0x100; off < 0xA00; off += 4) {
        DWORD candidate = *(DWORD*)((BYTE*)app + off);
        if (candidate == 0 || candidate < 0x10000) continue;
        if (!validate_board(candidate)) continue;
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

        /* Check shutdown flag */
        if (g_shutdown) break;

        /* Find App */
        DWORD app = *(DWORD*)APP_PTR_ADDR;
        if (!app || app < 0x10000) continue;
        if (IsBadReadPtr((void*)app, 0x300)) continue;

        /* Find board */
        DWORD board = find_board(app);
        if (!board) continue;

        /* Re-check shutdown after potentially slow operations */
        if (g_shutdown) break;

        /* Re-read colors from file every poll */
        read_colors_from_file();

        /* Count active players to select which color to use */
        int num_players = count_active_players(app);

        /* Select P1 color index: 1→0, 2→1, 3+→2 (4P mode) */
        int p1_color_idx;
        if (num_players <= 1) p1_color_idx = 0;      /* 1P */
        else if (num_players == 2) p1_color_idx = 1;   /* 2P */
        else p1_color_idx = 2;                          /* 3P or 4P (Arena/Rumble) */

        DWORD p1_color = g_colors[p1_color_idx];

        /* Convert hex to floats */
        float p1_r = ((p1_color >> 16) & 0xFF) / 255.0f;
        float p1_g = ((p1_color >> 8)  & 0xFF) / 255.0f;
        float p1_b = ( p1_color        & 0xFF) / 255.0f;

        /* Write P1 color into board's player 1 ball color slot.
         * We ALWAYS write — this fixes the "random tint" issue where
         * race restarts would sometimes lose the tint because malloc
         * reused the same board address (g_last_board matched). */
        set_board_ball_color(board, 0, p1_r, p1_g, p1_b);

        /* Write P2 color if in 2P or 4P mode */
        if (num_players >= 2) {
            /* P2 color: [3]=2P mode, [4]=4P mode */
            int p2_color_idx = (num_players >= 3) ? 4 : 3;
            DWORD p2_color = g_colors[p2_color_idx];

            float p2_r = ((p2_color >> 16) & 0xFF) / 255.0f;
            float p2_g = ((p2_color >> 8)  & 0xFF) / 255.0f;
            float p2_b = ( p2_color        & 0xFF) / 255.0f;

            set_board_ball_color(board, 1, p2_r, p2_g, p2_b);
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
        if (g_thread_handle) {
            /* Prevent DLL from being unloaded while thread is running */
            /* (not strictly necessary for bass.dll but good practice) */
        }
        break;

    case DLL_PROCESS_DETACH:
        /* Signal the background thread to stop and wait briefly.
         * This prevents the thread from accessing freed board memory
         * during game shutdown (fixes 0001:00057DD4 crash on Quit). */
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
