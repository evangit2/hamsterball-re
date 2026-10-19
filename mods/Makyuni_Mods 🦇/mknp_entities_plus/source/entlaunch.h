/* entlaunch.h -- E:Launch pads (v1dw, renamed v1b from e_Launch). Replicates native E:LAUNCH
 * (HandleArenaCollisionEvents 0x412850, Master/Level10): ball enters the
 * pad zone -> teleport to DEST, explode sound at FX, 1s cooldown.
 * Markers (S1, per pad instance, exact match, case-insens):
 *   E:<Name>       pad itself (spawns the visible solid mesh)
 *   E:<Name>_DEST  landing point (missing = pad stays idle + logs)
 *   E:<Name>_FX    flash/sound pos (missing = pad pos)
 * Needs: log_mod, snprintf, IsBadReadPtr, S1ENTRY_NAME/POS_X/POS_Y/POS_Z,
 * DWORD. Include with the other ent headers (after gridmesh.h).
 */
#ifndef ENTLAUNCH_H
#define ENTLAUNCH_H

/* case-insens equals (self-contained, no dep order) */
static int launch_streq(const char* a, const char* b) {
    int i = 0;
    if (!a || !b) return 0;
    while (a[i] && b[i]) {
        char ca = a[i], cb = b[i];
        if (ca >= 'A' && ca <= 'Z') ca += 32;
        if (cb >= 'A' && cb <= 'Z') cb += 32;
        if (ca != cb) return 0;
        i++;
    }
    return a[i] == b[i];
}

/* 1 = S1 marker ref (ends _DEST or _FX, ci): never spawns a pad. */
static int launch_is_marker(const char* nm) {
    int n = 0;
    if (!nm) return 0;
    while (nm[n]) n++;
    if (n > 5 && nm[n - 5] == '_' &&
        (nm[n - 4] == 'D' || nm[n - 4] == 'd') &&
        (nm[n - 3] == 'E' || nm[n - 3] == 'e') &&
        (nm[n - 2] == 'S' || nm[n - 2] == 's') &&
        (nm[n - 1] == 'T' || nm[n - 1] == 't'))
        return 1;
    if (n > 3 && nm[n - 3] == '_' &&
        (nm[n - 2] == 'F' || nm[n - 2] == 'f') &&
        (nm[n - 1] == 'X' || nm[n - 1] == 'x'))
        return 1;
    return 0;
}

/* Find the S1 ref named nm+suffix (exact ci), return its pos. 0 = none. */
static int launch_find_marker(DWORD* s1_data, int s1_count, const char* nm,
                              const char* suffix,
                              float* ox, float* oy, float* oz) {
    char want[96];
    int L = 0, i;
    if (!s1_data || !nm || !suffix || !ox || !oy || !oz) return 0;
    while (nm[L] && L < 80) { want[L] = nm[L]; L++; }
    i = 0;
    while (suffix[i] && L < 94) { want[L++] = suffix[i++]; }
    want[L] = 0;
    for (i = 0; i < s1_count; i++) {
        DWORD entry = s1_data[i];
        const char* enm = 0;
        char* ptr = 0;
        if (!entry || entry < 0x10000) continue;
        if (IsBadReadPtr((void*)entry, 16)) continue;
        ptr = *(char**)(entry + S1ENTRY_NAME);
        if (ptr && !IsBadReadPtr(ptr, 5)) enm = ptr;
        else enm = (const char*)entry;
        if (!enm || IsBadReadPtr((void*)enm, 8)) continue;
        if (!launch_streq(enm, want)) continue;
        *ox = *(float*)(entry + S1ENTRY_POS_X);
        *oy = *(float*)(entry + S1ENTRY_POS_Y);
        *oz = *(float*)(entry + S1ENTRY_POS_Z);
        return 1;
    }
    return 0;
}

/* ---- v1dx: S6 trigger planes (mesh=vertices). Per-def quad sets out of
 * the LEVEL file (same S6 walk as Tarpit in enttar.h, own tables, name-hit
 * geoms only). No quads => caller falls back to the S1 sphere. Needs
 * gridmesh (gm_*), entarea (area_name_hit, area_ci_contains), memcpy/free.
 * ---- */
#define LAU_MAXD 8
#define LAU_MAXQ 4
#define LAU_MAXV 8

static char  g_lau_def[LAU_MAXD][40];
static float g_lau_x[LAU_MAXD][LAU_MAXQ][LAU_MAXV];
static float g_lau_y[LAU_MAXD][LAU_MAXQ][LAU_MAXV];
static float g_lau_z[LAU_MAXD][LAU_MAXQ][LAU_MAXV];
static int   g_lau_nv[LAU_MAXD][LAU_MAXQ];
static int   g_lau_nq[LAU_MAXD];
static int   g_lau_nd = 0;

static void launch_reset(void) {
    int i, q;
    g_lau_nd = 0;
    for (i = 0; i < LAU_MAXD; i++) {
        g_lau_def[i][0] = 0;
        g_lau_nq[i] = 0;
        for (q = 0; q < LAU_MAXQ; q++) g_lau_nv[i][q] = 0;
    }
}

/* def slot, -1 = table full */
static int launch_slot(const char* defname) {
    int i;
    if (!defname || !defname[0]) return -1;
    for (i = 0; i < g_lau_nd; i++) {
        if (launch_streq(g_lau_def[i], defname)) return i;
    }
    if (g_lau_nd >= LAU_MAXD) return -1;
    i = g_lau_nd++;
    {
        int k = 0;
        while (k < 39 && defname[k]) {
            g_lau_def[i][k] = defname[k];
            k++;
        }
        g_lau_def[i][k] = 0;
    }
    g_lau_nq[i] = 0;
    return i;
}

static int lau_find_slot(const char* defname) {
    int i;
    if (!defname || !defname[0]) return -1;
    for (i = 0; i < g_lau_nd; i++) {
        if (launch_streq(g_lau_def[i], defname)) return i;
    }
    return -1;
}

static void lau_push(int dslot, int a, float x, float y, float z) {
    int i, n = g_lau_nv[dslot][a];
    for (i = 0; i < n; i++) {
        float dx = g_lau_x[dslot][a][i] - x, dz = g_lau_z[dslot][a][i] - z;
        float dy = g_lau_y[dslot][a][i] - y;
        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;
        if (dz < 0) dz = -dz;
        if (dx < 0.0001f && dy < 0.0001f && dz < 0.0001f) return;
    }
    if (n >= LAU_MAXV) return;
    g_lau_x[dslot][a][n] = x;
    g_lau_y[dslot][a][n] = y;
    g_lau_z[dslot][a][n] = z;
    g_lau_nv[dslot][a] = n + 1;
}

/* S6 walk storing name-hit geoms only into the def slot. */
static int lau_walk(GmCur* c, const unsigned char* vbuf, int nverts,
                    const char* want, int* nodes, int dslot) {
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
            int ln, sc, s, hit;
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
            hit = area_name_hit(nm, ln, want);
            if (hit && g_lau_nq[dslot] < LAU_MAXQ) {
                int a = g_lau_nq[dslot];
                g_lau_nv[dslot][a] = 0;
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
                        lau_push(dslot, a, x, y, z);
                    }
                }
                if (g_lau_nv[dslot][a] >= 3) g_lau_nq[dslot]++;
            }
        }
        return 1;
    }
    {
        int k;
        if (sub > 100000) return 0;
        for (k = 0; k < sub; k++) {
            int r = lau_walk(c, vbuf, nverts, want, nodes, dslot);
            if (r != 1) return r;
        }
    }
    return 1;
}

static int lau_half(float x, float z) {
    return (z > 0.0f || (z == 0.0f && x > 0.0f)) ? 0 : 1;
}

/* CCW-order quad verts around the centroid (no libm) */
static void lau_sort_ccw(int dslot, int a) {
    int n = g_lau_nv[dslot][a], i, pass;
    float cx = 0.0f, cz = 0.0f;
    for (i = 0; i < n; i++) {
        cx += g_lau_x[dslot][a][i];
        cz += g_lau_z[dslot][a][i];
    }
    cx /= (float)n;
    cz /= (float)n;
    for (pass = 0; pass < n; pass++) {
        for (i = 0; i < n - 1; i++) {
            float ax = g_lau_x[dslot][a][i] - cx;
            float az = g_lau_z[dslot][a][i] - cz;
            float bx = g_lau_x[dslot][a][i + 1] - cx;
            float bz = g_lau_z[dslot][a][i + 1] - cz;
            int ha = lau_half(ax, az), hb = lau_half(bx, bz);
            int swap = 0;
            if (ha != hb) swap = (ha > hb);
            else if (ax * bz - az * bx < 0.0f) swap = 1;
            if (swap) {
                float t;
                t = g_lau_x[dslot][a][i];
                g_lau_x[dslot][a][i] = g_lau_x[dslot][a][i + 1];
                g_lau_x[dslot][a][i + 1] = t;
                t = g_lau_y[dslot][a][i];
                g_lau_y[dslot][a][i] = g_lau_y[dslot][a][i + 1];
                g_lau_y[dslot][a][i + 1] = t;
                t = g_lau_z[dslot][a][i];
                g_lau_z[dslot][a][i] = g_lau_z[dslot][a][i + 1];
                g_lau_z[dslot][a][i + 1] = t;
            }
        }
    }
}

/* shoelace XZ area */
static float lau_size2(int dslot, int a) {
    int n = g_lau_nv[dslot][a], i;
    float s = 0.0f;
    for (i = 0; i < n; i++) {
        int j = (i + 1) % n;
        s += g_lau_x[dslot][a][i] * g_lau_z[dslot][a][j] -
             g_lau_x[dslot][a][j] * g_lau_z[dslot][a][i];
    }
    if (s < 0) s = -s;
    return s * 0.5f;
}

static int lau_inside(int dslot, int a, float px, float pz) {
    int n = g_lau_nv[dslot][a], i, pos = 0, neg = 0;
    for (i = 0; i < n; i++) {
        int j = (i + 1) % n;
        float cr = (g_lau_x[dslot][a][j] - g_lau_x[dslot][a][i]) *
                   (pz - g_lau_z[dslot][a][i]) -
                   (g_lau_z[dslot][a][j] - g_lau_z[dslot][a][i]) *
                   (px - g_lau_x[dslot][a][i]);
        if (cr > 0.001f) pos = 1;
        else if (cr < -0.001f) neg = 1;
        if (pos && neg) return 0;
    }
    return 1;
}

/* level-start pass: S6 name-hit quads for one launch def. */
static void launch_scan_file(const char* levelpath, const char* defname) {
    unsigned len = 0;
    unsigned char* d;
    GmCur c;
    const unsigned char* vbuf = 0;
    int nverts = 0, nodes = 0, q, dslot;
    char qbuf[96];
    if (!levelpath || !levelpath[0] || !defname || !defname[0]) return;
    dslot = launch_slot(defname);
    if (dslot < 0) {
        log_mod("  LAUQ: def table full, sphere fallback");
        return;
    }
    d = gm_read_file(levelpath, &len);
    if (!d || len < 32) {
        log_mod("  LAUQ: cannot read level file");
        if (d) free(d);
        return;
    }
    c.p = d;
    c.end = d + len;
    if (!gm_skip_to_s6(&c, &vbuf, &nverts) || !vbuf || nverts <= 0) {
        log_mod("  LAUQ: level S1..S5 prefix unreadable");
        free(d);
        return;
    }
    lau_walk(&c, vbuf, nverts, defname, &nodes, dslot);
    free(d);
    for (q = 0; q < g_lau_nq[dslot]; q++) {
        lau_sort_ccw(dslot, q);
        if (lau_size2(dslot, q) < 0.5f) {
            g_lau_nv[dslot][q] = 0;
            continue;
        }
    }
    {   /* compact out dropped quads */
        int w = 0, r, v;
        for (r = 0; r < g_lau_nq[dslot]; r++) {
            if (!g_lau_nv[dslot][r]) continue;
            if (w != r) {
                for (v = 0; v < g_lau_nv[dslot][r]; v++) {
                    g_lau_x[dslot][w][v] = g_lau_x[dslot][r][v];
                    g_lau_y[dslot][w][v] = g_lau_y[dslot][r][v];
                    g_lau_z[dslot][w][v] = g_lau_z[dslot][r][v];
                }
                g_lau_nv[dslot][w] = g_lau_nv[dslot][r];
            }
            w++;
        }
        g_lau_nq[dslot] = w;
    }
    snprintf(qbuf, sizeof(qbuf), "  LAUQ: %s: %d quad(s)%s", defname,
             g_lau_nq[dslot],
             g_lau_nq[dslot] ? ", trigger armed" : ", sphere fallback");
    log_mod(qbuf);
}

/* 1 = ball XZ touches any quad of the def slot (+-r rim samples). */
static int lau_cover_at(int dslot, float px, float pz, float r) {
    int a;
    if (dslot < 0 || dslot >= g_lau_nd) return 0;
    if (r < 0.0f) r = 0.0f;
    if (r > 60.0f) r = 60.0f;
    for (a = 0; a < g_lau_nq[dslot]; a++) {
        if (g_lau_nv[dslot][a] < 3) continue;
        if (lau_inside(dslot, a, px, pz)) return 1;
        if (r > 0.0f) {
            if (lau_inside(dslot, a, px + r, pz)) return 1;
            if (lau_inside(dslot, a, px - r, pz)) return 1;
            if (lau_inside(dslot, a, px, pz + r)) return 1;
            if (lau_inside(dslot, a, px, pz - r)) return 1;
        }
    }
    return 0;
}

#endif /* ENTLAUNCH_H */
