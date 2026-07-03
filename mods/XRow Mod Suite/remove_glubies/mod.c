/*
 * "remove glubies"
 * Auto-generated from XRow's Cheat Engine table (Hamsterball_3.6.C_MOD.CT)
 * Type: Pure byte patches (no allocs, no code caves)
 * Patches: 1
 *
 * Build: i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32
 * Install: Replace bass.dll in game directory (rename original to bass_real.dll first)
 */

#include <windows.h>

#define GAME_BASE 0x400000

static HANDLE g_Thread = NULL;
static int g_Patched = 0;

typedef struct {
    DWORD addr;
    DWORD size;
    BYTE* enable_bytes;
    BYTE* disable_bytes;
    int has_disable;
} PatchEntry;

static BYTE enable_bytes_0[] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
static BYTE disable_bytes_0[] = { 0x47, 0x4C, 0x55, 0x45, 0x42 };

static PatchEntry patches[] = {
    { 0x004CF680, 5, enable_bytes_0, disable_bytes_0, 1 },
};
#define NUM_PATCHES 1

static void ApplyPatches(void) {
    DWORD oldProtect;
    for (int i = 0; i < NUM_PATCHES; i++) {
        VirtualProtect((void*)patches[i].addr, patches[i].size, PAGE_EXECUTE_READWRITE, &oldProtect);
        memcpy((void*)patches[i].addr, patches[i].enable_bytes, patches[i].size);
        VirtualProtect((void*)patches[i].addr, patches[i].size, oldProtect, &oldProtect);
    }
    g_Patched = 1;
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
