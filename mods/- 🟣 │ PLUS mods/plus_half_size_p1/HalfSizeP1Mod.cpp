#include "HamsterballAPI.h"
#include <windows.h>
#include <stdio.h>

/* Half-Size Player 1 for HB+ v2.1
 *
 * Port of half_size_p1 (bass.dll proxy) to HB+ API.
 *
 * Simplifications vs bass.dll:
 * - No BASS proxy shell
 * - Uses HB+ API ApplyForce or direct Ball struct writes? The proxy version
 *   patched the radius writes at Ball_ctor2 and Scene_SpawnBallsAndObjects.
 *   We can't patch the same way without raw code patches, but we CAN still
 *   patch memory from Initialize via PatchMemory. This mod keeps the exact
 *   same 2 code caves + 2 patch sites, just wrapped as an HB+ mod.
 */

#define IMAGE_BASE 0x00400000

#define PATCH1_ADDR 0x00403C8B
static const BYTE PATCH1_ORIG[10] = {
    0xC7, 0x86, 0x84, 0x02, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x41
};

#define PATCH2_ADDR 0x0041C8AA
static const BYTE PATCH2_ORIG[10] = {
    0xC7, 0x86, 0x84, 0x02, 0x00, 0x00, 0x00, 0x00, 0xD0, 0x41
};

static const BYTE CAVE1[32] = {
    0x83, 0xBE, 0x18, 0x00, 0x00, 0x00, 0x00,
    0x75, 0x0C,
    0xC7, 0x86, 0x84, 0x02, 0x00, 0x00,
    0x00, 0x00, 0x58, 0x41,
    0xEB, 0x0A,
    0xC7, 0x86, 0x84, 0x02, 0x00, 0x00,
    0x00, 0x00, 0xD8, 0x41,
    0xC3
};

static const BYTE CAVE2[32] = {
    0x83, 0xBE, 0x18, 0x00, 0x00, 0x00, 0x00,
    0x75, 0x0C,
    0xC7, 0x86, 0x84, 0x02, 0x00, 0x00,
    0x00, 0x00, 0x50, 0x41,
    0xEB, 0x0A,
    0xC7, 0x86, 0x84, 0x02, 0x00, 0x00,
    0x00, 0x00, 0xD0, 0x41,
    0xC3
};

class HalfSizeP1Mod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
    bool m_patched = false;

    void* alloc_cave(SIZE_T size) {
        void* cave = VirtualAlloc((void*)(IMAGE_BASE + 0xF8000), size,
                                  MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (cave) return cave;
        SYSTEM_INFO si; GetSystemInfo(&si);
        SIZE_T page_size = si.dwPageSize;
        SIZE_T alloc_size = ((size + page_size - 1) / page_size) * page_size;
        for (DWORD_PTR addr = 0x500000; addr < 0x800000; addr += page_size) {
            cave = VirtualAlloc((void*)addr, alloc_size,
                               MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (cave) return cave;
        }
        return VirtualAlloc(NULL, alloc_size, MEM_COMMIT | MEM_RESERVE,
                           PAGE_EXECUTE_READWRITE);
    }

    bool write_bytes(BYTE* addr, const BYTE* data, SIZE_T len) {
        DWORD oldProtect;
        if (!VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect)) return false;
        memcpy(addr, data, len);
        VirtualProtect(addr, len, oldProtect, &oldProtect);
        FlushInstructionCache(GetCurrentProcess(), addr, len);
        return true;
    }

    bool patch_site(BYTE* site, const BYTE* expected, void* cave_addr, SIZE_T len) {
        if (memcmp(site, expected, len) != 0) return false;
        ptrdiff_t rel = (ptrdiff_t)((BYTE*)cave_addr - (site + 5));
        if (rel > 0x7FFFFFFF || rel < (ptrdiff_t)0x80000000) return false;
        BYTE replacement[10];
        replacement[0] = 0xE8;
        memcpy(replacement + 1, &rel, 4);
        for (int i = 5; i < (int)len; i++) replacement[i] = 0x90;
        return write_bytes(site, replacement, len);
    }

public:
    const char* GetModName() override      { return "Half-Size Player 1"; }
    const char* GetAuthorName() override   { return "RodentRacer"; }
    const char* GetContributors() override { return "Hamsterbot"; }
    int GetApiVersion() override           { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;

        CustomButton btn("HALF_SIZE_P1", "Half-Size Player 1");
        btn.defaultState = true;
        modApi->CreateToggleButton(btn, this);
    }

    void onButtonToggle(const char* buttonId, bool newState) override {
        if (strcmp(buttonId, "HALF_SIZE_P1") != 0) return;
        HMODULE hExe = GetModuleHandleA(NULL);
        if (!hExe) return;
        BYTE* base = (BYTE*)hExe;

        if (newState && !m_patched) {
            void* cave_mem = alloc_cave(4096);
            if (!cave_mem) return;
            BYTE* cave1_addr = (BYTE*)cave_mem;
            BYTE* cave2_addr = (BYTE*)cave_mem + 32;
            if (!write_bytes(cave1_addr, CAVE1, sizeof(CAVE1))) return;
            if (!write_bytes(cave2_addr, CAVE2, sizeof(CAVE2))) return;
            bool ok1 = patch_site(base + (PATCH1_ADDR - IMAGE_BASE), PATCH1_ORIG, cave1_addr, 10);
            bool ok2 = patch_site(base + (PATCH2_ADDR - IMAGE_BASE), PATCH2_ORIG, cave2_addr, 10);
            if (ok1 || ok2) m_patched = true;
        } else if (!newState && m_patched) {
            // Restore original bytes
            write_bytes(base + (PATCH1_ADDR - IMAGE_BASE), PATCH1_ORIG, 10);
            write_bytes(base + (PATCH2_ADDR - IMAGE_BASE), PATCH2_ORIG, 10);
            m_patched = false;
        }
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new HalfSizeP1Mod();
}
