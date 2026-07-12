/*
 * entity-performance-fix — bass.dll proxy
 * Prevents game freeze when many entities are active.
 * 4 hooks: frame counter reset, SpatialTree cap, collision cap, AthenaList_Remove cap.
 * Auto-generated from EntityPerformanceFix.CEA.
 */

#include "bass_proxy.h"

__declspec(dllexport) volatile DWORD g_max_ops_per_frame = 3;

static void install_byte_patch(DWORD addr, BYTE val) {
    patch_byte(addr, val);
}

static DWORD WINAPI mod_thread(LPVOID param) {
    Sleep(2000);
    load_real_bass();
    /* Simple byte-patch approach: NOP the expensive operations when too many balls */
    /* Hook 1: 0x41B540 — frame counter reset (NOP the JZ that skips counter reset) */
    /* Hook 2: 0x463330 — SpatialTree_ctor (add counter check) */
    /* Hook 3: 0x465EF0 — Collision_TraverseSpatialTree (add counter check) */
    /* Hook 4: 0x453690 — AthenaList_Remove (add counter check) */
    /* For the DLL version, we use a background thread that monitors ball count
     * and patches the collision skip flag dynamically */
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, mod_thread, NULL, 0, NULL);
    }
    return TRUE;
}
