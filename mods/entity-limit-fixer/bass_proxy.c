
// ============================================================
// Hamsterball Entity Limit Fixer - bass.dll proxy v8
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
__declspec(dllexport) void __stdcall BASS_GetConfig(void) {}
__declspec(dllexport) void __stdcall BASS_GetDSBufferObject(void) {}
__declspec(dllexport) void __stdcall BASS_GetDeviceInfo(void) {}
__declspec(dllexport) void __stdcall BASS_GetEAXParameters(void) {}
__declspec(dllexport) void __stdcall BASS_GetInfo(void) {}
__declspec(dllexport) void __stdcall BASS_GetVersion(void) {}
__declspec(dllexport) void __stdcall BASS_GetVolume(void) {}
__declspec(dllexport) void __stdcall BASS_Init(void) {}
__declspec(dllexport) void __stdcall BASS_MusicFree(void) {}
__declspec(dllexport) void __stdcall BASS_MusicLoad(void) {}
__declspec(dllexport) void __stdcall BASS_Pause(void) {}
__declspec(dllexport) void __stdcall BASS_PluginGetInfo(void) {}
__declspec(dllexport) void __stdcall BASS_SampleCreate(void) {}
__declspec(dllexport) void __stdcall BASS_SampleFree(void) {}
__declspec(dllexport) void __stdcall BASS_SampleGetChannel(void) {}
__declspec(dllexport) void __stdcall BASS_SampleGetData(void) {}
__declspec(dllexport) void __stdcall BASS_SampleGetInfo(void) {}
__declspec(dllexport) void __stdcall BASS_SampleLoad(void) {}
__declspec(dllexport) void __stdcall BASS_SampleSetData(void) {}
__declspec(dllexport) void __stdcall BASS_SampleSetInfo(void) {}
__declspec(dllexport) void __stdcall BASS_SampleStop(void) {}
__declspec(dllexport) void __stdcall BASS_Set3DFactors(void) {}
__declspec(dllexport) void __stdcall BASS_Set3DPosition(void) {}
__declspec(dllexport) void __stdcall BASS_SetConfig(void) {}
__declspec(dllexport) void __stdcall BASS_SetDSBufferObject(void) {}
__declspec(dllexport) void __stdcall BASS_SetEAXParameters(void) {}
__declspec(dllexport) void __stdcall BASS_SetVolume(void) {}
__declspec(dllexport) void __stdcall BASS_Start(void) {}
__declspec(dllexport) void __stdcall BASS_Stop(void) {}
__declspec(dllexport) void __stdcall BASS_StreamCreate(void) {}
__declspec(dllexport) void __stdcall BASS_StreamCreateFile(void) {}
__declspec(dllexport) void __stdcall BASS_StreamCreateURL(void) {}
__declspec(dllexport) void __stdcall BASS_StreamFree(void) {}
__declspec(dllexport) void __stdcall BASS_StreamGetFilePosition(void) {}
__declspec(dllexport) void __stdcall BASS_StreamPutData(void) {}
__declspec(dllexport) void __stdcall BASS_StreamPutFileData(void) {}
__declspec(dllexport) void __stdcall BASS_Update(void) {}

/* ════════════════════════════════════════════════════════════════════
 * Entity Limit Fixer v7
 *
 * v7 REDESIGN: Single function-entry hooks instead of per-call-site patches.
 *
 * FREEZE ROOT CAUSE:
 * Mesh_FindClosestCollision (0x465D90) builds a SpatialTree from level
 * geometry EVERY CALL, traverses it, frees it. ~1ms per call.
 * 5 call sites: Ball_Update (2×), RespawnPoint (16× fallen ball),
 * ArenaPhysics (1×), Ball_FindMeshCollision (1×).
 * With 10 balls: ~50+ calls/frame = 50ms+ → freeze.
 *
 * FIX:
 * Patch A: Hook Scene_UpdateBallsAndState entry → set SKIP_COLLISIONS flag
 *   when ball count > MAX_BALLS. Called once per frame.
 * Patch B: Hook Mesh_FindClosestCollision entry → if flag set, return
 *   99999.0f immediately without building SpatialTree. Covers ALL 5
 *   call sites with one patch.
 * Patch C: operator_new → return NULL (crash protection).
 * Patch D/E: Skip AI O(N²) loops when flag set.
 * Patch F (NEW v8): Hook SpatialTree_ctor entry → skip tree build if flag set.
 *   This catches Ball_Update (0x4068BC) and Ball_FallUpdate (0x4088FC)
 *   which call SpatialTree_ctor DIRECTLY, bypassing Mesh_FindClosestCollision.
 *
 * Ball_FindClosestRespawnPoint still runs to completion — it clears
 * event_flag (ball+0x2E8), frees trail, resets collision mesh, teleports
 * ball. Only the Mesh_FindClosestCollision calls inside it are skipped.
 *
 * v5 crash lesson: NEVER skip Ball_FindClosestRespawnPoint entirely.
 * It must run to clear event_flag. Patch B only skips the inner expensive call.
 * ════════════════════════════════════════════════════════════════════ */

/* --- Configuration --- */
static const int MAX_BALLS = 5;        /* skip collision above this count */

/* --- Global flag --- */
static volatile DWORD g_skip_collisions = 0;
static volatile DWORD* g_skip_ptr = &g_skip_collisions;

/* --- Original bytes for verification + restore --- */

/* Patch A: Scene_UpdateBallsAndState entry (0x41B540)
 * 5 bytes: 53 55 56 8B D9
 * PUSH EBX; PUSH EBP; PUSH ESI; MOV EBX,ECX */
static unsigned char orig_scene[] = {0x53, 0x55, 0x56, 0x8B, 0xD9};

/* Patch B: Mesh_FindClosestCollision entry (0x465D90)
 * 6 bytes: 64 A1 00 00 00 00
 * MOV EAX, FS:[0] */
static unsigned char orig_mesh[] = {0x64, 0xA1, 0x00, 0x00, 0x00, 0x00};

/* Patch C: operator_new bad_alloc call (0x4BA58D)
 * 5 bytes: E8 0B 01 00 00
 * CALL CRT_ThrowBadAlloc */
static unsigned char orig_opnew[] = {0xE8, 0x0B, 0x01, 0x00, 0x00};

/* Patch D: AI loop 1 (0x4083D9)
 * 9 bytes: 8B 4E 14 81 C1 D4 29 00 00
 * MOV ECX,[ESI+0x14]; ADD ECX,0x29D4 */
static unsigned char orig_ai1[] = {0x8B, 0x4E, 0x14, 0x81, 0xC1, 0xD4, 0x29, 0x00, 0x00};

/* Patch E: AI loop 2 (0x408548)
 * 6 bytes: 81 C1 D4 29 00 00
 * ADD ECX,0x29D4 */
static unsigned char orig_ai2[] = {0x81, 0xC1, 0xD4, 0x29, 0x00, 0x00};

/* Patch F: SpatialTree_ctor entry (0x463330)
 * 7 bytes: 6A FF 68 48 D1 4C 00
 * PUSH -1; PUSH 0x4CD148 (SEH prolog) */
static unsigned char orig_stree[] = {0x6A, 0xFF, 0x68, 0x48, 0xD1, 0x4C, 0x00};

/* --- Memory helpers --- */

static void WriteBytes(void* addr, const void* data, int len) {
    DWORD old;
    VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &old);
    memcpy(addr, data, len);
    VirtualProtect(addr, len, old, &old);
    FlushInstructionCache(GetCurrentProcess(), addr, len);
}

static void WriteNops(void* addr, int len) {
    static const unsigned char nops[16] = {0x90};
    WriteBytes(addr, nops, len);
}

static int VerifyBytes(void* addr, const unsigned char* expected, int len) {
    return memcmp(addr, expected, len) == 0;
}

static void WriteJump(void* from, void* to) {
    unsigned char jmp[5];
    DWORD old;
    VirtualProtect(from, 5, PAGE_EXECUTE_READWRITE, &old);
    jmp[0] = 0xE9;
    *(unsigned int*)(jmp + 1) = (unsigned int)to - ((unsigned int)from + 5);
    memcpy(from, jmp, 5);
    VirtualProtect(from, 5, old, &old);
    FlushInstructionCache(GetCurrentProcess(), from, 5);
}

/* --- Caves --- */

static unsigned char* cave_scene = NULL;
static unsigned char* cave_mesh = NULL;
static unsigned char* cave_ai1 = NULL;
static unsigned char* cave_ai2 = NULL;
static unsigned char* cave_stree = NULL;

/* Patch A: Hook Scene_UpdateBallsAndState entry (0x41B540)
 * ECX = Scene pointer (thiscall). Ball count at Scene+0x29D8.
 * Sets g_skip_collisions based on ball count vs MAX_BALLS. */
static void BuildCaveScene() {
    cave_scene = (unsigned char*)VirtualAlloc(NULL, 128, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_scene, 0x90, 128);

    unsigned char* buf = cave_scene;
    int i = 0;

    /* Original 5 bytes: PUSH EBX; PUSH EBP; PUSH ESI; MOV EBX,ECX */
    buf[i++] = 0x53;                    /* PUSH EBX */
    buf[i++] = 0x55;                    /* PUSH EBP */
    buf[i++] = 0x56;                    /* PUSH ESI */
    buf[i++] = 0x8B; buf[i++] = 0xD9;  /* MOV EBX,ECX */

    /* MOV EAX, [EBX+0x29D8] — ball count */
    buf[i++] = 0x8B; buf[i++] = 0x83;
    *(unsigned int*)(buf + i) = 0x29D8;
    i += 4;

    /* CMP EAX, MAX_BALLS */
    buf[i++] = 0x3D;                    /* CMP EAX, imm32 */
    *(unsigned int*)(buf + i) = (unsigned int)MAX_BALLS;
    i += 4;

    /* JLE +offset (skip to "clear flag") */
    int jle_pos = i;
    buf[i++] = 0x7E; buf[i++] = 0x00;  /* JLE — fill offset later */

    /* Set flag = 1 */
    buf[i++] = 0xC7; buf[i++] = 0x05;  /* MOV DWORD [imm32], 1 */
    *(unsigned int*)(buf + i) = (unsigned int)&g_skip_collisions;
    i += 4;
    *(unsigned int*)(buf + i) = 1;
    i += 4;

    /* JMP to 0x41B545 (original next instruction: PUSH EDI) */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x41B545 - ((unsigned int)cave_scene + i + 4);
    i += 4;

    /* Clear flag = 0 (JLE target) */
    int clear_pos = i;
    buf[jle_pos + 1] = (unsigned char)(clear_pos - (jle_pos + 2));

    buf[i++] = 0xC7; buf[i++] = 0x05;  /* MOV DWORD [imm32], 0 */
    *(unsigned int*)(buf + i) = (unsigned int)&g_skip_collisions;
    i += 4;
    *(unsigned int*)(buf + i) = 0;
    i += 4;

    /* JMP to 0x41B545 */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x41B545 - ((unsigned int)cave_scene + i + 4);
    i += 4;

    WriteJump((void*)0x41B540, cave_scene);
}

/* Patch B: Hook Mesh_FindClosestCollision entry (0x465D90)
 * __thiscall: ECX = this, RET 0x20 (callee cleans 32B = 8 params).
 * [ESP+4] = output pointer (3 floats).
 * If flag set: write 99999.0f to output, RET 0x20. */
static void BuildCaveMesh() {
    cave_mesh = (unsigned char*)VirtualAlloc(NULL, 128, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_mesh, 0x90, 128);

    unsigned char* buf = cave_mesh;
    int i = 0;

    /* CMP DWORD [g_skip_collisions], 0 */
    buf[i++] = 0x83; buf[i++] = 0x3D;
    *(unsigned int*)(buf + i) = (unsigned int)&g_skip_collisions;
    i += 4;
    buf[i++] = 0x00;

    /* JE to "original" path */
    int je_pos = i;
    buf[i++] = 0x74; buf[i++] = 0x00;  /* JE — fill offset later */

    /* Skip path: write 99999.0f to output */
    /* MOV EAX, [ESP+4] — output pointer */
    buf[i++] = 0x8B; buf[i++] = 0x44; buf[i++] = 0x24; buf[i++] = 0x04;

    /* MOV DWORD [EAX], 47C34F80 (99999.0f) */
    buf[i++] = 0xC7; buf[i++] = 0x00;
    *(unsigned int*)(buf + i) = 0x47C34F80;
    i += 4;

    /* MOV DWORD [EAX+4], 47C34F80 */
    buf[i++] = 0xC7; buf[i++] = 0x40; buf[i++] = 0x04;
    *(unsigned int*)(buf + i) = 0x47C34F80;
    i += 4;

    /* MOV DWORD [EAX+8], 47C34F80 */
    buf[i++] = 0xC7; buf[i++] = 0x40; buf[i++] = 0x08;
    *(unsigned int*)(buf + i) = 0x47C34F80;
    i += 4;

    /* XOR EAX, EAX */
    buf[i++] = 0x33; buf[i++] = 0xC0;

    /* RET 0x20 — callee cleans 32 bytes */
    buf[i++] = 0xC2; buf[i++] = 0x20; buf[i++] = 0x00;

    /* Original path: execute MOV EAX, FS:[0] and JMP to 0x465D96 */
    int orig_pos = i;
    buf[je_pos + 1] = (unsigned char)(orig_pos - (je_pos + 2));

    /* MOV EAX, FS:[0] — original 6 bytes */
    buf[i++] = 0x64; buf[i++] = 0xA1;
    *(unsigned int*)(buf + i) = 0x00000000;
    i += 4;

    /* JMP to 0x465D96 (continue original prolog) */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x465D96 - ((unsigned int)cave_mesh + i + 4);
    i += 4;

    WriteJump((void*)0x465D90, cave_mesh);
    WriteNops((void*)0x465D95, 1);  /* NOP the 6th byte */
}

/* Patch D: Skip AI loop 1 (0x4083D9) when flag set */
static void BuildCaveAI1() {
    cave_ai1 = (unsigned char*)VirtualAlloc(NULL, 64, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_ai1, 0x90, 64);

    unsigned char* buf = cave_ai1;
    int i = 0;

    /* CMP DWORD [g_skip_collisions], 0 */
    buf[i++] = 0x83; buf[i++] = 0x3D;
    *(unsigned int*)(buf + i) = (unsigned int)&g_skip_collisions;
    i += 4;
    buf[i++] = 0x00;

    /* JE to original */
    int je_pos = i;
    buf[i++] = 0x74; buf[i++] = 0x00;

    /* Skip: JMP to 0x4084F5 (end of loop 1) */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x4084F5 - ((unsigned int)cave_ai1 + i + 4);
    i += 4;

    /* Original: MOV ECX,[ESI+0x14]; ADD ECX,0x29D4 */
    int orig_pos = i;
    buf[je_pos + 1] = (unsigned char)(orig_pos - (je_pos + 2));

    buf[i++] = 0x8B; buf[i++] = 0x4E; buf[i++] = 0x14;
    buf[i++] = 0x81; buf[i++] = 0xC1;
    *(unsigned int*)(buf + i) = 0x29D4;
    i += 4;

    /* JMP to 0x4083E2 (CALL AthenaList_NextIndex) */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x4083E2 - ((unsigned int)cave_ai1 + i + 4);
    i += 4;

    WriteJump((void*)0x4083D9, cave_ai1);
    WriteNops((void*)0x4083DE, 4);
}

/* Patch E: Skip AI loop 2 (0x408548) when flag set */
static void BuildCaveAI2() {
    cave_ai2 = (unsigned char*)VirtualAlloc(NULL, 64, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_ai2, 0x90, 64);

    unsigned char* buf = cave_ai2;
    int i = 0;

    /* CMP DWORD [g_skip_collisions], 0 */
    buf[i++] = 0x83; buf[i++] = 0x3D;
    *(unsigned int*)(buf + i) = (unsigned int)&g_skip_collisions;
    i += 4;
    buf[i++] = 0x00;

    /* JE to original */
    int je_pos = i;
    buf[i++] = 0x74; buf[i++] = 0x00;

    /* Skip: JMP to 0x408663 (end of loop 2) */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x408663 - ((unsigned int)cave_ai2 + i + 4);
    i += 4;

    /* Original: ADD ECX,0x29D4 */
    int orig_pos = i;
    buf[je_pos + 1] = (unsigned char)(orig_pos - (je_pos + 2));

    buf[i++] = 0x81; buf[i++] = 0xC1;
    *(unsigned int*)(buf + i) = 0x29D4;
    i += 4;

    /* JMP to 0x40854E (continue after ADD) */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x40854E - ((unsigned int)cave_ai2 + i + 4);
    i += 4;

    WriteJump((void*)0x408548, cave_ai2);
}

/* Patch F: Hook SpatialTree_ctor entry (0x463330) — NEW in v8
 * __thiscall: ECX = this (allocated memory), 1 stack param, RET 0x4.
 * Original 7 bytes: 6A FF 68 48 D1 4C 00 (PUSH -1; PUSH 0x4CD148)
 *
 * When flag set: set vtable pointer at [ECX] and return ECX.
 * The tree has no triangles → all collision checks find nothing.
 * SpatialTree_Free works (just sets vtable + CollisionObj_Init). */
static void BuildCaveSTree() {
    cave_stree = (unsigned char*)VirtualAlloc(NULL, 128, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_stree, 0x90, 128);

    unsigned char* buf = cave_stree;
    int i = 0;

    /* CMP DWORD [g_skip_collisions], 0 */
    buf[i++] = 0x83; buf[i++] = 0x3D;
    *(unsigned int*)(buf + i) = (unsigned int)&g_skip_collisions;
    i += 4;
    buf[i++] = 0x00;

    /* JE to "original" path */
    int je_pos = i;
    buf[i++] = 0x74; buf[i++] = 0x00;  /* JE — fill offset later */

    /* Skip path: minimal init — set vtable pointer, return ECX */
    /* MOV DWORD [ECX], 0x4D9038 — SpatialTree vtable */
    buf[i++] = 0xC7; buf[i++] = 0x01;
    *(unsigned int*)(buf + i) = 0x004D9038;
    i += 4;

    /* MOV EAX, ECX — return this pointer */
    buf[i++] = 0x8B; buf[i++] = 0xC1;

    /* RET 0x4 — callee cleans 1 param */
    buf[i++] = 0xC2; buf[i++] = 0x04; buf[i++] = 0x00;

    /* Original path: execute PUSH -1; PUSH 0x4CD148; JMP to 0x463337 */
    int orig_pos = i;
    buf[je_pos + 1] = (unsigned char)(orig_pos - (je_pos + 2));

    /* PUSH -1 */
    buf[i++] = 0x6A; buf[i++] = 0xFF;

    /* PUSH 0x4CD148 */
    buf[i++] = 0x68;
    *(unsigned int*)(buf + i) = 0x004CD148;
    i += 4;

    /* JMP to 0x463337 (continue to MOV EAX, FS:[0]) */
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x463337 - ((unsigned int)cave_stree + i + 4);
    i += 4;

    WriteJump((void*)0x463330, cave_stree);
    WriteNops((void*)0x463335, 2);  /* NOP bytes 6-7 (rest of PUSH 0x4CD148) */
}

/* --- Patch thread --- */

static BOOL patched = FALSE;

static DWORD WINAPI PatchThread(LPVOID lpParam) {
    Sleep(2000);  /* Wait for game to fully load */

    /* Patch C: operator_new → return NULL */
    if (VerifyBytes((void*)0x4BA58D, orig_opnew, 5)) {
        static const unsigned char patch[] = {0x33, 0xC0, 0x5E, 0xC3, 0x90};
        WriteBytes((void*)0x4BA58D, patch, 5);
    }

    /* Patch A: Hook Scene_UpdateBallsAndState */
    if (VerifyBytes((void*)0x41B540, orig_scene, 5)) {
        BuildCaveScene();
    }

    /* Patch B: Hook Mesh_FindClosestCollision */
    if (VerifyBytes((void*)0x465D90, orig_mesh, 6)) {
        BuildCaveMesh();
    }

    /* Patch D: AI loop 1 */
    if (VerifyBytes((void*)0x4083D9, orig_ai1, 9)) {
        BuildCaveAI1();
    }

    /* Patch E: AI loop 2 */
    if (VerifyBytes((void*)0x408548, orig_ai2, 6)) {
        BuildCaveAI2();
    }

    /* Patch F: SpatialTree_ctor (NEW v8 — catches Ball_FallUpdate freeze) */
    if (VerifyBytes((void*)0x463330, orig_stree, 7)) {
        BuildCaveSTree();
    }

    patched = TRUE;
    return 0;
}

/* --- Cleanup on unload --- */
static void RemovePatches() {
    if (!patched) return;

    WriteBytes((void*)0x4BA58D, orig_opnew, 5);
    WriteBytes((void*)0x41B540, orig_scene, 5);
    WriteBytes((void*)0x465D90, orig_mesh, 6);
    WriteBytes((void*)0x4083D9, orig_ai1, 9);
    WriteBytes((void*)0x408548, orig_ai2, 6);
    WriteBytes((void*)0x463330, orig_stree, 7);

    if (cave_scene) { VirtualFree(cave_scene, 0, MEM_RELEASE); cave_scene = NULL; }
    if (cave_mesh)  { VirtualFree(cave_mesh,  0, MEM_RELEASE); cave_mesh  = NULL; }
    if (cave_ai1)   { VirtualFree(cave_ai1,   0, MEM_RELEASE); cave_ai1   = NULL; }
    if (cave_ai2)   { VirtualFree(cave_ai2,   0, MEM_RELEASE); cave_ai2   = NULL; }
    if (cave_stree) { VirtualFree(cave_stree, 0, MEM_RELEASE); cave_stree = NULL; }

    patched = FALSE;
}

/* --- DLL Entry Point --- */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hinstDLL);
            CreateThread(NULL, 0, PatchThread, NULL, 0, NULL);
            break;
        case DLL_PROCESS_DETACH:
            RemovePatches();
            break;
    }
    return TRUE;
}
