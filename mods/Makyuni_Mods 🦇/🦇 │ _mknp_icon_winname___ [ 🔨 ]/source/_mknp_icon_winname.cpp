/*
 * _mknp_icon_winname.cpp — Custom icon & window title mod (HB+ v2.1, MinGW)
 *
 * HB+ port of the icon_and_windowname bass.dll proxy. No BASS exports, no
 * threads, no EXE patches at load: the HB+ loader scans Mods\, calls
 * CreateModInstance, fires Initialize(), then ticks onGameUpdate().
 *
 * WHAT THIS MOD DOES
 * ------------------
 * 1. Custom window title — reads window_name from _mknp_icon_winname.txt
 *    (lives NEXT TO THIS DLL, e.g. Mods\) and applies it to the game
 *    window via SetWindowTextA. Re-applied on mismatch every tick.
 * 2. Custom icon — looks for icon.ico in the GAME ROOT folder
 *    (next to Hamsterball.exe, exe-side so the .exe patcher finds it).
 *    If found: runtime WM_SETICON immediately + permanent RT_ICON
 *    resource update on a temp exe copy (reboot-swap, once via state).
 *    If not found: game keeps its original icon.
 *
 * Own-DLL dir resolved via VirtualQuery (mod_self), same as entities_plus.
 *
 * Author: MAKYUNI / Hamsterbot
 */

#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

#include <windows.h>

#define strlen  nc_strlen
#define strncpy nc_strncpy
#define snprintf nc_snprintf

/* ═══════════════════════════════════════════════════════════════════════════
 * State
 * ═══════════════════════════════════════════════════════════════════════════ */

static IModAPI* g_api = NULL;
static HMODULE  g_module = NULL;

static char g_dll_dir[MAX_PATH];    /* folder of this DLL, trailing \ (Mods\) */
static char g_config_path[MAX_PATH];/* _mknp_icon_winname.txt next to DLL */
static char g_game_dir[MAX_PATH];   /* game root, trailing \ */
static char g_exe_path[MAX_PATH];   /* Hamsterball.exe full path */
static char g_icon_path[MAX_PATH];  /* icon.ico in game root */
static char g_state_path[MAX_PATH]; /* .icon_state.txt in game root */

static char g_window_name[256];

static int g_started      = 0;  /* init_impl ran */
static int g_icon_found   = 0;  /* icon.ico exists */
static int g_icon_applied = 0;  /* runtime WM_SETICON done */
static int g_exe_icon_tried = 0;/* permanent .exe patch attempted once */

/* ═══════════════════════════════════════════════════════════════════════════
 * Helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE mod_self(void) {
    if (g_module) return g_module;
    MEMORY_BASIC_INFORMATION mbi;
    nc_memset(&mbi, 0, sizeof(mbi));
    if (VirtualQuery((void*)mod_self, &mbi, sizeof(mbi)))
        g_module = (HMODULE)mbi.AllocationBase;
    return g_module;
}

/* copy src->dst, then append tail (both NUL-terminated). dst size MAX_PATH. */
static void path_join(char* dst, const char* dir, const char* tail) {
    nc_strncpy(dst, dir, MAX_PATH - 1);
    dst[MAX_PATH - 1] = '\0';
    size_t dl = nc_strlen(dst);
    size_t i = 0;
    while (tail[i] && dl + 1 < MAX_PATH) dst[dl++] = tail[i++];
    dst[dl] = '\0';
}

/* Render app+0x08 HWND (proven bass-proxy approach: HWND lives at App+0x08). */
static HWND game_hwnd(void) {
    if (!g_api) return NULL;
    App* app = HBAPI(g_api).GetApp();
    if (!app) return NULL;
    HWND hwnd = *(HWND*)((char*)app + 0x08);
    if (!hwnd || !IsWindow(hwnd)) return NULL;
    return hwnd;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config (_mknp_icon_winname.txt next to this DLL)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void parse_line(char* line) {
    while (*line == ' ' || *line == '\t') line++;
    if (*line == '#' || *line == ';' || *line == '\0' ||
        *line == '\n' || *line == '\r')
        return;
    char* eq = NULL;
    for (char* p = line; *p; p++) if (*p == '=') { eq = p; break; }
    if (!eq) return;
    *eq = '\0';
    char* key = line;
    /* rtrim key */
    char* ke = key + nc_strlen(key);
    while (ke > key && (ke[-1] == ' ' || ke[-1] == '\t')) *--ke = '\0';
    char* val = eq + 1;
    while (*val == ' ' || *val == '\t') val++;
    char* ve = val + nc_strlen(val);
    while (ve > val && (ve[-1] == ' ' || ve[-1] == '\t' ||
                        ve[-1] == '\n' || ve[-1] == '\r')) *--ve = '\0';
    if (lstrcmpiA(key, "window_name") == 0) {
        nc_strncpy(g_window_name, val, sizeof(g_window_name) - 1);
        g_window_name[sizeof(g_window_name) - 1] = '\0';
    }
}

static void load_config(void) {
    nc_strncpy(g_window_name, "Hamsterball", sizeof(g_window_name) - 1);

    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        /* Auto-generate defaults next to the DLL */
        h = CreateFileA(g_config_path, GENERIC_WRITE, 0,
                        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE) {
            const char* defaults =
                "# Icon & Window Name Mod Configuration\r\n"
                "# Lines starting with # are comments\r\n"
                "# This file lives next to the mod DLL (e.g. Mods\\)\r\n"
                "\r\n"
                "# Custom window title (shown in title bar)\r\n"
                "# Leave as Hamsterball for default\r\n"
                "window_name = Hamsterball\r\n"
                "\r\n"
                "# To change the game icon, place a file named\r\n"
                "# icon.ico in the game root folder (next to Hamsterball.exe)\r\n";
            DWORD wrote = 0;
            WriteFile(h, defaults, (DWORD)nc_strlen(defaults), &wrote, NULL);
            CloseHandle(h);
        }
        return;
    }
    static char buf[4096]; /* static: -nostdlib has no __chkstk_ms for 4KB frames */
    DWORD got = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &got, NULL);
    CloseHandle(h);
    buf[got] = '\0';

    char* line = buf;
    while (line && *line) {
        char* nl = NULL;
        for (char* p = line; *p; p++) if (*p == '\n') { nl = p; break; }
        if (nl) *nl = '\0';
        parse_line(line);
        if (nl) line = nl + 1;
        else break;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Permanent .exe icon update (game-root side, runs once)
 * ═══════════════════════════════════════════════════════════════════════════ */

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

static int icon_already_written(void) {
    HANDLE h = CreateFileA(g_state_path, GENERIC_READ, FILE_SHARE_READ,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return 0;
    char buf[MAX_PATH];
    DWORD got = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &got, NULL);
    CloseHandle(h);
    if (got == 0) return 0;
    while (got > 0 && (buf[got-1] == '\n' || buf[got-1] == '\r')) got--;
    buf[got] = '\0';
    return lstrcmpiA(buf, "1") == 0;
}

static void save_icon_state(void) {
    HANDLE h = CreateFileA(g_state_path, GENERIC_WRITE, 0,
                           NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD wrote = 0;
        WriteFile(h, "1", 1, &wrote, NULL);
        CloseHandle(h);
    }
}

static int update_exe_icon(void) {
    HANDLE hIco = CreateFileA(g_icon_path, GENERIC_READ, FILE_SHARE_READ,
                              NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hIco == INVALID_HANDLE_VALUE) return 0;
    DWORD icoSize = GetFileSize(hIco, NULL);
    if (icoSize == 0 || icoSize > 1024 * 1024) { CloseHandle(hIco); return 0; }

    BYTE* icoBuf = (BYTE*)HeapAlloc(GetProcessHeap(), 0, icoSize);
    if (!icoBuf) { CloseHandle(hIco); return 0; }
    DWORD got = 0;
    ReadFile(hIco, icoBuf, icoSize, &got, NULL);
    CloseHandle(hIco);
    if (got != icoSize) { HeapFree(GetProcessHeap(), 0, icoBuf); return 0; }

    ICONDIR* dir = (ICONDIR*)icoBuf;
    if (dir->Reserved != 0 || dir->Type != 1 || dir->Count == 0 || dir->Count > 20) {
        HeapFree(GetProcessHeap(), 0, icoBuf);
        return 0;
    }
    int count = dir->Count;
    ICONDIRENTRY* entries = (ICONDIRENTRY*)(icoBuf + sizeof(ICONDIR));

    DWORD groupSize = 6 + sizeof(GRPICONDIRENTRY) * (DWORD)count;
    BYTE* groupData = (BYTE*)HeapAlloc(GetProcessHeap(), 0, groupSize);
    if (!groupData) { HeapFree(GetProcessHeap(), 0, icoBuf); return 0; }
    WORD* grp = (WORD*)groupData;
    grp[0] = 0; grp[1] = 1; grp[2] = (WORD)count;
    GRPICONDIRENTRY* ge = (GRPICONDIRENTRY*)(groupData + 6);

    char tmp_path[MAX_PATH];
    path_join(tmp_path, g_exe_path, ".tmp");

    if (!CopyFileA(g_exe_path, tmp_path, FALSE)) {
        HeapFree(GetProcessHeap(), 0, icoBuf);
        HeapFree(GetProcessHeap(), 0, groupData);
        return 0;
    }
    HANDLE hU = BeginUpdateResourceA(tmp_path, FALSE);
    if (!hU) {
        DeleteFileA(tmp_path);
        HeapFree(GetProcessHeap(), 0, icoBuf);
        HeapFree(GetProcessHeap(), 0, groupData);
        return 0;
    }
    int ok = 1;
    for (int i = 0; i < count; i++) {
        DWORD off = entries[i].ImageOffset;
        DWORD sz  = entries[i].BytesInRes;
        if (off + sz > icoSize) { ok = 0; break; }
        if (!UpdateResourceA(hU, RT_ICON, MAKEINTRESOURCEA(i + 1),
                             MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                             icoBuf + off, sz)) { ok = 0; break; }
        ge[i].ID         = (WORD)(i + 1);
        ge[i].Width      = entries[i].Width;
        ge[i].Height     = entries[i].Height;
        ge[i].ColorCount = entries[i].ColorCount;
        ge[i].Reserved   = entries[i].Reserved;
        ge[i].Planes     = entries[i].Planes;
        ge[i].BitCount   = entries[i].BitCount;
        ge[i].BytesInRes = entries[i].BytesInRes;
    }
    if (ok) {
        if (!UpdateResourceA(hU, RT_GROUP_ICON, "MAINICON",
                             MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                             groupData, groupSize)) ok = 0;
    }
    if (!EndUpdateResourceA(hU, !ok)) { if (ok) ok = 0; }
    HeapFree(GetProcessHeap(), 0, icoBuf);
    HeapFree(GetProcessHeap(), 0, groupData);
    if (!ok) { DeleteFileA(tmp_path); return 0; }

    if (!MoveFileExA(tmp_path, g_exe_path,
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT)) {
        char bat_path[MAX_PATH];
        path_join(bat_path, g_game_dir, "update_icon.bat");
        HANDLE hB = CreateFileA(bat_path, GENERIC_WRITE, 0,
                                NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hB != INVALID_HANDLE_VALUE) {
            char bat[1024];
            nc_snprintf(bat, sizeof(bat),
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
                tmp_path, g_exe_path);
            DWORD wrote = 0;
            WriteFile(hB, bat, (DWORD)nc_strlen(bat), &wrote, NULL);
            CloseHandle(hB);
        }
    }
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Per-tick apply (window title + runtime icon)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void apply_window_name(HWND hwnd) {
    char cur[256];
    cur[0] = '\0';
    GetWindowTextA(hwnd, cur, sizeof(cur));
    if (nc_strlen(cur) != nc_strlen(g_window_name) || lstrcmpA(cur, g_window_name) != 0)
        SetWindowTextA(hwnd, g_window_name);
}

static void apply_custom_icon(HWND hwnd) {
    if (g_icon_applied || !g_icon_found) return;

    HICON big = NULL, small = NULL;
    /* PrivateExtractIcons handles PNG-compressed .ico (user32) */
    if (PrivateExtractIconsA(g_icon_path, 0, 48, 48, &big, NULL, 1, 0) || big) {
        PrivateExtractIconsA(g_icon_path, 0, 16, 16, &small, NULL, 1, 0);
        SendMessageA(hwnd, WM_SETICON, ICON_BIG, (LPARAM)big);
        if (small) SendMessageA(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)small);
        g_icon_applied = 1;
        return;
    }
    /* Fallback: LoadImageA for BMP-format .ico */
    big = (HICON)LoadImageA(NULL, g_icon_path, IMAGE_ICON, 48, 48, LR_LOADFROMFILE);
    if (big) {
        small = (HICON)LoadImageA(NULL, g_icon_path, IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
        SendMessageA(hwnd, WM_SETICON, ICON_BIG, (LPARAM)big);
        if (small) SendMessageA(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)small);
        g_icon_applied = 1;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HB+ callback wiring (17-entry vtable, MinGW __thiscall convention)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) operator delete(thisptr);
    return thisptr;
}
static const char* __thiscall get_mod_name(void*) { return "_mknp_icon_winname"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, IModAPI* api) {
    g_api = api;
    *(void**)((char*)thisptr + 4) = api;
    mod_self();

    /* DLL dir -> config lives next to this DLL (e.g. Mods\) */
    g_dll_dir[0] = '\0';
    if (g_module && GetModuleFileNameA(g_module, g_dll_dir, MAX_PATH) > 0) {
        char* s = NULL;
        for (char* p = g_dll_dir; *p; p++) if (*p == '\\') s = p;
        if (s) s[1] = '\0';
        else g_dll_dir[0] = '\0';
    }
    if (g_dll_dir[0])
        path_join(g_config_path, g_dll_dir, "_mknp_icon_winname.txt");
    else
        nc_strncpy(g_config_path, "_mknp_icon_winname.txt", MAX_PATH - 1);

    /* Game root (exe side) -> icon.ico / state / exe patching */
    GetModuleFileNameA(NULL, g_exe_path, MAX_PATH);
    nc_strncpy(g_game_dir, g_exe_path, MAX_PATH - 1);
    {
        char* s = NULL;
        for (char* p = g_game_dir; *p; p++) if (*p == '\\') s = p;
        if (s) s[1] = '\0';
        else g_game_dir[0] = '\0';
    }
    path_join(g_state_path, g_game_dir, ".icon_state.txt");
    path_join(g_icon_path, g_game_dir, "icon.ico");
    DWORD attr = GetFileAttributesA(g_icon_path);
    g_icon_found = (attr != INVALID_FILE_ATTRIBUTES &&
                    !(attr & FILE_ATTRIBUTE_DIRECTORY));

    load_config();
    g_started = 1;
}

static void __thiscall game_update(void*) {
    if (!g_started) return;

    /* One-shot: permanent .exe icon patch */
    if (!g_exe_icon_tried) {
        g_exe_icon_tried = 1;
        if (g_icon_found && !icon_already_written()) {
            if (update_exe_icon())
                save_icon_state();
        }
    }

    HWND hwnd = game_hwnd();
    if (!hwnd) return;

    /* window_name from _mknp_icon_winname.txt (DLL dir) */
    apply_window_name(hwnd);
    apply_custom_icon(hwnd);
}

static void __thiscall ball_update(void*, void*) {}
static void __thiscall render_apply(void*, void*, float*) {}
static void __thiscall button_toggle(void*, const char*, bool) {}
static void __thiscall slider_change(void*, const char*, float) {}
static void __thiscall cycle_option_change(void*, const char*, const char*) {}
static void __thiscall event_collide(void*, void*, char*) {}
static void __thiscall text_render(void*) {}
static void __thiscall ball_bump(void*, void*, void*) {}
static void __thiscall scene_end(void*) {}
static void __thiscall level_start(void*) {}

static void* g_vtable[17] = {
    (void*)sc_dtor,              // [0]  ~HamsterballAPI
    (void*)get_mod_name,         // [1]  GetModName
    (void*)get_author,           // [2]  GetAuthorName
    (void*)get_version,          // [3]  GetApiVersion
    (void*)get_contributors,     // [4]  GetContributors
    (void*)init_impl,            // [5]  Initialize
    (void*)ball_update,          // [6]  onBallUpdate
    (void*)render_apply,         // [7]  onRenderApply
    (void*)button_toggle,        // [8]  onButtonToggle
    (void*)slider_change,        // [9]  onSliderChange
    (void*)cycle_option_change,  // [10] onCycleOptionChange (v2.0+)
    (void*)game_update,          // [11] onGameUpdate
    (void*)event_collide,        // [12] onEventPlaneCollide
    (void*)text_render,          // [13] onTextRenderLoop
    (void*)ball_bump,            // [14] onBallBump
    (void*)scene_end,            // [15] onSceneEnd
    (void*)level_start,          // [16] onLevelStart
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = operator new(8);
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}

/* DllMain: loader-lock rules — DisableThreadLibraryCalls ONLY. */
extern "C" BOOL __stdcall DllMain(HINSTANCE h, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH)
        DisableThreadLibraryCalls(h);
    return TRUE;
}
