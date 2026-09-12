/* entbubcfg.h -- death-bubble sfx probe + pop redirect (v1da).
 * death_bubbles_sfx base: base1 required, base2..base9 probed via
 * Sound_LoadAndAppend 0x4664F0 + GetChannel-valid = file exists (same
 * shape as entsnd.h custom loads). Pops pick round-robin among loaded.
 * Redirect: 5-byte JMP detour on Play3D 0x459860. Prologue verified
 * 8B 54 24 08 56; mismatch = no install, native pops (foreign-exe safe).
 * Hook swaps ECX only: cached native bubble lists (app+0x488[0..1]) to
 * a custom list. No calls, no logging in hook (hot path, any thread).
 * Gate g_bubpop_on follows death_bubbles live. Native cache refreshes
 * every game_update tick. Probe once per base (sound lists live across
 * levels like def channels). Base "bubble" any case, or base1 missing
 * = native pops, hook never installed.
 * Include AFTER entdefs.h (g_bub_on, g_bub_sfx) + entsnd.h (snd typedefs).
 * Needs: log_mod, snprintf, IsBadReadPtr, windows.h alloc/protect fns.
 */
#ifndef ENTBUBCFG_H
#define ENTBUBCFG_H

#define BUBPOP_PLAY3D 0x00459860
#define BUBPOP_MAXV 9

/* Shared hook state lives in asm (.bss globals) because file-scope asm
 * refs resolve global-only; C statics are local and will not link. C
 * side uses extern decls (no mangling on variables). */
extern DWORD g_bubpop_nat0;
extern DWORD g_bubpop_nat1;
extern DWORD g_bubpop_lists[BUBPOP_MAXV];
extern int g_bubpop_n;
extern DWORD g_bubpop_rot;
extern int g_bubpop_on;
extern int g_bubpop_inst;
extern DWORD g_bubpop_tramp;
extern char g_bubsfx_done[56];
extern int g_bubsfx_nativelog;

extern "C" void bubpop_hook_asm(void);
__asm__(
".bss\n"
".globl _g_bubpop_nat0\n"
"_g_bubpop_nat0: .space 4\n"
".globl _g_bubpop_nat1\n"
"_g_bubpop_nat1: .space 4\n"
".globl _g_bubpop_lists\n"
"_g_bubpop_lists: .space 36\n"
".globl _g_bubpop_n\n"
"_g_bubpop_n: .space 4\n"
".globl _g_bubpop_rot\n"
"_g_bubpop_rot: .space 4\n"
".globl _g_bubpop_on\n"
"_g_bubpop_on: .space 4\n"
".globl _g_bubpop_inst\n"
"_g_bubpop_inst: .space 4\n"
".globl _g_bubpop_tramp\n"
"_g_bubpop_tramp: .space 4\n"
".globl _g_bubsfx_done\n"
"_g_bubsfx_done: .space 56\n"
".globl _g_bubsfx_nativelog\n"
"_g_bubsfx_nativelog: .space 4\n"
".text\n"
".globl _bubpop_hook_asm\n"
"_bubpop_hook_asm:\n"
" pushl %eax\n"
" movl _g_bubpop_on, %eax\n"
" testl %eax, %eax\n"
" jz 9f\n"
" movl _g_bubpop_nat0, %eax\n"
" cmpl %eax, %ecx\n"
" je 1f\n"
" movl _g_bubpop_nat1, %eax\n"
" cmpl %eax, %ecx\n"
" jne 9f\n"
"1:\n"
" movl _g_bubpop_n, %eax\n"
" testl %eax, %eax\n"
" jz 9f\n"
" lock; incl _g_bubpop_rot\n"
" movl _g_bubpop_rot, %eax\n"
"2:\n"
" cmpl _g_bubpop_n, %eax\n"
" jb 3f\n"
" subl _g_bubpop_n, %eax\n"
" jmp 2b\n"
"3:\n"
" movl _g_bubpop_lists(, %eax, 4), %ecx\n"
"9:\n"
" popl %eax\n"
" jmp *_g_bubpop_tramp\n");

static int bubcfg_streq(const char* a, const char* b) {
    int i = 0;
    while (i < 55 && a[i] && a[i] == b[i]) i++;
    return (a[i] == 0 && b[i] == 0) ? 1 : 0;
}

static int bubcfg_is_native_base(const char* s) {
    if (!s || !s[0]) return 1;
    if (s[0] != 'b' && s[0] != 'B') return 0;
    if (s[1] != 'u' && s[1] != 'U') return 0;
    if (s[2] != 'b' && s[2] != 'B') return 0;
    if (s[3] != 'b' && s[3] != 'B') return 0;
    if (s[4] != 'l' && s[4] != 'L') return 0;
    if (s[5] != 'e' && s[5] != 'E') return 0;
    return (s[6] == 0) ? 1 : 0;
}

static void bubpop_install(void) {
    unsigned char* p;
    unsigned char* t;
    DWORD old = 0;
    int i;
    if (g_bubpop_inst) return;
    p = (unsigned char*)BUBPOP_PLAY3D;
    if (IsBadReadPtr((void*)p, 8)) return;
    if (p[0] != 0x8B || p[1] != 0x54 || p[2] != 0x24 || p[3] != 0x08 ||
        p[4] != 0x56) {
        g_bubpop_inst = -1;
        log_mod("  BUBSFX: Play3D head mismatch, native pops");
        return;
    }
    t = (unsigned char*)VirtualAlloc(0, 32, MEM_COMMIT | MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE);
    if (!t) return;
    for (i = 0; i < 5; i++) t[i] = p[i];
    t[5] = 0xE9;
    *(DWORD*)(t + 6) = (DWORD)(p + 5) - (DWORD)(t + 10);
    if (!VirtualProtect((void*)p, 8, PAGE_EXECUTE_READWRITE, &old)) return;
    p[0] = 0xE9;
    *(DWORD*)(p + 1) = (DWORD)bubpop_hook_asm - (DWORD)(p + 5);
    VirtualProtect((void*)p, 8, old, &old);
    FlushInstructionCache(GetCurrentProcess(), (void*)p, 8);
    g_bubpop_tramp = (DWORD)t;
    g_bubpop_inst = 1;
    log_mod("  BUBSFX: pop redirect installed");
}

/* v1de: ambient sfx probe + proxy app. The proxy carries ambient pop
 * lists at +0x488/+0x48C and mirrors the real mesh holder at +0x5A4,
 * so the native update/render/dtor drive ambient bubbles untouched
 * (ctor stores app only, dtor swaps vtable + frees -- both verified
 * app-method-free). Ambient pops therefore never match the death
 * redirect (which keys on the two native lists) and need no hook. */
static DWORD g_buba_lists[BUBPOP_MAXV];
static int g_buba_n = 0;
static char g_buba_done[56] = { 0 };
static int g_buba_nativelog = 0;
static DWORD g_buba_fake = 0;

static void buba_service(DWORD app) {
    DWORD dev = 0;
    if (!app || IsBadReadPtr((void*)app, 0x5B0)) {
        g_bub_fakeapp = 0;
        return;
    }
    if (g_buba_fake && !IsBadReadPtr((void*)g_buba_fake, 0x600)) {
        DWORD* f = (DWORD*)g_buba_fake;
        f[0x122] = g_buba_lists[0];
        f[0x123] = (g_buba_n > 1) ? g_buba_lists[1] : g_buba_lists[0];
        if (!IsBadReadPtr((void*)(app + 0x5A4), 4))
            f[0x169] = *(DWORD*)(app + 0x5A4);
        g_bub_fakeapp = (g_buba_n > 0) ? g_buba_fake : 0;
    } else {
        g_bub_fakeapp = 0;
    }
    if (bubcfg_streq(g_buba_done, g_buba_sfx)) return;
    if (bubcfg_is_native_base(g_buba_sfx)) {
        int i = 0;
        while (i < 55 && g_buba_sfx[i]) {
            g_buba_done[i] = g_buba_sfx[i];
            i++;
        }
        g_buba_done[i] = 0;
        g_buba_n = 0;
        g_bub_fakeapp = 0;
        if (!g_buba_nativelog) {
            g_buba_nativelog = 1;
            log_mod("  BUBASFX: base bubble = native pops");
        }
        return;
    }
    if (IsBadReadPtr((void*)(app + 0x178), 4)) return;
    dev = *(DWORD*)(app + 0x178);
    if (dev < 0x10000 || IsBadReadPtr((void*)dev, 0x20)) return;
    {
        snd_loadappend_t load;
        snd_getch_t getch;
        char path[96];
        char lbuf[128];
        int d, ni, n = 0;
        const char* bs;
        load = (snd_loadappend_t)(DWORD)SND_LOADAPPEND;
        getch = (snd_getch_t)(DWORD)SND_GETCHANNEL;
        for (d = 1; d <= BUBPOP_MAXV; d++) {
            DWORD list, ch;
            ni = 0;
            path[ni++] = 's';
            path[ni++] = 'o';
            path[ni++] = 'u';
            path[ni++] = 'n';
            path[ni++] = 'd';
            path[ni++] = 's';
            path[ni++] = (char)92;
            bs = (const char*)g_buba_sfx;
            while (ni < 70 && *bs) {
                path[ni++] = *bs;
                bs++;
            }
            path[ni++] = (char)('0' + d);
            path[ni] = 0;
            list = load(dev, path, SND_CUSTOM_BUFS);
            ch = 0;
            if (list >= 0x10000 && !IsBadReadPtr((void*)list, 0x20))
                ch = getch(list);
            if (ch < 0x10000 || IsBadReadPtr((void*)ch, 0x20)) break;
            if (n < BUBPOP_MAXV) g_buba_lists[n++] = list;
        }
        g_buba_n = n;
        {
            int i = 0;
            while (i < 55 && g_buba_sfx[i]) {
                g_buba_done[i] = g_buba_sfx[i];
                i++;
            }
            g_buba_done[i] = 0;
        }
        if (n > 0) {
            snprintf(lbuf, sizeof(lbuf), "  BUBASFX: base %s %d variant(s)",
                     g_buba_sfx, n);
            log_mod(lbuf);
            if (!g_buba_fake) {
                DWORD* f = (DWORD*)malloc(0x600);
                if (f) {
                    int k;
                    for (k = 0; k < 0x180; k++) f[k] = 0;
                    g_buba_fake = (DWORD)f;
                    log_mod("  BUBASFX: proxy app ready");
                }
            }
        } else {
            snprintf(lbuf, sizeof(lbuf),
                     "  BUBASFX: base %s 1 missing, native pops",
                     g_buba_sfx);
            log_mod(lbuf);
        }
    }
}

/* Every game_update tick (board valid, unpaused caller). Refreshes the
 * native list cache + gate, probes a new base once (needs sound dev). */
static void bubsfx_service(DWORD board) {
    DWORD app = 0;
    DWORD dev = 0;
    if (!board || IsBadReadPtr((void*)(board + 0x878), 4)) return;
    app = *(DWORD*)(board + 0x878);
    if (app < 0x10000 || IsBadReadPtr((void*)app, 0x490)) return;
    g_bubpop_nat0 = *(DWORD*)(app + 0x488);
    g_bubpop_nat1 = *(DWORD*)(app + 0x48C);
    g_bubpop_on = (g_bub_on && g_bubpop_inst == 1 && g_bubpop_n > 0) ? 1 : 0;
    buba_service(app);   /* v1de: ambient proxy (own base, no hook) */
    if (bubcfg_streq(g_bubsfx_done, g_bub_sfx)) return;
    if (bubcfg_is_native_base(g_bub_sfx)) {
        int i = 0;
        while (i < 55 && g_bub_sfx[i]) {
            g_bubsfx_done[i] = g_bub_sfx[i];
            i++;
        }
        g_bubsfx_done[i] = 0;
        g_bubpop_n = 0;
        if (!g_bubsfx_nativelog) {
            g_bubsfx_nativelog = 1;
            log_mod("  BUBSFX: base bubble = native pops");
        }
        return;
    }
    if (IsBadReadPtr((void*)(app + 0x178), 4)) return;
    dev = *(DWORD*)(app + 0x178);
    if (dev < 0x10000 || IsBadReadPtr((void*)dev, 0x20)) return;
    {
        snd_loadappend_t load;
        snd_getch_t getch;
        char path[96];
        char lbuf[128];
        int d, ni, n = 0;
        const char* bs;
        load = (snd_loadappend_t)(DWORD)SND_LOADAPPEND;
        getch = (snd_getch_t)(DWORD)SND_GETCHANNEL;
        for (d = 1; d <= BUBPOP_MAXV; d++) {
            DWORD list, ch;
            ni = 0;
            path[ni++] = 's';
            path[ni++] = 'o';
            path[ni++] = 'u';
            path[ni++] = 'n';
            path[ni++] = 'd';
            path[ni++] = 's';
            path[ni++] = (char)92;
            bs = (const char*)g_bub_sfx;
            while (ni < 70 && *bs) {
                path[ni++] = *bs;
                bs++;
            }
            path[ni++] = (char)('0' + d);
            path[ni] = 0;
            list = load(dev, path, SND_CUSTOM_BUFS);
            ch = 0;
            if (list >= 0x10000 && !IsBadReadPtr((void*)list, 0x20))
                ch = getch(list);
            if (ch < 0x10000 || IsBadReadPtr((void*)ch, 0x20)) break;
            if (n < BUBPOP_MAXV) g_bubpop_lists[n++] = list;
        }
        g_bubpop_n = n;
        {
            int i = 0;
            while (i < 55 && g_bub_sfx[i]) {
                g_bubsfx_done[i] = g_bub_sfx[i];
                i++;
            }
            g_bubsfx_done[i] = 0;
        }
        if (n > 0) {
            snprintf(lbuf, sizeof(lbuf), "  BUBSFX: base %s %d variant(s)",
                     g_bub_sfx, n);
            log_mod(lbuf);
            bubpop_install();
        } else {
            snprintf(lbuf, sizeof(lbuf),
                     "  BUBSFX: base %s 1 missing, native pops", g_bub_sfx);
            log_mod(lbuf);
        }
    }
}

#endif /* ENTBUBCFG_H */
