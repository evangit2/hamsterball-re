/*
 * icon_and_windowname.c — Custom icon & window title mod for Hamsterball
 *
 * Features:
 *   1. Custom window title — replaces "Hamsterball" window name
 *   2. Custom icon — searches for icon.ico in the game root folder
 *      If found: replaces both .exe icon and runtime window icon
 *      If not found: game uses original icon
 *
 * Config file: icon_and_windowname.txt (auto-generated next to bass.dll)
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
    return 0;
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

#define GAME_BASE         0x00400000
#define APP_PTR_ADDR      (GAME_BASE + 0x001341E0)  /* 0x005341E0 */

static char g_config_path[MAX_PATH] = {0};
static char g_game_dir[MAX_PATH] = {0};     /* game root folder */
static char g_exe_path[MAX_PATH] = {0};     /* Hamsterball.exe path */
static char g_icon_path[MAX_PATH] = {0};     /* path to icon.ico in game dir */
static char g_state_path[MAX_PATH] = {0};    /* .icon_state.txt */
static char g_window_name[256] = "Hamsterball";
static int  g_icon_found = 0;                /* icon.ico exists */
static int  g_icon_applied = 0;             /* runtime WM_SETICON done */

/* ── Config parsing ────────────────────────────────────────────── */

static void parse_config_line(char *line)
{
    char *p = line;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '#' || *p == ';' || *p == '\0' || *p == '\n' || *p == '\r')
        return;

    char *eq = strchr(p, '=');
    if (!eq) return;
    *eq = '\0';

    char *key = p;
    char *kend = key + strlen(key) - 1;
    while (kend > key && (*kend == ' ' || *kend == '\t')) *kend-- = '\0';

    char *val = eq + 1;
    while (*val == ' ' || *val == '\t') val++;
    char *vend = val + strlen(val) - 1;
    while (vend > val && (*vend == ' ' || *vend == '\t' || *vend == '\n' || *vend == '\r'))
        *vend-- = '\0';

    if (stricmp(key, "window_name") == 0) {
        strncpy(g_window_name, val, sizeof(g_window_name) - 1);
        g_window_name[sizeof(g_window_name) - 1] = '\0';
    }
}

static void load_config(void)
{
    HANDLE hFile = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
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
                "# To change the game icon, place a file named\r\n"
                "# icon.ico in the game root folder (next to Hamsterball.exe)\r\n";
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

    char *line = buf;
    while (line && *line) {
        char *next = strchr(line, '\n');
        if (next) *next = '\0';
        parse_config_line(line);
        if (next) line = next + 1;
        else break;
    }
}

/* ── Path initialization ───────────────────────────────────────── */

static void init_paths(void)
{
    /* Get exe path and game directory */
    GetModuleFileNameA(NULL, g_exe_path, MAX_PATH);
    strncpy(g_game_dir, g_exe_path, MAX_PATH);
    char *p = strrchr(g_game_dir, '\\');
    if (p) {
        p[1] = '\0';
    } else {
        g_game_dir[0] = '\0';
    }

    /* Config file next to bass.dll (same directory as exe) */
    snprintf(g_config_path, MAX_PATH, "%sicon_and_windowname.txt", g_game_dir);

    /* State file */
    snprintf(g_state_path, MAX_PATH, "%s.icon_state.txt", g_game_dir);

    /* Icon file: search for icon.ico in game root */
    snprintf(g_icon_path, MAX_PATH, "%sicon.ico", g_game_dir);
    DWORD attr = GetFileAttributesA(g_icon_path);
    g_icon_found = (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

/* ── Permanent .exe icon update ────────────────────────────────── */

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

    while (bytesRead > 0 && (buf[bytesRead-1] == '\n' || buf[bytesRead-1] == '\r'))
        buf[--bytesRead] = '\0';

    /* State file contains "1" if icon was already written */
    return stricmp(buf, "1") == 0;
}

static void save_icon_state(void)
{
    HANDLE hFile = CreateFileA(g_state_path, GENERIC_WRITE, 0,
                               NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hFile, "1", 1, &written, NULL);
        CloseHandle(hFile);
    }
}

#pragma pack(push, 1)
typedef struct { WORD Reserved; WORD Type; WORD Count; } ICONDIR;
typedef struct {
    BYTE Width; BYTE Height; BYTE ColorCount; BYTE Reserved;
    WORD Planes; WORD BitCount; DWORD BytesInRes; DWORD ImageOffset;
} ICONDIRENTRY;
typedef struct {
    WORD ID; BYTE Width; BYTE Height; BYTE ColorCount; BYTE Reserved;
    WORD Planes; WORD BitCount; DWORD BytesInRes;
} GRPICONDIRENTRY;
#pragma pack(pop)

static int update_exe_icon(const char *exe_path, const char *ico_path)
{
    HANDLE hIco = CreateFileA(ico_path, GENERIC_READ, FILE_SHARE_READ,
                              NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hIco == INVALID_HANDLE_VALUE)
        return 0;

    DWORD icoSize = GetFileSize(hIco, NULL);
    if (icoSize == 0 || icoSize > 1024 * 1024) {
        CloseHandle(hIco);
        return 0;
    }

    BYTE *icoBuf = (BYTE *)HeapAlloc(GetProcessHeap(), 0, icoSize);
    if (!icoBuf) { CloseHandle(hIco); return 0; }

    DWORD bytesRead = 0;
    ReadFile(hIco, icoBuf, icoSize, &bytesRead, NULL);
    CloseHandle(hIco);
    if (bytesRead != icoSize) {
        HeapFree(GetProcessHeap(), 0, icoBuf);
        return 0;
    }

    ICONDIR *dir = (ICONDIR *)icoBuf;
    if (dir->Reserved != 0 || dir->Type != 1 || dir->Count == 0 || dir->Count > 20) {
        HeapFree(GetProcessHeap(), 0, icoBuf);
        return 0;
    }

    int count = dir->Count;
    ICONDIRENTRY *entries = (ICONDIRENTRY *)(icoBuf + sizeof(ICONDIR));

    DWORD groupSize = 6 + sizeof(GRPICONDIRENTRY) * count;
    BYTE *groupData = (BYTE *)HeapAlloc(GetProcessHeap(), 0, groupSize);
    if (!groupData) {
        HeapFree(GetProcessHeap(), 0, icoBuf);
        return 0;
    }

    WORD *grp = (WORD *)groupData;
    grp[0] = 0; grp[1] = 1; grp[2] = (WORD)count;
    GRPICONDIRENTRY *grpEntries = (GRPICONDIRENTRY *)(groupData + 6);

    /* Copy exe to temp, update temp copy's resources */
    char tmp_path[MAX_PATH];
    snprintf(tmp_path, MAX_PATH, "%s.tmp", exe_path);

    if (!CopyFileA(exe_path, tmp_path, FALSE)) {
        HeapFree(GetProcessHeap(), 0, icoBuf);
        HeapFree(GetProcessHeap(), 0, groupData);
        return 0;
    }

    HANDLE hUpdate = BeginUpdateResourceA(tmp_path, FALSE);
    if (!hUpdate) {
        DeleteFileA(tmp_path);
        HeapFree(GetProcessHeap(), 0, icoBuf);
        HeapFree(GetProcessHeap(), 0, groupData);
        return 0;
    }

    int success = 1;

    for (int i = 0; i < count; i++) {
        DWORD imgOffset = entries[i].ImageOffset;
        DWORD imgSize = entries[i].BytesInRes;

        if (imgOffset + imgSize > icoSize) { success = 0; break; }

        if (!UpdateResourceA(hUpdate, RT_ICON, MAKEINTRESOURCEA(i + 1),
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                            icoBuf + imgOffset, imgSize)) {
            success = 0; break;
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

    if (success) {
        if (!UpdateResourceA(hUpdate, RT_GROUP_ICON, "MAINICON",
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                            groupData, groupSize))
            success = 0;
    }

    if (!EndUpdateResourceA(hUpdate, !success)) {
        if (success) success = 0;
    }

    HeapFree(GetProcessHeap(), 0, icoBuf);
    HeapFree(GetProcessHeap(), 0, groupData);

    if (!success) {
        DeleteFileA(tmp_path);
        return 0;
    }

    /* Schedule temp to replace original on next reboot */
    if (!MoveFileExA(tmp_path, exe_path,
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT)) {
        /* Fallback: write update_icon.bat */
        char bat_path[MAX_PATH];
        snprintf(bat_path, MAX_PATH, "%supdate_icon.bat", g_game_dir);

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
                "  echo Failed. Make sure the game is closed.\r\n"
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

    return 1;
}

/* ── Runtime window icon ───────────────────────────────────────── */

static void apply_custom_icon(void)
{
    if (g_icon_applied || !g_icon_found)
        return;

    DWORD *appPtr = (DWORD *)APP_PTR_ADDR;
    if (*appPtr == 0)
        return;

    DWORD app = *appPtr;
    HWND hwnd = (HWND)(*(DWORD *)(app + 0x08));
    if (hwnd == NULL || !IsWindow(hwnd))
        return;

    HICON hIconBig = NULL;
    HICON hIconSmall = NULL;

    /* PrivateExtractIcons handles PNG-compressed .ico files */
    if (PrivateExtractIcons(g_icon_path, 0, 48, 48, &hIconBig, NULL, 1, 0) || hIconBig) {
        PrivateExtractIcons(g_icon_path, 0, 16, 16, &hIconSmall, NULL, 1, 0);
        SendMessageA(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
        if (hIconSmall)
            SendMessageA(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
        g_icon_applied = 1;
        return;
    }

    /* Fallback: LoadImageA for BMP-format .ico */
    hIconBig = (HICON)LoadImageA(NULL, g_icon_path, IMAGE_ICON, 48, 48, LR_LOADFROMFILE);
    if (hIconBig) {
        hIconSmall = (HICON)LoadImageA(NULL, g_icon_path, IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
        SendMessageA(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
        if (hIconSmall)
            SendMessageA(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
        g_icon_applied = 1;
        return;
    }
}

static void apply_window_name(HWND hwnd)
{
    if (hwnd == NULL || !IsWindow(hwnd))
        return;
    SetWindowTextA(hwnd, g_window_name);
}

/* ── Mod thread ───────────────────────────────────────────────── */

static DWORD WINAPI mod_thread(LPVOID param)
{
    /* Step 1: If icon.ico exists and hasn't been written to .exe yet, do it */
    if (g_icon_found && !icon_already_written()) {
        if (update_exe_icon(g_exe_path, g_icon_path))
            save_icon_state();
    }

    /* Step 2: Poll for window, apply runtime icon + window title */
    for (int i = 0; i < 120; i++) {
        DWORD *appPtr = (DWORD *)APP_PTR_ADDR;
        if (*appPtr) {
            DWORD app = *appPtr;
            HWND hwnd = (HWND)(*(DWORD *)(app + 0x08));
            if (hwnd && IsWindow(hwnd)) {
                apply_window_name(hwnd);
                apply_custom_icon();
                if (g_icon_applied || !g_icon_found)
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
        init_paths();
        load_config();
        CreateThread(NULL, 0, mod_thread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
