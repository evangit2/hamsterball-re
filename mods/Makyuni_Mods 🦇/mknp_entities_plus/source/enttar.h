/* enttar.h -- Tarpit/E:Cheesepit plane-cover zones (v1cm, renamed v1b).
 * Problem: the Tarpit entry test was an r30 sphere around the S1 point, but
 * the visible E:<Name> plane is bigger, so parts of the cheese never sank
 * the ball. Fix: load the S6 quad(s) whose geom name contains the Cheesepit def
 * name out of the LEVEL file (same walk as E:Woodbridge_area in entarea.h) and
 * test ball XZ against the real plane shape (+-ball-radius samples so the rim
 * grabs on visual touch). Y gate stays home-based in the caller (v1cl).
 * No quads found => caller falls back to the r30 sphere (old levels safe).
 * Own tables (never touches the AREA gate tables). Include AFTER entarea.h
 * (reuses area_name_hit) and gridmesh.h (gm_*). Needs: log_mod, snprintf,
 * memcpy/free.
 */
#ifndef ENTTAR_H
#define ENTTAR_H

#define TAR_MAXQ 8
#define TAR_MAXV 8

static float g_tar_x[TAR_MAXQ][TAR_MAXV];
static float g_tar_y[TAR_MAXQ][TAR_MAXV];
static float g_tar_z[TAR_MAXQ][TAR_MAXV];
static int   g_tar_nv[TAR_MAXQ];
static int   g_tar_count = 0;
static char  g_tar_tag[TAR_MAXQ][40];
/* v1co: every S6 geom stored (cap), cover binds nearest-to-S1, not by name */
static float g_tar_minx[TAR_MAXQ];
static float g_tar_maxx[TAR_MAXQ];
static float g_tar_minz[TAR_MAXQ];
static float g_tar_maxz[TAR_MAXQ];
static int   g_tar_namehit[TAR_MAXQ];
static int   g_tar_pick = -1;   /* cover quad index, -1 = sphere fallback */

static void tar_reset(void) {
    int i;
    g_tar_count = 0;
    g_tar_pick = -1;
    for (i = 0; i < TAR_MAXQ; i++) g_tar_nv[i] = 0;
}

static void tar_push(int a, float x, float y, float z) {
    int i, n = g_tar_nv[a];
    for (i = 0; i < n; i++) {
        float dx = g_tar_x[a][i] - x, dz = g_tar_z[a][i] - z;
        float dy = g_tar_y[a][i] - y;
        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;
        if (dz < 0) dz = -dz;
        if (dx < 0.0001f && dy < 0.0001f && dz < 0.0001f) return;
    }
    if (n >= TAR_MAXV) return;
    g_tar_x[a][n] = x;
    g_tar_y[a][n] = y;
    g_tar_z[a][n] = z;
    g_tar_nv[a] = n + 1;
    if (n == 0) {
        g_tar_minx[a] = x; g_tar_maxx[a] = x;
        g_tar_minz[a] = z; g_tar_maxz[a] = z;
    } else {
        if (x < g_tar_minx[a]) g_tar_minx[a] = x;
        if (x > g_tar_maxx[a]) g_tar_maxx[a] = x;
        if (z < g_tar_minz[a]) g_tar_minz[a] = z;
        if (z > g_tar_maxz[a]) g_tar_maxz[a] = z;
    }
}

/* S6 walk storing every geom (cap TAR_MAXQ), flagging name hits (mirrors
 * area_walk in entarea.h, writes the tar tables instead). */
static int tar_walk(GmCur* c, const unsigned char* vbuf, int nverts,
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
            if (g_tar_count < TAR_MAXQ) {
                int a = g_tar_count;
                int k2 = 0;
                g_tar_nv[a] = 0;
                g_tar_namehit[a] = area_name_hit(nm, ln, want);
                while (k2 < 39 && k2 < ln && nm[k2]) {
                    g_tar_tag[a][k2] = (char)nm[k2];
                    k2++;
                }
                g_tar_tag[a][k2] = '\0';
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
                        tar_push(a, x, y, z);
                    }
                }
                if (g_tar_nv[a] >= 3) {
                    g_tar_count++;
                } else {
                    char sbuf[96];
                    snprintf(sbuf, sizeof(sbuf),
                             "  TARQ: geom %s: only %d vert(s), skipped",
                             g_tar_tag[a], g_tar_nv[a]);
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
            int r = tar_walk(c, vbuf, nverts, want, nodes);
            if (r != 1) return r;
        }
    }
    return 1;
}

static int tar_half(float x, float z) {
    return (z > 0.0f || (z == 0.0f && x > 0.0f)) ? 0 : 1;
}

/* CCW-order verts around the centroid (quadrant + cross sort, no libm) */
static void tar_sort_ccw(int a) {
    int n = g_tar_nv[a], i, j, pass;
    float cx = 0.0f, cz = 0.0f;
    for (i = 0; i < n; i++) {
        cx += g_tar_x[a][i];
        cz += g_tar_z[a][i];
    }
    cx /= (float)n;
    cz /= (float)n;
    for (pass = 0; pass < n; pass++) {
        for (i = 0; i < n - 1; i++) {
            float ax = g_tar_x[a][i] - cx, az = g_tar_z[a][i] - cz;
            float bx = g_tar_x[a][i + 1] - cx, bz = g_tar_z[a][i + 1] - cz;
            int ha = tar_half(ax, az), hb = tar_half(bx, bz);
            int swap = 0;
            if (ha != hb) swap = (ha > hb);
            else if (ax * bz - az * bx < 0.0f) swap = 1;
            if (swap) {
                float t;
                t = g_tar_x[a][i]; g_tar_x[a][i] = g_tar_x[a][i + 1];
                g_tar_x[a][i + 1] = t;
                t = g_tar_y[a][i]; g_tar_y[a][i] = g_tar_y[a][i + 1];
                g_tar_y[a][i + 1] = t;
                t = g_tar_z[a][i]; g_tar_z[a][i] = g_tar_z[a][i + 1];
                g_tar_z[a][i + 1] = t;
            }
        }
    }
}

/* shoelace XZ area (signed, ~2x triangle units) */
static float tar_size2(int a) {
    int n = g_tar_nv[a], i;
    float s = 0.0f;
    for (i = 0; i < n; i++) {
        int j = (i + 1) % n;
        s += g_tar_x[a][i] * g_tar_z[a][j] -
             g_tar_x[a][j] * g_tar_z[a][i];
    }
    if (s < 0) s = -s;
    return s * 0.5f;
}

static int tar_inside(int a, float px, float pz) {
    int n = g_tar_nv[a], i, pos = 0, neg = 0;
    for (i = 0; i < n; i++) {
        int j = (i + 1) % n;
        float cr = (g_tar_x[a][j] - g_tar_x[a][i]) * (pz - g_tar_z[a][i]) -
                   (g_tar_z[a][j] - g_tar_z[a][i]) * (px - g_tar_x[a][i]);
        if (cr > 0.001f) pos = 1;
        else if (cr < -0.001f) neg = 1;
        if (pos && neg) return 0;
    }
    return 1;
}

/* level-start pass: S6 quads, cover binds to the def's plane. Pick order:
 * name hit > quad containing S1 home XZ > nearest centroid (<=200u).
 * GOAL geoms never picked. hx/hz = first instance home. Logs ints only. */
static void tar_scan_file(const char* levelpath, const char* defname,
                          float hx, float hz) {
    unsigned len = 0;
    unsigned char* d;
    GmCur c;
    const unsigned char* vbuf = 0;
    int nverts = 0, nodes = 0, q;
    char qbuf[96];
    if (!levelpath || !levelpath[0] || !defname || !defname[0]) return;
    d = gm_read_file(levelpath, &len);
    if (!d || len < 32) {
        log_mod("  TARQ: cannot read level file");
        if (d) free(d);
        return;
    }
    c.p = d;
    c.end = d + len;
    if (!gm_skip_to_s6(&c, &vbuf, &nverts) || !vbuf || nverts <= 0) {
        log_mod("  TARQ: level S1..S5 prefix unreadable");
        free(d);
        return;
    }
    tar_walk(&c, vbuf, nverts, defname, &nodes);
    free(d);
    for (q = 0; q < g_tar_count; q++) {
        tar_sort_ccw(q);
        if (tar_size2(q) < 0.5f) {
            snprintf(qbuf, sizeof(qbuf),
                     "  TARQ: quad %s degenerate, dropped",
                     g_tar_tag[q]);
            log_mod(qbuf);
            g_tar_nv[q] = 0;
            continue;
        }
        snprintf(qbuf, sizeof(qbuf), "  TARQ: quad %s: %d verts%s",
                 g_tar_tag[q], g_tar_nv[q],
                 g_tar_namehit[q] ? " (name)" : "");
        log_mod(qbuf);
    }
    /* compact out dropped quads */
    {
        int w = 0, r, v;
        for (r = 0; r < g_tar_count; r++) {
            if (!g_tar_nv[r]) continue;
            if (w != r) {
                for (v = 0; v < g_tar_nv[r]; v++) {
                    g_tar_x[w][v] = g_tar_x[r][v];
                    g_tar_y[w][v] = g_tar_y[r][v];
                    g_tar_z[w][v] = g_tar_z[r][v];
                }
                g_tar_nv[w] = g_tar_nv[r];
                g_tar_minx[w] = g_tar_minx[r];
                g_tar_maxx[w] = g_tar_maxx[r];
                g_tar_minz[w] = g_tar_minz[r];
                g_tar_maxz[w] = g_tar_maxz[r];
                g_tar_namehit[w] = g_tar_namehit[r];
                for (v = 0; v < 40; v++) {
                    g_tar_tag[w][v] = g_tar_tag[r][v];
                    if (!g_tar_tag[r][v]) break;
                }
            }
            w++;
        }
        g_tar_count = w;
    }
    /* pick the cover quad */
    {
        int qi, best = -1;
        float bestd = 40000.0f;   /* 200u */
        const char* why = "";
        for (qi = 0; qi < g_tar_count; qi++) {
            if (area_ci_contains(g_tar_tag[qi], "goal")) continue;
            if (g_tar_namehit[qi]) { best = qi; why = "named"; break; }
        }
        if (best < 0) {
            for (qi = 0; qi < g_tar_count; qi++) {
                if (area_ci_contains(g_tar_tag[qi], "goal")) continue;
                if (hx >= g_tar_minx[qi] && hx <= g_tar_maxx[qi] &&
                    hz >= g_tar_minz[qi] && hz <= g_tar_maxz[qi]) {
                    best = qi; why = "home-inside";
                    break;
                }
            }
        }
        if (best < 0) {
            for (qi = 0; qi < g_tar_count; qi++) {
                int v, n = g_tar_nv[qi];
                float cx = 0.0f, cz = 0.0f, dx, dz, dd;
                if (area_ci_contains(g_tar_tag[qi], "goal")) continue;
                if (n <= 0) continue;
                for (v = 0; v < n; v++) {
                    cx += g_tar_x[qi][v];
                    cz += g_tar_z[qi][v];
                }
                cx /= (float)n;
                cz /= (float)n;
                dx = cx - hx;
                dz = cz - hz;
                dd = dx * dx + dz * dz;
                if (dd < bestd) { bestd = dd; best = qi; why = "nearest"; }
            }
        }
        g_tar_pick = best;
        if (best >= 0) {
            snprintf(qbuf, sizeof(qbuf), "  TARQ: cover=%s (%s)",
                     g_tar_tag[best], why);
        } else {
            snprintf(qbuf, sizeof(qbuf),
                     "  TARQ: no cover geom, sphere fallback");
        }
        log_mod(qbuf);
    }
}

/* 1 = cover quad picked (tar_cover valid), 0 = caller uses S1 sphere */
static int tar_armed(void) {
    return (g_tar_pick >= 0 && g_tar_pick < g_tar_count &&
            g_tar_nv[g_tar_pick] >= 3);
}

/* 1 = ball XZ touches the picked cover quad. Samples center +-r so the
 * rim grabs on visual touch (r = ball radius). Pick < 0 => 0 (caller
 * falls back to the S1 sphere). */
static int tar_cover(float px, float pz, float r) {
    int q;
    if (!tar_armed()) return 0;
    q = g_tar_pick;
    if (r < 0.0f) r = 0.0f;
    if (r > 60.0f) r = 60.0f;
    if (tar_inside(q, px, pz)) return 1;
    if (r > 0.0f) {
        if (tar_inside(q, px + r, pz)) return 1;
        if (tar_inside(q, px - r, pz)) return 1;
        if (tar_inside(q, px, pz + r)) return 1;
        if (tar_inside(q, px, pz - r)) return 1;
    }
    return 0;
}

/* v1cn diag: list S6 geom names so a name mismatch shows in the log.
 * Same walk skeleton, no quad store. Caps at 12 lines. */
static int tar_list_walk(GmCur* c, int* shown, int* total) {
    int sub, mbc, i;
    if (++(*total) > GM_MAX_NODES + 100000) return 0;
    if (!gm_need(c, 28)) return 0;
    c->p += 24;
    sub = gm_i32(c);
    if (sub < 1) {
        if (!gm_need(c, 4)) return 0;
        mbc = gm_i32(c);
        if (mbc < 0 || mbc > 100000) return 0;
        for (i = 0; i < mbc; i++) {
            int ln, sc;
            const unsigned char* nm;
            char nb[40];
            int k = 0;
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
            c->p += (unsigned)sc * 8u;
            if (*shown < 12) {
                while (k < 39 && k < ln && nm[k]) {
                    nb[k] = (char)nm[k];
                    k++;
                }
                nb[k] = '\0';
                {
                    char lb[64];
                    snprintf(lb, sizeof(lb), "  TARQ: S6[%d]=%s", *shown, nb);
                    log_mod(lb);
                }
                (*shown)++;
            }
        }
        return 1;
    }
    {
        int k;
        if (sub > 100000) return 0;
        for (k = 0; k < sub; k++) {
            int r = tar_list_walk(c, shown, total);
            if (r != 1) return r;
        }
    }
    return 1;
}

static void tar_list_geoms(const char* levelpath) {
    unsigned len = 0;
    unsigned char* d;
    GmCur c;
    const unsigned char* vbuf = 0;
    int nverts = 0, shown = 0, total = 0;
    if (!levelpath || !levelpath[0]) return;
    d = gm_read_file(levelpath, &len);
    if (!d || len < 32) {
        log_mod("  TARQ: cannot read level file");
        if (d) free(d);
        return;
    }
    c.p = d;
    c.end = d + len;
    if (!gm_skip_to_s6(&c, &vbuf, &nverts) || !vbuf || nverts <= 0) {
        log_mod("  TARQ: level S1..S5 prefix unreadable");
        free(d);
        return;
    }
    tar_list_walk(&c, &shown, &total);
    free(d);
    {
        char lb[64];
        snprintf(lb, sizeof(lb), "  TARQ: listed %d S6 geom(s)", shown);
        log_mod(lb);
    }
}

#endif /* ENTTAR_H */
