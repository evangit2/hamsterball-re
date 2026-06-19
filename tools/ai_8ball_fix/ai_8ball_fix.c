/*
 * ai_8ball_fix.c — DLL proxy (bass.dll) that patches ONE gate to make
 * motionless grounded 8balls actually move and chase targets.
 *
 * ROOT CAUSE: At the very TOP of Ball_AI_ChaseNearest (vtable[4], 0x408390),
 * before ANY physics or targeting runs, there's a gate at 0x4083AE:
 *
 *   0x408396: MOV EAX, [ESI+0xC74]    ; ball+0xC74 (DWORD, render counter)
 *   0x40839F: CMP EAX, 0               ; is it zero?
 *   0x4083A1: JNZ 0x4083B4            ; if non-zero, proceed (skip App check)
 *   0x4083A3: MOV EAX, [ESI+0x10]     ; App*
 *   0x4083A6: MOV CL, [EAX+0x237]     ; App+0x237 (is_2P_split_screen flag)
 *   0x4083AC: TEST CL, CL
 *   0x4083AE: JZ 0x40882A             ; GATE 0: if BOTH zero → RETURN IMMEDIATELY
 *
 * For CreateBadBall'd 8balls in practice mode:
 *   - ball+0xC74 = 0 (freshly memset'd; only set during Ball_Render)
 *   - App+0x237 = 0 (only set in App_Start2PRace for 2-player split-screen)
 *   → BOTH zero → JZ fires → function exits immediately
 *   → Ball_Update never runs, targeting never runs, SetVelocity never runs
 *   → 8ball sits motionless forever
 *
 * FIX: NOP the 6-byte JZ at 0x4083AE (0F 84 76 04 00 00 → 90×6).
 *      This lets the function fall through to Ball_Update + targeting + SetVelocity.
 *
 * NOTE: The previous 3-patch approach (0x4083D3, 0x4085D1, 0x408598) was WRONG:
 *   - 0x4083D3: Skips notification code when ball was already inactive — harmless but irrelevant
 *   - 0x408598: Target filter — skips inactive target balls. NOP-ing risks crashing.
 *   - 0x4085D1: Target filter — skips balls with player_index==-1. NOP-ing risks crashing.
 *   All three were DOWNSTREAM of Gate 0 and never reached anyway.
 *
 * Build: i686-w64-mingw32-gcc -shared -o bass.dll ai_8ball_fix.c \
 *          bass_exports.def -lwinmm -Wl,--enable-stdcall-fixup \
 *          -O2 -static -static-libgcc -Wl,--add-stdcall-alias
 */

#include <windows.h>
#include <winbase.h>

/* ---- bass.dll proxy exports (stubs — Hamsterball only needs import resolution) ---- */

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

/* ---- Memory patching ---- */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    HMODULE hExe;
    BYTE* addr;
    DWORD oldProtect;

    if (dwReason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    hExe = GetModuleHandleA(NULL);
    if (!hExe) {
        return TRUE;
    }

    /* ── GATE 0: The ONE and ONLY root cause ──
     *
     * 0x4083AE: JZ 0x40882A (6 bytes: 0F 84 76 04 00 00)
     * RVA = 0x4083AE - 0x400000 = 0x83AE
     *
     * This fires when ball+0xC74 == 0 AND App+0x237 == 0.
     * For freshly-created 8balls in practice mode, both are 0.
     * It jumps directly to the function exit (0x40882A: POP/RET),
     * skipping EVERYTHING: Ball_Update, targeting loop, SetVelocity.
     *
     * NOP it so the function always proceeds to Ball_Update + targeting.
     */
    addr = (BYTE*)hExe + 0x83AE;
    if (VirtualProtect(addr, 6, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        /* Verify we're patching the right bytes: 0F 84 76 04 00 00 */
        if (addr[0] == 0x0F && addr[1] == 0x84) {
            addr[0] = 0x90; /* NOP */
            addr[1] = 0x90;
            addr[2] = 0x90;
            addr[3] = 0x90;
            addr[4] = 0x90;
            addr[5] = 0x90;
        }
        VirtualProtect(addr, 6, oldProtect, &oldProtect);
    }

    /* timeBeginPeriod(1) for smoother timer resolution */
    timeBeginPeriod(1);

    return TRUE;
}
