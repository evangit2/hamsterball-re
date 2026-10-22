/*
 /* _mknp_devmode v03i — F8 toggles a devmode flag with on-screen status text.
  *
  * Architecture (all state-independent):
  *  - F8 poll runs on a private background thread (Sleep 16ms): works in
  *    menus, races, loading — everywhere. Zero game interaction.
  *  - Status line is drawn from a Font_DrawGlyph (0x457440) entry detour:
  *    orig runs via trampoline (SEH setup replayed verbatim), then the
  *    post-draw emits one direct DrawGlyph call (machine-code stub, no
  *    inline-asm pushes) with the live hook font.
  * Both installs are verify-or-skip from the init thread, never DllMain.
  *  - Dev keybinds come from mknp_devmode.set (jsonc, next to the DLL):
  *    CTRL+<key> fires the action only while devmode is ON (1s cooldown).
 *  - While devmode is ON and the indicator is in WAIT (level select),
 *    the game is frozen with its native pause (board+0x874 held at 1,
 *    prior value restored on exit). Dev keys are never eaten or hidden -
 *    the selector uses plain GetAsyncKeyState edges.
  */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "bass_proxy.h"   /* BASS proxy exports + load_real_bass + patch helpers */

#include <stdlib.h>
#include <ctype.h>

/* ── Addresses ─────────────────────────────────────────────────────── */

#define APP_PTR     0x005341E0u
#define APP_QUIT    0x159u
#define APP_FONT28  0x318u
#define FN_DRAWTEXT 0x00409B90u   /* UI_DrawTextShadow_Wrapper __thiscall, RET 0x3C */
#define FN_DRAWGLYPH 0x00457440u  /* Font_DrawGlyph __thiscall, RET 0x20: (text,x,y,xform,r,g,b,a) */
#define HOOK_ADDR   0x0046C1F1u   /* frame epilogue: 5E 83 C4 08 C3 */
#define PRESENT_ADDR 0x00455B48u  /* pre-Present (else path) */
#define PRESENT2_ADDR 0x00455AA6u /* pre-Present (al==1 path): 4x6A00+50+FF51 3C */
#define HOOK2_ADDR  0x00457440u   /* Font_DrawGlyph entry (SEH frame, replayed by trampoline) */
#define SPRITE_ADDR 0x00418870u   /* sprite-rect draw (SEH frame, replayed by trampoline) */
#define VK_F8       0x77
#define VK_CTRL     0x11   /* either CTRL key via GetAsyncKeyState */
#define VK_LEFT     0x25
#define VK_RIGHT    0x27
#define VK_RETURN   0x0D
#define VK_ESCAPE   0x1B

#define DEV_X 20
#define DEV_Y 20
#define IND_X 20           /* indicator line: below devmode line */
#define IND_Y 44
#define IND_SCALE 0x3F4CCCCDu  /* 0.8f bits: slightly smaller via Font+0x428 */
#define IND_MS 3000            /* indicator message visible time */

/* ── Indicator states ──────────────────────────────────────────────── */
/* REST (white "-") -> WAIT (yellow, level select) -> OK (green) / FAIL
 * (red) -> auto-revert to REST after IND_MS. Only CTRL+key from REST
 * enters WAIT; devmode OFF forces REST. */

typedef enum { IND_REST = 0, IND_WAIT = 1, IND_OK = 2, IND_FAIL = 3, IND_WAIT2 = 4 } ind_state_t;

static char g_indicator[128] = "-";
static ind_state_t g_indState = IND_REST;
static int g_indSel = -1;              /* -1 = ALL, 0..14 = LEVEL01..15 */
static DWORD g_indUntil = 0;           /* OK/FAIL auto-revert expiry */

/* face colors as float-bit DWORDs: white / yellow / green / red */
static const DWORD IND_RGB[5][3] = {
    { 0x3F800000u, 0x3F800000u, 0x3F800000u },
    { 0x3F800000u, 0x3F800000u, 0x00000000u },
    { 0x00000000u, 0x3F800000u, 0x00000000u },
    { 0x3F800000u, 0x00000000u, 0x00000000u },
    { 0x3F800000u, 0x3F800000u, 0x00000000u },
};
/* WAIT2 = medals second question. in_wait() = true level-selector freeze.
 * Legacy transient (unknown-action name flash) also uses IND_WAIT but with
 * an expiry, so it stays out of in_wait: no freeze, no eating, no flags. */
static int in_wait(void) {
    return g_indState == IND_WAIT2 ||
           (g_indState == IND_WAIT && !g_indUntil);
}
static int g_flow = 0;        /* 0 none, 1 reset-best-times, 2 set-medals */
static int g_medSel = 0;      /* WAIT2 medal: 0 NONE .. 4 WEASEL */
static const char* MEDAL_NAMES[5] = { "NONE", "BRONZE", "SILVER", "GOLD", "WEASEL" };
static const DWORD WHITE3[3] = { 0x3F800000u, 0x3F800000u, 0x3F800000u };

static void ind_set_sel_text(char* dst) {
    /* append "ALL" or "LEVELxx" for g_indSel */
    if (g_indSel < 0) {
        strcpy(dst, "ALL");
    } else {
        int lv = g_indSel + 1;
        strcpy(dst, "LEVEL");
        dst[5] = (char)('0' + lv / 10);
        dst[6] = (char)('0' + lv % 10);
        dst[7] = 0;
    }
}

static void ind_wait_text(void) {
    if (g_flow == 2) strcpy(g_indicator, "TT - SET MEDALS: ");
    else strcpy(g_indicator, "RESET BEST TIME: ");
    ind_set_sel_text(g_indicator + strlen(g_indicator));
}

static void ind_medwait_text(void) {   /* WAIT2: level fixed, pick a medal */
    strcpy(g_indicator, "TT - SET MEDALS - ");
    ind_set_sel_text(g_indicator + strlen(g_indicator));
    strcat(g_indicator, " - MEDAL: ");
    strcat(g_indicator, MEDAL_NAMES[g_medSel]);
}

static void ind_meddone_text(const char* prefix) {   /* DONE/FAIL verbatim */
    strcpy(g_indicator, prefix);
    ind_set_sel_text(g_indicator + strlen(g_indicator));
    strcat(g_indicator, " - MEDAL");
    strcat(g_indicator, MEDAL_NAMES[g_medSel]);
}

static void ind_done_text(const char* prefix) {
    strcpy(g_indicator, prefix);
    ind_set_sel_text(g_indicator + strlen(g_indicator));
}

#define EAT_MS 400u   /* pause-input quarantine after WAIT exit */
static DWORD g_eatUntil = 0;   /* GetTickCount horizon; keys eaten while future */

static void ind_to_rest(void) {
    if (in_wait())
        g_eatUntil = GetTickCount() + EAT_MS;   /* quarantine the exit key */
    g_indState = IND_REST;
    g_indUntil = 0;
    strcpy(g_indicator, "-");
}

static const char* indicator_text(void) {
    /* Any state carrying an expiry reverts after IND_MS. The reset
     * selector's WAIT is sticky (g_indUntil == 0) until enter/escape. */
    if (g_indUntil && (int)(GetTickCount() - g_indUntil) >= 0) {
        ind_to_rest();
    }
    return g_indicator;
}

/* ── State ─────────────────────────────────────────────────────────── */

static int g_devmode = 0;     /* the devmode variable: F8 toggles 0/1 */
static int g_prevF8  = 0;     /* previous F8 down-state (edge detect) */
static volatile int g_drawArmed = 0;  /* set each frame by C6F1; consumed by DrawGlyph hook */
static DWORD g_lastDrawTick = 0;   /* fallback throttle when C6F1 never fires */
/* ── Self-contained pixel-text overlay (raw D3D8, no game fonts) ────── */
/* The menus/HUD draw text through private sprite paths, so cold DrawGlyph
 * cold DrawGlyph calls stay invisible (no atlas bound in our phases). This overlay draws
 * its own 3x5 pixels straight through the captured device: FVF XYZRHW |
 * DIFFUSE, no texture, cull none, Z off, wrapped in a stateblock save /
 * restore (nothing else on screen is touched). */
static void log_msg(const char* fmt, ...);   /* defined later near log_open */
static DWORD g_device = 0;                /* captured from Present ECX */

/* d3d_call_n: device call with exactly n args (0..4). Pushes exactly n+1
 * dwords (this + args): no stack leak, unlike fixed-width pushers. */
static DWORD d3d_call_n(DWORD dev, DWORD off, DWORD n, DWORD a1, DWORD a2, DWORD a3, DWORD a4) {
    DWORD fn, rc = 0;
    DWORD vt = *(DWORD*)dev;
    DWORD blk[5];
    fn = *(DWORD*)(vt + off);
    blk[0] = dev; blk[1] = a1; blk[2] = a2; blk[3] = a3; blk[4] = a4;
    switch (n) {
    case 0:
        __asm__ __volatile__(
            "pushl (%1)\n\tcall *%2\n\tmovl %%eax, %0\n"
            : "=r"(rc) : "r"(blk), "r"(fn) : "eax", "edx", "ecx", "memory");
        break;
    case 1:
        __asm__ __volatile__(
            "pushl 4(%1)\n\tpushl (%1)\n\tcall *%2\n\tmovl %%eax, %0\n"
            : "=r"(rc) : "r"(blk), "r"(fn) : "eax", "edx", "ecx", "memory");
        break;
    case 2:
        __asm__ __volatile__(
            "pushl 8(%1)\n\tpushl 4(%1)\n\tpushl (%1)\n\tcall *%2\n\tmovl %%eax, %0\n"
            : "=r"(rc) : "r"(blk), "r"(fn) : "eax", "edx", "ecx", "memory");
        break;
    case 3:
        __asm__ __volatile__(
            "pushl 12(%1)\n\tpushl 8(%1)\n\tpushl 4(%1)\n\tpushl (%1)\n\tcall *%2\n\tmovl %%eax, %0\n"
            : "=r"(rc) : "r"(blk), "r"(fn) : "eax", "edx", "ecx", "memory");
        break;
    default:
        __asm__ __volatile__(
            "pushl 16(%1)\n\tpushl 12(%1)\n\tpushl 8(%1)\n\tpushl 4(%1)\n\tpushl (%1)\n\tcall *%2\n\tmovl %%eax, %0\n"
            : "=r"(rc) : "r"(blk), "r"(fn) : "eax", "edx", "ecx", "memory");
        break;
    }
    return rc;
}

/* 3x5 pixel font. Rows top to bottom, bit2 = left pixel. Order matches
 * px_idx below: space - . / 0-9 : A-Z ( ) + */
static const unsigned char px_font[][5] = {
    {0,0,0,0,0}, {0,0,7,0,0}, {0,0,0,0,1}, {1,1,2,4,4},
    {2,5,5,5,2}, {2,6,2,2,7}, {7,1,7,4,7}, {7,1,7,1,7},
    {5,5,7,1,1}, {7,4,7,1,7}, {7,4,7,5,7}, {7,1,2,2,2},
    {7,5,7,5,7}, {7,5,7,1,7}, {0,2,0,2,0},
    {2,5,7,5,5}, {6,5,6,5,6}, {3,4,4,4,3}, {6,5,5,5,6},
    {7,4,6,4,7}, {7,4,6,4,4}, {3,4,5,5,3}, {5,5,7,5,5},
    {7,2,2,2,7}, {1,1,1,5,2}, {5,5,6,5,5}, {4,4,4,4,7},
    {5,7,7,5,5}, {6,5,5,5,5}, {2,5,5,5,2}, {6,5,6,4,4},
    {2,5,5,2,1}, {6,5,6,5,5}, {3,4,2,1,6}, {7,2,2,2,2},
    {5,5,5,5,7}, {5,5,5,5,2}, {5,5,7,5,5}, {5,5,2,5,5},
    {5,5,2,2,2}, {7,1,2,4,7}, {1,2,2,2,1}, {4,2,2,2,4},
    {0,2,7,2,0},
};

static int px_idx(char c) {
    if (c == ' ') return 0;
    if (c == '-') return 1;
    if (c == '.') return 2;
    if (c == '/') return 3;
    if (c >= '0' && c <= '9') return 4 + (c - '0');
    if (c == ':') return 14;
    if (c >= 'A' && c <= 'Z') return 15 + (c - 'A');
    if (c == '(') return 41;
    if (c == ')') return 42;
    if (c == '+') return 43;
    return 0;
}

typedef struct { float x, y, z, rhw; DWORD color; DWORD pad[3]; } pxv_t;
static pxv_t px_buf[4096] __attribute__((aligned(16)));
static int px_nv = 0;   /* verts in px_buf */

static void px_quad(int x, int y, int w, int h, DWORD color) {
    pxv_t* v;
    float fx = (float)x, fy = (float)y, fw = (float)(x + w), fh = (float)(y + h);
    if (px_nv + 6 > 4096) return;
    v = &px_buf[px_nv];
    v[0].x = fx;  v[0].y = fy;  v[0].z = 0.5f; v[0].rhw = 1.0f; v[0].color = color;
    v[1].x = fw;  v[1].y = fy;  v[1].z = 0.5f; v[1].rhw = 1.0f; v[1].color = color;
    v[2].x = fx;  v[2].y = fh;  v[2].z = 0.5f; v[2].rhw = 1.0f; v[2].color = color;
    v[3].x = fw;  v[3].y = fy;  v[3].z = 0.5f; v[3].rhw = 1.0f; v[3].color = color;
    v[4].x = fw;  v[4].y = fh;  v[4].z = 0.5f; v[4].rhw = 1.0f; v[4].color = color;
    v[5].x = fx;  v[5].y = fh;  v[5].z = 0.5f; v[5].rhw = 1.0f; v[5].color = color;
    px_nv += 6;
}

static void px_line(const char* s, int x, int y, int sc, DWORD color) {
    int cx = x;
    for (; *s; s++, cx += 4 * sc) {
        const unsigned char* g = px_font[px_idx(*s)];
        int r, b;
        for (r = 0; r < 5; r++)
            for (b = 0; b < 3; b++)
                if (g[r] & (4 >> b))
                    px_quad(cx + b * sc, y + r * sc, sc, sc, color);
    }
}

/* ARGB from an IND_RGB float triple (bits: 0x3F800000 = 1.0). */
static DWORD px_argb(const DWORD* rgb) {
    DWORD r = (rgb[0] & 0x7FFFFFFFu) >= 0x3F000000u ? 0xFFu : 0x00u;
    DWORD g = (rgb[1] & 0x7FFFFFFFu) >= 0x3F000000u ? 0xFFu : 0x00u;
    DWORD b = (rgb[2] & 0x7FFFFFFFu) >= 0x3F000000u ? 0xFFu : 0x00u;
    return 0xFF000000u | (r << 16) | (g << 8) | b;
}

static int g_pxDraws = 0;   /* pixel-overlay draws (diagnostic) */

static void draw_pixel_overlay(void) {
    DWORD dev, tok = 0;
    const char* l1 = g_devmode ? "DEVMODE: ON" : "DEVMODE: OFF";
    const char* l2 = indicator_text();
    DWORD face = px_argb(IND_RGB[g_indState]);
    if (!g_device || IsBadReadPtr((void*)g_device, 4)) return;
    dev = g_device;
    if (IsBadReadPtr(*(void**)dev, 0x140)) return;
    d3d_call_n(dev, 0xE4, 2, 1, (DWORD)&tok, 0, 0);   /* CreateStateBlock(ALL) */
    d3d_call_n(dev, 0x130, 1, 0x44, 0, 0, 0);          /* FVF XYZRHW|DIFFUSE */
    d3d_call_n(dev, 0xF4, 2, 0, 0, 0, 0);              /* texture0 NULL */
    d3d_call_n(dev, 0xC8, 2, 7, 0, 0, 0);             /* ZENABLE off */
    d3d_call_n(dev, 0xC8, 2, 22, 1, 0, 0);            /* CULLMODE none */
    /* No Begin/End: the game already has a scene open at pre-Present
     * (its own sprites draw here without bracketing). One UP per pass
     * (shadow/face, line 1/2) so the static buffer never overflows. */
#define PX_PASS(str, px, py, psc, pcol) \
    do { px_nv = 0; px_line(str, px, py, psc, pcol); \
         if (px_nv) d3d_call_n(dev, 0x120, 4, 4, (DWORD)(px_nv / 3), (DWORD)px_buf, 32); } while (0)
    PX_PASS(l1, 20 + 4, 20 + 4, 4, 0xFF000000u);
    PX_PASS(l1, 20, 20, 4, 0xFFFFFFFFu);
    PX_PASS(l2, 20 + 2, 52 + 2, 2, 0xFF000000u);
    PX_PASS(l2, 20, 52, 2, face);
    if (tok) {
        d3d_call_n(dev, 0xD8, 1, tok, 0, 0, 0);      /* ApplyStateBlock */
        d3d_call_n(dev, 0xE0, 1, tok, 0, 0, 0);      /* DeleteStateBlock */
    }
    if (g_pxDraws < 12)
        log_msg("PXD n=%d nv=%d l1=%s l2=%s\n", g_pxDraws, px_nv, l1, l2);
    g_pxDraws++;
}

static int g_frameHookOk = 0;     /* 1 once the 0x46C1F1 frame hook is live */
static volatile int g_inDraw = 0;     /* reentrancy guard for our own draw */
static void* g_drawFont = 0;
          /* live Font* captured from the hook's ECX */
static int g_drawCount = 0;           /* successful draw attempts (diagnostic) */
static volatile int g_hitCount = 0;     /* DrawGlyph cave entries (diagnostic) */
static volatile int g_presentCount = 0; /* pre-Present cave entries */
static volatile int g_spriteHits = 0;   /* sprite-hook cave entries */
static DWORD g_devVtbl = 0;           /* *(DWORD*)g_device at hook time */
static DWORD g_origPresent = 0;       /* orig IDirect3DDevice8::Present (slot 17) */
static int g_devHookOk = 0;           /* 1 once the device Present hook is live */
static DWORD g_devCave_ = 0;          /* our Present cave (self-heal compare) */
static DWORD g_spriteTex = 0;         /* ECX (texture object) of latest sprite */
static DWORD g_spriteRet = 0;         /* return address of latest sprite call */
static DWORD g_callerStack[16];         /* LIFO of stashed caller-rets */
static DWORD g_callerDepth = 0;         /* (our DrawGlyph nests glyph sprites) */
static int g_loaderGate = 1;              /* fail-closed: skip until proven menu/race */
static int g_bootPhase = 1;               /* 1 = first 20s: blank/unreadable also closes */
static int g_postBoot = 0;                /* 1 = DrawGlyph cave routes via afterCode */
static DWORD g_bootTick = 0;              /* GetTickCount at init */
static volatile int g_drawLock = 0;       /* cave spinlock: serializes threads */
static DWORD g_drawTid = 0;               /* owning thread id (0 = free) */

/* ── Log (_mknp_devmode.log next to the DLL, %TEMP% fallback) ────────── */

static FILE* g_log = NULL;

static void log_open(void) {
    char dir[MAX_PATH], path[MAX_PATH];
    HMODULE hSelf = NULL;
    if (g_log) return;
    dir[0] = 0;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                         | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                         (LPCSTR)log_open, &hSelf) && hSelf) {
        GetModuleFileNameA(hSelf, dir, MAX_PATH);
        {
            char* slash = strrchr(dir, '\\');
            if (slash) slash[1] = 0;
            else dir[0] = 0;
        }
    }
    if (dir[0]) {
        snprintf(path, MAX_PATH, "%s_mknp_devmode.log", dir);
        g_log = fopen(path, "a");
    }
    if (!g_log) {
        char tmp[MAX_PATH];
        if (GetTempPathA(MAX_PATH, tmp)) {
            snprintf(path, MAX_PATH, "%s_mknp_devmode.log", tmp);
            g_log = fopen(path, "a");
        }
    }
    if (!g_log) return;
    fprintf(g_log, "INIT _mknp_devmode v03i self=%p\n", hSelf);
    fflush(g_log);
}

static void log_msg(const char* fmt, ...) {
    va_list ap;
    if (!g_log) return;
    va_start(ap, fmt);
    vfprintf(g_log, fmt, ap);
    va_end(ap);
    fflush(g_log);
}

/* ── Devmode persistence (_mknp_devmode.state next to the DLL) ──────── */
/* Remember the F8 toggle across runs: 1 byte ("1"/"0"). Loaded in init
 * (default OFF when missing/unreadable), saved on every F8 toggle. */

#define STATE_FNAME "_mknp_devmode.state"

static void state_load(void) {
    char dir[MAX_PATH], path[MAX_PATH], buf[8];
    HMODULE hSelf = NULL;
    FILE* f;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                         | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                         (LPCSTR)state_load, &hSelf) && hSelf) {
        GetModuleFileNameA(hSelf, dir, MAX_PATH);
        {
            char* slash = strrchr(dir, '\\');
            if (slash) {
                strcpy(slash + 1, STATE_FNAME);
                f = fopen(dir, "r");
                if (f) {
                    if (fgets(buf, sizeof(buf), f) && buf[0] == '1')
                        g_devmode = 1;
                    fclose(f);
                    log_msg("STATE loaded devmode=%d\n", g_devmode);
                    return;
                }
            }
        }
    }
    log_msg("STATE no file, devmode=%d\n", g_devmode);
}

static void state_save(void) {
    char dir[MAX_PATH];
    HMODULE hSelf = NULL;
    FILE* f;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                         | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                         (LPCSTR)state_save, &hSelf) && hSelf) {
        GetModuleFileNameA(hSelf, dir, MAX_PATH);
        {
            char* slash = strrchr(dir, '\\');
            if (slash) {
                strcpy(slash + 1, STATE_FNAME);
                f = fopen(dir, "w");
                if (f) {
                    fputc(g_devmode ? '1' : '0', f);
                    fclose(f);
                }
            }
        }
    }
}

/* ── Crash spy (first-chance VEH): logs REAL fault EIP, then passes ── */
/* through to the game's own handler. Latches the primary.             */
static volatile LONG g_vehDone = 0;
#define MEDAL_BASE 0x8BCu   /* App+0x8BC medal bytes ( defined once, used early by guard) */
static DWORD g_guardOn = 0;         /* medal page guard active */
static DWORD g_guardPage = 0;       /* guarded page base */
static DWORD g_medalBase = 0;       /* app+0x8BC at arm time */
static DWORD g_medalReads = 0;      /* unique medal-range readers logged */
static DWORD g_guardFaults = 0;     /* total guard faults (storm cap) */
static DWORD g_readerEip[32];       /* unique reader EIPs */

static void medal_guard_off(void) {
    DWORD old = 0;
    if (!g_guardOn || !g_guardPage) { g_guardOn = 0; return; }
    VirtualProtect((void*)g_guardPage, 4096, PAGE_READWRITE, &old);
    g_guardOn = 0;
}

static void medal_guard_on(void) {
    DWORD app, old = 0;
    if (g_guardOn) return;
    if (g_medalReads >= 30 || g_guardFaults >= 5000) return;
    if (IsBadReadPtr((void*)APP_PTR, 4)) return;
    app = *(DWORD*)APP_PTR;
    if (!app || app < 0x10000) return;
    g_medalBase = app + MEDAL_BASE;
    g_guardPage = g_medalBase & ~0xFFFu;
    if (!VirtualProtect((void*)g_guardPage, 4096,
                        PAGE_READWRITE | PAGE_GUARD, &old)) return;
    g_guardOn = 1;
}

static LONG CALLBACK crash_spy(PEXCEPTION_POINTERS ep) {
    /* Medal-guard probe: log who reads the medal bytes, re-arm, continue. */
    {
        DWORD code = ep->ExceptionRecord->ExceptionCode;
        if (code == 0xC0000005u || code == 0x80000001u) {
            DWORD w = ep->ExceptionRecord->ExceptionInformation[0];
            DWORD addr = ep->ExceptionRecord->ExceptionInformation[1];
            DWORD eip;
            (void)w;
            if (g_guardOn && addr >= g_medalBase && addr < g_medalBase + 80) {
                int i, seen = 0;
                eip = (DWORD)ep->ExceptionRecord->ExceptionAddress;
                g_guardFaults++;
                {
                    DWORD old = 0;
                    VirtualProtect((void*)g_guardPage, 4096,
                                   PAGE_READWRITE | PAGE_GUARD, &old);
                }
                if (g_guardFaults >= 5000) medal_guard_off();
                if (eip >= 0x50000000u) return EXCEPTION_CONTINUE_EXECUTION;
                for (i = 0; i < 32; i++)
                    if (g_readerEip[i] == eip) { seen = 1; break; }
                if (!seen && g_medalReads < 30) {
                    for (i = 0; i < 32; i++)
                        if (!g_readerEip[i]) { g_readerEip[i] = eip; break; }
                    log_msg("MEDALREAD eip=%p\n", (void*)eip);
                    g_medalReads++;
                }
                return EXCEPTION_CONTINUE_EXECUTION;
            }
            if (g_guardOn && addr >= g_guardPage && addr < g_guardPage + 4096) {
                /* same-page noise (best-times etc.): re-arm silently */
                DWORD old = 0;
                VirtualProtect((void*)g_guardPage, 4096,
                               PAGE_READWRITE | PAGE_GUARD, &old);
                g_guardFaults++;
                if (g_guardFaults >= 5000) medal_guard_off();
                return EXCEPTION_CONTINUE_EXECUTION;
            }
        }
    }
    if (InterlockedExchange(&g_vehDone, 1)) return EXCEPTION_CONTINUE_SEARCH;
    {
        DWORD eip = (DWORD)ep->ExceptionRecord->ExceptionAddress;
        DWORD code = ep->ExceptionRecord->ExceptionCode;
        CONTEXT* c = ep->ContextRecord;
        log_msg("SPY primary code=%08X EIP=%08X eax=%08X ebx=%08X ecx=%08X edx=%08X esi=%08X edi=%08X ebp=%08X esp=%08X draws=%d hits=%d\n",
                code, eip, c->Eax, c->Ebx, c->Ecx, c->Edx, c->Esi, c->Edi, c->Ebp, c->Esp, g_drawCount, g_hitCount);
        if (!IsBadReadPtr((void*)eip, 8)) {
            BYTE* b = (BYTE*)eip;
            log_msg("SPY bytes: %02X %02X %02X %02X %02X %02X %02X %02X\n",
                    b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7]);
            }
            if (!IsBadReadPtr((void*)c->Esp, 0x70)) {
                DWORD* st = (DWORD*)c->Esp;
                log_msg("SPY stack s00=%08X s01=%08X s02=%08X s03=%08X s04=%08X s05=%08X s06=%08X s07=%08X\n",
                        st[0], st[1], st[2], st[3], st[4], st[5], st[6], st[7]);
                log_msg("SPY stack s08=%08X s09=%08X s10=%08X s11=%08X s12=%08X s13=%08X s14=%08X s15=%08X\n",
                        st[8], st[9], st[10], st[11], st[12], st[13], st[14], st[15]);
                log_msg("SPY stack s16=%08X s17=%08X s18=%08X s19=%08X s20=%08X s21=%08X s22=%08X s23=%08X\n",
                        st[16], st[17], st[18], st[19], st[20], st[21], st[22], st[23]);
            }
            if (!IsBadReadPtr((void*)c->Esp, 4)) {
                DWORD ret = *(DWORD*)(c->Esp + 0x38);
                if (!IsBadReadPtr((void*)ret, 16)) {
                    BYTE* b = (BYTE*)ret;
                    log_msg("SPY retcode: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
                            b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7],
                            b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15]);
                }
            }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

/* ── Status text draw (game font pipeline) ─────────────────────────── */
/* __thiscall: ECX = Font*, 15 stack DWORDs, callee cleans (RET 0x3C).  */
/* Floats travel as raw bit patterns in integer registers.             */

/* v01e: call Font_DrawGlyph via an emitted machine-code stub.
 *
 * ROOT CAUSE of the v01a-v01d boot crashes (found via VEH spy + DLL
 * disassembly): GCC miscompiles repeated `pushl %[m]` in one asm block.
 * Each push's "m" displacement is computed ONCE for the entry ESP, but
 * ESP drops 4 bytes per push — so push #2..#8 read progressively shifted
 * stack slots. Immediates ($20) and the first push stay correct, which is
 * why x/y looked right while text (last push) was garbage (p1=4..6) and
 * the game faulted at DrawGlyph+0x32 reading [5].
 * Fix: no inline pushes of memory operands at all. All 8 params travel
 * as plain C DWORDs into this emitter; the stub is pure immediate pushes
 * (ESP-independent) + mov ecx + call + ret, wrapped in pushad/pushfd.
 */
static unsigned char* g_callbuf = 0;

static void call_thiscall_n(DWORD fn, DWORD thi, DWORD* params, int n) {
    unsigned char* p;
    int i;
    if (!g_callbuf) {
        g_callbuf = (unsigned char*)alloc_executable(128);
        if (!g_callbuf) return;
    }
    p = g_callbuf;
    *p++ = 0x60;   /* pushad */
    *p++ = 0x9C;   /* pushfd */
    for (i = n - 1; i >= 0; i--) {
        *p++ = 0x68;                       /* push imm32 (pN first) */
        *(DWORD*)p = params[i]; p += 4;
    }
    *p++ = 0xB9;                           /* mov ecx, thi */
    *(DWORD*)p = thi; p += 4;
    *p++ = 0xE8;                           /* call fn */
    *(DWORD*)p = fn - ((DWORD)p + 4); p += 4;
    *p++ = 0x9D;   /* popfd */
    *p++ = 0x61;   /* popad */
    *p++ = 0xC3;   /* ret */
    ((void(*)())g_callbuf)();
}

/* v03i: single small line with bottom-right shadow (version-text style).
 * Two DrawGlyph passes: black at +1,+1, then white at x,y. (v01i's
 * 0.75-scale colored path rendered nothing — the scaled
 * Scene_CreateObject4f path needs setup our post-draw context lacks.
 * White glyph sprites always draw.) */
/* Draw one shadowed line. scaleBits=0 means native size; otherwise the
 * value is poked into Font+0x428 (native per-font scale, 1.0 = normal;
 * 0x4574C1 takes the scaled path when it differs) and restored after.
 * rgb = face color as 3 float-bit DWORDs (shadow stays black). */
static void draw_status_ex(void* font, const char* text, int x, int y, DWORD scaleBits, const DWORD* rgb) {
    const unsigned int one = 0x3F800000u;   /* 1.0f */
    const unsigned int zero = 0x00000000u;  /* 0.0f */
    const unsigned int xf = 0x004CF300u;    /* static identity transform */
    DWORD params[8];
    DWORD saved = 0;
    int poked = 0;
    if (scaleBits && !IsBadReadPtr((char*)font + 0x428, 4) &&
        !IsBadWritePtr((char*)font + 0x428, 4)) {
        saved = *(DWORD*)((char*)font + 0x428);
        *(DWORD*)((char*)font + 0x428) = scaleBits;
        poked = 1;
    }
    params[0] = (DWORD)text;                /* shadow: black, +1,+1 */
    params[1] = (DWORD)(x + 1);
    params[2] = (DWORD)(y + 1);
    params[3] = xf;
    params[4] = zero; params[5] = zero; params[6] = zero; params[7] = one;
    call_thiscall_n(FN_DRAWGLYPH, (DWORD)font, params, 8);
    params[1] = (DWORD)x;                   /* face at x,y in rgb */
    params[2] = (DWORD)y;
    params[4] = rgb[0]; params[5] = rgb[1]; params[6] = rgb[2]; params[7] = one;
    call_thiscall_n(FN_DRAWGLYPH, (DWORD)font, params, 8);
    if (poked) *(DWORD*)((char*)font + 0x428) = saved;
}

static void draw_status(void* font, const char* text) {
    draw_status_ex(font, text, DEV_X, DEV_Y, 0, WHITE3);
}

/* ── Per-frame handler (runs on the game thread at the epilogue) ───── */
/* F8 poll only. The status draw happens in render context via the      */
/* Font_DrawGlyph post-hook (g_drawArmed hands one draw per frame).     */

static void draw_direct(void);
static void install_dev_hook(void);
static void draw_from_frame(void);
static DWORD g_slotRet;
static DWORD g_dummy;         /* branchless store sink */
static DWORD g_routePtr = 0;      /* cave tail target: trampoline, then afterEntry */
static DWORD g_afterEntry = 0;      /* published by install, flipped in by poll */

/* ── .set file + dev keybindings ───────────────────────────────────── */
/* mknp_devmode.set (jsonc, next to the DLL) maps action names to
 * { "keybinding": "X" }. While devmode is ON, CTRL+X fires the action
 * (edge-detect, 1s global cooldown between dev fires). Devmode OFF =
 * keybinds do nothing. Set reloads when its mtime changes. */

#define MAX_BINDS   16
#define SET_FNAME "_mknp_devmode.set"

typedef struct { char name[96]; char key; UINT vk; int prev; } devbind_t;
static devbind_t g_binds[MAX_BINDS];
static int g_nbinds = 0;
static char g_setPath[MAX_PATH] = {0};
static DWORD g_setTick = 0;        /* last mtime check (GetTickCount) */
static DWORD s_lastDevFire = 0;    /* GetTickCount of last dev bind fire */

static const char* s_p;            /* set parser cursor */

static void ps_skip_ws(void) {
    while (*s_p == ' ' || *s_p == '\t' || *s_p == '\r' || *s_p == '\n') s_p++;
}

/* parse a "..." string (unescaped, truncated); 1 = ok */
static int ps_string(char* out, int outsz) {
    int n = 0;
    if (*s_p != '"') return 0;
    s_p++;
    while (*s_p && *s_p != '"') {
        char c = *s_p++;
        if (c == '\\' && *s_p) {
            char e = *s_p++;
            c = (e == 'n') ? '\n' : (e == 't') ? '\t' : (e == 'r') ? '\r' : e;
        }
        if (n + 1 < outsz) out[n++] = c;
    }
    out[n] = 0;
    if (*s_p == '"') s_p++;
    return 1;
}

/* strip // comments (outside strings), in place */
static void set_strip(char* b) {
    int i = 0, j = 0, instr = 0;
    while (b[i]) {
        char c = b[i];
        if (instr) {
            b[j++] = c;
            if (c == '\\' && b[i + 1]) { b[j++] = b[i + 1]; i += 2; continue; }
            if (c == '"') instr = 0;
            i++;
        } else if (c == '"') { instr = 1; b[j++] = c; i++; }
        else if (c == '/' && b[i + 1] == '/') { while (b[i] && b[i] != '\n') i++; }
        else { b[j++] = c; i++; }
    }
    b[j] = 0;
}

/* scan stripped jsonc: "Action" : { ... "keybinding" : "X" ... } */
static void set_parse(const char* buf) {
    char objkey[8][96];
    int bd = 0, i;
    for (i = 0; i < 8; i++) objkey[i][0] = 0;
    g_nbinds = 0;
    s_p = buf;
    for (;;) {
        ps_skip_ws();
        if (!*s_p) break;
        if (*s_p == '{') {
            s_p++;
            if (bd < 7) { bd++; objkey[bd][0] = 0; }
            continue;
        }
        if (*s_p == '}') { s_p++; if (bd > 0) bd--; continue; }
        if (*s_p == '"') {
            char s[96], v[96];
            const char* q;
            ps_string(s, sizeof s);
            q = s_p;
            while (*q == ' ' || *q == '\t' || *q == '\r' || *q == '\n') q++;
            if (*q != ':') continue;   /* bare string, not a key */
            s_p = q + 1;
            ps_skip_ws();
            if (*s_p == '{') {
                s_p++;
                if (bd < 7) {
                    bd++;
                    strncpy(objkey[bd], s, 95);
                    objkey[bd][95] = 0;
                }
                continue;
            }
            if (*s_p == '"') {
                ps_string(v, sizeof v);
                if (!strcmp(s, "keybinding") && v[0] && g_nbinds < MAX_BINDS) {
                    char k = (char)toupper((unsigned char)v[0]);
                    if ((k >= 'A' && k <= 'Z') || (k >= '0' && k <= '9')) {
                        devbind_t* b = &g_binds[g_nbinds++];
                        strncpy(b->name, objkey[bd], 95);
                        b->name[95] = 0;
                        b->key = k;
                        b->vk = (UINT)k;   /* A-Z 0-9 VK == ASCII uppercase */
                        b->prev = 0;
                    } else {
                        log_msg("SET skip bad keybinding '%s'\n", v);
                    }
                }
                continue;
            }
            while (*s_p && *s_p != ',' && *s_p != '}' && *s_p != ']') s_p++;
            continue;
        }
        s_p++;
    }
    log_msg("SET parsed: %d binds\n", g_nbinds);
    for (i = 0; i < g_nbinds; i++)
        log_msg("SET bind '%s' -> CTRL+%c\n", g_binds[i].name, g_binds[i].key);
}

static void set_locate(void) {
    HMODULE hSelf = NULL;
    if (g_setPath[0]) return;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                         | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                         (LPCSTR)set_locate, &hSelf) && hSelf) {
        GetModuleFileNameA(hSelf, g_setPath, MAX_PATH);
        {
            char* slash = strrchr(g_setPath, '\\');
            if (slash) strcpy(slash + 1, SET_FNAME);
            else g_setPath[0] = 0;
        }
    }
}

/* 1 = file mtime changed since last call (primes on first call) */
static int set_changed(void) {
    WIN32_FILE_ATTRIBUTE_DATA fad;
    static DWORD s_lo = 0, s_hi = 0;
    static int s_have = 0;
    if (!g_setPath[0]) return 0;
    if (!GetFileAttributesExA(g_setPath, GetFileExInfoStandard, &fad)) return 0;
    if (!s_have || fad.ftLastWriteTime.dwLowDateTime != s_lo ||
        fad.ftLastWriteTime.dwHighDateTime != s_hi) {
        s_lo = fad.ftLastWriteTime.dwLowDateTime;
        s_hi = fad.ftLastWriteTime.dwHighDateTime;
        s_have = 1;
        return 1;
    }
    return 0;
}

static void set_load(void) {
    FILE* f;
    long n;
    char* buf;
    set_locate();
    if (!g_setPath[0]) return;
    f = fopen(g_setPath, "rb");
    if (!f) { log_msg("SET no file: %s\n", g_setPath); return; }
    fseek(f, 0, SEEK_END); n = ftell(f); fseek(f, 0, SEEK_SET);
    if (n <= 0 || n > 65536) { fclose(f); return; }
    buf = (char*)malloc((size_t)n + 1);
    if (!buf) { fclose(f); return; }
    if (fread(buf, 1, (size_t)n, f) != (size_t)n) { free(buf); fclose(f); return; }
    buf[n] = 0;
    fclose(f);
    set_strip(buf);
    set_parse(buf);
    free(buf);
}

/* ── Dev actions (game memory writes) ───────────────────────────────── */

/* BestTime table: App+0x86c, 20 DWORDs. Fresh-profile default is
 * 0x05F5E09C (99999900; the loader's rep stos fill), NOT zero. */

#define APP_BESTTIME    0x86cu
#define BESTTIME_NATIVE 0x05F5E09Cu
#define BESTTIME_COUNT  20
#define BESTTIME_RACES  15   /* LEVEL01..15 = slots 1..15 (race number); slot 0 unused */

/* Reset BestTime slots. sel < 0 = ALL 15 races (slots 1..15), else the
 * single slot sel+1. The game indexes both arrays by race number (Warm-Up
 * reads slot 1, Impossible slot 15; slot 0 is unused).
 * Returns 1 on success, 0 when game memory is not writable (FAIL state). */
static int reset_best_times_sel(int sel) {
    DWORD app;
    int i, lo, hi;
    if (IsBadReadPtr((void*)APP_PTR, 4)) {
        log_msg("DEVBIND reset-best-times: no App\n");
        return 0;
    }
    app = *(DWORD*)APP_PTR;
    if (!app || app < 0x10000 || IsBadReadPtr((void*)app, 0x900)) {
        log_msg("DEVBIND reset-best-times: bad App %p\n", (void*)app);
        return 0;
    }
    if (IsBadWritePtr((void*)(app + APP_BESTTIME), BESTTIME_COUNT * 4)) {
        log_msg("DEVBIND reset-best-times: App+0x86c not writable\n");
        return 0;
    }
    lo = (sel < 0) ? 1 : sel + 1;
    hi = (sel < 0) ? BESTTIME_RACES : sel + 1;
    for (i = lo; i <= hi; i++)
        *(DWORD*)(app + APP_BESTTIME + (DWORD)i * 4) = BESTTIME_NATIVE;
    log_msg("DEVBIND reset-best-times: slots %d..%d at App+0x86c = 0x05F5E09C\n", lo, hi);
    return 1;
}

/* Set earned-medal bytes. App+0x8BC slots mirror the best-time array
 * (20 dword slots, races = 0..14): byte0 = BRONZE, byte1 = SILVER,
 * byte2 = GOLD (proven: earn-sites compare vs the BRONZE/SILVER/GOLD
 * threshold fields, AdvanceRace shows next-target per highest byte).
 * Patterns are cumulative, as if the player earned exactly that tier:
 * BRONZE lights bronze only, SILVER bronze+silver, GOLD all three.
 * WEASEL has no persistent byte in this build, so it sets all three
 * (max visible state). NONE clears all three (uncollected).
 * sel < 0 = ALL 15 races (slots 1..15), else that level only.
 * Returns 1/0. */
static const BYTE MEDAL_PAT[5][3] = {
    { 0, 0, 0 }, { 1, 0, 0 }, { 1, 1, 0 }, { 1, 1, 1 }, { 1, 1, 1 }
};

static int set_medals_sel(int sel, int medal) {
    DWORD app;
    int i, lo, hi;
    if (IsBadReadPtr((void*)APP_PTR, 4)) {
        log_msg("DEVBIND set-medals: no App\n");
        return 0;
    }
    app = *(DWORD*)APP_PTR;
    if (!app || app < 0x10000 || IsBadReadPtr((void*)app, 0x900)) {
        log_msg("DEVBIND set-medals: bad App %p\n", (void*)app);
        return 0;
    }
    if (IsBadWritePtr((void*)(app + MEDAL_BASE), BESTTIME_COUNT * 4)) {
        log_msg("DEVBIND set-medals: App+0x8bc not writable\n");
        return 0;
    }
    if (medal < 0) medal = 0;
    if (medal > 4) medal = 4;
    lo = (sel < 0) ? 1 : sel + 1;
    hi = (sel < 0) ? BESTTIME_RACES : sel + 1;
    for (i = lo; i <= hi; i++) {
        BYTE* slot = (BYTE*)(app + MEDAL_BASE + (DWORD)i * 4);
        slot[0] = MEDAL_PAT[medal][0];
        slot[1] = MEDAL_PAT[medal][1];
        slot[2] = MEDAL_PAT[medal][2];
    }
    log_msg("DEVBIND set-medals: slots %d..%d at App+0x8bc = %s\n",
            lo, hi, MEDAL_NAMES[medal]);
    return 1;
}

/* ── Pause-reader tap (0x469D2D): capture the TRUE board ───────────── */
/* App+0x178 can be a menu shell (vtable 0x4D911C) while the live race board
 * lives elsewhere. The game's own pause check (mov 0x874(%ecx),%al) always
 * runs with the true object, so a verify-or-skip detour stashes ECX on
 * every hit. wait_freeze_tick prefers the tap board, App+0x178 fallback. */

#define PAUSETAP_ADDR 0x469D2Du
#define PAUSETAP_NEXT 0x469D33u
#define SHELL_VTBL    0x4D911Cu

static DWORD g_tapBoard = 0;   /* last ECX seen by the game's pause reader */
static int g_tapOk = 0;        /* 1 once the tap is installed */

static void install_pause_tap(void) {
    BYTE* a = (BYTE*)PAUSETAP_ADDR;
    unsigned char* cave;
    unsigned char* q;
    if (IsBadReadPtr((void*)PAUSETAP_ADDR, 6)) {
        log_msg("PAUSETAP skip: 0x469D2D unreadable\n");
        return;
    }
    if (!(a[0] == 0x8A && a[1] == 0x81 && a[2] == 0x74 &&
          a[3] == 0x08 && a[4] == 0x00 && a[5] == 0x00)) {
        log_msg("PAUSETAP skip: bytes mismatch %02X %02X %02X %02X %02X %02X\n",
                a[0], a[1], a[2], a[3], a[4], a[5]);
        return;
    }
    cave = (unsigned char*)alloc_executable(32);
    if (!cave) {
        log_msg("PAUSETAP skip: VirtualAlloc failed\n");
        return;
    }
    q = cave;
    *q++ = 0x89; *q++ = 0x0D;                    /* mov [g_tapBoard],ecx */
    *(DWORD*)q = (DWORD)&g_tapBoard; q += 4;
    *q++ = 0x8A; *q++ = 0x81;                    /* replay: mov 0x874(ecx),al */
    *q++ = 0x74; *q++ = 0x08; *q++ = 0x00; *q++ = 0x00;
    *q++ = 0xE9;                                 /* jmp back to 0x469D33 */
    *(DWORD*)q = PAUSETAP_NEXT - ((DWORD)q + 4); q += 4;
    install_jmp_hook_nop(PAUSETAP_ADDR, (DWORD)cave, 6);
    g_tapOk = 1;
    log_msg("PAUSETAP installed at 0x469D2D -> cave %p\n", cave);
}

#define APP_BOARD     0x178u
#define BOARD_PAUSE   0x874u
#define BOARD_NAME    0x868u

static int g_wePaused = 0;
static DWORD g_exitTick = 0;   /* WAIT-exit stamp for release-hold cap */        /* 1 while WE hold the pause */
static BYTE g_pauseSaved = 0;     /* flag value before we took over */
static DWORD g_pauseBoard = 0;    /* board we paused (0 = none) */

static int wait_board_ok(DWORD *boardOut, char *nameOut) {
    DWORD app, board;
    int i;
    if (IsBadReadPtr((void*)APP_PTR, 4)) return 0;
    app = *(DWORD*)APP_PTR;
    if (!app || app < 0x10000 || IsBadReadPtr((void*)app, APP_BOARD + 4))
        return 0;
    board = *(DWORD*)(app + APP_BOARD);
    if (!board || board < 0x10000 ||
        IsBadReadPtr((void*)board, BOARD_PAUSE + 1) ||
        IsBadReadPtr((void*)(board + BOARD_NAME), 12))
        return 0;
    for (i = 0; i < 11; i++) {
        char c = *(char*)(board + BOARD_NAME + (DWORD)i);
        nameOut[i] = (c >= 32 && c < 127) ? c : 0;
        if (!c) break;
    }
    nameOut[11] = 0;
    if (!strstr(nameOut, "Board")) return 0;   /* menu shell, not a board */
    if (IsBadWritePtr((void*)(board + BOARD_PAUSE), 1)) return 0;
    *boardOut = board;
    return 1;
}

static void wait_enter_diag(void)
{   /* one-shot resolution facts: why freeze will/won't engage */
        DWORD app = 0, raw = 0, bb = 0;
        char nm[16] = {0};
        char hex[40];
        int ok, flg = -1, vt = 0, r178 = 0, hi = 0, k;
        ok = wait_board_ok(&bb, nm);
        if (!IsBadReadPtr((void*)APP_PTR, 4)) {
            app = *(DWORD*)APP_PTR;
            if (app > 0x10000 &&
                !IsBadReadPtr((void*)(app + APP_BOARD), 4)) {
                raw = *(DWORD*)(app + APP_BOARD);
                r178 = 1;
                if (raw > 0x10000 &&
                    !IsBadReadPtr((void*)raw, 4))
                    vt = *(DWORD*)raw;
                for (k = 0; k < 12 && hi < 36; k++) {
                    BYTE c = 0;
                    if (raw > 0x10000 &&
                        !IsBadReadPtr((void*)(raw + BOARD_NAME + (DWORD)k), 1))
                        c = *(BYTE*)(raw + BOARD_NAME + (DWORD)k);
                    hex[hi++] = "0123456789ABCDEF"[(c >> 4) & 15];
                    hex[hi++] = "0123456789ABCDEF"[c & 15];
                    hex[hi++] = ' ';
                }
                hex[hi] = 0;
                if (ok && !IsBadReadPtr((void*)bb, BOARD_PAUSE + 1))
                    flg = *(BYTE*)(bb + BOARD_PAUSE);
            }
        }
        if (!hi) { hex[0] = '-'; hex[1] = 0; }
        {   /* App+0x23C: 1 means a race is actually running */
            int st = -9;
            if (!IsBadReadPtr((void*)APP_PTR, 4)) {
                DWORD aa = *(DWORD*)APP_PTR;
                if (aa > 0x10000 &&
                    !IsBadReadPtr((void*)(aa + 0x23Cu), 4))
                    st = *(int*)(aa + 0x23Cu);
            }
            {   /* tap facts: vtable + name bytes at tap+0x868 */
                DWORD tv = 0;
                char tx[40]; int th = 0, k;
                if (g_tapBoard > 0x10000 &&
                    !IsBadReadPtr((void*)g_tapBoard, 4))
                    tv = *(DWORD*)g_tapBoard;
                for (k = 0; k < 8 && th < 32; k++) {
                    BYTE c = 0;
                    if (g_tapBoard > 0x10000 &&
                        !IsBadReadPtr((void*)(g_tapBoard + BOARD_NAME + (DWORD)k), 1))
                        c = *(BYTE*)(g_tapBoard + BOARD_NAME + (DWORD)k);
                    tx[th++] = "0123456789ABCDEF"[(c >> 4) & 15];
                    tx[th++] = "0123456789ABCDEF"[c & 15];
                    tx[th++] = ' ';
                }
                tx[th] = 0;
                {   /* foreground facts: who owns the fg window? */
                HWND fg = GetForegroundWindow();
                DWORD fp = 0;
                char ft[32] = {0};
                if (fg) {
                    GetWindowThreadProcessId(fg, &fp);
                    GetWindowTextA(fg, ft, (int)sizeof(ft) - 1);
                }
                log_msg("DEVBIND WAIT-enter: fgpid=%u me=%u fg=%.28s gate=%d\n",
                        fp, GetCurrentProcessId(), ft,
                        (fg && fp == GetCurrentProcessId()) ? 1 : 0);
            }
            log_msg("DEVBIND WAIT-enter: race=%d tap=%p tapvt=%08X tapname=%s\n",
                    st, (void*)g_tapBoard, tv, tx);
            }
        }
        log_msg("DEVBIND WAIT-enter: app=%p raw178=%s%p vtbl=%08X namehex=%s flag=%d\n",
                (void*)app, r178 ? "" : "BAD+", (void*)raw, vt, hex, flg);
    }

static void dev_action_fire(const char* name) {
    if (!strcmp(name, "Time Trials - Reset Best Times")) {
        if (g_indState != IND_REST) return;   /* only from resting state */
        g_flow = 1;
        g_indSel = -1;                        /* selector starts at ALL */
        g_indUntil = 0;                       /* WAIT is sticky, no expiry */
        ind_wait_text();
        g_indState = IND_WAIT;
        wait_enter_diag();
                log_msg("DEVBIND reset-best-times: awaiting level (ALL)\n");
        return;
    }
    if (!strcmp(name, "Time Trials - Set Medals")) {
        if (g_indState != IND_REST) return;   /* only from resting state */
        g_flow = 2;
        g_indSel = -1;                        /* level selector starts at ALL */
        g_indUntil = 0;                       /* WAIT1 is sticky, no expiry */
        ind_wait_text();
        g_indState = IND_WAIT;
        wait_enter_diag();
        log_msg("DEVBIND set-medals: awaiting level (ALL)\n");
        return;
    }
    log_msg("DEVBIND unhandled action: %s\n", name);
    /* unknown actions show their name briefly in yellow (legacy path) */
    strncpy(g_indicator, name, 127);
    g_indicator[127] = 0;
    g_indState = IND_WAIT;
    g_indUntil = GetTickCount() + IND_MS;
}

/* F8 poll on a private background thread: Sleep(16) loop, edge-detect,
 * toggle + log. Zero game interaction, works in every game state. Exits
 * when the App quit flag is set (polite shutdown, no hang). */

/* ── WAIT-state input capture ─────────────────────────────────────── */
/* ── WAIT input capture (LL hooks): game can't pause/unpause ────────── */
/* While devmode is ON and the indicator is WAIT, a WH_KEYBOARD_LL hook eats
 * the pause-bound keys (arrows/Enter/Escape + everything but F8/Ctrl/Alt)
 * and a WH_MOUSE_LL hook eats right-button down/up, so the game can neither
 * open its pause menu nor receive gameplay input. The selector is fed from
 * hook-recorded flags (InterlockedExchange edges) — dev keys keep working
 * even though the game never sees them. Foreground gate is pid-based
 * (fg window's pid == our pid == the game, we're its DLL), so other apps
 * are never affected. If hooks fail, the poll loop falls back to
 * GetAsyncKeyState edges (selector works, game also sees the keys). */

static HHOOK g_kbHook = NULL, g_msHook = NULL;
static volatile LONG g_hL = 0, g_hR = 0, g_hE = 0, g_hX = 0;

static int fg_is_game(void) {
    HWND fg = GetForegroundWindow();
    DWORD pid = 0;
    if (!fg) return 0;
    GetWindowThreadProcessId(fg, &pid);
    return pid == GetCurrentProcessId();
}

static LRESULT CALLBACK kb_proc(int code, WPARAM wp, LPARAM lp) {
    if (code == HC_ACTION && (wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN)) {
        DWORD vk = ((KBDLLHOOKSTRUCT*)lp)->vkCode;
        int quarantined = (int)(GetTickCount() - g_eatUntil) < 0;
        if (g_devmode && (in_wait() || quarantined) && fg_is_game()) {
            int wait = in_wait();
            if (vk == (DWORD)VK_LEFT) {
                if (wait) InterlockedExchange(&g_hL, 1); return 1;
            }
            if (vk == (DWORD)VK_RIGHT) {
                if (wait) InterlockedExchange(&g_hR, 1); return 1;
            }
            if (vk == (DWORD)VK_RETURN) {
                if (wait) InterlockedExchange(&g_hE, 1); return 1;
            }
            if (vk == (DWORD)VK_ESCAPE) {
                if (wait) InterlockedExchange(&g_hX, 1); return 1;
            }
            /* F8/Ctrl/Alt stay live; eat the rest (WAIT only) */
            if (wait && vk != (DWORD)VK_F8 && vk != (DWORD)VK_CONTROL &&
                vk != 0xA2u && vk != 0xA3u && vk != (DWORD)VK_MENU &&
                vk != 0xA4u && vk != 0xA5u) return 1;
        }
    }
    return CallNextHookEx(g_kbHook, code, wp, lp);
}

static LRESULT CALLBACK ms_proc(int code, WPARAM wp, LPARAM lp) {
    (void)lp;
    if (code == HC_ACTION && (wp == WM_RBUTTONDOWN || wp == WM_RBUTTONUP)) {
        int quarantined = (int)(GetTickCount() - g_eatUntil) < 0;
        if (g_devmode && (in_wait() || quarantined) && fg_is_game()) return 1;
    }
    return CallNextHookEx(g_msHook, code, wp, lp);
}

static DWORD WINAPI hook_thread(LPVOID param) {
    HMODULE self = NULL;
    MSG m;
    (void)param;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                     | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                     (LPCSTR)kb_proc, &self);
    g_kbHook = SetWindowsHookExA(WH_KEYBOARD_LL, kb_proc, self, 0);
    g_msHook = SetWindowsHookExA(WH_MOUSE_LL, ms_proc, self, 0);
    log_msg("KBHOOK kbd=%s mouse=%s\n",
            g_kbHook ? "ok" : "FAILED", g_msHook ? "ok" : "FAILED");
    if (!g_kbHook && !g_msHook) return 1;
    while (GetMessageA(&m, NULL, 0, 0) > 0) {
        TranslateMessage(&m);
        DispatchMessageA(&m);
    }
    return 0;
}

/* While devmode is ON and the indicator is WAIT, freeze the game with its
 * own pause: the poll loop holds board+0x874=1 on the live race board and
 * restores the prior value on every WAIT exit (confirm, escape, timeout,
 * devmode off). Dev keys are never eaten — the selector keeps its plain
 * GetAsyncKeyState edges. Identity guard: App+0x178 must name a *Board
 * (menu shells are skipped, never written). The native pause menu may
 * show while frozen; gameplay, timers and physics all stop. */


static int tap_board_ok(DWORD *boardOut, char *nameOut) {
    DWORD tb = g_tapBoard;
    int i;
    /* No vtable/name gates here: this ECX comes straight out of the game's
     * own pause check (mov 0x874(%ecx),%al), so +0x874 IS a pause flag on
     * it by construction. Readability guards only (object may die). */
    if (!g_tapOk || tb < 0x10000) return 0;
    if (IsBadReadPtr((void*)tb, 4)) return 0;
    if (IsBadReadPtr((void*)(tb + BOARD_PAUSE), 1)) return 0;
    if (IsBadWritePtr((void*)(tb + BOARD_PAUSE), 1)) return 0;
    for (i = 0; i < 11; i++) {
        char c = '?';
        if (!IsBadReadPtr((void*)(tb + BOARD_NAME + (DWORD)i), 1))
            c = *(char*)(tb + BOARD_NAME + (DWORD)i);
        nameOut[i] = (c >= 32 && c < 127) ? c : 0;
        if (!c || c == '?') { if (!c) break; else nameOut[i] = '?'; }
    }
    nameOut[11] = 0;
    *boardOut = tb;
    return 1;
}

static void wait_freeze_tick(void) {
    DWORD board;
    char name[16];
    if (!g_devmode || !in_wait()) {
        if (g_wePaused) {   /* WAIT exit: hold until the exit keys are up */
            int keysDown = ((GetAsyncKeyState(VK_RETURN) & 0x8000) != 0) ||
                           ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) ||
                           ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0);
            if (!g_exitTick) g_exitTick = GetTickCount();
            /* Cap: never hold more than 1.5s past exit (stuck key safety). */
            if (keysDown && (int)(GetTickCount() - g_exitTick - 1500) < 0) {
                if (g_pauseBoard &&
                    !IsBadReadPtr((void*)g_pauseBoard, BOARD_PAUSE + 1) &&
                    !IsBadWritePtr((void*)(g_pauseBoard + BOARD_PAUSE), 1))
                    *(BYTE*)(g_pauseBoard + BOARD_PAUSE) = 1;
                return;   /* still holding; restore comes after release */
            }
            if (g_pauseBoard && !IsBadReadPtr((void*)g_pauseBoard, BOARD_PAUSE + 1) &&
                !IsBadWritePtr((void*)(g_pauseBoard + BOARD_PAUSE), 1)) {
                *(BYTE*)(g_pauseBoard + BOARD_PAUSE) = g_pauseSaved;
                log_msg("DEVBIND WAIT exit: pause restored to %d\n", g_pauseSaved);
            }
            g_wePaused = 0;
            g_pauseBoard = 0;
            g_exitTick = 0;
        }
        return;
    }
    g_exitTick = 0;   /* in WAIT: exit stamp stays clear */
    if (!tap_board_ok(&board, name) &&
        !wait_board_ok(&board, name)) return;   /* menus: nothing to freeze */
    if (!g_wePaused || g_pauseBoard != board) {
        g_pauseSaved = *(BYTE*)(board + BOARD_PAUSE);
        *(BYTE*)(board + BOARD_PAUSE) = 1;
        g_wePaused = 1;
        g_pauseBoard = board;
        log_msg("DEVBIND WAIT: game frozen (pause held, was %d) on %s\n",
                g_pauseSaved, name);
        return;
    }
    if (!*(BYTE*)(board + BOARD_PAUSE)) {
        *(BYTE*)(board + BOARD_PAUSE) = 1;   /* menu RESUME clicked: re-hold */
        log_msg("DEVBIND WAIT: pause re-held\n");
    }
}

static DWORD WINAPI poll_thread(LPVOID param) {
    (void)param;
    for (;;) {
        SHORT ks;
        int down;
        Sleep(16);
        /* .set hot-reload (mtime check, 1s throttle) */
        {
            DWORD now = GetTickCount();
            if ((now - g_setTick) > 1000) {
                g_setTick = now;
                if (set_changed()) set_load();
            }
        }
        ks = GetAsyncKeyState(VK_F8);
        down = (ks & 0x8000) != 0;
        if (down && !g_prevF8) {
            g_devmode = !g_devmode;
            state_save();
            log_msg("F8 devmode=%d (%s)\n", g_devmode, g_devmode ? "on" : "off");
            if (g_devmode && set_changed()) set_load();
            if (!g_devmode && in_wait()) {
                ind_to_rest();   /* leaving devmode cancels level select */
                log_msg("DEVBIND reset-best-times: cancelled (devmode off)\n");
            }
        }
        g_prevF8 = down;
        /* dev keybinds: devmode ON + CTRL held, 1s global cooldown */
        {
            int ctrl = (GetAsyncKeyState(VK_CTRL) & 0x8000) != 0;
            int i;
            if (g_devmode && ctrl) {
                DWORD now = GetTickCount();
                if ((now - s_lastDevFire) >= 1000) {
                    for (i = 0; i < g_nbinds; i++) {
                        int d = (GetAsyncKeyState((int)g_binds[i].vk) & 0x8000) != 0;
                        if (d && !g_binds[i].prev) {
                            log_msg("DEVBIND fire '%s' (CTRL+%c)\n", g_binds[i].name, g_binds[i].key);
                            dev_action_fire(g_binds[i].name);
                            s_lastDevFire = now;
                            g_binds[i].prev = d;
                            break;   /* one bind per press */
                        }
                        g_binds[i].prev = d;
                    }
                } else {
                    for (i = 0; i < g_nbinds; i++)
                        g_binds[i].prev = (GetAsyncKeyState((int)g_binds[i].vk) & 0x8000) != 0;
                }
            } else {
                for (i = 0; i < g_nbinds; i++) g_binds[i].prev = 0;
            }
        }
        /* WAIT-state level select: arrows cycle ALL <-> LEVEL01..15
         * (wrap), enter confirms, escape cancels. Devmode ON only.
         * Pause-bound keys (arrows/Enter/Escape, right mouse) are eaten
         * by LL hooks so the game can neither pause nor unpause; the
         * selector is fed from hook flags (GetAsyncKeyState fallback if
         * the hooks failed). The freeze is held by wait_freeze_tick. */
        /* Loader gate: while CURRENTOBJECT is the loading screen, run
         * orig DrawGlyph only (no post-draw). Updated at 2Hz. */
        {
            static DWORD lastGateTick = 0;
            DWORD nowG = GetTickCount();
            if ((int)(nowG - lastGateTick) > 500) {
                lastGateTick = nowG;
                {
                    DWORD appG = 0, objG = 0;
                    char nmG[12] = {0};
                    int kG;
                    if (!IsBadReadPtr((void*)APP_PTR, 4)) {
                        appG = *(DWORD*)APP_PTR;
                        if (appG > 0x10000 &&
                            !IsBadReadPtr((void*)(appG + APP_BOARD), 4)) {
                            objG = *(DWORD*)(appG + APP_BOARD);
                            if (objG > 0x10000 &&
                                !IsBadReadPtr((void*)(objG + BOARD_NAME), 12)) {
                                for (kG = 0; kG < 11; kG++) {
                                    char cG = *(char*)(objG + BOARD_NAME + (DWORD)kG);
                                    nmG[kG] = (cG >= 32 && cG < 127) ? cG : 0;
                                    if (!cG) break;
                                }
                                nmG[11] = 0;
                                {
                                    int isLoad = (nmG[0]=='L'&&nmG[1]=='o'&&nmG[2]=='a'&&
                                                  nmG[3]=='d'&&nmG[4]=='i'&&nmG[5]=='n'&&
                                                  nmG[6]=='g');
                                    /* Boot phase: blank/unreadable also closes
                                     * (loader has a blank name). After the
                                     * boot phase only a real Loading name
                                     * closes; anything else opens. */
                                    int want = (isLoad || (g_bootPhase && nmG[0]==0)) ? 1 : 0;
                                    if (want != g_loaderGate) {
                                        g_loaderGate = want;
                                        log_msg("LOADERGATE %s (%.11s)\n",
                                                want ? "ON" : "off", nmG);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        if (g_bootPhase && (int)(GetTickCount() - g_bootTick) > 20000) {
            g_bootPhase = 0;
            log_msg("BOOTPHASE over\n");
        }

        {
            static DWORD lastBeat = 0;
            DWORD nowB = GetTickCount();
            if ((int)(nowB - lastBeat) > 10000) {
                lastBeat = nowB;
                log_msg("BEAT hits=%d draws=%d gate=%d pres=%d spr=%d\n",
                        g_hitCount, g_drawCount, g_loaderGate, g_presentCount, g_spriteHits);
            }
        }
        wait_freeze_tick();   /* HOLD native pause while WAIT, restore after */
        /* Medal-guard probe: arm while picking medals, disarm otherwise. */
        if (g_flow == 2 && (g_indState == IND_WAIT || g_indState == IND_WAIT2)) {
            if (!g_guardOn) {
                medal_guard_on();
                if (g_guardOn)
                    log_msg("GUARD armed (medal page %p)\n", (void*)g_guardPage);
            }
        } else if (g_guardOn) {
            medal_guard_off();
            log_msg("GUARD off\n");
        }
        /* Device Present hook: install once g_device is captured, heal if lost. */
        {
            static DWORD lastDevTick = 0;
            DWORD nowD = GetTickCount();
            if ((int)(nowD - lastDevTick) > 1000) {
                lastDevTick = nowD;
                install_dev_hook();
            }
        }
        {
            static int prevL = 0, prevR = 0, prevE = 0, prevX = 0;
            int l = 0, r = 0, e = 0, x = 0;
            if (g_devmode && in_wait()) {
                /* Hook flags OR physical edges: LL eating only stops
                 * message delivery, never GetAsyncKeyState, so the OR
                 * keeps the selector alive even if a flag never lands. */
                {
                    int pl = (GetAsyncKeyState(VK_LEFT) & 0x8000) != 0;
                    int pr = (GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0;
                    int pe = (GetAsyncKeyState(VK_RETURN) & 0x8000) != 0;
                    int px = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
                    l = InterlockedExchange(&g_hL, 0) || (pl && !prevL);
                    r = InterlockedExchange(&g_hR, 0) || (pr && !prevR);
                    e = InterlockedExchange(&g_hE, 0) || (pe && !prevE);
                    x = InterlockedExchange(&g_hX, 0) || (px && !prevX);
                    prevL = pl; prevR = pr; prevE = pe; prevX = px;
                }
                if (g_indState == IND_WAIT2) {
                    if (l || r) {
                        g_medSel += l ? -1 : 1;
                        if (g_medSel < 0) g_medSel = 4;
                        if (g_medSel > 4) g_medSel = 0;
                        ind_medwait_text();
                        /* realtime: write on every step so the menu
                         * medal icons change as you arrow through tiers */
                        set_medals_sel(g_indSel, g_medSel);
                        log_msg("DEVBIND set-medals: select %s\n", g_indicator);
                    }
                    if (e) {
                        if (set_medals_sel(g_indSel, g_medSel)) {
                            ind_meddone_text("DONE - TT SET MEDALS: ");
                            g_indState = IND_OK;
                        } else {
                            ind_meddone_text("FAIL - TT SET MEDALS: ");
                            g_indState = IND_FAIL;
                        }
                        g_indUntil = GetTickCount() + IND_MS;
                        g_eatUntil = g_indUntil - IND_MS + EAT_MS;
                        log_msg("DEVBIND set-medals: %s (%s)\n",
                                g_indState == IND_OK ? "SUCCESS" : "FAILED", g_indicator);
                    }
                    if (x) {
                        g_indState = IND_WAIT;   /* back to level question */
                        ind_wait_text();
                        log_msg("DEVBIND set-medals: back to level select\n");
                    }
                } else {
                if (l || r) {
                    g_indSel += l ? -1 : 1;
                    if (g_indSel < -1) g_indSel = BESTTIME_RACES - 1;
                    if (g_indSel > BESTTIME_RACES - 1) g_indSel = -1;
                    ind_wait_text();
                    log_msg("DEVBIND %s: select %s\n",
                            g_flow == 2 ? "set-medals" : "reset-best-times",
                            g_indicator);
                }
                if (e) {
                    if (g_flow == 2) {   /* level locked: ask the medal */
                        g_medSel = 0;
                        ind_medwait_text();
                        g_indState = IND_WAIT2;
                        /* sticky: no indUntil — WAIT2 never auto-reverts */
                        log_msg("DEVBIND set-medals: level locked, pick medal\n");
                    } else {
                        if (reset_best_times_sel(g_indSel)) {
                            ind_done_text("DONE - RESET BEST TIME: ");
                            g_indState = IND_OK;
                        } else {
                            ind_done_text("FAIL - RESET BEST TIME: ");
                            g_indState = IND_FAIL;
                        }
                        g_indUntil = GetTickCount() + IND_MS;
                        g_eatUntil = g_indUntil - IND_MS + EAT_MS;   /* quarantine the Enter */
                        log_msg("DEVBIND reset-best-times: %s (%s)\n",
                                g_indState == IND_OK ? "SUCCESS" : "FAILED", g_indicator);
                    }
                }
                if (x) {
                    ind_to_rest();
                    log_msg("DEVBIND %s: cancelled (escape)\n",
                            g_flow == 2 ? "set-medals" : "reset-best-times");
                }
                }
            } else {
                prevL = prevR = prevE = prevX = 0;
            }
        }
        if (!g_frameHookOk) g_drawArmed = 1;   /* fallback armer */
        if (!IsBadReadPtr((void*)APP_PTR, 4)) {
            DWORD app = *(DWORD*)APP_PTR;
            if (app && app > 0x10000 && !IsBadReadPtr((void*)app, 0x320)) {
                if (*(BYTE*)(app + APP_QUIT)) return 0;
            }
        }
    }
    return 0;
}

/* One status draw per frame, called from inside the game's own text    */
/* render (valid D3D scene guaranteed). Guarded against our own draw.  */

/* Glyph probe: every char we are about to print must have a live entry
 * (nonzero valid flag at entry+0x00) in this font's table at +0x42C.
 * Boot-time half-built fonts fail here -> we skip the draw.
 * NOTE: entry stride is 0x14; there is NO sprite pointer at +0x14
 * (live dump: that dword reads 0x1 = the NEXT glyph's valid flag).
 * Flag + readability is the correct readiness signal. */
static int font_ready(void* font, const char* s) {
    unsigned char* base;
    if (!font) return 0;
    if (IsBadReadPtr(font, 4)) return 0;
    if (IsBadReadPtr((char*)font + 0x42C, 0x500)) return 0;
    base = (unsigned char*)font + 0x42C;
    for (; *s; s++) {
        unsigned char* e = base + (unsigned char)(*s) * 0x14;
        if (IsBadReadPtr(e, 0x14)) return 0;
        if (!e[0]) return 0;   /* valid flag */
    }
    return 1;
}

static int g_entryCount = 0;
static int g_dtrace = 0;      /* verbose draw-path trace, first 40 */

static void draw_direct(void) {
    DWORD app;

    if (g_entryCount < 20) {
        DWORD hookvt = 0;
        if (g_drawFont && !IsBadReadPtr(g_drawFont, 4) &&
            !IsBadReadPtr(*(void**)g_drawFont, 4))
            hookvt = *(DWORD*)g_drawFont;
        log_msg("ENTRY n=%d hookfont=%p hookvt=%p gate=%d\n",
                g_entryCount, g_drawFont, (void*)hookvt, g_loaderGate);
    }
    g_entryCount++;
    /* No loader/boot gate: the pixel overlay draws at real Present on every
     * screen (menu/loading/race). The gate guarded the retired DrawGlyph
     * path; pixel-UP inside the game's own Present is the safe point. */
    if (IsBadReadPtr((void*)APP_PTR, 4)) return;
    app = *(DWORD*)APP_PTR;
    if (!app || app < 0x10000) return;
    if (IsBadReadPtr((void*)app, 0x320)) return;
    if (*(BYTE*)(app + APP_QUIT)) return;   /* quitting: skip */
    {
        DWORD now = GetTickCount();
        if ((int)(now - g_lastDrawTick) < 12) return;
        g_lastDrawTick = now;
    }
    if (g_drawCount < 12)
        log_msg("DRAW n=%d devmode=%d ind=%s\n", g_drawCount, g_devmode, indicator_text());
    g_drawCount++;
    draw_pixel_overlay();
}

/* ── Device Present hook: draw the overlay inside EVERY Present ───────── */
/* The menu presents through game code my 0x455B48/0x455AA6 hooks never see,
 * so the overlay only appeared in-race. IDirect3DDevice8::Present (vtable
 * slot 17) is the one funnel every screen flows through. Installed (and
 * self-healed if the device is recreated) from the poll thread.
 * Cave (__stdcall, 5 stack args untouched): pushad/pushfd, call
 * draw_direct, popfd/popad, jmp [g_origPresent]. */
static void install_dev_hook(void) {
    DWORD dev, vtbl, page, old = 0;
    BYTE* cave;
    BYTE* p;
    if (g_devHookOk) {
        /* self-heal: re-hook if the device was recreated */
        if (!g_device || IsBadReadPtr((void*)g_device, 4)) return;
        dev = g_device;
        vtbl = *(DWORD*)dev;
        if (vtbl == g_devVtbl && *(DWORD*)(vtbl + 68) == (DWORD)g_devCave_)
            return;
        g_devHookOk = 0;
        log_msg("DEVHOOK lost (device recreated?) — re-hooking\n");
    }
    if (!g_device || IsBadReadPtr((void*)g_device, 4)) return;
    dev = g_device;
    if (IsBadReadPtr(*(void**)dev, 0x140)) return;
    vtbl = *(DWORD*)dev;
    if (IsBadReadPtr((void*)(vtbl + 68), 4)) return;
    g_devVtbl = vtbl;
    g_origPresent = *(DWORD*)(vtbl + 68);
    cave = (BYTE*)alloc_executable(16);
    if (!cave) return;
    p = cave;
    *p++ = 0x60;                                            /* pushad */
    *p++ = 0x9C;                                            /* pushfd */
    *p++ = 0xE8;                                            /* call draw_direct */
    *(DWORD*)p = (DWORD)draw_direct - ((DWORD)p + 4); p += 4;
    *p++ = 0x9D;                                            /* popfd */
    *p++ = 0x61;                                            /* popad */
    *p++ = 0xFF; *p++ = 0x25;                               /* jmp [g_origPresent] */
    *(DWORD*)p = (DWORD)&g_origPresent; p += 4;
    page = vtbl & ~0xFFFu;
    if (!VirtualProtect((void*)page, 4096, PAGE_READWRITE, &old)) {
        log_msg("DEVHOOK VirtualProtect failed\n");
        return;
    }
    *(DWORD*)(vtbl + 68) = (DWORD)cave;
    VirtualProtect((void*)page, 4096, old, &old);
    g_devCave_ = (DWORD)cave;
    g_devHookOk = 1;
    log_msg("DEVHOOK ok (Present slot 17 -> %p, orig %p)\n",
            cave, (void*)g_origPresent);
}

/* ── Frame hook (0x46C1F1 epilogue): draws the overlay itself ───────── */
/* Stolen bytes (5): 5E 83 C4 08 C3 (pop esi / add esp,8 / ret).
 * Cave sets g_drawArmed=1, replays the epilogue, returns to the caller.
 * Runs on the game thread once per frame. */

static void install_frame_hook(void) {
    BYTE* a = (BYTE*)HOOK_ADDR;
    unsigned char* cave;
    unsigned char* p;
    if (IsBadReadPtr((void*)HOOK_ADDR, 5)) {
        log_msg("FRAMEHOOK skip: 0x46C1F1 unreadable\n");
        return;
    }
    if (!(a[0] == 0x5E && a[1] == 0x83 && a[2] == 0xC4 && a[3] == 0x08 && a[4] == 0xC3)) {
        log_msg("FRAMEHOOK skip: bytes mismatch %02X %02X %02X %02X %02X\n",
                a[0], a[1], a[2], a[3], a[4]);
        return;
    }
    cave = (unsigned char*)alloc_executable(16);
    if (!cave) {
        log_msg("FRAMEHOOK skip: VirtualAlloc failed\n");
        return;
    }
    p = cave;
    *p++ = 0x5E;                                /* pop esi */
    *p++ = 0x83; *p++ = 0xC4; *p++ = 0x08;      /* add esp,8 */
    *p++ = 0xC3;                                /* ret */
    install_jmp_hook_nop(HOOK_ADDR, (DWORD)cave, 5);
    g_frameHookOk = 1;
    log_msg("FRAMEHOOK installed at 0x46C1F1 -> cave %p\n", cave);
}

/* ── Present hook (0x455B48): draw right before Present ───────────── */
/* Stolen bytes (9, three whole instructions):
 *   50                push eax
 *   FF 51 3C          call dword ptr [ecx+0x3C] (Present, ECX = device)
 *   3D 68 08 76 88    cmp eax,0x88760868 (DEVICELOST)
 * Cave calls draw_direct first (pre-Present = end of the visible frame),
 * then replays the stolen bytes verbatim and returns to 0x455B51.
 * ECX (device) and the arg pushes are preserved across the draw via
 * pushad/popad, so the replayed Present sees exactly the caller state.
 * Straight line, no branches. */
static int g_presentHookOk = 0;   /* 1 once the pre-Present hook is live */

static void install_present_hook(void) {
    BYTE* a = (BYTE*)PRESENT_ADDR;
    unsigned char* cave;
    unsigned char* p;
    static const BYTE want[9] = {0x50,0xFF,0x51,0x3C,0x3D,0x68,0x08,0x76,0x88};
    int i;
    if (IsBadReadPtr((void*)PRESENT_ADDR, 9)) {
        log_msg("PRESENTHOOK skip: 0x455B48 unreadable\n");
        return;
    }
    for (i = 0; i < 9; i++) if (a[i] != want[i]) {
        log_msg("PRESENTHOOK skip: bytes mismatch\n");
        return;
    }
    cave = (unsigned char*)alloc_executable(64);
    if (!cave) {
        log_msg("PRESENTHOOK skip: VirtualAlloc failed\n");
        return;
    }
    p = cave;
    *p++ = 0xA3;                                /* mov [g_device],ecx (D3D device) */
    *(DWORD*)p = (DWORD)&g_device; p += 4;
    *p++ = 0xFF; *p++ = 0x05;                    /* inc dword [g_presentCount] */
    *(DWORD*)p = (DWORD)&g_presentCount; p += 4;
    *p++ = 0x60;                                /* pushad */
    *p++ = 0x9C;                                /* pushfd */
    *p++ = 0xE8;                                /* call draw_direct */
    *(DWORD*)p = (DWORD)draw_direct - ((DWORD)p + 4);
    p += 4;
    *p++ = 0x9D;                                /* popfd */
    *p++ = 0x61;                                /* popad */
    *p++ = 0x50;                                /* replay: push eax */
    *p++ = 0xFF; *p++ = 0x51; *p++ = 0x3C;      /* replay: call [ecx+0x3C] */
    *p++ = 0x3D; *p++ = 0x68; *p++ = 0x08; *p++ = 0x76; *p++ = 0x88;
                                                /* replay: cmp eax,DEVICELOST */
    *p++ = 0xE9;                                /* jmp back to 0x455B51 */
    *(DWORD*)p = (PRESENT_ADDR + 9) - ((DWORD)p + 4);
    p += 4;
    install_jmp_hook_nop(PRESENT_ADDR, (DWORD)cave, 9);
    g_presentHookOk = 1;
    log_msg("PRESENTHOOK installed at 0x455B48 -> cave %p\n", cave);
}

/* Second pre-Present site (0x455AA6, al==1 path): steals 4x push 0 +
 * push eax + call [ecx+0x3C] (12 bytes), replays them, returns to the
 * cmp at 0x455AB2. Same straight-line shape as the first site. */
static void install_present2_hook(void) {
    BYTE* a = (BYTE*)PRESENT2_ADDR;
    unsigned char* cave;
    unsigned char* p;
    static const BYTE want[12] = {0x6A,0x00,0x6A,0x00,0x6A,0x00,0x6A,0x00,
                                  0x50,0xFF,0x51,0x3C};
    int i;
    if (IsBadReadPtr((void*)PRESENT2_ADDR, 12)) {
        log_msg("PRESENT2HOOK skip: 0x455AA6 unreadable\n");
        return;
    }
    for (i = 0; i < 12; i++) if (a[i] != want[i]) {
        log_msg("PRESENT2HOOK skip: bytes mismatch\n");
        return;
    }
    cave = (unsigned char*)alloc_executable(64);
    if (!cave) {
        log_msg("PRESENT2HOOK skip: VirtualAlloc failed\n");
        return;
    }
    p = cave;
    *p++ = 0xA3;                                /* mov [g_device],ecx (D3D device) */
    *(DWORD*)p = (DWORD)&g_device; p += 4;
    *p++ = 0xFF; *p++ = 0x05;                    /* inc dword [g_presentCount] */
    *(DWORD*)p = (DWORD)&g_presentCount; p += 4;
    *p++ = 0x60;                                /* pushad */
    *p++ = 0x9C;                                /* pushfd */
    *p++ = 0xE8;                                /* call draw_direct */
    *(DWORD*)p = (DWORD)draw_direct - ((DWORD)p + 4);
    p += 4;
    *p++ = 0x9D;                                /* popfd */
    *p++ = 0x61;                                /* popad */
    *p++ = 0x6A; *p++ = 0x00;                   /* replay: 4x push 0 */
    *p++ = 0x6A; *p++ = 0x00;
    *p++ = 0x6A; *p++ = 0x00;
    *p++ = 0x6A; *p++ = 0x00;
    *p++ = 0x50;                                /* replay: push eax */
    *p++ = 0xFF; *p++ = 0x51; *p++ = 0x3C;      /* replay: call [ecx+0x3C] */
    *p++ = 0xE9;                                /* jmp back to 0x455AB2 */
    *(DWORD*)p = (PRESENT2_ADDR + 12) - ((DWORD)p + 4);
    p += 4;
    install_jmp_hook_nop(PRESENT2_ADDR, (DWORD)cave, 12);
    log_msg("PRESENT2HOOK installed at 0x455AA6 -> cave %p\n", cave);
}

/* ── Hook install (init thread only): DrawGlyph detour ──────────────── */

/* Font_DrawGlyph (0x457440) entry detour: capture only, fully transparent.
 *
 * Stolen bytes (7, two whole instructions, the SEH frame setup):
 *   6A FF             push -1
 *   68 50 CD 4C 00    push 0x4CCD50
 * Trampoline replays them verbatim then jumps to 0x457447, so the
 * game's SEH registration is established exactly as native.
 *
 * Cave (entered with [retC][p1..p8], ECX=font):
 *   if (g_inDraw) -> run orig only (our own draw must not recurse)
 *   else swap [esp] retC->AFTER and jump to trampoline; orig's
 *   RET 0x20 cleans p1..p8 and lands on AFTER with [retC] on stack,
 *   where draw_once_gated() runs (one status line per frame max).
 */

static void install_draw_hook(void) {
    BYTE* a = (BYTE*)HOOK2_ADDR;
    unsigned char* tramp;
    unsigned char* cave;
    unsigned char* p;
    DWORD afterAddr;
    DWORD afterEntry = 0;
    BYTE* afterImm = 0;

    /* verify-or-skip: expect 6A FF 68 50 CD 4C 00 */
    if (IsBadReadPtr((void*)HOOK2_ADDR, 7)) {
        log_msg("DRAWHOOK skip: 0x457440 unreadable\n");
        return;
    }
    if (!(a[0] == 0x6A && a[1] == 0xFF && a[2] == 0x68 && a[3] == 0x50 &&
          a[4] == 0xCD && a[5] == 0x4C && a[6] == 0x00)) {
        log_msg("DRAWHOOK skip: bytes mismatch %02X %02X %02X %02X %02X %02X %02X\n",
                a[0], a[1], a[2], a[3], a[4], a[5], a[6]);
        return;
    }

    tramp = (unsigned char*)alloc_executable(32);
    cave = (unsigned char*)alloc_executable(128);
    if (!tramp || !cave) {
        log_msg("DRAWHOOK skip: VirtualAlloc failed\n");
        return;
    }

    /* trampoline: replay stolen 7 bytes, jump to orig+7 */
    p = tramp;
    *p++ = 0x6A; *p++ = 0xFF;
    *p++ = 0x68; *p++ = 0x50; *p++ = 0xCD; *p++ = 0x4C; *p++ = 0x00;
    *p++ = 0xE9;
    *(DWORD*)p = (HOOK2_ADDR + 7) - ((DWORD)p + 4);
    p += 4;

    /* Minimal capture cave (boot-safe forever): count the hit, record
     * the live font, jump to the trampoline. No stack use, no register
     * use (flags saved and restored), no branches: orig+7 sees exactly
     * the caller state. All drawing happens in the frame hook. */
    p = cave;
    *p++ = 0x9C;                                 /* pushfd */
    *p++ = 0xFF; *p++ = 0x05;                    /* inc dword [g_hitCount] */
    *(DWORD*)p = (DWORD)&g_hitCount; p += 4;
    *p++ = 0x89; *p++ = 0x0D;                    /* mov [g_drawFont],ecx */
    *(DWORD*)p = (DWORD)&g_drawFont; p += 4;
    *p++ = 0x9D;                                 /* popfd */
    *p++ = 0xE9;                                /* jmp trampoline */
    *(DWORD*)p = (DWORD)tramp - ((DWORD)p + 4);
    p += 4;

    install_jmp_hook_nop(HOOK2_ADDR, (DWORD)cave, 7);
    log_msg("DRAWHOOK installed at 0x457440 -> cave %p tramp %p\n", cave, tramp);
}

/* ── Sprite hook (0x418870): piggyback the menu/HUD atlas ──────────── */
/* The menus and HUD draw their text as sprites through here with the
 * font atlas already bound. A post-call cave runs draw_direct AFTER
 * orig: our DrawGlyph call then samples the live atlas and paints.
 * Caller-ret is swapped (same stack depth: ESP-relative args and the
 * SEH record stay exactly native) and stashed on a LIFO (our own
 * DrawGlyph draws glyph sprites through this same hook: nesting).
 * 12ms throttle allows ~1 draw pass per frame. */
static void install_sprite_hook(void) {
    BYTE* a = (BYTE*)SPRITE_ADDR;
    unsigned char* tramp;
    unsigned char* cave;
    unsigned char* p;
    if (IsBadReadPtr((void*)SPRITE_ADDR, 7)) {
        log_msg("SPRITEHOOK skip: 0x418870 unreadable\n");
        return;
    }
    if (!(a[0] == 0x6A && a[1] == 0xFF && a[2] == 0x68 && a[3] == 0xE0 &&
          a[4] == 0x94 && a[5] == 0x4C && a[6] == 0x00)) {
        log_msg("SPRITEHOOK skip: bytes mismatch\n");
        return;
    }
    tramp = (unsigned char*)alloc_executable(32);
    cave = (unsigned char*)alloc_executable(128);
    if (!tramp || !cave) {
        log_msg("SPRITEHOOK skip: VirtualAlloc failed\n");
        return;
    }
    p = tramp;
    *p++ = 0x6A; *p++ = 0xFF;
    *p++ = 0x68; *p++ = 0xE0; *p++ = 0x94; *p++ = 0x4C; *p++ = 0x00;
    *p++ = 0xE9;
    *(DWORD*)p = (SPRITE_ADDR + 7) - ((DWORD)p + 4);
    p += 4;
    /* Post-call shape: capture ECX/retaddr, swap the caller-ret slot with
     * our postdraw address (same stack depth: ESP-relative args and the
     * SEH record stay exactly native), stash the caller-ret, replay SEH,
     * JMP orig+7. Orig's epilogue RETs into postdraw; postdraw draws,
     * then jmps to the stashed caller-ret. (A nested CALL-retaddr or an
     * extra push would shift the SEH record and corrupt unwinding.) */
    p = cave;
    *p++ = 0xFF; *p++ = 0x05;                    /* inc dword [g_spriteHits] */
    *(DWORD*)p = (DWORD)&g_spriteHits; p += 4;
    *p++ = 0x89; *p++ = 0x0D;                    /* mov [g_spriteTex],ecx */
    *(DWORD*)p = (DWORD)&g_spriteTex; p += 4;
    *p++ = 0x8B; *p++ = 0x04; *p++ = 0x24;       /* mov eax,[esp] (retaddr) */
    *p++ = 0xA3;                                /* mov [g_spriteRet],eax */
    *(DWORD*)p = (DWORD)&g_spriteRet; p += 4;
    *p++ = 0xE8;                                /* call origbody (cave+64) */
    *(DWORD*)p = (DWORD)(cave + 64) - ((DWORD)p + 4);
    p += 4;
    /* POSTDRAW (cave+25): pushad-guarded draw, then jmp to caller. */
    *p++ = 0x60;                                /* pushad */
    *p++ = 0x9C;                                /* pushfd */
    *p++ = 0xE8;                                /* call draw_direct */
    *(DWORD*)p = (DWORD)draw_direct - ((DWORD)p + 4);
    p += 4;
    *p++ = 0x9D;                                /* popfd */
    *p++ = 0x61;                                /* popad */
    *p++ = 0x8B; *p++ = 0x0D;                   /* mov ecx,[g_callerDepth] */
    *(DWORD*)p = (DWORD)&g_callerDepth; p += 4;
    *p++ = 0x49;                                /* dec ecx */
    *p++ = 0x89; *p++ = 0x0D;                   /* mov [g_callerDepth],ecx */
    *(DWORD*)p = (DWORD)&g_callerDepth; p += 4;
    *p++ = 0xFF; *p++ = 0x24; *p++ = 0x8D;       /* jmp [g_callerStack+ecx*4] */
    *(DWORD*)p = (DWORD)g_callerStack; p += 4;
    /* origbody at cave+64: pop postdraw-ret, swap [esp] with it, stash
     * caller-ret on the LIFO, replay SEH, jmp orig+7. ECX is the live
     * thiscall texture pointer: only EAX/EDX (caller-saved, dead) are
     * touched here (a mov ecx here killed the texture -> 0x40AF93). */
    {
        unsigned char* q = cave + 64;
        *q++ = 0x5A;                                            /* pop edx (=postdraw) */
        *q++ = 0x87; *q++ = 0x14; *q++ = 0x24;                  /* xchg edx,[esp] */
        *q++ = 0xA1;                                            /* mov eax,[g_callerDepth] */
        *(DWORD*)q = (DWORD)&g_callerDepth; q += 4;
        *q++ = 0x89; *q++ = 0x14; *q++ = 0x85;                  /* mov [stack+eax*4],edx */
        *(DWORD*)q = (DWORD)g_callerStack; q += 4;
        *q++ = 0x40;                                            /* inc eax */
        *q++ = 0xA3;                                            /* mov [g_callerDepth],eax */
        *(DWORD*)q = (DWORD)&g_callerDepth; q += 4;
        *q++ = 0x6A; *q++ = 0xFF;                               /* push -1 */
        *q++ = 0x68; *q++ = 0xE0; *q++ = 0x94; *q++ = 0x4C; *q++ = 0x00;
        *q++ = 0xE9;                                            /* jmp orig+7 */
        *(DWORD*)q = (SPRITE_ADDR + 7) - ((DWORD)q + 4);
        q += 4;
    }
    install_jmp_hook_nop(SPRITE_ADDR, (DWORD)cave, 7);
    log_msg("SPRITEHOOK installed at 0x418870 -> cave %p tramp %p\n", cave, tramp);
}

/* Frame-phase draw: stands down once the post-boot AFTER path is armed
 * (avoids double-draw). Plain C, no cave branches. */
static void draw_from_frame(void) {
    draw_direct();
}

static DWORD WINAPI init_thread(LPVOID param) {
    (void)param;
    g_bootTick = GetTickCount();
    log_open();
    state_load();   /* restore last F8 toggle */
    AddVectoredExceptionHandler(1, crash_spy);
    set_load();   /* initial .set read (logs binds or missing file) */
    Sleep(2000);   /* let game init settle before patching .text */
    install_frame_hook();
    install_draw_hook();
    install_present_hook();
    install_present2_hook();
    install_pause_tap();
    CreateThread(NULL, 0, hook_thread, NULL, 0, NULL);
    CreateThread(NULL, 0, poll_thread, NULL, 0, NULL);
    return 0;
}

/* ── DLL entry (loader-lock safe: no patches here) ─────────────────── */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    (void)lpReserved;
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        load_real_bass();
        CreateThread(NULL, 0, init_thread, NULL, 0, NULL);
    }
    return TRUE;
}
