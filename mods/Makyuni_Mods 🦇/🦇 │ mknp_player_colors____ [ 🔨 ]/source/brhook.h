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
extern float g_hook_rgb0;
extern float g_hook_rgb1;
extern float g_hook_rgb2;
extern DWORD g_br_tramp;
extern int g_br_inst;
extern int g_br_natlock;

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
".globl _g_hook_rgb0\n"
"_g_hook_rgb0: .space 4\n"
".globl _g_hook_rgb1\n"
"_g_hook_rgb1: .space 4\n"
".globl _g_hook_rgb2\n"
"_g_hook_rgb2: .space 4\n"
".globl _g_br_tramp\n"
"_g_br_tramp: .space 4\n"
".globl _g_br_inst\n"
"_g_br_inst: .space 4\n"
".globl _g_br_natlock\n"
"_g_br_natlock: .space 4\n"
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
/* other balls: full native restore (emissive + specular + R). Partial
 * restores tinted P3 red (R left at foreign value). */
" movl _g_br_natR, %eax\n"
" movl %eax, -4(%edx)\n"
" movl _g_br_nat0, %eax\n"
" movl %eax, (%edx)\n"
" movl _g_br_nat1, %eax\n"
" movl %eax, 4(%edx)\n"
" movl _g_br_nat2, %eax\n"
" movl %eax, 8(%edx)\n"
" movl _g_br_natS0, %eax\n"
" movl %eax, -16(%edx)\n"
" movl _g_br_natS1, %eax\n"
" movl %eax, -12(%edx)\n"
" movl _g_br_natS2, %eax\n"
" movl %eax, -8(%edx)\n"
" jmp 9f\n"
"1:\n"
/* P1: set color to emissive AND specular (mode = 1P/2P/4P set key). */
" movl _g_hook_rgb0, %eax\n"
" movl %eax, (%edx)\n"
" movl %eax, -16(%edx)\n"
" movl _g_hook_rgb1, %eax\n"
" movl %eax, 4(%edx)\n"
" movl %eax, -12(%edx)\n"
" movl _g_hook_rgb2, %eax\n"
" movl %eax, 8(%edx)\n"
" movl %eax, -8(%edx)\n"
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
    float* c30 = (float*)(slot + 0x30);
    if (IsBadReadPtr((const void*)c30, 4)) { g_br_ok = 0; return; }
    float* c24 = (float*)(slot + 0x24);
    if (IsBadReadPtr((const void*)c24, 12)) { g_br_ok = 0; return; }
    float* c34 = (float*)(slot + 0x34);
    if (IsBadReadPtr((const void*)c34, 12)) { g_br_ok = 0; return; }
    if (!g_br_natlock) {
        g_br_natR = c30[0];
        g_br_nat0 = c34[0];
        g_br_nat1 = c34[1];
        g_br_nat2 = c34[2];
        g_br_natS0 = c24[0];
        g_br_natS1 = c24[1];
        g_br_natS2 = c24[2];
        /* sanity: lock only on plausible native (not our green) */
        if (!(c34[0] == 0.0f && c34[1] == 1.0f && c34[2] == 0.0f))
            g_br_natlock = 1;
    }
    g_br_mat = (DWORD)c34;
    g_br_ok = 1;
}

#endif /* BRHOOK_H */
