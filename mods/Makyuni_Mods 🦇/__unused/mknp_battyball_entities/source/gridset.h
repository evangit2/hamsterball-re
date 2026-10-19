/* gridset.h -- per-level grid_speed multipliers
 * (mknp_battyball_entities_set.jsonc next to the DLL).
 * Needs from includer: log_mod(), gridmesh.h, malloc/free/memcpy/memcmp/
 * strlen, windows.h (or GRIDMESH_HARNESS posix stubs below).
 */
#ifndef GRIDSET_H
#define GRIDSET_H

#ifdef GRIDMESH_HARNESS
#include <sys/stat.h>
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif
#ifndef FILE_ATTRIBUTE_DIRECTORY
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#endif
static DWORD GetFileAttributesA(const char* p) {
    struct stat st;
    if (stat(p, &st) != 0) return INVALID_FILE_ATTRIBUTES;
    return S_ISDIR(st.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : 0x80;
}
#endif

static void log_mod(const char* msg);


/* ═══════════════════════════════════════════════════════════════════════════
 * Per-level grid_speed multipliers (mknp_battyball_entities_set.jsonc).
 * Level identity = S1 fingerprint (count + ref names), so file swaps and
 * renames still match. wait = slider seconds x multiplier.
 * ═══════════════════════════════════════════════════════════════════════════ */
#define SET_MAX_ENTRIES 32
static char  g_set_path[MAX_PATH];
static char  g_set_names[SET_MAX_ENTRIES][24];
static float g_set_mult[SET_MAX_ENTRIES];
static int   g_set_count = 0;
static float g_mult = 1.0f;          /* active level multiplier */
static unsigned g_s1_hash = 0;       /* runtime S1 fingerprint */
static int      g_s1_count = 0;

static const char g_set_default[] =
"\n"
"\n"
"// BATTYBALL ENTITIES MOD - SETTINGS //\n"
"\n"
"\n"
"\n"
"// ################################# \xe2\x94\x82    E N T I T I E S    \xe2\x94\x82 #################################\n"
"// \"Name\", { \"behaviour\": \"<behaviour>\", \"mesh\": \"<file in Levels/>\" } -- S1 REF:<Name>\n"
"[\n"
"    \"Woodbridge\", { \"behaviour\": \"Woodbridge\", \"mesh\": \"Woodbridge.MESHWORLD\" }\n"
"// ################################# \xe2\x94\x82    G R I D    \xe2\x94\x82 #################################\n"
"    \"grid_speed\", { \"level1\": 1.0, \"level2\": 1.0, \"level3\": 1.0, \"level4\": 1.0, \"level5\": 1.0, \"level6\": 1.0, \"level7\": 1.0, \"level8\": 1.0, \"level9\": 1.0, \"level10\": 1.0, \"level11\": 1.0, \"level12\": 1.0, \"level13\": 1.0, \"level14\": 1.0, \"level15\": 1.0 }\n"
"    \"neon_ballring_player1\", { \"level1\": [10.0, 10.0, 0.0, 1.0], \"level2\": [10.0, 10.0, 0.0, 1.0], \"level3\": [10.0, 10.0, 0.0, 1.0], \"level4\": [10.0, 10.0, 0.0, 1.0], \"level5\": [10.0, 10.0, 0.0, 1.0], \"level6\": [10.0, 10.0, 0.0, 1.0], \"level7\": [10.0, 10.0, 0.0, 1.0], \"level8\": [10.0, 10.0, 0.0, 1.0], \"level9\": [10.0, 10.0, 0.0, 1.0], \"level10\": [10.0, 10.0, 0.0, 1.0], \"level11\": [10.0, 10.0, 0.0, 1.0], \"level12\": [10.0, 10.0, 0.0, 1.0], \"level13\": [10.0, 10.0, 0.0, 1.0], \"level14\": [10.0, 10.0, 0.0, 1.0], \"level15\": [10.0, 10.0, 0.0, 1.0] }\n"
"    \"neon_glow_player1\", {\n"
"//  \"level7\": [0.0, 1.0, 1.0, 1.0]\n"
"}\n"
"]\n"
"// #####################################################################################\n"
"";

static void s1_feed_byte(unsigned char b) {
    g_s1_hash ^= b;
    g_s1_hash *= 16777619u;
}

static float set_parse_num(const char** pp) {
    const char* p = *pp;
    int sign = 1;
    float v = 0, frac = 0, div = 1;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    if (*p == '-') { sign = -1; p++; }
    else if (*p == '+') p++;
    while (*p >= '0' && *p <= '9') { v = v * 10 + (float)(*p - '0'); p++; }
    if (*p == '.') {
        p++;
        while (*p >= '0' && *p <= '9') {
            frac = frac * 10 + (float)(*p - '0');
            div *= 10;
            p++;
        }
    }
    *pp = p;
    return (float)sign * (v + frac / div);
}

static void set_lower(char* d, const char* s, int n) {
    int i = 0;
    while (i < n - 1 && s[i]) {
        char c = s[i];
        d[i] = (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
        i++;
    }
    d[i] = '\0';
}

static int set_name_eq(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return *a == *b;
}

static int set_isnum(char c) {
    return (c >= '0' && c <= '9') || c == '-' || c == '+' || c == '.';
}

static void load_set_file(void) {
    unsigned len = 0;
    unsigned char* data;
    unsigned char* txt;
    const char* p;
    const char* q;
    const char* qend;
    const char* found = NULL;
    unsigned i;
    int in_str = 0;
    g_set_count = 0;
    if (!g_set_path[0]) return;
    if (GetFileAttributesA(g_set_path) == INVALID_FILE_ATTRIBUTES) {
        gm_write_file(g_set_path, (const unsigned char*)g_set_default,
                      (unsigned)strlen(g_set_default));
        log_mod("  GRID set: created default jsonc");
    }
    data = gm_read_file(g_set_path, &len);
    if (!data) {
        log_mod("  GRID set: unreadable, x1.0");
        return;
    }
    txt = (unsigned char*)malloc(len + 1);
    if (!txt) {
        free(data);
        return;
    }
    memcpy(txt, data, len);
    txt[len] = 0;
    free(data);
    for (i = 0; i < len; i++) {
        unsigned char c = txt[i];
        if (in_str) {
            if (c == '"') in_str = 0;
        } else {
            if (c == '"') in_str = 1;
            else if (c == '/' && i + 1 < len && txt[i + 1] == '/') {
                while (i < len && txt[i] != '\n') {
                    txt[i] = ' ';
                    i++;
                }
            }
        }
    }
    p = (const char*)txt;
    q = p;
    qend = len > 12 ? p + len - 12 : p;
    while (q <= qend) {
        if (memcmp(q, "\"grid_speed\"", 12) == 0) {
            found = q;
            break;
        }
        q++;
    }
    if (!found) {
        free(txt);
        log_mod("  GRID set: no grid_speed");
        return;
    }
    p = found + 12;
    while (*p && *p != '{') p++;
    if (*p == '{') p++;
    while (*p && *p != '}' && g_set_count < SET_MAX_ENTRIES) {
        if (*p == '"') {
            char key[24];
            int ki = 0;
            float v;
            p++;
            while (*p && *p != '"' && ki < 23) {
                key[ki++] = *p++;
            }
            key[ki] = '\0';
            if (*p == '"') p++;
            while (*p && *p != '}' && *p != '"' && *p != ',' &&
                   !set_isnum(*p))
                p++;
            if (!*p || *p == '}' || *p == '"' || *p == ',') break;
            v = set_parse_num(&p);
            set_lower(g_set_names[g_set_count], key, 24);
            g_set_mult[g_set_count] = v;
            g_set_count++;
        } else {
            p++;
        }
    }
    free(txt);
}

/* Race slot 1-15 (MAKYUNI order): 1 WarmUp, 2 Beginner/Cascade, 3 Intermediate,
 * 4 Dizzy, 5 Tower, 6 Up, 7 Neon, 8 Expert, 9 Odd, 10 Toob, 11 Wobbly,
 * 12 Glass, 13 Sky, 14 Master, 15 Impossible. SlotID authoritative, filenames
 * ignored. */
static int gridset_key_for_slot(int slot, char* out, int outn) {
    int n = 0;
    int v = slot;
    char tmp[8];
    int tn = 0;
    if (slot < 1 || slot > 15 || !out || outn < 8) return 0;
    if (v >= 10) tmp[tn++] = (char)('0' + v / 10);
    tmp[tn++] = (char)('0' + v % 10);
    tmp[tn] = '\0';
    out[n++] = 'l'; out[n++] = 'e'; out[n++] = 'v';
    out[n++] = 'e'; out[n++] = 'l';
    {
        int i = 0;
        while (tmp[i] && n < outn - 1) out[n++] = tmp[i++];
    }
    out[n] = '\0';
    return 1;
}

/* Slot-based multiplier: level<N> key, N = race slot. Absent/commented = x1.0. */
static float gridset_level_mult_slot(int slot) {
    char want[24];
    float mult = 1.0f;
    int k;
    load_set_file();
    if (!gridset_key_for_slot(slot, want, sizeof(want))) {
        log_mod("  GRID set: bad slot, x1.0");
        return 1.0f;
    }
    for (k = 0; k < g_set_count; k++) {
        if (set_name_eq(want, g_set_names[k])) {
            mult = g_set_mult[k];
            break;
        }
    }
    {
        char mbuf[96];
        int m10 = (int)(mult * 10.0f);
        snprintf(mbuf, sizeof(mbuf), "  GRID set: slot%d mult=x%d.%d",
                 slot, m10 / 10, m10 % 10);
        log_mod(mbuf);
    }
    return mult;
}

/* BallBorder ring colors (neon_ballring_player1): levelN -> [r,g,b,a].
 * Absent/commented level = ring untouched on that level (no glow). */
static char  g_ring_names[SET_MAX_ENTRIES][24];
static float g_ring_col[SET_MAX_ENTRIES][4];
static int   g_ring_count = 0;
static float g_ring_active[4] = { 10.0f, 10.0f, 0.0f, 1.0f };
static int   g_ring_on = 0;

static void load_ring_file(void) {
    unsigned len = 0;
    unsigned char* data;
    unsigned char* txt;
    const char* p;
    const char* q;
    const char* qend;
    const char* found = NULL;
    unsigned i;
    int in_str = 0;
    g_ring_count = 0;
    if (!g_set_path[0]) return;
    data = gm_read_file(g_set_path, &len);
    if (!data) return;
    txt = (unsigned char*)malloc(len + 1);
    if (!txt) {
        free(data);
        return;
    }
    memcpy(txt, data, len);
    txt[len] = 0;
    free(data);
    for (i = 0; i < len; i++) {
        unsigned char c = txt[i];
        if (in_str) {
            if (c == '"') in_str = 0;
        } else {
            if (c == '"') in_str = 1;
            else if (c == '/' && i + 1 < len && txt[i + 1] == '/') {
                while (i < len && txt[i] != '\n') {
                    txt[i] = ' ';
                    i++;
                }
            }
        }
    }
    p = (const char*)txt;
    q = p;
    qend = len > 23 ? p + len - 23 : p;
    while (q <= qend) {
        if (memcmp(q, "\"neon_ballring_player1\"", 23) == 0) {
            found = q;
            break;
        }
        q++;
    }
    if (!found) {
        free(txt);
        return;
    }
    p = found + 23;
    while (*p && *p != '{') p++;
    if (*p == '{') p++;
    while (*p && *p != '}' && g_ring_count < SET_MAX_ENTRIES) {
        if (*p == '"') {
            char key[24];
            int ki = 0;
            float v[4];
            int vi;
            p++;
            while (*p && *p != '"' && ki < 23) {
                key[ki++] = *p++;
            }
            key[ki] = '\0';
            if (*p == '"') p++;
            while (*p && *p != '}' && *p != '[') p++;
            if (*p != '[') break;
            p++;
            for (vi = 0; vi < 4; vi++) {
                while (*p && *p != '}' && *p != ']' && !set_isnum(*p)) p++;
                if (!*p || *p == '}' || *p == ']') break;
                v[vi] = set_parse_num(&p);
            }
            if (vi < 4) break;
            set_lower(g_ring_names[g_ring_count], key, 24);
            g_ring_col[g_ring_count][0] = v[0];
            g_ring_col[g_ring_count][1] = v[1];
            g_ring_col[g_ring_count][2] = v[2];
            g_ring_col[g_ring_count][3] = v[3];
            g_ring_count++;
        } else {
            p++;
        }
    }
    free(txt);
}

/* Slot-based ring RGBA: level<N> key, N = race slot.
 * Returns 1 + RGBA when the slot key is present, 0 (ring off) otherwise. */
static int gridset_level_ring_slot(int slot, float* out) {
    char want[24];
    int k;
    load_ring_file();
    if (!out) return 0;
    if (!gridset_key_for_slot(slot, want, sizeof(want))) {
        log_mod("  BORDER set: bad slot, off");
        return 0;
    }
    for (k = 0; k < g_ring_count; k++) {
        if (set_name_eq(want, g_ring_names[k])) {
            char rbuf[96];
            out[0] = g_ring_col[k][0];
            out[1] = g_ring_col[k][1];
            out[2] = g_ring_col[k][2];
            out[3] = g_ring_col[k][3];
            snprintf(rbuf, sizeof(rbuf),
                     "  BORDER set: slot%d rgba=(%d,%d,%d,%d)", slot,
                     (int)(out[0] * 100.0f), (int)(out[1] * 100.0f),
                     (int)(out[2] * 100.0f), (int)(out[3] * 100.0f));
            log_mod(rbuf);
            return 1;
        }
    }
    {
        char obuf[96];
        snprintf(obuf, sizeof(obuf), "  BORDER set: slot%d off (not in set)", slot);
        log_mod(obuf);
    }
    return 0;
}

/* P1 emitter glow (neon_glow_player1): levelN -> [r,g,b,a]. Same syntax as
 * the ring. Absent/commented level = emitter untouched (native glow). */
static char  g_glow_names[SET_MAX_ENTRIES][24];
static float g_glow_col[SET_MAX_ENTRIES][4];
static int   g_glow_count = 0;
static float g_glow_active[4] = { 10.0f, 10.0f, 0.0f, 1.0f };
static int   g_glow_on = 0;

static void load_glow_file(void) {
    unsigned len = 0;
    unsigned char* data;
    unsigned char* txt;
    const char* p;
    const char* q;
    const char* qend;
    const char* found = NULL;
    unsigned i;
    int in_str = 0;
    g_glow_count = 0;
    if (!g_set_path[0]) return;
    data = gm_read_file(g_set_path, &len);
    if (!data) return;
    txt = (unsigned char*)malloc(len + 1);
    if (!txt) {
        free(data);
        return;
    }
    memcpy(txt, data, len);
    txt[len] = 0;
    free(data);
    for (i = 0; i < len; i++) {
        unsigned char c = txt[i];
        if (in_str) {
            if (c == '"') in_str = 0;
        } else {
            if (c == '"') in_str = 1;
            else if (c == '/' && i + 1 < len && txt[i + 1] == '/') {
                while (i < len && txt[i] != '\n') {
                    txt[i] = ' ';
                    i++;
                }
            }
        }
    }
    p = (const char*)txt;
    q = p;
    qend = len > 19 ? p + len - 19 : p;
    while (q <= qend) {
        if (memcmp(q, "\"neon_glow_player1\"", 19) == 0) {
            found = q;
            break;
        }
        q++;
    }
    if (!found) {
        free(txt);
        return;
    }
    p = found + 19;
    while (*p && *p != '{') p++;
    if (*p == '{') p++;
    while (*p && *p != '}' && g_glow_count < SET_MAX_ENTRIES) {
        if (*p == '"') {
            char key[24];
            int ki = 0;
            float v[4];
            int vi;
            p++;
            while (*p && *p != '"' && ki < 23) {
                key[ki++] = *p++;
            }
            key[ki] = '\0';
            if (*p == '"') p++;
            while (*p && *p != '}' && *p != '[') p++;
            if (*p != '[') break;
            p++;
            for (vi = 0; vi < 4; vi++) {
                while (*p && *p != '}' && *p != ']' && !set_isnum(*p)) p++;
                if (!*p || *p == '}' || *p == ']') break;
                v[vi] = set_parse_num(&p);
            }
            if (vi < 4) break;
            set_lower(g_glow_names[g_glow_count], key, 24);
            g_glow_col[g_glow_count][0] = v[0];
            g_glow_col[g_glow_count][1] = v[1];
            g_glow_col[g_glow_count][2] = v[2];
            g_glow_col[g_glow_count][3] = v[3];
            g_glow_count++;
        } else {
            p++;
        }
    }
    free(txt);
}

/* Slot-based glow RGBA: level<N> key, N = race slot.
 * Returns 1 + RGBA when the slot key is present, 0 (native glow) otherwise. */
static int gridset_level_glow_slot(int slot, float* out) {
    char want[24];
    int k;
    load_glow_file();
    if (!out) return 0;
    if (!gridset_key_for_slot(slot, want, sizeof(want))) {
        log_mod("  GLOW set: bad slot, native");
        return 0;
    }
    for (k = 0; k < g_glow_count; k++) {
        if (set_name_eq(want, g_glow_names[k])) {
            char rbuf[96];
            out[0] = g_glow_col[k][0];
            out[1] = g_glow_col[k][1];
            out[2] = g_glow_col[k][2];
            out[3] = g_glow_col[k][3];
            snprintf(rbuf, sizeof(rbuf),
                     "  GLOW set: slot%d rgba=(%d,%d,%d,%d)", slot,
                     (int)(out[0] * 100.0f), (int)(out[1] * 100.0f),
                     (int)(out[2] * 100.0f), (int)(out[3] * 100.0f));
            log_mod(rbuf);
            return 1;
        }
    }
    {
        char obuf[96];
        snprintf(obuf, sizeof(obuf), "  GLOW set: slot%d native (not in set)", slot);
        log_mod(obuf);
    }
    return 0;
}

static void gridset_init(const char* moddir) {
    g_set_path[0] = '\0';
    if (!moddir || !moddir[0]) return;
    snprintf(g_set_path, sizeof(g_set_path),
             "%s\\mknp_battyball_entities_set.jsonc", moddir);
}

#endif /* GRIDSET_H */
