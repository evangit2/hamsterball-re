/*
 * mknp_separate_music.c — Separate Music Mod v01a for Hamsterball
 *
 * One .mo3 file per song instead of everything in Music.mo3.
 * Map any game song (e.g. "Fight!") to its own file (e.g. Level8.mo3).
 *
 * How it works:
 *   Music.mo3 holds all songs as orders. Audio_PlayMusic (0x46A310) and
 *   Audio_PlayMusicAtSpeed (0x46A440) both end in BASS_MusicPlayEx with
 *   pos = HEX order from Jukebox.xml (e.g. Fight! = 0x08).
 *   This proxy sees that call, and if the HEX has a custom file in
 *   mknp_separate_music.txt, it loads that file via real BASS_MusicLoad
 *   and plays it instead. Otherwise it forwards untouched.
 *   No exe patching, no code caves.
 *
 * Config (next to DLL, auto-generated):
 *   "Track Name = file.mo3"  (empty value = use original Music.mo3)
 * Files live in the game's Music\ folder.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o mknp_separate_music.dll mknp_separate_music.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define MOD_VER "v01a"
#define MOD_NAME "mknp_separate_music"

static void mlog(const char* fmt, ...);

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Boilerplate
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;

typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, DWORD, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, DWORD b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, int, DWORD);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, int c, DWORD d);
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
    int r = 1;
    if (real_BASS_Init) r = real_BASS_Init(a, b, c, d, e);
    mlog("BASS_Init -> %d", r);
    return r;
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
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int a, void* b, DWORD c, DWORD d, DWORD e, DWORD f);
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a);
typedef int  (__stdcall *BASS_MusicFree_t)(DWORD);
static BASS_MusicFree_t real_BASS_MusicFree = NULL;
__declspec(dllexport) int __stdcall BASS_MusicFree(DWORD a) {
    int r = 1;
    if (real_BASS_MusicFree) r = real_BASS_MusicFree(a);
    return r;
}
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

static void load_real_bass(void) {
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (g_hRealBass == NULL) {
        char path[MAX_PATH];
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&load_real_bass, &hSelf);
        GetModuleFileNameA(hSelf, path, MAX_PATH);
        {
            char *p = strrchr(path, '\\');
            if (p) {
                strcpy(p + 1, "bass_real.dll");
                g_hRealBass = LoadLibraryA(path);
            }
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
        LOAD(BASS_MusicFree);
        #undef LOAD
        /* decorated-name fallback (some BASS builds export stdcall only as _Name@N) */
        if (!real_BASS_MusicLoad)
            real_BASS_MusicLoad = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "_BASS_MusicLoad@24");
        if (!real_BASS_MusicPlayEx)
            real_BASS_MusicPlayEx = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "_BASS_MusicPlayEx@16");
        if (!real_BASS_MusicFree)
            real_BASS_MusicFree = (BASS_MusicFree_t)GetProcAddress(g_hRealBass, "_BASS_MusicFree@4");
        if (!real_BASS_ChannelStop)
            real_BASS_ChannelStop = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "_BASS_ChannelStop@4");
        if (!real_BASS_ChannelSetAttributes)
            real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "_BASS_ChannelSetAttributes@16");
        if (!real_BASS_ErrorGetCode)
            real_BASS_ErrorGetCode = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "_BASS_ErrorGetCode@0");
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Track table — from Jukebox.xml (NAME -> HEX order in Music.mo3)
 * NOTE: pairs sharing one HEX share one slot (last config line wins).
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    const char* name;
    unsigned hex;
    const char* deffile;   /* default file for generated config ("" = original) */
} track_def_t;

static const track_def_t kTracks[] = {
    {"Main Theme",                0x02, ""},
    {"Main Theme - No Intro",     0x03, ""},
    {"Cascade Race",              0x7F, "Level1.mo3"},
    {"Cascade Race - No Intro",   0x81, "Level1.mo3"},
    {"Hamster Nation",            0x50, "Level2.mo3"},
    {"Hamster Nation - No Intro", 0x50, "Level2.mo3"},
    {"Gerbil Groove",             0x26, "Level3.mo3"},
    {"Gerbil Groove - No Intro",  0x28, "Level3.mo3"},
    {"Dizzy!",                    0x47, "Level4.mo3"},
    {"Dizzy! - No Intro",         0x47, "Level4.mo3"},
    {"Happy Rush",                0x2F, "Level5.mo3"},
    {"Happy Rush - No Intro",     0x2F, "Level5.mo3"},
    {"Up Race",                   0x8B, "Level6.mo3"},
    {"Up Race - No Intro",        0x8B, "Level6.mo3"},
    {"Neon Theme",                0xA8, "Level7.mo3"},
    {"Fight!",                    0x08, "Level8.mo3"},
    {"Fight! - No Intro",         0x09, "Level8.mo3"},
    {"Ninja Hamster",             0x38, "Level9.mo3"},
    {"Ninja Hamster - No Intro",  0x3A, "Level9.mo3"},
    {"Rodenthood",                0x55, "Level10.mo3"},
    {"Rodenthood - No Intro",     0x56, "Level10.mo3"},
    {"Hamster Chase",             0x19, "Level11.mo3"},
    {"Hamster Chase - No Intro",  0x1B, "Level11.mo3"},
    {"Glass Theme",               0x9F, "Level12.mo3"},
    {"Bucky Break",               0x5C, "Level13.mo3"},
    {"Bucky Break - No Intro",    0x5C, "Level13.mo3"},
    {"Master Theme",              0x71, "Level14.mo3"},
    {"Master Theme - No Intro",   0x71, "Level14.mo3"},
    {"Impossible Theme",          0x94, "Level15.mo3"},
    {"Game Over",                 0x62, ""},
    {"Tournament",                0x63, ""},
    {"Goal!",                     0x6B, ""},
    {"High Scores",               0x13, ""},
    {"Loading",                   0x6F, ""},
};
#define NUM_TRACKS (sizeof(kTracks) / sizeof(kTracks[0]))

/* hex (0..255) -> custom filename ("" = play original) + display name */
static char g_fileForHex[256][128];
static char g_nameForHex[256][64];

/* ═══════════════════════════════════════════════════════════════════════════
 * Paths + log
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE g_hSelf = NULL;
static char g_exeDir[MAX_PATH];
static char g_dllDir[MAX_PATH];
static char g_cfgPath[MAX_PATH];
static char g_logPath[MAX_PATH];

static DWORD g_gameMusic = 0;    /* HMUSIC of Music.mo3 (from game load) */
static DWORD g_customMusic = 0;  /* HMUSIC of active custom file (0 = none) */

static void mlog(const char* fmt, ...) {
    FILE* f = NULL;
    char msg[512];
    va_list ap;
    va_start(ap, fmt);
    _vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);
    msg[sizeof(msg) - 1] = '\0';
    if (fopen_s(&f, g_logPath, "a") != 0 || !f) return;
    fprintf(f, "%s\n", msg);
    fclose(f);
}

static void split_dir(const char* full, char* dir, DWORD dirlen) {
    strncpy(dir, full, dirlen - 1);
    dir[dirlen - 1] = '\0';
    {
        char* p = strrchr(dir, '\\');
        if (p) p[1] = '\0';
        else { dir[0] = '.'; dir[1] = '\\'; dir[2] = '\0'; }
    }
}

static int file_exists(const char* path) {
    DWORD a = GetFileAttributesA(path);
    return (a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY));
}

/* find "Music\<file>" under exe dir, else next to DLL. full = result. */
static int build_music_path(const char* file, char* full, DWORD len) {
    _snprintf(full, len, "%sMusic\\%s", g_exeDir, file);
    full[len - 1] = '\0';
    if (file_exists(full)) return 1;
    _snprintf(full, len, "%s%s", g_dllDir, file);
    full[len - 1] = '\0';
    if (file_exists(full)) return 1;
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Jukebox.xml — NAME -> HEX (overrides/adds to built-in table)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void parse_jukebox(void) {
    char path[MAX_PATH];
    FILE* f = NULL;
    static char buf[16384];
    size_t n;
    const char* p;
    int count = 0;

    _snprintf(path, sizeof(path), "%sJukebox.xml", g_exeDir);
    path[sizeof(path) - 1] = '\0';
    if (fopen_s(&f, path, "r") != 0 || !f) return;
    n = fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    buf[n] = '\0';

    p = buf;
    while ((p = strstr(p, "<NAME>")) != NULL) {
        const char* ne;
        const char* hs;
        const char* he;
        char name[64];
        unsigned hx;
        size_t nl;
        p += 6;
        ne = strstr(p, "</NAME>");
        if (!ne || (size_t)(ne - p) >= sizeof(name)) continue;
        nl = (size_t)(ne - p);
        memcpy(name, p, nl);
        name[nl] = '\0';
        hs = strstr(ne, "<HEX>");
        if (!hs || hs - ne > 512) continue;
        hs += 5;
        he = strstr(hs, "</HEX>");
        if (!he || (size_t)(he - hs) >= 16) continue;
        {
            char hexs[16];
            size_t hl = (size_t)(he - hs);
            memcpy(hexs, hs, hl);
            hexs[hl] = '\0';
            hx = (unsigned)strtoul(hexs, NULL, 16);
        }
        if (hx < 256 && g_nameForHex[hx][0] == '\0') {
            strncpy(g_nameForHex[hx], name, sizeof(g_nameForHex[hx]) - 1);
            count++;
        }
        p = he;
    }
    if (count > 0) mlog("jukebox: %d names from Jukebox.xml", count);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config — "Track Name = file" (empty value = original). Re-read often,
 * bad lines ignored, table reset each parse so removals revert to original.
 * Key may also be "0xHH" hex directly.
 * ═══════════════════════════════════════════════════════════════════════════ */

static void trim(char* s) {
    char* e;
    while (*s == ' ' || *s == '\t') s++;
    e = s + strlen(s);
    while (e > s && (e[-1] == ' ' || e[-1] == '\t' ||
                     e[-1] == '\n' || e[-1] == '\r'))
        *--e = '\0';
}

static void read_config(void) {
    FILE* f = NULL;
    char line[512];
    int n = 0;
    size_t i;

    memset(g_fileForHex, 0, sizeof(g_fileForHex));
    if (fopen_s(&f, g_cfgPath, "r") != 0 || !f) return;

    while (fgets(line, sizeof(line), f)) {
        char* eq;
        char* name;
        char* value;
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == ';' || *p == '#' || *p == '\n' || *p == '\r' || *p == '\0')
            continue;
        eq = strchr(p, '=');
        if (!eq) continue;
        *eq = '\0';
        name = p;
        value = eq + 1;
        trim(name);
        trim(value);
        /* re-trim name head after trim() tail-only for name start */
        while (*name == ' ' || *name == '\t') name++;
        if (*name == '\0') continue;

        if ((_strnicmp(name, "0x", 2) == 0)) {
            unsigned hx = (unsigned)strtoul(name, NULL, 16);
            if (hx < 256 && *value) {
                strncpy(g_fileForHex[hx], value, sizeof(g_fileForHex[hx]) - 1);
                n++;
            }
            continue;
        }
        for (i = 0; i < NUM_TRACKS; i++) {
            if (_stricmp(name, kTracks[i].name) == 0) {
                if (*value) {
                    strncpy(g_fileForHex[kTracks[i].hex], value,
                            sizeof(g_fileForHex[kTracks[i].hex]) - 1);
                }
                n++;
                break;
            }
        }
    }
    fclose(f);
    (void)n;
}

static void generate_config(const char* path) {
    FILE* f = NULL;
    size_t i;
    if (fopen_s(&f, path, "w") != 0 || !f) return;
    fprintf(f, "; Separate Music %s - one file per song\n", MOD_VER);
    fprintf(f, "; Usage: Track Name = file  (empty = play original Music.mo3)\n");
    fprintf(f, "; Files live in the game's Music\\ folder.\n");
    fprintf(f, "; Missing file = original plays (safe).\n");
    fprintf(f, "; Pairs sharing one HEX share one slot (last line wins).\n");
    fprintf(f, "; Key may also be hex directly, e.g. 0x08 = Level8.mo3\n");
    fprintf(f, ";\n; Races (defaults Level1..Level15):\n");
    for (i = 0; i < NUM_TRACKS; i++) {
        if (kTracks[i].deffile[0])
            fprintf(f, "%s = %s\n", kTracks[i].name, kTracks[i].deffile);
    }
    fprintf(f, ";\n; Menus (empty = original):\n");
    for (i = 0; i < NUM_TRACKS; i++) {
        if (!kTracks[i].deffile[0])
            fprintf(f, "%s =\n", kTracks[i].name);
    }
    fclose(f);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Intercepted BASS exports
 * ═══════════════════════════════════════════════════════════════════════════ */

__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int mem, void* file, DWORD off,
                                                     DWORD len, DWORD flags, DWORD freq) {
    DWORD h = 0;
    if (real_BASS_MusicLoad)
        h = real_BASS_MusicLoad(mem, file, off, len, flags, freq);
    if (!mem && file && h != 0) {
        const char* p = (const char*)file;
        char low[MAX_PATH];
        size_t i;
        strncpy(low, p, sizeof(low) - 1);
        low[sizeof(low) - 1] = '\0';
        for (i = 0; low[i]; i++)
            if (low[i] >= 'A' && low[i] <= 'Z') low[i] += 32;
        if (strstr(low, "music.mo3") && !strstr(low, "level")) {
            /* ends with music.mo3 proper (not mymusic.mo3), i.e. the game's file */
            const char* m = strstr(low, "music.mo3");
            if (m == low || m[-1] == '\\' || m[-1] == '/') {
                g_gameMusic = h;
                mlog("LOAD Music.mo3 -> %08X", h);
            }
        }
    }
    return h;
}

__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD handle, DWORD pos,
                                                     int restart, DWORD flags) {
    unsigned hx;
    const char* want;
    char full[MAX_PATH];

    read_config(); /* hot-reload, plays are rare */
    hx = pos & 0xFF;
    want = g_fileForHex[hx][0] ? g_fileForHex[hx] : NULL;

    if (want && build_music_path(want, full, sizeof(full))) {
        DWORD nh;
        int rc;
        int err;
        if (real_BASS_ChannelStop) {
            real_BASS_ChannelStop(handle);
            if (g_customMusic) real_BASS_ChannelStop(g_customMusic);
        }
        if (g_customMusic && real_BASS_MusicFree) {
            real_BASS_MusicFree(g_customMusic);
            g_customMusic = 0;
        }
        if (!real_BASS_MusicLoad || !real_BASS_MusicPlayEx) return 1;
        nh = real_BASS_MusicLoad(0, full, 0, 0, 4, 0);
        if (nh != 0) {
            if (real_BASS_ChannelSetAttributes)
                real_BASS_ChannelSetAttributes(nh, (DWORD)-1, 100, -101);
            rc = real_BASS_MusicPlayEx(nh, 0, 1, flags);
            err = real_BASS_ErrorGetCode ? real_BASS_ErrorGetCode() : -1;
            g_customMusic = nh;
            mlog("PLAY custom HEX=%02X (%s) file=%s -> %08X rc=%d err=%d",
                 hx, g_nameForHex[hx][0] ? g_nameForHex[hx] : "?", want, nh, rc, err);
            return rc;
        }
        err = real_BASS_ErrorGetCode ? real_BASS_ErrorGetCode() : -1;
        mlog("FAIL load HEX=%02X (%s) file=%s err=%d, fallback original",
             hx, g_nameForHex[hx][0] ? g_nameForHex[hx] : "?", want, err);
    }

    /* original path */
    if (g_customMusic) {
        if (real_BASS_ChannelStop) real_BASS_ChannelStop(g_customMusic);
        if (real_BASS_MusicFree) real_BASS_MusicFree(g_customMusic);
        g_customMusic = 0;
        mlog("STOP custom, original HEX=%02X (%s)",
             hx, g_nameForHex[hx][0] ? g_nameForHex[hx] : "?");
    }
    if (real_BASS_MusicPlayEx) {
        int r = real_BASS_MusicPlayEx(handle, pos, restart, flags);
        mlog("PLAY original HEX=%02X (%s) h=%08X rc=%d",
             hx, g_nameForHex[hx][0] ? g_nameForHex[hx] : "?", handle, r);
        return r;
    }
    return 1;
}

__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (a == g_customMusic) g_customMusic = 0;
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain — load real BASS, paths, INIT log, config, jukebox. No patches.
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    (void)lpReserved;
    if (reason == DLL_PROCESS_ATTACH) {
        char exepath[MAX_PATH];
        FILE* f = NULL;
        size_t i;
        DisableThreadLibraryCalls(hModule);
        g_hSelf = hModule;
        load_real_bass();

        GetModuleFileNameA(NULL, exepath, MAX_PATH);
        split_dir(exepath, g_exeDir, sizeof(g_exeDir));
        GetModuleFileNameA(hModule, g_dllDir, sizeof(g_dllDir));
        split_dir(g_dllDir, g_dllDir, sizeof(g_dllDir));
        _snprintf(g_cfgPath, sizeof(g_cfgPath), "%s" MOD_NAME ".txt", g_dllDir);
        g_cfgPath[sizeof(g_cfgPath) - 1] = '\0';
        _snprintf(g_logPath, sizeof(g_logPath), "%sseparate_music.log", g_dllDir);
        g_logPath[sizeof(g_logPath) - 1] = '\0';

        for (i = 0; i < NUM_TRACKS; i++) {
            if (g_nameForHex[kTracks[i].hex][0] == '\0')
                strncpy(g_nameForHex[kTracks[i].hex], kTracks[i].name,
                        sizeof(g_nameForHex[kTracks[i].hex]) - 1);
        }

        mlog("=== Separate Music %s Started === ExeDir: %s", MOD_VER, g_exeDir);

        if (fopen_s(&f, g_cfgPath, "r") != 0 || !f) {
            generate_config(g_cfgPath);
            mlog("config generated: %s", g_cfgPath);
        } else {
            fclose(f);
        }
        parse_jukebox();
        read_config();
    }
    return TRUE;
}
