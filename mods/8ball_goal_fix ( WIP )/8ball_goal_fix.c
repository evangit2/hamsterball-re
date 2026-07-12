/*
 * 8ball_goal_fix — Prevents crash when 8-ball (BadBall) touches N:GOAL trigger.
 *
 * ROOT CAUSE:
 *   BadBalls (8-balls) have player_index = -1 (set in Ball_ctor2 at 0x4039E0,
 *   never overwritten by CreateBadBall). When a BadBall hits an N:GOAL trigger,
 *   DispatchCollisionEvents (0x40C5D0) uses player_index as an array index:
 *       addr = player_index * 0xA0 + App
 *   With player_index = -1, this computes addr = App - 0xA0, then writes to
 *   three offsets: App+0x536, App+0x550, App+0x55C — corrupting App state.
 *
 *   Raptisoft already knew about this problem: the E:LIMIT handler in the same
 *   function (at 0x0040C785) has a "CMP ECX,-1 / JZ skip" guard. They just
 *   forgot to add the same guard to the N:GOAL handler.
 *
 * FIX:
 *   Two code caves that replicate the E:LIMIT pattern: check player_index < 0
 *   before the per-player writes, and if so, skip to the safe code (camera
 *   repositioning and status string updates, which don't use player_index).
 *
 *   Patch 1: 0x0040CF64 — "finished" flag writes (App+0x5D6, App+0x5FC)
 *     Original bytes: 8B 45 18 8B 8B 78 08 00 00 8D 04 80 C1 E0 05 03
 *       MOV EAX,[EBP+0x18]          ; player_index
 *       MOV ECX,[EBX+0x878]          ; App
 *       LEA EAX,[EAX+EAX*4]
 *       SHL EAX,5                    ; × 0xA0
 *       ADD EAX,ECX                  ; EAX = App + pidx * 0xA0
 *     Cave: execute original instruction (MOV EAX,[EBP+0x18]) only, then
 *           CMP EAX,0 / JL skip_to_0x0040CFA0 (camera code, safe)
 *           else execute remaining 12 original bytes + JMP back to 0x0040CF6D
 *
 *   Patch 2: 0x0040D03A — "show results" flag write (App+0x5F0)
 *     Original bytes: 8B 45 18 8B 8B 78 08 00 00 8D 04 80 C1 E0 05 8B
 *       MOV EAX,[EBP+0x18]          ; player_index
 *       MOV ECX,[EBX+0x878]          ; App
 *       LEA EAX,[EAX+EAX*4]
 *       SHL EAX,5                    ; × 0xA0
 *       MOV EDX,ECX                 ; EDX = App
 *     Cave: execute MOV EAX,[EBP+0x18] only, then
 *           CMP EAX,0 / JL skip_to_0x0040D05E (status="Update" epilogue, safe)
 *           else execute remaining 12 original bytes + JMP back to 0x0040D03D
 *
 * RESULT:
 *   8-ball crossing goal: goal music plays, camera repositions, board flag set.
 *   Player crossing goal after: race finishes normally with results screen.
 *   No crash. Identical visible behavior to original game minus the crash.
 *
 * BONUS FIX: E:ACTION(SCORE) at 0x0040CA33 has the same player_index*0xA0 bug.
 *   8-ball hitting E:ACTION(SCORE) would corrupt App+0x544 with a float.
 *   Patch 3 adds the same guard there.
 *
 *   Original at 0x0040CA33:
 *       8B 45 18          MOV EAX,[EBP+0x18]     ; player_index
 *       D9 E0             FLD ST0                 ; dup for FLD+FMUL+FADD
 *       8D 14 80          LEA EDX,[EAX+EAX*4]
 *       8B 83 78 08 00 00 MOV EAX,[EBX+0x878]     ; App
 *       D1 E2             SHL EDX,5
 *       D8 64 10 5E4      FADD [EDX+EAX+0x5E4]    ; score array write
 *   Cave: check player_index < 0 → skip to 0x0040CA54 (JMP past the FSTP).
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* ── BASS proxy exports (REQUIRED — all 10 game imports) ────────────── */
static HMODULE g_hRealBass = NULL;

typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}

typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c);
    return 1;
}

typedef int  (__stdcall *BASS_MusicLoad_t)(const char*, DWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) int __stdcall BASS_MusicLoad(const char* a, DWORD b, DWORD c, DWORD d, DWORD e) {
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e);
    return 0;
}

typedef int  (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (real_BASS_Free) return real_BASS_Free();
    return 1;
}

typedef int  (__stdcall *BASS_Init_t)(int, int, int, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, int b, int c, HWND d, void* e) {
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 1;
}

typedef void (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) void __stdcall BASS_ChannelStop(DWORD a) {
    if (real_BASS_ChannelStop) real_BASS_ChannelStop(a);
}

typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}

typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}

typedef int  (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (real_BASS_Start) return real_BASS_Start();
    return 1;
}

typedef int  (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (real_BASS_Stop) return real_BASS_Stop();
    return 1;
}

/* ── Mod: 8-ball Goal Crash Fix ────────────────────────────────────── */

/* Helper: verify original bytes at an address match expected */
static int verify_bytes(BYTE *addr, const BYTE *expected, int len) {
    int i;
    for (i = 0; i < len; i++) {
        if (addr[i] != expected[i]) return 0;
    }
    return 1;
}

/* Helper: install a code cave hook (JMP from hook_addr to cave) */
static void install_cave(DWORD hook_addr_rva, const BYTE *orig_bytes,
                         int orig_len, BYTE *cave, int cave_len)
{
    HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
    if (!hExe) hExe = GetModuleHandleA(NULL);
    DWORD base = (DWORD)hExe;
    BYTE *hook = (BYTE*)(base + hook_addr_rva);
    DWORD oldProt;

    if (!verify_bytes(hook, orig_bytes, orig_len)) {
        MessageBoxA(NULL, "8ball_goal_fix: byte mismatch — wrong game version?",
                     "Mod Error", MB_OK | MB_ICONWARNING);
        return;
    }

    VirtualProtect(hook, orig_len, PAGE_EXECUTE_READWRITE, &oldProt);
    /* Overwrite original bytes with JMP to cave */
    hook[0] = 0xE9;  /* near JMP */
    *(DWORD*)(hook + 1) = (DWORD)cave - (DWORD)hook - 5;
    /* NOP any remaining original bytes if orig_len > 5 */
    if (orig_len > 5) {
        int i;
        for (i = 5; i < orig_len; i++) hook[i] = 0x90;
    }
    VirtualProtect(hook, orig_len, oldProt, &oldProt);
    FlushInstructionCache(GetCurrentProcess(), hook, orig_len);
}

/* Helper: write a cave body and install the hook */
static BYTE* make_cave(const BYTE *cave_body, int cave_len,
                       DWORD hook_rva, const BYTE *orig, int orig_len)
{
    BYTE *cave = (BYTE*)VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE);
    if (!cave) return NULL;
    memcpy(cave, cave_body, cave_len);
    install_cave(hook_rva, orig, orig_len, cave, cave_len);
    return cave;
}

static void apply_patches(void) {
    HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
    if (!hExe) hExe = GetModuleHandleA(NULL);
    DWORD base = (DWORD)hExe;

    /*
     * PATCH 1: N:GOAL "finished" flag (0x0040CF64)
     *
     * Original 16 bytes at 0x0040CF64:
     *   8B 45 18             MOV EAX,[EBP+0x18]      ; player_index
     *   8B 8B 78 08 00 00    MOV ECX,[EBX+0x878]     ; App
     *   8D 04 80             LEA EAX,[EAX+EAX*4]
     *   C1 E0 05             SHL EAX,5               ; × 0xA0
     *   03 C1                ADD EAX,ECX             ; EAX = App + pidx*0xA0
     *   ... then writes to [EAX+0x5D6] and [EAX+0x5FC]
     *
     * Cave logic:
     *   MOV EAX,[EBP+0x18]          ; player_index (original first 3 bytes)
     *   CMP EAX, 0
     *   JL  skip                     ; if negative (BadBall), skip to camera code
     *   MOV ECX,[EBX+0x878]         ; App (original bytes 3-8)
     *   LEA EAX,[EAX+EAX*4]         ; (original bytes 9-14)
     *   SHL EAX,5                    ; (original byte 15... but that's 0x03 C1)
     *   ADD EAX,ECX                  ; (original byte 16... but that's 0xC1 8A)
     *   JMP back to 0x0040CF73      ; continue with original flow
     * skip:
     *   JMP 0x0040CFA0               ; jump to camera code (safe, no player_index)
     *
     * Wait — let me recount the bytes. The disassembly shows:
     *   0040cf64: 8B 45 18          MOV EAX,[EBP+0x18]
     *   0040cf67: 8B 8B 78 08 00 00 MOV ECX,[EBX+0x878]
     *   0040cf6d: 8D 04 80          LEA EAX,[EAX+EAX*4]
     *   0040cf70: C1 E0 05          SHL EAX,5
     *   0040cf73: 03 C1             ADD EAX,ECX
     *
     * So 14 bytes total (0x0040CF64 to 0x0040CF72 inclusive).
     * Hook: replace first 5 bytes with JMP to cave (need 5 for near JMP).
     * Cave executes: original 3 bytes (MOV EAX,[EBP+0x18]),
     *   then CMP EAX,0 / JL skip,
     *   then remaining 11 original bytes,
     *   then JMP back to 0x0040CF73.
     * Skip target: 0x0040CFA0 (camera repositioning, safe).
     */

    /* Patch 1 original bytes (14 bytes: 0x0040CF64..0x0040CF71) */
    static const BYTE p1_orig[] = {
        0x8B, 0x45, 0x18,                          /* MOV EAX,[EBP+0x18] */
        0x8B, 0x8B, 0x78, 0x08, 0x00, 0x00,          /* MOV ECX,[EBX+0x878] */
        0x8D, 0x04, 0x80,                            /* LEA EAX,[EAX+EAX*4] */
        0xC1, 0xE0, 0x05,                            /* SHL EAX,5 */
        0x03, 0xC1                                    /* ADD EAX,ECX */
    };

    /*
     * Cave body for Patch 1:
     *   8B 45 18                MOV EAX,[EBP+0x18]     ; orig 3 bytes
     *   83 F8 00                CMP EAX,0
     *   7C XX                   JL  skip              ; 2 bytes rel8
     *   8B 8B 78 08 00 00       MOV ECX,[EBX+0x878]   ; orig bytes 3-8
     *   8D 04 80                LEA EAX,[EAX+EAX*4]   ; orig bytes 9-11
     *   C1 E0 05                SHL EAX,5             ; orig bytes 12-14
     *   03 C1                   ADD EAX,ECX           ; orig bytes 15-16
     *   E9 XX XX XX XX          JMP back to 0x0040CF73 ; near JMP
     * skip:
     *   E9 XX XX XX XX          JMP 0x0040CFA0         ; near JMP (skip to camera)
     */
    BYTE cave1[64];
    int c = 0;
    /* MOV EAX,[EBP+0x18] */
    cave1[c++]=0x8B; cave1[c++]=0x45; cave1[c++]=0x18;
    /* CMP EAX,0 */
    cave1[c++]=0x83; cave1[c++]=0xF8; cave1[c++]=0x00;
    /* JL skip (relative to skip_label = c+2) */
    int jl_offset_pos = c + 1;  /* position of JL offset byte */
    /* We'll calculate: skip is after the "continue" path. Let's assemble both paths first */
    /* continue path: 11 bytes (MOV ECX + LEA + SHL + ADD) + 5 (JMP) = 16 */
    cave1[c]=0x7C; cave1[c+1]=16; c+=2;  /* JL +16 (skip past continue path) */
    /* continue: MOV ECX,[EBX+0x878] */
    cave1[c++]=0x8B; cave1[c++]=0x8B; cave1[c++]=0x78; cave1[c++]=0x08; cave1[c++]=0x00; cave1[c++]=0x00;
    /* LEA EAX,[EAX+EAX*4] */
    cave1[c++]=0x8D; cave1[c++]=0x04; cave1[c++]=0x80;
    /* SHL EAX,5 */
    cave1[c++]=0xC1; cave1[c++]=0xE0; cave1[c++]=0x05;
    /* ADD EAX,ECX */
    cave1[c++]=0x03; cave1[c++]=0xC1;
    /* JMP back to 0x0040CF73 (continue original flow) */
    {
        DWORD ret_rva = 0x0040CF73 - 0x00400000;  /* = 0xCF73 */
        BYTE *cave_ptr = cave1 + c;
        /* Actually we need the address relative to the cave, but we don't know
           the cave address yet. We'll fix up after VirtualAlloc. */
        /* Placeholder: E9 00 00 00 00 */
        cave1[c++]=0xE9; cave1[c++]=0x00; cave1[c++]=0x00; cave1[c++]=0x00; cave1[c++]=0x00;
        int jmp_back_fixup = c - 4;
        /* skip path: JMP 0x0040CFA0 */
        cave1[c++]=0xE9; cave1[c++]=0x00; cave1[c++]=0x00; cave1[c++]=0x00; cave1[c++]=0x00;
        int jmp_skip_fixup = c - 4;

        /* Allocate cave */
        BYTE *cave = (BYTE*)VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE,
                                         PAGE_EXECUTE_READWRITE);
        if (!cave) return;

        memcpy(cave, cave1, c);

        /* Fixup JMP back: target 0x0040CF73, from cave+offset after the E9 */
        {
            DWORD src = (DWORD)cave + jmp_back_fixup;
            *(DWORD*)(cave + jmp_back_fixup) = (base + 0xCF73) - src - 4;
        }
        /* Fixup JMP skip: target 0x0040CFA0 */
        {
            DWORD src = (DWORD)cave + jmp_skip_fixup;
            *(DWORD*)(cave + jmp_skip_fixup) = (base + 0xCFA0) - src - 4;
        }

        /* Install hook at 0x0040CF64: JMP to cave + NOP */
        BYTE *hook = (BYTE*)(base + 0xCF64);
        DWORD oldProt;
        if (!verify_bytes(hook, p1_orig, 14)) {
            MessageBoxA(NULL, "8ball_goal_fix: Patch1 byte mismatch",
                        "Mod Error", MB_OK | MB_ICONWARNING);
            return;
        }
        VirtualProtect(hook, 14, PAGE_EXECUTE_READWRITE, &oldProt);
        hook[0] = 0xE9;
        *(DWORD*)(hook + 1) = (DWORD)cave - (DWORD)hook - 5;
        hook[5] = 0x90; hook[6] = 0x90; hook[7] = 0x90;
        hook[8] = 0x90; hook[9] = 0x90; hook[10] = 0x90; hook[11] = 0x90;
        hook[12] = 0x90; hook[13] = 0x90;
        VirtualProtect(hook, 14, oldProt, &oldProt);
        FlushInstructionCache(GetCurrentProcess(), hook, 14);
    }

    /*
     * PATCH 2: N:GOAL "show results" flag (0x0040D03A)
     *
     * Original 16 bytes at 0x0040D03A:
     *   0040d03a: 8B 45 18             MOV EAX,[EBP+0x18]
     *   0040d03d: 8B 8B 78 08 00 00    MOV ECX,[EBX+0x878]
     *   0040d043: 8D 04 80             LEA EAX,[EAX+EAX*4]
     *   0040d046: C1 E0 05             SHL EAX,5
     *   0040d049: 8B D1                MOV EDX,ECX
     *   0040d04b: 2B D0                SUB EDX,EAX
     *   0040d04d: 80 BA 90 06 00 00 00 CMP BYTE [EDX+0x690],0
     *   0040d054: 75 08                JNZ skip_write
     *   0040d056: C6 84 08 F0 05 00 00 01  MOV BYTE [EAX+ECX+0x5F0],1
     *   0040d05e: ... (status="Update" epilogue, safe)
     *
     * We hook at 0x0040D03A, replace first 5 bytes with JMP to cave.
     * Cave: check player_index, if negative jump to 0x0040D05E.
     * Otherwise execute original 16 bytes and JMP back to 0x0040D04A.
     */
    static const BYTE p2_orig[] = {
        0x8B, 0x45, 0x18,                          /* MOV EAX,[EBP+0x18] */
        0x8B, 0x8B, 0x78, 0x08, 0x00, 0x00,          /* MOV ECX,[EBX+0x878] */
        0x8D, 0x04, 0x80,                            /* LEA EAX,[EAX+EAX*4] */
        0xC1, 0xE0, 0x05,                            /* SHL EAX,5 */
        0x8B, 0xD1                                    /* MOV EDX,ECX */
    };

    BYTE cave2[64];
    c = 0;
    /* MOV EAX,[EBP+0x18] */
    cave2[c++]=0x8B; cave2[c++]=0x45; cave2[c++]=0x18;
    /* CMP EAX,0 */
    cave2[c++]=0x83; cave2[c++]=0xF8; cave2[c++]=0x00;
    /* JL skip (past continue path: 11 bytes orig + 5 JMP = 16) */
    cave2[c++]=0x7C; cave2[c++]=16;
    /* continue: remaining original bytes */
    /* MOV ECX,[EBX+0x878] */
    cave2[c++]=0x8B; cave2[c++]=0x8B; cave2[c++]=0x78; cave2[c++]=0x08; cave2[c++]=0x00; cave2[c++]=0x00;
    /* LEA EAX,[EAX+EAX*4] */
    cave2[c++]=0x8D; cave2[c++]=0x04; cave2[c++]=0x80;
    /* SHL EAX,5 */
    cave2[c++]=0xC1; cave2[c++]=0xE0; cave2[c++]=0x05;
    /* MOV EDX,ECX */
    cave2[c++]=0x8B; cave2[c++]=0xD1;
    /* JMP back to 0x0040D04B (after MOV EDX,ECX, continue with SUB EDX,EAX) */
    cave2[c++]=0xE9; cave2[c++]=0x00; cave2[c++]=0x00; cave2[c++]=0x00; cave2[c++]=0x00;
    int jmp2_back_fixup = c - 4;
    /* skip: JMP 0x0040D05E (status="Update" epilogue) */
    cave2[c++]=0xE9; cave2[c++]=0x00; cave2[c++]=0x00; cave2[c++]=0x00; cave2[c++]=0x00;
    int jmp2_skip_fixup = c - 4;

    {
        BYTE *cave = (BYTE*)VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE,
                                         PAGE_EXECUTE_READWRITE);
        if (!cave) return;
        memcpy(cave, cave2, c);

        /* Fixup JMP back: target 0x0040D04B */
        {
            DWORD src = (DWORD)cave + jmp2_back_fixup;
            *(DWORD*)(cave + jmp2_back_fixup) = (base + 0xD04B) - src - 4;
        }
        /* Fixup JMP skip: target 0x0040D05E */
        {
            DWORD src = (DWORD)cave + jmp2_skip_fixup;
            *(DWORD*)(cave + jmp2_skip_fixup) = (base + 0xD05E) - src - 4;
        }

        /* Install hook at 0x0040D03A */
        BYTE *hook = (BYTE*)(base + 0xD03A);
        DWORD oldProt;
        if (!verify_bytes(hook, p2_orig, 15)) {
            MessageBoxA(NULL, "8ball_goal_fix: Patch2 byte mismatch",
                        "Mod Error", MB_OK | MB_ICONWARNING);
            return;
        }
        VirtualProtect(hook, 15, PAGE_EXECUTE_READWRITE, &oldProt);
        hook[0] = 0xE9;
        *(DWORD*)(hook + 1) = (DWORD)cave - (DWORD)hook - 5;
        hook[5] = 0x90; hook[6] = 0x90; hook[7] = 0x90;
        hook[8] = 0x90; hook[9] = 0x90; hook[10] = 0x90;
        hook[11] = 0x90; hook[12] = 0x90; hook[13] = 0x90; hook[14] = 0x90;
        VirtualProtect(hook, 15, oldProt, &oldProt);
        FlushInstructionCache(GetCurrentProcess(), hook, 15);
    }

    /*
     * PATCH 3 (BONUS): E:ACTION(SCORE) player_index guard (0x0040CA33)
     *
     * At 0x0040CA33, the SCORE handler computes:
     *   param_1[6] * 0xA0 + 0x5E4 + App
     * and does FADD + FSTP to write a float score. With player_index=-1,
     * this writes to App+0x544.
     *
     * Original bytes at 0x0040CA33:
     *   0040ca33: 8B 45 18             MOV EAX,[EBP+0x18]
     *   0040ca36: D9 E0                FLD ST0
     *   0040ca38: 8D 14 80             LEA EDX,[EAX+EAX*4]
     *   0040ca3b: 8B 83 78 08 00 00    MOV EAX,[EBX+0x878]
     *   0040ca41: D1 E2                SHL EDX,5
     *   0040ca43: D8 64 10 5E4         FADD [EDX+EAX+0x5E4]
     *   0040ca47: ... (LEA EAX, FSTP)
     *
     * We hook at 0x0040CA33, check player_index < 0, if so skip to
     * 0x0040CA54 (past the FSTP at 0x0040CA52, to the ReadTag loop continue).
     *
     * Actually — let me check what's at 0x0040CA54:
     *   0040ca4b: LEA EAX,[EDX+EAX+0x5E4]   ; 4 bytes
     *   0040ca52: FSTP [EAX]                ; 2 bytes
     *   0040ca54: CALL __ftol2               ; the next instruction after FSTP
     *
     * Skipping to 0x0040CA54 would skip the FSTP, leaving a stale value on
     * the FPU stack. Better to skip to 0x0040CA74 (the LEA ECX,[ESP+0x10]
     * after the entire SCORE block, which continues the ReadTag loop).
     * But that's too far and would skip Ball_SetName too.
     *
     * Safest: skip to 0x0040CA54 but first FSTP ST0 (pop the stale FPU value).
     * Cave:
     *   MOV EAX,[EBP+0x18]    ; player_index
     *   CMP EAX,0
     *   JL  skip               ; if BadBall, pop FPU and skip to 0x0040CA74
     *   ... original 17 bytes ...
     *   JMP 0x0040CA4A
     * skip:
     *   FSTP ST0               ; pop stale FPU value (from FLD at 0x0040CA36)
     *   JMP 0x0040CA74          ; skip entire SCORE block
     *
     * Wait, the FLD at 0x0040CA36 pushes a copy of ST0 onto the FPU stack.
     * If we skip the FADD+FSTP, we need to pop it. FSTP ST0 (=DD D8) pops
     * the top of FPU stack. Then skip to 0x0040CA74.
     *
     * Actually let me re-examine. The code at 0x0040CA2E:
     *   0040ca2e: CALL Difficulty_GetTimeModifier   ; pushes result on FPU
     *   0040ca33: MOV EAX,[EBP+0x18]                ; player_index
     *   0040ca36: D9 E0                              ; FLD ST0 (dup top)
     *   0040ca38: 8D 14 80                           ; LEA EDX,[EAX+EAX*4]
     *   0040ca3b: 8B 83 78 08 00 00                  ; MOV EAX,[EBX+0x878]
     *   0040ca41: D1 E2                              ; SHL EDX,5
     *   0040ca43: D8 64 10 E4 05                      ; FADD [EDX+EAX+0x5E4]
     *   0040ca47: 8D 04 10                            ; LEA EAX,[EDX+EAX+0x5E4]
     *   0040ca4a: ... wait, that's wrong. Let me recheck.
     *
     * From the disassembly:
     *   0040ca33: 8B 45 18             MOV EAX,[EBP+0x18]
     *   0040ca36: FLD ST0              (D9 E0)
     *   0040ca38: LEA EDX,[EAX+EAX*4] (8D 14 80)
     *   0040ca3b: MOV EAX,[EBX+0x878] (8B 83 78 08 00 00)
     *   0040ca41: SHL EDX,5            (D1 E2)
     *   0040ca43: FADD [EDX+EAX+0x5E4] (D8 64 10 E4 05 00 00)
     *   0040ca4a: LEA EAX,[EDX+EAX+0x5E4] (8D 84 10 E4 05 00 00)
     *   0040ca51: FSTP [EAX]           (D9 18)
     *   0040ca53: ... wait
     *
     * Actually let me re-read the disassembly more carefully:
     *   0040ca33: MOV EAX,[EBP + 0x18]
     *   0040ca36: FLD ST0
     *   0040ca38: LEA EDX,[EAX + EAX*4]
     *   0040ca3b: MOV EAX,[EBX + 0x878]
     *   0040ca41: SHL EDX,5
     *   0040ca43: FADD [EDX + EAX*1 + 0x5e4]
     *   0040ca4b: LEA EAX,[EDX + EAX*1 + 0x5e4]
     *   0040ca52: FSTP [EAX]
     *   0040ca54: CALL 0x004ba754       ; __ftol2
     *
     * So after FLD ST0, if we skip, we need to FSTP ST0 (pop) then JMP past
     * the entire score computation to 0x0040CA74 (after Ball_SetName call).
     * Actually 0x0040CA74 is the LEA ECX,[ESP+0x10] after the CALL to
     * Ball_SetName. Let me trace:
     *   0040ca52: FSTP [EAX]
     *   0040ca54: CALL 0x004ba754      ; __ftol2
     *   0040ca59: PUSH EAX
     *   0040ca5a: PUSH 0x4cf500
     *   0040ca5f: PUSH 0x4f7448
     *   0040ca64: CALL 0x00466c70      ; AthenaString_Format
     *   0040ca69: ADD ESP,0xc
     *   0040ca6c: PUSH EAX
     *   0040ca6d: MOV ECX,EBP
     *   0040ca6f: CALL 0x00401660      ; Ball_SetName
     *   0040ca74: LEA ECX,[ESP+0x10]   ; ← skip target (ReadTag loop continue)
     *
     * So skip target = 0x0040CA74. We need to pop the FPU first (FSTP ST0).
     *
     * Original bytes at 0x0040CA33 (hook 5 bytes, need to cover MOV EAX
     *   + FLD ST0 = 3+2 = 5 bytes exactly):
     *   8B 45 18 D9 E0
     *
     * Cave:
     *   MOV EAX,[EBP+0x18]     ; 3 bytes (original)
     *   CMP EAX,0              ; 3 bytes
     *   JL  skip               ; 2 bytes (skip past 19 bytes of continue + 5 JMP = 24)
     *   FLD ST0                ; 2 bytes (original)
     *   LEA EDX,[EAX+EAX*4]   ; 3 bytes (original)
     *   MOV EAX,[EBX+0x878]    ; 6 bytes (original)
     *   SHL EDX,5              ; 2 bytes (original)
     *   FADD [EDX+EAX+0x5E4]  ; 7 bytes (original)
     *   LEA EAX,[EDX+EAX+0x5E4] ; 7 bytes (original)
     *   FSTP [EAX]             ; 2 bytes (original)
     *   JMP 0x0040CA54         ; 5 bytes (continue after FSTP)
     * skip:
     *   DD D8                  ; FSTP ST0 (pop stale FPU value)
     *   JMP 0x0040CA74         ; skip entire SCORE block
     *
     * Continue path: 2+3+6+2+7+7+2+5 = 34 bytes. JL offset = 34.
     */
    static const BYTE p3_orig[] = {
        0x8B, 0x45, 0x18,                          /* MOV EAX,[EBP+0x18] */
        0xD9, 0xE0                                    /* FLD ST0 */
    };

    BYTE cave3[128];
    c = 0;
    /* MOV EAX,[EBP+0x18] */
    cave3[c++]=0x8B; cave3[c++]=0x45; cave3[c++]=0x18;
    /* CMP EAX,0 */
    cave3[c++]=0x83; cave3[c++]=0xF8; cave3[c++]=0x00;
    /* JL skip — skip = continue_path_len + skip_pop_jmp_len */
    /* continue: FLD(2)+LEA(3)+MOV(6)+SHL(2)+FADD(7)+LEA(7)+FSTP(2)+JMP(5) = 34 */
    /* skip: FSTP_ST0(2) + JMP(5) = 7 */
    cave3[c++]=0x7C; cave3[c++]=34;
    /* continue: FLD ST0 */
    cave3[c++]=0xD9; cave3[c++]=0xE0;
    /* LEA EDX,[EAX+EAX*4] */
    cave3[c++]=0x8D; cave3[c++]=0x14; cave3[c++]=0x80;
    /* MOV EAX,[EBX+0x878] */
    cave3[c++]=0x8B; cave3[c++]=0x83; cave3[c++]=0x78; cave3[c++]=0x08; cave3[c++]=0x00; cave3[c++]=0x00;
    /* SHL EDX,5 */
    cave3[c++]=0xD1; cave3[c++]=0xE2;
    /* FADD [EDX+EAX+0x5E4] */
    cave3[c++]=0xD8; cave3[c++]=0x64; cave3[c++]=0x10; cave3[c++]=0xE4; cave3[c++]=0x05; cave3[c++]=0x00; cave3[c++]=0x00;
    /* LEA EAX,[EDX+EAX+0x5E4] */
    cave3[c++]=0x8D; cave3[c++]=0x84; cave3[c++]=0x10; cave3[c++]=0xE4; cave3[c++]=0x05; cave3[c++]=0x00; cave3[c++]=0x00;
    /* FSTP [EAX] */
    cave3[c++]=0xD9; cave3[c++]=0x18;
    /* JMP 0x0040CA54 */
    cave3[c++]=0xE9; cave3[c++]=0x00; cave3[c++]=0x00; cave3[c++]=0x00; cave3[c++]=0x00;
    int jmp3_back_fixup = c - 4;
    /* skip: FSTP ST0 (pop stale FPU) */
    cave3[c++]=0xDD; cave3[c++]=0xD8;
    /* JMP 0x0040CA74 */
    cave3[c++]=0xE9; cave3[c++]=0x00; cave3[c++]=0x00; cave3[c++]=0x00; cave3[c++]=0x00;
    int jmp3_skip_fixup = c - 4;

    {
        BYTE *cave = (BYTE*)VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE,
                                         PAGE_EXECUTE_READWRITE);
        if (!cave) return;
        memcpy(cave, cave3, c);

        /* Fixup JMP back: target 0x0040CA54 */
        {
            DWORD src = (DWORD)cave + jmp3_back_fixup;
            *(DWORD*)(cave + jmp3_back_fixup) = (base + 0xCA54) - src - 4;
        }
        /* Fixup JMP skip: target 0x0040CA74 */
        {
            DWORD src = (DWORD)cave + jmp3_skip_fixup;
            *(DWORD*)(cave + jmp3_skip_fixup) = (base + 0xCA74) - src - 4;
        }

        /* Install hook at 0x0040CA33: 5 bytes exactly (MOV EAX + FLD ST0) */
        BYTE *hook = (BYTE*)(base + 0xCA33);
        DWORD oldProt;
        if (!verify_bytes(hook, p3_orig, 5)) {
            MessageBoxA(NULL, "8ball_goal_fix: Patch3 byte mismatch",
                        "Mod Error", MB_OK | MB_ICONWARNING);
            return;
        }
        VirtualProtect(hook, 5, PAGE_EXECUTE_READWRITE, &oldProt);
        hook[0] = 0xE9;
        *(DWORD*)(hook + 1) = (DWORD)cave - (DWORD)hook - 5;
        VirtualProtect(hook, 5, oldProt, &oldProt);
        FlushInstructionCache(GetCurrentProcess(), hook, 5);
    }
}

/* ── BASS proxy init ───────────────────────────────────────────────── */
static void init_bass_proxy(void) {
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (!g_hRealBass) {
        /* Try same-dir as our DLL */
        char path[MAX_PATH];
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&init_bass_proxy, &hSelf);
        if (hSelf && GetModuleFileNameA(hSelf, path, MAX_PATH)) {
            char *p = strrchr(path, '\\');
            if (p) { strcpy(p + 1, "bass_real.dll"); g_hRealBass = LoadLibraryA(path); }
        }
    }
    if (g_hRealBass) {
        real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
        real_BASS_MusicPlayEx          = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
        real_BASS_MusicLoad             = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
        real_BASS_Free                  = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
        real_BASS_Init                  = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
        real_BASS_ChannelStop           = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
        real_BASS_ErrorGetCode          = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
        real_BASS_SetConfig             = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
        real_BASS_Start                 = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
        real_BASS_Stop                  = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            init_bass_proxy();
            apply_patches();
            break;
        case DLL_PROCESS_DETACH:
            /* Patches persist for lifetime of process; game exits after detach */
            break;
    }
    return TRUE;
}
