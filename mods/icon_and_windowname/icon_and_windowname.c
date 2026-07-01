/*
 * icon_and_windowname.c — Custom icon & window title mod for Hamsterball
 *
 * Features:
 *   1. Custom window title — replaces "Hamsterball" window name
 *   2. Custom icon — loads a .ico file from a user-specified path
 *
 * Config file: icon_and_windowname.txt (auto-generated next to bass.dll)
 *
 * RE Findings (Ghidra):
 *   - Window name: "Hamsterball" string at 0x4D39A8, stored to App+0x20
 *     at 0x42AEE6: mov dword [esi+0x20], 0x4D39A8
 *     Then used as lpWindowName in CreateWindowExA at 0x46BA69.
 *     The mov writes a pointer to the string into the App struct field.
 *     We patch this to point to our own string buffer.
 *
 *   - Icon: Loaded via LoadIconA(hInstance, "MAINICON") at 0x46D1FA during
 *     RegisterClassA. The icon resource is at .rsrc 0x538210 (RT_ICON entries).
 *     The user's two addresses 0x538210 and 0x538ab8 are both RT_ICON resource
 *     data entries — two sizes for the same MAINICON resource.
 *
 *     For custom icon: we call LoadIconA with the EXE's hInstance but override
 *     the RT_ICON resource data to point to our .ico file's data before
 *     RegisterClassA runs. Simpler approach: after window creation, use
 *     SendMessageA(hwnd, WM_SETICON, ICON_BIG/ICON_SMALL, (LPARAM)hIcon)
 *     with an icon loaded from a user .ico file.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll icon_and_windowname.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdlib.h>

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
    return 0; /* FALSE — safe when no bass_real.dll */
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
 * MOD: Custom Icon & Window Name
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Game addresses */
#define GAME_BASE         0x00400000
#define APP_PTR_ADDR      (GAME_BASE + 0x001341E0)  /* 0x005341E0 — global App pointer */
#define ORIGINAL_NAME_STR 0x004D39A8                 /* "Hamsterball" */

static char g_config_path[MAX_PATH] = {0};
static char g_window_name[256] = "Hamsterball";
static char g_icon_path[MAX_PATH] = {0};
static int  g_icon_applied = 0;

/* Read a key=value line from config file */
static void parse_config_line(char *line)
{
    /* Strip whitespace and comments */
    char *p = line;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '#' || *p == ';' || *p == '\0' || *p == '\n' || *p == '\r')
        return;

    /* Find '=' */
    char *eq = strchr(p, '=');
    if (!eq) return;
    *eq = '\0';

    /* Trim key */
    char *key = p;
    char *kend = key + strlen(key) - 1;
    while (kend > key && (*kend == ' ' || *kend == '\t')) *kend-- = '\0';

    /* Trim value */
    char *val = eq + 1;
    while (*val == ' ' || *val == '\t') val++;
    char *vend = val + strlen(val) - 1;
    while (vend > val && (*vend == ' ' || *vend == '\t' || *vend == '\n' || *vend == '\r'))
        *vend-- = '\0';

    if (stricmp(key, "window_name") == 0) {
        strncpy(g_window_name, val, sizeof(g_window_name) - 1);
        g_window_name[sizeof(g_window_name) - 1] = '\0';
    }
    else if (stricmp(key, "icon_path") == 0) {
        strncpy(g_icon_path, val, sizeof(g_icon_path) - 1);
        g_icon_path[sizeof(g_icon_path) - 1] = '\0';
    }
}

static void load_config(void)
{
    HANDLE hFile = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        /* Config doesn't exist — create default */
        hFile = CreateFileA(g_config_path, GENERIC_WRITE, 0,
                           NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            const char *defaults =
                "# Icon & Window Name Mod Configuration\r\n"
                "# Lines starting with # are comments\r\n"
                "\r\n"
                "# Custom window title (shown in title bar)\r\n"
                "# Leave as Hamsterball for default\r\n"
                "window_name = Hamsterball\r\n"
                "\r\n"
                "# Path to a custom .ico file to replace the game icon\r\n"
                "# Use full path or relative path (relative to the game .exe)\r\n"
                "# Leave empty for default game icon\r\n"
                "icon_path =\r\n";
            DWORD written;
            WriteFile(hFile, defaults, (DWORD)strlen(defaults), &written, NULL);
            CloseHandle(hFile);
        }
        return;
    }

    char buf[4096];
    DWORD bytesRead = 0;
    ReadFile(hFile, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(hFile);
    buf[bytesRead] = '\0';

    /* Parse line by line */
    char *line = buf;
    while (line && *line) {
        char *next = strchr(line, '\n');
        if (next) *next = '\0';
        parse_config_line(line);
        if (next) line = next + 1;
        else break;
    }
}

static void init_config_path(void)
{
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&init_config_path, &hSelf);
    GetModuleFileNameA(hSelf, g_config_path, MAX_PATH);
    char *p = strrchr(g_config_path, '\\');
    if (p) strcpy(p + 1, "icon_and_windowname.txt");
}

/*
 * Apply custom icon.
 *
 * The game loads its icon via LoadIconA(hInstance, "MAINICON") during
 * RegisterClassA at 0x46D1FA. After the window is created, we can
 * override the icon by calling SendMessage(hwnd, WM_SETICON, ...).
 *
 * We get the game's HWND from the App struct: App+0x08 (window handle,
 * set at 0x46BA73: mov [esi+8], eax after CreateWindowExA).
 *
 * We poll for the window handle because DllMain runs before the window
 * is created.
 */
static void apply_custom_icon(void)
{
    if (g_icon_applied || g_icon_path[0] == '\0')
        return;

    /* Get App pointer */
    DWORD *appPtr = (DWORD *)APP_PTR_ADDR;
    if (*appPtr == 0)
        return;

    DWORD app = *appPtr;
    HWND hwnd = (HWND)(*(DWORD *)(app + 0x08));
    if (hwnd == NULL || !IsWindow(hwnd))
        return;

    /* Load the custom icon */
    HICON hIconBig = (HICON)LoadImageA(NULL, g_icon_path, IMAGE_ICON,
                                       0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
    if (hIconBig) {
        HICON hIconSmall = (HICON)LoadImageA(NULL, g_icon_path, IMAGE_ICON,
                                             16, 16, LR_LOADFROMFILE);
        SendMessageA(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
        if (hIconSmall)
            SendMessageA(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);

        /* Also set the taskbar icon via SetWindowLong */
        /* GWL_WNDPROC is -4, but we just need the icon via WM_SETICON */
    }
    /* If LoadImageA failed (bad path), silently continue — game uses default icon */

    g_icon_applied = 1;
}

static void apply_window_name(HWND hwnd)
{
    if (hwnd == NULL || !IsWindow(hwnd))
        return;
    SetWindowTextA(hwnd, g_window_name);
}

static DWORD WINAPI mod_thread(LPVOID param)
{
    /* Poll for window creation, then apply icon and set window text */
    for (int i = 0; i < 120; i++) {
        DWORD *appPtr = (DWORD *)APP_PTR_ADDR;
        if (*appPtr) {
            DWORD app = *appPtr;
            HWND hwnd = (HWND)(*(DWORD *)(app + 0x08));
            if (hwnd && IsWindow(hwnd)) {
                apply_window_name(hwnd);
                apply_custom_icon();
                if (g_icon_applied)
                    break;
            }
        }
        Sleep(250);
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry Point
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        load_real_bass();
        init_config_path();
        load_config();

        /* Start mod thread to apply patches after game initializes */
        CreateThread(NULL, 0, mod_thread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
