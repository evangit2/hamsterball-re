/* entarea.h -- E:Woodbridge_area gate zones (v1bz, renamed v1b).
 * jsonc (verbatim MAKYUNI schema):
 *     "E:Woodbridge_area", { "behaviour": "E:Woodbridge_area", "mesh": "vertices" }
 * mesh "vertices" = load the quad from the VERTICES of the S6 geom(s) whose
 * name contains the def name (E:Woodbridge_area01/02/...) in the LEVEL file
 * (S1 refs are single points and cannot hold verts; the S1 E: is only an
 * editor marker). S5 verts are world-space (GRID recentering proves it).
 * The area object is never rendered and spawns nothing: quads live only in
 * the tables below. Gate: ball inside ANY quad (XZ column, Y ignored) =>
 * Woodbridge proximity runs normally; outside => all Woodbridges act as if
 * the ball were absent (rise home). No quads found => gate OPEN (behaviour
 * unchanged, old levels keep working).
 * Needs from includer: log_mod(), snprintf, gm_* (gridmesh.h -- include
 * AFTER gridmesh.h), malloc/free/memcpy.
 */
#ifndef ENTAREA_H
#define ENTAREA_H

#define AREA_MAX 8
#define AREA_MAXV 8

static float g_area_x[AREA_MAX][AREA_MAXV];
static float g_area_y[AREA_MAX][AREA_MAXV];
static float g_area_z[AREA_MAX][AREA_MAXV];
static int   g_area_nv[AREA_MAX];
static int   g_area_count = 0;
static int   g_area_refs_seen = 0;
static char  g_area_tag[AREA_MAX][40];
static int   g_area_gate_last = 1;   /* edge-log state (1 = open) */

static void area_reset(void) {
    int i;
    g_area_count = 0;
    g_area_refs_seen = 0;
    g_area_gate_last = 1;
    for (i = 0; i < AREA_MAX; i++) g_area_nv[i] = 0;
}

/* case-insensitive substring (self-contained: no includer deps) */
static int area_ci_contains(const char* hay, const char* needle) {
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

/* geom-name hit: S6 name (maybe NUL-padded, len ln) contains def name.
 * v1f numeric wildcard: trailing digits in want (area0) match any digit
 * run in the geom name (area01..area99, literal incl). */
static int area_name_hit(const unsigned char* nm, int ln, const char* want) {
    char tmp[128];
    int i = 0, wl = 0, n = 0, hl = 0, sl = 0, a = 0, b = 0;
    if (!nm || ln < 1 || !want || !want[0]) return 0;
    while (i < ln && i < 127 && nm[i]) { tmp[i] = (char)nm[i]; i++; }
    tmp[i] = '\0';
    while (want[wl]) wl++;
    while (wl - n > 0 && want[wl - n - 1] >= '0' && want[wl - n - 1] <= '9') n++;
    if (n == 0 || n >= wl) return area_ci_contains(tmp, want);
    sl = wl - n;
    while (tmp[hl]) hl++;
    if (sl > hl) return 0;
    for (a = 0; a <= hl - sl; a++) {
        for (b = 0; b < sl; b++) {
            char ca = tmp[a + b], cb = want[b];
            if (ca >= 'A' && ca <= 'Z') ca += 32;
            if (cb >= 'A' && cb <= 'Z') cb += 32;
            if (ca != cb) break;
        }
        if (b == sl) {
            char c = tmp[a + sl];
            if (c >= '0' && c <= '9') return 1;
        }
    }
    return 0;
}

/* store one vertex into the open slot (dedupe ~1e-4, cap AREA_MAXV) */
static void area_push(int a, float x, float y, float z) {
    int i, n = g_area_nv[a];
    for (i = 0; i < n; i++) {
        float dx = g_area_x[a][i] - x, dz = g_area_z[a][i] - z;
        float dy = g_area_y[a][i] - y;
        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;
        if (dz < 0) dz = -dz;
        if (dx < 0.0001f && dy < 0.0001f && dz < 0.0001f) return;
    }
    if (n >= AREA_MAXV) return;
    g_area_x[a][n] = x;
    g_area_y[a][n] = y;
    g_area_z[a][n] = z;
    g_area_nv[a] = n + 1;
}

/* S6 walk collecting every geom whose name contains want. Mirrors the
 * gm_node leaf layout (name/mat/tex/sc*8B strips of tri-count + vref). */
static int area_walk(GmCur* c, const unsigned char* vbuf, int nverts,
                     const char* want, int* nodes) {
    int sub, mbc, i;
    if (++(*nodes) > GM_MAX_NODES) return 0;
    if (!gm_need(c, 28)) return 0;
    c->p += 24;
    sub = gm_i32(c);
    if (sub < 1) {
        if (!gm_need(c, 4)) return 0;
        mbc = gm_i32(c);
        if (mbc < 0 || mbc > 100000) return 0;
        for (i = 0; i < mbc; i++) {
            int ln, sc, s;
            const unsigned char* nm;
            const unsigned char* spp;
            if (!gm_need(c, 4)) return 0;
            ln = gm_i32(c);
            if (ln < 1 || ln > 1024 || !gm_need(c, (unsigned)ln)) return 0;
            nm = c->p;
            c->p += (unsigned)ln;
            if (!gm_need(c, 72 + 4)) return 0;
            c->p += 72;
            {
                unsigned ht = gm_u32(c);
                if (ht == 1) {
                    int tl;
                    if (!gm_need(c, 4)) return 0;
                    tl = gm_i32(c);
                    if (tl < 1 || tl > 1024 || !gm_need(c, (unsigned)tl))
                        return 0;
                    c->p += (unsigned)tl;
                }
            }
            if (!gm_need(c, 4)) return 0;
            sc = gm_i32(c);
            if (sc < 0 || sc > 1000000) return 0;
            if (!gm_need(c, (unsigned)sc * 8u)) return 0;
            spp = c->p;
            c->p += (unsigned)sc * 8u;
            if (g_area_count < AREA_MAX && area_name_hit(nm, ln, want)) {
                int a = g_area_count;
                int k2 = 0;
                g_area_nv[a] = 0;
                while (k2 < 39 && k2 < ln && nm[k2]) {
                    g_area_tag[a][k2] = (char)nm[k2];
                    k2++;
                }
                g_area_tag[a][k2] = '\0';
                for (s = 0; s < sc; s++) {
                    int tri, vr, k;
                    memcpy(&tri, spp + (unsigned)s * 8u, 4);
                    memcpy(&vr, spp + (unsigned)s * 8u + 4, 4);
                    if (tri < 0 || tri > 100000 || vr < 0 ||
                        vr + tri + 2 > nverts)
                        continue;
                    for (k = 0; k < tri + 2; k++) {
                        float x, y, z;
                        const unsigned char* vp =
                            vbuf + (unsigned)(vr + k) * 32u;
                        memcpy(&x, vp, 4);
                        memcpy(&y, vp + 4, 4);
                        memcpy(&z, vp + 8, 4);
                        if (x != x || y != y || z != z) continue;
                        if (x > 1000000.0f || x < -1000000.0f ||
                            y > 1000000.0f || y < -1000000.0f ||
                            z > 1000000.0f || z < -1000000.0f)
                            continue;
                        area_push(a, x, y, z);
                    }
                }
                if (g_area_nv[a] >= 3) {
                    g_area_count++;
                } else {
                    char sbuf[96];
                    snprintf(sbuf, sizeof(sbuf),
                             "  AREA: geom %s: only %d vert(s), skipped",
                             g_area_tag[a], g_area_nv[a]);
                    log_mod(sbuf);
                }
            }
        }
        return 1;
    }
    {
        int k;
        if (sub > 100000) return 0;
        for (k = 0; k < sub; k++) {
            int r = area_walk(c, vbuf, nverts, want, nodes);
            if (r != 1) return r;
        }
    }
    return 1;
}

static int area_half(float x, float z) {
    return (z > 0.0f || (z == 0.0f && x > 0.0f)) ? 0 : 1;
}

/* CCW-order verts around the centroid (quadrant + cross sort, no libm) */
static void area_sort_ccw(int a) {
    int n = g_area_nv[a], i, j, pass;
    float cx = 0.0f, cz = 0.0f;
    for (i = 0; i < n; i++) {
        cx += g_area_x[a][i];
        cz += g_area_z[a][i];
    }
    cx /= (float)n;
    cz /= (float)n;
    for (pass = 0; pass < n; pass++) {
        for (i = 0; i < n - 1; i++) {
            float ax = g_area_x[a][i] - cx, az = g_area_z[a][i] - cz;
            float bx = g_area_x[a][i + 1] - cx, bz = g_area_z[a][i + 1] - cz;
            int ha = area_half(ax, az), hb = area_half(bx, bz);
            int swap = 0;
            if (ha != hb) swap = (ha > hb);
            else if (ax * bz - az * bx < 0.0f) swap = 1;
            if (swap) {
                float t;
                t = g_area_x[a][i]; g_area_x[a][i] = g_area_x[a][i + 1];
                g_area_x[a][i + 1] = t;
                t = g_area_y[a][i]; g_area_y[a][i] = g_area_y[a][i + 1];
                g_area_y[a][i + 1] = t;
                t = g_area_z[a][i]; g_area_z[a][i] = g_area_z[a][i + 1];
                g_area_z[a][i + 1] = t;
            }
        }
    }
}

/* shoelace XZ area (signed, ~2x triangle units) */
static float area_size2(int a) {
    int n = g_area_nv[a], i;
    float s = 0.0f;
    for (i = 0; i < n; i++) {
        int j = (i + 1) % n;
        s += g_area_x[a][i] * g_area_z[a][j] -
             g_area_x[a][j] * g_area_z[a][i];
    }
    if (s < 0) s = -s;
    return s * 0.5f;
}

static int area_inside(int a, float px, float pz) {
    int n = g_area_nv[a], i, pos = 0, neg = 0;
    for (i = 0; i < n; i++) {
        int j = (i + 1) % n;
        float cr = (g_area_x[a][j] - g_area_x[a][i]) * (pz - g_area_z[a][i]) -
                   (g_area_z[a][j] - g_area_z[a][i]) * (px - g_area_x[a][i]);
        if (cr > 0.001f) pos = 1;
        else if (cr < -0.001f) neg = 1;
        if (pos && neg) return 0;
    }
    return 1;
}

/* level-start pass: one S6 walk per area def, quads from matching geoms.
 * vis=0 (jsonc visible:false) needs the E: name prefix for hiding: the
 * engine renders every plain geom itself, so warn when it is missing. */
static void area_scan_file(const char* levelpath, const char* defname,
                           int vis) {
    unsigned len = 0;
    unsigned char* d;
    GmCur c;
    const unsigned char* vbuf = 0;
    int nverts = 0, nodes = 0, q;
    if (!levelpath || !levelpath[0] || !defname || !defname[0]) return;
    d = gm_read_file(levelpath, &len);
    if (!d || len < 32) {
        log_mod("  AREA: cannot read level file");
        if (d) free(d);
        return;
    }
    c.p = d;
    c.end = d + len;
    if (!gm_skip_to_s6(&c, &vbuf, &nverts) || !vbuf || nverts <= 0) {
        log_mod("  AREA: level S1..S5 prefix unreadable");
        free(d);
        return;
    }
    area_walk(&c, vbuf, nverts, defname, &nodes);
    free(d);
    for (q = 0; q < g_area_count; q++) {
        char qbuf[256];
        int o = 0, v;
        area_sort_ccw(q);
        if (area_size2(q) < 0.5f) {
            snprintf(qbuf, sizeof(qbuf), "  AREA: quad %s degenerate, dropped",
                     g_area_tag[q]);
            log_mod(qbuf);
            g_area_nv[q] = 0;
            continue;
        }
        o = snprintf(qbuf, sizeof(qbuf), "  AREA: quad %s: %d verts",
                     g_area_tag[q], g_area_nv[q]);
        if (o < 0) o = 0;
        if (o >= (int)sizeof(qbuf)) o = (int)sizeof(qbuf) - 1;
        for (v = 0; v < g_area_nv[q] && o + 40 < (int)sizeof(qbuf); v++) {
            char vb[48];
            snprintf(vb, sizeof(vb), " (%f.1,%f.1,%f.1)",
                     g_area_x[q][v], g_area_y[q][v], g_area_z[q][v]);
            {
                int k = 0;
                while (vb[k] && o + 1 < (int)sizeof(qbuf)) qbuf[o++] = vb[k++];
            }
        }
        qbuf[o] = '\0';
        log_mod(qbuf);
        if (!vis) {
            char c0 = g_area_tag[q][0], c1 = g_area_tag[q][1];
            if (c0 >= 'A' && c0 <= 'Z') c0 += 32;
            if (!((c0 == 'e' && c1 == ':'))) {
                snprintf(qbuf, sizeof(qbuf),
                         "  AREA: WARN %s visible=false but no E: prefix; plane shows",
                         g_area_tag[q]);
                log_mod(qbuf);
            }
        }
    }
    /* compact out dropped quads */
    {
        int w = 0, r, v;
        for (r = 0; r < g_area_count; r++) {
            if (!g_area_nv[r]) continue;
            if (w != r) {
                for (v = 0; v < g_area_nv[r]; v++) {
                    g_area_x[w][v] = g_area_x[r][v];
                    g_area_y[w][v] = g_area_y[r][v];
                    g_area_z[w][v] = g_area_z[r][v];
                }
                g_area_nv[w] = g_area_nv[r];
                for (v = 0; v < 40; v++) {
                    g_area_tag[w][v] = g_area_tag[r][v];
                    if (!g_area_tag[r][v]) break;
                }
            }
            w++;
        }
        g_area_count = w;
    }
}

/* per-frame gate: 1 = Woodbridge proximity runs, 0 = ball counts absent */
static int area_gate(float bx, float bz, int has_ball) {
    int i, at = -1;
    char gb[96];
    if (!g_area_count) return 1;
    if (has_ball) {
        for (i = 0; i < g_area_count; i++) {
            if (area_inside(i, bx, bz)) { at = i; break; }
        }
    }
    if ((at >= 0) != g_area_gate_last) {
        g_area_gate_last = (at >= 0);
        if (at >= 0) {
            snprintf(gb, sizeof(gb), "  AREA: gate OPEN (inside %s)",
                     g_area_tag[at]);
        } else {
            snprintf(gb, sizeof(gb), "  AREA: gate CLOSED (ball outside)");
        }
        log_mod(gb);
    }
    return (at >= 0);
}

#endif /* ENTAREA_H */
