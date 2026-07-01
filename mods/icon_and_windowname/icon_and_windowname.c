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
#include <stdio.h>

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

/* State file path — tracks the last icon written into the .exe */
static char g_state_path[MAX_PATH] = {0};

/*
 * Read the last icon path we wrote into the .exe.
 * Returns TRUE if state file exists and path matches current g_icon_path.
 */
static int icon_already_written(void)
{
    HANDLE hFile = CreateFileA(g_state_path, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return 0;

    char buf[MAX_PATH] = {0};
    DWORD bytesRead = 0;
    ReadFile(hFile, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(hFile);

    if (bytesRead == 0)
        return 0;

    /* Trim trailing newlines */
    while (bytesRead > 0 && (buf[bytesRead-1] == '\n' || buf[bytesRead-1] == '\r'))
        buf[--bytesRead] = '\0';

    return stricmp(buf, g_icon_path) == 0;
}

/*
 * Save the icon path we just wrote into the .exe.
 */
static void save_icon_state(void)
{
    HANDLE hFile = CreateFileA(g_state_path, GENERIC_WRITE, 0,
                               NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hFile, g_icon_path, (DWORD)strlen(g_icon_path), &written, NULL);
        CloseHandle(hFile);
    }
}

/*
 * Permanently update the RT_ICON resources inside Hamsterball.exe.
 *
 * A .ico file contains an ICONDIR header (6 bytes) + ICONDIRENTRY array
 * (16 bytes each) + the raw PNG/BMP data for each size. We extract each
 * image and write it as a separate RT_ICON resource, then rebuild the
 * RT_GROUP_ICON to reference them.
 *
 * The .exe path is obtained from GetModuleFileNameA(NULL, ...).
 *
 * Returns TRUE on success.
 */
#pragma pack(push, 1)
typedef struct {
    WORD Reserved;
    WORD Type;     /* 1 = icon */
    WORD Count;    /* number of images */
} ICONDIR;

typedef struct {
    BYTE  Width;
    BYTE  Height;
    BYTE  ColorCount;
    BYTE  Reserved;
    WORD  Planes;
    WORD  BitCount;
    DWORD BytesInRes;
    DWORD ImageOffset;
} ICONDIRENTRY;

typedef struct {
    WORD  ID;
    BYTE  Width;
    BYTE  Height;
    BYTE  ColorCount;
    BYTE  Reserved;
    WORD  Planes;
    WORD  BitCount;
    DWORD BytesInRes;
} GRPICONDIRENTRY;
#pragma pack(pop)

static int update_exe_icon(const char *exe_path)
{
    /* Read the .ico file */
    HANDLE hIco = CreateFileA(g_icon_path, GENERIC_READ, FILE_SHARE_READ,
                              NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hIco == INVALID_HANDLE_VALUE)
        return 0;

    DWORD icoSize = GetFileSize(hIco, NULL);
    if (icoSize == 0 || icoSize > 1024 * 1024) { /* sanity: max 1MB */
        CloseHandle(hIco);
        return 0;
    }

    BYTE *icoBuf = (BYTE *)HeapAlloc(GetProcessHeap(), 0, icoSize);
    if (!icoBuf) {
        CloseHandle(hIco);
        return 0;
    }

    DWORD bytesRead = 0;
    ReadFile(hIco, icoBuf, icoSize, &bytesRead, NULL);
    CloseHandle(hIco);
    if (bytesRead != icoSize) {
        HeapFree(GetProcessHeap(), 0, icoBuf);
        return 0;
    }

    /* Parse the ICONDIR header */
    ICONDIR *dir = (ICONDIR *)icoBuf;
    if (dir->Reserved != 0 || dir->Type != 1 || dir->Count == 0 || dir->Count > 20) {
        HeapFree(GetProcessHeap(), 0, icoBuf);
        return 0;
    }

    int count = dir->Count;
    ICONDIRENTRY *entries = (ICONDIRENTRY *)(icoBuf + sizeof(ICONDIR));

    /* Build the RT_GROUP_ICON resource data */
    DWORD groupSize = 6 + sizeof(GRPICONDIRENTRY) * count;
    BYTE *groupData = (BYTE *)HeapAlloc(GetProcessHeap(), 0, groupSize);
    if (!groupData) {
        HeapFree(GetProcessHeap(), 0, icoBuf);
        return 0;
    }

    WORD *grp = (WORD *)groupData;
    grp[0] = 0;       /* Reserved */
    grp[1] = 1;       /* Type = icon */
    grp[2] = (WORD)count;

    GRPICONDIRENTRY *grpEntries = (GRPICONDIRENTRY *)(groupData + 6);

    /*
     * The .exe is locked while the game is running — BeginUpdateResourceA
     * will fail on the live .exe. We copy it to a temp file, update the
     * copy's resources, then schedule it to replace the original on next
     * reboot via MoveFileExA(MOVEFILE_DELAY_UNTIL_REBOOT).
     */
    char tmp_path[MAX_PATH];
    snprintf(tmp_path, MAX_PATH, "%s.tmp", exe_path);

    if (!CopyFileA(exe_path, tmp_path, FALSE)) {
        HeapFree(GetProcessHeap(), 0, icoBuf);
        HeapFree(GetProcessHeap(), 0, groupData);
        return 0;
    }

    /* Begin resource update on the temp copy */
    HANDLE hUpdate = BeginUpdateResourceA(tmp_path, FALSE);
    if (!hUpdate) {
        DeleteFileA(tmp_path);
        HeapFree(GetProcessHeap(), 0, icoBuf);
        HeapFree(GetProcessHeap(), 0, groupData);
        return 0;
    }

    int success = 1;

    /* Write each icon image as RT_ICON (resource IDs 1..count) */
    for (int i = 0; i < count; i++) {
        DWORD imgOffset = entries[i].ImageOffset;
        DWORD imgSize = entries[i].BytesInRes;

        if (imgOffset + imgSize > icoSize) {
            success = 0;
            break;
        }

        BYTE *imgData = icoBuf + imgOffset;

        if (!UpdateResourceA(hUpdate, RT_ICON, MAKEINTRESOURCEA(i + 1),
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                            imgData, imgSize)) {
            success = 0;
            break;
        }

        grpEntries[i].ID         = (WORD)(i + 1);
        grpEntries[i].Width      = entries[i].Width;
        grpEntries[i].Height     = entries[i].Height;
        grpEntries[i].ColorCount = entries[i].ColorCount;
        grpEntries[i].Reserved   = entries[i].Reserved;
        grpEntries[i].Planes     = entries[i].Planes;
        grpEntries[i].BitCount   = entries[i].BitCount;
        grpEntries[i].BytesInRes = entries[i].BytesInRes;
    }

    /* Write the RT_GROUP_ICON (MAINICON) */
    if (success) {
        if (!UpdateResourceA(hUpdate, RT_GROUP_ICON, "MAINICON",
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                            groupData, groupSize)) {
            success = 0;
        }
    }

    if (!EndUpdateResourceA(hUpdate, !success)) {
        if (success)
            success = 0;
    }

    HeapFree(GetProcessHeap(), 0, icoBuf);
    HeapFree(GetProcessHeap(), 0, groupData);

    if (!success) {
        DeleteFileA(tmp_path);
        return 0;
    }

    /*
     * Schedule the temp file to replace the original on next reboot.
     * Windows will delete the old .exe and rename the temp in before
     * any process opens it.
     *
     * MOVEFILE_REPLACE_EXISTING: overwrite the original
     * MOVEFILE_DELAY_UNTIL_REBOOT: schedule for next restart (requires admin)
     *
     * If MoveFileEx fails (no admin), we fall back to writing a batch script
     * that the user can run manually after closing the game.
     */
    if (!MoveFileExA(tmp_path, exe_path,
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT)) {
        /*
         * Fallback: write a .bat file next to the exe that the user can
         * double-click after closing the game to apply the icon change.
         */
        char bat_path[MAX_PATH];
        snprintf(bat_path, MAX_PATH, "%s_update_icon.bat", exe_path);
        char *slash = strrchr(bat_path, '\\');
        if (slash) slash[1] = '\0';
        else bat_path[0] = '\0';

        if (bat_path[0]) {
            snprintf(bat_path + strlen(bat_path), MAX_PATH - strlen(bat_path),
                     "update_icon.bat");

            HANDLE hBat = CreateFileA(bat_path, GENERIC_WRITE, 0,
                                     NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hBat != INVALID_HANDLE_VALUE) {
                char bat_content[1024];
                int len = snprintf(bat_content, sizeof(bat_content),
                    "@echo off\r\n"
                    "echo Updating Hamsterball icon...\r\n"
                    "timeout /t 2 /nobreak >nul\r\n"
                    "move /y \"%s\" \"%s\"\r\n"
                    "if errorlevel 1 (\r\n"
                    "  echo Failed to replace exe. Make sure the game is closed.\r\n"
                    "  pause\r\n"
                    ") else (\r\n"
                    "  echo Icon updated successfully!\r\n"
                    "  del \"%%~f0\"\r\n"
                    ")\r\n",
                    tmp_path, exe_path);
                DWORD written;
                WriteFile(hBat, bat_content, (DWORD)len, &written, NULL);
                CloseHandle(hBat);
            }
        }
    }

    return 1;
}

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
    if (p) {
        /* Save directory prefix */
        char dir[MAX_PATH];
        strncpy(dir, g_config_path, p - g_config_path + 1);
        dir[p - g_config_path + 1] = '\0';

        /* Config file */
        strcpy(p + 1, "icon_and_windowname.txt");

        /* State file */
        snprintf(g_state_path, MAX_PATH, "%s.icon_state.txt", dir);
    }
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
/*
 * Apply custom icon via runtime WM_SETICON.
 *
 * We use PrivateExtractIcons which handles both PNG and BMP-compressed
 * .ico files better than LoadImageA. We request specific sizes (48x48
 * for ICON_BIG, 16x16 for ICON_SMALL) since WM_SETICON needs separate
 * handles for each.
 */
static void apply_custom_icon(void)
{
    if (g_icon_applied || g_icon_path[0] == '\0')
        return;

    /* Get App pointer → HWND */
    DWORD *appPtr = (DWORD *)APP_PTR_ADDR;
    if (*appPtr == 0)
        return;

    DWORD app = *appPtr;
    HWND hwnd = (HWND)(*(DWORD *)(app + 0x08));
    if (hwnd == NULL || !IsWindow(hwnd))
        return;

    /* Try PrivateExtractIcons first (handles PNG-compressed .ico files) */
    HICON hIconBig = NULL;
    HICON hIconSmall = NULL;

    /* Extract 48x48 for ICON_BIG (taskbar/titlebar) */
    if (PrivateExtractIcons(g_icon_path, 0, 48, 48, &hIconBig, NULL, 1, 0) || hIconBig) {
        /* Extract 16x16 for ICON_SMALL */
        PrivateExtractIcons(g_icon_path, 0, 16, 16, &hIconSmall, NULL, 1, 0);

        SendMessageA(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
        if (hIconSmall)
            SendMessageA(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);

        g_icon_applied = 1;
        return;
    }

    /* Fallback: LoadImageA (works for BMP-format .ico files) */
    hIconBig = (HICON)LoadImageA(NULL, g_icon_path, IMAGE_ICON,
                                  48, 48, LR_LOADFROMFILE);
    if (hIconBig) {
        hIconSmall = (HICON)LoadImageA(NULL, g_icon_path, IMAGE_ICON,
                                       16, 16, LR_LOADFROMFILE);
        SendMessageA(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
        if (hIconSmall)
            SendMessageA(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);

        g_icon_applied = 1;
        return;
    }

    /* Both failed — don't set g_icon_applied so we retry next poll */
}

static void apply_window_name(HWND hwnd)
{
    if (hwnd == NULL || !IsWindow(hwnd))
        return;
    SetWindowTextA(hwnd, g_window_name);
}

static DWORD WINAPI mod_thread(LPVOID param)
{
    /*
     * Step 1: Permanently update the .exe icon if needed.
     * This rewrites the RT_ICON resources inside Hamsterball.exe so
     * the icon shows in Explorer/taskbar permanently. We only do this
     * once per unique icon_path (tracked via .icon_state.txt).
     */
    if (g_icon_path[0] != '\0' && !icon_already_written()) {
        char exe_path[MAX_PATH];
        if (GetModuleFileNameA(NULL, exe_path, MAX_PATH) > 0) {
            if (update_exe_icon(exe_path)) {
                save_icon_state();
            }
        }
    }

    /* Step 2: Poll for window creation, then apply runtime icon + window text.
     * Runtime WM_SETICON makes the icon appear immediately (before next restart).
     * The .exe update takes effect on the next launch.
     */
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
