/*
 * mknp_animated_textures.cpp — Animated Textures (HB+ v2.1, MinGW build)
 *
 * Hamsterball Plus mod: cycles numbered texture variants at a custom
 * framerate. Reads mknp_animated_textures_set.jsonc (next to THIS dll)
 * for the animation list + per-texture settings, scans the Graphics
 * texture cache for those baseNN.png frames (e.g. Circleanim02.png),
 * loads the frames via the game's own loader, and swaps the D3D texture
 * pointer in a background thread. Textures NOT in the set are ignored.
 *
 * ANY referenced frame anchors the animation: the level may point at
 * Circleanim01, 02, ... NN and the cycle starts AT that frame.
 * One animation per texture-cache ENTRY: if three materials point at
 * A01/A02/A03, all three objects animate, each phased from its own
 * start frame (A01 object: 1-2-3, A02 object: 2-3-1, ...).
 *
 * Set file (mknp_animated_textures_set.jsonc, next to the dll):
 *   [ "Base", { "framerate": 0.5, "looptype": 1, "proximity": 300 }, ... ]
 *   framerate = seconds between frame swaps (default 0.5)
 *   looptype  = 0 play once, 1 loop (default), 2 ping-pong, 3 proximity
 *   proximity = loop 3 only: near radius (default 300)
 * Re-read on every level enter (edit the file, replay the level).
 *
 * Loop 3 needs NO setup: anchors = mesh positions wearing the texture
 * (auto via material array) + optional manual S1 refs `PROX:<base>`.
 *
 * Frames on disk: base01.png..baseNN.png, NO underscore, 01..NN contiguous.
 * Log: mknp_animated_textures.log next to THIS dll (Mods folder),
 *      %TEMP% fallback if the folder is not writable (UAC).
 *
 * MinGW HB+ pattern: nocrt + manual 17-entry vtable, IModAPI via
 * hbplus_api.h manual dispatch (GetScene/GetPlayer, same as
 * mknp_battyball_entities) with direct-memory fallback.
 *
 * Author: MAKYUNI / Hamsterbot
 */

#include "nocrt.h"
#include "HamsterballAPI.h"

#include <windows.h>
#include "hbplus_api.h"

#define malloc nc_malloc
#define free nc_free
#define memcpy nc_memcpy
#define memset nc_memset
#define strlen nc_strlen
#define strcmp nc_strcmp
#define strncmp nc_strncmp
#define memcmp nc_memcmp
#define strncpy nc_strncpy
#define snprintf nc_snprintf

/* ── Game addresses ─────────────────────────────────────────────── */

#define APP_PTR           0x005341E0
#define OFF_APP_GRAPHICS  0x174
#define OFF_APP_SCENE     0x178
#define GFX_TEX_COUNT     0x2E8
#define GFX_TEX_ARRAY     0x6F0
#define TEX_OBJ_D3D       0x04
#define TEX_OBJ_NAME      0x08

#define MAX_ANIMATIONS 32
#define MAX_FRAMES      32
#define MAX_PROX        8
#define SET_MAX         32
#define DEFAULT_PROX_R  300.0f

/* Board/ball/S1 layout (same as mknp_battyball_entities) */
#define BOARD_LEVEL         0x8AC
#define LEVEL_SCENEOBJECT   0x480
#define SCENEOBJ_S1_OFF     0x894
#define S1ENTRY_NAME        0x00
#define S1ENTRY_POS_X       0x04
#define S1ENTRY_POS_Y       0x08
#define S1ENTRY_POS_Z       0x0C
#define ALIST_COUNT         0x04
#define ALIST_ITEMS         0x40C
#define BOARD_BALL_LIST     0x29D4
#define BALL_POS_X          0x164

/* LoadTexture: __thiscall(void* gfx, char* filename, char search_cache).
 * MinGW C lacks __thiscall, so use __fastcall with a dummy EDX param. */
typedef void* (__fastcall *LoadTexture_t)(void* gfx, void* dummy, const char* name, int search_cache);
static LoadTexture_t game_LoadTexture = (LoadTexture_t)0x00455C50;

/* ── State ──────────────────────────────────────────────────────── */

typedef struct {
    DWORD d3dTexObjAddr;
    DWORD originalD3DTex;
    DWORD frameTextures[MAX_FRAMES];
    int frameCount;
    float framerate;
    int looptype;
    int currentFrame;
    int startFrame;   /* cache-entry's own frame: A01->1, A02->2 (per-object phase) */
    int direction;
    double lastSwapTime;
    char baseName[64];
    /* looptype 3 (proximity gate) */
    float proxR;
    float pax[MAX_PROX], pay[MAX_PROX], paz[MAX_PROX];
    int acount;
    int autoCount;
    int isnear;
    int oneshotDone;
    int proxFbLogged;
    int ballFbLogged;
    int anchorTries;
    int fileTries;
    DWORD texObjs[MAX_FRAMES];
    int texObjCount;
} AnimTexture;

static AnimTexture g_anims[MAX_ANIMATIONS];
static int g_animCount = 0;
static int g_running = 1;
static int g_started = 0;
static int g_swapLogs = 0;
static HANDLE g_thread = NULL;
static IModAPI* g_api = NULL;
static HMODULE g_module = NULL;
static char g_gameDir[MAX_PATH];   /* exe dir, trailing backslash */
static char g_logPath[MAX_PATH];   /* resolved once at init */

/* ── Small utils (no CRT) ───────────────────────────────────────── */

static int is_digit(char c) { return c >= '0' && c <= '9'; }

static int str_eq_ci(const char* a, const char* b) {
    while (*a && *b) {
        char ca = (*a >= 'A' && *a <= 'Z') ? (char)(*a + 32) : *a;
        char cb = (*b >= 'A' && *b <= 'Z') ? (char)(*b + 32) : *b;
        if (ca != cb) return 0;
        a++; b++;
    }
    return *a == *b;
}

static int parse_int(const char* s) {
    int v = 0, neg = 0;
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '-') { neg = 1; s++; }
    while (is_digit(*s)) { v = v * 10 + (*s - '0'); s++; }
    return neg ? -v : v;
}

static float parse_float(const char* s) {
    float v = 0, neg = 1, div = 1;
    int frac = 0;
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '-') { neg = -1; s++; }
    while (is_digit(*s)) { v = v * 10 + (*s - '0'); s++; }
    if (*s == '.') {
        s++;
        while (is_digit(*s)) { v = v * 10 + (*s - '0'); div *= 10; frac = 1; s++; }
        if (frac) v /= div;
    }
    return neg * v;
}

/* "Circleanim" + 2 -> "Circleanim02" (nc_snprintf space-pads %02d). */
static void frame_name(const char* base, int f, char* out) {
    int i = 0;
    while (base[i] && i < 120) { out[i] = base[i]; i++; }
    if (f < 100) {
        out[i++] = (char)('0' + (f / 10));
        out[i++] = (char)('0' + (f % 10));
    } else {
        char t[12]; int ti = 0, v = f;
        while (v > 0 && ti < 11) { t[ti++] = (char)('0' + (v % 10)); v /= 10; }
        while (ti > 0) out[i++] = t[--ti];
    }
    out[i++] = '.'; out[i++] = 'p'; out[i++] = 'n'; out[i++] = 'g';
    out[i] = 0;
}

/* ── Paths + log (log lives next to THIS dll) ───────────────────── */

static HMODULE mod_self(void) {
    MEMORY_BASIC_INFORMATION mbi;
    if (!g_module && VirtualQuery((void*)mod_self, &mbi, sizeof(mbi)))
        g_module = (HMODULE)mbi.AllocationBase;
    return g_module;
}

static void strip_to_dir(char* path) {
    char* slash = NULL;
    for (char* p = path; *p; p++) if (*p == '\\') slash = p;
    if (slash) slash[1] = 0;
    else path[0] = 0;
}

static void resolve_log_path(char* out) {
    out[0] = 0;
    HMODULE self = mod_self();
    if (self) GetModuleFileNameA(self, out, MAX_PATH);
    if (!out[0]) GetModuleFileNameA(NULL, out, MAX_PATH);
    strip_to_dir(out);
}

static void log_msg(const char* msg) {
    HANDLE f;
    DWORD wrote = 0;
    if (!g_logPath[0]) resolve_log_path(g_logPath);
    if (g_logPath[0]) {
        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%smknp_animated_textures.log", g_logPath);
        f = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (f == INVALID_HANDLE_VALUE && g_gameDir[0]) {
            snprintf(path, sizeof(path), "%smknp_animated_textures.log", g_gameDir);
            f = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        }
        if (f == INVALID_HANDLE_VALUE) return;
        SetFilePointer(f, 0, NULL, FILE_END);
        WriteFile(f, msg, (DWORD)strlen(msg), &wrote, NULL);
        WriteFile(f, "\r\n", 2, &wrote, NULL);
        CloseHandle(f);
    }
}

static double get_time(void) {
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart / (double)freq.QuadPart;
}

/* ── Config + frames ────────────────────────────────────────────── */

/* Extract base name and frame number from "Circleanim02.png". */
static int parse_frame_name(const char* filename, char* outBase, int baseMaxLen) {
    const char* dot = NULL;
    const char* p = filename;
    int nameLen, digitStart, digitCount, frameNum, baseLen, i;
    while (*p) { if (*p == '.') dot = p; p++; }
    if (!dot || dot == filename) return 0;
    nameLen = (int)(dot - filename);
    if (nameLen < 3) return 0;
    digitStart = nameLen - 1;
    while (digitStart >= 0 && is_digit(filename[digitStart])) digitStart--;
    digitStart++;
    digitCount = nameLen - digitStart;
    if (digitCount < 1 || digitCount > 4) return 0;
    frameNum = 0;
    for (i = digitStart; i < nameLen; i++) frameNum = frameNum * 10 + (filename[i] - '0');
    if (frameNum < 1) return 0;
    baseLen = digitStart;
    if (baseLen == 0 || baseLen >= baseMaxLen) return 0;
    for (i = 0; i < baseLen; i++) outBase[i] = filename[i];
    outBase[baseLen] = 0;
    return frameNum;
}

static DWORD read_file(const char* path, char* buf, DWORD cap) {
    HANDLE f = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD got = 0, total = 0;
    if (f == INVALID_HANDLE_VALUE) return 0;
    while (total + 256 < cap) {
        if (!ReadFile(f, buf + total, 256, &got, NULL) || got == 0) break;
        total += got;
    }
    CloseHandle(f);
    if (total < cap) buf[total] = 0;
    return total;
}

static char g_modDir[MAX_PATH];   /* dll dir, trailing backslash */

typedef struct {
    char base[64];
    float framerate;
    int looptype;
    float proxR;
} SetEntry;
static SetEntry g_set[SET_MAX];
static int g_setCount = 0;

/* ── Set file (jsonc, next to the dll) ───────────────────────────── */

typedef struct { const char* p; const char* end; int err; } JC;

static void jc_skip(JC* j) {
    while (j->p < j->end) {
        char c = *j->p;
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') { j->p++; continue; }
        if (c == '/' && j->p + 1 < j->end && j->p[1] == '/') {
            j->p += 2;
            while (j->p < j->end && *j->p != '\n') j->p++;
            continue;
        }
        if (c == '/' && j->p + 1 < j->end && j->p[1] == '*') {
            j->p += 2;
            while (j->p + 1 < j->end && !(j->p[0] == '*' && j->p[1] == '/')) j->p++;
            if (j->p + 1 < j->end) j->p += 2;
            continue;
        }
        break;
    }
}
static int jc_expect(JC* j, char c) {
    jc_skip(j);
    if (j->p < j->end && *j->p == c) { j->p++; return 1; }
    j->err = 1; return 0;
}
static int jc_string(JC* j, char* out, int outCap) {
    int n = 0;
    jc_skip(j);
    if (j->p >= j->end || *j->p != '"') { j->err = 1; return 0; }
    j->p++;
    while (j->p < j->end && *j->p != '"') {
        char c = *j->p++;
        if (c == '\\' && j->p < j->end) {
            char e = *j->p++;
            if (e == 'n') c = '\n'; else if (e == 't') c = '\t'; else c = e;
        }
        if (n + 1 < outCap) out[n++] = c;
    }
    if (j->p >= j->end) { j->err = 1; return 0; }
    j->p++;
    out[n] = 0;
    return 1;
}
static float jc_number(JC* j) {
    char tmp[64];
    int n = 0, digits = 0;
    jc_skip(j);
    if (j->p < j->end && *j->p == '-') { tmp[n++] = *j->p++; }
    while (j->p < j->end && (is_digit(*j->p) || *j->p == '.')) {
        if (is_digit(*j->p)) digits = 1;
        if (n + 1 < 64) tmp[n++] = *j->p;
        j->p++;
    }
    tmp[n] = 0;
    if (!digits) { j->err = 1; return 0; }
    return parse_float(tmp);
}
/* Skip any value (unknown keys): strings, scalars, nested {} []. */
static void jc_skip_value(JC* j) {
    char d[8];
    int depth = 0;
    jc_skip(j);
    if (j->p >= j->end) { j->err = 1; return; }
    if (*j->p == '"') { jc_string(j, d, sizeof(d)); return; }
    while (j->p < j->end) {
        char c = *j->p;
        if (c == '"') {
            j->p++;
            while (j->p < j->end && *j->p != '"') { if (*j->p == '\\') j->p++; j->p++; }
            if (j->p < j->end) j->p++;
            continue;
        }
        if (c == '/' && j->p + 1 < j->end && (j->p[1] == '/' || j->p[1] == '*')) { jc_skip(j); continue; }
        if (c == '{' || c == '[') { depth++; j->p++; continue; }
        if (c == '}' || c == ']') { if (depth == 0) return; depth--; j->p++; continue; }
        if (c == ',' && depth == 0) return;
        j->p++;
    }
}
static void jc_object(JC* j, SetEntry* e) {
    e->framerate = 0.5f; e->looptype = 1; e->proxR = DEFAULT_PROX_R;
    jc_expect(j, '{');
    if (j->err) return;
    while (1) {
        char key[32];
        jc_skip(j);
        if (j->p < j->end && *j->p == '}') { j->p++; break; }
        if (j->p >= j->end) { j->err = 1; break; }
        if (!jc_string(j, key, sizeof(key))) return;
        if (!jc_expect(j, ':')) return;
        if (str_eq_ci(key, "framerate")) e->framerate = jc_number(j);
        else if (str_eq_ci(key, "looptype")) e->looptype = (int)jc_number(j);
        else if (str_eq_ci(key, "proximity")) e->proxR = jc_number(j);
        else jc_skip_value(j);
        if (j->err) return;
        jc_skip(j);
        if (j->p < j->end && *j->p == ',') j->p++;
    }
}
/* [ "Base", {...}, ... ] -> entry count, or -1 on syntax error. */
static int parse_set(const char* buf, int len, SetEntry* out, int maxn, int* errOff) {
    JC j = { buf, buf + len, 0 };
    int n = 0;
    if (errOff) *errOff = -1;
    if (len >= 3 && (unsigned char)buf[0] == 0xEF &&
        (unsigned char)buf[1] == 0xBB && (unsigned char)buf[2] == 0xBF) j.p += 3;
    if (!jc_expect(&j, '[')) { if (errOff) *errOff = (int)(j.p - buf); return -1; }
    while (1) {
        char base[64];
        jc_skip(&j);
        if (j.p < j.end && *j.p == ']') { j.p++; break; }
        if (j.p >= j.end) { j.err = 1; break; }
        if (!jc_string(&j, base, sizeof(base))) break;
        jc_skip(&j);
        if (j.p < j.end && *j.p == ',') j.p++;
        jc_skip(&j);
        if (n < maxn) {
            strncpy(out[n].base, base, 63); out[n].base[63] = 0;
            if (j.p < j.end && *j.p == '{') jc_object(&j, &out[n]);
            else { out[n].framerate = 0.5f; out[n].looptype = 1; out[n].proxR = DEFAULT_PROX_R; }
            if (!j.err) n++;
        } else {
            SetEntry tmp;
            if (j.p < j.end && *j.p == '{') jc_object(&j, &tmp);
        }
        if (j.err) break;
        jc_skip(&j);
        if (j.p < j.end && *j.p == ',') j.p++;
    }
    if (j.err) { if (errOff) *errOff = (int)(j.p - buf); return -1; }
    return n;
}

static SetEntry* find_set(const char* baseName) {
    int i;
    for (i = 0; i < g_setCount; i++)
        if (str_eq_ci(g_set[i].base, baseName)) return &g_set[i];
    return NULL;
}

/* (Re)read the set file. Missing file clears the set; a syntax error
 * keeps the previous set so a typo can't nuke a working config. */
static void load_set(void) {
    char path[MAX_PATH], msg[256];
    static char buf[8192];
    DWORD got;
    SetEntry tmp[SET_MAX];
    int n, off;
    if (!g_modDir[0]) {
        HMODULE self = mod_self();
        if (self) GetModuleFileNameA(self, g_modDir, MAX_PATH);
        strip_to_dir(g_modDir);
    }
    if (!g_modDir[0]) { log_msg("SET n/a: mod dir unknown"); return; }
    snprintf(path, sizeof(path), "%smknp_animated_textures_set.jsonc", g_modDir);
    got = read_file(path, buf, sizeof(buf) - 1);
    if (!got) {
        g_setCount = 0;
        log_msg("SET missing: mknp_animated_textures_set.jsonc (0 active)");
        return;
    }
    n = parse_set(buf, (int)got, tmp, SET_MAX, &off);
    if (n < 0) {
        snprintf(msg, sizeof(msg), "SET parse error @%d (kept %d)", off, g_setCount);
        log_msg(msg);
        return;
    }
    {
        int i;
        for (i = 0; i < n; i++) g_set[i] = tmp[i];
    }
    g_setCount = n;
    snprintf(msg, sizeof(msg), "SET %d texture(s): mknp_animated_textures_set.jsonc", n);
    log_msg(msg);
}

static int count_frames(const char* baseName) {
    char searchPath[MAX_PATH];
    WIN32_FIND_DATAA fd;
    HANDLE hFind;
    int validCount = 0;
    snprintf(searchPath, sizeof(searchPath), "%sTextures\\%s*.png", g_gameDir, baseName);
    hFind = FindFirstFileA(searchPath, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return 0;
    do {
        char dummyBase[64];
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        if (parse_frame_name(fd.cFileName, dummyBase, 64) > 0) {
            if (str_eq_ci(dummyBase, baseName)) validCount++;
        }
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
    return validCount;
}

/* ── Scene detect (direct memory, no IModAPI dispatch) ──────────── */

static int in_level(void) {
    DWORD appPtr;
    if (IsBadReadPtr((void*)APP_PTR, 4)) return 0;
    appPtr = *(DWORD*)APP_PTR;
    if (!appPtr || appPtr < 0x10000) return 0;
    if (IsBadReadPtr((void*)appPtr, 0x200)) return 0;
    {
        DWORD scene = *(DWORD*)(appPtr + OFF_APP_SCENE);
        return (scene && scene > 0x10000) ? 1 : 0;
    }
}

/* Board via HB+ API (same as mknp_battyball_entities player_board),
 * direct-memory fallback when the API is unavailable. */
static DWORD g_dbgBoard = 0;
static DWORD g_dbgLevel = 0;
static DWORD g_dbgRc = 0;
static int g_dbgMats = 0;

static DWORD get_board(void) {
    DWORD board = 0;
    if (g_api) {
        void* s = (void*)HBAPI(g_api).GetScene();
        if (s && (DWORD)s > 0x10000 && !IsBadReadPtr(s, 0x200))
            board = (DWORD)s;
    }
    if (!board && !IsBadReadPtr((void*)APP_PTR, 4)) {
        DWORD appPtr = *(DWORD*)APP_PTR;
        if (appPtr && appPtr > 0x10000 && !IsBadReadPtr((void*)appPtr, 0x200)) {
            DWORD b = *(DWORD*)(appPtr + OFF_APP_SCENE);
            if (b && b > 0x10000 && !IsBadReadPtr((void*)b, 0x200))
                board = b;
        }
    }
    g_dbgBoard = board;
    return board;
}

/* Player ball: HB+ GetPlayer() first (works on Battyball), direct
 * ball-list walk as fallback. 0 = unreadable. */
static int get_ball_pos(float* x, float* y, float* z) {
    DWORD board, list, items, ball;
    int count;
    if (g_api) {
        void* b = (void*)HBAPI(g_api).GetPlayer();
        if (b && (DWORD)b > 0x10000 && !IsBadReadPtr((void*)((char*)b + BALL_POS_X), 12)) {
            *x = *(float*)((char*)b + BALL_POS_X);
            *y = *(float*)((char*)b + BALL_POS_X + 4);
            *z = *(float*)((char*)b + BALL_POS_X + 8);
            return 1;
        }
    }
    board = get_board();
    if (!board) return 0;
    if (IsBadReadPtr((void*)(board + BOARD_BALL_LIST + 4), 4)) return 0;
    list = board + BOARD_BALL_LIST;
    count = *(int*)(list + ALIST_COUNT);
    if (count < 1) return 0;
    if (IsBadReadPtr((void*)(list + ALIST_ITEMS), 4)) return 0;
    items = *(DWORD*)(list + ALIST_ITEMS);
    if (!items || IsBadReadPtr((void*)items, 4)) return 0;
    ball = *(DWORD*)items;
    if (!ball || ball < 0x10000) return 0;
    if (IsBadReadPtr((void*)(ball + BALL_POS_X + 8), 4)) return 0;
    *x = *(float*)(ball + BALL_POS_X);
    *y = *(float*)(ball + BALL_POS_X + 4);
    *z = *(float*)(ball + BALL_POS_X + 8);
    return 1;
}

/* "PROX:<base>" S1 ref points -> proximity anchors for looptype 3. */
static const char* after_prox(const char* name) {
    if (!name) return NULL;
    if ((name[0] == 'P' || name[0] == 'p') &&
        (name[1] == 'R' || name[1] == 'r') &&
        (name[2] == 'O' || name[2] == 'o') &&
        (name[3] == 'X' || name[3] == 'x') && name[4] == ':')
        return name + 5;
    return NULL;
}

static void scan_prox_anchors(const char* base, AnimTexture* a) {
    DWORD appPtr, board, level, sceneobj, s1list, items;
    int count, i;
    const char* tail;
    char* nm;
    /* NOTE: no acount reset here — scan_anchors() owns the reset so manual
     * PROX refs append AFTER auto material anchors. */
    board = get_board();
    if (!board) return;
    if (IsBadReadPtr((void*)(board + BOARD_LEVEL), 4)) return;
    level = *(DWORD*)(board + BOARD_LEVEL);
    if (!level || level < 0x10000) return;
    if (IsBadReadPtr((void*)(level + LEVEL_SCENEOBJECT), 4)) return;
    sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
    if (!sceneobj || sceneobj < 0x10000) return;
    if (IsBadReadPtr((void*)(sceneobj + SCENEOBJ_S1_OFF + 4), 4)) return;
    s1list = sceneobj + SCENEOBJ_S1_OFF;
    count = *(int*)(s1list + ALIST_COUNT);
    if (count <= 0 || count > 2000) return;
    if (IsBadReadPtr((void*)(s1list + ALIST_ITEMS), 4)) return;
    items = *(DWORD*)(s1list + ALIST_ITEMS);
    if (!items || IsBadReadPtr((void*)items, count * 4)) return;
    for (i = 0; i < count && a->acount < MAX_PROX; i++) {
        DWORD entry = *(DWORD*)(items + i * 4);
        if (!entry || entry < 0x10000) continue;
        if (IsBadReadPtr((void*)entry, 16)) continue;
        nm = *(char**)(entry + S1ENTRY_NAME);
        if (!nm || IsBadReadPtr(nm, 8)) continue;
        tail = after_prox(nm);
        if (!tail || !str_eq_ci(tail, base)) continue;
        if (IsBadReadPtr((void*)(entry + S1ENTRY_POS_Z), 4)) continue;
        a->pax[a->acount] = *(float*)(entry + S1ENTRY_POS_X);
        a->pay[a->acount] = *(float*)(entry + S1ENTRY_POS_Y);
        a->paz[a->acount] = *(float*)(entry + S1ENTRY_POS_Z);
        a->acount++;
    }
}

/* RENDER MeshWorld: board(App+0x178)+0x8AC -> level+0x480 -> sceneobj+0x08.
 * (NOT level+0x08 = source mesh, materials never allocated there.) */
static DWORD render_meshworld(void) {
    DWORD board, level, sceneobj, mw;
    board = get_board();
    g_dbgLevel = 0;
    if (!board) return 0;
    if (IsBadReadPtr((void*)(board + BOARD_LEVEL), 4)) return 0;
    level = *(DWORD*)(board + BOARD_LEVEL);
    if (!level || level < 0x10000) return 0;
    g_dbgLevel = level;
    if (IsBadReadPtr((void*)(level + LEVEL_SCENEOBJECT), 4)) return 0;
    sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
    if (!sceneobj || sceneobj < 0x10000) return 0;
    if (IsBadReadPtr((void*)(sceneobj + 0x08), 4)) return 0;
    mw = *(DWORD*)(sceneobj + 0x08);
    if (!mw || mw < 0x10000) return 0;
    return mw;
}

static int tex_known(AnimTexture* a, DWORD v) {
    int i;
    if (!v) return 0;
    if (v == a->originalD3DTex) return 1;
    for (i = 0; i < a->frameCount && i < MAX_FRAMES; i++)
        if (v == a->frameTextures[i]) return 1;
    for (i = 0; i < a->texObjCount && i < MAX_FRAMES; i++)
        if (v == a->texObjs[i]) return 1;
    return 0;
}

/* TEX diag: our wanted pointers vs the first material slots seen. */
static void log_tex_diag(AnimTexture* a) {
    char msg[200];
    DWORD g0 = 0, g1 = 0, g2 = 0;
    int n, i;
    if (g_dbgRc && g_dbgMats > 0) {
        n = g_dbgMats < 3 ? g_dbgMats : 3;
        for (i = 0; i < n; i++) {
            DWORD rc = g_dbgRc + i * 0x50;
            DWORD t = 0;
            if (!IsBadReadPtr((void*)(rc + 0x48), 4)) t = *(DWORD*)(rc + 0x48);
            if (i == 0) g0 = t; else if (i == 1) g1 = t; else g2 = t;
        }
    }
    snprintf(msg, sizeof(msg), "TEX %s: want=0x%X/0x%X/0x%X got=0x%X,0x%X,0x%X",
             a->baseName, a->originalD3DTex,
             a->frameCount > 0 ? a->frameTextures[0] : 0,
             a->texObjCount > 0 ? a->texObjs[0] : 0, g0, g1, g2);
    log_msg(msg);
}

/* Walk ONE material array (render mw or source mw). tag "" or "2". */
static int scan_mw(DWORD mw, AnimTexture* a, const char* tag) {
    DWORD rcBase, rc, first, last;
    int count, i, hits = 0, skipped = 0;
    char msg[160];
    if (!mw) return 0;
    if (IsBadReadPtr((void*)(mw + 0x30), 4)) return 0;
    count = *(int*)(mw + 0x30);
    if (count <= 0 || count > 10000) {
        if (IsBadReadPtr((void*)(mw + 0x24), 4)) return 0;
        count = *(int*)(mw + 0x24);
        if (count <= 0 || count > 10000) return 0;
    }
    rcBase = 0;
    if (!IsBadReadPtr((void*)(mw + 0x28), 4)) {
        DWORD p = *(DWORD*)(mw + 0x28);
        if (p && p > 0x10000) {
            first = p;
            last = p + (count - 1) * 0x50;
            if (!IsBadReadPtr((void*)first, 0x50) && !IsBadReadPtr((void*)last, 0x50))
                rcBase = p;
        }
    }
    if (!rcBase) {
        first = mw + 0x28;
        last = first + (count - 1) * 0x50;
        if (!IsBadReadPtr((void*)first, 0x50) && !IsBadReadPtr((void*)last, 0x50))
            rcBase = first;
    }
    if (!rcBase) return 0;
    g_dbgRc = rcBase;
    g_dbgMats = count;
    for (i = 0; i < count && a->acount < MAX_PROX; i++) {
        DWORD t;
        float x, y, z;
        rc = rcBase + i * 0x50;
        if (IsBadReadPtr((void*)(rc + 0x48), 4)) { skipped++; continue; }
        t = *(DWORD*)(rc + 0x48);
        if (!tex_known(a, t)) continue;
        if (IsBadReadPtr((void*)(rc + 0x14), 12)) { skipped++; continue; }
        z = *(float*)(rc + 0x14);
        x = *(float*)(rc + 0x18);
        y = *(float*)(rc + 0x1C);
        if (x > 1e6 || x < -1e6 || y > 1e6 || y < -1e6 || z > 1e6 || z < -1e6) { skipped++; continue; }
        a->pax[a->acount] = x;
        a->pay[a->acount] = y;
        a->paz[a->acount] = z;
        a->acount++;
        hits++;
    }
    a->autoCount += hits;
    if (a->anchorTries <= 3) {
        snprintf(msg, sizeof(msg), "MATCH%s %s: mats=%d hit=%d skip=%d rc=0x%X", tag, a->baseName, count, hits, skipped, rcBase);
        log_msg(msg);
    }
    return hits;
}

/* Auto-anchors: render materials first, source-meshworld (level+0x08)
 * fallback when the render MeshWorld is NULL (REF-loaded levels). */
static void scan_material_anchors(AnimTexture* a) {
    char msg[160];
    DWORD mw = render_meshworld();
    int hits = 0, verbose = (a->anchorTries <= 3);
    if (verbose) {
        snprintf(msg, sizeof(msg), "MW %s: board=0x%X mw=0x%X lvl=0x%X", a->baseName, g_dbgBoard, mw, g_dbgLevel);
        log_msg(msg);
    }
    if (mw) hits = scan_mw(mw, a, "");
    if (hits <= 0 && g_dbgLevel) {
        DWORD src = 0;
        if (!IsBadReadPtr((void*)(g_dbgLevel + 0x08), 4)) src = *(DWORD*)(g_dbgLevel + 0x08);
        if (verbose) {
            snprintf(msg, sizeof(msg), "MW2 %s: src=0x%X", a->baseName, src);
            log_msg(msg);
        }
        if (src && src > 0x10000 && src != mw) hits = scan_mw(src, a, "2");
    }
    if (verbose) log_tex_diag(a);
}

static DWORD get_sceneobj(void) {
    DWORD board, level, so;
    board = get_board();
    if (!board) return 0;
    if (IsBadReadPtr((void*)(board + BOARD_LEVEL), 4)) return 0;
    level = *(DWORD*)(board + BOARD_LEVEL);
    if (!level || level < 0x10000) return 0;
    if (IsBadReadPtr((void*)(level + LEVEL_SCENEOBJECT), 4)) return 0;
    so = *(DWORD*)(level + LEVEL_SCENEOBJECT);
    if (!so || so < 0x10000) return 0;
    return so;
}

/* One-shot deep dump (retry 1, still anchorless): S1 count (chain check),
 * nonzero sceneobj pointers (render-MW hunt), source material quads. */
static void dump_debug(AnimTexture* a) {
    char msg[200];
    DWORD so = get_sceneobj();
    int off, k;
    if (!so) { log_msg("SC (null sceneobj)"); return; }
    {
        int cnt = -1;
        if (!IsBadReadPtr((void*)(so + SCENEOBJ_S1_OFF + 4), 4))
            cnt = *(int*)(so + SCENEOBJ_S1_OFF + ALIST_COUNT);
        snprintf(msg, sizeof(msg), "S1 %s: cnt=%d", a->baseName, cnt);
        log_msg(msg);
    }
    for (off = 0; off < 0x100; off += 40) {
        int pos = 0;
        pos += snprintf(msg + pos, sizeof(msg) - pos, "SC %s +%X:", a->baseName, off);
        for (k = 0; k < 10; k++) {
            DWORD v = 0;
            if (!IsBadReadPtr((void*)(so + off + k * 4), 4)) v = *(DWORD*)(so + off + k * 4);
            if (v > 0x10000 && pos < 150)
                pos += snprintf(msg + pos, sizeof(msg) - pos, " +%X=%X", off + k * 4, v);
        }
        log_msg(msg);
    }
    if (g_dbgRc && g_dbgMats > 0) {
        int n = g_dbgMats < 8 ? g_dbgMats : 8, i;
        for (i = 0; i < n; i++) {
            DWORD rc = g_dbgRc + i * 0x50;
            DWORD v40 = 0, v44 = 0, v48 = 0, v4c = 0;
            if (!IsBadReadPtr((void*)(rc + 0x40), 16)) {
                v40 = *(DWORD*)(rc + 0x40); v44 = *(DWORD*)(rc + 0x44);
                v48 = *(DWORD*)(rc + 0x48); v4c = *(DWORD*)(rc + 0x4C);
            }
            snprintf(msg, sizeof(msg), "M2 %s %d: 40=0x%X 44=0x%X 48=0x%X 4C=0x%X",
                     a->baseName, i, v40, v44, v48, v4c);
            log_msg(msg);
        }
    }
}

/* ── Level-file anchors (pure buffer parse, verified vs Level2 file) ──── */

typedef struct { const unsigned char* d; int len; int pos; int bad; } FACur;

static int fa32(FACur* c) {
    uint32_t v;
    if (c->pos + 4 > c->len) { c->bad = 1; return 0; }
    memcpy(&v, c->d + c->pos, 4); c->pos += 4; return (int)v;
}
static float fafloat(FACur* c) {
    float v = 0;
    if (c->pos + 4 > c->len) { c->bad = 1; return 0; }
    memcpy(&v, c->d + c->pos, 4); c->pos += 4; return v;
}
static const char* fastr(FACur* c) {
    int ln = fa32(c);
    const char* s;
    if (c->bad || ln <= 0 || c->pos + ln > c->len) { c->bad = 1; return NULL; }
    s = (const char*)(c->d + c->pos); c->pos += ln; return s;
}
static int fadigit(char ch) { return ch >= '0' && ch <= '9'; }
/* "Circleanim02.png" vs base "Circleanim" -> frame number or 0 */
static int frame_of(const char* fn, const char* base) {
    const char* dot = NULL; const char* p = fn;
    int nameLen, ds, dc, f = 0, i, bl;
    if (!fn || !base) return 0;
    while (*p) { if (*p == '.') dot = p; p++; }
    if (!dot) return 0;
    nameLen = (int)(dot - fn);
    ds = nameLen - 1;
    while (ds >= 0 && fadigit(fn[ds])) ds--;
    ds++;
    dc = nameLen - ds;
    if (dc < 1 || dc > 4) return 0;
    for (i = ds; i < nameLen; i++) f = f * 10 + (fn[i] - '0');
    if (f < 1) return 0;
    bl = 0; while (base[bl]) bl++;
    if (bl != ds) return 0;
    for (i = 0; i < bl; i++) {
        char ca = fn[i], cb = base[i];
        if (ca >= 'A' && ca <= 'Z') ca += 32;
        if (cb >= 'A' && cb <= 'Z') cb += 32;
        if (ca != cb) return 0;
    }
    return f;
}
static void faskipmat(FACur* c) {
    int i;
    for (i = 0; i < 16; i++) fafloat(c);
    fafloat(c); fa32(c);
    if (fa32(c) == 1) fastr(c);
}
static void faskips1(FACur* c) {
    int n = fa32(c), i;
    if (c->bad) return;
    for (i = 0; i < n; i++) {
        int k;
        fastr(c);
        for (k = 0; k < 6; k++) fafloat(c);
        if (c->pos + 4 > c->len) { c->bad = 1; break; }
        if (c->d[c->pos] != 0) { c->pos += 4; faskipmat(c); }
        else c->pos += 4;
        if (c->bad) break;
    }
}
/* S1 count, -1 on error */
static int file_s1_count(const unsigned char* d, int len) {
    FACur c = {d, len, 0, 0};
    int n = fa32(&c), i;
    if (c.bad || n < 0 || n > 100000) return -1;
    for (i = 0; i < n; i++) {
        int k;
        fastr(&c);
        for (k = 0; k < 6; k++) fafloat(&c);
        if (c.pos + 4 > c.len) { c.bad = 1; break; }
        if (c.d[c.pos] != 0) { c.pos += 4; faskipmat(&c); }
        else c.pos += 4;
        if (c.bad) break;
    }
    return c.bad ? -1 : n;
}
/* skip S2..S5, landing at octree start. 1 = ok. */
static int faskip26(FACur* c) {
    int n = fa32(c), i;
    for (i = 0; i < n; i++) {
        int dl = fa32(c);
        if (dl < 0 || c->pos + dl > c->len) { c->bad = 1; return 0; }
        c->pos += dl;
        {
            int pc = fa32(c);
            if (pc < 0 || c->pos + pc * 12 > c->len) { c->bad = 1; return 0; }
            c->pos += pc * 12;
        }
    }
    n = fa32(c);
    for (i = 0; i < n; i++) {
        if (fa32(c) == 0) {
            if (c->pos + 36 > c->len) { c->bad = 1; return 0; }
            c->pos += 36;
        }
    }
    if (c->pos + 24 > c->len) { c->bad = 1; return 0; }
    c->pos += 24;
    n = fa32(c);
    if (n < 0 || c->pos + n * 32 > c->len) { c->bad = 1; return 0; }
    c->pos += n * 32;
    return !c->bad;
}
static int fa_oct_has(FACur* c, const char* base, int depth) {
    int i;
    if (depth > 32 || c->pos + 28 > c->len) { c->bad = 1; return 0; }
    c->pos += 24;
    {
        int nsub = fa32(c);
        if (nsub < 1) {
            int nmb = fa32(c);
            for (i = 0; i < nmb; i++) {
                int k;
                fastr(c);
                for (k = 0; k < 16; k++) fafloat(c);
                fafloat(c); fa32(c);
                {
                    const char* tex = NULL;
                    int nstrip;
                    if (fa32(c) == 1) tex = fastr(c);
                    nstrip = fa32(c);
                    if (nstrip < 0 || nstrip > 1000000) { c->bad = 1; return 0; }
                    c->pos += nstrip * 8;
                    if (tex && frame_of(tex, base) > 0) return 1;
                    if (c->bad) return 0;
                }
            }
            return 0;
        }
        for (i = 0; i < nsub; i++) {
            if (fa_oct_has(c, base, depth + 1)) return 1;
            if (c->bad) return 0;
        }
    }
    return 0;
}
/* 1 if any S6 geom texture matches baseNN */
static int file_has_tex(const unsigned char* d, int len, const char* base) {
    FACur c = {d, len, 0, 0};
    faskips1(&c);
    if (!faskip26(&c)) return 0;
    return fa_oct_has(&c, base, 0) && !c.bad;
}
static int fa_oct_get(FACur* c, const unsigned char* vbase, int vn,
                      const char* base, float out[][3], int maxn, int* found, int depth) {
    int i;
    if (depth > 32 || c->pos + 28 > c->len) { c->bad = 1; return 0; }
    c->pos += 24;
    {
        int nsub = fa32(c);
        if (nsub < 1) {
            int nmb = fa32(c);
            for (i = 0; i < nmb; i++) {
                int k, s, ns, isOurs;
                const char* tex = NULL;
                fastr(c);
                for (k = 0; k < 16; k++) fafloat(c);
                fafloat(c); fa32(c);
                if (fa32(c) == 1) tex = fastr(c);
                ns = fa32(c);
                isOurs = (tex && frame_of(tex, base) > 0);
                for (s = 0; s < ns; s++) {
                    int tri = fa32(c), vref = fa32(c);
                    if (isOurs && s == 0 && *found < maxn && tri > 0 && vref >= 0) {
                        int nv = tri + 2, got = 0, q;
                        float sx = 0, sy = 0, sz = 0;
                        if (nv > 16) nv = 16;
                        for (q = 0; q < nv; q++) {
                            int idx = vref + q;
                            float x, y, z;
                            if (idx < 0 || idx >= vn) break;
                            memcpy(&x, vbase + idx * 32, 4);
                            memcpy(&y, vbase + idx * 32 + 4, 4);
                            memcpy(&z, vbase + idx * 32 + 8, 4);
                            sx += x; sy += y; sz += z; got++;
                        }
                        if (got > 0) {
                            out[*found][0] = sx / got;
                            out[*found][1] = sy / got;
                            out[*found][2] = sz / got;
                            (*found)++;
                        }
                    }
                }
                if (c->bad) return 0;
            }
            return 0;
        }
        for (i = 0; i < nsub; i++) {
            fa_oct_get(c, vbase, vn, base, out, maxn, found, depth + 1);
            if (c->bad) return 0;
        }
    }
    return 0;
}
/* anchors = avg of first strip's verts per matching geom. returns count. */
static int file_anchors(const unsigned char* d, int len, const char* base,
                        float out[][3], int maxn) {
    FACur c = {d, len, 0, 0};
    int found = 0, vn;
    const unsigned char* vbase;
    faskips1(&c);
    {
        int m, i;
        m = fa32(&c);
        for (i = 0; i < m; i++) {
            int dl = fa32(&c);
            c.pos += (dl < 0 ? 0 : dl);
            {
                int pc = fa32(&c);
                c.pos += (pc < 0 ? 0 : pc) * 12;
            }
        }
        m = fa32(&c);
        for (i = 0; i < m; i++) if (fa32(&c) == 0) c.pos += 36;
        c.pos += 24;
    }
    vn = fa32(&c); vbase = d + c.pos;
    if (c.bad || vn < 0 || c.pos + vn * 32 > len) return 0;
    c.pos += vn * 32;
    fa_oct_get(&c, vbase, vn, base, out, maxn, &found, 0);
    return c.bad ? 0 : found;
}

static int runtime_s1count(void) {
    DWORD so = get_sceneobj();
    int cnt;
    if (!so) return -1;
    if (IsBadReadPtr((void*)(so + SCENEOBJ_S1_OFF + 4), 4)) return -1;
    cnt = *(int*)(so + SCENEOBJ_S1_OFF + ALIST_COUNT);
    if (cnt < 0 || cnt > 100000) return -1;
    return cnt;
}

/* File anchors: find levels/*.MESHWORLD referencing baseNN (S1-count
 * fingerprint picks the current level), anchor = strip-vertex averages. */
static void scan_file_anchors(AnimTexture* a) {
    char pat[MAX_PATH], path[MAX_PATH], msg[256], best[MAX_PATH];
    WIN32_FIND_DATAA fd;
    HANDLE h;
    int rs = runtime_s1count(), k;
    best[0] = 0;
    snprintf(pat, sizeof(pat), "%slevels\\*.MESHWORLD", g_gameDir);
    h = FindFirstFileA(pat, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        snprintf(msg, sizeof(msg), "FILE %s: no levels dir", a->baseName);
        log_msg(msg);
        return;
    }
    do {
        HANDLE f;
        DWORD sz, got;
        unsigned char* data;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        snprintf(path, sizeof(path), "%slevels\\%s", g_gameDir, fd.cFileName);
        f = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (f == INVALID_HANDLE_VALUE) continue;
        sz = GetFileSize(f, NULL);
        data = 0;
        if (sz > 64 && sz < 32 * 1024 * 1024) {
            data = (unsigned char*)malloc(sz);
            if (data && (!ReadFile(f, data, sz, &got, NULL) || got != sz)) {
                free(data); data = 0;
            }
        }
        CloseHandle(f);
        if (!data) continue;
        if (file_has_tex(data, sz, a->baseName)) {
            int fs = file_s1_count(data, sz);
            if (rs > 0 && fs == rs) {
                float xyz[8][3];
                int g = file_anchors(data, sz, a->baseName, xyz, 8);
                for (k = 0; k < g && a->acount < MAX_PROX; k++) {
                    a->pax[a->acount] = xyz[k][0];
                    a->pay[a->acount] = xyz[k][1];
                    a->paz[a->acount] = xyz[k][2];
                    a->acount++;
                    a->autoCount++;
                }
                snprintf(msg, sizeof(msg), "FILE %s: %s s1=%d geoms=%d",
                         a->baseName, fd.cFileName, fs, g);
                log_msg(msg);
                free(data);
                break;
            } else if (!best[0]) {
                strncpy(best, path, MAX_PATH - 1);
                best[MAX_PATH - 1] = 0;
            }
        }
        free(data);
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    if (a->acount > 0 || !best[0]) {
        if (a->acount == 0) {
            snprintf(msg, sizeof(msg), "FILE %s: no s1-matched file", a->baseName);
            log_msg(msg);
        }
        return;
    }
    /* fallback: first file referencing the texture (wrong-level risk logged) */
    {
        HANDLE f = CreateFileA(best, GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (f != INVALID_HANDLE_VALUE) {
            DWORD sz = GetFileSize(f, NULL), got = 0;
            unsigned char* data = 0;
            if (sz > 64 && sz < 32 * 1024 * 1024) {
                data = (unsigned char*)malloc(sz);
                if (data && (!ReadFile(f, data, sz, &got, NULL) || got != sz)) {
                    free(data); data = 0;
                }
            }
            CloseHandle(f);
            if (data) {
                float xyz[8][3];
                int g = file_anchors(data, sz, a->baseName, xyz, 8);
                for (k = 0; k < g && a->acount < MAX_PROX; k++) {
                    a->pax[a->acount] = xyz[k][0];
                    a->pay[a->acount] = xyz[k][1];
                    a->paz[a->acount] = xyz[k][2];
                    a->acount++;
                    a->autoCount++;
                }
                snprintf(msg, sizeof(msg), "FILE %s: guessed %s geoms=%d", a->baseName, best, g);
                log_msg(msg);
                free(data);
            }
        }
    }
}

/* Full anchor pass: auto material anchors first, manual PROX: refs after. */
static void scan_anchors(AnimTexture* a) {
    char msg[160];
    int manual;
    a->acount = 0;
    a->autoCount = 0;
    scan_material_anchors(a);
    manual = a->acount;
    if (a->acount < MAX_PROX) {
        scan_prox_anchors(a->baseName, a);
        if (a->acount > manual) {
            snprintf(msg, sizeof(msg), "PROX %s: %d manual anchor(s)", a->baseName, a->acount - manual);
            log_msg(msg);
        }
    }
    if (a->acount == 0 && a->fileTries < 3) {
        if (runtime_s1count() > 0) { a->fileTries++; scan_file_anchors(a); }
    }
    snprintf(msg, sizeof(msg), "ANCHOR %s: %d auto + %d manual r=%f.0",
             a->baseName, a->autoCount, a->acount - a->autoCount, (double)a->proxR);
    log_msg(msg);
    if (a->anchorTries == 1 && a->acount == 0) dump_debug(a);
}

/* ── Animation setup + thread ───────────────────────────────────── */

static void try_setup_anims(void) {
    DWORD appPtr, graphics, arrPtr;
    int cacheCount, i, j;
    if (IsBadReadPtr((void*)APP_PTR, 4)) return;
    appPtr = *(DWORD*)APP_PTR;
    if (!appPtr || appPtr < 0x10000) return;
    if (IsBadReadPtr((void*)appPtr, 4)) return;
    if (IsBadReadPtr((void*)(appPtr + OFF_APP_GRAPHICS), 4)) return;
    graphics = *(DWORD*)(appPtr + OFF_APP_GRAPHICS);
    if (!graphics || IsBadReadPtr((void*)graphics, 4)) return;
    if (IsBadReadPtr((void*)(graphics + GFX_TEX_COUNT), 4)) return;
    cacheCount = *(int*)(graphics + GFX_TEX_COUNT);
    if (cacheCount < 1) return;
    if (IsBadReadPtr((void*)(graphics + GFX_TEX_ARRAY), 4)) return;
    arrPtr = *(DWORD*)(graphics + GFX_TEX_ARRAY);
    if (!arrPtr || IsBadReadPtr((void*)arrPtr, cacheCount * 4)) return;

    for (i = 0; i < cacheCount && g_animCount < MAX_ANIMATIONS; i++) {
        DWORD entry, namePtr, originalTex;
        const char* texName;
        char baseName[64], msg[256];
        int frameNum, totalFrames;
        AnimTexture* a;
        SetEntry* se;

        entry = *(DWORD*)(arrPtr + i * 4);
        if (!entry || IsBadReadPtr((void*)entry, 0x20)) continue;
        namePtr = *(DWORD*)(entry + TEX_OBJ_NAME);
        if (!namePtr || IsBadReadPtr((void*)namePtr, 1)) continue;
        texName = (const char*)namePtr;

        frameNum = parse_frame_name(texName, baseName, 64);
        /* Any referenced frame anchors the animation (not just 01) —
         * the cycle starts AT the frame the level points at. */
        if (frameNum < 1) continue;

        /* Set-driven: textures not listed are ignored (quietly). */
        se = find_set(baseName);
        if (!se) continue;
        snprintf(msg, sizeof(msg), "SCAN match %s frame=%d obj=0x%X", baseName, frameNum, entry);
        log_msg(msg);

        /* One animation per cache ENTRY (not per base): A01/A02/A03 are
         * three texture objects sharing one frame cycle, each phased from
         * its own start frame. Dedupe on entry address so every material's
         * texture animates. */
        for (j = 0; j < g_animCount; j++) {
            if (g_anims[j].d3dTexObjAddr == entry) break;
        }
        if (j < g_animCount) continue;

        totalFrames = count_frames(baseName);
        if (totalFrames < 2) {
            snprintf(msg, sizeof(msg), "SCAN skip %s: only %d frame file(s)", baseName, totalFrames);
            log_msg(msg);
            continue;
        }

        if (IsBadReadPtr((void*)(entry + TEX_OBJ_D3D), 4)) continue;
        originalTex = *(DWORD*)(entry + TEX_OBJ_D3D);
        if (!originalTex) continue;

        a = &g_anims[g_animCount];
        a->d3dTexObjAddr = entry;
        a->originalD3DTex = originalTex;
        a->frameCount = 0;
        a->currentFrame = 0;
        a->startFrame = frameNum;
        a->direction = 1;
        a->lastSwapTime = get_time();
        strncpy(a->baseName, baseName, 63);
        a->baseName[63] = 0;
        a->framerate = se->framerate;
        a->looptype = se->looptype;
        a->proxR = se->proxR;

        /* Load frames 1..N. The cached texture — whatever frame the level
         * referenced — is reused in place; the rest load via the game
         * loader. The animation starts AT the referenced frame. */
        {
            int anchorIdx = -1, f;
            a->texObjCount = 0;
            for (f = 1; f <= totalFrames && a->frameCount < MAX_FRAMES; f++) {
                DWORD d3dTex = 0;
                void* texObj = 0;
                if (f == frameNum) {
                    d3dTex = originalTex;
                    texObj = (void*)entry;
                } else {
                    char fname[128];
                    frame_name(baseName, f, fname);
                    {
                        texObj = game_LoadTexture((void*)graphics, NULL, fname, 1);
                        if (texObj && !IsBadReadPtr(texObj, 0x10))
                            d3dTex = *(DWORD*)((char*)texObj + TEX_OBJ_D3D);
                    }
                    if (!d3dTex) {
                        snprintf(msg, sizeof(msg), "SCAN load fail %s", fname);
                        log_msg(msg);
                    }
                }
                if (d3dTex) {
                    if (f == frameNum) anchorIdx = a->frameCount;
                    a->frameTextures[a->frameCount] = d3dTex;
                    a->frameCount++;
                    if (texObj && a->texObjCount < MAX_FRAMES)
                        a->texObjs[a->texObjCount++] = (DWORD)texObj;
                }
            }
            /* Referenced frame beyond the files on disk: append the cached
             * texture so the level's own frame still shows first. */
            if (anchorIdx < 0 && a->frameCount < MAX_FRAMES) {
                anchorIdx = a->frameCount;
                a->frameTextures[a->frameCount] = originalTex;
                a->frameCount++;
            }
            if (anchorIdx >= 0) a->currentFrame = anchorIdx;
        }

        if (a->frameCount < 2) {
            snprintf(msg, sizeof(msg), "SCAN skip %s: only %d frame(s) usable",
                     baseName, a->frameCount);
            log_msg(msg);
            continue;
        }
        if (a->looptype == 3)
            snprintf(msg, sizeof(msg), "SCAN setup %s(s%d): %d frames start=%d rate=%f.2 loop=3 prox=%f.0 obj=0x%X",
                     baseName, a->startFrame, a->frameCount, a->currentFrame + 1, a->framerate, (double)a->proxR, entry);
        else
            snprintf(msg, sizeof(msg), "SCAN setup %s(s%d): %d frames start=%d rate=%f.2 loop=%d obj=0x%X",
                     baseName, a->startFrame, a->frameCount, a->currentFrame + 1, a->framerate, a->looptype, entry);
        log_msg(msg);
        if (a->looptype == 3) {
            a->isnear = -1;
            a->oneshotDone = 0;
            a->proxFbLogged = 0;
            a->ballFbLogged = 0;
            a->anchorTries = 0;
            a->fileTries = 0;
            a->currentFrame = 0;
            scan_anchors(a);
        }
        g_animCount++;
    }
}

static void restore_textures(void) {
    int i;
    for (i = 0; i < g_animCount; i++) {
        AnimTexture* a = &g_anims[i];
        if (a->d3dTexObjAddr && !IsBadWritePtr((void*)(a->d3dTexObjAddr + TEX_OBJ_D3D), 4))
            *(DWORD*)(a->d3dTexObjAddr + TEX_OBJ_D3D) = a->originalD3DTex;
    }
    g_animCount = 0;
    g_swapLogs = 0;
}

static DWORD WINAPI anim_thread(LPVOID param) {
    int scanTimer = 0, wasInLevel = 0, anchorTimer = 0;
    char msg[256];
    (void)param;
    snprintf(msg, sizeof(msg), "INIT mknp_animated_textures v02b HB+ (mod loaded)");
    log_msg(msg);
    snprintf(msg, sizeof(msg), "THREAD started gameDir=%s", g_gameDir);
    log_msg(msg);
    load_set();
    Sleep(3000);

    while (g_running) {
        int inLv = in_level();
        Sleep(16);

        if (!inLv && wasInLevel) {
            restore_textures();
            wasInLevel = 0;
            scanTimer = 0;
            log_msg("EXIT level (textures restored)");
        }
        if (inLv && !wasInLevel) {
            wasInLevel = 1;
            scanTimer = 0;
            log_msg("ENTER level");
            load_set();
        }

        if (inLv && g_animCount == 0) {
            scanTimer++;
            if (scanTimer >= 60) {
                scanTimer = 0;
                try_setup_anims();
                snprintf(msg, sizeof(msg), "SCAN pass done: %d animation(s) active", g_animCount);
                log_msg(msg);
            }
        }

        /* Anchor retry: materials/S1 may stream in after textures. While any
         * looptype-3 anim has zero anchors, rescan every ~2s (quietly). */
        if (inLv && g_animCount > 0) {
            int k, need = 0;
            anchorTimer++;
            for (k = 0; k < g_animCount; k++)
                if (g_anims[k].looptype == 3 && g_anims[k].acount == 0) { need = 1; break; }
            if (need && anchorTimer >= 120) {
                anchorTimer = 0;
                for (k = 0; k < g_animCount; k++) {
                    AnimTexture* ra = &g_anims[k];
                    if (ra->looptype != 3 || ra->acount != 0) continue;
                    ra->anchorTries++;
                    if (ra->anchorTries <= 3) {
                        snprintf(msg, sizeof(msg), "ANCHOR %s: retry %d", ra->baseName, ra->anchorTries);
                        log_msg(msg);
                    }
                    scan_anchors(ra);
                }
            } else if (!need) anchorTimer = 0;
        }

        if (g_animCount == 0) continue;

        {
            double now = get_time();
            int i;
            for (i = 0; i < g_animCount; i++) {
                AnimTexture* a = &g_anims[i];
                double elapsed;
                int next, hold;
                DWORD newTex;
                if (a->frameCount < 2) continue;
                if (!a->d3dTexObjAddr) continue;
                if (IsBadReadPtr((void*)(a->d3dTexObjAddr + TEX_OBJ_D3D), 4)) {
                    snprintf(msg, sizeof(msg), "DROP %s: texture object went invalid", a->baseName);
                    log_msg(msg);
                    restore_textures();
                    break;
                }
                elapsed = now - a->lastSwapTime;
                if (elapsed < (double)a->framerate) continue;

                a->lastSwapTime = now;
                next = a->currentFrame + a->direction;
                hold = 0;

                if (a->looptype == 0) {
                    if (next >= a->frameCount) next = a->frameCount - 1;
                    else if (next < 0) next = 0;
                } else if (a->looptype == 1) {
                    if (next >= a->frameCount) next = 0;
                    else if (next < 0) next = a->frameCount - 1;
                } else if (a->looptype == 2) {
                    if (next >= a->frameCount) {
                        a->direction = -1;
                        next = a->frameCount - 2;
                        if (next < 0) next = 0;
                    } else if (next < 0) {
                        a->direction = 1;
                        next = 1;
                        if (next >= a->frameCount) next = a->frameCount - 1;
                    }
                } else if (a->looptype == 3) {
                    /* Proximity gate: near = hold frame 01; leaving = play
                     * 02..NN once, then hold last. Anchors = auto material
                     * positions + manual PROX:<base> S1 refs; none at all =
                     * plain loop fallback. */
                    float bx, by, bz;
                    if (!get_ball_pos(&bx, &by, &bz)) {
                        if (!a->ballFbLogged) {
                            a->ballFbLogged = 1;
                            snprintf(msg, sizeof(msg), "BALL %s: unreadable, holding", a->baseName);
                            log_msg(msg);
                        }
                        hold = 1;
                        next = a->currentFrame;
                    } else if (a->acount == 0) {
                        if (!a->proxFbLogged) {
                            a->proxFbLogged = 1;
                            snprintf(msg, sizeof(msg), "PROX %s: no ref, looping", a->baseName);
                            log_msg(msg);
                        }
                        if (next >= a->frameCount) next = 0;
                        else if (next < 0) next = a->frameCount - 1;
                    } else {
                        int k, isNear = 0;
                        float exitR = a->isnear == 1 ? a->proxR * 1.15f : a->proxR;
                        float rr = exitR * exitR;
                        for (k = 0; k < a->acount; k++) {
                            float dx = bx - a->pax[k];
                            float dy = by - a->pay[k];
                            float dz = bz - a->paz[k];
                            if (dx * dx + dy * dy + dz * dz <= rr) { isNear = 1; break; }
                        }
                        if (isNear) {
                            if (a->isnear != 1) {
                                a->isnear = 1;
                                snprintf(msg, sizeof(msg), "NEAR %s: holding 01", a->baseName);
                                log_msg(msg);
                            }
                            a->oneshotDone = 0;
                            next = 0;
                            if (a->currentFrame == 0) hold = 1;
                        } else {
                            if (a->isnear != 0) {
                                a->isnear = 0;
                                snprintf(msg, sizeof(msg), "FAR %s: playing once", a->baseName);
                                log_msg(msg);
                            }
                            if (a->oneshotDone) {
                                hold = 1;
                                next = a->currentFrame;
                            } else if (next >= a->frameCount) {
                                next = a->frameCount - 1;
                                a->oneshotDone = 1;
                                if (a->currentFrame == next) hold = 1;
                            } else if (next < 0) {
                                next = 0;
                            }
                        }
                    }
                } else {
                    if (next >= a->frameCount) next = 0;
                    else if (next < 0) next = a->frameCount - 1;
                }

                a->currentFrame = next;
                if (hold) continue;
                newTex = a->frameTextures[next];
                if (newTex && !IsBadWritePtr((void*)(a->d3dTexObjAddr + TEX_OBJ_D3D), 4)) {
                    *(DWORD*)(a->d3dTexObjAddr + TEX_OBJ_D3D) = newTex;
                    if (g_swapLogs < 24) {
                        g_swapLogs++;
                        snprintf(msg, sizeof(msg), "SWAP %s(s%d) -> frame %d tex=0x%X",
                                 a->baseName, a->startFrame, next + 1, newTex);
                        log_msg(msg);
                    }
                }
            }
        }
    }
    return 0;
}

/* ── HB+ callbacks (manual 17-entry vtable) ─────────────────────── */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) operator delete(thisptr);
    return thisptr;
}
static const char* __thiscall get_mod_name(void*) { return "Animated Textures"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, IModAPI* api) {
    (void)thisptr;
    g_api = api;
    if (g_gameDir[0] == 0) {
        GetModuleFileNameA(NULL, g_gameDir, MAX_PATH);
        strip_to_dir(g_gameDir);
    }
    resolve_log_path(g_logPath);
    if (!g_started) {
        g_started = 1;
        g_thread = CreateThread(NULL, 0, anim_thread, NULL, 0, NULL);
    }
}

static void __thiscall stub_ball(void*, void*) {}
static void __thiscall stub_render(void*, void*, void*) {}
static void __thiscall stub_toggle(void*, void*, int) {}
static void __thiscall stub_slider(void*, void*, float) {}
static void __thiscall stub_cycle(void*, void*, int) {}
static void __thiscall stub_game(void*) {}
static void __thiscall stub_event(void*, void*, void*) {}
static void __thiscall stub_text(void*) {}
static void __thiscall stub_bump(void*, void*, void*) {}

static void __thiscall scene_end(void*) {
    char msg[96];
    if (g_animCount > 0) {
        snprintf(msg, sizeof(msg), "SCENEEND wipe %d anim(s)", g_animCount);
        log_msg(msg);
    }
    restore_textures();
}

static void __thiscall level_start(void*) {
    /* Setup runs in the background thread (textures load after start). */
}

/* 17-entry vtable (HB+ v2.0/v2.1) */
static void* g_vtable[17] = {
    (void*)sc_dtor,              // [0]  ~HamsterballAPI
    (void*)get_mod_name,         // [1]  GetModName
    (void*)get_author,           // [2]  GetAuthorName
    (void*)get_version,          // [3]  GetApiVersion
    (void*)get_contributors,     // [4]  GetContributors
    (void*)init_impl,            // [5]  Initialize
    (void*)stub_ball,            // [6]  onBallUpdate
    (void*)stub_render,          // [7]  onRenderApply
    (void*)stub_toggle,          // [8]  onButtonToggle
    (void*)stub_slider,          // [9]  onSliderChange
    (void*)stub_cycle,           // [10] onCycleOptionChange (v2.0+)
    (void*)stub_game,            // [11] onGameUpdate
    (void*)stub_event,           // [12] onEventPlaneCollide
    (void*)stub_text,            // [13] onTextRenderLoop
    (void*)stub_bump,            // [14] onBallBump
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
