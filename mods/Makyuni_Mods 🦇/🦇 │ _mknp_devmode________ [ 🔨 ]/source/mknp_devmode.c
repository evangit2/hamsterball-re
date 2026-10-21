/*
 /* mknp_devmode v01j — F8 toggles a devmode flag with on-screen status text.
  *
  * Architecture (all state-independent):
  *  - F8 poll runs on a private background thread (Sleep 16ms): works in
  *    menus, races, loading — everywhere. Zero game interaction.
  *  - Status line is drawn from a Font_DrawGlyph (0x457440) entry detour:
  *    orig runs via trampoline (SEH setup replayed verbatim), then the
  *    post-draw emits one direct DrawGlyph call (machine-code stub, no
  *    inline-asm pushes) with the live hook font.
  * Both installs are verify-or-skip from the init thread, never DllMain.
  */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "bass_proxy.h"   /* BASS proxy exports + load_real_bass + patch helpers */

/* ── Addresses ─────────────────────────────────────────────────────── */

#define APP_PTR     0x005341E0u
#define APP_QUIT    0x159u
#define APP_FONT28  0x318u
#define FN_DRAWTEXT 0x00409B90u   /* UI_DrawTextShadow_Wrapper __thiscall, RET 0x3C */
#define FN_DRAWGLYPH 0x00457440u  /* Font_DrawGlyph __thiscall, RET 0x20: (text,x,y,xform,r,g,b,a) */
#define HOOK_ADDR   0x0046C1F1u   /* frame epilogue: 5E 83 C4 08 C3 */
#define HOOK2_ADDR  0x00457440u   /* Font_DrawGlyph entry (SEH frame, replayed by trampoline) */
#define VK_F8       0x77

#define DEV_X 20
#define DEV_Y 20

/* ── State ─────────────────────────────────────────────────────────── */

static int g_devmode = 0;     /* the devmode variable: F8 toggles 0/1 */
static int g_prevF8  = 0;     /* previous F8 down-state (edge detect) */
static volatile int g_drawArmed = 0;  /* set each frame by C6F1; consumed by DrawGlyph hook */
static volatile int g_inDraw = 0;     /* reentrancy guard for our own draw */
static void* g_drawFont = 0;          /* live Font* captured from the hook's ECX */
static int g_drawCount = 0;           /* successful draw attempts (diagnostic) */

/* ── Log (mknp_devmode.log next to the DLL, %TEMP% fallback) ────────── */

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
        snprintf(path, MAX_PATH, "%smknp_devmode.log", dir);
        g_log = fopen(path, "a");
    }
    if (!g_log) {
        char tmp[MAX_PATH];
        if (GetTempPathA(MAX_PATH, tmp)) {
            snprintf(path, MAX_PATH, "%smknp_devmode.log", tmp);
            g_log = fopen(path, "a");
        }
    }
    if (!g_log) return;
    fprintf(g_log, "INIT mknp_devmode v01j self=%p\n", hSelf);
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

/* ── Crash spy (first-chance VEH): logs REAL fault EIP, then passes ── */
/* through to the game's own handler. Latches the primary.             */
static volatile LONG g_vehDone = 0;
static LONG CALLBACK crash_spy(PEXCEPTION_POINTERS ep) {
    if (InterlockedExchange(&g_vehDone, 1)) return EXCEPTION_CONTINUE_SEARCH;
    {
        DWORD eip = (DWORD)ep->ExceptionRecord->ExceptionAddress;
        DWORD code = ep->ExceptionRecord->ExceptionCode;
        CONTEXT* c = ep->ContextRecord;
        log_msg("SPY primary code=%08X EIP=%08X eax=%08X ebx=%08X ecx=%08X edx=%08X esi=%08X edi=%08X ebp=%08X esp=%08X draws=%d\n",
                code, eip, c->Eax, c->Ebx, c->Ecx, c->Edx, c->Esi, c->Edi, c->Ebp, c->Esp, g_drawCount);
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

/* v01j: single white pass, NO scale tweak. (v01i's 0.75-scale colored
 * path rendered nothing — the scaled Scene_CreateObject4f path needs
 * setup our post-draw context lacks. White glyph sprites always draw.) */
static void draw_status(void* font, const char* text) {
    const unsigned int one = 0x3F800000u;   /* 1.0f */
    const unsigned int xf = 0x004CF300u;    /* static identity transform */
    DWORD params[8];
    params[0] = (DWORD)text;
    params[1] = (DWORD)DEV_X;
    params[2] = (DWORD)DEV_Y;
    params[3] = xf;
    params[4] = one; params[5] = one; params[6] = one; params[7] = one;
    call_thiscall_n(FN_DRAWGLYPH, (DWORD)font, params, 8);
}

/* ── Per-frame handler (runs on the game thread at the epilogue) ───── */
/* F8 poll only. The status draw happens in render context via the      */
/* Font_DrawGlyph post-hook (g_drawArmed hands one draw per frame).     */

static void draw_direct(void);

/* F8 poll on a private background thread: Sleep(16) loop, edge-detect,
 * toggle + log. Zero game interaction, works in every game state. Exits
 * when the App quit flag is set (polite shutdown, no hang). */
static DWORD WINAPI poll_thread(LPVOID param) {
    (void)param;
    for (;;) {
        SHORT ks;
        int down;
        Sleep(16);
        ks = GetAsyncKeyState(VK_F8);
        down = (ks & 0x8000) != 0;
        if (down && !g_prevF8) {
            g_devmode = !g_devmode;
            log_msg("F8 devmode=%d (%s)\n", g_devmode, g_devmode ? "on" : "off");
        }
        g_prevF8 = down;
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

static void draw_direct(void) {
    DWORD app;
    const char* text;
    void* font;

    if (g_entryCount < 20) {
        log_msg("ENTRY n=%d hookfont=%p\n", g_entryCount, g_drawFont);
    }
    g_entryCount++;

    if (IsBadReadPtr((void*)APP_PTR, 4)) return;
    app = *(DWORD*)APP_PTR;
    if (!app || app < 0x10000) return;
    if (IsBadReadPtr((void*)app, 0x320)) return;
    if (*(BYTE*)(app + APP_QUIT)) return;   /* quitting: skip */

    text = g_devmode ? "devmode: on" : "devmode: off";
    /* v01g: font cascade — hook font first (whatever the game draws with),
     * then App slots 0x318/0x320/0x324. In-race HUD draws with the 72pt
     * timer font (App+0x328) which lacks our glyphs entirely (live-verified
     * flag=0), so hook-font-only probing goes silent in races. */
    font = g_drawFont;
    if (font && font_ready(font, text)) goto probed;
    font = 0;
    if (!IsBadReadPtr((void*)app, 0x330)) {
        static const DWORD slots[] = { 0x318u, 0x320u, 0x324u };
        int i;
        for (i = 0; i < 3 && !font; i++) {
            void* f;
            if (IsBadReadPtr((void*)(app + slots[i]), 4)) continue;
            f = *(void**)(app + slots[i]);
            if (f && font_ready(f, text)) font = f;
        }
    }
    if (!font) {
        if (g_entryCount <= 20) log_msg("ENTRY n=%d NOFONT\n", g_entryCount - 1);
        return;
    }
probed:
    if (!font_ready(font, text)) return;   /* font not ready yet; try next text draw */

    g_inDraw = 1;
    if (g_drawCount < 6) log_msg("DRAW n=%d font=%p textptr=%p text=%s\n", g_drawCount, font, text, text);
    g_drawCount++;
    draw_status(font, text);
    g_inDraw = 0;
}

/* ── Hook install (init thread only): DrawGlyph detour ──────────────── */

/* Font_DrawGlyph (0x457440) entry detour with post-call draw.
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
static DWORD g_slotRet;

static void install_draw_hook(void) {
    BYTE* a = (BYTE*)HOOK2_ADDR;
    unsigned char* tramp;
    unsigned char* cave;
    unsigned char* p;
    DWORD afterAddr, fwdAddr;

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

    /* cave layout (offsets fixed): AFTER=0x23, FORWARD=0x33, size=0x38.
     * Entry: [retC][p1..p8], ECX=live Font*. First instruction captures
     * ECX so the post-draw uses the exact font the game is drawing with. */
    p = cave;
    *p++ = 0x89; *p++ = 0x0D;                    /* mov [g_drawFont],ecx */
    *(DWORD*)p = (DWORD)&g_drawFont; p += 4;
    *p++ = 0x83; *p++ = 0x3D;                    /* cmp byte ptr [g_inDraw],1 */
    *(DWORD*)p = (DWORD)&g_inDraw; p += 4;
    *p++ = 0x01;
    *p++ = 0x74; *p++ = 0x24;                    /* je FORWARD (0x33) */
    *p++ = 0x8B; *p++ = 0x04; *p++ = 0x24;       /* mov eax,[esp] (retC) */
    *p++ = 0xA3;                                /* mov [slotRet],eax */
    *(DWORD*)p = (DWORD)&g_slotRet; p += 4;
    afterAddr = (DWORD)cave + 0x23;
    *p++ = 0xC7; *p++ = 0x04; *p++ = 0x24;       /* mov dword [esp],AFTER */
    *(DWORD*)p = afterAddr; p += 4;
    *p++ = 0xE9;                                /* jmp trampoline */
    *(DWORD*)p = (DWORD)tramp - ((DWORD)p + 4);
    p += 4;
    /* AFTER (0x1D): orig did RET 0x20 -> stack holds caller frame.
     * draw, then push the saved retC and return to the caller. */
    *p++ = 0x60;                                /* pushad */
    *p++ = 0x9C;                                /* pushfd */
    *p++ = 0xE8;                                /* call draw_direct */
    *(DWORD*)p = (DWORD)draw_direct - ((DWORD)p + 4);
    p += 4;
    *p++ = 0x9D;                                /* popfd */
    *p++ = 0x61;                                /* popad */
    *p++ = 0xFF; *p++ = 0x35;                   /* push dword [slotRet] */
    *(DWORD*)p = (DWORD)&g_slotRet; p += 4;
    *p++ = 0xC3;                                /* ret -> retC */
    /* FORWARD (0x2D): reentrant (our own draw) -> orig only */
    *p++ = 0xE9;                                /* jmp trampoline */
    fwdAddr = (DWORD)p;
    *(DWORD*)p = (DWORD)tramp - (fwdAddr + 4);
    p += 4;

    install_jmp_hook_nop(HOOK2_ADDR, (DWORD)cave, 7);
    log_msg("DRAWHOOK installed at 0x457440 -> cave %p tramp %p\n", cave, tramp);
}

static DWORD WINAPI init_thread(LPVOID param) {
    (void)param;
    log_open();
    AddVectoredExceptionHandler(1, crash_spy);
    Sleep(2000);   /* let game init settle before patching .text */
    install_draw_hook();   /* v01f: single hook (C6F1 epilogue dropped) */
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
