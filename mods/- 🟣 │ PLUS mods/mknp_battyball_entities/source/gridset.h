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
"// ################################# \xe2\x94\x82    G R I D    \xe2\x94\x82 #################################\n"
"[\n"
"    \"grid_speed\", { \"level1\": 1.0, \"level2\": 1.0, \"level3\": 1.0, \"level4\": 1.0, \"level5\": 1.0, \"level6\": 1.0, \"level7\": 1.0, \"level8\": 1.0, \"level9\": 1.0, \"level10\": 1.0, \"level11\": 1.0, \"level12\": 1.0, \"level13\": 1.0, \"level14\": 1.0, \"level15\": 1.0 }\n"
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

/* Identify the level file by S1 fingerprint, return its multiplier. */
static float gridset_level_mult(const char* levels_dir, unsigned s1hash,
                                int s1count) {
    static char files[GM_MAX_LIST][MAX_PATH];
    int n, i, k;
    char lb[64];
    float mult = 1.0f;
    load_set_file();
    if (!levels_dir || !levels_dir[0] || !s1count) return 1.0f;
    n = gm_list_mw(levels_dir, files, GM_MAX_LIST);
    for (i = 0; i < n; i++) {
        unsigned len = 0;
        unsigned char* d = gm_read_file(files[i], &len);
        int fc = 0;
        unsigned fh;
        if (!d) continue;
        fh = gm_s1_hash(d, len, &fc);
        free(d);
        if (fc == s1count && fh == s1hash) {
            const char* b = files[i];
            const char* pp = b;
            int bl = 0;
            int L;
            while (*pp) {
                if (*pp == '\\' || *pp == '/') b = pp + 1;
                pp++;
            }
            while (bl < 63 && b[bl]) {
                char c = b[bl];
                lb[bl] = (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
                bl++;
            }
            lb[bl] = '\0';
            L = bl;
            if (L > 10 && set_name_eq(lb + L - 10, ".meshworld"))
                lb[L - 10] = '\0';
            for (k = 0; k < g_set_count; k++) {
                if (set_name_eq(lb, g_set_names[k])) {
                    mult = g_set_mult[k];
                    break;
                }
            }
            {
                char mbuf[96];
                int m10 = (int)(mult * 10.0f);
                snprintf(mbuf, sizeof(mbuf), "  GRID set: %s mult=x%d.%d",
                         lb, m10 / 10, m10 % 10);
                log_mod(mbuf);
            }
            return mult;
        }
    }
    log_mod("  GRID set: unknown level, x1.0");
    return 1.0f;
}

static void gridset_init(const char* moddir) {
    g_set_path[0] = '\0';
    if (!moddir || !moddir[0]) return;
    snprintf(g_set_path, sizeof(g_set_path),
             "%s\\mknp_battyball_entities_set.jsonc", moddir);
}

#endif /* GRIDSET_H */
