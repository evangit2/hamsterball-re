#include "bass_proxy.h"

/* mknp_ignore_cache — disables the game's .cached mesh cache system.
 * The cache master flag is a byte at Graphics + 0x7D1.
 * It is set to 1 once in App_Initialize_Full at 0x42956A:
 *   mov byte ptr [eax+0x7D1], 1
 * We patch that immediate to 0 and also force the byte to 0 every frame
 * through the Present hook so nothing can re-enable it.
 */

#define ADDR_CACHE_ENABLE 0x0042956A

static void force_cache_off(void)
{
    /* g_App = 0x005341E0, App + 0x174 = Graphics pointer */
    DWORD app = *(DWORD*)0x005341E0;
    if (app && app >= 0x10000 && !IsBadReadPtr((void*)app, 4)) {
        DWORD gfx = *(DWORD*)(app + 0x174);
        if (gfx && gfx >= 0x10000 && !IsBadReadPtr((void*)(gfx + 0x7D1), 1)) {
            *(BYTE*)(gfx + 0x7D1) = 0;
        }
    }
}

/* 5-byte JMP hook at Graphics_PresentOrEnd (0x453330) */
#define GRAPHICS_PRESENT_HOOK 0x00453330
static BYTE s_present_orig[5];
static void* s_cave = NULL;

static void install_present_hook(void)
{
    s_cave = alloc_executable(64);
    if (!s_cave) return;

    BYTE* cave = (BYTE*)s_cave;
    int i = 0;

    /* call force_cache_off */
    cave[i++] = 0xE8;
    *(DWORD*)(cave + i) = (DWORD)force_cache_off - ((DWORD)cave + i + 4);
    i += 4;

    /* original 5 bytes */
    memcpy(cave + i, (void*)GRAPHICS_PRESENT_HOOK, 5);
    i += 5;

    /* jmp back to original + 5 */
    cave[i++] = 0xE9;
    *(DWORD*)(cave + i) = (GRAPHICS_PRESENT_HOOK + 5) - ((DWORD)cave + i + 4);
    i += 4;

    /* save original */
    memcpy(s_present_orig, (void*)GRAPHICS_PRESENT_HOOK, 5);

    /* install hook */
    install_jmp_hook(GRAPHICS_PRESENT_HOOK, (DWORD)s_cave);
}

static void uninstall_present_hook(void)
{
    if (s_cave) {
        patch_bytes(GRAPHICS_PRESENT_HOOK, s_present_orig, 5);
        VirtualFree(s_cave, 0, MEM_RELEASE);
        s_cave = NULL;
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    (void)hModule;
    (void)lpReserved;

    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        load_real_bass();

        /* Patch the single instruction that turns the cache flag on. */
        DWORD old;
        VirtualProtect((void*)ADDR_CACHE_ENABLE, 7, PAGE_EXECUTE_READWRITE, &old);
        *(BYTE*)(ADDR_CACHE_ENABLE + 6) = 0; /* change immediate 1 -> 0 */
        VirtualProtect((void*)ADDR_CACHE_ENABLE, 7, old, &old);
        FlushInstructionCache(GetCurrentProcess(), (void*)ADDR_CACHE_ENABLE, 7);

        install_present_hook();
    }
    else if (reason == DLL_PROCESS_DETACH) {
        uninstall_present_hook();
    }
    return TRUE;
}
