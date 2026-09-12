/* entsnd.h — v1ce per-def creak sound (native Wobbly replication).
 *
 * Native (objdump-verified on test_env/Hamsterball.exe):
 *  - XML parser 0x474E3B: ECX=*(App+0x178)=SoundDevice; push bufs;
 *    push name; EAX=Sound_LoadAndAppend (0x4664F0, ECX=dev + 2 stack
 *    args [name, bufs], RET 8 -> SoundList*). NOTE the shape: ECX +
 *    stack pair = __thiscall typedef, NOT __fastcall (fastcall would
 *    put name in EDX and corrupt the stack on RET 8).
 *  - Wobbly ctor 0x4352d9: ECX=App+0x4E0 slot; EAX=Sound_GetChannel
 *    (0x459810, __thiscall(ECX=slot)->channel); prime 0.0 via 0x459610.
 *  - Wobbly update 0x43A963: vol=min(2*(|dx|+|dy|),1.0); replay via
 *    0x458EE0 (__thiscall(ECX=ch, float vol), RET 4) every frame.
 *
 * Mod mapping: `"sound": "<Name>"` per def loads `sounds\<Name>`
 * (.ogg/.wav auto-tried by the leaf) with 10 buffers through the same
 * LoadAndAppend chain; channel cached per def. Defs without it share
 * the creakyplatform channel. Custom load attempted EXACTLY once per
 * def (the leaf appends even on missing file — retrying leaks lists);
 * failure falls back to creakyplatform, logged. Never released
 * (acquire-once, zero teardown surface).
 */
#define SND_LOADAPPEND  0x004664F0   /* thiscall(dev,name,bufs)->list,RET8 */
#define SND_GETCHANNEL  0x00459810   /* __thiscall(slot)->channel */
#define SND_PLAYVOL     0x00458EE0   /* __thiscall(ch, float vol), RET 4 */
#define SND_PRIME       0x00459610   /* __thiscall(ch, float), RET 4 */
#define SND_CREAK_SLOT  0x4E0        /* App+0x4E0 = creakyplatform */
#define SND_BOARD_APP   0x878        /* board+0x878 = App */
#define SND_APP_DEV     0x178        /* App+0x178 = SoundDevice */
#define SND_CUSTOM_BUFS 10

typedef DWORD (__thiscall *snd_loadappend_t)(DWORD dev,
                                             const char* name, int bufs);
typedef DWORD (__thiscall *snd_getch_t)(DWORD slot);
typedef void (__thiscall *snd_playvol_t)(DWORD ch, float vol);

static DWORD g_snd_defch = 0;              /* shared creakyplatform ch */
static DWORD g_snd_ch[ENT_MAX_DEFS];       /* per-def channel (0=pending) */
static DWORD g_snd_list[ENT_MAX_DEFS];     /* v1cf: per-def custom list ptr (0=none/fail) */
static int   g_snd_tried[ENT_MAX_DEFS];    /* custom load attempted */
static float g_snd_sm[ENT_MAX_DEFS];       /* smoothed vol per def */
static int   g_snd_heard[ENT_MAX_DEFS];    /* edge-log state per def */

/* App from board, guarded. Returns 0 when unavailable. */
static DWORD snd_board_app(DWORD board) {
    DWORD app;
    if (!board || IsBadReadPtr((void*)(board + SND_BOARD_APP), 4))
        return 0;
    app = *(DWORD*)(board + SND_BOARD_APP);
    if (app < 0x10000 || IsBadReadPtr((void*)app, 0x200)) return 0;
    return app;
}

/* Shared default channel (creakyplatform). Retries cheaply till set. */
static void snd_default_acquire(DWORD app) {
    DWORD slot;
    snd_getch_t getch;
    snd_playvol_t prime;
    char sbuf[96];
    if (g_snd_defch) return;
    if (!app || IsBadReadPtr((void*)(app + SND_CREAK_SLOT), 4)) return;
    slot = *(DWORD*)(app + SND_CREAK_SLOT);
    if (slot < 0x10000 || IsBadReadPtr((void*)slot, 0x20)) return;
    getch = (snd_getch_t)(DWORD)SND_GETCHANNEL;
    g_snd_defch = getch(slot);
    if (g_snd_defch < 0x10000 ||
        IsBadReadPtr((void*)g_snd_defch, 0x20)) {
        g_snd_defch = 0;
        return;
    }
    prime = (snd_playvol_t)(DWORD)SND_PRIME;
    prime(g_snd_defch, 0.0f);
    snprintf(sbuf, sizeof(sbuf),
             "  SND: default creak ch=0x%08X (creakyplatform)", g_snd_defch);
    log_mod(sbuf);
}

/* One custom load for def d. Exactly-once (g_snd_tried); fail = default. */
static void snd_custom_once(DWORD app, int d) {
    DWORD dev, list, ch;
    snd_loadappend_t load;
    snd_getch_t getch;
    snd_playvol_t prime;
    char path[128];
    char sbuf[160];
    int ni = 0;
    const char* nm;
    if (g_snd_tried[d]) return;
    g_snd_tried[d] = 1;
    g_snd_ch[d] = g_snd_defch;   /* fallback pre-set (may be 0, retried) */
    g_snd_list[d] = 0;
    if (!app || IsBadReadPtr((void*)(app + SND_APP_DEV), 4)) return;
    dev = *(DWORD*)(app + SND_APP_DEV);
    if (dev < 0x10000 || IsBadReadPtr((void*)dev, 0x20)) return;
    /* sounds\<name>, no ext (leaf appends .ogg/.wav); strip ext if given */
    nm = g_ent_snd[d];
    path[ni++] = 's'; path[ni++] = 'o'; path[ni++] = 'u';
    path[ni++] = 'n'; path[ni++] = 'd'; path[ni++] = 's';
    path[ni++] = '\\';
    while (ni < 100 && nm[0]) {
        path[ni++] = nm[0];
        nm++;
    }
    path[ni] = '\0';
    /* strip a trailing .ogg/.wav the user may have typed */
    if (ni > 4 && path[ni - 4] == '.') ni -= 4;
    path[ni] = '\0';
    if (ni <= 7) return;
    load = (snd_loadappend_t)(DWORD)SND_LOADAPPEND;
    list = load(dev, path, SND_CUSTOM_BUFS);
    if (list < 0x10000 || IsBadReadPtr((void*)list, 0x20)) {
        snprintf(sbuf, sizeof(sbuf),
                 "  SND: def %s custom %s FAILED, default fallback",
                 g_ent_name[d], path);
        log_mod(sbuf);
        return;
    }
    getch = (snd_getch_t)(DWORD)SND_GETCHANNEL;
    ch = getch(list);
    if (ch < 0x10000 || IsBadReadPtr((void*)ch, 0x20)) {
        snprintf(sbuf, sizeof(sbuf),
                 "  SND: def %s custom %s no channel (missing file?), fallback",
                 g_ent_name[d], path);
        log_mod(sbuf);
        return;
    }
    prime = (snd_playvol_t)(DWORD)SND_PRIME;
    prime(ch, 0.0f);
    g_snd_ch[d] = ch;
    g_snd_list[d] = list;   /* v1cf: keep list ptr for Tarpit one-shot replay */
    snprintf(sbuf, sizeof(sbuf), "  SND: def %s custom %s ch=0x%08X",
             g_ent_name[d], path, ch);
    log_mod(sbuf);
}

/* Per-board entry: default channel + one custom load per named def.
 * Custom loads run once ever (g_snd_tried); default ch refresh points
 * plain defs at it (covers default arriving after a custom fallback). */
static void snd_level_acquire(DWORD board) {
    DWORD app;
    int d;
    app = snd_board_app(board);
    if (!app) return;
    snd_default_acquire(app);
    for (d = 0; d < g_ent_count && d < ENT_MAX_DEFS; d++) {
        if (!g_ent_snd[d][0]) {
            g_snd_ch[d] = g_snd_defch;
            continue;
        }
        snd_custom_once(app, d);
        if (!g_ent_snd[d][0]) g_snd_ch[d] = g_snd_defch;
    }
}

/* Per-def replay, v1cc shape: fire ONLY while audible; still = hands
 * off, tail rings out; ~12f smoothed release bridges flicker. */
static void snd_creak_frame_d(int d, float fravel) {
    snd_playvol_t play;
    DWORD ch;
    float vol, sm;
    char sbuf[64];
    if (d < 0 || d >= ENT_MAX_DEFS) return;
    ch = g_snd_ch[d];
    if (!ch) ch = g_snd_defch;   /* default arrived late */
    if (ch < 0x10000 || IsBadReadPtr((void*)ch, 0x20)) return;
    g_snd_ch[d] = ch;
    vol = fravel * 60.0f;
    if (vol < 0.0f) vol = 0.0f;
    if (vol > 1.0f) vol = 1.0f;
    sm = g_snd_sm[d];
    if (vol > sm) {
        sm = vol;                    /* instant attack */
    } else if (sm > 0.0f) {
        sm -= 0.08f;                 /* ~12-frame settle tail */
        if (sm < 0.0f) sm = 0.0f;
    }
    g_snd_sm[d] = sm;
    if (sm < 0.02f) {                /* still: hands off, let it ring */
        if (g_snd_heard[d]) {
            g_snd_heard[d] = 0;
            g_snd_sm[d] = 0.0f;
            snprintf(sbuf, sizeof(sbuf), "  SND: def %s stop (ring out)",
                     g_ent_name[d]);
            log_mod(sbuf);
        }
        return;
    }
    play = (snd_playvol_t)(DWORD)SND_PLAYVOL;
    play(ch, sm);
    if (!g_snd_heard[d]) {
        g_snd_heard[d] = 1;
        snprintf(sbuf, sizeof(sbuf), "  SND: def %s start vol=%f.2",
                 g_ent_name[d], sm);
        log_mod(sbuf);
    }
}
