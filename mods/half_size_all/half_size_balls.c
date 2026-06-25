/*
 * half_size_balls.c — BASS.dll proxy that shrinks the player's ball to half size.
 *
 * Hooks the player ball spawn in Scene_SpawnBallsAndObjects and inlines the
 * same field writes that Ball_Shrink (0x00402200) performs — but WITHOUT
 * calling the function, so no sound effect plays.
 *
 * Only affects player index 0. AI balls, split balls, follow balls, and
 * board-init balls are left at their normal size.
 *
 * Fields set (identical to Ball_Shrink):
 *   ball+0x284 = 0x41500000  (radius = 13.0, down from 26.0)
 *   ball+0x188 = 0x40200000  (physics_scale = 2.5, down from 5.0)
 *   ball+0xC4C = 1            (is_falling flag)
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
 *   3. Run the game — player ball will be half size
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports
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
 * Hook Constants
 * ═══════════════════════════════════════════════════════════════════════════ */

#define IMAGE_BASE  0x00400000

/*
 * Hook point: Scene_SpawnBallsAndObjects, after AthenaList_Append returns.
 *
 * At 0x0041C8D7 the game executes:
 *   C6 86 81 02 00 00 00    MOV byte [ESI+0x281], 0   (7 bytes)
 *
 * ESI = ball pointer, [ESI+0x18] = player_index (set at 0x0041C893).
 * This runs inside a loop that creates one ball per player. We replace it
 * with a CALL to a code cave that:
 *   1. Checks if [ESI+0x18] == 0 (player index 0)
 *   2. If yes, writes the three Ball_Shrink fields inline (no function call)
 *   3. Executes the original MOV byte [ESI+0x281], 0
 *   4. JMPs back to 0x0041C8DE
 */
#define HOOK_ADDR     0x0041C8D7
#define HOOK_ORIG    "\xC6\x86\x81\x02\x00\x00\x00"   /* MOV byte [ESI+0x281],0 */
#define HOOK_LEN      7
#define RETURN_ADDR   0x0041C8DE   /* instruction after the hooked one */

/*
 * Code cave layout (36 bytes):
 *
 *   83 BE 18 00 00 00 00           CMP dword [ESI+0x18], 0      ; 7 bytes
 *   75 1B                          JNE skip                      ; 2 bytes
 *   C7 86 84 02 00 00 00 00 50 41  MOV dword [ESI+0x284], 0x41500000  ; 10 bytes (radius=13.0)
 *   C7 86 88 01 00 00 00 00 20 40  MOV dword [ESI+0x188], 0x40200000  ; 10 bytes (physics=2.5)
 *   C6 86 4C 0C 00 00 01           MOV byte [ESI+0xC4C], 1      ; 7 bytes (is_falling=1)
 * skip:
 *   C6 86 81 02 00 00 00           MOV byte [ESI+0x281], 0      ; 7 bytes (original instruction)
 *   E9 xx xx xx xx                 JMP RETURN_ADDR              ; 5 bytes
 * Total: 7+2+10+10+7+7+5 = 48 bytes
 *
 * The JNE offset 0x1B = 27 = 10+10+7 (the three field writes it skips).
 */
#define CAVE_SIZE  48

static const unsigned char cave_template[CAVE_SIZE] = {
    /* CMP dword [ESI+0x18], 0 */
    0x83, 0xBE, 0x18, 0x00, 0x00, 0x00, 0x00,
    /* JNE skip (offset 0x1B = 27 bytes ahead) */
    0x75, 0x1B,
    /* MOV dword [ESI+0x284], 0x41500000 (radius = 13.0) */
    0xC7, 0x86, 0x84, 0x02, 0x00, 0x00, 0x00, 0x00, 0x50, 0x41,
    /* MOV dword [ESI+0x188], 0x40200000 (physics_scale = 2.5) */
    0xC7, 0x86, 0x88, 0x01, 0x00, 0x00, 0x00, 0x00, 0x20, 0x40,
    /* MOV byte [ESI+0xC4C], 1 (is_falling flag) */
    0xC6, 0x86, 0x4C, 0x0C, 0x00, 0x00, 0x01,
    /* skip: — MOV byte [ESI+0x281], 0 (original instruction) */
    0xC6, 0x86, 0x81, 0x02, 0x00, 0x00, 0x00,
    /* JMP rel32 back to RETURN_ADDR — filled at runtime */
    0xE9, 0x00, 0x00, 0x00, 0x00
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Memory Patching Helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

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

/* Allocate a code cave within ±2GB for relative CALL/JMP. */
static void *allocate_code_cave(void *near_addr, SIZE_T size)
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    SIZE_T alloc_size = ((size + si.dwPageSize - 1) / si.dwPageSize) * si.dwPageSize;

    /* Try just past .text section first */
    DWORD_PTR addr = (IMAGE_BASE + 0xF8000);
    void *cave = VirtualAlloc((void*)addr, alloc_size, MEM_COMMIT | MEM_RESERVE,
                              PAGE_EXECUTE_READWRITE);
    if (cave)
        return cave;

    /* Scan outward from target */
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

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static void patch_thread(void *param)
{
    (void)param;

    HMODULE hExe = GetModuleHandleA(NULL);
    if (!hExe) return;
    BYTE *base = (BYTE*)hExe;

    Sleep(500);

    int hook_ok = 0;
    BYTE *hook_addr = base + (HOOK_ADDR - IMAGE_BASE);

    /* Verify original bytes match */
    if (memcmp(hook_addr, HOOK_ORIG, HOOK_LEN) == 0)
    {
        /* Build code cave */
        unsigned char cave[CAVE_SIZE];
        memcpy(cave, cave_template, CAVE_SIZE);

        /* Allocate cave within relative JMP range */
        void *cave_mem = allocate_code_cave(hook_addr, CAVE_SIZE);
        if (cave_mem && write_bytes((BYTE*)cave_mem, cave, CAVE_SIZE))
        {
            /* Fix JMP rel32 at end of cave to target RETURN_ADDR */
            BYTE *jmp_addr = (BYTE*)cave_mem + CAVE_SIZE - 5;  /* E9 + 4 bytes */
            DWORD_PTR jmp_src = (DWORD_PTR)jmp_addr + 5;  /* address after JMP */
            ptrdiff_t jmp_rel = (ptrdiff_t)(base + (RETURN_ADDR - IMAGE_BASE) - jmp_src);
            if (jmp_rel <= 0x7FFFFFFF && jmp_rel >= (ptrdiff_t)0x80000000)
            {
                int32_t rel32 = (int32_t)jmp_rel;
                memcpy(jmp_addr + 1, &rel32, 4);

                /* Build CALL + 2 NOPs to replace original 7-byte instruction */
                BYTE call_nop[7];
                call_nop[0] = 0xE8;  /* CALL rel32 */
                /* CALL target = cave_mem */
                DWORD_PTR call_src = (DWORD_PTR)hook_addr + 5;
                ptrdiff_t call_rel = (ptrdiff_t)((DWORD_PTR)cave_mem - call_src);
                if (call_rel <= 0x7FFFFFFF && call_rel >= (ptrdiff_t)0x80000000)
                {
                    int32_t call_rel32 = (int32_t)call_rel;
                    memcpy(call_nop + 1, &call_rel32, 4);
                    call_nop[5] = 0x90;  /* NOP */
                    call_nop[6] = 0x90;  /* NOP */

                    hook_ok = write_bytes(hook_addr, call_nop, 7);
                }
            }
        }
    }

    /* Write log file */
    {
        char log_path[MAX_PATH];
        GetModuleFileNameA(hExe, log_path, MAX_PATH);
        char *dot = strrchr(log_path, '.');
        if (dot) strcpy(dot, "_half_size.log");
        else strcat(log_path, "_half_size.log");

        FILE *f = NULL;
        if (fopen_s(&f, log_path, "w") == 0 && f)
        {
            fprintf(f, "Hamsterball Half-Size Balls Mod (v3)\n");
            fprintf(f, "====================================\n\n");
            fprintf(f, "Inlines Ball_Shrink physics (no sound, no function call):\n");
            fprintf(f, "  ball+0x284 = 13.0 (radius)\n");
            fprintf(f, "  ball+0x188 = 2.5  (physics_scale)\n");
            fprintf(f, "  ball+0xC4C = 1    (is_falling)\n\n");
            fprintf(f, "Only applies to player index 0.\n\n");
            fprintf(f, "Hook (Scene_SpawnBallsAndObjects 0x0041C8D7): %s\n",
                    hook_ok ? "APPLIED" : "FAILED");
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
