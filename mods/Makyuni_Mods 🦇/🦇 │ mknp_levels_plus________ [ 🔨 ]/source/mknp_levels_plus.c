/*
 * mknp_levels_plus v01a — Levels Plus (bass.dll proxy)
 * Combines mknp_custom_filenames (mesh PUSH) + mknp_separate_music (per-level song)
 * + RaceData overrides (TIME/PAR/medals/CAM -> board).
 * Set: mknp_levels_plus.set next to DLL (single jsonc array-pairs file).
 * Absent key = no-op (never write defaults). Re-read per loop, keep-old-on-error.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define MOD_NAME "mknp_levels_plus"
#define MOD_VER "v01aj"

/* ── log (DLL dir -> exe dir -> %TEMP%), mknp_ prefix ── */
static HMODULE g_hSelf = NULL;
static char g_log_path[MAX_PATH] = "";
static char g_set_path[MAX_PATH] = "";
static char g_exeDir[MAX_PATH] = "";
static char g_dllDir[MAX_PATH] = "";

static int try_commit_log(const char *dir, const char *fname, char *out) {
    char p[MAX_PATH];
    FILE *probe = NULL;
    snprintf(p, MAX_PATH, "%s%s", dir, fname);
    p[MAX_PATH-1] = '\0';
    if (fopen_s(&probe, p, "a") != 0 || !probe) return 0;
    fclose(probe);
    strcpy_s(out, MAX_PATH, p);
    return 1;
}
static void split_dir(char *path) {
    char *s = strrchr(path, '\\');
    if (s) *(s+1) = '\0';
}
static void init_paths(void) {
    char mod_path[MAX_PATH];
    g_log_path[0] = '\0'; g_set_path[0] = '\0';
    if (g_hSelf && GetModuleFileNameA(g_hSelf, mod_path, MAX_PATH) > 0) {
        strcpy_s(g_dllDir, MAX_PATH, mod_path);
        split_dir(g_dllDir);
        snprintf(g_set_path, MAX_PATH, "%smknp_levels_plus.set", g_dllDir);
        if (try_commit_log(g_dllDir, "mknp_levels_plus.log", g_log_path)) {
            /* ok */
        }
    }
    {
        HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
        if (!hExe) hExe = GetModuleHandleA(NULL);
        if (hExe && GetModuleFileNameA(hExe, mod_path, MAX_PATH) > 0) {
            strcpy_s(g_exeDir, MAX_PATH, mod_path);
            split_dir(g_exeDir);
            if (!g_log_path[0]) try_commit_log(g_exeDir, "mknp_levels_plus.log", g_log_path);
            if (!g_set_path[0] || GetFileAttributesA(g_set_path) == INVALID_FILE_ATTRIBUTES) {
                char alt[MAX_PATH];
                snprintf(alt, MAX_PATH, "%smknp_levels_plus.set", g_exeDir);
                if (GetFileAttributesA(alt) != INVALID_FILE_ATTRIBUTES)
                    strcpy_s(g_set_path, MAX_PATH, alt);
            }
        }
    }
    if (!g_log_path[0]) {
        char tmp[MAX_PATH];
        if (GetTempPathA(MAX_PATH, tmp) > 0)
            try_commit_log(tmp, "mknp_levels_plus.log", g_log_path);
    }
    if (!g_log_path[0]) strcpy_s(g_log_path, MAX_PATH, "mknp_levels_plus.log");
}
static void mod_log(const char *fmt, ...) {
    FILE *f = NULL;
    va_list ap;
    if (fopen_s(&f, g_log_path, "a") != 0 || !f) return;
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);
    fprintf(f, "\n");
    fclose(f);
}

/* ── BASS proxy ── */
static HMODULE g_hRealBass = NULL;
typedef int (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}
typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
static DWORD g_gameMusic = 0;
static DWORD g_customMusic = 0;
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int a, void* b, DWORD c, DWORD d, DWORD e, DWORD f) {
    DWORD h = 0;
    if (real_BASS_MusicLoad) h = real_BASS_MusicLoad(a, b, c, d, e, f);
    if (!a && b && h) {
        const char *p = (const char*)b;
        char low[MAX_PATH];
        size_t i;
        strncpy(low, p, sizeof(low)-1); low[sizeof(low)-1] = '\0';
        for (i = 0; low[i]; i++) if (low[i] >= 'A' && low[i] <= 'Z') low[i] += 32;
        if (strstr(low, "music.mo3")) {
            const char *m = strstr(low, "music.mo3");
            if (m == low || m[-1] == '\\' || m[-1] == '/') { g_gameMusic = h; }
        }
    }
    return h;
}
typedef int (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL, DWORD);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
typedef int (__stdcall *BASS_MusicFree_t)(DWORD);
static BASS_MusicFree_t real_BASS_MusicFree = NULL;
typedef void (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
typedef int (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
typedef int (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
typedef int (__stdcall *BASS_Init_t)(int, int, int, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, int b, int c, HWND d, void* e) {
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 1;
}
typedef int (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (real_BASS_Free) return real_BASS_Free();
    return 1;
}
typedef int (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (real_BASS_Start) return real_BASS_Start();
    return 1;
}
typedef int (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (real_BASS_Stop) return real_BASS_Stop();
    return 1;
}
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}
__declspec(dllexport) void __stdcall BASS_ChannelStop(DWORD a) {
    if (a == g_customMusic) g_customMusic = 0;
    if (real_BASS_ChannelStop) real_BASS_ChannelStop(a);
}

/* ── set state: 15 levels (1..15) ── */
#define MAX_LEVEL_NAME 64
typedef struct {
    int has_mesh; char mesh[MAX_LEVEL_NAME+16];
    int has_time; float time_v;
    int has_par; float par_v;
    int has_bronze; float bronze_v;
    int has_silver; float silver_v;
    int has_gold; float gold_v;
    int has_weasel; float weasel_v;
    int has_cam; float cam_v;
    int has_rotx; float rotx_v;
    int has_roty; float roty_v;
    int has_rotz; float rotz_v;
    int has_offx; float offx_v;
    int has_offy; float offy_v;
    int has_offz; float offz_v;
    int has_thumb_t; char thumb_t[128];
    int has_thumb_p; char thumb_p[128];
    int has_song; char song[128];
    int has_song_hex; unsigned song_hex;
    int has_intro_hex; unsigned intro_hex;
    int has_intro_file; char intro_file[128];
    int has_name; char name_v[64];
    int has_desc; char desc_v[256];
    int has_name_color; float nc_r, nc_g, nc_b, nc_a;
    int has_strip; float sc_r, sc_g, sc_b, sc_a;
    int has_timeblot; float tb_r, tb_g, tb_b, tb_a;
} level_t;
static level_t g_lv[15];
static float g_blot_alpha = 1.0f;
static char g_race_strings[15][MAX_LEVEL_NAME+16];
static char g_arena_strings[15][MAX_LEVEL_NAME+16];
/* live display names: stock copy at startup, set overrides where has_name */
static char g_names[15][64];
static const DWORD g_menu_name_rvas[15] = {
    0x2EEA7,0x2EEE1,0x2EF1B,0x2EFA0,0x2F01A,0x2F094,0x2F114,
    0x2F18E,0x2F205,0x2F288,0x2F302,0x2F37C,0x2F3FF,0x2F479,0x2F4ED
};
#define NAME_TABLE_BASE 0x4F7080
/* menu text-color PUSH-immediate operand addrs (PracticeMenu_ctor): A,B,G,R per level */
static const DWORD g_menu_col_rvas[15][4] = {
    {0x2EE85,0x2EE8A,0x2EE8F,0x2EE94},   /* L01 WarmUp: push A,B,G,R */
    {0x2EEC4,0x2EEC9,0x2EECE,0x2EED3},   /* L02 */
    {0x2EEFE,0x2EF03,0x2EF08,0x2EF0D},   /* L03 */
    {0x2EF51,0x2EF56,0x2EF5B,0x2EF60},   /* L04 */
    {0x2EFCB,0x2EFD0,0x2EFD5,0x2EFDA},   /* L05 */
    {0x2F045,0x2F04A,0x2F04F,0x2F054},   /* L06 */
    {0,0,0,0},                           /* L07 Neon: code cave (push 0) */
    {0x2F13F,0x2F144,0x2F149,0x2F14E},   /* L08 */
    {0,0,0,0},                           /* L09 Odd: code cave */
    {0x2F239,0x2F23E,0x2F243,0x2F248},   /* L10 */
    {0x2F2B3,0x2F2B8,0x2F2BD,0x2F2C2},   /* L11 */
    {0x2F32D,0x2F332,0x2F337,0x2F33C},   /* L12 */
    {0x2F3B0,0x2F3B5,0x2F3BA,0x2F3BF},   /* L13 */
    {0x2F42A,0x2F42F,0x2F434,0x2F439},   /* L14 */
    {0,0,0,0},                           /* L15 Impossible: code cave */
};
static void patch_float_rva(DWORD rva, float v) {
    HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
    DWORD addr;
    DWORD old;
    if (!hExe) hExe = GetModuleHandleA(NULL);
    if (!hExe) return;
    addr = (DWORD)hExe + rva;
    if (VirtualProtect((void*)addr, 4, PAGE_READWRITE, &old)) {
        *(float*)addr = v;
        VirtualProtect((void*)addr, 4, old, &old);
    }
}

static const char *kDefRace[15] = {
    "levels\\level1","levels\\levelcascade","levels\\level2",
    "levels\\level3","levels\\level4","levels\\levelup",
    "levels\\leveldark","levels\\level5","levels\\level6",
    "levels\\level8","levels\\level7","levels\\levelglass",
    "levels\\level9","levels\\level10","levels\\levelimpossible"
};
static const char *kDefArena[15] = {
    "levels\\arena-WarmUp","levels\\arena-beginner","levels\\arena-intermediate",
    "levels\\arena-dizzy","levels\\arena-tower","levels\\arena-up",
    "levels\\arena-neon","levels\\arena-expert","levels\\arena-Odd",
    "levels\\arena-Toob","levels\\arena-Wobbly","levels\\arena-glass",
    "levels\\arena-Sky","levels\\arena-Master","levels\\arena-impossible"
};

static const DWORD g_race_push_rvas[15] = {
    0x0d202,0x11115,0x0d2c2,0x0d3d4,0x0d712,0x1158b,0x162b5,
    0x0e1d2,0x0ead3,0x0fa65,0x0f3a2,0x17683,0x10874,0x11fa2,0x17f62
};
static const DWORD g_arena_push_rvas[15] = {
    0x13c62,0x13d25,0x141c2,0x14282,0x144f2,0x149a2,0x16f83,
    0x14b52,0x14d22,0x14f45,0x153e2,0x17e32,0x15904,0x160c2,0x18582
};

/* tiny helpers */
static const char *skip_ws(const char *p) {
    while (*p==' '||*p=='\t'||*p=='\r'||*p=='\n') p++;
    return p;
}
static int parse_float(const char *s, float *out) {
    char *e = NULL;
    double d = strtod(s, &e);
    if (e == s) return 0;
    *out = (float)d;
    return 1;
}
static int parse_uint(const char *s, unsigned *out) {
    char *e = NULL;
    unsigned long v;
    if (s[0]=='0' && (s[1]=='x'||s[1]=='X')) v = strtoul(s, &e, 16);
    else v = strtoul(s, &e, 10);
    if (e == s) return 0;
    *out = (unsigned)v;
    return 1;
}
static void parse_quoted(const char *s, char *out, int outlen) {
    const char *q1, *q2;
    size_t n;
    out[0] = '\0';
    q1 = strchr(s, '"');
    if (!q1) return;
    q2 = strchr(q1+1, '"');
    if (!q2 || q2 <= q1+1) return;
    n = (size_t)(q2-(q1+1));
    if (n >= (size_t)outlen) n = (size_t)outlen-1;
    memcpy(out, q1+1, n);
    out[n] = '\0';
}

/* parse one "key": value pair inside { ... } into level idx (0-based) */
static int parse_rrggbbaa(const char *s, float *r, float *g, float *b, float *a) {
    char tmp[64]; int n = 0, i = 0;
    while (*s==' '||*s=='\t'||*s=='"') s++;
    if (*s=='#'||(*s=='0'&&(s[1]=='x'||s[1]=='X'))) { if (*s=='#') s++; else s+=2; }
    while (n < 8 && s[i]) {
        char c = s[i];
        if (!((c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F'))) break;
        tmp[n++] = c; i++;
    }
    tmp[n] = '\0';
    if (n != 6 && n != 8) return 0;
    {
        unsigned v = (unsigned)strtoul(tmp, NULL, 16);
        unsigned R, G, B, A;
        if (n == 6) { R=(v>>16)&255; G=(v>>8)&255; B=v&255; A=255; }
        else { R=(v>>24)&255; G=(v>>16)&255; B=(v>>8)&255; A=v&255; }
        *r = R/255.0f; *g = G/255.0f; *b = B/255.0f; *a = A/255.0f;
        return 1;
    }
}
static void apply_kv(int li, const char *key, const char *val) {
    float f; unsigned u; char tmp[256];
    if (strcmp(key,"mesh")==0) {
        parse_quoted(val, tmp, sizeof(tmp));
        if (tmp[0]) {
            snprintf(g_lv[li].mesh, sizeof(g_lv[li].mesh), "levels\\%s", tmp);
            g_lv[li].has_mesh = 1;
            snprintf(g_race_strings[li], sizeof(g_race_strings[li]), "levels\\%s", tmp);
        }
    } else if (strcmp(key,"time")==0) {
        if (parse_float(val,&f)) { g_lv[li].time_v=f; g_lv[li].has_time=1; }
    } else if (strcmp(key,"time_par")==0) {
        if (parse_float(val,&f)) { g_lv[li].par_v=f; g_lv[li].has_par=1; }
    } else if (strcmp(key,"time_bronze")==0) {
        if (parse_float(val,&f)) { g_lv[li].bronze_v=f; g_lv[li].has_bronze=1; }
    } else if (strcmp(key,"time_silver")==0) {
        if (parse_float(val,&f)) { g_lv[li].silver_v=f; g_lv[li].has_silver=1; }
    } else if (strcmp(key,"time_gold")==0) {
        if (parse_float(val,&f)) { g_lv[li].gold_v=f; g_lv[li].has_gold=1; }
    } else if (strcmp(key,"time_weasel")==0) {
        if (parse_float(val,&f)) { g_lv[li].weasel_v=f; g_lv[li].has_weasel=1; }
    } else if (strcmp(key,"camlocus_start")==0) {
        if (parse_float(val,&f)) { g_lv[li].cam_v=f; g_lv[li].has_cam=1; }
    } else if (strcmp(key,"cam_rotX")==0) {
        if (parse_float(val,&f)) { g_lv[li].rotx_v=f; g_lv[li].has_rotx=1; }
    } else if (strcmp(key,"cam_rotY")==0) {
        if (parse_float(val,&f)) { g_lv[li].roty_v=f; g_lv[li].has_roty=1; }
    } else if (strcmp(key,"cam_rotZ")==0) {
        if (parse_float(val,&f)) { g_lv[li].rotz_v=f; g_lv[li].has_rotz=1;
            { static int rz_logged = 0; if (!rz_logged) { rz_logged = 1;
            mod_log("[levels_plus] cam_rotZ ignored (orbit has no roll input)"); } } }
    } else if (strcmp(key,"cam_offX")==0) {
        if (parse_float(val,&f)) { g_lv[li].offx_v=f; g_lv[li].has_offx=1; }
    } else if (strcmp(key,"cam_offY")==0) {
        if (parse_float(val,&f)) { g_lv[li].offy_v=f; g_lv[li].has_offy=1; }
    } else if (strcmp(key,"cam_offZ")==0) {
        if (parse_float(val,&f)) { g_lv[li].offz_v=f; g_lv[li].has_offz=1; }
    } else if (strcmp(key,"thumbnail_tourney")==0) {
        parse_quoted(val, tmp, sizeof(tmp));
        if (!tmp[0]) { strncpy(tmp, skip_ws(val), sizeof(tmp)-1); tmp[sizeof(tmp)-1] = '\0'; }
        if (tmp[0]) { strncpy(g_lv[li].thumb_t, tmp, sizeof(g_lv[li].thumb_t)-1);
            g_lv[li].thumb_t[sizeof(g_lv[li].thumb_t)-1] = '\0'; g_lv[li].has_thumb_t=1; }
    } else if (strcmp(key,"thumbnail_practice")==0) {
        parse_quoted(val, tmp, sizeof(tmp));
        if (!tmp[0]) { strncpy(tmp, skip_ws(val), sizeof(tmp)-1); tmp[sizeof(tmp)-1] = '\0'; }
        if (tmp[0]) { strncpy(g_lv[li].thumb_p, tmp, sizeof(g_lv[li].thumb_p)-1);
            g_lv[li].thumb_p[sizeof(g_lv[li].thumb_p)-1] = '\0'; g_lv[li].has_thumb_p=1; }
    } else if (strcmp(key,"song")==0) {
        parse_quoted(val, tmp, sizeof(tmp));
        if (tmp[0]) { strncpy(g_lv[li].song, tmp, sizeof(g_lv[li].song)-1); g_lv[li].has_song=1; }
    } else if (strcmp(key,"song_hex")==0) {
        /* Jukebox HEX semantics: base-16 ("50" = 0x50 = 80).
         * Bare 50, "50" and '50' all accepted: strip quotes first. */
        {
            char t3[64];
            char *ee = NULL;
            const char *vv = skip_ws(val);
            unsigned long wv;
            size_t n3;
            strncpy(t3, vv, sizeof(t3)-1); t3[sizeof(t3)-1] = '\0';
            n3 = strlen(t3);
            while (n3 > 0 && (t3[n3-1]==' '||t3[n3-1]=='\t'||t3[n3-1]=='\r'||t3[n3-1]=='\n'||t3[n3-1]=='"'||t3[n3-1]=='\'')) t3[--n3] = '\0';
            vv = t3;
            while (*vv=='"'||*vv=='\'') vv++;
            wv = strtoul(vv, &ee, 16);
            if (ee && ee != vv) { g_lv[li].song_hex=(unsigned)wv; g_lv[li].has_song_hex=1; }
            else if (parse_uint(val,&u)) { g_lv[li].song_hex=u; g_lv[li].has_song_hex=1; }
        }
    } else if (strcmp(key,"name_color")==0) {
        float R, G, B, A;
        if (parse_rrggbbaa(val, &R, &G, &B, &A)) {
            g_lv[li].nc_r=R; g_lv[li].nc_g=G; g_lv[li].nc_b=B; g_lv[li].nc_a=A;
            g_lv[li].has_name_color=1;
        } else {
            static int logged_c = 0;
            if (!logged_c) { logged_c = 1; mod_log("[levels_plus] L%02d name_color PARSE FAIL val=%.40s", li+1, val); }
        }
    } else if (strcmp(key,"strip_color")==0) {
        float R, G, B, A;
        if (parse_rrggbbaa(val, &R, &G, &B, &A)) {
            g_lv[li].sc_r=R; g_lv[li].sc_g=G; g_lv[li].sc_b=B; g_lv[li].sc_a=A;
            g_lv[li].has_strip=1;
        } else {
            mod_log("[levels_plus] L%02d strip_color PARSE FAIL val=%.40s", li+1, val);
        }
    } else if (strcmp(key,"timeblot_color")==0) {
        float R, G, B, A;
        if (parse_rrggbbaa(val, &R, &G, &B, &A)) {
            g_lv[li].tb_r=R; g_lv[li].tb_g=G; g_lv[li].tb_b=B; g_lv[li].tb_a=A;
            g_lv[li].has_timeblot=1;
        } else {
            mod_log("[levels_plus] L%02d timeblot_color PARSE FAIL val=%.40s", li+1, val);
        }
    } else if (strcmp(key,"song_intro")==0) {
        const char *vv = skip_ws(val);
        if (*vv == '"' || *vv == '\'') {
            char qq = *vv;
            const char *q1 = vv+1, *q2 = strchr(q1, qq);
            char t4[256];
            size_t n4;
            if (!q2) q2 = q1 + strlen(q1);
            n4 = (size_t)(q2-q1);
            if (n4 >= sizeof(t4)) n4 = sizeof(t4)-1;
            memcpy(t4, q1, n4); t4[n4] = '\0';
            {
                char *ee = NULL;
                unsigned long wv = strtoul(t4, &ee, 16);
                if (ee && *ee == '\0') {
                    g_lv[li].intro_hex = (unsigned)wv;
                    g_lv[li].has_intro_hex = 1;
                } else if (t4[0]) {
                    strncpy(g_lv[li].intro_file, t4, sizeof(g_lv[li].intro_file)-1);
                    g_lv[li].intro_file[sizeof(g_lv[li].intro_file)-1] = '\0';
                    g_lv[li].has_intro_file = 1;
                }
            }
        } else {
            char t2[256];
            strncpy(t2, vv, sizeof(t2)-1); t2[sizeof(t2)-1] = '\0';
            {
                int nn = (int)strlen(t2);
                while (nn > 0 && (t2[nn-1]==' '||t2[nn-1]=='\t'||t2[nn-1]=='\r'||t2[nn-1]=='\n')) t2[--nn] = '\0';
            }
            if (t2[0]) {
                char *ee = NULL;
                unsigned long wv = strtoul(t2, &ee, 16);
                if (ee && *ee == '\0') {
                    g_lv[li].intro_hex = (unsigned)wv;
                    g_lv[li].has_intro_hex = 1;
                } else {
                    strncpy(g_lv[li].intro_file, t2, sizeof(g_lv[li].intro_file)-1);
                    g_lv[li].intro_file[sizeof(g_lv[li].intro_file)-1] = '\0';
                    g_lv[li].has_intro_file = 1;
                }
            }
        }
    } else if (strcmp(key,"name")==0) {
        parse_quoted(val, tmp, sizeof(tmp));
        if (tmp[0]) { strncpy(g_lv[li].name_v, tmp, sizeof(g_lv[li].name_v)-1); g_lv[li].has_name=1; }
    } else if (strcmp(key,"description")==0) {
        parse_quoted(val, tmp, sizeof(tmp));
        if (tmp[0]) { strncpy(g_lv[li].desc_v, tmp, sizeof(g_lv[li].desc_v)-1); g_lv[li].has_desc=1; }
    } else {
        /* name_color/lockname/strip/timeblot etc: v01a logged only */
    }
}

/* set file is jsonc array: "Level_01", { ... }, ... — prefix-match, keep-old-on-error */
static int read_set_file(void) {
    FILE *f = NULL;
    static char buf[32768];
    size_t n;
    const char *p;
    level_t bak[15];
    char bak_race[15][MAX_LEVEL_NAME+16];
    int ok = 0;
    memcpy(bak, g_lv, sizeof(bak));
    memcpy(bak_race, g_race_strings, sizeof(bak_race));
    if (!g_set_path[0]) return 0;
    if (fopen_s(&f, g_set_path, "r") != 0 || !f) return 0;
    n = fread(buf, 1, sizeof(buf)-1, f);
    fclose(f);
    if (n == 0) return 0;
    buf[n] = '\0';
    p = buf;
    if ((unsigned char)p[0]==0xEF && (unsigned char)p[1]==0xBB && (unsigned char)p[2]==0xBF) p += 3;
    while (*p) {
        int li = -1;
        const char *brace, *end;
        char lvl[16];
        if (strncmp(p, "Level_", 6)==0 && p[6]>='0' && p[6]<='9' && p[7]>='0' && p[7]<='9') {
            int v = (p[6]-'0')*10 + (p[7]-'0');
            if (v>=1 && v<=15) li = v-1;
            snprintf(lvl, sizeof(lvl), "Level_%02d", v);
        }
        if (li < 0) { p++; continue; }
        brace = strchr(p, '{');
        if (!brace) break;
        /* find matching } (no nesting in values) */
        end = strchr(brace, '}');
        if (!end) { memcpy(g_lv, bak, sizeof(bak)); memcpy(g_race_strings, bak_race, sizeof(bak_race)); return 0; }
        {
            const char *q = brace+1;
            while (q < end) {
                const char *kq1, *kq2, *colon, *comma, *nl;
                char key[64], val[256];
                size_t kn, vn;
                q = skip_ws(q);
                if (q >= end || *q=='}') break;
                if (*q=='/' && *(q+1)=='/') { while (q<end && *q!='\n') q++; continue; }
                if (*q!='"') { while (q<end && *q!='\n' && *q!=',') q++; if (q<end) q++; continue; }
                kq1 = q+1; kq2 = strchr(kq1, '"');
                if (!kq2 || kq2 >= end) break;
                kn = (size_t)(kq2-kq1);
                if (kn >= sizeof(key)) kn = sizeof(key)-1;
                memcpy(key, kq1, kn); key[kn]='\0';
                colon = strchr(kq2, ':');
                if (!colon || colon >= end) break;
                q = colon+1;
                q = skip_ws(q);
                /* value ends at comma or newline (quoted strings contain no comma that matters except colors? colors have no comma) */
                comma = strchr(q, ',');
                nl = strchr(q, '\n');
                {
                    const char *ve = end;
                    if (comma && comma < ve) ve = comma;
                    /* allow trailing // comment: cut at // outside quotes */
                    vn = (size_t)(ve-q);
                    if (vn >= sizeof(val)) vn = sizeof(val)-1;
                    memcpy(val, q, vn); val[vn]='\0';
                }
                apply_kv(li, key, val);
                if (comma && comma < end) q = comma+1;
                else if (nl && nl < end) q = nl+1;
                else break;
            }
        }
        ok = 1;
        p = end+1;
    }
    if (!ok) { memcpy(g_lv, bak, sizeof(bak)); memcpy(g_race_strings, bak_race, sizeof(bak_race)); return 0; }
    return 1;
}

static void patch_table_name(int idx, const char *s) {
    DWORD *slot = (DWORD*)(NAME_TABLE_BASE + idx*4);
    DWORD old;
    if (VirtualProtect(slot, 4, PAGE_READWRITE, &old)) {
        *slot = (DWORD)s;
        VirtualProtect(slot, 4, old, &old);
    }
    mod_log("[levels_plus] name table[%d] -> %s", idx+1, s);
}
static void patch_push(DWORD rva, const char *s) {
    HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
    DWORD base, addr;
    if (!hExe) hExe = GetModuleHandleA(NULL);
    if (!hExe) return;
    base = (DWORD)hExe;
    addr = base + rva;
    if (*(BYTE*)addr != 0x68) {
        mod_log("[levels_plus] WARN push RVA 0x%05x not 0x68 (got 0x%02x), skip", rva, *(BYTE*)addr);
        return;
    }
    {
        DWORD old;
        if (VirtualProtect((void*)(addr+1), 4, PAGE_READWRITE, &old)) {
            *(DWORD*)(addr+1) = (DWORD)s;
            VirtualProtect((void*)(addr+1), 4, old, &old);
        }
    }
}

static int file_exists(const char *p) {
    DWORD a = GetFileAttributesA(p);
    return (a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY));
}
static int build_music_path(const char *file, char *full) {
    snprintf(full, MAX_PATH, "%sMusic\\%s", g_exeDir, file);
    if (file_exists(full)) return 1;
    snprintf(full, MAX_PATH, "%s%s", g_dllDir, file);
    if (file_exists(full)) return 1;
    return 0;
}
static int is_mo3(const char *fn) {
    size_t n = strlen(fn);
    if (n < 4) return 0;
    return (_stricmp(fn+n-4, ".mo3")==0);
}

/* current race 1..15 or 0 (read App->profile) */
static int current_race(void) {
    DWORD app = 0, prof = 0;
    {
        app = *(DWORD*)0x5341E0;
        if (app < 0x10000 || IsBadReadPtr((void*)app, 0x240)) return 0;
        prof = *(DWORD*)(app+0x220);
        if (!prof || prof < 0x10000 || IsBadReadPtr((void*)prof, 0x10)) return 0;
        {
            int idx = *(int*)(prof+0x08);
            if (idx >= 1 && idx <= 15) return idx;
        }
    }
    return 0;
}
static DWORD current_board(void) {
    DWORD app = 0, prof = 0, board = 0;
    {
        app = *(DWORD*)0x5341E0;
        if (app < 0x10000 || IsBadReadPtr((void*)app, 0x240)) return 0;
        prof = *(DWORD*)(app+0x220);
        if (!prof || prof < 0x10000 || IsBadReadPtr((void*)prof, 0x10)) return 0;
        board = *(DWORD*)(prof+0x0C);
        if (!board || board < 0x10000 || IsBadReadPtr((void*)board, 0x3000)) return 0;
        return board;
    }
}

static int is_arena(void) {
    DWORD app = *(DWORD*)0x5341E0;
    if (app < 0x10000 || IsBadReadPtr((void*)app, 0x240)) return 0;
    if (!IsBadReadPtr((void*)(app+0x237), 1) && *(BYTE*)(app+0x237) != 0) return 1;
    return 0;
}
/* board liveness: live race has >=1 readable ball in the scene list */
static int board_has_ball(DWORD b) {
    DWORD cnt, items, ball;
    if (!b || IsBadReadPtr((void*)(b+0x29D4), 0x410)) return 0;
    cnt = *(DWORD*)(b+0x29D4+4);
    if (cnt == 0 || cnt > 64) return 0;
    items = *(DWORD*)(b+0x29D4+0x40C);
    if (!items || (DWORD)items < 0x10000 || IsBadReadPtr((void*)items, 4)) return 0;
    ball = *(DWORD*)items;
    if (!ball || (DWORD)ball < 0x10000 || IsBadReadPtr((void*)(ball+0x164), 12)) return 0;
    return 1;
}
static DWORD g_last_board = 0;
/* one-shot TIME seed: native copies board+0x2998 -> App+0x5E8 (+0xA0 stride)
 * once in AdvanceRace; per-tick board writes land after the copy. On a new
 * board (race 1-2, non-practice), seed both so the countdown starts exact.
 * No difficulty bonus: the set value rules as-is. */
static void seed_race_time(DWORD b, int r, level_t *L) {
    DWORD app;
    int v;
    if (r < 1 || r > 2 || !L->has_time) return;
    app = *(DWORD*)0x5341E0;
    if (app < 0x10000 || IsBadReadPtr((void*)app, 0x240)) return;
    {
        DWORD prof = *(DWORD*)(app+0x220);
        if (!prof || prof < 0x10000 || IsBadReadPtr((void*)prof, 0x12)) return;
        if (*(BYTE*)(prof+0x11) != 0) return; /* practice/TT counts up, don't touch */
    }
    v = (int)(L->time_v*100.0f);
    if (!IsBadReadPtr((void*)(b+0x2998), 4)) *(int*)(b+0x2998) = v;
    {
        int i;
        for (i = 0; i < 4; i++) {
            DWORD slot = app + 0x5E8 + i*0xA0;
            if (!IsBadReadPtr((void*)slot, 4)) *(int*)slot = v;
        }
    }
    mod_log("[levels_plus] L%02d time seeded %d cs", r, v);
}
/* per-tick board enforcement (absent key = no-op; arena skipped: separate names) */
static void enforce_board(void) {
    int r;
    DWORD b;
    if (is_arena()) { g_last_board = 0; return; }
    r = current_race();
    if (r < 1) { g_last_board = 0; return; }
    b = current_board();
    if (!b) { g_last_board = 0; return; }
    /* stability gate: skip first sighting (teardown/setup in flight) */
    if (b != g_last_board) {
        g_last_board = b;
        seed_race_time(b, r, &g_lv[r-1]);
        return;
    }
    /* liveness gate: no live ball = no live race = don't touch */
    if (!board_has_ball(b)) return;
    {
        level_t *L = &g_lv[r-1];
        /* pause gate */
        if (*(BYTE*)(b+0x874) != 0) return;
        if (L->has_time) *(int*)(b+0x2998) = (int)(L->time_v*100.0f);
        if (L->has_par) *(int*)(b+0x2990) = 9 + (int)(L->par_v*100.0f);
        if (L->has_weasel) *(int*)(b+0x2994) = 9 + (int)(L->weasel_v*100.0f);
        if (L->has_gold) *(int*)(b+0x29A0) = 9 + (int)(L->gold_v*100.0f);
        if (L->has_silver) *(int*)(b+0x29A4) = 9 + (int)(L->silver_v*100.0f);
        if (L->has_bronze) *(int*)(b+0x29A8) = 9 + (int)(L->bronze_v*100.0f);
        if (L->has_cam) *(float*)(b+0x299C) = L->cam_v;
        if (L->has_timeblot) {
            *(float*)(b+0x1508) = L->tb_r; *(float*)(b+0x150C) = L->tb_g;
            *(float*)(b+0x1510) = L->tb_b; *(float*)(b+0x1514) = L->tb_a;
            /* NOTE: +0x4340 left stock: HUD-global alpha (digits/Target).
             * Blot-only alpha comes from the slot patch reading g_blot_alpha. */
        }
        if (L->has_name && g_names[r-1][0]) {
            if (!IsBadReadPtr((void*)(b+0x29B4), 4))
                *(DWORD*)(b+0x29B4) = (DWORD)g_names[r-1];
        }
    }
}

/* ── music engine v01s: mo3 + song_hex + numeric intro, mp3/ogg/wav + file intro ──
 * mo3 numeric intro: start at intro_hex, BASS_SYNC_LOOP fires at first wrap,
 * music thread moves position to song_hex (dynamic APIs, fallback = song_hex).
 * non-mo3: BASS streams (ABI-detected @20/@28); string intro plays once via
 * BASS_SYNC_END, then the looped main song starts. Volume tracks
 * BASS_GetConfig(6) (music slider). Yield-old-custom on every new PlayEx. */
#define BASS_SYNC_LOOP 4
#define BASS_SYNC_END 2
#define BASS_POS_MUSIC_ORDER 1
#define BASS_SAMPLE_LOOP 4
typedef void (__stdcall *BASS_SYNCPROC_t)(DWORD, DWORD, DWORD, void*);
typedef DWORD (__stdcall *BASS_ChannelSetSync24_t)(DWORD, DWORD, unsigned long long, BASS_SYNCPROC_t, void*);
typedef DWORD (__stdcall *BASS_ChannelSetSync20_t)(DWORD, DWORD, DWORD, BASS_SYNCPROC_t, void*);
typedef int (__stdcall *BASS_ChannelSetPosition16_t)(DWORD, unsigned long long, DWORD);
typedef int (__stdcall *BASS_ChannelSetPosition8_t)(DWORD, DWORD);
typedef DWORD (__stdcall *BASS_StreamCreateFile5_t)(int, const void*, DWORD, DWORD, DWORD);
typedef DWORD (__stdcall *BASS_StreamCreateFile7_t)(int, const void*, unsigned long long, unsigned long long, DWORD);
typedef int (__stdcall *BASS_ChannelPlay_t)(DWORD, int);
typedef int (__stdcall *BASS_StreamPlay_t)(DWORD, int, DWORD);
typedef int (__stdcall *BASS_StreamFree_t)(DWORD);
typedef int (__stdcall *BASS_ChannelRemoveSync_t)(DWORD, DWORD);
typedef DWORD (__stdcall *BASS_GetConfig_t)(DWORD);
static BASS_ChannelSetSync24_t real_BASS_ChannelSetSync = NULL; /* keep name: QWORD form */
static BASS_ChannelSetSync20_t real_BASS_SetSync20 = NULL;
static BASS_ChannelSetPosition16_t real_BASS_ChannelSetPosition = NULL; /* keep name */
static BASS_ChannelSetPosition8_t real_BASS_SetPosition8 = NULL;
static BASS_StreamCreateFile5_t real_BASS_SCF5 = NULL;
static BASS_StreamCreateFile7_t real_BASS_SCF7 = NULL;
static BASS_ChannelPlay_t real_BASS_ChannelPlay = NULL;
static BASS_StreamPlay_t real_BASS_StreamPlay = NULL;
static int stream_play(DWORD h, int restart, int loop) {
    if (real_BASS_ChannelPlay) return real_BASS_ChannelPlay(h, restart);
    if (real_BASS_StreamPlay) return real_BASS_StreamPlay(h, restart, loop ? BASS_SAMPLE_LOOP : 0);
    return 0;
}
static BASS_StreamFree_t real_BASS_StreamFree = NULL;
static BASS_ChannelRemoveSync_t real_BASS_ChannelRemoveSync = NULL;
static BASS_GetConfig_t real_BASS_GetConfig = NULL;
static DWORD g_introStream = 0;
static DWORD g_mainStream = 0;
static DWORD g_musicSync = 0;
static DWORD g_musicChan = 0;
static volatile int g_need_switch = 0;
static int g_intro_mode = 0; /* 0 none, 1 mo3-pos, 2 stream-intro */
static unsigned g_intro_target = 0;
static DWORD g_loop_flags = 0;
/* race-song identity: only the race's own song request redirects; jingles
 * (Goal!, win, game-over...) arrive with race still set -> forward original
 * so Jukebox.xml keeps working for everything else. */
static int g_last_race = -1;
static int g_race_pos = -1;
static char g_pending_song[128];
static int g_lastVol = -1;
static void __stdcall sync_loop_proc(DWORD h, DWORD ch, DWORD d, void *u) {
    (void)h; (void)ch; (void)d; (void)u;
    if (g_intro_mode == 1) g_need_switch = 1;
}
static void __stdcall sync_end_proc(DWORD h, DWORD ch, DWORD d, void *u) {
    (void)h; (void)ch; (void)d; (void)u;
    if (g_intro_mode == 2) g_need_switch = 1;
}
static int music_vol(void) {
    if (real_BASS_GetConfig) {
        DWORD v = real_BASS_GetConfig(6);
        if (v <= 100) return (int)v;
    }
    return 100;
}
static void stream_attr_vol(DWORD h) {
    if (h && real_BASS_ChannelSetAttributes)
        real_BASS_ChannelSetAttributes(h, (DWORD)-1, music_vol(), -101);
}
static DWORD stream_load(const char *path, DWORD flags) {
    if (real_BASS_SCF7) return real_BASS_SCF7(0, path, 0, 0, flags);
    if (real_BASS_SCF5) return real_BASS_SCF5(0, path, 0, 0, flags);
    return 0;
}
static void stream_drop(DWORD *ph) {
    if (!ph || !*ph) return;
    if (real_BASS_ChannelStop) real_BASS_ChannelStop(*ph);
    if (real_BASS_StreamFree) real_BASS_StreamFree(*ph);
    *ph = 0;
}
static void yield_custom(void) {
    if (real_BASS_ChannelRemoveSync && g_musicSync && g_musicChan)
        real_BASS_ChannelRemoveSync(g_musicChan, g_musicSync);
    g_musicSync = 0; g_musicChan = 0;
    if (g_customMusic) {
        if (real_BASS_ChannelStop) real_BASS_ChannelStop(g_customMusic);
        if (real_BASS_MusicFree) real_BASS_MusicFree(g_customMusic);
        g_customMusic = 0;
    }
    stream_drop(&g_introStream);
    stream_drop(&g_mainStream);
    g_need_switch = 0; g_intro_mode = 0;
}
static DWORD music_setsync(DWORD h, DWORD type, BASS_SYNCPROC_t proc);
static int music_setpos(DWORD h, unsigned order);
static DWORD WINAPI music_thread(LPVOID p) {
    int tick = 0;
    (void)p;
    for (;;) {
        Sleep(50);
        tick++;
        if (g_need_switch && g_intro_mode == 1 && g_customMusic) {
            g_need_switch = 0;
            {
                int ok = 0;
                /* END-switch: replay same handle at loop order (no SetPosition needed) */
                if (real_BASS_MusicPlayEx) {
                    stream_attr_vol(g_customMusic);
                    ok = real_BASS_MusicPlayEx(g_customMusic, g_intro_target, 1, g_loop_flags);
                }
                mod_log("[levels_plus] INTRO end, loop order=%u rc=%d", g_intro_target, ok);
                if (ok && real_BASS_ChannelRemoveSync && g_musicSync) {
                    real_BASS_ChannelRemoveSync(g_customMusic, g_musicSync);
                    g_musicSync = 0;
                }
            }
            g_intro_mode = 0;
        }
        if (g_need_switch && g_intro_mode == 2) {
            g_need_switch = 0;
            g_intro_mode = 0;
            stream_drop(&g_introStream);
            if (g_pending_song[0]) {
                char full[MAX_PATH];
                if (build_music_path(g_pending_song, full)) {
                    DWORD hs = stream_load(full, BASS_SAMPLE_LOOP);
                    if (hs) {
                        stream_attr_vol(hs);
                        if (stream_play(hs, 1, 1)) {
                            g_mainStream = hs;
                            mod_log("[levels_plus] INTRO file done, loop song=%s", g_pending_song);
                        } else { stream_drop(&hs); }
                    } else {
                        mod_log("[levels_plus] FAIL loop song load %s", g_pending_song);
                    }
                }
            }
            g_pending_song[0] = '\0';
        }
        if ((tick % 40) == 0) {
            int v = music_vol();
            if (v != g_lastVol) {
                g_lastVol = v;
                if (g_customMusic && real_BASS_ChannelSetAttributes)
                    real_BASS_ChannelSetAttributes(g_customMusic, (DWORD)-1, v, -101);
                if (g_mainStream && real_BASS_ChannelSetAttributes)
                    real_BASS_ChannelSetAttributes(g_mainStream, (DWORD)-1, v, -101);
                if (g_introStream && real_BASS_ChannelSetAttributes)
                    real_BASS_ChannelSetAttributes(g_introStream, (DWORD)-1, v, -101);
            }
        }
    }
    return 0;
}
/* ── BASS_MusicPlayEx per-level redirect (mo3 + streams v01s) ── */
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD handle, DWORD pos, BOOL restart, DWORD flags) {
    int r;
    {
        int rr = current_race();
        int aa = is_arena();
        mod_log("[levels_plus] PlayEx entry pos=%u race=%d arena=%d song=%d",
            (unsigned)(pos & 0xFF), rr, aa,
            (rr >= 1 && g_lv[rr-1].has_song) ? 1 : 0);
    }
    yield_custom(); /* every new PlayMusic yields old custom first */
    if (is_arena()) goto orig_play;
    r = current_race();
    if (r != g_last_race) {
        g_last_race = r;
        g_race_pos = (r >= 1) ? (int)(pos & 0xFF) : -1;
    }
    if (r >= 1) {
        level_t *L = &g_lv[r-1];
        if (L->has_song && L->song[0]) {
            if ((int)(pos & 0xFF) != g_race_pos) {
                mod_log("[levels_plus] jingle fwd pos=%u race=%d", (unsigned)(pos & 0xFF), r);
                goto orig_play;
            }
            char full[MAX_PATH];
            if (!build_music_path(L->song, full)) {
                mod_log("[levels_plus] MISSING song file L%02d %s, fallback", r, L->song);
                goto orig_play;
            }
            if (is_mo3(L->song)) {
                DWORD nh = 0;
                unsigned startOrder = L->has_song_hex ? L->song_hex : 0;
                int useIntro = 0;
                if (L->has_intro_hex
                    && (real_BASS_ChannelSetSync || real_BASS_SetSync20)) {
                    startOrder = L->intro_hex;
                    useIntro = 1;
                }
                if (real_BASS_ChannelStop) real_BASS_ChannelStop(handle);
                if (real_BASS_MusicLoad && real_BASS_MusicPlayEx) {
                    int rc, err;
                    g_intro_mode = 0; g_need_switch = 0;
                    g_musicSync = 0; g_musicChan = 0;
                    nh = real_BASS_MusicLoad(0, full, 0, 0, 4, 0);
                    if (nh) {
                        stream_attr_vol(nh);
                        if (useIntro) {
                            g_musicSync = music_setsync(nh, BASS_SYNC_END, sync_end_proc);
                            if (g_musicSync) {
                                g_musicChan = nh; g_intro_mode = 1;
                                g_intro_target = L->has_song_hex ? L->song_hex : 0;
                            } else {
                                int serr = real_BASS_ErrorGetCode ? real_BASS_ErrorGetCode() : -1;
                                useIntro = 0;
                                startOrder = L->has_song_hex ? L->song_hex : 0;
                                mod_log("[levels_plus] no END sync err=%d, intro skipped", serr);
                            }
                        }
                        if (useIntro) {
                            /* intro plays ONCE (loop bit cleared); END fires -> loop song_hex */
                            g_loop_flags = flags | BASS_SAMPLE_LOOP;
                            rc = real_BASS_MusicPlayEx(nh, startOrder, 1, flags & ~BASS_SAMPLE_LOOP);
                        } else {
                            g_loop_flags = flags;
                            rc = real_BASS_MusicPlayEx(nh, startOrder, 1, flags);
                        }
                        err = real_BASS_ErrorGetCode ? real_BASS_ErrorGetCode() : -1;
                        if (rc) {
                            g_customMusic = nh;
                            g_lastVol = music_vol();
                            mod_log("[levels_plus] PLAY custom L%02d file=%s order=%u%s -> %08X rc=%d err=%d",
                                r, L->song, startOrder, useIntro ? " INTRO" : "", nh, rc, err);
                            return rc;
                        }
                        if (real_BASS_ChannelStop) real_BASS_ChannelStop(nh);
                        if (real_BASS_MusicFree) real_BASS_MusicFree(nh);
                        g_intro_mode = 0; g_musicSync = 0;
                        mod_log("[levels_plus] FAIL play L%02d err=%d, fallback", r, err);
                    } else {
                        err = real_BASS_ErrorGetCode ? real_BASS_ErrorGetCode() : -1;
                        mod_log("[levels_plus] FAIL load L%02d file=%s err=%d, fallback", r, L->song, err);
                    }
                }
            } else {
                /* mp3/ogg/wav via BASS streams */
                if ((!real_BASS_SCF5 && !real_BASS_SCF7) || (!real_BASS_ChannelPlay && !real_BASS_StreamPlay)) {
                    mod_log("[levels_plus] TODO non-mo3 L%02d %s (no stream API, fallback original)", r, L->song);
                    goto orig_play;
                }
                if (real_BASS_ChannelStop) real_BASS_ChannelStop(handle);
                if (L->has_intro_file && L->intro_file[0]) {
                    char ifull[MAX_PATH];
                    if (build_music_path(L->intro_file, ifull)) {
                        DWORD hs = stream_load(ifull, 0);
                        if (hs) {
                            DWORD sy = 0;
                            stream_attr_vol(hs);
                            {
                            sy = music_setsync(hs, BASS_SYNC_END, sync_end_proc);
                        }
                            if (stream_play(hs, 1, 0)) {
                                g_introStream = hs;
                                g_musicSync = sy; g_musicChan = hs;
                                g_intro_mode = 2; g_need_switch = 0;
                                strncpy(g_pending_song, L->song, sizeof(g_pending_song)-1);
                                g_pending_song[sizeof(g_pending_song)-1] = '\0';
                                g_lastVol = music_vol();
                                mod_log("[levels_plus] PLAY intro L%02d %s -> main %s", r, L->intro_file, L->song);
                                return 1;
                            }
                            stream_drop(&hs);
                        }
                    }
                    mod_log("[levels_plus] FAIL intro L%02d %s, trying main", r, L->intro_file);
                }
                {
                    DWORD hs = stream_load(full, BASS_SAMPLE_LOOP);
                    if (hs) {
                        stream_attr_vol(hs);
                        if (stream_play(hs, 1, 1)) {
                            g_mainStream = hs;
                            g_lastVol = music_vol();
                            mod_log("[levels_plus] PLAY stream L%02d %s", r, L->song);
                            return 1;
                        }
                        stream_drop(&hs);
                    }
                    mod_log("[levels_plus] FAIL stream L%02d %s, fallback", r, L->song);
                }
            }
        }
    }
orig_play:
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(handle, pos, restart, flags);
    return 1;
}
/* v01u: we load from Mods/ (proof: log path), NOT as game-root bass.dll,
 * so the game never calls our BASS exports. Resolve the REAL game BASS
 * (game-root bass.dll module) and hook its IAT slot for BASS_MusicPlayEx
 * (pure 4-byte data patch, same class as menu PUSHes). */
static HMODULE g_gameBass = NULL;
static int g_playex_hooked = 0;
static int in_own_module(DWORD addr) {
    HMODULE hSelf = NULL;
    BYTE *base;
    IMAGE_DOS_HEADER *dos;
    IMAGE_NT_HEADERS *nt;
    DWORD size;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
        | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR)&in_own_module, &hSelf);
    if (!hSelf) return 0;
    base = (BYTE*)hSelf;
    dos = (IMAGE_DOS_HEADER*)base;
    if (dos->e_magic != 0x5A4D) return 0;
    nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);
    if (nt->Signature != 0x4550) return 0;
    size = nt->OptionalHeader.SizeOfImage;
    return (addr >= (DWORD)base && addr < (DWORD)base + size);
}
/* per-function resolution: game bass -> bass_real (mapped) -> bass_real (load).
 * proxy forwarders export only a subset; the backend has the rest. */
static void *bass_fn(HMODULE hGame, const char *plain, const char *dec) {
    HMODULE hs[3];
    int n = 0, i;
    void *p;
    if (hGame) hs[n++] = hGame;
    {
        HMODULE hR = GetModuleHandleA("bass_real.dll");
        if (hR && (n == 0 || hR != hs[0])) hs[n++] = hR;
    }
    {
        HMODULE hL = LoadLibraryA("bass_real.dll");
        if (hL && (n == 0 || (hL != hs[0] && (n < 2 || hL != hs[1])))) hs[n++] = hL;
    }
    for (i = 0; i < n; i++) {
        p = GetProcAddress(hs[i], plain);
        if (p && !in_own_module((DWORD)p)) return p;
    }
    if (dec) for (i = 0; i < n; i++) {
        p = GetProcAddress(hs[i], dec);
        if (p && !in_own_module((DWORD)p)) return p;
    }
    return NULL;
}
static DWORD music_setsync(DWORD h, DWORD type, BASS_SYNCPROC_t proc) {
    if (real_BASS_ChannelSetSync) return real_BASS_ChannelSetSync(h, type, 0, proc, 0);
    if (real_BASS_SetSync20) return real_BASS_SetSync20(h, type, 0, proc, 0);
    return 0;
}
static int music_setpos(DWORD h, unsigned order) {
    if (real_BASS_ChannelSetPosition)
        return real_BASS_ChannelSetPosition(h, (unsigned long long)order, BASS_POS_MUSIC_ORDER);
    if (real_BASS_SetPosition8) return real_BASS_SetPosition8(h, order);
    return 0;
}
static void resolve_game_bass(void) {
    HMODULE h;
    if (g_gameBass && real_BASS_MusicPlayEx) return;
    h = GetModuleHandleA("bass.dll");
    if (h) {
        DWORD a = (DWORD)GetProcAddress(h, "BASS_MusicPlayEx");
        if (!a || in_own_module(a)) h = NULL; /* us, not the game BASS */
    }
    if (!h) h = LoadLibraryA("bass_real.dll");
    if (!h) { mod_log("[levels_plus] game BASS not found"); return; }
    g_gameBass = h;
    g_hRealBass = h;
#define RG(n) real_##n = (n##_t)GetProcAddress(h, #n)
    RG(BASS_ChannelSetAttributes);
    RG(BASS_MusicPlayEx);
    RG(BASS_MusicLoad);
    RG(BASS_MusicFree);
    RG(BASS_ChannelStop);
    RG(BASS_ErrorGetCode);
    RG(BASS_SetConfig);
    RG(BASS_Init);
    RG(BASS_Free);
    RG(BASS_Start);
    RG(BASS_Stop);
#undef RG
    if (!real_BASS_MusicLoad) real_BASS_MusicLoad = (BASS_MusicLoad_t)GetProcAddress(h, "_BASS_MusicLoad@24");
    if (!real_BASS_MusicPlayEx) real_BASS_MusicPlayEx = (BASS_MusicPlayEx_t)GetProcAddress(h, "_BASS_MusicPlayEx@16");
    if (!real_BASS_MusicFree) real_BASS_MusicFree = (BASS_MusicFree_t)GetProcAddress(h, "BASS_MusicFree");
    if (!real_BASS_ChannelStop) real_BASS_ChannelStop = (BASS_ChannelStop_t)GetProcAddress(h, "_BASS_ChannelStop@4");
    /* extended fns: proxy subset -> real backend (decorated-proven ABI) */
    {
        void *s24 = bass_fn(h, "x", "_BASS_ChannelSetSync@24");
        void *s20 = bass_fn(h, "x", "_BASS_ChannelSetSync@20");
        void *p16 = bass_fn(h, "x", "_BASS_ChannelSetPosition@16");
        void *p08 = bass_fn(h, "x", "_BASS_ChannelSetPosition@8");
        void *plain_sync = bass_fn(h, "BASS_ChannelSetSync", NULL);
        void *plain_pos = bass_fn(h, "BASS_ChannelSetPosition", NULL);
        void *oldfam = bass_fn(h, "x", "_BASS_ChannelSetAttributes@16");
        real_BASS_ChannelSetSync = NULL; real_BASS_SetSync20 = NULL;
        real_BASS_ChannelSetPosition = NULL; real_BASS_SetPosition8 = NULL;
        if (s24) real_BASS_ChannelSetSync = (BASS_ChannelSetSync24_t)s24;
        else if (s20) real_BASS_SetSync20 = (BASS_ChannelSetSync20_t)s20;
        else if (plain_sync && oldfam) real_BASS_SetSync20 = (BASS_ChannelSetSync20_t)plain_sync;
        else if (plain_sync) real_BASS_ChannelSetSync = (BASS_ChannelSetSync24_t)plain_sync;
        if (p16) real_BASS_ChannelSetPosition = (BASS_ChannelSetPosition16_t)p16;
        else if (p08) real_BASS_SetPosition8 = (BASS_ChannelSetPosition8_t)p08;
        else if (plain_pos && oldfam) real_BASS_SetPosition8 = (BASS_ChannelSetPosition8_t)plain_pos;
        else if (plain_pos) real_BASS_ChannelSetPosition = (BASS_ChannelSetPosition16_t)plain_pos;
    }
    real_BASS_ChannelPlay = (BASS_ChannelPlay_t)bass_fn(h, "BASS_ChannelPlay", "_BASS_ChannelPlay@8");
    real_BASS_StreamPlay = (BASS_StreamPlay_t)bass_fn(h, "BASS_StreamPlay", "_BASS_StreamPlay@12");
    real_BASS_StreamFree = (BASS_StreamFree_t)bass_fn(h, "BASS_StreamFree", "_BASS_StreamFree@4");
    real_BASS_ChannelRemoveSync = (BASS_ChannelRemoveSync_t)bass_fn(h, "BASS_ChannelRemoveSync", "_BASS_ChannelRemoveSync@8");
    real_BASS_GetConfig = (BASS_GetConfig_t)bass_fn(h, "BASS_GetConfig", "_BASS_GetConfig@4");
    real_BASS_SCF7 = NULL; real_BASS_SCF5 = NULL;
    {
        void *p28 = bass_fn(h, "x", "_BASS_StreamCreateFile@28");
        void *p20 = bass_fn(h, "x", "_BASS_StreamCreateFile@20");
        void *ppl = bass_fn(h, "BASS_StreamCreateFile", NULL);
        void *oldfam = bass_fn(h, "x", "_BASS_ChannelSetAttributes@16");
        if (p20) real_BASS_SCF5 = (BASS_StreamCreateFile5_t)p20;
        else if (p28) real_BASS_SCF7 = (BASS_StreamCreateFile7_t)p28;
        else if (ppl && oldfam) real_BASS_SCF5 = (BASS_StreamCreateFile5_t)ppl;
        else if (ppl) real_BASS_SCF7 = (BASS_StreamCreateFile7_t)ppl;
    }
    {
        char gb[MAX_PATH], rb[MAX_PATH];
        DWORD gv = 0, rv = 0;
        typedef DWORD (__stdcall *GV_t)(void);
        GV_t ggv, grv;
        gb[0] = rb[0] = '\0';
        if (g_gameBass) GetModuleFileNameA((HMODULE)g_gameBass, gb, sizeof(gb));
        {
            HMODULE hR = GetModuleHandleA("bass_real.dll");
            if (hR) GetModuleFileNameA(hR, rb, sizeof(rb));
        }
        ggv = (GV_t)bass_fn(h, "BASS_GetVersion", "_BASS_GetVersion@0");
        if (ggv) gv = ggv();
        mod_log("[levels_plus] BASS mods game=%s real=%s ver=%08X", gb, rb, gv);
        (void)rv; (void)grv;
    }
    mod_log("[levels_plus] game BASS resolved PlayEx=%d sync24=%d sync20=%d pos16=%d pos8=%d scf5=%d scf7=%d play=%d streamplay=%d cfg=%d",
        !!real_BASS_MusicPlayEx, !!real_BASS_ChannelSetSync, !!real_BASS_SetSync20,
        !!real_BASS_ChannelSetPosition, !!real_BASS_SetPosition8,
        !!real_BASS_SCF5, !!real_BASS_SCF7, !!real_BASS_ChannelPlay, !!real_BASS_StreamPlay, !!real_BASS_GetConfig);
}
static void install_playex_iat(void) {
    BYTE *base = (BYTE*)0x400000;
    IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER*)base;
    IMAGE_NT_HEADERS *nt;
    IMAGE_IMPORT_DESCRIPTOR *imp;
    DWORD realAddr;
    if (g_playex_hooked) return;
    if (!real_BASS_MusicPlayEx) return;
    if (dos->e_magic != 0x5A4D) return;
    nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);
    if (nt->Signature != 0x4550) return;
    imp = (IMAGE_IMPORT_DESCRIPTOR*)(base + nt->OptionalHeader.DataDirectory[1].VirtualAddress);
    if (!imp || IsBadReadPtr(imp, sizeof(*imp))) return;
    realAddr = (DWORD)real_BASS_MusicPlayEx;
    for (; imp->Name; imp++) {
        const char *dll;
        DWORD *iat;
        if (IsBadReadPtr(imp, sizeof(*imp))) break;
        dll = (const char*)(base + imp->Name);
        if (!dll || IsBadReadPtr((void*)dll, 8)) continue;
        if (_stricmp(dll, "bass.dll") != 0) continue;
        iat = (DWORD*)(base + imp->FirstThunk);
        for (; *iat; iat++) {
            DWORD old;
            if (*iat != realAddr) continue;
            if (VirtualProtect(iat, 4, PAGE_READWRITE, &old)) {
                *iat = (DWORD)BASS_MusicPlayEx;
                VirtualProtect(iat, 4, old, &old);
                g_playex_hooked = 1;
                mod_log("[levels_plus] PlayEx IAT hooked");
                return;
            }
        }
    }
}
static void init_bass(void) {
    resolve_game_bass();
    install_playex_iat();
}
/* tourney description pointer table (15 char*, from mknp_race_descriptions) */
#define DESC_TABLE_BASE 0x4F7148
static void patch_table_desc(int idx, const char *str) {
    DWORD *slot = (DWORD*)(DESC_TABLE_BASE + idx*4);
    DWORD old;
    if (VirtualProtect(slot, 4, PAGE_READWRITE, &old)) {
        *slot = (DWORD)str;
        VirtualProtect(slot, 4, old, &old);
    }
    mod_log("[levels_plus] desc table[%d] applied", idx+1);
}
/* name-strip color cave (from mknp_namestrip_colors): detours the 14-byte
 * push-sequence at 0x41B763 in Scene_DrawNameStrip into patchable operands.
 * Installed one-shot while site idle (worker, pre-race); per-level values
 * written as data afterwards. Absent key = stock yellow. */
#define STRIP_PATCH_ADDR 0x41B763
#define STRIP_PATCH_SIZE 14
#define STRIP_RETURN_ADDR 0x41B771
#define STRIP_MATRIX_CALL 0x453150
static const BYTE kStripOrig[14] = {
    0x68,0x00,0x00,0x40,0x3F, 0x6A,0x00, 0x55,0x55, 0xE8,0xDF,0x79,0x03,0x00
};
static BYTE *g_strip_cave = NULL;
static DWORD g_strip_off[4]; /* operand addrs: [0]=R [1]=G [2]=B [3]=A */
static int g_strip_ok = 0;
static void strip_write_float(int idx, float v) {
    DWORD old;
    if (!g_strip_ok || !g_strip_cave) return;
    if (VirtualProtect((void*)(g_strip_cave + g_strip_off[idx]), 4, PAGE_EXECUTE_READWRITE, &old)) {
        *(float*)(g_strip_cave + g_strip_off[idx]) = v;
        VirtualProtect((void*)(g_strip_cave + g_strip_off[idx]), 4, old, &old);
    }
}
static void install_strip_cave(void) {
    /* v01aa diag reverted */
    BYTE *w;
    DWORD old, ca, ja;
    int i;
    float inits[4] = { 0.75f, 0.0f, 1.0f, 1.0f }; /* push order A,B,G,R */
    int map[4] = { 3, 2, 1, 0 };                  /* push i holds comp map[i] */
    BYTE *site = (BYTE*)STRIP_PATCH_ADDR;
    if (g_strip_ok) return;
    for (i = 0; i < STRIP_PATCH_SIZE; i++) if (site[i] != kStripOrig[i]) {
        mod_log("[levels_plus] strip cave site mismatch, skip");
        return;
    }
    w = (BYTE*)VirtualAlloc(NULL, 64, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!w) return;
    g_strip_cave = w;
    for (i = 0; i < 4; i++) {
        g_strip_off[map[i]] = (DWORD)(w + 1) - (DWORD)g_strip_cave;
        *w++ = 0x68; *(float*)w = inits[i]; w += 4;
    }
    ca = (DWORD)w;
    *w++ = 0xE8; *(DWORD*)w = STRIP_MATRIX_CALL - (ca + 5); w += 4;
    ja = (DWORD)w;
    *w++ = 0xE9; *(DWORD*)w = STRIP_RETURN_ADDR - (ja + 5); w += 4;
    FlushInstructionCache(GetCurrentProcess(), g_strip_cave, 32);
    if (VirtualProtect(site, STRIP_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &old)) {
        site[0] = 0xE9;
        *(DWORD*)(site+1) = (DWORD)g_strip_cave - ((DWORD)site + 5);
        memset(site+5, 0x90, 9);
        FlushInstructionCache(GetCurrentProcess(), site, STRIP_PATCH_SIZE);
        VirtualProtect(site, STRIP_PATCH_SIZE, old, &old);
        g_strip_ok = 1;
        mod_log("[levels_plus] strip cave installed");
    }
}
static void update_strip_cave(void) {
    float r = 1.0f, g = 1.0f, b = 0.0f, a = 0.75f;
    int rr = current_race();
    if (rr >= 1 && !is_arena() && g_lv[rr-1].has_strip) {
        r = g_lv[rr-1].sc_r; g = g_lv[rr-1].sc_g;
        b = g_lv[rr-1].sc_b; a = g_lv[rr-1].sc_a;
    }
    strip_write_float(0, r); strip_write_float(1, g);
    strip_write_float(2, b); strip_write_float(3, a);
}
/* blot-scoped alpha (timeblot_color A): the 3 blot draws build their color
 * struct from board RGB + board+0x4340 alpha, but +0x4340 is HUD-global
 * (timer digits, Target text...). Patch ONLY the 3 blot alpha reads
 * (mov EDX,[ESI+0x4340] = 8B 96 40 43 00 00) into absolute loads from our
 * own float. Pure data patch, no cave, no hook. */
static const DWORD kBlotAlphaSites[3] = { 0x41BC86, 0x41C0AA, 0x41C2B1 };
static const BYTE kBlotAlphaOrig[6] = { 0x8B, 0x96, 0x40, 0x43, 0x00, 0x00 };
static int g_blot_patched = 0;
static void install_blot_alpha(void) {
    /* v01aa diag reverted */
    int i;
    DWORD old;
    if (g_blot_patched) return;
    for (i = 0; i < 3; i++) {
        BYTE *site = (BYTE*)kBlotAlphaSites[i];
        int k, ok = 1;
        for (k = 0; k < 6; k++) if (site[k] != kBlotAlphaOrig[k]) ok = 0;
        if (!ok) { mod_log("[levels_plus] blot alpha site %d mismatch, skip", i); continue; }
        if (VirtualProtect(site, 6, PAGE_EXECUTE_READWRITE, &old)) {
            site[0] = 0x8B; site[1] = 0x15;
            *(DWORD*)(site+2) = (DWORD)&g_blot_alpha;
            FlushInstructionCache(GetCurrentProcess(), site, 6);
            VirtualProtect(site, 6, old, &old);
        }
    }
    g_blot_patched = 1;
    mod_log("[levels_plus] blot alpha slots installed");
}
static void update_blot_alpha(void) {
    float a = 1.0f;
    int rr = current_race();
    if (rr >= 1 && !is_arena() && g_lv[rr-1].has_timeblot) a = g_lv[rr-1].tb_a;
    g_blot_alpha = a;
}
/* ── per-level camera v01ad ──
 * rotY: scene(board)+0x29BC orbit angle is write-once per race (2 init sites,
 * 0 runtime writers) -> write base+rotY per race. rotX: pitch imm32 at
 * 0x41A2B5 (stock 0x3F666666=0.9) -> data patch 0.9+rotX. offX/Y/Z: 9-byte
 * cave over the target loads at 0x41A201, FPU adds, 0.0 = no-op. rotZ: the
 * orbit path has no roll input -> parsed, logged, ignored. */
#define CAM_PITCH_ADDR 0x41A2B5
#define CAM_PITCH_STOCK 0x3F666666u
#define CAM_TGT_ADDR 0x41A201
#define CAM_TGT_NEXT 0x41A20A
static const BYTE kCamTgtOrig[9] = { 0x8B,0x4F,0x60, 0x8B,0x47,0x68, 0x8B,0x57,0x64 };
static float g_cam_off[3] = { 0.0f, 0.0f, 0.0f };
static int g_cam_cave_ok = 0;
static void install_cam_cave(void) {
    BYTE *site = (BYTE*)CAM_TGT_ADDR;
    BYTE *w;
    DWORD old;
    int i;
    if (g_cam_cave_ok) return;
    for (i = 0; i < 9; i++) if (site[i] != kCamTgtOrig[i]) {
        mod_log("[levels_plus] cam cave site mismatch, skip");
        return;
    }
    w = (BYTE*)VirtualAlloc(NULL, 96, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!w) return;
    {
        BYTE *c = w;
        /* replicate 3 loads */
        *c++ = 0x8B; *c++ = 0x4F; *c++ = 0x60;
        *c++ = 0x8B; *c++ = 0x47; *c++ = 0x68;
        *c++ = 0x8B; *c++ = 0x57; *c++ = 0x64;
        /* store to esp slots (x->+0xC, z->+0x14, y->+0x10) */
        *c++ = 0x89; *c++ = 0x4C; *c++ = 0x24; *c++ = 0x0C;
        *c++ = 0x89; *c++ = 0x44; *c++ = 0x24; *c++ = 0x14;
        *c++ = 0x89; *c++ = 0x54; *c++ = 0x24; *c++ = 0x10;
        /* fadd per-axis offsets in place */
        *c++ = 0xD9; *c++ = 0x44; *c++ = 0x24; *c++ = 0x0C;
        *c++ = 0xD8; *c++ = 0x05; *(DWORD*)c = (DWORD)&g_cam_off[0]; c += 4;
        *c++ = 0xD9; *c++ = 0x5C; *c++ = 0x24; *c++ = 0x0C;
        *c++ = 0xD9; *c++ = 0x44; *c++ = 0x24; *c++ = 0x10;
        *c++ = 0xD8; *c++ = 0x05; *(DWORD*)c = (DWORD)&g_cam_off[1]; c += 4;
        *c++ = 0xD9; *c++ = 0x5C; *c++ = 0x24; *c++ = 0x10;
        *c++ = 0xD9; *c++ = 0x44; *c++ = 0x24; *c++ = 0x14;
        *c++ = 0xD8; *c++ = 0x05; *(DWORD*)c = (DWORD)&g_cam_off[2]; c += 4;
        *c++ = 0xD9; *c++ = 0x5C; *c++ = 0x24; *c++ = 0x14;
        /* carry offsets in regs: downstream re-stores ecx/eax/edx to esp */
        *c++ = 0x8B; *c++ = 0x4C; *c++ = 0x24; *c++ = 0x0C;
        *c++ = 0x8B; *c++ = 0x44; *c++ = 0x24; *c++ = 0x14;
        *c++ = 0x8B; *c++ = 0x54; *c++ = 0x24; *c++ = 0x10;
        /* jmp back */
        {
            DWORD ja = (DWORD)c;
            *c++ = 0xE9; *(DWORD*)c = CAM_TGT_NEXT - (ja + 5); c += 4;
        }
        FlushInstructionCache(GetCurrentProcess(), w, (SIZE_T)(c - w));
    }
    if (VirtualProtect(site, 9, PAGE_EXECUTE_READWRITE, &old)) {
        site[0] = 0xE9;
        *(DWORD*)(site+1) = (DWORD)w - ((DWORD)site + 5);
        memset(site+5, 0x90, 4);
        FlushInstructionCache(GetCurrentProcess(), site, 9);
        VirtualProtect(site, 9, old, &old);
        g_cam_cave_ok = 1;
        mod_log("[levels_plus] cam cave installed");
    }
}
static DWORD g_cam_board = 0;
static float g_cam_base = 0.0f;
static int g_cam_pitch_set = 0;
static void patch_pitch(float v) {
    DWORD old;
    BYTE *p = (BYTE*)CAM_PITCH_ADDR;
    DWORD bits;
    memcpy(&bits, &v, 4);
    if (VirtualProtect(p, 4, PAGE_EXECUTE_READWRITE, &old)) {
        *(DWORD*)p = bits;
        FlushInstructionCache(GetCurrentProcess(), p, 4);
        VirtualProtect(p, 4, old, &old);
    }
}
static void update_cam(void) {
    int r;
    DWORD b;
    if (is_arena()) return;
    r = current_race();
    if (r < 1) return;
    b = current_board();
    if (!b) return;
    if (b != g_cam_board) {
        /* new board: capture stock base, apply or restore */
        g_cam_board = b;
        if (!IsBadReadPtr((void*)(b+0x29BC), 4)) g_cam_base = *(float*)(b+0x29BC);
        if (g_lv[r-1].has_roty) {
            if (!IsBadReadPtr((void*)(b+0x29BC), 4))
                *(float*)(b+0x29BC) = g_cam_base + g_lv[r-1].roty_v;
            mod_log("[levels_plus] L%02d cam rotY base=%.2f off=%.2f", r, g_cam_base, g_lv[r-1].roty_v);
        }
        if (g_lv[r-1].has_rotx) {
            patch_pitch(0.9f + g_lv[r-1].rotx_v);
            g_cam_pitch_set = 1;
            mod_log("[levels_plus] L%02d cam rotX pitch=%.3f", r, 0.9f + g_lv[r-1].rotx_v);
        } else if (g_cam_pitch_set) {
            patch_pitch(0.9f);
            g_cam_pitch_set = 0;
        }
        if (g_lv[r-1].has_offx || g_lv[r-1].has_offy || g_lv[r-1].has_offz)
            mod_log("[levels_plus] L%02d cam off %.1f %.1f %.1f", r,
                g_lv[r-1].has_offx ? g_lv[r-1].offx_v : 0.0f,
                g_lv[r-1].has_offy ? g_lv[r-1].offy_v : 0.0f,
                g_lv[r-1].has_offz ? g_lv[r-1].offz_v : 0.0f);
    }
    /* per-tick target offsets: scene+0x434C/50/54 added to camera target
     * every frame in SetCamera; only init writes them, never in-race. */
    if (g_lv[r-1].has_offx && !IsBadReadPtr((void*)(b+0x434C), 4))
        *(float*)(b+0x434C) = g_lv[r-1].offx_v;
    if (g_lv[r-1].has_offy && !IsBadReadPtr((void*)(b+0x4350), 4))
        *(float*)(b+0x4350) = g_lv[r-1].offy_v;
    if (g_lv[r-1].has_offz && !IsBadReadPtr((void*)(b+0x4354), 4))
        *(float*)(b+0x4354) = g_lv[r-1].offz_v;
}
/* tourney/practice thumbnails: 15 PUSH sites each, builders run race order.
 * Stock shape mirrored: tourney bare filename, practice Textures\\-prefixed. */
static const DWORD g_tourney_push_rvas[15] = {
    0x2A35A,0x2A371,0x2A388,0x2A39F,0x2A3B6,0x2A3CD,0x2A3E4,
    0x2A3FB,0x2A412,0x2A429,0x2A440,0x2A457,0x2A46E,0x2A485,0x2A49C
};
static const DWORD g_practice_push_rvas[15] = {
    0x2EAC1,0x2EB03,0x2EB45,0x2EB87,0x2EBC9,0x2EC0B,0x2EC4D,
    0x2EC8F,0x2ECD1,0x2ED13,0x2ED55,0x2ED97,0x2EDD9,0x2EE1B,0x2EE5D
};
static char g_thumbT_str[15][160];
static char g_thumbP_str[15][160];
static int thumb_exists(const char *file) {
    char full[MAX_PATH];
    char name[160];
    DWORD a;
    if (strchr(file, '.')) snprintf(name, sizeof(name), "%s", file);
    else snprintf(name, sizeof(name), "%s.png", file);
    snprintf(full, sizeof(full), "%sTextures\\%s", g_exeDir, name);
    a = GetFileAttributesA(full);
    if (a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY)) return 1;
    snprintf(full, sizeof(full), "%s%s", g_dllDir, name);
    a = GetFileAttributesA(full);
    if (a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY)) return 1;
    return 0;
}
static void install_thumbnails(void) {
    int i;
    for (i = 0; i < 15; i++) {
        if (g_lv[i].has_thumb_t && g_lv[i].thumb_t[0]) {
            if (!thumb_exists(g_lv[i].thumb_t)) {
                mod_log("[levels_plus] L%02d thumbnail_tourney MISSING %s, skipped", i+1, g_lv[i].thumb_t);
            } else {
                if (strchr(g_lv[i].thumb_t, '.'))
                    snprintf(g_thumbT_str[i], sizeof(g_thumbT_str[i]), "%s", g_lv[i].thumb_t);
                else
                    snprintf(g_thumbT_str[i], sizeof(g_thumbT_str[i]), "%s.png", g_lv[i].thumb_t);
                patch_push(g_tourney_push_rvas[i], g_thumbT_str[i]);
            }
        }
        if (g_lv[i].has_thumb_p && g_lv[i].thumb_p[0]) {
            if (!thumb_exists(g_lv[i].thumb_p)) {
                mod_log("[levels_plus] L%02d thumbnail_practice MISSING %s, skipped", i+1, g_lv[i].thumb_p);
            } else {
                if (strchr(g_lv[i].thumb_p, '.'))
                    snprintf(g_thumbP_str[i], sizeof(g_thumbP_str[i]), "Textures\\%s", g_lv[i].thumb_p);
                else
                    snprintf(g_thumbP_str[i], sizeof(g_thumbP_str[i]), "Textures\\%s.png", g_lv[i].thumb_p);
                patch_push(g_practice_push_rvas[i], g_thumbP_str[i]);
            }
        }
    }
    mod_log("[levels_plus] thumbnails installed (has_thumb only)");
}
static DWORD WINAPI worker(LPVOID p) {
    int i;
    (void)p;
    for (i = 0; i < 15; i++) {
        patch_push(g_race_push_rvas[i], g_race_strings[i]);
        patch_push(g_arena_push_rvas[i], g_arena_strings[i]);
    }
    mod_log("[levels_plus] mesh PUSHes installed");
    install_thumbnails();
    for (i = 0; i < 15; i++) {
        DWORD *slot = (DWORD*)(NAME_TABLE_BASE + i*4);
        g_names[i][0] = '\0';
        if (!IsBadReadPtr(slot, 4) && *slot > 0x10000 && !IsBadReadPtr((void*)*slot, 8)) {
            strncpy(g_names[i], (const char*)*slot, sizeof(g_names[i])-1);
        }
        if (g_lv[i].has_name && g_lv[i].name_v[0]) {
            strncpy(g_names[i], g_lv[i].name_v, sizeof(g_names[i])-1);
        }
        g_names[i][sizeof(g_names[i])-1] = '\0';
        mod_log("[levels_plus] L%02d flags name=%d color=%d strip=%d blot=%d desc=%d intro=%d th=%d mesh=%d time=%d cam=%d song=%d",
            i+1, g_lv[i].has_name, g_lv[i].has_name_color, g_lv[i].has_strip,
            g_lv[i].has_timeblot, g_lv[i].has_desc,
            (g_lv[i].has_intro_hex || g_lv[i].has_intro_file) ? 1 : 0,
            (g_lv[i].has_thumb_t || g_lv[i].has_thumb_p) ? 1 : 0,
            g_lv[i].has_mesh, g_lv[i].has_time, g_lv[i].has_cam, g_lv[i].has_song);
    }
    for (i = 0; i < 15; i++) {
        if (g_lv[i].has_name && g_names[i][0]) {
            patch_table_name(i, g_names[i]);
            patch_push(g_menu_name_rvas[i], g_names[i]);
        }
    }
    mod_log("[levels_plus] names installed (has_name only)");
    for (i = 0; i < 15; i++) {
        if (g_lv[i].has_name_color && g_menu_col_rvas[i][0]) {
            /* operand order in EntryDef: A,B,G,R */
            patch_float_rva(g_menu_col_rvas[i][0], g_lv[i].nc_a);
            patch_float_rva(g_menu_col_rvas[i][1], g_lv[i].nc_b);
            patch_float_rva(g_menu_col_rvas[i][2], g_lv[i].nc_g);
            patch_float_rva(g_menu_col_rvas[i][3], g_lv[i].nc_r);
            mod_log("[levels_plus] L%02d name_color applied", i+1);
        } else if (g_lv[i].has_name_color) {
            mod_log("[levels_plus] L%02d name_color needs cave (TODO)", i+1);
        }
    }
    for (i = 0; i < 15; i++) {
        if (g_lv[i].has_desc && g_lv[i].desc_v[0]) {
            patch_table_desc(i, g_lv[i].desc_v);
        }
    }
    mod_log("[levels_plus] descs installed (has_desc only)");
    install_strip_cave();
    install_blot_alpha();
    Sleep(2500);
    resolve_game_bass();
    install_playex_iat();
    for (;;) {
        read_set_file();
        enforce_board();
        update_cam();
        update_strip_cave();
        update_blot_alpha();
        if (!g_playex_hooked) { resolve_game_bass(); install_playex_iat(); }
        Sleep(500);
    }
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE h, DWORD reason, LPVOID r) {
    (void)r;
    if (reason == DLL_PROCESS_ATTACH) {
        int i;
        g_hSelf = (HMODULE)h;
        DisableThreadLibraryCalls(h);
        init_paths();
        init_bass();
        for (i = 0; i < 15; i++) {
            strncpy_s(g_race_strings[i], sizeof(g_race_strings[i]), kDefRace[i], _TRUNCATE);
            strncpy_s(g_arena_strings[i], sizeof(g_arena_strings[i]), kDefArena[i], _TRUNCATE);
        }
        memset(g_lv, 0, sizeof(g_lv));
        mod_log("[levels_plus] INIT log=%s set=%s %s", g_log_path, g_set_path, MOD_VER);
        read_set_file();
        {
            int c = 0;
            for (i = 0; i < 15; i++) {
                if (g_lv[i].has_mesh || g_lv[i].has_time || g_lv[i].has_song) c++;
                if (g_lv[i].has_name) mod_log("[levels_plus] L%02d name=%s (TODO apply)", i+1, g_lv[i].name_v);
                if (g_lv[i].has_desc) mod_log("[levels_plus] L%02d desc set", i+1);
            }
            mod_log("[levels_plus] set parsed, %d levels with overrides", c);
        }
        CreateThread(NULL, 0, worker, NULL, 0, NULL);
        CreateThread(NULL, 0, music_thread, NULL, 0, NULL);
    }
    return TRUE;
}
