/* gridmesh.h — extract one S6 geom from a levels\ MESHWORLD into a minimal
 * standalone mesh file for PopCylinder spawning.
 *
 * Affix rules apply natively at load (name preserved incl. (NOCOLLIDE)).
 * Pure logic + small OS layer (WinAPI in mod, posix under GRIDMESH_HARNESS).
 * Only needs malloc/free/memcpy/memset/strlen/strcmp/strncmp from includer.
 */
#ifndef GRIDMESH_H
#define GRIDMESH_H

#ifdef GRIDMESH_HARNESS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
typedef unsigned int DWORD;
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#else
/* mod build: windows.h + nc_* mappings come from the including .cpp */
#endif

#define GM_MAX_FILE (8u * 1024u * 1024u)
#define GM_MAX_NODES 20000
#define GM_MAX_LIST 256

/* ---------- OS layer ---------- */
#ifndef GRIDMESH_HARNESS
static unsigned char* gm_read_file(const char* path, unsigned* out_len) {
    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return 0;
    DWORD hi = 0;
    DWORD lo = GetFileSize(h, &hi);
    if (hi || lo > GM_MAX_FILE || lo < 32) { CloseHandle(h); return 0; }
    unsigned char* buf = (unsigned char*)malloc(lo);
    if (!buf) { CloseHandle(h); return 0; }
    DWORD rd = 0;
    if (!ReadFile(h, buf, lo, &rd, NULL) || rd != lo) {
        free(buf);
        buf = 0;
    }
    CloseHandle(h);
    if (out_len) *out_len = buf ? lo : 0;
    return buf;
}
static int gm_write_file(const char* path, const unsigned char* d,
                         unsigned n) {
    HANDLE h = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return 0;
    DWORD w = 0;
    int ok = WriteFile(h, d, n, &w, NULL) && w == n;
    CloseHandle(h);
    return ok;
}
static DWORD gm_file_size(const char* path) {
    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return (DWORD)-1;
    DWORD hi = 0;
    DWORD lo = GetFileSize(h, &hi);
    CloseHandle(h);
    return hi ? (DWORD)-1 : lo;
}
/* out[][MAX_PATH]; returns count. pattern dir already includes trailing \ */
static int gm_list_mw(const char* dir, char out[][MAX_PATH], int maxn) {
    char pat[MAX_PATH];
    int di = 0;
    while (di < MAX_PATH - 14 && dir[di]) { pat[di] = dir[di]; di++; }
    pat[di] = 0;
    {
        const char* star = "*.MESHWORLD";
        int m = 0;
        while (m < 12 && di < MAX_PATH - 1 && star[m]) {
            pat[di] = star[m];
            di++;
            m++;
        }
        pat[di] = 0;
    }
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pat, &fd);
    if (h == INVALID_HANDLE_VALUE) return 0;
    int n = 0;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && n < maxn) {
            int k = 0;
            while (k < MAX_PATH - 1 && dir[k]) { out[n][k] = dir[k]; k++; }
            int j = 0;
            while (k < MAX_PATH - 1 && fd.cFileName[j]) {
                out[n][k] = fd.cFileName[j];
                k++;
                j++;
            }
            out[n][k] = 0;
            n++;
        }
    } while (FindNextFileA(h, &fd) && n < maxn);
    FindClose(h);
    return n;
}
#else
static unsigned char* gm_read_file(const char* path, unsigned* out_len) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 32 || sz > (long)GM_MAX_FILE) { fclose(f); return 0; }
    unsigned char* buf = (unsigned char*)malloc((unsigned)sz);
    if (!buf) { fclose(f); return 0; }
    if (fread(buf, 1, (unsigned)sz, f) != (unsigned)sz) {
        free(buf);
        buf = 0;
    }
    fclose(f);
    if (out_len) *out_len = buf ? (unsigned)sz : 0;
    return buf;
}
static int gm_write_file(const char* path, const unsigned char* d,
                         unsigned n) {
    FILE* f = fopen(path, "wb");
    if (!f) return 0;
    int ok = fwrite(d, 1, n, f) == n;
    fclose(f);
    return ok;
}
static DWORD gm_file_size(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return (DWORD)-1;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fclose(f);
    return (DWORD)sz;
}
static int gm_list_mw(const char* dir, char out[][MAX_PATH], int maxn) {
    DIR* dp = opendir(dir);
    if (!dp) return 0;
    int n = 0;
    struct dirent* e;
    while ((e = readdir(dp)) && n < maxn) {
        int L = 0;
        while (e->d_name[L]) L++;
        if (L < 11) continue;
        if (strcmp(e->d_name + L - 10, ".MESHWORLD") != 0) continue;
        int k = 0;
        while (k < MAX_PATH - 1 && dir[k]) { out[n][k] = dir[k]; k++; }
        int j = 0;
        while (k < MAX_PATH - 1 && e->d_name[j]) {
            out[n][k] = e->d_name[j];
            k++;
            j++;
        }
        out[n][k] = 0;
        n++;
    }
    closedir(dp);
    return n;
}
#endif

/* ---------- cursor ---------- */
typedef struct {
    const unsigned char* p;
    const unsigned char* end;
} GmCur;

static int gm_need(GmCur* c, unsigned n) {
    return c->p + n <= c->end;
}
static int gm_i32(GmCur* c) {
    int v;
    memcpy(&v, c->p, 4);
    c->p += 4;
    return v;
}
static unsigned gm_u32(GmCur* c) {
    unsigned v;
    memcpy(&v, c->p, 4);
    c->p += 4;
    return v;
}
static float gm_f32(GmCur* c) {
    float v;
    memcpy(&v, c->p, 4);
    c->p += 4;
    return v;
}

/* Skip S1..S5, leave cursor at S6 start. Returns S5 base/count. */
static int gm_skip_to_s6(GmCur* c, const unsigned char** vbuf, int* nverts) {
    int i, n;
    if (!gm_need(c, 4)) return 0;
    n = gm_i32(c);
    if (n < 0 || n > 100000) return 0;
    for (i = 0; i < n; i++) {
        int ln;
        if (!gm_need(c, 4)) return 0;
        ln = gm_i32(c);
        if (ln < 1 || ln > 1024 || !gm_need(c, (unsigned)ln + 24 + 4))
            return 0;
        c->p += (unsigned)ln + 24;
        {
            unsigned char hm = *c->p;
            c->p += 4;
            if (hm) {
                unsigned ht;
                if (!gm_need(c, 64 + 4 + 4 + 4)) return 0;
                c->p += 64 + 4 + 4;
                ht = gm_u32(c);
                if (ht == 1) {
                    int tl;
                    if (!gm_need(c, 4)) return 0;
                    tl = gm_i32(c);
                    if (tl < 1 || tl > 1024 || !gm_need(c, (unsigned)tl))
                        return 0;
                    c->p += (unsigned)tl;
                }
            }
        }
    }
    /* S2 */
    if (!gm_need(c, 4)) return 0;
    n = gm_i32(c);
    if (n < 0 || n > 100000) return 0;
    for (i = 0; i < n; i++) {
        int dl, pc;
        if (!gm_need(c, 4)) return 0;
        dl = gm_i32(c);
        if (dl < 0 || dl > 1000000 || !gm_need(c, (unsigned)dl + 4)) return 0;
        c->p += (unsigned)dl;
        pc = gm_i32(c);
        if (pc < 0 || pc > 1000000 || !gm_need(c, (unsigned)pc * 12u))
            return 0;
        c->p += (unsigned)pc * 12u;
    }
    /* S3 */
    if (!gm_need(c, 4)) return 0;
    n = gm_i32(c);
    if (n < 0 || n > 100000) return 0;
    for (i = 0; i < n; i++) {
        unsigned t;
        if (!gm_need(c, 4)) return 0;
        t = gm_u32(c);
        if (t == 0) {
            if (!gm_need(c, 36)) return 0;
            c->p += 36;
        }
    }
    /* S4 */
    if (!gm_need(c, 24)) return 0;
    c->p += 24;
    /* S5 */
    if (!gm_need(c, 4)) return 0;
    n = gm_i32(c);
    if (n < 0 || n > 3000000) return 0;
    if (!gm_need(c, (unsigned)n * 32u)) return 0;
    *vbuf = c->p;
    *nverts = n;
    c->p += (unsigned)n * 32u;
    return 1;
}

typedef struct {
    const unsigned char* name;
    int namelen;
    const unsigned char* mat; /* 72 raw */
    unsigned has_tex;
    const unsigned char* tex;
    int texlen;
    const unsigned char* strips;
    int nstrips;
} GmGeom;

typedef struct {
    const char* want1;
    const char* want2;
    int hit;
    GmGeom g;
    int nodes;
} GmFind;

static int gm_name_eq(const unsigned char* raw, int rawlen, const char* want) {
    int wl = 0;
    if (!want || !*want) return 0;
    while (want[wl]) wl++;
    if (rawlen != wl + 1) return 0;
    return memcmp(raw, want, (unsigned)wl) == 0;
}

/* returns 0=fail, 1=continue, 2=found (unwind) */
static int gm_node(GmCur* c, GmFind* f) {
    int sub, mbc, i;
    if (++f->nodes > GM_MAX_NODES) return 0;
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
            const unsigned char* matp;
            unsigned ht;
            const unsigned char* txp = 0;
            int txl = 0;
            const unsigned char* spp;
            if (!gm_need(c, 4)) return 0;
            ln = gm_i32(c);
            if (ln < 1 || ln > 1024 || !gm_need(c, (unsigned)ln)) return 0;
            nm = c->p;
            c->p += (unsigned)ln;
            matp = c->p;
            if (!gm_need(c, 72 + 4)) return 0;
            c->p += 72;
            ht = gm_u32(c);
            if (ht == 1) {
                int tl;
                if (!gm_need(c, 4)) return 0;
                tl = gm_i32(c);
                if (tl < 1 || tl > 1024 || !gm_need(c, (unsigned)tl))
                    return 0;
                txp = c->p;
                txl = tl;
                c->p += (unsigned)tl;
            }
            if (!gm_need(c, 4)) return 0;
            sc = gm_i32(c);
            if (sc < 0 || sc > 1000000) return 0;
            if (!gm_need(c, (unsigned)sc * 8u)) return 0;
            spp = c->p;
            c->p += (unsigned)sc * 8u;
            if (!f->hit && (gm_name_eq(nm, ln, f->want1) ||
                            gm_name_eq(nm, ln, f->want2))) {
                f->hit = 1;
                f->g.name = nm;
                f->g.namelen = ln;
                f->g.mat = matp;
                f->g.has_tex = ht;
                f->g.tex = txp;
                f->g.texlen = txl;
                f->g.strips = spp;
                f->g.nstrips = sc;
                return 2;
            }
        }
        return 1;
    }
    {
        int k;
        if (sub > 100000) return 0;
        for (k = 0; k < sub; k++) {
            int r = gm_node(c, f);
            if (r != 1) return r;
        }
    }
    return 1;
}

/* Emit minimal standalone mesh: S1=0,S2=0,S3=0,S4=0,S5=slice rebased so its
 * centroid sits at origin, S6=single leaf with the geom (name/affix kept).
 * Caller spawns at the ref point. Returns malloc'd buffer (caller frees). */
static unsigned char* gm_emit(const GmGeom* g, const unsigned char* vbuf,
                              int nverts, unsigned* out_len) {
    int i, lo = -1, hi = -1;
    float cx = 0, cy = 0, cz = 0;
    int nv = 0;
    (void)nverts;
    for (i = 0; i < g->nstrips; i++) {
        int tri, vr;
        memcpy(&tri, g->strips + (unsigned)i * 8u, 4);
        memcpy(&vr, g->strips + (unsigned)i * 8u + 4, 4);
        if (tri < 0 || tri > 100000 || vr < 0) return 0;
        if (lo < 0 || vr < lo) lo = vr;
        if (vr + tri + 2 > hi) hi = vr + tri + 2;
    }
    if (lo < 0 || hi <= lo) return 0;
    nv = hi - lo;
    {
        int k;
        for (k = 0; k < nv; k++) {
            float x, y, z;
            memcpy(&x, vbuf + (unsigned)(lo + k) * 32u, 4);
            memcpy(&y, vbuf + (unsigned)(lo + k) * 32u + 4, 4);
            memcpy(&z, vbuf + (unsigned)(lo + k) * 32u + 8, 4);
            cx += x;
            cy += y;
            cz += z;
        }
        cx /= (float)nv;
        cy /= (float)nv;
        cz /= (float)nv;
    }
    {
        unsigned cap = 128u + (unsigned)nv * 32u + 64u +
                       (unsigned)g->namelen + 72u + 4u +
                       (g->has_tex == 1 ? (unsigned)g->texlen + 4u : 0u) + 4u +
                       (unsigned)g->nstrips * 8u + 64u;
        unsigned char* o = (unsigned char*)malloc(cap);
        unsigned char* w;
        float mnx = 0, mny = 0, mnz = 0, mxx = 0, mxy = 0, mxz = 0;
        int k;
        unsigned u0 = 0;
        if (!o) return 0;
        w = o;
        memcpy(w, &u0, 4);
        w += 4; /* S1 */
        memcpy(w, &u0, 4);
        w += 4; /* S2 */
        memcpy(w, &u0, 4);
        w += 4; /* S3 */
        memset(w, 0, 24);
        w += 24; /* S4 */
        {
            unsigned unv = (unsigned)nv;
            memcpy(w, &unv, 4);
            w += 4;
            for (k = 0; k < nv; k++) {
                float x, y, z;
                memcpy(&x, vbuf + (unsigned)(lo + k) * 32u, 4);
                memcpy(&y, vbuf + (unsigned)(lo + k) * 32u + 4, 4);
                memcpy(&z, vbuf + (unsigned)(lo + k) * 32u + 8, 4);
                x -= cx;
                y -= cy;
                z -= cz;
                if (k == 0) {
                    mnx = mxx = x;
                    mny = mxy = y;
                    mnz = mxz = z;
                } else {
                    if (x < mnx) mnx = x;
                    if (x > mxx) mxx = x;
                    if (y < mny) mny = y;
                    if (y > mxy) mxy = y;
                    if (z < mnz) mnz = z;
                    if (z > mxz) mxz = z;
                }
                memcpy(w, &x, 4);
                w += 4;
                memcpy(w, &y, 4);
                w += 4;
                memcpy(w, &z, 4);
                w += 4;
                memcpy(w, vbuf + (unsigned)(lo + k) * 32u + 12, 20);
                w += 20;
            }
        }
        /* S6 root leaf */
        memcpy(w, &mnx, 4);
        w += 4;
        memcpy(w, &mny, 4);
        w += 4;
        memcpy(w, &mnz, 4);
        w += 4;
        memcpy(w, &mxx, 4);
        w += 4;
        memcpy(w, &mxy, 4);
        w += 4;
        memcpy(w, &mxz, 4);
        w += 4;
        memcpy(w, &u0, 4);
        w += 4; /* sub=0 */
        {
            unsigned u1 = 1;
            memcpy(w, &u1, 4);
            w += 4; /* 1 geom */
        }
        {
            unsigned unl = (unsigned)g->namelen;
            memcpy(w, &unl, 4);
            w += 4;
        }
        memcpy(w, g->name, (unsigned)g->namelen);
        w += (unsigned)g->namelen;
        memcpy(w, g->mat, 72);
        w += 72;
        {
            unsigned uht = g->has_tex == 1 ? 1u : 0u;
            memcpy(w, &uht, 4);
            w += 4;
            if (g->has_tex == 1) {
                unsigned utl = (unsigned)g->texlen;
                memcpy(w, &utl, 4);
                w += 4;
                memcpy(w, g->tex, (unsigned)g->texlen);
                w += (unsigned)g->texlen;
            }
        }
        {
            unsigned usc = (unsigned)g->nstrips;
            memcpy(w, &usc, 4);
            w += 4;
            for (i = 0; i < g->nstrips; i++) {
                int tri, vr, nvr;
                memcpy(&tri, g->strips + (unsigned)i * 8u, 4);
                memcpy(&vr, g->strips + (unsigned)i * 8u + 4, 4);
                nvr = vr - lo;
                memcpy(w, &tri, 4);
                w += 4;
                memcpy(w, &nvr, 4);
                w += 4;
            }
        }
        *out_len = (unsigned)(w - o);
        return o;
    }
}

/* Extract geom (want1, else want2) from file into outpath, rebased so its
 * centroid is at origin. Returns 1 on success. */
static int gm_extract(const char* mwpath, const char* want1,
                      const char* want2, const char* outpath) {
    unsigned len = 0;
    unsigned char* data = gm_read_file(mwpath, &len);
    GmCur c;
    const unsigned char* vbuf = 0;
    int nverts = 0;
    GmFind f;
    unsigned char* emit = 0;
    unsigned elen = 0;
    int ok = 0;
    if (!data) return 0;
    c.p = data;
    c.end = data + len;
    if (!gm_skip_to_s6(&c, &vbuf, &nverts)) {
        free(data);
        return 0;
    }
    memset(&f, 0, sizeof(f));
    f.want1 = want1;
    f.want2 = want2;
    {
        int r = gm_node(&c, &f);
        if ((r == 2 || r == 1) && f.hit) {
            emit = gm_emit(&f.g, vbuf, nverts, &elen);
            if (emit && elen > 64)
                ok = gm_write_file(outpath, emit, elen);
            if (emit) free(emit);
        }
    }
    free(data);
    return ok;
}

static int gm_name_is_temp(const char* fname) {
    const char* b = fname;
    const char* p = fname;
    while (*p) {
        if (*p == '\\' || *p == '/') b = p + 1;
        p++;
    }
    return b[0] == 'm' && b[1] == 'k' && b[2] == 'n' && b[3] == 'p' &&
           b[4] == '_' && b[5] == 'g' && b[6] == 'r' && b[7] == 'i' &&
           b[8] == 'd';
}

/* Find first levels\ file (not our temp) holding geom want1/want2, extract
 * rebased to origin into outpath. levels_dir ends with \ (or /). */
static int gm_find_and_extract(const char* levels_dir, const char* want1,
                               const char* want2, const char* outpath) {
    static char files[GM_MAX_LIST][MAX_PATH];
    int n = gm_list_mw(levels_dir, files, GM_MAX_LIST);
    int i;
    for (i = 0; i < n; i++) {
        DWORD sz;
        if (gm_name_is_temp(files[i])) continue;
        sz = gm_file_size(files[i]);
        if (sz == (DWORD)-1 || sz > GM_MAX_FILE || sz < 64) continue;
        if (gm_extract(files[i], want1, want2, outpath)) return 1;
    }
    return 0;
}
/* FNV-1a over S1 (count + every ref name, no NUL). Runtime side feeds the
 * same stream, so equal hash + equal count identifies the level file. */
static unsigned gm_s1_hash(const unsigned char* data, unsigned len,
                           int* out_count) {
    GmCur c;
    unsigned h = 2166136261u;
    int n, i;
    if (out_count) *out_count = 0;
    if (!data || len < 4) return 0;
    c.p = data;
    c.end = data + len;
    n = gm_i32(&c);
    if (n < 0 || n > 100000) return 0;
    if (out_count) *out_count = n;
    h ^= (unsigned)(n & 0xFF);
    h *= 16777619u;
    h ^= (unsigned)((n >> 8) & 0xFF);
    h *= 16777619u;
    h ^= (unsigned)((n >> 16) & 0xFF);
    h *= 16777619u;
    h ^= (unsigned)((n >> 24) & 0xFF);
    h *= 16777619u;
    for (i = 0; i < n; i++) {
        int ln, k;
        if (!gm_need(&c, 4)) return 0;
        ln = gm_i32(&c);
        if (ln < 1 || ln > 1024 || !gm_need(&c, (unsigned)ln)) return 0;
        for (k = 0; k < ln - 1; k++) {
            h ^= c.p[k];
            h *= 16777619u;
        }
        c.p += (unsigned)ln;
        if (!gm_need(&c, 24 + 4)) return 0;
        c.p += 24;
        {
            unsigned char hm = *c.p;
            c.p += 4;
            if (hm) {
                unsigned ht;
                int tl;
                if (!gm_need(&c, 64 + 4 + 4 + 4)) return 0;
                c.p += 64 + 4 + 4;
                ht = gm_u32(&c);
                if (ht == 1) {
                    if (!gm_need(&c, 4)) return 0;
                    tl = gm_i32(&c);
                    if (tl < 1 || tl > 1024 || !gm_need(&c, (unsigned)tl))
                        return 0;
                    c.p += (unsigned)tl;
                }
            }
        }
    }
    return h;
}

#endif /* GRIDMESH_H */
