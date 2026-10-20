/* entyaw.h -- REF-point yaw for named entities (v1bk).
 *
 * Yaw sources (first hit wins):
 *   1. ROT_Y= degrees inside a <DAT>..</DAT> block of the S1 ref name
 *      (MAKYUNI convention, same as the bass proxy mod).
 *   2. The on-disk level file S1 entry rot triple (file x,z,y order, so
 *      the game-Y yaw is the 3rd float). Degrees when |v| > 2*PI.
 *   3. 0 (old behaviour, byte-identical path).
 *
 * Use: mesh source verts are rotated about Y around their own centroid
 * BEFORE the entity ctor runs (visual + collision stay consistent, zero
 * render-matrix writes -- matrix writes crash, see bass v55n_84). The
 * Woodbridge trigger ellipsoid uses the same angle, so zone and visual
 * always agree (absolute sign vs the editor is verified live).
 * Normals are intentionally untouched (positions only, like the proven
 * bass translate pattern).
 *
 * INCLUDE POINT: same as aibeh.h (needs IsBadReadPtr, log_mod,
 * gm_read_file, free via nocrt macros).
 */
#ifndef ENTYAW_H
#define ENTYAW_H

#define ENTYAW_PI 3.14159265f
#define ENTYAW_2PI 6.2831853f
#define ENTYAW_DEG2RAD (ENTYAW_PI / 180.0f)
#define ENTYAW_RAD2DEG (180.0f / ENTYAW_PI)

/* sin+cos via Taylor after range reduction (no libm under -nostdlib). */
static void ent_sincos(float rad, float* out_s, float* out_c) {
    float x = rad, x2, s, c, y;
    int fs = 1, fc = 1;
    while (x > ENTYAW_PI) x -= ENTYAW_2PI;
    while (x < -ENTYAW_PI) x += ENTYAW_2PI;
    /* fold to [-PI/2, PI/2] so Taylor stays tight everywhere */
    y = x;
    if (y > ENTYAW_PI * 0.5f) { y = ENTYAW_PI - y; fc = -1; }
    else if (y < -ENTYAW_PI * 0.5f) { y = y + ENTYAW_PI; fs = -1; fc = -1; }
    x2 = y * y;
    s = y * (1.0f - x2 * (0.16666666f - x2 * (0.00833333f - x2 * (0.00019841f - x2 * 0.0000027556f))));
    c = 1.0f - x2 * (0.5f - x2 * (0.04166666f - x2 * (0.00138888f - x2 * (0.0000248016f - x2 * 0.0000002756f))));
    if (out_s) *out_s = s * (float)fs;
    if (out_c) *out_c = c * (float)fc;
}

/* ROT_Y= degrees inside <DAT>..</DAT> (case-insensitive). Returns 1 + deg. */
static int ent_dat_yaw_deg(const char* nm, float* out_deg) {
    const char* p, *bs, *be, *q;
    if (!nm || !out_deg) return 0;
    bs = 0;
    for (p = nm; *p; p++) {
        if (p[0] == '<' && (p[1] == 'D' || p[1] == 'd') &&
            (p[2] == 'A' || p[2] == 'a') && (p[3] == 'T' || p[3] == 't') &&
            p[4] == '>') { bs = p + 5; break; }
    }
    if (!bs) return 0;
    be = bs;
    while (*be) {
        if (be[0] == '<' && be[1] == '/' && (be[2] == 'D' || be[2] == 'd') &&
            (be[3] == 'A' || be[3] == 'a') && (be[4] == 'T' || be[4] == 't') &&
            be[5] == '>') break;
        be++;
    }
    for (q = bs; q < be; q++) {
        const char* v;
        int neg = 0;
        long ip = 0;
        float frac = 0.0f, div = 1.0f;
        if (!((q[0] == 'R' || q[0] == 'r') && (q[1] == 'O' || q[1] == 'o') &&
              (q[2] == 'T' || q[2] == 't') && q[3] == '_' &&
              (q[4] == 'Y' || q[4] == 'y') && q[5] == '=')) continue;
        v = q + 6;
        while (v < be && (*v == ' ' || *v == 9 || *v == '"')) v++;
        if (v < be && (*v == '-' || *v == '+')) { neg = (*v == '-'); v++; }
        if (v >= be || !(*v >= '0' && *v <= '9')) return 0;
        while (v < be && *v >= '0' && *v <= '9') { ip = ip * 10 + (*v - '0'); v++; }
        if (v < be && *v == '.') {
            v++;
            while (v < be && *v >= '0' && *v <= '9') {
                frac = frac * 10.0f + (float)(*v - '0');
                div *= 10.0f;
                v++;
            }
        }
        *out_deg = (neg ? -1.0f : 1.0f) * ((float)ip + frac / div);
        return 1;
    }
    return 0;
}

/* ci substring: 1 when needle occurs in haystack (empty needle = 0). */
static int ent_ci_contains(const char* haystack, const char* needle) {
    const char* h;
    if (!haystack || !needle || !*needle) return 0;
    for (h = haystack; *h; h++) {
        const char* p1 = h;
        const char* p2 = needle;
        while (*p1 && *p2) {
            char c1 = *p1, c2 = *p2;
            if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
            if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
            if (c1 != c2) break;
            p1++; p2++;
        }
        if (!*p2) return 1;
    }
    return 0;
}

/* Rotate one xyz point about ALL THREE game axes around (cx,cy,cz).
 * R = Rz(rz)*Ry(ry)*Rx(rx), right-handed. Single-axis data is
 * order-independent; combined-axis order is documented here. */
static void ent_rot_pt3(float* x, float* y, float* z,
                        float cx, float cy, float cz,
                        float cx1, float sx, float cy1, float sy,
                        float cz1, float sz) {
    float rx = *x - cx, ry = *y - cy, rz = *z - cz;
    float t1, t2, t3;
    /* Rx */
    t2 = ry * cx1 - rz * sx;
    t3 = ry * sx + rz * cx1;
    ry = t2; rz = t3;
    /* Ry */
    t1 = rx * cy1 + rz * sy;
    t3 = -rx * sy + rz * cy1;
    rx = t1; rz = t3;
    /* Rz */
    t1 = rx * cz1 - ry * sz;
    t2 = rx * sz + ry * cz1;
    rx = t1; ry = t2;
    *x = cx + rx;
    *y = cy + ry;
    *z = cz + rz;
}

/* Inverse of ent_rot_pt3 (transpose): entity frame from world offset. */
static void ent_unrot_pt3(float dx, float dy, float dz,
                          float rx, float ry, float rz,
                          float* ox, float* oy, float* oz) {
    float sx, cx, sy, cy, sz, cz;
    float t1, t2, t3;
    ent_sincos(rx, &sx, &cx);
    ent_sincos(ry, &sy, &cy);
    ent_sincos(rz, &sz, &cz);
    /* un-Z */
    t1 = dx * cz + dy * sz;
    t2 = -dx * sz + dy * cz;
    dx = t1; dy = t2;
    /* un-Y */
    t1 = dx * cy - dz * sy;
    t3 = dx * sy + dz * cy;
    dx = t1; dz = t3;
    /* un-X */
    t2 = dy * cx + dz * sx;
    t3 = -dy * sx + dz * cx;
    dy = t2; dz = t3;
    *ox = dx; *oy = dy; *oz = dz;
}

/* S1 file rotation for the entry matching this ref. Match = position
 * within 2u (file S1 pos is PLAIN x,y,z in Marble-exported levels, NOT
 * the x,z,y of the repo doc -- LevelC bridge3 raw == spawn, proven)
 * AND name ci-equal (or either contains the other). v1bo+: the engine
 * yaw channel (what Mousetrap reads as facing) is the THIRD float, so
 * rot2->ry; v1bu: that ref angle is applied to Z (bank). Outputs rad. */
static int ent_file_rot(const char* levelpath, const char* s1name,
                        float px, float py, float pz,
                        float* out_rx, float* out_ry, float* out_rz) {
    unsigned len = 0;
    unsigned char* d;
    const unsigned char* p;
    const unsigned char* end;
    unsigned n, i;
    if (!levelpath || !levelpath[0] || !s1name || !out_rx || !out_ry || !out_rz) return 0;
    d = gm_read_file(levelpath, &len);
    if (!d || len < 4) return 0;
    p = d;
    end = d + len;
    n = *(unsigned*)p;
    p += 4;
    if (n > 4096) { free(d); return 0; }
    for (i = 0; i < n; i++) {
        unsigned nl, hm, hrefl, htex, tnl;
        const char* nm;
        float pos0, pos1, pos2, rot0, rot1, rot2;
        const float* f;
        if (p + 4 > end) break;
        nl = *(unsigned*)p;
        p += 4;
        if (nl == 0 || nl > 256 || p + nl > end) break;
        nm = (const char*)p;
        p += nl;
        if (p + 24 > end) break;
        f = (const float*)p;
        pos0 = f[0]; pos1 = f[1]; pos2 = f[2];
        rot0 = f[3]; rot1 = f[4]; rot2 = f[5];
        p += 24;
        if (p + 4 > end) break;
        hm = *(unsigned*)p;
        p += 4;
        if (hm & 0xFF) {
            if (p + 72 > end) break;
            p += 68;
            hrefl = *(unsigned*)p;
            p += 4;
            (void)hrefl;
            htex = *(unsigned*)p;
            p += 4;
            if (htex) {
                if (p + 4 > end) break;
                tnl = *(unsigned*)p;
                p += 4;
                if (tnl > 256 || p + tnl > end) break;
                p += tnl;
            }
        }
        {
            /* v1bm: PLAIN x,y,z (LevelC-proven), not x,z,y */
            float dx = pos0 - px, dy = pos1 - py, dz = pos2 - pz;
            const char* a = nm;
            const char* b = s1name;
            int eq = 1;
            while (*a && *b) {
                char ca = *a, cb = *b;
                if (ca >= 'A' && ca <= 'Z') ca += 32;
                if (cb >= 'A' && cb <= 'Z') cb += 32;
                if (ca != cb) { eq = 0; break; }
                a++; b++;
            }
            if (*a || *b) eq = 0;
            /* lenient: either string contains the other (REF: prefixes) */
            if (!eq && (ent_ci_contains(nm, s1name) ||
                        ent_ci_contains(s1name, nm))) eq = 1;
            if (eq && dx * dx + dy * dy + dz * dz < 4.0f) {
                /* v1bo: engine yaw channel = THIRD float (Mousetrap reads
                 * S1 entry[5] as its facing, 360-x). Map rot2->ry (yaw),
                 * rot0->rx, rot1->rz (kept for trigger, no draw path). */
                *out_rx = rot0 * ENTYAW_DEG2RAD;
                *out_ry = rot2 * ENTYAW_DEG2RAD;
                *out_rz = rot1 * ENTYAW_DEG2RAD;
                free(d);
                return 1;
            }
        }
    }
    free(d);
    return 0;
}

/* Triple rotation of a loaded MeshWorld about its own centroid
 * (spawn-time, pre-ctor). Walks the proven bass offsets: mw+0x18 tree
 * items, mw+0x2C MeshBuffers -> sub verts (sub+0x448, +0x10 transient)
 * + strips (mb+0x10/+0x418). Returns verts rotated, 0 = nothing. */
static int ent_rotate_mesh3(DWORD mw, float rx, float ry, float rz) {
    float ax = rx < 0.0f ? -rx : rx;
    float ay = ry < 0.0f ? -ry : ry;
    float az = rz < 0.0f ? -rz : rz;
    float cx = 0.0f, cy = 0.0f, cz = 0.0f;
    float sx, cx1, sy, cy1, sz, cz1;
    int n = 0, total = 0;
    DWORD mwlist, mb_list;
    if (!mw || mw < 0x10000 || IsBadReadPtr((void*)mw, 0x460)) return 0;
    if (ax < 0.000001f && ay < 0.000001f && az < 0.000001f) return 0;
    ent_sincos(rx, &sx, &cx1);
    ent_sincos(ry, &sy, &cy1);
    ent_sincos(rz, &sz, &cz1);
    /* pass 1: centroid from tree items, else from sub verts */
    mwlist = mw + 0x18;
    if (!IsBadReadPtr((void*)mwlist, 0x20)) {
        int tcount = *(int*)(mwlist + 0x04);
        DWORD* titems = 0;
        int ti;
        if (tcount > 0 && tcount < 65536 &&
            !IsBadReadPtr((void*)(mwlist + 0x40C), 4)) {
            titems = *(DWORD**)(mwlist + 0x40C);
            if (titems && !IsBadReadPtr((void*)titems, tcount * 4)) {
                for (ti = 0; ti < tcount; ti++) {
                    DWORD item = titems[ti];
                    float* fp;
                    if (!item || item < 0x10000 || IsBadReadPtr((void*)item, 0x10)) continue;
                    fp = (float*)item;
                    cx += fp[0];
                    cy += fp[1];
                    cz += fp[2];
                    n++;
                }
            }
        }
    }
    mb_list = mw + 0x2C;
    if (n == 0 && !IsBadReadPtr((void*)mb_list, 0x20)) {
        int mb_count = *(int*)(mb_list + 0x04);
        if (mb_count > 0 && mb_count <= 64 &&
            !IsBadReadPtr((void*)(mb_list + 0x40C), 4)) {
            DWORD* mb_items = *(DWORD**)(mb_list + 0x40C);
            int bi;
            if (mb_items && !IsBadReadPtr((void*)mb_items, mb_count * 4)) {
                for (bi = 0; bi < mb_count && n < 1000000; bi++) {
                    DWORD mb = mb_items[bi];
                    int scnt;
                    DWORD* sitems;
                    int si;
                    if (!mb || mb < 0x10000 || IsBadReadPtr((void*)mb, 0x850)) continue;
                    scnt = *(int*)((char*)mb + 0x428);
                    if (scnt <= 0 || scnt > 4096) continue;
                    if (IsBadReadPtr((void*)((char*)mb + 0x830), 4)) continue;
                    sitems = *(DWORD**)((char*)mb + 0x830);
                    if (!sitems || IsBadReadPtr((void*)sitems, scnt * 4)) continue;
                    for (si = 0; si < scnt; si++) {
                        DWORD sub = sitems[si];
                        DWORD tri;
                        int vcnt;
                        float* verts;
                        int vi;
                        if (!sub || sub < 0x10000 || IsBadReadPtr((void*)sub, 0x20)) continue;
                        tri = *(DWORD*)((char*)sub + 0x04);
                        vcnt = (int)tri + 2;
                        if (vcnt <= 0 || vcnt > 65536) continue;
                        verts = *(float**)((char*)sub + 0x448);
                        if (!verts || IsBadReadPtr((void*)verts, vcnt * 32)) continue;
                        for (vi = 0; vi < vcnt; vi++) {
                            cx += verts[vi * 8 + 0];
                            cy += verts[vi * 8 + 1];
                            cz += verts[vi * 8 + 2];
                            n++;
                        }
                    }
                }
            }
        }
    }
    if (n <= 0) {
        log_mod("  ENT yaw: no geometry, rotation skipped");
        return 0;
    }
    cx /= (float)n;
    cy /= (float)n;
    cz /= (float)n;
    /* pass 2a: tree items */
    if (!IsBadReadPtr((void*)mwlist, 0x20)) {
        int tcount = *(int*)(mwlist + 0x04);
        if (tcount > 0 && tcount < 65536 &&
            !IsBadReadPtr((void*)(mwlist + 0x40C), 4)) {
            DWORD* titems = *(DWORD**)(mwlist + 0x40C);
            int ti;
            if (titems && !IsBadReadPtr((void*)titems, tcount * 4)) {
                for (ti = 0; ti < tcount; ti++) {
                    DWORD item = titems[ti];
                    float* fp;
                    if (!item || item < 0x10000 || IsBadReadPtr((void*)item, 0x10)) continue;
                    fp = (float*)item;
                    ent_rot_pt3(&fp[0], &fp[1], &fp[2], cx, cy, cz,
                                cx1, sx, cy1, sy, cz1, sz);
                    total++;
                }
            }
        }
    }
    /* pass 2b: sub verts (+0x448 source, +0x10 transient) + strips */
    if (!IsBadReadPtr((void*)mb_list, 0x20)) {
        int mb_count = *(int*)(mb_list + 0x04);
        if (mb_count > 0 && mb_count <= 64 &&
            !IsBadReadPtr((void*)(mb_list + 0x40C), 4)) {
            DWORD* mb_items = *(DWORD**)(mb_list + 0x40C);
            int bi;
            if (mb_items && !IsBadReadPtr((void*)mb_items, mb_count * 4)) {
                for (bi = 0; bi < mb_count; bi++) {
                    DWORD mb = mb_items[bi];
                    int scnt, strip_count, si, si2;
                    DWORD* sitems;
                    DWORD* strip_items;
                    if (!mb || mb < 0x10000 || IsBadReadPtr((void*)mb, 0x850)) continue;
                    scnt = *(int*)((char*)mb + 0x428);
                    if (scnt > 0 && scnt <= 4096 &&
                        !IsBadReadPtr((void*)((char*)mb + 0x830), 4)) {
                        sitems = *(DWORD**)((char*)mb + 0x830);
                        if (sitems && !IsBadReadPtr((void*)sitems, scnt * 4)) {
                            for (si = 0; si < scnt; si++) {
                                DWORD sub = sitems[si];
                                DWORD tri;
                                int vcnt, vi, vi2;
                                float* verts;
                                float* verts2;
                                if (!sub || sub < 0x10000 || IsBadReadPtr((void*)sub, 0x20)) continue;
                                tri = *(DWORD*)((char*)sub + 0x04);
                                vcnt = (int)tri + 2;
                                if (vcnt <= 0 || vcnt > 65536) continue;
                                verts = *(float**)((char*)sub + 0x448);
                                if (verts && !IsBadReadPtr((void*)verts, vcnt * 32)) {
                                    for (vi = 0; vi < vcnt; vi++) {
                                        ent_rot_pt3(&verts[vi * 8 + 0], &verts[vi * 8 + 1], &verts[vi * 8 + 2],
                                                    cx, cy, cz, cx1, sx, cy1, sy, cz1, sz);
                                    }
                                    total += vcnt;
                                }
                                verts2 = *(float**)((char*)sub + 0x10);
                                if (verts2 && verts2 != verts &&
                                    !IsBadReadPtr((void*)verts2, vcnt * 32)) {
                                    for (vi2 = 0; vi2 < vcnt; vi2++) {
                                        ent_rot_pt3(&verts2[vi2 * 8 + 0], &verts2[vi2 * 8 + 1], &verts2[vi2 * 8 + 2],
                                                    cx, cy, cz, cx1, sx, cy1, sy, cz1, sz);
                                    }
                                }
                            }
                        }
                    }
                    strip_count = *(int*)((char*)mb + 0x10);
                    if (strip_count > 0 && strip_count <= 4096 &&
                        !IsBadReadPtr((void*)((char*)mb + 0x418), 4)) {
                        strip_items = *(DWORD**)((char*)mb + 0x418);
                        if (strip_items && !IsBadReadPtr((void*)strip_items, strip_count * 4)) {
                            for (si2 = 0; si2 < strip_count; si2++) {
                                DWORD strip = strip_items[si2];
                                float* sp;
                                int v;
                                if (!strip || strip < 0x10000 || IsBadReadPtr((void*)strip, 0x60)) continue;
                                sp = (float*)strip;
                                for (v = 0; v < 3; v++) {
                                    ent_rot_pt3(&sp[v * 8 + 0], &sp[v * 8 + 1], &sp[v * 8 + 2],
                                                cx, cy, cz, cx1, sx, cy1, sy, cz1, sz);
                                }
                                total += 3;
                            }
                        }
                    }
                }
            }
        }
    }
    return total;
}

#endif /* ENTYAW_H */
