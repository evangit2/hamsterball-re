/*
 * Custom Mousetrap XYZ Force
 * Auto-generated from XRow's Cheat Engine table (Hamsterball_3.6.C_MOD.CT)
 * Type: Float + byte patches
 * Patches: 3
 *
 * Build: i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
 */

#include <windows.h>

#define GAME_BASE 0x400000

static HANDLE g_Thread = NULL;

/* RVA 0xCF370: XZ launch force (float) 0.0 = disabled */
static BYTE enable_cf370[] = { 0x00, 0x00, 0x00, 0x00 };
static BYTE disable_cf370[] = { 0x00, 0x00, 0xA0, 0x41 };

/* RVA 0xD109: redirect offset byte */
static BYTE enable_d109[] = { 0x04 };
static BYTE disable_d109[] = { 0x00 };

/* RVA 0xD10A: Y launch force (float) 30.0 */
static BYTE enable_d10a[] = { 0x00, 0x00, 0xF0, 0x41 };
static BYTE disable_d10a[] = { 0x85, 0xEB, 0x41, 0x41 };

typedef struct {
    DWORD addr;
    DWORD size;
    BYTE* enable_bytes;
    BYTE* disable_bytes;
} PatchEntry;

static PatchEntry patches[] = {
    { 0x004CF370, 4, enable_cf370, disable_cf370 },
    { 0x0040D109, 1, enable_d109, disable_d109 },
    { 0x0040D10A, 4, enable_d10a, disable_d10a },
};
#define NUM_PATCHES 3

static void ApplyPatches(void) {
    DWORD oldProtect;
    for (int i = 0; i < NUM_PATCHES; i++) {
        VirtualProtect((void*)patches[i].addr, patches[i].size, PAGE_EXECUTE_READWRITE, &oldProtect);
        memcpy((void*)patches[i].addr, patches[i].enable_bytes, patches[i].size);
        VirtualProtect((void*)patches[i].addr, patches[i].size, oldProtect, &oldProtect);
    }
}

static DWORD WINAPI PatchThread(LPVOID lpParam) {
    Sleep(2000);
    ApplyPatches();
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        g_Thread = CreateThread(NULL, 0, PatchThread, NULL, 0, NULL);
    }
    return TRUE;
}

/* BASS DLL proxy stubs */
__declspec(dllexport) void __stdcall BASS_Init() {}
__declspec(dllexport) void __stdcall BASS_Free() {}
__declspec(dllexport) void __stdcall BASS_Start() {}
__declspec(dllexport) void __stdcall BASS_Stop() {}
__declspec(dllexport) void __stdcall BASS_Pause() {}
__declspec(dllexport) void __stdcall BASS_SetVolume() {}
__declspec(dllexport) void __stdcall BASS_GetVolume() {}
__declspec(dllexport) void __stdcall BASS_SetConfig() {}
__declspec(dllexport) void __stdcall BASS_GetConfig() {}
__declspec(dllexport) void __stdcall BASS_MusicLoad() {}
__declspec(dllexport) void __stdcall BASS_MusicFree() {}
__declspec(dllexport) void __stdcall BASS_ChannelPlay() {}
__declspec(dllexport) void __stdcall BASS_ChannelStop() {}
__declspec(dllexport) void __stdcall BASS_ChannelSetAttribute() {}
__declspec(dllexport) void __stdcall BASS_ChannelGetAttribute() {}
__declspec(dllexport) void __stdcall BASS_ChannelGetPosition() {}
__declspec(dllexport) void __stdcall BASS_ChannelSetPosition() {}
__declspec(dllexport) void __stdcall BASS_ChannelIsActive() {}
__declspec(dllexport) void __stdcall BASS_ChannelGetLevel() {}
