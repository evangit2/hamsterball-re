/*
 * ai_8ball_full_v3.c — DLL proxy (bass.dll) that patches 8ball AI:
 *   1. Fixes motionless 8balls (Gate 0 bypass)
 *   2. Makes 8balls target ALL entities including other 8balls (self-exclusion filter)
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * PATCH 1: Gate 0 @ 0x4083AE (RVA 0x83AE) — motionless fix
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Function: Ball_UpdateAndAI at 0x408390 (Ball vtable[4])
 *
 *   0x408396: MOV EAX, [ESI+0xC74]    ; ball+0xC74
 *   0x40839F: CMP EAX, 0
 *   0x4083A1: JNZ 0x4083B4            ; if non-zero, proceed
 *   0x4083A3: MOV EAX, [ESI+0x10]     ; App*
 *   0x4083A6: MOV CL, [EAX+0x237]     ; App+0x237 (is_game_in_progress)
 *   0x4083AC: TEST CL, CL
 *   0x4083AE: JZ 0x40882A             ; GATE 0: if BOTH zero → EXIT
 *
 * For CreateBadBall 8balls: ball+0xC74 == 0 (only Shatter sets it to 1)
 * In practice/countdown: App+0x237 == 0 (set to 1 by Game_SetInProgress)
 * → Both zero → JZ fires → Ball_Update + targeting + SetVelocity NEVER run.
 *
 * FIX: NOP 6 bytes → always fall through to Ball_Update + targeting.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * PATCH 2: Targeting filter @ 0x4085CD (RVA 0x85CD) — target all entities
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Inside the targeting loop (iterates ball list at Scene+0x29D4):
 *
 *   0x4085CD: CMP DWORD PTR [EDI+0x18], 0xFFFFFFFF   ; EDI = candidate ball
 *   0x4085D1: JE 0x408634                             ; skip if player_index == -1
 *
 * ball+0x18 = player_index, set to -1 by Ball_ctor2 for ALL balls.
 * Player balls get 0–3. CreateBadBall 8balls keep -1.
 * This filter skips ALL 8balls → 8balls can only target players 1–4.
 *
 * FIX: Replace "cmp [edi+0x18], -1" with "cmp esi, edi" (self-check):
 *   39 FE 90 90 74 61
 *   cmp esi, edi    ; is candidate ball ME?
 *   nop nop
 *   je 0x408634     ; skip if target == self
 *
 * This makes 8balls consider ALL other balls as valid targets — players AND
 * other 8balls — while preventing self-targeting (zero distance → NaN velocity).
 *
 * Other targeting filters (all left intact):
 *   0x408598: [edi+0x768]==0 → skip (fallen/inactive ball)
 *   0x4085A6: [edi+0x2F9]!=0 → skip (special state)
 *   0x4085B4: [edi+0x300]!=0 → skip (finished/eliminated)
 *   0x4085BE: [edi+0x324]!=0 → skip (special mode)
 *   0x4085CB: Scene+0x3A4C==0 → skip (scene flag)
 *
 * Build: i686-w64-mingw32-gcc -shared -o bass.dll ai_8ball_full_v3.c \
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

/* Helper: verify-then-patch with byte signature check */
static void patch_bytes(BYTE* addr, const BYTE* expected, const BYTE* replacement,
                        size_t len, DWORD oldProtect)
{
    if (memcmp(addr, expected, len) != 0) {
        /* Signature mismatch — wrong game version or already patched */
        return;
    }
    memcpy(addr, replacement, len);
    VirtualProtect(addr, len, oldProtect, &oldProtect);
}

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

    /* ══ PATCH 1: Gate 0 — NOP the 6-byte JZ at RVA 0x83AE ══ */
    addr = (BYTE*)hExe + 0x83AE;
    if (VirtualProtect(addr, 6, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        /* 0F 84 76 04 00 00 → 90 90 90 90 90 90 */
        static const BYTE gate0_orig[6]   = { 0x0F, 0x84, 0x76, 0x04, 0x00, 0x00 };
        static const BYTE gate0_patch[6]  = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        patch_bytes(addr, gate0_orig, gate0_patch, 6, oldProtect);
    }

    /* ══ PATCH 2: Targeting filter — self-exclusion at RVA 0x85CD ══ */
    addr = (BYTE*)hExe + 0x85CD;
    if (VirtualProtect(addr, 6, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        /* 83 7F 18 FF 74 61 → 39 FE 90 90 74 61 */
        /* cmp [edi+0x18],-1; je skip  →  cmp esi,edi; nop; nop; je skip */
        static const BYTE filter_orig[6]  = { 0x83, 0x7F, 0x18, 0xFF, 0x74, 0x61 };
        static const BYTE filter_patch[6]  = { 0x39, 0xFE, 0x90, 0x90, 0x74, 0x61 };
        patch_bytes(addr, filter_orig, filter_patch, 6, oldProtect);
    }

    /* timeBeginPeriod(1) for smoother timer resolution */
    timeBeginPeriod(1);

    return TRUE;
}
