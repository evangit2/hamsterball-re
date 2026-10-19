/*
 * mknp_player_colors.cpp — Per-player ball colors (HB+ v2.1, MinGW)
 *
 * HB+ port of the ball_tint bass.dll proxy (v5). No BASS exports, no
 * threads: the HB+ loader scans Mods\, calls CreateModInstance, fires
 * Initialize(), then ticks onGameUpdate() every frame.
 *
 * WHAT THIS MOD DOES
 * ------------------
 * Tints player balls to hex colors read from mknp_player_colors.set
 * (jsonc, lives NEXT TO THIS DLL, e.g. Mods\). Supports all 4 players
 * across 1P/2P/4P modes. Config re-read ~every 2s for live editing.
 *
 * Set file (mknp_player_colors.set), 7 keys:
 *   p1_single: Player 1 color in 1-player mode
 *   p1_2p:     Player 1 color in 2-player mode
 *   p1_4p:     Player 1 color in 4-player mode (Arena/Rumble)
 *   p2_2p:     Player 2 color in 2-player mode
 *   p2_4p:     Player 2 color in 4-player mode (Arena/Rumble)
 *   p3_4p:     Player 3 color in 4-player mode (Arena/Rumble)
 *   p4_4p:     Player 4 color in 4-player mode (Arena/Rumble)
 *
 * Board color table (set by Board_ctor's Vec3_Init calls):
 *   board+0x3AB0 = P1, +0x3AC4 = P2, +0x3AD8 = P3, +0x3AEC = P4
 *   (4 RGBA floats each, 0x14 stride). Scene == Board via GetScene().
 * Player count from Scene.player_count (+0x3630, API struct field).
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
#define memcpy  nc_memcpy

/* ═══════════════════════════════════════════════════════════════════════════
 * State
 * ═══════════════════════════════════════════════════════════════════════════ */

#define BOARD_COLOR_BASE    0x3AB0
#define BOARD_COLOR_STRIDE  0x14

#define NUM_COLORS 7 /* legacy count, see g_ball/g_ui */
static unsigned int g_colors[NUM_COLORS] = {
    0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
    0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF
}; /* legacy, unused */

static const char* K_MODES[3] = { "1P_mode", "2P_mode", "4P_mode" };
static unsigned int g_ball[3] = { 0xFFFFFF, 0xFFFFFF, 0xFFFFFF };
static unsigned int g_ui[3] = { 0xFFFFFF, 0xFFFFFF, 0xFFFFFF };

static IModAPI* g_api = NULL;
static HMODULE  g_module = NULL;

static char g_dll_dir[MAX_PATH];
static char g_config_path[MAX_PATH];
static char g_log_path[MAX_PATH];

static int g_started = 0;
static int g_deep_done = 0;
static unsigned int g_tick = 0;

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

static void path_join(char* dst, const char* dir, const char* tail) {
    nc_strncpy(dst, dir, MAX_PATH - 1);
    dst[MAX_PATH - 1] = '\0';
    size_t dl = nc_strlen(dst);
    size_t i = 0;
    while (tail[i] && dl + 1 < MAX_PATH) dst[dl++] = tail[i++];
    dst[dl] = '\0';
}

/* Parse hex color: optional # / 0x, 6 digits (or 3-digit shorthand). */
static unsigned int parse_hex(const char* text) {
    const char* p = text;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (*p == '#') p++;
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) p += 2;
    unsigned int hex = 0;
    int digits = 0;
    while (digits < 6 && *p) {
        char c = *p;
        unsigned int v;
        if (c >= '0' && c <= '9') v = (unsigned int)(c - '0');
        else if (c >= 'a' && c <= 'f') v = (unsigned int)(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') v = (unsigned int)(c - 'A' + 10);
        else break;
        hex = (hex << 4) | v;
        digits++;
        p++;
    }
    if (digits >= 6) return hex;
    if (digits >= 3) {
        unsigned int r = ((hex >> 8) & 0xF) * 0x11;
        unsigned int g = ((hex >> 4) & 0xF) * 0x11;
        unsigned int b = (hex & 0xF) * 0x11;
        return (r << 16) | (g << 8) | b;
    }
    return 0xFFFFFF;
}

static void create_defaults(void) {
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) { CloseHandle(h); return; }
    const char* defs =
        "// mknp_player_colors settings (jsonc — // and /* */ comments allowed)\r\n"
        "[\r\n"
        "    \"Player1\",    { \"1P_mode\": \"FFFFFF\", \"2P_mode\": \"FFFFFF\", \"4P_mode\": \"FFFFFF\" },\r\n"
        "    \"Player1_UI\", { \"1P_mode\": \"FFFFFF\", \"2P_mode\": \"FFFFFF\", \"4P_mode\": \"FFFFFF\" }\r\n"
        "]\r\n";;
    h = CreateFileA(g_config_path, GENERIC_WRITE, 0, NULL,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD wrote = 0;
        WriteFile(h, defs, (DWORD)nc_strlen(defs), &wrote, NULL);
        CloseHandle(h);
    }
}

/* Strip jsonc comments (// to EOL, slash-star blocks) into dst. Strings respected. */
static int strip_jsonc(const char* src, int n, char* dst, int cap) {
    int i = 0, o = 0;
    int in_str = 0;
    while (i < n && o + 1 < cap) {
        char c = src[i];
        if (in_str) {
            dst[o++] = c;
            if (c == '\\' && i + 1 < n) { dst[o++] = src[++i]; i++; continue; }
            if (c == '"') in_str = 0;
            i++;
            continue;
        }
        if (c == '"') { in_str = 1; dst[o++] = c; i++; continue; }
        if (c == '/' && i + 1 < n && src[i + 1] == '/') {
            i += 2;
            while (i < n && src[i] != '\n') i++;
            continue;
        }
        if (c == '/' && i + 1 < n && src[i + 1] == '*') {
            i += 2;
            while (i + 1 < n && !(src[i] == '*' && src[i + 1] == '/')) i++;
            i += 2;
            continue;
        }
        dst[o++] = c;
        i++;
    }
    dst[o] = '\0';
    return o;
}

/* Find "Group" { ... } — sets *s/*e to the brace span. Returns 1 on hit. */
static int find_group(const char* buf, const char* name, const char** s, const char** e) {
    int nl = 0;
    while (name[nl]) nl++;
    const char* p = buf;
    while (*p) {
        if (*p != '"') { p++; continue; }
        int i = 0;
        while (i < nl && p[1 + i] == name[i]) i++;
        if (i != nl || p[1 + i] != '"') { p++; continue; }
        p += 1 + i + 1;
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' || *p == ',') p++;
        if (*p == ':') {
            p++;
            while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        }
        if (*p != '{') continue;
        *s = p;
        int depth = 0;
        while (*p) {
            if (*p == '{') depth++;
            else if (*p == '}') { depth--; if (!depth) { *e = p; return 1; } }
            p++;
        }
        return 0;
    }
    return 0;
}

/* Find "key" : "value" within [gs, ge) — copies value (no escapes) into out. Returns 1 on hit. */
/* Find "key" : "value" — copies value (no escapes) into out. Returns 1 on hit. */
static int find_key_str(const char* gs, const char* ge, const char* key, char* out, int cap) {
    int kl = 0;
    while (key[kl]) kl++;
    const char* p = gs;
    while (p < ge && *p) {
        if (*p != '"') { p++; continue; }
        const char* k = p + 1;
        int i = 0;
        while (i < kl && k[i] == key[i]) i++;
        if (i != kl || k[i] != '"') { p++; continue; }
        p = k + i + 1;
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        if (*p != ':') continue;
        p++;
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        if (*p != '"') continue;
        p++;
        int o = 0;
        while (*p && *p != '"' && o + 1 < cap) out[o++] = *p++;
        out[o] = '\0';
        return (*p == '"');
    }
    return 0;
}

static void read_set(void) {
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ,
                           FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    static char raw[2048]; /* static: -nostdlib has no __chkstk_ms */
    static char buf[2048];
    DWORD got = 0;
    ReadFile(h, raw, sizeof(raw) - 1, &got, NULL);
    CloseHandle(h);
    if (got == 0) return;
    raw[got] = '\0';

    strip_jsonc(raw, (int)got, buf, sizeof(buf));
    const char *gs, *ge;
    char val[32];
    int m;
    if (find_group(buf, "Player1", &gs, &ge)) {
        for (m = 0; m < 3; m++) {
            if (find_key_str(gs, ge, K_MODES[m], val, sizeof(val)))
                g_ball[m] = parse_hex(val);
            /* missing mode: keep current value (partial sets allowed) */
        }
    }
    if (find_group(buf, "Player1_UI", &gs, &ge)) {
        for (m = 0; m < 3; m++) {
            if (find_key_str(gs, ge, K_MODES[m], val, sizeof(val)))
                g_ui[m] = parse_hex(val);
        }
    }
}

static void log_line(const char* msg) {
    HANDLE h = CreateFileA(g_log_path, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                           OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    SetFilePointer(h, 0, NULL, FILE_END);
    DWORD wrote = 0, n = 0;
    while (msg[n]) n++;
    WriteFile(h, msg, n, &wrote, NULL);
    WriteFile(h, "\r\n", 2, &wrote, NULL);
    CloseHandle(h);
}

#include "brhook.h"

/* P1 hook color (floats, storage in brhook.h .bss; fed from g_ball). */
static int g_mode = 0;

static int hex4(unsigned int v, char* out) {
    const char* H = "0123456789ABCDEF";
    int i;
    for (i = 0; i < 8; i++) out[i] = H[(v >> (28 - i * 4)) & 0xF];
    out[8] = '\0';
    return 8;
}
/* Wide probe: identify hammy slot object layout + ball mesh material.
   Logs vtable, then every offset holding four consecutive 1.0f (D3DMAT-like)
   within +0..+0x100 of slot (hammy) / +0..+0x400 of ball mesh. Read-only. */
static void scan_white(const char* base, int span, char* line, int* po, int cap) {
    int o = *po, off;
    for (off = 0; off + 16 <= span; off += 4) {
        float* f = (float*)(base + off);
        if (IsBadReadPtr((const void*)f, 16)) break;
        if (f[0] == 1.0f && f[1] == 1.0f && f[2] == 1.0f && f[3] == 1.0f) {
            char hb[9];
            hex4((unsigned int)off, hb);
            if (o + 10 >= cap) break;
            line[o++] = '@';
            for (int k = 4; k < 8; k++) line[o++] = hb[k];
        }
    }
    *po = o;
}

static void deep_probe(App* app) {
    static char line[512];
    int o = 0, i;
    const char* tag = "DEEP";
    while (*tag) line[o++] = *tag++;
    for (i = 0; i < 3; i++) {
        DWORD off = 0x330 + (DWORD)i * 4;
        line[o++] = ' ';
        line[o++] = 'h';
        line[o++] = (char)('0' + i);
        line[o++] = '=';
        if (!app || IsBadReadPtr((const void*)((char*)app + off), 4)) {
            line[o++] = '?';
            continue;
        }
        DWORD slot = *(DWORD*)((char*)app + off);
        if (!slot || IsBadReadPtr((const void*)slot, 4)) {
            line[o++] = '0';
            continue;
        }
        DWORD vt = *(DWORD*)slot;
        char hb[9];
        hex4(slot, hb);
        for (int k = 0; k < 8; k++) line[o++] = hb[k];
        line[o++] = ':';
        hex4(vt, hb);
        for (int k = 0; k < 8; k++) line[o++] = hb[k];
        scan_white((const char*)slot, 0x100, line, &o, (int)sizeof(line));
    }
    /* ball mesh material hunt: App+0x244[0] object + white clusters */
    line[o++] = ' ';
    line[o++] = 'm';
    line[o++] = '=';
    if (!app || IsBadReadPtr((const void*)((char*)app + 0x244), 4)) {
        line[o++] = '?';
    } else {
        DWORD mesh = *(DWORD*)((char*)app + 0x244);
        if (!mesh || IsBadReadPtr((const void*)mesh, 4)) {
            line[o++] = '0';
        } else {
            char hb[9];
            hex4(mesh, hb);
            for (int k = 0; k < 8; k++) line[o++] = hb[k];
            line[o++] = ':';
            hex4(*(DWORD*)mesh, hb);
            for (int k = 0; k < 8; k++) line[o++] = hb[k];
            scan_white((const char*)mesh, 0x400, line, &o, (int)sizeof(line));
        }
    }
    line[o] = '\0';
    log_line(line);
}

/* UI probe: Hammy texture slots (App+0x330/334/338) + mode fields. Read-only. */

static void probe_ui(App* app, int mode) {
    static char line[512];
    int o = 0, i;
    const char* tag = "UI-PROBE mode=";
    while (*tag) line[o++] = *tag++;
    line[o++] = (char)('0' + mode);
    for (i = 0; i < 3; i++) {
        DWORD off = 0x330 + (DWORD)i * 4;
        line[o++] = ' ';
        line[o++] = 's';
        line[o++] = (char)('0' + i);
        line[o++] = '=';
        if (!app || IsBadReadPtr((const void*)((char*)app + off), 4)) {
            line[o++] = '?';
            continue;
        }
        DWORD slot = *(DWORD*)((char*)app + off);
        if (!slot || IsBadReadPtr((const void*)slot, 0x4C)) {
            line[o++] = '0';
            continue;
        }
        char hb[9];
        hex4(slot, hb);
        for (int k = 0; k < 8; k++) line[o++] = hb[k];
        line[o++] = ':';
        /* guessed D3DMAT at sprite+0xC: dif rgba floats */
        float* f = (float*)(slot + 0xC);
        for (int k = 0; k < 4; k++) {
            int v = (int)(f[k] * 100.0f);
            if (v < -9999) v = -9999;
            if (v > 99999) v = 99999;
            char nb[8];
            int n2 = 0, neg = 0;
            if (v < 0) { neg = 1; v = -v; }
            do { nb[n2++] = (char)('0' + v % 10); v /= 10; } while (v && n2 < 6);
            if (neg && o + 1 < (int)sizeof(line)) line[o++] = '-';
            while (n2 && o + 1 < (int)sizeof(line)) line[o++] = nb[--n2];
            if (k < 3 && o + 1 < (int)sizeof(line)) line[o++] = ',';
        }
    }
    /* App gameMode1-4 (+0x550..0x55C, unverified offsets, raw read) */
    line[o++] = ' ';
    line[o++] = 'g';
    line[o++] = 'm';
    for (i = 0; i < 4; i++) {
        line[o++] = '=';
        if (!app || IsBadReadPtr((const void*)((char*)app + 0x550 + i * 4), 4)) {
            line[o++] = '?';
        } else {
            char hb[9];
            hex4(*(DWORD*)((char*)app + 0x550 + i * 4), hb);
            for (int k = 0; k < 8; k++) line[o++] = hb[k];
        }
    }
    line[o] = '\0';
    log_line(line);
}

/* Read-back check: is the P1 tint actually in the table? Player 1 only. */
static void ball_check(DWORD board, int mode) {
    static char line[256];
    int o = 0, i;
    const char* tag = "BALL-CHECK mode=";
    while (*tag) line[o++] = *tag++;
    line[o++] = (char)('0' + mode);
    line[o++] = ' ';
    line[o++] = 'e'; line[o++] = 'x'; line[o++] = 'p'; line[o++] = '=';
    unsigned int hex = g_ball[mode];
    const char* H = "0123456789ABCDEF";
    for (i = 5; i >= 0; i--) line[o++] = H[(hex >> (i * 4)) & 0xF];
    line[o++] = 'F';
    line[o++] = 'F';
    line[o++] = ' ';
    line[o++] = 'g'; line[o++] = 'o'; line[o++] = 't'; line[o++] = '=';
    float* f = (float*)(board + BOARD_COLOR_BASE);
    unsigned char want[4];
    want[0] = (unsigned char)((hex >> 16) & 0xFF);
    want[1] = (unsigned char)((hex >> 8) & 0xFF);
    want[2] = (unsigned char)(hex & 0xFF);
    want[3] = 255;
    int diff = 0;
    for (i = 0; i < 4; i++) {
        int v = (int)(f[i] * 255.0f + 0.5f);
        if (v < 0) v = 0;
        if (v > 255) v = 255;
        if (v != want[i]) diff = 1;
        line[o++] = H[(v >> 4) & 0xF];
        line[o++] = H[v & 0xF];
    }
    line[o++] = ' ';
    if (diff) { line[o++] = 'D'; line[o++] = 'I'; line[o++] = 'F'; line[o++] = 'F'; }
    else { line[o++] = 'O'; line[o++] = 'K'; }
    line[o] = '\0';
    log_line(line);
}

/* Hammy portrait tint: App+0x330/334/338 hold sprite/material objects
   (verified live: white 1,1,1,1 D3DMAT at +0xC). Writes Diffuse RGB only.
   Validation: slot readable, dif alpha == 1.0 before first write per boot. */
static int g_ui_ok[3] = { 0, 0, 0 };

static void tint_ui(App* app, int mode) {
    unsigned int hex = g_ui[mode];
    float r = (float)((hex >> 16) & 0xFF) / 255.0f;
    float g = (float)((hex >> 8) & 0xFF) / 255.0f;
    float b = (float)(hex & 0xFF) / 255.0f;
    int i;
    for (i = 0; i < 3; i++) {
        DWORD off = 0x330 + (DWORD)i * 4;
        if (!app || IsBadReadPtr((const void*)((char*)app + off), 4)) continue;
        DWORD slot = *(DWORD*)((char*)app + off);
        if (!slot || IsBadReadPtr((const void*)slot, 0x4C)) continue;
        float* dif = (float*)(slot + 0xC);
        if (!g_ui_ok[i]) {
            if (dif[3] != 1.0f) continue; /* not the expected material: skip */
            g_ui_ok[i] = 1;
        }
        dif[0] = r;
        dif[1] = g;
        dif[2] = b;
        dif[3] = 1.0f;
    }
}

/* UI read-back: are the Hammy slot materials still our color? */
static void ui_check(App* app, int mode) {
    static char line[256];
    int o = 0, i;
    const char* tag = "UI-CHECK mode=";
    while (*tag) line[o++] = *tag++;
    line[o++] = (char)('0' + mode);
    unsigned int hex = g_ui[mode];
    const char* H = "0123456789ABCDEF";
    int want[3];
    want[0] = (hex >> 16) & 0xFF;
    want[1] = (hex >> 8) & 0xFF;
    want[2] = hex & 0xFF;
    for (i = 0; i < 3; i++) {
        line[o++] = ' ';
        line[o++] = 's';
        line[o++] = (char)('0' + i);
        line[o++] = '=';
        DWORD off = 0x330 + (DWORD)i * 4;
        if (!app || IsBadReadPtr((const void*)((char*)app + off), 4)) {
            line[o++] = '?';
            continue;
        }
        DWORD slot = *(DWORD*)((char*)app + off);
        if (!slot || IsBadReadPtr((const void*)slot, 0x4C)) {
            line[o++] = '0';
            continue;
        }
        float* dif = (float*)(slot + 0xC);
        int diff = 0, k;
        for (k = 0; k < 3; k++) {
            int v = (int)(dif[k] * 255.0f + 0.5f);
            if (v < 0) v = 0;
            if (v > 255) v = 255;
            if (v != want[k]) diff = 1;
            line[o++] = H[(v >> 4) & 0xF];
            line[o++] = H[v & 0xF];
        }
        line[o++] = diff ? '!' : '.';
    }
    line[o] = '\0';
    log_line(line);
}

/* TEMP GREEN TEST r2 (revert after MAKYUNI confirms):
   r1 wrote +0x10 (Diffuse-assumed) white->green and the ball lost its
   texture, so: (1) restore +0x10 to white, (2) drive green through +0x34
   instead, mirroring mknp_bumper_colors (released, working: emissive RGB
   at slot +0x34/38/3C, stride 0x50). Outline untouched. */
/* Status line (was TEMP GREEN TEST, now set-driven via the draw hook):
   hook/ok state, active mode, P1 RGB. No writes. */
static void green_test(App* app) {
    static char line[160];
    int o = 0, k;
    const char* H = "0123456789ABCDEF";
    const char* t = "BALL-COLOR hook=";
    (void)app;
    while (*t) line[o++] = *t++;
    line[o++] = (char)('0' + (g_br_inst == 1 ? 1 : 0));
    const char* t2 = " ok=";
    while (*t2) line[o++] = *t2++;
    line[o++] = (char)('0' + (g_br_ok ? 1 : 0));
    const char* t3 = " mode=";
    while (*t3) line[o++] = *t3++;
    line[o++] = (char)('0' + g_mode);
    const char* t4 = " rgb=";
    while (*t4) line[o++] = *t4++;
    float rgb[3];
    rgb[0] = g_hook_rgb0;
    rgb[1] = g_hook_rgb1;
    rgb[2] = g_hook_rgb2;
    for (k = 0; k < 3; k++) {
        int v = (int)(rgb[k] * 255.0f + 0.5f);
        if (v < 0) v = 0;
        if (v > 255) v = 255;
        line[o++] = H[(v >> 4) & 0xF];
        line[o++] = H[v & 0xF];
    }
    /* locked native: emissive triple + R + specular triple */
    const char* t5 = " nt=";
    while (*t5) line[o++] = *t5++;
    float nt[4];
    nt[0] = g_br_natR;
    nt[1] = g_br_nat0;
    nt[2] = g_br_nat1;
    nt[3] = g_br_nat2;
    for (k = 0; k < 4; k++) {
        int v = (int)(nt[k] * 255.0f + 0.5f);
        if (v < 0) v = 0;
        if (v > 255) v = 255;
        line[o++] = H[(v >> 4) & 0xF];
        line[o++] = H[v & 0xF];
    }
    const char* t6 = " ns=";
    while (*t6) line[o++] = *t6++;
    float ns[3];
    ns[0] = g_br_natS0;
    ns[1] = g_br_natS1;
    ns[2] = g_br_natS2;
    for (k = 0; k < 3; k++) {
        int v = (int)(ns[k] * 255.0f + 0.5f);
        if (v < 0) v = 0;
        if (v > 255) v = 255;
        line[o++] = H[(v >> 4) & 0xF];
        line[o++] = H[v & 0xF];
    }
    /* submesh slot identity: catches a fluctuating *(mw+4) */
    const char* t7 = " sub=";
    while (*t7) line[o++] = *t7++;
    line[o++] = (char)('0' + (g_br_sub > 9 ? 9 : g_br_sub));
    const char* t8 = " slot=";
    while (*t8) line[o++] = *t8++;
    {
        unsigned int sv = g_br_mat ? g_br_mat - 0x34 : 0;
        int sh;
        for (sh = 7; sh >= 0; sh--) line[o++] = H[(sv >> (sh * 4)) & 0xF];
    }
    line[o] = '\0';
    log_line(line);
}

static void write_slot(DWORD board, int slot, unsigned int hex) {
    float r = (float)((hex >> 16) & 0xFF) / 255.0f;
    float g = (float)((hex >> 8) & 0xFF) / 255.0f;
    float b = (float)(hex & 0xFF) / 255.0f;
    DWORD addr = board + BOARD_COLOR_BASE + (DWORD)slot * BOARD_COLOR_STRIDE;
    *(float*)(addr + 0x00) = r;
    *(float*)(addr + 0x04) = g;
    *(float*)(addr + 0x08) = b;
    *(float*)(addr + 0x0C) = 1.0f;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HB+ callback wiring (17-entry vtable, MinGW __thiscall convention)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) operator delete(thisptr);
    return thisptr;
}
static const char* __thiscall get_mod_name(void*) { return "mknp_player_colors"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, IModAPI* api) {
    g_api = api;
    *(void**)((char*)thisptr + 4) = api;
    mod_self();

    g_dll_dir[0] = '\0';
    if (g_module && GetModuleFileNameA(g_module, g_dll_dir, MAX_PATH) > 0) {
        char* s = NULL;
        for (char* p = g_dll_dir; *p; p++) if (*p == '\\') s = p;
        if (s) s[1] = '\0';
        else g_dll_dir[0] = '\0';
    }
    if (g_dll_dir[0])
        path_join(g_config_path, g_dll_dir, "mknp_player_colors.set");
    else
        nc_strncpy(g_config_path, "mknp_player_colors.set", MAX_PATH - 1);

    if (g_dll_dir[0])
        path_join(g_log_path, g_dll_dir, "mknp_player_colors.log");
    else
        nc_strncpy(g_log_path, "mknp_player_colors.log", MAX_PATH - 1);

    create_defaults();
    read_set();
    g_started = 1;
    {
        static char init[320];
        int o = 0, i;
        const char* t = "INIT log=";
        while (*t) init[o++] = *t++;
        for (i = 0; g_log_path[i] && o + 1 < (int)sizeof(init); i++) init[o++] = g_log_path[i];
        t = " set=";
        while (*t) init[o++] = *t++;
        for (i = 0; g_config_path[i] && o + 1 < (int)sizeof(init); i++) init[o++] = g_config_path[i];
        init[o] = '\0';
        log_line(init);
    }
}

static void __thiscall game_update(void*) {
    if (!g_started || !g_api) return;
    g_tick++;
    if ((g_tick % 120) == 0) read_set(); /* live edit ~every 2s */
    {
        static int hook_tried = 0;
        if (!hook_tried) {
            hook_tried = 1;
            ballhook_install();
            g_hook_rgb0 = 0.27f;
            g_hook_rgb1 = 0.27f;
            g_hook_rgb2 = 0.27f;
        }
    }

    /* UI tint runs with or without a scene (menus have no scene).
       Fresh boot defaults to mode 0 (1P); races update it. */
    static int last_ui_mode = -1;
    Scene* sc = HBAPI(g_api).GetScene();
    int np = 0;
    int mode = (last_ui_mode < 0) ? 0 : last_ui_mode;
    if (sc) {
        np = sc->player_count;
        if (np < 1) np = 1;
        if (np > 4) np = 4;
        /* Player1 ball only (schema defines Player1 + Player1_UI).
           1P_mode covers Tournament and Time Trials (both run np==1). */
        mode = (np <= 1) ? 0 : (np == 2) ? 1 : 2;
    }
    DWORD board = (DWORD)sc;
    {
        if (mode != last_ui_mode) {
            last_ui_mode = mode;
            tint_ui(HBAPI(g_api).GetApp(), mode);
            {
                static char ml[64];
                int o = 0;
                const char* t = "UI-APPLIED mode=";
                while (*t) ml[o++] = *t++;
                ml[o++] = (char)('0' + mode);
                ml[o++] = ' ';
                ml[o++] = 'u'; ml[o++] = 'i'; ml[o++] = '=';
                const char* H2 = "0123456789ABCDEF";
                unsigned int h2 = g_ui[mode];
                int i2;
                for (i2 = 5; i2 >= 0; i2--) ml[o++] = H2[(h2 >> (i2 * 4)) & 0xF];
                ml[o] = '\0';
                log_line(ml);
            }
        }
    }
    if (sc) write_slot(board, 0, g_ball[mode]);
    {
        unsigned int hex = g_ball[mode];
        g_mode = mode;
        g_hook_rgb0 = (float)((hex >> 16) & 0xFF) / 255.0f;
        g_hook_rgb1 = (float)((hex >> 8) & 0xFF) / 255.0f;
        g_hook_rgb2 = (float)(hex & 0xFF) / 255.0f;
    }
    if (sc) {
        static DWORD lastsc = 0;
        if ((DWORD)sc != lastsc) { lastsc = (DWORD)sc; g_br_needscrub = 1; }
        br_resolve(HBAPI(g_api).GetApp());
    }
    if (sc && (g_tick % 600) == 1) green_test(HBAPI(g_api).GetApp());
    if (!sc && g_deep_done) g_deep_done = 0; /* re-probe next race */
    if (!sc) { g_br_ok = 0; g_br_needscrub = 1; } /* menu: scrub on entry */
    {
        static int lastmode = -1;
        if (mode != lastmode) {
            static char ml[32];
            int o = 0;
            const char* t = "MODE now=";
            while (*t) ml[o++] = *t++;
            ml[o++] = (char)('0' + mode);
            ml[o++] = ' ';
            ml[o++] = 'n'; ml[o++] = 'p'; ml[o++] = '=';
            ml[o++] = (char)('0' + np);
            ml[o] = '\0';
            log_line(ml);
            lastmode = mode;
        }
    }
    if (sc && !g_deep_done) { g_deep_done = 1; deep_probe(HBAPI(g_api).GetApp()); }
    if ((g_tick % 600) == 1) {
        App* app = HBAPI(g_api).GetApp();
        tint_ui(app, mode); /* re-assert: slots fill late, native may reset */
        if (sc) ball_check(board, mode);
        ui_check(app, mode);
        probe_ui(app, mode);
    }
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
