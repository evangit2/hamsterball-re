/*
 * mknp_separate_music_MinGW.cpp — Separate Music (HB+ v2.1, MinGW) v01b
 *
 * One .mo3 file per song instead of everything in Music.mo3.
 * Map any game song (e.g. "Fight!") to its own file (e.g. Level8.mo3).
 *
 * How it works:
 *   Music.mo3 holds all songs as orders. Audio_PlayMusic (0x46A310) looks
 *   up the track NAME and plays its HEX order via BASS_MusicPlayEx.
 *   Audio_PlayMusicAtSpeed (0x46A440) calls Audio_PlayMusic internally,
 *   so one hook covers both.
 *   Entry bytes at 0x46A310: 53 55 8B E9 56 (5 bytes, no split) -> JMP cave.
 *   The cave calls music_play_handler(channelObj, trackName):
 *     file mapped + exists -> stop game handle, load + play custom file,
 *     skip original (RET 8). Else run original.
 *   BASS reached via the loader's own exports (GetModuleHandleA("bass.dll")).
 *
 * Config (next to DLL in Mods\, auto-generated, same format as v01a):
 *   "Track Name = file.mo3"  (empty value = use original Music.mo3)
 * Files live in the game's Music\ folder. Missing file = original plays.
 */
#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

/* ═══════════════════════════════════════════════════════════════════════════
 * Constants
 * ═══════════════════════════════════════════════════════════════════════════ */

#define PLAY_MUSIC_ADDR  0x46A310
#define PLAY_MUSIC_CONT  0x46A315  /* address after patched 5 bytes */
// Expected entry bytes (verify before patching, never blind-patch)
#define EXP_B0 0x53
#define EXP_B1 0x55
#define EXP_B2 0x8B
#define EXP_B3 0xE9
#define EXP_B4 0x56

/* ═══════════════════════════════════════════════════════════════════════════
 * Track table — from Jukebox.xml (NAME -> default file)
 * Lookup is by NAME directly (the hook receives the name string).
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    const char* name;
    const char* deffile;   /* default file for generated config ("" = original) */
    char file[128];        /* active mapping ("" = original) */
} track_entry_t;

static track_entry_t g_tracks[] = {
    {"Main Theme",                "", ""},
    {"Main Theme - No Intro",     "", ""},
    {"Cascade Race",              "Level1.mo3", ""},
    {"Cascade Race - No Intro",   "Level1.mo3", ""},
    {"Hamster Nation",            "Level2.mo3", ""},
    {"Hamster Nation - No Intro", "Level2.mo3", ""},
    {"Gerbil Groove",             "Level3.mo3", ""},
    {"Gerbil Groove - No Intro",  "Level3.mo3", ""},
    {"Dizzy!",                    "Level4.mo3", ""},
    {"Dizzy! - No Intro",         "Level4.mo3", ""},
    {"Happy Rush",                "Level5.mo3", ""},
    {"Happy Rush - No Intro",     "Level5.mo3", ""},
    {"Up Race",                   "Level6.mo3", ""},
    {"Up Race - No Intro",        "Level6.mo3", ""},
    {"Neon Theme",                "Level7.mo3", ""},
    {"Fight!",                    "Level8.mo3", ""},
    {"Fight! - No Intro",         "Level8.mo3", ""},
    {"Ninja Hamster",             "Level9.mo3", ""},
    {"Ninja Hamster - No Intro",  "Level9.mo3", ""},
    {"Rodenthood",                "Level10.mo3", ""},
    {"Rodenthood - No Intro",     "Level10.mo3", ""},
    {"Hamster Chase",             "Level11.mo3", ""},
    {"Hamster Chase - No Intro",  "Level11.mo3", ""},
    {"Glass Theme",               "Level12.mo3", ""},
    {"Bucky Break",               "Level13.mo3", ""},
    {"Bucky Break - No Intro",    "Level13.mo3", ""},
    {"Master Theme",              "Level14.mo3", ""},
    {"Master Theme - No Intro",   "Level14.mo3", ""},
    {"Impossible Theme",          "Level15.mo3", ""},
    {"Game Over",                 "", ""},
    {"Tournament",                "", ""},
    {"Goal!",                     "", ""},
    {"High Scores",               "", ""},
    {"Loading",                   "", ""},
};
#define NUM_TRACKS (sizeof(g_tracks) / sizeof(g_tracks[0]))

/* ═══════════════════════════════════════════════════════════════════════════
 * Global state
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* g_storedApi = NULL;
static char g_dllDir[MAX_PATH] = "";
static char g_exeDir[MAX_PATH] = "";
static char g_configPath[MAX_PATH] = "";
static char g_logPath[MAX_PATH] = "";
static bool g_started = false;
static bool g_hookInstalled = false;
static BYTE* g_cave = NULL;
static DWORD g_customMusic = 0;  /* HMUSIC of active custom file (0 = none) */

/* BASS via loader exports */
typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);
typedef int (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, int, DWORD);
typedef int (__stdcall *BASS_ChannelStop_t)(DWORD);
typedef int (__stdcall *BASS_MusicFree_t)(DWORD);
typedef int (__stdcall *BASS_ChannelSetAttr_t)(DWORD, DWORD, int, int);
static BASS_MusicLoad_t pMusicLoad = NULL;
static BASS_MusicPlayEx_t pMusicPlayEx = NULL;
static BASS_ChannelStop_t pChannelStop = NULL;
static BASS_MusicFree_t pMusicFree = NULL;
static BASS_ChannelSetAttr_t pChanAttr = NULL;

/* ═══════════════════════════════════════════════════════════════════════════
 * Log (append, next to DLL)
 * ═══════════════════════════════════════════════════════════════════════════ */

static char g_logBuf[512];

static void mlog(const char* msg) {
    HANDLE h;
    DWORD written = 0;
    size_t n;
    if (!g_logPath[0]) return;
    h = CreateFileA(g_logPath, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                    OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    SetFilePointer(h, 0, NULL, FILE_END);
    n = nc_strlen(msg);
    WriteFile(h, msg, (DWORD)n, &written, NULL);
    WriteFile(h, "\r\n", 2, &written, NULL);
    CloseHandle(h);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Paths
 * ═══════════════════════════════════════════════════════════════════════════ */

static void split_dir(char* path) {
    char* last = NULL;
    char* p = path;
    while (*p) {
        if (*p == '\\' || *p == '/') last = p;
        p++;
    }
    if (last) *(last + 1) = '\0';
}

static void buildPaths(void) {
    MEMORY_BASIC_INFORMATION mbi;
    char dllPath[MAX_PATH];
    char exePath[MAX_PATH];
    if (VirtualQuery((void*)buildPaths, &mbi, sizeof(mbi)) > 0) {
        HMODULE hMod = (HMODULE)mbi.AllocationBase;
        if (hMod && GetModuleFileNameA(hMod, dllPath, MAX_PATH) > 0) {
            nc_strncpy(g_dllDir, dllPath, MAX_PATH - 1);
            split_dir(g_dllDir);
        }
    }
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) > 0) {
        nc_strncpy(g_exeDir, exePath, MAX_PATH - 1);
        split_dir(g_exeDir);
    }
    nc_snprintf(g_configPath, MAX_PATH, "%smknp_separate_music.txt", g_dllDir);
    g_configPath[MAX_PATH - 1] = '\0';
    nc_snprintf(g_logPath, MAX_PATH, "%sseparate_music.log", g_dllDir);
    g_logPath[MAX_PATH - 1] = '\0';
}

static int file_exists(const char* path) {
    DWORD a = GetFileAttributesA(path);
    return (a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY));
}

/* find "Music\<file>" under exe dir, else next to DLL. full = result. */
static int build_music_path(const char* file, char* full) {
    nc_snprintf(full, MAX_PATH, "%sMusic\\%s", g_exeDir, file);
    full[MAX_PATH - 1] = '\0';
    if (file_exists(full)) return 1;
    nc_snprintf(full, MAX_PATH, "%s%s", g_dllDir, file);
    full[MAX_PATH - 1] = '\0';
    if (file_exists(full)) return 1;
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config — "Track Name = file" (empty = original). Reset each parse so
 * removals revert to original. Bad lines ignored (keep-old-on-error).
 * ═══════════════════════════════════════════════════════════════════════════ */

static char g_cfgBuf[8192];

static void readConfig(void) {
    HANDLE h;
    DWORD bytesRead = 0;
    char* p;
    size_t i;

    for (i = 0; i < NUM_TRACKS; i++) g_tracks[i].file[0] = '\0';
    if (!g_configPath[0]) return;
    h = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    ReadFile(h, g_cfgBuf, sizeof(g_cfgBuf) - 1, &bytesRead, NULL);
    CloseHandle(h);
    if (bytesRead == 0) return;
    g_cfgBuf[bytesRead] = '\0';

    p = g_cfgBuf;
    if ((unsigned char)p[0] == 0xEF && (unsigned char)p[1] == 0xBB &&
        (unsigned char)p[2] == 0xBF) p += 3;

    while (*p) {
        char nameBuf[64];
        char valBuf[128];
        int ni = 0, vi = 0;
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        if (*p == '\0') break;
        if (*p == ';' || *p == '#') {
            while (*p && *p != '\n') p++;
            continue;
        }
        while (*p && *p != '=' && *p != '\n' && *p != '\r' && ni < 63) {
            if (*p != ' ' && *p != '\t') nameBuf[ni++] = *p;
            p++;
        }
        nameBuf[ni] = '\0';
        if (*p != '=') { while (*p && *p != '\n') p++; continue; }
        p++;
        while (*p == ' ' || *p == '\t') p++;
        while (*p && *p != '\n' && *p != '\r' && vi < 127) valBuf[vi++] = *p++;
        valBuf[vi] = '\0';
        while (vi > 0 && (valBuf[vi-1] == ' ' || valBuf[vi-1] == '\t'))
            valBuf[--vi] = '\0';
        if (ni == 0) continue;
        for (i = 0; i < NUM_TRACKS; i++) {
            if (nc_strcmp(nameBuf, g_tracks[i].name) == 0) {
                if (vi > 0) {
                    nc_strncpy(g_tracks[i].file, valBuf, 127);
                    g_tracks[i].file[127] = '\0';
                }
                break;
            }
        }
    }
}

static void generateConfig(const char* path) {
    HANDLE h;
    DWORD written = 0;
    char buf[8192];
    int pos = 0;
    size_t i;
    h = CreateFileA(path, GENERIC_WRITE, 0, NULL,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
        "; Separate Music v01b (HB+) - one file per song\r\n"
        "; Usage: Track Name = file  (empty = play original Music.mo3)\r\n"
        "; Files live in the game's Music\\ folder.\r\n"
        "; Missing file = original plays (safe).\r\n"
        ";\r\n; Races (defaults Level1..Level15):\r\n");
    for (i = 0; i < NUM_TRACKS; i++) {
        if (g_tracks[i].deffile[0])
            pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
                "%s = %s\r\n", g_tracks[i].name, g_tracks[i].deffile);
    }
    pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
        ";\r\n; Menus (empty = original):\r\n");
    for (i = 0; i < NUM_TRACKS; i++) {
        if (!g_tracks[i].deffile[0])
            pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
                "%s =\r\n", g_tracks[i].name);
    }
    if (pos > 0) WriteFile(h, buf, pos, &written, NULL);
    CloseHandle(h);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook handler — called from cave as handler(channelObj, trackName).
 * Returns 1 if custom music started (skip original), 0 to run original.
 * ═══════════════════════════════════════════════════════════════════════════ */

static char g_hookName[64];
static char g_hookFull[MAX_PATH];

static int __cdecl music_play_handler(DWORD channelObj, const char* trackName) {
    const char* want = NULL;
    DWORD gameH = 0;
    DWORD nh;
    size_t i, n;

    if (!trackName || IsBadReadPtr((void*)trackName, 8)) return 0;
    if (channelObj < 0x10000 || IsBadReadPtr((void*)(channelObj + 8), 4)) return 0;
    if (!pMusicLoad || !pMusicPlayEx) return 0;

    /* bounded name copy */
    for (n = 0; n < sizeof(g_hookName) - 1; n++) {
        char c;
        if (IsBadReadPtr((void*)(trackName + n), 1)) break;
        c = trackName[n];
        g_hookName[n] = c;
        if (!c) break;
    }
    g_hookName[n] = '\0';
    if (!g_hookName[0]) return 0;

    for (i = 0; i < NUM_TRACKS; i++) {
        if (nc_strcmp(g_hookName, g_tracks[i].name) == 0) {
            if (g_tracks[i].file[0]) want = g_tracks[i].file;
            break;
        }
    }
    if (!want) return 0; /* no mapping -> original */
    if (!build_music_path(want, g_hookFull)) return 0; /* missing -> original */

    gameH = *(DWORD*)(channelObj + 8); /* HMUSIC of Music.mo3 */
    if (pChannelStop) {
        if (gameH) pChannelStop(gameH);
        if (g_customMusic) pChannelStop(g_customMusic);
    }
    if (g_customMusic && pMusicFree) {
        pMusicFree(g_customMusic);
        g_customMusic = 0;
    }
    nh = pMusicLoad(0, g_hookFull, 0, 0, 4, 0);
    if (!nh) {
        nc_snprintf(g_logBuf, sizeof(g_logBuf),
            "FAIL load (%s) file=%s", g_hookName, want);
        mlog(g_logBuf);
        return 0; /* original restarts game handle */
    }
    if (pChanAttr) pChanAttr(nh, (DWORD)-1, 100, -101);
    {
        int rc = pMusicPlayEx(nh, 0, 1, 1);
        (void)rc;
    }
    g_customMusic = nh;
    nc_snprintf(g_logBuf, sizeof(g_logBuf),
        "PLAY custom (%s) file=%s", g_hookName, want);
    mlog(g_logBuf);
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook install — JMP at 0x46A310 to cave. Idempotent + byte-verified.
 *
 * Cave (38 bytes):
 *   PUSHAD
 *   MOV EAX,[ESP+36]   ; trackName (orig ESP+4)
 *   PUSH EAX
 *   MOV EAX,[ESP+28]   ; channelObj (orig ECX via PUSHAD)
 *   PUSH EAX
 *   CALL handler
 *   ADD ESP,8
 *   TEST EAX,EAX
 *   JZ not_handled
 *   POPAD
 *   RET 8              ; skip original (callee cleans 2 params)
 * not_handled:
 *   POPAD
 *   PUSH EBX / PUSH EBP / MOV EBP,ECX / PUSH ESI   ; stolen bytes
 *   JMP 0x46A315
 * ═══════════════════════════════════════════════════════════════════════════ */

static void install_hook(void) {
    DWORD target = PLAY_MUSIC_ADDR;
    DWORD old_prot;
    int pos = 0;
    BYTE* c;

    if (g_hookInstalled && g_cave) {
        if (*(BYTE*)target == 0xE9 &&
            *(DWORD*)(target + 1) == (DWORD)g_cave - (target + 5))
            return; /* already ours */
    }
    /* verify expected bytes — never blind-patch */
    if (IsBadReadPtr((void*)target, 5)) {
        mlog("hook: exe not mapped, skip");
        return;
    }
    if (*(BYTE*)(target) != EXP_B0 || *(BYTE*)(target + 1) != EXP_B1 ||
        *(BYTE*)(target + 2) != EXP_B2 || *(BYTE*)(target + 3) != EXP_B3 ||
        *(BYTE*)(target + 4) != EXP_B4) {
        mlog("hook: entry bytes mismatch, NOT patching");
        return;
    }

    c = (BYTE*)VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE,
                            PAGE_EXECUTE_READWRITE);
    if (!c) return;
    g_cave = c;

    c[pos++] = 0x60;                          /* PUSHAD */
    c[pos++] = 0x8B; c[pos++] = 0x44; c[pos++] = 0x24; c[pos++] = 0x24; /* MOV EAX,[ESP+36] */
    c[pos++] = 0x50;                          /* PUSH EAX (trackName) */
    c[pos++] = 0x8B; c[pos++] = 0x44; c[pos++] = 0x24; c[pos++] = 0x1C; /* MOV EAX,[ESP+28] */
    c[pos++] = 0x50;                          /* PUSH EAX (channelObj) */
    c[pos++] = 0xE8;                          /* CALL handler */
    {
        DWORD rel = (DWORD)&music_play_handler - (DWORD)(c + pos + 4);
        *(DWORD*)(c + pos) = rel;
        pos += 4;
    }
    c[pos++] = 0x83; c[pos++] = 0xC4; c[pos++] = 0x08; /* ADD ESP,8 */
    c[pos++] = 0x85; c[pos++] = 0xC0;                 /* TEST EAX,EAX */
    c[pos++] = 0x74; c[pos++] = 0x04;                 /* JZ +4 -> not_handled */
    c[pos++] = 0x61;                          /* POPAD */
    c[pos++] = 0xC2; c[pos++] = 0x08; c[pos++] = 0x00; /* RET 8 */
    /* not_handled: */
    c[pos++] = 0x61;                          /* POPAD */
    c[pos++] = 0x53;                          /* PUSH EBX */
    c[pos++] = 0x55;                          /* PUSH EBP */
    c[pos++] = 0x8B; c[pos++] = 0xE9;         /* MOV EBP,ECX */
    c[pos++] = 0x56;                          /* PUSH ESI */
    c[pos++] = 0xE9;                          /* JMP PLAY_MUSIC_CONT */
    {
        DWORD rel = (DWORD)PLAY_MUSIC_CONT - (DWORD)(c + pos + 4);
        *(DWORD*)(c + pos) = rel;
        pos += 4;
    }

    VirtualProtect((void*)target, 5, PAGE_EXECUTE_READWRITE, &old_prot);
    *(BYTE*)(target) = 0xE9;
    *(DWORD*)(target + 1) = (DWORD)c - (target + 5);
    VirtualProtect((void*)target, 5, old_prot, &old_prot);
    FlushInstructionCache(GetCurrentProcess(), (void*)target, 5);

    g_hookInstalled = true;
    mlog("hook installed at Audio_PlayMusic");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS resolution via loader exports
 * ═══════════════════════════════════════════════════════════════════════════ */

static void resolve_bass(void) {
    HMODULE hBass = GetModuleHandleA("bass.dll");
    if (!hBass) hBass = GetModuleHandleA("BASS.dll");
    if (!hBass) return;
    pMusicLoad = (BASS_MusicLoad_t)GetProcAddress(hBass, "BASS_MusicLoad");
    pMusicPlayEx = (BASS_MusicPlayEx_t)GetProcAddress(hBass, "BASS_MusicPlayEx");
    pChannelStop = (BASS_ChannelStop_t)GetProcAddress(hBass, "BASS_ChannelStop");
    pMusicFree = (BASS_MusicFree_t)GetProcAddress(hBass, "BASS_MusicFree");
    pChanAttr = (BASS_ChannelSetAttr_t)GetProcAddress(hBass, "BASS_ChannelSetAttributes");
    if (!pMusicLoad)
        pMusicLoad = (BASS_MusicLoad_t)GetProcAddress(hBass, "_BASS_MusicLoad@24");
    if (!pMusicPlayEx)
        pMusicPlayEx = (BASS_MusicPlayEx_t)GetProcAddress(hBass, "_BASS_MusicPlayEx@16");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HB+ vtable implementation (17 entries, v2.1)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Separate Music"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, void* modApi) {
    HANDLE h;
    *(void**)((char*)thisptr + 4) = modApi;
    g_storedApi = modApi;

    buildPaths();
    mlog("=== Separate Music v01b (HB+) Started ===");

    if (g_configPath[0]) {
        h = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h == INVALID_HANDLE_VALUE) {
            generateConfig(g_configPath);
            mlog("config generated");
        } else {
            CloseHandle(h);
        }
    }
    readConfig();
    resolve_bass();
    if (!pMusicLoad || !pMusicPlayEx)
        mlog("BASS fns missing, mod inert");
    install_hook();
    g_started = true;
}

static void __thiscall level_start_impl(void*) {
    readConfig(); /* live edit */
    g_started = true;
}

static void __thiscall scene_end_impl(void*) {
    if (g_customMusic) {
        if (pChannelStop) pChannelStop(g_customMusic);
        if (pMusicFree) pMusicFree(g_customMusic);
        g_customMusic = 0;
    }
    g_started = true;
}

static void __thiscall game_update_impl(void*) {
    static int tickCounter = 0;
    tickCounter++;
    if (tickCounter >= 120) {
        tickCounter = 0;
        readConfig();
    }
}

static void __thiscall ball_update_impl(void*, void*) {}
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall button_toggle_impl(void*, const char*, bool) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall event_collide_impl(void*, void*, char*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}

static void* g_vtable[17] = {
    (void*)sc_dtor,            // [0]
    (void*)get_mod_name,       // [1]
    (void*)get_author,         // [2]
    (void*)get_version,        // [3]
    (void*)get_contributors,   // [4]
    (void*)init_impl,          // [5]
    (void*)ball_update_impl,   // [6]
    (void*)render_apply_impl,  // [7]
    (void*)button_toggle_impl, // [8]
    (void*)slider_change_impl, // [9]
    (void*)cycle_change_impl,  // [10]
    (void*)game_update_impl,   // [11]
    (void*)event_collide_impl, // [12]
    (void*)text_render_impl,   // [13]
    (void*)ball_bump_impl,     // [14]
    (void*)scene_end_impl,     // [15]
    (void*)level_start_impl,   // [16]
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = nc_malloc(8);
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}
