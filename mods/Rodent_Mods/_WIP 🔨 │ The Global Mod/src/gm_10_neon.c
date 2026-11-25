/* Chapter 10 - Neon glow follower lights (scene-loader tail instance #2) ...
 * ... (LevelFeatures.c aggregator includes this file last, single TU) ...
 *
 * Native Neon scene loader (0x416270+) builds TWO Light_ctor (0x46B4F0)
 * objects at board+0x436C (P1) / board+0x4370 (P2, skipped in single-player
 * via App+0x677), emitter (10,10,0), Range 400.0, TYPE POINT (+0xD0=1),
 * registers gfx slots 0/1 (0x453BD0), and positions them at
 * ball+(+20,+30,-20) every frame from Neon RaceState (0x424790).
 * LIGHTSOFF/ON dispatch toggles vtable[4](0/1) on board+0x436C[playerIdx].
 * Disasm-verified via objdump 2026-09-10 (see README v13.1).
 *
 * RODENTRACER RULE (2026-11-10, marker 2026-11-19): the glow builds when the
 * level MESHWORLD has NO S3 light of its own (stock Neon/LevelDark, count 0
 * → native yellow default), OR when any type-0 S3 light has pos=(0,0,0) AND
 * target=(0,0,0) — a MARKER, not a real light: the glow builds and the
 * marker's up-triple becomes the glow color (0.0-1.0 each, clamped).
 * Files with real S3 lights and no marker keep native lighting untouched.
 * Custom authors opt in either by shipping zero S3 lights (yellow) or by
 * adding one marker light (any color).
 * Gate flag lives per-board at ext+UNI_NEON_GLOW_ACTIVE; the S3 verdict is
 * cached at ext+UNI_NEON_HAS_NATIVE.
 */

/* Light_ctor 0x46B4F0: __thiscall/mem on ECX... second arg (gfx) on stack. */
typedef void *(__thiscall *Light_ctor_t)(void *mem, void *gfx);
static Light_ctor_t s_NeonLightCtor = NULL;

static int Neon_IsActive(void *board) {
    void *ext = GetBoardExt(board);
    if (!ext) return 0;
    if (!ExtHasOffset(ext, UNI_NEON_GLOW_ACTIVE, 1)) return 0;
    return *(BYTE *)((char *)ext + UNI_NEON_GLOW_ACTIVE) ? 1 : 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Pure S3 light counter over a MESHWORLD image. No WinAPI — the /tmp
 * harness compiles this same function against real level files.
 * Returns light count (>=0), or -1 on malformed input.
 * S1 skip uses the game rule: has_mat is a FIRST-BYTE check (& 0xFF).
 * ═══════════════════════════════════════════════════════════════════════════ */
static int Neon_CountS3Lights(const unsigned char *buf, unsigned int len) {
    unsigned int pos = 0;
    unsigned int i;
    unsigned int rpc, sc, lc;
    if (!buf || len < 4) return -1;
    rpc = (unsigned int)buf[0] | ((unsigned int)buf[1] << 8) |
          ((unsigned int)buf[2] << 16) | ((unsigned int)buf[3] << 24);
    pos = 4;
    if (rpc > 65536) return -1;
    for (i = 0; i < rpc; i++) {
        unsigned int slen, has_mat, has_tex;
        if (pos + 4 > len) return -1;
        slen = (unsigned int)buf[pos] | ((unsigned int)buf[pos+1] << 8) |
               ((unsigned int)buf[pos+2] << 16) | ((unsigned int)buf[pos+3] << 24);
        if (slen < 1 || slen > 512) return -1;
        pos += 4;
        if (pos + slen > len) return -1;
        pos += slen;
        if (pos + 24 + 4 > len) return -1;
        pos += 24;
        has_mat = (unsigned int)buf[pos] | ((unsigned int)buf[pos+1] << 8) |
                  ((unsigned int)buf[pos+2] << 16) | ((unsigned int)buf[pos+3] << 24);
        pos += 4;
        if (has_mat & 0xFF) {
            if (pos + 64 + 4 + 4 + 4 > len) return -1;
            pos += 64 + 4 + 4; /* 16 color floats + power + has_refl */
            has_tex = (unsigned int)buf[pos] | ((unsigned int)buf[pos+1] << 8) |
                      ((unsigned int)buf[pos+2] << 16) | ((unsigned int)buf[pos+3] << 24);
            pos += 4;
            if (has_tex) {
                if (pos + 4 > len) return -1;
                slen = (unsigned int)buf[pos] | ((unsigned int)buf[pos+1] << 8) |
                       ((unsigned int)buf[pos+2] << 16) | ((unsigned int)buf[pos+3] << 24);
                if (slen < 1 || slen > 512) return -1;
                pos += 4;
                if (pos + slen > len) return -1;
                pos += slen;
            }
        }
    }
    /* S2 splines: count + per-spline (string, point count, 12B per point) */
    if (pos + 4 > len) return -1;
    sc = (unsigned int)buf[pos] | ((unsigned int)buf[pos+1] << 8) |
         ((unsigned int)buf[pos+2] << 16) | ((unsigned int)buf[pos+3] << 24);
    pos += 4;
    if (sc > 4096) return -1;
    for (i = 0; i < sc; i++) {
        unsigned int slen, pc;
        if (pos + 4 > len) return -1;
        slen = (unsigned int)buf[pos] | ((unsigned int)buf[pos+1] << 8) |
               ((unsigned int)buf[pos+2] << 16) | ((unsigned int)buf[pos+3] << 24);
        if (slen < 1 || slen > 512) return -1;
        pos += 4;
        if (pos + slen + 4 > len) return -1;
        pos += slen;
        pc = (unsigned int)buf[pos] | ((unsigned int)buf[pos+1] << 8) |
             ((unsigned int)buf[pos+2] << 16) | ((unsigned int)buf[pos+3] << 24);
        pos += 4;
        if (pc > 65536) return -1;
        if (pos + pc * 12 > len) return -1;
        pos += pc * 12;
    }
    /* S3 lights: uint32 count. Any entry (type 0 or not) = authored light. */
    if (pos + 4 > len) return -1;
    lc = (unsigned int)buf[pos] | ((unsigned int)buf[pos+1] << 8) |
         ((unsigned int)buf[pos+2] << 16) | ((unsigned int)buf[pos+3] << 24);
    if (lc > 4096) return -1;
    return (int)lc;
}

/* RODENTRACER RULE (2026-11-19): the S3 marker light. A type-0 S3 light with
 * pos=(0,0,0) AND target=(0,0,0) is NOT a real light — it is a glow marker:
 * LFL builds its followers and takes the MARKER's up-triple as the glow
 * color (0.0-1.0 each, clamped). Stock zero-light files keep the native
 * yellow default. Files with real S3 lights and no marker keep native.
 * Marker + real lights: marker wins for the glow (author opted in).
 */
/* 1 = marker found (color out, clamped 0..1), 0 = none, -1 = malformed. */
static int Neon_ScanMarker(const unsigned char *buf, unsigned int len,
                           float *pr, float *pg, float *pb) {
    unsigned int pos = 0, i;
    unsigned int rpc, sc, lc;
    if (!buf || len < 4) return -1;
    rpc = (unsigned int)buf[0] | ((unsigned int)buf[1] << 8) |
          ((unsigned int)buf[2] << 16) | ((unsigned int)buf[3] << 24);
    pos = 4;
    if (rpc > 65536) return -1;
    for (i = 0; i < rpc; i++) {
        unsigned int slen, has_mat, has_tex;
        if (pos + 4 > len) return -1;
        slen = (unsigned int)buf[pos] | ((unsigned int)buf[pos+1] << 8) |
               ((unsigned int)buf[pos+2] << 16) | ((unsigned int)buf[pos+3] << 24);
        if (slen < 1 || slen > 512) return -1;
        pos += 4;
        if (pos + slen > len) return -1;
        pos += slen;
        if (pos + 24 + 4 > len) return -1;
        pos += 24;
        has_mat = (unsigned int)buf[pos] | ((unsigned int)buf[pos+1] << 8) |
                  ((unsigned int)buf[pos+2] << 16) | ((unsigned int)buf[pos+3] << 24);
        pos += 4;
        if (has_mat & 0xFF) {
            if (pos + 64 + 4 + 4 + 4 > len) return -1;
            pos += 64 + 4 + 4;
            has_tex = (unsigned int)buf[pos] | ((unsigned int)buf[pos+1] << 8) |
                      ((unsigned int)buf[pos+2] << 16) | ((unsigned int)buf[pos+3] << 24);
            pos += 4;
            if (has_tex) {
                if (pos + 4 > len) return -1;
                slen = (unsigned int)buf[pos] | ((unsigned int)buf[pos+1] << 8) |
                       ((unsigned int)buf[pos+2] << 16) | ((unsigned int)buf[pos+3] << 24);
                if (slen < 1 || slen > 512) return -1;
                pos += 4;
                if (pos + slen > len) return -1;
                pos += slen;
            }
        }
    }
    if (pos + 4 > len) return -1;
    sc = (unsigned int)buf[pos] | ((unsigned int)buf[pos+1] << 8) |
         ((unsigned int)buf[pos+2] << 16) | ((unsigned int)buf[pos+3] << 24);
    pos += 4;
    if (sc > 4096) return -1;
    for (i = 0; i < sc; i++) {
        unsigned int slen, pc;
        if (pos + 4 > len) return -1;
        slen = (unsigned int)buf[pos] | ((unsigned int)buf[pos+1] << 8) |
               ((unsigned int)buf[pos+2] << 16) | ((unsigned int)buf[pos+3] << 24);
        if (slen < 1 || slen > 512) return -1;
        pos += 4;
        if (pos + slen + 4 > len) return -1;
        pos += slen;
        pc = (unsigned int)buf[pos] | ((unsigned int)buf[pos+1] << 8) |
             ((unsigned int)buf[pos+2] << 16) | ((unsigned int)buf[pos+3] << 24);
        pos += 4;
        if (pc > 65536) return -1;
        if (pos + pc * 12 > len) return -1;
        pos += pc * 12;
    }
    if (pos + 4 > len) return -1;
    lc = (unsigned int)buf[pos] | ((unsigned int)buf[pos+1] << 8) |
         ((unsigned int)buf[pos+2] << 16) | ((unsigned int)buf[pos+3] << 24);
    pos += 4;
    if (lc > 4096) return -1;
    for (i = 0; i < lc; i++) {
        int ltype;
        float px, py, pz, tx, ty, tz, ux, uy, uz;
        if (pos + 4 > len) return -1;
        ltype = (int)((unsigned int)buf[pos] | ((unsigned int)buf[pos+1] << 8) |
                ((unsigned int)buf[pos+2] << 16) | ((unsigned int)buf[pos+3] << 24));
        pos += 4;
        if (ltype != 0) continue;
        if (pos + 36 > len) return -1;
        px = *(float *)(buf + pos); py = *(float *)(buf + pos + 4); pz = *(float *)(buf + pos + 8);
        tx = *(float *)(buf + pos + 12); ty = *(float *)(buf + pos + 16); tz = *(float *)(buf + pos + 20);
        ux = *(float *)(buf + pos + 24); uy = *(float *)(buf + pos + 28); uz = *(float *)(buf + pos + 32);
        pos += 36;
        if (px == 0.0f && py == 0.0f && pz == 0.0f &&
            tx == 0.0f && ty == 0.0f && tz == 0.0f) {
            if (ux != ux || uy != uy || uz != uz) continue; /* NaN: not a marker */
            *pr = ux < 0.0f ? 0.0f : (ux > 1.0f ? 1.0f : ux);
            *pg = uy < 0.0f ? 0.0f : (uy > 1.0f ? 1.0f : uy);
            *pb = uz < 0.0f ? 0.0f : (uz > 1.0f ? 1.0f : uz);
            return 1;
        }
    }
    return 0;
}

/* Ball tint (8-ball fix 2026-11-19, channel fix same day): helper 0x4011C0
 * builds the ball color struct via 0x453150 from its three push-imm32s plus
 * the caller's 4th arg (ball+0x2FC/0xC60 alpha). The pushes fill the struct
 * in REVERSE: last push (0x4011D5) -> +0x7B0, middle (0x4011D0) -> +0x7B4,
 * first (0x4011C9) -> +0x7B8 — and the mesh reads +0x7B0 as RED. Writing
 * (r,g,b) in push order put b=0 into +0x7B0 for yellow = green hamster on
 * real Windows (user-proven). Correct order: C9=b, D0=g, D5=r. Exe .text
 * never persists (fresh image each launch), so PostSetup apply/restore
 * covers every cross-level transition. Idempotent + verify-or-skip. */
static void Neon_PatchSpriteColor(float r, float g, float b) {
    static const DWORD kImm[3] = { 0x4011CA, 0x4011D1, 0x4011D6 };
    int i, need = 0, ok = 1;
    char dbg[128];
    for (i = 0; i < 3; i++) {
        DWORD cur;
        float tf = (i == 0) ? b : (i == 1) ? g : r;
        if (IsBadReadPtr((void *)kImm[i], 4)) return;
        cur = *(DWORD *)kImm[i];
        if (cur != 0x3F800000) {
            if (cur != *(DWORD *)&tf) { ok = 0; break; }
        } else need = 1;
    }
    if (!ok) {
        DebugLog("Neon: sprite-color site unexpected, leaving stock white");
        return;
    }
    for (i = 0; i < 3; i++) {
        float tf = (i == 0) ? b : (i == 1) ? g : r;
        DWORD bits = *(DWORD *)&tf;
        DWORD old;
        SIZE_T wr = 0;
        if (*(DWORD *)kImm[i] == bits) continue;
        VirtualProtect((void *)(kImm[i]), 4, PAGE_EXECUTE_READWRITE, &old);
        WriteProcessMemory(GetCurrentProcess(), (void *)(kImm[i]),
                           &bits, 4, &wr);
        VirtualProtect((void *)(kImm[i]), 4, old, &old);
    }
    wsprintfA(dbg, "Neon: sprite-ball color -> (%.2f,%.2f,%.2f)%s",
              r, g, b, need ? "" : " (already set)");
    DebugLog(dbg);
}

/* Returns 1 = file has >=1 S3 light (or unreadable/malformed: conservative
 * skip). Marker out: 1 = S3 marker light found (glow color in mr/mg/mb),
 * 0 = none, -1 = malformed/unreadable (treated as native). */
static int Neon_FileHasLights(const char *meshPath, int *marker,
                              float *mr, float *mg, float *mb) {
    char exe[MAX_PATH], cand[MAX_PATH];
    int elen = 0, i;
    HANDLE h;
    DWORD fsize, br;
    char *buf;
    int lc;
    char dbg[384];
    if (!meshPath || !meshPath[0]) return 1;
    GetModuleFileNameA(NULL, exe, MAX_PATH);
    exe[MAX_PATH-1] = '\0';
    elen = 0; while (exe[elen]) elen++;
    while (elen > 0 && exe[elen-1] != '\\' && exe[elen-1] != '/') elen--;
    /* Candidate: <exedir>\<meshPath>.MESHWORLD (ResolveRacePath strips ext). */
    for (i = 0; i < 2; i++) {
        int p = 0, k;
        for (k = 0; k < elen && p < MAX_PATH-1; k++) cand[p++] = exe[k];
        for (k = 0; meshPath[k] && p < MAX_PATH-1; k++) cand[p++] = meshPath[k];
        if (i == 0) {
            const char *ext = ".MESHWORLD";
            for (k = 0; ext[k] && p < MAX_PATH-1; k++) cand[p++] = ext[k];
        }
        cand[p] = '\0';
        h = CreateFileA(cand, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h == INVALID_HANDLE_VALUE) continue;
        fsize = GetFileSize(h, NULL);
        if (fsize == 0 || fsize == (DWORD)-1 || fsize > 0x2000000) {
            CloseHandle(h);
            continue;
        }
        buf = (char *)HeapAlloc(GetProcessHeap(), 0, fsize);
        if (!buf) { CloseHandle(h); return 1; }
        if (!ReadFile(h, buf, fsize, &br, NULL) || br != fsize) {
            HeapFree(GetProcessHeap(), 0, buf);
            CloseHandle(h);
            continue;
        }
        CloseHandle(h);
        lc = Neon_CountS3Lights((unsigned char *)buf, fsize);
        if (lc >= 0) {
            int mk = Neon_ScanMarker((unsigned char *)buf, fsize, mr, mg, mb);
            if (marker) *marker = mk;
            HeapFree(GetProcessHeap(), 0, buf);
            if (mk == 1) {
                wsprintfA(dbg, "Neon S3 scan: '%s' MARKER glow=(%.2f,%.2f,%.2f)",
                          cand, *mr, *mg, *mb);
                DebugLog(dbg);
                return 0;
            }
            wsprintfA(dbg, "Neon S3 scan: '%s' lights=%d -> %s",
                      cand, lc, lc > 0 ? "native (glow skipped)" : "none (glow builds)");
            DebugLog(dbg);
            return lc > 0 ? 1 : 0;
        }
        if (marker) *marker = -1;
        HeapFree(GetProcessHeap(), 0, buf);
        {
            wsprintfA(dbg, "Neon S3 scan: '%s' malformed, conservative skip", cand);
            DebugLog(dbg);
            return 1;
        }
    }
    if (marker) *marker = -1;
    wsprintfA(dbg, "Neon S3 scan: '%s' unreadable, conservative skip", meshPath);
    DebugLog(dbg);
    return 1;
}

/* Native Neon ball setup (scene loader 0x416556+, disasm-verified 2026-11-19):
 * pushes RGBA (1,1,0,1.0), then ball+0x1D0=1.0, ball+0x1D4=0.0,
 * ball+0x1CC=1.0(bits), ball+0x1D8=1.0 — i.e. a (1,1,0,1) yellow quad at
 * ball+0x1CC — plus the same (1,1,0,1) at ball+0x1BC (0x4165C8 block) and
 * ball+0x1EC (0x416632 block): three quads total, all yellow. Render draws
 * the +0x1B8-based (quadA) sprite, so all three must be written. */
/* Ball quads are RGBA in order: +0x1CC=R, +0x1D0=G, +0x1D4=B, +0x1D8=A(1.0).
 * Native Neon loader writes ALL THREE ball quads (1,1,0,1): quadA +0x1BC,
 * quadB +0x1CC, quadC +0x1EC (objdump 0x41655B/0x4165C8/0x416632). Render
 * draws the quadA sprite (lea 0x1B8 -> 0x45D660 at 0x4042AE) — writing only
 * quadB left quadA at universal-ctor defaults = green hamster (red lost).
 * Mirror all three, exactly like native. */
static void Neon_BallGlow(DWORD ball, int pnum, float r, float g, float b) {
    char dbg[96];
    if (!ball || IsBadReadPtr((void *)ball, 0x210)) return;
    *(float *)(ball + 0x1BC) = r;
    *(float *)(ball + 0x1C0) = g;
    *(float *)(ball + 0x1C4) = b;
    *(float *)(ball + 0x1C8) = 1.0f;
    *(float *)(ball + 0x1CC) = r;
    *(float *)(ball + 0x1D0) = g;
    *(float *)(ball + 0x1D4) = b;
    *(float *)(ball + 0x1D8) = 1.0f;
    *(float *)(ball + 0x1EC) = r;
    *(float *)(ball + 0x1F0) = g;
    *(float *)(ball + 0x1F4) = b;
    *(float *)(ball + 0x1F8) = 1.0f;
    wsprintfA(dbg, "Neon: ball glow written (P%d ball=0x%08X)", pnum, ball);
    DebugLog(dbg);
}

/* Build ONE follower light. Native values: emitter (10,10,0), Range 400. */
static void *Neon_BuildOneLight(void *board, void *gfx, DWORD ball,
                                int slot, DWORD boardOff,
                                float r, float g, float b) {
    void *mem, *light;
    float bx, by, bz;
    char dbg[128];
    if (!s_NeonLightCtor) {
        if (!g_moduleBase) return NULL;
        s_NeonLightCtor = (Light_ctor_t)(g_moduleBase + RVA_Light_ctor);
    }
    if (!g_operatorNew || !g_SceneRegisterObject || !s_NeonLightCtor) return NULL;
    if (!ball || IsBadReadPtr((void *)ball, 0x210)) return NULL;
    if (!BoardHasOffset(board, boardOff, 4)) return NULL;
    mem = g_operatorNew(0xD4);
    if (!mem || IsBadReadPtr(mem, 0xD4)) return NULL;
    light = s_NeonLightCtor(mem, gfx);
    if (!light || IsBadReadPtr(light, 0xD4)) return NULL;
    {
        DWORD old = *(DWORD *)((char *)board + boardOff);
        if (old) {
            wsprintfA(dbg, "Neon: board+0x%X had 0x%08X, overwriting with light",
                      boardOff, old);
            DebugLog(dbg);
        }
    }
    *(DWORD *)((char *)board + boardOff) = (DWORD)light;
    *(int *)((char *)light + 0xD0) = 1;            /* TYPE POINT */
    *(float *)((char *)light + 0x94) = r * 10.0f;  /* emitter = color x10 */
    *(float *)((char *)light + 0x98) = g * 10.0f;
    *(float *)((char *)light + 0x9C) = b * 10.0f;
    *(float *)((char *)light + 0xA0) = 1.0f;
    bx = *(float *)((char *)ball + 0x164);
    by = *(float *)((char *)ball + 0x168);
    bz = *(float *)((char *)ball + 0x16C);
    /* Direct writes first (Wine-safe), then native vtable[1] SetPosition. */
    *(float *)((char *)light + 0x08) = bx + 20.0f;
    *(float *)((char *)light + 0x0C) = by + 30.0f;
    *(float *)((char *)light + 0x10) = bz - 20.0f;
    {
        DWORD *vtbl = *(DWORD **)light;
        if (vtbl && !IsBadReadPtr(vtbl, 8)) {
            void (__thiscall *setPos)(DWORD, float, float, float) =
                (void (__thiscall *)(DWORD, float, float, float))vtbl[1];
            if (setPos) setPos((DWORD)light, bx + 20.0f, by + 30.0f, bz - 20.0f);
        }
    }
    *(float *)((char *)light + 0xCC) = 400.0f;    /* D3D Range */
    g_SceneRegisterObject(gfx, slot, (int *)light);
    wsprintfA(dbg, "Neon: built P%d light obj=0x%08X gfxslot=%d",
              slot + 1, (DWORD)light, slot);
    DebugLog(dbg);
    return light;
}

/* BadBall glow gate (2026-11-19): native Neon scene loader (0x41631D loop
 * over board+0x2DEC) sets ball+0xC80=1 on EVERY ball incl the home BadBall;
 * BadBall render v[2] (0x402DE0) draws its emission + outline sprites ONLY
 * when +0xC80!=0. LFL only armed players, so the BadBall renders black.
 * Mirror native for BadBall-class balls (vtable 0x4CF3A0; never touch
 * player-class balls — +0xC80 is a live pointer for 0x4CF314). */
#define BADBALL_VTABLE 0x4CF3A0
static void Neon_BadBallGlow(void *board, float r, float g, float b) {
    int n, i;
    DWORD *items;
    if (!board || !g_AthenaListGetSize) return;
    n = g_AthenaListGetSize((void *)((char *)board + UNI_BALL_LIST));
    if (n <= 0 || n > 64) return;
    items = *(DWORD **)((char *)board + UNI_BALL_LIST + 0x40C);
    if (!items || IsBadReadPtr(items, n * 4)) return;
    for (i = 0; i < n; i++) {
        DWORD bb = items[i];
        DWORD vt;
        if (!bb || IsBadReadPtr((void *)bb, 0xD00)) continue;
        vt = *(DWORD *)bb;
        if (vt != BADBALL_VTABLE) continue;
        if (*(BYTE *)(bb + 0xC80)) continue;  /* already armed */
        *(float *)(bb + 0xC88) = r;
        *(float *)(bb + 0xC8C) = g;
        *(float *)(bb + 0xC90) = b;
        *(BYTE *)(bb + 0xC80) = 1;
        /* Native loader (0x416353+) bakes RenderQuad params via two
         * 0x453150 calls; stock-Neon outputs recorded live from vanilla
         * (quadA +0x1BC..+0x1C8, quadB +0x1EC..+0x1F8, flag +0x204).
         * Without these the glow/border sprites draw degenerate
         * (invisible) even with the gate set. Hardcoded = stock derivation;
         * marker-colored levels need the call version if ever reported. */
        *(float *)(bb + 0x1BC) = 1.0f;
        *(float *)(bb + 0x1C0) = 1.0f;
        *(float *)(bb + 0x1C4) = 0.0f;
        *(float *)(bb + 0x1C8) = 1.0f;
        *(float *)(bb + 0x1EC) = 1.0f;
        *(float *)(bb + 0x1F0) = 1.0f;
        *(float *)(bb + 0x1F4) = 0.0f;
        *(float *)(bb + 0x1F8) = 1.0f;
        *(BYTE *)(bb + 0x204) = 0;
        DebugLog("Neon: BadBall glow armed");
    }
}

static void Neon_BuildLights(void *board, void *ext,
                             float r, float g, float b) {
    DWORD app, ball1;
    void *gfx;
    void *l1;
    if (!board || !ext) return;
    app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x680)) {
        DebugLog("Neon: bad app, build skipped");
        return;
    }
    gfx = *(void **)((char *)app + 0x174);
    if (!gfx) { DebugLog("Neon: no gfx, build skipped"); return; }
    ball1 = *(DWORD *)(app + APP_BALL_PTR);
    l1 = Neon_BuildOneLight(board, gfx, ball1, 0, 0x436C, r, g, b);
    if (l1) Neon_BallGlow(ball1, 1, r, g, b);
    if (!*(char *)(app + 0x677)) {
        DWORD b2 = *(DWORD *)(app + 0x67C);
        if (Neon_BuildOneLight(board, gfx, b2, 1, 0x4370, r, g, b))
            Neon_BallGlow(b2, 2, r, g, b);
    } else {
        if (BoardHasOffset(board, 0x4370, 4))
            *(DWORD *)((char *)board + 0x4370) = 0;
        DebugLog("Neon: single-player, P2 light skipped");
    }
    /* Arm pre-placed BadBalls (Neon home BadBall): native lights every
     * ball, LFL only did players. Vtable-gated, players untouched. */
    Neon_BadBallGlow(board, r, g, b);
    if (l1) {
        if (ExtHasOffset(ext, UNI_NEON_GLOW_ACTIVE, 1))
            *(BYTE *)((char *)ext + UNI_NEON_GLOW_ACTIVE) = 1;
        OrBoardFeat(board, FEAT_NEON_CAM);
        DebugLog("Neon: ACTIVE (FEAT_NEON_CAM set)");
    } else {
        DebugLog("Neon: P1 build failed, staying inactive");
    }
}

/* Step 6c entry — called from UniversalConstructor after Pennant_PostSetup. */
static void Neon_PostSetup(void *board, void *ext, int raceIndex, const char *meshPath) {
    int has, marker = 0;
    float mr = 1.0f, mg = 1.0f, mb = 0.0f;
    char dbg[384];
    if (!board || !ext || !meshPath || !meshPath[0]) return;
    /* Every load starts inactive: ext can be a stale heap-reused entry from
     * a previous board at the same address (retry path), so a leftover 1
     * would aim per-frame code at garbage board slots. Build sets it below. */
    if (ExtHasOffset(ext, UNI_NEON_GLOW_ACTIVE, 1))
        *(BYTE *)((char *)ext + UNI_NEON_GLOW_ACTIVE) = 0;
    has = Neon_FileHasLights(meshPath, &marker, &mr, &mg, &mb);
    if (ExtHasOffset(ext, UNI_NEON_HAS_NATIVE, 1))
        *(BYTE *)((char *)ext + UNI_NEON_HAS_NATIVE) = has ? 1 : 0;
    if (marker == 1) {
        /* S3 marker light: glow with the marker's up-triple as color. */
        wsprintfA(dbg, "Neon: '%s' MARKER glow=(%.2f,%.2f,%.2f), building followers",
                  meshPath, mr, mg, mb);
        DebugLog(dbg);
        Neon_BuildLights(board, ext, mr, mg, mb);
        Neon_PatchSpriteColor(mr, mg, mb);
        return;
    }
    if (has) {
        /* Native lights win. On the Neon slot only, NULL the follower slots
         * so no reader derefs stale garbage (other slots: Dizzy tipper at
         * 0x436C / Up lifter list — never touch those). */
        if (GetCurrentLevel(board) == 7 && BoardHasOffset(board, 0x436C, 8)) {
            *(DWORD *)((char *)board + 0x436C) = 0;
            *(DWORD *)((char *)board + 0x4370) = 0;
        }
        wsprintfA(dbg, "Neon: '%s' has native S3 light(s), glow skipped", meshPath);
        DebugLog(dbg);
        Neon_PatchSpriteColor(1.0f, 1.0f, 1.0f);
        return;
    }
    wsprintfA(dbg, "Neon: '%s' has no S3 lights, building followers", meshPath);
    DebugLog(dbg);
    Neon_BuildLights(board, ext, 1.0f, 1.0f, 0.0f);
    Neon_PatchSpriteColor(1.0f, 1.0f, 0.0f);
}
