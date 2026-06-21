
// ============================================================
// Hamsterball Entity Limit Fixer - bass.dll proxy v4
// Exports all BASS functions as stubs + delayed patching
// ============================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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

/* ---- Entity Limit Fixer ---- */

static BOOL patched = FALSE;

static unsigned char orig_A[] = {0x8B, 0x8E, 0x14, 0x00, 0x00, 0x00};
static unsigned char orig_B[] = {0x8B, 0x4E, 0x14, 0x81, 0xC1, 0xD4, 0x29, 0x00, 0x00};
static unsigned char orig_C[] = {0x81, 0xEC, 0x84, 0x00, 0x00, 0x00};

static const int MAX_BALLS = 30;
static volatile int frame_counter = 0;
static const int RESPAWN_THROTTLE = 3;

static unsigned char* cave_a = NULL;
static unsigned char* cave_b = NULL;
static unsigned char* cave_c = NULL;

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
    VirtualProtect(addr, count, PAGE_EXECUTE_READ, &old);
    BOOL match = (memcmp(addr, expected, count) == 0);
    VirtualProtect(addr, count, old, &old);
    return match;
}

// Cave A: Skip CollisionNode creation when ball count > MAX_BALLS
static void BuildAndPatchCaveA() {
    cave_a = (unsigned char*)VirtualAlloc(NULL, 256, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_a, 0x90, 256);
    
    unsigned char* buf = cave_a;
    int i = 0;
    // push eax
    buf[i++] = 0x50;
    // mov eax, [esi+0x14]
    buf[i++] = 0x8B; buf[i++] = 0x46; buf[i++] = 0x14;
    // test eax, eax
    buf[i++] = 0x85; buf[i++] = 0xC0;
    // jz skip
    int jz_pos = i;
    buf[i++] = 0x74; buf[i++] = 0x00;
    // mov eax, [eax+0x29D8]
    buf[i++] = 0x8B; buf[i++] = 0x80; buf[i++] = 0xD8; buf[i++] = 0x29; buf[i++] = 0x00; buf[i++] = 0x00;
    // cmp eax, MAX_BALLS
    buf[i++] = 0x83; buf[i++] = 0xF8; buf[i++] = (unsigned char)MAX_BALLS;
    // jg skip
    int jg_pos = i;
    buf[i++] = 0x7F; buf[i++] = 0x00;
    // pop eax (restore original EAX)
    buf[i++] = 0x58;
    // Original: mov ecx, [esi+0x14]
    buf[i++] = 0x8B; buf[i++] = 0x8E; buf[i++] = 0x14; buf[i++] = 0x00; buf[i++] = 0x00; buf[i++] = 0x00;
    // add ecx, 0x29D4
    buf[i++] = 0x81; buf[i++] = 0xC1; buf[i++] = 0xD4; buf[i++] = 0x29; buf[i++] = 0x00; buf[i++] = 0x00;
    // push ecx
    buf[i++] = 0x51;
    // mov ecx, eax
    buf[i++] = 0x89; buf[i++] = 0xC1;
    // call 0x00466CF0
    buf[i++] = 0xE8;
    *(int*)(buf + i) = 0x00466CF0 - ((unsigned int)cave_a + i + 4);
    i += 4;
    // jmp 0x00406924
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x00406924 - ((unsigned int)cave_a + i + 4);
    i += 4;
    // skip:
    int skip_label = i;
    // pop eax
    buf[i++] = 0x58;
    // xor eax, eax
    buf[i++] = 0x31; buf[i++] = 0xC0;
    // jmp 0x00406924
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x00406924 - ((unsigned int)cave_a + i + 4);
    i += 4;
    
    buf[jz_pos + 1] = (unsigned char)(skip_label - (jz_pos + 2));
    buf[jg_pos + 1] = (unsigned char)(skip_label - (jg_pos + 2));
    
    WriteJump((void*)0x40690F, cave_a);
    WriteNops((void*)0x406914, 1);
}

// Cave B: Skip AI target search when ball count > MAX_BALLS
static void BuildAndPatchCaveB() {
    cave_b = (unsigned char*)VirtualAlloc(NULL, 256, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_b, 0x90, 256);
    
    unsigned char* buf = cave_b;
    int i = 0;
    // push eax
    buf[i++] = 0x50;
    // mov eax, [esi+0x14]
    buf[i++] = 0x8B; buf[i++] = 0x46; buf[i++] = 0x14;
    // test eax, eax
    buf[i++] = 0x85; buf[i++] = 0xC0;
    // jz skip
    int jz_pos = i;
    buf[i++] = 0x74; buf[i++] = 0x00;
    // mov eax, [eax+0x29D8]
    buf[i++] = 0x8B; buf[i++] = 0x80; buf[i++] = 0xD8; buf[i++] = 0x29; buf[i++] = 0x00; buf[i++] = 0x00;
    // cmp eax, MAX_BALLS
    buf[i++] = 0x83; buf[i++] = 0xF8; buf[i++] = (unsigned char)MAX_BALLS;
    // jg skip
    int jg_pos = i;
    buf[i++] = 0x7F; buf[i++] = 0x00;
    // pop eax
    buf[i++] = 0x58;
    // Original: mov ecx, [esi+0x14]
    buf[i++] = 0x8B; buf[i++] = 0x8E; buf[i++] = 0x14; buf[i++] = 0x00; buf[i++] = 0x00; buf[i++] = 0x00;
    // add ecx, 0x29D4
    buf[i++] = 0x81; buf[i++] = 0xC1; buf[i++] = 0xD4; buf[i++] = 0x29; buf[i++] = 0x00; buf[i++] = 0x00;
    // push ecx
    buf[i++] = 0x51;
    // call 0x004532B0 (AthenaList_NextIndex)
    buf[i++] = 0xE8;
    *(int*)(buf + i) = 0x004532B0 - ((unsigned int)cave_b + i + 4);
    i += 4;
    // jmp 0x004083E7
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x004083E7 - ((unsigned int)cave_b + i + 4);
    i += 4;
    // skip:
    int skip_label = i;
    // pop eax
    buf[i++] = 0x58;
    // jmp 0x004084F5 (end of AI function)
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x004084F5 - ((unsigned int)cave_b + i + 4);
    i += 4;
    
    buf[jz_pos + 1] = (unsigned char)(skip_label - (jz_pos + 2));
    buf[jg_pos + 1] = (unsigned char)(skip_label - (jg_pos + 2));
    
    WriteJump((void*)0x4083D9, cave_b);
    WriteNops((void*)0x4083DE, 4);
}

// Cave C: Throttle respawn search
static void BuildAndPatchCaveC() {
    cave_c = (unsigned char*)VirtualAlloc(NULL, 256, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(cave_c, 0x90, 256);
    
    unsigned char* buf = cave_c;
    int i = 0;
    // push eax
    buf[i++] = 0x50;
    // push edx
    buf[i++] = 0x52;
    // mov eax, [frame_counter]
    buf[i++] = 0xA1;
    *(unsigned int*)(buf + i) = (unsigned int)&frame_counter;
    i += 4;
    // inc eax
    buf[i++] = 0x40;
    // mov [frame_counter], eax
    buf[i++] = 0xA3;
    *(unsigned int*)(buf + i) = (unsigned int)&frame_counter;
    i += 4;
    // xor edx, edx
    buf[i++] = 0x31; buf[i++] = 0xD2;
    // push RESPAWN_THROTTLE
    buf[i++] = 0x6A; buf[i++] = (unsigned char)RESPAWN_THROTTLE;
    // div [esp]
    buf[i++] = 0xF7; buf[i++] = 0x34; buf[i++] = 0x24;
    // add esp, 4
    buf[i++] = 0x83; buf[i++] = 0xC4; buf[i++] = 0x04;
    // test edx, edx
    buf[i++] = 0x85; buf[i++] = 0xD2;
    // jnz skip
    int jnz_pos = i;
    buf[i++] = 0x75; buf[i++] = 0x00;
    // pop edx
    buf[i++] = 0x5A;
    // pop eax
    buf[i++] = 0x58;
    // sub esp, 0x84 (original)
    buf[i++] = 0x81; buf[i++] = 0xEC; buf[i++] = 0x84; buf[i++] = 0x00; buf[i++] = 0x00; buf[i++] = 0x00;
    // jmp 0x00405196
    buf[i++] = 0xE9;
    *(int*)(buf + i) = 0x00405196 - ((unsigned int)cave_c + i + 4);
    i += 4;
    // skip:
    int skip_label = i;
    // pop edx
    buf[i++] = 0x5A;
    // pop eax
    buf[i++] = 0x58;
    // xor eax, eax
    buf[i++] = 0x31; buf[i++] = 0xC0;
    // ret 0x04
    buf[i++] = 0xC2; buf[i++] = 0x04; buf[i++] = 0x00;
    
    buf[jnz_pos + 1] = (unsigned char)(skip_label - (jnz_pos + 2));
    
    WriteJump((void*)0x405190, cave_c);
    WriteNops((void*)0x405195, 1);
}

static DWORD WINAPI PatchThread(LPVOID param) {
    Sleep(5000);
    
    if (!VerifyBytes((void*)0x40690F, orig_A, 6)) return 1;
    if (!VerifyBytes((void*)0x4083D9, orig_B, 9)) return 1;
    if (!VerifyBytes((void*)0x405190, orig_C, 6)) return 1;
    
    BuildAndPatchCaveA();
    BuildAndPatchCaveB();
    BuildAndPatchCaveC();
    
    patched = TRUE;
    return 0;
}

static void RemovePatches() {
    if (!patched) return;
    WriteBytes((void*)0x40690F, orig_A, 6);
    WriteBytes((void*)0x4083D9, orig_B, 9);
    WriteBytes((void*)0x405190, orig_C, 6);
    if (cave_a) { VirtualFree(cave_a, 0, MEM_RELEASE); cave_a = NULL; }
    if (cave_b) { VirtualFree(cave_b, 0, MEM_RELEASE); cave_b = NULL; }
    if (cave_c) { VirtualFree(cave_c, 0, MEM_RELEASE); cave_c = NULL; }
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
