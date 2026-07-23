/*
 * IconAndWindowName_MinGW.cpp — MinGW HB+ port of icon_and_windowname bass.dll proxy mod.
 *
 * Features:
 *   1. Custom window title — replaces "Hamsterball" window name
 *   2. Custom icon — searches for icon.ico in the game root folder
 *      If found: replaces runtime window icon
 *      If not found: game uses original icon
 *
 * Config file: mkn_plus_icon_and_windowname.txt (auto-generated next to DLL)
 *
 * Uses nocrt + manual 17-entry vtable for HB+ v2.1 compatibility.
 * The original mod used a background thread to poll for the window handle.
 * In HB+ we use onGameUpdate() for the polling instead.
 */
#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

#define GAME_BASE         0x00400000
#define APP_PTR_ADDR      (GAME_BASE + 0x001341E0)  /* 0x005341E0 */

static char g_configPath[MAX_PATH] = {0};
static char g_gameDir[MAX_PATH] = {0};     /* game root folder */
static char g_iconPath[MAX_PATH] = {0};     /* path to icon.ico in game dir */
static char g_windowName[256] = "Hamsterball";
static bool g_iconFound = false;            /* icon.ico exists */
static bool g_iconApplied = false;          /* runtime WM_SETICON done */
static bool g_windowNameApplied = false;    /* SetWindowText done */
static bool g_configLoaded = false;
static void* g_storedApi = NULL;

/* ── Config path via VirtualQuery ─────────────────────────────────── */

static void buildConfigPath(void) {
    MEMORY_BASIC_INFORMATION mbi;
    char dllPath[MAX_PATH];
    if (VirtualQuery((void*)buildConfigPath, &mbi, sizeof(mbi)) > 0) {
        HMODULE hMod = (HMODULE)mbi.AllocationBase;
        if (hMod && GetModuleFileNameA(hMod, dllPath, MAX_PATH) > 0) {
            char* last = NULL;
            char* p = dllPath;
            while (*p) {
                if (*p == '\\' || *p == '/') last = p;
                p++;
            }
            if (last) {
                *(last + 1) = '\0';
                nc_strncpy(g_configPath, dllPath, MAX_PATH - 1);
                nc_strncpy(g_configPath + nc_strlen(g_configPath),
                           "mkn_plus_icon_and_windowname.txt",
                           MAX_PATH - nc_strlen(g_configPath) - 1);
                g_configPath[MAX_PATH - 1] = '\0';

                /* Also derive game root from Hamsterball.exe module */
                HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
                if (!hExe) hExe = GetModuleHandleA(NULL);
                char exePath[MAX_PATH];
                if (GetModuleFileNameA(hExe, exePath, MAX_PATH) > 0) {
                    nc_strncpy(g_gameDir, exePath, MAX_PATH - 1);
                    char* p2 = g_gameDir;
                    char* last2 = NULL;
                    while (*p2) {
                        if (*p2 == '\\' || *p2 == '/') last2 = p2;
                        p2++;
                    }
                    if (last2) {
                        *(last2 + 1) = '\0';
                    } else {
                        g_gameDir[0] = '\0';
                    }
                }

                /* Icon file: search for icon.ico in game root */
                nc_strncpy(g_iconPath, g_gameDir, MAX_PATH - 1);
                nc_strncpy(g_iconPath + nc_strlen(g_iconPath),
                           "icon.ico",
                           MAX_PATH - nc_strlen(g_iconPath) - 1);
                g_iconPath[MAX_PATH - 1] = '\0';

                DWORD attr = GetFileAttributesA(g_iconPath);
                g_iconFound = (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));

                return;
            }
        }
    }
    nc_strncpy(g_configPath, "mkn_plus_icon_and_windowname.txt", MAX_PATH - 1);
    g_configPath[MAX_PATH - 1] = '\0';
}

/* ── Config parsing ────────────────────────────────────────────────── */

static int nc_stricmp(const char* a, const char* b) {
    while (*a && *b) {
        char ca = *a;
        char cb = *b;
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return (unsigned char)ca - (unsigned char)cb;
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

static void trimInPlace(char* s) {
    /* Trim trailing */
    int len = (int)nc_strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t' ||
                       s[len-1] == '\n' || s[len-1] == '\r')) {
        s[--len] = '\0';
    }
    /* Trim leading */
    char* p = s;
    while (*p == ' ' || *p == '\t') p++;
    if (p != s) {
        size_t remain = nc_strlen(p);
        nc_memcpy(s, p, remain + 1);
    }
}

static void parseConfigLine(char *line) {
    char *p = line;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '#' || *p == ';' || *p == '\0' || *p == '\n' || *p == '\r')
        return;

    /* Find '=' */
    char *eq = p;
    while (*eq && *eq != '=') eq++;
    if (!*eq) return;
    *eq = '\0';

    char *key = p;
    char *val = eq + 1;

    /* Trim key */
    int klen = (int)nc_strlen(key);
    while (klen > 0 && (key[klen-1] == ' ' || key[klen-1] == '\t')) {
        key[--klen] = '\0';
    }

    /* Trim val */
    while (*val == ' ' || *val == '\t') val++;
    trimInPlace(val);

    if (nc_stricmp(key, "window_name") == 0) {
        nc_strncpy(g_windowName, val, sizeof(g_windowName) - 1);
        g_windowName[sizeof(g_windowName) - 1] = '\0';
    }
}

static void writeString(HANDLE h, const char* s) {
    DWORD written;
    WriteFile(h, s, (DWORD)nc_strlen(s), &written, NULL);
}

static void loadConfig(void) {
    if (g_configLoaded) return;
    g_configLoaded = true;

    HANDLE hFile = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        /* Auto-generate default config */
        hFile = CreateFileA(g_configPath, GENERIC_WRITE, 0,
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
            writeString(hFile, defaults);
            CloseHandle(hFile);
        }
        return;
    }

    char buf[4096];
    DWORD bytesRead = 0;
    ReadFile(hFile, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(hFile);
    buf[bytesRead] = '\0';

    /* Process line by line */
    char *line = buf;
    while (line && *line) {
        char *next = line;
        while (*next && *next != '\n') next++;
        if (*next) {
            *next = '\0';
            parseConfigLine(line);
            line = next + 1;
        } else {
            parseConfigLine(line);
            break;
        }
    }
}

/* ── Runtime window icon ───────────────────────────────────────────── */

static void applyCustomIcon(void) {
    if (g_iconApplied || !g_iconFound)
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
    if (PrivateExtractIcons(g_iconPath, 0, 48, 48, &hIconBig, NULL, 1, 0) || hIconBig) {
        PrivateExtractIcons(g_iconPath, 0, 16, 16, &hIconSmall, NULL, 1, 0);
        SendMessageA(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
        if (hIconSmall)
            SendMessageA(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
        g_iconApplied = true;
        return;
    }

    /* Fallback: LoadImageA for BMP-format .ico */
    hIconBig = (HICON)LoadImageA(NULL, g_iconPath, IMAGE_ICON, 48, 48, LR_LOADFROMFILE);
    if (hIconBig) {
        hIconSmall = (HICON)LoadImageA(NULL, g_iconPath, IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
        SendMessageA(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
        if (hIconSmall)
            SendMessageA(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
        g_iconApplied = true;
        return;
    }
}

static void applyWindowName(void) {
    if (g_windowNameApplied)
        return;

    DWORD *appPtr = (DWORD *)APP_PTR_ADDR;
    if (*appPtr == 0)
        return;

    DWORD app = *appPtr;
    HWND hwnd = (HWND)(*(DWORD *)(app + 0x08));
    if (hwnd == NULL || !IsWindow(hwnd))
        return;

    SetWindowTextA(hwnd, g_windowName);
    g_windowNameApplied = true;
}

/* ── HB+ vtable ───────────────────────────────────────────────────── */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Icon & Window Name"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Ported from bass.dll proxy"; }

static void __thiscall init_impl(void* thisptr, void* modApi) {
    *(void**)((char*)thisptr + 4) = modApi;
    g_storedApi = modApi;
    buildConfigPath();
    loadConfig();
}

/* In HB+, we use onGameUpdate for the polling that the original mod
 * did in a background thread. This is called every frame. */
static void __thiscall game_update_impl(void* thisptr) {
    if (!g_configLoaded) return;
    applyCustomIcon();
    applyWindowName();
}

/* No-op implementations for unused callbacks */
static void __thiscall ball_update_impl(void*, void*) {}
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall button_toggle_impl(void*, const char*, bool) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall event_collide_impl(void*, void*, char*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}
static void __thiscall level_start_impl(void*) {}
static void __thiscall scene_end_impl(void*) {}

static void* g_vtable[17] = {
    (void*)sc_dtor,              // [0]
    (void*)get_mod_name,         // [1]
    (void*)get_author,           // [2]
    (void*)get_version,          // [3]
    (void*)get_contributors,     // [4]
    (void*)init_impl,            // [5]
    (void*)ball_update_impl,     // [6]
    (void*)render_apply_impl,    // [7]
    (void*)button_toggle_impl,   // [8]
    (void*)slider_change_impl,   // [9]
    (void*)cycle_change_impl,    // [10]
    (void*)game_update_impl,     // [11]
    (void*)event_collide_impl,   // [12]
    (void*)text_render_impl,     // [13]
    (void*)ball_bump_impl,       // [14]
    (void*)scene_end_impl,       // [15]
    (void*)level_start_impl,     // [16]
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = nc_malloc(8);
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}
