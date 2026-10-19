/* entdefs.h -- named ENTITY definitions from mknp_entities_plus_set.jsonc.
 * Format (verbatim MAKYUNI schema, top-level pairs next to grid_speed):
 *     "REF:Woodbridge", { "behaviour": "REF:Woodbridge", "mesh": "Woodbridge.MESHWORLD" },
 * Key   = entity name with prefix, also the S1 ref substring (REF:Woodbridge).
 * Prefix split (v1b): REF:<name> = entity (S1 ref point spawn),
 * E:<name> = event (E:Launch / E:Woodbridge_area / E:Cheesepit drivers below,
 * treated as event quads/planes). v1e: the prefix is a ROUTING label only --
 * stored/matched names are prefix-STRIPPED, so "E:Cheesepit" matches S1
 * "REF:Cheesepit", "E:Cheesepit01" or bare "Cheesepit" alike. Behaviour
 * match is prefix-blind: bare, REF: and E: forms map identically. Mesh always loads from Levels/.
 * Needs from includer: log_mod(), gm_read_file(), malloc/free/memcpy/strlen,
 *   windows.h (MAX_PATH), g_set_path (gridset.h -- include AFTER gridset.h).
 */
#ifndef ENTDEFS_H
#define ENTDEFS_H

#define ENT_MAX_DEFS 64
#define ENT_NAME_N 48
#define ENT_BEH_N 32
#define ENT_MESH_N 64
#define ENT_SND_N 64

static char g_ent_name[ENT_MAX_DEFS][ENT_NAME_N];
static char g_ent_beh[ENT_MAX_DEFS][ENT_BEH_N];
static char g_ent_mesh[ENT_MAX_DEFS][ENT_MESH_N];
static char g_ent_snd[ENT_MAX_DEFS][ENT_SND_N];   /* v1ce "sound" file */
static float g_ent_low[ENT_MAX_DEFS];   /* v1bh low_Y: sink depth below home */
static float g_ent_spd[ENT_MAX_DEFS];   /* v1bh speed_Y: full-travel seconds */
static float g_ent_prox[ENT_MAX_DEFS];  /* v1bh proximity: trigger radius */
static float g_ent_sx[ENT_MAX_DEFS];    /* v1bj proximity_scaleX (def 1) */
static float g_ent_sy[ENT_MAX_DEFS];    /* v1bj proximity_scaleY (def 1) */
static float g_ent_sz[ENT_MAX_DEFS];    /* v1bj proximity_scaleZ (def 1) */
static float g_ent_sndsens[ENT_MAX_DEFS]; /* v1cd sound_sensitivity (def 1) */
static float g_ent_speedx[ENT_MAX_DEFS];  /* v1dq speed_X: sec/leg (def 2) */
static float g_ent_movedelay[ENT_MAX_DEFS]; /* v1dp moveX_delay (def 0s) */
static float g_ent_highx[ENT_MAX_DEFS];   /* v1dp high_X (def 100u) */
static float g_ent_push[ENT_MAX_DEFS];    /* v1j push: kick strength (def 3) */
static float g_ent_pushr[ENT_MAX_DEFS];   /* v1j push_radius: touch XZ (def 70) */
static int  g_ent_area[ENT_MAX_DEFS];   /* v1bz Woodbridge_area gate def */
static int  g_ent_vis[ENT_MAX_DEFS];    /* v1ca visible (def 1) */
static int  g_ent_count = 0;
static int  g_bub_on = 1;               /* v1da death_bubbles (def true) */
static char g_bub_tex[56] = "Tar";      /* v1da death_bubbles_tex (def+fb Tar) */
static char g_bub_sfx[56] = "bubble";   /* v1da death_bubbles_sfx (def bubble) */
static int  g_buba_on = 1;              /* v1de bubbles (def true) */
static char g_buba_tex[56] = "TarBlot"; /* v1de bubbles_tex (def+fb TarBlot) */
static char g_buba_sfx[56] = "bubble";  /* v1de bubbles_sfx (def bubble) */

/* strip E:/REF: prefix (ci) for prefix-blind behaviour compare (v1b) */
static const char* ent_strip_prefix(const char* s) {
    if (!s) return s;
    if ((s[0] == 'E' || s[0] == 'e') && s[1] == ':') return s + 2;
    if ((s[0] == 'R' || s[0] == 'r') && (s[1] == 'E' || s[1] == 'e') &&
        (s[2] == 'F' || s[2] == 'f') && s[3] == ':') return s + 4;
    return s;
}

/* ci substring (self-contained) */
static int ent_ci_substr(const char* hay, const char* needle) {
    int hl = 0, nl = 0, i, j;
    if (!hay || !needle || !needle[0]) return 0;
    while (hay[hl]) hl++;
    while (needle[nl]) nl++;
    if (nl > hl) return 0;
    for (i = 0; i <= hl - nl; i++) {
        for (j = 0; j < nl; j++) {
            char a = hay[i + j], b = needle[j];
            if (a >= 'A' && a <= 'Z') a += 32;
            if (b >= 'A' && b <= 'Z') b += 32;
            if (a != b) break;
        }
        if (j == nl) return 1;
    }
    return 0;
}

/* v1f def-name match with numeric wildcard. A trailing digit run in the
 * def (MouseA0, Woodbridge00) matches any digit at that spot in S1
 * (A1..A9, 01..99, literal incl). Digit-free defs = plain ci substring. */
static int ent_match_name(const char* s1name, const char* defname) {
    int dl = 0, n = 0, sl = 0, hl = 0, i, j;
    if (!s1name || !defname || !defname[0]) return 0;
    while (defname[dl]) dl++;
    while (dl - n > 0 && defname[dl - n - 1] >= '0' &&
           defname[dl - n - 1] <= '9') n++;
    if (n == 0 || n >= dl) return ent_ci_substr(s1name, defname);
    sl = dl - n;
    while (s1name[hl]) hl++;
    if (sl > hl) return 0;
    for (i = 0; i <= hl - sl; i++) {
        for (j = 0; j < sl; j++) {
            char a = s1name[i + j], b = defname[j];
            if (a >= 'A' && a <= 'Z') a += 32;
            if (b >= 'A' && b <= 'Z') b += 32;
            if (a != b) break;
        }
        if (j == sl) {
            char c = s1name[i + sl];
            if (c >= '0' && c <= '9') return 1;
        }
    }
    return 0;
}

/* behaviour ci-equals "woodbridge_area" (prefix-blind, v1b) */
static int ent_beh_is_area(const char* beh) {
    const char* w = "woodbridge_area";
    int i = 0;
    if (!beh || !beh[0]) return 0;
    beh = ent_strip_prefix(beh);
    while (w[i] && beh[i]) {
        char c = beh[i];
        if (c >= 'A' && c <= 'Z') c += 32;
        if (c != w[i]) return 0;
        i++;
    }
    return (w[i] == '\0' && beh[i] == '\0') ? 1 : 0;
}

/* v1e: 1 = block holds a quoted "behaviour"/"behavior" key (entity driver).
 * E: keys route on content: with behaviour -> entity parse, without -> ev_. */
static int ent_block_has_beh(const char* b, const char* bend) {
    static const char* keys[2] = { "\"behaviour\"", "\"behavior\"" };
    int ki;
    for (ki = 0; ki < 2; ki++) {
        const char* k = keys[ki];
        int kl = 0;
        const char* p;
        while (k[kl]) kl++;
        p = b;
        while (p + kl <= bend) {
            if (*p == '"' && memcmp(p, k, (unsigned)kl) == 0) {
                const char* v = p + kl;
                while (v < bend && (*v == ' ' || *v == 9)) v++;
                if (v < bend && *v == ':') return 1;
            }
            p++;
        }
    }
    return 0;
}

static int ent_key_known(const char* key, const char* bs, const char* be) {
    char low[64];
    int i = 0;
    while (i < 63 && key[i]) {
        char c = key[i];
        low[i] = (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
        i++;
    }
    low[i] = '\0';
    /* v1e: E: keys route on block content -- "behaviour" present means
     * entity-driver (event quads/planes), else sound event for ev_. The
     * E:/REF: prefix itself is only a label; matching uses stripped names. */
    if ((key[0] == 'E' || key[0] == 'e') && key[1] == ':') {
        return ent_block_has_beh(bs, be) ? 0 : 1;
    }
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

/* Extract "subkey": true/false/number inside a {..} block (v1ca visible).
 * Exact key match (quoted), like ent_block_float. true=>1, false=>0,
 * number=>!=0. Returns 1 + value. */
static int ent_block_bool(const char* b, const char* bend,
                          const char* sub, int* out) {
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
                while (v < bend && (*v == ' ' || *v == 9 || *v == 10 ||
                       *v == 13 || *v == ':' || *v == ',')) v++;
                if (v < bend && *v == 't' && v + 4 <= bend &&
                    v[1] == 'r' && v[2] == 'u' && v[3] == 'e') {
                    *out = 1;
                    return 1;
                }
                if (v < bend && *v == 'f' && v + 5 <= bend &&
                    v[1] == 'a' && v[2] == 'l' && v[3] == 's' &&
                    v[4] == 'e') {
                    *out = 0;
                    return 1;
                }
                if (v < bend && ((*v >= '0' && *v <= '9') ||
                    *v == '-' || *v == '+')) {
                    long ip = 0;
                    int neg = 0, any = 0;
                    if (*v == '-' || *v == '+') {
                        neg = (*v == '-');
                        v++;
                    }
                    while (v < bend && *v >= '0' && *v <= '9') {
                        ip = ip * 10 + (*v - '0');
                        v++;
                        any = 1;
                    }
                    if (!any) return 0;
                    *out = ((neg ? -ip : ip) != 0) ? 1 : 0;
                    return 1;
                }
                return 0;
            }
            p = ke;
        }
        p++;
    }
    return 0;
}

/* Extract "subkey": number inside a {..} block. Returns 1 + value.
 * Plain decimal only (no exponent). Char codes numeric (9 tab 10 LF
 * 13 CR 32 space) to stay patch-tool safe. Non-positive use is up
 * to the caller (Woodbridge ignores <= 0 and keeps defaults). */
static int ent_block_float(const char* b, const char* bend,
                           const char* sub, float* out) {
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
                int neg = 0;
                long ip = 0;
                float frac = 0.0f;
                float div = 1.0f;
                while (v < bend && (*v == ' ' || *v == 9 || *v == 10 ||
                       *v == 13 || *v == ':' || *v == ',')) v++;
                if (v < bend && (*v == '-' || *v == '+')) {
                    neg = (*v == '-');
                    v++;
                }
                if (v >= bend || !(*v >= '0' && *v <= '9')) return 0;
                while (v < bend && *v >= '0' && *v <= '9') {
                    ip = ip * 10 + (*v - '0');
                    v++;
                }
                if (v < bend && *v == '.') {
                    v++;
                    while (v < bend && *v >= '0' && *v <= '9') {
                        frac = frac * 10.0f + (float)(*v - '0');
                        div *= 10.0f;
                        v++;
                    }
                }
                *out = (neg ? -1.0f : 1.0f) * ((float)ip + frac / div);
                return 1;
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
            if (!ent_key_known(key, bs, be)) {
                char beh[ENT_BEH_N];
                char mesh[ENT_MESH_N];
                char snd[ENT_SND_N];
                beh[0] = '\0';
                mesh[0] = '\0';
                snd[0] = '\0';
                ent_block_str(bs, be, "behaviour", beh, sizeof(beh));
                ent_block_str(bs, be, "behavior", beh, sizeof(beh));
                ent_block_str(bs, be, "mesh", mesh, sizeof(mesh));
                ent_block_str(bs, be, "sound", snd, sizeof(snd));
                /* v1e: store the prefix-STRIPPED name (E:/REF: is only a
                 * routing label). Empty after strip = label-only key, skip. */
                if (mesh[0] && ent_strip_prefix(key)[0]) {
                    int ni = 0;
                    float fv = 0.0f;
                    int iv = 1;
                    const char* kn = ent_strip_prefix(key);
                    while (ni < ENT_NAME_N - 1 && kn[ni]) {
                        g_ent_name[g_ent_count][ni] = kn[ni];
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
                    ni = 0;   /* v1ce: sound file (may stay empty) */
                    while (ni < ENT_SND_N - 1 && snd[ni]) {
                        g_ent_snd[g_ent_count][ni] = snd[ni];
                        ni++;
                    }
                    g_ent_snd[g_ent_count][ni] = '\0';
                    g_ent_low[g_ent_count] = 50.0f;
                    g_ent_spd[g_ent_count] = 0.5f;
                    g_ent_prox[g_ent_count] = 150.0f;
                    g_ent_sx[g_ent_count] = 1.0f;
                    g_ent_sy[g_ent_count] = 1.0f;
                    g_ent_sz[g_ent_count] = 1.0f;
                    g_ent_sndsens[g_ent_count] = 1.0f;
                    g_ent_speedx[g_ent_count] = 2.0f;
                    g_ent_movedelay[g_ent_count] = 0.0f;
                    g_ent_highx[g_ent_count] = 100.0f;
                    if (ent_block_float(bs, be, "low_Y", &fv) && fv > 0.0f)
                        g_ent_low[g_ent_count] = fv;
                    if (ent_block_float(bs, be, "speed_Y", &fv) && fv > 0.0f)
                        g_ent_spd[g_ent_count] = fv;
                    if (ent_block_float(bs, be, "proximity", &fv) && fv > 0.0f)
                        g_ent_prox[g_ent_count] = fv;
                    if (ent_block_float(bs, be, "proximity_scaleX", &fv) && fv > 0.0f)
                        g_ent_sx[g_ent_count] = fv;
                    if (ent_block_float(bs, be, "proximity_scaleY", &fv) && fv > 0.0f)
                        g_ent_sy[g_ent_count] = fv;
                    if (ent_block_float(bs, be, "proximity_scaleZ", &fv) && fv > 0.0f)
                        g_ent_sz[g_ent_count] = fv;
                    if (ent_block_float(bs, be, "sound_sensitivity", &fv) && fv > 0.0f)
                        g_ent_sndsens[g_ent_count] = fv;
                    if (ent_block_float(bs, be, "speed_X", &fv) && fv > 0.0f)
                        g_ent_speedx[g_ent_count] = fv;
                    if (ent_block_float(bs, be, "moveX_delay", &fv) && fv >= 0.0f)
                        g_ent_movedelay[g_ent_count] = fv;
                    if (ent_block_float(bs, be, "high_X", &fv) && fv > 0.0f)
                        g_ent_highx[g_ent_count] = fv;
                    g_ent_push[g_ent_count] = 3.0f;
                    if (ent_block_float(bs, be, "push", &fv) && fv > 0.0f)
                        g_ent_push[g_ent_count] = fv;
                    g_ent_pushr[g_ent_count] = 70.0f;
                    if (ent_block_float(bs, be, "push_radius", &fv) && fv > 0.0f)
                        g_ent_pushr[g_ent_count] = fv;
                    g_ent_area[g_ent_count] =
                        ent_beh_is_area(beh) ? 1 : 0;
                    g_ent_vis[g_ent_count] = 1;
                    if (ent_block_bool(bs, be, "visible", &iv) && !iv)
                        g_ent_vis[g_ent_count] = 0;
                    g_ent_count++;
                }
            }
            p = be + 1;
            continue;
        }
        p++;
    }
    /* v1da: top-level death-bubble keys (whole-file scan; def blocks
     * never use these names). Missing = keep old (re-read safe). */
    {
        const char* base = (const char*)txt;
        int iv = 0;
        char sb[56];
        int si;
        if (ent_block_bool(base, end, "death_bubbles", &iv))
            g_bub_on = iv;
        if (ent_block_str(base, end, "death_bubbles_tex", sb, sizeof(sb)) &&
            sb[0]) {
            for (si = 0; si < 55 && sb[si]; si++) g_bub_tex[si] = sb[si];
            g_bub_tex[si] = 0;
        }
        if (ent_block_str(base, end, "death_bubbles_sfx", sb, sizeof(sb)) &&
            sb[0]) {
            for (si = 0; si < 55 && sb[si]; si++) g_bub_sfx[si] = sb[si];
            g_bub_sfx[si] = 0;
        }
        {
            char cbuf[160];
            snprintf(cbuf, sizeof(cbuf), "  BUBCFG: on=%d tex=%s sfx=%s",
                     g_bub_on, g_bub_tex, g_bub_sfx);
            log_mod(cbuf);
        }
        /* v1de: ambient bubble keys (exact-match, no clash with
         * death_bubbles*). Missing = keep old (re-read safe). */
        if (ent_block_bool(base, end, "bubbles", &iv))
            g_buba_on = iv;
        if (ent_block_str(base, end, "bubbles_tex", sb, sizeof(sb)) &&
            sb[0]) {
            for (si = 0; si < 55 && sb[si]; si++) g_buba_tex[si] = sb[si];
            g_buba_tex[si] = 0;
        }
        if (ent_block_str(base, end, "bubbles_sfx", sb, sizeof(sb)) &&
            sb[0]) {
            for (si = 0; si < 55 && sb[si]; si++) g_buba_sfx[si] = sb[si];
            g_buba_sfx[si] = 0;
        }
        {
            char cbuf[160];
            snprintf(cbuf, sizeof(cbuf), "  BUBACFG: on=%d tex=%s sfx=%s",
                     g_buba_on, g_buba_tex, g_buba_sfx);
            log_mod(cbuf);
        }
    }
    free(txt);
    {
        char ebuf[64];
        snprintf(ebuf, sizeof(ebuf), "  ENT set: %d def(s)", g_ent_count);
        log_mod(ebuf);
        for (i = 0; (int)i < g_ent_count; i++) {
            char dbuf[160];
            snprintf(dbuf, sizeof(dbuf), "  ENT def %s low=%f.1  spd=%f.1  prox=%f.1",
                     g_ent_name[i], g_ent_low[i], g_ent_spd[i], g_ent_prox[i]);
            log_mod(dbuf);
            snprintf(dbuf, sizeof(dbuf), "  ENT def %s scX=%f.1  scY=%f.1  scZ=%f.1",
                     g_ent_name[i], g_ent_sx[i], g_ent_sy[i], g_ent_sz[i]);
            log_mod(dbuf);
            snprintf(dbuf, sizeof(dbuf), "  ENT def %s spX=%f.1 dly=%f.1 hiX=%f.1",
                     g_ent_name[i], g_ent_speedx[i], g_ent_movedelay[i],
                     g_ent_highx[i]);
            log_mod(dbuf);
            if (g_ent_sndsens[i] != 1.0f) {
                snprintf(dbuf, sizeof(dbuf), "  ENT def %s sndsens=%f.2",
                         g_ent_name[i], g_ent_sndsens[i]);
                log_mod(dbuf);
            }
            if (g_ent_snd[i][0]) {
                snprintf(dbuf, sizeof(dbuf), "  ENT def %s sound=%s",
                         g_ent_name[i], g_ent_snd[i]);
                log_mod(dbuf);
            }
            if (g_ent_area[i]) {
                snprintf(dbuf, sizeof(dbuf),
                         "  ENT def %s: AREA mode (mesh=vertices)",
                         g_ent_name[i]);
                log_mod(dbuf);
            }
            if (!g_ent_vis[i]) {
                snprintf(dbuf, sizeof(dbuf),
                         "  ENT def %s: visible=false", g_ent_name[i]);
                log_mod(dbuf);
            }
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
