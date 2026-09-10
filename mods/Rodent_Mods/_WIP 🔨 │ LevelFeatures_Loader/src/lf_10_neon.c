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
 * RODENTRACER RULE (2026-09-10): ALL of this neon code runs ONLY when the
 * level MESHWORLD has NO S3 light object of its own. Stock Neon
 * (LevelDark.MESHWORLD, S3 count 0) gets followers; every other stock race
 * (S3 count 1, Odd has 2) keeps its native file lights with zero
 * double-lighting. Custom authors opt out by adding any S3 light.
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

/* 1 = file has >=1 S3 light (or unreadable/malformed: conservative skip). */
static int Neon_FileHasLights(const char *meshPath) {
    char exe[MAX_PATH], cand[MAX_PATH];
    int elen = 0, i;
    HANDLE h;
    DWORD fsize, br;
    char *buf;
    int lc;
    char dbg[256];
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
        HeapFree(GetProcessHeap(), 0, buf);
        if (lc < 0) {
            wsprintfA(dbg, "Neon S3 scan: '%s' malformed, conservative skip", cand);
            DebugLog(dbg);
            return 1;
        }
        wsprintfA(dbg, "Neon S3 scan: '%s' lights=%d -> %s",
                  cand, lc, lc > 0 ? "native (glow skipped)" : "none (glow builds)");
        DebugLog(dbg);
        return lc > 0 ? 1 : 0;
    }
    wsprintfA(dbg, "Neon S3 scan: '%s' unreadable, conservative skip", meshPath);
    DebugLog(dbg);
    return 1;
}

/* Minimal ball glow (mkn_level_system v6.7-proven values, real-Windows). */
static void Neon_BallGlow(DWORD ball, int pnum) {
    char dbg[96];
    if (!ball || IsBadReadPtr((void *)ball, 0x210)) return;
    *(float *)(ball + 0x1D0) = 0.0f;
    *(float *)(ball + 0x1D8) = 1.0f;
    *(float *)(ball + 0x1C0) = 0.0f;
    *(float *)(ball + 0x1F0) = 0.0f;
    *(BYTE *)(ball + 0x204) = 1;
    wsprintfA(dbg, "Neon: ball glow written (P%d ball=0x%08X)", pnum, ball);
    DebugLog(dbg);
}

/* Build ONE follower light. Native values: emitter (10,10,0,1), Range 400. */
static void *Neon_BuildOneLight(void *board, void *gfx, DWORD ball,
                                int slot, DWORD boardOff) {
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
    *(float *)((char *)light + 0x94) = 10.0f;     /* emitter */
    *(float *)((char *)light + 0x98) = 10.0f;
    *(float *)((char *)light + 0x9C) = 0.0f;
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

static void Neon_BuildLights(void *board, void *ext) {
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
    l1 = Neon_BuildOneLight(board, gfx, ball1, 0, 0x436C);
    if (l1) Neon_BallGlow(ball1, 1);
    if (!*(char *)(app + 0x677)) {
        DWORD b2 = *(DWORD *)(app + 0x67C);
        if (Neon_BuildOneLight(board, gfx, b2, 1, 0x4370))
            Neon_BallGlow(b2, 2);
    } else {
        if (BoardHasOffset(board, 0x4370, 4))
            *(DWORD *)((char *)board + 0x4370) = 0;
        DebugLog("Neon: single-player, P2 light skipped");
    }
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
    int has;
    char dbg[192];
    if (!board || !ext || !meshPath || !meshPath[0]) return;
    /* Every load starts inactive: ext can be a stale heap-reused entry from
     * a previous board at the same address (retry path), so a leftover 1
     * would aim per-frame code at garbage board slots. Build sets it below. */
    if (ExtHasOffset(ext, UNI_NEON_GLOW_ACTIVE, 1))
        *(BYTE *)((char *)ext + UNI_NEON_GLOW_ACTIVE) = 0;
    has = Neon_FileHasLights(meshPath);
    if (ExtHasOffset(ext, UNI_NEON_HAS_NATIVE, 1))
        *(BYTE *)((char *)ext + UNI_NEON_HAS_NATIVE) = has ? 1 : 0;
    if (has) {
        /* Native lights win. On the Neon slot only, NULL the follower slots
         * so no reader derefs stale garbage (other slots: Dizzy tipper at
         * 0x436C / Up lifter list — never touch those). */
        if (raceIndex == 7 && BoardHasOffset(board, 0x436C, 8)) {
            *(DWORD *)((char *)board + 0x436C) = 0;
            *(DWORD *)((char *)board + 0x4370) = 0;
        }
        wsprintfA(dbg, "Neon: '%s' has native S3 light(s), glow skipped", meshPath);
        DebugLog(dbg);
        return;
    }
    wsprintfA(dbg, "Neon: '%s' has no S3 lights, building followers", meshPath);
    DebugLog(dbg);
    Neon_BuildLights(board, ext);
}
