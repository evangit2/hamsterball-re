#include "bass_proxy.h"

/* mknp_ignore_cache — disables the game's .cached mesh cache system.
 *
 * The cache master flag is a byte at Graphics + 0x7D1.
 * It is set to 1 once in App_Initialize_Full at 0x42956A:
 *   mov byte ptr [eax+0x7D1], 1
 * We patch that immediate to 0 so the flag is never turned on.
 *
 * The flag is also set to 0 in the Graphics constructor at 0x454532,
 * so the initial state is always 0 after patching.
 *
 * Two read sites check the flag:
 *   - 0x46F546: if flag=0, skip loading .cached files
 *   - 0x46F691: if flag=0, skip writing .cached files
 * Both already work correctly with the flag held at 0.
 */

#define ADDR_CACHE_ENABLE 0x0042956A

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    (void)hModule;
    (void)lpReserved;

    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        load_real_bass();

        /* Patch the single instruction that turns the cache flag on.
         * Original:  C6 80 D1 07 00 00 01  (mov byte ptr [eax+0x7D1], 1)
         * Patched:                        (mov byte ptr [eax+0x7D1], 0)
         * The immediate byte is at offset +6 from the instruction start. */
        DWORD old;
        VirtualProtect((void*)ADDR_CACHE_ENABLE, 7, PAGE_EXECUTE_READWRITE, &old);
        *(BYTE*)(ADDR_CACHE_ENABLE + 6) = 0; /* change immediate 1 -> 0 */
        VirtualProtect((void*)ADDR_CACHE_ENABLE, 7, old, &old);
        FlushInstructionCache(GetCurrentProcess(), (void*)ADDR_CACHE_ENABLE, 7);
    }
    return TRUE;
}