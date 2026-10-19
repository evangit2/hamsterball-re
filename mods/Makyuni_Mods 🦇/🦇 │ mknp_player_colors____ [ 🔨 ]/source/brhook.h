/* brhook.h -- per-draw P1 ball isolation for mknp_player_colors.
 * 5-byte JMP detour on Ball_Render 0x403DC0 (ECX = ball, __thiscall;
 * prologue 6A FF 64 A1 00... verified-or-skip, foreign-exe safe).
 * The hook writes the cached mesh-mat +0x34 green for P1, native gray
 * for everyone else. Outline (border mat) untouched. Hot path: no
 * calls, no logging; C side (br_resolve) owns validation + caching.
 * State lives in asm .bss globals (MinGW cannot reference C statics
 * from file-scope asm). Include AFTER HamsterballAPI.h/hbplus_api.h.
 */
#ifndef BRHOOK_H
#define BRHOOK_H

#define BALL_RENDER_HOOK 0x00403DC0

extern DWORD g_br_p1;
extern DWORD g_br_mat;
extern int g_br_ok;
extern float g_br_nat0;
extern float g_br_nat1;
extern float g_br_nat2;
extern float g_br_natS0;
extern float g_br_natS1;
extern float g_br_natS2;
extern float g_br_natR;
extern float g_br_nat20;
extern float g_hook_rgb0;
extern float g_hook_rgb1;
extern float g_hook_rgb2;
extern float g_hook_spc0;
extern float g_hook_spc1;
extern float g_hook_spc2;
extern float g_br_natD0;
extern float g_br_natD1;
extern float g_br_natD2;
extern float g_hook_pow;
extern float g_br_natPow;
extern DWORD g_br_tramp;
extern int g_br_inst;
extern int g_br_natlock;
extern int g_do_body;
extern int g_do_spec;
extern int g_br_needscrub;
extern int g_br_sub;

extern "C" void ballhook_asm(void);
__asm__(
".bss\n"
".globl _g_br_p1\n"
"_g_br_p1: .space 4\n"
".globl _g_br_mat\n"
"_g_br_mat: .space 4\n"
".globl _g_br_ok\n"
"_g_br_ok: .space 4\n"
".globl _g_br_nat0\n"
"_g_br_nat0: .space 4\n"
".globl _g_br_nat1\n"
"_g_br_nat1: .space 4\n"
".globl _g_br_nat2\n"
"_g_br_nat2: .space 4\n"
".globl _g_br_natS0\n"
"_g_br_natS0: .space 4\n"
".globl _g_br_natS1\n"
"_g_br_natS1: .space 4\n"
".globl _g_br_natS2\n"
"_g_br_natS2: .space 4\n"
".globl _g_br_natR\n"
"_g_br_natR: .space 4\n"
".globl _g_br_nat20\n"
"_g_br_nat20: .space 4\n"
".globl _g_hook_rgb0\n"
"_g_hook_rgb0: .space 4\n"
".globl _g_hook_rgb1\n"
"_g_hook_rgb1: .space 4\n"
".globl _g_hook_rgb2\n"
"_g_hook_rgb2: .space 4\n"
".globl _g_hook_spc0\n"
"_g_hook_spc0: .space 4\n"
".globl _g_hook_spc1\n"
"_g_hook_spc1: .space 4\n"
".globl _g_hook_spc2\n"
"_g_hook_spc2: .space 4\n"
".globl _g_br_natD0\n"
"_g_br_natD0: .space 4\n"
".globl _g_br_natD1\n"
"_g_br_natD1: .space 4\n"
".globl _g_br_natD2\n"
"_g_br_natD2: .space 4\n"
".globl _g_hook_pow\n"
"_g_hook_pow: .space 4\n"
".globl _g_br_natPow\n"
"_g_br_natPow: .space 4\n"
".globl _g_br_tramp\n"
"_g_br_tramp: .space 4\n"
".globl _g_br_inst\n"
"_g_br_inst: .space 4\n"
".globl _g_br_natlock\n"
"_g_br_natlock: .space 4\n"
".globl _g_do_body\n"
"_g_do_body: .space 4\n"
".globl _g_do_spec\n"
"_g_do_spec: .space 4\n"
".globl _g_br_needscrub\n"
"_g_br_needscrub: .space 4\n"
".globl _g_br_sub\n"
"_g_br_sub: .space 4\n"
".text\n"
".globl _ballhook_asm\n"
"_ballhook_asm:\n"
" pushl %eax\n"
" pushl %edx\n"
" movl _g_br_ok, %eax\n"
" testl %eax, %eax\n"
" jz 9f\n"
" movl _g_br_mat, %edx\n"
" testl %edx, %edx\n"
" jz 9f\n"
" cmpl _g_br_p1, %ecx\n"
" je 1f\n"
/* P1 vs others differ ONLY in emissive + diffuse (both = set color) plus
 * P1's set specular. Others get a BLACK spec-zone: with SPECULARENABLE now
 * forced on for the body, native spec-R=1.0 would paint red dots on them.
 * The black test proved zeroing is visually safe. +0x30/+0x2C untouched. */
" movl _g_br_natR, %eax\n"
" movl %eax, -4(%edx)\n"
" movl $0, -20(%edx)\n"
" movl _g_br_nat0, %eax\n"
" movl %eax, (%edx)\n"
" movl _g_br_nat1, %eax\n"
" movl %eax, 4(%edx)\n"
" movl _g_br_nat2, %eax\n"
" movl %eax, 8(%edx)\n"
" movl $0, -16(%edx)\n"
" movl $0, -12(%edx)\n"
" movl $0, -8(%edx)\n"
" movl _g_br_natD0, %eax\n"
" movl %eax, -48(%edx)\n"
" movl _g_br_natD1, %eax\n"
" movl %eax, -44(%edx)\n"
" movl _g_br_natD2, %eax\n"
" movl %eax, -40(%edx)\n"
" movl _g_br_natPow, %eax\n"
" movl %eax, 12(%edx)\n"
" jmp 9f\n"
"1:\n"
" movl _g_hook_rgb0, %eax\n"
" movl %eax, (%edx)\n"
" movl %eax, -48(%edx)\n"
" movl _g_hook_rgb1, %eax\n"
" movl %eax, 4(%edx)\n"
" movl %eax, -44(%edx)\n"
" movl _g_hook_rgb2, %eax\n"
" movl %eax, 8(%edx)\n"
" movl %eax, -40(%edx)\n"
/* DOT-BISECT step 2: P1 diffuse back to set color (spec still native).
 * If the white dot turns body-colored/vanishes => dot follows diffuse.
 * Gates: no ball key => P1 fully native; no spec key => spec-zone black. */
" cmpl $0, _g_do_body\n"
" je 4f\n"
/* step 3 (dif-only isolation): emissive restored native even with the
 * ball key present. If the body still tints green => diffuse channel is
 * live (dot green => dot follows diffuse). If the body goes vanilla
 * white => diffuse writes are dead and the body was emissive-only. */
" movl _g_br_nat0, %eax\n"
" movl %eax, (%edx)\n"
" movl _g_br_nat1, %eax\n"
" movl %eax, 4(%edx)\n"
" movl _g_br_nat2, %eax\n"
" movl %eax, 8(%edx)\n"
" movl _g_hook_rgb0, %eax\n"
" movl %eax, -48(%edx)\n"
" movl _g_hook_rgb1, %eax\n"
" movl %eax, -44(%edx)\n"
" movl _g_hook_rgb2, %eax\n"
" movl %eax, -40(%edx)\n"
" jmp 5f\n"
"4:\n"
" movl _g_br_nat0, %eax\n"
" movl %eax, (%edx)\n"
" movl _g_br_nat1, %eax\n"
" movl %eax, 4(%edx)\n"
" movl _g_br_nat2, %eax\n"
" movl %eax, 8(%edx)\n"
" movl _g_br_natD0, %eax\n"
" movl %eax, -48(%edx)\n"
" movl _g_br_natD1, %eax\n"
" movl %eax, -44(%edx)\n"
" movl _g_br_natD2, %eax\n"
" movl %eax, -40(%edx)\n"
"5:\n"
" cmpl $0, _g_do_spec\n"
" je 6f\n"
" movl _g_hook_spc0, %eax\n"
" movl %eax, -20(%edx)\n"
" movl _g_hook_spc1, %eax\n"
" movl %eax, -16(%edx)\n"
" movl _g_hook_spc2, %eax\n"
" movl %eax, -12(%edx)\n"
" movl _g_br_natS2, %eax\n"
" movl %eax, -8(%edx)\n"
" jmp 7f\n"
"6:\n"
" movl $0, -20(%edx)\n"
" movl $0, -16(%edx)\n"
" movl $0, -12(%edx)\n"
" movl $0, -8(%edx)\n"
"7:\n"
" movl _g_br_natPow, %eax\n"
" movl %eax, 12(%edx)\n"
"9:\n"
" popl %edx\n"
" popl %eax\n"
/* stolen prologue lives in the trampoline; replaying it here would push
 * twice and shift ESP (v1 crash). Jump straight to the trampoline. */
" jmp *_g_br_tramp\n");

static void ballhook_install(void) {
    unsigned char* p;
    unsigned char* t;
    DWORD old = 0;
    int i;
    if (g_br_inst) return;
    p = (unsigned char*)BALL_RENDER_HOOK;
    if (IsBadReadPtr((void*)p, 8)) return;
    if (p[0] != 0x6A || p[1] != 0xFF || p[2] != 0x64 || p[3] != 0xA1) {
        g_br_inst = -1;
        log_line("BALLHOOK head mismatch, no P1 isolation");
        return;
    }
    t = (unsigned char*)VirtualAlloc(0, 32, MEM_COMMIT | MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE);
    if (!t) return;
    for (i = 0; i < 8; i++) t[i] = p[i];
    t[8] = 0xE9;
    *(DWORD*)(t + 9) = (DWORD)(p + 8) - (DWORD)(t + 13);
    if (!VirtualProtect((void*)p, 8, PAGE_EXECUTE_READWRITE, &old)) return;
    p[0] = 0xE9;
    *(DWORD*)(p + 1) = (DWORD)ballhook_asm - (DWORD)(p + 5);
    p[5] = 0x90;
    p[6] = 0x90;
    p[7] = 0x90;
    VirtualProtect((void*)p, 8, old, &old);
    FlushInstructionCache(GetCurrentProcess(), (void*)p, 8);
    g_br_tramp = (DWORD)t;
    g_br_inst = 1;
    log_line("BALLHOOK installed");
    /* SPEC-EN: Ball_Render kills specular for the body (0x4041CF: push 0 =
       SetRenderState(29,0)) then re-enables it for the outline (0x4042EF:
       push 1). Flip the body site to 1 once per boot so P1's set specular
       can show; verify-or-skip (fresh image each launch). Others get a
       black spec-zone per draw, so only P1 gains a dot. */
    {
        unsigned char* s = (unsigned char*)0x4041CF;
        if (!IsBadReadPtr((void*)s, 2) && s[0] == 0x6A && s[1] == 0x00) {
            DWORD o2 = 0;
            if (VirtualProtect((void*)s, 2, PAGE_EXECUTE_READWRITE, &o2)) {
                s[1] = 0x01;
                VirtualProtect((void*)s, 2, o2, &o2);
                FlushInstructionCache(GetCurrentProcess(), (void*)s, 2);
                log_line("SPEC patch 6A00->6A01 ok");
            } else log_line("SPEC patch unprotect failed");
        } else log_line("SPEC patch site unexpected, skipped");
    }
}

/* Resolve + cache the P1 mesh-mat slot (native chain:
 * mat = *(*(mw+8)+0x28) + *(mw+4)*0x50, color at +0x34).
 * Heals r1 damage (+0x10 RGB back to native white). Captures native
 * +0x34 once (never our green). Owns no color writes otherwise. */
static void br_resolve(App* app) {
    void* b = (void*)HBAPI(g_api).GetPlayer();
    if (!b || IsBadReadPtr(b, 0x758)) { g_br_ok = 0; return; }
    g_br_p1 = (DWORD)b;
    int meshidx = *(int*)((char*)b + 0x754);
    if (meshidx < 0 || meshidx > 10) meshidx = 0;
    if (!app || IsBadReadPtr((const void*)((char*)app + 0x248), 4)) {
        g_br_ok = 0;
        return;
    }
    DWORD mw = *(DWORD*)((char*)app + 0x244 + (DWORD)meshidx * 4);
    if (!mw || IsBadReadPtr((const void*)mw, 0x10)) { g_br_ok = 0; return; }
    DWORD inner = *(DWORD*)(mw + 8);
    if (!inner || IsBadReadPtr((const void*)inner, 0x2C)) {
        g_br_ok = 0;
        return;
    }
    DWORD arr = *(DWORD*)(inner + 0x28);
    int sub = *(int*)(mw + 4);
    if (sub < 0 || sub > 64) sub = 0;
    g_br_sub = sub;
    if (!arr || IsBadReadPtr((const void*)arr, 0x50)) {
        g_br_ok = 0;
        return;
    }
    char* slot = (char*)(arr + (DWORD)sub * 0x50);
    if (IsBadReadPtr((const void*)slot, 0x50)) { g_br_ok = 0; return; }
    /* heal r1: +0x10 is native white. SURGICAL: only repair exact r1
       damage signature (0,1,0); never force white over native values
       (animated levels may legitimately hold other colors there). */
    float* c10 = (float*)(slot + 0x10);
    if (!IsBadReadPtr((const void*)c10, 16) && c10[3] == 1.0f &&
        c10[0] == 0.0f && c10[1] == 1.0f && c10[2] == 0.0f) {
        c10[0] = 1.0f;
        c10[1] = 1.0f;
        c10[2] = 1.0f;
    }
    float* c20 = (float*)(slot + 0x20);
    if (IsBadReadPtr((const void*)c20, 4)) { g_br_ok = 0; return; }
    float* c04 = (float*)(slot + 0x04);
    if (IsBadReadPtr((const void*)c04, 12)) { g_br_ok = 0; return; }
    float* c40 = (float*)(slot + 0x40);
    if (IsBadReadPtr((const void*)c40, 4)) { g_br_ok = 0; return; }
    float* c30 = (float*)(slot + 0x30);
    if (IsBadReadPtr((const void*)c30, 4)) { g_br_ok = 0; return; }
    float* c24 = (float*)(slot + 0x24);
    if (IsBadReadPtr((const void*)c24, 12)) { g_br_ok = 0; return; }
    float* c34 = (float*)(slot + 0x34);
    if (IsBadReadPtr((const void*)c34, 12)) { g_br_ok = 0; return; }
    /* Boot capture: the mesh is App-global, so level-1 values are the
       pristine truth. Sanity rejects our green (retry next tick).
       Later levels NEVER recapture (would absorb last race's residue);
       they scrub the mat back to boot values on entry instead. */
    if (!g_br_natlock) {
        g_br_natR = c30[0];
        g_br_nat20 = c20[0];
        g_br_natD0 = c04[0];
        g_br_natD1 = c04[1];
        g_br_natD2 = c04[2];
        g_br_natPow = c40[0];
        g_br_nat0 = c34[0];
        g_br_nat1 = c34[1];
        g_br_nat2 = c34[2];
        g_br_natS0 = c24[0];
        g_br_natS1 = c24[1];
        g_br_natS2 = c24[2];
        /* sanity: lock only on plausible native (not our green) */
        if (!(c34[0] == 0.0f && c34[1] == 1.0f && c34[2] == 0.0f)) {
            g_br_natlock = 1;
            /* one-time full material map: slot+0x00..+0x4C as raw bytes */
            static int dump_done = 0;
            if (!dump_done && !IsBadReadPtr((const void*)slot, 0x50)) {
                dump_done = 1;
                static char dl[220];
                int q = 0, b;
                const char* HX = "0123456789ABCDEF";
                const char* dt = "MATDUMP ";
                while (*dt) dl[q++] = *dt++;
                unsigned char* rb = (unsigned char*)slot;
                for (b = 0; b < 0x50 && q < 210; b++) {
                    dl[q++] = HX[(rb[b] >> 4) & 0xF];
                    dl[q++] = HX[rb[b] & 0xF];
                }
                dl[q] = '\0';
                log_line(dl);
            }
        }
    } else if (g_br_needscrub) {
        c30[0] = g_br_natR;
        c20[0] = g_br_nat20;
        c04[0] = g_br_natD0;
        c04[1] = g_br_natD1;
        c04[2] = g_br_natD2;
        c40[0] = g_br_natPow;
        c34[0] = g_br_nat0;
        c34[1] = g_br_nat1;
        c34[2] = g_br_nat2;
        c24[0] = g_br_natS0;
        c24[1] = g_br_natS1;
        c24[2] = g_br_natS2;
        g_br_needscrub = 0;
    }
    g_br_mat = (DWORD)c34;
    g_br_ok = 1;
    /* SPEC2: slot spec (+0x20) is 0.0437 yet the highlight is white, so the
       draw consumes specular from the +0x44 extension struct. Dump it once;
       write set-color spec there only when strictly validated (white triple
       + alpha 1.0 at +0x20..+0x2C). Writes run every tick to hold against
       native rewrites; logs throttled to ~10s. */
    {
        static int speclog = 0;
        DWORD ext = 0;
        if (!IsBadReadPtr((const void*)(slot + 0x44), 4))
            ext = *(DWORD*)(slot + 0x44);
        if (!ext || IsBadReadPtr((const void*)ext, 0x50)) {
            if (g_tick - speclog >= 600) {
                speclog = g_tick;
                log_line("SPEC2 ext=0");
            }
        } else {
            static int specdump = 0;
            float* e = (float*)ext;
            if (!specdump) {
                specdump = 1;
                if (!IsBadReadPtr((const void*)ext, 0x50)) {
                    static char sd[220];
                    int q = 0, b;
                    const char* HX = "0123456789ABCDEF";
                    const char* dt = "SPEC2DUMP ";
                    while (*dt) sd[q++] = *dt++;
                    unsigned char* rb = (unsigned char*)ext;
                    for (b = 0; b < 0x50 && q < 210; b++) {
                        sd[q++] = HX[(rb[b] >> 4) & 0xF];
                        sd[q++] = HX[rb[b] & 0xF];
                    }
                    sd[q] = '\0';
                    log_line(sd);
                }
            }
            int w = 0;
            char pre[7];
            const char* H2 = "0123456789ABCDEF";
            int k;
            for (k = 0; k < 3; k++) {
                int v = (int)(e[8 + k] * 255.0f + 0.5f);
                if (v < 0) v = 0;
                if (v > 255) v = 255;
                pre[k * 2] = H2[(v >> 4) & 0xF];
                pre[k * 2 + 1] = H2[v & 0xF];
            }
            pre[6] = '\0';
            if (!IsBadReadPtr((const void*)e, 0x30) &&
                e[8] > 0.95f && e[8] < 1.05f &&
                e[9] > 0.95f && e[9] < 1.05f &&
                e[10] > 0.95f && e[10] < 1.05f && e[11] == 1.0f) {
                e[8] = g_hook_rgb0;
                e[9] = g_hook_rgb1;
                e[10] = g_hook_rgb2;
                w = 1;
            }
            if (g_tick - speclog >= 600) {
                speclog = g_tick;
                static char sl[64];
                int o = 0, i;
                const char* st = "SPEC2 wrote=";
                while (*st) sl[o++] = *st++;
                sl[o++] = (char)('0' + w);
                const char* st2 = " pre=";
                while (*st2) sl[o++] = *st2++;
                for (i = 0; i < 6; i++) sl[o++] = pre[i];
                sl[o] = '\0';
                log_line(sl);
            }
        }
    }
}

#endif /* BRHOOK_H */
