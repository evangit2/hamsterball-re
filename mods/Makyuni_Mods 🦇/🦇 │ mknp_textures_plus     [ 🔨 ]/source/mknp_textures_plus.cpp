/*
 * mknp_textures_plus.cpp — Textures Plus (HB+ v2.1, MinGW build) v01c
 *
 * Reads mknp_textures_plus_set.jsonc (next to THIS dll) — a jsonc array of
 *   "label", { "texture_type": ..., ... } pairs:
 *   "tex_checker_01",     { "texture_type": "checker", "texture": "checker01" }
 *   "tex_goal-darkround", { "texture_type": "goal",
 *                           "goal_off_tex": "goal-darkround",
 *                           "goal_on_tex": "goal-darkround-lit" }
 * Names accept bare ("checker01") or pathed ("Textures/checker01") form,
 * with or without extension — compared case-insensitive.
 *
 * CHECKER: forces POINT (nearest) MAG/MIN/MIP filtering for the listed
 * texture via a D3D8 device vtable hook (SetTexture[61]/SetTextureStageState
 * [63]) — same technique as mknp_sharp_textures. Only listed textures are
 * touched; everything else passes through to the game's own values.
 *
 * GOAL: replicates the native N:GOAL swap (DispatchCollisionEvents @0x40C5D0:
 * on goal it copies the lit texture's D3D ptr over the goal slot, e.g.
 * App+0x294 <- App+0x29C). Here the OFF cache entry's D3D ptr is replaced
 * with the ON texture's D3D ptr when N:GOAL fires (event callback + board
 * +0xCD0 poll fallback), restored on level start/end. Both textures load
 * through the game's own loader (0x455C50) if not cached yet.
 *
 * Set re-read on every level enter (keep-old-on-syntax-error). Log:
 * mknp_textures_plus.log next to THIS dll (INIT + SET + resolve/swap lines).
 *
 * MinGW HB+ pattern: nocrt + manual 17-entry vtable, IModAPI via hbplus_api.h.
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

/* ── Game addresses ───────────────────────────────────────────── */

#define APP_PTR           0x005341E0
#define OFF_APP_GRAPHICS  0x174
#define OFF_APP_SCENE     0x178
#define OFF_D3D_DEVICE    0x154
#define GFX_TEX_COUNT     0x2E8
#define GFX_TEX_ARRAY     0x6F0
#define TEX_OBJ_D3D       0x04
#define TEX_OBJ_NAME      0x08
#define BOARD_GOAL_FLAG   0xCD0

/* D3D8: device vtable[61]=SetTexture, [63]=SetTextureStageState;
 * TSS 16/17/18 = MAG/MIN/MIP, POINT(1) = nearest. */
#define VTBL_SET_TEXTURE  61
#define VTBL_SET_TSS      63
#define D3DTSS_MAGFILTER  16
#define D3DTSS_MINFILTER  17
#define D3DTSS_MIPFILTER  18
#define D3DTEXF_POINT     1

/* LoadTexture: __thiscall(void* gfx, char* filename, char search_cache). */
typedef void* (__fastcall *LoadTexture_t)(void* gfx, void* dummy, const char* name, int search_cache);
static LoadTexture_t game_LoadTexture = (LoadTexture_t)0x00455C50;

#define SET_MAX       32
#define MAX_CHECKER   64
#define TYPE_CHECKER  1
#define TYPE_GOAL     2

/* ── State ────────────────────────────────────────────────────── */

static IModAPI* g_api = NULL;
static HMODULE g_module = NULL;
static char g_logPath[MAX_PATH];
static char g_modDir[MAX_PATH];
static int g_enabled = 1;

typedef struct {
    char label[64];
    int type;               /* 1=checker, 2=goal */
    char tex[64];           /* checker: affected texture */
    char off[64];           /* goal: OFF texture */
    char on[64];            /* goal: ON texture */
} SetEntry;
static SetEntry g_set[SET_MAX];
static int g_setCount = 0;

typedef struct {
    DWORD offObj;           /* cache texobj addr (OFF entry) */
    DWORD origD3D;          /* OFF's D3D ptr before swap */
    DWORD onD3D;            /* ON texture's D3D ptr */
    int resolved;
    int swapped;
} GoalState;
static GoalState g_goal[SET_MAX];

static DWORD g_checkerD3D[MAX_CHECKER];
static int g_checkerCount = 0;

static int g_lastTexCount = -1;
static int g_rescanTimer = 99;   /* force resolve on first tick */
static int g_missingLogged = 0;
static DWORD g_raceBoard = 0;    /* board seen this level (stale-board guard) */
static int g_seenClean = 0;      /* 1 once CD0==0 observed on g_raceBoard */

/* ── Small utils (no CRT) ─────────────────────────────────────── */

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

/* "Textures/checker01.png" -> "checker01" (strip path + extension). */
static void norm_tex(const char* in, char* out, int outsz) {
    const char* base = in;
    const char* p = in;
    const char* dot = 0;
    const char* q;
    int n = 0;
    while (*p) { if (*p == '/' || *p == '\\') base = p + 1; p++; }
    for (q = base; *q; q++) if (*q == '.') dot = q;
    p = dot ? dot : base + strlen(base);
    while (base < p && n + 1 < outsz) out[n++] = *base++;
    out[n] = 0;
}

static int tex_name_eq(const char* cacheName, const char* cfgVal) {
    char a[64], b[64];
    if (!cacheName || !cfgVal) return 0;
    norm_tex(cacheName, a, sizeof(a));
    norm_tex(cfgVal, b, sizeof(b));
    if (!a[0] || !b[0]) return 0;
    return str_eq_ci(a, b);
}

/* ── Paths + log (log lives next to THIS dll) ─────────────────── */

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

static void resolve_mod_dir(void) {
    if (g_modDir[0]) return;
    HMODULE self = mod_self();
    if (self) GetModuleFileNameA(self, g_modDir, MAX_PATH);
    strip_to_dir(g_modDir);
    if (!g_logPath[0] && g_modDir[0]) {
        int i = 0;
        while (g_modDir[i] && i + 1 < MAX_PATH) { g_logPath[i] = g_modDir[i]; i++; }
        g_logPath[i] = 0;
    }
}

static void log_msg(const char* msg) {
    HANDLE f;
    DWORD wrote = 0;
    char path[MAX_PATH];
    resolve_mod_dir();
    if (!g_logPath[0]) return;
    snprintf(path, sizeof(path), "%smknp_textures_plus.log", g_logPath);
    f = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                    OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f == INVALID_HANDLE_VALUE) return;
    SetFilePointer(f, 0, NULL, FILE_END);
    WriteFile(f, msg, (DWORD)strlen(msg), &wrote, NULL);
    WriteFile(f, "\r\n", 2, &wrote, NULL);
    CloseHandle(f);
}

/* ── jsonc set parser ([ "label", {...}, ... ], // + block comments) ── */

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
    e->type = 0; e->tex[0] = 0; e->off[0] = 0; e->on[0] = 0;
    jc_expect(j, '{');
    if (j->err) return;
    while (1) {
        char key[32], val[64];
        jc_skip(j);
        if (j->p < j->end && *j->p == '}') { j->p++; break; }
        if (j->p >= j->end) { j->err = 1; break; }
        if (!jc_string(j, key, sizeof(key))) return;
        if (!jc_expect(j, ':')) return;
        jc_skip(j);
        if (j->p < j->end && *j->p == '"') {
            if (!jc_string(j, val, sizeof(val))) return;
            if (str_eq_ci(key, "texture_type")) {
                if (str_eq_ci(val, "checker")) e->type = TYPE_CHECKER;
                else if (str_eq_ci(val, "goal")) e->type = TYPE_GOAL;
            }
            else if (str_eq_ci(key, "texture")) { strncpy(e->tex, val, 63); e->tex[63] = 0; }
            else if (str_eq_ci(key, "goal_off_tex")) { strncpy(e->off, val, 63); e->off[63] = 0; }
            else if (str_eq_ci(key, "goal_on_tex")) { strncpy(e->on, val, 63); e->on[63] = 0; }
        } else jc_skip_value(j);
        if (j->err) return;
        jc_skip(j);
        if (j->p < j->end && *j->p == ',') j->p++;
    }
}

static int parse_set(const char* buf, int len, SetEntry* out, int maxn, int* errOff) {
    JC j = { buf, buf + len, 0 };
    int n = 0;
    if (errOff) *errOff = -1;
    if (len >= 3 && (unsigned char)buf[0] == 0xEF &&
        (unsigned char)buf[1] == 0xBB && (unsigned char)buf[2] == 0xBF) j.p += 3;
    if (!jc_expect(&j, '[')) { if (errOff) *errOff = (int)(j.p - buf); return -1; }
    while (1) {
        char label[64];
        jc_skip(&j);
        if (j.p < j.end && *j.p == ']') { j.p++; break; }
        if (j.p >= j.end) { j.err = 1; break; }
        if (!jc_string(&j, label, sizeof(label))) break;
        jc_skip(&j);
        if (j.p < j.end && *j.p == ',') j.p++;
        jc_skip(&j);
        if (n < maxn) {
            strncpy(out[n].label, label, 63); out[n].label[63] = 0;
            if (j.p < j.end && *j.p == '{') jc_object(&j, &out[n]);
            else { out[n].type = 0; out[n].tex[0] = 0; out[n].off[0] = 0; out[n].on[0] = 0; }
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

static void clear_goal_state(void) {
    for (int i = 0; i < SET_MAX; i++) {
        g_goal[i].offObj = 0; g_goal[i].origD3D = 0;
        g_goal[i].onD3D = 0; g_goal[i].resolved = 0; g_goal[i].swapped = 0;
    }
}

static void load_set(void) {
    char path[MAX_PATH], msg[256];
    static char buf[8192];
    DWORD got;
    static SetEntry tmp[SET_MAX];   /* static: 8KB frame needs no chkstk (nostdlib) */
    int n, off, i;
    resolve_mod_dir();
    if (!g_modDir[0]) { log_msg("SET n/a: mod dir unknown"); return; }
    snprintf(path, sizeof(path), "%smknp_textures_plus_set.jsonc", g_modDir);
    got = read_file(path, buf, sizeof(buf) - 1);
    if (!got) {
        g_setCount = 0;
        clear_goal_state();
        log_msg("SET missing: mknp_textures_plus_set.jsonc (0 active)");
        return;
    }
    n = parse_set(buf, (int)got, tmp, SET_MAX, &off);
    if (n < 0) {
        snprintf(msg, sizeof(msg), "SET parse error @%d (kept %d)", off, g_setCount);
        log_msg(msg);
        return;
    }
    for (i = 0; i < n; i++) g_set[i] = tmp[i];
    g_setCount = n;
    clear_goal_state();
    g_checkerCount = 0;
    g_lastTexCount = -1;
    g_rescanTimer = 99;
    g_missingLogged = 0;
    snprintf(msg, sizeof(msg), "SET %d entr(s): mknp_textures_plus_set.jsonc", n);
    log_msg(msg);
    for (i = 0; i < n; i++) {
        if (g_set[i].type == TYPE_CHECKER)
            snprintf(msg, sizeof(msg), "SET checker %s: tex=%s", g_set[i].label, g_set[i].tex);
        else if (g_set[i].type == TYPE_GOAL)
            snprintf(msg, sizeof(msg), "SET goal %s: off=%s on=%s", g_set[i].label, g_set[i].off, g_set[i].on);
        else
            snprintf(msg, sizeof(msg), "SET skip %s: unknown texture_type", g_set[i].label);
        log_msg(msg);
    }
}

/* ── Texture cache helpers ────────────────────────────────────── */

static int get_gfx(DWORD* outGfx, int* outCount, DWORD* outArr) {
    DWORD appPtr, gfx;
    int count;
    DWORD arr;
    if (IsBadReadPtr((void*)APP_PTR, 4)) return 0;
    appPtr = *(DWORD*)APP_PTR;
    if (!appPtr || appPtr < 0x10000 || IsBadReadPtr((void*)appPtr, 0x200)) return 0;
    if (IsBadReadPtr((void*)(appPtr + OFF_APP_GRAPHICS), 4)) return 0;
    gfx = *(DWORD*)(appPtr + OFF_APP_GRAPHICS);
    if (!gfx || IsBadReadPtr((void*)gfx, 0x800)) return 0;
    if (IsBadReadPtr((void*)(gfx + GFX_TEX_COUNT), 4)) return 0;
    count = *(int*)(gfx + GFX_TEX_COUNT);
    if (count < 0 || count > 2048) return 0;
    if (IsBadReadPtr((void*)(gfx + GFX_TEX_ARRAY), 4)) return 0;
    arr = *(DWORD*)(gfx + GFX_TEX_ARRAY);
    if (!arr || IsBadReadPtr((void*)arr, 4)) return 0;
    if (outGfx) *outGfx = gfx;
    if (outCount) *outCount = count;
    if (outArr) *outArr = arr;
    return 1;
}

static DWORD find_texobj(DWORD arr, int count, const char* cfgName) {
    for (int i = 0; i < count; i++) {
        DWORD entry;
        DWORD namePtr;
        if (IsBadReadPtr((void*)(arr + i * 4), 4)) continue;
        entry = *(DWORD*)(arr + i * 4);
        if (!entry || IsBadReadPtr((void*)entry, 0x20)) continue;
        if (IsBadReadPtr((void*)(entry + TEX_OBJ_NAME), 4)) continue;
        namePtr = *(DWORD*)(entry + TEX_OBJ_NAME);
        if (!namePtr || IsBadReadPtr((void*)namePtr, 1)) continue;
        if (tex_name_eq((const char*)namePtr, cfgName)) return entry;
    }
    return 0;
}

/* Load "<name>" through the game loader (tries as-is + .png/.bmp/.tga). */
static DWORD load_texobj(DWORD gfx, const char* cfgName) {
    char fname[128], base[64];
    const char* ext;
    void* texObj;
    norm_tex(cfgName, base, sizeof(base));
    if (!base[0]) return 0;
    ext = 0;
    for (const char* p = cfgName; *p; p++) if (*p == '.') ext = p;
    if (ext && (str_eq_ci(ext, ".png") || str_eq_ci(ext, ".bmp") || str_eq_ci(ext, ".tga"))) {
        texObj = game_LoadTexture((void*)gfx, NULL, cfgName, 1);
        if (texObj && !IsBadReadPtr(texObj, 0x10)) {
            DWORD d = *(DWORD*)((char*)texObj + TEX_OBJ_D3D);
            if (d) return (DWORD)texObj;
        }
    }
    {
        const char* exts[3] = { ".png", ".bmp", ".tga" };
        for (int i = 0; i < 3; i++) {
            int n = 0;
            while (base[n] && n < 100) { fname[n] = base[n]; n++; }
            {
                const char* e = exts[i];
                while (*e && n < 120) fname[n++] = *e++;
            }
            fname[n] = 0;
            texObj = game_LoadTexture((void*)gfx, NULL, fname, 1);
            if (texObj && !IsBadReadPtr(texObj, 0x10)) {
                DWORD d = *(DWORD*)((char*)texObj + TEX_OBJ_D3D);
                if (d) return (DWORD)texObj;
            }
        }
    }
    return 0;
}

static DWORD resolve_texobj(DWORD gfx, DWORD arr, int count, const char* cfgName) {
    DWORD e = find_texobj(arr, count, cfgName);
    if (e) return e;
    return load_texobj(gfx, cfgName);
}

/* Rebuild checker D3D list + resolve goal entries. Logs transitions. */
static void resolve_all(void) {
    DWORD gfx, arr;
    int count, i;
    char msg[160];
    if (!get_gfx(&gfx, &count, &arr)) return;

    g_checkerCount = 0;
    for (i = 0; i < g_setCount; i++) {
        DWORD e;
        if (g_set[i].type != TYPE_CHECKER || !g_set[i].tex[0]) continue;
        e = resolve_texobj(gfx, arr, count, g_set[i].tex);
        if (e && !IsBadReadPtr((void*)(e + TEX_OBJ_D3D), 4)) {
            DWORD d = *(DWORD*)(e + TEX_OBJ_D3D);
            if (d && g_checkerCount < MAX_CHECKER)
                g_checkerD3D[g_checkerCount++] = d;
        } else if (!g_missingLogged) {
            snprintf(msg, sizeof(msg), "CHECKER %s: tex %s not found", g_set[i].label, g_set[i].tex);
            log_msg(msg);
        }
    }

    for (i = 0; i < g_setCount; i++) {
        GoalState* g = &g_goal[i];
        DWORD offE, onE, onD;
        if (g_set[i].type != TYPE_GOAL) continue;
        if (g->resolved) {
            /* Revalidate: entry may move when the cache rebuilds. */
            offE = find_texobj(arr, count, g_set[i].off);
            if (offE && offE == g->offObj) continue;
            g->resolved = 0; g->swapped = 0;
        }
        if (!g_set[i].off[0] || !g_set[i].on[0]) continue;
        offE = resolve_texobj(gfx, arr, count, g_set[i].off);
        onE = find_texobj(arr, count, g_set[i].on);
        if (!onE) onE = load_texobj(gfx, g_set[i].on);
        if (!offE || !onE) {
            if (!g_missingLogged) {
                snprintf(msg, sizeof(msg), "GOAL %s: waiting off=%s on=%s",
                         g_set[i].label, g_set[i].off, g_set[i].on);
                log_msg(msg);
            }
            continue;
        }
        if (IsBadReadPtr((void*)(offE + TEX_OBJ_D3D), 4)) continue;
        if (IsBadReadPtr((void*)(onE + TEX_OBJ_D3D), 4)) continue;
        onD = *(DWORD*)(onE + TEX_OBJ_D3D);
        if (!onD) continue;
        g->offObj = offE;
        g->origD3D = *(DWORD*)(offE + TEX_OBJ_D3D);
        g->onD3D = onD;
        g->resolved = 1;
        g->swapped = 0;
        snprintf(msg, sizeof(msg), "GOAL %s: ready off=%s on=%s",
                 g_set[i].label, g_set[i].off, g_set[i].on);
        log_msg(msg);
    }
}

static int need_resolve(void) {
    for (int i = 0; i < g_setCount; i++)
        if (g_set[i].type == TYPE_GOAL && !g_goal[i].resolved) return 1;
    return 0;
}

/* ── Goal swap (native parity: D3D ptr copy on N:GOAL) ─────────── */

static void fire_goals(const char* src) {
    char msg[160];
    for (int i = 0; i < g_setCount; i++) {
        GoalState* g = &g_goal[i];
        if (g_set[i].type != TYPE_GOAL || !g->resolved || g->swapped) continue;
        if (IsBadWritePtr((void*)(g->offObj + TEX_OBJ_D3D), 4)) continue;
        g->origD3D = *(DWORD*)(g->offObj + TEX_OBJ_D3D);
        *(DWORD*)(g->offObj + TEX_OBJ_D3D) = g->onD3D;
        g->swapped = 1;
        snprintf(msg, sizeof(msg), "GOAL %s: ON via %s (obj=0x%X off=0x%X on=0x%X)",
                 g_set[i].label, src, (unsigned int)g->offObj,
                 (unsigned int)g->origD3D, (unsigned int)g->onD3D);
        log_msg(msg);
    }
}

static void restore_goals(void) {
    char msg[128];
    for (int i = 0; i < g_setCount; i++) {
        GoalState* g = &g_goal[i];
        if (g_set[i].type != TYPE_GOAL || !g->swapped) continue;
        if (!IsBadWritePtr((void*)(g->offObj + TEX_OBJ_D3D), 4)) {
            *(DWORD*)(g->offObj + TEX_OBJ_D3D) = g->origD3D;
            snprintf(msg, sizeof(msg), "GOAL %s: restored %s", g_set[i].label, g_set[i].off);
        } else {
            snprintf(msg, sizeof(msg), "GOAL %s: restore SKIPPED (obj gone)", g_set[i].label);
        }
        g->swapped = 0;
        log_msg(msg);
    }
}

/* ── D3D filter hook (checker = POINT/nearest, rest pass through) ─ */

typedef int (__stdcall *SetTSS_t)(void*, DWORD, DWORD, DWORD);
typedef int (__stdcall *SetTexture_t)(void*, DWORD, void*);
static SetTSS_t g_orig_SetTSS = NULL;
static SetTexture_t g_orig_SetTexture = NULL;
static void* g_curTex[8] = { 0 };
static int g_hookInstalled = 0;

static int is_checker_d3d(void* t) {
    DWORD v = (DWORD)t;
    if (!v) return 0;
    for (int i = 0; i < g_checkerCount; i++)
        if (g_checkerD3D[i] == v) return 1;
    return 0;
}

static int __stdcall hook_SetTexture(void* device, DWORD stage, void* tex) {
    int r = g_orig_SetTexture(device, stage, tex);
    if (stage < 8) {
        g_curTex[stage] = tex;
        if (tex && is_checker_d3d(tex)) {
            g_orig_SetTSS(device, stage, D3DTSS_MAGFILTER, D3DTEXF_POINT);
            g_orig_SetTSS(device, stage, D3DTSS_MINFILTER, D3DTEXF_POINT);
            g_orig_SetTSS(device, stage, D3DTSS_MIPFILTER, D3DTEXF_POINT);
        }
    }
    return r;
}

static int __stdcall hook_SetTSS(void* device, DWORD stage, DWORD type, DWORD value) {
    if (stage < 8 && (type == D3DTSS_MAGFILTER || type == D3DTSS_MINFILTER || type == D3DTSS_MIPFILTER)) {
        if (g_curTex[stage] && is_checker_d3d(g_curTex[stage]))
            value = D3DTEXF_POINT;
    }
    return g_orig_SetTSS(device, stage, type, value);
}

static void try_install_hook(void) {
    DWORD appPtr, gfx;
    int* device;
    int* vtable;
    DWORD old;
    if (IsBadReadPtr((void*)APP_PTR, 4)) return;
    appPtr = *(DWORD*)APP_PTR;
    if (!appPtr || appPtr < 0x10000 || IsBadReadPtr((void*)appPtr, 0x200)) return;
    if (IsBadReadPtr((void*)(appPtr + OFF_APP_GRAPHICS), 4)) return;
    gfx = *(DWORD*)(appPtr + OFF_APP_GRAPHICS);
    if (!gfx || IsBadReadPtr((void*)(gfx + OFF_D3D_DEVICE), 4)) return;
    device = *(int**)(gfx + OFF_D3D_DEVICE);
    if (!device || IsBadReadPtr(device, 4)) return;
    vtable = *(int**)device;
    if (!vtable || IsBadReadPtr(vtable, (VTBL_SET_TSS + 1) * 4)) return;
    if (!vtable[VTBL_SET_TSS] || !vtable[VTBL_SET_TEXTURE]) return;
    g_orig_SetTSS = (SetTSS_t)vtable[VTBL_SET_TSS];
    g_orig_SetTexture = (SetTexture_t)vtable[VTBL_SET_TEXTURE];
    if (VirtualProtect(&vtable[VTBL_SET_TSS], 4, PAGE_READWRITE, &old)) {
        vtable[VTBL_SET_TSS] = (int)&hook_SetTSS;
        VirtualProtect(&vtable[VTBL_SET_TSS], 4, old, &old);
    } else return;
    if (VirtualProtect(&vtable[VTBL_SET_TEXTURE], 4, PAGE_READWRITE, &old)) {
        vtable[VTBL_SET_TEXTURE] = (int)&hook_SetTexture;
        VirtualProtect(&vtable[VTBL_SET_TEXTURE], 4, old, &old);
    }
    g_hookInstalled = 1;
    log_msg("HOOK D3D SetTexture/SetTextureStageState installed");
}

/* ── HB+ callbacks (manual 17-entry vtable) ───────────────────── */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) operator delete(thisptr);
    return thisptr;
}
static const char* __thiscall get_mod_name(void*) { return "Textures Plus"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, IModAPI* api) {
    g_api = api;
    *(void**)((char*)thisptr + 4) = api;
    log_msg("INIT mknp_textures_plus v01c HB+ (mod loaded)");
    load_set();
    CustomButton btn("TEXTURES_PLUS", "Textures Plus");
    btn.defaultState = true;
    HBAPI(api).CreateToggleButton(btn, (HamsterballAPI*)thisptr);
}

static void __thiscall button_toggle(void*, const char* id, bool state) {
    if (strcmp(id, "TEXTURES_PLUS") == 0) {
        g_enabled = state ? 1 : 0;
        if (!state) restore_goals();
        else { g_lastTexCount = -1; g_rescanTimer = 99; }
        log_msg(state ? "TOGGLE Textures Plus ON" : "TOGGLE Textures Plus OFF");
    }
}

static void __thiscall slider_change(void*, const char*, float) {}
static void __thiscall stub_ball(void*, void*) {}
static void __thiscall stub_render(void*, void*, void*) {}
static void __thiscall stub_cycle(void*, void*, int) {}
static void __thiscall stub_text(void*) {}
static void __thiscall stub_bump(void*, void*, void*) {}

static void __thiscall game_update(void*) {
    DWORD gfx, arr;
    int count;
    if (!g_enabled || !g_api) return;
    if (!g_hookInstalled) try_install_hook();
    if (!get_gfx(&gfx, &count, &arr)) return;
    if (count != g_lastTexCount || need_resolve()) {
        if (++g_rescanTimer >= 30) {
            g_rescanTimer = 0;
            g_lastTexCount = count;
            resolve_all();
            if (!need_resolve()) g_missingLogged = 1;
        }
    }
    /* N:GOAL fallback poll (native sets board+0xCD0=1 on goal).
     * Stale-board guard: only fire after we've SEEN this exact board
     * with CD0==0 (armed). A previous race's won board (CD0==1) visible
     * during load transitions must NOT pre-light the goal. */
    {
        DWORD appPtr = *(DWORD*)APP_PTR;
        if (appPtr && !IsBadReadPtr((void*)(appPtr + OFF_APP_SCENE), 4)) {
            DWORD board = *(DWORD*)(appPtr + OFF_APP_SCENE);
            if (board && board > 0x10000 && !IsBadReadPtr((void*)(board + BOARD_GOAL_FLAG), 1)) {
                int cd0 = *(BYTE*)(board + BOARD_GOAL_FLAG) ? 1 : 0;
                if (board != g_raceBoard) { g_raceBoard = board; g_seenClean = 0; }
                if (!cd0) g_seenClean = 1;
                else if (g_seenClean) fire_goals("POLL");
            }
        }
    }
}

static void __thiscall event_collide(void*, void*, char* id) {
    if (id && str_eq_ci(id, "N:GOAL")) fire_goals("EVENT");
}

static void __thiscall scene_end(void*) {
    restore_goals();
}

static void __thiscall level_start(void*) {
    char msg[96];
    DWORD appPtr = 0, board = 0;
    int cd0 = -1;
    restore_goals();
    if (!IsBadReadPtr((void*)APP_PTR, 4)) {
        appPtr = *(DWORD*)APP_PTR;
        if (appPtr && appPtr > 0x10000 && !IsBadReadPtr((void*)(appPtr + OFF_APP_SCENE), 4)) {
            board = *(DWORD*)(appPtr + OFF_APP_SCENE);
            if (board && board > 0x10000 && !IsBadReadPtr((void*)(board + BOARD_GOAL_FLAG), 1))
                cd0 = *(BYTE*)(board + BOARD_GOAL_FLAG) ? 1 : 0;
        }
    }
    g_raceBoard = 0;
    g_seenClean = 0;
    snprintf(msg, sizeof(msg), "ENTER board=0x%X cd0=%d", (unsigned int)board, cd0);
    log_msg(msg);
    load_set();
    g_missingLogged = 0;
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
    (void*)button_toggle,          // [8]  onButtonToggle
    (void*)slider_change,        // [9]  onSliderChange
    (void*)stub_cycle,           // [10] onCycleOptionChange (v2.0+)
    (void*)game_update,          // [11] onGameUpdate
    (void*)event_collide,        // [12] onEventPlaneCollide
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
