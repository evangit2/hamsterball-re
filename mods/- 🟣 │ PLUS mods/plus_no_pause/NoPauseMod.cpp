#include "HamsterballAPI.h"
#include <windows.h>

/* No Pause for HB+ v2.1
 *
 * Port of no_pause v2 (bass.dll proxy) to HB+ API.
 *
 * The proxy version simply patched 3 JZ/JNZ bytes in the EXE at load time.
 * We do the same 3 byte patches via PatchMemory from Initialize, gated by
 * a toggle button.
 */

#define PAUSE_PATCH_PATH1 0x19D5B
#define PAUSE_PATCH_PATH2 0x130B5
#define PAUSE_PATCH_PATH3 0x0B405

class NoPauseMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
    bool m_patched = false;
    BYTE m_origBytes[3] = {0};

    void patchByte(DWORD rva, BYTE newVal, BYTE* saved) {
        DWORD base = api->GetGameBaseAddress();
        BYTE* addr = (BYTE*)(base + rva);
        DWORD oldProtect;
        if (!VirtualProtect(addr, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) return;
        *saved = *addr;
        *addr = newVal;
        VirtualProtect(addr, 1, oldProtect, &oldProtect);
        FlushInstructionCache(GetCurrentProcess(), addr, 1);
    }

    void restoreByte(DWORD rva, BYTE val) {
        DWORD base = api->GetGameBaseAddress();
        BYTE* addr = (BYTE*)(base + rva);
        DWORD oldProtect;
        if (!VirtualProtect(addr, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) return;
        *addr = val;
        VirtualProtect(addr, 1, oldProtect, &oldProtect);
        FlushInstructionCache(GetCurrentProcess(), addr, 1);
    }

public:
    const char* GetModName() override      { return "No Pause"; }
    const char* GetAuthorName() override   { return "RodentRacer"; }
    const char* GetContributors() override { return "Hamsterbot"; }
    int GetApiVersion() override           { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;

        CustomButton btn("NO_PAUSE", "Disable Pause");
        btn.defaultState = true;
        modApi->CreateToggleButton(btn, this);

        // Apply patches immediately at launch.
        applyPatches();
    }

    void applyPatches() {
        if (m_patched) return;
        patchByte(PAUSE_PATCH_PATH1, 0xEB, &m_origBytes[0]); // JZ -> JMP
        patchByte(PAUSE_PATCH_PATH2, 0xEB, &m_origBytes[1]); // JZ -> JMP
        patchByte(PAUSE_PATCH_PATH3, 0xEB, &m_origBytes[2]); // JNZ -> JMP
        m_patched = true;
    }

    void removePatches() {
        if (!m_patched) return;
        restoreByte(PAUSE_PATCH_PATH1, m_origBytes[0]);
        restoreByte(PAUSE_PATCH_PATH2, m_origBytes[1]);
        restoreByte(PAUSE_PATCH_PATH3, m_origBytes[2]);
        m_patched = false;
    }

    void onButtonToggle(const char* buttonId, bool newState) override {
        if (strcmp(buttonId, "NO_PAUSE") != 0) return;
        if (newState) applyPatches();
        else removePatches();
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new NoPauseMod();
}
