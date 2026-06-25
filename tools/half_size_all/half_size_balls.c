/*
 * half_size_balls.c — BASS.dll proxy that shrinks all balls to half size.
 *
 * Uses the game's own Ball_Shrink function (0x00402200) — the same function
 * the Odd Race uses to shrink the ball in its pipe maze. This is cleaner than
 * patching float immediates because it exactly replicates the game's shrink
 * behavior: radius 26→13, physics_scale 5→2.5, is_falling flag set.
 *
 * Two hooks:
 *   1. Ball_ctor2 exit (0x00403DB1): catches ALL balls at creation
 *      (player, split, follow, board-init). Calls Ball_Shrink(ball) via
 *      code cave that saves/restores EAX (return value).
 *
 *   2. CreateBadBall FSTP (0x0040BE74): catches AI balls after their
 *      custom SIZE value is computed. Replaces the FSTP [ESI+0x284]
 *      with Ball_Shrink(ESI), which overrides the SIZE with a fixed 13.0.
 *
 * Ball_Shrink (0x00402200) — __fastcall(ball):
 *   ball+0xC4C = 1          (is_falling flag)
 *   ball+0x284 = 13.0       (radius, was 26.0)
 *   ball+0x188 = 2.5        (physics_scale, was 5.0)
 *   Sound_Play3D(fall_sound, ball->pos)
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * BUILD
 * ═══════════════════════════════════════════════════════════════════════════
 *
 *   i686-w64-mingw32-gcc -shared -o bass.dll half_size_balls.c \
 *       -lwinmm -Wl,--enable-stdcall-fixup \
 *       -O2 -static -static-libgcc -Wl,--add-stdcall-alias
 *
 * Installation (Windows):
 *   1. In your Hamsterball game folder, rename bass.dll → bass_real.dll
 *   2. Copy this proxy bass.dll into the game folder
 *   3. Run the game — all balls will be half their normal size
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports (stubs — Hamsterball only needs import resolution)
 * ═══════════════════════════════════════════════════════════════════════════ */

__declspec(dllexport) void BASS_Init(void) {}
__declspec(dllexport) void BASS_Free(void) {}
__declspec(dllexport) void BASS_Start(void) {}
__declspec(dllexport) void BASS_Stop(void) {}
__declspec(dllexport) void BASS_Pause(void) {}
__declspec(dllexport) void BASS_SetVolume(void) {}
__declspec(dllexport) void BASS_GetVolume(void) {}
__declspec(dllexport) void BASS_GetDevice(void) {}
__declspec(dllexport) void BASS_SetDevice(void) {}
__declspec(dllexport) void BASS_GetInfo(void) {}
__declspec(dllexport) void BASS_Update(void) {}
__declspec(dllexport) void BASS_ErrorGetCode(void) {}
__declspec(dllexport) void BASS_StreamCreateFile(void) {}
__declspec(dllexport) void BASS_MusicLoad(void) {}
__declspec(dllexport) void BASS_SampleLoad(void) {}
__declspec(dllexport) void BASS_ChannelPlay(void) {}
__declspec(dllexport) void BASS_ChannelStop(void) {}
__declspec(dllexport) void BASS_ChannelSetAttribute(void) {}
__declspec(dllexport) void BASS_ChannelGetAttribute(void) {}
__declspec(dllexport) void BASS_ChannelGetData(void) {}
__declspec(dllexport) void BASS_ChannelGetLevel(void) {}
__declspec(dllexport) void BASS_ChannelSetPosition(void) {}
__declspec(dllexport) void BASS_ChannelGetPosition(void) {}
__declspec(dllexport) void BASS_ChannelIsActive(void) {}
__declspec(dllexport) void BASS_ChannelRemoveSync(void) {}
__declspec(dllexport) void BASS_ChannelSetSync(void) {}
__declspec(dllexport) void BASS_SampleCreate(void) {}
__declspec(dllexport) void BASS_SampleGetChannel(void) {}

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch Constants
 * ═══════════════════════════════════════════════════════════════════════════ */

#define IMAGE_BASE  0x00400000

/* Ball_Shrink (0x00402200) — __fastcall(ball in ECX)
 * Sets radius=13.0, physics_scale=2.5, is_falling=1, plays fall sound */
#define BALL_SHRINK_ADDR  0x00402200

/*
 * Hook 1: Ball_ctor2 exit
 *
 *   Address:  0x00403DB1
 *   Original: 83 C4 20          ADD ESP,0x20  (3 bytes)
 *             C2 04 00          RET 0x4       (3 bytes)
 *   Total:    6 bytes
 *   Replace:  E8 xx xx xx xx    CALL code_cave1 (5 bytes)
 *             90                NOP            (1 byte)
 *
 *   At this point EAX = ball pointer (return value from Ball_ctor2).
 *   ECX = frame chain (from FS:[0] pop), not the ball.
 *
 *   Code cave 1 (15 bytes):
 *     50              PUSH EAX          ; save ball pointer
 *     8B C8           MOV ECX,EAX       ; __fastcall this = ball
 *     E8 xx xx xx xx  CALL Ball_Shrink  ; shrink the ball
 *     58              POP EAX           ; restore ball pointer (return value)
 *     83 C4 20        ADD ESP,0x20      ; original instruction
 *     C2 04 00        RET 0x4           ; original instruction
 */
#define HOOK1_ADDR     0x00403DB1
#define HOOK1_ORIG    "\x83\xC4\x20\xC2\x04\x00"
#define HOOK1_LEN      6

/*
 * Hook 2: CreateBadBall — FSTP [ESI+0x284]
 *
 *   Address:  0x0040BE74
 *   Original: D9 9E 84 02 00 00    FSTP [ESI+0x284]  (6 bytes)
 *   Replace:  E8 xx xx xx xx       CALL code_cave2    (5 bytes)
 *             90                   NOP                  (1 byte)
 *
 *   At this point ESI = ball pointer, and the SIZE value from the level data
 *   is on the FPU stack. Ball_Shrink doesn't use the FPU stack, so the SIZE
 *   value will be lost (popped by nothing). That's fine — Ball_Shrink sets
 *   a fixed radius of 13.0, overriding whatever SIZE was specified.
 *
 *   Wait — we need to pop the FPU value to avoid stack corruption!
 *   Code cave 2 (18 bytes):
 *     D9 9E 84 02 00 00   FSTP [ESI+0x284]   ; original: store SIZE (pops FPU)
 *     56                  PUSH ESI            ; save ball pointer
 *     8B CE               MOV ECX,ESI        ; __fastcall this = ball
 *     E8 xx xx xx xx      CALL Ball_Shrink   ; shrink (overrides SIZE)
 *     5E                  POP ESI            ; restore ESI
 *     C3                  RET
 */
#define HOOK2_ADDR     0x0040BE74
#define HOOK2_ORIG    "\xD9\x9E\x84\x02\x00\x00"
#define HOOK2_LEN      6

/* ═══════════════════════════════════════════════════════════════════════════
 * Memory Patching Helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

static int patch_bytes(BYTE *addr, const BYTE *expected, const BYTE *replacement, SIZE_T len)
{
    DWORD oldProtect;
    if (memcmp(addr, expected, len) != 0)
        return 0;
    if (!VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect))
        return 0;
    memcpy(addr, replacement, len);
    VirtualProtect(addr, len, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), addr, len);
    return 1;
}

static int write_bytes(BYTE *addr, const BYTE *data, SIZE_T len)
{
    DWORD oldProtect;
    if (!VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect))
        return 0;
    memcpy(addr, data, len);
    VirtualProtect(addr, len, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), addr, len);
    return 1;
}

/* Allocate a code cave within ±2GB of a target address for relative CALL. */
static void *allocate_code_cave(void *near_addr, SIZE_T size)
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    /* Try just past the .text section first (preferred) */
    DWORD_PTR addr = (IMAGE_BASE + 0xF8000);
    SIZE_T alloc_size = ((size + si.dwPageSize - 1) / si.dwPageSize) * si.dwPageSize;
    void *cave = VirtualAlloc((void*)addr, alloc_size, MEM_COMMIT | MEM_RESERVE,
                              PAGE_EXECUTE_READWRITE);
    if (cave)
        return cave;

    /* Scan outward from the target */
    DWORD_PTR target = (DWORD_PTR)near_addr;
    DWORD_PTR lo = target > 0x40000000 ? target - 0x40000000 : (DWORD_PTR)si.lpMinimumApplicationAddress;
    DWORD_PTR hi = target + 0x40000000;

    for (DWORD_PTR a = target & ~((DWORD_PTR)si.dwPageSize - 1);
         a >= lo && a <= hi;
         a += si.dwPageSize)
    {
        cave = VirtualAlloc((void*)a, alloc_size, MEM_COMMIT | MEM_RESERVE,
                           PAGE_EXECUTE_READWRITE);
        if (cave)
            return cave;
        if (a <= lo) break;
    }

    return VirtualAlloc(NULL, alloc_size, MEM_COMMIT | MEM_RESERVE,
                       PAGE_EXECUTE_READWRITE);
}

/* Build a relative CALL (E8 rel32) at call_addr targeting target_addr. */
static int make_rel32_call(BYTE *call_addr, DWORD_PTR target_addr)
{
    DWORD_PTR src = (DWORD_PTR)call_addr + 5;  /* E8 + 4 bytes = 5 */
    ptrdiff_t rel = (ptrdiff_t)(target_addr - src);
    if (rel > 0x7FFFFFFF || rel < (ptrdiff_t)0x80000000)
        return 0;
    int32_t rel32 = (int32_t)rel;
    call_addr[0] = 0xE8;
    memcpy(call_addr + 1, &rel32, 4);
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Code Cave Builders
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Build code cave 1: Ball_ctor2 exit hook.
 *   PUSH EAX; MOV ECX,EAX; CALL Ball_Shrink; POP EAX; ADD ESP,0x20; RET 0x4
 * Returns total size. */
static int build_cave1(BYTE *cave, DWORD_PTR cave_addr)
{
    int off = 0;

    /* 50: PUSH EAX */
    cave[off++] = 0x50;

    /* 8B C8: MOV ECX,EAX */
    cave[off++] = 0x8B; cave[off++] = 0xC8;

    /* E8 rel32: CALL Ball_Shrink */
    off += 5;  /* placeholder — filled below */
    make_rel32_call(&cave[off - 5], (DWORD_PTR)BALL_SHRINK_ADDR);

    /* 58: POP EAX */
    cave[off++] = 0x58;

    /* 83 C4 20: ADD ESP,0x20 (original instruction) */
    cave[off++] = 0x83; cave[off++] = 0xC4; cave[off++] = 0x20;

    /* C2 04 00: RET 0x4 (original instruction) */
    cave[off++] = 0xC2; cave[off++] = 0x04; cave[off++] = 0x00;

    return off;
}

/* Build code cave 2: CreateBadBall FSTP hook.
 *   FSTP [ESI+0x284]; PUSH ESI; MOV ECX,ESI; CALL Ball_Shrink; POP ESI; RET
 * The FSTP pops the FPU stack (prevents corruption), then Ball_Shrink overrides. */
static int build_cave2(BYTE *cave, DWORD_PTR cave_addr)
{
    int off = 0;

    /* D9 9E 84 02 00 00: FSTP [ESI+0x284] (original — pops FPU stack) */
    cave[off++] = 0xD9; cave[off++] = 0x9E;
    cave[off++] = 0x84; cave[off++] = 0x02;
    cave[off++] = 0x00; cave[off++] = 0x00;

    /* 56: PUSH ESI (save ball pointer) */
    cave[off++] = 0x56;

    /* 8B CE: MOV ECX,ESI (__fastcall this = ball) */
    cave[off++] = 0x8B; cave[off++] = 0xCE;

    /* E8 rel32: CALL Ball_Shrink */
    off += 5;
    make_rel32_call(&cave[off - 5], (DWORD_PTR)BALL_SHRINK_ADDR);

    /* 5E: POP ESI (restore) */
    cave[off++] = 0x5E;

    /* C3: RET */
    cave[off++] = 0xC3;

    return off;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static void patch_thread(void *param)
{
    (void)param;

    HMODULE hExe = GetModuleHandleA(NULL);
    if (!hExe) return;
    BYTE *base = (BYTE*)hExe;

    /* Wait for the game to finish loading */
    Sleep(500);

    int hook1_ok = 0, hook2_ok = 0;

    /* ═══════════════════════════════════════════════════════════════════════
     * Hook 1: Ball_ctor2 exit → Ball_Shrink
     * Catches: all balls (player, split, follow, board-init)
     * ═══════════════════════════════════════════════════════════════════════ */
    {
        BYTE *hook1_addr = base + (HOOK1_ADDR - IMAGE_BASE);

        if (memcmp(hook1_addr, HOOK1_ORIG, HOOK1_LEN) == 0)
        {
            BYTE cave1[32];
            int cave1_size = build_cave1(cave1, 0);  /* rel32 filled by make_rel32_call */

            void *cave = allocate_code_cave(hook1_addr, cave1_size);
            if (cave && write_bytes((BYTE*)cave, cave1, cave1_size))
            {
                BYTE call_nop[6];
                call_nop[0] = 0xE8;
                call_nop[5] = 0x90;  /* NOP */

                /* Fix CALL target to point to our cave */
                DWORD_PTR src = (DWORD_PTR)hook1_addr + 5;
                ptrdiff_t rel = (ptrdiff_t)((DWORD_PTR)cave - src);
                if (rel <= 0x7FFFFFFF && rel >= (ptrdiff_t)0x80000000)
                {
                    int32_t rel32 = (int32_t)rel;
                    memcpy(call_nop + 1, &rel32, 4);
                    hook1_ok = write_bytes(hook1_addr, call_nop, 6);
                }
            }
        }
    }

    /* ═══════════════════════════════════════════════════════════════════════
     * Hook 2: CreateBadBall FSTP → Ball_Shrink
     * Catches: AI/bad balls (after SIZE computed)
     * ═══════════════════════════════════════════════════════════════════════ */
    {
        BYTE *hook2_addr = base + (HOOK2_ADDR - IMAGE_BASE);

        if (memcmp(hook2_addr, HOOK2_ORIG, HOOK2_LEN) == 0)
        {
            BYTE cave2[32];
            int cave2_size = build_cave2(cave2, 0);

            void *cave = allocate_code_cave(hook2_addr, cave2_size);
            if (cave && write_bytes((BYTE*)cave, cave2, cave2_size))
            {
                BYTE call_nop[6];
                call_nop[0] = 0xE8;
                call_nop[5] = 0x90;

                DWORD_PTR src = (DWORD_PTR)hook2_addr + 5;
                ptrdiff_t rel = (ptrdiff_t)((DWORD_PTR)cave - src);
                if (rel <= 0x7FFFFFFF && rel >= (ptrdiff_t)0x80000000)
                {
                    int32_t rel32 = (int32_t)rel;
                    memcpy(call_nop + 1, &rel32, 4);
                    hook2_ok = write_bytes(hook2_addr, call_nop, 6);
                }
            }
        }
    }

    /* Write a log file so users can verify the patches applied */
    {
        char log_path[MAX_PATH];
        GetModuleFileNameA(hExe, log_path, MAX_PATH);
        char *dot = strrchr(log_path, '.');
        if (dot) strcpy(dot, "_half_size.log");
        else strcat(log_path, "_half_size.log");

        FILE *f = NULL;
        if (fopen_s(&f, log_path, "w") == 0 && f)
        {
            fprintf(f, "Hamsterball Half-Size Balls Mod (v2)\n");
            fprintf(f, "====================================\n\n");
            fprintf(f, "Uses Ball_Shrink (0x00402200) — the game's own shrink function\n");
            fprintf(f, "from Odd Race. Sets radius=13.0, physics_scale=2.5, is_falling=1.\n\n");
            fprintf(f, "Hook 1 (Ball_ctor2 exit → Ball_Shrink):  %s\n",
                    hook1_ok ? "APPLIED" : "FAILED");
            fprintf(f, "  Catches: player, split, follow, board-init balls\n");
            fprintf(f, "Hook 2 (CreateBadBall FSTP → Ball_Shrink): %s\n",
                    hook2_ok ? "APPLIED" : "FAILED");
            fprintf(f, "  Catches: AI/bad balls (after SIZE computed)\n\n");
            fprintf(f, "Exe base: 0x%08X\n", (unsigned)(DWORD_PTR)base);
            fclose(f);
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInst);
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)patch_thread, NULL, 0, NULL);
        break;
    }
    return TRUE;
}
