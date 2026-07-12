/*
 * ignore_cache — bass.dll proxy
 * Forces the game to always parse .MESHWORLD text files, ignoring .cached binary files
 *
 * Auto-generated from CEA script.
 */

#include "bass_proxy.h"

static DWORD WINAPI patch_thread(LPVOID param) {
    Sleep(2000);
    load_real_bass();
    patch_byte(0x45DE77, 0xEB);
    patch_byte(0x4717DB, 0xEB);
    BYTE patch_46F67D[] = {0xE9, 0xD0, 0x00, 0x00, 0x00, 0x90};
    patch_bytes(0x46F67D, patch_46F67D, 6);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
    }
    return TRUE;
}
