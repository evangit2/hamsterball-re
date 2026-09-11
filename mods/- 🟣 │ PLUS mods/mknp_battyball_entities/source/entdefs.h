/* entdefs.h -- named ENTITY definitions from mknp_battyball_entities_set.jsonc.
 * Format (verbatim MAKYUNI schema, top-level pairs next to grid_speed):
 *     "Woodbridge", { "behaviour": "Woodbridge", "mesh": "Woodbridge.MESHWORLD" },
 * Key   = entity name, also the S1 ref substring (REF:Woodbridge).
 * Value = { "behaviour": "<name>", "mesh": "<file in Levels/>" }.
 * Behaviour is stored for dispatch; v1be implements no behaviours (static).
 * Mesh always loads from Levels/ (basename + g_levels_dir validate).
 * Needs from includer: log_mod(), gm_read_file(), malloc/free/memcpy/strlen,
 *   windows.h (MAX_PATH), g_set_path (gridset.h -- include AFTER gridset.h).
 */
#ifndef ENTDEFS_H
#define ENTDEFS_H

#define ENT_MAX_DEFS 16
#define ENT_NAME_N 48
#define ENT_BEH_N 32
#define ENT_MESH_N 64

static char g_ent_name[ENT_MAX_DEFS][ENT_NAME_N];
static char g_ent_beh[ENT_MAX_DEFS][ENT_BEH_N];
static char g_ent_mesh[ENT_MAX_DEFS][ENT_MESH_N];
static int  g_ent_count = 0;

static int ent_key_known(const char* key) {
    char low[64];
    int i = 0;
    while (i < 63 && key[i]) {
        char c = key[i];
        low[i] = (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
        i++;
    }
    low[i] = '\0';
    if (set_name_eq(low, "grid_speed")) return 1;
    if (set_name_eq(low, "neon_ballring_player1")) return 1;
    if (set_name_eq(low, "neon_glow_player1")) return 1;
    return 0;
}

/* Extract "subkey": "value" inside a {..} block. Returns 1 + value. */
static int ent_block_str(const char* b, const char* bend,
                         const char* sub, char* out, int outn) {
    const char* p = b;
    int sl = 0;
    while (sub[sl]) sl++;
    while (p < bend) {
        if (*p == '"') {
            const char* k = p + 1;
            const char* ke = k;
            while (ke < bend && *ke && *ke != '"') ke++;
            if (ke < bend && (ke - k) == sl && memcmp(k, sub, (unsigned)sl) == 0) {
                const char* v = ke + 1;
                while (v < bend && (*v == ' ' || *v == '\t' || *v == '\n' ||
                       *v == '\r' || *v == ':' || *v == ',')) v++;
                if (v < bend && *v == '"') {
                    const char* vs = v + 1;
                    const char* ve = vs;
                    int i = 0;
                    while (ve < bend && *ve && *ve != '"' && i < outn - 1) {
                        out[i++] = *ve++;
                    }
                    out[i] = '\0';
                    return (*ve == '"') ? 1 : 0;
                }
                return 0;
            }
            p = ke;
        }
        p++;
    }
    return 0;
}

static void load_entities_file(void) {
    unsigned len = 0;
    unsigned char* data;
    unsigned char* txt;
    const char* p;
    const char* end;
    unsigned i;
    int in_str = 0;
    g_ent_count = 0;
    if (!g_set_path[0]) return;
    data = gm_read_file(g_set_path, &len);
    if (!data) {
        log_mod("  ENT set: unreadable, 0 defs");
        return;
    }
    txt = (unsigned char*)malloc(len + 1);
    if (!txt) { free(data); return; }
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
                while (i < len && txt[i] != '\n') { txt[i] = ' '; i++; }
            }
        }
    }
    p = (const char*)txt;
    end = p + len;
    while (p < end && g_ent_count < ENT_MAX_DEFS) {
        if (*p == '"') {
            const char* ks = p + 1;
            const char* ke = ks;
            char key[ENT_NAME_N];
            int ki = 0;
            const char* q;
            const char* bs;
            const char* be;
            int depth;
            while (ke < end && *ke && *ke != '"') ke++;
            if (ke >= end || !*ke) break;
            while (ks < ke && ki < ENT_NAME_N - 1) key[ki++] = *ks++;
            key[ki] = '\0';
            q = ke + 1;
            while (q < end && (*q == ' ' || *q == '\t' || *q == '\n' ||
                   *q == '\r' || *q == ',' || *q == ':')) q++;
            if (q >= end || *q != '{') { p = ke + 1; continue; }
            bs = q + 1;
            depth = 1;
            be = bs;
            while (be < end && depth > 0) {
                if (*be == '{') depth++;
                else if (*be == '}') depth--;
                be++;
            }
            if (depth != 0) break;
            be--; /* at closing } */
            if (!ent_key_known(key)) {
                char beh[ENT_BEH_N];
                char mesh[ENT_MESH_N];
                beh[0] = '\0';
                mesh[0] = '\0';
                ent_block_str(bs, be, "behaviour", beh, sizeof(beh));
                ent_block_str(bs, be, "behavior", beh, sizeof(beh));
                ent_block_str(bs, be, "mesh", mesh, sizeof(mesh));
                if (mesh[0]) {
                    int ni = 0;
                    while (ni < ENT_NAME_N - 1 && key[ni]) {
                        g_ent_name[g_ent_count][ni] = key[ni];
                        ni++;
                    }
                    g_ent_name[g_ent_count][ni] = '\0';
                    ni = 0;
                    while (ni < ENT_BEH_N - 1 && beh[ni]) {
                        g_ent_beh[g_ent_count][ni] = beh[ni];
                        ni++;
                    }
                    g_ent_beh[g_ent_count][ni] = '\0';
                    ni = 0;
                    while (ni < ENT_MESH_N - 1 && mesh[ni]) {
                        g_ent_mesh[g_ent_count][ni] = mesh[ni];
                        ni++;
                    }
                    g_ent_mesh[g_ent_count][ni] = '\0';
                    g_ent_count++;
                }
            }
            p = be + 1;
            continue;
        }
        p++;
    }
    free(txt);
    {
        char ebuf[64];
        snprintf(ebuf, sizeof(ebuf), "  ENT set: %d def(s)", g_ent_count);
        log_mod(ebuf);
        for (i = 0; (int)i < g_ent_count; i++) {
            char dbuf[128];
            snprintf(dbuf, sizeof(dbuf), "  ENT def %s behaviour=%s mesh=%s",
                     g_ent_name[i], g_ent_beh[i], g_ent_mesh[i]);
            log_mod(dbuf);
        }
    }
}

/* Basename after last / or \\ (mesh values may carry a path). */
static void ent_basename(const char* in, char* out, int outn) {
    const char* b = in;
    const char* p = in;
    int i = 0;
    while (*p) {
        if (*p == '/' || *p == '\\') b = p + 1;
        p++;
    }
    while (i < outn - 1 && b[i]) { out[i] = b[i]; i++; }
    out[i] = '\0';
}

/* Strip .MESHWORLD suffix (case-insens) for the ctor path. */
static void ent_strip_ext(const char* in, char* out, int outn) {
    int n = 0;
    int i = 0;
    while (in[n] && n < outn - 1) n++;
    if (n > 10) {
        const char* e = in + n - 10;
        if ((e[0]=='.'||e[0]=='_') &&
            (e[1]=='M'||e[1]=='m') && (e[2]=='E'||e[2]=='e') &&
            (e[3]=='S'||e[3]=='s') && (e[4]=='H'||e[4]=='h') &&
            (e[5]=='W'||e[5]=='w') && (e[6]=='O'||e[6]=='o') &&
            (e[7]=='R'||e[7]=='r') && (e[8]=='L'||e[8]=='l') &&
            (e[9]=='D'||e[9]=='d')) n -= 10;
    }
    while (i < n && i < outn - 1) { out[i] = in[i]; i++; }
    out[i] = '\0';
}

#endif /* ENTDEFS_H */
