/*
 * setparse.inc — jsonc set-file parser for mknp_separate_music.
 * Shared by the HB+ mod and the native unit test (test_parse.c).
 *
 * Includer provides:
 *   track_entry_t { const char* name; const char* deffile;
 *                   char file[128]; int hexord; char id[32]; }
 *   g_tracks[], NUM_TRACKS, nc_strcmp(a, b)
 *
 * Format: [ "id", { "name": "...", "file": "...", "hex": "00" }, ... ]
 *   // and slash-star comments allowed anywhere. Unknown props ignored.
 *   Lookup is by name (trimmed). Last entry wins. Missing/empty file =
 *   play original. Missing hex = order 0. Table resets each parse so
 *   removals revert to original.
 */

typedef struct { const char* p; } jcur;

static void j_skip(jcur* j) {
    for (;;) {
        char c = j->p[0];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') { j->p++; continue; }
        if (c == '/' && j->p[1] == '/') {
            j->p += 2;
            while (j->p[0] && j->p[0] != '\n') j->p++;
            continue;
        }
        if (c == '/' && j->p[1] == '*') {
            j->p += 2;
            while (j->p[0] && !(j->p[0] == '*' && j->p[1] == '/')) j->p++;
            if (j->p[0]) j->p += 2;
            continue;
        }
        return;
    }
}

/* parse "..." into out (cap incl NUL). Returns 1 on ok. */
static int j_string(jcur* j, char* out, int cap) {
    int n = 0;
    if (j->p[0] != '"') return 0;
    j->p++;
    while (j->p[0] && j->p[0] != '"') {
        char c = *j->p++;
        if (c == '\\') {
            char e = *j->p;
            int k;
            if (!e) break;
            j->p++;
            if (e == 'n') c = '\n';
            else if (e == 't') c = '\t';
            else if (e == 'r') c = '\r';
            else if (e == 'u') {
                for (k = 0; k < 4 && *j->p; k++) j->p++;
                c = '?';
            } else c = e;
        }
        if (n < cap - 1) out[n++] = c;
    }
    out[n] = '\0';
    if (j->p[0] == '"') j->p++;
    return 1;
}

/* skip any value: string / { } / [ ] / bare word */
static void j_skipval(jcur* j) {
    char tmp[8];
    j_skip(j);
    if (j->p[0] == '"') { j_string(j, tmp, sizeof(tmp)); return; }
    if (j->p[0] == '{' || j->p[0] == '[') {
        char open = *j->p++;
        char close = (open == '{') ? '}' : ']';
        int depth = 1;
        while (j->p[0] && depth > 0) {
            if (j->p[0] == '"') { j_string(j, tmp, sizeof(tmp)); continue; }
            if (j->p[0] == '/' && (j->p[1] == '/' || j->p[1] == '*')) { j_skip(j); continue; }
            if (j->p[0] == open) depth++;
            else if (j->p[0] == close) depth--;
            j->p++;
        }
        return;
    }
    while (j->p[0] && j->p[0] != ',' && j->p[0] != '}' && j->p[0] != ']' &&
           j->p[0] != ' ' && j->p[0] != '\t' && j->p[0] != '\r' && j->p[0] != '\n')
        j->p++;
}

static void j_rtrim(char* s) {
    int n = 0;
    while (s[n]) n++;
    while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t')) s[--n] = '\0';
}

/* hex string -> 0..255 ("00"=0, "0A"=10, "0x1B"=27). Garbage -> 0. */
static int j_hexval(const char* s) {
    int v = 0;
    int i;
    int any = 0;
    while (*s == ' ' || *s == '\t') s++;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s += 2;
    for (i = 0; i < 4 && s[i]; i++) {
        char c = s[i];
        int dd;
        if (c >= '0' && c <= '9') dd = c - '0';
        else if (c >= 'a' && c <= 'f') dd = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') dd = c - 'A' + 10;
        else break;
        v = v * 16 + dd;
        any = 1;
    }
    if (!any) return 0;
    if (v > 255) v = 255;
    return v;
}

static void parse_set_jsonc(const char* text) {
    jcur j;
    size_t i;
    for (i = 0; i < NUM_TRACKS; i++) {
        g_tracks[i].file[0] = '\0';
        g_tracks[i].hexord = 0;
        g_tracks[i].id[0] = '\0';
    }
    if (!text) return;
    j.p = text;
    j_skip(&j);
    if (j.p[0] == '[') j.p++;
    for (;;) {
        char id[32];
        char nm[64];
        char fl[128];
        char hx[16];
        int have_name = 0;
        int have_file = 0;
        int have_hex = 0;
        size_t k;
        j_skip(&j);
        if (!j.p[0] || j.p[0] == ']') break;
        if (!j_string(&j, id, sizeof(id))) {
            if (j.p[0] && j.p[0] != ']') j.p++;
            continue;
        }
        j_skip(&j);
        if (j.p[0] == ',') j.p++; /* tolerate "id", { ... } */
        j_skip(&j);
        if (j.p[0] != '{') continue;
        j.p++;
        nm[0] = fl[0] = hx[0] = '\0';
        for (;;) {
            char prop[16];
            j_skip(&j);
            if (!j.p[0]) break;
            if (j.p[0] == '}') { j.p++; break; }
            if (!j_string(&j, prop, sizeof(prop))) { j_skipval(&j); }
            else {
                j_skip(&j);
                if (j.p[0] != ':') { j_skipval(&j); }
                else {
                    j.p++;
                    j_skip(&j);
                    if (j.p[0] != '"') { j_skipval(&j); }
                    else if (!nc_strcmp(prop, "name")) {
                        j_string(&j, nm, sizeof(nm)); have_name = 1;
                    } else if (!nc_strcmp(prop, "file")) {
                        j_string(&j, fl, sizeof(fl)); have_file = 1;
                    } else if (!nc_strcmp(prop, "hex")) {
                        j_string(&j, hx, sizeof(hx)); have_hex = 1;
                    } else j_skipval(&j);
                }
            }
            j_skip(&j);
            if (j.p[0] == ',') j.p++;
        }
        if (!have_name) continue;
        j_rtrim(nm);
        j_rtrim(fl);
        for (i = 0; i < NUM_TRACKS; i++) {
            if (!nc_strcmp(nm, g_tracks[i].name)) {
                if (have_file && fl[0]) {
                    for (k = 0; fl[k] && k < sizeof(g_tracks[i].file) - 1; k++)
                        g_tracks[i].file[k] = fl[k];
                    g_tracks[i].file[k] = '\0';
                } else g_tracks[i].file[0] = '\0';
                g_tracks[i].hexord = have_hex ? j_hexval(hx) : 0;
                for (k = 0; id[k] && k < sizeof(g_tracks[i].id) - 1; k++)
                    g_tracks[i].id[k] = id[k];
                g_tracks[i].id[k] = '\0';
                break;
            }
        }
        j_skip(&j);
        if (j.p[0] == ',') j.p++;
    }
}
