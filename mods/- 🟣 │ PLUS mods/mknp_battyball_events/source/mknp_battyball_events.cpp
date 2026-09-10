/*
 * mknp_battyball_events.cpp — Battyball Events (HB+ v2.1, MinGW build)
 *
 * Second of the "battyball" series of HB+ mods. Shell v1a: no custom
 * behavior yet — provides the mod skeleton (17-entry vtable, toggle,
 * per-DLL log with INIT line) plus an E: event tap so MAKYUNI can see
 * which event-plane names fire per level before the real design lands.
 *
 * Author: MAKYUNI / Hamsterbot
 */

#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

#include <windows.h>

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

/* ═══════════════ Mod state ═══════════════ */

static IModAPI* g_api     = NULL;
static bool     g_enabled = true;
static bool     g_applied = false;   /* one-shot GetButtonState guard */
static HMODULE  g_module  = NULL;

static HMODULE mod_self(void);

static HMODULE mod_self(void) {
    if (g_module) return g_module;
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery((void*)mod_self, &mbi, sizeof(mbi)))
        g_module = (HMODULE)mbi.AllocationBase;
    return g_module;
}

static char g_log_path[MAX_PATH];   /* resolved once at init */

static void resolve_log_path(char* out) {
    out[0] = '\0';
    HMODULE self = mod_self();
    if (self) GetModuleFileNameA(self, out, MAX_PATH);
    if (!out[0]) GetModuleFileNameA(NULL, out, MAX_PATH);
    char* slash = NULL;
    for (char* p = out; *p; p++) if (*p == '\\') slash = p;
    if (slash) {
        strncpy(slash + 1, "mknp_battyball_events.log",
                MAX_PATH - (slash + 1 - out) - 1);
    } else {
        strncpy(out, "mknp_battyball_events.log", MAX_PATH - 1);
        out[MAX_PATH - 1] = '\0';
    }
}

static void log_mod(const char* msg) {
    char path[MAX_PATH];
    if (g_log_path[0]) {
        strncpy(path, g_log_path, MAX_PATH);
        path[MAX_PATH - 1] = '\0';
    } else {
        resolve_log_path(path);
    }
    HANDLE f = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                           OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f == INVALID_HANDLE_VALUE) {
        /* Fallback: game dir via exe path */
        char exep[MAX_PATH];
        if (GetModuleFileNameA(NULL, exep, MAX_PATH) > 0) {
            char* s2 = NULL;
            for (char* p = exep; *p; p++) if (*p == '\\') s2 = p;
            if (s2) {
                strncpy(s2 + 1, "mknp_battyball_events.log",
                        MAX_PATH - (s2 + 1 - exep) - 1);
                f = CreateFileA(exep, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                                OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            }
        }
        if (f == INVALID_HANDLE_VALUE) return;
    }
    SetFilePointer(f, 0, NULL, FILE_END);
    DWORD wrote = 0;
    WriteFile(f, msg, (DWORD)strlen(msg), &wrote, NULL);
    WriteFile(f, "\r\n", 2, &wrote, NULL);
    CloseHandle(f);
}

/* ═══════════════ Set file (mknp_battyball_events_set.jsonc) ═══════════════
 * Array of "E:name", { "sound": "Name" } pairs. Lives next to the DLL.
 * Re-read every level start; syntax error keeps the previous table. */

#define MAX_EVENTS 32
#define SET_SIZE_CAP 16384

static char g_set_path[MAX_PATH];   /* resolved once at init */

static void resolve_set_path(char* out) {
    out[0] = '\0';
    HMODULE self = mod_self();
    if (self) GetModuleFileNameA(self, out, MAX_PATH);
    if (!out[0]) GetModuleFileNameA(NULL, out, MAX_PATH);
    char* slash = NULL;
    for (char* p = out; *p; p++) if (*p == '\\') slash = p;
    if (slash) {
        strncpy(slash + 1, "mknp_battyball_events_set.jsonc",
                MAX_PATH - (slash + 1 - out) - 1);
    } else {
        strncpy(out, "mknp_battyball_events_set.jsonc", MAX_PATH - 1);
        out[MAX_PATH - 1] = '\0';
    }
}

static void create_default_set(void) {
    const char* def_text =
        "// mknp_battyball_events_set.jsonc - Battyball Events settings\r\n"
        "[\r\n"
        "    // Sound Effects\r\n"
        "    \"E:sfx_bell\", { \"sound\": \"Bell\" }\r\n"
        "]\r\n";
    HANDLE f = CreateFileA(g_set_path, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                           CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f == INVALID_HANDLE_VALUE) return;   /* exists or unwritable: leave it */
    DWORD wrote = 0;
    WriteFile(f, def_text, (DWORD)strlen(def_text), &wrote, NULL);
    CloseHandle(f);
}

/* 61 native sounds: name -> App+offset (base 0x43C, stride 4). */
typedef struct { const char* name; int off; } sndtab_t;
static const sndtab_t g_sndtab[] = {
    {"collide",0x43C},{"roll",0x440},{"whistle",0x444},{"bumper",0x448},
    {"ballbreak",0x44C},{"ballbreaksmall",0x450},{"thwomp",0x454},{"snap",0x458},
    {"popup",0x45C},{"dropin",0x460},{"dropinshort",0x464},{"popout",0x468},
    {"pipebump1",0x46C},{"pipebump2",0x470},{"pipebump3",0x474},{"gearclank",0x478},
    {"bridgeslam",0x47C},{"platformtick",0x480},{"gluestuck",0x484},{"bubble1",0x488},
    {"bubble2",0x48C},{"wheelcreak",0x490},{"catapult",0x494},{"trapdoor",0x498},
    {"fwing",0x49C},{"clink",0x4A0},{"whoosh",0x4A4},{"chomp",0x4A8},
    {"fan-start",0x4AC},{"fan-blow",0x4B0},{"crack",0x4B4},{"crumble",0x4B8},
    {"sawstartup",0x4BC},{"sawcut",0x4C0},{"minipop",0x4C4},{"bell",0x4C8},
    {"zip",0x4CC},{"ting",0x4D0},{"shrink",0x4D4},{"grow",0x4D8},
    {"tweet",0x4DC},{"creakyplatform",0x4E0},{"wubba",0x4E4},{"saw",0x4E8},
    {"sawspeedy",0x4EC},{"dawgstep1",0x4F0},{"dawgstep2",0x4F4},{"dawgsmash",0x4F8},
    {"sizzle",0x4FC},{"explode",0x500},{"vac-o-sux",0x504},{"speedcylinder",0x508},
    {"bonuspop",0x50C},{"buzzbonus",0x510},{"breakbridge",0x514},{"unlock",0x518},
    {"NeonRide",0x51C},{"NeonFlicker",0x520},{"ZoopDown",0x524},{"LightsOff",0x528},
    {"GlassBonus",0x52C},
};
#define SNDTAB_N (sizeof(g_sndtab) / sizeof(g_sndtab[0]))

static int tolower_c(int c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; }

static int name_eq_ci(const char* a, const char* b) {
    while (*a && *b) {
        if (tolower_c(*a) != tolower_c(*b)) return 0;
        a++; b++;
    }
    return *a == *b;
}

static int sound_offset(const char* name) {
    int i;
    if (!name || !name[0]) return -1;
    for (i = 0; i < SNDTAB_N; i++)
        if (name_eq_ci(name, g_sndtab[i].name)) return g_sndtab[i].off;
    return -1;
}

/* Tiny LCG (no CRT rand) — variant picker. */
static unsigned long g_rng_state = 0x9E3779B9UL;

static void rng_seed(void) {
    g_rng_state = (unsigned long)GetTickCount() ^ 0x85EBCA6BUL;
}

static float rng_unit(void) {   /* [0, 1) */
    g_rng_state = g_rng_state * 1664525UL + 1013904223UL;
    return (float)((g_rng_state >> 8) & 0xFFFFFF) / 16777216.0f;
}

#define MAX_VARS 9
typedef struct {
    char  name[64];
    int   off;      /* native App offset, -1 = custom */
    void* cslot;    /* custom preloaded SoundList (NULL until loaded) */
    int   dur;      /* measured length ms (0=unmeasured, -1=unknown) */
} sndvar_t;
typedef struct { char ev[64]; sndvar_t v[MAX_VARS]; int nvars; int trig; int loop; int delay_ms; int pause; int fired; int looping; int loop_var; DWORD lastfire; DWORD loop_last; } evtab_t;
static evtab_t g_evtab[MAX_EVENTS];
static int     g_evcount = 0;
static DWORD   g_prev_tick = 0;   /* game_update tick tracker (pause clock-shift) */

/* BOARD_PAUSED lives on the Board/Scene object returned by HB+ GetScene(). */
#define BOARD_PAUSED 0x874

/* 1 while the game is paused (pause menu), 0 otherwise. */
static int game_paused(void) {
    DWORD board;
    if (!g_api) return 0;
    board = (DWORD)HBAPI(g_api).GetScene();
    if (!board || IsBadReadPtr((void*)(board + BOARD_PAUSED), 4)) return 0;
    return *(int*)(board + BOARD_PAUSED) ? 1 : 0;
}

/* Sound durations (ms) for gapless loops: vorbis granule + wav header. */
static char g_game_dir[MAX_PATH];   /* exe dir, resolved at init (def below) */
static unsigned dur_rd32le(const unsigned char* p) {
    return (unsigned)p[0] | ((unsigned)p[1] << 8) |
           ((unsigned)p[2] << 16) | ((unsigned)p[3] << 24);
}

static int ogg_duration_ms(const unsigned char* head, int headlen,
                           const unsigned char* tail, int taillen) {
    int i, rate = 0;
    for (i = 0; i + 16 < headlen; i++) {
        if (head[i] == 'v' && head[i+1] == 'o' && head[i+2] == 'r' &&
            head[i+3] == 'b' && head[i+4] == 'i' && head[i+5] == 's') {
            if (head[i+10] < 1 || head[i+10] > 8) continue;
            rate = (int)dur_rd32le(head + i + 11);
            if (rate < 8000 || rate > 192000) { rate = 0; continue; }
            break;
        }
    }
    if (!rate) return -1;
    for (i = taillen - 27; i >= 0; i--) {
        if (tail[i] == 'O' && tail[i+1] == 'g' &&
            tail[i+2] == 'g' && tail[i+3] == 'S') {
            unsigned lo = dur_rd32le(tail + i + 6);
            unsigned hi = dur_rd32le(tail + i + 10);
            double ms;
            if (hi == 0xFFFFFFFFUL && lo == 0xFFFFFFFFUL) continue;
            ms = ((double)hi * 4294967296.0 + (double)lo) * 1000.0 /
                 (double)rate;
            if (ms < 50.0 || ms > 60000.0) return -1;
            return (int)ms;
        }
    }
    return -1;
}

static int wav_duration_ms(const unsigned char* b, int len) {
    int i, byterate = 0, datasize = 0;
    double ms;
    if (len < 44 || b[0] != 'R' || b[1] != 'I' || b[2] != 'F' || b[3] != 'F')
        return -1;
    for (i = 12; i + 8 < len; i++) {
        if (b[i] == 'f' && b[i+1] == 'm' && b[i+2] == 't' && b[i+3] == ' ')
            byterate = (int)dur_rd32le(b + i + 16);
        else if (b[i] == 'd' && b[i+1] == 'a' && b[i+2] == 't' && b[i+3] == 'a')
            datasize = (int)dur_rd32le(b + i + 4);
    }
    if (byterate < 1000 || datasize <= 0) return -1;
    ms = (double)datasize * 1000.0 / (double)byterate;
    if (ms < 50.0 || ms > 60000.0) return -1;
    return (int)ms;
}

#define DUR_HEAD 4096
#define DUR_TAIL 66000
static unsigned char g_dur_head[DUR_HEAD];
static unsigned char g_dur_tail[DUR_TAIL];

/* Measure Sounds/<name>.ogg|.wav length. Returns ms, or -1. */
static int measure_file_dur(const char* name) {
    char path[MAX_PATH];
    HANDLE f;
    DWORD size, got;
    int isogg, ms;
    snprintf(path, sizeof(path), "%sSounds\\%s.ogg", g_game_dir, name);
    f = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    isogg = 1;
    if (f == INVALID_HANDLE_VALUE) {
        snprintf(path, sizeof(path), "%sSounds\\%s.wav", g_game_dir, name);
        f = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        isogg = 0;
        if (f == INVALID_HANDLE_VALUE) return -1;
    }
    size = GetFileSize(f, NULL);
    if (size == 0xFFFFFFFF || size < 64) { CloseHandle(f); return -1; }
    got = 0;
    if (!ReadFile(f, g_dur_head, DUR_HEAD, &got, NULL) || got < 64) {
        CloseHandle(f);
        return -1;
    }
    if (!isogg) {
        CloseHandle(f);
        return wav_duration_ms(g_dur_head, (int)got);
    }
    {
        DWORD tlen = size > DUR_TAIL ? DUR_TAIL : size;
        DWORD troff = size - tlen;
        DWORD tg = 0;
        SetFilePointer(f, (LONG)troff, NULL, FILE_BEGIN);
        if (!ReadFile(f, g_dur_tail, tlen, &tg, NULL) || tg < 64) {
            CloseHandle(f);
            return -1;
        }
        CloseHandle(f);
        ms = ogg_duration_ms(g_dur_head, (int)got, g_dur_tail, (int)tg);
    }
    return ms;
}

/* Custom (non-vanilla) sounds: persistent game SoundLists, loaded once. */
#define MAX_CUSTOM 16
typedef struct { char name[64]; void* slot; } custom_t;
static custom_t g_custom[MAX_CUSTOM];
static int      g_custom_n = 0;

static void resolve_game_dir(char* out) {
    out[0] = '\0';
    GetModuleFileNameA(NULL, out, MAX_PATH);
    {
        char* slash = NULL;
        char* p;
        for (p = out; *p; p++) if (*p == '\\') slash = p;
        if (slash) *(slash + 1) = '\0';
    }
}

static void* find_custom(const char* name) {
    int i;
    for (i = 0; i < g_custom_n; i++)
        if (name_eq_ci(name, g_custom[i].name)) return g_custom[i].slot;
    return NULL;
}

/* Replicate native Sound_LoadAndAppend (0x4664F0), verified in binary:
 *   L = game_operator_new(0x420)
 *   SoundList_Ctor(L, device)          __thiscall, RET 4
 *   AthenaList_Append(device+4, L)     __thiscall, RET 4
 *   Sound_LoadOggOrWav(L, name, bufs)  ECX=L, push bufs, push name, RET 8
 * device = *(App+0x178); its +0x84C is the DSound interface. */
typedef void* (__cdecl *opnew_t)(unsigned);
typedef void (__thiscall *sndctor_t)(void*, void*);
typedef void (__thiscall *append_t)(void*, void*);
static opnew_t   pfn_opnew   = (opnew_t)0x004BA57B;
static sndctor_t pfn_sndctor = (sndctor_t)0x00458F20;
static append_t  pfn_append  = (append_t)0x00453810;

static void call_sound_load(void* L, const char* name, int bufs) {
    DWORD fn = 0x00459660;
    __asm__ __volatile__(
        "push %3\n"
        "push %2\n"
        "movl %1, %%ecx\n"
        "call *%0\n"
        : /* no output */
        : "r" (fn), "r" (L), "r" (name), "r" (bufs)
        : "eax", "ecx", "edx", "memory");
}

static void* load_custom_sound(const char* name) {
    App* app;
    void* device;
    void* L;
    char rel[128];
    char chk[MAX_PATH];
    DWORD fa;
    if (!name || !name[0] || !g_api) return NULL;
    /* file must exist: Sounds\<name>.ogg or .wav (exe-dir absolute) */
    snprintf(chk, sizeof(chk), "%sSounds\\%s.ogg", g_game_dir, name);
    fa = GetFileAttributesA(chk);
    if (fa == 0xFFFFFFFF) {
        snprintf(chk, sizeof(chk), "%sSounds\\%s.wav", g_game_dir, name);
        fa = GetFileAttributesA(chk);
        if (fa == 0xFFFFFFFF) return NULL;
    }
    app = HBAPI(g_api).GetApp();
    if (!app || IsBadReadPtr(app, 0x600)) return NULL;
    device = *(void**)((char*)app + 0x178);
    if (!device || IsBadReadPtr(device, 0x900)) return NULL;
    L = pfn_opnew(0x420);
    if (!L || IsBadReadPtr(L, 0x420)) return NULL;
    pfn_sndctor(L, device);
    pfn_append((char*)device + 4, L);
    snprintf(rel, sizeof(rel), "sounds\\%s", name);
    call_sound_load(L, rel, 3);
    return L;
}

/* Minimal scanner: finds "E:..." then its { "sound"[1-9]: "..." } variants. */
static int parse_set_buf(const char* buf, evtab_t* out, int cap) {
    int n = 0;
    const char* p = buf;
    while (*p && n < cap) {
        const char* q;
        int ni = 0;
        char nm[64]; nm[0] = '\0';
        int nv = 0;
        int trig = 0;
        int loop = 0;
        int delay_ms = 0;
        int pausev = 1;   /* default true: pause event sounds with the game */
        if (*p != '"') { p++; continue; }
        /* candidate string: must start with E: */
        if (!(p[1] == 'E' && p[2] == ':')) { p++; continue; }
        q = p + 1;
        while (*q && *q != '"' && ni < 63) { nm[ni++] = *q++; }
        nm[ni] = '\0';
        if (*q != '"') { p++; continue; }
        p = q + 1;
        /* find opening brace of its property block */
        while (*p && *p != '{' && *p != '"') p++;
        if (*p != '{') continue;
        p++;
        /* inside block: trigger_type, loop_sound, "sound"/"sound1".."sound9" */
        while (*p && *p != '}') {
            /* "trigger_type": 0/1/2/3 */
            if (*p == '"' && p[1] == 't' && p[2] == 'r' && p[3] == 'i' &&
                p[4] == 'g' && p[5] == 'g' && p[6] == 'e' && p[7] == 'r' &&
                p[8] == '_' && p[9] == 't' && p[10] == 'y' && p[11] == 'p' &&
                p[12] == 'e' && p[13] == '"') {
                const char* r = p + 14;
                while (*r == ':' || *r == ' ' || *r == '\t') r++;
                trig = (*r >= '0' && *r <= '9') ? (*r - '0') : 0;
                if (trig < 0 || trig > 3) trig = 0;
                p = r + 1;
                continue;
            }
            /* "loop_delay": seconds (decimals ok) -> ms */
            if (*p == '"' && p[1] == 'l' && p[2] == 'o' && p[3] == 'o' &&
                p[4] == 'p' && p[5] == '_' && p[6] == 'd' && p[7] == 'e' &&
                p[8] == 'l' && p[9] == 'a' && p[10] == 'y' && p[11] == '"') {
                const char* r = p + 12;
                int neg = 0, ip = 0, fp = 0, fd = 0;
                while (*r == ':' || *r == ' ' || *r == '\t') r++;
                if (*r == '-') { neg = 1; r++; }
                while (*r >= '0' && *r <= '9') { ip = ip * 10 + (*r - '0'); r++; }
                if (*r == '.') {
                    r++;
                    while (*r >= '0' && *r <= '9' && fd < 3) {
                        fp = fp * 10 + (*r - '0'); r++; fd++;
                    }
                    while (*r >= '0' && *r <= '9') r++;
                }
                while (fd < 3) { fp *= 10; fd++; }
                delay_ms = ip * 1000 + fp;
                if (neg) delay_ms = -delay_ms;
                if (delay_ms < 0) delay_ms = 0;
                if (delay_ms > 86400000) delay_ms = 86400000;
                while (*r && *r != ',' && *r != '}') r++;
                p = r;
                continue;
            }
            /* "loop_sound": true/false/1/0 */
            if (*p == '"' && p[1] == 'l' && p[2] == 'o' && p[3] == 'o' &&
                p[4] == 'p' && p[5] == '_' && p[6] == 's' && p[7] == 'o' &&
                p[8] == 'u' && p[9] == 'n' && p[10] == 'd' && p[11] == '"') {
                const char* r = p + 12;
                while (*r == ':' || *r == ' ' || *r == '\t') r++;
                if (*r == 't' || *r == 'T' || *r == '1') loop = 1;
                else loop = 0;
                while (*r && *r != ',' && *r != '}') r++;
                p = r;
                continue;
            }
            /* "pause": true/false/1/0 (default true) */
            if (*p == '"' && p[1] == 'p' && p[2] == 'a' && p[3] == 'u' &&
                p[4] == 's' && p[5] == 'e' && p[6] == '"') {
                const char* r = p + 7;
                while (*r == ':' || *r == ' ' || *r == '\t') r++;
                if (*r == 't' || *r == 'T' || *r == '1') pausev = 1;
                else if (*r == 'f' || *r == 'F' || *r == '0') pausev = 0;
                while (*r && *r != ',' && *r != '}') r++;
                p = r;
                continue;
            }
            if (*p == '"' && p[1] == 's' && p[2] == 'o' && p[3] == 'u' &&
                p[4] == 'n' && p[5] == 'd') {
                const char* r = p + 6;
                char sv[64]; int vi = 0;
                if (nv >= MAX_VARS) { p++; continue; }   /* max 9 variants */
                if (*r >= '1' && *r <= '9') r++;   /* soundN variant */
                if (*r != '"') { p++; continue; }
                r++;   /* past key's closing quote */
                /* skip : and spaces to the value's opening quote */
                while (*r == ':' || *r == ' ' || *r == '\t') r++;
                if (*r != '"') { p = r; continue; }
                r++;
                while (*r && *r != '"' && vi < 63) { sv[vi++] = *r++; }
                sv[vi] = '\0';
                if (*r == '"') {
                    int off = sound_offset(sv);
                    char lbuf[128];
                    strncpy(out[n].v[nv].name, sv,
                            sizeof(out[n].v[nv].name) - 1);
                    out[n].v[nv].name[sizeof(out[n].v[nv].name) - 1] = '\0';
                    out[n].v[nv].off = off;
                    out[n].v[nv].cslot = NULL;
                    out[n].v[nv].dur = 0;
                    snprintf(lbuf, sizeof(lbuf), "SET event %s sound=%s off=0x%X",
                             nm, sv, off < 0 ? 0 : (unsigned)off);
                    log_mod(lbuf);
                    nv++;
                    p = r + 1;
                    continue;
                }
            }
            p++;
        }
        if (nv == 0) continue;   /* no sound vars: not a sound event (yet) */
        strncpy(out[n].ev, nm, sizeof(out[n].ev) - 1);
        out[n].ev[sizeof(out[n].ev) - 1] = '\0';
        out[n].nvars = nv;
        out[n].trig = trig;
        out[n].loop = loop;
        out[n].delay_ms = delay_ms;
        out[n].pause = pausev;
        out[n].fired = 0;
        out[n].looping = 0;
        out[n].loop_var = 0;
        out[n].lastfire = 0;
        {
            char tbuf[112];
            snprintf(tbuf, sizeof(tbuf), "SET event %s trig=%d loop=%d delay=%dms pause=%d vars=%d",
                     nm, trig, loop, delay_ms, pausev, nv);
            log_mod(tbuf);
        }
        n++;
    }
    return n;
}

/* Strip // comments (outside quotes) + CR bytes, in place. */
static void strip_comments(char* buf) {
    int inq = 0;
    char* r = buf;
    char* w = buf;
    while (*r) {
        if (*r == '"') { inq = !inq; *w++ = *r++; continue; }
        if (!inq && *r == '/' && *(r + 1) == '/') {
            while (*r && *r != '\n') r++;
            continue;
        }
        if (*r == '\r') { r++; continue; }
        *w++ = *r++;
    }
    *w = '\0';
}

static void set_load(const char* why) {
    HANDLE f;
    DWORD got = 0;
    static char sbuf[SET_SIZE_CAP + 1];
    static evtab_t tmp[MAX_EVENTS];
    int n;
    char hbuf[64];
    if (!g_set_path[0]) return;
    f = CreateFileA(g_set_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f == INVALID_HANDLE_VALUE) {
        create_default_set();
        f = CreateFileA(g_set_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (f == INVALID_HANDLE_VALUE) {
            log_mod("SET missing, default not writable");
            return;
        }
    }
    if (!ReadFile(f, sbuf, SET_SIZE_CAP, &got, NULL)) got = 0;
    CloseHandle(f);
    sbuf[got < SET_SIZE_CAP ? got : SET_SIZE_CAP] = '\0';
    /* skip UTF-8 BOM */
    {
        char* start = sbuf;
        if (got >= 3 && (unsigned char)start[0] == 0xEF &&
            (unsigned char)start[1] == 0xBB && (unsigned char)start[2] == 0xBF)
            start += 3;
        if (start != sbuf) {
            char* d = sbuf;
            while ((*d++ = *start++) != '\0') ;
        }
    }
    strip_comments(sbuf);
    n = parse_set_buf(sbuf, tmp, MAX_EVENTS);
    if (n == 0 && g_evcount > 0) {
        log_mod("SET parse yielded 0 events, keeping previous table");
        return;
    }
    {
        int i, k;
        for (i = 0; i < n; i++) { tmp[i].lastfire = 0; tmp[i].loop_last = 0; }
        for (i = 0; i < n; i++) {
            for (k = 0; k < tmp[i].nvars; k++) {
                if (tmp[i].v[k].off < 0)
                    tmp[i].v[k].cslot = find_custom(tmp[i].v[k].name);
                /* carry measured duration across set re-reads */
                {
                    int oi, ok;
                    for (oi = 0; oi < g_evcount; oi++) {
                        if (!name_eq_ci(tmp[i].ev, g_evtab[oi].ev)) continue;
                        for (ok = 0; ok < g_evtab[oi].nvars; ok++) {
                            if (name_eq_ci(tmp[i].v[k].name,
                                           g_evtab[oi].v[ok].name)) {
                                tmp[i].v[k].dur = g_evtab[oi].v[ok].dur;
                                break;
                            }
                        }
                    }
                }
            }
            g_evtab[i] = tmp[i];
        }
        g_evcount = n;
    }
    snprintf(hbuf, sizeof(hbuf), "SET %s: %d sound events", why, n);
    log_mod(hbuf);
}

/* ═══════════════ Callbacks ═══════════════ */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) operator delete(thisptr);
    return thisptr;
}
static const char* __thiscall get_mod_name(void*) { return "Battyball Events"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, IModAPI* api) {
    g_api = api;
    *(void**)((char*)thisptr + 4) = api;

    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery((void*)&init_impl, &mbi, sizeof(mbi));
    g_module = (HMODULE)mbi.AllocationBase;
    resolve_log_path(g_log_path);
    resolve_set_path(g_set_path);
    resolve_game_dir(g_game_dir);
    rng_seed();

    CustomButton btn("BATTY_EVENTS", "Battyball Events");
    btn.defaultState = true;
    HBAPI(api).CreateToggleButton(btn, (HamsterballAPI*)thisptr);

    log_mod("INIT Battyball Events v1j (mod loaded)");
    set_load("init");
}

static void __thiscall button_toggle(void*, const char* id, bool state) {
    if (!id || IsBadReadPtr((void*)id, 1)) return;
    if (strcmp(id, "BATTY_EVENTS") == 0) {
        int i;
        g_enabled = state;
        g_applied = true;
        if (!state)
            for (i = 0; i < g_evcount; i++) g_evtab[i].looping = 0;
        log_mod(state ? "STATE Battyball Events ON" : "STATE Battyball Events OFF");
    }
}

static void __thiscall slider_change(void*, const char*, float) {}
static void __thiscall cycle_option_change(void*, const char*, const char*) {}
static void __thiscall ball_update(void*, void*) {}
static void __thiscall render_apply(void*, void*, float*) {}
static void __thiscall text_render(void*) {}
static void __thiscall ball_bump(void*, void*, void*) {}

static void ensure_dur(sndvar_t* sv);
static void play_event_sound(int app_off, void* cslot, void* ball);

static void __thiscall level_start(void*) {
    int i, k;
    log_mod("LEVEL start");
    set_load("level");
    /* preload custom (non-vanilla) sounds, once per name per session */
    for (i = 0; i < g_evcount; i++) {
        for (k = 0; k < g_evtab[i].nvars; k++) {
            sndvar_t* sv;
            void* L;
            char lbuf[128];
            if (g_evtab[i].v[k].off >= 0 || g_evtab[i].v[k].cslot) continue;
            if (g_custom_n >= MAX_CUSTOM) {
                log_mod("SFX custom table full, skipping");
                break;
            }
            sv = &g_evtab[i].v[k];
            L = load_custom_sound(sv->name);
            if (L) {
                strncpy(g_custom[g_custom_n].name, sv->name,
                        sizeof(g_custom[g_custom_n].name) - 1);
                g_custom[g_custom_n].name[sizeof(g_custom[g_custom_n].name) - 1] = '\0';
                g_custom[g_custom_n].slot = L;
                g_custom_n++;
                sv->cslot = L;
            }
            snprintf(lbuf, sizeof(lbuf), "SFX preload %s %s",
                     sv->name, L ? "OK" : "FAIL");
            log_mod(lbuf);
        }
    }
    /* trig 2/3: fire at level start (2 loops with loop_delay gap) */
    for (i = 0; i < g_evcount; i++) {
        int nv, pick;
        sndvar_t* sv;
        void* pb;
        char abuf[128];
        if (g_evtab[i].trig < 2 || g_evtab[i].nvars <= 0) continue;
        nv = g_evtab[i].nvars;
        pick = (int)(rng_unit() * (float)nv);
        if (pick < 0) pick = 0;
        if (pick >= nv) pick = nv - 1;
        sv = &g_evtab[i].v[pick];
        if (sv->off < 0 && !sv->cslot) {
            snprintf(abuf, sizeof(abuf), "SFX not loaded %s for %s",
                     sv->name, g_evtab[i].ev);
            log_mod(abuf);
            continue;
        }
        ensure_dur(sv);
        pb = g_api ? (void*)HBAPI(g_api).GetPlayer() : NULL;
        play_event_sound(sv->off, sv->cslot, pb);
        g_evtab[i].fired = 1;
        if (g_evtab[i].trig == 2) {
            g_evtab[i].looping = 1;
            g_evtab[i].loop_var = pick;
            g_evtab[i].loop_last = GetTickCount();
            snprintf(abuf, sizeof(abuf), "SFX autoloop start %s for %s (#%d)",
                     sv->name, g_evtab[i].ev, pick + 1);
        } else {
            snprintf(abuf, sizeof(abuf), "SFX autostart %s for %s (#%d)",
                     sv->name, g_evtab[i].ev, pick + 1);
        }
        log_mod(abuf);
    }
}

static void __thiscall scene_end(void*) {
    int i;
    for (i = 0; i < g_evcount; i++) g_evtab[i].looping = 0;
    g_prev_tick = 0;   /* re-sync pause clock next level */
    log_mod("SCENE end");
}

static void play_event_sound(int app_off, void* cslot, void* ball);

static void __thiscall game_update(void*) {
    int i;
    /* One-shot guard: saved toggle state does NOT re-fire
     * onButtonToggle, so adopt it here on first frame. */
    if (!g_applied && g_api) {
        g_enabled = HBAPI(g_api).GetButtonState("BATTY_EVENTS");
        g_applied = true;
        char sbuf[48];
        snprintf(sbuf, sizeof(sbuf), "STATEstart enabled=%d",
                 g_enabled ? 1 : 0);
        log_mod(sbuf);
    }
    /* loop: re-pick randomly each cycle, re-fire when it ends */
    if (!g_enabled) return;
    {
        DWORD now = GetTickCount();
        int paused = game_paused();
        if (!g_prev_tick) g_prev_tick = now;
        if (paused) {
            /* Freeze pause-aware loop clocks so paused time never counts:
             * resume continues exactly where it left off, no burst. */
            DWORD delta = now - g_prev_tick;
            if (delta) {
                for (i = 0; i < g_evcount; i++)
                    if (g_evtab[i].looping && g_evtab[i].pause)
                        g_evtab[i].loop_last += delta;
            }
            g_prev_tick = now;
        } else {
            g_prev_tick = now;
        }
        for (i = 0; i < g_evcount; i++) {
            sndvar_t* sv;
            void* pb;
            int gate, pick;
            if (!g_evtab[i].looping) continue;
            if (g_evtab[i].nvars <= 0) continue;
            if (paused && g_evtab[i].pause) continue;   /* pause=true: silent while paused */
            /* gate: loop_delay = period between starts (overlap ok);
             * delay 0 = back-to-back by sound length */
            gate = g_evtab[i].delay_ms > 0 ? g_evtab[i].delay_ms : 1000;
            if (g_evtab[i].delay_ms <= 0 &&
                g_evtab[i].loop_var >= 0 &&
                g_evtab[i].loop_var < g_evtab[i].nvars &&
                g_evtab[i].v[g_evtab[i].loop_var].dur > 0)
                gate = g_evtab[i].v[g_evtab[i].loop_var].dur;
            if ((int)(now - g_evtab[i].loop_last) < gate) continue;
            pick = (int)(rng_unit() * (float)g_evtab[i].nvars);
            if (pick < 0) pick = 0;
            if (pick >= g_evtab[i].nvars) pick = g_evtab[i].nvars - 1;
            sv = &g_evtab[i].v[pick];
            g_evtab[i].loop_last = now;
            g_evtab[i].loop_var = pick;
            pb = g_api ? (void*)HBAPI(g_api).GetPlayer() : NULL;
            play_event_sound(sv->off, sv->cslot, pb);
        }
    }
}

/* E: tap — logs each distinct event-plane name (throttled per name). */
static char  g_last_ev[64] = { 0 };
static DWORD g_last_ev_tick = 0;

/* Measure variant length once (0=unmeasured, -1=unknown). */
static void ensure_dur(sndvar_t* sv) {
    char dbuf[96];
    if (sv->dur != 0) return;
    sv->dur = measure_file_dur(sv->name);
    if (sv->dur < 0) sv->dur = -1;
    snprintf(dbuf, sizeof(dbuf), "SFX dur %s %dms", sv->name, sv->dur);
    log_mod(dbuf);
}

/* Play native slot (off>=0) or custom preloaded SoundList (cslot). */
static void play_event_sound(int app_off, void* cslot, void* ball) {
    App* app;
    void* slot;
    float x = 0.0f, y = 0.0f, z = 0.0f;
    if (!g_api) return;
    app = HBAPI(g_api).GetApp();
    if (!app || IsBadReadPtr(app, 0x600)) return;
    if (app_off >= 0) {
        slot = *(void**)((char*)app + app_off);
    } else {
        slot = cslot;
    }
    if (!slot || IsBadReadPtr(slot, 4)) return;
    if (ball && !IsBadReadPtr(ball, 0x200)) {
        x = *(float*)((char*)ball + 0x164);
        y = *(float*)((char*)ball + 0x168);
        z = *(float*)((char*)ball + 0x16C);
    }
    HBAPI(g_api).Play3dSoundEffect(slot, Vec3(x, y, z), 1.0f);
}

static void __thiscall event_collide(void* self, void* ball, char* name) {
    DWORD now;
    int i;
    char ebuf[128];
    char nm[64];
    (void)self;
    if (!g_enabled) return;
    if (!name || IsBadReadPtr(name, 1)) return;
    /* copy name safely (max 63 chars) */
    for (i = 0; i < 63; i++) {
        char c = 0;
        if (IsBadReadPtr(name + i, 1)) break;
        c = name[i];
        nm[i] = c;
        if (!c) break;
    }
    nm[i] = '\0';
    if (!nm[0]) return;
    now = GetTickCount();
    if (strcmp(nm, g_last_ev) == 0 && (int)(now - g_last_ev_tick) < 500)
        return;   /* same plane spamming: throttle */
    strncpy(g_last_ev, nm, sizeof(g_last_ev) - 1);
    g_last_ev[sizeof(g_last_ev) - 1] = '\0';
    g_last_ev_tick = now;
    ebuf[0] = '\0';
    snprintf(ebuf, sizeof(ebuf), "EVENT %s", nm);
    log_mod(ebuf);
    /* sound event? trig 2/3 fire at level start, never on touch */
    for (i = 0; i < g_evcount; i++) {
        if (name_eq_ci(nm, g_evtab[i].ev)) {
            int nv = g_evtab[i].nvars;
            int pick;
            sndvar_t* sv;
            if (nv <= 0) break;
            if (g_evtab[i].trig >= 2) break;   /* level-start events: no touch */
            if (g_evtab[i].trig == 1 && g_evtab[i].fired) break;   /* once ever */
            if (g_evtab[i].pause && game_paused()) {
                char qbuf[96];
                snprintf(qbuf, sizeof(qbuf), "SFX paused skip %s", nm);
                log_mod(qbuf);
                break;   /* pause=true: silent while paused */
            }
            pick = (int)(rng_unit() * (float)nv);
            if (pick < 0) pick = 0;
            if (pick >= nv) pick = nv - 1;
            sv = &g_evtab[i].v[pick];
            if (sv->off < 0 && !sv->cslot) {
                char ubuf[96];
                snprintf(ubuf, sizeof(ubuf), "SFX not loaded %s for %s",
                         sv->name, nm);
                log_mod(ubuf);
                break;
            }
            if ((int)(now - g_evtab[i].lastfire) >= 1000) {
                char pbuf[128];
                g_evtab[i].lastfire = now;
                if (g_evtab[i].trig == 1) g_evtab[i].fired = 1;
                if (g_evtab[i].loop == 1) {
                    g_evtab[i].looping = 1;
                    g_evtab[i].loop_var = pick;   /* current pick for gate */
                    ensure_dur(sv);
                    play_event_sound(sv->off, sv->cslot, ball);
                    g_evtab[i].loop_last = now;
                    snprintf(pbuf, sizeof(pbuf), "SFX loop start %s for %s (#%d)",
                             sv->name, nm, pick + 1);
                } else {
                    play_event_sound(sv->off, sv->cslot, ball);
                    snprintf(pbuf, sizeof(pbuf), "SFX play %s for %s (#%d)",
                             sv->name, nm, pick + 1);
                }
                log_mod(pbuf);
            }
            break;
        }
    }
}

/* 17-entry vtable (HB+ v2.0/v2.1) */
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
