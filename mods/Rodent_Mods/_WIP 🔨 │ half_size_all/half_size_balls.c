/*
 * half_size_balls.c — BASS.dll proxy that halves all ball sizes.
 *
 * Patches 3 sites:
 *   1. Ball_ctor2 default radius:  27.0 → 13.5  (MOV immediate patch)
 *   2. Player ball spawn radius:   26.0 → 13.0  (MOV immediate patch)
 *   3. CreateBadBall SIZE tag:     halve FPU value via code cave
 *
 * Ball_Shatter copies the parent's radius, which is already halved
 * by patch #1 or #2, so split balls are automatically half size too.
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
 *
 * If you already use another bass.dll proxy mod (FPS unlock, player clones,
 * etc.), you can merge this into that proxy — just combine the patch_thread
 * functions and DllMain calls.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
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

/*
 * Patch 1: Ball_ctor2 default radius
 *
 *   Address:  0x00403C8B
 *   Original: C7 86 84 02 00 00 00 00 D8 41
 *             MOV dword ptr [ESI+0x284], 0x41D80000  (27.0f)
 *   Patched:  C7 86 84 02 00 00 00 00 58 41
 *             MOV dword ptr [ESI+0x284], 0x41580000  (13.5f)
 *
 * The immediate value starts at offset +6 from the instruction start.
 * We patch 4 bytes at 0x00403C91.
 */
#define PATCH1_ADDR      0x00403C91
#define PATCH1_ORIGINAL  "\x00\x00\xD8\x41"   /* 27.0f */
#define PATCH1_PATCHED   "\x00\x00\x58\x41"   /* 13.5f */
#define PATCH1_LEN       4

/*
 * Patch 2: Player ball spawn radius (Scene_SpawnBallsAndObjects)
 *
 *   Address:  0x0041C8AA
 *   Original: C7 86 84 02 00 00 00 00 D0 41
 *             MOV dword ptr [ESI+0x284], 0x41D00000  (26.0f)
 *   Patched:  C7 86 84 02 00 00 00 00 50 41
 *             MOV dword ptr [ESI+0x284], 0x41500000  (13.0f)
 *
 * The immediate value starts at offset +6 from the instruction start.
 * We patch 4 bytes at 0x0041C8B0.
 */
#define PATCH2_ADDR      0x0041C8B0
#define PATCH2_ORIGINAL  "\x00\x00\xD0\x41"   /* 26.0f */
#define PATCH2_PATCHED   "\x00\x00\x50\x41"   /* 13.0f */
#define PATCH2_LEN       4

/*
 * Patch 3: CreateBadBall SIZE tag handler
 *
 *   Address:  0x0040BE74
 *   Original: D9 9E 84 02 00 00
 *             FSTP float ptr [ESI+0x284]
 *
 * This instruction writes the SIZE value from the FPU stack to ball+0x284.
 * We replace it with a CALL to a code cave that:
 *   1. Executes the original FSTP (stores the SIZE value)
 *   2. Reloads the value
 *   3. Multiplies by 0.5f (halving it)
 *   4. Stores the halved value back
 *
 * The CALL is 5 bytes (E8 + rel32), plus 1 NOP = 6 bytes (same as original).
 *
 * Code cave (25 bytes):
 *   D9 9E 84 02 00 00     FSTP [ESI+0x284]      ; original store
 *   D9 86 84 02 00 00     FLD  [ESI+0x284]      ; reload value
 *   D8 0D <addr>          FMUL dword [0x41C89C]  ; multiply by 0.5f
 *   D9 9E 84 02 00 00     FSTP [ESI+0x284]      ; store halved value
 *   C3                     RET
 *
 * 0.5f (0x3F000000) lives at VA 0x0041C89C as the immediate of the
 * MOV [ESI+0x278], 0x3F000000 instruction in Scene_SpawnBallsAndObjects.
 */
#define PATCH3_ADDR      0x0040BE74
#define PATCH3_ORIGINAL  "\xD9\x9E\x84\x02\x00\x00"   /* FSTP [ESI+0x284] */
#define PATCH3_LEN       6

/* Address of 0.5f float constant in .text (immediate of nearby MOV) */
#define HALF_FLOAT_ADDR   0x0041C89C

/* Code cave machine code (built at runtime) */
static unsigned char code_cave[] = {
    0xD9, 0x9E, 0x84, 0x02, 0x00, 0x00,   /* FSTP [ESI+0x284] */
    0xD9, 0x86, 0x84, 0x02, 0x00, 0x00,   /* FLD  [ESI+0x284] */
    0xD8, 0x0D, 0x9C, 0xC8, 0x41, 0x00,   /* FMUL dword [0x0041C89C] */
    0xD9, 0x9E, 0x84, 0x02, 0x00, 0x00,   /* FSTP [ESI+0x284] */
    0xC3                                    /* RET */
};
#define CODE_CAVE_SIZE   sizeof(code_cave)

/* ═══════════════════════════════════════════════════════════════════════════
 * Memory Patching Helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Verify bytes match expected pattern, then patch in place.
 * Returns 1 on success, 0 on mismatch or failure. */
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

/* Write arbitrary bytes to an address (no verify, for code cave). */
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

/* Allocate a code cave near a target address (within ±2GB for relative CALL).
 * Returns the cave address, or NULL on failure. */
static void *allocate_code_cave(void *near_addr, SIZE_T size)
{
    /* Try VirtualAlloc near the target. If the exe is loaded at its preferred
     * base (0x400000), we can allocate right after the .text section. */
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    /* Search for free memory pages within ±2GB of the target */
    DWORD_PTR target = (DWORD_PTR)near_addr;
    DWORD_PTR lo = target > 0x40000000 ? target - 0x40000000 : (DWORD_PTR)si.lpMinimumApplicationAddress;
    DWORD_PTR hi = target + 0x40000000;

    /* Round size to page boundary */
    SIZE_T alloc_size = ((size + si.dwPageSize - 1) / si.dwPageSize) * si.dwPageSize;

    /* Try addresses near the exe first (preferred base region) */
    DWORD_PTR addr = (IMAGE_BASE + 0xF8000);  /* just past typical .text end */
    void *cave = VirtualAlloc((void*)addr, alloc_size, MEM_COMMIT | MEM_RESERVE,
                              PAGE_EXECUTE_READWRITE);
    if (cave)
        return cave;

    /* Scan outward from the target */
    for (DWORD_PTR a = target & ~((DWORD_PTR)si.dwPageSize - 1);
         a >= lo && a <= hi;
         a += (a > target ? si.dwPageSize : -(DWORD_PTR)si.dwPageSize))
    {
        cave = VirtualAlloc((void*)a, alloc_size, MEM_COMMIT | MEM_RESERVE,
                           PAGE_EXECUTE_READWRITE);
        if (cave)
            return cave;
        if (a <= lo) break;
    }

    /* Last resort: let the OS pick anywhere (may fail for relative CALL) */
    return VirtualAlloc(NULL, alloc_size, MEM_COMMIT | MEM_RESERVE,
                       PAGE_EXECUTE_READWRITE);
}

/* Calculate relative offset for E8 CALL instruction.
 * CALL rel32: E8 xx xx xx xx
 * rel32 = target - (call_addr + 5) */
static int make_rel32_call(BYTE *call_addr, void *target)
{
    DWORD_PTR src = (DWORD_PTR)call_addr + 5;  /* E8 + 4 bytes = 5 bytes */
    DWORD_PTR dst = (DWORD_PTR)target;
    ptrdiff_t rel = (ptrdiff_t)(dst - src);

    /* Check it fits in int32 */
    if (rel > 0x7FFFFFFF || rel < (ptrdiff_t)0x80000000)
        return 0;

    int32_t rel32 = (int32_t)rel;
    memcpy(call_addr + 1, &rel32, 4);
    return 1;
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

    /* Resolve ASLR-safe addresses */
    BYTE *patch1_addr = base + (PATCH1_ADDR - IMAGE_BASE);
    BYTE *patch2_addr = base + (PATCH2_ADDR - IMAGE_BASE);
    BYTE *patch3_addr = base + (PATCH3_ADDR - IMAGE_BASE);

    /* Wait for the game to finish loading (it patches its own code during init) */
    Sleep(500);

    int results = 0;

    /* ═══════════════════════════════════════════════════════════════════════
     * Patch 1: Ball_ctor2 default radius 27.0 → 13.5
     * ═══════════════════════════════════════════════════════════════════════ */
    {
        int ok = patch_bytes(patch1_addr,
                             (const BYTE*)PATCH1_ORIGINAL,
                             (const BYTE*)PATCH1_PATCHED,
                             PATCH1_LEN);
        results += ok;
    }

    /* ═══════════════════════════════════════════════════════════════════════
     * Patch 2: Player ball spawn radius 26.0 → 13.0
     * ═══════════════════════════════════════════════════════════════════════ */
    {
        int ok = patch_bytes(patch2_addr,
                             (const BYTE*)PATCH2_ORIGINAL,
                             (const BYTE*)PATCH2_PATCHED,
                             PATCH2_LEN);
        results += ok;
    }

    /* ═══════════════════════════════════════════════════════════════════════
     * Patch 3: CreateBadBall SIZE tag — code cave to halve FPU value
     *
     * Replace:  FSTP [ESI+0x284]            (6 bytes)
     * With:     CALL code_cave + NOP        (5+1 bytes)
     *
     * Code cave does the FSTP, then halves the stored value via FMUL 0.5f.
     * ═══════════════════════════════════════════════════════════════════════ */
    {
        /* Verify original bytes */
        if (memcmp(patch3_addr, PATCH3_ORIGINAL, PATCH3_LEN) == 0)
        {
            /* Allocate code cave within relative CALL range */
            void *cave = allocate_code_cave(patch3_addr, CODE_CAVE_SIZE);
            if (cave)
            {
                /* Write the code cave */
                if (write_bytes((BYTE*)cave, code_cave, CODE_CAVE_SIZE))
                {
                    /* Build the CALL+NOP replacement */
                    BYTE call_nop[6];
                    call_nop[0] = 0xE8;  /* CALL rel32 */
                    call_nop[5] = 0x90;  /* NOP (pad to 6 bytes) */

                    if (make_rel32_call(call_nop, cave))
                    {
                        /* Patch the FSTP → CALL cave + NOP */
                        if (write_bytes(patch3_addr, call_nop, 6))
                        {
                            results++;
                        }
                    }
                }
            }
        }
    }

    /* Write a log file so users can verify the patches applied */
    {
        char log_path[MAX_PATH];
        GetModuleFileNameA(hExe, log_path, MAX_PATH);
        /* Replace .exe with _half_size.log in the path */
        char *dot = strrchr(log_path, '.');
        if (dot) strcpy(dot, "_half_size.log");
        else strcat(log_path, "_half_size.log");

        FILE *f = fopen(log_path, "w");
        if (f)
        {
            fprintf(f, "Hamsterball Half-Size Balls Mod\n");
            fprintf(f, "================================\n");
            fprintf(f, "\n");
            fprintf(f, "Patch 1 (Ball_ctor2 default 27.0→13.5): %s\n",
                    (results & 1) ? "APPLIED" : "FAILED");
            fprintf(f, "Patch 2 (Player spawn 26.0→13.0):       %s\n",
                    (results & 2) ? "APPLIED" : "FAILED");
            fprintf(f, "Patch 3 (CreateBadBall SIZE code cave):  %s\n",
                    (results & 4) ? "APPLIED" : "FAILED");
            fprintf(f, "\n");
            fprintf(f, "Exe base: 0x%08X\n", (unsigned)(DWORD_PTR)base);
            fprintf(f, "Patch1 at: 0x%08X\n", (unsigned)(DWORD_PTR)patch1_addr);
            fprintf(f, "Patch2 at: 0x%08X\n", (unsigned)(DWORD_PTR)patch2_addr);
            fprintf(f, "Patch3 at: 0x%08X\n", (unsigned)(DWORD_PTR)patch3_addr);
            fprintf(f, "\n");
            fprintf(f, "All balls should now render at half their normal size.\n");
            fprintf(f, "Collision is also halved (balls fit through smaller gaps).\n");
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
        /* Spawn the patch thread — don't block DllMain */
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)patch_thread, NULL, 0, NULL);
        break;
    }
    return TRUE;
}
