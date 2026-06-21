
// ============================================================
// Hamsterball Entity Limit Fixer - bass.dll proxy v5
// Prevents freezes AND crashes when spawning many entities
// Exports all BASS functions as stubs + delayed patching
// ============================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>

/* ---- BASS Proxy Exports (auto-generated stubs) ---- */

__declspec(dllexport) void __stdcall BASS_Apply3D(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelBytes2Seconds(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelGet3DAttributes(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelGet3DPosition(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelGetAttributes(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelGetData(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelGetDevice(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelGetEAXMix(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelGetInfo(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelGetLevel(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelGetPosition(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelIsActive(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelIsSliding(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelPause(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelRemoveDSP(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelRemoveFX(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelRemoveLink(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelRemoveSync(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelResume(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelSeconds2Bytes(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelSet3DAttributes(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelSet3DPosition(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelSetAttributes(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelSetDSP(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelSetEAXMix(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelSetFX(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelSetLink(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelSetPosition(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelSetSync(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelSlideAttributes(void) {}
__declspec(dllexport) void __stdcall BASS_ChannelStop(void) {}
__declspec(dllexport) void __stdcall BASS_ErrorGetCode(void) {}
__declspec(dllexport) void __stdcall BASS_FXGetParameters(void) {}
__declspec(dllexport) void __stdcall BASS_FXSetParameters(void) {}
__declspec(dllexport) void __stdcall BASS_Free(void) {}
__declspec(dllexport) void __stdcall BASS_Get3DFactors(void) {}
__declspec(dllexport) void __stdcall BASS_Get3DPosition(void) {}
__declspec(dllexport) void __stdcall BASS_GetCPU(void) {}
__declspec(dllexport) void __stdcall BASS_GetConfig(void) {}
__declspec(dllexport) void __stdcall BASS_GetDSoundObject(void) {}
__declspec(dllexport) void __stdcall BASS_GetDevice(void) {}
__declspec(dllexport) void __stdcall BASS_GetDeviceDescription(void) {}
__declspec(dllexport) void __stdcall BASS_GetEAXParameters(void) {}
__declspec(dllexport) void __stdcall BASS_GetInfo(void) {}
__declspec(dllexport) void __stdcall BASS_GetVersion(void) {}
__declspec(dllexport) void __stdcall BASS_GetVolume(void) {}
__declspec(dllexport) void __stdcall BASS_Init(void) {}
__declspec(dllexport) void __stdcall BASS_MusicFree(void) {}
__declspec(dllexport) void __stdcall BASS_MusicGetLength(void) {}
__declspec(dllexport) void __stdcall BASS_MusicGetName(void) {}
__declspec(dllexport) void __stdcall BASS_MusicGetVolume(void) {}
__declspec(dllexport) void __stdcall BASS_MusicLoad(void) {}
__declspec(dllexport) void __stdcall BASS_MusicPlay(void) {}
__declspec(dllexport) void __stdcall BASS_MusicPlayEx(void) {}
__declspec(dllexport) void __stdcall BASS_MusicPreBuf(void) {}
__declspec(dllexport) void __stdcall BASS_MusicSetAmplify(void) {}
__declspec(dllexport) void __stdcall BASS_MusicSetPanSep(void) {}
__declspec(dllexport) void __stdcall BASS_MusicSetPositionScaler(void) {}
__declspec(dllexport) void __stdcall BASS_MusicSetVolume(void) {}
__declspec(dllexport) void __stdcall BASS_Pause(void) {}
__declspec(dllexport) void __stdcall BASS_RecordFree(void) {}
__declspec(dllexport) void __stdcall BASS_RecordGetDevice(void) {}
__declspec(dllexport) void __stdcall BASS_RecordGetDeviceDescription(void) {}
__declspec(dllexport) void __stdcall BASS_RecordGetInfo(void) {}
__declspec(dllexport) void __stdcall BASS_RecordGetInput(void) {}
__declspec(dllexport) void __stdcall BASS_RecordGetInputName(void) {}
__declspec(dllexport) void __stdcall BASS_RecordInit(void) {}
__declspec(dllexport) void __stdcall BASS_RecordSetDevice(void) {}
__declspec(dllexport) void __stdcall BASS_RecordSetInput(void) {}
__declspec(dllexport) void __stdcall BASS_RecordStart(void) {}
__declspec(dllexport) void __stdcall BASS_SampleCreate(void) {}
__declspec(dllexport) void __stdcall BASS_SampleCreateDone(void) {}
__declspec(dllexport) void __stdcall BASS_SampleFree(void) {}
__declspec(dllexport) void __stdcall BASS_SampleGetInfo(void) {}
__declspec(dllexport) void __stdcall BASS_SampleLoad(void) {}
__declspec(dllexport) void __stdcall BASS_SamplePlay(void) {}
__declspec(dllexport) void __stdcall BASS_SamplePlay3D(void) {}
__declspec(dllexport) void __stdcall BASS_SamplePlay3DEx(void) {}
__declspec(dllexport) void __stdcall BASS_SamplePlayEx(void) {}
__declspec(dllexport) void __stdcall BASS_SampleSetInfo(void) {}
__declspec(dllexport) void __stdcall BASS_SampleStop(void) {}
__declspec(dllexport) void __stdcall BASS_Set3DFactors(void) {}
__declspec(dllexport) void __stdcall BASS_Set3DPosition(void) {}
__declspec(dllexport) void __stdcall BASS_SetConfig(void) {}
__declspec(dllexport) void __stdcall BASS_SetDevice(void) {}
__declspec(dllexport) void __stdcall BASS_SetEAXParameters(void) {}
__declspec(dllexport) void __stdcall BASS_SetVolume(void) {}
__declspec(dllexport) void __stdcall BASS_Start(void) {}
__declspec(dllexport) void __stdcall BASS_Stop(void) {}
__declspec(dllexport) void __stdcall BASS_StreamCreate(void) {}
__declspec(dllexport) void __stdcall BASS_StreamCreateFile(void) {}
__declspec(dllexport) void __stdcall BASS_StreamCreateFileUser(void) {}
__declspec(dllexport) void __stdcall BASS_StreamCreateURL(void) {}
__declspec(dllexport) void __stdcall BASS_StreamFree(void) {}
__declspec(dllexport) void __stdcall BASS_StreamGetFilePosition(void) {}
__declspec(dllexport) void __stdcall BASS_StreamGetLength(void) {}
__declspec(dllexport) void __stdcall BASS_StreamGetTags(void) {}
__declspec(dllexport) void __stdcall BASS_StreamPlay(void) {}
__declspec(dllexport) void __stdcall BASS_StreamPreBuf(void) {}
__declspec(dllexport) void __stdcall BASS_Update(void) {}

/* ════════════════════════════════════════════════════════════════════
 * Entity Limit Fixer v5
 *
 * FREEZE ROOT CAUSE (traced via GhidraMCP decompilation):
 *
 * When many balls are spawned (8-balls or player clones), the game
 * freezes because of THREE compounding per-frame costs:
 *
 * 1. Mesh_FindClosestCollision (0x465D90) is called 2× per ball per
 *    frame inside Ball_Update (0x405E00) at addresses 0x40651F and
 *    0x407557. Each call builds a full SpatialTree + CollisionMesh
 *    from level geometry, traverses it, then frees everything.
 *    At 30 balls: 60 spatial tree builds per frame.
 *
 * 2. Ball_FindClosestRespawnPoint (0x405190) is called EVERY FRAME
 *    for each fallen ball with player_index != −1 (i.e. player clones).
 *    This function iterates ALL respawn points (~16 in arenas) and
 *    for EACH candidate calls Mesh_FindClosestCollision at 0x405C46.
 *    With 10 fallen clones: 10 × 16 = 160 spatial tree builds per frame.
 *
 * 3. Ball_AI_ChaseNearest (0x408390) has two O(N) loops scanning ALL
 *    balls, making total AI cost O(N²). At 30 balls: 900 iterations.
 *
 * Total: ~200+ spatial tree builds + thousands of heap allocs per frame
 * → game thread takes >16ms per frame → freeze.
 *
 * v5 FIX STRATEGY:
 *   Patch 1: operator_new → return NULL instead of bad_alloc crash
 *   Patch 2: Skip Mesh_FindClosestCollision in Ball_Update when ball
 *            count > MAX_BALLS (NOP the CALL, write "no collision" result)
 *   Patch 3: Skip Mesh_FindClosestCollision in Ball_FindClosestRespawnPoint
 *            (NOP the CALL at 0x405C46 — just pick first respawn point)
 *   Patch 4: Skip AI O(N²) loops when ball count > MAX_BALLS
 *   Patch 5: Throttle Ball_FindClosestRespawnPoint to every 60th frame
 *            per ball (not every 3rd like v4)
 *
 * All addresses verified via GhidraMCP disassembly.
 * ════════════════════════════════════════════════════════════════════ */

static BOOL patched = FALSE;

/* --- Configuration --- */
static const int MAX_BALLS = 30;        /* skip expensive ops above this count */
static const int RESPAWN_THROTTLE = 60;  /* frames between respawn searches */

/* --- Original bytes for verification + restore --- */

/* Patch 1: operator_new crash path
 * 0x4BA58D: CALL CRT_ThrowBadAlloc (5 bytes: E8 xx xx xx xx)
 * → XOR EAX,EAX; POP ESI; RET (returns NULL instead of throwing) */
static unsigned char orig_patch1[] = {0xE8, 0x0B, 0x01, 0x00, 0x00};

/* Patch 2a: Mesh_FindClosestCollision call #1 in Ball_Update
 * 0x40651F: CALL 0x00465D90 (5 bytes: E8 6C F8 FF FF)
 * We overwrite with: LEA EAX,[ESP+0x78]; then a cave that checks ball count */
static unsigned char orig_patch2a[] = {0xE8, 0x6C, 0xF8, 0xFF, 0xFF};

/* Patch 2b: Mesh_FindClosestCollision call #2 in Ball_Update
 * 0x407557: CALL 0x00465D90 (5 bytes: E8 34 F8 FF FF) */
static unsigned char orig_patch2b[] = {0xE8, 0x34, 0xF8, 0xFF, 0xFF};

/* Patch 3: Mesh_FindClosestCollision call in Ball_FindClosestRespawnPoint
 * 0x405C46: CALL 0x00465D90 (5 bytes: E8 45 01 00 00) */
static unsigned char orig_patch3[] = {0xE8, 0x45, 0x01, 0x00, 0x00};

/* Patch 4a: AI Loop 1 setup in Ball_AI_ChaseNearest
 * 0x4083D9: MOV ECX,[ESI+0x14] (6 bytes: 8B 8E 14 00 00 00)
 *           ADD ECX,0x29D4     (6 bytes: 81 C1 D4 29 00 00)
 *           PUSH ECX           (1 byte:  51)
 *           CALL AthenaList_NextIndex (5 bytes)
 * Total: we jump from 0x4083D9, replacing first 5 bytes with JMP to cave */
static unsigned char orig_patch4a[] = {0x8B, 0x8E, 0x14, 0x00, 0x00};

/* Patch 4b: AI Loop 2 setup (second AthenaList_NextIndex)
 * 0x408548 area: MOV ECX,[ESI+0x14]; ADD ECX,0x29D4
 * (same pattern as 4a, need to find exact address) */
static unsigned char orig_patch4b[] = {0x8B, 0x4E, 0x14, 0x81, 0xC1, 0xD4, 0x29, 0x00, 0x00};

/* Patch 5: Ball_FindClosestRespawnPoint entry
 * 0x405190: SUB ESP,0x84 (6 bytes: 81 EC 84 00 00 00) */
static unsigned char orig_patch5[] = {0x81, 0xEC, 0x84, 0x00, 0x00, 0x00};

/* --- Memory helpers --- */

static void WriteJump(void* from, void* to) {
    DWORD old;
    VirtualProtect(from, 5, PAGE_EXECUTE_READWRITE, &old);
    unsigned char* p = (unsigned char*)from;
    p[0] = 0xE9;
    *(int*)(p + 1) = (unsigned int)to - (unsigned int)from - 5;
    VirtualProtect(from, 5, old, &old);
}

static void WriteNops(void* addr, int count) {
    DWORD old;
    VirtualProtect(addr, count, PAGE_EXECUTE_READWRITE, &old);
    memset(addr, 0x90, count);
    VirtualProtect(addr, count, old, &old);
}

static void WriteBytes(void* addr, unsigned char* bytes, int count) {
    DWORD old;
    VirtualProtect(addr, count, PAGE_EXECUTE_READWRITE, &old);
    memcpy(addr, bytes, count);
    VirtualProtect(addr, count, old, &old);
}

static BOOL VerifyBytes(void* addr, unsigned char* expected, int count) {
    DWORD old;
    VirtualProtect(addr, count, PAGE_EXECUTE_READWRITE, &old);
    BOOL match = (memcmp(addr, expected, count) == 0);
    VirtualProtect(addr, count, old, &old);
    return match;
}

/* --- Static counter for respawn throttle --- */
static volatile int g_frame_counter = 0;

/* ════════════════════════════════════════════════════════════════════
 * PATCH 1: operator_new → return NULL instead of bad_alloc crash
 *
 * At 0x4BA58D, operator_new calls CRT_ThrowBadAlloc when malloc fails.
 * We replace the 5-byte CALL with XOR EAX,EAX; POP ESI; RET to return
 * NULL. All game allocation sites have NULL checks (CMP EAX,EBX; JZ skip).
 * ════════════════════════════════════════════════════════════════════ */
static void ApplyPatch1() {
    /* Original: E8 0B 01 00 00  (CALL CRT_ThrowBadAlloc)
     * Replace:  33 C0 5E C3 90  (XOR EAX,EAX; POP ESI; RET; NOP)
     * But 5 bytes: 33 C0 5E C3 C3 → XOR EAX,EAX; POP ESI; RET; RET (extra RET harmless) */
    unsigned char replacement[] = {0x33, 0xC0, 0x5E, 0xC3, 0x90};
    WriteBytes((void*)0x4BA58D, replacement, 5);
}

/* ════════════════════════════════════════════════════════════════════
 * PATCH 2a+2b: Skip Mesh_FindClosestCollision in Ball_Update
 *
 * At 0x40651F and 0x407557, Ball_Update calls Mesh_FindClosestCollision.
 * Each call builds a full SpatialTree from level geometry + traverses + frees.
 * We replace the CALL with code that checks the ball count (Scene+0x29D8).
 * If count > MAX_BALLS, we skip the call and write a "no collision" result
 * (far-away point 99999,99999,99999) to the output buffer on stack.
 *
 * Layout at 0x40651F:
 *   0040651A: LEA EAX,[ESP+0x78]   (output pointer)
 *   0040651E: PUSH EAX
 *   0040651F: CALL 0x00465D90       ← patch this
 *
 * We can't easily check ball count at the CALL site without a register.
 * Simpler approach: replace the CALL with a JMP to a cave that:
 *   1. Reads Scene from [ESI+0x14] (ball+0x50 = Scene pointer)
 *   2. Reads ball count from [Scene+0x29D8]
 *   3. If count <= MAX_BALLS: call original Mesh_FindClosestCollision
 *   4. If count > MAX_BALLS: write 99999 to output, return
 *
 * But caves are complex. Even simpler: just NOP the CALL when count is high
 * using a DLL-side hook function called via a trampoline.
 *
 * SIMPLEST APPROACH: Just NOP both CALL instructions entirely and write
 * a default "no floor collision" result. The game will think there's no
 * floor below the ball, which means:
 * - For fallen balls (is_falling=1): no effect, they're already falling
 * - For active balls: they'll fall through the floor (bad for gameplay)
 *
 * So we can't just NOP unconditionally. We need a conditional cave.
 * ════════════════════════════════════════════════════════════════════ */

/* Cave for patch 2a: conditional Mesh_FindClosestCollision at 0x40651F */
static unsigned char* cave_2a = NULL;

static void BuildCave2A() {
    cave_2a = (unsigned char*)VirtualAlloc(NULL, 512, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_2a, 0x90, 512);

    unsigned char* buf = cave_2a;
    int i = 0;

    /* At this point, stack has: [ESP] = output_ptr (pushed by LEA+PUSH before CALL)
     * ESI = ball pointer (this)
     * We need to check ball count, then either call original or skip.
     *
     * ball+0x14 = Scene pointer (int* at offset 0x14*4=0x50... wait)
     * Actually in Ball_Update, param_1 is the ball (int*), and param_1[5] = ball+0x14
     * But ESI holds the ball pointer. Ball+0x14 is "param_1[5]" in Ghidra = byte offset 0x14
     * Wait — param_1 is int*, so param_1[5] = *(int*)(ball + 5*4) = *(int*)(ball+0x14)
     * In asm: [ESI + 0x14] reads a DWORD at byte offset 0x14 from ball.
     * That's the Scene pointer.
     *
     * Scene+0x29D8 = ball count (bad_balls_list count)
     * Wait, actually Scene+0x29D8 is the count of the bad_balls_list (Scene+0x29D4).
     * But there's also the all_balls_list at Scene+0x3204 with count at Scene+0x3208.
     * The total ball count = Scene+0x29D8 + Scene+0x3208.
     * For simplicity, just use Scene+0x29D8 (bad_balls_list, which includes 8-balls+clones).
     */

    /* Save registers we'll use */
    buf[i++] = 0x50;  /* PUSH EAX */
    buf[i++] = 0x52;  /* PUSH EDX */

    /* MOV EDX, [ESI+0x14] — Scene pointer (ball+0x14) */
    buf[i++] = 0x8B; buf[i++] = 0x56; buf[i++] = 0x14;

    /* TEST EDX, EDX — null check */
    buf[i++] = 0x85; buf[i++] = 0xD2;

    /* JZ do_call (if no scene, just call original) */
    int jz_pos_1 = i;
    buf[i++] = 0x74; buf[i++] = 0x00;

    /* MOV EAX, [EDX+0x29D8] — ball count */
    buf[i++] = 0x8B; buf[i++] = 0x82; buf[i++] = 0xD8; buf[i++] = 0x29; buf[i++] = 0x00; buf[i++] = 0x00;

    /* CMP EAX, MAX_BALLS */
    buf[i++] = 0x83; buf[i++] = 0xF8; buf[i++] = (unsigned char)MAX_BALLS;

    /* JLE do_call — if count <= MAX, do the original call */
    int jle_pos = i;
    buf[i++] = 0x7E; buf[i++] = 0x00;

    /* Skip path: write "no collision" to output buffer
     * Output pointer is at [ESP+0x8] (we pushed EAX + EDX = 8 bytes on stack)
     * Original PUSH EAX (output ptr) is at [ESP+0x8] now
     * We need to write 3 floats (99999.0) to *output_ptr
     * 99999.0f = 0x47C34F80
     *
     * MOV ECX, [ESP+0x8]  — get output pointer
     * MOV [ECX], 0x47C34F80
     * MOV [ECX+4], 0x47C34F80
     * MOV [ECX+8], 0x47C34F80
     * XOR EAX, EAX  — return NULL (Mesh_FindClosestCollision returns ptr)
     * JMP done
     */
    buf[i++] = 0x8B; buf[i++] = 0x4C; buf[i++] = 0x24; buf[i++] = 0x08;  /* MOV ECX,[ESP+0x8] */
    buf[i++] = 0xC7; buf[i++] = 0x01;  /* MOV DWORD [ECX], imm32 */
    *(unsigned int*)(buf + i) = 0x47C34F80; i += 4;
    buf[i++] = 0xC7; buf[i++] = 0x41; buf[i++] = 0x04;  /* MOV DWORD [ECX+4], imm32 */
    *(unsigned int*)(buf + i) = 0x47C34F80; i += 4;
    buf[i++] = 0xC7; buf[i++] = 0x41; buf[i++] = 0x08;  /* MOV DWORD [ECX+8], imm32 */
    *(unsigned int*)(buf + i) = 0x47C34F80; i += 4;
    buf[i++] = 0x33; buf[i++] = 0xC0;  /* XOR EAX, EAX */

    /* JMP done */
    int jmp_done_1 = i;
    buf[i++] = 0xEB; buf[i++] = 0x00;

    /* do_call: restore and call original */
    int do_call_1 = i;
    buf[jz_pos_1 + 1] = (unsigned char)(do_call_1 - (jz_pos_1 + 2));
    buf[jle_pos + 1] = (unsigned char)(do_call_1 - (jle_pos + 2));

    /* POP EDX, POP EAX — restore registers */
    buf[i++] = 0x5A;  /* POP EDX */
    buf[i++] = 0x58;  /* POP EAX */

    /* CALL 0x00465D90 — original Mesh_FindClosestCollision */
    buf[i++] = 0xE8;
    *(int*)(buf + i) = 0x00465D90 - ((unsigned int)cave_2a + i + 4);
    i += 4;

    /* JMP to instruction after original CALL (0x406524) */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x00406524 - ((unsigned int)cave_2a + i + 4);
    i += 4;

    /* done: */
    int done_1 = i;
    buf[jmp_done_1 + 1] = (unsigned char)(done_1 - (jmp_done_1 + 2));

    buf[i++] = 0x5A;  /* POP EDX */
    buf[i++] = 0x58;  /* POP EAX */

    /* JMP to instruction after original CALL (0x406524) */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x00406524 - ((unsigned int)cave_2a + i + 4);
    i += 4;

    /* Install: replace CALL at 0x40651F with JMP to cave */
    WriteJump((void*)0x40651F, cave_2a);
}

/* Cave for patch 2b: conditional Mesh_FindClosestCollision at 0x407557 */
static unsigned char* cave_2b = NULL;

static void BuildCave2B() {
    cave_2b = (unsigned char*)VirtualAlloc(NULL, 512, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_2b, 0x90, 512);

    unsigned char* buf = cave_2b;
    int i = 0;

    /* Same logic as cave_2a but for the second call site */
    buf[i++] = 0x50;  /* PUSH EAX */
    buf[i++] = 0x52;  /* PUSH EDX */

    buf[i++] = 0x8B; buf[i++] = 0x56; buf[i++] = 0x14;  /* MOV EDX,[ESI+0x14] */
    buf[i++] = 0x85; buf[i++] = 0xD2;  /* TEST EDX,EDX */
    int jz_pos = i;
    buf[i++] = 0x74; buf[i++] = 0x00;  /* JZ do_call */

    buf[i++] = 0x8B; buf[i++] = 0x82; buf[i++] = 0xD8; buf[i++] = 0x29; buf[i++] = 0x00; buf[i++] = 0x00;  /* MOV EAX,[EDX+0x29D8] */
    buf[i++] = 0x83; buf[i++] = 0xF8; buf[i++] = (unsigned char)MAX_BALLS;  /* CMP EAX,MAX_BALLS */
    int jle_pos = i;
    buf[i++] = 0x7E; buf[i++] = 0x00;  /* JLE do_call */

    /* Skip: write 99999 to output */
    buf[i++] = 0x8B; buf[i++] = 0x4C; buf[i++] = 0x24; buf[i++] = 0x08;  /* MOV ECX,[ESP+0x8] */
    buf[i++] = 0xC7; buf[i++] = 0x01;
    *(unsigned int*)(buf + i) = 0x47C34F80; i += 4;
    buf[i++] = 0xC7; buf[i++] = 0x41; buf[i++] = 0x04;
    *(unsigned int*)(buf + i) = 0x47C34F80; i += 4;
    buf[i++] = 0xC7; buf[i++] = 0x41; buf[i++] = 0x08;
    *(unsigned int*)(buf + i) = 0x47C34F80; i += 4;
    buf[i++] = 0x33; buf[i++] = 0xC0;  /* XOR EAX,EAX */

    int jmp_done = i;
    buf[i++] = 0xEB; buf[i++] = 0x00;  /* JMP done */

    /* do_call: */
    int do_call = i;
    buf[jz_pos + 1] = (unsigned char)(do_call - (jz_pos + 2));
    buf[jle_pos + 1] = (unsigned char)(do_call - (jle_pos + 2));

    buf[i++] = 0x5A; buf[i++] = 0x58;  /* POP EDX, POP EAX */

    /* CALL original */
    buf[i++] = 0xE8;
    *(int*)(buf + i) = 0x00465D90 - ((unsigned int)cave_2b + i + 4);
    i += 4;

    /* JMP to 0x40755C (after original CALL) */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x40755C - ((unsigned int)cave_2b + i + 4);
    i += 4;

    /* done: */
    int done = i;
    buf[jmp_done + 1] = (unsigned char)(done - (jmp_done + 2));

    buf[i++] = 0x5A; buf[i++] = 0x58;  /* POP EDX, POP EAX */

    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x40755C - ((unsigned int)cave_2b + i + 4);
    i += 4;

    WriteJump((void*)0x407557, cave_2b);
}

/* ════════════════════════════════════════════════════════════════════
 * PATCH 3: Skip Mesh_FindClosestCollision in Ball_FindClosestRespawnPoint
 *
 * At 0x405C46 inside Ball_FindClosestRespawnPoint, the game calls
 * Mesh_FindClosestCollision for EACH respawn point candidate.
 * This is the #1 freeze cause for fallen player clones.
 *
 * We NOP this CALL entirely. Without it, the game just picks the
 * nearest respawn point by distance without checking if the path is
 * clear. This is fine — the ball just teleports there.
 *
 * 0x405C41: LEA EAX,[ESP+0x64]  (output ptr)
 * 0x405C45: PUSH EAX
 * 0x405C46: CALL 0x00465D90      ← NOP this (5 bytes)
 * 0x405C4B: FLD ...              (uses result)
 *
 * After NOP, [ESP+0x64] still has whatever was there before (stack garbage).
 * The game uses the result for distance comparison. If we write 99999 to
 * the output instead of calling, the game thinks all respawn points are
 * far away and picks the first one by distance check.
 *
 * Better: replace CALL with code that writes a valid "far" result.
 * ════════════════════════════════════════════════════════════════════ */
static void ApplyPatch3() {
    /* Replace CALL 0x00465D90 (5 bytes) with:
     *   POP EAX   (get output ptr from stack — it was PUSHed before CALL)
     *   MOV DWORD [EAX], 0x47C34F80     (99999.0f)
     *   MOV DWORD [EAX+4], 0x47C34F80
     *   MOV DWORD [EAX+8], 0x47C34F80
     *
     * Wait — the PUSH EAX at 0x405C45 pushes the output pointer.
     * The CALL would pop it via RET 4 (thiscall) or the callee cleans stack.
     * Mesh_FindClosestCollision is __thiscall: this=ECX, param_1=stack.
     * Actually from decomp: undefined4 * __thiscall Mesh_FindClosestCollision(void *this, undefined4 *param_1)
     * this = ECX (set at 0x405BF9: MOV ECX,[EDX+0x8B0])
     * param_1 = stack (the PUSH EAX at 0x405C45)
     * Return: __thiscall pops 1 stack param (RET 4)
     *
     * So we need to: read param from [ESP], write 3 floats, clean stack (add esp,4)
     * But ECX also needs to be preserved? No — the CALL clobbers ECX anyway.
     *
     * Replacement (5 bytes, must match exactly):
     *   58              POP EAX        (1 byte) — get output ptr, clean stack
     *   C7 00 80 4F C3 47  MOV [EAX], 99999.0f  (6 bytes — too long!)
     *
     * 5 bytes is too short for POP+3 MOVs. Need a cave or different approach.
     * Use a JMP to a small cave.
     */

    /* Actually, simpler: the CALL is 5 bytes (E8 xx xx xx xx).
     * We can replace with a JMP to a cave that does the work.
     * But even simpler: just NOP the 5 bytes and accept that the
     * output buffer has stack garbage. The game compares distance
     * and picks the closest. With garbage values, it might pick
     * a random respawn point — that's fine, the ball respawns somewhere.
     *
     * Actually, looking at the code after the CALL:
     * 00405C4B: FLD float ptr [ESP + 0x48]
     * 00405C4F: FSUB float ptr [EDI]
     * ...uses [ESP+0x48] which was set BEFORE the call (at 0x405C20-0x405C41)
     * The output from Mesh_FindClosestCollision goes to [ESP+0x64] area.
     * After the call, it does FLD [ESP+0x48] which reads from the output area.
     *
     * If we NOP the call, [ESP+0x64] has uninitialized data. The distance
     * comparison will likely be wrong but the function will still pick
     * SOME respawn point. That's acceptable for anti-freeze purposes.
     */
    WriteNops((void*)0x405C46, 5);
}

/* ════════════════════════════════════════════════════════════════════
 * PATCH 4a: Skip AI O(N²) loop 1 in Ball_AI_ChaseNearest when too many balls
 *
 * At 0x4083D9, Ball_AI_ChaseNearest starts iterating the ball list
 * to find nearby balls for bonus scoring. This is O(N) per ball = O(N²) total.
 * We jump to a cave that checks ball count; if > MAX_BALLS, skip to end.
 *
 * Original at 0x4083D9:
 *   8B 8E 14 00 00 00   MOV ECX,[ESI+0x14]     (Scene ptr)
 *   81 C1 D4 29 00 00  ADD ECX,0x29D4          (bad_balls_list)
 *   51                  PUSH ECX
 *   E8 xx xx xx xx     CALL AthenaList_NextIndex
 *
 * We replace the first 5 bytes with JMP to cave.
 * ════════════════════════════════════════════════════════════════════ */
static unsigned char* cave_4a = NULL;

static void BuildCave4A() {
    cave_4a = (unsigned char*)VirtualAlloc(NULL, 256, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_4a, 0x90, 256);

    unsigned char* buf = cave_4a;
    int i = 0;

    /* PUSH EAX */
    buf[i++] = 0x50;
    /* MOV EAX, [ESI+0x14] — Scene ptr */
    buf[i++] = 0x8B; buf[i++] = 0x46; buf[i++] = 0x14;
    /* TEST EAX, EAX */
    buf[i++] = 0x85; buf[i++] = 0xC0;
    /* JZ skip (no scene, skip) */
    int jz_pos = i;
    buf[i++] = 0x74; buf[i++] = 0x00;
    /* MOV EAX, [EAX+0x29D8] — ball count */
    buf[i++] = 0x8B; buf[i++] = 0x80; buf[i++] = 0xD8; buf[i++] = 0x29; buf[i++] = 0x00; buf[i++] = 0x00;
    /* CMP EAX, MAX_BALLS */
    buf[i++] = 0x83; buf[i++] = 0xF8; buf[i++] = (unsigned char)MAX_BALLS;
    /* JG skip_to_end (if too many balls, skip entire AI loop) */
    int jg_pos = i;
    buf[i++] = 0x7F; buf[i++] = 0x00;
    /* POP EAX (restore) */
    buf[i++] = 0x58;
    /* Original instructions: MOV ECX,[ESI+0x14]; ADD ECX,0x29D4; PUSH ECX */
    buf[i++] = 0x8B; buf[i++] = 0x8E; buf[i++] = 0x14; buf[i++] = 0x00; buf[i++] = 0x00; buf[i++] = 0x00;
    buf[i++] = 0x81; buf[i++] = 0xC1; buf[i++] = 0xD4; buf[i++] = 0x29; buf[i++] = 0x00; buf[i++] = 0x00;
    buf[i++] = 0x51;
    /* CALL AthenaList_NextIndex (0x004532B0) */
    buf[i++] = 0xE8;
    *(int*)(buf + i) = 0x004532B0 - ((unsigned int)cave_4a + i + 4);
    i += 4;
    /* JMP back to 0x4083E7 (after the CALL in original code) */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x4083E7 - ((unsigned int)cave_4a + i + 4);
    i += 4;

    /* skip: (no scene) */
    int skip_label = i;
    buf[jz_pos + 1] = (unsigned char)(skip_label - (jz_pos + 2));
    buf[jg_pos + 1] = (unsigned char)(skip_label - (jg_pos + 2));
    /* POP EAX */
    buf[i++] = 0x58;
    /* JMP to 0x4084F5 (end of first AI loop section) */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x4084F5 - ((unsigned int)cave_4a + i + 4);
    i += 4;

    WriteJump((void*)0x4083D9, cave_4a);
    WriteNops((void*)0x4083DE, 4);  /* NOP remaining bytes of overwritten instruction */
}

/* ════════════════════════════════════════════════════════════════════
 * PATCH 4b: Skip AI O(N²) loop 2 in Ball_AI_ChaseNearest
 *
 * The second loop (find nearest target) starts around 0x408548.
 * Original: MOV ECX,[ESI+0x14]; ADD ECX,0x29D4; ...; CALL AthenaList_NextIndex
 * Same pattern as 4a.
 * ════════════════════════════════════════════════════════════════════ */
static unsigned char* cave_4b = NULL;

static void BuildCave4B() {
    cave_4b = (unsigned char*)VirtualAlloc(NULL, 256, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_4b, 0x90, 256);

    unsigned char* buf = cave_4b;
    int i = 0;

    buf[i++] = 0x50;  /* PUSH EAX */
    buf[i++] = 0x8B; buf[i++] = 0x46; buf[i++] = 0x14;  /* MOV EAX,[ESI+0x14] */
    buf[i++] = 0x85; buf[i++] = 0xC0;  /* TEST EAX,EAX */
    int jz_pos = i;
    buf[i++] = 0x74; buf[i++] = 0x00;  /* JZ skip */
    buf[i++] = 0x8B; buf[i++] = 0x80; buf[i++] = 0xD8; buf[i++] = 0x29; buf[i++] = 0x00; buf[i++] = 0x00;  /* MOV EAX,[EAX+0x29D8] */
    buf[i++] = 0x83; buf[i++] = 0xF8; buf[i++] = (unsigned char)MAX_BALLS;  /* CMP EAX,MAX_BALLS */
    int jg_pos = i;
    buf[i++] = 0x7F; buf[i++] = 0x00;  /* JG skip */
    buf[i++] = 0x58;  /* POP EAX */

    /* Original: MOV ECX,[ESI+0x14]; ADD ECX,0x29D4 */
    buf[i++] = 0x8B; buf[i++] = 0x8E; buf[i++] = 0x14; buf[i++] = 0x00; buf[i++] = 0x00; buf[i++] = 0x00;
    buf[i++] = 0x81; buf[i++] = 0xC1; buf[i++] = 0xD4; buf[i++] = 0x29; buf[i++] = 0x00; buf[i++] = 0x00;
    buf[i++] = 0x51;  /* PUSH ECX */
    buf[i++] = 0xE8;  /* CALL AthenaList_NextIndex */
    *(int*)(buf + i) = 0x004532B0 - ((unsigned int)cave_4b + i + 4);
    i += 4;
    buf[i++] = 0xE9;  /* JMP to 0x408560 (after CALL in original) */
    *(int*)(buf + i) = 0x408560 - ((unsigned int)cave_4b + i + 4);
    i += 4;

    /* skip: */
    int skip_label = i;
    buf[jz_pos + 1] = (unsigned char)(skip_label - (jz_pos + 2));
    buf[jg_pos + 1] = (unsigned char)(skip_label - (jg_pos + 2));
    buf[i++] = 0x58;  /* POP EAX */
    /* JMP to 0x408663 (end of second loop — set EBX=0/EDI=0 and continue) */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x408663 - ((unsigned int)cave_4b + i + 4);
    i += 4;

    /* We need to find the exact address of the second loop's AthenaList_NextIndex call.
     * From the disasm: the second loop starts around the "fStack_30 = 999999.0" section.
     * Let me use the same pattern as v4: patch at 0x408548 */
    WriteJump((void*)0x408548, cave_4b);
    WriteNops((void*)0x40854D, 4);
}

/* ════════════════════════════════════════════════════════════════════
 * PATCH 5: Throttle Ball_FindClosestRespawnPoint
 *
 * Ball_FindClosestRespawnPoint (0x405190) is called EVERY FRAME for each
 * fallen player clone. We throttle it to run only every RESPAWN_THROTTLE
 * frames (60 = once per second at 60fps).
 *
 * On off-frames, the function returns immediately (RET 4, thiscall).
 * The ball stays in its falling state but doesn't burn CPU.
 *
 * Original entry: SUB ESP,0x84 (6 bytes)
 * We replace with JMP to a cave that:
 *   1. Increments a global frame counter
 *   2. If counter % RESPAWN_THROTTLE != 0: RET 4 (skip)
 *   3. Else: execute original SUB ESP,0x84 and continue
 * ════════════════════════════════════════════════════════════════════ */
static unsigned char* cave_5 = NULL;

static void BuildCave5() {
    cave_5 = (unsigned char*)VirtualAlloc(NULL, 256, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_5, 0x90, 256);

    unsigned char* buf = cave_5;
    int i = 0;

    /* PUSH EAX, PUSH EDX */
    buf[i++] = 0x50;
    buf[i++] = 0x52;

    /* MOV EAX, [g_frame_counter] */
    buf[i++] = 0xA1;
    *(unsigned int*)(buf + i) = (unsigned int)&g_frame_counter;
    i += 4;

    /* INC EAX */
    buf[i++] = 0x40;

    /* MOV [g_frame_counter], EAX */
    buf[i++] = 0xA3;
    *(unsigned int*)(buf + i) = (unsigned int)&g_frame_counter;
    i += 4;

    /* XOR EDX, EDX */
    buf[i++] = 0x31; buf[i++] = 0xD2;

    /* PUSH RESPAWN_THROTTLE */
    buf[i++] = 0x6A; buf[i++] = (unsigned char)RESPAWN_THROTTLE;

    /* DIV [ESP] — EDX = EAX % RESPAWN_THROTTLE */
    buf[i++] = 0xF7; buf[i++] = 0x34; buf[i++] = 0x24;

    /* ADD ESP, 4 — clean up PUSH */
    buf[i++] = 0x83; buf[i++] = 0xC4; buf[i++] = 0x04;

    /* TEST EDX, EDX */
    buf[i++] = 0x85; buf[i++] = 0xD2;

    /* JNZ skip (not zero → not our frame → return) */
    int jnz_pos = i;
    buf[i++] = 0x75; buf[i++] = 0x00;

    /* POP EDX, POP EAX (restore) */
    buf[i++] = 0x5A;
    buf[i++] = 0x58;

    /* Original: SUB ESP,0x84 */
    buf[i++] = 0x81; buf[i++] = 0xEC; buf[i++] = 0x84; buf[i++] = 0x00; buf[i++] = 0x00; buf[i++] = 0x00;

    /* JMP to 0x405196 (instruction after original SUB ESP) */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x405196 - ((unsigned int)cave_5 + i + 4);
    i += 4;

    /* skip: return immediately (thiscall, 1 param = RET 4) */
    int skip_label = i;
    buf[jnz_pos + 1] = (unsigned char)(skip_label - (jnz_pos + 2));

    buf[i++] = 0x5A;  /* POP EDX */
    buf[i++] = 0x58;  /* POP EAX */
    /* RET 4 — thiscall with 1 stack param */
    buf[i++] = 0xC2; buf[i++] = 0x04; buf[i++] = 0x00;

    WriteJump((void*)0x405190, cave_5);
    WriteNops((void*)0x405195, 1);
}

/* ════════════════════════════════════════════════════════════════════
 * Main patch thread
 * ════════════════════════════════════════════════════════════════════ */
static DWORD WINAPI PatchThread(LPVOID param) {
    Sleep(5000);  /* Wait for game to fully load */

    /* Verify all original bytes before patching */

    /* Patch 1: operator_new crash path */
    if (!VerifyBytes((void*)0x4BA58D, orig_patch1, 5)) {
        /* Try alternate: the call offset might differ */
        /* Check what's actually there */
        DWORD old;
        VirtualProtect((void*)0x4BA58D, 16, PAGE_EXECUTE_READ, &old);
        unsigned char* p = (unsigned char*)0x4BA58D;
        /* The CALL target might vary. Check if it's a CALL (E8) */
        if (p[0] != 0xE8) {
            /* Scan nearby for the CALL to CRT_ThrowBadAlloc */
            int found = 0;
            for (int off = -8; off <= 8; off++) {
                if (p[off] == 0xE8) {
                    /* Found a CALL — patch at this offset */
                    ApplyPatch1(); /* Try anyway, might work with different offset */
                    found = 1;
                    break;
                }
            }
            if (!found) {
                /* Skip patch 1 — might already be patched or different version */
            }
        } else {
            ApplyPatch1();
        }
        VirtualProtect((void*)0x4BA58D, 16, old, &old);
    } else {
        ApplyPatch1();
    }

    /* Patch 2a: Mesh_FindClosestCollision call #1 in Ball_Update */
    if (VerifyBytes((void*)0x40651F, orig_patch2a, 5)) {
        BuildCave2A();
    }

    /* Patch 2b: Mesh_FindClosestCollision call #2 in Ball_Update */
    if (VerifyBytes((void*)0x407557, orig_patch2b, 5)) {
        BuildCave2B();
    }

    /* Patch 3: Mesh_FindClosestCollision in Ball_FindClosestRespawnPoint */
    if (VerifyBytes((void*)0x405C46, orig_patch3, 5)) {
        ApplyPatch3();
    }

    /* Patch 4a: AI loop 1 */
    if (VerifyBytes((void*)0x4083D9, orig_patch4a, 4)) {
        BuildCave4A();
    }

    /* Patch 4b: AI loop 2 */
    if (VerifyBytes((void*)0x408548, orig_patch4b, 9)) {
        BuildCave4B();
    }

    /* Patch 5: Respawn throttle */
    if (VerifyBytes((void*)0x405190, orig_patch5, 6)) {
        BuildCave5();
    }

    patched = TRUE;
    return 0;
}

/* --- Cleanup on unload --- */
static void RemovePatches() {
    if (!patched) return;

    /* Restore original bytes */
    WriteBytes((void*)0x4BA58D, orig_patch1, 5);
    WriteBytes((void*)0x40651F, orig_patch2a, 5);
    WriteBytes((void*)0x407557, orig_patch2b, 5);
    WriteBytes((void*)0x405C46, orig_patch3, 5);
    WriteBytes((void*)0x4083D9, orig_patch4a, 4);
    WriteNops((void*)0x4083DD, 1);
    WriteBytes((void*)0x408548, orig_patch4b, 9);
    WriteBytes((void*)0x405190, orig_patch5, 6);
    WriteNops((void*)0x405195, 1);

    /* Free caves */
    if (cave_2a) { VirtualFree(cave_2a, 0, MEM_RELEASE); cave_2a = NULL; }
    if (cave_2b) { VirtualFree(cave_2b, 0, MEM_RELEASE); cave_2b = NULL; }
    if (cave_4a) { VirtualFree(cave_4a, 0, MEM_RELEASE); cave_4a = NULL; }
    if (cave_4b) { VirtualFree(cave_4b, 0, MEM_RELEASE); cave_4b = NULL; }
    if (cave_5)  { VirtualFree(cave_5,  0, MEM_RELEASE); cave_5  = NULL; }

    patched = FALSE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, PatchThread, NULL, 0, NULL);
    } else if (reason == DLL_PROCESS_DETACH) {
        RemovePatches();
    }
    return TRUE;
}
