// WobblyBridgeFix.cpp — HB+ version (MSVC) of WobblyBridgeFix
// Fixes invisible wavy bridge in Wobbly Race (Level 7)
// Same root cause as bass.dll version: VB Lock DISCARD(0x2000) at 0x440752
#include "HamsterballAPI.h"
#include <windows.h>
#include <string.h>

class WobblyBridgeFix : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
    bool patched = false;
    static constexpr DWORD PATCH_ADDR = 0x00440752;
    // 68 00 20 00 00 = PUSH 0x2000 → 68 00 00 00 00 = PUSH 0
    static constexpr BYTE PATCH_ON[5]  = {0x68, 0x00, 0x00, 0x00, 0x00};
    static constexpr BYTE PATCH_OFF[5] = {0x68, 0x00, 0x20, 0x00, 0x00};

    bool writePatch(const BYTE* bytes) {
        // Prefer HB+ PatchMemory if available, fallback to VirtualProtect
        if (api) {
            api->PatchMemory(PATCH_ADDR, (const char*)bytes, 5);
            return true;
        }
        DWORD old;
        if (!VirtualProtect((void*)PATCH_ADDR, 5, PAGE_EXECUTE_READWRITE, &old)) return false;
        memcpy((void*)PATCH_ADDR, bytes, 5);
        VirtualProtect((void*)PATCH_ADDR, 5, old, &old);
        FlushInstructionCache(GetCurrentProcess(), (void*)PATCH_ADDR, 5);
        return true;
    }

public:
    const char* GetModName() override { return "Wobbly Bridge Fix"; }
    const char* GetAuthorName() override { return "RodentRacer"; }
    const char* GetContributors() override { return "Hamsterbot"; }
    int GetApiVersion() override { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;
        // Toggle button so user can verify / disable if needed
        CustomButton btn("WOBBLY_FIX", "Wobbly Bridge Fix");
        btn.defaultState = true;
        api->CreateToggleButton(btn, this);
        // Auto-apply if default is on — onButtonToggle is NOT called for saved-state on load,
        // so we patch here unconditionally and the button will keep state in sync.
        writePatch(PATCH_ON);
        patched = true;
    }

    void onButtonToggle(const char* buttonId, bool newState) override {
        if (strcmp(buttonId, "WOBBLY_FIX") != 0) return;
        if (newState && !patched) {
            writePatch(PATCH_ON);
            patched = true;
        } else if (!newState && patched) {
            writePatch(PATCH_OFF);
            patched = false;
        }
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new WobblyBridgeFix();
}
